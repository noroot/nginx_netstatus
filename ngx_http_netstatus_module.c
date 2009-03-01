/*
 * nginx network status module
 * show network connections from lsof output
 *
 * @author Dmitry R <public@falsetrue.io> (c) 2009
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static char *ngx_http_set_netstatus(ngx_conf_t *cf, ngx_command_t *cmd,
                                 void *conf);

static ngx_command_t  ngx_http_netstatus_commands[] = {

    { ngx_string("netstatus"),
      NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
      ngx_http_set_netstatus,
      0,
      0,
      NULL },

      ngx_null_command
};



static ngx_http_module_t  ngx_http_netstatus_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    NULL,                                  /* create location configuration */
    NULL                                   /* merge location configuration */
};


ngx_module_t  ngx_http_netstatus_module = {
    NGX_MODULE_V1,
    &ngx_http_netstatus_module_ctx,      /* module context */
    ngx_http_netstatus_commands,              /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t ngx_http_netstatus_handler(ngx_http_request_t *r)
{
    size_t             size;
    ngx_int_t          rc;
    ngx_buf_t         *b;
    ngx_chain_t        out;
    ngx_atomic_int_t   ap, hn, ac, rq, rd, wr;

    if (r->method != NGX_HTTP_GET && r->method != NGX_HTTP_HEAD) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *) "text/plain";
    
    if (r->method == NGX_HTTP_HEAD) {
        r->headers_out.status = NGX_HTTP_OK;

        rc = ngx_http_send_header(r);

        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
        }
    }

    size = 100000;
    
    b = ngx_create_temp_buf(r->pool, size);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out.buf = b;
    out.next = NULL;
    
    ap = *ngx_stat_accepted;
    hn = *ngx_stat_handled;
    ac = *ngx_stat_active;
    rq = *ngx_stat_requests;
    rd = *ngx_stat_reading;
    wr = *ngx_stat_writing;

    b->last = ngx_sprintf(b->last, CRLF "network connections: %uA (%uA, %uA, %uA)" CRLF, ac, rd, wr, ac - (rd - wr) );
    b->last = ngx_sprintf(b->last, "-------------------" CRLF);

    FILE * pf;
    char pbuff[1024];
    
    system("lsof -i tcp -P -n | grep nginx > /var/tmp/nginx.stats");
    
    pf = fopen ("/var/tmp/nginx.stats" , "r");
    if (pf == NULL) perror ("Error opening file");
    else
    {
        while ((fgets(pbuff, 1023, pf)) != (char *) 0)
        {
            b->last = ngx_sprintf(b->last, "%s", pbuff);
        }
        fclose (pf);
    }


    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = b->last - b->pos;

    b->last_buf = 1;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out);
}


static char *ngx_http_set_netstatus(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_netstatus_handler;

    return NGX_CONF_OK;
}
