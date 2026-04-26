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
 * The main entry point for the COKit process serving
 * a document editing session.
 */

#include <config.h>

#include "KitWebSocket.hpp"

// Work around a problem in Poco 1.14.2 and/or Visual Studio and clang-cl: Include <typeinfo> here.
#include <typeinfo>

#include <common/Anonymizer.hpp>
#include <common/JsonUtil.hpp>
#include <common/Seccomp.hpp>
#include <common/SigUtil.hpp>
#include <common/TraceEvent.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <kit/ChildSession.hpp>
#include <kit/Kit.hpp>

#include <Poco/URI.h>

#include <sys/types.h>

#ifndef _WIN32
#include <sys/wait.h>
#include <sysexits.h>
#endif

using Poco::Exception;

void KitWebSocketHandler::handleMessage(const std::vector<char>& data)
{
    // To get A LOT of Trace Events, to exercise their handling, uncomment this:
    // ProfileZone profileZone("KitWebSocketHandler::handleMessage");

    std::string message(data.data(), data.size());

    if (!Util::isMobileApp() && UnitKit::get().filterKitMessage(this, message))
        return;

    StringVector tokens = StringVector::tokenize(message);

    LOG_DBG(_socketName << ": recv [" << [&](auto& log) {
        for (const auto& token : tokens)
        {
            // Don't log user-data, there are anonymized versions that get logged instead.
            if (tokens.startsWith(token, "jail") || tokens.startsWith(token, "author") ||
                tokens.startsWith(token, "name") || tokens.startsWith(token, "url"))
                continue;

            std::string tokenStr = tokens.getParam(token);

            std::string::size_type pos = tokenStr.find("\"Author\":{\"type\":\"string\",\"value\":\"");
            if (pos != std::string::npos) {
                auto start = tokenStr.find("\"value\":\"", pos) + 9;
                auto end = tokenStr.find("\"", start);
                std::string value = tokenStr.substr(start, end - start);
                tokenStr.replace(start, end - start, Anonymizer::anonymize(value));
            }

            log << tokenStr << ' ';
        }
    });

    // Note: Syntax or parsing errors here are unexpected and fatal.
    if (SigUtil::getTerminationFlag())
    {
        LOG_DBG("Too late, TerminationFlag is set, we're going down");
    }
    else if (tokens.equals(0, "session"))
    {
        const std::string& sessionId = tokens[1];
        _docKey = tokens[2];
        const std::string& docId = tokens[3];
        const std::string url = Uri::decode(_docKey);
        const std::string fileId = Uri::getFilenameFromURL(url);
        Anonymizer::mapAnonymized(fileId,
                                  fileId); // Identity mapping, since fileId is already obfuscated

        if (!_document)
        {
#if !defined(IOS) && !defined(QTAPP) && !defined(MACOS) && !defined(_WIN32)
            ProcUtil::setThreadName("kit" SHARED_DOC_THREADNAME_SUFFIX + docId);
#endif
            _document = std::make_shared<Document>(
                _loKit, _jailId, _docKey, docId, url,
                std::static_pointer_cast<WebSocketHandler>(shared_from_this()), _mobileAppDocId);
            _ksPoll->setDocument(_document);

            // We need to send the process name information to WSD if Trace Event recording is enabled (but
            // not turned on) because it might be turned on later.
            // We can do this only after creating the Document object.
            TraceEvent::emitOneRecordingIfEnabled(
                std::string("{\"name\":\"process_name\",\"ph\":\"M\",\"args\":{\"name\":\"") +
                "Kit-" + docId + "\"},\"pid\":" + std::to_string(ProcUtil::getProcessId()) +
                ",\"tid\":" + std::to_string(ProcUtil::getThreadId()) + "},\n");
        }

        // Validate and create session.
        if (!(url == _document->getUrl() && _document->createSession(sessionId)))
        {
            LOG_DBG("CreateSession failed.");
        }
    }
    else if (!Util::isFuzzing() && tokens.equals(0, "exit"))
    {
        if constexpr (!Util::isMobileApp())
        {
            LOG_INF("Terminating immediately due to parent 'exit' command.");
            flushTraceEventRecordings();
            // flushes logging
            if (_document)
                _document->joinThreads();
            _document.reset();
            if constexpr (!Util::isKitInProcess())
                Util::forcedExit(EX_OK);
            else
                SigUtil::setTerminationFlag();
        }
        else
        {
#if defined(IOS) || defined(QTAPP) || defined(MACOS) || defined(_WIN32)
            LOG_INF("Setting our KitSocketPoll's termination flag due to 'exit' command.");
            std::unique_lock<std::mutex> lock(_ksPoll->termination->mutex);
            _ksPoll->termination->flag = true;
            _ksPoll->termination->cv.notify_all();
#else
            LOG_INF("Setting TerminationFlag due to 'exit' command.");
            SigUtil::setTerminationFlag();
#endif
            _document.reset();
        }
    }
    else if (tokens.equals(0, "tile") || tokens.equals(0, "tilecombine") ||
             tokens.equals(0, "getslide") ||
             tokens.equals(0, "paintwindow") || tokens.equals(0, "resizewindow") ||
             COOLProtocol::getFirstToken(tokens[0], '-') == "child")
    {
        if (_document)
        {
            _document->queueMessage(message);
        }
        else
        {
            LOG_WRN("No document while processing " << tokens[0] << " request.");
        }
    }
    else if (!Util::isFuzzing() && tokens.size() == 3 && tokens.equals(0, "setconfig"))
    {
#if !MOBILEAPP && !defined(BUILDING_TESTS)
        // Currently only rlimit entries are supported.
        if (!Rlimit::handleSetrlimitCommand(tokens))
        {
            LOG_ERR("Unknown setconfig command: " << message);
        }
#endif
    }
    else if (!Util::isFuzzing() && tokens.equals(0, "setloglevel"))
    {
        Log::setLevel(tokens[1]);
    }
    else if constexpr (!Util::isFuzzing())
    {
        LOG_ERR("Bad or unknown token [" << tokens[0] << ']');
    }
}

void KitWebSocketHandler::enableProcessInput(bool enable)
{
    LOG_TRC("Kit socket - input processing now: " <<
            (enable ? "enabled" : "disabled") <<
            " was " <<
            (WebSocketHandler::processInputEnabled() ? "enabled" : "disabled"));
    WebSocketHandler::enableProcessInput(enable);

    // Wake up poll to process data from socket input buffer
    if (enable && _ksPoll)
        _ksPoll->wakeup();
}

void KitWebSocketHandler::shutdownForBackgroundSave()
{
    // Don't shutdown the app when this socket closes
    _backgroundSaver = true;
}

void KitWebSocketHandler::onDisconnect()
{
    if (_backgroundSaver)
    {
        LOG_TRC("Ignoring hard disconnect of duplicate kit -> wsd socket in wsd");
        return;
    }

    if constexpr (!Util::isMobileApp())
    {
        //FIXME: We could try to recover.
        LOG_ERR("Kit for DocBroker ["
                << _docKey
                << "] connection lost without exit arriving from wsd. Setting TerminationFlag");
        SigUtil::setTerminationFlag();
    }
#if defined(IOS) || defined(QTAPP) || defined(MACOS) || defined(_WIN32)
    {
        std::unique_lock<std::mutex> lock(_ksPoll->termination->mutex);
        _ksPoll->termination->flag = true;
        _ksPoll->termination->cv.notify_all();
    }
#endif
    _ksPoll.reset();
}

// transient background save child message handler

void BgSaveChildWebSocketHandler::handleMessage(const std::vector<char>& data)
{
    LOG_DBG(_socketName << ": recv from parent [" <<
            COOLProtocol::getAbbreviatedMessage(data));
}

void BgSaveChildWebSocketHandler::onDisconnect()
{
    LOG_TRC("Disconnected background web socket to parent kit");
    UnitKit::get().preBackgroundSaveExit();
#if !MOBILEAPP
    Document::shutdownBackgroundWatchdog();
#endif
    Util::forcedExit(EX_OK);
}

BgSaveChildWebSocketHandler::~BgSaveChildWebSocketHandler()
{
    LOG_TRC("Close web socket to parent kit");
}

// Kit handler for messages from transient background save Kit

BgSaveParentWebSocketHandler::BgSaveParentWebSocketHandler(
    const std::string& socketName, const pid_t childPid, std::shared_ptr<Document> document,
    const std::shared_ptr<ChildSession>& session)
    : WebSocketHandler(/* isClient = */ false, /* isMasking */ false)
    , _childPid(childPid)
    , _saveCompleted(false)
    , _socketName(socketName)
    , _document(std::move(document))
    , _session(session)
{
    _document->bgSaveStarted();
}

BgSaveParentWebSocketHandler::~BgSaveParentWebSocketHandler() { _document->bgSaveEnded(); }

void BgSaveParentWebSocketHandler::terminateSave(const std::string &reason)
{
    LOG_TRC("terminating bgsave: " << reason);

    // Hard terminate the bgsave child
    sendTextMessage("exit");
    shutdown(true, "unexpected jsdialog");

    reportFailedSave(reason);
}

void BgSaveParentWebSocketHandler::reportFailedSave(const std::string &reason)
{
    // next time we get a non-background save.
    _document->disableBgSave(reason);

    // Synthesize a failed save result
    // FIXME: could this allow another new manual save to race against the ongoing bgsave ?
    // either way - that's better than hanging and blocking if we get interactive dialogs on save.
    const std::string saveFailed =
        "client-" + _session->getId() +
        " unocommandresult: { \"commandName\": \".uno:Save\", \"success\": false }";
    _document->sendFrame(saveFailed, WSOpCode::Text);

    _document->updateModifiedOnFailedBgSave();
    _saveCompleted = true;
}

void BgSaveParentWebSocketHandler::handleMessage(const std::vector<char>& data)
{
    const StringVector tokens = StringVector::tokenize(data.data(), data.size());

    // Only accept messages that make sense
    if (tokens[1] != "error:" &&
        tokens[1] != "jsdialog:" &&
        tokens[1] != "progress:" &&
        tokens[1] != "traceevent:" &&
        tokens[1] != "forcedtraceevent:" &&
        tokens[1] != "unocommandresult:")
    {
        LOG_TRC("Ignore un-wanted message from bg child: " <<
                COOLProtocol::getAbbreviatedMessage(data));
        return;
    }

    LOG_DBG(_socketName << ": recv from bg child [" <<
            COOLProtocol::getAbbreviatedMessage(data));

    if (tokens[1] == "jsdialog:")
    {
        Poco::JSON::Object::Ptr object;
        if (JsonUtil::parseJSON(tokens.cat(' ', 2), object) &&
            (object->get("jsontype").toString() == "notebookbar" ||
             object->get("jsontype").toString() == "sidebar" ||
             object->get("jsontype").toString() == "formulabar"))
            // white-listing to avoid popup & dialog & other interactive errors
        {
            LOG_DBG("Unexpected but benign jsdialog message from bgsave process " <<
                    COOLProtocol::getAbbreviatedMessage(data));
            return;
        }

        terminateSave("Unexpected jsdialog message: " +
                      COOLProtocol::getAbbreviatedMessage(data));
        return;
    }

    // Messages already include client-foo prefixes inherited from ourselves
    _document->sendFrame(std::string_view(data.data(), data.size()), WSOpCode::Text);

    if (tokens[1] == "error:")
        _document->disableBgSave("on save error");

    // Status update messages are stuck in the bgsave's Idle CallbackFlushHandler
    if (tokens[1] == "unocommandresult:")
    {
        std::string msg(data.data(), data.size());
        Poco::JSON::Object::Ptr object;
        if (JsonUtil::parseJSON(msg, object) &&
            object->get("commandName").toString() == ".uno:Save")
        {
            if (object->get("success").toString() == "true")
            {
                _document->notifySyntheticUnmodifiedState();
                _session->saveLogUiBackground();
            }
            else
            {
                _document->updateModifiedOnFailedBgSave();
                LOG_DBG("Failed to save, not synthesizing modified state");
                _document->disableBgSave("on failed save");
            }
            _saveCompleted = true;
        }
    }
}

void BgSaveParentWebSocketHandler::onDisconnect()
{
    LOG_TRC("Disconnected background web socket to child " << _childPid);

    if constexpr (!Util::isMobileApp())
    {
        // reap and de-zombify children.
        const auto [ret, sig] = SigUtil::reapZombieChild(_childPid, /*sighandler=*/false);
        if (sig)
            reportFailedSave(std::string("crashed with status ") + SigUtil::signalName(sig));
        else if (ret <= 0)
            LOG_WRN("Background save process disconnected but not terminated " << _childPid);
    }

    if (!_saveCompleted)
        reportFailedSave("terminated without saving");

    WebSocketHandler::onDisconnect(); // Invoke the default handler.
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
