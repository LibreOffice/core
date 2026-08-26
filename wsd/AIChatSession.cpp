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
#include <wsd/DeckSpec.hpp>
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
#include <cctype>
#include <cstddef>
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

/// The largest client message the AI chat parses, 5MB.
constexpr std::size_t MAX_AI_PAYLOAD_SIZE = 5 * 1024 * 1024;

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

bool isInsufficientQuotaError(const std::string& body)
{
    Poco::JSON::Object::Ptr root;
    if (!JsonUtil::parseJSON(body, root) || !root)
        return false;
    const Poco::JSON::Object::Ptr err = root->getObject("error");
    if (!err)
        return false;
    std::string code;
    JsonUtil::findJSONValue(err, "code", code);
    if (code == "insufficient_quota")
        return true;
    std::string type;
    JsonUtil::findJSONValue(err, "type", type);
    return type == "insufficient_quota";
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

// The image style appended to a generated-image prompt when the picked template
// declares no art direction, so every deck image still gets a cohesive look.
const std::string NEUTRAL_ART_DIRECTION =
    "Clean, minimal presentation illustration with a cohesive colour palette, no text in the "
    "image";

/// The tone sentence to append to a system prompt for the picked tone, or the
/// empty string when no tone applies. A "custom" tone uses the user's own
/// description as the sentence.
std::string toneSentence(const std::string& tone, const std::string& customToneDescription)
{
    if (tone == "natural")
        return TONE_NATURAL;
    if (tone == "formal")
        return TONE_FORMAL;
    if (tone == "short")
        return TONE_SHORT;
    if (tone == "friendly")
        return TONE_FRIENDLY;
    if (tone == "professional")
        return TONE_PROFESSIONAL;
    if (tone == "casual")
        return TONE_CASUAL;
    if (tone == "custom" && !customToneDescription.empty())
        return " " + customToneDescription;
    return std::string();
}

/// Append a note listing the image prompts that failed to render to a result
/// string. Returns the result unchanged when no prompts failed.
std::string appendImageFailureNote(const std::string& result,
                                   const std::vector<std::string>& prompts)
{
    if (prompts.empty())
        return result;

    std::string augmented = result;
    augmented += "\n\nNote: ";
    augmented += std::to_string(prompts.size());
    augmented += " image(s) failed to generate and still show a loading placeholder."
                 " Failed prompts: ";
    for (std::size_t i = 0; i < prompts.size(); ++i)
    {
        if (i > 0)
            augmented += ", ";
        augmented += "\"";
        augmented += prompts[i];
        augmented += "\"";
    }
    return augmented;
}

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
    {
        desc += DocumentToolDescriptions::TRANSFORM_IMPRESS_INTRO;
        desc += AIUtil::getSlideCommandDocs();
        desc += DocumentToolDescriptions::TRANSFORM_IMPRESS_DETAILS;
    }

    // Writer (text/drawing) and unknown types use content controls.
    if (!isImpress)
        desc += DocumentToolDescriptions::TRANSFORM_WRITER;

    return desc;
}

/// Compose the extract_document_structure description for the open document
/// type, advertising only the filters that work for it. Unknown type gets
/// every fragment (previous all-types behaviour).
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
constexpr std::string_view ProposeOutline             = "propose_outline";
constexpr std::string_view WriteSlide                 = "write_slide";
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
    const std::string& body, const std::string& context)
{
    switch (statusCode)
    {
        case 400 /* Bad Request */:
            return context.empty() ? "Invalid request"
                                   : "Invalid " + context + " request";
        case 401 /* Unauthorized */:        return "Invalid API key";
        case 403 /* Forbidden */:           return "API key lacks permissions";
        case 429 /* Too Many Requests */:
            return isInsufficientQuotaError(body)
                       ? "API quota exceeded - check your plan and billing details"
                       : "Rate limited - please wait a moment and retry";
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
                            "full prose; Calc: the active sheet; Impress: the text of every "
                            "slide) for summarizing or answering "
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

    // transform_document_structure - modify the open document. Not offered for
    // Calc: a spreadsheet has no handler for it, so a call would silently do
    // nothing. The DSL is scoped to the open document type so a Writer doc does
    // not carry the large Impress slide grammar, and vice versa.
    if (!isCalc)
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

    // The deck tool descriptions carry the budget limits, so compose them at
    // call time from the request's live budgets. buildToolDefinitions runs with
    // the tool loop live; the default budgets are a safe fallback otherwise.
    const DeckSpec::Budgets budgets = _toolLoop ? _toolLoop->budgets : DeckSpec::Budgets{};

    // propose_outline - outline-first deck creation. The model sketches an
    // outline the user reviews and edits before the server builds the slides
    // from it.
    if (docType == "presentation")
        tools->add(makeAITool(
            std::string(AIToolNames::ProposeOutline),
            std::string(DocumentToolDescriptions::PROPOSE_OUTLINE_HEAD) + "Give at most " +
                std::to_string(budgets.maxSlides) +
                DocumentToolDescriptions::PROPOSE_OUTLINE_TAIL_OPEN_CLOSE,
            makeParamSchema(
                {{"outline", {"object",
                    "The outline: an object with a \"title\" and a \"slides\" array."}}},
                {"outline"})));

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
        "grouped by category. Returns US English function names and signatures, "
        "whatever the user's language is. "
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

namespace
{
// A template or master name reaches the system prompt, so the naming contract
// is deliberately narrow: at most 64 bytes, each an ASCII letter, digit,
// space, hyphen, or underscore. Every place a design name crosses the process
// boundary checks this same contract, so a change here is a change of the
// wire contract, not of one check. The checks use explicit ASCII ranges
// rather than std::isalnum, which follows the current locale and could admit
// non-ASCII letters.
bool isSafeDesignName(const std::string& rName)
{
    if (rName.empty() || rName.size() > 64)
        return false;
    for (const char c : rName)
    {
        const bool bAllowed = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                              || (c >= '0' && c <= '9') || c == ' ' || c == '-' || c == '_';
        if (!bAllowed)
            return false;
    }
    return true;
}

// Read the deck-spec slide cap from configuration. It defaults to the built-in
// DeckSpec limit and is clamped to at least 1 so a misconfigured zero or
// negative cannot make every deck fail validation. The other limits are the
// built-in DeckSpec constants. A template manifest may lower these further;
// that tightening is applied by the caller.
DeckSpec::Budgets budgetsFromConfig()
{
    DeckSpec::Budgets budgets;
    const int maxSlides =
        ConfigUtil::getConfigValue<int>("ai.deck_budgets.max_slides", budgets.maxSlides);
    budgets.maxSlides = maxSlides < 1 ? 1 : maxSlides;
    return budgets;
}

// The art direction comes from a template file and only ever composes an image
// prompt, so it must not smuggle control characters or unbounded length into it.
// Turn every control character into a space, collapse runs of whitespace to one
// space, trim the ends, and cap the result at 300 bytes, rounded up to the end
// of the character in progress so no multi-byte character is cut in half.
std::string sanitizeArtDirection(const std::string& raw)
{
    std::string out;
    out.reserve(std::min<size_t>(raw.size(), 300));
    bool previousSpace = false;
    for (const char c : raw)
    {
        const unsigned char uc = static_cast<unsigned char>(c);
        // The cap stops the copy on a character boundary: once it is reached,
        // the continuation bytes of the character in progress still land, and
        // the copy ends before the next character starts. The second bound
        // holds even when malformed input never starts a new character.
        if ((out.size() >= 300 && (uc & 0xC0) != 0x80) || out.size() >= 304)
            break;
        const char ch = (uc < 0x20 || uc == 0x7F) ? ' ' : c;
        if (ch == ' ')
        {
            if (out.empty() || previousSpace)
                continue;
            previousSpace = true;
        }
        else
            previousSpace = false;
        out += ch;
    }
    while (!out.empty() && out.back() == ' ')
        out.pop_back();
    return out;
}
}

bool AIChatSession::handleAction(const std::string& firstLine)
{
    static constexpr size_t MAX_AI_MESSAGE_LENGTH = 100 * 1024; // 100KB per message

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

    PendingChatRequest req;
    JsonUtil::findJSONValue(requestObj, "requestId", req.requestId);

    // A design fetch from an earlier request may still be outstanding. That
    // request never reached the model, so report it as failed to its client
    // before this new request takes over the single pending-fetch slot.
    if (_pendingDesignFetch)
    {
        sendChatResult(false, "Request superseded by a newer request",
                       _pendingDesignFetch->requestId);
        _pendingDesignFetch.reset();
    }

    JsonUtil::findJSONValue(requestObj, "docType", req.docType);
    if (req.docType != "text" && req.docType != "spreadsheet" && req.docType != "presentation"
        && req.docType != "drawing")
        req.docType.clear();

    JsonUtil::findJSONValue(requestObj, "tone", req.tone);
    if (req.tone != "natural" && req.tone != "formal" && req.tone != "short"
        && req.tone != "friendly" && req.tone != "professional" && req.tone != "casual"
        && req.tone != "custom")
        req.tone.clear();

    JsonUtil::findJSONValue(requestObj, "customToneDescription", req.customToneDescription);
    if (req.customToneDescription.size() > 1000)
        req.customToneDescription.resize(1000);
    // Prevent the description from breaking out of the appended sentence by
    // inserting fake role headers or fence markers. Replace CR/LF/NUL with a
    // single space; the upstream prompt remains a one-line continuation.
    for (char& c : req.customToneDescription)
    {
        if (c == '\n' || c == '\r' || c == '\0')
            c = ' ';
    }

    JsonUtil::findJSONValue(requestObj, "emojify", req.emojify);

    // The design template the user picked in the in-chat picker, sent only for a
    // presentation. The engine rejects a name it cannot resolve. The masters and
    // layouts the template provides are not taken from the request: wsd fetches
    // them from the kit (see the branch below), so the prompt is built from the
    // engine's own data rather than anything the client supplied.
    if (req.docType == "presentation")
    {
        JsonUtil::findJSONValue(requestObj, "designTemplate", req.designTemplate);
        if (!isSafeDesignName(req.designTemplate))
            req.designTemplate.clear();
    }

    Poco::JSON::Array::Ptr messages = requestObj->getArray("messages");
    if (!messages || messages->size() == 0)
    {
        sendChatResult(false, "No messages provided", req.requestId);
        return true;
    }

    Poco::JSON::Array::Ptr sanitizedMessages = new Poco::JSON::Array();
    for (std::size_t i = 0; i < messages->size(); ++i)
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
            sendChatResult(false, "Message too long", req.requestId);
            return true;
        }

        sanitizedMessages->add(msg);
    }
    req.messages = std::move(sanitizedMessages);

    // Check whether the last user message includes selected text from the document.
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
                req.hasSelectedText = true;
            break;
        }
    }

    // Get AI provider settings
    const std::string apiKey = _session.getAIProviderAPIKey();
    const std::string model = _session.getAIProviderModel();
    std::string baseUrl = _session.getAIProviderURL();

#if !MOBILEAPP
    // The desktop apps have no server-wide admin switch; AI is configured per-user
    // through the Options dialog, so this gate only applies to the WSD server.
    if (!ConfigUtil::getConfigValue<bool>("ai.enabled", false))
    {
        sendChatResult(false, "AI features are disabled by the administrator", req.requestId);
        return true;
    }

    // AI is refused for an anonymous user (for example a public share-link
    // visitor), so a server-wide provider is not spent on them.
    if (_session.isAnonymousUser())
    {
        sendChatResult(false, "AI is not available for guests", req.requestId);
        return true;
    }
#endif

    if (_session.isDisableAISettings())
    {
        sendChatResult(false, "AI features are disabled for this document", req.requestId);
        return true;
    }

    // The API key is optional (self-hosted endpoints often need none); a model
    // and a base URL are the minimum needed to reach the provider.
    if (model.empty() || baseUrl.empty())
    {
        sendChatResult(false, "AI settings not configured", req.requestId);
        return true;
    }

    req.model = model;
    req.apiKey = apiKey;
    req.requestUrl = AIUtil::normalizeAIBaseUrl(baseUrl);
    req.requestUrl.append("/v1/chat/completions");

    // A presentation themed with a picked template needs that template's design
    // masters and layouts in the prompt. Fetch them from the kit, which knows the
    // template's real masters, instead of trusting the client: stash the request,
    // ask the kit, and launch once the reply arrives in tryConsumeDesignFetch.
    // Every other request launches straight away with no designs.
    if (req.docType == "presentation" && !req.designTemplate.empty())
    {
        std::shared_ptr<DocumentBroker> docBroker = _session.getDocumentBroker();
        if (docBroker)
        {
            std::string encodedName;
            Poco::URI::encode(req.designTemplate, "", encodedName);
            req.fetchCommand = ".uno:GetDesignTemplateDesigns?name=" + encodedName;
            _pendingDesignFetch = std::make_unique<PendingChatRequest>(std::move(req));
            // Bound the wait for the kit. The fetch is a local query that
            // normally returns in well under a second; if the reply never
            // arrives the request would otherwise stall until the browser gives
            // up. A short cap leaves the rest of the AI request budget for the
            // model call, and a poll-thread check gives up on the fetch once
            // this deadline passes.
            _designFetchDeadline
                = std::chrono::steady_clock::now() + std::chrono::seconds(15);
            docBroker->forwardToChild(_session.client_from_this(),
                "commandvalues command=" + _pendingDesignFetch->fetchCommand);
            return true;
        }
        // No document broker to ask: fall through and launch without designs.
    }

    launchChatRequest(req, {});
    return true;
}

void AIChatSession::launchChatRequest(const PendingChatRequest& req, const DesignInfo& design)
{
    static constexpr unsigned MAX_AI_MESSAGES = 50;

    // A new request supersedes any deck expansion still parked from an earlier
    // one, and any approved build still waiting for its design fetch; drop
    // both so a late kit reply for them is ignored.
    _deckExpansion.reset();
    _pendingApprovedBuild.reset();

    const std::string& docType = req.docType;
    const std::string& tone = req.tone;
    const std::string& customToneDescription = req.customToneDescription;
    const bool emojify = req.emojify;
    const bool hasSelectedText = req.hasSelectedText;

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
            " Keep the function names in English even when you are writing to the user in"
            " another language, and never translate them yourself. The spreadsheet stores"
            " the formula and shows it back in the user's own language on its own."
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
            " and evaluate again before inserting."
            " You can set cell contents and formulas with set_cell_formula, but you cannot"
            " change cell formatting (bold, italic, colors, number formats, borders) yet."
            " If the user asks to format a cell, say briefly that formatting is not supported"
            " yet and offer to change the cell content or a formula instead.";

    if (docType == "presentation")
    {
        systemPrompt +=
            " To build a new deck of any size, first call propose_outline with"
            " one entry per slide and stop; the user reviews and edits the"
            " outline, and the slides are built from it after approval. Use"
            " transform_document_structure only to edit or rearrange slides that"
            " already exist, not to create new ones."
            " Choose an intent that"
            " fits each slide and vary it across the deck. Do not prefix list items"
            " with '- ' (bullet markers are added automatically) and put only the"
            " items themselves in each block."
            " Put spoken detail in each slide's notes and keep the slide text short"
            " and scannable."
            " Include a 'summary' parameter with a"
            " short markdown preview of the slides being created."
            " If the user asks to rewrite, rephrase, shorten, summarise, condense, or"
            " make text more concise, and they have provided selected text, reply with"
            " the rewritten text directly in your message. Do NOT call a tool for"
            " these requests. Never emit transform JSON, tool names, or .uno: commands"
            " in your plain-text replies.";
    }

    systemPrompt += " You have tools to inspect and modify the document."
                    " Use transform_document_structure to make changes."
                    " To summarize or answer questions about the document's content, call"
                    " extract_document_structure with filter=\"text\" to read the body text"
                    " (Writer prose, the active Calc sheet, or the text of every slide) as"
                    " markdown."
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

    systemPrompt += toneSentence(tone, customToneDescription);

    if (emojify)
        systemPrompt += EMOJIFY_PROMPT;

    // Prepend system message: build a new array with system first, then the rest.
    Poco::JSON::Array::Ptr finalMessages = new Poco::JSON::Array();
    Poco::JSON::Object::Ptr systemMsg = new Poco::JSON::Object();
    systemMsg->set("role", "system");
    systemMsg->set("content", systemPrompt);
    finalMessages->add(systemMsg);
    for (std::size_t i = 0; i < req.messages->size(); ++i)
        finalMessages->add(req.messages->get(i));

    // Trim to most recent messages if over limit (keep system prompt at index 0)
    while (finalMessages->size() > MAX_AI_MESSAGES + 1)
        finalMessages->remove(1);

    LOG_DBG("AIChatAction: request [" << req.requestId << "] with "
            << finalMessages->size() << " messages, model: " << req.model);

    // Initialize the tool loop state
    _toolLoop = std::make_unique<AIToolLoopState>();
    _toolLoop->requestId = req.requestId;
    _toolLoop->messages = std::move(finalMessages);
    _toolLoop->model = req.model;
    _toolLoop->requestUrl = req.requestUrl;
    _toolLoop->apiKey = req.apiKey;
    _toolLoop->docType = req.docType;
    _toolLoop->designTemplate = req.designTemplate;
    _toolLoop->tone = req.tone;
    _toolLoop->customToneDescription = req.customToneDescription;

    applyDesignToToolLoop(design);

    callLLMAPI();
}

void AIChatSession::applyDesignToToolLoop(const DesignInfo& design)
{
    // The configured budgets are the ceiling; a template manifest may only lower
    // a limit, never raise it, and a value below one is ignored.
    DeckSpec::Budgets budgets = budgetsFromConfig();
    auto tighten = [](int& limit, const std::optional<int>& manifest)
    {
        if (manifest && *manifest >= 1)
            limit = std::min(limit, *manifest);
    };
    tighten(budgets.maxSlides, design.maxSlides);
    tighten(budgets.maxItemsPerBullets, design.maxItemsPerBullets);
    tighten(budgets.maxItemLength, design.maxItemLength);
    tighten(budgets.maxTitleLength, design.maxTitleLength);
    _toolLoop->budgets = budgets;

    // Every generated image gets a cohesive style: the template's art direction
    // when it declares one, otherwise a neutral default.
    _toolLoop->artDirection =
        design.artDirection.empty() ? NEUTRAL_ART_DIRECTION : design.artDirection;
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

namespace
{
// True when the provider rejected the request specifically for the
// "temperature" parameter (e.g. reasoning models that only allow the default).
bool isUnsupportedTemperatureError(const std::string& body)
{
    Poco::JSON::Object::Ptr root;
    if (!JsonUtil::parseJSON(body, root) || !root)
        return false;
    const Poco::JSON::Object::Ptr err = root->getObject("error");
    if (!err)
        return false;
    std::string param;
    JsonUtil::findJSONValue(err, "param", param);
    return param == "temperature";
}
} // namespace

void AIChatSession::callLLMAPI()
{
    if (!_toolLoop)
        return;

#if !MOBILEAPP
    // A built-in provider's host is a fixed public endpoint and is always
    // allowed; only a custom host goes through the net.lok_allow allowlist.
    Poco::URI uri(_toolLoop->requestUrl);
    if (!AIUtil::isPreCannedAIProviderHost(uri.getHost()) &&
        HostUtil::isForbiddenKitHost(uri.getHost()))
    {
        LOG_WRN("Rejected AI chat request to host not in KIT allowlist ["
                << Anonymizer::anonymizeUrl(_toolLoop->requestUrl) << ']');
        sendChatResult(false, "Target host is not in the allowed host list, contact your administrator",
                       _toolLoop->requestId);
        _toolLoop.reset();
        return;
    }
#endif

    Poco::JSON::Object::Ptr payload = new Poco::JSON::Object();
    payload->set("model", _toolLoop->model);
    payload->set("messages", _toolLoop->messages);
    payload->set("tools", buildToolDefinitions(_toolLoop->docType));
    // Low temperature for deterministic, format-adherent output; explicit
    // auto so the model still chooses between a tool call and a text answer.
    // Some reasoning models only accept the default temperature and 400 on any
    // explicit value, so omit it after such a rejection (see onResponse).
    if (!_toolLoop->retriedWithoutTemperature)
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
    auto onResponse = [clientSessionPtr = std::move(clientSessionPtr), self](
        int statusCode, const std::string& body, const std::string& reason)
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

        if (statusCode == 400 && !self->_toolLoop->retriedWithoutTemperature &&
            isUnsupportedTemperatureError(body))
        {
            LOG_WRN("AIChat: model rejected 'temperature'; retrying without it ["
                    << requestId << ']');
            self->_toolLoop->retriedWithoutTemperature = true;
            self->callLLMAPI();
            return;
        }

        if (statusCode != 200)
        {
            LOG_WRN("AIChat: provider returned HTTP " << statusCode
                    << " for request [" << requestId << "]; body: " << body);
            self->sendChatResult(false, mapHttpStatusToError(statusCode, reason, body), requestId);
            self->_toolLoop.reset();
            return;
        }

        self->handleLLMResponse(body);
    };

    postChatCompletion(std::move(payloadStr), onResponse);
}

void AIChatSession::postChatCompletion(
    std::string payloadStr,
    std::function<void(int statusCode, const std::string& body,
                       const std::string& reason)> onResponse)
{
    if (!_toolLoop)
        return;

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
        [onResponse = std::move(onResponse)](const std::shared_ptr<http::Session>& /*session*/)
    {
        onResponse(ai::HttpConnectFailed, std::string(), std::string());
    });

    http::Request httpRequest(Poco::URI(_toolLoop->requestUrl).getPathAndQuery());
    httpRequest.setVerb(http::Request::VERB_POST);
    httpRequest.set("Content-Type", "application/json");
    httpRequest.set("Authorization", std::move(authHeader));
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
        for (std::size_t i = 0; i < toolCalls->size(); ++i)
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

void AIChatSession::spliceSlideCommands(const Poco::JSON::Object::Ptr& transformObj)
{
    // Server-only commands are the server's to add, never the model's: a
    // model-emitted ApplyTemplate would override the user's design pick, or
    // theme a deck the user chose to keep plain, because the engine takes the
    // last ApplyTemplate in the array. Drop any server-only command, then
    // prepend the user's pick so the engine maps the slides this transform
    // produces onto the template's masters. Re-applying it on a later transform
    // is harmless - the engine reuses the master copy already in the document.
    Poco::JSON::Object::Ptr transforms = transformObj->getObject("Transforms");
    Poco::JSON::Array::Ptr cmds =
        transforms ? transforms->getArray("SlideCommands") : nullptr;
    if (!cmds)
        return;

    Poco::JSON::Array::Ptr newCmds = new Poco::JSON::Array();

    if (!_toolLoop->designTemplate.empty())
    {
        Poco::JSON::Object::Ptr applyCmd = new Poco::JSON::Object();
        applyCmd->set("ApplyTemplate", _toolLoop->designTemplate);
        newCmds->add(applyCmd);
    }

    for (std::size_t i = 0; i < cmds->size(); ++i)
    {
        Poco::JSON::Object::Ptr cmd = cmds->getObject(i);
        if (cmd)
        {
            std::vector<std::string> keys;
            cmd->getNames(keys);
            std::vector<std::string> serverOnlyKeys;
            for (const std::string& key : keys)
            {
                if (AIUtil::isServerOnlySlideCommand(key))
                    serverOnlyKeys.push_back(key);
            }
            if (!serverOnlyKeys.empty())
            {
                LOG_WRN("AIToolLoop: dropping model-emitted server-only command '"
                        << serverOnlyKeys.front() << "' [" << _toolLoop->requestId << ']');
                for (const std::string& key : serverOnlyKeys)
                    cmd->remove(key);
                if (serverOnlyKeys.size() == keys.size())
                    continue;
            }
        }
        newCmds->add(cmds->get(i));
    }

    transforms->set("SlideCommands", newCmds);
}

void AIChatSession::sendTransformForApproval(const std::string& toolCallId,
                                             const std::string& fnName,
                                             const Poco::JSON::Object::Ptr& transformObj,
                                             std::string summary)
{
    spliceSlideCommands(transformObj);

    std::ostringstream oss;
    transformObj->stringify(oss);
    const std::string transform = oss.str();

    _toolLoop->awaitingApproval = true;
    _toolLoop->pendingToolCallId = toolCallId;
    _toolLoop->pendingToolName = fnName;
    _toolLoop->pendingTransformArgs = transform;
    _toolLoop->pendingSummary = std::move(summary);

    sendToolApproval(fnName, transform);
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
            std::string name = std::move(target);
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
            "commandvalues command=.uno:CalcFunctionList?english=true");
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

            for (std::size_t i = 0; i < parsed->size(); ++i)
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
            for (std::size_t i = 0; i < pairs->size(); ++i)
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

    // propose_outline - the model sketches an outline for a new deck. The server
    // validates it, stores it, and sends it to the browser for the user to edit
    // and approve; the deck is built slide by slide only after approval.
    if (fnName == AIToolNames::ProposeOutline)
    {
        Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
        Poco::JSON::Object::Ptr outlineObj;
        if (parseLenientArgs(argsJson, argsObj))
        {
            // Accept the outline nested, as a JSON string, or as the whole
            // argument object when the model skips the "outline" wrapper.
            if (argsObj->isObject("outline"))
                outlineObj = argsObj->getObject("outline");
            else
            {
                std::string outlineStr;
                JsonUtil::findJSONValue(argsObj, "outline", outlineStr);
                if (!outlineStr.empty())
                    JsonUtil::parseJSON(outlineStr, outlineObj);
            }
            if (!outlineObj && argsObj->has("slides"))
                outlineObj = std::move(argsObj);
        }

        if (!outlineObj)
        {
            continueToolLoop(toolCallId, "{\"error\":\"No outline parameter provided\"}");
            return true;
        }

        // On a schema failure feed the precise error back so the model can
        // self-correct silently, drawing from the same retry budget the other
        // tools use.
        if (auto outlineErr = DeckSpec::validateOutline(outlineObj, _toolLoop->budgets))
        {
            if (_toolLoop->validationRetriesRemaining > 0)
                --_toolLoop->validationRetriesRemaining;
            else
                LOG_WRN("AIToolLoop: outline still invalid after retries [" << requestId
                        << "]: " << *outlineErr);
            Poco::JSON::Object::Ptr err = new Poco::JSON::Object();
            err->set("error", *outlineErr);
            continueToolLoop(toolCallId, JsonUtil::jsonToString(err));
            return true;
        }

        // Store the outline and wait for the user to review it. The outline card
        // the browser draws is deterministic from the outline, so no model
        // summary is needed.
        std::ostringstream stored;
        outlineObj->stringify(stored);
        _toolLoop->awaitingApproval = true;
        _toolLoop->pendingToolCallId = toolCallId;
        _toolLoop->pendingToolName = std::string(AIToolNames::ProposeOutline);
        _toolLoop->pendingTransformArgs = stored.str();

        std::string title;
        JsonUtil::findJSONValue(outlineObj, "title", title);
        Poco::JSON::Object::Ptr outlineMsg = new Poco::JSON::Object();
        outlineMsg->set("requestId", _toolLoop->requestId);
        outlineMsg->set("title", title);
        outlineMsg->set("slides", outlineObj->getArray("slides"));
        std::ostringstream frame;
        outlineMsg->stringify(frame);
        _session.sendTextFrame("aichatoutline: " + frame.str());
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
                for (std::size_t i = 0; i < cmds->size(); ++i)
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

        sendTransformForApproval(toolCallId, fnName, transformObj, std::move(summary));
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

    // While a deck expansion is running the tool loop is parked: a kit reply for
    // an applied slide (or an error, or an image-patch completion) funnels here.
    // Route it to the expansion driver and do not append a tool message - the
    // model conversation is not advancing during the build.
    if (_deckExpansion)
    {
        onExpansionSlideApplied(result);
        return;
    }

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
        for (std::size_t i = 0; i < choices->size(); ++i)
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

    if (jsonPayload.size() > MAX_AI_PAYLOAD_SIZE)
    {
        LOG_WRN("AIChatApprove: payload too large: " << jsonPayload.size());
        return true;
    }

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

    // A proposed outline is answered here too, but it does not flow through the
    // generic transform dispatch: an approval starts a server-driven slide-by-slide
    // build, a rejection just nudges the model to propose a different outline.
    if (_toolLoop->pendingToolName == AIToolNames::ProposeOutline)
    {
        if (action == "approve")
        {
            // The browser can rename or remove slides before approving, so the
            // returned outline is untrusted and re-validated. Fall back to the
            // stored outline when the browser sends none.
            Poco::JSON::Object::Ptr outline = obj->getObject("outline");
            if (!outline)
                JsonUtil::parseJSON(_toolLoop->pendingTransformArgs, outline);

            if (auto outlineErr = DeckSpec::validateOutline(outline, _toolLoop->budgets))
            {
                sendChatResult(false, "The edited outline is not valid: " + *outlineErr,
                               _toolLoop->requestId);
                _toolLoop.reset();
                return true;
            }

            // The whole outline goes into the system prompt of every per-slide
            // request, so from here on it is the stripped copy: the validated
            // fields and nothing else.
            outline = DeckSpec::sanitizeOutline(outline);

            _toolLoop->awaitingApproval = false;
            if (beginDesignFetchForApproval(obj, outline))
                return true;
            startDeckExpansion(outline);
        }
        else
        {
            _toolLoop->awaitingApproval = false;
            continueToolLoop(toolCallId,
                "{\"error\":\"User rejected the proposed outline. Ask what they would "
                "like changed and propose a new outline.\"}");
        }
        return true;
    }

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

            // Dispatch SetCellFormula for each pair
            Poco::JSON::Array resultArr;
            std::string lastCell;
            for (std::size_t i = 0; i < pairs->size(); ++i)
            {
                auto p = pairs->getObject(i);
                if (!p) continue;
                std::string cell, formula;
                JsonUtil::findJSONValue(p, "cell", cell);
                JsonUtil::findJSONValue(p, "formula", formula);

                std::string escapedCell = JsonUtil::escapeJSONValue(cell);
                std::string escapedFormula = JsonUtil::escapeJSONValue(formula);

                std::string setArgs = "{\"Cell\":{\"type\":\"string\",\"value\":\""
                    + escapedCell + "\"},\"Formula\":{\"type\":\"string\",\"value\":\""
                    + escapedFormula + "\"}}";
                docBroker->forwardToChild(_session.client_from_this(),
                    "uno .uno:SetCellFormula " + setArgs);

                Poco::JSON::Object::Ptr r = new Poco::JSON::Object();
                r->set("cell", cell);
                r->set("formula", formula);
                resultArr.add(r);

                lastCell = std::move(escapedCell);
            }

            // Leave the cursor on the last cell written, so the user sees the result.
            if (!lastCell.empty())
            {
                std::string goToArgs = "{\"ToPoint\":{\"type\":\"string\",\"value\":\""
                    + lastCell + "\"}}";
                docBroker->forwardToChild(_session.client_from_this(),
                    "uno .uno:GoToCell " + goToArgs);
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

bool AIChatSession::beginDesignFetchForApproval(const Poco::JSON::Object::Ptr& approveObj,
                                                const Poco::JSON::Object::Ptr& outline)
{
    // The pick comes from the browser, so it is held to the same naming
    // contract as a request-time pick; a name that fails it is treated as no
    // pick. A pick recorded earlier in the conversation stays authoritative.
    std::string picked;
    JsonUtil::findJSONValue(approveObj, "designTemplate", picked);
    if (picked.empty() || !_toolLoop->designTemplate.empty()
        || _toolLoop->docType != "presentation" || !isSafeDesignName(picked))
        return false;

    std::shared_ptr<DocumentBroker> docBroker = _session.getDocumentBroker();
    if (!docBroker)
        return false;

    _toolLoop->designTemplate = picked;

    std::string encodedName;
    Poco::URI::encode(picked, "", encodedName);
    _pendingApprovedBuild = std::make_unique<PendingApprovedBuild>();
    _pendingApprovedBuild->outline = outline;
    _pendingApprovedBuild->fetchCommand = ".uno:GetDesignTemplateDesigns?name=" + encodedName;
    // The same bounded wait as a request-time design fetch: the kit answers a
    // local query in well under a second, and the deadline keeps a lost reply
    // from stalling the approved build.
    _designFetchDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
    docBroker->forwardToChild(_session.client_from_this(),
                              "commandvalues command=" + _pendingApprovedBuild->fetchCommand);
    return true;
}

void AIChatSession::continueApprovedBuild(const DesignInfo& design)
{
    const std::unique_ptr<PendingApprovedBuild> build = std::move(_pendingApprovedBuild);
    if (!build || !_toolLoop)
        return;

    applyDesignToToolLoop(design);
    startDeckExpansion(build->outline);
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
    // transport callbacks bail out once _toolLoop is null. A deck expansion, if
    // one is running, shares the same transport, and an approved build waiting
    // for its design fetch belongs to the same request, so drop them too.
    _deckExpansion.reset();
    _pendingApprovedBuild.reset();
    _toolLoop.reset();

    return true;
}

void AIChatSession::startDeckExpansion(const Poco::JSON::Object::Ptr& outline)
{
    if (!_toolLoop || !outline)
        return;

    _deckExpansion = std::make_unique<DeckExpansionState>();
    JsonUtil::findJSONValue(outline, "title", _deckExpansion->outlineTitle);
    _deckExpansion->slides = outline->getArray("slides");

    std::ostringstream oss;
    outline->stringify(oss);
    _deckExpansion->outlineJson = oss.str();

    expandNextSlide();
}

void AIChatSession::expandNextSlide()
{
    if (!_deckExpansion || !_toolLoop)
        return;

    const unsigned total = _deckExpansion->slides ? _deckExpansion->slides->size() : 0;
    if (_deckExpansion->nextIndex >= total)
    {
        finishDeckExpansion();
        return;
    }

    const unsigned slideNumber = _deckExpansion->nextIndex + 1;
    sendToolProgress(std::string(AIToolNames::ProposeOutline),
                     "Building slide " + std::to_string(slideNumber) + " of " +
                         std::to_string(total) + "...");

    Poco::JSON::Object::Ptr entry = _deckExpansion->slides->getObject(_deckExpansion->nextIndex);

    // The system prompt fixes the model's job to writing this one slide's
    // content, and carries the whole approved outline for context.
    std::string systemPrompt =
        "You write one slide of a presentation deck whose outline the user has"
        " approved. Reply by calling write_slide exactly once, never with plain"
        " text. The approved outline, in order, is: " +
        _deckExpansion->outlineJson + ".";
    if (_toolLoop->designTemplate.empty())
        systemPrompt += " There is no design template, so the deck uses the default look.";
    else
        systemPrompt += " The design template '" + _toolLoop->designTemplate +
                        "' styles the slides, so write the content only and leave the look"
                        " to it.";
    systemPrompt += toneSentence(_toolLoop->tone, _toolLoop->customToneDescription);
    systemPrompt +=
        " Write content that fits this slide's place in the deck and does not repeat"
        " other slides. Do not prefix items with \"- \"."
        " Put spoken detail in each slide's notes and keep the slide text short and"
        " scannable.";

    const std::string userMessage = DeckSpec::buildExpansionUserMessage(
        entry, slideNumber, total,
        _deckExpansion->retriedCurrentSlide ? _deckExpansion->lastSlideError : std::string());

    Poco::JSON::Array::Ptr messages = new Poco::JSON::Array();
    Poco::JSON::Object::Ptr systemMsg = new Poco::JSON::Object();
    systemMsg->set("role", "system");
    systemMsg->set("content", systemPrompt);
    messages->add(systemMsg);
    Poco::JSON::Object::Ptr userMsg = new Poco::JSON::Object();
    userMsg->set("role", "user");
    userMsg->set("content", userMessage);
    messages->add(userMsg);

    // The one tool the model may call for a slide, with a forced choice so it
    // must call it rather than reply with prose.
    Poco::JSON::Array::Ptr tools = new Poco::JSON::Array();
    tools->add(makeAITool(
        std::string(AIToolNames::WriteSlide),
        std::string("Write one slide of the deck. Pass a \"slide\" object.\n\n") +
            DocumentToolDescriptions::DECK_SLIDE_SHAPE +
            DeckSpec::limitsSentence(_toolLoop->budgets),
        makeParamSchema({ { "slide", { "object", "The slide description." } } }, { "slide" })));

    Poco::JSON::Object::Ptr payload = new Poco::JSON::Object();
    payload->set("model", _toolLoop->model);
    payload->set("messages", messages);
    payload->set("tools", tools);
    if (!_toolLoop->retriedWithoutTemperature)
        payload->set("temperature", 0.1);
    Poco::JSON::Object::Ptr toolChoice = new Poco::JSON::Object();
    toolChoice->set("type", "function");
    Poco::JSON::Object::Ptr toolChoiceFn = new Poco::JSON::Object();
    toolChoiceFn->set("name", std::string(AIToolNames::WriteSlide));
    toolChoice->set("function", toolChoiceFn);
    payload->set("tool_choice", toolChoice);

    std::ostringstream payloadStream;
    payload->stringify(payloadStream);

    AIChatSession* self = this;
    postChatCompletion(payloadStream.str(),
                       [self](int statusCode, const std::string& body, const std::string& reason)
    {
        self->_activeChatSession.reset();
        if (!self->_deckExpansion || !self->_toolLoop)
            return;
        self->handleExpansionResponse(statusCode, body, reason);
    });
}

void AIChatSession::handleExpansionResponse(int statusCode, const std::string& body,
                                            const std::string& reason)
{
    if (!_deckExpansion || !_toolLoop)
        return;

    if (statusCode == ai::HttpConnectFailed)
    {
        failCurrentExpansionSlide("a network error reaching the model");
        return;
    }
    if (statusCode == ai::HttpNoResponse)
    {
        failCurrentExpansionSlide("the model request timed out");
        return;
    }
    if (statusCode == 400 && !_toolLoop->retriedWithoutTemperature &&
        isUnsupportedTemperatureError(body))
    {
        // A reasoning model that rejects an explicit temperature: re-issue the
        // same slide with it omitted. This is not one of the slide's own retries.
        _toolLoop->retriedWithoutTemperature = true;
        expandNextSlide();
        return;
    }
    if (statusCode != 200)
    {
        failCurrentExpansionSlide(mapHttpStatusToError(statusCode, reason));
        return;
    }

    Poco::JSON::Object::Ptr root;
    if (!JsonUtil::parseJSON(body, root) || !root)
    {
        failCurrentExpansionSlide("the model returned no usable response");
        return;
    }

    Poco::JSON::Array::Ptr choices = root->getArray("choices");
    Poco::JSON::Object::Ptr choice =
        (choices && choices->size() > 0) ? choices->getObject(0) : nullptr;
    Poco::JSON::Object::Ptr message = choice ? choice->getObject("message") : nullptr;
    Poco::JSON::Array::Ptr toolCalls = message ? message->getArray("tool_calls") : nullptr;
    Poco::JSON::Object::Ptr call =
        (toolCalls && toolCalls->size() > 0) ? toolCalls->getObject(0) : nullptr;
    Poco::JSON::Object::Ptr fn = call ? call->getObject("function") : nullptr;
    if (!fn)
    {
        failCurrentExpansionSlide("the model did not call write_slide");
        return;
    }

    std::string arguments;
    const Poco::Dynamic::Var argsVar = fn->get("arguments");
    if (argsVar.type() == typeid(Poco::JSON::Object::Ptr))
        arguments = JsonUtil::jsonToString(argsVar.extract<Poco::JSON::Object::Ptr>());
    else if (!argsVar.isEmpty())
        arguments = argsVar.toString();

    // Extract the slide leniently: a nested "slide" object, a JSON string, or the
    // whole argument object when the model skips the wrapper.
    Poco::JSON::Object::Ptr argsObj = new Poco::JSON::Object();
    Poco::JSON::Object::Ptr slide;
    if (parseLenientArgs(arguments, argsObj))
    {
        if (argsObj->isObject("slide"))
            slide = argsObj->getObject("slide");
        else
        {
            std::string slideStr;
            JsonUtil::findJSONValue(argsObj, "slide", slideStr);
            if (!slideStr.empty())
                JsonUtil::parseJSON(slideStr, slide);
        }
        if (!slide && (argsObj->has("intent") || argsObj->has("blocks")))
            slide = std::move(argsObj);
    }
    if (!slide)
    {
        failCurrentExpansionSlide("the model's write_slide call carried no slide object");
        return;
    }

    // Kill drift: the part, intent and title come from the approved outline; the
    // model only contributes the content blocks and any image. Overwrite them
    // before validating so a wandering model cannot reshape the deck.
    if (Poco::JSON::Object::Ptr entry =
            _deckExpansion->slides->getObject(_deckExpansion->nextIndex))
    {
        std::string value;
        JsonUtil::findJSONValue(entry, "part", value);
        slide->set("part", value);
        value.clear();
        JsonUtil::findJSONValue(entry, "intent", value);
        slide->set("intent", value);
        value.clear();
        JsonUtil::findJSONValue(entry, "title", value);
        slide->set("title", value);
    }

    if (auto slideErr =
            DeckSpec::validateSlideSpec(slide, _deckExpansion->nextIndex, _toolLoop->budgets))
    {
        failCurrentExpansionSlide(*slideErr);
        return;
    }

    applyExpansionSlide(slide);
}

void AIChatSession::applyExpansionSlide(const Poco::JSON::Object::Ptr& slide)
{
    if (!_deckExpansion || !_toolLoop)
        return;

    std::shared_ptr<DocumentBroker> docBroker = _session.getDocumentBroker();
    if (!docBroker)
    {
        failCurrentExpansionSlide("the document is not available");
        return;
    }

    // docSlideIndex is the number of slides already built, not the outline
    // index: a skipped slide leaves the next one reusing the deck's current
    // slide instead of inserting after a slide that was never added.
    const DeckSpec::CompileOptions options{ !_toolLoop->designTemplate.empty(),
                                            _toolLoop->artDirection };
    const std::string transform =
        DeckSpec::compileSlideSpec(slide, _deckExpansion->builtCount, options);

    Poco::JSON::Object::Ptr transformObj = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(transform, transformObj))
    {
        failCurrentExpansionSlide("the slide could not be compiled");
        return;
    }

    // The design template splice rides every per-slide transform so the engine
    // maps the newly added slide onto the template's master.
    spliceSlideCommands(transformObj);

    std::ostringstream oss;
    transformObj->stringify(oss);
    _toolLoop->pendingTransformArgs = oss.str();
    _toolLoop->pendingToolName = std::string(AIToolNames::TransformDocumentStructure);

    // Forward through the image-aware transform path: it rewrites any
    // GenerateImage into a loading placeholder, applies the transform, then fills
    // in the image. When it finishes it calls continueToolLoop, which routes back
    // to onExpansionSlideApplied because a deck expansion is running.
    processTransformImageGenerations(docBroker, std::max(_deckExpansion->builtCount, 1));
}

void AIChatSession::onExpansionSlideApplied(const std::string& result)
{
    if (!_deckExpansion || !_toolLoop)
        return;

    // Carry any image briefs that failed to render into the deck-wide tally.
    for (const std::string& prompt : _toolLoop->failedImagePrompts)
        _deckExpansion->failedImagePrompts.push_back(prompt);
    _toolLoop->failedImagePrompts.clear();

    bool applied = false;
    Poco::JSON::Object::Ptr resultObj;
    if (JsonUtil::parseJSON(result, resultObj) && resultObj)
        JsonUtil::findJSONValue(resultObj, "success", applied);

    if (applied)
        ++_deckExpansion->builtCount;
    else
    {
        // The content passed validation; the kit could not apply it. A model
        // retry would not change that, so record the slide as skipped.
        _deckExpansion->skippedSlides.push_back(
            static_cast<int>(_deckExpansion->nextIndex) + 1);
        LOG_WRN("AIChat: deck expansion could not apply slide "
                << (_deckExpansion->nextIndex + 1) << ": " << result);
    }

    ++_deckExpansion->nextIndex;
    _deckExpansion->retriedCurrentSlide = false;
    _deckExpansion->lastSlideError.clear();
    expandNextSlide();
}

void AIChatSession::failCurrentExpansionSlide(const std::string& reason)
{
    if (!_deckExpansion || !_toolLoop)
        return;

    const unsigned slideNumber = _deckExpansion->nextIndex + 1;

    if (!_deckExpansion->retriedCurrentSlide)
    {
        // First failure: try the same slide once more, telling the model what
        // went wrong.
        _deckExpansion->retriedCurrentSlide = true;
        _deckExpansion->lastSlideError = reason;
        LOG_WRN("AIChat: deck expansion slide " << slideNumber << " failed, retrying: "
                                                << reason);
        expandNextSlide();
        return;
    }

    // Second failure: give up on this slide and move on to the next.
    LOG_WRN("AIChat: deck expansion slide " << slideNumber << " failed again, skipping: "
                                            << reason);
    _deckExpansion->skippedSlides.push_back(static_cast<int>(slideNumber));
    sendToolProgress(std::string(AIToolNames::ProposeOutline),
                     "Skipping slide " + std::to_string(slideNumber) + "...");
    ++_deckExpansion->nextIndex;
    _deckExpansion->retriedCurrentSlide = false;
    _deckExpansion->lastSlideError.clear();
    expandNextSlide();
}

void AIChatSession::finishDeckExpansion()
{
    if (!_deckExpansion || !_toolLoop)
        return;

    const std::string requestId = _toolLoop->requestId;

    if (_deckExpansion->builtCount == 0)
    {
        sendChatResult(false, "Could not build any slides from the outline. Please try again.",
                       requestId);
        _deckExpansion.reset();
        _toolLoop.reset();
        return;
    }

    const auto isSkipped = [this](unsigned position)
    {
        return std::find(_deckExpansion->skippedSlides.begin(),
                         _deckExpansion->skippedSlides.end(), static_cast<int>(position)) !=
               _deckExpansion->skippedSlides.end();
    };

    // Model-facing manifest: a numbered list of the built slides plus a note for
    // each skipped one. This is what the next turn's conversation carries, so a
    // follow-up like "make slide 3 punchier" has something to refer to.
    std::ostringstream content;
    content << "Built the deck '" << _deckExpansion->outlineTitle
            << "' from the approved outline with " << _deckExpansion->builtCount << " slides:";
    const unsigned total = _deckExpansion->slides ? _deckExpansion->slides->size() : 0;
    int built = 0;
    std::ostringstream skips;
    for (unsigned i = 0; i < total; ++i)
    {
        Poco::JSON::Object::Ptr entry = _deckExpansion->slides->getObject(i);
        std::string part, intent, title;
        if (entry)
        {
            JsonUtil::findJSONValue(entry, "part", part);
            JsonUtil::findJSONValue(entry, "intent", intent);
            JsonUtil::findJSONValue(entry, "title", title);
        }
        if (isSkipped(i + 1))
        {
            skips << "\nSlide " << (i + 1) << " '" << title
                  << "' was skipped because its content could not be generated.";
            continue;
        }
        ++built;
        content << "\n" << built << ". " << title << " (" << part << ", " << intent << ")";
    }
    content << skips.str();
    const std::string modelContent =
        appendImageFailureNote(content.str(), _deckExpansion->failedImagePrompts);

    // User-facing message: a short ready line, plus brief notes for anything that
    // did not come out.
    std::ostringstream display;
    display << "Your deck is ready: " << _deckExpansion->builtCount
            << " slides built from the approved outline.";
    if (!_deckExpansion->skippedSlides.empty())
        display << " " << _deckExpansion->skippedSlides.size()
                << " slide(s) could not be built and were skipped.";
    if (!_deckExpansion->failedImagePrompts.empty())
        display << " " << _deckExpansion->failedImagePrompts.size()
                << " image(s) could not be generated and show a placeholder.";

    sendChatResult(true, modelContent, requestId, display.str());
    _deckExpansion.reset();
    _toolLoop.reset();
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

    req.requestUrl = AIUtil::normalizeAIBaseUrl(baseUrl) + "/v1/images/generations";

#if !MOBILEAPP
    // A built-in provider's host is a fixed public endpoint and is always
    // allowed; only a custom host goes through the net.lok_allow allowlist.
    Poco::URI uri(req.requestUrl);
    if (!AIUtil::isPreCannedAIProviderHost(uri.getHost()) &&
        HostUtil::isForbiddenKitHost(uri.getHost()))
    {
        req.error = "Target host is not in the allowed host list, contact your administrator";
        return req;
    }
#endif

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
    auto sendImageResult = [clientSession = std::move(clientSessionPtr), requestId](
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
    auto onResponse = [self, sendImageResult = std::move(sendImageResult)](int statusCode,
                                                                           const std::string& body)
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
        [onResponse = std::move(onResponse)](const std::shared_ptr<http::Session>& /*session*/)
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
    const std::shared_ptr<DocumentBroker>& docBroker, int nExistingSlides)
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

    _toolLoop->pendingImageGens.clear();
    _toolLoop->nextImageGenIndex = 0;
    _toolLoop->generatingImages = false;
    _toolLoop->outstandingImageTransforms = 0;
    _toolLoop->mainTransformResult.clear();
    _toolLoop->failedImagePrompts.clear();

    // Rewrite each GenerateImage command into a loading placeholder and collect
    // its target slide. The scan starts from the slides already in the document
    // so an image on a freshly appended slide is recorded against that slide.
    const std::string placeholderUrl = "file://" + std::string(JAILED_DOCUMENT_ROOT)
                                        + "insertfile/ai-loading-placeholder.png";
    for (const DeckSpec::ImageInsertion& img :
         DeckSpec::rewriteGenerateImageCommands(transformObj, nExistingSlides, placeholderUrl))
    {
        _toolLoop->pendingImageGens.push_back(
            { img.slideIndex, img.objId, img.prompt, std::string(), img.alt });
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
    if (!_toolLoop)
        return result;
    return appendImageFailureNote(result, _toolLoop->failedImagePrompts);
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
            [clientSessionPtr = std::move(clientSessionPtr), self, docBroker, idx,
             onImageFail = std::move(onImageFail)](int statusCode, const std::string& body)
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
            // image on the correct slide. When the image carries alt text the
            // command value is an object {"url","alt"}; otherwise it stays the
            // plain URL string the engine has always accepted.
            const auto& imgGen = self->_toolLoop->pendingImageGens[idx];
            const std::string insertKey = "InsertImageAt." +
                                          std::to_string(imgGen.slideIndex) + "." +
                                          std::to_string(imgGen.objId);
            Poco::JSON::Object::Ptr insertCmd = new Poco::JSON::Object();
            if (imgGen.alt.empty())
                insertCmd->set(insertKey, jailFileUrl);
            else
            {
                Poco::JSON::Object::Ptr target = new Poco::JSON::Object();
                target->set("url", jailFileUrl);
                target->set("alt", imgGen.alt);
                insertCmd->set(insertKey, target);
            }
            Poco::JSON::Array::Ptr insertCmds = new Poco::JSON::Array();
            insertCmds->add(insertCmd);
            Poco::JSON::Object::Ptr insertTransforms = new Poco::JSON::Object();
            insertTransforms->set("SlideCommands", insertCmds);
            Poco::JSON::Object::Ptr insertRoot = new Poco::JSON::Object();
            insertRoot->set("Transforms", insertTransforms);
            std::ostringstream miniStream;
            insertRoot->stringify(miniStream);
            const std::string miniTransform = miniStream.str();

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
            [onResponse = std::move(onResponse)](const std::shared_ptr<http::Session>& /*session*/)
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

namespace
{
// The values of a GetDesignTemplateDesigns reply: the template's distinct
// example-slide layouts and its design masters, each with the part it plays,
// plus the template manifest's art direction and budget limits when it
// declares them. We keep the layouts, the distinct parts the masters cover,
// and the manifest values; the master names stay inside the engine.
DesignInfo parseDesignFetchValues(const Poco::JSON::Object::Ptr& values)
{
    DesignInfo design;
    if (values)
    {
        Poco::JSON::Array::Ptr layouts = values->getArray("layouts");
        const unsigned nLayouts = layouts ? std::min<unsigned>(layouts->size(), 100) : 0;
        for (unsigned i = 0; i < nLayouts; ++i)
        {
            const Poco::Dynamic::Var var = layouts->get(i);
            if (!var.isString())
                continue;
            const std::string layout = var.toString();
            const bool seen = std::find(design.layouts.begin(), design.layouts.end(), layout)
                              != design.layouts.end();
            if (AIUtil::isKnownSlideLayout(layout) && !seen)
                design.layouts.push_back(layout);
        }

        // The parts this template has a design for: the distinct master roles,
        // leaving out "other" (a utility master that plays no part). These parts
        // are offered to the model; the master names stay inside the engine.
        Poco::JSON::Array::Ptr masters = values->getArray("masters");
        const unsigned nMasters = masters ? std::min<unsigned>(masters->size(), 50) : 0;
        for (unsigned i = 0; i < nMasters; ++i)
        {
            Poco::JSON::Object::Ptr master = masters->getObject(i);
            if (!master)
                continue;
            const std::string role = JsonUtil::getJSONValue<std::string>(master, "role");
            if (role.empty() || role == "other")
                continue;
            const bool seen = std::find(design.parts.begin(), design.parts.end(), role)
                              != design.parts.end();
            if (!seen)
                design.parts.push_back(role);
        }

        // The manifest's image style. It comes from a user-editable template
        // file and only ever composes image prompts, so sanitize it here at the
        // trust boundary.
        std::string artDirection;
        JsonUtil::findJSONValue(values, "artDirection", artDirection);
        design.artDirection = sanitizeArtDirection(artDirection);

        // The manifest's budget limits. Only positive integers are kept; the
        // launch applies them tighten-only against the configured budgets.
        if (Poco::JSON::Object::Ptr budgets = values->getObject("budgets"))
        {
            auto readLimit = [&budgets](const std::string& key) -> std::optional<int>
            {
                int value = 0;
                if (JsonUtil::findJSONValue(budgets, key, value) && value >= 1)
                    return value;
                return std::nullopt;
            };
            design.maxSlides = readLimit("maxSlides");
            design.maxItemsPerBullets = readLimit("maxItemsPerBullets");
            design.maxItemLength = readLimit("maxItemLength");
            design.maxTitleLength = readLimit("maxTitleLength");
        }
    }
    return design;
}
}

bool AIChatSession::tryConsumeDesignFetch(const std::shared_ptr<Message>& payload)
{
    if (!_pendingDesignFetch && !_pendingApprovedBuild)
        return false;

    Poco::JSON::Object::Ptr reply = new Poco::JSON::Object();
    if (!JsonUtil::parseJSON(payload->jsonString(), reply))
        return false;

    // Only the reply to the exact GetDesignTemplateDesigns query we sent is
    // ours; the picker's template-list reply and any other commandvalues fall
    // through.
    std::string commandName;
    JsonUtil::findJSONValue(reply, "commandName", commandName);

    if (_pendingDesignFetch && commandName == _pendingDesignFetch->fetchCommand)
    {
        const DesignInfo design = parseDesignFetchValues(reply->getObject("commandValues"));
        const PendingChatRequest req = std::move(*_pendingDesignFetch);
        _pendingDesignFetch.reset();
        launchChatRequest(req, design);
        return true;
    }

    if (_pendingApprovedBuild && commandName == _pendingApprovedBuild->fetchCommand)
    {
        continueApprovedBuild(parseDesignFetchValues(reply->getObject("commandValues")));
        return true;
    }

    return false;
}

void AIChatSession::checkDesignFetchTimeout(std::chrono::steady_clock::time_point now)
{
    if ((!_pendingDesignFetch && !_pendingApprovedBuild) || now < _designFetchDeadline)
        return;

    LOG_WRN("AIChat: the design template fetch was not answered in time; continuing "
            "without design information");
    if (_pendingDesignFetch)
    {
        const PendingChatRequest req = std::move(*_pendingDesignFetch);
        _pendingDesignFetch.reset();
        launchChatRequest(req, {});
        return;
    }
    continueApprovedBuild({});
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
