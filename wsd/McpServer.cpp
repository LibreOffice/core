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

#include <wsd/AIUtil.hpp>
#include <wsd/DeckSpec.hpp>
#include <wsd/DocumentToolDescriptions.hpp>

#include <common/Log.hpp>
#include <common/Util.hpp>
#include <common/base64.hpp>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>

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

// The text in a description that stands for the address of this endpoint, which each request puts
// in.
const std::string EndpointPlaceholder = "{mcp_endpoint}";

// How a client gives the server a document to work on. This paragraph is added to every tool
// description so a client knows the ways to pass a document and the trade-offs between them.
const std::string InputAlternativesDescription =
    " The document is passed in exactly one of two ways. For a file on the client's disk, an "
    "upload costs the least: send the raw bytes in the body of a POST to " +
    EndpointPlaceholder +
    "/upload?filename=<name>, with an Authorization header that carries the bearer token the MCP "
    "client is configured with, and set \"file_id\" to the file_id it answers with. That token "
    "lives in the client's configuration and is not passed to the tools, so when it is not "
    "available, pass the document as \"data\" instead. A file_id stays valid for 15 minutes "
    "and can be used in any number of calls. Or set \"data\" to the document bytes encoded as "
    "base64, for a document of up to 16 MB. Set \"filename\" to the file name with its "
    "extension, so the server can tell which kind of document it is.";

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

// Wrap a tool outcome as a tools/call result. A text outcome is carried in a single text content
// block. A document outcome is carried in one embedded resource content block whose blob is the
// document bytes encoded as base64. A picture of the result follows as an image content block, and
// a note as a further text content block. isError says whether the outcome is a failure the model
// should read or the tool's successful output.
std::string makeToolResultBody(const Poco::Dynamic::Var& id,
                               const McpServer::McpToolResult& outcome)
{
    Poco::JSON::Object::Ptr block = new Poco::JSON::Object();
    if (outcome.isDocument)
    {
        // The file name is one path segment of the URI, with every character outside the unreserved
        // set percent-encoded.
        std::string encodedFilename;
        Poco::URI::encode(outcome.documentFilename, "!#$&'()*+,/:;=?@[]%", encodedFilename);

        Poco::JSON::Object::Ptr resource = new Poco::JSON::Object();
        resource->set("uri", "cool://converted/" + encodedFilename);
        resource->set("mimeType", outcome.documentMimeType);
        resource->set("blob", macaron::Base64::Encode(outcome.documentBytes));

        block->set("type", "resource");
        block->set("resource", resource);
    }
    else
    {
        block->set("type", "text");
        block->set("text", outcome.text);
    }

    Poco::JSON::Array::Ptr content = new Poco::JSON::Array();
    content->add(block);

    if (!outcome.previewPng.empty())
    {
        Poco::JSON::Object::Ptr image = new Poco::JSON::Object();
        image->set("type", "image");
        image->set("data", macaron::Base64::Encode(outcome.previewPng));
        image->set("mimeType", "image/png");
        content->add(image);
    }

    if (!outcome.note.empty())
    {
        Poco::JSON::Object::Ptr note = new Poco::JSON::Object();
        note->set("type", "text");
        note->set("text", outcome.note);
        content->add(note);
    }

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

// Add a string property whose value must be one of the given names.
void addEnumProperty(Poco::JSON::Object::Ptr properties, const std::string& name,
                     const std::string& description, const std::vector<std::string>& allowed)
{
    Poco::JSON::Object::Ptr property = new Poco::JSON::Object();
    property->set("type", "string");
    property->set("description", description);
    Poco::JSON::Array::Ptr values = new Poco::JSON::Array();
    for (const std::string& value : allowed)
        values->add(value);
    property->set("enum", values);
    properties->set(name, property);
}

// Add the properties that say which document to work on. Every tool shares these, so they are
// described in one place.
void addInputProperties(Poco::JSON::Object::Ptr properties)
{
    addStringProperty(properties, "data",
                      "The document bytes encoded as base64. Pass exactly one of data or "
                      "file_id.");
    addStringProperty(properties, "file_id",
                      "An identifier from an earlier upload, valid for 15 minutes and in any "
                      "number of calls. Pass exactly one of data or file_id.");
    addStringProperty(properties, "filename",
                      "The file name with its extension, used to tell which kind of document "
                      "this is.");
}

// How a tool that produces a document returns it. This is added to the two write-path tools so a
// client can ask for a large document as a download link.
const std::string OutputDescription =
    " Set \"output\" to \"inline\" (the default) to get the produced document in the result, or to "
    "\"link\" to get a download_url instead, which suits a large document. Fetch the download_url "
    "with a GET request whose Authorization header carries the bearer token the MCP client is "
    "configured with. That token lives in the client's configuration and is not passed to the "
    "tools, so when it is not available, ask the user for it, or give the user the link to fetch. "
    "A link serves the document once, and expires after 15 minutes. A document larger than 16 "
    "MB comes as a download_url even with \"inline\".";

// Add the output property that says how a produced document is returned.
void addOutputProperty(Poco::JSON::Object::Ptr properties)
{
    addEnumProperty(properties, "output",
                    "How the produced document is returned. \"inline\" returns the document in "
                    "the result. \"link\" returns a download_url, fetched once within 15 "
                    "minutes with the bearer token the MCP client is configured with.",
                    { "inline", "link" });
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

// Mark the named properties as required in a schema.
void setRequired(Poco::JSON::Object::Ptr schema, const std::vector<std::string>& names)
{
    Poco::JSON::Array::Ptr required = new Poco::JSON::Array();
    for (const std::string& name : names)
        required->add(name);
    schema->set("required", required);
}

Poco::JSON::Object::Ptr makeConvertDocumentSchema()
{
    Poco::JSON::Object::Ptr properties = new Poco::JSON::Object();
    addInputProperties(properties);
    addStringProperty(properties, "format",
                      "The target file format as an extension, for example \"pdf\", \"odt\" or "
                      "\"docx\".");
    addStringProperty(properties, "options",
                      "Filter options passed to the document engine, in the same form the "
                      "convert-to endpoint accepts.");
    addStringProperty(properties, "lang",
                      "The language to load the document in, as a language tag such as \"en-US\".");
    addOutputProperty(properties);
    Poco::JSON::Object::Ptr schema = makeObjectSchema(properties);
    setRequired(schema, { "format" });
    return schema;
}

Poco::JSON::Object::Ptr makeTransformDocumentStructureSchema()
{
    Poco::JSON::Object::Ptr properties = new Poco::JSON::Object();
    addInputProperties(properties);
    addStringProperty(properties, "transform",
                      "A JSON string describing the changes to make, following the transform "
                      "grammar.");
    addStringProperty(properties, "format",
                      "The file format the changed document is returned in, as an extension. When "
                      "it is left out the document is returned in the format it came in.");
    addOutputProperty(properties);
    Poco::JSON::Object::Ptr schema = makeObjectSchema(properties);
    setRequired(schema, { "transform" });
    return schema;
}

std::string makeConvertDocumentDescription()
{
    return std::string(
               "Convert a document to another file format and return the converted document. Set "
               "\"format\" to the target format as an extension, for example \"pdf\", \"odt\" or "
               "\"docx\". The converted document comes back as an embedded resource.") +
           InputAlternativesDescription + OutputDescription;
}

std::string makeTransformDocumentStructureDescription()
{
    return std::string(
               "Change a text document or a presentation and return the changed document. A "
               "spreadsheet cannot be transformed. For a text document, a transform fills content "
               "controls, sets document properties under \"DocumentProperties\" (for example "
               "\"Title\", \"Subject\", \"Author\", \"Description\" or \"Keywords\" as a list), "
               "changes chart data under \"Charts.ByIndex.N\", replaces text throughout the "
               "document under \"ReplaceText\", and runs a top-level \"UnoCommand\" from the "
               "list of allowed commands. For a presentation, it runs \"SlideCommands\", "
               "described below. A "
               "transform that names a command or key the server does not know, or a command "
               "that is not allowed, is refused, and the failure names it. Warnings about "
               "commands that ran with a problem come back as a note after the document. A "
               "transform on a presentation also returns a picture of its slides, laid out in "
               "rows.\n\n") +
           DocumentToolDescriptions::TRANSFORM_INTRO +
           DocumentToolDescriptions::TRANSFORM_IMPRESS_INTRO + AIUtil::getSlideCommandDocs() +
           DocumentToolDescriptions::TRANSFORM_IMPRESS_DETAILS +
           DocumentToolDescriptions::TRANSFORM_WRITER +
           " The changed document is returned as an embedded resource. Set \"format\" to return it "
           "in a different file format." +
           InputAlternativesDescription + OutputDescription;
}

Poco::JSON::Object::Ptr makeListDesignTemplatesSchema()
{
    Poco::JSON::Object::Ptr properties = new Poco::JSON::Object();
    return makeObjectSchema(properties);
}

std::string makeListDesignTemplatesDescription()
{
    return "List the design templates a new presentation can be built with. Each entry gives the "
           "template's name, the slide parts it has a design for, the image style its pictures "
           "follow, and the limits a deck built on it must stay within. Call this before "
           "create_presentation to pick a design by name. The list is empty when the server has no "
           "design templates installed, in which case a presentation is built unstyled.";
}

// The shape of one content block of a slide: what it holds, and the text it holds.
Poco::JSON::Object::Ptr makeSlideBlockSchema()
{
    Poco::JSON::Object::Ptr properties = new Poco::JSON::Object();
    addEnumProperty(properties, "kind", "What the block holds: bullet items, or a run of text.",
                    { "bullets", "text" });

    Poco::JSON::Object::Ptr itemText = new Poco::JSON::Object();
    itemText->set("type", "string");
    Poco::JSON::Object::Ptr items = new Poco::JSON::Object();
    items->set("type", "array");
    items->set("items", itemText);
    items->set("description", "The bullet items of a \"bullets\" block, one string for each.");
    properties->set("items", items);

    addStringProperty(properties, "heading",
                      "The label above the items of a \"bullets\" block, on a two-column or "
                      "comparison slide.");
    addStringProperty(properties, "text", "The text of a \"text\" block.");
    return makeObjectSchema(properties);
}

// The shape of one slide: where it sits in the deck, what it is for, its title, its content blocks,
// its speaker notes and its picture.
Poco::JSON::Object::Ptr makeSlideSchema()
{
    Poco::JSON::Object::Ptr blocks = new Poco::JSON::Object();
    blocks->set("type", "array");
    blocks->set("items", makeSlideBlockSchema());
    blocks->set("description", "The content of the slide, in the order it appears.");

    Poco::JSON::Object::Ptr imageProperties = new Poco::JSON::Object();
    addStringProperty(imageProperties, "brief", "What the picture on the slide shows.");
    addStringProperty(imageProperties, "alt", "The text that stands in for the picture.");
    Poco::JSON::Object::Ptr image = makeObjectSchema(imageProperties);
    image->set("description", "The picture the slide carries.");

    Poco::JSON::Object::Ptr properties = new Poco::JSON::Object();
    addEnumProperty(properties, "part", "Where the slide sits in the deck.",
                    DeckSpec::partNames());
    addEnumProperty(properties, "intent", "What the slide is for, which settles its layout.",
                    DeckSpec::intentNames());
    addStringProperty(properties, "title", "The title of the slide.");
    properties->set("blocks", blocks);
    addStringProperty(properties, "notes", "The speaker notes of the slide.");
    properties->set("image", image);

    Poco::JSON::Object::Ptr slide = makeObjectSchema(properties);
    setRequired(slide, { "part", "intent", "title" });
    return slide;
}

Poco::JSON::Object::Ptr makeCreatePresentationSchema()
{
    Poco::JSON::Object::Ptr slides = new Poco::JSON::Object();
    slides->set("type", "array");
    slides->set("items", makeSlideSchema());
    slides->set("description", "The slides of the deck, in the order they appear.");

    Poco::JSON::Object::Ptr spec = new Poco::JSON::Object();
    spec->set("type", "object");
    spec->set("description", "The deck to build, an object with a \"slides\" array.");
    Poco::JSON::Object::Ptr specProperties = new Poco::JSON::Object();
    specProperties->set("slides", slides);
    spec->set("properties", specProperties);
    setRequired(spec, { "slides" });

    Poco::JSON::Object::Ptr properties = new Poco::JSON::Object();
    properties->set("spec", spec);
    addStringProperty(properties, "design",
                      "The name of a design template from list_design_templates. Leave it out to "
                      "build the deck unstyled.");
    addStringProperty(properties, "format",
                      "The file format of the produced presentation, as an extension. Defaults to "
                      "\"odp\".");
    addOutputProperty(properties);

    Poco::JSON::Object::Ptr schema = makeObjectSchema(properties);
    setRequired(schema, { "spec" });
    return schema;
}

std::string makeCreatePresentationDescription()
{
    return std::string(
               "Build a new presentation from a description of its slides and return it. Pass "
               "\"spec\" as an object with a \"slides\" array. Each slide describes what it is "
               "about rather than how to draw it, and the server lays it out.\n\n") +
           DocumentToolDescriptions::DECK_SLIDE_SHAPE + "\n\n" +
           DocumentToolDescriptions::DECK_VARIETY +
           "\n\nThe \"image\" intent needs a picture generator, which this tool does not have, so "
           "a slide asking for one is refused. Set \"design\" to a name from "
           "list_design_templates to style the deck; the limits that design declares are the ones "
           "the deck is checked against.\n\nThe result also carries a picture of every slide, "
           "laid out in rows. Look at it before you finish, and if a slide is crowded or mostly "
           "empty, change that slide and send the deck again. When the picture cannot be "
           "painted, a note says so and the deck is still returned." +
           OutputDescription;
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

        McpTool convert;
        convert.name = ToolName::ConvertDocument;
        convert.title = "Convert document";
        convert.description = makeConvertDocumentDescription();
        convert.inputSchema = makeConvertDocumentSchema();
        tools.push_back(std::move(convert));

        McpTool transform;
        transform.name = ToolName::TransformDocumentStructure;
        transform.title = "Transform document";
        transform.description = makeTransformDocumentStructureDescription();
        transform.inputSchema = makeTransformDocumentStructureSchema();
        tools.push_back(std::move(transform));

        McpTool designs;
        designs.name = ToolName::ListDesignTemplates;
        designs.title = "List design templates";
        designs.description = makeListDesignTemplatesDescription();
        designs.inputSchema = makeListDesignTemplatesSchema();
        tools.push_back(std::move(designs));

        McpTool presentation;
        presentation.name = ToolName::CreatePresentation;
        presentation.title = "Create presentation";
        presentation.description = makeCreatePresentationDescription();
        presentation.inputSchema = makeCreatePresentationSchema();
        tools.push_back(std::move(presentation));

        return tools;
    }();

    return definitions;
}

std::string McpServer::withEndpointAddress(const std::string& description,
                                           const std::string& endpointUrl)
{
    return Util::replace(description, EndpointPlaceholder, endpointUrl);
}

std::optional<McpServer::McpToolResult>
McpServer::validateCommandArguments(const Poco::JSON::Object::Ptr& arguments)
{
    // An extension or a language tag is a short run of ASCII letters, digits and hyphens.
    const auto isPlainWord = [](const std::string& value)
    {
        return value.size() <= 35 &&
               std::all_of(value.begin(), value.end(),
                           [](char character)
                           {
                               return (character >= 'a' && character <= 'z') ||
                                      (character >= 'A' && character <= 'Z') ||
                                      (character >= '0' && character <= '9') || character == '-';
                           });
    };

    if (!isPlainWord(arguments->optValue<std::string>("format", std::string())))
        return McpToolResult(/*error=*/true, "Set the format argument to a file extension such "
                                             "as \"pdf\", \"odt\" or \"docx\", made of "
                                             "letters and digits only.");

    if (!isPlainWord(arguments->optValue<std::string>("lang", std::string())))
        return McpToolResult(/*error=*/true, "Set the lang argument to a language tag such as "
                                             "\"en-US\", made of letters, digits and hyphens.");

    const std::string options = arguments->optValue<std::string>("options", std::string());
    if (std::any_of(options.begin(), options.end(),
                    [](char character) { return static_cast<unsigned char>(character) < 0x20; }))
        return McpToolResult(/*error=*/true,
                             "The options argument holds a control character, such as a line "
                             "break. Write the options on one line.");

    return std::nullopt;
}

std::optional<McpServer::McpToolResult>
McpServer::validateExtractArguments(const Poco::JSON::Object::Ptr& arguments)
{
    static const std::set<std::string> filters = { "text",     "contentcontrol", "charts",
                                                   "docprops", "trackchanges",   "slides" };
    const std::string filter = arguments->optValue<std::string>("filter", std::string());
    if (!filter.empty() && filters.count(filter) == 0)
        return McpToolResult(/*error=*/true,
                             "Set the filter argument to one of \"text\", \"contentcontrol\", "
                             "\"charts\", \"docprops\", \"trackchanges\" or \"slides\", or "
                             "leave it out.");

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
        return McpToolResult(/*error=*/true,
                             "Set the range argument to a cell range such as \"A1:D100\", made "
                             "of letters, digits, colons and dollar signs.");

    return std::nullopt;
}

std::string McpServer::contentDisposition(const std::string& filename)
{
    // The plain name keeps printable ASCII only, with each other byte, a backslash and a quote
    // replaced by an underscore. The full name follows as percent-encoded UTF-8 (RFC 6266).
    std::string plainName = filename;
    for (char& character : plainName)
    {
        const unsigned char value = static_cast<unsigned char>(character);
        if (value < 0x20 || value >= 0x7f || character == '\\' || character == '"')
            character = '_';
    }

    std::string encodedName;
    Poco::URI::encode(filename, "#&+:;=?@/", encodedName);
    return "attachment; filename=\"" + plainName + "\"; filename*=UTF-8''" + encodedName;
}

std::optional<McpServer::McpToolResult>
McpServer::validateConvertArguments(const Poco::JSON::Object::Ptr& arguments)
{
    const std::string format = arguments->optValue<std::string>("format", std::string());
    if (format.empty())
    {
        McpToolResult result;
        result.isError = true;
        result.text = "Set the format argument to the target file format as an extension, for "
                      "example \"pdf\", \"odt\" or \"docx\".";
        return result;
    }
    return validateCommandArguments(arguments);
}

std::optional<McpServer::McpToolResult>
McpServer::validateTransformArguments(const Poco::JSON::Object::Ptr& arguments)
{
    const std::string transform = arguments->optValue<std::string>("transform", std::string());
    if (transform.empty())
    {
        McpToolResult result;
        result.isError = true;
        result.text = "Set the transform argument to a JSON string describing the changes to make.";
        return result;
    }

    try
    {
        Poco::JSON::Parser parser;
        parser.parse(transform);
    }
    catch (const std::exception& exc)
    {
        McpToolResult result;
        result.isError = true;
        result.text = "The transform argument is not valid JSON: " + std::string(exc.what());
        return result;
    }
    return validateCommandArguments(arguments);
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
