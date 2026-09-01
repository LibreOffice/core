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

#include "DeckSpec.hpp"

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class ClientSession;
class DocumentBroker;
class Message;
// A forward declaration is all that's needed to hold a shared_ptr<http::Session>;
// the type is only completed (and used) in the server's #if !MOBILEAPP code paths.
namespace http { class Session; }

/// A chat failure as sent to the client: a stable code the browser maps to a
/// translated message (see translateChatError in Control.AIChatSidebar.ts),
/// the English text older clients show verbatim, and the message's variable
/// part (for example a rejected host name) interpolated client-side.
struct ChatError
{
    std::string code;
    std::string message;
    std::string arg;
};

/// Result of preparing an AI image generation HTTP request.
struct ImageGenRequest
{
    std::shared_ptr<http::Session> httpSession; // server transport; null on the desktop/error
    std::string requestUrl;
    std::string apiKey;
    std::string payloadStr;
    std::string error; // non-empty if setup failed
    std::string errorCode; // stable identifier for the client's translation
    std::string errorArg; // variable part of the message (for example the host)
};

/// Result of parsing an image-generation response: the image payload on
/// success, or the failure mapped for the client (error/errorCode/errorArg
/// as in ImageGenRequest).
struct ImageGenResult
{
    std::string b64Json;
    std::string error;
    std::string errorCode;
    std::string errorArg;
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
    std::string alt;       // accessibility label for the inserted image, may be empty
};

/// The design information a picked template contributes to a chat request: the
/// slide parts and layouts it has designs for, its image art direction, and any
/// budget limits its manifest tightens. Empty when no template was picked or the
/// template carries no manifest.
struct DesignInfo
{
    std::vector<std::string> parts;
    std::vector<std::string> layouts;
    // The template manifest's one-sentence image style, already sanitized, or
    // empty when the template declares none.
    std::string artDirection;
    // Budget limits the manifest declares. Each is applied as a tighten-only
    // lower bound against the configured budgets; an absent value leaves the
    // configured limit in place.
    std::optional<int> maxSlides;
    std::optional<int> maxItemsPerBullets;
    std::optional<int> maxItemLength;
    std::optional<int> maxTitleLength;
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
    // Name of the design template the user picked for this presentation, empty
    // when none was chosen.
    std::string designTemplate;
    // The reply tone the user picked, and the free-text description when the
    // tone is "custom". Kept so the per-slide expansion prompts can carry the
    // same tone as the initial request.
    std::string tone;
    std::string customToneDescription;
    // The deck-spec limits this request is validated against, read from
    // configuration and possibly lowered by the picked template's manifest.
    DeckSpec::Budgets budgets;
    // The image style appended to every generated-image prompt: the picked
    // template's art direction, or a neutral default when none applies.
    std::string artDirection;
    // Read-verify-insert tasks (e.g. "add a formula for each record") legitimately
    // need several rounds: read the sheet, check functions, evaluate, then insert.
    // Keep a ceiling to prevent runaway loops, but high enough to finish the work.
    static constexpr int InitialToolRounds = 12;
    int toolRoundsRemaining = InitialToolRounds; // max rounds to prevent infinite loops
    int validationRetriesRemaining = 3; // silent re-prompts for malformed payloads
    int reasoningOnlyRetriesRemaining =
        1; // nudges when a turn ends with reasoning and no answer/tool call
    bool retriedWithoutTemperature = false; // one-shot retry when the model rejects temperature
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

/// A parsed aichat request held while wsd fetches the picked design template's
/// masters and layouts from the kit. The system prompt depends on those, so a
/// presentation request that picked a template is stashed here, the fetch is
/// issued, and the request is launched once the reply arrives.
struct PendingChatRequest
{
    std::string requestId;
    std::string docType;            // text|spreadsheet|presentation|drawing, or empty
    std::string designTemplate;     // picked template name, empty when none
    std::string tone;
    std::string customToneDescription;
    bool emojify = false;
    bool hasSelectedText = false;   // the last user message carries selected document text
    Poco::JSON::Array::Ptr messages; // sanitized user/assistant messages, no system prompt
    std::string model;
    std::string requestUrl;
    std::string apiKey;
    // The ".uno:GetDesignTemplateDesigns?name=..." command whose reply this
    // request waits for, matched against the reply's commandName. Empty when no
    // fetch is outstanding.
    std::string fetchCommand;
};

/// An approved deck build held while wsd fetches the design template picked on
/// the decision card from the kit. Holds the approved outline for the
/// slide-by-slide build.
struct PendingApprovedBuild
{
    Poco::JSON::Object::Ptr outline;
    // The ".uno:GetDesignTemplateDesigns?name=..." command whose reply this
    // build waits for, matched against the reply's commandName.
    std::string fetchCommand;
};

/// State for building a deck one slide at a time after the user approved an
/// outline. The whole tool loop is parked while this runs: each slide is one
/// forced write_slide call to the model, validated, compiled, and applied to
/// the kit without a further approval.
struct DeckExpansionState
{
    std::string outlineTitle;        // the deck title from the approved outline
    Poco::JSON::Array::Ptr slides;   // the approved outline entries, in order
    std::string outlineJson;         // the compact outline, resent in each prompt
    unsigned nextIndex = 0;          // outline index of the slide being built
    int builtCount = 0;              // slides successfully applied to the deck
    bool retriedCurrentSlide = false; // the current slide already had one retry
    std::string lastSlideError;      // why the current slide's last attempt failed
    std::vector<int> skippedSlides;  // 1-based outline positions that were skipped
    std::vector<std::string> failedImagePrompts; // image briefs that did not render
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
    /// Consumes the GetDesignTemplateDesigns reply for a request stashed in
    /// _pendingDesignFetch: parses the template's masters and layouts and
    /// launches the chat request with them. Returns false (leaving the reply
    /// to its normal path) when no design fetch is outstanding or the reply's
    /// commandName does not match the one we sent.
    bool tryConsumeDesignFetch(const std::shared_ptr<Message>& payload);
    /// Give up on an outstanding design fetch that the kit has not answered by
    /// its deadline: launch the stashed request, or start the stashed approved
    /// build, without any design information, and clear the pending state.
    /// Does nothing when no fetch is outstanding or the deadline has not
    /// passed.
    void checkDesignFetchTimeout(std::chrono::steady_clock::time_point now);
    bool tryConsumeExtractedLinkTargets(const std::shared_ptr<Message>& payload);
    bool tryConsumeExtractedDocumentStructure(const std::shared_ptr<Message>& payload);
    bool tryConsumeTransformedDocumentStructure(const std::shared_ptr<Message>& payload);

private:
    /// Send an aichatresult: frame. When success is true and displayText
    /// is non-empty, the message carries a separate user-facing
    /// rendering (displayContent) distinct from the model-facing text;
    /// otherwise the single text is used for both. When the display text is
    /// server-composed (not model output), displayCode names it for the
    /// client's translation map in Control.AIChatSidebar.ts and displayArgs
    /// carries its variable parts; displayText stays the English fallback.
    void sendChatResult(bool success, const std::string& text, const std::string& requestId,
                        const std::string& displayText = std::string(),
                        const std::string& displayCode = std::string(),
                        const std::vector<std::string>& displayArgs = {});
    /// Send a failed aichatresult: frame. The code is the stable identifier
    /// the client maps to a translated message and must match the map in
    /// Control.AIChatSidebar.ts; text is the English fallback older clients
    /// show verbatim; arg carries the message's variable part (for example
    /// the rejected host) for client-side interpolation.
    void sendChatError(const std::string& code, const std::string& text,
                       const std::string& requestId, const std::string& arg = std::string());
    /// Maps an HTTP status (or an ai::Http* sentinel) to a user-facing error.
    /// The response body lets a 429 tell an exhausted quota apart from a
    /// throttle, which need opposite advice.
    static ChatError mapHttpStatusToError(int statusCode,
                                          const std::string& reasonPhrase,
                                          const std::string& body = "");
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
    /// Builds the full system prompt and the message list, initialises the
    /// tool loop, and makes the first LLM call. The design carries the picked
    /// template's parts, layouts, art direction, and any tightened budgets;
    /// pass a default-constructed DesignInfo when no template was picked.
    void launchChatRequest(const PendingChatRequest& req, const DesignInfo& design);
    /// Sets the tool loop's budgets (the configured ceiling, tightened by the
    /// design's manifest limits) and its art direction (the design's, or the
    /// neutral default).
    void applyDesignToToolLoop(const DesignInfo& design);
    /// Starts the design fetch for a template picked on a decision card. The
    /// approved outline is stashed and the build continues when the reply
    /// arrives or the fetch deadline passes. Returns false, leaving the caller
    /// to continue the build directly, when the approval carries no usable pick.
    bool beginDesignFetchForApproval(const Poco::JSON::Object::Ptr& approveObj,
                                     const Poco::JSON::Object::Ptr& outline);
    /// Continues the approved build stashed by beginDesignFetchForApproval:
    /// applies the fetched design, then starts the slide-by-slide expansion of
    /// the approved outline.
    void continueApprovedBuild(const DesignInfo& design);
    void callLLMAPI();
    /// POST a chat-completion payload to the model endpoint. Reads the URL and
    /// key from the current tool loop, sets the active transport, and delivers
    /// the result to onResponse on the document broker's polling thread.
    /// statusCode is an HTTP code or an ai::Http* sentinel; body is the response
    /// body (empty when there was no response); reason is the HTTP reason phrase
    /// (empty on the desktop transport).
    void postChatCompletion(
        std::string payloadStr,
        std::function<void(int statusCode, const std::string& body,
                           const std::string& reason)> onResponse);
    void handleLLMResponse(const std::string& responseBody);
    bool executeToolCall(const std::string& toolCallId,
                         const std::string& fnName,
                         const std::string& argsJson);
    void processNextPendingToolCall();
    void continueToolLoop(const std::string& toolCallId,
                          const std::string& result);
    /// Send an aichatprogress: frame. The status is the English fallback older
    /// clients show verbatim; statusKey is the stable identifier the client
    /// maps to a translated message and must match the map in
    /// Control.AIChatSidebar.ts; args carries the message's variable parts
    /// (slide or image counters) for client-side interpolation.
    void sendToolProgress(const std::string& toolName, const std::string& status,
                          const std::string& statusKey = std::string(),
                          const std::vector<std::string>& args = {});
    void sendToolApproval(const std::string& toolName,
                          const std::string& description);
    /// Rewrite a slide-command transform in place for forwarding to the kit:
    /// drop any server-only command the model emitted, and prepend the user's
    /// design template as an ApplyTemplate command when one is set, so the engine
    /// maps this transform's slides onto the template's masters.
    void spliceSlideCommands(const Poco::JSON::Object::Ptr& transformObj);
    /// Prepare a slide-command transform for user approval: drop any server-only
    /// command the model emitted, splice in the user's design template when one
    /// is set, store the result as the pending transform, and send the approval
    /// message.
    void sendTransformForApproval(const std::string& toolCallId,
                                  const std::string& fnName,
                                  const Poco::JSON::Object::Ptr& transformObj,
                                  std::string summary);
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
    /// Parses an image-generation response: on success b64Json is non-empty,
    /// otherwise error (with errorCode/errorArg) describes the failure.
    /// statusCode may be an ai::Http* sentinel.
    static ImageGenResult parseImageGenResponse(int statusCode, const std::string& body);
    /// Rewrite the pending transform's GenerateImage commands into loading
    /// placeholders, apply it, then fetch and fill in the real images. Each
    /// image's target slide is worked out from nExistingSlides, the number of
    /// slides already in the document when the transform runs, so an image on a
    /// freshly appended slide lands on that slide and not an earlier one.
    void processTransformImageGenerations(const std::shared_ptr<DocumentBroker>& docBroker,
                                          int nExistingSlides = 1);
    void generateNextTransformImage(const std::shared_ptr<DocumentBroker>& docBroker);
    std::string appendImageGenFailures(const std::string& result) const;

    /// Start building the deck from the approved outline: store the outline in
    /// _deckExpansion and expand the first slide.
    void startDeckExpansion(const Poco::JSON::Object::Ptr& outline);
    /// Advance to the next outline slide: send progress, build the per-slide
    /// system and user prompts, and post one forced write_slide call. When no
    /// slides remain, finish the deck.
    void expandNextSlide();
    /// Handle the model's per-slide reply: on a transport, protocol, or
    /// validation problem fail the slide; otherwise force the part, intent and
    /// title back to the approved outline entry and apply the slide.
    void handleExpansionResponse(int statusCode, const std::string& body,
                                 const std::string& reason);
    /// Compile one expanded slide, splice in the design template, and forward it
    /// to the kit through the image-aware transform path, without an approval.
    void applyExpansionSlide(const Poco::JSON::Object::Ptr& slide);
    /// Handle the kit's reply to an applied slide: fold in any image failures,
    /// count the slide as built or skipped by its success flag, and move on. A
    /// kit apply failure does not trigger a model retry.
    void onExpansionSlideApplied(const std::string& result);
    /// The current slide's attempt failed: retry it once with the error as a
    /// hint, then skip it and advance if it fails again.
    void failCurrentExpansionSlide(const std::string& reason);
    /// End the expansion: send the final result with a numbered deck manifest
    /// for the model and a short ready message for the user, and clear state.
    void finishDeckExpansion();

    ClientSession& _session;
    std::shared_ptr<http::Session> _activeChatSession; // server transport; unused on the desktop
    std::unique_ptr<AIToolLoopState> _toolLoop;
    // Deck being built slide by slide from an approved outline. Non-null only
    // while an expansion is running; the tool loop stays alive but parked.
    std::unique_ptr<DeckExpansionState> _deckExpansion;
    // A request waiting for its design template's masters and layouts from the
    // kit, before its prompt is built and the first LLM call is made. Null when
    // no such fetch is outstanding.
    std::unique_ptr<PendingChatRequest> _pendingDesignFetch;
    // An approved deck build waiting for the designs of the template picked on
    // the decision card. Null when no such fetch is outstanding.
    std::unique_ptr<PendingApprovedBuild> _pendingApprovedBuild;
    // The point in time by which the kit must answer the outstanding design
    // fetch. Only meaningful while _pendingDesignFetch or _pendingApprovedBuild
    // is set; the two are never outstanding together.
    std::chrono::steady_clock::time_point _designFetchDeadline;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
