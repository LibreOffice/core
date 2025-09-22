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
 * Kit process child session handling COKit commands.
 * Classes: ChildSession - Document session command processing
 */

#include <config.h>

#include "ChildSession.hpp"

#include <common/Anonymizer.hpp>
#include <common/Clipboard.hpp>
#include <common/CommandControl.hpp>
#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/HexUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/NumUtil.hpp>
#include <common/Png.hpp>
#include <common/SpookyV2.h>
#include <common/TraceEvent.hpp>
#include <common/Unit.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <common/base64.hpp>
#include <kit/KitHelper.hpp>
#include <kit/SlideCompressor.hpp>

#define KIT_USE_UNSTABLE_API
#include <COKit/COKit.hxx>
#include <COKit/COKitEnums.h>

#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/BinaryReader.h>
#if !MOBILEAPP
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#endif

#ifdef __ANDROID__
#include <androidapp.hpp>
#endif

#ifdef IOS
#include "DocumentViewController.h"
#endif

#if WASMAPP
#include <wasmapp.hpp>
#endif

#include <cassert>
#include <climits>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <zlib.h>
#include <zstd.h>

using Poco::JSON::Object;
using Poco::JSON::Parser;
using Poco::URI;

using namespace COOLProtocol;

bool ChildSession::NoCapsForKit = false;

namespace {

/// Formats the uno command information for logging
std::string formatUnoCommandInfo(const std::string_view unoCommand)
{
    // E.g. '2023-09-06 12:19:32', matching systemd format.
    std::string recorded_time = Util::getTimeNow("%Y-%m-%d %T");

    std::string unoCommandInfo;
    unoCommandInfo.reserve(unoCommand.size() * 2);

    // unoCommand(sessionId) : command - time
    unoCommandInfo.append("unoCommand");
    unoCommandInfo.append(" : ");
    unoCommandInfo.append(Util::eliminatePrefix(unoCommand,".uno:"));
    unoCommandInfo.append(" - ");
    unoCommandInfo.append(std::move(recorded_time));

    return unoCommandInfo;
}

}

ChildSession::ChildSession(const std::shared_ptr<ProtocolHandlerInterface>& protocol,
                           const std::string& id, const std::string& jailId,
                           const std::string& jailRoot, Document& docManager)
    : Session(protocol, "ToMaster-" + id, id, false)
    , _jailId(jailId)
    , _jailRoot(jailRoot)
    , _docManager(&docManager)
    , _viewId(-1)
    , _currentPart(-1)
    , _isDocLoaded(false)
    , _copyToClipboard(false)
    , _canonicalViewId(CanonicalViewId::Invalid)
    , _isDumpingTiles(false)
    , _clientVisibleArea(0, 0, 0, 0)
    , _URPContext(nullptr)
    , _hasURP(false)
{
#if !MOBILEAPP
    if (isURPEnabled())
    {
        LOG_WRN("URP is enabled in the config: Starting a URP tunnel for this session ["
                << getName() << "]");

        _hasURP = startURP(docManager.getLOKit(), &_URPContext);

        if (!_hasURP)
            LOG_INF("Failed to start a URP bridge for this session [" << getName()
                                                                      << "], disabling URP");
    }
#endif
    LOG_INF("ChildSession ctor [" << getName() << "]. JailRoot: [" << _jailRoot << ']');
}

ChildSession::~ChildSession()
{
    LOG_INF("~ChildSession dtor [" << getName() << ']');
    disconnect();

    if (_hasURP)
    {
        _docManager->getLOKit()->stopURP(_URPContext);
    }
}

void ChildSession::disconnect()
{
    if (!isDisconnected())
    {
        if (_viewId >= 0)
        {
            if (_docManager != nullptr)
            {
                _docManager->onUnload(*this);

                // Notify that we've unloaded this view.
                std::ostringstream oss;
                oss << "unloaded: viewid=" << _viewId
                    << " views=" << _docManager->getViewsCount();
                sendTextFrame(oss.str());
            }
        }
        else
        {
            LOG_WRN("Skipping unload on incomplete view [" << getName()
                                                           << "], viewId: " << _viewId);
        }

// This shuts down the shared socket, which is not what we want.
//        Session::disconnect();
    }
}

namespace
{
    // disable Watchdog for scope
    class WatchdogGuard
    {
    public:
        WatchdogGuard()
        {
            // disable watchdog - we want to just watch interactive responsiveness
            if (KitSocketPoll* kitPoll = KitSocketPoll::getMainPoll())
                kitPoll->disableWatchdog();
        }

        ~WatchdogGuard()
        {
            // reenable watchdog
            if (KitSocketPoll* kitPoll = KitSocketPoll::getMainPoll())
                kitPoll->enableWatchdog();
        }
    };
}

bool ChildSession::_handleInput(const char *buffer, int length)
{
    LOG_TRC("handling [" << getAbbreviatedMessage(buffer, length) << ']');
    const std::string firstLine = getFirstLine(buffer, length);
    const StringVector tokens = StringVector::tokenize(firstLine.data(), firstLine.size());

    // if _clientVisibleArea.getWidth() == 0, then it is probably not a real user.. probably is a convert-to or similar
    LogUiCommands logUndoRelatedcommandAtfunctionEnd(*this, &tokens);
    if (_isDocLoaded && Log::isLogUIEnabled() && _clientVisibleArea.getWidth() != 0)
    {
        LOKitHelper::ScopedString undoCountString(getLOKitDocument()->getCommandValues(".uno:UndoCount"));
        logUndoRelatedcommandAtfunctionEnd._lastUndoCount = undoCountString ? atoi(undoCountString.get()) : 0;
    }

    if (COOLProtocol::tokenIndicatesUserInteraction(tokens[0]))
    {
        // Keep track of timestamps of incoming client messages that indicate user activity.
        updateLastActivityTime();
    }

    if (tokens.size() > 0 && tokens.equals(0, "useractive") && getLOKitDocument() != nullptr)
    {
        LOG_DBG("Handling message after inactivity of " << getInactivityMS());
        setIsActive(true);

        // Client is getting active again.
        // Send invalidation and other sync-up messages.
        getLOKitDocument()->setView(_viewId);

        int curPart = 0;
        if (getLOKitDocument()->getDocumentType() != KIT_DOCTYPE_TEXT)
            curPart = getLOKitDocument()->getPart();

        // Notify all views about updated view info
        _docManager->notifyViewInfo();

        if (getLOKitDocument()->getDocumentType() != KIT_DOCTYPE_TEXT)
        {
            sendTextFrame("curpart: part=" + std::to_string(curPart));
            sendTextFrame("setpart: part=" + std::to_string(curPart));
        }

        // Invalidate if we have to
        // TODO instead just a "_invalidate" flag, we should remember / grow
        // the rectangle to invalidate; invalidating everything is sub-optimal
        if (_stateRecorder.isInvalidate())
        {
            const std::string payload = "0, 0, 1000000000, 1000000000, " + std::to_string(curPart);
            loKitCallback(KIT_CALLBACK_INVALIDATE_TILES, payload);
        }

        for (const auto& viewPair : _stateRecorder.getRecordedViewEvents())
        {
            for (const auto& eventPair : viewPair.second)
            {
                const RecordedEvent& event = eventPair.second;
                LOG_TRC("Replaying missed view event: " << viewPair.first << ' '
                                                        << kitCallbackTypeToString(event.getType())
                                                        << ": " << event.getPayload());
                loKitCallback(event.getType(), event.getPayload());
            }
        }

        for (const auto& eventPair : _stateRecorder.getRecordedEvents())
        {
            const RecordedEvent& event = eventPair.second;
            LOG_TRC("Replaying missed event: " << kitCallbackTypeToString(event.getType()) << ": "
                                               << event.getPayload());
            loKitCallback(event.getType(), event.getPayload());
        }

        for (const auto& pair : _stateRecorder.getRecordedStates())
        {
            LOG_TRC("Replaying missed state-change: " << pair.second);
            loKitCallback(KIT_CALLBACK_STATE_CHANGED, pair.second);
        }

        for (const auto& event : _stateRecorder.getRecordedEventsVector())
        {
            LOG_TRC("Replaying missed event (part of sequence): " <<
                    kitCallbackTypeToString(event.getType()) << ": " << event.getPayload());
            loKitCallback(event.getType(), event.getPayload());
        }

        _stateRecorder.clear();

        LOG_TRC("Finished replaying messages.");
    }

    if (tokens.equals(0, "dummymsg"))
    {
        // Just to update the activity of a view-only client.
        return true;
    }
    else if (tokens.equals(0, "commandvalues"))
    {
        return getCommandValues(tokens);
    }
    else if (tokens.equals(0, "dialogevent"))
    {
        return dialogEvent(tokens);
    }
    else if (tokens.equals(0, "load"))
    {
        if (_isDocLoaded)
        {
            sendTextFrameAndLogError("error: cmd=load kind=docalreadyloaded");
            return false;
        }

        std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();

        // Disable processing of other messages while loading document
        InputProcessingManager processInput(getProtocol(), false);
        // disable watchdog while loading
        WatchdogGuard watchdogGuard;
        _isDocLoaded = loadDocument(tokens);

        LogUiCommands uiLog(*this);
        uiLog.logSaveLoad("load", Poco::URI(getJailedFilePath()).getPath(), timeStart);

        LOG_TRC("isDocLoaded state after loadDocument: " << _isDocLoaded);
        return _isDocLoaded;
    }
    else if (tokens.equals(0, "extractlinktargets"))
    {
        if (tokens.size() < 2)
        {
            sendTextFrameAndLogError("error: cmd=extractlinktargets kind=syntax");
            return false;
        }

        if (!_isDocLoaded)
        {
            sendTextFrameAndLogError("error: cmd=extractlinktargets kind=docnotloaded");
            return false;
        }

        assert(!getDocURL().empty());
        assert(!getJailedFilePath().empty());

        LOKitHelper::ScopedString data(_docManager->getLOKit()->extractRequest(getJailedFilePath().c_str()));
        if (!data)
        {
            LOG_TRC("extractRequest returned no data.");
            sendTextFrame("extractedlinktargets: { }");
            return false;
        }

        LOG_TRC("Extracted link targets: " << data);
        bool success = sendTextFrame("extractedlinktargets: " + std::string(data.get()));

        return success;
    }
    else if (tokens.equals(0, "extractdocumentstructure"))
    {
        if (tokens.size() < 2)
        {
            sendTextFrameAndLogError("error: cmd=extractdocumentstructure kind=syntax");
            return false;
        }

        if (!_isDocLoaded)
        {
            sendTextFrameAndLogError("error: cmd=extractdocumentstructure kind=docnotloaded");
            return false;
        }

        assert(!getDocURL().empty());
        assert(!getJailedFilePath().empty());

        std::string filter;
        if (tokens.size() > 2)
        {
            getTokenString(tokens[2], "filter", filter);
        }

        LOKitHelper::ScopedString data(_docManager->getLOKit()->extractDocumentStructureRequest(getJailedFilePath().c_str(),
                                                                              filter.c_str()));
        if (!data)
        {
            LOG_TRC("extractDocumentStructureRequest returned no data.");
            sendTextFrame("extracteddocumentstructure: { }");
            return false;
        }

        LOG_TRC("Extracted document structure: " << data);
        bool success = sendTextFrame("extracteddocumentstructure: " + std::string(data.get()));

        return success;
    }
    else if (tokens.equals(0, "transformdocumentstructure"))
    {
        if (tokens.size() < 3)
        {
            sendTextFrameAndLogError("error: cmd=transformdocumentstructure kind=syntax");
            return false;
        }

        if (!_isDocLoaded)
        {
            sendTextFrameAndLogError("error: cmd=transformdocumentstructure kind=docnotloaded");
            return false;
        }

        assert(!getDocURL().empty());
        assert(!getJailedFilePath().empty());

        const std::string command = ".uno:TransformDocumentStructure";

        std::string encodedTransformQueryJSON;
        getTokenString(tokens[2], "transform", encodedTransformQueryJSON);

        if (encodedTransformQueryJSON.empty())
        {
            LOG_TRC("Transformation JSON was not provided.");
            return false;
        }
        // Send encoded string, this way it survive until it arrive at core.
        const std::string arguments = "{"
            "\"DataJson\":{"
                "\"type\":\"string\","
                "\"value\":\"" + encodedTransformQueryJSON + "\""
            "}}";

        try
        {
            // For interactive sessions, request a notification so we get
            // the real success/failure via LOK_CALLBACK_UNO_COMMAND_RESULT.
            // For MCP/convert-to, the saveas flow returns the modified
            // document, so no notification is needed.
            std::string url;
            getTokenString(tokens[1], "url", url);
            bool interactive = (url == "interactive");
            getLOKitDocument()->postUnoCommand(command.c_str(), arguments.c_str(), interactive);
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("transformdocumentstructure: postUnoCommand failed: " << exc.what());
            sendTextFrameAndLogError("error: cmd=transformdocumentstructure kind=parseerror");
        }

        LOG_TRC("Transformation JSON application was requested.");

        return true;
    }
    else if (tokens.equals(0, "getthumbnail"))
    {
        if (tokens.size() < 3)
        {
            sendTextFrameAndLogError("error: cmd=getthumbnail kind=syntax");
            return false;
        }

        if (!_isDocLoaded)
        {
            sendTextFrameAndLogError("error: cmd=getthumbnail kind=docnotloaded");
            return false;
        }

        int x, y;
        if (!getTokenInteger(tokens[1], "x", x))
            x = 0;

        if (!getTokenInteger(tokens[2], "y", y))
            y = 0;

        bool success = false;

        // Size of thumbnail in pixels
        constexpr int width = 1200;
        constexpr int height = 630;

        // Unclear what this "zoom" level means
        constexpr float zoom = 2;

        // The magic number 15 is the number of twips per pixel for a resolution of 96 pixels per
        // inch, which apparently is some "standard".
        constexpr int widthTwips = width * 15 / zoom;
        constexpr int heightTwips = height * 15 / zoom;
        constexpr int offsetXTwips = 15 * 15; // start 15 pixels before the target to get a clearer thumbnail
        constexpr int offsetYTwips = 15 * 15;

        const auto mode = static_cast<COKitTileMode>(getLOKitDocument()->getTileMode());

        std::vector<unsigned char> thumbnail(width * height * 4);
        getLOKitDocument()->paintTile(thumbnail.data(), width, height, x - offsetXTwips, y - offsetYTwips, widthTwips, heightTwips);

        std::vector<char> pngThumbnail;
        if (Png::encodeBufferToPNG(thumbnail.data(), width, height, pngThumbnail, mode))
        {
            std::ostringstream oss;
            oss << "sendthumbnail:\n";
            oss.write(pngThumbnail.data(), pngThumbnail.size());

            std::string sendThumbnailCommand = oss.str();
            success = sendBinaryFrame(sendThumbnailCommand.data(), sendThumbnailCommand.size());
        }
        else
        {
            LOG_ERR("Encoding thumbnail failed.");
            std::string error = "sendthumbnail: error";
            sendTextFrame(error.data(), error.size());
            success = false;
        }

        return success;
    }
    else if (tokens.equals(0, "addconfig"))
    {
        Poco::Path presetsPath(JAILED_CONFIG_ROOT);
        getLOKit()->setOption("addconfig", Poco::URI(presetsPath).toString().c_str());
    }
    else if (!_isDocLoaded)
    {
        sendTextFrameAndLogError("error: cmd=" + tokens[0] + " kind=nodocloaded");
        return false;
    }
    else if (tokens.equals(0, "setclientpart"))
    {
        return setClientPart(tokens);
    }
    else if (tokens.equals(0, "selectclientpart"))
    {
        return selectClientPart(tokens);
    }
    else if (tokens.equals(0, "moveselectedclientparts"))
    {
        return moveSelectedClientParts(tokens);
    }
    else if (tokens.equals(0, "setpage"))
    {
        return setPage(tokens);
    }
    else if (tokens.equals(0, "status"))
    {
        return getStatus();
    }
    else if (tokens.equals(0, "getslide"))
    {
        return renderSlide(tokens);
    }
    else if (tokens.equals(0, "paintwindow"))
    {
        return renderWindow(tokens);
    }
    else if (tokens.equals(0, "resizewindow"))
    {
        return resizeWindow(tokens);
    }
    else if (tokens.equals(0, "tile") || tokens.equals(0, "tilecombine"))
    {
        assert(false && "Tile traffic should go through the DocumentBroker-LoKit WS.");
    }
    else if (tokens.equals(0, "blockingcommandstatus"))
    {
#if (ENABLE_FEATURE_LOCK || ENABLE_FEATURE_RESTRICTION || ENABLE_DEBUG) && !MOBILEAPP
        return updateBlockingCommandStatus(tokens);
#endif
    }
    else
    {
        // All other commands are such that they always require a COKitDocument session,
        // i.e. need to be handled in a child process.

        assert(Util::isFuzzing() ||
               tokens.equals(0, "clientzoom") ||
               tokens.equals(0, "clientvisiblearea") ||
               tokens.equals(0, "outlinestate") ||
               tokens.equals(0, "downloadas") ||
               tokens.equals(0, "getchildid") ||
               tokens.equals(0, "gettextselection") ||
               tokens.equals(0, "getclipboard") ||
               tokens.equals(0, "setclipboard") ||
               tokens.equals(0, "paste") ||
               tokens.equals(0, "insertfile") ||
               tokens.equals(0, "key") ||
               tokens.equals(0, "textinput") ||
               tokens.equals(0, "windowkey") ||
               tokens.equals(0, "mouse") ||
               tokens.equals(0, "windowmouse") ||
               tokens.equals(0, "windowgesture") ||
               tokens.equals(0, "uno") ||
               tokens.equals(0, "save") ||
               tokens.equals(0, "selecttext") ||
               tokens.equals(0, "windowselecttext") ||
               tokens.equals(0, "selectgraphic") ||
               tokens.equals(0, "resetselection") ||
               tokens.equals(0, "saveas") ||
               tokens.equals(0, "exportas") ||
               tokens.equals(0, "useractive") ||
               tokens.equals(0, "userinactive") ||
               tokens.equals(0, "windowcommand") ||
               tokens.equals(0, "asksignaturestatus") ||
               tokens.equals(0, "rendershapeselection") ||
               tokens.equals(0, "removetextcontext") ||
               tokens.equals(0, "dialogevent") ||
               tokens.equals(0, "completefunction")||
               tokens.equals(0, "formfieldevent") ||
               tokens.equals(0, "traceeventrecording") ||
               tokens.equals(0, "sallogoverride") ||
               tokens.equals(0, "rendersearchresult") ||
               tokens.equals(0, "contentcontrolevent") ||
               tokens.equals(0, "a11ystate") ||
               tokens.equals(0, "geta11yfocusedparagraph") ||
               tokens.equals(0, "geta11ycaretposition") ||
               tokens.equals(0, "toggletiledumping") ||
               tokens.equals(0, "getpresentationinfo"));

        ProfileZone pz("ChildSession::_handleInput:" + tokens[0]);
        if (tokens.equals(0, "clientzoom"))
        {
            return clientZoom(tokens);
        }
        else if (tokens.equals(0, "clientvisiblearea"))
        {
            return clientVisibleArea(tokens);
        }
        else if (tokens.equals(0, "outlinestate"))
        {
            return outlineState(tokens);
        }
        else if (tokens.equals(0, "downloadas"))
        {
            return downloadAs(tokens);
        }
        else if (tokens.equals(0, "getchildid"))
        {
            return getChildId();
        }
        else if (tokens.equals(0, "gettextselection")) // deprecated.
        {
            return getTextSelection(tokens);
        }
        else if (tokens.equals(0, "getclipboard"))
        {
            return getClipboard(tokens);
        }
        else if (tokens.equals(0, "setclipboard"))
        {
            return setClipboard(tokens);
        }
        else if (tokens.equals(0, "paste"))
        {
            return paste(buffer, length, tokens);
        }
        else if (tokens.equals(0, "insertfile"))
        {
            return insertFile(tokens);
        }
        else if (tokens.equals(0, "key"))
        {
            return keyEvent(tokens, LokEventTargetEnum::Document);
        }
        else if (tokens.equals(0, "textinput"))
        {
            return extTextInputEvent(tokens);
        }
        else if (tokens.equals(0, "windowkey"))
        {
            return keyEvent(tokens, LokEventTargetEnum::Window);
        }
        else if (tokens.equals(0, "mouse"))
        {
            return mouseEvent(tokens, LokEventTargetEnum::Document);
        }
        else if (tokens.equals(0, "windowmouse"))
        {
            return mouseEvent(tokens, LokEventTargetEnum::Window);
        }
        else if (tokens.equals(0, "windowgesture"))
        {
            return gestureEvent(tokens);
        }
        else if (tokens.equals(0, "uno"))
        {
            // SpellCheckApplySuggestion might contain non separator spaces
            if (tokens[1].find(".uno:SpellCheckApplySuggestion") != std::string::npos ||
                tokens[1].find(".uno:LanguageStatus") != std::string::npos)
            {
                StringVector newTokens;
                newTokens.push_back(tokens[0]);
                newTokens.push_back(firstLine.substr(4)); // Copy the remaining part.
                return unoCommand(newTokens);
            }
            else if (tokens[1].find(".uno:SaveGraphic") != std::string::npos)
            {
                // SaveGraphic is not a document save - it exports an image
                return unoCommand(tokens);
            }
            else if (tokens[1].find(".uno:Save") != std::string::npos)
            {
                LOG_ERR("Unexpected UNO Save command in client");
                // save should go through path below
                assert(false);
                return false;
            }
            else if (tokens[1].find(".uno:SetDocumentProperties") != std::string::npos && tokens.size() == 2)
            {
                // Don't append anything if command has any parameters
                // It maybe json and appending plain string makes everything broken
                std::string PossibleFileExtensions[3] = {"", TO_UPLOAD_SUFFIX + std::string(UPLOADING_SUFFIX), TO_UPLOAD_SUFFIX};
                for (size_t i = 0; i < 3; i++)
                {
                    const auto st = FileUtil::Stat(Poco::URI(getJailedFilePath()).getPath() + PossibleFileExtensions[i]);
                    if (st.exists())
                    {
                        const std::size_t size = (st.good() ? st.size() : 0);
                        std::string addedProperty = firstLine + "?FileSize:string=" + std::to_string(size);
                        StringVector newTokens = StringVector::tokenize(addedProperty.data(), addedProperty.size());
                        return unoCommand(newTokens);
                    }
                }
            }
            else if (tokens[1] == ".uno:Signature" || tokens[1] == ".uno:InsertSignatureLine")
            {
                // See if the command has parameters: if not, annotate with sign cert/key.
                if (tokens.size() == 2 && unoSignatureCommand(tokens[1]))
                {
                    // The command has been sent with parameters from user private info, done.
                    return true;
                }
            }

            return unoCommand(tokens);
        }
        else if (tokens.equals(0, "save"))
        {
            bool background = tokens[1] == "background=true";
            SigUtil::addActivity(getId(), (background ? "bg " : "") + firstLine);

            StringVector unoSave = StringVector::tokenize("uno .uno:Save " + tokens.cat(' ', 2));

            bool saving = false;
            if (background)
                saving = saveDocumentBackground(unoSave);

            if (!saving)
            { // fallback to foreground save

                if (!Util::isMobileApp())
                    UnitKit::get().preSaveHook();

                // Disable processing of other messages while saving document
                InputProcessingManager processInput(getProtocol(), false);
                // disable watchdog while saving
                WatchdogGuard watchdogGuard;

                std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();
                bool result = unoCommand(unoSave);
                if (result)
                {
#if WASMAPP
                    saveToServer();
#endif
                    LogUiCommands uiLog(*this);
                    uiLog.logSaveLoad("save", Poco::URI(getJailedFilePath()).getPath(), timeStart);
                }

                return result;
            }

            return true;
        }
        else if (tokens.equals(0, "selecttext"))
        {
            return selectText(tokens, LokEventTargetEnum::Document);
        }
        else if (tokens.equals(0, "windowselecttext"))
        {
            return selectText(tokens, LokEventTargetEnum::Window);
        }
        else if (tokens.equals(0, "selectgraphic"))
        {
            return selectGraphic(tokens);
        }
        else if (tokens.equals(0, "resetselection"))
        {
            return resetSelection(tokens);
        }
        else if (tokens.equals(0, "saveas"))
        {
            std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();
            bool result = saveAs(tokens);
            if (result)
            {
                LogUiCommands uiLog(*this);
                uiLog.logSaveLoad("saveas", Poco::URI(getJailedFilePath()).getPath(), timeStart);
            }
            return result;
        }
        else if (tokens.equals(0, "exportas"))
        {
            std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();
            bool result = exportAs(tokens);
            if (result)
            {
                LogUiCommands uiLog(*this);
                uiLog.logSaveLoad("exportas", Poco::URI(getJailedFilePath()).getPath(), timeStart);
            }
            return result;
        }
        else if (tokens.equals(0, "useractive"))
        {
            setIsActive(true);
        }
        else if (tokens.equals(0, "userinactive"))
        {
            setIsActive(false);
            _docManager->trimIfInactive();
        }
        else if (tokens.equals(0, "windowcommand"))
        {
            sendWindowCommand(tokens);
        }
        else if (tokens.equals(0, "asksignaturestatus"))
        {
            askSignatureStatus(buffer, length, tokens);
        }
        else if (tokens.equals(0, "rendershapeselection"))
        {
            return renderShapeSelection(tokens);
        }
        else if (tokens.equals(0, "removetextcontext"))
        {
            return removeTextContext(tokens);
        }
        else if (tokens.equals(0, "completefunction"))
        {
            return completeFunction(tokens);
        }
        else if (tokens.equals(0, "formfieldevent"))
        {
            return formFieldEvent(buffer, length, tokens);
        }
        else if (tokens.equals(0, "contentcontrolevent"))
        {
            return contentControlEvent(tokens);
        }
        else if (tokens.equals(0, "traceeventrecording"))
        {
            static const bool traceEventsEnabled =
                ConfigUtil::getBool("trace_event[@enable]", false);
            if (traceEventsEnabled)
            {
                if (tokens.size() > 0)
                {
                    if (tokens.equals(1, "start"))
                    {
                        getLOKit()->setOption("traceeventrecording", "start");
                        TraceEvent::startRecording();
                        LOG_INF("Trace Event recording in this Kit process turned on (might have been on already)");
                    }
                    else if (tokens.equals(1, "stop"))
                    {
                        getLOKit()->setOption("traceeventrecording", "stop");
                        TraceEvent::stopRecording();
                        LOG_INF("Trace Event recording in this Kit process turned off (might have been off already)");
                    }
                }
            }
        }
        else if (tokens.equals(0, "sallogoverride"))
        {
            if (tokens.empty() || tokens.equals(1, "default"))
            {
                getLOKit()->setOption("sallogoverride", nullptr);
            }
            else if (tokens.size() > 0 && tokens.equals(1, "off"))
            {
                getLOKit()->setOption("sallogoverride", "-WARN-INFO");
            }
            else if (tokens.size() > 0)
            {
                getLOKit()->setOption("sallogoverride", tokens[1].c_str());
            }
        }
        else if (tokens.equals(0, "rendersearchresult"))
        {
            return renderSearchResult(buffer, length, tokens);
        }
        else if (tokens.equals(0, "a11ystate"))
        {
            return setAccessibilityState(tokens[1] == "true");
        }
        else if (tokens.equals(0, "geta11yfocusedparagraph"))
        {
            return getA11yFocusedParagraph();
        }
        else if (tokens.equals(0, "geta11ycaretposition"))
        {
            return getA11yCaretPosition();
        }
        else if (tokens.equals(0, "toggletiledumping"))
        {
            setDumpTiles(tokens[1] == "true");
        }
        else if (tokens.equals(0, "getpresentationinfo"))
        {
            return getPresentationInfo();
        }
        else
        {
            assert(Util::isFuzzing() && "Unknown command token.");
        }
    }

    return true;
}

std::string getMimeFromFileType(const std::string & fileType)
{
    if (fileType == "pdf")
        return "application/pdf";
    else if (fileType == "odt")
        return "application/vnd.oasis.opendocument.text";
    else if (fileType == "docx")
        return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";

    return std::string();
}

namespace {

    /**
     * Create the 'upload' file regardless of success or failure,
     * because we don't know if the last upload worked or not.
     * DocBroker will have to decide to upload or skip.
     */
    [[maybe_unused]]
    void copyForUpload(const std::string& url)
    {
        const std::string oldName = Poco::URI(url).getPath();
        const std::string newName = oldName + TO_UPLOAD_SUFFIX;
        if (!FileUtil::copyAtomic(oldName, newName, /*preserveTimestamps=*/true))
        {
            // It's not an error if there was no file to copy, when the document isn't modified.
            LOG_TRC_SYS("Failed to copy [" << oldName << "] to [" << newName << ']');
        }
        else
        {
            LOG_TRC("Copied [" << oldName << "] to [" << newName << ']');
        }
    }
}

bool ChildSession::loadDocument(const StringVector& tokens)
{
    int part = -1;
    if (tokens.size() < 2)
    {
        sendTextFrameAndLogError("error: cmd=load kind=syntax");
        return false;
    }

    std::string timestamp;
    parseDocOptions(tokens, part, timestamp);

    std::string renderOpts;
    if (!getDocOptions().empty())
    {
        Parser parser;
        Poco::Dynamic::Var var = parser.parse(getDocOptions());
        const Object::Ptr& object = var.extract<Object::Ptr>();
        Poco::Dynamic::Var rendering = object->get("rendering");
        if (!rendering.isEmpty())
            renderOpts = rendering.toString();
    }

    assert(!getDocURL().empty());
    assert(!getJailedFilePath().empty());

#if ENABLE_DEBUG && !MOBILEAPP
    if (std::getenv("PAUSEFORDEBUGGER"))
    {
        std::cerr << getDocURL()
                  << " paused waiting for a debugger to attach: " << ProcUtil::getProcessId()
                  << std::endl;
        SigUtil::setDebuggerSignal();
        pause();
    }
#endif

    SigUtil::addActivity(getId(), "load doc: " + getJailedFilePathAnonym());

    // Note: _isDocLoaded is set on our return.
    const bool isFirstView = !_docManager->isLoaded();

    const bool loaded = _docManager->onLoad(getId(), getJailedFilePathAnonym(), renderOpts);
    if (!loaded || _viewId < 0)
    {
        // Failed and communicated with the reason; do not send errors to the client.
        LOG_ERR("Failed to get LoKitDocument instance for [" << getJailedFilePathAnonym() << ']');
        return false;
    }

    assert(getLOKitDocument() && "Expected valid LOKitDocument instance");
    LOG_INF("Created new view with viewid: [" << _viewId << "] for username: ["
                                              << getUserNameAnonym() << "] in session: [" << getId()
                                              << "], template: [" << getDocTemplate() << ']');

    if (!getDocTemplate().empty())
    {
        // If we aren't chroot-ed, we need to use the absolute path.
        // Because that's where Storage in WSD expects the document.
        std::string url;
        if (!_jailRoot.empty())
        {
            static constexpr std::string_view Protocol = "file://";

            url = std::string(Protocol) + _jailRoot;
            if (getJailedFilePath().starts_with(url))
                url = getJailedFilePath(); // JailedFilePath is already the absolute path.
            else if (getJailedFilePath().starts_with(Protocol))
                url += getJailedFilePath().substr(Protocol.size());
            else
                url += getJailedFilePath();
        }
        else
            url += getJailedFilePath();

        LOG_INF("Saving the template document after loading to ["
                << url << "], jailRoot: [" << _jailRoot << "], jailedFilePath: ["
                << getJailedFilePath() << ']');

        const bool success = getLOKitDocument()->saveAs(url.c_str(), nullptr, "TakeOwnership,FromTemplate");
        if (!success)
        {
            LOG_ERR("Failed to save template [" << url << ']');
            return false;
        }

        if constexpr (!Util::isMobileApp())
            copyForUpload(url);
    }

    getLOKitDocument()->setView(_viewId);

    _docType = LOKitHelper::getDocumentTypeAsString(getLOKitDocument()->get());
    if (_docType != "text" && part != -1)
    {
        getLOKitDocument()->setPart(part);
        _currentPart = part;
    }
    else
        _currentPart = getLOKitDocument()->getPart();

    // Respond by the document status
    LOG_DBG("Sending status after loading view " << _viewId);
    const std::string status = LOKitHelper::documentStatus(getLOKitDocument()->get());
    if (status.empty() || !sendTextFrame("status: " + status))
    {
        LOG_ERR("Failed to get/forward document status [" << status << ']');
        return false;
    }

    // Inform everyone (including this one) about updated view info
    _docManager->notifyViewInfo();
    sendTextFrame("editor: " + std::to_string(_docManager->getEditorId()));

    // now we have the doc options parsed and set.
    _docManager->updateActivityHeader();

    // Notify that we've loaded this view.
    std::ostringstream oss;
    oss << "loaded: viewid=" << _viewId << " views=" << _docManager->getViewsCount()
        << " isfirst=" << (isFirstView ? "true" : "false");
    sendTextFrame(oss.str());

    LOG_INF("Loaded session " << getId());
    return true;
}

// attempt to shutdown threads, fork and execute in the background
bool ChildSession::saveDocumentBackground([[maybe_unused]] const StringVector& tokens)
{
    if constexpr (!Util::isMobileApp())
    {
        LOG_TRC("Attempting background save");
        _logUiSaveBackGroundTimeStart = std::chrono::steady_clock::now();

        // Keep the session alive over the lifetime of an async save
        if (_docManager->forkToSave(
                [this, tokens]
                {
                    // Called back in the bgsave process: so do the save !

                    // FIXME: re-directing our sockets perhaps over
                    // a pipe to our parent process ?
                    unoCommand(tokens);

                    // FIXME: did we send our responses properly ? ...
                    SigUtil::addActivity("async save process exiting");

                    LOG_TRC("Finished synchronous background saving ...");
                    // Next: we wait for an async UNO_COMMAND_RESULT on .uno:Save
                    // cf. Document::handleSaveMessage.
                },
                getViewId()))
        {
            LOG_TRC("saveDocumentBackground returns successful start");
            return true;
        }

        // fork failed
    }

    return false;
}

bool ChildSession::getStatus()
{
    std::string status;

    getLOKitDocument()->setView(_viewId);

    status = LOKitHelper::documentStatus(getLOKitDocument()->get());

    if (status.empty())
    {
        LOG_ERR("Failed to get document status.");
        return false;
    }

    return sendTextFrame("status: " + status);
}

bool ChildSession::getPartStatus()
{
    std::string status;

    getLOKitDocument()->setView(_viewId);

    status = LOKitHelper::documentStatus(getLOKitDocument()->get(), true);

    if (status.empty())
    {
        LOG_ERR("Failed to get part status.");
        return false;
    }

    return sendTextFrame("partstatus:" + status);
}

namespace
{

/// Given a view ID <-> user name map and a .uno:DocumentRepair result, annotate with user names.
void insertUserNames(const std::map<int, UserInfo>& viewInfo, std::string& json)
{
    Poco::JSON::Parser parser;
    const Poco::JSON::Object::Ptr root = parser.parse(json).extract<Poco::JSON::Object::Ptr>();
    std::vector<std::string> directions { "Undo", "Redo" };
    for (const auto& directionName : directions)
    {
        Poco::JSON::Object::Ptr direction = root->get(directionName).extract<Poco::JSON::Object::Ptr>();
        if (direction->get("actions").type() == typeid(Poco::JSON::Array::Ptr))
        {
            Poco::JSON::Array::Ptr actions = direction->get("actions").extract<Poco::JSON::Array::Ptr>();
            for (const auto& actionVar : *actions)
            {
                Poco::JSON::Object::Ptr action = actionVar.extract<Poco::JSON::Object::Ptr>();
                int viewId = action->getValue<int>("viewId");
                auto it = viewInfo.find(viewId);
                if (it != viewInfo.end())
                    action->set("userName", Poco::Dynamic::Var(it->second.getUserName()));
            }
        }
    }
    std::stringstream ss;
    root->stringify(ss);
    json = ss.str();
}

}

bool ChildSession::getCommandValues(const StringVector& tokens)
{
    bool success;
    std::string command;
    if (tokens.size() != 2 || !getTokenString(tokens[1], "command", command))
    {
        sendTextFrameAndLogError("error: cmd=commandvalues kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    if (command == ".uno:DocumentRepair")
    {
        LOKitHelper::ScopedString values(getLOKitDocument()->getCommandValues(".uno:Redo"));
        LOKitHelper::ScopedString undo(getLOKitDocument()->getCommandValues(".uno:Undo"));
        std::ostringstream jsonTemplate;
        jsonTemplate << R"({"commandName":".uno:DocumentRepair","Redo":)"
                     << (values.get() == nullptr ? "" : values.get())
                     << ",\"Undo\":" << (undo.get() == nullptr ? "" : undo.get()) << "}";
        std::string json = jsonTemplate.str();
        // json only contains view IDs, insert matching user names.
        std::map<int, UserInfo> viewInfo = _docManager->getViewInfo();
        insertUserNames(viewInfo, json);
        success = sendTextFrame("commandvalues: " + json);
    }
    else
    {
        LOKitHelper::ScopedString values(getLOKitDocument()->getCommandValues(command.c_str()));
        success = sendTextFrame("commandvalues: " + std::string(values.get() == nullptr ? "{}" : values.get()));
    }

    return success;
}

bool ChildSession::clientZoom(const StringVector& tokens)
{
    int tilePixelWidth, tilePixelHeight, tileTwipWidth, tileTwipHeight;
    std::string dpiScale, zoom;

    if (tokens.size() < 5 ||
        !getTokenInteger(tokens[1], "tilepixelwidth", tilePixelWidth) ||
        !getTokenInteger(tokens[2], "tilepixelheight", tilePixelHeight) ||
        !getTokenInteger(tokens[3], "tiletwipwidth", tileTwipWidth) ||
        !getTokenInteger(tokens[4], "tiletwipheight", tileTwipHeight))
    {
        sendTextFrameAndLogError("error: cmd=clientzoom kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    getLOKitDocument()->setClientZoom(tilePixelWidth, tilePixelHeight, tileTwipWidth, tileTwipHeight);

    if (tokens.size() == 7 &&
        getTokenString(tokens[5], "dpiscale", dpiScale) &&
        getTokenString(tokens[6], "zoompercent", zoom))
    {
        getLOKitDocument()->setViewOption("dpiscale", dpiScale.c_str());
        getLOKitDocument()->setViewOption("zoom", zoom.c_str());
    }

    return true;
}

bool ChildSession::clientVisibleArea(const StringVector& tokens)
{
    int x;
    int y;
    int width;
    int height;

    if ((tokens.size() != 5 && tokens.size() != 7) ||
        !getTokenInteger(tokens[1], "x", x) ||
        !getTokenInteger(tokens[2], "y", y) ||
        !getTokenInteger(tokens[3], "width", width) ||
        !getTokenInteger(tokens[4], "height", height))
    {
        sendTextFrameAndLogError("error: cmd=clientvisiblearea kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    _clientVisibleArea = Util::Rectangle(x, y, width, height);
    getLOKitDocument()->setClientVisibleArea(x, y, width, height);
    return true;
}

TilePrioritizer::Priority ChildSession::getTilePriority(const TileDesc &tile) const
{
    // previews are least interesting
    if (tile.isPreview())
        return TilePrioritizer::Priority::LOWEST;

    // different part less interesting than session's current part
    if (tile.getPart() != _currentPart)
        return TilePrioritizer::Priority::LOW;

    // most important to render things close to the cursor fast
    if (tile.intersects(_cursorPosition))
        return TilePrioritizer::Priority::ULTRAHIGH;

    // inside viewing area more important than outside it
    if (tile.intersects(_clientVisibleArea))
        return TilePrioritizer::Priority::VERYHIGH;

    // pre-loading near the viewing area is also more important than far away
    Util::Rectangle r = tile.toAABBox();
    // grow in each direction
    Util::Rectangle enlarged =
        Util::Rectangle::create(r.getLeft() - r.getWidth(), r.getTop() - r.getHeight(),
                                r.getRight() + r.getWidth(), r.getBottom() + r.getHeight());
    if (enlarged.intersects(_clientVisibleArea))
        return TilePrioritizer::Priority::HIGH;

    return TilePrioritizer::Priority::NORMAL;
}

bool ChildSession::outlineState(const StringVector& tokens)
{
    std::string type, state;
    int level, index;

    if (tokens.size() != 5 ||
        !getTokenString(tokens[1], "type", type) ||
        (type != "column" && type != "row") ||
        !getTokenInteger(tokens[2], "level", level) ||
        !getTokenInteger(tokens[3], "index", index) ||
        !getTokenString(tokens[4], "state", state) ||
        (state != "visible" && state != "hidden"))
    {
        sendTextFrameAndLogError("error: cmd=outlinestate kind=syntax");
        return false;
    }

    bool column = type == "column";
    bool hidden = state == "hidden";

    getLOKitDocument()->setView(_viewId);

    getLOKitDocument()->setOutlineState(column, level, index, hidden);
    return true;
}

std::string ChildSession::getJailDocRoot() const
{
    std::string jailDoc = JAILED_DOCUMENT_ROOT;
    if (NoCapsForKit)
    {
        jailDoc = Poco::URI(getJailedFilePath()).getPath();
        jailDoc = jailDoc.substr(0, jailDoc.find(JAILED_DOCUMENT_ROOT)) + JAILED_DOCUMENT_ROOT;
    }
    return jailDoc;
}

bool ChildSession::downloadAs(const StringVector& tokens)
{
#ifdef IOS
    NSLog(@"We should never come here, aborting");
    std::abort();
#elif defined(_WIN32)
    // Presumably ditto for CODA-W
    std::abort();
#else
    std::string name, id, format, filterOptions;

    if (tokens.size() < 5 ||
        !getTokenString(tokens[1], "name", name) ||
        !getTokenString(tokens[2], "id", id))
    {
        sendTextFrameAndLogError("error: cmd=downloadas kind=syntax");
        return false;
    }

    // Obfuscate the new name.
    Anonymizer::mapAnonymized(Uri::getFilenameFromURL(name), _docManager->getObfuscatedFileId());

    getTokenString(tokens[3], "format", format);

    if (getTokenString(tokens[4], "options", filterOptions))
    {
        if (tokens.size() > 5)
        {
            filterOptions += tokens.cat(' ', 5);
        }
    }

    if (filterOptions.empty() && format == "html")
    {
        // Opt-in to avoid linked images, those would not leave the chroot.
        filterOptions = "EmbedImages";
    }

    // Hack pass watermark by filteroptions to saveas
    if ( getWatermarkText().length() > 0) {
        filterOptions += std::string(",Watermark=") + getWatermarkText() + std::string("WATERMARKEND");
    }

    // Prevent user inputting anything funny here.
    // A "name" should always be a name, not a path
    const Poco::Path filenameParam(name);
    const std::string nameAnonym = anonymizeUrl(name);

    std::string jailDoc = getJailDocRoot();

    if constexpr (!Util::isMobileApp())
        consistencyCheckJail();

    // The file is removed upon downloading.
    const std::string tmpDir = FileUtil::createRandomDir(jailDoc);
    const std::string urlToSend = tmpDir + '/' + filenameParam.getFileName();
    const std::string url = jailDoc + urlToSend;
    const std::string filename = Poco::Path(nameAnonym).getFileName();
    const std::string urlAnonym = jailDoc + tmpDir + '/' + filename;

    LOG_DBG("Calling COKit's saveAs with URL: ["
            << urlAnonym << "], Format: [" << (format.empty() ? "(nullptr)" : format.c_str())
            << "], Filter Options: ["
            << (filterOptions.empty() ? "(nullptr)" : filterOptions.c_str()) << ']');

    bool success = getLOKitDocument()->saveAs(url.c_str(),
                               format.empty() ? nullptr : format.c_str(),
                               filterOptions.empty() ? nullptr : filterOptions.c_str());

    if (!success)
    {
        LOG_ERR("SaveAs Failed for id=" << id << " [" << url << "]. error= " << getLOKitLastError());
        sendTextFrameAndLogError("error: cmd=downloadas kind=saveasfailed");
        return false;
    }

    // Register download id -> URL mapping in the DocumentBroker
    const std::string docBrokerMessage =
        "registerdownload: downloadid=" + tmpDir + " url=" + urlToSend + " clientid=" + getId();
    _docManager->sendFrame(docBrokerMessage);

    // Send download id to the client
    sendTextFrame("downloadas: downloadid=" + tmpDir + " port=" + std::to_string(ClientPortNumber) +
                  " id=" + id + " filename=" + filename);
#endif
    return true;
}

bool ChildSession::getChildId()
{
    sendTextFrame("getchildid: id=" + _jailId);
    return true;
}

std::string ChildSession::getTextSelectionInternal(const std::string& mimeType)
{
    getLOKitDocument()->setView(_viewId);

    LOKitHelper::ScopedString textSelection(getLOKitDocument()->getTextSelection(mimeType.c_str(), nullptr));

    std::string str(textSelection ? textSelection.get() : "");
    return str;
}

bool ChildSession::getTextSelection(const StringVector& tokens)
{
    std::string mimeTypeList;

    if (tokens.size() != 2 ||
        !getTokenString(tokens[1], "mimetype", mimeTypeList))
    {
        sendTextFrameAndLogError("error: cmd=gettextselection kind=syntax");
        return false;
    }

    std::vector<std::string> mimeTypes = Util::splitStringToVector(mimeTypeList, ',');
    if (mimeTypes.empty())
    {
        sendTextFrameAndLogError("error: cmd=gettextselection kind=syntax");
        return false;
    }

    std::string mimeType = mimeTypes[0];
    SigUtil::addActivity(getId(), "getTextSelection");

    if (getLOKitDocument()->getDocumentType() != KIT_DOCTYPE_TEXT &&
        getLOKitDocument()->getDocumentType() != KIT_DOCTYPE_SPREADSHEET)
    {
        const std::string selection = getTextSelectionInternal(mimeType);
        if (selection.size() >= 1024 * 1024) // Don't return huge data.
        {
            // Flag complex data so the client will download async.
            sendTextFrame("complexselection:");
            return true;
        }

        sendTextFrame("textselectioncontent: " + selection);
        return true;
    }

    getLOKitDocument()->setView(_viewId);
    Poco::JSON::Object selectionObject;
    for (const auto& type : mimeTypes)
    {
        char* textSelection = nullptr;
        const int selectionType = getLOKitDocument()->getSelectionTypeAndText(type.c_str(), &textSelection);
        std::string selection(textSelection ? textSelection : "");
        free(textSelection);
        if (selectionType == KIT_SELTYPE_LARGE_TEXT || selectionType == KIT_SELTYPE_COMPLEX)
        {
            // Flag complex data so the client will download async.
            sendTextFrame("complexselection:");
            return true;
        }
        if (mimeTypes.size() == 1)
        {
            // Single format: send that as-is.
            sendTextFrame("textselectioncontent: " + selection);
            return true;
        }

        selectionObject.set(type, selection);
    }

    // Multiple formats: send in JSON.
    std::stringstream selectionStream;
    selectionObject.stringify(selectionStream);
    std::string selection = selectionStream.str();
    sendTextFrame("textselectioncontent:\n" + selection);
    return true;
}

bool ChildSession::getClipboard(const StringVector& tokens)
{
    std::vector<std::string> specifics;
    const char **mimeTypes = nullptr; // fetch all for now.
    std::vector<const char*> inMimeTypes;
    size_t       outCount = 0;
    char       **outMimeTypes = nullptr;
    size_t      *outSizes = nullptr;
    char       **outStreams = nullptr;

    std::string tagName;
    if (tokens.size() < 2 || !getTokenString(tokens[1], "name", tagName))
    {
        sendTextFrameAndLogError("error: cmd=getclipboard kind=syntax");
        return false;
    }

    std::string mimeType;
    bool hasMimeRequest = tokens.size() > 2 && getTokenString(tokens[2], "mimetype", mimeType);
    if (hasMimeRequest)
    {
        specifics = Util::splitStringToVector(mimeType, ',');
        for (const auto& specific : specifics)
        {
            inMimeTypes.push_back(specific.c_str());
        }
        inMimeTypes.push_back(nullptr);
        mimeTypes = inMimeTypes.data();
    }

    SigUtil::addActivity(getId(), "getClipboard");

    bool success = false;
    getLOKitDocument()->setView(_viewId);

    success = getLOKitDocument()->getClipboard(mimeTypes, &outCount, &outMimeTypes,
                                               &outSizes, &outStreams);

    if (!success || outCount == 0)
    {
        LOG_WRN("Get clipboard failed " << getLOKitLastError());
        sendTextFrame("clipboardcontent: error");
        return false;
    }

    size_t outGuess = 32;
    for (size_t i = 0; i < outCount; ++i)
        outGuess += outSizes[i] + strlen(outMimeTypes[i]) + 10;

    std::vector<char> output;
    output.reserve(outGuess);

    bool json = !specifics.empty();
    Poco::JSON::Object selectionObject;
    LOG_TRC("Building clipboardcontent: " << outCount << " items");
    for (size_t i = 0; i < outCount; ++i)
    {
        LOG_TRC("\t[" << i << " - type " << outMimeTypes[i] << " size " << outSizes[i]);
        if (json)
        {
            std::string selection(outStreams[i], outSizes[i]);
            selectionObject.set(outMimeTypes[i], selection);
        }
        else
        {
            Util::vectorAppend(output, outMimeTypes[i]);
            Util::vectorAppend(output, "\n", 1);
            std::stringstream sstream;
            sstream << std::hex << outSizes[i];
            std::string hex = sstream.str();
            Util::vectorAppend(output, hex.data(), hex.size());
            Util::vectorAppend(output, "\n", 1);
            Util::vectorAppend(output, outStreams[i], outSizes[i]);
            Util::vectorAppend(output, "\n", 1);
        }
        free(outMimeTypes[i]);
        free(outStreams[i]);
    }
    free(outSizes);
    free(outMimeTypes);
    free(outStreams);
    if (json)
    {
        std::stringstream selectionStream;
        selectionObject.stringify(selectionStream);
        std::string selection = selectionStream.str();
        Util::vectorAppend(output, selection.c_str(), selection.size());
    }

    std::string clipFile = ChildSession::getJailDocRoot() + "clipboard." + tagName;

    std::ofstream fileStream;
    fileStream.open(clipFile);
    fileStream.write(output.data(), output.size());
    fileStream.close();

    if (fileStream.fail())
    {
        LOG_ERR("GetClipboard Failed for tag: " << tagName);
        return false;
    }

    LOG_TRC("Sending clipboardcontent of size " << output.size() << " bytes");
    sendTextFrame("clipboardcontent: file=" + clipFile);

    return true;
}

bool ChildSession::setClipboard(const StringVector& tokens)
{
    std::string clipFile;

    if (tokens.size() < 2 || !getTokenString(tokens[1], "name", clipFile))
    {
        sendTextFrameAndLogError("error: cmd=setclipboard name=filename");
        return false;
    }

    try {
        ClipboardData data;
        std::ifstream stream(clipFile);

        if (!stream)
        {
            LOG_ERR("unable to open clipboard: " << clipFile);
            return false;
        }

        const auto clipFileSize = FileUtil::Stat(clipFile).size();
        SigUtil::addActivity(getId(), "setClipboard " + std::to_string(clipFileSize) + " bytes");

        if (clipFileSize == 0)
        {
            LOG_WRN("Ignoring empty clipboard file: " << clipFile);
            return false;
        }

        // See if the data is in the usual mimetype-size-content format or is just plain HTML.
        std::string firstLine;
        std::getline(stream, firstLine, '\n');
        std::vector<char> html;
        bool hasHTML = firstLine.starts_with("<!DOCTYPE html>");
        stream.seekg(0, stream.beg);
        if (hasHTML)
        {
            // It's just HTML: copy that as-is.
            std::vector<char> buf(std::istreambuf_iterator<char>(stream), {});
            html = std::move(buf);
        }
        else
        {
            data.read(stream);
        }

        const size_t inCount = html.empty() ? data.size() : 1;
        std::vector<size_t> inSizes(inCount);
        std::vector<const char*> inMimeTypes(inCount);
        std::vector<const char*> inStreams(inCount);

        if (html.empty())
        {
            for (size_t i = 0; i < inCount; ++i)
            {
                inSizes[i] = data._content[i].length();
                inStreams[i] = data._content[i].c_str();
                inMimeTypes[i] = data._mimeTypes[i].c_str();
            }
        }
        else
        {
            inSizes[0] = html.size();
            inStreams[0] = html.data();
            inMimeTypes[0] = "text/html";
        }

        getLOKitDocument()->setView(_viewId);

        if (!getLOKitDocument()->setClipboard(inCount, inMimeTypes.data(), inSizes.data(),
                                              inStreams.data()))
            LOG_ERR("set clipboard returned failure");
        else
            LOG_TRC("set clipboard succeeded");
    } catch (const std::exception& ex) {
        LOG_ERR("set clipboard failed with exception: " << ex.what());
    } catch (...) {
        LOG_ERR("set clipboard failed with exception");
    }
    // FIXME: implement me [!] ...
    return false;
}

bool ChildSession::paste(const char* buffer, int length, const StringVector& tokens)
{
    std::string mimeType;
    if (tokens.size() < 2 || !getTokenString(tokens[1], "mimetype", mimeType) ||
        mimeType.empty())
    {
        sendTextFrameAndLogError("error: cmd=paste kind=syntax");
        return false;
    }

    if (mimeType.find("application/x-openoffice-embed-source-xml") == 0)
    {
        LOG_TRC("Re-writing garbled mime-type " << mimeType);
        mimeType = "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
    }

    const std::string firstLine = getFirstLine(buffer, length);
    const char* data = buffer + firstLine.size() + 1;
    int size = length - firstLine.size() - 1;
#if defined QTAPP || defined _WIN32
    // In CODA-Q, to work around a qtwebchannel "Could not convert argument QJsonValue(object,
    // QJsonObject()) to target type QString ." bug, _pasteTypedBlob in browser/src/map/Clipboard.js
    // base64-encoded the payload:
    //
    // The same root problem in CODA-W, although there we end up with a "the server encountered a
    // unknown error while parsing the [object command" error message.
    std::string dec;
    [[maybe_unused]] auto const res = macaron::Base64::Decode(std::string_view(data, size), dec);
    assert(res.empty());
    data = dec.data();
    size = dec.size();
#endif
    bool success = false;
    std::string result = "pasteresult: ";
    if (size > 0)
    {
        getLOKitDocument()->setView(_viewId);

        if (Log::traceEnabled())
        {
            // Ensure 8 byte alignment for the start of the data, SpookyHash needs it.
            std::vector<char> toHash(data, data + size);
            LOG_TRC("Paste data of size " << size << " bytes and hash " << SpookyHash::Hash64(toHash.data(), toHash.size(), 0));
        }
        success = getLOKitDocument()->paste(mimeType.c_str(), data, size);
        if (!success)
            LOG_WRN("Paste failed " << getLOKitLastError());
    }
    if (success)
        result += "success";
    else
        result += "fallback";
    sendTextFrame(result);

    return true;
}

bool ChildSession::insertFile(const StringVector& tokens)
{
    std::string name, type, data;
    int multimedia_width = 0;
    int multimedia_height = 0;

    if constexpr (!Util::isMobileApp())
    {
        if (tokens.size() < 3 || !getTokenString(tokens[1], "name", name) ||
            !getTokenString(tokens[2], "type", type))
        {
            sendTextFrameAndLogError("error: cmd=insertfile kind=syntax");
            return false;
        }

        if (type == "multimedia") {
            if (tokens.size() != 5 || !getTokenInteger(tokens[3], "width", multimedia_width) ||
                !getTokenInteger(tokens[4], "height", multimedia_height)) {
                sendTextFrameAndLogError("error: cmd=insertfile kind=syntax");
                return false;
            }
        } else if (tokens.size() != 3) {
            sendTextFrameAndLogError("error: cmd=insertfile kind=syntax");
            return false;
        }
    }
    else
    {
        if (tokens.size() < 4 || !getTokenString(tokens[1], "name", name) ||
            !getTokenString(tokens[2], "type", type) || !getTokenString(tokens[3], "data", data))
        {
            sendTextFrameAndLogError("error: cmd=insertfile kind=syntax");
            return false;
        }

        if (type == "multimedia") {
            if (tokens.size() != 6 || !getTokenInteger(tokens[4], "width", multimedia_width) ||
                !getTokenInteger(tokens[5], "height", multimedia_height)) {
                sendTextFrameAndLogError("error: cmd=insertfile kind=syntax");
                return false;
            }
        } else if (tokens.size() != 4) {
            sendTextFrameAndLogError("error: cmd=insertfile kind=syntax");
            return false;
        }
    }

    SigUtil::addActivity(getId(), "insertFile " + type);

    LOG_TRC("InsertFile with arguments: " << type << ": " << (data.empty() ? name : std::string("binary data")));

    if (type == "graphic" ||
        type == "graphicurl" ||
        type == "selectbackground" ||
        type == "comparedocuments" ||
        type == "comparedocumentsurl" ||
        type == "multimedia" ||
        type == "multimediaurl" )
    {
        std::string url;

        if constexpr (!Util::isMobileApp())
        {
            if (type == "graphic" || type == "selectbackground" || type == "multimedia" ||
                type == "comparedocuments")
            {
                std::string jailDoc = getJailDocRoot();
                url = "file://" + jailDoc + "insertfile/" + name;
            }
            else if (type == "graphicurl" || type == "multimediaurl" || type == "comparedocumentsurl")
            {
                URI::decode(name, url);
                if (!Util::toLower(url).starts_with("http"))
                {
                    // Do not allow arbitrary schemes, especially "file://".
                    sendTextFrameAndLogError("error: cmd=insertfile kind=syntax");
                    return false;
                }
            }
            else
                sendTextFrameAndLogError("error: cmd=insertfile kind=syntax");
        }
        else
        {
            assert(type == "graphic" || type == "multimedia");
            std::string binaryData;
            macaron::Base64::Decode(data, binaryData);
            const std::string tempFile = FileUtil::createRandomTmpDir() + '/' + name;
            std::ofstream fileStream;
            fileStream.open(tempFile, std::ios::out | std::ios::binary);
            fileStream.write(binaryData.data(), binaryData.size());
            fileStream.close();
            url = Poco::URI(Poco::Path(tempFile)).toString();
        }

        std::string command;
        std::string arguments;
        if (type == "multimedia" || type == "multimediaurl") {
            command = ".uno:InsertAVMedia";
            arguments = "{"
                "\"URL\":{"
                    "\"type\":\"string\","
                    "\"value\":\"" + url + "\""
                "},"
                "\"IsLink\":{"
                    "\"type\":\"boolean\","
                    "\"value\":\"false\""
                "},"
                "\"Size\":{"
                    "\"type\":\"any\","
                    "\"value\":{"
                        "\"type\":\"com.sun.star.awt.Size\","
                        "\"value\":{"
                            // Core can't calculate the size for us due to a lack of gstreamer,
                            // but for multimedia (not multimediaurl) we can do it in online with a <video> element
                            "\"Width\":{"
                                "\"type\":\"long\","
                                "\"value\":" + std::to_string(multimedia_width) +
                            "},"
                            "\"Height\":{"
                                "\"type\":\"long\","
                                "\"value\":" + std::to_string(multimedia_height) +
                            "}"
                        "}"
                    "}"
                "}"
            "}";
        }
        else if (type == "comparedocuments" || type == "comparedocumentsurl")
        {
            command = ".uno:CompareDocuments";
            arguments = "{"
                "\"URL\":{"
                    "\"type\":\"string\","
                    "\"value\":\"" + url + "\""
                "}}";
        } else {
            command = (type == "selectbackground" ? ".uno:SelectBackground" : ".uno:InsertGraphic");
            arguments = "{"
                "\"FileName\":{"
                    "\"type\":\"string\","
                    "\"value\":\"" + url + "\""
                "}}";
        }

        getLOKitDocument()->setView(_viewId);

        LOG_TRC("Inserting " << type << ": " << command << ' ' << arguments.c_str());

        getLOKitDocument()->postUnoCommand(command.c_str(), arguments.c_str(), false);
    }

    return true;
}

bool ChildSession::extTextInputEvent(const StringVector& tokens)
{
    int id = -1;
    std::string text;
    bool error = false;

    if (tokens.size() < 3)
        error = true;
    else if (!getTokenInteger(tokens[1], "id", id) || id < 0)
        error = true;
    else {
        error = !getTokenString(tokens[2], "text", text);
    }

    if (error)
    {
        sendTextFrameAndLogError("error: cmd=" + std::string(tokens[0]) + " kind=syntax");
        return false;
    }

    std::string decodedText;
    URI::decode(text, decodedText);

    getLOKitDocument()->setView(_viewId);
    getLOKitDocument()->postWindowExtTextInputEvent(id, KIT_EXT_TEXTINPUT, decodedText.c_str());
    getLOKitDocument()->postWindowExtTextInputEvent(id, KIT_EXT_TEXTINPUT_END, decodedText.c_str());

    return true;
}

bool ChildSession::keyEvent(const StringVector& tokens,
                            const LokEventTargetEnum target)
{
    int type = 0;
    int charcode = 0;
    int keycode = 0;
    unsigned winId = 0;
    unsigned counter = 1;
    unsigned expectedTokens = 4; // cmdname(key), type, char, key are strictly required
    if (target == LokEventTargetEnum::Window)
    {
        if (tokens.size() <= counter ||
            !getTokenUInt32(tokens[counter++], "id", winId))
        {
            LOG_ERR("Window key event expects a valid id= attribute");
            sendTextFrameAndLogError("error: cmd=" + std::string(tokens[0]) + " kind=syntax");
            return false;
        }
        else // id= attribute is found
            expectedTokens++;
    }

    if (tokens.size() != expectedTokens ||
        !getTokenKeyword(tokens[counter++], "type",
                         {{"input", KIT_KEYEVENT_KEYINPUT}, {"up", KIT_KEYEVENT_KEYUP}},
                         type) ||
        !getTokenInteger(tokens[counter++], "char", charcode) ||
        !getTokenInteger(tokens[counter++], "key", keycode))
    {
        sendTextFrameAndLogError("error: cmd=" + std::string(tokens[0]) + "  kind=syntax");
        return false;
    }

    // Don't close LO window!
    constexpr int KEY_CTRL = 0x2000;
    constexpr int KEY_W = 0x0216;
#if !MOBILEAPP
    constexpr int KEY_INSERT = 0x0505;
#endif
    if (keycode == (KEY_CTRL | KEY_W))
    {
        return true;
    }

    // Ctrl+Tab switching browser tabs,
    // Doesn't insert tabs.
    constexpr int KEY_TAB = 0x0502;
    if (keycode == (KEY_CTRL | KEY_TAB))
    {
        return true;
    }

    getLOKitDocument()->setView(_viewId);
    if (target == LokEventTargetEnum::Document)
    {
#if !MOBILEAPP
        // Check if override mode is disabled.
        if (type == KIT_KEYEVENT_KEYINPUT && charcode == 0 && keycode == KEY_INSERT &&
            !ConfigUtil::getBool("overwrite_mode.enable", false))
            return true;
#endif
        getLOKitDocument()->postKeyEvent(type, charcode, keycode);
    }
    else if (winId != 0)
        getLOKitDocument()->postWindowKeyEvent(winId, type, charcode, keycode);

    return true;
}

bool ChildSession::gestureEvent(const StringVector& tokens)
{
    bool success = true;

    unsigned int windowID = 0;
    int x = 0;
    int y = 0;
    int offset = 0;
    std::string type;

    if (tokens.size() < 6)
        success = false;

    if (!success ||
        !getTokenUInt32(tokens[1], "id", windowID) ||
        !getTokenString(tokens[2], "type", type) ||
        !getTokenInteger(tokens[3], "x", x) ||
        !getTokenInteger(tokens[4], "y", y) ||
        !getTokenInteger(tokens[5], "offset", offset))
    {
        success = false;
    }

    if (!success)
    {
        sendTextFrameAndLogError("error: cmd=" +  std::string(tokens[0]) + " kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    getLOKitDocument()->postWindowGestureEvent(windowID, type.c_str(), x, y, offset);

    return true;
}

bool ChildSession::mouseEvent(const StringVector& tokens,
                              const LokEventTargetEnum target)
{
    bool success = true;

    // default values for compatibility reasons with older cools
    int buttons = 1; // left button
    int modifier = 0;

    unsigned winId = 0;
    unsigned counter = 1;
    unsigned minTokens = 5; // cmdname(mouse), type, x, y, count are strictly required
    if (target == LokEventTargetEnum::Window)
    {
        if (tokens.size() <= counter ||
            !getTokenUInt32(tokens[counter++], "id", winId))
        {
            LOG_ERR("Window mouse event expects a valid id= attribute");
            success = false;
        }
        else // id= attribute is found
            minTokens++;
    }

    int type = 0;
    int x = 0;
    int y = 0;
    int count = 0;
    if (tokens.size() < minTokens ||
        !getTokenKeyword(tokens[counter++], "type",
                         {{"buttondown", KIT_MOUSEEVENT_MOUSEBUTTONDOWN},
                          {"buttonup", KIT_MOUSEEVENT_MOUSEBUTTONUP},
                          {"move", KIT_MOUSEEVENT_MOUSEMOVE}},
                         type) ||
        !getTokenInteger(tokens[counter++], "x", x) ||
        !getTokenInteger(tokens[counter++], "y", y) ||
        !getTokenInteger(tokens[counter++], "count", count))
    {
        success = false;
    }

    // compatibility with older cools
    if (success && tokens.size() > counter && !getTokenInteger(tokens[counter++], "buttons", buttons))
        success = false;

    // compatibility with older cools
    if (success && tokens.size() > counter && !getTokenInteger(tokens[counter++], "modifier", modifier))
        success = false;

    if (!success)
    {
        sendTextFrameAndLogError("error: cmd=" +  std::string(tokens[0]) + " kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);
    switch (target)
    {
    case LokEventTargetEnum::Document:
        getLOKitDocument()->postMouseEvent(type, x, y, count, buttons, modifier);
        break;
    case LokEventTargetEnum::Window:
        getLOKitDocument()->postWindowMouseEvent(winId, type, x, y, count, buttons, modifier);
        break;
    default:
        assert(false && "Unsupported mouse target type");
    }

    return true;
}

bool ChildSession::dialogEvent(const StringVector& tokens)
{
    if (tokens.size() <= 2)
    {
        sendTextFrameAndLogError("error: cmd=dialogevent kind=syntax");
        return false;
    }

    unsigned long long int lokWindowId = 0;

    try
    {
        lokWindowId = std::stoull(tokens[1]);
    }
    catch (const std::exception&)
    {
        sendTextFrameAndLogError("error: cmd=dialogevent kind=syntax");
        return false;
    }

    if (_isDocLoaded)
    {
        getLOKitDocument()->setView(_viewId);
        getLOKitDocument()->sendDialogEvent(lokWindowId,
                                            tokens.substrFromToken(2).c_str());
    }
    else
    {
        getLOKit()->sendDialogEvent(lokWindowId, tokens.substrFromToken(2).c_str());
    }

    return true;
}

bool ChildSession::formFieldEvent(const char* buffer, int length, const StringVector& /*tokens*/)
{
    std::string firstLine = getFirstLine(buffer, length);
    std::string arguments = firstLine.substr(std::string_view("formfieldevent ").size());

    if (arguments.empty())
    {
        sendTextFrameAndLogError("error: cmd=formfieldevent kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);
    getLOKitDocument()->sendFormFieldEvent(arguments.c_str());

    return true;
}

bool ChildSession::contentControlEvent(const StringVector& tokens)
{
    std::string type;
    if (tokens.size() != 3 || !getTokenString(tokens[1], "type", type))
    {
        sendTextFrameAndLogError("error: cmd=contentcontrolevent kind=syntax");
        return false;
    }
    std::string arguments = R"({"type":")" + type + "\",";

    if (type == "picture")
    {
        std::string name;
        if (getTokenString(tokens[2], "name", name))
        {
            std::string jailDoc = getJailDocRoot();
            std::string url = "file://" + jailDoc + "insertfile/" + name;
            arguments += R"("changed":")" + url + "\"}";
        }
    }
    else if (type == "pictureurl")
    {
        std::string name;
        if (getTokenString(tokens[2], "name", name))
        {
            std::string url;
            URI::decode(name, url);
            arguments = R"({"type":"picture","changed":")" + url + "\"}";
        }
    }
    else if (type == "date" || type == "drop-down")
    {
        std::string data;
        getTokenString(tokens[2], "selected", data);
        arguments += R"("selected":")" + data + "\"" + "}";
    }

    getLOKitDocument()->setView(_viewId);
    getLOKitDocument()->sendContentControlEvent(arguments.c_str());

    return true;
}

bool ChildSession::renderSearchResult(const char* buffer, int length, const StringVector& /*tokens*/)
{
    std::string content(buffer, length);
    std::string command("rendersearchresult ");
    std::string arguments = content.substr(command.size());

    if (arguments.empty())
    {
        sendTextFrameAndLogError("error: cmd=rendersearchresult kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    const auto tileMode = static_cast<COKitTileMode>(getLOKitDocument()->getTileMode());

    unsigned char* bitmapBuffer = nullptr;

    int width = 0;
    int height = 0;
    size_t byteSize = 0;

    bool success = getLOKitDocument()->renderSearchResult(arguments.c_str(), &bitmapBuffer, &width, &height, &byteSize);

    if (success && byteSize > 0)
    {
        std::vector<char> output;
        output.reserve(byteSize * 3 / 4); // reserve 75% of original size

        if (Png::encodeBufferToPNG(bitmapBuffer, width, height, output, tileMode))
        {
            constexpr std::string_view header = "rendersearchresult:\n";
            const size_t responseSize = header.size() + output.size();
            std::vector<char> response(responseSize);
            std::copy(header.begin(), header.end(), response.begin());
            std::copy(output.begin(), output.end(), response.begin() + header.size());
            sendBinaryFrame(response.data(), response.size());
        }
        else
        {
            sendTextFrameAndLogError("error: cmd=rendersearchresult kind=failure");
        }
    }
    else
    {
        sendTextFrameAndLogError("error: cmd=rendersearchresult kind=failure");
    }

    free(bitmapBuffer);

    return true;
}


bool ChildSession::completeFunction(const StringVector& tokens)
{
    std::string functionName;

    if (tokens.size() != 2 ||
        !getTokenString(tokens[1], "name", functionName) ||
        functionName.empty())
    {
        sendTextFrameAndLogError("error: cmd=completefunction kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    getLOKitDocument()->completeFunction(functionName.c_str());
    return true;
}

bool ChildSession::unoSignatureCommand(const std::string_view commandName)
{
    // See if user private info has a signing key/cert: if so, annotate the UNO command with those
    // parameters before sending.
    const std::string& userPrivateInfo = getUserPrivateInfo();
    if (userPrivateInfo.empty())
    {
        return false;
    }

    Object::Ptr userPrivateInfoObj;
    Parser parser;
    Poco::Dynamic::Var var = parser.parse(userPrivateInfo);
    try
    {
        userPrivateInfoObj = var.extract<Object::Ptr>();
    }
    catch (const Poco::BadCastException& exception)
    {
        LOG_DBG("user private data is not a dictionary: " << exception.what());
    }
    if (!userPrivateInfoObj)
    {
        return false;
    }

    std::string signatureCert;
    JsonUtil::findJSONValue(userPrivateInfoObj, "SignatureCert", signatureCert);
    Object::Ptr argumentsObj = new Object();
    if (signatureCert.empty())
    {
        return false;
    }

    argumentsObj->set("SignatureCert", JsonUtil::makePropertyValue("string", signatureCert));
    std::string signatureKey;
    JsonUtil::findJSONValue(userPrivateInfoObj, "SignatureKey", signatureKey);
    if (signatureKey.empty())
    {
        return false;
    }

    argumentsObj->set("SignatureKey", JsonUtil::makePropertyValue("string", signatureKey));

    std::ostringstream oss;
    oss << "uno ";
    oss << commandName;
    oss << " ";
    argumentsObj->stringify(oss);
    std::string str = oss.str();
    StringVector tokens = StringVector::tokenize(str.data(), str.size());
    return unoCommand(tokens);
}

bool ChildSession::unoCommand(const StringVector& tokens)
{
    if (tokens.size() <= 1)
    {
        sendTextFrameAndLogError("error: cmd=uno kind=syntax");
        return false;
    }

    SigUtil::addActivity(getId(), formatUnoCommandInfo(tokens[1]));

    // we need to get KIT_CALLBACK_UNO_COMMAND_RESULT callback when saving
    const bool notify = (tokens.equals(1, ".uno:Save") ||
                          tokens.equals(1, ".uno:Undo") ||
                          tokens.equals(1, ".uno:Redo") ||
                          tokens.equals(1, ".uno:Cut") ||
                          tokens.equals(1, ".uno:Copy") ||
                          tokens.equals(1, ".uno:CopySlide") ||
                          tokens.equals(1, ".uno:OpenHyperlink") ||
                          tokens.startsWith(1, "vnd.sun.star.script:"));

    const std::string saveArgs = tokens.substrFromToken(2);
    LOG_TRC("uno command " << tokens[1] << " " << saveArgs << " notify: " << notify);

    // check that internal UNO commands don't make it to the core
    assert (!tokens.equals(1, ".uno:AutoSave"));

    getLOKitDocument()->setView(_viewId);

    if (tokens.equals(1, ".uno:Copy") || tokens.equals(1, ".uno:CopyHyperlinkLocation"))
        _copyToClipboard = true;

    if (tokens.size() == 2 && tokens.equals(1, ".uno:fakeDiskFull"))
    {
        _docManager->alertAllUsers("internal", "diskfull");
        return true;
    }

    getLOKitDocument()->postUnoCommand(tokens[1].c_str(), saveArgs.c_str(), notify);
    return true;
}

bool ChildSession::selectText(const StringVector& tokens,
                              const LokEventTargetEnum target)
{
    std::string swap;
    unsigned winId = 0;
    int type = 0, x = 0, y = 0;
    if (target == LokEventTargetEnum::Window)
    {
        if (tokens.size() != 5 ||
            !getTokenUInt32(tokens[1], "id", winId) ||
            !getTokenString(tokens[2], "swap", swap) ||
            (swap != "true" && swap != "false") ||
            !getTokenInteger(tokens[3], "x", x) ||
            !getTokenInteger(tokens[4], "y", y))
        {
            LOG_ERR("error: cmd=windowselecttext kind=syntax");
            return false;
        }
    }
    else if (target == LokEventTargetEnum::Document)
    {
        if (tokens.size() != 4 ||
            !getTokenKeyword(tokens[1], "type",
                             {{"start", KIT_SETTEXTSELECTION_START},
                              {"end", KIT_SETTEXTSELECTION_END},
                              {"reset", KIT_SETTEXTSELECTION_RESET}},
                             type) ||
            !getTokenInteger(tokens[2], "x", x) ||
            !getTokenInteger(tokens[3], "y", y))
        {
            sendTextFrameAndLogError("error: cmd=selecttext kind=syntax");
            return false;
        }
    }

    getLOKitDocument()->setView(_viewId);

    switch (target)
    {
    case LokEventTargetEnum::Document:
        getLOKitDocument()->setTextSelection(type, x, y);
        break;
    case LokEventTargetEnum::Window:
        getLOKitDocument()->setWindowTextSelection(winId, swap == "true", x, y);
        break;
    default:
        assert(false && "Unsupported select text target type");
    }

    return true;
}

// FIXME: remove SpookyHash et. al.

namespace {
inline
uint64_t hashSubBuffer(unsigned char* pixmap, size_t startX, size_t startY,
                       long width, long height, int bufferWidth, int bufferHeight)
{
    if (bufferWidth < width || bufferHeight < height)
        return 0; // magic invalid hash.

    // assume a consistent mode - RGBA vs. BGRA for process
    SpookyHash hash;
    hash.Init(1073741789, 1073741789); // Seeds can be anything.
    for (long y = 0; y < height; ++y)
    {
        const size_t position = ((startY + y) * bufferWidth * 4) + (startX * 4);
        hash.Update(pixmap + position, width * 4);
    }

    uint64_t hash1;
    uint64_t hash2;
    hash.Final(&hash1, &hash2);
    return hash1;
}
}

bool ChildSession::renderNextSlideLayer(SlideCompressor& scomp, const unsigned width,
                                        const unsigned height, double devicePixelRatio, bool& done,
                                        const std::string& cacheKey, bool isCompressed = false)
{
    // FIXME: we need a multi-user / view cache somewhere here (?)
    auto pixmap = std::make_shared<std::vector<unsigned char>>(static_cast<size_t>(4) * width * height);
    bool isBitmapLayer = false;
    char* msg = nullptr;
    done = getLOKitDocument()->renderNextSlideLayer(pixmap->data(), &isBitmapLayer, &devicePixelRatio, &msg);
    std::string jsonMsg(msg != nullptr ? msg : "");
    free(msg);

    if (jsonMsg.empty())
        return true;

    const auto tileMode = static_cast<COKitTileMode>(getLOKitDocument()->getTileMode());
    scomp.pushWork(
        [=, this, pixmap = std::move(pixmap),
         jsonMsg = std::move(jsonMsg)](std::vector<char>& output)
        {
            std::string json = jsonMsg;
            Poco::JSON::Parser parser;
            Poco::JSON::Object::Ptr root = parser.parse(json).extract<Poco::JSON::Object::Ptr>();
            root->set("cacheKey", cacheKey);
            root->set("isCompressed", isCompressed);

            json = JsonUtil::jsonToString(root);

            if (!isBitmapLayer)
            {
                std::string response = "slidelayer: " + json;
                Util::vectorAppend(output, response);
                return;
            }

            if (watermark())
            {
                const int watermarkWidth = width / 4;
                const int watermarkHeight = height / 3;
                const int stampsByX = 4;
                const int stampsByY = 3;
                for (int i = 0; i < stampsByX; ++i)
                {
                    int offsetX = i * watermarkWidth;
                    for (int j = 0; j < stampsByY; ++j)
                    {
                        int offsetY = j * watermarkHeight;
                        // presumed thread-safe
                        watermark()->blending(pixmap->data(), offsetX, offsetY,
                                              width, height, watermarkWidth, watermarkHeight,
                                              tileMode, /*isSlideShowLayer*/ true);
                    }
                }
            }

            uint64_t pixmapHash = hashSubBuffer(pixmap->data(), 0, 0, width, height, width, height) + getViewId();
            if (size_t start = json.find("%IMAGECHECKSUM%"); start != std::string::npos)
                json.replace(start, 15, std::to_string(pixmapHash));

            // Use ZSTD to compress the slide layer
            if (size_t start = json.find("%IMAGETYPE%"); start != std::string::npos)
                json.replace(start, 11, "zstd");

            root = parser.parse(json).extract<Poco::JSON::Object::Ptr>();
            root->set("width", width);
            root->set("height", height);
            json = JsonUtil::jsonToString(root);

            std::string response = "zstdslidelayer: " + json;

            response += "\n";

            size_t compressed_max_size = ZSTD_COMPRESSBOUND(pixmap->size());
            size_t max_required_size = response.size() + compressed_max_size;
            output.resize(max_required_size);
            std::memcpy(output.data(), response.data(), response.size());

            if (tileMode == COKitTileMode::KIT_TILEMODE_BGRA)
            {
                png_row_info rowInfo;
                rowInfo.rowbytes = pixmap->size();
                // Following function just needs row size to transform from BGRA to RGBA
                // We have a flat array so its safe to pass pixmap size as row size
                Png::unpremultiply_bgra_data(nullptr, &rowInfo, pixmap->data());
            }
            size_t compSize = ZSTD_compress(&output[response.size()], compressed_max_size,
                                            pixmap->data(), pixmap->size(), -3);

            if (ZSTD_isError(compSize))
            {
                output.resize(0);
                LOG_ERR("Failed to compress slidelayer of size " << pixmap->size() << " with "
                                                                << ZSTD_getErrorName(compSize));
                return;
            }
            output.resize(response.size() + compSize);

            LOG_TRC("Compressed slidelayer of size " << pixmap->size() << " to size " << compSize);
        });
    return true;
}

bool ChildSession::renderSlide(const StringVector& tokens)
{
    if (tokens.size() < 5)
    {
        sendTextFrameAndLogError("error: cmd=getslide kind=syntax");
        return false;
    }

    std::string hash;
    getTokenString(tokens[1], "hash", hash);

    int part = -1;
    std::string partString;
    if (getTokenString(tokens[2], "part", partString))
        part = NumUtil::stoi(partString);

    unsigned suggestedWidth = 0;
    std::string widthString;
    if (getTokenString(tokens[3], "width", widthString))
        suggestedWidth = NumUtil::stoi(widthString);

    unsigned suggestedHeight = 0;
    std::string heightString;
    if (getTokenString(tokens[4], "height", heightString))
        suggestedHeight = NumUtil::stoi(heightString);

    if (hash.empty() || part < 0 || suggestedWidth == 0 || suggestedHeight == 0)
    {
        sendTextFrameAndLogError("error: cmd=getslide kind=syntax");
        return false;
    }

    bool renderBackground = true;
    std::string renderBackgroundString;
    if (tokens.size() > 5 && getTokenString(tokens[5], "renderBackground", renderBackgroundString))
        renderBackground = NumUtil::stoi(renderBackgroundString) > 0;

    bool renderMasterPage = true;
    std::string renderMasterPageString;
    if (tokens.size() > 6 && getTokenString(tokens[6], "renderMasterPage", renderMasterPageString))
        renderMasterPage = NumUtil::stoi(renderMasterPageString) > 0;

    double devicePixelRatio = 1.0;
    std::string devicePixelRatioString;
    if (tokens.size() > 7 && getTokenString(tokens[7], "devicePixelRatio", devicePixelRatioString))
        devicePixelRatio = std::stod(devicePixelRatioString);

    bool compressedLayers = false;
    std::string compressedLayersString;
    if (tokens.size() > 8 && getTokenString(tokens[8], "compressedLayers", compressedLayersString))
        compressedLayers = NumUtil::stoi(compressedLayersString) > 0;

    unsigned bufferWidth = suggestedWidth;
    unsigned bufferHeight = suggestedHeight;
    bool success = getLOKitDocument()->createSlideRenderer(hash.c_str(), part,
                                                           &bufferWidth, &bufferHeight,
                                                           renderBackground, renderMasterPage);
    if (!success) {
        sendTextFrame(R"(sliderenderingcomplete: {"status": "fail"})");
        return false;
    }

    assert(bufferWidth <= suggestedWidth);
    assert(bufferHeight <= suggestedHeight);

    bool done = false;
    SlideCompressor scomp(_docManager->getSyncPool());
    while (!done)
    {
        success = renderNextSlideLayer(scomp, bufferWidth, bufferHeight, devicePixelRatio, done,
                                       tokens.substrFromToken(1), compressedLayers);
        if (!success)
            break;
    }

    scomp.compress([this](const std::vector<char>& output) {
        size_t pos = Util::findInVector(output, "\n");
        LOG_TRC("Sending response (" << output.size() << " bytes) for: " <<
                std::string(output.data(), pos == std::string::npos ? output.size() : pos - 1));
        sendBinaryFrame(output.data(), output.size());
    });

    getLOKitDocument()->postSlideshowCleanup();

    std::ostringstream msg;
    msg << "sliderenderingcomplete: " << R"({"status": ")" << (success ? "success" : "fail");
    if (EnableExperimental)
    {
        msg << R"(", "slidehash": ")" << hash << R"(", "compressedLayers": )"
            << (compressedLayers ? "true" : "false") << R"(, "cacheKey": ")"
            << tokens.substrFromToken(1);
    }

    msg << "\"}";
    sendTextFrame(msg.str());

    return success;
}

bool ChildSession::renderWindow(const StringVector& tokens)
{
    const unsigned winId = (tokens.size() > 1 ? std::stoul(tokens[1]) : 0);

    int startX = 0, startY = 0;
    int bufferWidth = 800, bufferHeight = 600;
    double dpiScale = 1.0;
    std::string paintRectangle;
    if (tokens.size() > 2 && getTokenString(tokens[2], "rectangle", paintRectangle)
        && paintRectangle != "undefined")
    {
        const StringVector rectParts
            = StringVector::tokenize(paintRectangle.c_str(), paintRectangle.length(), ',');
        if (rectParts.size() == 4)
        {
            startX = std::atoi(rectParts[0].c_str());
            startY = std::atoi(rectParts[1].c_str());
            bufferWidth = std::atoi(rectParts[2].c_str());
            bufferHeight = std::atoi(rectParts[3].c_str());
        }
    }
    else
        LOG_WRN("windowpaint command doesn't specify a rectangle= attribute.");

    std::string dpiScaleString;
    if (tokens.size() > 3 && getTokenString(tokens[3], "dpiscale", dpiScaleString))
    {
        dpiScale = std::stod(dpiScaleString);
        if (dpiScale < 0.001)
            dpiScale = 1.0;
    }

    constexpr int maxDimension = 4096;
    if (bufferWidth <= 0 || bufferHeight <= 0 || bufferWidth > maxDimension || bufferHeight > maxDimension)
    {
        LOG_WRN("paintwindow: rejecting invalid dimensions " << bufferWidth << 'x' << bufferHeight);
        return true;
    }

    const size_t pixmapDataSize = static_cast<size_t>(4) * bufferWidth * bufferHeight;
    std::vector<unsigned char> pixmap(pixmapDataSize);
    const int width = bufferWidth;
    const int height = bufferHeight;
    const auto start = std::chrono::steady_clock::now();
    getLOKitDocument()->paintWindow(winId, pixmap.data(), startX, startY, width, height, dpiScale,
                                    _viewId);
    const double area = width * height;

    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    const double elapsedMics = elapsedMs.count() * 1000.; // Need MPixels/second, use Pixels/mics.
    LOG_TRC("paintWindow for " << winId << " returned " << width << 'X' << height << "@(" << startX
                               << ',' << startY << ',' << " with dpi scale: " << dpiScale
                               << " and rendered in " << elapsedMs << " (" << (elapsedMics ? area / elapsedMics : 0)
                               << " MP/s).");

    uint64_t pixmapHash = hashSubBuffer(pixmap.data(), 0, 0, width, height, bufferWidth, bufferHeight) + getViewId();

    auto found = std::find(_pixmapCache.begin(), _pixmapCache.end(), pixmapHash);

    assert(_pixmapCache.size() <= LOKitHelper::tunnelledDialogImageCacheSize);

    // If not found in cache, we need to encode to PNG and send to client

    // To artificially induce intentional cache inconsistency between server and client, to be able
    // to test error handling, you can do something like:
    // const bool doPng = (found == _pixmapCache.end() || (time(NULL) % 10 == 0)) && ((time(NULL) % 10) < 8);

    const bool doPng = (found == _pixmapCache.end());

    LOG_DBG("Pixmap hash: " << pixmapHash << (doPng ? " NOT in cache, doing PNG" : " in cache, not encoding to PNG") << ", cache size now:" << _pixmapCache.size());

    // If it is already the first in the cache, no need to do anything. Otherwise, if in cache, move
    // to beginning. If not in cache, add it as first. Keep cache size limited.
    if (_pixmapCache.size() > 0)
    {
        if (found != _pixmapCache.begin())
        {
            if (found != _pixmapCache.end())
            {
                LOG_DBG("Erasing found entry");
                _pixmapCache.erase(found);
            }
            else if (_pixmapCache.size() == LOKitHelper::tunnelledDialogImageCacheSize)
            {
                LOG_DBG("Popping last entry");
                _pixmapCache.pop_back();
            }
            _pixmapCache.insert(_pixmapCache.begin(), pixmapHash);
        }
    }
    else
        _pixmapCache.insert(_pixmapCache.begin(), pixmapHash);

    LOG_DBG("Pixmap cache size now:" << _pixmapCache.size());

    assert(_pixmapCache.size() <= LOKitHelper::tunnelledDialogImageCacheSize);

    std::string response = "windowpaint: id=" + std::to_string(winId) + " width=" + std::to_string(width)
                           + " height=" + std::to_string(height);

    if (!paintRectangle.empty())
        response += " rectangle=" + paintRectangle;

    response += " hash=" + std::to_string(pixmapHash);

    if (!doPng)
    {
        // Just so that we might see in the client console log that no PNG was included.
        response += " nopng";
        sendTextFrame(response.c_str());
        return true;
    }

    response += "\n";

    std::vector<char> output;
    output.reserve(response.size() + pixmapDataSize);
    output.resize(response.size());
    std::memcpy(output.data(), response.data(), response.size());

    const auto mode = static_cast<COKitTileMode>(getLOKitDocument()->getTileMode());

    // TODO: use png cache for dialogs too
    if (!Png::encodeSubBufferToPNG(pixmap.data(), 0, 0, width, height, bufferWidth, bufferHeight, output, mode))
    {
        LOG_ERR("Failed to encode into PNG.");
        return false;
    }

#if 0
    {
        static const std::string tempDir = FileUtil::createRandomTmpDir();
        static int pngDumpCounter = 0;
        std::stringstream ss;
        ss << tempDir << "/" << "renderwindow-" << pngDumpCounter++ << ".png";
        LOG_INF("Dumping PNG to '"<< ss.str() << "'");
        FILE *f = fopen(ss.str().c_str(), "w");
        fwrite(output.data() + response.size(), output.size() - response.size(), 1, f);
        fclose(f);
    }
#endif

    LOG_TRC("Sending response (" << output.size() << " bytes) for: " << std::string(output.data(), response.size() - 1));
    sendBinaryFrame(output.data(), output.size());
    return true;
}

bool ChildSession::resizeWindow(const StringVector& tokens)
{
    const unsigned winId = (tokens.size() > 1 ? std::stoul(tokens[1], nullptr, 10) : 0);

    getLOKitDocument()->setView(_viewId);

    std::string size;
    if (tokens.size() > 2 && getTokenString(tokens[2], "size", size))
    {
        const std::vector<int> sizeParts = COOLProtocol::tokenizeInts(size, ',');
        if (sizeParts.size() == 2)
        {
            getLOKitDocument()->resizeWindow(winId, sizeParts[0], sizeParts[1]);
            return true;
        }
    }

    LOG_WRN("resizewindow command doesn't specify sensible size= attribute.");
    return true;
}

bool ChildSession::sendWindowCommand(const StringVector& tokens)
{
    const unsigned winId = (tokens.size() > 1 ? NumUtil::u64FromString(tokens[1], 0) : 0);

    getLOKitDocument()->setView(_viewId);

    if (tokens.size() > 2 && tokens.equals(2, "close"))
        getLOKitDocument()->postWindow(winId, KIT_WINDOW_CLOSE, nullptr);
    else if (tokens.size() > 3 && tokens.equals(2, "paste"))
        getLOKitDocument()->postWindow(winId, KIT_WINDOW_PASTE, tokens[3].c_str());

    return true;
}

namespace
{

std::string extractCertificate(const std::string & certificate)
{
    constexpr std::string_view header("-----BEGIN CERTIFICATE-----");
    constexpr std::string_view footer("-----END CERTIFICATE-----");

    size_t pos1 = certificate.find(header);
    if (pos1 == std::string::npos)
        return std::string();

    size_t pos2 = certificate.find(footer, pos1 + 1);
    if (pos2 == std::string::npos)
        return std::string();

    pos1 = pos1 + header.length();
    pos2 = pos2 - pos1;

    return certificate.substr(pos1, pos2);
}

}

bool ChildSession::askSignatureStatus(const char* buffer, int length, const StringVector& /*tokens*/)
{
    bool result = true;

    const std::string firstLine = getFirstLine(buffer, length);
    const char* data = buffer + firstLine.size() + 1;
    const int size = length - firstLine.size() - 1;
    std::string json(data, size);

    Poco::JSON::Parser parser;
    Poco::JSON::Object::Ptr root = parser.parse(json).extract<Poco::JSON::Object::Ptr>();

    if (root)
    {
        for (const auto& chainPtr : *root->getArray("certificates"))
        {
            if (!chainPtr.isString())
                return false;

            std::string chainCertificate = chainPtr;
            std::string binaryChainCertificate;
            macaron::Base64::Decode(extractCertificate(chainCertificate), binaryChainCertificate);

            result = getLOKitDocument()->addCertificate(
                reinterpret_cast<const unsigned char*>(binaryChainCertificate.data()),
                binaryChainCertificate.size());

            if (!result)
                return false;
        }
    }

    int status = getLOKitDocument()->getSignatureState();

    sendTextFrame("signaturestatus: " + std::to_string(status));
    return true;
}

bool ChildSession::selectGraphic(const StringVector& tokens)
{
    int type, x, y;
    if (tokens.size() != 4 ||
        !getTokenKeyword(tokens[1], "type",
                         {{"start", KIT_SETGRAPHICSELECTION_START},
                          {"end", KIT_SETGRAPHICSELECTION_END}},
                         type) ||
        !getTokenInteger(tokens[2], "x", x) ||
        !getTokenInteger(tokens[3], "y", y))
    {
        sendTextFrameAndLogError("error: cmd=selectgraphic kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    getLOKitDocument()->setGraphicSelection(type, x, y);

    return true;
}

bool ChildSession::resetSelection(const StringVector& tokens)
{
    if (tokens.size() != 1)
    {
        sendTextFrameAndLogError("error: cmd=resetselection kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    getLOKitDocument()->resetSelection();

    return true;
}

bool ChildSession::saveAs(const StringVector& tokens)
{
    std::string wopiFilename, url, format, filterOptions;

    if (tokens.size() <= 1 ||
        !getTokenString(tokens[1], "url", url))
    {
        sendTextFrameAndLogError("error: cmd=saveas kind=syntax");
        return false;
    }

    // if the url is a 'wopi:///something/blah.odt', then save to a temporary
    Poco::URI wopiURL(url);
    bool encodeURL = false;
    if (wopiURL.getScheme() == "wopi")
    {
        std::vector<std::string> pathSegments;
        wopiURL.getPathSegments(pathSegments);

        if (pathSegments.empty())
        {
            sendTextFrameAndLogError("error: cmd=saveas kind=syntax");
            return false;
        }

        std::string jailDoc = getJailDocRoot();

        const std::string tmpDir = FileUtil::createRandomDir(jailDoc);
        const Poco::Path filenameParam(pathSegments[pathSegments.size() - 1]);
        url = std::string("file://") + jailDoc + tmpDir + '/' + filenameParam.getFileName();
        // url becomes decoded at this stage
        // on saveAs we should send encoded!
        encodeURL = true;
        wopiFilename = wopiURL.getPath();
    }

    if (tokens.size() > 2)
        getTokenString(tokens[2], "format", format);

    if (tokens.size() > 3 && getTokenString(tokens[3], "options", filterOptions))
    {
        if (tokens.size() > 4)
        {
            // Syntax is options=<options>, and <options> may contain spaces, account for that.
            filterOptions += " " + tokens.cat(' ', 4);
        }
    }

    bool success = false;

    if (filterOptions.empty() && format == "html")
    {
        // Opt-in to avoid linked images, those would not leave the chroot.
        filterOptions = "EmbedImages";
    }

    if (_docManager->isDocPasswordProtected() && _docManager->haveDocPassword())
    {
        if (_docManager->getDocPasswordType() == DocumentPasswordType::ToView)
        {
            filterOptions += std::string(",Password=") + _docManager->getDocPassword() +
                             std::string("PASSWORDEND");
        }
        else
        {
            filterOptions += std::string(",PasswordToModify=") + _docManager->getDocPassword() +
                             std::string("PASSWORDTOMODIFYEND");
        }
        // Password might have changed since load
        setHaveDocPassword(true);
        setDocPassword(_docManager->getDocPassword());
    }

    // We don't have the FileId at this point, just a new filename to save-as.
    // So here the filename will be obfuscated with some hashing, which later will
    // get a proper FileId that we will use going forward.
    LOG_DBG("Calling COKit's saveAs with URL: ["
            << anonymizeUrl(wopiFilename) << "], Format: ["
            << (format.empty() ? "(nullptr)" : format.c_str()) << "], Filter Options: ["
            << (filterOptions.empty() ? "(nullptr)" : filterOptions.c_str()) << ']');

    getLOKitDocument()->setView(_viewId);

    std::string encodedURL;
    if (encodeURL)
        Poco::URI::encode(url, "", encodedURL);
    else
        // url is already encoded
        encodedURL = url;

    if constexpr (!Util::isMobileApp())
        consistencyCheckJail();

    std::string encodedWopiFilename;
    Poco::URI::encode(wopiFilename, "", encodedWopiFilename);

    success = getLOKitDocument()->saveAs(encodedURL.c_str(),
                                         format.empty() ? nullptr : format.c_str(),
                                         filterOptions.empty() ? nullptr : filterOptions.c_str());

    if (!success)
    {
        // a desperate try - add an extension hoping that it'll help
        bool retry = true;
        switch (getLOKitDocument()->getDocumentType())
        {
            case KIT_DOCTYPE_TEXT:         url += ".odt"; wopiFilename += ".odt"; break;
            case KIT_DOCTYPE_SPREADSHEET:  url += ".ods"; wopiFilename += ".ods"; break;
            case KIT_DOCTYPE_PRESENTATION: url += ".odp"; wopiFilename += ".odp"; break;
            case KIT_DOCTYPE_DRAWING:      url += ".odg"; wopiFilename += ".odg"; break;
            default:                       retry = false; break;
        }

        if (retry)
        {
            LOG_DBG("Retry: calling COKit's saveAs with URL: ["
                    << url << "], Format: [" << (format.empty() ? "(nullptr)" : format.c_str())
                    << "], Filter Options: ["
                    << (filterOptions.empty() ? "(nullptr)" : filterOptions.c_str()) << ']');

            success = getLOKitDocument()->saveAs(
                encodedURL.c_str(), format.empty() ? nullptr : format.c_str(),
                filterOptions.empty() ? nullptr : filterOptions.c_str());
        }
    }

    if (success)
        sendTextFrame("saveas: url=" + encodedURL + " filename=" + encodedWopiFilename);
    else
        sendTextFrameAndLogError("error: cmd=saveas kind=savefailed");

    return true;
}

bool ChildSession::exportAs(const StringVector& tokens)
{
    std::string wopiFilename, url;

    if (tokens.size() <= 1 ||
        !getTokenString(tokens[1], "url", url))
    {
        sendTextFrameAndLogError("error: cmd=exportas kind=syntax");
        return false;
    }

    Poco::URI wopiURL(url);
    if (wopiURL.getScheme() == "wopi")
    {
        std::vector<std::string> pathSegments;
        wopiURL.getPathSegments(pathSegments);

        if (pathSegments.empty())
        {
            sendTextFrameAndLogError("error: cmd=exportas kind=syntax");
            return false;
        }

        wopiFilename = wopiURL.getPath();
    }
    else
    {
        sendTextFrameAndLogError("error: cmd=exportas kind=syntax");
        return false;
    }

    // for PDF and EPUB show dialog with export options first
    // when options will be chosen and file exported we will
    // receive KIT_CALLBACK_EXPORT_FILE message
    std::string extension = FileUtil::extractFileExtension(wopiFilename);

    const bool isPDF = extension == "pdf";
    const bool isEPUB = extension == "epub";

    // We don't have the FileId at this point, just a new filename to save-as.
    // So here the filename will be obfuscated with some hashing, which later will
    // get a proper FileId that we will use going forward.
    LOG_DBG("Calling COKit's exportAs with: [" << anonymizeUrl(wopiFilename) << ']');

    getLOKitDocument()->setView(_viewId);

    std::string encodedWopiFilename;
    Poco::URI::encode(wopiFilename, "", encodedWopiFilename);

    _exportAsWopiUrl = std::move(encodedWopiFilename);

    if (isPDF || isEPUB)
    {
        const std::string arguments = "{"
            "\"SynchronMode\":{"
                "\"type\":\"boolean\","
                "\"value\": false"
            "}}";

        if (isPDF)
            getLOKitDocument()->postUnoCommand(".uno:ExportToPDF", arguments.c_str(), false);
        else if (isEPUB)
            getLOKitDocument()->postUnoCommand(".uno:ExportToEPUB", arguments.c_str(), false);

        return true;
    }

    // For image export (triggered from the image context menu).
    // SaveGraphic writes the image in its native format to /tmp/
    // and fires KIT_CALLBACK_EXPORT_FILE. If no graphic is selected,
    // the command is a no-op.
    // NOTE: new document export formats must be handled above this,
    // like PDF and EPUB.
    getLOKitDocument()->postUnoCommand(".uno:SaveGraphic", nullptr, false);
    return true;
}

bool ChildSession::setClientPart(const StringVector& tokens)
{
    int part = 0;
    if (tokens.size() < 2 ||
        !getTokenInteger(tokens[1], "part", part))
    {
        sendTextFrameAndLogError("error: cmd=setclientpart kind=invalid");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    if (getLOKitDocument()->getDocumentType() != KIT_DOCTYPE_TEXT && part != getLOKitDocument()->getPart())
    {
        getLOKitDocument()->setPart(part);
        _currentPart = part;
    }

    return true;
}

bool ChildSession::selectClientPart(const StringVector& tokens)
{
    int part = 0;
    int select = 0;
    if (tokens.size() < 3 ||
        !getTokenInteger(tokens[1], "part", part) ||
        !getTokenInteger(tokens[2], "how", select))
    {
        sendTextFrameAndLogError("error: cmd=selectclientpart kind=invalid");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    if (getLOKitDocument()->getDocumentType() != KIT_DOCTYPE_TEXT)
    {
        if (part != getLOKitDocument()->getPart())
        {
            getLOKitDocument()->selectPart(part, select);

            // Notify the client of the selection update.
            const std::string status = LOKitHelper::documentStatus(getLOKitDocument()->get());
            if (!status.empty())
                return sendTextFrame("statusupdate: " + status);
        }
    }
    else
    {
        LOG_WRN("ChildSession::selectClientPart[" << getName() << "]: error selecting part on text documents.");
    }

    return true;
}

bool ChildSession::moveSelectedClientParts(const StringVector& tokens)
{
    int position = 0;
    if (tokens.size() < 2 ||
        !getTokenInteger(tokens[1], "position", position))
    {
        sendTextFrameAndLogError("error: cmd=moveselectedclientparts kind=invalid");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    if (getLOKitDocument()->getDocumentType() != KIT_DOCTYPE_TEXT)
    {
        getLOKitDocument()->moveSelectedParts(position, false); // Move, don't duplicate.

        // Get the status to notify clients of the reordering and selection change.
        const std::string status = LOKitHelper::documentStatus(getLOKitDocument()->get());
        if (!status.empty())
            return _docManager->notifyAll("statusupdate: " + status);
    }
    else
    {
        LOG_WRN("ChildSession::moveSelectedClientParts[" << getName() << "]: error moving parts on text documents.");
    }

    return true; // Non-fatal to fail.
}


/// Only used for writer
bool ChildSession::setPage(const StringVector& tokens)
{
    int page;
    if (tokens.size() < 2 ||
        !getTokenInteger(tokens[1], "page", page))
    {
        sendTextFrameAndLogError("error: cmd=setpage kind=invalid");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    getLOKitDocument()->setPart(page);

    return true;
}

bool ChildSession::renderShapeSelection(const StringVector& tokens)
{
    std::string mimeType;
    if (tokens.size() != 2 ||
        !getTokenString(tokens[1], "mimetype", mimeType) ||
        mimeType != "image/svg+xml")
    {
        sendTextFrameAndLogError("error: cmd=rendershapeselection kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);

    char* output = nullptr;
    const std::size_t outputSize = getLOKitDocument()->renderShapeSelection(&output);
    if (output != nullptr && outputSize > 0)
    {
        constexpr std::string_view header = "shapeselectioncontent:\n";
        const size_t responseSize = header.size() + outputSize;
        std::unique_ptr<char[]> response(new char[responseSize]);
        std::memcpy(response.get(), header.data(), header.size());
        std::memcpy(response.get() + header.size(), output, outputSize);
        free(output);

        LOG_TRC("Sending response (" << responseSize << " bytes) for shapeselectioncontent on view #" << _viewId);
        sendBinaryFrame(response.get(), responseSize);
    }
    else
    {
        LOG_ERR("Failed to renderShapeSelection for view #" << _viewId);
    }

    return true;
}

bool ChildSession::removeTextContext(const StringVector& tokens)
{
    int id, before, after;
    std::string text;
    if (tokens.size() < 4 ||
        !getTokenInteger(tokens[1], "id", id) || id < 0 ||
        !getTokenInteger(tokens[2], "before", before) ||
        !getTokenInteger(tokens[3], "after", after))
    {
        sendTextFrameAndLogError("error: cmd=" + std::string(tokens[0]) + " kind=syntax");
        return false;
    }

    getLOKitDocument()->setView(_viewId);
    getLOKitDocument()->removeTextContext(id, before, after);

    return true;
}

bool ChildSession::setAccessibilityState(bool enable)
{
    getLOKitDocument()->setAccessibilityState(_viewId, enable);
    return true;
}

bool ChildSession::getA11yFocusedParagraph()
{
    getLOKitDocument()->setView(_viewId);

    LOKitHelper::ScopedString paragraphContent(getLOKitDocument()->getA11yFocusedParagraph());
    std::string paragraph(paragraphContent.get());
    sendTextFrame("a11yfocusedparagraph: " + paragraph);
    return true;
}

bool ChildSession::getA11yCaretPosition()
{
    getLOKitDocument()->setView(_viewId);
    int pos = getLOKitDocument()->getA11yCaretPosition();
    sendTextFrame("a11ycaretposition: " + std::to_string(pos));
    return true;
}

bool ChildSession::getPresentationInfo()
{
    getLOKitDocument()->setView(_viewId);

    LOKitHelper::ScopedString info(getLOKitDocument()->getPresentationInfo());
    std::string data(info.get());
    sendTextFrame("presentationinfo: " + data);
    return true;
}

/* If the user is inactive we have to remember important events so that when
 * the user becomes active again, we can replay the events.
 */
void ChildSession::rememberEventsForInactiveUser(const int type, const std::string& payload)
{
    if (type == KIT_CALLBACK_INVALIDATE_TILES)
    {
        _stateRecorder.recordInvalidate(); // TODO remember the area, not just a bool ('true' invalidates everything)
    }
    else if (type == KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR ||
             type == KIT_CALLBACK_CURSOR_VISIBLE ||
             type == KIT_CALLBACK_TEXT_SELECTION ||
             type == KIT_CALLBACK_TEXT_SELECTION_START ||
             type == KIT_CALLBACK_TEXT_SELECTION_END ||
             type == KIT_CALLBACK_CELL_FORMULA ||
             type == KIT_CALLBACK_CELL_CURSOR ||
             type == KIT_CALLBACK_GRAPHIC_SELECTION ||
             type == KIT_CALLBACK_DOCUMENT_SIZE_CHANGED ||
             type == KIT_CALLBACK_INVALIDATE_HEADER ||
             type == KIT_CALLBACK_INVALIDATE_SHEET_GEOMETRY ||
             type == KIT_CALLBACK_CELL_ADDRESS ||
             type == KIT_CALLBACK_REFERENCE_MARKS ||
             type == KIT_CALLBACK_A11Y_FOCUS_CHANGED ||
             type == KIT_CALLBACK_A11Y_CARET_CHANGED ||
             type == KIT_CALLBACK_A11Y_TEXT_SELECTION_CHANGED)
    {
        _stateRecorder.recordEvent(type, payload);
    }
    else if (type == KIT_CALLBACK_INVALIDATE_VIEW_CURSOR ||
             type == KIT_CALLBACK_TEXT_VIEW_SELECTION ||
             type == KIT_CALLBACK_CELL_VIEW_CURSOR ||
             type == KIT_CALLBACK_GRAPHIC_VIEW_SELECTION ||
             type == KIT_CALLBACK_VIEW_CURSOR_VISIBLE ||
             type == KIT_CALLBACK_VIEW_LOCK)
    {
        Poco::JSON::Parser parser;

        Poco::JSON::Object::Ptr root = parser.parse(payload).extract<Poco::JSON::Object::Ptr>();
        int viewId = root->getValue<int>("viewId");
        _stateRecorder.recordViewEvent(viewId, type, payload);
    }
    else if (type == KIT_CALLBACK_STATE_CHANGED)
    {
        std::string name;
        std::string value;
        if (COOLProtocol::parseNameValuePair(payload, name, value, '='))
        {
            _stateRecorder.recordState(name, payload);
        }
    }
    else if (type == KIT_CALLBACK_REDLINE_TABLE_SIZE_CHANGED ||
             type == KIT_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED ||
             type == KIT_CALLBACK_COMMENT)
    {
        _stateRecorder.recordEventSequence(type, payload);
    }
}

void ChildSession::updateSpeed()
{
    std::chrono::steady_clock::time_point now(std::chrono::steady_clock::now());

    while (_cursorInvalidatedEvent.size() != 0 &&
           (now - _cursorInvalidatedEvent.front()) > EventStorageInterval)
    {
        _cursorInvalidatedEvent.pop();
    }

    _cursorInvalidatedEvent.push(now);
    _docManager->updateEditorSpeeds(_viewId, _cursorInvalidatedEvent.size());
}

int ChildSession::getSpeed()
{
    std::chrono::steady_clock::time_point now(std::chrono::steady_clock::now());

    while (_cursorInvalidatedEvent.size() > 0 &&
           (now - _cursorInvalidatedEvent.front()) > EventStorageInterval)
    {
        _cursorInvalidatedEvent.pop();
    }

    return _cursorInvalidatedEvent.size();
}

#if (ENABLE_FEATURE_LOCK || ENABLE_FEATURE_RESTRICTION || ENABLE_DEBUG) && !MOBILEAPP
bool ChildSession::updateBlockingCommandStatus(const StringVector& tokens)
{
    std::string lockStatus, restrictedStatus;
    if (tokens.size() < 2 || !getTokenString(tokens[1], "isRestrictedUser", restrictedStatus))
    {
        sendTextFrameAndLogError("error: cmd=restrictionstatus kind=failure");
        return false;
    }
    else if (tokens.size() < 2 || !getTokenString(tokens[2], "isLockedUser", lockStatus))
    {
        sendTextFrameAndLogError("error: cmd=lockstatus kind=failure");
        return false;
    }
    std::string blockedCommands;
    if (restrictedStatus == "true")
    {
        blockedCommands += CommandControl::RestrictionManager::getRestrictedCommandListString();
#if ENABLE_DEBUG
        // Extract restricted commands passed from the wsd process.
        // Format: blockingcommandstatus isRestrictedUser=true isLockedUser=... test_restrictedCommands=cmd1 cmd2 ...
        std::string firstCmd;
        if (tokens.size() > 3 && getTokenString(tokens[3], "test_restrictedCommands", firstCmd))
        {
            blockedCommands += firstCmd;
            for (std::size_t i = 4; i < tokens.size(); ++i)
                blockedCommands += " " + tokens[i];
        }
#endif
    }
    if (lockStatus == "true")
        blockedCommands += blockedCommands.empty()
                               ? CommandControl::LockManager::getLockedCommandListString()
                               : " " + CommandControl::LockManager::getLockedCommandListString();

    getLOKitDocument()->setBlockedCommandList(_viewId, blockedCommands.c_str());
    return true;
}

std::string ChildSession::getBlockedCommandType(const std::string& command)
{
    if(CommandControl::RestrictionManager::getRestrictedCommandList().find(command)
    != CommandControl::RestrictionManager::getRestrictedCommandList().end())
        return "restricted";

    if (CommandControl::LockManager::getLockedCommandList().find(command) !=
        CommandControl::LockManager::getLockedCommandList().end())
        return "locked";

    return std::string();
}
#endif

bool ChildSession::sendProgressFrame(const char* id, const std::string& jsonProps,
                                     const std::string& forcedID)
{
    std::string msg = R"(progress: { "id":")";
    msg += id;
    msg += "\"";
    if (_docManager->isBackgroundSaveProcess())
        msg += R"(, "type":"bg")";
    if (!jsonProps.empty())
    {
        msg += ", ";
        msg += jsonProps;
    }
    if (!forcedID.empty())
    {
        msg += R"(, "forceid": ")" + forcedID + "\"";
    }
    msg += " }";
    return sendTextFrame(msg);
}

void ChildSession::loKitCallback(const int type, const std::string& payload)
{
    const char* const typeName = kitCallbackTypeToString(type);
    LOG_TRC("ChildSession::loKitCallback: " << typeName << " [" << payload << ']');

    if (!Util::isMobileApp() && UnitKit::get().filterLoKitCallback(type, payload))
        return;

    if (isCloseFrame())
    {
        LOG_TRC("Skipping callback [" << typeName << "] on closing session " << getName());
        return;
    }

    if (isDisconnected())
    {
        LOG_TRC("Skipping callback [" << typeName << "] on disconnected session " << getName());
        return;
    }

    if (!isActive())
    {
        rememberEventsForInactiveUser(type, payload);

        // Pass save and ModifiedStatus notifications through, block others.
        if (type != KIT_CALLBACK_UNO_COMMAND_RESULT || payload.find(".uno:Save") == std::string::npos)
        {
            if (payload.find(".uno:ModifiedStatus") == std::string::npos)
            {
                LOG_TRC("Skipping callback [" << typeName << "] on inactive session " << getName());
                return;
            }
        }
    }

    switch (static_cast<COKitCallbackType>(type))
    {
    case KIT_CALLBACK_INVALIDATE_TILES:
        {
            StringVector tokens(StringVector::tokenize(payload, ','));
            if (tokens.size() == 5 || tokens.size() == 6)
            {
                int part, x, y, width, height, mode = 0;
                try
                {
                    x = NumUtil::stoi(tokens[0]);
                    y = NumUtil::stoi(tokens[1]);
                    width = NumUtil::stoi(tokens[2]);
                    height = NumUtil::stoi(tokens[3]);
                    part = (_docType != "text" ? NumUtil::stoi(tokens[4])
                                               : 0); // Writer renders everything as part 0.
                    if (tokens.size() == 6)
                        mode = NumUtil::stoi(tokens[5]);
                }
                catch (const std::out_of_range&)
                {
                    // We might get INT_MAX +/- some delta that
                    // can overflow signed int and we end up here.
                    x = 0;
                    y = 0;
                    width = INT_MAX;
                    height = INT_MAX;
                    part = 0;
                    mode = 0;
                }

                sendTextFrame("invalidatetiles:"
                              " part=" + std::to_string(part) +
                              " mode=" + std::to_string(mode) +
                              " x=" + std::to_string(x) +
                              " y=" + std::to_string(y) +
                              " width=" + std::to_string(width) +
                              " height=" + std::to_string(height) +
                              " wid=" + std::to_string(getCurrentWireId()));
            }
            else if (tokens.size() == 2 && tokens.equals(0, "EMPTY"))
            {
                // without mode: "EMPTY, <part>, 0"
                const std::string part = (_docType != "text" ? tokens[1].c_str() : "0"); // Writer renders everything as part 0.
                sendTextFrame("invalidatetiles: EMPTY, " + part + ", 0" + " wid=" + std::to_string(getCurrentWireId()));
            }
            else if (tokens.size() == 3 && tokens.equals(0, "EMPTY"))
            {
                // with mode:    "EMPTY, <part>, <mode>"
                const std::string part = (_docType != "text" ? tokens[1].c_str() : "0"); // Writer renders everything as part 0.
                const std::string mode = tokens[2];
                sendTextFrame("invalidatetiles: EMPTY, " + part + ", " + mode +
                              " wid=" + std::to_string(getCurrentWireId()));
            }
            else
            {
                sendTextFrame("invalidatetiles: " + payload +
                              " wid=" + std::to_string(getCurrentWireId()));
            }
        }
        break;
    case KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        updateSpeed();
        updateCursorPositionJSON(payload);
        sendTextFrame("invalidatecursor: " + payload);
        break;
    case KIT_CALLBACK_TEXT_SELECTION:
        sendTextFrame("textselection: " + payload);
        break;
    case KIT_CALLBACK_TEXT_SELECTION_START:
        sendTextFrame("textselectionstart: " + payload);
        break;
    case KIT_CALLBACK_TEXT_SELECTION_END:
        sendTextFrame("textselectionend: " + payload);
        break;
    case KIT_CALLBACK_CURSOR_VISIBLE:
        sendTextFrame("cursorvisible: " + payload);
        break;
    case KIT_CALLBACK_GRAPHIC_SELECTION:
        sendTextFrame("graphicselection: " + payload);
        break;
    case KIT_CALLBACK_SHAPE_INNER_TEXT:
        sendTextFrame("graphicinnertextarea: " + payload);
        break;
    case KIT_CALLBACK_CELL_CURSOR:
        updateCursorPosition(payload);
        sendTextFrame("cellcursor: " + payload);
        break;
    case KIT_CALLBACK_CELL_FORMULA:
        sendTextFrame("cellformula: " + payload);
        break;
    case KIT_CALLBACK_MOUSE_POINTER:
        sendTextFrame("mousepointer: " + payload);
        break;
    case KIT_CALLBACK_HYPERLINK_CLICKED:
        sendTextFrame("hyperlinkclicked: " + payload);
        break;
    case KIT_CALLBACK_STATE_CHANGED:
    {
        if (payload == ".uno:NotesMode=true" || payload == ".uno:NotesMode=false" ||
            payload == ".uno:RedlineRenderMode=true" || payload == ".uno:RedlineRenderMode=false")
        {
            getLOKitDocument()->setView(_viewId);
            std::string status = LOKitHelper::documentStatus(getLOKitDocument()->get());
            sendTextFrame("statusupdate: " + status);
        }
        else if (payload.find(".uno:ModifiedStatus") != std::string::npos)
        {
            if (!_docManager->trackDocModifiedState(payload))
            {
                LOG_TRC("Forwarding " << payload << " after tracking modified state");
                sendTextFrame("statechanged: " + payload);
            }
            else
                LOG_TRC("Ignoring " << payload << " after tracking modified state");
        }
        else if (payload.find(".uno:CurrentPageResize") != std::string::npos)
        {
            getPartStatus();
        }
        else if (payload.find(".uno:PageZoomChange") != std::string::npos)
        {
            std::string zoomPercent = getZoomPercent(payload);
            sendTextFrame("changepagezoom:" + zoomPercent);
        }
        else
            sendTextFrame("statechanged: " + payload);

        break;
    }
    case KIT_CALLBACK_SEARCH_NOT_FOUND:
        sendTextFrame("searchnotfound: " + payload);
        break;
    case KIT_CALLBACK_SEARCH_RESULT_SELECTION:
        sendTextFrame("searchresultselection: " + payload);
        break;
    case KIT_CALLBACK_DOCUMENT_SIZE_CHANGED:
        getStatus();
        break;
    case KIT_CALLBACK_SET_PART:
    {
        int part;
        StringVector tokens(StringVector::tokenize(payload, ','));
        if (getTokenInteger(tokens[1], "part", part) &&
            getLOKitDocument()->getDocumentType() != KIT_DOCTYPE_TEXT)
            _currentPart = part;

        sendTextFrame("setpart: " + payload);
        break;
    }
    case KIT_CALLBACK_UNO_COMMAND_RESULT:
    {
        Parser parser;
        Poco::Dynamic::Var var = parser.parse(payload);
        const Object::Ptr& object = var.extract<Object::Ptr>();

        auto commandName = object->get("commandName");
        auto success = object->get("success");

        bool saveCommand = false;

        if (!commandName.isEmpty() && commandName.toString() == ".uno:Save")
        {
            if constexpr (!Util::isMobileApp())
            {
                consistencyCheckJail();

                copyForUpload(getJailedFilePath());

                saveCommand = true;
            }
            else
            {
                // After the document has been saved (into the temporary copy that we set up in
                // -[CODocument loadFromContents:ofType:error:]), save it also using the system API so
                // that file provider extensions notice.
                if (!success.isEmpty() && success.toString() == "true")
                {
#if defined(IOS)
                    CODocument* document =
                        DocumentData::get(_docManager->getMobileAppDocId()).coDocument;
                    [document saveToURL:[document fileURL]
                         forSaveOperation:UIDocumentSaveForOverwriting
                        completionHandler:^(BOOL success) {
                          LOG_TRC("ChildSession::loKitCallback() save completion handler gets "
                                  << (success ? "YES" : "NO"));
                          if (![[NSFileManager defaultManager] removeItemAtURL:document->copyFileURL
                                                                         error:nil])
                          {
                              LOG_SYS("Could not remove copy of document at "
                                      << [[document->copyFileURL path] UTF8String]);
                          }
                        }];
#elif defined(__ANDROID__)
                    postDirectMessage("SAVE " + payload);
#endif
                }
            }
        }

        if (!commandName.isEmpty() &&
            commandName.toString() == ".uno:TransformDocumentStructure")
        {
            // Core may return a detailed JSON result via SetReturnValue.
            // Extract the result string value if present; otherwise fall back
            // to the simple success boolean from the dispatch result.
            std::string resultJson;
            auto resultObj = object->getObject("result");
            if (resultObj)
            {
                std::string resultType;
                JsonUtil::findJSONValue(resultObj, "type", resultType);
                if (resultType == "string")
                    JsonUtil::findJSONValue(resultObj, "value", resultJson);
            }

            if (resultJson.empty())
            {
                bool bSuccess = !success.isEmpty() && success.toString() == "true";
                resultJson = "{\"success\":" + std::string(bSuccess ? "true" : "false") + "}";
            }
            sendTextFrame("transformeddocumentstructure: " + resultJson);
            break;
        }

        const std::string saveMessage = "unocommandresult: " + payload;
        sendTextFrame(saveMessage);
        if (saveCommand)
            _docManager->handleSaveMessage(saveMessage);
    }
    break;
    case KIT_CALLBACK_ERROR:
        {
            LOG_ERR("CALLBACK_ERROR: " << payload);
            Parser parser;
            Poco::Dynamic::Var var = parser.parse(payload);
            const Object::Ptr& object = var.extract<Object::Ptr>();

            sendTextFrameAndLogError("error: cmd=" + object->get("cmd").toString() +
                    " kind=" + object->get("kind").toString() + " code=" + object->get("code").toString());
        }
        break;
    case KIT_CALLBACK_CONTEXT_MENU:
        sendTextFrame("contextmenu: " + payload);
        break;
    case KIT_CALLBACK_STATUS_INDICATOR_START:
        sendProgressFrame("start",
                          std::string(R"("text": ")") + JsonUtil::escapeJSONValue(payload) + "\"");
        break;
    case KIT_CALLBACK_STATUS_INDICATOR_SET_VALUE:
        sendProgressFrame("setvalue", std::string("\"value\": ") + payload);
        break;
    case KIT_CALLBACK_STATUS_INDICATOR_FINISH:
        sendProgressFrame("finish", "");
        break;
    case KIT_CALLBACK_INVALIDATE_VIEW_CURSOR:
        updateCursorPositionJSON(payload);
        sendTextFrame("invalidateviewcursor: " + payload);
        break;
    case KIT_CALLBACK_TEXT_VIEW_SELECTION:
        sendTextFrame("textviewselection: " + payload);
        break;
    case KIT_CALLBACK_CELL_VIEW_CURSOR:
        updateCursorPositionJSON(payload);
        sendTextFrame("cellviewcursor: " + payload);
        break;
    case KIT_CALLBACK_GRAPHIC_VIEW_SELECTION:
        sendTextFrame("graphicviewselection: " + payload);
        break;
    case KIT_CALLBACK_VIEW_CURSOR_VISIBLE:
        sendTextFrame("viewcursorvisible: " + payload);
        break;
    case KIT_CALLBACK_VIEW_LOCK:
        sendTextFrame("viewlock: " + payload);
        break;
    case KIT_CALLBACK_REDLINE_TABLE_SIZE_CHANGED:
        sendTextFrame("redlinetablechanged: " + payload);
        break;
    case KIT_CALLBACK_REDLINE_TABLE_ENTRY_MODIFIED:
        sendTextFrame("redlinetablemodified: " + payload);
        break;
    case KIT_CALLBACK_COMMENT:
    {
        sendTextFrame("comment: " + payload);
        getStatus();
        break;
    }
    case KIT_CALLBACK_INVALIDATE_HEADER:
        sendTextFrame("invalidateheader: " + payload);
        break;
    case KIT_CALLBACK_CELL_ADDRESS:
        sendTextFrame("celladdress: " + payload);
        break;
    case KIT_CALLBACK_RULER_UPDATE:
        sendTextFrame("hrulerupdate: " + payload);
        break;
    case KIT_CALLBACK_VERTICAL_RULER_UPDATE:
        sendTextFrame("vrulerupdate: " + payload);
        break;
    case KIT_CALLBACK_WINDOW:
        sendTextFrame("window: " + payload);
        break;
    case KIT_CALLBACK_VALIDITY_LIST_BUTTON:
        sendTextFrame("validitylistbutton: " + payload);
        break;
    case KIT_CALLBACK_VALIDITY_INPUT_HELP:
        sendTextFrame("validityinputhelp: " + payload);
        break;
    case KIT_CALLBACK_CLIPBOARD_CHANGED:
    {
        if (_copyToClipboard)
        {
            _copyToClipboard = false;
            if (payload.empty())
                getTextSelectionInternal("");
            else
                sendTextFrame("clipboardchanged: " + payload);
        }

        break;
    }
    case KIT_CALLBACK_CONTEXT_CHANGED:
        sendTextFrame("context: " + payload);
        break;
    case KIT_CALLBACK_SIGNATURE_STATUS:
        sendTextFrame("signaturestatus: " + payload);
        break;

    case KIT_CALLBACK_PROFILE_FRAME:
    case KIT_CALLBACK_DOCUMENT_PASSWORD:
    case KIT_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY:
    case KIT_CALLBACK_DOCUMENT_PASSWORD_RESET:
        // these are not handled here.
        break;
    case KIT_CALLBACK_CELL_SELECTION_AREA:
        sendTextFrame("cellselectionarea: " + payload);
        break;
    case KIT_CALLBACK_CELL_AUTO_FILL_AREA:
        sendTextFrame("cellautofillarea: " + payload);
        break;
    case KIT_CALLBACK_TABLE_SELECTED:
        sendTextFrame("tableselected: " + payload);
        break;
    case KIT_CALLBACK_REFERENCE_MARKS:
        sendTextFrame("referencemarks: " + payload);
        break;
    case KIT_CALLBACK_JSDIALOG:
        sendTextFrame("jsdialog: " + payload);
        break;
    case KIT_CALLBACK_CALC_FUNCTION_LIST:
        sendTextFrame("calcfunctionlist: " + payload);
        break;
    case KIT_CALLBACK_TAB_STOP_LIST:
        sendTextFrame("tabstoplistupdate: " + payload);
        break;
    case KIT_CALLBACK_FORM_FIELD_BUTTON:
        sendTextFrame("formfieldbutton: " + payload);
        break;
    case KIT_CALLBACK_INVALIDATE_SHEET_GEOMETRY:
        sendTextFrame("invalidatesheetgeometry: " + payload);
        break;
    case KIT_CALLBACK_DOCUMENT_BACKGROUND_COLOR:
        sendTextFrame("documentbackgroundcolor: " + payload);
        break;
    case KIT_CALLBACK_APPLICATION_BACKGROUND_COLOR:
        sendTextFrame("applicationbackgroundcolor: " + payload);
        break;
    case KIT_CALLBACK_MEDIA_SHAPE:
        sendTextFrame("mediashape: " + payload);
        break;
    case KIT_CALLBACK_CONTENT_CONTROL:
        sendTextFrame("contentcontrol: " + payload);
        break;
    case KIT_COMMAND_BLOCKED:
        {
#if ENABLE_FEATURE_LOCK || ENABLE_FEATURE_RESTRICTION
            LOG_INF("COMMAND_BLOCKED: " << payload);
            Parser parser;
            Poco::Dynamic::Var var = parser.parse(payload);
            Object::Ptr object = var.extract<Object::Ptr>();

            std::string cmd = object->get("cmd").toString();
            sendTextFrame("blockedcommand: cmd=" + cmd +
                    " kind=" + getBlockedCommandType(cmd) + " code=" + object->get("code").toString());
#endif
        }
        break;
    case KIT_CALLBACK_PRINT_RANGES:
        sendTextFrame("printranges: " + payload);
        break;
    case KIT_CALLBACK_FONTS_MISSING:
        if constexpr (!Util::isMobileApp())
        {
            // This environment variable is always set in COOLWSD::innerInitialize().
            static std::string fontsMissingHandling = std::string(std::getenv("FONTS_MISSING_HANDLING"));
            if (fontsMissingHandling == "report" || fontsMissingHandling == "both")
                sendTextFrame("fontsmissing: " + payload);
            if (fontsMissingHandling == "log" || fontsMissingHandling == "both")
            {
#if 0
                Poco::JSON::Parser parser;
                Poco::JSON::Object::Ptr root = parser.parse(payload).extract<Poco::JSON::Object::Ptr>();

                const Poco::Dynamic::Var fontsMissing = root->get("fontsmissing");
                if (fontsMissing.isArray())
                    for (const auto &f : fontsMissing)
                        LOG_INF("Font missing: " << f.convert<std::string>());
#else
                LOG_INF("Fonts missing: " << payload);
#endif
            }
        }
        break;
    case KIT_CALLBACK_EXPORT_FILE:
    {
        bool isAbort = payload == "ABORT";
        bool isError = payload == "ERROR";
        bool isPending = payload == "PENDING";
        bool exportWasRequested = !_exportAsWopiUrl.empty();

        if (isPending) // dialog ret=ok, local save has been started
        {
            sendTextFrame("blockui: ");
            sendProgressFrame("start", "", "exporting");
            return;
        }
        else if (isAbort) // dialog ret=cancel, local save was aborted
        {
            _exportAsWopiUrl.clear();
            sendProgressFrame("finish", "", "exporting");
            return;
        }

        // this is export status message
        sendTextFrame("unblockui: ");
        sendProgressFrame("finish", "", "exporting");

        if (isError) // local save failed
        {
            _exportAsWopiUrl.clear();
            sendTextFrameAndLogError("error: cmd=exportas kind=saveasfailed");
            return;
        }

        // local save was successful

        if (exportWasRequested)
        {
            // The payload from LOKit is already a properly encoded file:// URL
            // (e.g., spaces as %20). Pass it through as-is — do NOT re-encode
            // with Poco::URI::encode(), which would double-encode percent signs
            // (%20 -> %2520) producing a path that doesn't match the file on disk.
            sendTextFrame("exportas: url=" + payload + " filename=" + _exportAsWopiUrl);

            _exportAsWopiUrl.clear();
            return;
        }

        // it was download request

#ifdef IOS
        NSURL *payloadURL = [NSURL URLWithString:[NSString stringWithUTF8String:payload.c_str()]];
        dispatch_async(dispatch_get_main_queue(), ^{
            CODocument *document = DocumentData::get(_docManager->getMobileAppDocId()).coDocument;
            [[document viewController] exportFileURL:payloadURL];
        });
#elif defined(_WIN32) || defined(QTAPP)
        // We don't need the registerdownload approach used by the browser.
        // Send the exported file URL to JS, which forwards it to the native
        // message handler for presenting a save dialog to the user.
        sendTextFrame("exportfile: url=" + payload);
#else
        // Register download id -> URL mapping in the DocumentBroker
        auto url = std::string("../../") + payload.substr(payload.find_last_of('/'));
        auto downloadId = Util::rng::getFilename(64);
        const std::string docBrokerMessage =
            "registerdownload: downloadid=" + downloadId + " url=" + url + " clientid=" + getId();
        _docManager->sendFrame(docBrokerMessage);
        std::string message = "downloadas: downloadid=" + downloadId + " port=" + std::to_string(ClientPortNumber) + " id=export";
        sendTextFrame(message);
#endif
        break;
    }
    case KIT_CALLBACK_A11Y_FOCUS_CHANGED:
    {
        sendTextFrame("a11yfocuschanged: " + payload);
        break;
    }
    case KIT_CALLBACK_A11Y_CARET_CHANGED:
    {
        sendTextFrame("a11ycaretchanged: " + payload);
        break;
    }
    case KIT_CALLBACK_A11Y_TEXT_SELECTION_CHANGED:
    {
        sendTextFrame("a11ytextselectionchanged: " + payload);
        break;
    }
    case KIT_CALLBACK_A11Y_FOCUSED_CELL_CHANGED:
    {
        sendTextFrame("a11yfocusedcellchanged: " + payload);
        break;
    }
    case KIT_CALLBACK_COLOR_PALETTES:
        sendTextFrame("colorpalettes: " + payload);
        break;
    case KIT_CALLBACK_A11Y_EDITING_IN_SELECTION_STATE:
    {
        sendTextFrame("a11yeditinginselectionstate: " + payload);
        break;
    }
    case KIT_CALLBACK_A11Y_SELECTION_CHANGED:
    {
        sendTextFrame("a11yselectionchanged: " + payload);
        break;
    }
    case KIT_CALLBACK_CORE_LOG:
    {
        sendTextFrame("corelog: " + payload);
        break;
    }
    case KIT_CALLBACK_TOOLTIP:
    {
        sendTextFrame("tooltip: " + payload);
        break;
    }
    default:
        LOG_ERR("Unknown callback event (" << kitCallbackTypeToString(type) << "): " << payload);
    }
}

void ChildSession::saveLogUiBackground()
{
    LogUiCommands uiLog(*this);
    uiLog.logSaveLoad("savebg", Poco::URI(getJailedFilePath()).getPath(), _logUiSaveBackGroundTimeStart);
}

void LogUiCommands::logLine(LogUiCommandsLine &line, bool isUndoChange)
{
    if constexpr (Util::isMobileApp())
        return;

    // log command
    double timeDiffStart = std::chrono::duration<double>(line._timeStart - _session._docManager->getLogUiCmd().getKitStartTimeSec()).count();

    // Load / Save event made by application will reach here.
    // In that case save without real userID
    int userID = _session._viewId;
    if (_session._clientVisibleArea.getWidth() == 0)
        userID = -1;

    std::stringstream strToLog;
    strToLog << "kit=" << _session._docManager->getDocId();
    strToLog << " time=" << std::fixed << std::setprecision(3) << timeDiffStart;
    if (Log::isLogUITimeEnd())
    {
        double timeDiffEnd = std::chrono::duration<double>(line._timeEnd - line._timeStart).count();
        strToLog << " dur=" << std::fixed << std::setprecision(3) << timeDiffEnd;
    }
    strToLog << " user=" << userID;
    if (!isUndoChange)
    {
        strToLog << " rep=" << line._repeat;
        strToLog << " cmd:" << line._cmd;
        if (line._subCmd != "")
            strToLog << " " << line._subCmd;
    }
    else
    {
        int changeRep=line._undoChange > 0 ? line._undoChange : -line._undoChange;
        strToLog << " rep=" << changeRep;
        strToLog << " undo-count-change:";
        if (line._undoChange > 0)
            strToLog << "+1";
        else
            strToLog << "-1";

        if (line._cmd == "uno" && (line._subCmd == ".uno:Undo" || line._subCmd == ".uno:Redo"))
        {
            strToLog << " " << line._subCmd;
        }
    }

    _session._docManager->getLogUiCmd().logUiCmdLine(userID, strToLog.str());

    if (!isUndoChange && line._undoChange != 0)
    {
        logLine(line, true);
    }
}

void LogUiCommands::logSaveLoad(std::string cmd, const std::string & path, std::chrono::steady_clock::time_point timeStart)
{
    if constexpr (Util::isMobileApp())
        return;

    LogUiCommandsLine uiLogLine;
    uiLogLine._timeStart = timeStart;
    uiLogLine._timeEnd = std::chrono::steady_clock::now();
    uiLogLine._repeat = 1;
    uiLogLine._cmd = std::move(cmd);

    std::size_t size = 0;
    const auto st = FileUtil::Stat(path);
    if (st.exists() && st.good())
    {
        size = st.size();
    }

    std::set<std::string> fileExtensions = { "sxw", "odt", "fodt", "sxc", "ods", "fods", "sxi", "odp", "fodp", "sxd", "odg", "fodg", "doc", "xls", "ppt", "docx", "xlsx", "pptx" };
    std::string extension = Poco::Path(path).getExtension();
    if (fileExtensions.find(extension) == fileExtensions.end())
        extension = "unknown";

    std::stringstream strToLog;
    strToLog << "size=" << size;
    strToLog << " ext=" << extension;

    uiLogLine._subCmd = strToLog.str();

    logLine(uiLogLine);
}

LogUiCommands::LogUiCommands(ChildSession& session, const StringVector* tokens)
    : _session(session), _tokens(tokens)
{
    if constexpr (Util::isMobileApp())
        return;

    if (_session._isDocLoaded)
        _document = session.getLOKitDocument();
}

LogUiCommands::~LogUiCommands()
{
    if constexpr (Util::isMobileApp())
        return;

    auto document = _document.lock();
    if (!document)
        return;
    if (!Log::isLogUIEnabled() || _session._clientVisibleArea.getWidth() == 0)
        return;
    if (_tokens->empty() || _cmdToLog.find((*_tokens)[0]) == _cmdToLog.end())
        return;
    int& lineCount = _session._lastUiCmdLinesLoggedCount;
    LogUiCommandsLine& line0 = _session._lastUiCmdLinesLogged[0];
    LogUiCommandsLine& line1 = _session._lastUiCmdLinesLogged[1];
    std::string actCmd;
    std::string actSubCmd;
    bool commandHandled = false;
    // drop, or modify some of the commands
    if (_tokens->equals(0, "key"))
    {
        // Do not log key release
        if (_tokens->equals(1, "type=up"))
            return;
        if (_tokens->equals(2, "char=0"))
        {
            uint32_t keyCode=0;
            (void)_tokens->getUInt32(3,"key",keyCode);
            actSubCmd.clear();
            if (keyCode & 8192)
                actSubCmd += "ctrl-";
            if (keyCode & 4096)
                actSubCmd += "shft-";
            keyCode &= 4095;
            if (keyCode >= 1024 && keyCode <= 1031)
            {
                // arrow keys = 1024-1027  home/end = 1028-1029  page up/down = 1030-1031
                const std::vector<std::string> navigationKeys = {"down","up","left","right","home","end","page-up","page-down"};
                actCmd = "key";
                actSubCmd += navigationKeys[keyCode-1024];
            }
            else
            {
                return;
            }
        }
        else
        {
            // if char!=0, this is probably a textinput key
            actCmd = "textinput";
        }
    }
    else if (_tokens->equals(0, "uno"))
    {
        if ( std::find_if(_unoCmdToNotLog.begin(), _unoCmdToNotLog.end(), [this](std::string const &S) { return (*_tokens)[1].starts_with(S); }) != _unoCmdToNotLog.end() )
            return;
        actCmd = (*_tokens)[0];
        actSubCmd = (*_tokens)[1];
        std::size_t pos = actSubCmd.find_first_of ('?');
        if (pos != std::string::npos) {
            actSubCmd = actSubCmd.substr (0,pos);
        }
    }
    else if (_tokens->equals(0, "mouse"))
    {
        actCmd = (*_tokens)[0];
        if ((*_tokens)[1].starts_with("type="))
        {
            actSubCmd = (*_tokens)[1].substr(5);

            // If it is a buttonup and we have a saved buttondown, than exchange it to click
            if (actSubCmd == "buttonup" && lineCount > 0
                && _session._lastUiCmdLinesLogged[lineCount - 1]._cmd == "mouse"
                && _session._lastUiCmdLinesLogged[lineCount - 1]._subCmd == "buttondown")
            {
                if (lineCount == 1)
                {
                    line0._subCmd = "click";
                    commandHandled = true;
                }
                else
                {
                    // drop the previous "buttondown", and change the actual to click
                    lineCount = 1;
                    actSubCmd = "click";
                    // If "buttondown" generated an undo state change, we should save it too.
                    line0._undoChange += line1._undoChange;
                }
            }
        }
        else
        {
            actSubCmd = (*_tokens)[1];
        }
    }
    else
    {
        actCmd = (*_tokens)[0];
    }

    // Here we are sure we want to log this command sometime...
    std::chrono::steady_clock::time_point actTime = std::chrono::steady_clock::now();

    // We have to check if undo-count-change happened because of it
    int undoAct = 0;
    int undoChg = 0;
    LOKitHelper::ScopedString undoCountString(document->getCommandValues(".uno:UndoCount"));
    undoAct = undoCountString ? atoi(undoCountString.get()) : 0;
    // If undo count decrease without an undo .uno:Undo, then it is probably a fake (when cap reached)
    if (_lastUndoCount!=undoAct && (_lastUndoCount<undoAct || actSubCmd == ".uno:Undo"))
    {
        if (undoAct - _lastUndoCount > 0)
            undoChg = 1;
        else
            undoChg = -1;
    }
    if (commandHandled)
    {
        // Now, possible only if a buttonup become click
        line0._undoChange += undoChg;
        line0._timeEnd = actTime;
        return;
    }

    // If there is a Stored command, we check if the new is mergeable
    //  Megre if we can
    //  Log previous and store new command, if we cannot merge
    if (lineCount >= 2)
    {
        // Possible only if, the stored commands are: mouse click + mouse button down
        // but the actual is not a mouse up .. that was handled before
        // We have to log the 1. line, and copy the 2. to the first.
        logLine(line1);
        line0 = line1;
        lineCount = 1;
    }
    if (lineCount > 0)
    {
        // Can we merge?
        if (line0._cmd == actCmd && line0._subCmd == actSubCmd)
        {
            // We can merge. We just change the last stored command
            line0._repeat += 1;
            line0._timeEnd = actTime;
            line0._undoChange += undoChg;
            return;
        }
        else if (line0._cmd == "mouse" && line0._subCmd == "click"
                 && actCmd == "mouse" && actSubCmd == "buttondown")
        {
            // mouse button down after a click, may become 2x click later, do not log it yet
            // Nothing to do here now. (lineCount == 1)
        }
        else
        {
            // We can not merge. We log the last stored command, and continue to store the actual command
            logLine(line0);
            lineCount = 0;
        }
    }
    // Store new command
    LogUiCommandsLine& lineAct = _session._lastUiCmdLinesLogged[lineCount];
    lineAct._cmd = std::move(actCmd);
    lineAct._subCmd = std::move(actSubCmd);
    lineAct._repeat = 1;
    lineAct._undoChange = undoChg;
    lineAct._timeStart = actTime;
    lineAct._timeEnd = actTime;
    lineCount++;

    if (!Log::isLogUIMerged())
    {
        // If we are not to merge the commands, then log the saved command now.
        logLine(line0);
        lineCount = 0;
    }
}


void ChildSession::updateCursorPosition(const std::string &rect)
{
    Util::Rectangle r(rect);
    if (r.getWidth() != 0 && r.getHeight() != 0)
        _cursorPosition = r;
    // else 'EMPTY' eg.
}

void ChildSession::updateCursorPositionJSON(const std::string &rect)
{
    Poco::JSON::Parser parser;
    const Poco::Dynamic::Var result = parser.parse(rect);
    const auto& command = result.extract<Poco::JSON::Object::Ptr>();
    updateCursorPosition(command->get("rectangle").toString());
}

std::string ChildSession::getZoomPercent(const std::string &payload)
{
    const auto eq = payload.find('=');
    if (eq == std::string::npos || eq + 1 >= payload.size())
        return std::string();

    size_t i = eq + 1;
    while (i < payload.size() && std::isdigit(payload[i]))
        ++i;

    return payload.substr(eq + 1, i - (eq + 1));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
