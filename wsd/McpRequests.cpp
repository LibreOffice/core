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

#include <config.h>

#include <wsd/McpRequests.hpp>

#include <common/ConfigUtil.hpp>
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/Util.hpp>
#include <net/HttpHelper.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/McpServer.hpp>
#include <wsd/RequestDetails.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <cassert>
#include <cstddef>
#include <istream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace
{
/// Compare two strings without letting the time taken reveal how much of the key matched or how
/// long the key is. The comparison always runs over the length of the expected key and folds any
/// length difference into the result.
bool constantTimeEquals(const std::string& expected, const std::string& given)
{
    // The difference is accumulated at the width of a size, so a difference in length is carried
    // whole.
    std::size_t difference = expected.size() ^ given.size();
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        const unsigned char givenChar =
            i < given.size() ? static_cast<unsigned char>(given[i]) : 0;
        difference |= static_cast<unsigned char>(expected[i]) ^ givenChar;
    }
    return difference == 0;
}

/// The number of bytes in message from its read position to its end, or -1 when the stream cannot
/// tell.
std::streamsize remainingBytes(std::istream& message)
{
    const std::streampos position = message.tellg();
    if (position == std::streampos(-1))
        return -1;
    message.seekg(0, std::ios::end);
    const std::streampos end = message.tellg();
    message.seekg(position);
    return end == std::streampos(-1) ? -1 : static_cast<std::streamsize>(end - position);
}
}

bool McpRequests::isMcpRequest(const RequestDetails& requestDetails)
{
    return !requestDetails.isWebSocket() &&
           (requestDetails.equals(RequestDetails::Field::Type, "cool") ||
            requestDetails.equals(RequestDetails::Field::Type, "lool")) &&
           requestDetails.equals(1, "mcp");
}

bool McpRequests::refuseMcpRequest(const Poco::Net::HTTPRequest& request,
                                   const std::shared_ptr<StreamSocket>& socket)
{
    const bool enabled = ConfigUtil::getBool("ai.mcp.enable", false);
    const std::string apiKey = ConfigUtil::getString("ai.mcp.api_key", std::string());
    if (!enabled || apiKey.empty())
    {
        HttpHelper::sendErrorAndShutdown(http::StatusCode::NotFound, socket);
        return true;
    }

    // Guard against DNS-rebinding. A protocol client sends no Origin header; a browser page always
    // does, so the presence of one means the request came from a web page and is refused.
    if (request.has("Origin"))
    {
        LOG_WRN_S("MCP: rejecting request that carries an Origin header");
        HttpHelper::sendErrorAndShutdown(http::StatusCode::Forbidden, socket);
        return true;
    }

    // The caller must present the configured key as a bearer token. The header holds the scheme, a
    // space and the credential. RFC 7235 makes the scheme case-insensitive, so it is matched letter
    // by letter without regard to case, and the credential alone is compared in constant time.
    const std::string authorization = request.get("Authorization", std::string());
    const std::size_t schemeEnd = authorization.find(' ');
    const std::string scheme = authorization.substr(0, schemeEnd);
    const std::string credential =
        schemeEnd == std::string::npos ? std::string() : authorization.substr(schemeEnd + 1);
    if (!Util::iequal(scheme, "Bearer") || !constantTimeEquals(apiKey, credential))
    {
        HttpHelper::sendErrorAndShutdown(http::StatusCode::Unauthorized, socket,
                                         std::string_view(), "WWW-Authenticate: Bearer\r\n");
        return true;
    }

    // A body states its length. A chunked body, or a POST with no Content-Length, states none and
    // is refused.
    if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST &&
        (request.getChunkedTransferEncoding() || request.getContentLength() < 0))
    {
        LOG_WRN_S("MCP: refusing a request that does not state its length");
        HttpHelper::sendErrorAndShutdown(http::StatusCode::LengthRequired, socket,
                                         "Send the request with a Content-Length header.\n");
        return true;
    }

    return false;
}

bool McpRequests::refuseMcpRequestEarly(const Poco::Net::HTTPRequest& request,
                                        const std::shared_ptr<StreamSocket>& socket)
{
    // Only an address that can name the endpoint, plainly or in the hex form, is parsed here.
    const std::string& uri = request.getURI();
    if (uri.find("/mcp") == std::string::npos && uri.find("cool/0x") == std::string::npos)
        return false;

    // Reading the details takes the service root off the address of the request they are read from,
    // so they are read from a copy, and the request itself reaches the dispatcher as it came.
    Poco::Net::HTTPRequest stripped(request);
    try
    {
        if (!isMcpRequest(RequestDetails(stripped, COOLWSD::ServiceRoot)))
            return false;
    }
    catch (const std::exception& exc)
    {
        // An address that cannot be read is answered by the dispatcher with the other requests.
        LOG_DBG_S("MCP: leaving an unreadable address to the dispatcher: " << exc.what());
        return false;
    }

    return refuseMcpRequest(stripped, socket);
}

bool McpRequests::handleMcpRequest(const Poco::Net::HTTPRequest& request,
                                   std::istream& message,
                                   const std::shared_ptr<StreamSocket>& socket)
{
    assert(socket && "Must have a valid socket");

    if (refuseMcpRequest(request, socket))
        return true;

    // The endpoint only takes POST.
    if (request.getMethod() != Poco::Net::HTTPRequest::HTTP_POST)
    {
        HttpHelper::sendErrorAndShutdown(http::StatusCode::MethodNotAllowed, socket,
                                         std::string_view(), "Allow: POST\r\n");
        return true;
    }

    // The body is the number of bytes the request states, and anything after it in the buffer
    // belongs to another request.
    const std::streamsize bodyLength = request.getContentLength();
    if (remainingBytes(message) < bodyLength)
    {
        LOG_WRN_S("MCP: refusing a request whose body is shorter than its stated length");
        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);
        return true;
    }

    std::string body(NumUtil::makeUnsigned(bodyLength), '\0');
    message.read(body.data(), bodyLength);
    const std::string protocolVersionHeader = request.get("MCP-Protocol-Version", std::string());

    // The response is always sent with sendAndShutdown, so the connection is closed after this one
    // message either way.
    std::weak_ptr<StreamSocket> socketWeak = socket;
    McpServer::handleMessage(
        body, protocolVersionHeader,
        [socketWeak](McpServer::McpResponse mcpResponse)
        {
            std::shared_ptr<StreamSocket> streamSocket = socketWeak.lock();
            if (!streamSocket)
            {
                LOG_WRN_S("MCP: socket gone before the response could be sent");
                return;
            }

            http::Response httpResponse(static_cast<http::StatusCode>(mcpResponse.statusCode));
            if (mcpResponse.body.empty())
                httpResponse.setContentLength(0);
            else
                httpResponse.setBody(std::move(mcpResponse.body), "application/json");
            httpResponse.setConnectionToken(http::Header::ConnectionToken::Close);
            streamSocket->sendAndShutdown(httpResponse);
            streamSocket->ignoreInput();
        });

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
