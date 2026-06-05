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
 * Implementation of request detail parsing.
 * Classes: RequestDetails
 */

#include <config.h>

#include "RequestDetails.hpp"

#include <common/HexUtil.hpp>
#include <common/Log.hpp>
#include <common/Util.hpp>
#if !MOBILEAPP
#include <wsd/HostUtil.hpp>
#endif // !MOBILEAPP
#include <wsd/Exceptions.hpp>

#include <Poco/URI.h>

#include <cctype>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace
{

std::map<std::string, std::string> getParams(const std::string& uri)
{
    std::map<std::string, std::string> result;
    for (const auto& param : Poco::URI(uri).getQueryParameters())
    {
        // getQueryParameters() decodes the values. Compare with the URI.
        if (param.first == "WOPISrc" && uri.find(param.second) != std::string::npos)
        {
            LOG_WRN("WOPISrc validation error: unencoded WOPISrc [" << param.second
                                                                    << "] in URL: " << uri);
            if constexpr (Util::isDebugEnabled())
            {
                throw std::runtime_error("WOPISrc must be URI-encoded");
            }
        }

        if (!param.first.empty())
        {
            LOG_TRC("Found param [" << param.first << "] = [" << param.second << ']');
            result.emplace(param);
        }
    }

    return result;
}
}

RequestDetails::RequestDetails(Poco::Net::HTTPRequest &request, const std::string& serviceRoot)
{
    // Check and remove the ServiceRoot from the request.getURI()
    if (!request.getURI().starts_with(serviceRoot))
        throw BadRequestException("The request does not start with prefix: " + serviceRoot);

    // re-writes ServiceRoot out of request
    _uriString = request.getURI().substr(serviceRoot.length());
    dehexify();
    request.setURI(_uriString);
    _method = stringToMethod(request.getMethod());
    auto it = request.find("ProxyPrefix");
    _isProxy = it != request.end();
    if (_isProxy)
        _proxyPrefix = it->second;
    it = request.find("Upgrade");
    _isWebSocket = it != request.end() && Util::iequal(it->second, "websocket");
    _closeConnection = !request.getKeepAlive(); // HTTP/1.1: closeConnection true w/ "Connection: close" only!
    // request.getHost fires an exception on mobile.
    if constexpr (!Util::isMobileApp())
        _hostUntrusted = request.getHost();

    processURI();
}

RequestDetails::RequestDetails(http::RequestParser& request, const std::string& serviceRoot)
{
    // Check and remove the ServiceRoot from the request.getURI()
    if (!request.getUrl().starts_with(serviceRoot))
        throw BadRequestException("The request does not start with prefix: " + serviceRoot);

    // re-writes ServiceRoot out of request
    _uriString = request.getUrl().substr(serviceRoot.length());
    dehexify();
    request.setUrl(_uriString);
    _method = stringToMethod(request.getVerb());
    _isProxy = request.has("ProxyPrefix");
    if (_isProxy)
        _proxyPrefix = request.get("ProxyPrefix");
    _isWebSocket = Util::iequal(request.get("Upgrade"), "websocket");
    _closeConnection =
        !request.isKeepAlive(); // HTTP/1.1: closeConnection true w/ "Connection: close" only!
    // request.getHost fires an exception on mobile.
    if constexpr (!Util::isMobileApp())
        _hostUntrusted = request.get("Host");

    processURI();
}

RequestDetails::RequestDetails(std::string mobileURI)
    : _uriString(std::move(mobileURI))
    , _method(Method::GET)
    , _isProxy(false)
    , _isWebSocket(false)
    , _closeConnection(false)
{
    dehexify();
    processURI();
}

RequestDetails::RequestDetails(const std::string& wopiSrc, const std::vector<std::string>& options,
                               const std::string& compat)
    : _method(Method::GET)
    , _isProxy(false)
    , _isWebSocket(false)
    , _closeConnection(false)
{
    // /cool/<encoded-document-URI+options>/ws?WOPISrc=<encoded-document-URI>&compat=/ws[/<sessionId>/<command>/<serial>]

    const std::string decodedWopiSrc = Uri::decode(wopiSrc);
    std::string wopiSrcWithOptions = decodedWopiSrc;
    if (!options.empty())
    {
        wopiSrcWithOptions += '?';
    }

    for (const std::string& option : options)
    {
        wopiSrcWithOptions += option;
        wopiSrcWithOptions += '&';
    }

    // To avoid duplicating the complex logic in processURI(),
    // and to have a single canonical implementation, we
    // create a valid URI and let it parse and set the various
    // members, as necessary.
    std::ostringstream oss;
    oss << "/cool/" << Uri::encode(wopiSrcWithOptions);
    oss << "/ws?WOPISrc=" << Uri::encode(decodedWopiSrc);
    oss << "&compat=/ws" << compat;
    _uriString = oss.str();

    processURI();
}

RequestDetails::Method RequestDetails::stringToMethod(const std::string_view method)
{
    if (method == "GET") {
        return Method::GET;
    } else if (method == "HEAD") {
        return Method::HEAD;
    } else if (method == "POST") {
        return Method::POST;
    } else {
        return Method::unknown;
    }
}

void RequestDetails::dehexify()
{
    // For now, we only hexify cool/ URLs.
    constexpr std::string_view Prefix = "cool/0x";
    constexpr auto PrefixLen = Prefix.size();

    const auto hexPos = _uriString.find(Prefix);
    if (hexPos != std::string::npos)
    {
        // The start of the hex token.
        const auto start = hexPos + PrefixLen;
        // Find the next '/' after the hex token.
        const auto end = _uriString.find_first_of('/', start);

        std::string res = _uriString.substr(0, start - 2); // The prefix, without '0x'.

        const std::string encoded =
            _uriString.substr(start, (end == std::string::npos) ? end : end - start);
        std::string decoded;
        HexUtil::dataFromHexString(encoded, decoded);
        res += decoded;

        res += _uriString.substr(end); // Concatenate the remainder.

        _uriString = std::move(res); // Replace the original uri with the decoded one.
    }
}

void RequestDetails::processURI()
{
    // Poco::SyntaxException is thrown when the syntax is invalid.
    _params = getParams(_uriString);

    // First tokenize by '/' then by '?'.
    std::vector<StringToken> tokens;
    const auto len = _uriString.size();
    if (len > 0)
    {
        std::size_t i, start;
        for (i = start = 0; i < len; ++i)
        {
            if (_uriString[i] == '/' || _uriString[i] == '?')
            {
                if (i - start > 0) // ignore empty
                    tokens.emplace_back(start, i - start);
                start = i + 1;
            }
        }
        if (i - start > 0) // ignore empty
            tokens.emplace_back(start, i - start);
        _pathSegs = StringVector(_uriString, std::move(tokens));
    }


    std::size_t off = 0;
    std::size_t posDocUri = _uriString.find_first_of('/');
    if (posDocUri == 0)
    {
        off = 1;
        posDocUri = _uriString.find_first_of('/', 1);
    }

    _fields[Field::Type] = _uriString.substr(off, posDocUri - off); // The first is always the type.
    const std::string uriRes =
        (posDocUri == std::string::npos) ? std::string() : _uriString.substr(posDocUri + 1);

    // Cool URI 2.0 starts with /cool/ws.
    const bool isCool2 = _pathSegs.equals(0, "cool") && _pathSegs.equals(1, "ws");

    if (_pathSegs.equals(0, "cool") || _pathSegs.equals(0, "wasm"))
    {
        // Find the DocumentURI proper.
        std::size_t end = uriRes.find_first_of("/?", 0, 2);
        _fields[Field::DocumentURI] = Uri::decode(uriRes.substr(0, end));
    }
    else // Otherwise, it's the full URI.
    {
        _fields[Field::DocumentURI] = _uriString;
    }

    _docUriParams = getParams(_fields[Field::DocumentURI]);

    _fields[Field::WOPISrc] = getParam("WOPISrc");

    // DocumentURI is the second segment in cool URIs.
    if (isCool2)
    {
        // The params are now part of the main URI, not a sub-section (i.e. Document-URI).
        _docUriParams = _params;
        _docUriParams.erase("WOPISrc"); // Remove circular reference.
        _docUriParams.erase("compat"); // This is internal (for proxies), not for the host.
        std::string docUri;
        docUri.reserve(_fields[Field::WOPISrc].size() + _docUriParams.size());
        docUri = _fields[Field::WOPISrc];
        bool first = true;
        for (const auto& [key, value] : _docUriParams)
        {
            docUri += first ? '?' : '&';
            docUri += key;
            docUri += '=';
            docUri += Uri::encode(value);
            first = false;
        }

        _fields[Field::DocumentURI] = std::move(docUri);
    }

    // &compat=
    std::string compat = getParam("compat");
    std::string lastWS = compat;
    if (!compat.empty())
        _fields[Field::Compat] = std::move(compat);

    if (!isCool2)
    {
        // /ws[/<sessionId>/<command>/<serial>]
        const auto posLastWS = uriRes.rfind("/ws");
        if (posLastWS != std::string::npos)
        {
            lastWS = uriRes.substr(posLastWS + 3);
        }
    }

    const auto proxyTokens = StringVector::tokenize(std::move(lastWS), '/');
    if (proxyTokens.size() > 0)
    {
        _fields[Field::SessionId] = proxyTokens[0];
        if (proxyTokens.size() > 1)
        {
            _fields[Field::Command] = proxyTokens[1];
            if (proxyTokens.size() > 2)
            {
                _fields[Field::Serial] = proxyTokens[2];
            }
        }
    }
}

Poco::URI RequestDetails::sanitizeURI(const std::string& uri)
{
    const std::string decoded = Uri::decode(uri);

    // Detect local file paths before constructing Poco::URI, because a bare '%'
    // (from the WebSocket URL decode of %25) would cause Poco::URI to throw.
    if (decoded[0] == '/' || decoded.starts_with("file://"))
    {
        // Any remaining '%' after the decode is literal, not URI encoding.
        return sanitizeLocalPath(decoded);
    }

    Poco::URI uriPublic(decoded);

    if (uriPublic.getPath().empty())
    {
        throw std::runtime_error("Invalid URI.");
    }

    // We decoded access token before embedding it in cool.html
    // So, we need to decode it now to get its actual value
    Poco::URI::QueryParameters queryParams = uriPublic.getQueryParameters();
    for (auto& param : queryParams)
    {
        // look for encoded query params (access token as of now)
        if (param.first == "access_token")
        {
            param.second = Uri::decode(param.second);
        }
    }

    uriPublic.setQueryParameters(queryParams);

    LOG_DBG("Sanitized URI [" << uri << "] to [" << uriPublic.toString() << ']');
    return uriPublic;
}

Poco::URI RequestDetails::sanitizeLocalPath(const std::string& path)
{
    // For local file paths, '%' is always a literal character, never URI encoding.
    // Encode every '%' so that Poco::URI's automatic decoding restores the originals.
    Poco::URI uriPublic(Uri::encodeAllPercent(path));

    if (uriPublic.isRelative() || uriPublic.getScheme() == "file")
    {
        uriPublic.normalize();
#ifdef _WIN32
        std::string p = uriPublic.getPath();
        if (p.length() > 4 && p[0] == '/' && std::isalpha(p[1]) && p[2] == ':' && p[3] == '/')
            uriPublic.setPath(p.substr(1));
#endif
    }

    if (uriPublic.getPath().empty())
    {
        throw std::runtime_error("Invalid URI.");
    }

    LOG_DBG("Sanitized local path [" << path << "] to [" << uriPublic.toString() << ']');
    return uriPublic;
}

std::string RequestDetails::getLineModeKey(const std::string& /*access_token*/) const
{
    // This key is based on the WOPISrc and the access_token only.
    // However, we strip the host:port and scheme from the WOPISrc.
    return Poco::URI(getField(RequestDetails::Field::WOPISrc)).getPath();
}

#if !defined(BUILDING_TESTS)
std::string RequestDetails::getDocKey(const Poco::URI& uri)
{
    // resolve aliases
#if !MOBILEAPP
    const std::string newUri = HostUtil::getNewUri(uri);
    if (newUri != uri.toString())
    {
        LOG_TRC("Canonicalized URI [" << uri.toString() << "] to [" << newUri << ']');
    }
#else
    const std::string& newUri = uri.getPath();
#endif

    std::string docKey = Uri::encode(newUri);
    LOG_INF("DocKey from URI [" << uri.toString() << "] => [" << docKey << ']');
    return docKey;
}
#endif // !defined(BUILDING_TESTS)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
