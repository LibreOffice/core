# Universal Content Broker (UCB)

Universal Content Broker (has ucps) which do things like convert files
to strings in content broker world, or connect LibreOffice with
various DMS and fileshare systems like WebDAV, CMIS, or GIO.

The UCPs implement the Universal Content Provider UNO interfaces in
C++, in particular the `com.sun.star.ucb.ContentProvider` service.

## WebDAV ucp

The WebDAV content provider is based on `libcurl` for much of the
network and protocol stuff, including authentication.

WebDAV as implemented here is defined in an IETF RFC 4918 extensions,
and the code supports both unencrypted HTTP/1.1 (IETF RFC 2616) as
well as TLS 1.2 or later.

Our WebDAV `com.sun.star.ucb.ContentProvider` service implementation
registers the `vnd.sun.star.webdav` and `http` URI schemes (and their
encrypted TLS variants). See here for the specification:
https://wiki.openoffice.org/wiki/Documentation/DevGuide/AppendixC/The_WebDAV_Content_Provider

Historically, webdav had two ucps, one based on `neon`, the second one
based on `serf`. Both are superseded by the current `libcurl`
implementation (since LibreOffice 7.3), but in case of behavioural
differences, go hunt for code differences (ucb/source/ucp/webdav-neon
and ucb/source/ucp/webdav).

The WebDAV protocol is implemented on top of libcurl basic http GET,
PUT, and POST requests (and is relatively straight-forward - see
`ucb/source/ucp/webdav-curl/webdavcontent.cxx` for the main
functionality), but incorporates custom handling for a number of
server idiosyncrasies:

* Nextcloud will reply to a PROPFIND request with "100 Continue" and
  then after the data is uploaded it will send a "401 Unauthorized" if
  the auth header is missing in the headers to which it replied with
  "100 Continue". So we reuse that.
* Sharepoint 16 responds to PROPFIND with "Transfer-Encoding: chunked"
  with "HTTP/1.1 200 OK" and an actual error message in the response *body*.
  * apparently setting Content-Length works better, so we use that
  * Sharepoint 16 has the same problem with PROPFIND.
  * and for when using chunked encoding for LOCK
* Sharepoint returns redirect urls that curl can't parse, so we encode
  them (check `WebDAVResponseParser` for the code)
* avoiding chunked encoding for PUT, since for Nextcloud:
  * Transfer-Encoding: chunked creates a 0 byte file with response
    "201 Created"
  * see upstream bug: https://github.com/nextcloud/server/issues/7995
  * apparently this doesn't happen with header Content-Length: 8347,
    so we use that

