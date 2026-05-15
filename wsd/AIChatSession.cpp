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

#include "ClientSession.hpp"

#include <common/AIHttpTransport.hpp>
#include <common/Common.hpp>
#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Message.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <common/base64.hpp>
#include <net/Socket.hpp>
#include <wsd/AIUtil.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/DocumentToolDescriptions.hpp>
#include <wsd/HostUtil.hpp>

#if !MOBILEAPP
// The COOL HTTP client stack (http::Session) is server-only; the desktop apps
// use the registered ai::HttpPostFn transport instead.
#include <net/HttpServer.hpp>
#endif

#include <Poco/File.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <exception>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <sstream>
#include <string_view>
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

const std::string TONE_NATURAL = " Respond in a natural, conversational tone - warm, human, plain.";
const std::string TONE_FORMAL =
    " Respond in a formal tone - structured, precise, avoid contractions and"
    " colloquialisms.";
const std::string TONE_SHORT =
    " Keep your response brief and to the point. Use as few words as possible"
    " while still answering the question.";
const std::string TONE_FRIENDLY =
    " Respond in a warm, friendly tone - approachable, encouraging, with"
    " personable phrasing.";
const std::string TONE_PROFESSIONAL =
    " Respond in a professional tone - clear, polished, business-appropriate,"
    " free of slang.";
const std::string TONE_CASUAL =
    " Respond in a casual, relaxed tone - conversational, easy-going, like"
    " chatting with a friend.";
const std::string EMOJIFY_PROMPT =
    " Add tasteful emoji throughout your response to reinforce mood and key"
    " points.";

/// Helper to create an OpenAI function-calling tool object.
Poco::JSON::Object::Ptr makeAITool(const std::string& name,
                                    const std::string& description,
                                    const Poco::JSON::Object::Ptr& parameters)
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

/// Compose the transform_document_structure description for the open document
/// type, advertising only the relevant grammar. Unknown type gets the full
/// grammar (previous behaviour).
std::string transformDescription(const std::string& docType)
{
    std::string desc =
        "Transform the currently-open document's structure using a JSON command "
        "sequence.\n\n";
    desc += DocumentToolDescriptions::TRANSFORM_INTRO;

    const bool isImpress = (docType == "presentation");
    const bool unknownType = docType.empty();

    if (isImpress || unknownType)
        desc += DocumentToolDescriptions::TRANSFORM_IMPRESS;

    // Writer (text/drawing), Calc, and unknown types use content controls.
    if (!isImpress)
        desc += DocumentToolDescriptions::TRANSFORM_WRITER_CALC;

    return desc;
}

/// Compose the extract_document_structure description for the open document
/// type, advertising only the filters that work for it. filter="text" is only
/// implemented for Writer and Calc, so it is omitted for Impress. Unknown type
/// gets every fragment (previous all-types behaviour).
std::string extractDescription(const std::string& docType)
{
    std::string desc = DocumentToolDescriptions::EXTRACT_INTRO;

    const bool isCalc = (docType == "spreadsheet");
    const bool isImpress = (docType == "presentation");
    const bool isWriter = (docType == "text");
    const bool unknownType = docType.empty();

    if (isWriter || unknownType)
        desc += DocumentToolDescriptions::EXTRACT_WRITER;
    if (isCalc || unknownType)
        desc += DocumentToolDescriptions::EXTRACT_CALC;
    if (isImpress || unknownType)
        desc += DocumentToolDescriptions::EXTRACT_IMPRESS;

    return desc;
}

/// Walk a link_targets subtree (as emitted by core's WriteLinkTargets) and add
/// one {label,value} entry per summarizable leaf to outChoices. A leaf is
/// summarizable when its target string ends in |outline (a heading) or
/// |region (a named section); tables, frames, bookmarks, and images are
/// skipped because the model has no way to summarize them as a slice.
void collectSectionChoices(const Poco::JSON::Object::Ptr& node, Poco::JSON::Array::Ptr& outChoices)
{
    if (!node)
        return;
    std::vector<std::string> keys;
    node->getNames(keys);
    for (const std::string& key : keys)
    {
        Poco::JSON::Object::Ptr sub = node->getObject(key);
        if (sub)
        {
            collectSectionChoices(sub, outChoices);
            continue;
        }
        std::string target;
        try
        {
            target = node->getValue<std::string>(key);
        }
        catch (const std::exception&)
        {
            continue;
        }
        if (!target.ends_with("|outline") && !target.ends_with("|region"))
            continue;
        Poco::JSON::Object::Ptr choice = new Poco::JSON::Object();
        choice->set("label", key);
        choice->set("value", target);
        outChoices->add(choice);
    }
}

namespace AIToolNames
{
constexpr std::string_view GenerateImage              = "generate_image";
constexpr std::string_view ExtractDocumentStructure   = "extract_document_structure";
constexpr std::string_view TransformDocumentStructure = "transform_document_structure";
constexpr std::string_view ExtractLinkTargets         = "extract_link_targets";
constexpr std::string_view ListCalcFunctions          = "list_calc_functions";
constexpr std::string_view EvaluateFormula            = "evaluate_formula";
constexpr std::string_view SetCellFormula             = "set_cell_formula";
}

} // anonymous namespace

using AIUtil::parseLenientArgs;

AIChatSession::AIChatSession(ClientSession& session)
    : _session(session)
{
}

AIChatSession::~AIChatSession() = default;

void AIChatSession::sendChatResult(bool success, const std::string& text,
                                   const std::string& requestId, const std::string& displayText)
{
    Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
    result->set("success", success);
    if (success)
    {
        result->set("content", text);
        if (!displayText.empty())
            result->set("displayContent", displayText);
    }
    else
        result->set("error", text);
    result->set("requestId", requestId);

    std::ostringstream oss;
    result->stringify(oss);
    _session.sendTextFrame("aichatresult: " + oss.str());
}

std::string AIChatSession::mapHttpStatusToError(
    int statusCode, const std::string& reasonPhrase,
    const std::string& context)
{
    switch (statusCode)
    {
        case 400 /* Bad Request */:
            return context.empty() ? "Invalid request"
                                   : "Invalid " + context + " request";
        case 401 /* Unauthorized */:        return "Invalid API key";
        case 403 /* Forbidden */:           return "API key lacks permissions";
        case 429 /* Too Many Requests */:   return "Rate limited - please wait a moment and retry";
        case 500 /* Internal Server Error */: return "API server error - try again later";
        case 503 /* Service Unavailable */:   return "Service temporarily unavailable";
        default:
        {
            std::string err = "API error (";
            err.append(std::to_string(statusCode));
            err.append("): ");
            err.append(reasonPhrase);
            return err;
        }
    }
}

Poco::JSON::Array::Ptr AIChatSession::buildToolDefinitions(const std::string& docType) const
{
    Poco::JSON::Array::Ptr tools = new Poco::JSON::Array();

    const bool isCalc = (docType == "spreadsheet");
    const bool unknownType = docType.empty();

    // generate_image - existing tool
    tools->add(makeAITool(
        std::string(AIToolNames::GenerateImage),
        "Generate an image based on the user's description. Call this when the "
        "user asks to create, draw, generate, sketch, or make an image or picture.",
        makeParamSchema(
            {{"prompt", {"string", "A detailed description of the image to generate"}}},
            {"prompt"})));

    // extract_document_structure - inspect the open document. The description is
    // scoped to the open document type so each only advertises filters that work.
    tools->add(makeAITool(
        std::string(AIToolNames::ExtractDocumentStructure), extractDescription(docType),
        makeParamSchema(
            { { "filter",
                { "string", "Filter results to a specific structure type. "
                            "Use 'text' to read the document body as markdown (Writer: the "
                            "full prose; Calc: the active sheet) for summarizing or answering "
                            "questions about the content. "
                            "For Impress: 'slides'. For Writer: 'contentcontrol'. "
                            "Omit to get the full structure." } },
              { "range",
                { "string", "Calc only, used with filter='text': limit reading to a cell "
                            "range like 'A1:D100'. Omit to read the active sheet's used "
                            "range." } },
              { "target",
                { "string", "Writer only, used with filter='text': read only one slice of "
                            "the document instead of the whole body. Pass a target string "
                            "from extract_link_targets, e.g. 'Introduction|outline' for a "
                            "heading's section or 'Summary|region' for a named section. "
                            "Omit to read the whole body." } } },
            {})));

    // transform_document_structure - modify the open document. The DSL is
    // scoped to the open document type so a Writer/Calc doc does not carry the
    // large Impress slide grammar, and vice versa.
    tools->add(makeAITool(
        std::string(AIToolNames::TransformDocumentStructure),
        transformDescription(docType),
        makeParamSchema(
            {{"transform", {"string", "JSON transformation commands"}},
             {"summary", {"string",
                "Markdown summary of the changes for the user to review before "
                "approving. List each slide with its title and "
                "key content points."}}},
            {"transform"})));

    // extract_link_targets - Writer/Impress navigation (not relevant to Calc)
    if (!isCalc)
        tools->add(makeAITool(
            std::string(AIToolNames::ExtractLinkTargets),
            DocumentToolDescriptions::EXTRACT_LINK_TARGETS_DESCRIPTION,
            makeParamSchema({}, {})));

    // Calc-only tools. Skip entirely for Writer/Impress; include for unknown
    // type to preserve the previous all-tools behaviour.
    if (!isCalc && !unknownType)
        return tools;

    // list_calc_functions - discover available spreadsheet functions
    tools->add(makeAITool(
        std::string(AIToolNames::ListCalcFunctions),
        "List all available spreadsheet functions in the current Calc document, "
        "grouped by category. Returns function names and signatures. "
        "Call this when you need to verify a function exists or discover "
        "the right function for a task. Only works for Calc/spreadsheet documents.",
        makeParamSchema({}, {})));

    // evaluate_formula - test a formula without inserting it
    tools->add(makeAITool(
        std::string(AIToolNames::EvaluateFormula),
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
        std::string(AIToolNames::SetCellFormula),
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

    std::string tone;
    JsonUtil::findJSONValue(requestObj, "tone", tone);
    if (tone != "natural" && tone != "formal" && tone != "short" && tone != "friendly" &&
        tone != "professional" && tone != "casual" && tone != "custom")
        tone.clear();

    std::string customToneDescription;
    JsonUtil::findJSONValue(requestObj, "customToneDescription", customToneDescription);
    if (customToneDescription.size() > 1000)
        customToneDescription.resize(1000);
    // Prevent the description from breaking out of the appended sentence by
    // inserting fake role headers or fence markers. Replace CR/LF/NUL with a
    // single space; the upstream prompt remains a one-line continuation.
    for (char& c : customToneDescription)
    {
        if (c == '\n' || c == '\r' || c == '\0')
            c = ' ';
    }

    bool emojify = false;
    JsonUtil::findJSONValue(requestObj, "emojify", emojify);

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
            "where the column letters come from the header row and the row number from the Row "
            "column."
            " The extract_document_structure result for a spreadsheet is a markdown grid whose "
            "first"
            " header row lists the spreadsheet column letters (A, B, C, ...) and whose Row column "
            "gives"
            " the spreadsheet row numbers. The data's own header labels may sit several rows down "
            "and"
            " some columns may be empty. To find which column holds a field, read across the grid's"
            " label row to match the field name to its column letter, then use that exact column "
            "letter"
            " and the data's actual first data row in your formula. Do not guess a column letter "
            "or probe"
            " cells one at a time - the column letters and row numbers are already in the grid."
            " When the user asks for a formula, use the set_cell_formula tool to insert it directly"
            " into the spreadsheet. Always use US English formula syntax with commas as argument"
            " separators (e.g., =VLOOKUP(A1,B:C,2,FALSE) not =VLOOKUP(A1;B:C;2;FALSE))."
            " Use standard Excel/Calc function names: SUM, AVERAGE, VLOOKUP, IF, COUNTIF,"
            " SUMIF, INDEX, MATCH, etc."
            " If you are unsure whether a function exists, call list_calc_functions to check."
            " If the user has selected spreadsheet data, use the cell addresses visible in that "
            "data"
            " to construct accurate cell references. If no target cell is specified by the user,"
            " choose a sensible empty cell near the data (e.g., below the last row or to the "
            "right)."
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
            " this transform call, not pre-existing slides."
            " If the user asks to rewrite, rephrase, shorten, summarise, condense,"
            " or make text more concise, and they have provided selected text, reply"
            " with the rewritten text directly in your message. Do NOT call"
            " transform_document_structure for these requests."
            " Only call transform_document_structure when the user explicitly asks to"
            " insert, add, create, replace, edit, modify, or delete slides or slide"
            " content. Never emit transform JSON, tool names, or .uno: commands in"
            " your plain-text replies.";

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

    systemPrompt += " You have tools to inspect and modify the document."
                    " Use transform_document_structure to make changes."
                    " To summarize or answer questions about the document's content, call"
                    " extract_document_structure with filter=\"text\" to read the body text"
                    " (Writer prose, or the active Calc sheet) as markdown."
                    " If a Writer whole-body read returns no text and instead carries"
                    " link_targets and an instruction, the document is too large to read"
                    " in full: show the headings and sections from link_targets to the"
                    " user and ask which one to summarize. Do not guess and do not"
                    " summarize from prior context - wait for the user's choice, then call"
                    " this tool again with filter=\"text\" and the chosen target string."
                    " If link_targets is empty, follow the instruction and ask the user to"
                    " select the relevant text in the document, then resend the request."
                    " For Calc, if the result is marked truncated, ask the user to give a"
                    " cell range via the range argument."
                    " If your earlier responses in this conversation already contain the"
                    " document content needed to answer a follow-up question, rely on them"
                    " instead of calling extract_document_structure to read the body again."
                    " Read the body again when the user asks about content your earlier"
                    " responses do not already cover, or when the user indicates they have"
                    " edited or changed the document."
                    " When you do need to read content for a question about a specific part"
                    " of a Writer document, do not read the whole body: first call"
                    " extract_link_targets to get the relevant heading or section target,"
                    " then call extract_document_structure with filter=\"text\" and that"
                    " target string in the target argument to read only that slice. Read the"
                    " whole body only for genuine whole-document tasks. For a spreadsheet,"
                    " pass the range argument to read only the relevant cells.";

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

    if (tone == "natural")
        systemPrompt += TONE_NATURAL;
    else if (tone == "formal")
        systemPrompt += TONE_FORMAL;
    else if (tone == "short")
        systemPrompt += TONE_SHORT;
    else if (tone == "friendly")
        systemPrompt += TONE_FRIENDLY;
    else if (tone == "professional")
        systemPrompt += TONE_PROFESSIONAL;
    else if (tone == "casual")
        systemPrompt += TONE_CASUAL;
    else if (tone == "custom" && !customToneDescription.empty())
        systemPrompt += " " + customToneDescription;

    if (emojify)
        systemPrompt += EMOJIFY_PROMPT;

    // Prepend system message: build a new array with system first, then the rest.
    Poco::JSON::Array::Ptr finalMessages = new Poco::JSON::Array();
    Poco::JSON::Object::Ptr systemMsg = new Poco::JSON::Object();
    systemMsg->set("role", "system");
    systemMsg->set("content", systemPrompt);
    finalMessages->add(systemMsg);
    for (unsigned i = 0; i < sanitizedMessages->size(); ++i)
        finalMessages->add(sanitizedMessages->get(i));
    sanitizedMessages = std::move(finalMessages);

    // Trim to most recent messages if over limit (keep system prompt at index 0)
    while (sanitizedMessages->size() > MAX_AI_MESSAGES + 1)
        sanitizedMessages->remove(1);

    // Get AI provider settings
    const std::string apiKey = _session.getAIProviderAPIKey();
    const std::string model = _session.getAIProviderModel();
    std::string baseUrl = _session.getAIProviderURL();

#if !MOBILEAPP
    // The desktop apps have no server-wide admin switch; AI is configured per-user
    // through the Options dialog, so this gate only applies to the WSD server.
    if (!ConfigUtil::getConfigValue<bool>("ai.enabled", false))
    {
        sendChatResult(false, "AI features are disabled by the administrator", requestId);
        return true;
    }
#endif

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
    _toolLoop->requestId = std::move(requestId);
    _toolLoop->messages = std::move(sanitizedMessages);
    _toolLoop->model = model;
    _toolLoop->requestUrl = std::move(requestUrl);
    _toolLoop->apiKey = apiKey;
    _toolLoop->docType = std::move(docType);

    callLLMAPI();
    return true;
}

#if MOBILEAPP
void AIChatSession::postViaTransport(
    const std::shared_ptr<DocumentBroker>& docBroker, const std::string& url,
    const std::string& authHeader, std::string body,
    std::function<void(int statusCode, std::string body)> onResponse)
{
    const ai::HttpPostFn& post = ai::httpPostFn();
    if (!post)
    {
        LOG_WRN("AIChat: no HTTP transport registered for the desktop app");
        onResponse(ai::HttpConnectFailed, std::string());
        return;
    }

    std::weak_ptr<DocumentBroker> docBrokerWeak = docBroker;
    post(url, authHeader, std::move(body), _session.getAIRequestTimeoutSeconds(),
         [docBrokerWeak, onResponse = std::move(onResponse)](int statusCode, std::string body)
    {
        // The transport may complete on another thread (e.g. the Qt GUI thread);
        // hop back onto the polling thread the rest of AIChatSession runs on.
        auto docBroker = docBrokerWeak.lock();
        if (!docBroker)
            return;
        auto poll = docBroker->getPoll().lock();
        if (!poll)
            return;
        poll->addCallback(
            [onResponse, statusCode, body = std::move(body)]() mutable
        {
            onResponse(statusCode, std::move(body));
        });
    });
}
#endif

void AIChatSession::callLLMAPI()
{
    if (!_toolLoop)
        return;

    Poco::URI uri(_toolLoop->requestUrl);
    if (HostUtil::isForbiddenKitHost(uri.getHost()))
    {
        LOG_WRN("Rejected AI chat request to host not in KIT allowlist ["
                << COOLWSD::anonymizeUrl(_toolLoop->requestUrl) << ']');
        sendChatResult(false, "Target host is not in the allowed host list, contact your administrator",
                       _toolLoop->requestId);
        _toolLoop.reset();
        return;
    }

    Poco::JSON::Object::Ptr payload = new Poco::JSON::Object();
    payload->set("model", _toolLoop->model);
    payload->set("messages", _toolLoop->messages);
    payload->set("tools", buildToolDefinitions(_toolLoop->docType));
    // Low temperature for deterministic, format-adherent output; explicit
    // auto so the model still chooses between a tool call and a text answer.
    payload->set("temperature", 0.1);
    payload->set("tool_choice", "auto");

    std::ostringstream payloadStream;
    payload->stringify(payloadStream);
    std::string payloadStr = payloadStream.str();

    auto clientSessionPtr = _session.client_from_this();
    AIChatSession* self = this;

    // Shared completion handler, invoked on the document broker's polling thread.
    // statusCode is an HTTP code or an ai::Http* sentinel; body is the response
    // body (empty when there was no response); reason is the HTTP reason phrase.
    auto onResponse = [clientSessionPtr, self](int statusCode, const std::string& body,
                                               const std::string& reason)
    {
        self->_activeChatSession.reset();

        if (!self->_toolLoop)
            return;

        const std::string& requestId = self->_toolLoop->requestId;

        if (statusCode == ai::HttpConnectFailed)
        {
            self->sendChatResult(
                false, "Network error - please check your connection", requestId);
            self->_toolLoop.reset();
            return;
        }

        if (statusCode == ai::HttpNoResponse)
        {
            self->sendChatResult(false, "Request timeout", requestId);
            self->_toolLoop.reset();
            return;
        }

        if (statusCode != 200)
        {
            self->sendChatResult(false, mapHttpStatusToError(statusCode, reason), requestId);
            self->_toolLoop.reset();
            return;
        }

        self->handleLLMResponse(body);
    };

    std::string authHeader = "Bearer ";
    authHeader.append(_toolLoop->apiKey);

    LOG_DBG("AIToolLoop: sending request ["
            << _toolLoop->requestId << "] round "
            << (AIToolLoopState::InitialToolRounds + 1 - _toolLoop->toolRoundsRemaining) << " to "
            << _toolLoop->requestUrl);

    std::shared_ptr<DocumentBroker> docBroker = _session.getDocumentBroker();

#if MOBILEAPP
    postViaTransport(docBroker, _toolLoop->requestUrl, authHeader, std::move(payloadStr),
                     [onResponse](int statusCode, std::string body)
                     { onResponse(statusCode, body, std::string()); });
#else
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

    httpSession->setFinishedHandler(
        [onResponse](const std::shared_ptr<http::Session>& session)
    {
        const std::shared_ptr<const http::Response> r = session->response();
        onResponse(static_cast<int>(r->statusLine().statusCode()), r->getBody(),
                   r->statusLine().reasonPhrase());
    });
    httpSession->setConnectFailHandler(
        [onResponse](const std::shared_ptr<http::Session>& /*session*/)
    {
        onResponse(ai::HttpConnectFailed, std::string(), std::string());
    });

    http::Request httpRequest(Poco::URI(_toolLoop->requestUrl).getPathAndQuery());
    httpRequest.setVerb(http::Request::VERB_POST);
    httpRequest.set("Content-Type", "application/json");
    httpRequest.set("Authorization", authHeader);
    httpRequest.setBody(std::move(payloadStr), "application/json");

    _activeChatSession = httpSession;
    httpSession->asyncRequest(httpRequest, docBroker->getPoll());
#endif
}

void AIChatSession::handleLLMResponse(const std::string& responseBody)
{
    if (!_toolLoop)
        return;

    const std::string& requestId = _toolLoop->requestId;

    LOG_DBG("AIToolLoop: raw LLM response [" << requestId
            << "] (" << responseBody.size() << " bytes): "
            << responseBody.substr(0, 2000));

    Poco::JSON::Object::Ptr responseObject = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(responseBody, responseObject))
    {
        LOG_WRN("AIToolLoop: LLM response is not valid JSON [" << requestId
                << "], bodySize=" << responseBody.size()
                << " bodyHead=" << responseBody.substr(0, 300));
        sendChatResult(false, "No response from AI", requestId);
        _toolLoop.reset();
        return;
    }

    Poco::JSON::Array::Ptr choices = responseObject->getArray("choices");
    if (!choices || choices->size() == 0)
    {
        LOG_WRN("AIToolLoop: LLM response missing or empty 'choices' [" << requestId
                << "], bodyHead=" << responseBody.substr(0, 300));
        sendChatResult(false, "No response from AI", requestId);
        _toolLoop.reset();
        return;
    }

    Poco::JSON::Object::Ptr choice = choices->getObject(0);
    if (!choice)
    {
        LOG_WRN("AIToolLoop: LLM response choices[0] is null [" << requestId << ']');
        sendChatResult(false, "No response from AI", requestId);
        _toolLoop.reset();
        return;
    }

    std::string finishReason;
    if (!choice->isNull("finish_reason"))
        JsonUtil::findJSONValue(choice, "finish_reason", finishReason);

    Poco::JSON::Object::Ptr message = choice->getObject("message");
    if (!message)
    {
        LOG_WRN("AIToolLoop: LLM response choices[0].message is null ["
                << requestId << "], finishReason='" << finishReason << "'");
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
        if (!message->isNull("content"))
            JsonUtil::findJSONValue(message, "content", assistantContent);
        if (!assistantContent.empty())
            _toolLoop->pendingSummary = std::move(assistantContent);

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

            // arguments is a JSON string per the OpenAI spec, but some models
            // emit it as an inline object - normalize both to a string.
            const Poco::Dynamic::Var argsVar = fn->get("arguments");
            if (argsVar.type() == typeid(Poco::JSON::Object::Ptr))
                pending.arguments =
                    JsonUtil::jsonToString(argsVar.extract<Poco::JSON::Object::Ptr>());
            else if (!argsVar.isEmpty())
                pending.arguments = argsVar.toString();

            _toolLoop->pendingToolCalls.push_back(std::move(pending));
        }

        // Start processing the first queued tool call
        processNextPendingToolCall();
        return;
    }

    std::string result;
    std::string reasoning;
    if (!message->isNull("content"))
        JsonUtil::findJSONValue(message, "content", result);
    if (!message->isNull("reasoning"))
        JsonUtil::findJSONValue(message, "reasoning", reasoning);

    if (result.empty())
    {
        if (!reasoning.empty())
        {
            // Reasoning models (e.g. gpt-oss) sometimes end a turn in their
            // analysis channel with no tool call and no final answer, so content
            // is empty while reasoning is full. This is an unfinished turn, not a
            // finished one: the model may still owe an action (e.g. actually
            // inserting the formula it just verified). Nudge it to continue with
            // tools still available so it can finish the work or answer. Only if
            // it stalls again do we surface the reasoning, and even then we do
            // not claim the task succeeded.
            if (_toolLoop->reasoningOnlyRetriesRemaining > 0)
            {
                --_toolLoop->reasoningOnlyRetriesRemaining;
                Poco::JSON::Object::Ptr nudge = new Poco::JSON::Object();
                nudge->set("role", "user");
                nudge->set("content",
                           "You have not replied yet. If the task still requires changes "
                           "to the document, call the appropriate tool now to make them. "
                           "If everything is already done, reply with your final answer in "
                           "plain text. Do not respond with only internal reasoning, and "
                           "do not claim a change was made unless you actually called the "
                           "tool to make it.");
                _toolLoop->messages->add(nudge);
                LOG_DBG("AIToolLoop: reasoning-only turn [" << requestId
                                                            << "], nudging the model to continue");
                callLLMAPI();
                return;
            }

            LOG_WRN("AIToolLoop: model still produced only reasoning after a nudge ["
                    << requestId << "], surfacing reasoning as the answer");
            sendChatResult(true, reasoning, requestId);
        }
        else if (finishReason == "length")
        {
            sendChatResult(false,
                "The model ran out of tokens before producing output. Try a "
                "shorter input or a model with a larger output budget.", requestId);
        }
        else if (finishReason.empty())
        {
            // Zero-token blank: provider returned a well-formed envelope
            // with all fields null and no completion. Usually transient.
            LOG_WRN("AIToolLoop: provider returned zero-token blank ["
                    << requestId << "], bodySize=" << responseBody.size());
            sendChatResult(false,
                "The model returned an empty response (no tokens generated). "
                "This is usually a temporary provider issue — please retry, "
                "or try a different model.", requestId);
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
    if (fnName == AIToolNames::GenerateImage)
    {
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        std::string imagePrompt;
        if (parseLenientArgs(argsJson, argsObj))
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
    if (fnName == AIToolNames::ExtractDocumentStructure)
    {
        std::string filter, range, target;
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        if (parseLenientArgs(argsJson, argsObj))
        {
            JsonUtil::findJSONValue(argsObj, "filter", filter);
            JsonUtil::findJSONValue(argsObj, "range", range);
            JsonUtil::findJSONValue(argsObj, "target", target);
        }

        const bool bTextFilter = filter.starts_with("text");

        // A Calc range or a Writer scope target is carried as a sub-arg of the
        // text filter. The target may contain spaces and the '|' separator, so
        // percent-encode it: the command is space-delimited and the value is
        // later split as a URL query, then decoded, on the core side.
        if (bTextFilter && !range.empty())
            filter += ",range:" + range;
        if (bTextFilter && !target.empty())
            filter += ",target:" + Uri::encode(target, "%|,/?:@&=+$#");

        std::string command = "extractdocumentstructure url=interactive";
        if (!filter.empty())
            command += " filter=" + filter;

        _toolLoop->awaitingApproval = true;
        _toolLoop->pendingToolCallId = toolCallId;
        _toolLoop->pendingToolName = fnName;
        _toolLoop->pendingForwardCommand = std::move(command);

        // The approval copy is read from pendingSummary by the sidebar. Reading
        // the body text sends document content to the external model, so make
        // the consent explicit; structural inspection keeps the default copy. A
        // scoped read names the slice so the user knows only part is sent.
        if (!bTextFilter)
            _toolLoop->pendingSummary = "";
        else if (!target.empty())
        {
            std::string name = target;
            if (const auto bar = name.rfind('|'); bar != std::string::npos)
                name = name.substr(0, bar);
            _toolLoop->pendingSummary =
                "Read the \"" + name + "\" section of your document to answer your request.";
        }
        else if (!range.empty())
            _toolLoop->pendingSummary =
                "Read the \"" + range + "\" range of your document to answer your request.";
        else
            _toolLoop->pendingSummary =
                "Read the full text of your document to answer your request.";

        sendToolApproval(fnName, "");
        return true;
    }

    // extract_link_targets - read-only, send to kit
    if (fnName == AIToolNames::ExtractLinkTargets)
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
    if (fnName == AIToolNames::EvaluateFormula)
    {
        std::string cell, formula;
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        if (parseLenientArgs(argsJson, argsObj))
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
    if (fnName == AIToolNames::ListCalcFunctions)
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
    if (fnName == AIToolNames::SetCellFormula)
    {
        std::string cell, formula, formulasJson, summary;
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        if (parseLenientArgs(argsJson, argsObj))
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
            catch (const std::exception& e)
            {
                LOG_DBG("set_cell_formula: invalid 'formulas' JSON: " << e.what()
                        << ", payload head: " << formulasJson.substr(0, 200));
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
        _toolLoop->pendingSummary = std::move(summary);

        // Store the pairs array for execution after approval.
        std::ostringstream storedJson;
        pairs->stringify(storedJson);
        _toolLoop->pendingTransformArgs = storedJson.str();

        sendToolApproval(fnName, "");
        return true;
    }

    // transform_document_structure - requires user approval
    if (fnName == AIToolNames::TransformDocumentStructure)
    {
        std::string transform;
        std::string summary;
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        if (parseLenientArgs(argsJson, argsObj))
        {
            JsonUtil::findJSONValue(argsObj, "transform", transform);

            // Lenient: when the model skips the {"transform": "..."}
            // wrapper and puts {"Transforms": {...}} at the top level,
            // treat the whole args object as the transform.(eg. for small models)
            if (transform.empty() && argsObj->has("Transforms"))
            {
                JsonUtil::findJSONValue(argsObj, "Transforms", transform);
            }

            JsonUtil::findJSONValue(argsObj, "summary", summary);
            if (summary.empty() && argsObj->has("Summary"))
                JsonUtil::findJSONValue(argsObj, "Summary", summary);
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
            transform = std::move(sanitized);
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

        // Structural validation before bothering the user with an approval
        // dialog. On failure, feed a precise error back so the model can
        // self-correct silently, drawing from a budget separate from the
        // multi-step tool-round budget.
        if (auto structErr = AIUtil::validateTransformStructure(transformObj))
        {
            if (_toolLoop->validationRetriesRemaining > 0)
            {
                --_toolLoop->validationRetriesRemaining;
                Poco::JSON::Object::Ptr err = new Poco::JSON::Object();
                err->set("error", *structErr);
                continueToolLoop(toolCallId, JsonUtil::jsonToString(err));
                return true;
            }
            // Budget exhausted: fall through to approval and let the user decide.
            LOG_WRN("AIToolLoop: transform still structurally invalid after retries ["
                    << requestId << "]: " << *structErr);
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
        _toolLoop->pendingSummary = std::move(summary);

        sendToolApproval(fnName, transform);
        return true;
    }

    // Unknown tool - feed error back to LLM
    LOG_WRN("AIToolLoop: model called unknown tool '" << fnName << "' ["
            << requestId << ']');
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

bool AIChatSession::tryShortCircuitBigDocumentRead(const std::string& payloadJson)
{
    if (!_toolLoop)
        return false;

    Poco::JSON::Object::Ptr root;
    if (!JsonUtil::parseJSON(payloadJson, root))
        return false;

    // The kit's extracteddocumentstructure payload wraps everything in a
    // DocStructure root; for filter=text it puts a BodyText node underneath
    // that, with a truncated flag both Writer and Calc emit.
    Poco::JSON::Object::Ptr docStructure = root->getObject("DocStructure");
    if (!docStructure)
        return false;

    Poco::JSON::Object::Ptr body = docStructure->getObject("BodyText");
    if (!body)
        return false;

    bool isTruncated = false;
    if (!JsonUtil::findJSONValue(body, "truncated", isTruncated) || !isTruncated)
        return false;

    // Writer renders clickable section picks from the link_targets the kit
    // inlines on a truncated whole-body read. Calc has no equivalent
    // structure, so it nudges the user toward the two existing paths to
    // narrow the read. Other doc types fall through unchanged.
    if (_toolLoop->docType == "text")
    {
        Poco::JSON::Object::Ptr linkTargets = body->getObject("link_targets");
        if (!linkTargets)
            return false;

        Poco::JSON::Array::Ptr choices = new Poco::JSON::Array();
        collectSectionChoices(linkTargets, choices);
        if (choices->size() == 0)
            return false;

        // Send the choices so the frontend can render the picks as inline
        // clickable text in the upcoming assistant message.
        {
            Poco::JSON::Object::Ptr msg = new Poco::JSON::Object();
            msg->set("requestId", _toolLoop->requestId);
            msg->set("context", "writer-section");
            msg->set("choices", choices);

            std::ostringstream oss;
            msg->stringify(oss);
            _session.sendTextFrame("aichatchoices: " + oss.str());
        }

        // Compose two parallel views of the synthetic reply: a markdown
        // list for the user (rendered as <li> items that the sidebar
        // decorator turns into clickable links), and a hidden instruction
        // for the model that lists the canonical target strings so it can
        // call extract_document_structure correctly when the user picks.
        std::ostringstream displayMd;
        displayMd << "This document is too large to read in full. Pick a section "
                     "to focus on:\n\n";
        std::ostringstream modelTxt;
        modelTxt << "The document is too large to read in full. The user is "
                    "picking which section to scope the read by. Available "
                    "section target strings: ";
        for (unsigned i = 0; i < choices->size(); ++i)
        {
            Poco::JSON::Object::Ptr c = choices->getObject(i);
            if (!c)
                continue;
            std::string label, value;
            JsonUtil::findJSONValue(c, "label", label);
            JsonUtil::findJSONValue(c, "value", value);
            displayMd << "- " << label << "\n";
            if (i > 0)
                modelTxt << ", ";
            modelTxt << value;
        }
        modelTxt << ". When the user picks one, call extract_document_structure "
                    "with that exact target string as the target argument. Then "
                    "answer the user's earlier request using only the content of "
                    "the chosen section.";

        sendChatResult(true, modelTxt.str(), _toolLoop->requestId, displayMd.str());
        _toolLoop.reset();
        return true;
    }

    if (_toolLoop->docType == "spreadsheet")
    {
        // Calc has no clickable picks. Point the user at the two existing
        // narrowing paths: a multi-cell selection is inlined into the next
        // message by buildUserMessage with no further tool call, or a typed
        // range argument lets the model call this tool again with range=.
        const std::string displayMd =
            "This sheet is too large to read in full. To narrow it down, you can:\n\n"
            "- Select a range of cells in the sheet, then ask your question again\n"
            "- Reply with a range to focus on, like `A1:D100`";
        const std::string modelTxt =
            "The sheet is too large to read in full. The user is choosing how to "
            "narrow the read. They will either select a range and resend their "
            "question (in which case the selection content is attached inline as "
            "[Selected text from document: ...] and you can answer directly without "
            "another tool call), or reply with a range like 'A1:D100' (in which "
            "case call extract_document_structure with filter='text' and "
            "range='<their range>'). Do not call extract_document_structure with "
            "no range argument again on this document.";

        sendChatResult(true, modelTxt, _toolLoop->requestId, displayMd);
        _toolLoop.reset();
        return true;
    }

    return false;
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
        else if (_toolLoop->pendingToolName == AIToolNames::SetCellFormula)
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
            catch (const std::exception& e)
            {
                LOG_WRN("handleApprove: failed to parse stored formulas (internal bug): "
                        << e.what());
                continueToolLoop(toolCallId, "{\"error\":\"Internal error parsing stored formulas\"}");
                return true;
            }

            sendToolProgress(std::string(AIToolNames::SetCellFormula), "Setting formulas...");

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
        if (_toolLoop->pendingToolName == AIToolNames::ExtractDocumentStructure)
            rejectionMsg =
                "{\"error\":\"User declined document inspection. "
                "Answer their request directly without inspecting the document. "
                "If the request is to create new content, just generate it.\"}";
        else if (_toolLoop->pendingToolName == AIToolNames::SetCellFormula)
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

#if !MOBILEAPP
    if (_activeChatSession)
    {
        _activeChatSession->asyncShutdown();
        _activeChatSession.reset();
    }
#endif

    // Dropping the tool-loop state makes any in-flight response a no-op: the
    // transport callbacks bail out once _toolLoop is null.
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

    Poco::URI uri(req.requestUrl);
    if (HostUtil::isForbiddenKitHost(uri.getHost()))
    {
        req.error = "Target host is not in the allowed host list, contact your administrator";
        return req;
    }

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
    payload->set("model", imageModel);

    LOG_DBG("AIImageGeneration: model=" << imageModel << ", size=" << imageSize);

    std::ostringstream payloadStream;
    payload->stringify(payloadStream);
    req.payloadStr = payloadStream.str();

#if !MOBILEAPP
    req.httpSession = http::Session::create(req.requestUrl);
    if (!req.httpSession)
    {
        req.error = "Failed to create HTTP session";
        return req;
    }

    req.httpSession->setTimeout(std::chrono::seconds(_session.getAIRequestTimeoutSeconds()));
#endif
    return req;
}

std::pair<std::string, std::string> AIChatSession::parseImageGenResponse(
    int statusCode, const std::string& body)
{
    if (statusCode == ai::HttpConnectFailed)
        return {"", "Network error - please check your connection"};

    if (statusCode == ai::HttpNoResponse)
        return {"", "Request timeout"};

    if (statusCode != 200)
    {
        LOG_WRN_S("AIImageGeneration: HTTP " << statusCode << ": " << body);

        std::string errorMsg = "HTTP " + std::to_string(statusCode);
        Poco::JSON::Object::Ptr errObj;
        if (JsonUtil::parseJSON(body, errObj))
        {
            Poco::JSON::Object::Ptr errorDetail = errObj->getObject("error");
            if (errorDetail)
            {
                std::string message;
                JsonUtil::findJSONValue(errorDetail, "message", message);
                if (!message.empty())
                    errorMsg = std::move(message);
            }
        }
        return {"", errorMsg};
    }

    Poco::JSON::Object::Ptr responseObject = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(body, responseObject))
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

    // Shared completion handler, invoked on the document broker's polling thread.
    auto onResponse = [self, sendImageResult](int statusCode, const std::string& body)
    {
        self->_activeChatSession.reset();

        auto [b64Json, error] = parseImageGenResponse(statusCode, body);
        if (!error.empty())
        {
            sendImageResult(false, "", error);
            return;
        }

        sendImageResult(true, b64Json, "");
    };

    LOG_DBG("AIImageGeneration: sending request [" << requestId << "] to "
            << req.requestUrl);

    std::shared_ptr<DocumentBroker> docBroker = _session.getDocumentBroker();

#if MOBILEAPP
    postViaTransport(docBroker, req.requestUrl, "Bearer " + req.apiKey,
                     req.payloadStr, onResponse);
#else
    req.httpSession->setFinishedHandler(
        [onResponse](const std::shared_ptr<http::Session>& session)
    {
        const std::shared_ptr<const http::Response> r = session->response();
        onResponse(static_cast<int>(r->statusLine().statusCode()), r->getBody());
    });
    req.httpSession->setConnectFailHandler(
        [onResponse](const std::shared_ptr<http::Session>& /*session*/)
    {
        onResponse(ai::HttpConnectFailed, std::string());
    });

    http::Request httpRequest(Poco::URI(req.requestUrl).getPathAndQuery());
    httpRequest.setVerb(http::Request::VERB_POST);
    httpRequest.set("Content-Type", "application/json");
    httpRequest.set("Authorization", "Bearer " + req.apiKey);
    httpRequest.setBody(req.payloadStr, "application/json");

    _activeChatSession = req.httpSession;
    req.httpSession->asyncRequest(httpRequest, docBroker->getPoll());
#endif
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
                        {currentSlide, objId, std::move(prompt), std::string()});

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

void AIChatSession::generateNextTransformImage(const std::shared_ptr<DocumentBroker>& docBroker)
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

        // Shared completion handler, invoked on the document broker's polling thread.
        auto onResponse =
            [clientSessionPtr, self, docBroker, idx, onImageFail](int statusCode,
                                                                  const std::string& body)
        {
            self->_activeChatSession.reset();

            if (!self->_toolLoop)
                return;

            auto [b64Json, error] = parseImageGenResponse(statusCode, body);
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

        LOG_DBG("TransformImageGen: generating image " << (idx + 1) << " of " << total
                                                       << ", prompt: " << gen.prompt);

#if MOBILEAPP
        postViaTransport(docBroker, req.requestUrl, "Bearer " + req.apiKey,
                         req.payloadStr, onResponse);
#else
        req.httpSession->setFinishedHandler(
            [onResponse](const std::shared_ptr<http::Session>& session)
        {
            const std::shared_ptr<const http::Response> r = session->response();
            onResponse(static_cast<int>(r->statusLine().statusCode()), r->getBody());
        });
        req.httpSession->setConnectFailHandler(
            [onResponse](const std::shared_ptr<http::Session>& /*session*/)
        {
            onResponse(ai::HttpConnectFailed, std::string());
        });

        http::Request httpRequest(Poco::URI(req.requestUrl).getPathAndQuery());
        httpRequest.setVerb(http::Request::VERB_POST);
        httpRequest.set("Content-Type", "application/json");
        httpRequest.set("Authorization", "Bearer " + req.apiKey);
        httpRequest.setBody(req.payloadStr, "application/json");

        _activeChatSession = req.httpSession;
        req.httpSession->asyncRequest(httpRequest, docBroker->getPoll());
#endif
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
    if (_toolLoop->pendingToolName != AIToolNames::ListCalcFunctions
        && _toolLoop->pendingToolName != AIToolNames::EvaluateFormula)
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
    const std::string payloadJson = payload->jsonString();
    // On the big-document truncation branch we already have everything
    // we need to ask the user which section to read; skip a wasted LLM
    // round-trip that would just re-emit the heading list as prose.
    if (tryShortCircuitBigDocumentRead(payloadJson))
        return true;
    continueToolLoop(_toolLoop->pendingToolCallId, payloadJson);
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
                generateNextTransformImage(std::move(broker));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
