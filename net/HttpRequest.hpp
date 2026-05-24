/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Asynchronous HTTP/1.1 client implementation with request/response handling.
 * Classes: http::Session, http::Request, http::Response, http::Header
 */

#pragma once

#include <common/Common.hpp>
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/StateEnum.hpp>
#include <common/StringVector.hpp>
#include <common/Util.hpp>
#include <net/NetUtil.hpp>
#include <net/Socket.hpp>
#include <net/Uri.hpp>

#if ENABLE_SSL
#include <net/SslSocket.hpp>
#endif

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ios>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#if !MOBILEAPP
#include <sys/socket.h>
#include <netdb.h>
#endif

// This is a partial implementation of RFC 7230
// and its related RFCs, with focus on the core
// HTTP/1.1 messaging of GET and POST requests.
//
// There is no attempt to support all possible
// features of the RFC. However, we do attempt
// to be maximally compatible and accommodating
// to server and client implementations in the
// wild. This code is designed to work primarily
// on the client side, with provision for being
// on the server side as well.
//
// Design Principles:
// -----------------
//
// The goal here is primarily to have an async
// HTTP implementation that is performant.
// To that end, the design is modular and with
// well-defined state-machine for each component.
// To support async requests, an instance of
// SocketPoll is associated with http::Session,
// and events are triggered via callbacks.
// Clients interested in acting on the results
// of their http::Request should register
// callbacks and implement the necessary
// continuation logic. For working examples
// see HttpRequestTests.cpp.
//
// Code Structure:
// ---------------
//
// http::Session is the primary class that is
// used for all requests. It is created for a
// specific host. Because http::Session is
// designed to be reusable, the connection itself
// can also be reused, improving efficiency.
//
// An http::Session takes an http::Request and,
// only after initiating a new request, access
// to the current/last request's response is
// accessible via http::Session::response().
//
// To make a request, an http::Request is created
// with a given URL and any necessary header
// entries are set via the http::Header member.
// A request is made by simply passing an
// http::Request instance to an http::Session
// instance's asyncRequest (or syncRequest).
//
// http::Response is the server's response.
// It communicates the readiness of the response
// via a well-defined set of states, which
// can be introspected at any time.
// http::Response::done() returns true when the
// request is completed, regardless of the
// outcome, which must be checked via the
// http::Response::state() member.
//
// A timeout can be set on the http::Session
// instance, which, when hit, will disconnect
// and fire the onFinished handler. The client
// should introspect the state of the
// http::Response, which is reached via the
// http::Session argument's response() member.
//
// To upload or download files, convenient
// API exists.
// To upload a file via POST, http::Request
// supports setting the file path via
// setBodyFile(), or setBodySource() which is
// a generic callback to provide the payload
// data to upload, whereupon the file contents
// will be set in the body of the POST request
// message.
// To download a file via GET, http::Response
// has saveBodyToFile() member that accepts
// a path to file where the response body
// is stored. Similarly, saveBodyToMemory will
// store the body as a string to be retrieved
// via getBody(). saveBodyToHandler() is
// also provided as a generic handler.
// Note that when the response status code
// is anything but success (i.e. not 200 status
// code), the body is instead stored in memory
// to be read via getBody(). The state of
// the http::Response will indicate success
// or failure.
// Incidentally, http::Response exposes the
// response status line via statusLine(),
// which contains the status code, reason,
// and a convenient status category.
//
// Finally, if a synchronous request is needed,
// http::Session provides syncRequest that
// blocks until the request completes. However,
// the onFinished callback is still triggered
// as expected and all other APIs behave as
// with the async case. Indeed, internally
// the async logic is used, which
// guarantees consistency.

namespace http
{
static constexpr int64_t MaxBodyLen = 2LL * 1024 * 1024 * 1024; ///< Prevent runaway cases.
static constexpr int64_t MaxChunkLen = 128LL * 1024 * 1024; ///< Prevent runaway cases.

/// The parse-state of a field.
STATE_ENUM(FieldParseState,
           Unknown, ///< Not yet parsed.
           Incomplete, ///< Not enough data to parse this field. Need more data.
           Invalid, ///< The field is invalid/unexpected/long.
           Valid ///< The field is both complete and valid.
);

/// Named HTTP Status Codes.
/// See https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
enum class StatusCode : unsigned
{
    None = 0, // Undefined status (unknown); implies time-out.

    // Informational
    Continue = 100,
    SwitchingProtocols = 101,
    Processing = 102, // RFC 2518 (WebDAV)
    EarlyHints = 103, // RFC 8297

    // Successful
    OK = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,
    MultiStatus = 207, // RFC 4918 (WebDAV)
    AlreadyReported = 208, // RFC 5842 (WebDAV)
    IMUsed = 226, // RFC 3229

    // Redirection
    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304,
    UseProxy = 305,
    // Unused: 306
    TemporaryRedirect = 307,
    PermanentRedirect = 308,

    // Client Error
    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    ProxyAuthenticationRequired = 407,
    RequestTimeout = 408,
    Conflict = 409,
    Gone = 410,
    LengthRequired = 411,
    PreconditionFailed = 412,
    PayloadTooLarge = 413, // Previously called "Request Entity Too Large"
    URITooLong = 414, // Previously called "Request-URI Too Long"
    UnsupportedMediaType = 415,
    RangeNotSatisfiable = 416, // Previously called "Requested Range Not Satisfiable"
    ExpectationFailed = 417,
    ImATeapot = 418, // RFC 2324, RFC 7168 (April's fool)
    MisdirectedRequest = 421, // RFC 7540
    UnprocessableEntity = 422, // RFC 4918 (WebDAV)
    Locked = 423, // RFC 4918 (WebDAV)
    FailedDependency = 424, // RFC 4918 (WebDAV)
    TooEarly = 425, // RFC 8470
    UpgradeRequired = 426,
    PreconditionRequired = 428, // RFC 6585
    TooManyRequests = 429, // RFC 6585
    RequestHeaderFieldsTooLarge = 431, // RFC 6585
    LoginTimeout = 440, // IIS
    RetryWith = 449, // IIS
    UnavailableForLegalReasons = 451, // RFC 7725 and IIS Redirect

    // Server Error
    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    GatewayTimeout = 504,
    HTTPVersionNotSupported = 505,
    InsufficientStorage = 507, // RFC 4918 (WebDAV)
    LoopDetected = 508, // RFC 5842 (WebDAV)
    NotExtended = 510, // RFC 2774
    NetworkAuthenticationRequired = 511, // RFC 6585
};

/// Returns true if the given StatusCode is a redirect (301, 302, 307, 308).
constexpr bool isRedirectStatusCode(StatusCode code)
{
    return code == StatusCode::MovedPermanently || code == StatusCode::Found ||
           code == StatusCode::TemporaryRedirect || code == StatusCode::PermanentRedirect;
}

/// Returns true for status codes that indicate authorization failure (401, 403, 404).
constexpr bool isUnauthorizedStatusCode(StatusCode code)
{
    return code == StatusCode::Unauthorized || code == StatusCode::Forbidden ||
           code == StatusCode::NotFound;
}

/// Returns the Reason Phrase for a given HTTP Status Code.
/// If not defined, "Unknown" is returned.
/// The Reason Phrase is informational only, but it helps
/// to use the canonical one for consistency.
/// See https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
static inline const char* getReasonPhraseForCode(int code)
{
#define CASE(C, R)                                                                                 \
    case C:                                                                                        \
        return R;
    switch (code)
    {
        // Informational
        CASE(100, "Continue");
        CASE(101, "Switching Protocols");
        CASE(102, "Processing"); // RFC 2518 (WebDAV)
        CASE(103, "Early Hints"); // RFC 8297

        // Successful
        CASE(200, "OK");
        CASE(201, "Created");
        CASE(202, "Accepted");
        CASE(203, "Non Authoritative Information");
        CASE(204, "No Content");
        CASE(205, "Reset Content");
        CASE(206, "Partial Content");
        CASE(207, "Multi Status"); // RFC 4918 (WebDAV)
        CASE(208, "Already Reported"); // RFC 5842 (WebDAV)
        CASE(226, "IM Used"); // RFC 3229

        // Redirection
        CASE(300, "Multiple Choices");
        CASE(301, "Moved Permanently");
        CASE(302, "Found");
        CASE(303, "See Other");
        CASE(304, "Not Modified");
        CASE(305, "Use Proxy");
        CASE(307, "Temporary Redirect");

        // Client Error
        CASE(400, "Bad Request");
        CASE(401, "Unauthorized");
        CASE(402, "Payment Required");
        CASE(403, "Forbidden");
        CASE(404, "Not Found");
        CASE(405, "Method Not Allowed");
        CASE(406, "Not Acceptable");
        CASE(407, "Proxy Authentication Required");
        CASE(408, "Request Timeout");
        CASE(409, "Conflict");
        CASE(410, "Gone");
        CASE(411, "Length Required");
        CASE(412, "Precondition Failed");
        CASE(413, "Payload Too Large"); // Previously called "Request Entity Too Large"
        CASE(414, "URI Too Long"); // Previously called "Request-URI Too Long"
        CASE(415, "Unsupported Media Type");
        CASE(416, "Range Not Satisfiable"); // Previously called "Requested Range Not Satisfiable"
        CASE(417, "Expectation Failed");
        CASE(418, "I'm A Teapot"); // RFC 2324, RFC 7168 (April's fool)
        CASE(421, "Misdirected Request"); // RFC 7540
        CASE(422, "Unprocessable Entity"); // RFC 4918 (WebDAV)
        CASE(423, "Locked"); // RFC 4918 (WebDAV)
        CASE(424, "Failed Dependency"); // RFC 4918 (WebDAV)
        CASE(425, "Too Early"); // RFC 8470
        CASE(426, "Upgrade Required");
        CASE(428, "Precondition Required"); // RFC 6585
        CASE(429, "Too Many Requests"); // RFC 6585
        CASE(431, "Request Header Fields Too Large"); // RFC 6585
        CASE(440, "Login Time-out"); // IIS
        CASE(449, "Retry With"); // IIS
        CASE(451, "Unavailable For Legal Reasons"); // RFC 7725 and IIS Redirect

        // Server Error
        CASE(500, "Internal Server Error");
        CASE(501, "Not Implemented");
        CASE(502, "Bad Gateway");
        CASE(503, "Service Unavailable");
        CASE(504, "Gateway Timeout");
        CASE(505, "HTTP Version Not Supported");
        CASE(507, "Insufficient Storage"); // RFC 4918 (WebDAV)
        CASE(508, "Loop Detected"); // RFC 5842 (WebDAV)
        CASE(510, "Not Extended"); // RFC 2774
        CASE(511, "Network Authentication Required"); // RFC 6585
    }
#undef CASE

    return "Unknown";
}

static inline const char* getReasonPhraseForCode(StatusCode statusCode)
{
    return getReasonPhraseForCode(static_cast<unsigned>(statusCode));
}

std::string getAgentString();
std::string getServerString();

/// The callback signature for handling IO writes.
/// Returns the number of bytes read from the buffer,
/// -1 for error (terminates the transfer).
/// The second argument is the data size in the buffer.
using IoWriteFunc = std::function<int64_t(const char*, int64_t)>;

/// The callback signature for handling IO reads.
/// Returns the number of bytes written to the buffer,
/// 0 when no more data is left to read,
/// -1 for error (terminates the transfer).
/// The second argument is the buffer size.
using IoReadFunc = std::function<int64_t(char*, int64_t)>;

/// HTTP Header.
class Header
{
public:
    static constexpr std::string_view CONTENT_TYPE = "Content-Type";
    static constexpr std::string_view CONTENT_LENGTH = "Content-Length";
    static constexpr std::string_view TRANSFER_ENCODING = "Transfer-Encoding";
    static constexpr std::string_view Authorization = "Authorization";
    static constexpr std::string_view COOKIE = "Cookie";
    static constexpr std::string_view HOST = "Host";

    static constexpr int64_t MaxNumberFields = 128; // Arbitrary large number.
    static constexpr int64_t MaxNameLen = 512;
    static constexpr int64_t MaxValueLen = 9 * 1024; // 8000 bytes recommended by rfc.
    static constexpr int64_t MaxFieldLen = MaxNameLen + MaxValueLen;
    static constexpr int64_t MaxHeaderLen = MaxNumberFields * MaxFieldLen; // ~1.18 MB.

    static constexpr const char* CONNECTION = "Connection";

    /// Describes the `Connection` header token value
    STATE_ENUM(
        ConnectionToken,
        None, ///< No `Connection` header token set
        Close, ///< `Connection: close` [RFC2616 14.10](https://www.rfc-editor.org/rfc/rfc2616#section-14.10)
        KeepAlive, ///< `Connection: Keep-Alive` Obsolete [RFC2068 19.7.1](https://www.rfc-editor.org/rfc/rfc2068#section-19.7.1)
        Upgrade ///< `Connection: Upgrade` HTTP/1.1 only [RFC2817](https://www.rfc-editor.org/rfc/rfc2817)
    );

    /// Describes the header state during parsing.
    STATE_ENUM(State, New,
               Incomplete, ///< Haven't reached the end yet.
               InvalidField, ///< Too long, no colon, etc.
               TooManyFields, ///< Too many fields to accept.
               Complete ///< Header is complete and valid.
    );

    using Pair = std::pair<std::string, std::string>;
    using Container = std::vector<Pair>;
    using Iterator = std::vector<Pair>::iterator;
    using ConstIterator = std::vector<Pair>::const_iterator;

    ConstIterator begin() const { return _headers.begin(); }
    ConstIterator end() const { return _headers.end(); }

    /// Returns the number of entries in the header.
    std::size_t size() const { return _headers.size(); }

    /// Parse the given data as an HTTP header.
    /// Returns the number of bytes consumed (and must be removed from the input).
    int64_t parse(const char* p, int64_t len);

    /// Add an HTTP header field.
    void add(std::string key, std::string value)
    {
        _headers.emplace_back(std::move(key), std::move(value));
    }

    /// Set an HTTP header field, replacing an earlier value, if exists (case insensitive).
    void set(const std::string_view key, std::string value)
    {
        const Iterator end = _headers.end();
        const Iterator it = std::find_if(_headers.begin(), end, [&key](const Pair& pair) -> bool
                                         { return Util::iequal(pair.first, key); });
        if (it != end)
        {
            it->second.swap(value);
        }
        else
        {
            _headers.emplace_back(key, std::move(value));
        }
    }

    // Returns true if the HTTP header field exists (case insensitive)
    [[nodiscard]] bool has(const std::string_view key) const
    {
        const ConstIterator end = this->end();
        return std::find_if(begin(), end, [&key](const Pair& pair) -> bool
                            { return Util::iequal(pair.first, key); }) != end;
    }

    [[nodiscard]] ConstIterator find(const std::string_view key) const
    {
        const ConstIterator end = this->end();
        return std::find_if(begin(), end, [&key](const Pair& pair) -> bool
                            { return Util::iequal(pair.first, key); });
    }

    /// Remove the first matching HTTP header field (case insensitive), returning true if found and removed.
    bool remove(const std::string_view key)
    {
        const ConstIterator end = this->end();
        const ConstIterator it = std::find_if(begin(), end, [&key](const Pair& pair) -> bool
                                              { return Util::iequal(pair.first, key); });
        if (it != end)
        {
            _headers.erase(it);
            return true;
        }

        return false;
    }

    /// Get a header entry value by key, if found, defaulting to @def, if missing.
    [[nodiscard]] std::string get(const std::string_view key,
                                  const std::string& def = std::string()) const
    {
        // There are typically half a dozen header
        // entries, rarely much more. A map would
        // probably not be faster but would add complexity.
        const ConstIterator end = this->end();
        const ConstIterator it = std::find_if(begin(), end, [&key](const Pair& pair) -> bool
                                              { return Util::iequal(pair.first, key); });
        if (it != end)
            return it->second;
        return def;
    }

    /// Return the HOST header.
    [[nodiscard]] std::string getHost() const { return get(HOST); }

    /// Set the Content-Type header.
    void setContentType(std::string type) { set(CONTENT_TYPE, std::move(type)); }
    /// Get the Content-Type header.
    [[nodiscard]] std::string getContentType() const { return get(CONTENT_TYPE); }
    /// Returns true iff a Content-Type header exists.
    [[nodiscard]] bool hasContentType() const { return has(CONTENT_TYPE); }

    /// Set the Content-Length header.
    void setContentLength(int64_t length) { set(CONTENT_LENGTH, std::to_string(length)); }
    /// Get the Content-Length header.
    [[nodiscard]] int64_t getContentLength() const;
    /// Returns true iff a Content-Length header exists.
    [[nodiscard]] bool hasContentLength() const { return has(CONTENT_LENGTH); }

    /// Get the Transfer-Encoding header, if any.
    [[nodiscard]] std::string getTransferEncoding() const { return get(TRANSFER_ENCODING); }

    /// Return true iff Transfer-Encoding is set to chunked (the last entry).
    bool getChunkedTransferEncoding() const { return _chunked; }

    [[nodiscard]] bool hasConnectionToken() const { return has(CONNECTION); }
    [[nodiscard]] ConnectionToken getConnectionToken() const
    {
        const std::string token = get(CONNECTION);
        if (Util::iequal("close", token))
        {
            return ConnectionToken::Close;
        }

        if (Util::iequal("keep-alive", token))
        {
            return ConnectionToken::KeepAlive;
        }

        if (Util::iequal("upgrade", token))
        {
            return ConnectionToken::Upgrade;
        }

        return ConnectionToken::None;
    }

    void setConnectionToken(ConnectionToken token)
    {
        std::string value;
        switch (token)
        {
            case ConnectionToken::Close:
                value = "close";
                break;
            case ConnectionToken::KeepAlive:
                value = "Keep-Alive";
                break;
            case ConnectionToken::Upgrade:
                value = "Upgrade";
                break;
            default:
                remove(CONNECTION);
                return;
        }

        set(CONNECTION, std::move(value));
    }

    /// Adds a new "Cookie" header entry with the given content.
    void addCookie(std::string cookie) { add(std::string(COOKIE), std::move(cookie)); }

    /// Adds a new "Cookie" header entry with the given pairs.
    void addCookie(const Container& pairs)
    {
        std::string s;
        s.reserve(256);
        for (const auto& pair : pairs)
        {
            if (!s.empty())
                s += "; ";
            s += pair.first;
            s += '=';
            s += pair.second;
        }

        add(std::string(COOKIE), std::move(s));
    }

    /// Gets the name=value pairs of all "Cookie" header entries.
    [[nodiscard]] Container getCookies() const
    {
        Container cookies;
        for (const auto& pair : _headers)
        {
            if (Util::iequal(pair.first, COOKIE))
            {
                const auto tokens = StringVector::tokenize(pair.second, ';');
                for (const auto cookie : tokens)
                {
                    cookies.emplace_back(Util::split(tokens.getParam(cookie), '='));
                }
            }
        }

        return cookies;
    }

    bool writeData(Buffer& out) const
    {
        // Note: we don't add the end-of-header '\r\n'
        // to allow for manually extending the headers.
        for (const auto& pair : _headers)
        {
            out.append(pair.first);
            out.append(": ");
            out.append(pair.second);
            out.append("\r\n");
        }

        return true;
    }

    /// Serialize the header to an output stream.
    template <typename T> T& serialize(T& os) const
    {
        // Note: we don't add the end-of-header '\r\n'.
        for (const auto& pair : _headers)
        {
            os << pair.first << ": " << pair.second << "\r\n";
        }

        return os;
    }

    /// Serialize the header to string. For logging only.
    [[nodiscard]] std::string toString() const
    {
        std::ostringstream oss;
        return serialize(oss).str();
    }

private:
    /// The headers are ordered key/value pairs.
    /// This isn't designed for lookup performance, but to preserve order.
    //TODO: We might not need this and get away with a map.
    Container _headers;
    bool _chunked = false;
};

/// An HTTP Request made over Session.
class RequestCommon
{
public:
    static constexpr int64_t VersionLen = 8;
    static constexpr int64_t MinRequestHeaderLen = sizeof("GET / HTTP/0.0\r\n") - 1;
    static constexpr std::string_view VERB_GET = "GET";
    static constexpr std::string_view VERB_POST = "POST";
    static constexpr std::string_view VERS_1_1 = "HTTP/1.1";

    RequestCommon()
        : _stage(Stage::RequestLine)
    {
    }

    /// The stages of processing the request.
    STATE_ENUM(Stage,
               RequestLine, ///< Sending/Parsing the request-line.
               Header, ///< Sending/Parsing the header.
               Body, ///< Sending/Parsing the body (if any).
               Finished ///< Done.
    );

    /// Get the request URL.
    const std::string& getUrl() const { return _url; }
    /// Set the request URL. Necessary to decode hexified URLs.
    void setUrl(const std::string& url) { _url = url; }

    /// Get the request verb.
    const std::string& getVerb() const { return _verb; }

    /// Get the protocol version.
    const std::string& getVersion() const { return _version; }

    /// Return the HOST header.
    std::string getHost() const { return _header.getHost(); }

    /// The header object.
    const Header& header() const { return _header; }

    /// Returns true if the HTTP header field exists (case insensitive).
    [[nodiscard]] bool has(const std::string_view key) const { return _header.has(key); }

    /// Returns the iterator to the header's key in question, if found. Otherwise end().
    [[nodiscard]] Header::ConstIterator find(const std::string_view key) const
    {
        return _header.find(key);
    }
    Header::ConstIterator end() const { return _header.end(); }

    /// Get a header entry value by key, if found, defaulting to @def, if missing.
    [[nodiscard]] std::string get(const std::string_view key,
                                  const std::string& def = std::string()) const
    {
        return _header.get(key, def);
    }

    Stage stage() const { return _stage; }

    /// True if we are a Keep-Alive request.
    [[nodiscard]] bool isKeepAlive() const
    {
        const std::string token = get(Header::CONNECTION);
        if (!token.empty())
        {
            return !Util::iequal("close", token);
        }

        // 1.1 and newer are reusable by default (i.e. keep-alive).
        return getVersion() != "HTTP/1.0";
    }

    void dumpState(std::ostream& os, const std::string& indent = "\n  ") const
    {
        os << indent << "http::Request: " << _version << ' ' << _verb << ' ' << _url;
        os << indent << "\tstage: " << name(_stage);
        os << indent << "\theaders: ";
        Util::joinPair(os, _header, indent, '\t');
    }

protected:
    RequestCommon(std::string url, std::string verb, Header header, std::string version)
        : _header(std::move(header))
        , _url(std::move(url))
        , _verb(std::move(verb))
        , _version(std::move(version))
        , _stage(Stage::RequestLine)
    {
    }

    /// Set the request verb (typically GET or POST).
    void setVerb(const std::string& verb) { _verb = verb; }
    /// Set the protocol version (typically HTTP/1.1).
    void setVersion(const std::string& version) { _version = version; }
    /// Add an HTTP header field.
    void add(std::string key, std::string value) { _header.add(std::move(key), std::move(value)); }

    Header& editHeader() { return _header; }

    /// Set an HTTP header field, replacing an earlier value, if exists.
    void set(const std::string& key, std::string value) { _header.set(key, std::move(value)); }

    void setStage(Stage stage) { _stage = stage; }

private:
    Header _header;
    std::string _url; ///< The URL to request, without hostname.
    std::string _verb; ///< Used as-is, but only POST supported.
    std::string _version; ///< The protocol version, currently 1.1.
    Stage _stage;
};

/// An HTTP Request made over Session.
class Request : public RequestCommon
{
public:
    static constexpr int64_t VersionLen = 8;
    static constexpr int64_t MinRequestHeaderLen = sizeof("GET / HTTP/0.0\r\n") - 1;
    static constexpr const char* VERB_GET = "GET";
    static constexpr const char* VERB_POST = "POST";
    static constexpr const char* VERS_1_1 = "HTTP/1.1";

    /// Create a Request given a @url, http @verb, @header, and http @version.
    /// All are optional, since they can be overwritten later.
    explicit Request(std::string url = "/", std::string verb = VERB_GET,
                     Header headerObj = Header(), std::string version = VERS_1_1)
        : RequestCommon(std::move(url), std::move(verb), std::move(headerObj), std::move(version))
        , _bodyReaderCb([](const char*, int64_t) { return 0; })
    {
    }

    using RequestCommon::add;
    using RequestCommon::set;
    using RequestCommon::setUrl;
    using RequestCommon::setVerb;
    using RequestCommon::setVersion;

    void setConnectionToken(Header::ConnectionToken token)
    {
        editHeader().setConnectionToken(token);
    }
    void setContentType(std::string type) { editHeader().setContentType(std::move(type)); }
    void setContentLength(int64_t length) { editHeader().setContentLength(length); }

    /// Add an HTTP header field.
    void add(std::string key, std::string value)
    {
        editHeader().add(std::move(key), std::move(value));
    }

    /// Set an HTTP header field, replacing an earlier value, if exists.
    void set(const std::string& key, std::string value) { editHeader().set(key, std::move(value)); }

    /// Set the request body source to upload some data. Meaningful for POST.
    /// Size is needed to set the Content-Length.
    void setBodySource(IoReadFunc bodyReaderCb, int64_t size)
    {
        editHeader().setContentLength(size);
        _bodyReaderCb = std::move(bodyReaderCb);
    }

    /// Set the file to send as the body of the request.
    void setBodyFile(const std::string& path)
    {
        auto ifs = std::make_shared<std::ifstream>(path, std::ios::binary);

        ifs->seekg(0, std::ios_base::end);
        const int64_t size = ifs->tellg();
        ifs->seekg(0, std::ios_base::beg);

        setBodySource(
            [ifs = std::move(ifs)](char* buf, int64_t len) -> int64_t
            {
                ifs->read(buf, len);
                return ifs->gcount();
            },
            size);
    }

    void setBody(std::string body, std::string contentType = "text/html;charset=utf-8")
    {
        if (!body.empty()) // Type is only meaningful if there is a body.
            editHeader().setContentType(std::move(contentType));

        editHeader().setContentLength(body.size());

        const size_t bodySize = body.size();

        auto iss = std::make_shared<std::istringstream>(std::move(body), std::ios::binary);

        setBodySource(
            [iss = std::move(iss)](char* buf, int64_t len) -> int64_t
            {
                iss->read(buf, len);
                return iss->gcount();
            },
            bodySize);
    }

    /// Serialize the Request into the buffer.
    bool writeData(Buffer& out, std::size_t capacity);

    /// Sets the username and password in the Authorization header, per RFC-7235.
    void setBasicAuth(std::string_view username, std::string_view password)
    {
        std::string basicAuth{ username };
        basicAuth.append(":");
        basicAuth.append(password);
        editHeader().add(std::string(Header::Authorization),
                         "Basic " + Util::base64Encode(basicAuth));
    }

    /// Returns the username and password from the Authorization header, per RFC-7235.
    /// Only 'Basic' Authentication is supported. Retuns empty pair if not found.
    [[nodiscard]] std::pair<std::string, std::string> getBasicAuth() const
    {
        const auto& [scheme, param] = getCredentials();
        if (Util::iequal(scheme, "Basic"))
        {
            return Util::split(Util::base64Decode(param), ':');
        }

        return {};
    }

    /// Returns the auth-scheme and auth-param from the Authorization header, per RFC-7235.
    [[nodiscard]] std::pair<std::string, std::string> getCredentials() const
    {
        return Util::split(get(Header::Authorization));
    }

private:
    IoReadFunc _bodyReaderCb;
};

class MultipartDataParser final
{
    STATE_ENUM(State, FirstPart, NextPart, LastPart);

    static constexpr std::size_t MaxLineLength = 512;

public:
    explicit MultipartDataParser(const std::string& boundary)
        : _delimiter("\r\n--" + boundary)
        , _dashBoundary(&_delimiter[2], _delimiter.size() - 2) // Skip CRLF
        , _boundary(&_delimiter[4], _delimiter.size() - 4) // Skip CRLF--
        , _state(State::FirstPart)
    {
    }

    /// Returns the boundary used for this multipart-data.
    std::string_view boundary() const { return _boundary; }

    /// True after calling readPart iff we read the last part.
    /// Calling readPart when this is true is undefined.
    bool isLast() const { return _state == State::LastPart; }

    /// Read the current part and return the payload and header.
    /// Returns an empty string if there is not enough data, or we're at the last part.
    int64_t readPart(std::string_view data, Header& header, std::string_view& body);

private:
    /// Finds the given delimiter (which can be _dashBoundary or _delimiter).
    /// Returns a triad with the following values: {the offset to the start of the marker,
    /// the offset to the end of the marker, true if last boundary}.
    /// The first value is -1 when there is not enough data.
    /// The second value is 0, if no end is found, -1 for invalid data.
    std::tuple<int64_t, int64_t, bool> findBoundary(std::string_view data,
                                                    std::string_view delimiter, int64_t off);

    /// Finds and parses the next part.
    int64_t parsePart(std::string_view data, Header& header, std::string_view& body);

    /// The delimiter is CRLF--boundary.
    const std::string _delimiter;
    /// The dash-boundary is --boundary.
    const std::string_view _dashBoundary;
    /// The boundary name as provided by the 'Content-Type:' header.
    const std::string_view _boundary;
    /// The state of the parser.
    State _state;
};

/// A server-side HTTP Request parser for incoming request.
class RequestParser final : public RequestCommon
{
public:
    /// Create a default RequestParser.
    RequestParser()
        : _recvBodySize(0)
    {
        // By default we store the body in memory.
        saveBodyToMemory();
    }

    /// Construct a parser from a Request instance.
    /// Typically used for testing.
    explicit RequestParser(http::Request& request)
        : _recvBodySize(0)
    {
        // By default we store the body in memory.
        saveBodyToMemory();

        Buffer out;
        request.writeData(out, INT_MAX);
        [[maybe_unused]] const auto read = readData(out.getBlock(), out.getBlockSize());
        assert(read == static_cast<int64_t>(out.getBlockSize()) &&
               "Expected to read all the serialized data");
    }

    /// Handles incoming data.
    /// Returns the number of bytes consumed, or -1 for error
    /// and/or to interrupt transmission.
    int64_t readData(const char* p, int64_t len);

    std::string_view getBody() const { return _body; }

    /// Redirect the response body, if any, to a file.
    /// If the server responds with a non-success status code (i.e. not 2xx)
    /// the body is redirected to memory to be read via getBody().
    /// Check the statusLine().statusCategory() for the status code.
    void saveBodyToFile(const std::string& path)
    {
        _bodyFile.open(path, std::ios_base::out | std::ios_base::binary);
        if (!_bodyFile.good())
            LOG_ERR("Unable to open [" << path << "] for saveBodyToFile");
        _onBodyWriteCb = [this](const char* p, int64_t len)
        {
            LOG_TRC("Writing " << len << " bytes");
            if (_bodyFile.good())
                _bodyFile.write(p, len);
            return _bodyFile.good() ? len : -1;
        };
    }

    /// Generic handler for the body payload.
    /// See IoWriteFunc documentation for the contract.
    void saveBodyToHandler(IoWriteFunc onBodyWriteCb) { _onBodyWriteCb = std::move(onBodyWriteCb); }

    /// The response body, if any, is stored in memory.
    /// Use getBody() to read it.
    void saveBodyToMemory()
    {
        _onBodyWriteCb = [this](const char* p, int64_t len)
        {
            _body.insert(_body.end(), p, p + len);
            // LOG_TRC("Body: " << len << "\n" << _body);
            return len;
        };
    }

    void dumpState(std::ostream& os, const std::string& indent = "\n  ") const
    {
        os << indent << "http::RequestParser: ";
        RequestCommon::dumpState(os, indent);
        os << indent << "\trecvBodySize: " << _recvBodySize;

        std::string childIndent = indent + '\t';
        os << indent;
        HexUtil::dumpHex(os, _body, "\tbody:\n",
                         Util::replace(std::move(childIndent), "\n", "").c_str());
    }

private:
    std::string _body;
    std::ofstream _bodyFile; ///< Used when _bodyHandling is OnDisk.
    IoWriteFunc _onBodyWriteCb; ///< Used to handling body receipt in all cases.
    int64_t _recvBodySize; ///< The amount of data we received (compared to the Content-Length).
};

/// HTTP Status Line is the first line of a response sent by a server.
class StatusLine
{
public:
    static constexpr int64_t VersionLen = 8;
    static constexpr int64_t StatusCodeLen = 3;
    static constexpr int64_t MaxReasonPhraseLen = 512; // Arbitrary large number.
    static constexpr int64_t MinStatusLineLen =
        sizeof("HTTP/0.0 000\r\n") - 1; // Reason phrase is optional.
    static constexpr int64_t MaxStatusLineLen = VersionLen + StatusCodeLen + MaxReasonPhraseLen;
    static constexpr int64_t MinValidStatusCode = 100;
    static constexpr int64_t MaxValidStatusCode = 599;

    static constexpr const char* HTTP_1_1 = "HTTP/1.1";
    static constexpr const char* OK = "OK";

    /// Construct an invalid StatusLine, used for parsing.
    StatusLine()
        : _versionMajor(1)
        , _versionMinor(1)
        , _statusCode(0)
    {
    }

    /// Construct a StatusLine with a given code and
    /// the default protocol version.
    explicit StatusLine(unsigned statusCodeNumber)
        : _httpVersion(HTTP_1_1)
        , _versionMajor(1)
        , _versionMinor(1)
        , _statusCode(statusCodeNumber)
        , _reasonPhrase(getReasonPhraseForCode(statusCodeNumber))
    {
    }

    explicit StatusLine(StatusCode statusCode)
        : StatusLine(static_cast<unsigned>(statusCode))
    {
    }

    /// The Status Code class of the response.
    /// None of these implies complete receipt of the response.
    STATE_ENUM(StatusCodeClass,
               Invalid, ///< Not a valid Status Code.
               Informational, ///< Request being processed, not final response.
               Successful, ///< Successfully processed request, response on the way.
               Redirection, ///< Redirected to a different resource.
               Client_Error, ///< Bad request, cannot respond.
               Server_Error ///< Bad server, cannot respond.
    );

    [[nodiscard]] StatusCodeClass statusCategory() const
    {
        if (_statusCode >= 500 && _statusCode < 600)
            return StatusCodeClass::Server_Error;
        if (_statusCode >= 400)
            return StatusCodeClass::Client_Error;
        if (_statusCode >= 300)
            return StatusCodeClass::Redirection;
        if (_statusCode >= 200)
            return StatusCodeClass::Successful;
        if (_statusCode >= 100)
            return StatusCodeClass::Informational;
        return StatusCodeClass::Invalid;
    }

    /// Parses a Status Line.
    /// Returns the state and clobbers the len on success to the number of bytes read.
    FieldParseState parse(const char* p, int64_t& len);

    bool writeData(Buffer& out) const
    {
        out.append(_httpVersion);
        out.append(" ");
        out.append(std::to_string(_statusCode));
        out.append(" ");
        out.append(_reasonPhrase);
        out.append("\r\n");
        return true;
    }

    const std::string& httpVersion() const { return _httpVersion; }
    unsigned versionMajor() const { return _versionMajor; }
    unsigned versionMinor() const { return _versionMinor; }
    StatusCode statusCode() const { return static_cast<StatusCode>(_statusCode); }
    const std::string& reasonPhrase() const { return _reasonPhrase; }

private:
    std::string _httpVersion; ///< Typically "HTTP/1.1"
    unsigned _versionMajor; ///< The first version digit (typically 1).
    unsigned _versionMinor; ///< The second version digit (typically 1).
    unsigned _statusCode;
    std::string _reasonPhrase; ///< A client SHOULD ignore the reason-phrase content.
};

/// The response for an HTTP request.
class Response final
{
public:
    using FinishedCallback = std::function<void()>;

    /// A response received from a server.
    /// Used for parsing an incoming response.
    explicit Response(FinishedCallback finishedCallback, int fd = -1)
        : _state(State::New)
        , _parserStage(ParserStage::StatusLine)
        , _recvBodySize(0)
        , _finishedCallback(std::move(finishedCallback))
        , _fd(fd)
    {
        // By default we store the body in memory.
        saveBodyToMemory();
    }

    /// A response received from a server.
    /// Used for parsing an incoming response.
    Response()
        : Response(nullptr)
    {
    }

    /// A response sent from a server.
    /// Used for generating an outgoing response.
    explicit Response(StatusLine statusLineObj, int fd = -1)
        : _statusLine(std::move(statusLineObj))
        , _state(State::New)
        , _parserStage(ParserStage::StatusLine)
        , _recvBodySize(0)
        , _fd(fd)
    {
        _header.add("Date", Util::getHttpTimeNow());
        _header.add("Server", http::getServerString());
    }

    /// A response sent from a server.
    /// Used for generating an outgoing response.
    explicit Response(StatusCode statusCode, int fd = -1)
        : Response(StatusLine(statusCode), fd)
    {
    }

    /// The state of an incoming response, when parsing.
    STATE_ENUM(State,
               New, ///< Valid but meaningless.
               Incomplete, ///< In progress, no errors.
               Error, ///< This is for protocol errors, not 400 and 500 reponses.
               Timeout, ///< The request has exceeded the time allocated.
               Complete ///< Successfully completed (does *not* imply 200 OK).
    );

    /// The state of the Response (for the server's response use statusLine).
    State state() const { return _state; }

    /// Returns true iff there is no more data to expect and the state is final.
    bool done() const
    {
        return (_state == State::Error || _state == State::Timeout || _state == State::Complete);
    }

    const StatusLine& statusLine() const { return _statusLine; }
    StatusCode statusCode() const { return _statusLine.statusCode(); }

    const Header& header() const { return _header; }

    /// Add an HTTP header field.
    void add(std::string key, std::string value) { _header.add(std::move(key), std::move(value)); }

    /// Set an HTTP header field, replacing an earlier value, if exists.
    void set(const std::string& key, std::string value) { _header.set(key, std::move(value)); }

    /// Set the Connection header.
    void setConnectionToken(Header::ConnectionToken token) { _header.setConnectionToken(token); }

    /// Set the Content-Type header.
    void setContentType(std::string type) { _header.setContentType(std::move(type)); }

    /// Set the Content-Length header.
    void setContentLength(int64_t length) { _header.setContentLength(length); }

    /// Adds a new "Cookie" header entry with the given content.
    void addCookie(const std::string& cookie) { _header.addCookie(cookie); }

    /// Get a header entry value by key, if found, defaulting to @def, if missing.
    [[nodiscard]] std::string get(const std::string& key,
                                  const std::string& def = std::string()) const
    {
        return _header.get(key, def);
    }

    /// Redirect the response body, if any, to a file.
    /// If the server responds with a non-success status code (i.e. not 2xx)
    /// the body is redirected to memory to be read via getBody().
    /// Check the statusLine().statusCategory() for the status code.
    void saveBodyToFile(const std::string& path)
    {
        _bodyFile.open(path, std::ios_base::out | std::ios_base::binary);
        if (!_bodyFile.good())
            LOG_ERR("Unable to open [" << path << "] for saveBodyToFile");
        _onBodyWriteCb = [this](const char* p, int64_t len)
        {
            LOG_TRC("Writing " << len << " bytes");
            if (_bodyFile.good())
                _bodyFile.write(p, len);
            return _bodyFile.good() ? len : -1;
        };
    }

    /// Generic handler for the body payload.
    /// See IoWriteFunc documentation for the contract.
    void saveBodyToHandler(IoWriteFunc onBodyWriteCb) { _onBodyWriteCb = std::move(onBodyWriteCb); }

    /// The response body, if any, is stored in memory.
    /// Use getBody() to read it.
    void saveBodyToMemory()
    {
        _onBodyWriteCb = [this](const char* p, int64_t len)
        {
            _body.insert(_body.end(), p, p + len);
            // LOG_TRC("Body: " << len << "\n" << _body);
            return len;
        };
    }

    /// Returns the body, assuming it wasn't redirected to file or callback.
    const std::string& getBody() const { return _body; }

    /// Set the body to be sent to the client.
    /// Also sets Content-Length and Content-Type.
    void setBody(std::string body, std::string contentType = "text/html;charset=utf-8")
    {
        _body = std::move(body);
        _header.setContentLength(_body.size()); // Always set it, even if 0.
        if (!_body.empty()) // Type is only meaningful if there is a body.
            _header.setContentType(std::move(contentType));
    }

    /// Append a chunk to the body. Must have Transfer-Encoding: chunked.
    void appendChunk(std::string_view chunk)
    {
        assert(get("transfer-encoding").find("chunked") != std::string::npos &&
               "Expected to have chunked transfer-encoding header");
        assert(!_header.has("content-length") &&
               "Unexpected to have content-length header with transfer-encoding defined");

        _body.reserve(_body.size() + chunk.size() + 32);

        std::stringstream ss;
        ss << std::hex << chunk.size();
        _body.append(ss.str());
        _body.append("\r\n");
        _body.append(chunk);
        _body.append("\r\n");
    }

    /// Handles incoming data (from the Server) in the Client.
    /// Returns the number of bytes consumed, or -1 for error
    /// and/or to interrupt transmission.
    int64_t readData(const char* p, int64_t len);

    /// Serializes the Server Response into the given buffer.
    bool writeData(Buffer& out) const
    {
        assert(!get("Date").empty() && "Date is always set in http::Response ctor");
        assert(get("Server") == http::getServerString() &&
               "Server Agent is always set in http::Response ctor");

        _statusLine.writeData(out);
        _header.writeData(out);
        out.append("\r\n"); // End of header.
        out.append(_body);
        return true;
    }

    /// Signifies that we got all the data we expected
    /// and cleans up and updates the states.
    void complete()
    {
        LOG_TRC("State::Complete");
        finish(State::Complete);
    }

    /// The request has exceeded the expected duration
    /// and has ended prematurely.
    void timeout()
    {
        LOG_TRC("State::Timeout");
        finish(State::Timeout);
    }

    /// If not already in done state, finish with State::Error.
    void error()
    {
        // We expect to have completed successfully, or timed out,
        // anything else means we didn't get complete data.
        LOG_TRC("State::Error");
        finish(State::Error);
    }

    /// Sets the context used by logPrefix.
    void setLogContext(int fd) { _fd = fd; }

    void dumpState(std::ostream& os, const std::string& indent = "\n  ") const
    {
        os << indent << "http::Response: #" << _fd;
        os << indent << "\tstatusLine: " << _statusLine.httpVersion() << ' '
           << getReasonPhraseForCode(_statusLine.statusCode()) << ' ' << _statusLine.reasonPhrase();
        os << indent << "\tstate: " << name(_state);
        os << indent << "\tparseStage: " << name(_parserStage);
        os << indent << "\trecvBodySize: " << _recvBodySize;
        os << indent << "\theaders: ";

        std::string childIndent = indent + '\t';
        Util::joinPair(os, _header, childIndent);
        os << indent;
        HexUtil::dumpHex(os, _body, "\tbody:\n", Util::replace(std::move(childIndent), "\n", "").c_str());
    }

private:
    void logPrefix(std::ostream& os) const { os << '#' << _fd << ": "; }

    void finish(State newState)
    {
        if (!done())
        {
            LOG_TRC("Finishing: " << name(newState));
            _bodyFile.close();
            _state = newState;
            if (_finishedCallback)
                _finishedCallback();
        }
    }

    /// The stage we're at in consuming the received data.
    STATE_ENUM(ParserStage, StatusLine, Header, Body, Finished);

    StatusLine _statusLine;
    Header _header;
    std::atomic<State> _state; ///< The state of the Response.
    ParserStage _parserStage; ///< The parser's state.
    int64_t _recvBodySize; ///< The amount of data we received (compared to the Content-Length).
    std::string _body; ///< Used when _bodyHandling is InMemory.
    std::ofstream _bodyFile; ///< Used when _bodyHandling is OnDisk.
    IoWriteFunc _onBodyWriteCb; ///< Used to handling body receipt in all cases.
    FinishedCallback _finishedCallback; ///< Called when response is finished.
    int _fd; ///< The socket file-descriptor.
};

/// A client socket to make asynchronous HTTP requests.
/// Designed to be reused for multiple requests.
class Session final : public ProtocolHandlerInterface
{
public:
    STATE_ENUM(Protocol, HttpUnencrypted, HttpSsl, );

private:
    /// Construct a Session instance from a hostname, protocol and port.
    /// @hostname is *not* a URI, it's either an IP or a domain name.
    Session(std::string hostname, Protocol protocolType, int portNumber)
        : _host(std::move(hostname))
        , _port(std::to_string(portNumber))
        , _protocol(protocolType)
        , _fd(-1)
        , _handshakeSslVerifyFailure(0)
        , _timeout(getDefaultTimeout())
        , _connected(false)
        , _asyncShutdownOnFinish(false)
        , _result(net::AsyncConnectResult::Ok)
    {
        assert(!_host.empty() && portNumber > 0 && !_port.empty() &&
               "Invalid hostname and portNumber for http::Sesssion");

        if constexpr (Util::isDebugEnabled())
        {
            std::string scheme;
            std::string hostString;
            std::string portString;
            assert(net::parseUri(_host, scheme, hostString, portString) && scheme.empty() &&
                   portString.empty() && hostString == _host &&
                   "http::Session expects a hostname and not a URI");
        }
    }

    /// Returns the given protocol's scheme.
    static const char* getProtocolScheme(Protocol protocol)
    {
        switch (protocol)
        {
            case Protocol::HttpUnencrypted:
                return "http";
            case Protocol::HttpSsl:
                return "https";
        }

        return "";
    }

public:
    /// Create a new HTTP Session to the given host.
    /// The port defaults to the protocol's default port.
    static std::shared_ptr<Session> create(std::string host, Protocol protocol, int port = 0);

    /// Create a new unencrypted HTTP Session to the given host.
    /// @port <= 0 will default to the http default port.
    static std::shared_ptr<Session> createHttp(std::string host, int port = 0)
    {
        return create(std::move(host), Protocol::HttpUnencrypted, port);
    }

    /// Create a new SSL HTTP Session to the given host.
    /// @port <= 0 will default to the https default port.
    static std::shared_ptr<Session> createHttpSsl(std::string host, int port = 0)
    {
        return create(std::move(host), Protocol::HttpSsl, port);
    }

    /// Create a new HTTP Session to the given URI.
    /// The @uri must include the scheme, e.g. https://domain.com:9980
    static std::shared_ptr<Session> create(const std::string& uri)
    {
        std::string scheme;
        std::string hostname;
        std::string portString;
        if (!net::parseUri(uri, scheme, hostname, portString))
        {
            LOG_ERR_S("Invalid URI [" << uri << "] to http::Session::create");
            return nullptr;
        }

        const bool secure = (Util::iequal(scheme, "https://") || Util::iequal(scheme, "wss://"));
        const auto protocol = secure ? Protocol::HttpSsl : Protocol::HttpUnencrypted;
        if (portString.empty())
            return create(std::move(hostname), protocol, getDefaultPort(protocol));

        const auto [port, success] = NumUtil::i32FromString(portString);
        if (success && port > 0)
            return create(std::move(hostname), protocol, port);

        LOG_ERR_S("Invalid port [" << portString << "] in URI [" << uri
                                   << "] to http::Session::create");
        return nullptr;
    }

    /// Returns the given protocol's default port.
    static int getDefaultPort(Protocol protocol)
    {
        switch (protocol)
        {
            case Protocol::HttpUnencrypted:
                return 80;
            case Protocol::HttpSsl:
                return 443;
        }

        return 0;
    }

    /// Returns the default timeout.
    static constexpr std::chrono::milliseconds getDefaultTimeout()
    {
        return std::chrono::seconds(30);
    }

    /// Returns the current protocol scheme.
    const char* getProtocolScheme() const { return getProtocolScheme(_protocol); }

    const std::string& host() const { return _host; }
    const std::string& port() const { return _port; }
    Protocol protocol() const { return _protocol; }
    bool isSecure() const { return _protocol == Protocol::HttpSsl; }
    bool isConnected() const { return _connected; };

    /// Set the timeout, in microseconds.
    void setTimeout(const std::chrono::microseconds timeout) { _timeout = timeout; }
    /// Get the timeout, in microseconds.
    std::chrono::microseconds getTimeout() const { return _timeout; }

    /// The response we _got_ for our request. Do *not* use this to _send_ a response!
    const std::shared_ptr<Response>& response() const { return _response; }
    const std::string& getUrl() const { return _request.getUrl(); }

    /// The onFinished callback handler signature.
    using FinishedCallback = std::function<void(const std::shared_ptr<Session>& session)>;

    /// Set a callback to handle onFinished events from this session.
    /// onFinished is triggered whenever a request has finished,
    /// regardless of the reason (error, timeout, completion).
    void setFinishedHandler(FinishedCallback onFinished) { _onFinished = std::move(onFinished); }

    /// The onConnectFail callback handler signature.
    using ConnectFailCallback = std::function<void(const std::shared_ptr<Session>& session)>;

    void setConnectFailHandler(ConnectFailCallback onConnectFail) { _onConnectFail = std::move(onConnectFail); }

    /// Make a synchronous request to download a file to the given path.
    /// Note: when the server returns an error, the response body,
    /// if any, will be stored in memory and can be read via getBody().
    /// I.e. when statusLine().statusCategory() != StatusLine::StatusCodeClass::Successful.
    const std::shared_ptr<const Response>
    syncDownload(const Request& req, const std::string& saveToFilePath, SocketPoll& poller)
    {
        LOG_TRC_S("syncDownload: " << req.getVerb() << ' ' << host() << ':' << port() << ' '
                                   << req.getUrl());

        newRequest(req, false);

        if (!saveToFilePath.empty())
            _response->saveBodyToFile(saveToFilePath);

        syncRequestImpl(poller);
        return _response;
    }

    /// Make a synchronous request to download a file to the given path.
    const std::shared_ptr<const Response> syncDownload(const Request& req,
                                                       const std::string& saveToFilePath)
    {
        std::shared_ptr<TerminatingPoll> poller(std::make_shared<TerminatingPoll>("HttpSynReqPoll"));
        poller->runOnClientThread();
        return syncDownload(req, saveToFilePath, *poller);
    }

    /// Make a synchronous request.
    /// The payload body of the response, if any, can be read via getBody().
    const std::shared_ptr<const Response> syncRequest(const Request& req, SocketPoll& poller)
    {
        LOG_TRC_S("syncRequest: " << req.getVerb() << ' ' << host() << ':' << port() << ' '
                                  << req.getUrl());

        newRequest(req, false);
        syncRequestImpl(poller);
        return _response;
    }

    /// Make a synchronous request.
    /// The payload body of the response, if any, can be read via getBody().
    const std::shared_ptr<const Response> syncRequest(const Request& req)
    {
        std::shared_ptr<TerminatingPoll> poller(std::make_shared<TerminatingPoll>("HttpSynReqPoll"));
        poller->runOnClientThread();
        return syncRequest(req, *poller);
    }

    /// Make a synchronous request with the given timeout.
    /// After returning the timeout set by setTimeout is restored.
    const std::shared_ptr<const Response> syncRequest(const Request& req,
                                                      std::chrono::milliseconds timeout)
    {
        LOG_TRC("syncRequest: " << req.getVerb() << ' ' << host() << ':' << port() << ' '
                                << req.getUrl());

        const auto origTimeout = getTimeout();
        setTimeout(timeout);

        auto responsePtr = syncRequest(req);

        setTimeout(origTimeout);

        return responsePtr;
    }

    /// Start an asynchronous request on the given SocketPoll.
    /// Return true when it dispatches the socket to the SocketPoll.
    /// Use asyncShutdownOnFinish of true to shutdown when finished (typical).
    /// Use asyncShutdownOnFinish of false to leave socket open to reuse.
    /// Note: when reusing this Session, it is assumed that the socket
    /// is already added to the SocketPoll on a previous call (do not
    /// use multiple SocketPoll instances on the same Session).
    /// Returns false when it fails to start the async request.
    bool asyncRequest(const Request& req, const std::weak_ptr<SocketPoll>& poll, bool asyncShutdownOnFinish = true)
    {
        std::shared_ptr<SocketPoll> socketPoll(poll.lock());
        if (!socketPoll)
        {
            LOG_ERR("Cannot start new asyncRequest without a valid SocketPoll: "
                    << req.getVerb() << ' ' << host() << ':' << port() << ' ' << req.getUrl());

            if (_onConnectFail)
            {
                // Call directly since we haven't started the async
                // connect to pass the validation in callOnConnectFail().
                _onConnectFail(shared_from_this());
            }

            return false;
        }

        LOG_TRC("New asyncRequest on [" << socketPoll->name() << "]: " << req.getVerb() << ' '
                                        << host() << ':' << port() << ' ' << req.getUrl());

        newRequest(req, asyncShutdownOnFinish);

        if (!isConnected())
        {
            asyncConnect(poll);
        }
        else
        {
            // Technically, there is a race here. The socket can
            // get disconnected and removed right after isConnected.
            // In that case, we will timeout and no request will be sent.
            socketPoll->wakeup();
        }

        LOG_DBG("Starting asyncRequest on [" << socketPoll->name() << "]: " << req.getVerb() << ' '
                                             << host() << ':' << port() << ' ' << req.getUrl());
        return true;
    }

    void asyncShutdown()
    {
        LOG_TRC("asyncShutdown");
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
        {
            socket->asyncShutdown();
        }
    }

    std::string getSslVerifyMessage() const
    {
#if ENABLE_SSL
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
            return SslStreamSocket::getSslVerifyString(socket->getSslVerifyResult());
        return SslStreamSocket::getSslVerifyString(_handshakeSslVerifyFailure);
#else
        return std::string();
#endif
    }

    long getSslVerifyResult() const
    {
#if ENABLE_SSL
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
            return socket->getSslVerifyResult();
        return _handshakeSslVerifyFailure;
#else
        return 0; // X509_V_OK
#endif
    }

    std::string getSslCert(std::string& subjectHash) const
    {
#if ENABLE_SSL
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
            return socket->getSslCert(subjectHash);
#else
        (void) subjectHash;
#endif
        return std::string();
    }

    net::AsyncConnectResult connectionResult() const
    {
        return _result;
    }

    /// Returns the socket FD, for logging/informational purposes.
    int getFD() const { return _fd; }

    void dumpState(std::ostream& os, const std::string& indent) const override
    {
        const auto now = std::chrono::steady_clock::now();
        os << indent << "http::Session: #" << _fd << " (" << (_socket.lock() ? "have" : "no")
           << " socket)";
        os << indent << "\tconnected: " << _connected;
        os << indent << "\tasyncShutdownOnFinish: " << _asyncShutdownOnFinish;
        os << indent << "\ttimeout: " << _timeout;
        os << indent << "\thost: " << _host;
        os << indent << "\tport: " << _port;
        os << indent << "\tprotocol: " << name(_protocol);
        os << indent << "\thandshakeSslVerifyFailure: " << _handshakeSslVerifyFailure;
        os << indent << "\tstartTime: " << Util::getTimeForLog(now, _startTime);
        _request.dumpState(os, indent + '\t');
        if (_response)
            _response->dumpState(os, indent + '\t');
        else
            os << indent << "\tresponse: null";

        os << '\n';

        // We are typically called from the StreamSocket, so don't
        // recurse back by calling dumpState on the socket again.
    }

private:
    void logPrefix(std::ostream& os) const { os << '#' << _fd << ": "; }

    /// Make a synchronous request.
    bool syncRequestImpl(SocketPoll& poller)
    {
        const std::chrono::microseconds timeout = getTimeout();
        const auto deadline = std::chrono::steady_clock::now() + timeout;

        assert(!!_response && "Response must be set!");

        if (!isConnected())
        {
            std::shared_ptr<StreamSocket> socket = connect();
            if (!socket)
            {
                LOG_ERR("Failed to connect to " << _host << ':' << _port);
                return false;
            }

            poller.insertNewSocket(socket);
        }

        LOG_TRC("Starting syncRequest: " << _request.getVerb() << ' ' << host() << ':' << port()
                                         << ' ' << _request.getUrl());

        poller.poll(timeout);
        while (!_response->done())
        {
            const auto now = std::chrono::steady_clock::now();
            if (checkTimeout(now))
                return false;

            const auto remaining =
                std::chrono::duration_cast<std::chrono::microseconds>(deadline - now);
            poller.poll(remaining);
        }

        return _response->state() == Response::State::Complete;
    }

    void callOnFinished()
    {
        if (_asyncShutdownOnFinish)
            asyncShutdown();

        if (!_onFinished)
            return;

        LOG_TRC("onFinished calling client");
        std::shared_ptr<Session> self = shared_from_this();
        try
        {
            [[maybe_unused]] const long references = self.use_count();
            assert(references > 1 && "Expected more than 1 reference to http::Session.");

            _onFinished(self);

            assert(self.use_count() > 1 &&
                    "Erroneously onFinish reset 'this'. Use 'addCallback()' on the "
                    "SocketPoll to reset on idle instead.");
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Error while invoking onFinished client callback: " << exc.what());
        }
    }

    /// Set up a new request and response.
    void newRequest(const Request& req, bool asyncShutdownOnFinish)
    {
        _startTime = std::chrono::steady_clock::now();

        // Called when the response is finished.
        // We really need only delegate it to our client.
        // We need to do this extra hop because Response
        // doesn't have our (Session) reference. Also,
        // it's good that we are notified that the request
        // has retired, so we can perform housekeeping.
        Response::FinishedCallback onFinished = [this]()
        {
            LOG_TRC("onFinished");
            assert(_response && "Must have response object");
            assert(_response->state() != Response::State::New &&
                   "Unexpected response in New state");
            assert(_response->state() != Response::State::Incomplete &&
                   "Unexpected response in Incomplete state");
            assert(_response->done() && "Must have response in done state");

            callOnFinished();

            if (_response->header().getConnectionToken() == Header::ConnectionToken::Close)
            {
                LOG_TRC("Our peer has sent the 'Connection: close' token. Disconnecting.");
                onDisconnect();
                assert(isConnected() == false);
            }
        };

        _response.reset();
        _response = std::make_shared<Response>(onFinished, _fd);

        _request = req;

        _asyncShutdownOnFinish = asyncShutdownOnFinish;

        std::string host = _host;

        if (_port != "80" && _port != "443")
        {
            host.push_back(':');
            host.append(_port);
        }
        _request.set("Host", std::move(host)); // Make sure the host is set.
        _request.set("Date", Util::getHttpTimeNow());
        _request.set("User-Agent", http::getAgentString());
    }

    void onConnect(const std::shared_ptr<StreamSocket>& socket) override
    {
        ASSERT_CORRECT_THREAD();

        if (socket)
        {
            _fd = socket->getFD();
            _response->setLogContext(_fd);
            LOG_TRC("Connected");
            _connected = true;
        }
        else
        {
            LOG_DBG("Error: onConnect without a valid socket");
            _fd = -1;
            _handshakeSslVerifyFailure = 0;
            _connected = false;
        }
    }

    void shutdown(bool /*goingAway*/, const std::string_view /*statusMessage*/) override
    {
        LOG_TRC("shutdown");
    }

    void getIOStats(uint64_t& sent, uint64_t& recv) override
    {
        LOG_TRC("getIOStats");
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
            socket->getIOStats(sent, recv);
        else
        {
            sent = 0;
            recv = 0;
        }
    }

    int getPollEvents(std::chrono::steady_clock::time_point /*now*/,
                      int64_t& /*timeoutMaxMicroS*/) override
    {
        ASSERT_CORRECT_THREAD();
        int events = POLLIN;
        if (_request.stage() != Request::Stage::Finished)
            events |= POLLOUT;
        return events;
    }

    void handleIncomingMessage(SocketDisposition& disposition) override
    {
        LOG_TRC("handleIncomingMessage");
        ASSERT_CORRECT_THREAD();
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (isConnected() && socket)
        {
            // Consume the incoming data by parsing and processing the body.
            Buffer& data = socket->getInBuffer();
            if (data.empty())
            {
                LOG_DBG("No data to process from the socket");
                return;
            }

            LOG_TRC("HandleIncomingMessage: buffer has:\n"
                    << HexUtil::dumpHex(
                           std::string(data.data(), std::min<size_t>(data.size(), 256UL))));

#if !(defined QTAPP || defined _WIN32 || defined(MACOS))
            // Response::readData is not build with CODA.
            const int64_t read = _response->readData(data.data(), data.size());
            if (read >= 0)
            {
                // Remove consumed data.
                if (read)
                    data.eraseFirst(read);
                return;
            }
#endif
        }
        else
        {
            LOG_ERR("handleIncomingMessage called when not connected");
            assert(!socket && "Expected no socket when not connected");
            assert(!isConnected() && "Expected not connected when no socket");
        }

        // Protocol error: Interrupt the transfer.
        disposition.setClosed();
        onDisconnect();
    }

    void performWrites(std::size_t capacity) override
    {
        ASSERT_CORRECT_THREAD();
        // We may get called after disconnecting and freeing the Socket instance.
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
        {
            Buffer& out = socket->getOutBuffer();
            LOG_TRC("performWrites: sending request (buffered: "
                    << out.size() << " bytes, capacity: " << capacity << ')');

#if !(defined QTAPP || defined _WIN32 || defined(MACOS))
            // StreamSocket::send(...) is excluded in CODA.
            if (!socket->send(_request))
            {
                _result = net::AsyncConnectResult::SocketError;
                LOG_ERR("Error while writing to socket");
            }
#endif
        }
    }

    std::shared_ptr<Session> shared_from_this()
    {
        return std::static_pointer_cast<Session>(ProtocolHandlerInterface::shared_from_this());
    }

    void callOnConnectFail()
    {
        if (!_onConnectFail)
            return;

        std::shared_ptr<Session> self = shared_from_this();
        try
        {
            [[maybe_unused]] const long references = self.use_count();
            assert(references > 1 && "Expected more than 1 reference to http::Session.");

            _onConnectFail(self);

            assert(self.use_count() > 1 &&
                    "Erroneously onConnectFail reset 'this'. Use 'addCallback()' on the "
                    "SocketPoll to reset on idle instead.");
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Error while invoking onConnectFail client callback: " << exc.what());
        }
    }

    // on failure the stream will be discarded, so save the ssl verification
    // result while it is still available
    void onHandshakeFail() override
    {
        ASSERT_CORRECT_THREAD();
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
        {
            LOG_TRC("onHandshakeFail");
            _handshakeSslVerifyFailure = socket->getSslVerifyResult();
            _result = net::AsyncConnectResult::SSLHandShakeFailure;
        }

        callOnConnectFail();
    }

    void onDisconnect() override
    {
        ASSERT_CORRECT_THREAD();
        // Make sure the socket is disconnected and released.
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
        {
            LOG_TRC("onDisconnect");
            socket->asyncShutdown(); // Flag for shutdown for housekeeping in SocketPoll.
            socket->shutdownConnection(); // Immediately disconnect.
            _socket.reset();
        }

        _connected = false;
        if (_response)
            _response->error();

        _fd = -1; // No longer our socket fd.
    }

    std::shared_ptr<StreamSocket> connect()
    {
        ASSERT_CORRECT_THREAD();
        _socket.reset(); // Reset to make sure we are disconnected.
        std::shared_ptr<StreamSocket> socket =
            net::connect(_host, _port, isSecure(), shared_from_this());
        assert((!socket || _fd == socket->getFD()) &&
               "The socket FD must have been set in onConnect");

        // When used with proxy.php we may indeed get nullptr here.
        // assert(socket && "Unexpected nullptr returned from net::connect");
        _socket = socket; // Hold a weak pointer to it.
        return socket; // Return the shared pointer.
    }

    void asyncConnectFailed(net::AsyncConnectResult result)
    {
        assert(!_socket.use_count());
        _result = result;

        LOG_ERR("Failed to connect to " << _host << ':' << _port);
        callOnConnectFail();
    }

    void asyncConnectSuccess(const std::shared_ptr<StreamSocket> &socket, net::AsyncConnectResult result)
    {
        ASSERT_CORRECT_THREAD();
        assert(socket && _fd == socket->getFD() && "The socket FD must have been set in onConnect");

        _socket = socket; // Hold a weak pointer to it.
        _result = result;

        LOG_ASSERT_MSG(_socket.lock(), "Connect must set the _socket member.");
        LOG_ASSERT_MSG(_socket.lock()->getFD() == socket->getFD(),
                       "Socket FD's mismatch after connect().");
    }

    void asyncConnect(const std::weak_ptr<SocketPoll>& poll)
    {
        ASSERT_CORRECT_THREAD();
        _socket.reset(); // Reset to make sure we are disconnected.

        auto pushConnectCompleteToPoll =
            [this, poll](std::shared_ptr<StreamSocket> socket, net::AsyncConnectResult result)
        {
            std::shared_ptr<SocketPoll> socketPoll(poll.lock());
            if (!socketPoll || !socketPoll->isAlive())
            {
                LOG_WRN("asyncConnect completed after poll " << (!socketPoll ? "destroyed" : "finished"));
                return;
            }

            if (!socket)
            {
                // When used with proxy.php we may indeed get nullptr here.
                socketPoll->addCallback([selfLifecycle = shared_from_this(), this, result]()
                                        { asyncConnectFailed(result); });
                return;
            }

            SocketDisposition disposition(socket);
            disposition.setTransfer(*socketPoll,
                                    [selfLifecycle = shared_from_this(), this,
                                     socket = std::move(socket),
                                     result]([[maybe_unused]] const std::shared_ptr<Socket>& moveSocket)
                                    {
                                        assert(socket == moveSocket);
                                        asyncConnectSuccess(socket, result);
                                    });
            disposition.execute();
        };

        net::asyncConnect(_host, _port, isSecure(), shared_from_this(), pushConnectCompleteToPoll);
    }

    bool checkTimeout(std::chrono::steady_clock::time_point now) override
    {
        ASSERT_CORRECT_THREAD();
        if (!_response || _response->done())
            return false;

        const std::chrono::microseconds timeout = getTimeout();
        const auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime);

        if (now < _startTime ||
            (timeout > std::chrono::microseconds::zero() && duration > timeout) ||
            SigUtil::getTerminationFlag())
        {
            LOG_WRN("CheckTimeout: Timeout while requesting [" << _request.getVerb() << ' ' << _host
                                                               << _request.getUrl() << "] after " << duration);

            // Flag that we timed out.
            _response->timeout();

            // Disconnect and trigger the right events and handlers.
            // Note that this is the right way to end a request in HTTP, it's also
            // no good maintaining a poor connection (if that's the issue).
            onDisconnect(); // Trigger manually (why wait for poll to do it?).
            assert(isConnected() == false);
            return true;
        }
        return false;
    }

    int sendTextMessage(std::string_view, bool) const override { return 0; }
    int sendBinaryMessage(std::string_view, bool) const override { return 0; }

private:
    const std::string _host;
    const std::string _port;
    const Protocol _protocol;
    int _fd; ///< The socket file-descriptor.
    long _handshakeSslVerifyFailure; ///< Save SslVerityResult at onHandshakeFail
    std::chrono::microseconds _timeout;
    std::chrono::steady_clock::time_point _startTime;
    bool _connected;
    bool _asyncShutdownOnFinish;
    Request _request;
    net::AsyncConnectResult _result; // last connection tentative result
    FinishedCallback _onFinished;
    ConnectFailCallback _onConnectFail;
    std::shared_ptr<Response> _response;
    /// Keep _socket as last member so it is destructed first, ensuring that
    /// the peer members it depends on are not destructed before it
    std::weak_ptr<StreamSocket> _socket;
};

/// HTTP Get a URL synchronously.
inline const std::shared_ptr<const http::Response>
get(const std::string& url, std::chrono::milliseconds timeout = Session::getDefaultTimeout())
{
    auto httpSession = http::Session::create(url);
    return httpSession->syncRequest(http::Request(std::string(net::parseUrl(url))), timeout);
}

/// HTTP Get synchronously given a url and a path.
inline const std::shared_ptr<const http::Response>
get(const std::string& url, const std::string& path,
    std::chrono::milliseconds timeout = Session::getDefaultTimeout())
{
    auto httpSession = http::Session::create(url);
    return httpSession->syncRequest(http::Request(path), timeout);
}

inline std::ostream& operator<<(std::ostream& os, const http::Header& header)
{
    Util::joinPair(os, header, " / ");

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const http::StatusCode& statusCode)
{
    os << static_cast<int>(statusCode) << " (" << getReasonPhraseForCode(statusCode) << ')';
    return os;
}

inline std::ostringstream& operator<<(std::ostringstream& os, const http::StatusCode& statusCode)
{
    os << static_cast<int>(statusCode) << " (" << getReasonPhraseForCode(statusCode) << ')';
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const http::Header::ConnectionToken& token)
{
    os << http::Header::name(token);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const http::FieldParseState& fieldParseState)
{
    os << http::name(fieldParseState);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const http::Request::Stage& stage)
{
    os << http::Request::name(stage);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const http::Response::State& state)
{
    os << http::Response::name(state);
    return os;
}

/// Format seconds with the units suffix until we migrate to C++20.
inline std::ostream& operator<<(std::ostream& os, const std::chrono::seconds& s)
{
    os << s.count() << 's';
    return os;
}

/// Format milliseconds with the units suffix until we migrate to C++20.
inline std::ostream& operator<<(std::ostream& os, const std::chrono::milliseconds& ms)
{
    os << ms.count() << "ms";
    return os;
}

/// Format microseconds with the units suffix until we migrate to C++20.
inline std::ostream& operator<<(std::ostream& os, const std::chrono::microseconds& ms)
{
    os << ms.count() << "us";
    return os;
}

} // namespace http

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
