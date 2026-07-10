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

#include <wsd/McpServer.hpp>

#include <common/Log.hpp>
#include <common/Util.hpp>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include <sstream>
#include <string>

namespace
{
// The one JSON-RPC version this server speaks.
const std::string JsonRpcVersion = "2.0";

// The version answered when a client asks for one we do not know, or does not ask at all. This is
// the newest version we support.
const std::string LatestProtocolVersion = "2025-11-25";

std::string stringify(const Poco::JSON::Object::Ptr& object)
{
    std::ostringstream stream;
    object->stringify(stream);
    return stream.str();
}

// Build a JSON-RPC envelope carrying the request id back unchanged. An empty id Var is written as
// JSON null, which is what a parse error or an invalid request must answer with. A numeric id comes
// back as a number and a string id as a string, because the original value keeps its type.
Poco::JSON::Object::Ptr makeEnvelope(const Poco::Dynamic::Var& id)
{
    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("jsonrpc", JsonRpcVersion);
    root->set("id", id);
    return root;
}

std::string makeErrorBody(const Poco::Dynamic::Var& id, int code, const std::string& message)
{
    Poco::JSON::Object::Ptr root = makeEnvelope(id);
    Poco::JSON::Object::Ptr error = new Poco::JSON::Object();
    error->set("code", code);
    error->set("message", message);
    root->set("error", error);
    return stringify(root);
}

std::string makeResultBody(const Poco::Dynamic::Var& id, const Poco::JSON::Object::Ptr& result)
{
    Poco::JSON::Object::Ptr root = makeEnvelope(id);
    root->set("result", result);
    return stringify(root);
}

// The initialize result: the negotiated protocol version, the set of capabilities this server
// offers, and identifying information about the server itself.
std::string makeInitializeResult(const Poco::Dynamic::Var& id, const std::string& protocolVersion)
{
    Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
    result->set("protocolVersion", protocolVersion);

    Poco::JSON::Object::Ptr capabilities = new Poco::JSON::Object();
    Poco::JSON::Object::Ptr toolsCapability = new Poco::JSON::Object();
    capabilities->set("tools", toolsCapability);
    result->set("capabilities", capabilities);

    Poco::JSON::Object::Ptr serverInfo = new Poco::JSON::Object();
    serverInfo->set("name", "Collabora Online");
    serverInfo->set("version", Util::getCoolVersion());
    result->set("serverInfo", serverInfo);

    return makeResultBody(id, result);
}
}

bool McpServer::isSupportedProtocolVersion(const std::string& version)
{
    // Both versions take one JSON-RPC message per request. The versions before them also take a
    // batch of messages in one request, which this server does not handle.
    return version == "2025-06-18" || version == "2025-11-25";
}

void McpServer::handleMessage(const std::string& body,
                              const std::string& protocolVersionHeader,
                              const std::function<void(McpResponse)>& sendResponse)
{
    // A client that names a protocol version we do not support in the header is turned away before
    // any JSON-RPC processing.
    if (!protocolVersionHeader.empty() && !isSupportedProtocolVersion(protocolVersionHeader))
    {
        LOG_DBG("MCP: unsupported MCP-Protocol-Version header [" << protocolVersionHeader << ']');
        sendResponse({ 400, "{\"error\":\"Unsupported MCP-Protocol-Version\"}" });
        return;
    }

    Poco::Dynamic::Var parsed;
    try
    {
        Poco::JSON::Parser parser;
        parsed = parser.parse(body);
    }
    catch (const std::exception& exc)
    {
        LOG_DBG("MCP: failed to parse request body: " << exc.what());
        sendResponse({ 200, makeErrorBody(Poco::Dynamic::Var(), -32700, "Parse error") });
        return;
    }

    // An array is a batch. The supported versions have no batches, so it is answered as an invalid
    // request.
    if (parsed.type() == typeid(Poco::JSON::Array::Ptr))
    {
        sendResponse({ 200, makeErrorBody(Poco::Dynamic::Var(), -32600,
                                          "Invalid Request: batch requests are not supported") });
        return;
    }

    Poco::JSON::Object::Ptr request;
    try
    {
        request = parsed.extract<Poco::JSON::Object::Ptr>();
    }
    catch (const std::exception&)
    {
        request = nullptr;
    }
    if (!request)
    {
        sendResponse({ 200, makeErrorBody(Poco::Dynamic::Var(), -32600, "Invalid Request") });
        return;
    }

    // Keep the id exactly as it arrived so the response echoes back the same JSON type. An id is a
    // string or an integer. When it is missing or of any other type, an error is answered with a
    // null id, the form JSON-RPC gives an error whose request id cannot be read.
    Poco::Dynamic::Var id;
    if (request->has("id"))
        id = request->get("id");
    const bool validId = id.isString() || id.isInteger();
    const Poco::Dynamic::Var replyId = validId ? id : Poco::Dynamic::Var();

    if (request->optValue<std::string>("jsonrpc", std::string()) != JsonRpcVersion)
    {
        sendResponse({ 200, makeErrorBody(replyId, -32600,
                                          "Invalid Request: jsonrpc must be \"2.0\"") });
        return;
    }

    // A message without a method is the client's answer to a request of ours, and a message without
    // an id is a notification. Neither gets a JSON-RPC response, only an acknowledgement that it
    // was accepted.
    if (!request->has("method"))
    {
        if (request->has("result") || request->has("error"))
        {
            sendResponse({ 202, std::string() });
            return;
        }

        sendResponse({ 200, makeErrorBody(replyId, -32600, "Invalid Request: missing method") });
        return;
    }

    if (!request->has("id"))
    {
        sendResponse({ 202, std::string() });
        return;
    }

    if (!validId)
    {
        sendResponse({ 200, makeErrorBody(replyId, -32600,
                                          "Invalid Request: id must be a string or an integer") });
        return;
    }

    if (!request->get("method").isString())
    {
        sendResponse({ 200, makeErrorBody(replyId, -32600,
                                          "Invalid Request: method must be a string") });
        return;
    }

    const std::string method = request->get("method").extract<std::string>();

    if (method == "initialize")
    {
        // Answer with the version the client asked for when we support it, otherwise with the
        // newest version we support.
        std::string negotiated = LatestProtocolVersion;
        Poco::JSON::Object::Ptr params = request->getObject("params");
        if (params)
        {
            const std::string requested =
                params->optValue<std::string>("protocolVersion", std::string());
            if (isSupportedProtocolVersion(requested))
                negotiated = requested;
        }

        sendResponse({ 200, makeInitializeResult(id, negotiated) });
        return;
    }

    if (method == "ping")
    {
        sendResponse({ 200, makeResultBody(id, new Poco::JSON::Object()) });
        return;
    }

    if (method == "tools/list")
    {
        Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
        Poco::JSON::Array::Ptr tools = new Poco::JSON::Array();
        result->set("tools", tools);
        sendResponse({ 200, makeResultBody(id, result) });
        return;
    }

    if (method == "tools/call")
    {
        // No tools are registered yet, so every call names a tool that does not exist. A later
        // version replaces this with a real tool registry.
        sendResponse({ 200, makeErrorBody(id, -32602, "Invalid params: unknown tool") });
        return;
    }

    sendResponse({ 200, makeErrorBody(id, -32601, "Method not found") });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
