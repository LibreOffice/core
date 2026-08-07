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
 * Every path other than a tool call answers before handleMessage returns. A tool call may answer
 * later, once the work it started has finished, and the return value of handleMessage says which of
 * the two happened.
 */

#pragma once

#include <config.h>

#include <Poco/JSON/Object.h>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

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

    /// The outcome of running one tool.
    ///
    /// A tool that finishes its work reports a result here. A failure that the calling model should
    /// read and react to, such as an input document that cannot be loaded, is a normal result with
    /// isError set to true and text explaining what went wrong. The protocol-level errors stay
    /// reserved for a malformed call.
    struct McpToolResult
    {
        McpToolResult() = default;
        /// A text result: the text and whether it describes a failure.
        McpToolResult(bool error, std::string resultText)
            : isError(error)
            , text(std::move(resultText))
        {
        }

        /// True when the text describes a failure the model should read, false when the text is the
        /// tool's successful output.
        bool isError = false;
        /// The text placed in the single text content block of the result.
        std::string text;
        /// True when the result carries a produced document rather than text. The document is
        /// written as one embedded resource content block: documentBytes holds the raw bytes,
        /// documentFilename names the file, and documentMimeType gives its media type. The bytes
        /// are turned into base64 when the block is built.
        bool isDocument = false;
        /// The raw bytes of the produced document, when isDocument is true.
        std::string documentBytes;
        /// The name of the produced document, carrying its extension.
        std::string documentFilename;
        /// The media type of the produced document.
        std::string documentMimeType;
        /// A remark about the result, or empty for none. It is written as a text content block
        /// after the main one.
        std::string note;
    };

    /// Delivers the outcome of a tool once it is known.
    using McpToolCompletion = std::function<void(McpToolResult)>;

    /// Runs one tool. The arguments are the already parsed "arguments" object of a tools/call
    /// request. The result is delivered through complete.
    ///
    /// The return value says whether the result was delivered before the executor returned. A tool
    /// that finishes its work right away, for example one that rejects its input, calls complete
    /// and returns true. A tool that starts asynchronous work and will call complete later returns
    /// false, which lets the transport keep the request open until then.
    using McpToolExecutor =
        std::function<bool(const Poco::JSON::Object::Ptr& arguments,
                           const McpToolCompletion& complete)>;

    /// One tool the server offers: how it is named and described to a client, the JSON Schema for
    /// its arguments, and the code that runs it.
    struct McpTool
    {
        std::string name;
        /// The name a client shows to a person, in a few words.
        std::string title;
        std::string description;
        Poco::JSON::Object::Ptr inputSchema;
        /// True when the tool reads the document it is given and changes nothing, neither on the
        /// server nor at the caller. A tool that returns a changed copy also counts.
        bool readOnly = true;
        McpToolExecutor executor;
    };

    /// The name a client calls each document tool by. Each is the name of one tool, and the same
    /// name appears in the tool listing and in a tools/call request for that tool.
    struct ToolName
    {
        static constexpr std::string_view ExtractDocumentStructure =
            "extract_document_structure";
        static constexpr std::string_view ExtractLinkTargets = "extract_link_targets";
        static constexpr std::string_view ConvertDocument = "convert_document";
        static constexpr std::string_view TransformDocumentStructure =
            "transform_document_structure";
        static constexpr std::string_view ListDesignTemplates = "list_design_templates";
        static constexpr std::string_view CreatePresentation = "create_presentation";
    };

    /// The document tools with their names, descriptions and argument schemas filled in and their
    /// executors left empty. One list of definitions backs both the tool listing and the tool call,
    /// so the two always describe the same tools.
    static std::vector<McpTool> documentToolDefinitions();

    /// The description with the address of this endpoint, as the client reaches it, put in place of
    /// the placeholder the descriptions carry for it.
    static std::string withEndpointAddress(const std::string& description,
                                           const std::string& endpointUrl);

    /// Check the arguments a convert_document call carries before any document work starts. Returns
    /// an empty optional when they are usable, or a readable failure result when a required
    /// argument such as the target format is missing.
    static std::optional<McpToolResult>
    validateConvertArguments(const Poco::JSON::Object::Ptr& arguments);

    /// Check the arguments an extract_document_structure call carries before any document work
    /// starts. Returns an empty optional when the filter is absent or one of the documented views,
    /// and the range is absent or made of letters, digits, colons and dollar signs. Otherwise
    /// returns a readable failure naming the argument.
    static std::optional<McpToolResult>
    validateExtractArguments(const Poco::JSON::Object::Ptr& arguments);

    /// The value of a Content-Disposition header that offers a download named filename: the name as
    /// printable ASCII in filename, and in full as percent-encoded UTF-8 in filename*.
    static std::string contentDisposition(const std::string& filename);

    /// Check the format, lang and options arguments any tool may carry. Returns an empty optional
    /// when each is absent or well formed: a format is an extension and a lang a language tag, both
    /// made of ASCII letters, digits and hyphens, and options hold no control characters. Otherwise
    /// returns a readable failure naming the argument.
    static std::optional<McpToolResult>
    validateCommandArguments(const Poco::JSON::Object::Ptr& arguments);

    /// Check the arguments a transform_document_structure call carries before any document work
    /// starts. Returns an empty optional when the transform is present and parses as JSON, or a
    /// readable failure result when it is missing or cannot be parsed.
    static std::optional<McpToolResult>
    validateTransformArguments(const Poco::JSON::Object::Ptr& arguments);

    /// The MCP protocol versions this server understands.
    static bool isSupportedProtocolVersion(const std::string& version);

    /// Turn one JSON-RPC message into one response.
    ///
    /// body is the raw request body. protocolVersionHeader is the value of the MCP-Protocol-Version
    /// HTTP header, or an empty string when the header is absent. tools is the set of tools the
    /// server offers, each with an executor attached. The result is delivered through sendResponse.
    ///
    /// The return value says whether the response was delivered before this call returned. Every
    /// path other than a tool call answers right away and returns true. A tool call returns
    /// whatever its executor returns, so an asynchronous tool returns false and answers later.
    static bool handleMessage(const std::string& body,
                              const std::string& protocolVersionHeader,
                              const std::vector<McpTool>& tools,
                              const std::function<void(McpResponse)>& sendResponse);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
