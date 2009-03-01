# Nginx NetStatus Modules (Legacy) 

**nginx_netstatus** - nginx webserver module to show network connection status.

This is my legacy project which mimics Aparch `/server-status` page. 
There is time in past when nginx does't have server-status page, so I decide to write my own nginx module for that.
Logic is very simple, it parses `lsof` output `grep` **nginx** and store results into file, then read file and show results on specified uri.
This approach is very rudiment and probablty will not work with the current nginx API. (it was made with 0.6 or 0.7 version)

## [*] Setup 

Download nginx sources from https://github.com/nginx/nginx

Clone nginx_netstatus reposity

Compile nginx with nginx_netstatus

```
$ ./configure --add-module={path to nginx_netstatus}
$ make && make install

```

## [*] Configuration

```
location /netstats$ {
  netstatus on;
}

```
 
## [*] Usage
 
Run nginx
 
open in browser http://127.0.0.1:8080/netstats 

-----------------
thanks for usage, any questions and improvment requests send to email.
 
// Legacy 
