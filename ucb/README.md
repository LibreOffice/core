# Universal Content Broker (UCB)

Universal Content Broker (has ucps) which do things like convert files
to strings in content broker world, or connect LibreOffice with
various DMS and fileshare systems like WebDAV, CMIS, or GIO.

The UCPs implement the Universal Content Provider UNO interfaces in
C++, in particular the `com.sun.star.ucb.ContentProvider` service.

## WebDAV UCP

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
  "100 Continue".
* Sharepoint 16 responds to PROPFIND, PROPPATCH and LOCK with
  "Transfer-Encoding: chunked"
  with "HTTP/1.1 200 OK" and an actual error message in the response *body*.
  * apparently setting Content-Length works better, so we use that
* Sharepoint returns redirect urls that curl can't parse, so we encode
  them (check `WebDAVResponseParser` for the code)
* Sharepoint may reply to HEAD with 200 OK but then 404 NOT FOUND to PROPFIND
* Sharepoint does not appear to support Dead Properties
* avoiding chunked encoding for PUT, since for Nextcloud:
  * Transfer-Encoding: chunked creates a 0 byte file with response
    "201 Created"
  * see upstream bug: https://github.com/nextcloud/server/issues/7995

To a first approximation, there are 3 parts involved in the UCP:

* The upper layer implements the UNO API which is called by LibreOffice, and
  translates the calls from generic sequence-of-any stringly typed abstractness
  into HTTP or WebDAV protocol calls, and does some high level protocol
  handling to figure out what the server supports and so on. This is
  independent of the low-level library.
* Then there is the lower layer of the UCP, which translates the generic HTTP
  or WebDAV protocol calls to something that the particular third-party library
  can understand, hook up its callbacks for data transfer and authentication,
  and parse the reply XML documents.
* At the bottom, there is the third-party library that implements the HTTP
  protocol.

The most important classes are:
* ContentProvider: the UNO entry point/factory, creates Content instances
* Content: the main UNO service, translates the UCP API to WebDAV methods,
  one instance per URL
* DAVResourceAccess: sits between Content and CurlSession
* DAVSessionFactory: creates CurlSession for DAVResourceAccess
* CurlSession: low-level interfacing with libcurl
* SerfLockStore: singleton used by CurlSession to store DAV lock tokens, runs
  a thread to refresh locks when they expire
* WebDAVResponseParser: parse XML responses to LOCK, PROPFIND requests
* DAVAuthListener_Impl: request credentials from UI via UNO

