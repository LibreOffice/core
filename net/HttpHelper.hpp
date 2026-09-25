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
 * HTTP utility functions for sending error responses and files.
 * Functions: sendError(), sendErrorAndShutdown(), sendFileAndShutdown()
 */

#pragma once

#include <common/Log.hpp>
#include <common/Uri.hpp>
#include <net/HttpRequest.hpp>
#include <net/NetUtil.hpp>

#include <Poco/URI.h>

#include <memory>
#include <string>

class StreamSocket;

namespace HttpHelper
{
/// Write headers and body for an error response.
inline void sendError(http::StatusCode errorCode, const std::shared_ptr<StreamSocket>& socket,
                      std::string_view body = std::string_view(),
                      std::string_view extraHeader = std::string_view())
{
    std::ostringstream oss;
    oss << "HTTP/1.1 " << errorCode << "\r\n"
           "Date: " << Util::getHttpTimeNow() << "\r\n"
           "Content-Length: " << body.size() << "\r\n"
           "X-Content-Type-Options: nosniff\r\n"
        << extraHeader << "\r\n"
        << body;
    socket->send(oss.str());
}

/// Write headers and body for an error response. Afterwards, shutdown the socket.
inline void sendErrorAndShutdown(http::StatusCode errorCode,
                                 const std::shared_ptr<StreamSocket>& socket,
                                 std::string_view body = std::string_view(),
                                 const std::string& extraHeader = std::string())
{
    sendError(errorCode, socket, body, extraHeader + "Connection: close\r\n");
    socket->asyncShutdown();
    socket->ignoreInput();
}

/// Sends file as HTTP response and shutdown the socket.
void sendFileAndShutdown(const std::shared_ptr<StreamSocket>& socket, const std::string& path,
                         http::Response& response, bool noCache = false, bool deflate = false,
                         bool headerOnly = false);

/// Sends file as HTTP response.
void sendFile(const std::shared_ptr<StreamSocket>& socket, const std::string& path,
              http::Response& response, bool noCache = false, bool deflate = false,
              bool headerOnly = false);

/// Verifies that the host in the given WOPISrc holds only bytes a host name can hold, and that
/// the WOPISrc is properly URI-encoded. A malformed host gets an HTTP error in every build. An
/// unencoded WOPISrc only warns in release builds, but closes the socket in debug builds, to
/// help developers.
inline bool verifyWOPISrc(const std::string& uri, const std::string& wopiSrc,
                          const std::shared_ptr<StreamSocket>& socket = {},
                          LOG_CAPTURE_CALLER_DECLARATION)
{
    try
    {
        const Poco::URI uriWopiSrc(wopiSrc);
        const std::string& host = uriWopiSrc.getHost();
        if (!net::isValidHost(host))
        {
            LOG_ERR_S("WOPISrc validation error: malformed host [" << host << "] in WOPISrc ["
                                                                   << wopiSrc << "] in URL [" << uri
                                                                   << ']');
            if (socket)
            {
                sendErrorAndShutdown(http::StatusCode::BadRequest, socket,
                                     "Malformed host in WOPISrc");
            }

            return false;
        }
    }
    catch (const Poco::SyntaxException&)
    {
        // A WOPISrc Poco cannot parse is left to the caller that parses it again.
    }

    // getQueryParameters(), which is used to extract wopiSrc, decodes the values.
    // Compare with the URI. WopiSrc is complex enough to require encoding.
    // But, if it matches, check if the WOPISrc actually needed encoding.
    if (uri.find(wopiSrc) != std::string::npos && Uri::needsEncoding(wopiSrc))
    {
        if constexpr (!Util::isDebugEnabled())
        {
            LOG_WRN_ONCE_S(
                "WOPISrc validation error: unencoded WOPISrc ["
                << wopiSrc << "] in URL [" << uri
                << "]. WOPISrc must be URI-encoded. This is highly problematic with proxies, "
                   "load balancers, and when tunneling. Will not warn again");
        }
        else
        {
            // In debug mode, be assertive. Logs might go unnoticed.
            LOG_ERR_S("WOPISrc validation error: unencoded WOPISrc ["
                      << wopiSrc << "] in URL [" << uri
                      << "]. This is highly problematic with proxies, load balancers, and when "
                         "tunneling");
            if (socket)
            {
                sendErrorAndShutdown(http::StatusCode::BadRequest, socket,
                                     "WOPISrc must be URI-encoded");
            }

            return false;
        }
    }

    return true;
}

} // namespace HttpHelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
