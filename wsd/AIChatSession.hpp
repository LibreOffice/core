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
 * AI chat session: multi-round LLM tool-calling loop driven by a ClientSession.
 * Extracted from ClientSession so the session itself stays focused on protocol
 * and lifecycle. Owned by its ClientSession, which delegates the aichat:,
 * aichatcancel:, and aichatapprove: protocol messages here and routes matching
 * kit responses in through the tryConsume* methods.
 */

#pragma once

#include <config.h>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class ClientSession;
class DocumentBroker;
class Message;
// A forward declaration is all that's needed to hold a shared_ptr<http::Session>;
// the type is only completed (and used) in the server's #if !MOBILEAPP code paths.
namespace http { class Session; }

/// Result of preparing an AI image generation HTTP request.
struct ImageGenRequest
{
    std::shared_ptr<http::Session> httpSession; // server transport; null on the desktop/error
    std::string requestUrl;
    std::string apiKey;
    std::string payloadStr;
    std::string error; // non-empty if setup failed
};

/// A single tool call from the LLM that is queued for execution.
struct PendingToolCall
{
    std::string toolCallId;
    std::string functionName;
    std::string arguments;
};

/// A single pending image generation within a transform.
struct PendingImageGen
{
    int slideIndex;        // target slide index
    int objId;             // placeholder object index (N from GenerateImage.N)
    std::string prompt;    // image generation prompt
    std::string filePath;  // filled after generation with file:// URL for kit
};

/// State for the AI chat multi-round tool loop.
/// The server drives the loop: LLM response -> tool execution -> LLM response -> ...
struct AIToolLoopState
{
    std::string requestId;
    Poco::JSON::Array::Ptr messages; // accumulated conversation
    std::string model;
    std::string requestUrl;
    std::string apiKey;
    std::string docType;             // text|spreadsheet|presentation|drawing, or empty
    // Read-verify-insert tasks (e.g. "add a formula for each record") legitimately
    // need several rounds: read the sheet, check functions, evaluate, then insert.
    // Keep a ceiling to prevent runaway loops, but high enough to finish the work.
    static constexpr int InitialToolRounds = 12;
    int toolRoundsRemaining = InitialToolRounds; // max rounds to prevent infinite loops
    int validationRetriesRemaining = 3; // silent re-prompts for malformed payloads
    int reasoningOnlyRetriesRemaining =
        1; // nudges when a turn ends with reasoning and no answer/tool call
    bool awaitingKitResponse = false;
    bool awaitingApproval = false;
    std::string pendingToolCallId;
    std::string pendingToolName;
    std::string pendingTransformArgs; // stored while awaiting approval
    std::string pendingSummary;        // markdown summary for approval UI
    std::string pendingForwardCommand; // command to forward to kit after approval
    std::vector<PendingToolCall> pendingToolCalls; // queued tool calls

    // Image generation state for transform_document_structure
    std::vector<PendingImageGen> pendingImageGens;
    std::size_t nextImageGenIndex = 0;
    bool generatingImages = false;       // main transform forwarded, generating images
    int outstandingImageTransforms = 0;  // mini-transform responses still expected
    std::string mainTransformResult;     // kit response from the initial transform
    std::vector<std::string> failedImagePrompts; // prompts of images that failed to generate
};

class AIChatSession
{
public:
    explicit AIChatSession(ClientSession& session);
    ~AIChatSession();

    AIChatSession(const AIChatSession&) = delete;
    AIChatSession& operator=(const AIChatSession&) = delete;

    /// Handle aichat: protocol message from the client.
    bool handleAction(const std::string& firstLine);
    /// Handle aichatcancel: protocol message from the client.
    bool handleCancel(const std::string& firstLine);
    /// Handle aichatapprove: protocol message from the client.
    bool handleApprove(const std::string& firstLine);

    /// Kit-response intercepts. Each returns true iff it consumed the
    /// message and the caller should stop processing it.
    bool tryConsumeKitError(const std::string& errorCommand,
                            const std::string& errorKind);
    bool tryConsumeCommandValues(const std::shared_ptr<Message>& payload);
    bool tryConsumeExtractedLinkTargets(const std::shared_ptr<Message>& payload);
    bool tryConsumeExtractedDocumentStructure(const std::shared_ptr<Message>& payload);
    bool tryConsumeTransformedDocumentStructure(const std::shared_ptr<Message>& payload);

private:
    /// Send an aichatresult: frame. When success is true and displayText
    /// is non-empty, the message carries a separate user-facing
    /// rendering (displayContent) distinct from the model-facing text;
    /// otherwise the single text is used for both.
    void sendChatResult(bool success, const std::string& text, const std::string& requestId,
                        const std::string& displayText = std::string());
    /// Maps an HTTP status (or an ai::Http* sentinel) to a user-facing message.
    static std::string mapHttpStatusToError(int statusCode,
                                            const std::string& reasonPhrase,
                                            const std::string& context = "");
    Poco::JSON::Array::Ptr buildToolDefinitions(const std::string& docType) const;
#if MOBILEAPP
    /// Desktop transport: POST via the registered ai::HttpPostFn and deliver the
    /// result to \p onResponse on \p docBroker's polling thread (statusCode is an
    /// HTTP code or an ai::Http* sentinel).
    void postViaTransport(const std::shared_ptr<DocumentBroker>& docBroker,
                          const std::string& url, const std::string& authHeader,
                          std::string body,
                          std::function<void(int statusCode, std::string body)> onResponse);
#endif
    void callLLMAPI();
    void handleLLMResponse(const std::string& responseBody);
    bool executeToolCall(const std::string& toolCallId,
                         const std::string& fnName,
                         const std::string& argsJson);
    void processNextPendingToolCall();
    void continueToolLoop(const std::string& toolCallId,
                          const std::string& result);
    void sendToolProgress(const std::string& toolName,
                          const std::string& status);
    void sendToolApproval(const std::string& toolName,
                          const std::string& description);
    /// If the kit's extract_document_structure result describes the
    /// big-document truncation branch (link_targets present in the
    /// BodyText payload with at least one heading or named section),
    /// short-circuit the tool loop: forward the picks as an
    /// aichatchoices: message and a synthetic assistant reply, then
    /// end the loop instead of paying for a model round-trip that
    /// would just rewrite the heading list. Returns true when the
    /// short-circuit fired; the caller then skips continueToolLoop.
    bool tryShortCircuitBigDocumentRead(const std::string& payloadJson);
    bool handleImageGeneration(const std::string& prompt,
                               const std::string& requestId);
    ImageGenRequest createImageGenRequest(const std::string& prompt);
    /// Parses an image-generation response into {base64Image, errorMessage};
    /// exactly one is non-empty. statusCode may be an ai::Http* sentinel.
    static std::pair<std::string, std::string> parseImageGenResponse(
        int statusCode, const std::string& body);
    void processTransformImageGenerations(const std::shared_ptr<DocumentBroker>& docBroker);
    void generateNextTransformImage(const std::shared_ptr<DocumentBroker>& docBroker);
    std::string appendImageGenFailures(const std::string& result) const;

    ClientSession& _session;
    std::shared_ptr<http::Session> _activeChatSession; // server transport; unused on the desktop
    std::unique_ptr<AIToolLoopState> _toolLoop;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
