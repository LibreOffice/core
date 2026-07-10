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

#include <wsd/DocumentToolDescriptions.hpp>

#include <common/Log.hpp>
#include <common/Util.hpp>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include <algorithm>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace
{
// The one JSON-RPC version this server speaks.
const std::string JsonRpcVersion = "2.0";

// The version answered when a client asks for one we do not know, or does not ask at all. This is
// the newest version we support.
const std::string LatestProtocolVersion = "2025-11-25";

// How a client gives the server a document to work on. This paragraph is added to every tool
// description so a client knows how to pass a document.
const std::string InputAlternativesDescription =
    " Set \"data\" to the document bytes encoded as base64. Set \"filename\" to the original "
    "file name so the server can tell which kind of document it is.";

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

// Wrap a tool outcome as a tools/call result. The text is carried in a single text content block,
// and isError says whether that text is a failure the model should read or the tool's successful
// output.
std::string makeToolResultBody(const Poco::Dynamic::Var& id,
                               const McpServer::McpToolResult& outcome)
{
    Poco::JSON::Object::Ptr textBlock = new Poco::JSON::Object();
    textBlock->set("type", "text");
    textBlock->set("text", outcome.text);

    Poco::JSON::Array::Ptr content = new Poco::JSON::Array();
    content->add(textBlock);

    Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
    result->set("content", content);
    result->set("isError", outcome.isError);

    return makeResultBody(id, result);
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

// Add a string property to a schema properties object.
void addStringProperty(Poco::JSON::Object::Ptr properties, const std::string& name,
                       const std::string& description)
{
    Poco::JSON::Object::Ptr property = new Poco::JSON::Object();
    property->set("type", "string");
    property->set("description", description);
    properties->set(name, property);
}

// Add the properties that say which document to work on. Every tool shares these, so they are
// described in one place.
void addInputProperties(Poco::JSON::Object::Ptr properties)
{
    addStringProperty(properties, "data", "The document bytes encoded as base64.");
    addStringProperty(properties, "filename",
                      "The original file name, used to tell which kind of document this is.");
}

Poco::JSON::Object::Ptr makeObjectSchema(const Poco::JSON::Object::Ptr& properties)
{
    Poco::JSON::Object::Ptr schema = new Poco::JSON::Object();
    schema->set("type", "object");
    schema->set("properties", properties);
    return schema;
}

Poco::JSON::Object::Ptr makeExtractDocumentStructureSchema()
{
    Poco::JSON::Object::Ptr properties = new Poco::JSON::Object();
    addInputProperties(properties);
    addStringProperty(
        properties, "filter",
        "Which view of the document to return. For a text document: \"text\" (the body as "
        "markdown), \"contentcontrol\", \"charts\", \"docprops\" (the document properties) "
        "or \"trackchanges\". For a spreadsheet: \"text\" only. For a presentation: \"text\" "
        "(the text and speaker notes of every slide) or \"slides\" (the layout, master, objects "
        "and speaker notes of every slide). Left out, a text document gives every view except "
        "\"text\", and a presentation gives \"slides\".");
    addStringProperty(properties, "range",
                      "For a spreadsheet, the cell range the text view is limited to, such as "
                      "\"A1:D100\".");
    addStringProperty(properties, "sheet",
                      "For a spreadsheet, the name of the sheet the text view shows. Left out, it "
                      "shows the active sheet. The text view lists the name of every sheet under "
                      "Sheets.");
    addStringProperty(properties, "target",
                      "For a text document, the link target the text view is limited to, copied "
                      "exactly as extract_link_targets gives it, such as "
                      "\"1.Introduction|outline\".");
    return makeObjectSchema(properties);
}

Poco::JSON::Object::Ptr makeExtractLinkTargetsSchema()
{
    Poco::JSON::Object::Ptr properties = new Poco::JSON::Object();
    addInputProperties(properties);
    return makeObjectSchema(properties);
}

std::string makeExtractDocumentStructureDescription()
{
    return std::string("Read a text document, a spreadsheet or a presentation and return what it "
                       "holds as JSON.") +
           DocumentToolDescriptions::EXTRACT_WRITER + DocumentToolDescriptions::EXTRACT_CALC +
           " With sheet set to a sheet's name, it returns that sheet in place of the active one, "
           "and the result lists every sheet's name under Sheets." +
           DocumentToolDescriptions::EXTRACT_IMPRESS + InputAlternativesDescription;
}

std::string makeExtractLinkTargetsDescription()
{
    return std::string(DocumentToolDescriptions::EXTRACT_LINK_TARGETS_DESCRIPTION) +
           InputAlternativesDescription;
}

// Serialise the tools as the array a tools/list response carries.
Poco::JSON::Array::Ptr makeToolsArray(const std::vector<McpServer::McpTool>& tools)
{
    Poco::JSON::Array::Ptr array = new Poco::JSON::Array();
    for (const auto& tool : tools)
    {
        Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
        entry->set("name", tool.name);
        if (!tool.title.empty())
            entry->set("title", tool.title);
        entry->set("description", tool.description);
        entry->set("inputSchema", tool.inputSchema);

        // The hints let a client tell a tool that only reads from one that changes something. Every
        // tool works on the document it is given and reaches nothing outside the server.
        Poco::JSON::Object::Ptr annotations = new Poco::JSON::Object();
        annotations->set("readOnlyHint", tool.readOnly);
        annotations->set("openWorldHint", false);
        entry->set("annotations", annotations);

        array->add(entry);
    }
    return array;
}

const McpServer::McpTool* findTool(const std::vector<McpServer::McpTool>& tools,
                                   const std::string& name)
{
    for (const auto& tool : tools)
    {
        if (tool.name == name)
            return &tool;
    }
    return nullptr;
}
}

std::vector<McpServer::McpTool> McpServer::documentToolDefinitions()
{
    // The descriptions and schemas are the same for every request, and they run to several
    // kilobytes, so they are built on the first call and copied afterwards. The copy shares each
    // schema, which is only read from here on.
    static const std::vector<McpTool> definitions = []
    {
        std::vector<McpTool> tools;

        McpTool structure;
        structure.name = ToolName::ExtractDocumentStructure;
        structure.title = "Extract document structure";
        structure.description = makeExtractDocumentStructureDescription();
        structure.inputSchema = makeExtractDocumentStructureSchema();
        tools.push_back(std::move(structure));

        McpTool links;
        links.name = ToolName::ExtractLinkTargets;
        links.title = "List link targets";
        links.description = makeExtractLinkTargetsDescription();
        links.inputSchema = makeExtractLinkTargetsSchema();
        tools.push_back(std::move(links));

        return tools;
    }();

    return definitions;
}

std::optional<McpServer::McpToolResult>
McpServer::validateExtractArguments(const Poco::JSON::Object::Ptr& arguments)
{
    static const std::set<std::string> filters = { "text",     "contentcontrol", "charts",
                                                   "docprops", "trackchanges",   "slides" };
    const std::string filter = arguments->optValue<std::string>("filter", std::string());
    if (!filter.empty() && filters.count(filter) == 0)
        return McpToolResult{ /*isError=*/true,
                              "Set the filter argument to one of \"text\", \"contentcontrol\", "
                              "\"charts\", \"docprops\", \"trackchanges\" or \"slides\", or "
                              "leave it out." };

    // A range is a cell or a block of cells, such as A1, $A$1 or A1:D100.
    const std::string range = arguments->optValue<std::string>("range", std::string());
    if (range.size() > 64 ||
        !std::all_of(range.begin(), range.end(),
                     [](char character)
                     {
                         return (character >= 'a' && character <= 'z') ||
                                (character >= 'A' && character <= 'Z') ||
                                (character >= '0' && character <= '9') || character == ':' ||
                                character == '$';
                     }))
        return McpToolResult{ /*isError=*/true,
                              "Set the range argument to a cell range such as \"A1:D100\", made "
                              "of letters, digits, colons and dollar signs." };

    return std::nullopt;
}

bool McpServer::isSupportedProtocolVersion(const std::string& version)
{
    // Both versions take one JSON-RPC message per request. The versions before them also take a
    // batch of messages in one request, which this server does not handle.
    return version == "2025-06-18" || version == "2025-11-25";
}

bool McpServer::handleMessage(const std::string& body, const std::string& protocolVersionHeader,
                              const std::vector<McpTool>& tools,
                              const std::function<void(McpResponse)>& sendResponse)
{
    // A client that names a protocol version we do not support in the header is turned away before
    // any JSON-RPC processing.
    if (!protocolVersionHeader.empty() && !isSupportedProtocolVersion(protocolVersionHeader))
    {
        LOG_DBG("MCP: unsupported MCP-Protocol-Version header [" << protocolVersionHeader << ']');
        sendResponse({ 400, "{\"error\":\"Unsupported MCP-Protocol-Version\"}" });
        return true;
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
        return true;
    }

    // An array is a batch. The supported versions have no batches, so it is answered as an invalid
    // request.
    if (parsed.type() == typeid(Poco::JSON::Array::Ptr))
    {
        sendResponse({ 200, makeErrorBody(Poco::Dynamic::Var(), -32600,
                                          "Invalid Request: batch requests are not supported") });
        return true;
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
        return true;
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
        return true;
    }

    // A message without a method is the client's answer to a request of ours, and a message without
    // an id is a notification. Neither gets a JSON-RPC response, only an acknowledgement that it
    // was accepted.
    if (!request->has("method"))
    {
        if (request->has("result") || request->has("error"))
        {
            sendResponse({ 202, std::string() });
            return true;
        }

        sendResponse({ 200, makeErrorBody(replyId, -32600, "Invalid Request: missing method") });
        return true;
    }

    if (!request->has("id"))
    {
        sendResponse({ 202, std::string() });
        return true;
    }

    if (!validId)
    {
        sendResponse({ 200, makeErrorBody(replyId, -32600,
                                          "Invalid Request: id must be a string or an integer") });
        return true;
    }

    if (!request->get("method").isString())
    {
        sendResponse({ 200, makeErrorBody(replyId, -32600,
                                          "Invalid Request: method must be a string") });
        return true;
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
        return true;
    }

    if (method == "ping")
    {
        sendResponse({ 200, makeResultBody(id, new Poco::JSON::Object()) });
        return true;
    }

    if (method == "tools/list")
    {
        Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
        result->set("tools", makeToolsArray(tools));
        sendResponse({ 200, makeResultBody(id, result) });
        return true;
    }

    if (method == "tools/call")
    {
        Poco::JSON::Object::Ptr params = request->getObject("params");
        const std::string name = params ? params->optValue<std::string>("name", std::string())
                                        : std::string();
        if (name.empty())
        {
            sendResponse({ 200, makeErrorBody(id, -32602, "Invalid params: missing tool name") });
            return true;
        }

        // The arguments object is optional, but when it is present it must be an object. Anything
        // else is a malformed call.
        Poco::JSON::Object::Ptr arguments;
        if (params->has("arguments"))
        {
            arguments = params->getObject("arguments");
            if (!arguments)
            {
                sendResponse({ 200, makeErrorBody(id, -32602,
                                                  "Invalid params: arguments must be an object") });
                return true;
            }
        }
        else
        {
            arguments = new Poco::JSON::Object();
        }

        const McpTool* tool = findTool(tools, name);
        if (!tool || !tool->executor)
        {
            sendResponse({ 200, makeErrorBody(id, -32602, "Invalid params: unknown tool") });
            return true;
        }

        // The completion owns copies of the id and the response callback so it can build and
        // deliver the result after this call has returned.
        Poco::Dynamic::Var idCopy = id;
        std::function<void(McpResponse)> send = sendResponse;
        McpToolCompletion complete = [idCopy, send](McpToolResult outcome)
        { send({ 200, makeToolResultBody(idCopy, outcome) }); };

        return tool->executor(arguments, complete);
    }

    sendResponse({ 200, makeErrorBody(id, -32601, "Method not found") });
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
