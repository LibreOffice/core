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
 * Transport-agnostic core of the Model Context Protocol server.
 *
 * The core turns one incoming JSON-RPC message into one response without ever touching a socket.
 * The caller passes the raw request body and the value of the MCP-Protocol-Version header, and
 * receives the result through a callback. That keeps the protocol logic free of any particular
 * transport, so the same code can back an HTTP endpoint or a future stdio transport.
 *
 * In this first version every path calls the callback right away. The callback shape already allows
 * a later version to answer after some asynchronous work (for example once a document broker has
 * finished).
 */

#pragma once

#include <config.h>

#include <functional>
#include <string>

class McpServer
{
public:
    /// One MCP reply ready for a transport to send.
    struct McpResponse
    {
        /// The HTTP status a transport should use: 200 for a JSON-RPC response (including a
        /// JSON-RPC error), 202 for a notification that has no body, 400 when the requested
        /// protocol version is not supported.
        int statusCode = 200;
        /// The response body. Empty for a notification.
        std::string body;
    };

    /// The MCP protocol versions this server understands.
    static bool isSupportedProtocolVersion(const std::string& version);

    /// Turn one JSON-RPC message into one response.
    ///
    /// body is the raw request body. protocolVersionHeader is the value of the MCP-Protocol-Version
    /// HTTP header, or an empty string when the header is absent. The result is delivered through
    /// sendResponse, which this version always calls before returning.
    static void handleMessage(const std::string& body,
                              const std::string& protocolVersionHeader,
                              const std::function<void(McpResponse)>& sendResponse);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
