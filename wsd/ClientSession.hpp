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
 * Client session management in WSD process.
 * Classes: ClientSession
 */

#pragma once

#include <common/Rectangle.hpp>
#include <common/Session.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/SenderQueue.hpp>
#include <wsd/ServerURL.hpp>
#include <wsd/Storage.hpp>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/SharedPtr.h>
#include <Poco/URI.h>

#include <deque>
#include <optional>
#include <utility>

class DocumentBroker;
namespace http { class Session; }

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

/// Represents a session to a COOL client, in the WSD process.
class ClientSession final : public Session
{
public:
    ClientSession(const std::shared_ptr<ProtocolHandlerInterface>& ws, const std::string& id,
                  const std::shared_ptr<DocumentBroker>& docBroker, const Poco::URI& uriPublic,
                  bool isReadOnly, const RequestDetails& requestDetails,
                  const AdditionalFilePocoUris& additionalFileUrisPublic = {});
    void construct();
    virtual ~ClientSession();

    void setReadOnly(bool value = true) override;

    void sendFileMode(bool readOnly, bool editComments, bool manageRedlines);

    void setLockFailed(const std::string& reason);

    STATE_ENUM(SessionState,
               DETACHED, // initial
               LOADING, // attached to a DocBroker & waiting for load
               LIVE, // Document is loaded & editable or viewable.
               WAIT_DISCONNECT // closed and waiting for Kit's disconnected message
    );

    /// Returns true if this session has loaded a view (i.e. we got status message).
    bool isViewLoaded() const { return _state == SessionState::LIVE; }

    /// returns true if we're waiting for the kit to acknowledge disconnect.
    bool inWaitDisconnected() const { return _state == SessionState::WAIT_DISCONNECT; }

    /// transition to a new state
    void setState(SessionState newState);

    void setDocumentOwner(const bool documentOwner) { _isDocumentOwner = documentOwner; }
    bool isDocumentOwner() const { return _isDocumentOwner; }

    /// Returns true iff the view is loaded and not disconnected
    /// from either the client or the Kit.
    bool isLive() const { return _state == SessionState::LIVE && !isCloseFrame(); }

    /// Handle kit-to-client message.
    bool handleKitToClientMessage(const std::shared_ptr<Message>& payload);

    std::optional<bool>
    handleOpenDocKitToClientMessage(const std::shared_ptr<Message>& payload,
                                    const std::shared_ptr<DocumentBroker>& docBroker,
                                    const std::shared_ptr<StreamSocket>& saveAsSocket);

    /// Integer id of the view in the kit process, or -1 if unknown
    int getKitViewId() const { return _kitViewId; }

    /// Disconnect the session and do final cleanup, @returns true if we should not wait.
    bool disconnectFromKit();

    // sendTextFrame that takes std::string and string literal.
    using Session::sendTextFrame;

    bool sendBinaryFrame(const char* buffer, int length) override
    {
        if (!isCloseFrame())
        {
            enqueueSendMessage(std::make_shared<Message>(buffer, length, Message::Dir::Out));
            return true;
        }

        return false;
    }

    void resetTileSeq(const TileDesc &desc)
    {
        _tracker.resetTileSeq(desc);
    }

    // no tile data - just notify the client the ids/versions updated
    bool sendUpdateNow(const TileDesc &desc)
    {
        TileWireId lastSentId = _tracker.updateTileSeq(desc);
        std::string header = desc.serialize("update:", "\n");
        LOG_TRC("Sending update from " << lastSentId << " to " << header);
        return sendTextFrame(header.data(), header.size());
    }

    bool sendTileNow(const TileDesc &desc, const Tile &tile)
    {
        TileWireId lastSentId = _tracker.updateTileSeq(desc);

        std::string header;
        if (tile->needsKeyframe(lastSentId) || tile->isPng())
            header = desc.serialize("tile:", "\n");
        else
            header = desc.serialize("delta:", "\n");

        // FIXME: performance - optimize away this copy ...
        std::vector<char> output;

        // copy in the header
        output.resize(header.size());
        std::memcpy(output.data(), header.data(), header.size());

        bool hasContent = tile->appendChangesSince(output, tile->isPng() ? 0 : lastSentId);
        LOG_TRC("Sending tile message: " << header << " lastSendId " << lastSentId << " content " << hasContent);
        return sendBinaryFrame(output.data(), output.size());
    }

    bool sendBlob(const std::string &header, const Blob &blob)
    {
        // FIXME: performance - optimize away this copy ...
        std::vector<char> output;

        output.resize(header.size() + blob->size());
        std::memcpy(output.data(), header.data(), header.size());
        std::memcpy(output.data() + header.size(), blob->data(), blob->size());

        return sendBinaryFrame(output.data(), output.size());
    }

    bool sendTextFrame(const char* buffer, const int length) override
    {
        if (!isCloseFrame())
        {
            enqueueSendMessage(std::make_shared<Message>(buffer, length, Message::Dir::Out));
            return true;
        }

        return false;
    }

    void enqueueSendMessage(const std::shared_ptr<Message>& data);

    /// Set the save-as socket which is used to send convert-to results.
    void setSaveAsSocket(const std::shared_ptr<StreamSocket>& socket)
    {
        _saveAsSocket = socket;
        _isConvertTo = static_cast<bool>(socket);
    }

    std::shared_ptr<DocumentBroker> getDocumentBroker() const { return _docBroker.lock(); }

    /// Exact URI (including query params - access tokens etc.) with which
    /// client made the request to us
    ///
    /// Note: This URI is unsafe - when connecting to existing sessions, we must
    /// ignore everything but the access_token, and use the access_token with
    /// the URI of the initial request.
    const Poco::URI& getPublicUri() const { return _uriPublic; }

    const AdditionalFilePocoUris& getAdditionalFilePublicUri() const { return _additionalFileUrisPublic; }

    /// The access token of this session.
    const Authorization& getAuthorization() const { return _auth; }

    void invalidateAuthorizationToken()
    {
        LOG_DBG("Session [" << getId() << "] expiring its authorization token");
        _auth.expire();
    }

    /// Set WOPI fileinfo object
    void setWopiFileInfo(std::unique_ptr<WopiStorage::WOPIFileInfo> wopiFileInfo) { _wopiFileInfo = std::move(wopiFileInfo); }

    /// Get requested tiles waiting for sending to the client
    std::deque<TileDesc>& getRequestedTiles() { return _requestedTiles; }

    /// Mark a new tile as sent
    void addTileOnFly(TileWireId wireId);
    size_t getTilesOnFlyCount() const { return _tilesOnFly.size(); }
    size_t getTilesOnFlyUpperLimit() const;
    void removeOutdatedTilesOnFly(std::chrono::steady_clock::time_point now);
    void onTileProcessed(TileWireId wireId);

    Util::Rectangle getVisibleArea() const { return _clientVisibleArea; }
    /// Visible area can have negative value as position, but we have tiles only in the positive range
    Util::Rectangle getNormalizedVisibleArea() const;

    /// The client's visible area can be divided into a maximum of 4 panes.
    enum SplitPaneName : std::uint8_t
    {
        TOPLEFT_PANE,
        TOPRIGHT_PANE,
        BOTTOMLEFT_PANE,
        BOTTOMRIGHT_PANE
    };

    /// Returns true if the given split-pane is currently valid.
    bool isSplitPane(SplitPaneName) const;

    /// Returns the normalized visible area of a given split-pane.
    Util::Rectangle getNormalizedVisiblePaneArea(SplitPaneName) const;

    int getTileWidthInTwips() const { return _tileWidthTwips; }
    int getTileHeightInTwips() const { return _tileHeightTwips; }

    bool isTextDocument() const { return _isTextDocument; }

    void setThumbnailSession(const bool val) { _thumbnailSession = val; }

    void setThumbnailTarget(const std::string& target) { _thumbnailTarget = target; }

    const std::string& getThumbnailTarget() const { return _thumbnailTarget; }

    void setThumbnailPosition(const std::pair<int, int>& pos) { _thumbnailPosition = pos; }

    const std::pair<int, int>& getThumbnailPosition() const { return _thumbnailPosition; }

    bool thumbnailSession() const { return _thumbnailSession; }

    /// Do we recognize this clipboard ?
    bool matchesClipboardKeys(const std::string &viewId, const std::string &tag);

    /// Handle presentation info request
    bool handlePresentationInfo(const std::shared_ptr<Message>& payload, const std::shared_ptr<DocumentBroker>& docBroker);

    /// Handle a clipboard fetch / put request.
    void handleClipboardRequest(DocumentBroker::ClipboardRequest     type,
                                const std::shared_ptr<StreamSocket> &socket,
                                const std::string                   &tag,
                                const std::string                   &clipFile);

    /// Create URI for transient clipboard content.
    std::string getClipboardURI(bool encode = true);

    /// Utility to create a publicly accessible URI.
    std::string createPublicURI(const std::string& subPath, const std::string& tag, bool encode);

    /// Adds and/or modified the copied payload before sending on to the client.
    void postProcessCopyPayload(const std::shared_ptr<Message>& payload);
    bool postProcessCopyPayload(std::istream&, std::ostream&);

    /// Removes the <meta name="origin" ...> tag which was added in
    /// ClientSession::postProcessCopyPayload().
    bool preProcessSetClipboardPayload(std::istream&, std::ostream&);

    /// Returns true if we're expired waiting for a clipboard and should be removed
    bool staleWaitDisconnect(std::chrono::steady_clock::time_point now);

    /// Generate and rotate a new clipboard hash, sending it if appropriate
    void rotateClipboardKey(bool notifyClient);

    /// Generate an access token for this session via proxy protocol.
    const std::string &getOrCreateProxyAccess();

#if ENABLE_FEATURE_LOCK
    void sendLockedInfo();
#endif

#if ENABLE_FEATURE_RESTRICTION
    void sendRestrictionInfo();
#endif

    /// Process an SVG to replace embedded file:/// media URIs with public http URLs.
    std::string processSVGContent(const std::string& svg);

    CanonicalViewId getCanonicalViewId() const { return _canonicalViewId; }

    bool getSentBrowserSetting() const { return _sentBrowserSetting; }

    void setSentBrowserSetting(const bool sentBrowserSetting)
    {
        _sentBrowserSetting = sentBrowserSetting;
    }

    void setBrowserSettingsJSON(const Poco::SharedPtr<Poco::JSON::Object>& jsonObject)
    {
        _browserSettingsJSON = jsonObject;
    }

    Poco::SharedPtr<Poco::JSON::Object> getBrowserSettingJSON() const
    {
        return _browserSettingsJSON;
    }

    void uploadBrowserSettingsToWopiHost();

    void setViewSettingsJSON(const Poco::SharedPtr<Poco::JSON::Object>& jsonObject)
    {
        _viewSettingsJSON = jsonObject;
    }

    Poco::SharedPtr<Poco::JSON::Object> getViewSettingsJSON() const
    {
        return _viewSettingsJSON;
    }

    void uploadViewSettingsToWopiHost();

    /// Override parsedDocOption values we get from browser setting json
    /// Because when client sends `load url` it doesn't have information about browser setting json
    void overrideDocOption();

#if !MOBILEAPP
    void updateBrowserSettingsJSON(const std::string& json);
#endif

private:
    std::shared_ptr<ClientSession> client_from_this()
    {
        return std::static_pointer_cast<ClientSession>(shared_from_this());
    }

    /// SocketHandler: disconnection event.
    void onDisconnect() override;

    /// Does SocketHandler: have messages to send ?
    bool hasQueuedMessages() const override;

    /// SocketHandler: send those messages
    void writeQueuedMessages(std::size_t capacity) override;

    virtual bool _handleInput(const char* buffer, int length) override;

    bool handleSignatureAction(const StringVector& tokens);

    bool handleAIAction(const StringVector& tokens);

    bool handleAIChatAction(const std::string& firstLine);
    bool handleAIChatCancel(const std::string& firstLine);
    bool handleAIChatApprove(const std::string& firstLine);
    bool handleUpdateViewSettings(const std::string& firstLine);
    void sendAIChatResult(bool success, const std::string& text,
                          const std::string& requestId);

    bool handleAIImageGeneration(const std::string& prompt,
                                  const std::string& requestId);

    /// Start generating images for GenerateImage.N commands in a transform,
    /// then forward the modified transform to the kit.
    void processTransformImageGenerations(const std::shared_ptr<DocumentBroker>& docBroker);
    void generateNextTransformImage(std::shared_ptr<DocumentBroker> docBroker);
    std::string appendImageGenFailures(const std::string& result) const;

    Poco::JSON::Array::Ptr buildAIToolDefinitions() const;
    void callLLMAPI();
    void handleLLMResponse(const std::string& responseBody);
    bool executeAIToolCall(const std::string& toolCallId,
                           const std::string& fnName,
                           const std::string& argsJson);
    void processNextPendingToolCall();
    void continueAIToolLoop(const std::string& toolCallId,
                            const std::string& result);
    void sendAIToolProgress(const std::string& toolName,
                            const std::string& status);
    void sendAIToolApproval(const std::string& toolName,
                            const std::string& description);

    /// Map an HTTP status code from an AI API response to a user-facing error string.
    static std::string mapAIHttpStatusToError(http::StatusCode statusCode,
                                              const std::string& reasonPhrase,
                                              const std::string& context = "");

    bool loadDocument(const char* buffer, int length, const StringVector& tokens,
                      const std::shared_ptr<DocumentBroker>& docBroker);
    bool getStatus(const char* buffer, int length,
                   const std::shared_ptr<DocumentBroker>& docBroker);
    bool getCommandValues(const char* buffer, int length, const StringVector& tokens,
                          const std::shared_ptr<DocumentBroker>& docBroker);
    bool sendTile(const char* buffer, int length, const StringVector& tokens,
                  const std::shared_ptr<DocumentBroker>& docBroker);
    bool sendCombinedTiles(const char* buffer, int length, const StringVector& tokens,
                           const std::shared_ptr<DocumentBroker>& docBroker);

    bool handleGetSlideRequest(const StringVector& tokens,
                               const std::shared_ptr<DocumentBroker>& docBroker);

    bool forwardToChild(const std::string& message,
                        const std::shared_ptr<DocumentBroker>& docBroker);

    bool forwardToClient(const std::shared_ptr<Message>& payload);

    /// Returns true if given message from the client should be allowed or not
    /// Eg. in readonly mode only few messages should be allowed
    bool filterMessage(const std::string& msg) const;

    /// Returns true if the download message of type 'id' should be allowed or not
    bool filterDownloadAs(const std::string& id) const;

    void dumpState(std::ostream& os) override;

    /// Handle invalidation message coming from a kit and transfer it to a tile request.
    void handleTileInvalidation(const std::string& message,
                                const std::shared_ptr<DocumentBroker>& docBroker);

    bool isTileInsideVisibleArea(const TileDesc& tile) const;

    /// If this session is read-only because of failed lock, try to unlock and make it read-write.
    bool attemptLock(const std::shared_ptr<DocumentBroker>& docBroker);

    std::string getIsAdminUserStatus() const;

    /// Abort conversion due to failure.
    void abortConversion(const std::shared_ptr<DocumentBroker>& docBroker,
                         const std::shared_ptr<StreamSocket>& saveAsSocket, std::string errorKind);

#if !MOBILEAPP

    /// Handles saveas: and exportas: in handleKitToClientMessage.
    bool handleSaveAs(const std::shared_ptr<Message>& payload,
                      const std::shared_ptr<DocumentBroker>& docBroker,
                      const std::shared_ptr<StreamSocket>& saveAsSocket);
#endif // !MOBILEAPP

private:
    /// URI with which client made request to us
    const Poco::URI _uriPublic;

    const AdditionalFilePocoUris _additionalFileUrisPublic;

    SenderQueue<std::shared_ptr<Message>> _senderQueue;

    /// Requested tiles are stored in this list, before we can send them to the client
    std::deque<TileDesc> _requestedTiles;

    /// How to find our service from the client.
    const ServerURL _serverURL;

    /// Authorization data - either access_token or access_header.
    Authorization _auth;

    /// Rotating clipboard remote access identifiers - protected by GlobalSessionMapMutex
    std::string _clipboardKeys[2];

    /// Target used for thumbnail rendering
    std::string _thumbnailTarget;

    /// Secure session id token for proxyprotocol authentication
    std::string _proxyAccess;

    /// Store last sent payload of form field button, so we can filter out redundant messages.
    std::string _lastSentFormFielButtonMessage;

    std::weak_ptr<DocumentBroker> _docBroker;

    /// The socket to which the converted (saveas) doc is sent.
    std::weak_ptr<StreamSocket> _saveAsSocket;

    /// Time of last state transition
    std::chrono::steady_clock::time_point _lastStateTime;

    /// Wopi FileInfo object
    std::unique_ptr<WopiStorage::WOPIFileInfo> _wopiFileInfo;

    /// wire-ids's of the in-flight tiles. Push by sending and pop by tileprocessed message from the client.
    std::vector<std::pair<TileWireId, std::chrono::steady_clock::time_point>> _tilesOnFly;

    /// Sockets to send binary selection content to
    std::vector<std::weak_ptr<StreamSocket>> _clipSockets;

    /// Delta tile tracker.
    ClientDeltaTracker _tracker;

    /// Visible area of the client
    Util::Rectangle _clientVisibleArea;

    Poco::SharedPtr<Poco::JSON::Object> _browserSettingsJSON;

    /// Time when loading of view started
    std::chrono::steady_clock::time_point _viewLoadStart;

    /// Count of key-strokes
    uint64_t _keyEvents;

    /// Epoch of the client's performance.now() function, as microseconds since Unix epoch
    uint64_t _performanceCounterEpoch;

    /// Split position that defines the current split panes
    int _splitX;
    int _splitY;

    /// Selected part of the document viewed by the client (no parts in Writer)
    int _clientSelectedPart;

    /// Selected mode of the presentation viewed by the client (in Impress)
    int _clientSelectedMode;

    /// Zoom properties of the client
    int _tileWidthPixel;
    int _tileHeightPixel;
    int _tileWidthTwips;
    int _tileHeightTwips;

    /// The integer id of the view in the Kit process
    int _kitViewId;

    /// the canonical id unique to the set of rendering properties of this session
    CanonicalViewId _canonicalViewId;

    // Position used for thumbnail rendering
    std::pair<int, int> _thumbnailPosition;

    /// The phase of our lifecycle that we're in.
    SessionState _state;

    /// Whether this session is the owner of currently opened document
    bool _isDocumentOwner;

    /// If it is allowed to try to switch from read-only to edit mode,
    /// because it's read-only just because of transient lock failure.
    bool _isLockFailed = false;

    /// Client is using a text document?
    bool _isTextDocument;

    /// Session used to generate thumbnail
    bool _thumbnailSession;

    // Saves time from setting/fetching user info multiple times using zotero API
    bool _isZoteroUserInfoSet = false;

    /// If server audit was already sent
    bool _sentAudit;

    /// If browser setting was already sent
    bool _sentBrowserSetting;

    /// If Session is for convert-to
    bool _isConvertTo;

    Poco::SharedPtr<Poco::JSON::Object> _viewSettingsJSON;

    /// Active AI chat HTTP session for cancellation support
    std::shared_ptr<http::Session> _activeAIChatSession;

    /// AI tool loop state for multi-round LLM tool calling
    std::unique_ptr<AIToolLoopState> _aiToolLoop;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
