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
 * Implementation of AIChatSession - multi-round LLM tool-calling loop
 * extracted from ClientSession.
 */

#include <config.h>

#include "AIChatSession.hpp"

#if !MOBILEAPP

#include "ClientSession.hpp"

#include <common/Common.hpp>
#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Message.hpp>
#include <common/Util.hpp>
#include <common/base64.hpp>
#include <net/HttpServer.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/DocumentToolDescriptions.hpp>

#include <Poco/File.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>

#include <chrono>
#include <cstring>
#include <exception>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <utility>

namespace
{
constexpr std::size_t MAX_AI_IMAGE_GENERATIONS = 20;

bool isValidImageSize(const std::string& size)
{
    const auto pos = size.find('x');
    if (pos == std::string::npos || pos == 0 || pos == size.size() - 1)
        return false;
    const std::string widthStr = size.substr(0, pos);
    const std::string heightStr = size.substr(pos + 1);
    try
    {
        const int width = std::stoi(widthStr);
        const int height = std::stoi(heightStr);
        return width > 0 && height > 0
            && widthStr == std::to_string(width)
            && heightStr == std::to_string(height);
    }
    catch (const std::exception&)
    {
        return false;
    }
}

const std::string AI_SYSTEM_PROMPT =
    "You are a helpful assistant for Collabora Online. "
    "Help users with their documents - answering questions, suggesting edits, "
    "rewriting text, and more. When the user shares selected text from their document, "
    "provide relevant help with that text. When no selected text is provided, answer "
    "general questions about documents, formatting, writing, and the application. "
    "When providing rewritten or edited text, return it in markdown format preserving "
    "the original formatting structure. IMPORTANT: Return the markdown text directly "
    "without wrapping it in code fences (do NOT use ```markdown or ``` blocks). "
    "Just return the raw markdown content. Be concise and helpful.";

/// Helper to create an OpenAI function-calling tool object.
Poco::JSON::Object::Ptr makeAITool(const std::string& name,
                                    const std::string& description,
                                    Poco::JSON::Object::Ptr parameters)
{
    Poco::JSON::Object::Ptr fn = new Poco::JSON::Object();
    fn->set("name", name);
    fn->set("description", description);
    fn->set("parameters", parameters);

    Poco::JSON::Object::Ptr tool = new Poco::JSON::Object();
    tool->set("type", "function");
    tool->set("function", fn);
    return tool;
}

/// Helper to create an OpenAI function parameter schema object.
Poco::JSON::Object::Ptr makeParamSchema(
    std::initializer_list<std::pair<std::string, std::pair<std::string, std::string>>> props,
    std::initializer_list<std::string> required)
{
    Poco::JSON::Object::Ptr properties = new Poco::JSON::Object();
    for (const auto& p : props)
    {
        Poco::JSON::Object::Ptr prop = new Poco::JSON::Object();
        prop->set("type", p.second.first);
        prop->set("description", p.second.second);
        properties->set(p.first, prop);
    }

    Poco::JSON::Array::Ptr reqArr = new Poco::JSON::Array();
    for (const auto& r : required)
        reqArr->add(r);

    Poco::JSON::Object::Ptr schema = new Poco::JSON::Object();
    schema->set("type", "object");
    schema->set("properties", properties);
    schema->set("required", reqArr);
    return schema;
}

} // anonymous namespace

AIChatSession::AIChatSession(ClientSession& session)
    : _session(session)
{
}

AIChatSession::~AIChatSession() = default;

void AIChatSession::sendChatResult(bool success, const std::string& text,
                                   const std::string& requestId)
{
    Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
    result->set("success", success);
    if (success)
        result->set("content", text);
    else
        result->set("error", text);
    result->set("requestId", requestId);

    std::ostringstream oss;
    result->stringify(oss);
    _session.sendTextFrame("aichatresult: " + oss.str());
}

std::string AIChatSession::mapHttpStatusToError(
    http::StatusCode statusCode, const std::string& reasonPhrase,
    const std::string& context)
{
    switch (statusCode)
    {
        case http::StatusCode::BadRequest:
            return context.empty() ? "Invalid request"
                                   : "Invalid " + context + " request";
        case http::StatusCode::Unauthorized:     return "Invalid API key";
        case http::StatusCode::Forbidden:        return "API key lacks permissions";
        case http::StatusCode::TooManyRequests:  return "Rate limited - please wait a moment and retry";
        case http::StatusCode::InternalServerError: return "API server error - try again later";
        case http::StatusCode::ServiceUnavailable:  return "Service temporarily unavailable";
        default:
        {
            std::string err = "API error (";
            err.append(std::to_string(static_cast<int>(statusCode)));
            err.append("): ");
            err.append(reasonPhrase);
            return err;
        }
    }
}

Poco::JSON::Array::Ptr AIChatSession::buildToolDefinitions() const
{
    Poco::JSON::Array::Ptr tools = new Poco::JSON::Array();

    // generate_image - existing tool
    tools->add(makeAITool(
        "generate_image",
        "Generate an image based on the user's description. Call this when the "
        "user asks to create, draw, generate, sketch, or make an image or picture.",
        makeParamSchema(
            {{"prompt", {"string", "A detailed description of the image to generate"}}},
            {"prompt"})));

    // extract_document_structure - inspect the open document
    tools->add(makeAITool(
        "extract_document_structure",
        DocumentToolDescriptions::EXTRACT_DOC_STRUCTURE_DESCRIPTION,
        makeParamSchema(
            {{"filter", {"string",
                "Filter results to a specific structure type. "
                "For Impress: 'slides'. For Writer: 'contentcontrol'. "
                "Omit to get the full structure."}}},
            {})));

    // transform_document_structure - modify the open document
    tools->add(makeAITool(
        "transform_document_structure",
        std::string(
            "Transform the currently-open document's structure using a JSON command "
            "sequence. Supports Impress slide operations, Writer/Calc content control "
            "updates, and arbitrary UNO commands.\n\n")
            + DocumentToolDescriptions::TRANSFORM_PARAM_DESCRIPTION,
        makeParamSchema(
            {{"transform", {"string", "JSON transformation commands"}},
             {"summary", {"string",
                "Markdown summary of the changes for the user to review before "
                "approving. List each slide with its title and "
                "key content points."}}},
            {"transform"})));

    // extract_link_targets - get link targets from the open document
    tools->add(makeAITool(
        "extract_link_targets",
        DocumentToolDescriptions::EXTRACT_LINK_TARGETS_DESCRIPTION,
        makeParamSchema({}, {})));

    // list_calc_functions - discover available spreadsheet functions
    tools->add(makeAITool(
        "list_calc_functions",
        "List all available spreadsheet functions in the current Calc document, "
        "grouped by category. Returns function names and signatures. "
        "Call this when you need to verify a function exists or discover "
        "the right function for a task. Only works for Calc/spreadsheet documents.",
        makeParamSchema({}, {})));

    // evaluate_formula - test a formula without inserting it
    tools->add(makeAITool(
        "evaluate_formula",
        "Evaluate a formula without inserting it into the spreadsheet. "
        "Returns the computed result so you can verify correctness before inserting. "
        "Always call this before set_cell_formula to check your formula produces "
        "the expected result. Uses US English syntax (comma separators).",
        makeParamSchema(
            {{"cell", {"string", "Cell address for evaluation context, e.g. 'G1'"}},
             {"formula", {"string", "The formula to evaluate, starting with ="}}},
            {"cell", "formula"})));

    // set_cell_formula - insert formulas into cells (Calc only)
    tools->add(makeAITool(
        "set_cell_formula",
        "Set formulas or values in one or more cells of the currently open spreadsheet. "
        "Use US English formula syntax (commas as argument separators, period as decimal "
        "separator). Always prefix formulas with =. Example: =AVERAGE(A1:A10). "
        "Can also set plain text or numbers. Only works for Calc/spreadsheet documents.\n\n"
        "For a single cell, provide 'cell' and 'formula' parameters.\n"
        "For multiple cells, provide 'formulas' as a JSON array of objects, each with "
        "'cell' and 'formula' keys. Example: [{\"cell\":\"E1\",\"formula\":\"Total\"}, "
        "{\"cell\":\"E2\",\"formula\":\"=SUM(A2:D2)\"}]\n"
        "Always prefer the batch 'formulas' array when setting more than one cell.",
        makeParamSchema(
            {{"cell", {"string", "Target cell address for a single cell, e.g. 'A1', 'B5'"}},
             {"formula", {"string", "The formula or value for a single cell"}},
             {"formulas", {"string", "JSON array of {cell, formula} objects for batch operations"}},
             {"summary", {"string",
                 "Brief human-readable description of the changes, shown to the user for approval"}}},
            {})));

    return tools;
}

bool AIChatSession::handleAction(const std::string& firstLine)
{
    static constexpr size_t MAX_AI_PAYLOAD_SIZE = 5 * 1024 * 1024; // 5MB
    static constexpr size_t MAX_AI_MESSAGE_LENGTH = 100 * 1024; // 100KB per message
    static constexpr unsigned MAX_AI_MESSAGES = 50;

    // Extract JSON payload after "aichat: "
    const std::string jsonPayload = firstLine.substr(strlen("aichat: "));

    if (jsonPayload.size() > MAX_AI_PAYLOAD_SIZE)
    {
        sendChatResult(false, "Request too large", "");
        return true;
    }

    Poco::JSON::Object::Ptr requestObj = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(jsonPayload, requestObj))
    {
        sendChatResult(false, "Invalid request format", "");
        return true;
    }

    std::string requestId;
    JsonUtil::findJSONValue(requestObj, "requestId", requestId);

    std::string docType;
    JsonUtil::findJSONValue(requestObj, "docType", docType);

    if (docType != "text" && docType != "spreadsheet" && docType != "presentation" && docType != "drawing")
        docType.clear();

    Poco::JSON::Array::Ptr messages = requestObj->getArray("messages");
    if (!messages || messages->size() == 0)
    {
        sendChatResult(false, "No messages provided", requestId);
        return true;
    }

    // Build system prompt with document-type context
    std::string systemPrompt = AI_SYSTEM_PROMPT;
    if (!docType.empty())
        systemPrompt += " You are currently working with a " + docType + " document.";

    if (docType == "spreadsheet")
        systemPrompt +=
            " When referencing specific spreadsheet cells in your responses, "
            "format them as clickable links using this pattern: [B2](cell://B2), "
            "where the column letters come from the header row and the row number from the Row column."
            " When the user asks for a formula, use the set_cell_formula tool to insert it directly"
            " into the spreadsheet. Always use US English formula syntax with commas as argument"
            " separators (e.g., =VLOOKUP(A1,B:C,2,FALSE) not =VLOOKUP(A1;B:C;2;FALSE))."
            " Use standard Excel/Calc function names: SUM, AVERAGE, VLOOKUP, IF, COUNTIF,"
            " SUMIF, INDEX, MATCH, etc."
            " If you are unsure whether a function exists, call list_calc_functions to check."
            " If the user has selected spreadsheet data, use the cell addresses visible in that data"
            " to construct accurate cell references. If no target cell is specified by the user,"
            " choose a sensible empty cell near the data (e.g., below the last row or to the right)."
            " When setting multiple cells, always use a single set_cell_formula call with the"
            " 'formulas' array parameter to batch all cells into one operation."
            " Before inserting formulas with set_cell_formula, call evaluate_formula first"
            " to verify the result is correct. If the result is unexpected, fix the formula"
            " and evaluate again before inserting.";

    if (docType == "presentation")
        systemPrompt +=
            " When creating or modifying slides, you MUST format every slide:"
            " bold every title using EditTextObject with .uno:Bold,"
            " apply .uno:DefaultBullet to content placeholders that list multiple items,"
            " and choose the most appropriate layout for each slide's content from the"
            " Available layouts list in the tool description."
            " Do not use the same layout for every slide."
            " Do not prefix lines with '- ' when using DefaultBullet (the bullet marker"
            " is added automatically)."
            " In content placeholders, put only the items to be bulleted. Do not add"
            " sub-headings or blank lines before the bullet items."
            " When calling transform_document_structure, include a 'summary' parameter"
            " with a markdown preview of ONLY the slides being created or modified in"
            " this transform call, not pre-existing slides.";

    Poco::JSON::Array::Ptr sanitizedMessages = new Poco::JSON::Array();

    for (unsigned i = 0; i < messages->size(); ++i)
    {
        auto msg = messages->getObject(i);
        if (!msg)
            continue;

        std::string role;
        JsonUtil::findJSONValue(msg, "role", role);

        // Only allow user and assistant roles
        if (role != "user" && role != "assistant")
            continue;

        std::string content;
        JsonUtil::findJSONValue(msg, "content", content);
        if (content.size() > MAX_AI_MESSAGE_LENGTH)
        {
            sendChatResult(false, "Message too long", requestId);
            return true;
        }

        sanitizedMessages->add(msg);
    }

    // Check whether the last user message includes selected text from the document.
    bool hasSelectedText = false;
    for (int i = static_cast<int>(messages->size()) - 1; i >= 0; --i)
    {
        auto msg = messages->getObject(i);
        if (!msg)
            continue;
        std::string role;
        JsonUtil::findJSONValue(msg, "role", role);
        if (role == "user")
        {
            std::string content;
            JsonUtil::findJSONValue(msg, "content", content);
            if (content.find("[Selected text from document:") != std::string::npos)
                hasSelectedText = true;
            break;
        }
    }

    systemPrompt +=
        " You have tools to inspect and modify the document."
        " Use transform_document_structure to make changes.";

    if (hasSelectedText)
        systemPrompt +=
            " The user has shared selected text from the document as context."
            " Use that context directly to answer their question or make changes."
            " Only call extract_document_structure if you need information about"
            " parts of the document beyond the selection.";
    else
        systemPrompt +=
            " Use extract_document_structure when you need to understand the"
            " existing document layout before making changes."
            " When the user asks you to create new content from scratch (like a table"
            " or text), just generate it directly without extracting first.";

    // Prepend system message: build a new array with system first, then the rest.
    Poco::JSON::Array::Ptr finalMessages = new Poco::JSON::Array();
    Poco::JSON::Object::Ptr systemMsg = new Poco::JSON::Object();
    systemMsg->set("role", "system");
    systemMsg->set("content", systemPrompt);
    finalMessages->add(systemMsg);
    for (unsigned i = 0; i < sanitizedMessages->size(); ++i)
        finalMessages->add(sanitizedMessages->get(i));
    sanitizedMessages = finalMessages;

    // Trim to most recent messages if over limit (keep system prompt at index 0)
    while (sanitizedMessages->size() > MAX_AI_MESSAGES + 1)
        sanitizedMessages->remove(1);

    // Get AI provider settings
    const std::string apiKey = _session.getAIProviderAPIKey();
    const std::string model = _session.getAIProviderModel();
    std::string baseUrl = _session.getAIProviderURL();

    if (!ConfigUtil::getConfigValue<bool>("ai.enabled", false))
    {
        sendChatResult(false, "AI features are disabled by the administrator", requestId);
        return true;
    }

    if (_session.isDisableAISettings())
    {
        sendChatResult(false, "AI features are disabled for this document", requestId);
        return true;
    }

    if (apiKey.empty() || model.empty() || baseUrl.empty())
    {
        sendChatResult(false, "AI settings not configured", requestId);
        return true;
    }

    if (!baseUrl.empty() && baseUrl.back() == '/')
        baseUrl.pop_back();

    LOG_DBG("AIChatAction: request [" << requestId << "] with "
            << sanitizedMessages->size() << " messages, model: " << model);

    std::string requestUrl = std::move(baseUrl);
    requestUrl.append("/v1/chat/completions");

    // Initialize the tool loop state
    _toolLoop = std::make_unique<AIToolLoopState>();
    _toolLoop->requestId = requestId;
    _toolLoop->messages = sanitizedMessages;
    _toolLoop->model = model;
    _toolLoop->requestUrl = requestUrl;
    _toolLoop->apiKey = apiKey;

    callLLMAPI();
    return true;
}

void AIChatSession::callLLMAPI()
{
    if (!_toolLoop)
        return;

    Poco::JSON::Object::Ptr payload = new Poco::JSON::Object();
    payload->set("model", _toolLoop->model);
    payload->set("messages", _toolLoop->messages);
    payload->set("tools", buildToolDefinitions());

    std::ostringstream payloadStream;
    payload->stringify(payloadStream);
    std::string payloadStr = payloadStream.str();

    std::shared_ptr<http::Session> httpSession =
        http::Session::create(_toolLoop->requestUrl);
    if (!httpSession)
    {
        LOG_WRN("AIToolLoop: failed to create HTTP session");
        sendChatResult(false, "Failed to create HTTP session", _toolLoop->requestId);
        _toolLoop.reset();
        return;
    }

    httpSession->setTimeout(std::chrono::seconds(_session.getAIRequestTimeoutSeconds()));

    auto clientSessionPtr = _session.client_from_this();
    AIChatSession* self = this;

    http::Session::FinishedCallback finishedCallback =
        [clientSessionPtr, self](const std::shared_ptr<http::Session>& session)
    {
        self->_activeChatSession.reset();

        if (!self->_toolLoop)
            return;

        const std::string& requestId = self->_toolLoop->requestId;
        const std::shared_ptr<const http::Response> httpResponse = session->response();
        const http::StatusCode statusCode = httpResponse->statusLine().statusCode();

        if (statusCode == http::StatusCode::None)
        {
            self->sendChatResult(false, "Request timeout", requestId);
            self->_toolLoop.reset();
            return;
        }

        if (statusCode != http::StatusCode::OK)
        {
            const std::string& body = httpResponse->getBody();
            std::cerr << "AIToolLoop: API returned "
                << static_cast<int>(statusCode) << ' '
                << httpResponse->statusLine().reasonPhrase()
                << " body: " << body.substr(0, 500) << std::endl;
            const std::string errorMessage = mapHttpStatusToError(
                statusCode, httpResponse->statusLine().reasonPhrase());
            self->sendChatResult(false, errorMessage, requestId);
            self->_toolLoop.reset();
            return;
        }

        self->handleLLMResponse(httpResponse->getBody());
    };

    httpSession->setFinishedHandler(std::move(finishedCallback));

    http::Session::ConnectFailCallback connectFailCallback =
        [clientSessionPtr, self](const std::shared_ptr<http::Session>& /*session*/)
    {
        self->_activeChatSession.reset();
        if (self->_toolLoop)
        {
            self->sendChatResult(
                false, "Network error - please check your connection",
                self->_toolLoop->requestId);
            self->_toolLoop.reset();
        }
    };
    httpSession->setConnectFailHandler(std::move(connectFailCallback));

    http::Request httpRequest(Poco::URI(_toolLoop->requestUrl).getPathAndQuery());
    httpRequest.setVerb(http::Request::VERB_POST);
    httpRequest.set("Content-Type", "application/json");
    std::string authHeader = "Bearer ";
    authHeader.append(_toolLoop->apiKey);
    httpRequest.set("Authorization", std::move(authHeader));
    httpRequest.setBody(std::move(payloadStr), "application/json");

    LOG_DBG("AIToolLoop: sending request [" << _toolLoop->requestId
            << "] round " << (6 - _toolLoop->toolRoundsRemaining)
            << " to " << _toolLoop->requestUrl);

    _activeChatSession = httpSession;
    std::shared_ptr<DocumentBroker> docBroker = _session.getDocumentBroker();
    httpSession->asyncRequest(httpRequest, docBroker->getPoll());
}

void AIChatSession::handleLLMResponse(const std::string& responseBody)
{
    if (!_toolLoop)
        return;

    const std::string& requestId = _toolLoop->requestId;

    Poco::JSON::Object::Ptr responseObject = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(responseBody, responseObject))
    {
        sendChatResult(false, "No response from AI", requestId);
        _toolLoop.reset();
        return;
    }

    Poco::JSON::Array::Ptr choices = responseObject->getArray("choices");
    if (!choices || choices->size() == 0)
    {
        sendChatResult(false, "No response from AI", requestId);
        _toolLoop.reset();
        return;
    }

    Poco::JSON::Object::Ptr choice = choices->getObject(0);
    if (!choice)
    {
        sendChatResult(false, "No response from AI", requestId);
        _toolLoop.reset();
        return;
    }

    std::string finishReason;
    JsonUtil::findJSONValue(choice, "finish_reason", finishReason);

    Poco::JSON::Object::Ptr message = choice->getObject("message");
    if (!message)
    {
        sendChatResult(false, "No response from AI", requestId);
        _toolLoop.reset();
        return;
    }

    // Check for tool calls
    Poco::JSON::Array::Ptr toolCalls = message->getArray("tool_calls");
    if (toolCalls && toolCalls->size() > 0)
    {
        if (_toolLoop->toolRoundsRemaining <= 0)
        {
            sendChatResult(false, "AI used too many tool steps", requestId);
            _toolLoop.reset();
            return;
        }
        --_toolLoop->toolRoundsRemaining;

        // Append the assistant message (with tool_calls) to the conversation
        _toolLoop->messages->add(message);

        // Capture the AI's text content so it can be shown in approval dialogs.
        std::string assistantContent;
        JsonUtil::findJSONValue(message, "content", assistantContent);
        if (!assistantContent.empty())
            _toolLoop->pendingSummary = assistantContent;

        // Queue all tool calls for sequential processing
        _toolLoop->pendingToolCalls.clear();
        for (unsigned i = 0; i < toolCalls->size(); ++i)
        {
            Poco::JSON::Object::Ptr call = toolCalls->getObject(i);
            if (!call)
                continue;

            PendingToolCall pending;
            JsonUtil::findJSONValue(call, "id", pending.toolCallId);

            Poco::JSON::Object::Ptr fn = call->getObject("function");
            if (!fn)
                continue;

            JsonUtil::findJSONValue(fn, "name", pending.functionName);
            JsonUtil::findJSONValue(fn, "arguments", pending.arguments);
            _toolLoop->pendingToolCalls.push_back(std::move(pending));
        }

        // Start processing the first queued tool call
        processNextPendingToolCall();
        return;
    }

    // No tool calls - this is the final text response
    std::string result;
    std::string reasoning;
    JsonUtil::findJSONValue(message, "content", result);
    JsonUtil::findJSONValue(message, "reasoning", reasoning);

    if (result.empty())
    {
        if (!reasoning.empty())
        {
            sendChatResult(false,
                "This model returned only internal reasoning and no output. Try a "
                "different model or shorter input.", requestId);
        }
        else if (finishReason == "length")
        {
            sendChatResult(false,
                "The model ran out of tokens before producing output. Try a "
                "shorter input or a model with a larger output budget.", requestId);
        }
        else
        {
            sendChatResult(false, "No response from AI", requestId);
        }
        _toolLoop.reset();
        return;
    }

    sendChatResult(true, result, requestId);
    _toolLoop.reset();
}

bool AIChatSession::executeToolCall(const std::string& toolCallId,
                                    const std::string& fnName,
                                    const std::string& argsJson)
{
    if (!_toolLoop)
        return false;

    const std::string requestId = _toolLoop->requestId;

    // generate_image - delegate to existing handler (terminates tool loop)
    if (fnName == "generate_image")
    {
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        std::string imagePrompt;
        if (JsonUtil::parseJSON(argsJson, argsObj))
            JsonUtil::findJSONValue(argsObj, "prompt", imagePrompt);

        if (imagePrompt.empty())
        {
            sendChatResult(false, "Image generation failed: no prompt from model", requestId);
            _toolLoop.reset();
            return true;
        }

        _toolLoop.reset(); // image generation is terminal
        handleImageGeneration(imagePrompt, requestId);
        return true;
    }

    std::shared_ptr<DocumentBroker> docBroker = _session.getDocumentBroker();
    if (!docBroker)
    {
        sendChatResult(false, "Document not available", requestId);
        _toolLoop.reset();
        return true;
    }

    // extract_document_structure - requires user approval
    if (fnName == "extract_document_structure")
    {
        std::string filter;
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        if (JsonUtil::parseJSON(argsJson, argsObj))
            JsonUtil::findJSONValue(argsObj, "filter", filter);

        std::string command = "extractdocumentstructure url=interactive";
        if (!filter.empty())
            command += " filter=" + filter;

        _toolLoop->awaitingApproval = true;
        _toolLoop->pendingToolCallId = toolCallId;
        _toolLoop->pendingToolName = fnName;
        _toolLoop->pendingForwardCommand = command;

        sendToolApproval(fnName, "");
        return true;
    }

    // extract_link_targets - read-only, send to kit
    if (fnName == "extract_link_targets")
    {
        _toolLoop->awaitingKitResponse = true;
        _toolLoop->pendingToolCallId = toolCallId;
        _toolLoop->pendingToolName = fnName;

        sendToolProgress(fnName, "Extracting link targets...");
        docBroker->forwardToChild(_session.client_from_this(),
            "extractlinktargets url=interactive");
        return true;
    }

    // evaluate_formula - read-only, send to kit
    if (fnName == "evaluate_formula")
    {
        std::string cell, formula;
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        if (JsonUtil::parseJSON(argsJson, argsObj))
        {
            JsonUtil::findJSONValue(argsObj, "cell", cell);
            JsonUtil::findJSONValue(argsObj, "formula", formula);
        }

        if (cell.empty() || formula.empty())
        {
            continueToolLoop(toolCallId,
                "{\"error\":\"Missing cell or formula parameter\"}");
            return true;
        }

        _toolLoop->awaitingKitResponse = true;
        _toolLoop->pendingToolCallId = toolCallId;
        _toolLoop->pendingToolName = fnName;

        std::string encodedFormula;
        Poco::URI::encode(formula, "", encodedFormula);

        sendToolProgress(fnName, "Evaluating formula...");
        docBroker->forwardToChild(_session.client_from_this(),
            "commandvalues command=.uno:EvaluateFormula?cell="
            + cell + "&formula=" + encodedFormula);
        return true;
    }

    // list_calc_functions - read-only, send to kit
    if (fnName == "list_calc_functions")
    {
        _toolLoop->awaitingKitResponse = true;
        _toolLoop->pendingToolCallId = toolCallId;
        _toolLoop->pendingToolName = fnName;

        sendToolProgress(fnName, "Loading function catalog...");
        docBroker->forwardToChild(_session.client_from_this(),
            "commandvalues command=.uno:CalcFunctionList");
        return true;
    }

    // set_cell_formula - requires user approval (single or batch)
    if (fnName == "set_cell_formula")
    {
        std::string cell, formula, formulasJson, summary;
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        if (JsonUtil::parseJSON(argsJson, argsObj))
        {
            JsonUtil::findJSONValue(argsObj, "cell", cell);
            JsonUtil::findJSONValue(argsObj, "formula", formula);
            JsonUtil::findJSONValue(argsObj, "formulas", formulasJson);
            JsonUtil::findJSONValue(argsObj, "summary", summary);
        }

        // Build a JSON array of {cell, formula} pairs for uniform handling.
        Poco::JSON::Array::Ptr pairs = new Poco::JSON::Array();

        if (!formulasJson.empty())
        {
            // Batch mode: parse the formulas array
            Poco::JSON::Array::Ptr parsed;
            try
            {
                Poco::JSON::Parser parser;
                auto result = parser.parse(formulasJson);
                parsed = result.extract<Poco::JSON::Array::Ptr>();
            }
            catch (const std::exception&)
            {
                continueToolLoop(toolCallId,
                    "{\"error\":\"Invalid JSON in formulas parameter. "
                    "Must be an array of {cell, formula} objects.\"}");
                return true;
            }

            for (unsigned i = 0; i < parsed->size(); ++i)
            {
                auto obj = parsed->getObject(i);
                if (!obj) continue;
                std::string c, f;
                JsonUtil::findJSONValue(obj, "cell", c);
                JsonUtil::findJSONValue(obj, "formula", f);
                if (c.empty() || f.empty()) continue;
                Poco::JSON::Object::Ptr pair = new Poco::JSON::Object();
                pair->set("cell", c);
                pair->set("formula", f);
                pairs->add(pair);
            }
        }
        else if (!cell.empty() && !formula.empty())
        {
            // Single cell mode
            Poco::JSON::Object::Ptr pair = new Poco::JSON::Object();
            pair->set("cell", cell);
            pair->set("formula", formula);
            pairs->add(pair);
        }

        if (pairs->size() == 0)
        {
            continueToolLoop(toolCallId,
                "{\"error\":\"No valid cell/formula pairs provided. "
                "Use 'cell'+'formula' for one cell or 'formulas' array for batch.\"}");
            return true;
        }

        // Build summary for approval UI
        if (summary.empty())
        {
            summary = "";
            for (unsigned i = 0; i < pairs->size(); ++i)
            {
                auto p = pairs->getObject(i);
                std::string c, f;
                JsonUtil::findJSONValue(p, "cell", c);
                JsonUtil::findJSONValue(p, "formula", f);
                summary += "- ";
                summary += c;
                summary += ": `";
                summary += f;
                summary += "`\n";
            }
        }

        _toolLoop->awaitingApproval = true;
        _toolLoop->pendingToolCallId = toolCallId;
        _toolLoop->pendingToolName = fnName;
        _toolLoop->pendingSummary = summary;

        // Store the pairs array for execution after approval.
        std::ostringstream storedJson;
        pairs->stringify(storedJson);
        _toolLoop->pendingTransformArgs = storedJson.str();

        sendToolApproval(fnName, "");
        return true;
    }

    // transform_document_structure - requires user approval
    if (fnName == "transform_document_structure")
    {
        std::string transform;
        std::string summary;
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        if (JsonUtil::parseJSON(argsJson, argsObj))
        {
            JsonUtil::findJSONValue(argsObj, "transform", transform);
            JsonUtil::findJSONValue(argsObj, "summary", summary);
        }

        if (transform.empty())
        {
            continueToolLoop(toolCallId,
                "{\"error\":\"No transform parameter provided\"}");
            return true;
        }

        // findJSONValue resolves JSON escapes, so literal control characters
        // (newlines, tabs) may appear inside string values. Escape them so
        // the inner JSON is valid, then re-serialize through Poco for a
        // clean string that core can parse.
        {
            std::string sanitized;
            sanitized.reserve(transform.size());
            bool inStr = false;
            for (std::size_t i = 0; i < transform.size(); ++i)
            {
                char c = transform[i];
                if (c == '"' && (i == 0 || transform[i - 1] != '\\'))
                    inStr = !inStr;
                if (inStr)
                {
                    if (c == '\n') { sanitized += "\\n"; continue; }
                    if (c == '\r') { sanitized += "\\r"; continue; }
                    if (c == '\t') { sanitized += "\\t"; continue; }
                }
                sanitized += c;
            }
            transform = sanitized;
        }

        Poco::JSON::Object::Ptr transformObj = new Poco::JSON::Object();
        if (JsonUtil::parseJSON(transform, transformObj))
        {
            std::ostringstream oss;
            transformObj->stringify(oss);
            transform = oss.str();
        }
        else
        {
            continueToolLoop(toolCallId,
                "{\"error\":\"Invalid JSON in transform parameter. "
                "All slides must be in a single SlideCommands array within one "
                "Transforms object. Use InsertMasterSlide to add slides within "
                "the same array.\"}");
            return true;
        }

        // Navigation-only transforms (JumpToSlide) do not modify the document
        // and can be executed without user approval.
        bool navigationOnly = false;
        {
            Poco::JSON::Object::Ptr transforms = transformObj->getObject("Transforms");
            Poco::JSON::Array::Ptr cmds =
                transforms ? transforms->getArray("SlideCommands") : nullptr;
            if (cmds && cmds->size() > 0)
            {
                navigationOnly = true;
                for (unsigned i = 0; i < cmds->size(); ++i)
                {
                    Poco::JSON::Object::Ptr cmd = cmds->getObject(i);
                    if (!cmd ||
                        !(cmd->has("JumpToSlide") || cmd->has("JumpToSlideByName")))
                    {
                        navigationOnly = false;
                        break;
                    }
                }
            }
        }

        if (navigationOnly)
        {
            _toolLoop->pendingToolCallId = toolCallId;
            _toolLoop->pendingToolName = fnName;
            _toolLoop->awaitingKitResponse = true;

            std::string encodedTransform;
            Poco::URI::encode(transform, "", encodedTransform);
            docBroker->forwardToChild(_session.client_from_this(),
                "transformdocumentstructure url=interactive transform=" + encodedTransform);
            return true;
        }

        _toolLoop->awaitingApproval = true;
        _toolLoop->pendingToolCallId = toolCallId;
        _toolLoop->pendingToolName = fnName;
        _toolLoop->pendingTransformArgs = transform;
        _toolLoop->pendingSummary = summary;

        sendToolApproval(fnName, transform);
        return true;
    }

    // Unknown tool - feed error back to LLM
    continueToolLoop(toolCallId, "{\"error\":\"Unknown tool: " + fnName + "\"}");
    return true;
}

void AIChatSession::processNextPendingToolCall()
{
    if (!_toolLoop)
        return;

    if (_toolLoop->pendingToolCalls.empty())
    {
        // All tool calls processed, call LLM with all results
        sendToolProgress(_toolLoop->pendingToolName, "Thinking...");
        callLLMAPI();
        return;
    }

    PendingToolCall next = std::move(_toolLoop->pendingToolCalls.front());
    _toolLoop->pendingToolCalls.erase(_toolLoop->pendingToolCalls.begin());

    LOG_DBG("AIToolLoop: tool call [" << next.functionName << "] id=" << next.toolCallId
            << " for request [" << _toolLoop->requestId << ']');

    executeToolCall(next.toolCallId, next.functionName, next.arguments);
}

void AIChatSession::continueToolLoop(const std::string& toolCallId,
                                     const std::string& result)
{
    if (!_toolLoop)
        return;

    // Append tool result message to the conversation
    Poco::JSON::Object::Ptr toolResult = new Poco::JSON::Object();
    toolResult->set("role", "tool");
    toolResult->set("tool_call_id", toolCallId);
    toolResult->set("content", result);
    _toolLoop->messages->add(toolResult);

    _toolLoop->awaitingKitResponse = false;
    _toolLoop->awaitingApproval = false;

    // Process the next queued tool call, or call LLM if all done
    processNextPendingToolCall();
}

void AIChatSession::sendToolProgress(const std::string& toolName,
                                     const std::string& status)
{
    if (!_toolLoop)
        return;

    Poco::JSON::Object::Ptr progress = new Poco::JSON::Object();
    progress->set("requestId", _toolLoop->requestId);
    progress->set("toolName", toolName);
    progress->set("status", status);

    std::ostringstream oss;
    progress->stringify(oss);
    _session.sendTextFrame("aichatprogress: " + oss.str());
}

void AIChatSession::sendToolApproval(const std::string& toolName,
                                     const std::string& transformJson)
{
    if (!_toolLoop)
        return;

    Poco::JSON::Object::Ptr approval = new Poco::JSON::Object();
    approval->set("requestId", _toolLoop->requestId);
    approval->set("toolName", toolName);
    approval->set("transformJson", transformJson);
    if (!_toolLoop->pendingSummary.empty())
        approval->set("summary", _toolLoop->pendingSummary);

    std::ostringstream oss;
    approval->stringify(oss);
    _session.sendTextFrame("aichatapproval: " + oss.str());
}

bool AIChatSession::handleApprove(const std::string& firstLine)
{
    const std::string jsonPayload = firstLine.substr(strlen("aichatapprove: "));

    Poco::JSON::Object::Ptr obj = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(jsonPayload, obj))
    {
        LOG_WRN("AIChatApprove: invalid JSON");
        return true;
    }

    std::string action;
    JsonUtil::findJSONValue(obj, "action", action);

    if (!_toolLoop || !_toolLoop->awaitingApproval)
    {
        LOG_WRN("AIChatApprove: no pending approval");
        return true;
    }

    const std::string toolCallId = _toolLoop->pendingToolCallId;

    if (action == "approve")
    {
        std::shared_ptr<DocumentBroker> docBroker = _session.getDocumentBroker();
        if (!docBroker)
        {
            sendChatResult(false, "Document not available", _toolLoop->requestId);
            _toolLoop.reset();
            return true;
        }

        std::string command;
        if (!_toolLoop->pendingForwardCommand.empty())
        {
            // Generic forwarding (extract_document_structure, etc.)
            command = _toolLoop->pendingForwardCommand;
            _toolLoop->pendingForwardCommand.clear();

            _toolLoop->awaitingApproval = false;
            _toolLoop->awaitingKitResponse = true;

            sendToolProgress(_toolLoop->pendingToolName, "Working...");
            docBroker->forwardToChild(_session.client_from_this(), command);
        }
        else if (_toolLoop->pendingToolName == "set_cell_formula")
        {
            _toolLoop->awaitingApproval = false;

            // Parse the stored array of {cell, formula} pairs
            Poco::JSON::Array::Ptr pairs;
            try
            {
                Poco::JSON::Parser parser;
                auto result = parser.parse(_toolLoop->pendingTransformArgs);
                pairs = result.extract<Poco::JSON::Array::Ptr>();
            }
            catch (const std::exception&)
            {
                continueToolLoop(toolCallId, "{\"error\":\"Internal error parsing stored formulas\"}");
                return true;
            }

            sendToolProgress("set_cell_formula", "Setting formulas...");

            // Dispatch GoToCell + EnterString for each pair
            Poco::JSON::Array resultArr;
            for (unsigned i = 0; i < pairs->size(); ++i)
            {
                auto p = pairs->getObject(i);
                if (!p) continue;
                std::string cell, formula;
                JsonUtil::findJSONValue(p, "cell", cell);
                JsonUtil::findJSONValue(p, "formula", formula);

                std::string escapedCell = JsonUtil::escapeJSONValue(cell);
                std::string escapedFormula = JsonUtil::escapeJSONValue(formula);

                std::string goToArgs = "{\"ToPoint\":{\"type\":\"string\",\"value\":\""
                    + escapedCell + "\"}}";
                docBroker->forwardToChild(_session.client_from_this(),
                    "uno .uno:GoToCell " + goToArgs);

                std::string enterArgs = "{\"StringName\":{\"type\":\"string\",\"value\":\""
                    + escapedFormula + "\"}}";
                docBroker->forwardToChild(_session.client_from_this(),
                    "uno .uno:EnterString " + enterArgs);

                Poco::JSON::Object::Ptr r = new Poco::JSON::Object();
                r->set("cell", cell);
                r->set("formula", formula);
                resultArr.add(r);
            }

            // Continue tool loop with success
            std::ostringstream resultJson;
            Poco::JSON::Object resultObj;
            resultObj.set("success", true);
            resultObj.set("cells", resultArr);
            resultObj.stringify(resultJson);
            continueToolLoop(toolCallId, resultJson.str());
        }
        else
        {
            // transform_document_structure - check for GenerateImage commands
            _toolLoop->awaitingApproval = false;
            processTransformImageGenerations(docBroker);
        }
    }
    else
    {
        // User rejected - feed rejection back to LLM with tool-specific message
        _toolLoop->awaitingApproval = false;
        std::string rejectionMsg;
        if (_toolLoop->pendingToolName == "extract_document_structure")
            rejectionMsg =
                "{\"error\":\"User declined document inspection. "
                "Answer their request directly without inspecting the document. "
                "If the request is to create new content, just generate it.\"}";
        else if (_toolLoop->pendingToolName == "set_cell_formula")
            rejectionMsg =
                "{\"error\":\"User rejected the formula insertion. "
                "Show them the formula in a code block so they can copy it manually, "
                "and ask if they would like a different formula.\"}";
        else
            rejectionMsg =
                "{\"error\":\"User rejected the document modification. "
                "Explain what you wanted to do and ask if they would like a different approach.\"}";
        continueToolLoop(toolCallId, rejectionMsg);
    }

    return true;
}

bool AIChatSession::handleCancel(const std::string& firstLine)
{
    const std::string cancelRequestId = firstLine.substr(strlen("aichatcancel: "));
    LOG_DBG("AIChatCancel: cancelling request [" << cancelRequestId << ']');

    if (_activeChatSession)
    {
        _activeChatSession->asyncShutdown();
        _activeChatSession.reset();
    }

    _toolLoop.reset();

    return true;
}

ImageGenRequest AIChatSession::createImageGenRequest(const std::string& prompt)
{
    ImageGenRequest req;

    req.apiKey = _session.getAIImageProviderAPIKey();
    if (req.apiKey.empty())
        req.apiKey = _session.getAIProviderAPIKey();
    std::string baseUrl = _session.getAIImageProviderURL();
    if (baseUrl.empty())
        baseUrl = _session.getAIProviderURL();

    if (req.apiKey.empty() || baseUrl.empty())
    {
        req.error = "AI image settings not configured";
        return req;
    }

    if (!baseUrl.empty() && baseUrl.back() == '/')
        baseUrl.pop_back();

    req.requestUrl = baseUrl + "/v1/images/generations";

    const std::string imageModel = _session.getAIImageModel();
    if (imageModel.empty())
    {
        req.error = "Image model not configured";
        return req;
    }

    Poco::JSON::Object::Ptr payload = new Poco::JSON::Object();
    payload->set("prompt", prompt);
    std::string imageSize = _session.getAIImageSize();
    if (imageSize.empty() || !isValidImageSize(imageSize))
        imageSize = "1024x1024";
    payload->set("size", imageSize);
    payload->set("n", 1);
    payload->set("response_format", "b64_json");
    payload->set("model", imageModel);

    LOG_DBG("AIImageGeneration: model=" << imageModel << ", size=" << imageSize);

    std::ostringstream payloadStream;
    payload->stringify(payloadStream);
    req.payloadStr = payloadStream.str();

    req.httpSession = http::Session::create(req.requestUrl);
    if (!req.httpSession)
    {
        req.error = "Failed to create HTTP session";
        return req;
    }

    req.httpSession->setTimeout(std::chrono::seconds(_session.getAIRequestTimeoutSeconds()));
    return req;
}

std::pair<std::string, std::string> AIChatSession::parseImageGenResponse(
    const std::shared_ptr<const http::Response>& httpResponse)
{
    const http::StatusCode statusCode = httpResponse->statusLine().statusCode();

    if (statusCode == http::StatusCode::None)
        return {"", "Request timeout"};

    if (statusCode != http::StatusCode::OK)
    {
        const std::string& body = httpResponse->getBody();
        LOG_WRN_S("AIImageGeneration: HTTP " << static_cast<int>(statusCode) << ": " << body);

        std::string errorMsg = "HTTP " + std::to_string(static_cast<int>(statusCode));
        Poco::JSON::Object::Ptr errObj;
        if (JsonUtil::parseJSON(body, errObj))
        {
            Poco::JSON::Object::Ptr errorDetail = errObj->getObject("error");
            if (errorDetail)
            {
                std::string message;
                JsonUtil::findJSONValue(errorDetail, "message", message);
                if (!message.empty())
                    errorMsg = message;
            }
        }
        return {"", errorMsg};
    }

    const std::string& responseBody = httpResponse->getBody();
    Poco::JSON::Object::Ptr responseObject = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(responseBody, responseObject))
        return {"", "Failed to parse response"};

    Poco::JSON::Array::Ptr dataArray = responseObject->getArray("data");
    if (!dataArray || dataArray->size() == 0)
        return {"", "No image generated"};

    Poco::JSON::Object::Ptr firstItem = dataArray->getObject(0);
    if (!firstItem)
        return {"", "No image generated"};

    std::string b64Json;
    JsonUtil::findJSONValue(firstItem, "b64_json", b64Json);
    if (b64Json.empty())
        return {"", "No image data in response"};

    return {std::move(b64Json), ""};
}

bool AIChatSession::handleImageGeneration(const std::string& prompt,
                                          const std::string& requestId)
{
    LOG_DBG("AIImageGeneration: request [" << requestId
            << "], prompt: " << prompt);

    ImageGenRequest req = createImageGenRequest(prompt);
    if (!req.error.empty())
    {
        sendChatResult(false, req.error, requestId);
        return true;
    }

    // Send image result via aichatresult with imageData field
    auto clientSessionPtr = _session.client_from_this();
    auto sendImageResult = [clientSession = clientSessionPtr, requestId](
                               bool success, const std::string& imageData,
                               const std::string& error)
    {
        Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
        result->set("success", success);
        if (success)
            result->set("imageData", imageData);
        else
            result->set("error", error);
        result->set("requestId", requestId);

        std::ostringstream oss;
        result->stringify(oss);
        clientSession->sendTextFrame("aichatresult: " + oss.str());
    };

    AIChatSession* self = this;

    http::Session::FinishedCallback finishedCallback =
        [clientSessionPtr, self, sendImageResult](const std::shared_ptr<http::Session>& session)
    {
        self->_activeChatSession.reset();

        auto [b64Json, error] = parseImageGenResponse(session->response());
        if (!error.empty())
        {
            sendImageResult(false, "", error);
            return;
        }

        sendImageResult(true, b64Json, "");
    };

    req.httpSession->setFinishedHandler(std::move(finishedCallback));

    http::Session::ConnectFailCallback connectFailCallback =
        [clientSessionPtr = std::move(clientSessionPtr), self,
         sendImageResult = std::move(sendImageResult)](
            const std::shared_ptr<http::Session>& /*session*/)
    {
        self->_activeChatSession.reset();
        sendImageResult(false, "", "Network error - please check your connection");
    };
    req.httpSession->setConnectFailHandler(std::move(connectFailCallback));

    http::Request httpRequest(Poco::URI(req.requestUrl).getPathAndQuery());
    httpRequest.setVerb(http::Request::VERB_POST);
    httpRequest.set("Content-Type", "application/json");
    httpRequest.set("Authorization", "Bearer " + req.apiKey);
    httpRequest.setBody(req.payloadStr, "application/json");

    LOG_DBG("AIImageGeneration: sending request [" << requestId << "] to "
            << req.requestUrl);

    _activeChatSession = req.httpSession;
    std::shared_ptr<DocumentBroker> docBroker = _session.getDocumentBroker();
    req.httpSession->asyncRequest(httpRequest, docBroker->getPoll());
    return true;
}

void AIChatSession::processTransformImageGenerations(
    const std::shared_ptr<DocumentBroker>& docBroker)
{
    if (!_toolLoop)
        return;

    const std::string& transform = _toolLoop->pendingTransformArgs;

    // Parse transform to find GenerateImage.N commands in SlideCommands
    Poco::JSON::Object::Ptr transformObj = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(transform, transformObj))
    {
        // Should not happen - already validated earlier
        _toolLoop->awaitingKitResponse = true;
        std::string encodedTransform;
        Poco::URI::encode(transform, "", encodedTransform);
        sendToolProgress(_toolLoop->pendingToolName, "Working...");
        docBroker->forwardToChild(_session.client_from_this(),
            "transformdocumentstructure url=interactive transform=" + encodedTransform);
        return;
    }

    Poco::JSON::Object::Ptr transforms = transformObj->getObject("Transforms");
    Poco::JSON::Array::Ptr cmds =
        transforms ? transforms->getArray("SlideCommands") : nullptr;

    _toolLoop->pendingImageGens.clear();
    _toolLoop->nextImageGenIndex = 0;
    _toolLoop->generatingImages = false;
    _toolLoop->outstandingImageTransforms = 0;
    _toolLoop->mainTransformResult.clear();
    _toolLoop->failedImagePrompts.clear();

    // Track current slide index as we scan commands
    int currentSlide = 0;
    int pageCount = 1;

    if (cmds)
    {
        for (unsigned i = 0; i < cmds->size(); ++i)
        {
            Poco::JSON::Object::Ptr cmd = cmds->getObject(i);
            if (!cmd)
                continue;

            // Track slide navigation to determine which slide each
            // GenerateImage targets
            if (cmd->has("JumpToSlide"))
            {
                std::string val = cmd->getValue<std::string>("JumpToSlide");
                if (val == "last")
                    currentSlide = pageCount - 1;
                else
                {
                    try
                    {
                        currentSlide = std::stoi(val);
                    }
                    catch (const std::exception&)
                    {
                        LOG_WRN("TransformImageGen: invalid JumpToSlide value: " << val);
                    }
                }
            }
            else if (cmd->has("InsertMasterSlide")
                     || cmd->has("InsertMasterSlideByName"))
            {
                currentSlide++;
                pageCount++;
            }
            else if (cmd->has("DeleteSlide"))
            {
                if (pageCount > 1)
                    pageCount--;
            }

            static const std::string kGenerateImagePrefix = "GenerateImage.";
            for (const auto& key : cmd->getNames())
            {
                if (key.substr(0, kGenerateImagePrefix.size()) == kGenerateImagePrefix)
                {
                    int objId;
                    try
                    {
                        objId = std::stoi(key.substr(kGenerateImagePrefix.size()));
                    }
                    catch (const std::exception&)
                    {
                        LOG_WRN("TransformImageGen: invalid GenerateImage key: " << key);
                        continue;
                    }

                    std::string prompt = cmd->getValue<std::string>(key);
                    _toolLoop->pendingImageGens.push_back(
                        {currentSlide, objId, prompt, std::string()});

                    // Replace GenerateImage.N with InsertImage.N pointing
                    // to the loading placeholder
                    cmd->remove(key);
                    cmd->set("InsertImage." + std::to_string(objId),
                        "file://" + std::string(JAILED_DOCUMENT_ROOT)
                        + "insertfile/ai-loading-placeholder.png");
                }
            }
        }
    }

    if (_toolLoop->pendingImageGens.size() > MAX_AI_IMAGE_GENERATIONS)
    {
        LOG_WRN("TransformImageGen: capping image generations from "
                << _toolLoop->pendingImageGens.size()
                << " to " << MAX_AI_IMAGE_GENERATIONS);
        _toolLoop->pendingImageGens.resize(MAX_AI_IMAGE_GENERATIONS);
    }

    // Copy the placeholder image to the jail insertfile directory
    if (!_toolLoop->pendingImageGens.empty())
    {
        const std::string jailId = docBroker->getJailId();
        const std::string dirPath = FileUtil::buildLocalPathToJail(
            COOLWSD::EnableMountNamespaces, COOLWSD::ChildRoot + jailId,
            std::string(JAILED_DOCUMENT_ROOT) + "insertfile");
        Poco::File(dirPath).createDirectories();

        const std::string srcPath = COOLWSD::FileServerRoot
            + "/browser/dist/images/ai-loading-placeholder.png";
        const std::string dstPath = dirPath + "/ai-loading-placeholder.png";

        try
        {
            Poco::File src(srcPath);
            if (src.exists())
                src.copyTo(dstPath);
            else
                LOG_WRN("TransformImageGen: placeholder not found at " << srcPath);
        }
        catch (const std::exception& ex)
        {
            LOG_WRN("TransformImageGen: failed to copy placeholder: " << ex.what());
        }
    }

    // Serialize the modified transform (with placeholders or unchanged)
    std::ostringstream oss;
    transformObj->stringify(oss);
    std::string modifiedTransform = oss.str();

    // Forward the transform immediately (with loading placeholders if any)
    _toolLoop->awaitingKitResponse = true;
    std::string encodedTransform;
    Poco::URI::encode(modifiedTransform, "", encodedTransform);
    sendToolProgress(_toolLoop->pendingToolName, "Working...");
    docBroker->forwardToChild(_session.client_from_this(),
        "transformdocumentstructure url=interactive transform=" + encodedTransform);

    // Mark that we need to generate images after the kit responds
    if (!_toolLoop->pendingImageGens.empty())
        _toolLoop->generatingImages = true;
}

std::string AIChatSession::appendImageGenFailures(const std::string& result) const
{
    if (!_toolLoop || _toolLoop->failedImagePrompts.empty())
        return result;

    std::string augmented = result;
    augmented += "\n\nNote: ";
    augmented += std::to_string(_toolLoop->failedImagePrompts.size());
    augmented += " image(s) failed to generate and still show a loading placeholder."
                 " Failed prompts: ";
    for (std::size_t i = 0; i < _toolLoop->failedImagePrompts.size(); ++i)
    {
        if (i > 0)
            augmented += ", ";
        augmented += "\"";
        augmented += _toolLoop->failedImagePrompts[i];
        augmented += "\"";
    }
    return augmented;
}

void AIChatSession::generateNextTransformImage(std::shared_ptr<DocumentBroker> docBroker)
{
    if (!_toolLoop)
        return;

    for (std::size_t& idx = _toolLoop->nextImageGenIndex;
         idx < _toolLoop->pendingImageGens.size(); ++idx)
    {
        const PendingImageGen& gen = _toolLoop->pendingImageGens[idx];
        const std::size_t total = _toolLoop->pendingImageGens.size();

        sendToolProgress(_toolLoop->pendingToolName, "Generating image " +
                                                         std::to_string(idx + 1) + " of " +
                                                         std::to_string(total) + "...");

        ImageGenRequest req = createImageGenRequest(gen.prompt);
        if (!req.error.empty())
        {
            LOG_WRN("TransformImageGen: " << req.error);
            _toolLoop->failedImagePrompts.push_back(gen.prompt);
            continue;
        }

        auto clientSessionPtr = _session.client_from_this();
        AIChatSession* self = this;

        auto onImageFail = [self, docBroker, idx]()
        {
            self->_toolLoop->failedImagePrompts.push_back(
                self->_toolLoop->pendingImageGens[idx].prompt);
            self->_toolLoop->nextImageGenIndex++;
            self->generateNextTransformImage(docBroker);
        };

        http::Session::FinishedCallback finishedCallback =
            [clientSessionPtr, self, docBroker, idx, onImageFail](
                const std::shared_ptr<http::Session>& session)
        {
            self->_activeChatSession.reset();

            if (!self->_toolLoop)
                return;

            auto [b64Json, error] = parseImageGenResponse(session->response());
            if (!error.empty())
            {
                LOG_WRN_S("TransformImageGen: " << error);
                onImageFail();
                return;
            }

            // Decode base64 and write to jail insertfile directory
            std::string binaryData;
            macaron::Base64::Decode(b64Json, binaryData);

            const std::string jailId = docBroker->getJailId();
            const std::string dirPath = FileUtil::buildLocalPathToJail(
                COOLWSD::EnableMountNamespaces, COOLWSD::ChildRoot + jailId,
                std::string(JAILED_DOCUMENT_ROOT) + "insertfile");

            Poco::File(dirPath).createDirectories();

            const std::string fileName =
                "ai_" + Util::rng::getHexString(8) + "_" + std::to_string(idx) + ".png";
            std::string filePath = dirPath;
            filePath += '/';
            filePath += fileName;

            std::ofstream fileStream(filePath, std::ios::out | std::ios::binary);
            fileStream.write(binaryData.data(), binaryData.size());
            fileStream.close();

            if (!fileStream.good())
            {
                LOG_WRN_S("TransformImageGen: failed to write image to " << filePath);
                FileUtil::removeFile(filePath);
                onImageFail();
                return;
            }

            // Build jail-internal file:// URL for core to load
            const std::string jailFileUrl =
                "file://" + std::string(JAILED_DOCUMENT_ROOT) + "insertfile/" + fileName;
            self->_toolLoop->pendingImageGens[idx].filePath = jailFileUrl;

            LOG_DBG_S("TransformImageGen: wrote image " << (idx + 1) << " to " << filePath);

            // Send a transform to replace the loading placeholder with the real
            // image on the correct slide.
            const auto& imgGen = self->_toolLoop->pendingImageGens[idx];
            std::string miniTransform = "{\"Transforms\":{\"SlideCommands\":["
                                        "{\"InsertImageAt." +
                                        std::to_string(imgGen.slideIndex) + "." +
                                        std::to_string(imgGen.objId) + "\":\"" + jailFileUrl +
                                        "\"}]}}";

            std::string encodedMini;
            Poco::URI::encode(miniTransform, "", encodedMini);
            self->_toolLoop->outstandingImageTransforms++;
            docBroker->forwardToChild(clientSessionPtr,
                "transformdocumentstructure url=interactive transform=" + encodedMini);

            self->_toolLoop->nextImageGenIndex++;
            self->generateNextTransformImage(docBroker);
        };

        req.httpSession->setFinishedHandler(std::move(finishedCallback));

        http::Session::ConnectFailCallback connectFailCallback =
            [clientSessionPtr, self, onImageFail](const std::shared_ptr<http::Session>& /*session*/)
        {
            self->_activeChatSession.reset();

            if (!self->_toolLoop)
                return;

            LOG_WRN_S("TransformImageGen: connection failed");
            onImageFail();
        };
        req.httpSession->setConnectFailHandler(std::move(connectFailCallback));

        http::Request httpRequest(Poco::URI(req.requestUrl).getPathAndQuery());
        httpRequest.setVerb(http::Request::VERB_POST);
        httpRequest.set("Content-Type", "application/json");
        httpRequest.set("Authorization", "Bearer " + req.apiKey);
        httpRequest.setBody(req.payloadStr, "application/json");

        LOG_DBG("TransformImageGen: generating image " << (idx + 1) << " of " << total
                                                       << ", prompt: " << gen.prompt);

        _activeChatSession = req.httpSession;
        req.httpSession->asyncRequest(httpRequest, docBroker->getPoll());
        return; // async request launched, callbacks will call back into this function
    }

    // All entries processed or skipped - check completion
    if (_toolLoop->outstandingImageTransforms > 0)
        return; // responses still pending, they will finish up
    _toolLoop->generatingImages = false;
    continueToolLoop(_toolLoop->pendingToolCallId,
        appendImageGenFailures(_toolLoop->mainTransformResult));
}

bool AIChatSession::tryConsumeKitError(const std::string& errorCommand,
                                       const std::string& errorKind)
{
    if (!_toolLoop || !_toolLoop->awaitingKitResponse)
        return false;
    if (errorCommand != "extractdocumentstructure"
        && errorCommand != "extractlinktargets"
        && errorCommand != "transformdocumentstructure")
        return false;

    LOG_WRN("AIToolLoop: kit error for " << errorCommand << ": " << errorKind);
    _toolLoop->awaitingKitResponse = false;
    continueToolLoop(_toolLoop->pendingToolCallId,
        "{\"error\":\"" + errorCommand + " failed: " + errorKind + "\"}");
    return true;
}

bool AIChatSession::tryConsumeCommandValues(const std::shared_ptr<Message>& payload)
{
    if (!_toolLoop || !_toolLoop->awaitingKitResponse)
        return false;
    if (_toolLoop->pendingToolName != "list_calc_functions"
        && _toolLoop->pendingToolName != "evaluate_formula")
        return false;

    _toolLoop->awaitingKitResponse = false;
    continueToolLoop(_toolLoop->pendingToolCallId, payload->jsonString());
    return true;
}

bool AIChatSession::tryConsumeExtractedLinkTargets(const std::shared_ptr<Message>& payload)
{
    if (!_toolLoop || !_toolLoop->awaitingKitResponse)
        return false;

    _toolLoop->awaitingKitResponse = false;
    continueToolLoop(_toolLoop->pendingToolCallId, payload->jsonString());
    return true;
}

bool AIChatSession::tryConsumeExtractedDocumentStructure(const std::shared_ptr<Message>& payload)
{
    if (!_toolLoop || !_toolLoop->awaitingKitResponse)
        return false;

    _toolLoop->awaitingKitResponse = false;
    continueToolLoop(_toolLoop->pendingToolCallId, payload->jsonString());
    return true;
}

bool AIChatSession::tryConsumeTransformedDocumentStructure(const std::shared_ptr<Message>& payload)
{
    if (!_toolLoop)
        return false;

    if (_toolLoop->awaitingKitResponse)
    {
        _toolLoop->awaitingKitResponse = false;

        // If we have pending image generations, start generating them
        // instead of continuing the AI tool loop. The main transform
        // (with loading placeholders) has been applied - now we
        // progressively replace placeholders with real images.
        if (_toolLoop->generatingImages)
        {
            _toolLoop->mainTransformResult = payload->jsonString();
            std::shared_ptr<DocumentBroker> broker = _session.getDocumentBroker();
            if (broker)
                generateNextTransformImage(broker);
            else
            {
                _toolLoop->generatingImages = false;
                continueToolLoop(_toolLoop->pendingToolCallId,
                    payload->jsonString());
            }
            return true;
        }

        continueToolLoop(_toolLoop->pendingToolCallId, payload->jsonString());
        return true;
    }

    // During progressive image generation, image insertion transforms
    // produce responses that reach here. Track their success and, when
    // all responses are in, continue the AI tool loop.
    if (_toolLoop->generatingImages)
    {
        if (_toolLoop->outstandingImageTransforms > 0)
        {
            _toolLoop->outstandingImageTransforms--;

            const std::string jsonResult = payload->jsonString();
            Poco::JSON::Object::Ptr resultObj;
            if (JsonUtil::parseJSON(jsonResult, resultObj))
            {
                bool success = false;
                JsonUtil::findJSONValue(resultObj, "success", success);
                if (!success)
                    LOG_WRN("Image insertion transform failed: " << jsonResult);
            }
        }

        if (_toolLoop->outstandingImageTransforms <= 0
            && _toolLoop->nextImageGenIndex
                   >= _toolLoop->pendingImageGens.size())
        {
            _toolLoop->generatingImages = false;
            continueToolLoop(_toolLoop->pendingToolCallId,
                appendImageGenFailures(_toolLoop->mainTransformResult));
        }
        return true;
    }

    return false;
}

#endif // !MOBILEAPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
