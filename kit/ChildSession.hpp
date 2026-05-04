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

#pragma once

#include <common/Session.hpp>
#include <kit/Kit.hpp>
#include <kit/StateRecorder.hpp>
#include <kit/Watermark.hpp>

#include <chrono>
#include <queue>

namespace kit
{
class Document;
class Office;
}

class Document;
class ChildSession;

struct LogUiCommandsLine {
    std::chrono::steady_clock::time_point _timeStart;
    std::chrono::steady_clock::time_point _timeEnd;
    int _repeat = 0;
    int _undoChange = 0;
    std::string _cmd;
    std::string _subCmd;
};

class LogUiCommands {
public:
    ChildSession& _session;
    int _lastUndoCount = 0;
    const StringVector* _tokens;
    LogUiCommands(ChildSession& session, const StringVector* tokens);
    LogUiCommands(ChildSession& session) : _session(session),_tokens(nullptr) {}
    ~LogUiCommands();
    void logSaveLoad(std::string cmd, const std::string & path, std::chrono::steady_clock::time_point timeStart);
private:
    std::weak_ptr<kit::Document> _document;
    // list the commands to log here.
    std::set<std::string> _cmdToLog = {
        "uno", "key", "mouse", "textinput", "removetextcontext",
        "paste", "insertfile", "dialogevent" };
    // list the the uno commands here, that are not to log. It will search these strings as a prefixes
    std::set<std::string> _unoCmdToNotLog = {
        ".uno:SidebarShow", ".uno:ToolbarMode" };
    void logLine(LogUiCommandsLine &line, bool isUndoChange=false);
};

enum class LokEventTargetEnum: std::uint8_t
{
    Document,
    Window
};

class SlideCompressor;

/// Represents a session to the WSD process, in a Kit process. Note that this is not a singleton.
class ChildSession final : public Session
{
public:
    static bool NoCapsForKit;

    /// Create a new ChildSession
    /// jailId The JailID of the jail root directory,
    //         used by downloadas to construct jailed path.
    ChildSession(
        const std::shared_ptr<ProtocolHandlerInterface> &protocol,
        const std::string& id,
        const std::string& jailId,
        const std::string& jailRoot,
        Document& document);
    virtual ~ChildSession();

    bool getStatus();
    bool getPartStatus();
    int getViewId() const { return _viewId; }
    void setViewId(const int viewId) { _viewId = viewId; }
    const std::string& getViewUserId() const { return getUserId(); }
    const std::string& getViewUserName() const { return getUserName(); }
    const std::string& getViewUserExtraInfo() const { return getUserExtraInfo(); }
    const std::string& getViewUserPrivateInfo() const { return getUserPrivateInfo(); }
    void updateSpeed();
    int getSpeed();
    bool isDocLoaded() const { return _isDocLoaded; }

    void loKitCallback(int type, const std::string& payload);

    /// Initializes the watermark support, if enabled and required.
    /// Returns true if watermark is enabled and initialized.
    bool initWatermark()
    {
        if (hasWatermark())
        {
            _docWatermark = std::make_shared<Watermark>(getLOKitDocument(), getWatermarkText(),
                                                        getWatermarkOpacity());
        }

        return _docWatermark != nullptr;
    }

    const std::shared_ptr<Watermark>& watermark() const { return _docWatermark; };

    bool sendTextFrame(const char* buffer, int length) override
    {
        if (_docManager == nullptr)
        {

            LOG_TRC("No DocManager; dropping message to client-"
                    << getId() << ": " << std::string_view(buffer, length));

            return false;
        }
        const auto msg = "client-" + getId() + ' ' + std::string(buffer, length);
        return _docManager->sendFrame(msg, WSOpCode::Text);
    }

    bool sendBinaryFrame(const char* buffer, int length) override
    {
        if (_docManager == nullptr)
        {
            LOG_TRC("No DocManager; dropping binary to client-" << getId());

            return false;
        }
        const auto msg = "client-" + getId() + ' ' + std::string(buffer, length);
        return _docManager->sendFrame(msg, WSOpCode::Binary);
    }

    bool sendProgressFrame(const char* id, const std::string& jsonProps,
                           const std::string& forcedID = "");

    using Session::sendTextFrame;

    bool getClipboard(const StringVector& tokens);

    void resetDocManager()
    {
        disconnect();
        _docManager = nullptr;
    }

    // Only called by kit.
    void setCanonicalViewId(CanonicalViewId viewId) { _canonicalViewId = viewId; }

    CanonicalViewId getCanonicalViewId() const { return _canonicalViewId; }

    void setViewRenderState(const std::string& state) { _viewRenderState = state; }

    bool getDumpTiles() const { return _isDumpingTiles; }

    void setDumpTiles(bool dumpTiles) { _isDumpingTiles = dumpTiles; }

    const std::string& getViewRenderState() const { return _viewRenderState; }

    TilePrioritizer::Priority getTilePriority(const TileDesc &desc) const;

    void saveLogUiBackground()
#if defined(BUILDING_TESTS)
    {}
#else
    ;
#endif

private:
    bool loadDocument(const StringVector& tokens);
    bool saveDocumentBackground(const StringVector &tokens);

    bool getCommandValues(const StringVector& tokens);

    bool clientZoom(const StringVector& tokens);
    bool clientVisibleArea(const StringVector& tokens);
    bool outlineState(const StringVector& tokens);
    bool downloadAs(const StringVector& tokens);
    bool getChildId();
    bool getTextSelection(const StringVector& tokens);
    bool setClipboard(const StringVector& tokens);
    std::string getTextSelectionInternal(const std::string& mimeType);
    bool paste(const char* buffer, int length, const StringVector& tokens);
    bool insertFile(const StringVector& tokens);
    bool keyEvent(const StringVector& tokens, LokEventTargetEnum target);
    bool extTextInputEvent(const StringVector& tokens);
    bool dialogKeyEvent(const char* buffer, int length, const std::vector<std::string>& tokens);
    bool mouseEvent(const StringVector& tokens, LokEventTargetEnum target);
    bool gestureEvent(const StringVector& tokens);
    bool dialogEvent(const StringVector& tokens);
    bool completeFunction(const StringVector& tokens);
    bool unoCommand(const StringVector& tokens);
    bool unoSignatureCommand(std::string_view commandName);
    bool selectText(const StringVector& tokens, LokEventTargetEnum target);
    bool selectGraphic(const StringVector& tokens);
    bool renderNextSlideLayer(SlideCompressor& scomp, unsigned width, unsigned height,
                              double devicePixelRatio, bool& done, const std::string& cacheKey,
                              bool isCompressed);
    bool renderSlide(const StringVector& tokens);
    bool renderWindow(const StringVector& tokens);
    bool resizeWindow(const StringVector& tokens);
    bool resetSelection(const StringVector& tokens);
    bool saveAs(const StringVector& tokens);
    bool exportAs(const StringVector& tokens);
    bool setClientPart(const StringVector& tokens);
    bool selectClientPart(const StringVector& tokens);
    bool moveSelectedClientParts(const StringVector& tokens);
    bool setPage(const StringVector& tokens);
    bool sendWindowCommand(const StringVector& tokens);
    bool askSignatureStatus(const char* buffer, int length, const StringVector& tokens);
    bool renderShapeSelection(const StringVector& tokens);
    bool removeTextContext(const StringVector& tokens);
#if ENABLE_FEATURE_LOCK || ENABLE_FEATURE_RESTRICTION || ENABLE_DEBUG
    bool updateBlockingCommandStatus(const StringVector& tokens);
    std::string getBlockedCommandType(const std::string& command);
#endif
    bool handleZoteroMessage(const StringVector& tokens);
    bool formFieldEvent(const char* buffer, int length, const StringVector& tokens);
    bool contentControlEvent(const StringVector& tokens);
    bool renderSearchResult(const char* buffer, int length, const StringVector& tokens);
    bool setAccessibilityState(bool enable);
    bool getA11yFocusedParagraph();
    bool getA11yCaretPosition();
    bool getPresentationInfo();
    bool executeScript(char const * buffer, int length, StringVector const & tokens);

    void rememberEventsForInactiveUser(int type, const std::string& payload);

    virtual void disconnect() override;
    virtual bool _handleInput(const char* buffer, int length) override;

    static void dumpRecordedUnoCommands();

    std::shared_ptr<kit::Document> getLOKitDocument() const
    {
        return _docManager->getLOKitDocument();
    }

    std::shared_ptr<kit::Office> getLOKit() const
    {
        return _docManager->getLOKit();
    }

    std::string getLOKitLastError() const
    {
        char *lastErr = _docManager->getLOKit()->getError();
        std::string ret;
        if (lastErr != nullptr)
        {
            ret = std::string(lastErr, strlen(lastErr));
            free (lastErr);
        }
        return ret;
    }

    void updateCursorPosition(const std::string &rect);
    void updateCursorPositionJSON(const std::string &payload);
    std::string getJailDocRoot() const;
    std::string getZoomPercent(const std::string &payload);

public:
    // simple one line for priming
    std::string getActivityState()
    {
        std::stringstream ss;
        ss << "view: " << _viewId
           << ", session " << getId()
           << (isReadOnly() ? ", ro": ", rw")
           << ", user: '" << getUserNameAnonym() << "'"
           << ", load" << (_isDocLoaded ? "ed" : "ing")
           << ", type: " << _docType
           << ", lang: " << getLang();
        return ss.str();
    }

    void dumpState(std::ostream& oss) override
    {
        Session::dumpState(oss);

        oss << "\n\tviewId: " << _viewId
            << "\n\tpart: " << _currentPart
            << "\n\tcursor: " << _cursorPosition.toString()
            << "\n\tcanonicalViewId: " << _canonicalViewId
            << "\n\tisDocLoaded: " << _isDocLoaded
            << "\n\tdocType: " << _docType
            << "\n\tcopyingToClipboard: " << _copyToClipboard
            << "\n\tdocType: " << _docType
            // FIXME: _pixmapCache
            << "\n\texportAsWopiUrl: " << _exportAsWopiUrl
            << "\n\tviewRenderedState: " << _viewRenderState
            << "\n\tisDumpingTiles: " <<_isDumpingTiles
            << "\n\tclientVisibleArea: " << _clientVisibleArea.toString()
            << "\n\thasURP: " << _hasURP
            << "\n\tURPContext?: " << (_URPContext == nullptr)
            << '\n';

        _stateRecorder.dumpState(oss);
    }

private:
    const std::string _jailId;
    const std::string _jailRoot;
    Document* _docManager;

    std::shared_ptr<Watermark> _docWatermark;

    std::queue<std::chrono::steady_clock::time_point> _cursorInvalidatedEvent;
    static constexpr std::chrono::seconds EventStorageInterval{ 15 };

    /// View ID, returned by createView() or 0 by default.
    int _viewId;

    /// Currently visible part
    int _currentPart;

    /// Last known position of a cursor for prioritizing rendering
    Util::Rectangle _cursorPosition;

    /// Whether document has been opened successfully
    bool _isDocLoaded;

    std::string _docType;

    StateRecorder _stateRecorder;

    /// If we are copying to clipboard.
    bool _copyToClipboard;

    std::vector<uint64_t> _pixmapCache;

    /// How many sessions / clients we have
    static size_t NumSessions;

    /// stores wopi url for export as operation
    std::string _exportAsWopiUrl;

    /// stores info about the view
    std::string _viewRenderState;

    /// the canonical id unique to the set of rendering properties of this session
    CanonicalViewId _canonicalViewId;

    /// whether we are dumping tiles as they are being drawn
    bool _isDumpingTiles;

    Util::Rectangle _clientVisibleArea;

    void* _URPContext;

    /// whether there is a URP session created for this ChildSession
    bool _hasURP;

    // When state is added - please update dumpState above.

    friend class LogUiCommands;
    int _lastUiCmdLinesLoggedCount = 0;
    LogUiCommandsLine _lastUiCmdLinesLogged[2];
    std::chrono::steady_clock::time_point _logUiSaveBackGroundTimeStart;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
