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

#if !MOBILEAPP

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class ClientSession;
class DocumentBroker;
class Message;
namespace http { class Session; class Response; enum class StatusCode : unsigned; }

/// Result of preparing an AI image generation HTTP request.
struct ImageGenRequest
{
    std::shared_ptr<http::Session> httpSession; // null on error
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
    int toolRoundsRemaining = 5;     // max rounds to prevent infinite loops
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
    void sendChatResult(bool success, const std::string& text,
                        const std::string& requestId);
    static std::string mapHttpStatusToError(http::StatusCode statusCode,
                                            const std::string& reasonPhrase,
                                            const std::string& context = "");
    Poco::JSON::Array::Ptr buildToolDefinitions() const;
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
    bool handleImageGeneration(const std::string& prompt,
                               const std::string& requestId);
    ImageGenRequest createImageGenRequest(const std::string& prompt);
    static std::pair<std::string, std::string> parseImageGenResponse(
        const std::shared_ptr<const http::Response>& httpResponse);
    void processTransformImageGenerations(const std::shared_ptr<DocumentBroker>& docBroker);
    void generateNextTransformImage(std::shared_ptr<DocumentBroker> docBroker);
    std::string appendImageGenFailures(const std::string& result) const;

    ClientSession& _session;
    std::shared_ptr<http::Session> _activeChatSession;
    std::unique_ptr<AIToolLoopState> _toolLoop;
};

#endif // !MOBILEAPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
