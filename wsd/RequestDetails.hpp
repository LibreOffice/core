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
 * HTTP request parsing and details extraction.
 * Classes: RequestDetails
 */

#pragma once

#include <common/Log.hpp>
#include <common/StringVector.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>

/**
 * A class to encapsulate various useful pieces from the request.
 * as well as path parsing goodness.
 *
 * The URI is complex and encapsulates multiple segments with
 * different purposes and consumers. There are three URIs
 * formats/modes that are supported.
 *
 * literal URI: used by ConvertToBroker.
 * Origin: ConvertToBroker::startConversion
 * Format:
 *  <anything>
 * Identifier: special constructor that takes a string.
 * Example:
 *  convert-to
 *
 * browser URI: used to load cool.html and other static files.
 * Origin: the page where the document will be embedded.
 * Format:
 *  /browser/<coolwsd-version-hash>/[path/]<filename>.<ext>[?WOPISrc=<encoded-document-URI>]
 * Identifier: /browser/.
 * Examples:
 *  /browser/49c225146/src/map/Clipboard.js
 *  /browser/49c225146/cool.html?WOPISrc=http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2Ffiles%2F593_ocqiesh0cngs&title=empty.odt&lang=en-us&closebutton=1&revisionhistory=1
 *
 * cool URI: used to load the document (see below for newer format).
 * Origin: cool.html
 * Format (up-to 25.04):
 *  /cool/<encoded-document-URI+options>/ws?WOPISrc=<encoded-document-URI>&compat=/ws[/<sessionId>/<command>/<serial>]
 * Identifier: /cool/.
 *
 * The 'document-URI' is the original URL in the client that is used to load the document page.
 * The optional section at the end, in square-brackets, is for richproxy.
 *
 * Example:
 *  /cool/http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2Ffiles%2F165_ocgdpzbkm39u%3F
 *  access_token%3DODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ%26access_token_ttl%3D0/ws?
 *  WOPISrc=http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2F
 *  files%2F165_ocgdpzbkm39u&compat=/ws/1c99a7bcdbf3209782d7eb38512e6564/write/2
 *  Where:
 *      encoded-document-URI+options:
 *          http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2Ffiles%2F165_ocgdpzbkm39u%3F
 *          access_token%3DODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ%26access_token_ttl%3D0
 *      encoded-document-URI:
 *          http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2Ffiles%2F165_ocgdpzbkm39u
 *      sessionId:
 *          1c99a7bcdbf3209782d7eb38512e6564
 *      command:
 *          write
 *      serial:
 *          2
 *  In decoded form:
 *      document-URI+options:
 *          http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/165_ocgdpzbkm39u?access_token=
 *          ODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ&access_token_ttl=0
 *      document-URI:
 *          http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/165_ocgdpzbkm39u
 *
 * Note that the options are still encoded and need decoding separately.
 *
 * Due to the multi-layer nature of the URI, it raises many difficulties, not least
 * the fact that it has multiple query parameters ('?' sections). It also has slash
 * delimiters after query parameters.
 *
 * The different sections are henceforth given names to help both in documenting and
 * communicating them, and to facilitate parsing them.
 *
 * /cool/<encoded-document-URI+options>/ws?WOPISrc=<encoded-document-URI>&compat=/ws[/<sessionId>/<command>/<serial>]
 *       |--------documentURI---------|            |-------WOPISrc------|        |--------------compat--------------|
 *                            |options|                                               |sessionId| |command| |serial|
 *
 * Alternatively, the documentURI (encoded) could be hexified, as follows:
 * /cool/0x123456789/ws?WOPISrc=<encoded-document-URI>&compat=/ws[/<sessionId>/<command>/<serial>]
 *
 * cool URI 2.0: used to load the document.
 * Origin: cool.html
 * Format (26.04+):
 *  /cool/ws?WOPISrc=<encoded-document-URI>&<options>&compat=/ws[/<sessionId>/<command>/<serial>]
 * Identifier: /cool/.
 *
 * Example:
 *  /cool/ws?
 *  WOPISrc=http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2F
 *  files%2F165_ocgdpzbkm39u&access_token=ODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ&access_token_ttl=0
 *  &compat=%2F1c99a7bcdbf3209782d7eb38512e6564%2Fwrite%2F2
 *  Where:
 *      options:
 *          access_token=ODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ&access_token_ttl=0
 *      WOPISrc:
 *          http%3A%2F%2Flocalhost%2Fowncloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2Ffiles%2F165_ocgdpzbkm39u
 *      sessionId:
 *          1c99a7bcdbf3209782d7eb38512e6564
 *      command:
 *          write
 *      serial:
 *          2
 *  In decoded form:
 *      document-URI+options:
 *          http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/165_ocgdpzbkm39u?access_token=
 *          ODhIXdJdbsVYQoKKCuaYofyzrovxD3MQ&access_token_ttl=0
 *      document-URI:
 *          http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/165_ocgdpzbkm39u
 *
 * This is the new and simplified form. It avoids duplicate document-URI (once in the document-URI+options and once
 * in the WOPISrc), and makes the options more transparent.
 *
 * The different sections are henceforth given names to help both in documenting and
 * communicating them, and to facilitate parsing them.
 *
 * /cool/ws?WOPISrc=<encoded-document-URI>[&<options>][&compat=<encoded-sessionId-command-serial>
 *                  |-------WOPISrc------|  |options|          |--------------compat------------|
 *                                                             |sessionId|/|-command-|/|-serial-|
 *
 */
class RequestDetails
{
public:

    /// The fields of the URI.
    enum class Field : std::uint8_t
    {
        Type,
        DocumentURI,
        WOPISrc,
        Compat,
        SessionId,
        Command,
        Serial
    };

private:
    STATE_ENUM(Method, unknown, GET, HEAD, POST);

    StringVector _pathSegs;
    std::map<std::string, std::string> _params;
    std::map<Field, std::string> _fields;
    std::map<std::string, std::string> _docUriParams;
    std::string _uriString;
    std::string _proxyPrefix;
    std::string _hostUntrusted;
    Method _method;
    bool _isProxy : 1;
    bool _isWebSocket : 1;
    bool _closeConnection : 1;

    static Method stringToMethod(std::string_view method);

    void dehexify();
    void processURI();

public:
    RequestDetails(Poco::Net::HTTPRequest &request, const std::string& serviceRoot);
    RequestDetails(http::RequestParser& request, const std::string& serviceRoot);
    explicit RequestDetails(std::string mobileURI);

    /// Constructs from its components.
    /// wopiSrc is typically encoded.
    /// options are in the form of 'x=y' strings.
    /// compat is in the form of '/sessionId/command/serial' string. Optional.
    /// /cool/<encoded-document-URI+options>/ws?WOPISrc=<encoded-document-URI>&compat=/ws[/<sessionId>/<command>/<serial>]
    RequestDetails(const std::string& wopiSrc, const std::vector<std::string>& options,
                   const std::string& compat);

    /// Decode and sanitize a URI.
    static Poco::URI sanitizeURI(const std::string& uri);

    /// Sanitize a local file path where '%' is always a literal character.
    static Poco::URI sanitizeLocalPath(const std::string& path);

    /// Returns a document-specific key, based
    /// on the URI of the document (aka the wopiSrc).
    static std::string getDocKey(const Poco::URI& uri);

    /// Sanitize the URI and return the document-specific key.
    static std::string getDocKey(const std::string& uri) { return getDocKey(sanitizeURI(uri)); }

    /// Returns false if the WOPISrc is not encoded correctly.
    static bool validateWOPISrc(const std::string& uri) { return !Uri::needsEncoding(uri); }

    /// This is a per-document, per-user request key.
    /// If a user makes two requests on the same document at the same time,
    /// they will have the same request-key and we won't differentiate between them.
    static std::string getRequestKey(const std::string& wopiSrc, const std::string& accessToken)
    {
        const std::string decodedWopiSrc = Uri::decode(wopiSrc);
        const Poco::URI wopiSrcSanitized = RequestDetails::sanitizeURI(decodedWopiSrc);

        std::string requestKey = RequestDetails::getDocKey(wopiSrcSanitized);
        requestKey += '_';
        requestKey += accessToken;

        return requestKey;
    }

    /// This is a per-document, per-user request key.
    [[nodiscard]] std::string getRequestKey() const
    {
        const std::string wopiSrc = getField(RequestDetails::Field::WOPISrc);
        if (!wopiSrc.empty())
        {
            std::string accessToken;
            getParamByName("access_token", accessToken);

            return getRequestKey(wopiSrc, accessToken);
        }

        return std::string();
    }

    /// The DocumentURI, decoded. Doesn't contain WOPISrc or any other appendages.
    [[nodiscard]] std::string getDocumentURI() const { return getField(Field::DocumentURI); }

    /// Returns the document-specific key from the DocumentURI.
    [[nodiscard]] std::string getDocKey() const
    {
        return RequestDetails::getDocKey(RequestDetails::sanitizeURI(getDocumentURI()));
    }

    /// The DocumentURI, decoded and sanitized. Doesn't contain WOPISrc or any other appendages.
    [[nodiscard]] std::string getDocumentURISanitized() const
    {
        return sanitizeURI(getField(Field::DocumentURI)).toString();
    }

    /// Returns a key to be used with Online/Offline mode.
    /// This is based on the WOPISrc path + access_token.
    [[nodiscard]] std::string getLineModeKey(const std::string& access_token) const;

    const std::map<std::string, std::string>& getDocumentURIParams() const { return _docUriParams; }

    /// Returns a param, if it exists.
    bool getParamByName(const std::string& name, std::string& value) const
    {
        const auto it = _docUriParams.find(name);
        if (it != _docUriParams.end())
        {
            value = it->second;
            return true;
        }

        return false;
    }

    const std::string& getURI() const
    {
        return _uriString;
    }
    bool isProxy() const
    {
        return _isProxy;
    }
    const std::string& getProxyPrefix() const
    {
        return _proxyPrefix;
    }
    const std::string& getHostUntrusted() const
    {
        return _hostUntrusted;
    }
    bool isWebSocket() const
    {
        return _isWebSocket;
    }
    bool closeConnection() const
    {
        return _closeConnection;
    }
    bool isGet() const
    {
        return _method == Method::GET;
    }
    bool isGet(const char *path) const
    {
        return _method == Method::GET && _uriString == path;
    }
    bool isGetOrHead(const char *path) const
    {
        return (_method == Method::GET || _method == Method::HEAD) && _uriString == path;
    }
    bool isPost() const
    {
        return _method == Method::POST;
    }

    bool equals(std::size_t index, const std::string_view string) const
    {
        return _pathSegs.equals(index, string);
    }

    /// Return the segment of the URI at index.
    /// URI segments are delimited by '/'.
    std::string operator[](std::size_t index) const
    {
        return _pathSegs[index];
    }

    /// Returns the number of segments in the URI.
    std::size_t size() const
    {
        return _pathSegs.size();
    }

    std::string getParam(const std::string& name) const
    {
        const auto it = _params.find(name);
        return it != _params.end() ? it->second : std::string();
    }

    std::string getField(const Field field) const
    {
        const auto it = _fields.find(field);
        return it != _fields.end() ? it->second : std::string();
    }

    bool equals(const Field field, const std::string_view string) const
    {
        const auto it = _fields.find(field);
        return it != _fields.end() ? it->second == string : string.empty();
    }

    bool operator==(const RequestDetails& rhs) const
    {
        if (_method != rhs._method)
            return false;
        if (_isProxy != rhs._isProxy)
            return false;
        if (_isWebSocket != rhs._isWebSocket)
            return false;
        if (_closeConnection != rhs._closeConnection)
            return false;
        if (_uriString != rhs._uriString)
            return false;
        if (_proxyPrefix != rhs._proxyPrefix)
            return false;
        if (_hostUntrusted != rhs._hostUntrusted)
            return false;

        return Util::equal(_params, rhs._params) && Util::equal(_fields, rhs._fields) &&
               Util::equal(_docUriParams, rhs._docUriParams) &&
               Util::equal(_pathSegs, rhs._pathSegs);
    }

    [[nodiscard]] std::string toString() const
    {
        std::ostringstream oss;
        oss << _uriString << ' ' << nameShort(_method)
            << (_isProxy?"Proxy":"")
            << (_isWebSocket?"WebSocket":"");
        oss << ", host: " << _hostUntrusted;
        oss << ", path: " << _pathSegs.size();
        for (std::size_t i = 0; i < _pathSegs.size(); ++i)
            oss << "\n[" << i << "] '" << _pathSegs[i] << '\'';
        oss << "\nfull URI: " << _uriString;
        return oss.str();
    }
};

inline std::ostream& operator<<(std::ostream& os, const RequestDetails& details)
{
    os << details.toString();
    return os;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
