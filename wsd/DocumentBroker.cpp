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
 * Implementation of document broker and session management.
 * Classes: DocumentBroker, ConvertToBroker
 */

#include <config.h>

#include "DocumentBroker.hpp"

#include <common/Anonymizer.hpp>
#include <common/Authorization.hpp>
#include <common/Clipboard.hpp>
#include <common/CommandControl.hpp>
#include <common/Common.hpp>
#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Message.hpp>
#include <common/Protocol.hpp>
#include <common/TraceEvent.hpp>
#include <common/Unit.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <net/HttpServer.hpp>
#include <net/Socket.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/CacheUtil.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/Exceptions.hpp>
#include <wsd/FileServer.hpp>
#include <wsd/PlatformDesktop.hpp>
#include <wsd/PresetsInstall.hpp>
#include <wsd/Process.hpp>
#include <wsd/ProxyProtocol.hpp>
#include <wsd/QuarantineUtil.hpp>
#include <wsd/Storage.hpp>
#include <wsd/TileCache.hpp>

#include <Poco/DigestStream.h>
#include <Poco/Exception.h>
#include <Poco/Path.h>
#include <Poco/SHA1Engine.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <ctime>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#ifndef _WIN32
#include <sysexits.h>
#endif
#include <utility>

using namespace std::literals;
using namespace COOLProtocol;

using Poco::JSON::Object;

void UrpHandler::handleIncomingMessage(SocketDisposition&)
{
    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (!socket)
    {
        LOG_ERR("Invalid socket while handling incoming client request");
        return;
    }

    Buffer& data = socket->getInBuffer();
    if (data.empty())
    {
        LOG_DBG("No data to process from the socket");
        return;
    }

    ChildProcess* child = _childProcess;
    std::shared_ptr<DocumentBroker> docBroker =
        child && child->getPid() > 0 ? child->getDocumentBroker() : nullptr;
    if (docBroker)
        docBroker->onUrpMessage(data.data(), data.size());

    // Remove consumed data.
    data.clear();
}

void ChildProcess::setDocumentBroker(const std::shared_ptr<DocumentBroker>& docBroker)
{
    assert(docBroker && "Invalid DocumentBroker instance.");
    _docBroker = docBroker;

    // The prisoner socket is added in 'takeSocket'

    // if URP is enabled, also add its socket to the poll
    if (_urpFromKitFD != -1 && _urpToKitFD != -1)
    {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

        std::shared_ptr<StreamSocket> urpFromKit = StreamSocket::create<StreamSocket>(
            std::string(), _urpFromKitFD, Socket::Type::Unix, /*isClient=*/false,
            HostType::Other, std::make_shared<UrpHandler>(this),
            StreamSocket::ReadType::NormalRead, now);
        docBroker->addSocketToPoll(urpFromKit);
        _urpFromKit = urpFromKit;

        std::shared_ptr<StreamSocket> urpToKit = StreamSocket::create<StreamSocket>(
            std::string(), _urpToKitFD, Socket::Type::Unix, /*isClient=*/false,
            HostType::Other, std::make_shared<UrpHandler>(this),
            StreamSocket::ReadType::NormalRead, now);
       docBroker->addSocketToPoll(urpToKit);
       _urpToKit = urpToKit;
    }

    if (UnitWSD::isUnitTesting())
    {
        UNITWSD_CALL(onDocBrokerAttachKitProcess(docBroker->getDocKey(), getPid()));
    }
}

void DocumentBroker::broadcastLastModificationTime(
    const std::shared_ptr<ClientSession>& session) const
{
    if (_storageManager.getLastModifiedServerTimeString().empty())
        // No time from the storage (e.g., SharePoint 2013 and 2016) -> don't send
        return;

    std::ostringstream stream;
    stream << "lastmodtime: " << _storageManager.getLastModifiedServerTimeString();
    const std::string message = stream.str();

    // While loading, the current session is not yet added to
    // the sessions container, so we need to send to it directly.
    if (session)
        session->sendTextFrame(message);
    broadcastMessage(message);
}

/// The Document Broker Poll - one of these in a thread per document
class DocumentBroker::DocumentBrokerPoll final : public TerminatingPoll
{
    /// The DocumentBroker owning us.
    DocumentBroker& _docBroker;

public:
    DocumentBrokerPoll(const std::string &threadName, DocumentBroker& docBroker) :
        TerminatingPoll(threadName),
        _docBroker(docBroker)
    {
    }

    void pollingThread() override
    {
        // Delegate to the docBroker.
        _docBroker.pollThread();

        // We are done; let's clean up. (Is it excessive to be impatient?)
        LOG_TRC("Waking up world after finishing DocBroker poll");
        SocketPoll::wakeupWorld();
    }
};

std::atomic<unsigned> DocumentBroker::DocBrokerId(1);

DocumentBroker::DocumentBroker(ChildType type, const std::string& uri, const Poco::URI& uriPublic,
                               const std::string& docKey, const std::string& configId,
                               unsigned mobileAppDocId)
    : _unitWsd(UnitWSD::isUnitTesting() ? &UnitWSD::get() : nullptr)
    , _uriOrig(uri)
    , _limitLifeSeconds(std::chrono::seconds::zero())
    , _uriPublic(uriPublic)
    , _saveManager((std::getenv("COOL_NO_AUTOSAVE") != nullptr)
                       ? std::chrono::seconds::zero()
                       : ConfigUtil::getConfigValue<std::chrono::seconds>(
                             "per_document.idlesave_duration_secs", 30),
                   (std::getenv("COOL_NO_AUTOSAVE") != nullptr)
                       ? std::chrono::seconds::zero()
                       : ConfigUtil::getConfigValue<std::chrono::seconds>(
                             "per_document.autosave_duration_secs", 300),
                   ConfigUtil::getConfigValue<std::chrono::milliseconds>(
                       "per_document.min_time_between_saves_ms", 500))
    , _storageManager(ConfigUtil::getConfigValue<std::chrono::milliseconds>(
          "per_document.min_time_between_uploads_ms", 5000))
    , _docKey(docKey)
    , _docId(Util::encodeId(DocBrokerId++, 3))
    , _configId(configId)
    , _poll(
          std::make_shared<DocumentBrokerPoll>("doc" SHARED_DOC_THREADNAME_SUFFIX + _docId, *this))
    , _slideLayerCache(25)
    , _lockCtx(std::make_unique<LockContext>())
#if !MOBILEAPP
    , _admin(Admin::instance())
#endif
    , _createTime(std::chrono::steady_clock::now())
    , _loadDuration(0)
    , _wopiDownloadDuration(0)
    , _tileVersion(0)
    , _cursorPosX(0)
    , _cursorPosY(0)
    , _cursorWidth(0)
    , _cursorHeight(0)
    , _debugRenderedTileCount(0)
    , _mobileAppDocId(mobileAppDocId)
    , _type(type)
    , _isModified(false)
    , _stop(false)
    , _documentChangedInStorage(false)
    , _isViewFileExtension(false)
    , _isViewSettingsUpdated(false)
    , _alwaysSaveOnExit(ConfigUtil::getConfigValue<bool>("per_document.always_save_on_exit", false))
    , _backgroundAutoSave(
          ConfigUtil::getConfigValue<bool>("per_document.background_autosave", true))
    , _backgroundManualSave(
          ConfigUtil::getConfigValue<bool>("per_document.background_manualsave", true))
{
    assert(!_docKey.empty());
    assert(!COOLWSD::ChildRoot.empty());

    if constexpr (!Util::isMobileApp())
        assert(_mobileAppDocId == 0 && "Unexpected to have mobileAppDocId in the non-mobile build");
#if defined(IOS) || defined(QTAPP) || defined(MACOS) || defined(_WIN32)
    assert(_mobileAppDocId > 0 && "Unexpected to have no mobileAppDocId in a mobile app");
#endif

    LOG_INF("DocumentBroker [" << COOLWSD::anonymizeUrl(_uriPublic.toString())
                               << "] created with docKey [" << _docKey
                               << "], always_save_on_exit: " << _alwaysSaveOnExit);

    UNITWSD_CALL_INSTANCE(_unitWsd, DocBrokerCreate(_docKey));
}

pid_t DocumentBroker::getPid() const { return _childProcess ? _childProcess->getPid() : 0; }

void DocumentBroker::setupPriorities()
{
    if constexpr (Util::isMobileApp())
        return;
    if (_type == ChildType::Batch)
    {
        const int prio = ConfigUtil::getConfigValue<int>("per_document.batch_priority", 5);
        ProcUtil::setProcessAndThreadPriorities(_childProcess->getPid(), prio);
    }
}

void DocumentBroker::setupTransfer(SocketDisposition &disposition,
                                   SocketDisposition::MoveFunction transferFn)
{
    disposition.setTransfer(*_poll, std::move(transferFn));
}

void DocumentBroker::setupTransfer(SocketPoll& from, const std::weak_ptr<StreamSocket>& socket,
                                   SocketDisposition::MoveFunction transferFn) const
{
    from.transferSocketTo(socket, getPoll(), std::move(transferFn), nullptr);
}

static std::chrono::seconds getLimitLoadSecs()
{
    // 0 = infinite.
    CONFIG_STATIC const auto value =
        ConfigUtil::getConfigValue<std::chrono::seconds>("per_document.limit_load_secs", 100, 0);
    if (value < std::chrono::seconds::zero())
    {
        return std::chrono::seconds(100);
    }

    return value;
}

void DocumentBroker::assertCorrectThread(LOG_CAPTURE_CALLER) const
{
    _poll->assertCorrectThread(LOG_PASS_PARENT_CALLER);
}

void DocumentBroker::clearCaches()
{
    if (_tileCache)
        _tileCache->clear();
    _slideLayerCache.erase_all();
}

// The inner heart of the DocumentBroker - our poll loop.
void DocumentBroker::pollThread()
{
    std::chrono::steady_clock::time_point threadStart = std::chrono::steady_clock::now();

    LOG_INF("Starting docBroker polling thread for docKey [" << _docKey << ']' << " and configId [" << _configId << ']');

    // Request a kit process for this doc.
    do
    {
        static constexpr std::chrono::milliseconds timeoutMs(COMMAND_TIMEOUT_MS * 5);
        _childProcess = getNewChild_Blocks(_poll, _configId, _mobileAppDocId);
        if (_childProcess || (std::chrono::steady_clock::now() - threadStart) > timeoutMs)
            break;

        // Nominal time between retries, lest we busy-loop. getNewChild could also wait, so don't double that here.
        std::this_thread::sleep_for(std::chrono::milliseconds(CHILD_REBALANCE_INTERVAL_MS / 10));
    } while (!_stop && _poll->continuePolling() && !SigUtil::getShutdownRequestFlag());

    if (!_childProcess)
    {
        // Let the client know we can't serve now.
        LOG_ERR("Failed to get new child.");

        // FIXME: need to notify all clients and shut this down ...
        // FIXME: return something good down the websocket ...
#if 0
        const std::string msg = SERVICE_UNAVAILABLE_INTERNAL_ERROR;
        ws.sendMessage(msg);
        // abnormal close frame handshake
        ws.shutdown(WebSocketHandler::StatusCodes::ENDPOINT_GOING_AWAY);
#endif
        stop("Failed to get new child.");

        // Stop to mark it done and cleanup.
        _poll->stop();

        // Async cleanup.
        COOLWSD::doHousekeeping();

        LOG_INF("Finished docBroker polling thread for docKey [" << _docKey << ']');
        return;
    }

    // We have a child process.
    _childProcess->setDocumentBroker(shared_from_this());
    LOG_INF("Doc [" << _docKey << "] attached to child [" << _childProcess->getPid() << ']');

    setupPriorities();

    CONFIG_STATIC const std::chrono::seconds IdleDocTimeoutSecs =
        ConfigUtil::getConfigValue<std::chrono::seconds>("per_document.idle_timeout_secs", 3600);
#if !MOBILEAPP
    if (IdleDocTimeoutSecs <= std::chrono::seconds(15))
    {
        LOG_WRN("The configured per_document.idle_timeout_secs ["
                << IdleDocTimeoutSecs << "] is too low, consider increasing it");
    }

    // Used to accumulate B/W deltas.
    uint64_t adminSent = 0;
    uint64_t adminRecv = 0;
    auto lastBWUpdateTime = std::chrono::steady_clock::now();
    auto lastClipboardHashUpdateTime = std::chrono::steady_clock::now();

    const std::chrono::seconds limit_load_secs =
#if ENABLE_DEBUG
        // paused waiting for a debugger to attach
        // ignore load time out
        std::getenv("PAUSEFORDEBUGGER") ? std::chrono::seconds::max() :
#endif
                                        getLimitLoadSecs();

    auto loadDeadline = std::chrono::steady_clock::now() + limit_load_secs;
#endif

    const auto limStoreFailures =
        ConfigUtil::getConfigValue<int>("per_document.limit_store_failures", 5);

    bool waitingForMigrationMsg = false;
    std::chrono::time_point<std::chrono::steady_clock> migrationMsgStartTime;
    CONFIG_STATIC const std::chrono::microseconds migrationMsgTimeout =
        ConfigUtil::getConfigValue<std::chrono::seconds>(
            "indirection_endpoint.migration_timeout_secs", 180);

    const auto defaultPollTimeout = std::min<std::chrono::microseconds>(
        _lockCtx->refreshPeriod(), SocketPoll::DefaultPollTimeoutMicroS);

    // Main polling loop goodness.
    while (!_stop && _poll->continuePolling() && !SigUtil::getTerminationFlag())
    {
        // Poll more frequently while unloading to cleanup sooner.
        _poll->poll(isUnloading() ? SocketPoll::DefaultPollTimeoutMicroS / 16 : defaultPollTimeout);

        // Consolidate updates across multiple processed events.
        processBatchUpdates();

        if (_stop)
        {
            LOG_DBG("Doc [" << _docKey << "] is flagged to stop after returning from poll.");
            break;
        }

        if (UNITWSD_CALL_INSTANCE(_unitWsd, isFinished()))
        {
            stop("UnitTestFinished");
            break;
        }

#if !MOBILEAPP
        const auto now = std::chrono::steady_clock::now();

        // a tile's data is ~8k, a 4k screen is ~256 256x256 tiles -
        // so double that - 4Mb per view.
        if (_tileCache)
            _tileCache->setMaxCacheSize(8 * 1024 * 256 * 2 * _sessions.size());

        if (isInteractive())
        {
            // It is possible to dismiss the interactive dialog,
            // exit the Kit process, or even crash. We would deadlock.
            if (isUnloading())
            {
                // We expect to have either isMarkedToDestroy() or
                // isCloseRequested() in that case.
                stop("abortedinteractive");
            }

            // Extend the deadline while we are interactiving with the user.
            loadDeadline = now + limit_load_secs;
            continue;
        }

        if (!isLoaded() && (limit_load_secs > std::chrono::seconds::zero()) && (now > loadDeadline))
        {
            LOG_ERR("Doc [" << _docKey << "] is taking too long to load. Will kill process ["
                    << _childProcess->getPid() << "]. per_document.limit_load_secs set to "
                    << limit_load_secs << " secs.");
            broadcastMessage("error: cmd=load kind=docloadtimeout");

            // Brutal but effective.
            if (_childProcess)
                _childProcess->terminate();

            stop("Doc lifetime expired");
            continue;
        }

        // Check if we had a sunset time and expired.
        if (_limitLifeSeconds > std::chrono::seconds::zero() &&
            (now - threadStart) > _limitLifeSeconds)
        {
            LOG_WRN("Doc [" << _docKey << "] is taking too long to convert. Will kill process ["
                            << _childProcess->getPid()
                            << "]. per_document.limit_convert_secs set to "
                            << _limitLifeSeconds.count() << " secs.");
            broadcastMessage("error: cmd=load kind=docexpired");

            // Brutal but effective.
            if (_childProcess)
                _childProcess->terminate();

            stop("Convert-to timed out");
            continue;
        }

        if ((now - lastBWUpdateTime) >= std::chrono::milliseconds(COMMAND_TIMEOUT_MS))
        {
            lastBWUpdateTime = now;
            uint64_t sent = 0, recv = 0;
            getIOStats(sent, recv);

            uint64_t deltaSent = 0, deltaRecv = 0;

            // connection drop transiently reduces this.
            if (sent > adminSent)
            {
                deltaSent = sent - adminSent;
                adminSent = sent;
            }
            if (recv > deltaRecv)
            {
                deltaRecv = recv - adminRecv;
                adminRecv = recv;
            }
            LOG_TRC("Doc [" << _docKey << "] added stats sent: +" << deltaSent << ", recv: +" << deltaRecv << " bytes to totals.");

            // send change since last notification.
            _admin.addBytes(getDocKey(), deltaSent, deltaRecv);
        }

        if (_storage && !_lockStateUpdateRequest && _lockCtx->needsRefresh(now))
        {
            refreshLock();
        }
#endif

        LOG_TRC("Poll: current activity: " << DocumentState::name(_docState.activity()));
        switch (_docState.activity())
        {
            case DocumentState::Activity::None:
            {
#if !MOBILEAPP
                if (_checkFileInfo)
                {
                    // We are done. Safe to reset.
                    LOG_TRC("Resetting checkFileInfo instance");
                    _checkFileInfo.reset();
                }
#endif

                if (_uploadRequest && _uploadRequest->isComplete())
                {
                    // We are done. Safe to reset.
                    LOG_TRC("Resetting uploadRequest instance");
                    _uploadRequest.reset();
                }

                // Check if there are queued activities.
                if (!_renameFilename.empty() && !_renameSessionId.empty())
                {
                    startRenameFileCommand();
                    // Nothing more to do until the save is complete.
                    continue;
                }

                // Remove idle documents after the configured time.
                if (!Util::isMobileApp() && isLoaded() && getIdleTime() >= IdleDocTimeoutSecs)
                {
                    autoSaveAndStop("idle");
                }
                else if (_sessions.empty() && (isLoaded() || _docState.isMarkedToDestroy()))
                {
                    if (!isLoaded())
                    {
                        // Nothing to do; no sessions, not loaded, marked to destroy.
                        stop("dead");
                    }
                    else if (_saveManager.isSaving() || isAsyncUploading())
                    {
                        LOG_DBG("Don't terminate dead DocumentBroker: async saving in progress for "
                                "docKey ["
                                << getDocKey() << ']');
                        continue;
                    }

                    autoSaveAndStop("dead");
                }
                else if (COOLWSD::IndirectionServerEnabled && SigUtil::getShutdownRequestFlag() &&
                         !_migrateMsgReceived)
                {
                    if (!waitingForMigrationMsg)
                    {
                        migrationMsgStartTime = std::chrono::steady_clock::now();
                        waitingForMigrationMsg = true;
                        break;
                    }

                    const auto timeNow = std::chrono::steady_clock::now();
                    const auto elapsedMicroS =
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            timeNow - migrationMsgStartTime);
                    if (elapsedMicroS > migrationMsgTimeout)
                    {
                        LOG_WRN("Timeout waiting for migration message for docKey[" << _docKey
                                                                                    << ']');
                        _migrateMsgReceived = true;
                        break;
                    }
                    LOG_DBG("Waiting for migration message to arrive before closing the document "
                            "for docKey["
                            << _docKey << ']');
                }
                else if (_docState.isUnloadRequested() || SigUtil::getShutdownRequestFlag() ||
                         _docState.isCloseRequested())
                {
                    if (limStoreFailures > 0 && (_saveManager.saveFailureCount() >=
                                                     static_cast<std::size_t>(limStoreFailures) ||
                                                 _storageManager.uploadFailureCount() >=
                                                     static_cast<std::size_t>(limStoreFailures)))
                    {
                        LOG_ERR(
                            "Failed to store the document and reached maximum retry count of "
                            << limStoreFailures
                            << " Save failures: " << _saveManager.saveFailureCount()
                            << ", Upload failures: " << _storageManager.uploadFailureCount()
#if !MOBILEAPP
                            << ". Giving up"
                            << (_storage && _quarantine && _quarantine->isEnabled()
                                    ? ". The document should be recoverable from the quarantine. "
                                    : ", but Quarantine is disabled. ")
#endif // !MOBILEAPP
                        );
                        stop("storefailed");
                        continue;
                    }

                    const std::string reason =
                        SigUtil::getShutdownRequestFlag()
                            ? "recycling"
                            : (!_closeReason.empty() ? _closeReason : "unloading");
                    autoSaveAndStop(reason);
                }
                else if (!_stop && _saveManager.needAutoSaveCheck())
                {
                    LOG_TRC("Triggering an autosave by timer");
                    autoSave(/*force=*/false, /*dontSaveIfUnmodified=*/true);
                }
                else if (!isAsyncUploading() && !_storageManager.lastUploadSuccessful() &&
                         needToUploadToStorage() != NeedToUpload::No)
                {
                    // Retry uploading, if the last one failed and we can try again.
                    const auto session = getWriteableSession();
                    if (session && !session->getAuthorization().isExpired())
                    {
                        checkAndUploadToStorage(session, /*justSaved=*/false);
                    }
                }
            }
            break;

            case DocumentState::Activity::Save:
            case DocumentState::Activity::SaveAs:
            {
                if (_docState.isKitDisconnected())
                {
                    // We will never save. No need to wait for timeout.
                    LOG_DBG("Doc disconnected while saving. Ending save activity.");
                    _saveManager.setLastSaveResult(/*success=*/false, /*newVersion=*/false);
                    endActivity();
                }
                else
                if (_saveManager.hasSavingTimedOut())
                {
                    LOG_DBG("Saving timedout. Ending save activity.");
                    _saveManager.setLastSaveResult(/*success=*/false, /*newVersion=*/false);
                    endActivity();
                }
            }
            break;

            case DocumentState::Activity::SyncFileTimestamp:
            {
                // Last upload failed, redo CheckFileInfo to reset the modified time.
                assert(!isAsyncUploading() && "Unexpected async-upload in progress");

#if !MOBILEAPP
                if (!_checkFileInfo)
                {
                    const auto session = getFirstAuthorizedSession();
                    if (!session)
                    {
                        // No session to synchronize the timestamp with.
                        // Last resort; reset the timestamp and let it be.
                        // We can't upload without a valid token anyway.
                        LOG_WRN("No valid session to synchronize the timestamp with. Setting "
                                "timestamp as unsafe");
                        assert(_storage && "existed at uploadLocalFileToStorageAsync call");
                        _storage->setLastModifiedTimeUnSafe();
                        endActivity(); // End the SyncFileTimestamp activity.
                    }
                    else
                    {
                        checkFileInfo(session, HTTP_REDIRECTION_LIMIT);
                    }
                }
#endif
            }
            break;

            // We have some activity ongoing.
            default:
            {
                constexpr std::chrono::seconds postponeAutosaveDuration(30);
                LOG_TRC("Postponing autosave check by " << postponeAutosaveDuration);
                _saveManager.postponeAutosave(postponeAutosaveDuration);
            }
            break;
        }

#if !MOBILEAPP
        if ((now - lastClipboardHashUpdateTime) >= 2min)
        {
            for (const auto& it : _sessions)
            {
                if (it.second->staleWaitDisconnect(now))
                {
                    LOG_WRN("Unusual, Kit session " << it.second->getId()
                                                    << " failed its disconnect handshake, killing");
                    finalRemoveSession(it.second);
                    break; // it invalid.
                }
            }
        }

        if ((now - lastClipboardHashUpdateTime) >= 5min)
        {
            LOG_TRC("Rotating clipboard keys");
            for (const auto& it : _sessions)
                it.second->rotateClipboardKey(true);

            lastClipboardHashUpdateTime = now;
        }
#endif
    }

    LOG_INF("Finished polling doc ["
            << _docKey << "]. stop: " << _stop << ", continuePolling: " << _poll->continuePolling()
            << ", CloseReason: [" << _closeReason << ']'
            << ", ShutdownRequestFlag: " << SigUtil::getShutdownRequestFlag()
            << ", TerminationFlag: " << SigUtil::getTerminationFlag());

    if (_childProcess && _sessions.empty())
    {
        LOG_INF("Requesting termination of child [" << getPid() << "] for doc [" << _docKey
                                                    << "] as there are no sessions");
        _childProcess->requestTermination();
    }

    // Check for data-loss.
    std::string reason;
#if !MOBILEAPP
    bool dataLoss = false;
#endif
    if (haveModifyActivityAfterSaveRequest() || !_saveManager.lastSaveSuccessful() ||
        !_storageManager.lastUploadSuccessful() || isStorageOutdated())
    {
        // If we are exiting because the owner discarded conflict changes, don't detect data loss.
        if (!(_docState.isCloseRequested() && _documentChangedInStorage))
        {
#if !MOBILEAPP
            dataLoss = true;
#endif
            if (haveModifyActivityAfterSaveRequest())
                reason = "have unsaved modifications";
            else
                reason = !_saveManager.lastSaveSuccessful() ? "flagged as modified"
                                                            : "not uploaded to storage";

            // The test may override (if it was expected).
            if (_unitWsd &&
                !UNITWSD_CALL_INSTANCE(_unitWsd, onDataLoss("Data-loss detected while exiting [" +
                                                            _docKey + "]: " + reason)))
                reason.clear();
        }
    }

    if (!reason.empty() || (UNITWSD_CALL_INSTANCE(_unitWsd, isFinished()) &&
                            UNITWSD_CALL_INSTANCE(_unitWsd, failed())))
    {
        std::ostringstream state(Util::makeDumpStateStream());
        state << "DocBroker [" << _docKey << "] stopped "
              << (reason.empty() ? "because of test failure" : ("although " + reason));
        if (!UnitWSD::isUnitTesting())
        {
            // When running unit-tests, we issue USR1.
            state << ": ";
            dumpState(state);
        }

        LOG_WRN(state.str());
    }

    if (_lockCtx && _lockCtx->supportsLocks() && _lockCtx->isLocked())
    {
        LOG_DBG("Document [" << _docKey << "] is locked and needs unlocking before unloading");
        const std::shared_ptr<ClientSession> session = getWriteableSession();
        if (!session)
        {
            LOG_ERR("No write-able session to unlock with");
            _lockCtx->bumpTimer();
        }
        else if (session->getAuthorization().isExpired())
        {
            LOG_ERR("No write-able session with valid authorization to unlock with");
            _lockCtx->bumpTimer();
        }
        else
        {
            const std::string unlockSessionId = session->getId();
            LOG_INF("Unlocking " << _lockCtx->lockToken() << " with session [" << unlockSessionId
                                 << ']');
            std::string error;
            if (!updateStorageLockState(*session, StorageBase::LockState::UNLOCK, error))
            {
                LOG_ERR("Failed to unlock docKey [" << _docKey << "] with session ["
                                                    << unlockSessionId << "]: " << error);
            }
        }
    }

    failLoadingSessions(/*remove=*/false);

    // Flush socket data first, if any.
    if (_poll->getSocketCount())
    {
        constexpr std::chrono::microseconds flushTimeoutMicroS(std::chrono::seconds(2));
        LOG_INF("Flushing " << _poll->getSocketCount() << " sockets for doc [" << _docKey
                            << "] for " << flushTimeoutMicroS);

        const auto flushStartTime = std::chrono::steady_clock::now();
        while (_poll->getSocketCount())
        {
            const auto now = std::chrono::steady_clock::now();
            const auto elapsedMicroS =
                std::chrono::duration_cast<std::chrono::microseconds>(now - flushStartTime);
            if (elapsedMicroS > flushTimeoutMicroS)
                break;

            const std::chrono::microseconds timeoutMicroS =
                std::min(flushTimeoutMicroS - elapsedMicroS, flushTimeoutMicroS/10);
            if (_poll->poll(timeoutMicroS) == 0 && UnitWSD::isUnitTesting())
            {
                // Polling timed out, no more data to flush.
                break;
            }

            processBatchUpdates();
        }

        LOG_INF("Finished flushing socket for doc [" << _docKey << ']');
    }

    // Terminate properly while we can.
    LOG_DBG("Terminating child with reason: [" << _closeReason << ']');
    terminateChild(_closeReason);

    // Stop to mark it done and cleanup.
    _poll->stop();

#if !MOBILEAPP
    if (dataLoss || _docState.kitDisconnected() == DocumentState::KitDisconnected::Unexpected)
    {
        // Quarantine the last copy, if different.
        LOG_WRN((dataLoss ? "Data loss " : "Crash ")
                << "detected on [" << getDocKey() << ']'
                << (_storage && _quarantine && _quarantine->isEnabled()
                        ? ". Will quarantine the last version. "
                        : ", but Quarantine is disabled. ")
                << "Storage available: " << bool(_storage));
        if (_storage && _quarantine && _quarantine->isEnabled())
        {
            const std::string uploading = _storage->getRootFilePathUploading();
            if (FileUtil::Stat(uploading).exists())
            {
                LOG_WRN("Quarantining the .uploading file: " << uploading);
                _quarantine->quarantineFile(uploading);
            }
            else
            {
                const std::string upload = _storage->getRootFilePathToUpload();
                if (FileUtil::Stat(upload).exists())
                {
                    LOG_WRN("Quarantining the .upload file: " << upload);
                    _quarantine->quarantineFile(upload);
                }
                else
                {
                    // Fallback to quarantining the original document.
                    LOG_WRN("Quarantining the original document file: " << _filename);
                    _quarantine->quarantineFile(_storage->getRootFilePath());
                }
            }
        }
    }

    // Async cleanup.
    COOLWSD::doHousekeeping();
#endif

    clearCaches();

    LOG_INF("Finished docBroker polling thread for docKey [" << _docKey << ']');
}

bool DocumentBroker::isAlive() const
{
    if (!_stop || _poll->isAlive())
        return true; // Polling thread not started or still running.

    // Shouldn't have live child process outside of the polling thread.
    return _childProcess && _childProcess->isAlive();
}

void DocumentBroker::timeoutNotLoaded(std::chrono::steady_clock::time_point now)
{
    if (!_stop && !_poll->isAlive() && !isLoaded() && now - _createTime > getLimitLoadSecs())
        stop("neverloaded");
}

DocumentBroker::~DocumentBroker()
{
    ASSERT_CORRECT_THREAD();

    LOG_INF("~DocumentBroker [" << _docKey << "] destroyed with " << _sessions.size()
                                << " sessions left");

    // Do this early - to avoid operating on _childProcess from two threads.
    _poll->joinThread();

    for (const auto& [id, session] : _sessions)
    {
        if (session->isLive())
        {
            LOG_WRN("Destroying DocumentBroker ["
                    << _docKey << "] while having " << _sessions.size()
                    << " unremoved sessions, at least one is still live");
            break;
        }

        if (session.use_count() > 1)
        {
            LOG_WRN("Destroying DocumentBroker [" << _docKey << "] while having session [" << id
                                                  << "] with " << session.use_count()
                                                  << " references");
        }
    }

    _sessions.clear();

    // Need to first make sure the child exited, socket closed,
    // and thread finished before we are destroyed.
    _childProcess.reset();

#if !MOBILEAPP
    // Remove from the admin last, to avoid racing the next test.
    _admin.rmDoc(_docKey);
#endif

    UNITWSD_CALL_INSTANCE(_unitWsd, DocBrokerDestroy(_docKey));
}

void DocumentBroker::joinThread()
{
    _poll->joinThread();
}

void DocumentBroker::stop(const std::string_view reason)
{
    if (_closeReason.empty() || _closeReason == reason)
    {
        LOG_DBG("Stopping DocumentBroker for docKey [" << _docKey << "] with reason: " << reason);
        _closeReason = reason; // used later in the polling loop
    }
    else
    {
        LOG_DBG("Stopping DocumentBroker for docKey ["
                << _docKey << "] with existing close reason: " << _closeReason
                << " (ignoring requested reason: " << reason << ')');
    }

    _stop = true;
    _poll->wakeup();
}

bool DocumentBroker::download(
    const std::shared_ptr<ClientSession>& session, const std::string& jailId,
    const Poco::URI& uriPublic,
    const AdditionalFilePocoUris& additionalFileUrisPublic,
    [[maybe_unused]] std::unique_ptr<WopiStorage::WOPIFileInfo> wopiFileInfo)
{
    ASSERT_CORRECT_THREAD();

    const std::string sessionId = session ? session->getId() : "000";
    LOG_INF("Loading [" << _docKey << "] for session [" << sessionId << "] in jail [" << jailId
                        << "] from URI [" << uriPublic.toString() << ']');

    {
        bool result = false;
        if (UNITWSD_CALL_INSTANCE(_unitWsd, filterLoad(sessionId, jailId, result)))
            return result;
    }

    if (_docState.isMarkedToDestroy())
    {
        // Tearing down.
        LOG_WRN("Will not load document marked to destroy. DocKey: [" << _docKey << ']');
        return false;
    }

    _jailId = jailId;

    // The URL is the publicly visible one, not visible in the chroot jail.
    // We need to map it to a jailed path and copy the file there.

    // /tmp/user/docs/<dirName>, root under getJailRoot()
    const Poco::Path jailPath(JAILED_DOCUMENT_ROOT, Util::rng::getFilename(16));
    const std::string jailRoot = getJailRoot();

    LOG_INF("JailPath for docKey [" << _docKey << "]: [" << jailPath.toString() << "], jailRoot: ["
                                    << jailRoot << ']');

    bool firstInstance = false;
    if (_storage == nullptr)
    {
        _docState.setStatus(DocumentState::Status::Downloading);

        UNITWSD_CALL_INSTANCE(_unitWsd, onPerfDocumentLoading());

        // Pass the public URI to storage as it needs to load using the token
        // and other storage-specific data provided in the URI.
        LOG_DBG("Creating new storage instance for URI ["
                << COOLWSD::anonymizeUrl(uriPublic.toString()) << ']');

        try
        {
            _storage = StorageBase::create(uriPublic, jailRoot, jailPath.toString(),
                                           /*takeOwnership=*/isConvertTo(),
                                           additionalFileUrisPublic);
        }
        catch (...)
        {
            if (session)
                session->sendTextFrame("loadstorage: failed");
            throw;
        }

        if (_storage == nullptr)
        {
            // We should get an exception, not null.
            LOG_ERR("Failed to create Storage instance for [" << _docKey << "] in "
                                                              << jailPath.toString());
            return false;
        }
        firstInstance = true;
    }

    LOG_ASSERT(_storage);

    // Call the storage specific fileinfo functions
    std::string templateSource;

    std::string userSettingsUri;

#if !MOBILEAPP
    std::chrono::milliseconds checkFileInfoCallDurationMs = std::chrono::milliseconds::zero();
    WopiStorage* wopiStorage = dynamic_cast<WopiStorage*>(_storage.get());
    if (wopiStorage != nullptr)
    {
        LOG_DBG("CheckFileInfo for docKey [" << _docKey << "] "
                                             << (wopiFileInfo ? "already exists" : "is missing"));
        if (!wopiFileInfo)
        {
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

            if (!session)
                LOG_ERR("No session for CheckFileInfo");
            else
            {
                auto poller = std::make_shared<TerminatingPoll>("CFISynReqPoll");
                poller->runOnClientThread();
                auto checkFileInfo = std::make_shared<CheckFileInfo>(poller, session->getPublicUri(), [](CheckFileInfo&) {});
                checkFileInfo->checkFileInfoSync(HTTP_REDIRECTION_LIMIT);
                wopiFileInfo = checkFileInfo->wopiFileInfo(session->getPublicUri());
            }
            if (!wopiFileInfo)
            {
                throw std::runtime_error(
                    "CheckFileInfo failed or timed out while adding session #" + sessionId);
            }

            checkFileInfoCallDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);
        }

        wopiStorage->handleWOPIFileInfo(*wopiFileInfo, *_lockCtx);
        _isViewFileExtension = COOLWSD::IsViewFileExtension(wopiStorage->getFileExtension());

        if (session)
        {
            userSettingsUri = wopiFileInfo->getUserSettingsUri();
            templateSource =
                updateSessionWithWopiInfo(session, wopiStorage, std::move(wopiFileInfo));
        }
    }
    else
#endif
    {
        // Could be a conversion request.
        LocalStorage* localStorage = dynamic_cast<LocalStorage*>(_storage.get());
        if (localStorage != nullptr)
        {
            std::unique_ptr<LocalStorage::LocalFileInfo> localfileinfo =
                localStorage->getLocalFileInfo();

            _isViewFileExtension = COOLWSD::IsViewFileExtension(localStorage->getFileExtension());
            if (session)
            {
                if (_isViewFileExtension)
                {
                    LOG_DBG("Setting session [" << sessionId << "] as readonly");
                    session->setReadOnly(true);
                    LOG_DBG("Allow session [" << sessionId
                                              << "] to change comments on document with extension ["
                                              << localStorage->getFileExtension() << ']');
                    session->setAllowChangeComments(true);

                    // Related to fix for issue #5887: only send a read-only
                    // message for "view file extension" document types
                    session->sendFileMode(session->isReadOnly(), session->isAllowChangeComments(), session->isAllowManageRedlines());
                }
                else if constexpr (Util::isMobileApp())
                {
                    // Fix issue #5887 by assuming that documents are writable on iOS and Android
                    // The iOS and Android app saves directly to local disk so, other than for
                    // "view file extension" document types or other cases that
                    // I am missing, we can assume the document is writable until
                    // a write failure occurs.
                    LOG_DBG("Setting session [" << sessionId
                                                << "] to writable and allowing comments");
                    session->setWritable(true);
                    session->setReadOnly(false);
                    session->setAllowChangeComments(true);
                }

                session->setUserId(localfileinfo->getUserId());
                session->setUserName(localfileinfo->getUsername());
            }
        }
        else
        {
            LOG_FTL("Unknown or unsupported storage");
            Util::forcedExit(EX_SOFTWARE);
        }
    }

    if (session)
    {
        LOG_DBG("Setting username ["
                << COOLWSD::anonymizeUsername(session->getUserName()) << "] and userId ["
                << COOLWSD::anonymizeUsername(session->getUserId()) << "] for session ["
                << sessionId << "] with canonical id " << session->getCanonicalViewId());
    }

    // Basic file information was stored by the above getWOPIFileInfo() or getLocalFileInfo() calls
    const StorageBase::FileInfo fileInfo = _storage->getFileInfo();
    if (!fileInfo.isValid())
    {
        LOG_ERR("Invalid fileinfo for URI [" << uriPublic.toString() << ']');
        return false;
    }

    if (firstInstance)
    {
        _storageManager.setLastModifiedServerTimeString(fileInfo.getLastModifiedServerTimeString());
        LOG_DBG("Document timestamp: " << _storageManager.getLastModifiedServerTimeString());
    }
    else
    {
        // Check if document has been modified by some external action,
        // but only if *we* aren't uploading. Otherwise, it might be us.
        LOG_TRC("Document modified time: " << fileInfo.getLastModifiedServerTimeString());
        if (!_storageManager.getLastModifiedServerTimeString().empty() &&
            !fileInfo.getLastModifiedServerTimeString().empty() &&
            _storageManager.getLastModifiedServerTimeString() !=
                fileInfo.getLastModifiedServerTimeString())
        {
            if (_uploadRequest)
            {
                LOG_DBG("Document ["
                        << _docKey << "] timestamp checked for a match during an up-load (started "
                        << Util::getTimeForLog(std::chrono::steady_clock::now(),
                                               _uploadRequest->startTime())
                        << ", " << (_uploadRequest->isComplete() ? "" : "in")
                        << "complete), results may race, "
                           "so ignoring inconsistent timestamp. Expected: "
                        << _storageManager.getLastModifiedServerTimeString()
                        << ", Actual: " << fileInfo.getLastModifiedServerTimeString());
            }
            else
            {
                LOG_WRN("Document [" << _docKey << "] has been modified behind our back. "
                                     << "Informing all clients. Expected: "
                                     << _storageManager.getLastModifiedServerTimeString()
                                     << ", Actual: " << fileInfo.getLastModifiedServerTimeString());

                handleDocumentConflict();
            }
        }
    }

    if (session)
        broadcastLastModificationTime(session);

    // Let's download the document now, if not downloaded.
    std::chrono::milliseconds getFileCallDurationMs = std::chrono::milliseconds::zero();
    if (!_storage->isDownloaded())
    {
        const Authorization auth =
            session ? session->getAuthorization() : Authorization::create(uriPublic);
        if (!doDownloadDocument(auth, templateSource, fileInfo.getFilename(),
                                getFileCallDurationMs))
        {
            LOG_DBG("Failed to download or process downloaded document");
            return false;
        }
    }

#if !MOBILEAPP
    COOLWSD::dumpNewSessionTrace(getJailId(), sessionId, _uriOrig, _storage->getRootFilePath());

    // Since document has been loaded, send the stats if its WOPI
    if (wopiStorage != nullptr)
    {
        // Add the time taken to load the file from storage and to check file info.
        _wopiDownloadDuration += getFileCallDurationMs + checkFileInfoCallDurationMs;
        if (session)
        {
            const auto downloadSecs = _wopiDownloadDuration.count() / 1000.;
            const std::string msg =
                "stats: wopiloadduration " + std::to_string(downloadSecs); // In seconds.
            LOG_TRC("Sending to Client [" << msg << ']');
            session->sendTextFrame(msg);
        }
    }

    // if async browsersetting json request is not downloaded even after document download is complete
    // we do sync request to make sure the browser setting json sent before document starts to load
    if (session && !userSettingsUri.empty())
    {
        LOG_DBG("browsersetting for docKey ["
                << _docKey << "] for session #" << session->getId()
                << (session->getSentBrowserSetting() ? " already exists" : " is missing"));
        if (!session->getSentBrowserSetting())
        {
            getBrowserSettingSync(session, userSettingsUri);
            if (!session->getSentBrowserSetting())
            {
                const std::string uriAnonym = COOLWSD::anonymizeUrl(userSettingsUri);
                LOG_ERR("Request to uri["
                        << uriAnonym
                        << "] failed or timedout while adding session #" + session->getId());
            }
        }
    }

#endif // !MOBILEAPP

    return true;
}

void DocumentBroker::lockIfEditing(const std::shared_ptr<ClientSession>& session)
{
    if (_lockCtx == nullptr || !_lockCtx->supportsLocks() || _lockCtx->isLocked())
    {
        return; // Nothing to do.
    }

    // If we have a session, isReadOnly() will be correctly set
    // based on the URI (which may include a readonly permission),
    // as well as the WOPI Info that we got above.
    if (!session->isReadOnly())
    {
        LOG_DBG("Locking docKey [" << _docKey
                                   << "] asynchronously, which is editable, with session ["
                                   << session->getId() << ']');
        //TODO: Convert to Async. Unfortunately, that complicates
        // things quite a bit and makes tests tricky.
        std::string error;
        if (!updateStorageLockState(*session, StorageBase::LockState::LOCK, error))
        {
            LOG_ERR("Failed to lock docKey ["
                    << _docKey << "] with session [" << session->getId()
                    << "] before downloading. Session will be read-only: " << error);
            session->setWritable(false);
        }
    }
    else
    {
        LOG_DBG("Session [" << session->getId() << "] is read-only and cannot lock the document");
    }
}

bool DocumentBroker::doDownloadDocument(const Authorization& auth,
                                        const std::string& templateSource,
                                        const std::string& filename,
                                        std::chrono::milliseconds& getFileCallDurationMs)
{
    assert(_storage && !_storage->isDownloaded());

    LOG_DBG("Download file for docKey [" << _docKey << ']');
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    AdditionalFilePaths additionalFileLocalPaths;
    std::string localPath = _storage->downloadStorageFileToLocal(auth, *_lockCtx, templateSource, additionalFileLocalPaths);
    if (localPath.empty())
    {
        throw std::runtime_error("Failed to retrieve document from storage");
    }

    getFileCallDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    _docState.setStatus(DocumentState::Status::Loading); // Done downloading.

#if !MOBILEAPP
    if (!processPlugins(localPath))
    {
        // FIXME: Why don't we resume anyway?
        LOG_WRN("Failed to process plugins on file [" << localPath << ']');
        return false;
    }
#endif //!MOBILEAPP

    std::string localFilePath =
        Poco::Path(FileUtil::buildLocalPathToJail(COOLWSD::EnableMountNamespaces, getJailRoot(),
                                                  localPath))
            .toString();
    std::ifstream istr(localFilePath, std::ios::binary);
    Poco::SHA1Engine sha1;
    Poco::DigestOutputStream dos(sha1);
    Poco::StreamCopier::copyStream(istr, dos);
    dos.close();
    LOG_INF("SHA1 for DocKey [" << _docKey << "] of [" << COOLWSD::anonymizeUrl(localPath)
                                << "]: " << Poco::DigestEngine::digestToHex(sha1.digest()));

    std::string localPathEncoded;
    Poco::URI::encode(localPath, "#?", localPathEncoded);
    _uriJailed = Poco::URI(Poco::Path(localPath)).toString();
    _uriJailedAnonym =
        Poco::URI(Poco::Path(COOLWSD::anonymizeUrl(localPathEncoded))).toString();
    for (const auto& it : additionalFileLocalPaths)
    {
        std::string additionalFileLocalPathEncoded;
        Poco::URI::encode(it.second, "#?", additionalFileLocalPathEncoded);
        _additionalFileUrisJailed[it.first] = Poco::URI(Poco::Path(additionalFileLocalPathEncoded)).toString();
    }

    _filename = filename;
    if constexpr (!Util::isMobileApp())
    {
        _quarantine = std::make_unique<Quarantine>(*this, _filename);
    }

    if (!templateSource.empty())
    {
        // Invalid timestamp for templates, to force uploading once we save-after-loading.
        _saveManager.setLastModifiedLocalTime(std::chrono::system_clock::time_point());
        _storageManager.setLastUploadedFileModifiedLocalTime(
            std::chrono::system_clock::time_point());
    }
    else
    {
        // Use the local temp file's timestamp.
        const auto timepoint = FileUtil::Stat(localFilePath).modifiedTimepoint();
        _saveManager.setLastModifiedLocalTime(timepoint);
        _storageManager.setLastUploadedFileModifiedLocalTime(
            timepoint); // Used to detect modifications.
    }

    const bool dontUseCache = Util::isMobileApp();

    _tileCache = std::make_unique<TileCache>(_storage->getUri().toString(),
                                             _saveManager.getLastModifiedLocalTime(), dontUseCache);
    _tileCache->setThreadOwner(ProcUtil::getThreadId());

    return true;
}

#if !MOBILEAPP
std::string
DocumentBroker::updateSessionWithWopiInfo(const std::shared_ptr<ClientSession>& session,
                                          WopiStorage* wopiStorage,
                                          std::unique_ptr<WopiStorage::WOPIFileInfo> wopiFileInfo)
{
    const std::string sessionId = session->getId();

    const std::string userId = wopiFileInfo->getUserId();
    const std::string username = wopiFileInfo->getUsername();
    const std::string userExtraInfo = wopiFileInfo->getUserExtraInfo();
    const std::string userPrivateInfo = wopiFileInfo->getUserPrivateInfo();
    const std::string serverPrivateInfo = wopiFileInfo->getServerPrivateInfo();
    const std::string watermarkText =
        (ConfigUtil::isSupportKeyEnabled() && !COOLWSD::OverrideWatermark.empty())
            ? COOLWSD::OverrideWatermark
            : wopiFileInfo->getWatermarkText();
    std::string templateSource = wopiFileInfo->getTemplateSource();

    std::optional<bool> isAdminUser = wopiFileInfo->getIsAdminUser();
    if (!wopiFileInfo->getIsAdminUserError().empty())
        _serverAudit.set("is_admin", wopiFileInfo->getIsAdminUserError());

    if (!COOLWSD::getHardwareResourceWarning().empty())
        _serverAudit.set("hardwarewarning", COOLWSD::getHardwareResourceWarning());

    if (_childProcess)
        _serverAudit.mergeSettings(_childProcess);

    // Explicitly set the write-permission to match the UserCanWrite flag.
    // Technically, we only need to disable it when UserCanWrite=false,
    // but this is more readily comprehensible and easier to reason about.
    session->setWritePermission(wopiFileInfo->getUserCanWrite());

    if (wopiFileInfo->getUserCanOnlyComment())
    {
        LOG_DBG("Setting session ["
                << sessionId << "] to readonly for UserCanOnlyComment=true and allowing comments");
        session->setWritePermission(true);
        session->setWritable(true);
        session->setReadOnly(true);
        session->setAllowChangeComments(true);
    }
    else if (wopiFileInfo->getUserCanOnlyManageRedlines())
    {
        LOG_DBG("Setting session ["
                << sessionId << "] to readonly for UserCanOnlyManageRedlines=true and allowing redline management");
        session->setWritePermission(true);
        session->setWritable(true);
        session->setReadOnly(true);
        session->setAllowManageRedlines(true);
    }
    else if (!wopiFileInfo->getUserCanWrite())
    {
        // We can't write in the storage, so we can't even add comments.
        LOG_DBG("Setting session [" << sessionId << "] to readonly for UserCanWrite=false");
        session->setWritePermission(false); // Disable editing and commenting.
    }
    else if (session->isReadOnly()) // Readonly. Checks for URL "permission=readonly".
    {
        LOG_DBG("Setting session [" << sessionId << "] to readonly for permission=readonly");
        session->setWritable(false);
        // TODO: Somewhere around here, we need to put "setAllowChangeComments" if we allow editing comments in readonly mode.
    }
    else if (CommandControl::LockManager::isLockedReadOnlyUser()) // Readonly.
    {
        LOG_DBG("Setting session [" << sessionId << "] to readonly for LockedReadOnlyUser");
        session->setWritable(false);
    }
    else if (_isViewFileExtension) // PDF and the like: only commenting, no editing.
    {
        LOG_DBG("Setting session [" << sessionId << "] to readonly for ViewFileExtension ["
                                    << wopiStorage->getFileExtension()
                                    << "] and allowing comments");
        session->setWritable(true);
        session->setReadOnly(true);
        session->setAllowChangeComments(true);
    }
    else // Fully writable document, with comments.
    {
        LOG_DBG("Setting session [" << sessionId << "] to writable and allowing comments");
        session->setWritable(true);
        session->setReadOnly(false);
        session->setAllowChangeComments(true);
    }

    // Mark the session as 'Document owner' if WOPI hosts supports it
    if (userId == _storage->getFileInfo().getOwnerId())
    {
        LOG_DBG("Session [" << sessionId << "] is the document owner");
        session->setDocumentOwner(true);
    }

    // We will send the client about information of the usage type of the file.
    // Some file types may be treated differently than others.
    session->sendFileMode(session->isReadOnly(), session->isAllowChangeComments(), session->isAllowManageRedlines());

    // Construct a JSON containing relevant WOPI host properties
    Object::Ptr wopiInfo = new Object();
    if (!wopiFileInfo->getPostMessageOrigin().empty())
    {
        wopiInfo->set("PostMessageOrigin", wopiFileInfo->getPostMessageOrigin());
    }

    // If print, export are disabled, order client to hide these options in the UI
    if (wopiFileInfo->getDisablePrint())
        wopiFileInfo->setHidePrintOption(true);
    if (wopiFileInfo->getDisableExport())
        wopiFileInfo->setHideExportOption(true);

    wopiInfo->set("BaseFileName", wopiStorage->getFileInfo().getFilename());
    if (wopiFileInfo->getBreadcrumbDocName().size())
        wopiInfo->set("BreadcrumbDocName", wopiFileInfo->getBreadcrumbDocName());

    if (!wopiFileInfo->getTemplateSaveAs().empty())
        wopiInfo->set("TemplateSaveAs", wopiFileInfo->getTemplateSaveAs());

    if (!templateSource.empty())
        wopiInfo->set("TemplateSource", templateSource);

    wopiInfo->set("HidePrintOption", wopiFileInfo->getHidePrintOption());
    wopiInfo->set("HideSaveOption", wopiFileInfo->getHideSaveOption());
    wopiInfo->set("HideExportOption", wopiFileInfo->getHideExportOption());
    wopiInfo->set("HideRepairOption", wopiFileInfo->getHideRepairOption());
    wopiInfo->set("DisablePrint", wopiFileInfo->getDisablePrint());
    wopiInfo->set("DisableExport", wopiFileInfo->getDisableExport());
    wopiInfo->set("DisableCopy", wopiFileInfo->getDisableCopy());
    wopiInfo->set("DisableInactiveMessages", wopiFileInfo->getDisableInactiveMessages());
    wopiInfo->set("DownloadAsPostMessage", wopiFileInfo->getDownloadAsPostMessage());
    wopiInfo->set("UserCanNotWriteRelative", wopiFileInfo->getUserCanNotWriteRelative());
    wopiInfo->set("EnableInsertRemoteImage", wopiFileInfo->getEnableInsertRemoteImage());
    wopiInfo->set("EnableInsertRemoteFile", wopiFileInfo->getEnableInsertRemoteFile());
    wopiInfo->set("DisableInsertLocalImage", wopiFileInfo->getDisableInsertLocalImage());
    wopiInfo->set("EnableRemoteLinkPicker", wopiFileInfo->getEnableRemoteLinkPicker());
    wopiInfo->set("EnableRemoteAIContent", wopiFileInfo->getEnableRemoteAIContent());
    wopiInfo->set("DisableAISettings",
                  !ConfigUtil::getConfigValue<bool>("ai.enabled", false) ||
                      wopiFileInfo->getDisableAISettings());

    // Resolve default AI credentials from UserPrivateInfo, falling back to
    // coolwsd.xml. This makes AI usable on integrations that don't implement
    // the UserSettings preset storage (where viewsetting.json would never
    // exist). User View Settings, if present, override these later via
    // extractViewSettings / handleUpdateViewSettings.
    Object::Ptr userPrivateInfoObj;
    if (!userPrivateInfo.empty())
        JsonUtil::parseJSON(userPrivateInfo, userPrivateInfoObj);
    bool unusedMutated = false;
    std::string resolvedAIModel;
    const bool aiConfigured = session->resolveAndApplyAICredentials(
        /*viewSettings=*/nullptr, userPrivateInfoObj,
        wopiFileInfo->getDisableAISettings(), unusedMutated, resolvedAIModel);
    wopiInfo->set("AIConfigured", aiConfigured);
    if (aiConfigured)
        wopiInfo->set("AIModelName", resolvedAIModel);
    wopiInfo->set("EnableShare", wopiFileInfo->getEnableShare());
    wopiInfo->set("HideUserList", wopiFileInfo->getHideUserList());
    wopiInfo->set("SupportsRename", wopiFileInfo->getSupportsRename());
    wopiInfo->set("UserCanRename", wopiFileInfo->getUserCanRename());
    wopiInfo->set("FileUrl", wopiFileInfo->getFileUrl());
    wopiInfo->set("UserCanWrite", wopiFileInfo->getUserCanWrite() && !session->isReadOnly());
    if (wopiFileInfo->getHideChangeTrackingControls() != WopiStorage::WOPIFileInfo::TriState::Unset)
        wopiInfo->set("HideChangeTrackingControls", wopiFileInfo->getHideChangeTrackingControls() ==
                                                        WopiStorage::WOPIFileInfo::TriState::True);
    wopiInfo->set("IsOwner", session->isDocumentOwner());

    if (!wopiFileInfo->getPresentationLeader().empty())
        wopiInfo->set("PresentationLeader", wopiFileInfo->getPresentationLeader());

    bool disablePresentation = wopiFileInfo->getDisableExport() || wopiFileInfo->getHideExportOption();
    // the new slideshow supports watermarking, anyway it's still an experimental features
    disablePresentation = disablePresentation || (!ConfigUtil::getBool("canvas_slideshow_enabled", true) && !watermarkText.empty());
    wopiInfo->set("DisablePresentation", disablePresentation);

    const std::string commentAvatarUrl = ConfigUtil::getString("comment_avatar", "");
    if (!commentAvatarUrl.empty())
        wopiInfo->set("CommentAvatarUrl", commentAvatarUrl);

    std::ostringstream ossWopiInfo;
    wopiInfo->stringify(ossWopiInfo);
    const std::string wopiInfoString = ossWopiInfo.str();
    LOG_TRC("Sending wopi info to client: " << wopiInfoString);

    // Contains PostMessageOrigin property which is necessary to post messages to parent
    // frame. Important to send this message immediately and not enqueue it so that in case
    // document load fails, cool is able to tell its parent frame via PostMessage API.
    session->sendTextFrame("wopi: " + wopiInfoString);

    if (ConfigUtil::getBool("logging.userstats", false))
    {
        // using json because fetching details from json string is easier and will be consistent
        Object::Ptr userStats = new Object();
        userStats->set("PostMessageOrigin", wopiFileInfo->getPostMessageOrigin());
        userStats->set("UserID", COOLWSD::anonymizeUsername(userId));
        userStats->set("BaseFileName", wopiStorage->getFileInfo().getFilename());
        userStats->set("UserCanWrite", wopiFileInfo->getUserCanWrite());

        std::ostringstream ossUserStats;
        userStats->stringify(ossUserStats);
        const std::string userStatsString = ossUserStats.str();

        LOG_ANY("User stats: " << userStatsString);
    }

    if (ConfigUtil::getBool("logging.disable_server_audit", false))
    {
        _serverAudit.disable();
    }

    std::string userSettingsUri = wopiFileInfo->getUserSettingsUri();
    if (_sessions.empty() && !userSettingsUri.empty())
    {
        std::string jailPresetsPath = FileUtil::buildLocalPathToJail(COOLWSD::EnableMountNamespaces,
                                                                     getJailRoot(),
                                                                     JAILED_CONFIG_ROOT);
        std::string configId = "user-" + userId + "-" + Cache::getConfigId(userSettingsUri);
        asyncInstallPresets(session, configId, userSettingsUri, jailPresetsPath);
    }

    // Pass the ownership to the client session.
    session->setWopiFileInfo(std::move(wopiFileInfo));
    session->setUserId(userId);
    session->setUserName(username);
    session->setUserExtraInfo(userExtraInfo);
    session->setIsAdminUser(isAdminUser);
    session->setUserPrivateInfo(userPrivateInfo);
    session->setServerPrivateInfo(serverPrivateInfo);
    session->setWatermarkText(watermarkText);

    return templateSource;
}

void PresetsInstallTask::asyncInstall(const std::string& uri, const std::string& stamp, const std::string& fileName,
                                      const std::shared_ptr<ClientSession>& session)
{
    std::shared_ptr<SocketPoll> poll = _poll.lock();
    if (!poll)
    {
        LOG_WRN("asyncInstall started after poll was destroyed");
        return;
    }

    auto presetInstallFinished = [selfWeak = weak_from_this(), this](const std::string& id, bool presetResult)
    {
        std::shared_ptr<PresetsInstallTask> selfLifecycle = selfWeak.lock();
        if (!selfLifecycle)
            return;

        installPresetFinished(id, presetResult);
    };

    // just something unique for this resource
    std::string id = std::to_string(_idCount++);

    installPresetStarted(id);

    DocumentBroker::asyncInstallPreset(poll, _configId, uri, stamp, fileName, id,
                                       presetInstallFinished, session);
}

void PresetsInstallTask::installPresetStarted(const std::string& id)
{
    _installingPresets.insert(id);
}

void PresetsInstallTask::installPresetFinished(const std::string& id, bool presetResult)
{
    _overallSuccess &= presetResult;
    _installingPresets.erase(id);
    // If there are no remaining presets to fetch, or this one has
    // failed, then we can respond. TODO could we cancel outstanding
    // downloads?
    if (_installingPresets.empty() && !_reportedStatus)
    {
        LOG_INF("Async fetch of presets for " << _configId << " completed. Success: " << _overallSuccess);
        completed();
    }
}

void PresetsInstallTask::completed()
{
    auto selfLifecycle = shared_from_this();
    _reportedStatus = true;
    for (const auto& cb : _installFinishedCBs)
        cb(_overallSuccess);
}

void PresetsInstallTask::addGroup(const Poco::JSON::Object::Ptr& settings, const std::string& groupName,
              std::vector<CacheQuery>& queries)
{
    if (!settings->has(groupName))
        return;

    auto group = settings->get(groupName).extract<Poco::JSON::Array::Ptr>();
    for (std::size_t i = 0, count = group->size(); i < count; ++i)
    {
        auto elem = group->get(i).extract<Poco::JSON::Object::Ptr>();
        if (!elem)
            continue;

        const std::string uri = JsonUtil::getJSONValue<std::string>(elem, "uri");
        const std::string stamp = JsonUtil::getJSONValue<std::string>(elem, "stamp");

        Poco::Path destDir(_presetsPath, groupName);
        Poco::File(destDir).createDirectories();
        std::string filePath;
        if (groupName == "xcu")
            filePath = Poco::Path(destDir.toString(), "config.xcu").toString();
        else if (groupName == "browsersetting")
            filePath = Poco::Path(destDir.toString(), "browsersetting.json").toString();
        else if (groupName == "viewsetting")
            filePath = Poco::Path(destDir.toString(), "viewsetting.json").toString();
        else
        {
            // Check for a file_name='something' and use that if it exists and
            // if it is safe to use, otherwise use a file name derived from the
            // url itself.
            std::string fileName;
            for (const auto& param : Poco::URI(uri).getQueryParameters())
            {
                if (param.first == "file_name")
                    fileName = param.second;
            }

            if (fileName.empty())
                fileName = Uri::getFilenameWithExtFromURL(uri);

            if (fileName.empty() || fileName == "." || fileName == ".." ||
                fileName.find_first_of('/') != std::string::npos)
            {
                LOG_ERR("Invalid settings filename of: " << fileName);
                continue;
            }

            filePath = Poco::Path(destDir.toString(), fileName).toString();
        }

        queries.emplace_back(uri, stamp, filePath);
    }
}

PresetsInstallTask::PresetsInstallTask(const std::shared_ptr<SocketPoll>& poll,
                   const std::string& configId,
                   const std::string& presetsPath,
                   const std::function<void(bool)>& installFinishedCB)
    : _configId(configId)
    , _presetsPath(presetsPath)
    , _poll(poll)
    , _idCount(0)
    , _reportedStatus(false)
    , _overallSuccess(true)
{
    appendCallback(installFinishedCB);
}

void PresetsInstallTask::install(const Poco::JSON::Object::Ptr& settings,
             const std::shared_ptr<ClientSession>& session)
{
    try
    {
        std::vector<CacheQuery> presets;
        if (!settings)
            _overallSuccess = false;
        else
        {
            addGroup(settings, "browsersetting", presets);
            addGroup(settings, "autotext", presets);
            addGroup(settings, "wordbook", presets);
            addGroup(settings, "viewsetting", presets);
            addGroup(settings, "xcu", presets);
            addGroup(settings, "template", presets);
        }

        Cache::supplyConfigFiles(_configId, presets);

        // If there are no presets to fetch then we can respond now, otherwise
        // that happens when the last preset is installed.
        if (!presets.empty())
        {
            LOG_INF("Async fetch of presets for " << _configId << " launched");
            for (const auto& preset : presets)
                asyncInstall(preset._uri, preset._stamp, preset._dest, session);
        }
        else
        {
            LOG_INF("Fetch of presets for "
                    << _configId << " completed immediately. Success: " << _overallSuccess);
            completed();
        }
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("Failed to install presets with exception: " << exc.what());
    }
}

static std::string extractViewSettings(const std::string& viewSettingsPath,
                                       const std::shared_ptr<ClientSession>& session,
                                       bool& _isViewSettingsUpdated)
{
    std::string viewSettingsString;
    std::ifstream ifs(viewSettingsPath);
    try
    {
        LOG_TRC("Parsing view settings JSON");
        Poco::JSON::Parser parser;
        auto result = parser.parse(ifs);
        Poco::JSON::Object::Ptr viewSettings = result.extract<Poco::JSON::Object::Ptr>();

        const std::string& userPrivateInfo = session->getUserPrivateInfo();
        Object::Ptr userPrivateInfoObj;
        if (!userPrivateInfo.empty())
        {
            try
            {
                Poco::JSON::Parser privateInfoParser;
                Poco::Dynamic::Var var = privateInfoParser.parse(userPrivateInfo);
                userPrivateInfoObj = var.extract<Object::Ptr>();
            }
            catch (const std::exception& exc)
            {
                LOG_DBG("User private data is not valid JSON: " << exc.what());
            }
        }

        std::string zoteroAPIKey, signatureCertificate, signatureKey, signatureCa,
            aiImageProviderAPIKey, aiImageProviderURL, aiImageModel, aiImageSize;

        bool viewSettingsNeedUpdate = false;

        auto migrateViewSettingsField = [&](const std::string& viewSettingKey,
                                          const std::string& privateInfoKey,
                                          std::string& value) -> bool
        {
            JsonUtil::findJSONValue(viewSettings, viewSettingKey, value);
            if (value.empty() && userPrivateInfoObj)
            {
                std::string migratedValue;
                JsonUtil::findJSONValue(userPrivateInfoObj, privateInfoKey, migratedValue);
                if (!migratedValue.empty())
                {
                    LOG_INF("Migrating field [" << viewSettingKey << "] from user private info");
                    viewSettings->set(viewSettingKey, migratedValue);
                    value = std::move(migratedValue);
                    return true;
                }
            }
            return false;
        };

        viewSettingsNeedUpdate |= migrateViewSettingsField("zoteroAPIKey", "ZoteroAPIKey", zoteroAPIKey);
        session->setZoteroAPIKey(zoteroAPIKey);

        viewSettingsNeedUpdate |= migrateViewSettingsField("signatureCert", "SignatureCert", signatureCertificate);
        session->setSignatureCertificate(signatureCertificate);

        viewSettingsNeedUpdate |= migrateViewSettingsField("signatureKey", "SignatureKey", signatureKey);
        session->setSignatureKey(signatureKey);

        viewSettingsNeedUpdate |= migrateViewSettingsField("signatureCa", "SignatureCa", signatureCa);
        session->setSignatureCa(signatureCa);

        _isViewSettingsUpdated = true;

        std::string resolvedAIModel;
        const bool aiConfigured = session->resolveAndApplyAICredentials(
            viewSettings, userPrivateInfoObj, session->isDisableAISettings(),
            viewSettingsNeedUpdate, resolvedAIModel);

        JsonUtil::findJSONValue(viewSettings, "aiImageProviderAPIKey", aiImageProviderAPIKey);
        JsonUtil::findJSONValue(viewSettings, "aiImageProviderURL", aiImageProviderURL);
        JsonUtil::findJSONValue(viewSettings, "aiImageModel", aiImageModel);
        JsonUtil::findJSONValue(viewSettings, "aiImageSize", aiImageSize);

        session->setAIImageProviderAPIKey(aiImageProviderAPIKey);
        session->setAIImageProviderURL(aiImageProviderURL);
        session->setAIImageModel(aiImageModel);
        session->setAIImageSize(aiImageSize);

        if (viewSettingsNeedUpdate)
        {
            LOG_INF("View settings updated with migrated fields, uploading to WOPI host");
            session->setViewSettingsJSON(viewSettings);
            session->uploadViewSettingsToWopiHost();
        }

        // remove API key from view settings before sending to client, client doesn't need to know about it
        // and it will be set in session for later use when calling AI provider,
        // also it is safer to not expose it to client side
        viewSettings->remove("aiProviderAPIKey");
        viewSettings->remove("aiProviderModel");
        viewSettings->remove("aiProviderURL");
        viewSettings->remove("aiImageProviderAPIKey");
        viewSettings->remove("aiImageProviderURL");
        viewSettings->remove("aiImageModel");

        viewSettings->set("aiConfigured", aiConfigured);
        if (aiConfigured)
            viewSettings->set("aiModelName", resolvedAIModel);
        viewSettingsString = JsonUtil::jsonToString(viewSettings);
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to parse view settings JSON with[" << ifs.rdbuf() << "] error[" << exc.what()
                                                         << "], for session[" << session->getId()
                                                         << ']');
        return viewSettingsString;
    }
    return viewSettingsString;
}

void DocumentBroker::asyncInstallPresets(const std::shared_ptr<ClientSession>& session,
                                         const std::string& configId,
                                         const std::string& userSettingsUri,
                                         const std::string& presetsPath)
{
    auto installFinishedCB =
        [selfWeak = weak_from_this(), this, session, userSettingsUri, presetsPath](bool success)
    {
        std::shared_ptr<DocumentBroker> selfLifecycle = selfWeak.lock();
        if (!selfLifecycle)
            return;

        if (success)
        {
            std::string searchDir = presetsPath;
            searchDir.append("wordbook");
            const auto fileNames = FileUtil::getDirEntries(searchDir);
            std::error_code ec;
            for (const auto& fileName : fileNames)
            {
                std::string filePath = searchDir;
                filePath.push_back('/');
                filePath.append(fileName);
                std::filesystem::file_time_type ts = std::filesystem::last_write_time(filePath, ec);
                if (ec)
                {
                    LOG_ERR("File[" << fileName << "] doesn't exist");
                    continue;
                }
                _presetTimestamp[fileName] = ts;
            }

            const std::string viewSettings = presetsPath + "viewsetting/viewsetting.json";
            if (FileUtil::Stat(viewSettings).exists())
            {
                const std::string settings = extractViewSettings(viewSettings, session, _isViewSettingsUpdated);
                session->sendTextFrame("viewsetting: " + settings);
            }
            forwardToChild(session, "addconfig");
        }
        else
        {
            const std::string uriAnonym = COOLWSD::anonymizeUrl(userSettingsUri);
            LOG_ERR("Failed to load all settings from [" << uriAnonym << ']');
            stop("configfailed");
        }

        UNITWSD_CALL_INSTANCE(_unitWsd, onDocBrokerPresetsInstallEnd(success));
    };

    UNITWSD_CALL_INSTANCE(_unitWsd, onDocBrokerPresetsInstallStart());
    _asyncInstallTask = asyncInstallPresets(_poll, configId, userSettingsUri,
                                            presetsPath, session, installFinishedCB);
    _asyncInstallTask->appendCallback([selfWeak = weak_from_this(), this,
                                       keepPollAlive=_poll](bool){
        // For the edge case where the DocumentBroker lifecycle ends before the document
        // gets loaded, extend life of _poll to ensure it exists until any pending
        // asyncConnect have completed (which require the poll to exist), and their
        // callbacks detect that the DocumentBroker has been destroyed.
        std::shared_ptr<DocumentBroker> selfLifecycle = selfWeak.lock();
        if (!selfLifecycle)
            return;

        _asyncInstallTask.reset();
    });
}

std::shared_ptr<const http::Response>
DocumentBroker::sendHttpSyncRequest(const std::string& url, const std::string& logContext)
{
    const Poco::URI uri{ url };
    std::shared_ptr<http::Session> httpSession(StorageConnectionManager::getHttpSession(uri));
    http::Request request(uri.getPathAndQuery());

    const std::string uriAnonym = COOLWSD::anonymizeUrl(url);
    LOG_DBG("Getting " << logContext << " from [" << uriAnonym << "] using sync request");
    std::shared_ptr<const http::Response> httpResponse = httpSession->syncRequest(request);
    const http::StatusLine statusLine = httpResponse->statusLine();

    LOG_TRC("sendHttpSyncRequest returned " << statusLine.statusCode() << " when fetching "
                                            << logContext << " json");

    if (statusLine.statusCode() != http::StatusCode::OK)
    {
        LOG_ERR("Failed to get " << logContext << " json from [" << uriAnonym << "] with status["
                                 << statusLine.reasonPhrase() << ']');
        return nullptr;
    }

    return httpResponse;
}

void DocumentBroker::sendBrowserSetting(const std::shared_ptr<ClientSession>& session)
{
    auto browsersetting = session->getBrowserSettingJSON();
    std::ostringstream jsonStream;
    browsersetting->stringify(jsonStream, 2);
    const std::string& jsonStr = jsonStream.str();
    session->sendTextFrame("browsersetting: " + jsonStr);
    session->setSentBrowserSetting(true);
    LOG_TRC("Sent browsersetting json[" << jsonStr << "] to client with sessionId["
                                        << session->getId() << ']');
}

void DocumentBroker::getBrowserSettingSync(const std::shared_ptr<ClientSession>& session,
                                           const std::string& userSettingsUri)
{
    if (session == nullptr || session->getSentBrowserSetting())
        return;

    const auto userSettingsResponse = sendHttpSyncRequest(userSettingsUri, "usersetting");
    if (!userSettingsResponse)
        return;

    const std::string& body = userSettingsResponse->getBody();
    Poco::JSON::Object::Ptr settings;
    if (!JsonUtil::parseJSON(body, settings))
    {
        LOG_ERR("Failed to parse usersetting json");
        return;
    }

    const auto browsersetting = settings->getArray("browsersetting");
    if (!browsersetting || browsersetting->size() == 0)
    {
        LOG_INF("browsersetting doesn't exist in usersetting json or empty");
        sendBrowserSetting(session);
        return;
    }

    const auto firstElem = browsersetting->get(0).extract<Poco::JSON::Object::Ptr>();
    if (!firstElem)
    {
        sendBrowserSetting(session);
        return;
    }

    const std::string browsersettingUri = JsonUtil::getJSONValue<std::string>(firstElem, "uri");

    const auto browsersettingResponse = sendHttpSyncRequest(browsersettingUri, "browsersetting");
    if (!browsersettingResponse)
    {
        sendBrowserSetting(session);
        return;
    }

    if (parseBrowserSettings(session, browsersettingResponse->getBody()))
        sendBrowserSetting(session);
}

struct PresetRequest
{
    std::string _uri;
    std::string _fileName;
    std::string _id;
};

std::shared_ptr<PresetsInstallTask>
DocumentBroker::asyncInstallPresets(const std::shared_ptr<SocketPoll>& poll, const std::string& configId,
                                    const std::string& userSettingsUri,
                                    const std::string& presetsPath,
                                    const std::shared_ptr<ClientSession>& session,
                                    const std::function<void(bool)>& installFinishedCB)
{
    // Download the json for settings
    const Poco::URI settingsUri{userSettingsUri};
    std::shared_ptr<http::Session> httpSession(StorageConnectionManager::getHttpSession(settingsUri));
    http::Request request(settingsUri.getPathAndQuery());

    std::string uriAnonym = COOLWSD::anonymizeUrl(userSettingsUri);
    LOG_DBG("Getting settings from [" << uriAnonym << ']');

    auto presetTasks = std::make_shared<PresetsInstallTask>(poll, configId, presetsPath,
                                                            installFinishedCB);

    // When result arrives, extract uris of what we want to install to the jail's user presets
    // and async download and install those.
    http::Session::FinishedCallback finishedCallback =
        [uriAnonym = std::move(uriAnonym), presetsPath, presetTasks,
         session](const std::shared_ptr<http::Session>& configSession)
    {
        if (SigUtil::getShutdownRequestFlag())
        {
            LOG_DBG("Shutdown flagged, giving up on in-flight requests");
            presetTasks->install(nullptr, nullptr);
            return;
        }

        const std::shared_ptr<const http::Response> httpResponse = configSession->response();
        const http::StatusLine statusLine = httpResponse->statusLine();

        LOG_TRC("DocumentBroker::asyncInstallPresets returned " << statusLine.statusCode());
        const bool failed = (statusLine.statusCode() != http::StatusCode::OK);
        if (failed)
        {
            LOG_ERR("Failed to get settings json from [" << uriAnonym << "] with status["
                                                         << statusLine.reasonPhrase() << ']');
            presetTasks->install(nullptr, nullptr);
            return;
        }

        const std::string& body = httpResponse->getBody();

        LOG_DBG("Presets JSON for [" << uriAnonym << "] is: " << body);

        Poco::JSON::Object::Ptr settings;
        if (!JsonUtil::parseJSON(body, settings))
        {
            LOG_ERR("Parse of userSettings json: " << uriAnonym << " failed");
            presetTasks->install(nullptr, nullptr);
            return;
        }

        presetTasks->install(settings, session);
    };

    httpSession->setFinishedHandler(std::move(finishedCallback));

    // Run the request on the WebServer Poll.
    httpSession->asyncRequest(request, poll);

    return presetTasks;
}

void DocumentBroker::asyncInstallPreset(
    const std::shared_ptr<SocketPoll>& poll, const std::string& configId, const std::string& presetUri,
    const std::string& presetStamp, const std::string& presetFile, const std::string& id,
    const std::function<void(const std::string&, bool)>& finishedCB,
    const std::shared_ptr<ClientSession>& session)
{
    std::string uriAnonym = COOLWSD::anonymizeUrl(presetUri);
    LOG_DBG("Getting preset from [" << uriAnonym << ']');

    const Poco::URI uri{presetUri};
    std::shared_ptr<http::Session> httpSession(StorageConnectionManager::getHttpSession(uri));
    http::Request request(uri.getPathAndQuery());

    http::Session::FinishedCallback finishedCallback =
        [configId, presetUri, presetStamp, uriAnonym=std::move(uriAnonym),
         presetFile, id, finishedCB](const std::shared_ptr<http::Session>& presetSession)
    {
        if (SigUtil::getShutdownRequestFlag())
        {
            LOG_DBG("Shutdown flagged, giving up on in-flight requests");
            return;
        }

        const std::shared_ptr<const http::Response> presetHttpResponse = presetSession->response();

        bool success = false;
        const http::StatusLine statusLine = presetHttpResponse->statusLine();
        if (statusLine.statusCode() != http::StatusCode::OK)
        {
            LOG_ERR("Failed to fetch preset uri[" << uriAnonym << "] with status["
                                                  << statusLine.reasonPhrase() << ']');
            FileUtil::removeFile(presetFile);
        }
        else
        {
            success = true;
            LOG_INF("Fetch of preset uri[" << uriAnonym << "] to " << presetFile << " succeeded");
            Cache::cacheConfigFile(configId, presetUri, presetStamp, presetFile);

            // delete the browsersetting json from jail
            // we only saved it to make sure cache util can copy it
            if (presetFile.ends_with("browsersetting.json"))
                FileUtil::removeFile(presetFile);
        }

        if (finishedCB)
            finishedCB(id, success);
    };

    httpSession->setFinishedHandler(std::move(finishedCallback));

    // Run the request on the WebServer Poll.
    httpSession->asyncRequest(request, poll);

    const std::shared_ptr<http::Response> presetHttpResponse = httpSession->response();

    if (presetFile == "browsersetting.json")
    {
        if (session == nullptr || session->getSentBrowserSetting())
            return;
        const std::string& body = presetHttpResponse->getBody();
        if (DocumentBroker::parseBrowserSettings(session, body))
            DocumentBroker::sendBrowserSetting(session);
    }

    LOG_DBG("Saving preset file to jailPath[" << presetFile << ']');
    presetHttpResponse->saveBodyToFile(presetFile);
}

bool DocumentBroker::parseBrowserSettings(const std::shared_ptr<ClientSession>& session,
                                          const std::string& responseBody)
{
    try
    {
        LOG_TRC("Parsing browsersetting json from repsonseBody[" << responseBody << ']');
        Poco::JSON::Parser parser;
        auto result = parser.parse(responseBody);
        const auto& browsersetting = result.extract<Poco::JSON::Object::Ptr>();
        if (browsersetting.isNull())
        {
            LOG_INF("browsersetting.json is empty");
            return true;
        }

        LOG_TRC("Setting _browserSettingsJSON for clientsession[" << session->getId() << ']');
        session->setBrowserSettingsJSON(browsersetting);
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to parse browsersetting json["
                << responseBody << "] with error[" << exc.what()
                << "], disabling browsersetting for session[" << session->getId() << ']');
        return false;
    }
    return true;
}

bool DocumentBroker::processPlugins(std::string& localPath)
{
    // Check if we have a prefilter "plugin" for this document format
    for (const auto& plugin : COOLWSD::PluginConfigurations)
    {
        try
        {
            const std::string extension(plugin->getString("prefilter.extension"));
            const std::string newExtension(plugin->getString("prefilter.newextension"));
            std::string commandLine(plugin->getString("prefilter.commandline"));

            if (localPath.length() > extension.length() + 1 &&
                strcasecmp(localPath.substr(localPath.length() - extension.length() - 1).data(),
                           (std::string(".") + extension).data()) == 0)
            {
                // Extension matches, try the conversion. We convert the file to another one in
                // the same (jail) directory, with just the new extension tacked on.

                const std::string newRootPath = _storage->getRootFilePath() + '.' + newExtension;

                // The commandline must contain the space-separated substring @INPUT@ that is
                // replaced with the input file name, and @OUTPUT@ for the output file name.
                int inputs(0), outputs(0);

                std::string input("@INPUT");
                std::size_t pos = commandLine.find(input);
                if (pos != std::string::npos)
                {
                    commandLine.replace(pos, input.length(), _storage->getRootFilePath());
                    ++inputs;
                }

                std::string output("@OUTPUT@");
                pos = commandLine.find(output);
                if (pos != std::string::npos)
                {
                    commandLine.replace(pos, output.length(), newRootPath);
                    ++outputs;
                }

                StringVector args(StringVector::tokenize(commandLine, ' '));
                std::string command(args[0]);
                args.erase(args.begin()); // strip the command

                if (inputs != 1 || outputs != 1)
                    throw std::exception();

                const int process = ProcUtil::spawnProcess(command, args);
                int status = -1;
                const int rc = ::waitpid(process, &status, 0);
                if (rc != 0)
                {
                    LOG_ERR("Conversion from " << extension << " to " << newExtension << " failed ("
                                               << rc << ") while running plugin [" << commandLine
                                               << ']');
                    return false;
                }

                _storage->setRootFilePath(newRootPath);
                localPath += '.' + newExtension;
            }

            // We successfully converted the file to something LO can use; break out of the for
            // loop.
            break;
        }
        catch (const std::exception&)
        {
            // This plugin is not a proper prefilter one
        }
    }

    return true;
}
#endif // !MOBILEAPP

std::string DocumentBroker::handleRenameFileCommand(std::string sessionId,
                                                    std::string newFilename)
{
    if (newFilename.empty())
        return "error: cmd=renamefile kind=invalid"; //TODO: better filename validation.

    if (_docState.activity() == DocumentState::Activity::Rename)
    {
        if (_renameFilename != newFilename)
            return "error: cmd=renamefile kind=conflict"; // Renaming in progress.
    }

    _renameFilename = std::move(newFilename);
    _renameSessionId = std::move(sessionId);

    if (_docState.activity() == DocumentState::Activity::None)
    {
        // We can start by saving now.
        startRenameFileCommand();
    }

    return std::string();
}

void DocumentBroker::startRenameFileCommand()
{
    LOG_DBG("Starting renamefile command execution");

    if (_renameSessionId.empty() || _renameFilename.empty())
    {
        assert(!"Saving before renaming without valid filename or sessionId.");
        LOG_DBG("Error: Trying to saveBeforeRename with invalid filename ["
                << _renameFilename << "] and/or sessionId [" << _renameSessionId << "]");
        return;
    }

    // Transition.
    if (!startActivity(DocumentState::Activity::Rename))
    {
        return;
    }

    blockUI("rename"); // Prevent user interaction while we start renaming.

    const auto it = _sessions.find(_renameSessionId);
    if (it == _sessions.end())
    {
        LOG_ERR("Session [" << _renameSessionId << "] not found to save docKey [" << _docKey
                            << "] before renaming. The document will not be renamed.");
        broadcastSaveResult(false, "Renaming session not found");
        endRenameFileCommand();
        return;
    }

    constexpr bool dontTerminateEdit = false; // We will save, rename, and reload: terminate.
    constexpr bool dontSaveIfUnmodified = true;
    constexpr bool isAutosave = false;
    constexpr bool finalWrite = true;
    sendUnoSave(it->second, dontTerminateEdit, dontSaveIfUnmodified, isAutosave, finalWrite);
}

void DocumentBroker::endRenameFileCommand()
{
    LOG_DBG("Ending renamefile command execution");

    _renameSessionId.clear();
    _renameFilename.clear();

    unblockUI();

    endActivity();
}

bool DocumentBroker::updateStorageLockState(ClientSession& session, StorageBase::LockState lock,
                                            std::string& error)
{
    LOG_TRC("Requesting async " << StorageBase::nameShort(lock) << "ing of [" << _docKey
                                << "] by session #" << session.getId());

    if (session.getAuthorization().isExpired())
    {
        error = "Expired authorization token";
        return false;
    }

    if (lock == StorageBase::LockState::LOCK && session.isReadOnly())
    {
        // Readonly sessions cannot lock, only editors can.
        error = "Readonly session";
        return false;
    }

    if (!_storage)
    {
        error = "Missing storage";
        return false;
    }

    const StorageBase::LockUpdateResult result = _storage->updateLockState(
        session.getAuthorization(), *_lockCtx, lock, _currentStorageAttrs);

    return handleLockResult(session, result);
}

bool DocumentBroker::updateStorageLockStateAsync(const std::shared_ptr<ClientSession>& session,
                                                 StorageBase::LockState lock, std::string& error)
{
    LOG_TRC("Requesting async " << StorageBase::nameShort(lock) << "ing of [" << _docKey
                                << "] by session #" << session->getId());

    if (session->getAuthorization().isExpired())
    {
        error = "Expired authorization token";
        return false;
    }

    if (lock == StorageBase::LockState::LOCK && session->isReadOnly())
    {
        // Readonly sessions cannot lock, only editors can.
        error = "Readonly session";
        return false;
    }

    if (_lockStateUpdateRequest)
    {
        error = "A lock-update request is already in progress";
        return false;
    }

    // Do *not* capture the session shared_ptr, to let it close if necessary.
    // Instead, we capture a weak_ptr, which allows for graceful cleanup of closed sesssions.
    StorageBase::AsyncLockStateCallback asyncLockCallback =
        [this](const StorageBase::AsyncLockUpdate& asyncLock)
    {
        if (!_lockStateUpdateRequest)
        {
            LOG_ERR("There is no asynchronous lock-state update request to process callback");
            return;
        }

        if (asyncLock.state() == StorageBase::AsyncLockUpdate::State::Running)
        {
            LOG_TRC("Async locking of [" << _docKey << "] is in progress during "
                                         << DocumentState::name(_docState.activity()));
            return;
        }

        const std::shared_ptr<ClientSession> requestingSession = _lockStateUpdateRequest->session();
        _lockStateUpdateRequest.reset(); // No longer needed.

        if (!requestingSession)
        {
            LOG_DBG("RequestingSession no longer exists");
            return;
        }

        // We have some result, look at the result status.
        handleLockResult(*requestingSession, asyncLock.result());
    };

    _lockStateUpdateRequest = std::make_unique<LockStateUpdateRequest>(lock, session);

    _storage->updateLockStateAsync(session->getAuthorization(), *_lockCtx, lock,
                                   _currentStorageAttrs, _poll, asyncLockCallback);
    return true;
}

bool DocumentBroker::handleLockResult(ClientSession& session,
                                      const StorageBase::LockUpdateResult& result)
{
    const StorageBase::LockState requestedLock = result.requestedLockState();
    const std::string& reason = result.getReason();

    switch (result.getStatus())
    {
        case StorageBase::LockUpdateResult::Status::UNSUPPORTED:
            LOG_DBG("Locks on docKey [" << _docKey << "] are unsupported while trying to "
                                        << StorageBase::nameShort(requestedLock));
            return true; // Not an error.
            break;

        case StorageBase::LockUpdateResult::Status::OK:
            LOG_DBG(StorageBase::nameShort(requestedLock)
                    << "ed docKey [" << _docKey << "] successfully");
            _lockCtx->setState(requestedLock);
            return true;
            break;

        case StorageBase::LockUpdateResult::Status::UNAUTHORIZED:
        {
            LOG_ERR("Failed to " << StorageBase::nameShort(requestedLock) << " docKey [" << _docKey
                                 << "]. Invalid or expired access token. Notifying client and "
                                    "invalidating the authorization token of session ["
                                 << session.getId() << "]. This session will now be read-only");
            session.invalidateAuthorizationToken();
            if (requestedLock == StorageBase::LockState::LOCK)
            {
                // If we can't unlock, we don't want to set the document to read-only mode.
                session.setLockFailed(reason);
            }
        }
        break;

        case StorageBase::LockUpdateResult::Status::FAILED:
        {
            LOG_ERR("Failed to " << StorageBase::nameShort(requestedLock) << " docKey [" << _docKey
                                 << "] with reason [" << reason
                                 << "]. Notifying client and making session [" << session.getId()
                                 << "] read-only");

            if (requestedLock == StorageBase::LockState::LOCK)
            {
                // If we can't unlock, we don't want to set the document to read-only mode.
                session.setLockFailed(reason);
            }
        }
        break;
    }

    return false;
}

bool DocumentBroker::attemptLock(ClientSession& session, std::string& failReason)
{
    return updateStorageLockState(session, StorageBase::LockState::LOCK, failReason);
}

DocumentBroker::NeedToUpload DocumentBroker::needToUploadToStorage() const
{
    const CanUpload canUpload = canUploadToStorage();
    if (canUpload != CanUpload::Yes)
    {
        // This can happen when we reject the connection (unauthorized).
        LOG_TRC("Cannot upload to storage: " << name(canUpload));
        return NeedToUpload::No;
    }

    // When destroying, we might have to force uploading if always_save_on_exit=true.
    // If unloadRequested is set, assume we will unload after uploading and exit.
    if (isUnloading() && _alwaysSaveOnExit && _saveManager.version() > 0)
    {
        if (_documentChangedInStorage)
        {
            LOG_INF("Need to upload per always_save_on_exit config while the document has a "
                    "conflict");
        }
        else
        {
            LOG_INF("Need to upload per always_save_on_exit config "
                    << (isMarkedToDestroy() ? "MarkedToDestroy" : "Unloading"));
        }

        return NeedToUpload::Yes;
    }

    // Retry uploading only for retryable failures, not conflicts.
    if (!_storageManager.lastUploadSuccessful() && !_documentChangedInStorage)
    {
        LOG_DBG("Uploading to storage as last attempt had failed");
        return NeedToUpload::Yes;
    }

    // Finally, see if we have a newer version than storage.
    if (isStorageOutdated())
        return NeedToUpload::Yes; // Timestamp changed, upload.

    return NeedToUpload::No; // No reason to upload, seems up-to-date.
}

bool DocumentBroker::isStorageOutdated() const
{
    if (!_storage)
    {
        return false;
    }

    // Get the modified-time of the file on disk.
    const auto st = FileUtil::Stat(_storage->getRootFilePathUploading());
    if (!st.exists())
    {
        LOG_TRC("File to upload to storage [" << _storage->getRootFilePathUploading()
                                              << "] does not exist");
        return false;
    }

    const std::chrono::system_clock::time_point currentModifiedTime = st.modifiedTimepoint();
    const std::chrono::system_clock::time_point lastModifiedTime =
        _storageManager.getLastUploadedFileModifiedLocalTime();

    LOG_TRC("File to upload to storage ["
            << _storage->getRootFilePathUploading() << "] was modified at " << currentModifiedTime
            << " and the last uploaded file was modified at " << lastModifiedTime << ", which are "
            << (currentModifiedTime == lastModifiedTime ? "identical" : "different"));

    if (Util::isDebugEnabled() && _storageManager.getLastUploadedFileModifiedLocalTime() !=
                                      _saveManager.getLastModifiedLocalTime())
    {
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        LOG_ERR("StorageManager's lastModifiedTime ["
                << Util::getTimeForLog(now, _storageManager.getLastUploadedFileModifiedLocalTime())
                << "] doesn't match that of SaveManager's ["
                << Util::getTimeForLog(now, _saveManager.getLastModifiedLocalTime())
                << "]. File lastModifiedTime: [" << Util::getTimeForLog(now, currentModifiedTime)
                << ']');
    }

    // Compare to the last uploaded file's modified-time.
    return currentModifiedTime != lastModifiedTime;
}

bool DocumentBroker::isNextSaveAutosave() const
{
    return _nextStorageAttrs.isAutosave();
}

void DocumentBroker::handleSaveResponse(const std::shared_ptr<ClientSession>& session,
                                        const Poco::JSON::Object::Ptr& json)
{
    ASSERT_CORRECT_THREAD();

    // When dontSaveIfUnmodified=true, there is a shortcut in LOKit
    // that shortcuts saving when the document is not modified.
    // In that case, success=false and result=unmodified.
    const bool success = json->get("success").toString() == "true";
    std::string result;
    if (json->has("result"))
    {
        const Poco::Dynamic::Var parsedResultJSON = json->get("result");
        const auto& resultObj = parsedResultJSON.extract<Poco::JSON::Object::Ptr>();
        if (resultObj->get("type").toString() == "string")
            result = resultObj->get("value").toString();
    }

    // wasModified is only set when LOKit saves the document.
    // If the document was modified before saving, it would
    // be true. Otherwise, it's false. Meaningful when forced
    // saving (i.e. dontSaveIfUnmodified=false), otherwise
    // result is blank in that case and we can't know if
    // the document saved was modified or not.
    if (json->has("wasModified"))
    {
        // If Core reports the modified state before saving,
        // use it to report to the Storage with more confidence.
        const bool wasModified = (json->get("wasModified").toString() == "true");
        LOG_DBG("Core reported that the file was " << (wasModified ? "" : "not ")
                                                   << "modified before saving");
        _nextStorageAttrs.setUserModified(wasModified);
    }

    // Update the storage attributes to capture what's
    // new and applies to this new version and reset the next.
    // These are the attributes of the version to be uploaded.
    // Note: these are owned by us and this is thread-safe.
    _currentStorageAttrs.merge(_nextStorageAttrs);
    _nextStorageAttrs.reset();

    // Record that we got a response to avoid timing out on saving.
    _saveManager.setLastSaveResult(success || result == "unmodified", /*newVersion=*/success);

    if (success)
        LOG_DBG("Save result from Core: saved (during " << DocumentState::name(_docState.activity())
                                                        << ") in "
                                                        << _saveManager.lastSaveDuration());
    else if (result == "unmodified")
        LOG_DBG("Save result from Core: unmodified (during "
                << DocumentState::name(_docState.activity()) << ") in "
                << _saveManager.lastSaveDuration());
    else // Failure with error.
        LOG_WRN("Save result from Core (failure): " << result << " (during "
                                                    << DocumentState::name(_docState.activity())
                                                    << ") in " << _saveManager.lastSaveDuration());

    if constexpr (!Util::isMobileApp())
    {
        // Create the 'upload' file regardless of success or failure,
        // because we don't know if the last upload worked or not.
        // DocBroker will have to decide to upload or skip.
        const std::string oldName = _storage->getRootFilePathToUpload();
        if (FileUtil::Stat(oldName).exists())
        {
            if (_quarantine && _quarantine->isEnabled())
            {
                // Quarantine the file before renaming, if it exists.
                LOG_DBG("Quarantining the old file after saving: " << oldName);
                _quarantine->quarantineFile(oldName);
            }

            // Rename even if no new save, in case we have an older version.
            const std::string newName = _storage->getRootFilePathUploading();
            if (::rename(oldName.c_str(), newName.c_str()) < 0)
            {
                LOG_SYS("Failed to rename [" << oldName << "] to [" << newName << ']');
            }
            else
            {
                LOG_TRC("Renamed [" << oldName << "] to [" << newName << ']');
            }
        }
    }

    // Let the clients know of any save failures.
    if (!success && result != "unmodified")
    {
        LOG_INF("Failed to save docKey [" << _docKey
                                          << "] as .uno:Save has failed in COKit. Notifying clients");
        session->sendTextFrameAndLogError("error: cmd=storage kind=savefailed");
        broadcastSaveResult(false, "Could not save the document");
    }

    checkAndUploadToStorage(session, /*justSaved=*/success || result == "unmodified");
}

// This is called when either we just got save response, or,
// there was nothing to save and want to check for uploading.
void DocumentBroker::checkAndUploadToStorage(const std::shared_ptr<ClientSession>& session,
                                             bool justSaved)
{
    const std::string sessionId = session->getId();
    LOG_TRC("checkAndUploadToStorage with session [" << sessionId << "], justSaved: " << justSaved);

    // See if we have anything to upload.
    const NeedToUpload needToUploadState = needToUploadToStorage();

    LOG_TRC("checkAndUploadToStorage with session ["
            << sessionId << "], justSaved: " << justSaved
            << ", activity: " << DocumentState::name(_docState.activity())
            << ", needToUpload: " << name(needToUploadState));

    // Handle activity-specific logic.
    switch (_docState.activity())
    {
        case DocumentState::Activity::Rename:
        {
            // If we have nothing to upload, do the rename now.
            if (needToUploadState == NeedToUpload::No)
            {
                const auto it = _sessions.find(_renameSessionId);
                if (it == _sessions.end())
                {
                    LOG_ERR("Session [" << _renameSessionId << "] not found to rename docKey ["
                                        << _docKey << "]. The document will not be renamed.");
                    broadcastSaveResult(false, "Renaming session not found");
                    endRenameFileCommand();
                }
                else
                {
                    LOG_DBG("Renaming in storage as there is no new version to upload first");
                    std::string uploadAsPath;
                    constexpr bool isRename = true;
                    constexpr bool isExport = false;
                    constexpr bool force = false;
                    uploadToStorageInternal(it->second, uploadAsPath, _renameFilename, isRename,
                                            isExport, force);
                }

                return;
            }
        }
        break;

        case DocumentState::Activity::Save:
        {
            // Done saving.
            endActivity();
        }
        break;

#if !MOBILEAPP && !WASMAPP
        case DocumentState::Activity::SwitchingToOffline:
        {
            // If we have nothing to upload, do the switching now.
            if (needToUploadState == NeedToUpload::No)
            {
                switchToOffline();
                return;
            }
        }
        break;
#endif // !MOBILEAPP && !WASMAPP

        default:
        break;
    }

    if constexpr (!Util::isMobileApp())
    {
        // Avoid multiple uploads during unloading if we know we need to save a new version.
        const bool unloading = isUnloading();
        const bool modified =
            justSaved ? haveModifyActivityAfterSaveRequest() : needToSaveToDisk() != NeedToSave::No;

        if (modified && unloading)
        {
            // We are unloading but have possible modifications. Save again (done in poll).
            LOG_DBG(
                "Document [" << getDocKey()
                             << "] is unloading, but was possibly modified during saving. Skipping "
                                "upload to save again before unloading");

            assert(canSaveToDisk() == CanSave::Yes && "Cannot save to disk");
            return;
        }
    }

    if (needToUploadState != NeedToUpload::No)
    {
        uploadToStorage(session, /*force=*/false);
    }
    else if (!isAsyncUploading())
    {
        // If session is disconnected, remove.
        LOG_TRC("Nothing to upload, disconnecting closed sessions");
        for (const auto& pair : _sessions)
        {
            if (pair.second->isCloseFrame() && !pair.second->inWaitDisconnected())
            {
                LOG_TRC("Disconnecting session [" << pair.second->getName() << ']');
                disconnectSessionInternal(pair.second);
            }
        }
    }
}

void DocumentBroker::uploadToStorage(const std::shared_ptr<ClientSession>& session, bool force)
{
    ASSERT_CORRECT_THREAD();

    LOG_TRC("uploadToStorage [" << session->getId() << "]: " << (force ? "" : "not") << " forced");

    // Upload immediately if forced or had no failures. Otherwise, throttle (on failure).
    if (force || _storageManager.lastUploadSuccessful() ||
        _storageManager.canUploadNow(isUnloading()))
    {
        constexpr bool isRename = false;
        constexpr bool isExport = false;
        uploadToStorageInternal(session, /*saveAsPath*/ std::string(),
                                /*saveAsFilename*/ std::string(), isRename, isExport, force);
    }
    else
    {
        LOG_DBG("Last upload had failed and it's only been "
                << _storageManager.timeSinceLastUploadResponse()
                << " since. Min time between uploads: " << _storageManager.minTimeBetweenUploads());
    }
}

void DocumentBroker::uploadAsToStorage(const std::shared_ptr<ClientSession>& session,
                                       const std::string& uploadAsPath,
                                       const std::string& uploadAsFilename, const bool isRename,
                                       const bool isExport)
{
    ASSERT_CORRECT_THREAD();

    uploadToStorageInternal(session, uploadAsPath, uploadAsFilename, isRename, isExport, /*force=*/false);
}

void DocumentBroker::uploadAfterLoadingTemplate(const std::shared_ptr<ClientSession>& session)
{
    LOG_ASSERT_MSG(session, "Must have a valid ClientSession");

    if constexpr (!Util::isMobileApp())
    {
        // Create the 'upload' file as it gets created only when
        // handling .uno:Save, which isn't issued for templates
        // (save is done in Kit right after loading a template).
        const std::string oldName = _storage->getRootFilePathToUpload();
        const std::string newName = _storage->getRootFilePathUploading();
        if (::rename(oldName.c_str(), newName.c_str()) < 0)
        {
            // It's not an error if there was no file to rename, when the document isn't modified.
            LOG_SYS("Expected to renamed the document ["
                    << oldName << "] after template-loading to [" << newName << ']');
        }
        else
        {
            LOG_TRC("Renamed [" << oldName << "] to [" << newName << ']');
        }
    }

    uploadToStorage(session, /*force=*/false);
}

void DocumentBroker::uploadToStorageInternal(const std::shared_ptr<ClientSession>& session,
                                             const std::string& saveAsPath,
                                             const std::string& saveAsFilename, const bool isRename,
                                             const bool isExport, const bool force)
{
    ASSERT_CORRECT_THREAD();
    LOG_ASSERT_MSG(session, "Must have a valid ClientSession");

    const std::string sessionId = session->getId();
    if (!session->isEditable())
    {
        LOG_WRN("Session [" << sessionId << "] is read-only and cannot upload docKey [" << _docKey
                            << ']');
        return;
    }

    LOG_DBG("Uploading to storage docKey [" << _docKey << "] for session [" << sessionId
                                            << "]. Force: " << force);

    const bool isSaveAs = !saveAsPath.empty();
    const std::string uri = isSaveAs ? saveAsPath : session->getPublicUri().toString();

    // Map the FileId from the docKey to the new filename to anonymize the new filename as the FileId.
    const std::string newFilename = Uri::getFilenameFromURL(uri);
    const std::string fileId = Uri::getFilenameFromURL(Uri::decode(_docKey));
    if (COOLWSD::AnonymizeUserData)
    {
        LOG_DBG("New filename [" << COOLWSD::anonymizeUrl(newFilename)
                                 << "] will be known by its fileId [" << fileId << ']');

        Anonymizer::mapAnonymized(newFilename, fileId);
    }

    if (!_storage)
    {
        LOG_WRN("Expected to have a valid Storage instance, but doesn't have one");
        return;
    }

    const std::string uriAnonym = COOLWSD::anonymizeUrl(uri);

    // If the file timestamp hasn't changed, skip uploading.
    const std::chrono::system_clock::time_point newFileModifiedTime
        = FileUtil::Stat(_storage->getRootFilePathUploading()).modifiedTimepoint();
    if (!isSaveAs && newFileModifiedTime == _saveManager.getLastModifiedLocalTime() && !isRename &&
        !force)
    {
        // We can end up here when an earlier upload attempt had failed because
        // of a connection failure. In that case, _storageManger.lastUploadSuccessful()
        // will be false (see below: _storageManager.setLastUploadResult()), so
        // needToUploadToStorage() will return true. However, since there are no
        // new document saves, the timestamps will match. Instead of skipping uploading,
        // which would leave the lastUplaodSuccessful() as false permanently, we upload.
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        LOG_WRN("Uploading to URI ["
                << uriAnonym << "] with docKey [" << _docKey
                << "] even though it's unnecessary as the file lastModifiedTime [].  File "
                   "lastModifiedTime ["
                << Util::getTimeForLog(now, newFileModifiedTime)
                << "] is identical to the SaveManager's ["
                << Util::getTimeForLog(now, _saveManager.getLastModifiedLocalTime())
                << "]. StorageManager's lastModifiedTime ["
                << Util::getTimeForLog(now, _storageManager.getLastUploadedFileModifiedLocalTime())
                << ']');
    }

    LOG_DBG("Uploading [" << _docKey << "] after saving to URI [" << uriAnonym << ']');

    _uploadRequest = std::make_unique<UploadRequest>(uriAnonym, newFileModifiedTime, session,
                                                     isSaveAs, isExport, isRename);

    StorageBase::AsyncUploadCallback asyncUploadCallback =
        [this](const StorageBase::AsyncUpload& asyncUp)
    {
        // asyncUploadCallback called twice, 2nd time with onHandshakeFail/callOnConnectFail
        if (!_uploadRequest)
        {
            LOG_WRN("Expected to have a valid UploadRequest instance");
            return;
        }

        switch (asyncUp.state())
        {
            case StorageBase::AsyncUpload::State::Running:
                LOG_TRC("Async upload of [" << _docKey << "] is in progress during "
                                            << DocumentState::name(_docState.activity()));
                return;

            case StorageBase::AsyncUpload::State::Complete:
            {
                LOG_TRC("Finished uploading [" << _docKey << "] during "
                                               << DocumentState::name(_docState.activity())
                                               << ", processing results.");
                _uploadRequest->setComplete();
                return handleUploadToStorageResponse(asyncUp.result());
            }

            case StorageBase::AsyncUpload::State::None: // Unexpected: fallback.
            case StorageBase::AsyncUpload::State::Error:
                _uploadRequest->setComplete();
                broadcastSaveResult(false, "Could not upload document to storage",
                                    asyncUp.result().getReason());
                // [[fallthrough]]
        }

        LOG_WRN("Failed to upload [" << _docKey << "] asynchronously. "
                                     << DocumentState::name(_docState.activity()));
        _storageManager.setLastUploadResult(false);

        switch (_docState.activity())
        {
            case DocumentState::Activity::Rename:
            {
                LOG_DBG("Failed to renameFile because uploading post-save failed.");
                const std::string renameSessionId = _renameSessionId;
                endRenameFileCommand();

                auto it = _sessions.find(renameSessionId);
                if (it != _sessions.end() && it->second)
                    it->second->sendTextFrameAndLogError("error: cmd=renamefile kind=failed");
            }
            break;

#if !MOBILEAPP && !WASMAPP
            case DocumentState::Activity::SwitchingToOffline:
            {
                LOG_DBG("Failed to switch to Offline because uploading post-save failed");
                endSwitchingToOffline();
                //TODO: Send error to the user.
            }
            break;
#endif // !MOBILEAPP && !WASMAPP

            default:
                reportUploadToStorageFailed();
            break;
        }
    };

    _lastStorageAttrs = _currentStorageAttrs;
    _currentStorageAttrs.reset();

    // Once set, isUnloading shouldn't be unset.
    _lastStorageAttrs.setIsExitSave(isUnloading());

    if (force)
    {
        // Don't reset the force flag if it was set
        // (which would imply we failed to upload).
        _lastStorageAttrs.setForced(true);
    }

    _nextStorageAttrs.reset();

    _storageManager.markLastUploadRequestTime();
    const std::size_t size = _storage->uploadLocalFileToStorageAsync(
        session->getAuthorization(), *_lockCtx, saveAsPath, saveAsFilename, isRename,
        _lastStorageAttrs, _poll, asyncUploadCallback);

    _storageManager.setSizeAsUploaded(size);
}

void DocumentBroker::handleUploadToStorageSuccessful(const StorageBase::UploadResult& uploadResult)
{
    assert(_uploadRequest && "Expected to have a valid UploadRequest instance");
    LOG_DBG("Last upload result: OK");

#if !MOBILEAPP
    WopiStorage* wopiStorage = dynamic_cast<WopiStorage*>(_storage.get());
    if (wopiStorage != nullptr)
        _admin.setDocWopiUploadDuration(_docKey, wopiStorage->getWopiSaveDuration());
#endif

    if (!_uploadRequest->isSaveAs() && !_uploadRequest->isRename())
    {
        // Saved and stored; update flags.
        _saveManager.setLastModifiedLocalTime(_uploadRequest->newFileModifiedLocalTime());

        // Save the storage timestamp.
        _storageManager.setLastModifiedServerTimeString(_storage->getLastModifiedTime());

        // Set the timestamp of the file we uploaded, to detect changes.
        _storageManager.setLastUploadedFileModifiedLocalTime(
            _uploadRequest->newFileModifiedLocalTime());

        // After a successful save, we are sure that document in the storage is same as ours
        _documentChangedInStorage = false;

        // Reset the storage attributes; They've been used and we can discard them.
        _lastStorageAttrs.reset();

        LOG_DBG("Uploaded docKey ["
                << _docKey << "] to URI [" << _uploadRequest->uriAnonym()
                << "] and updated timestamps. Document modified timestamp: "
                << _storageManager.getLastModifiedServerTimeString()
                << ", newFileModifiedTime: " << _uploadRequest->newFileModifiedLocalTime()
                << ". Current Activity: " << DocumentState::name(_docState.activity()));

        // Handle activity-specific logic.
        switch (_docState.activity())
        {
            case DocumentState::Activity::Rename:
            {
                const auto it = _sessions.find(_renameSessionId);
                if (it == _sessions.end())
                {
                    LOG_ERR("Session [" << _renameSessionId << "] not found to rename docKey ["
                                        << _docKey << "]. The document will not be renamed.");
                    broadcastSaveResult(false, "Renaming session not found");
                    endRenameFileCommand();
                }
                else
                {
                    LOG_DBG("Renaming in storage as we just finished pending upload");
                    std::string uploadAsPath;
                    constexpr bool isRename = true;
                    constexpr bool isExport = false;
                    constexpr bool force = false;
                    uploadToStorageInternal(it->second, uploadAsPath, _renameFilename, isRename,
                                            isExport, force);
                }
            }
            break;

#if !MOBILEAPP && !WASMAPP
            case DocumentState::Activity::SwitchingToOffline:
            {
                switchToOffline();
            }
            break;
#endif // !MOBILEAPP && !WASMAPP

            default:
            {
                // Check stop conditions.
            }
            break;
        }

        // Resume polling.
        _poll->wakeup();
    }
    else if (_uploadRequest->isRename())
    {
        endRenameFileCommand();

        // encode the name
        const std::string& filename = uploadResult.getSaveAsName();
        auto uri = Poco::URI(uploadResult.getSaveAsUrl());

        // Remove the access_token, which belongs to the renaming user.
        Poco::URI::QueryParameters queryParams = uri.getQueryParameters();
        queryParams.erase(std::remove_if(queryParams.begin(), queryParams.end(),
                                         [](const std::pair<std::string, std::string>& pair)
                                         { return pair.first == "access_token"; }),
                          queryParams.end());
        uri.setQueryParameters(queryParams);

        const std::string url = uri.toString();
        std::string encodedName = Uri::encode(filename);
        const std::string filenameAnonym = COOLWSD::anonymizeUrl(filename);
        std::ostringstream oss;
        oss << "renamefile: " << "filename=" << encodedName << " url=" << url;
        broadcastMessage(oss.str());
        broadcastMessage("close: reloadafterrename");
    }
    else
    {
        // normalize the url (mainly to " " -> "%20")
        const std::string url = Poco::URI(uploadResult.getSaveAsUrl()).toString();

        const std::string& filename = uploadResult.getSaveAsName();

        // encode the name
        std::string encodedName;
        Poco::URI::encode(filename, "", encodedName);
        const std::string filenameAnonym = COOLWSD::anonymizeUrl(filename);

        const auto session = _uploadRequest->session();
        if (session)
        {
            LOG_DBG("Uploaded SaveAs docKey [" << _docKey << "] to URI ["
                                               << COOLWSD::anonymizeUrl(url) << "] with name ["
                                               << filenameAnonym << "] successfully.");

            std::ostringstream oss;
            oss << (_uploadRequest->isExport() ? "exportas:" : "saveas:") << " url=" << url
                << " filename=" << encodedName << " xfilename=" << filenameAnonym;
            session->sendTextFrame(oss.str());

            const auto fileExtension = _filename.substr(_filename.find_last_of('.'));
            if (!strcasecmp(fileExtension.c_str(), ".csv") ||
                !strcasecmp(fileExtension.c_str(), ".txt"))
            {
                broadcastMessageToOthers(
                    "warn: " + oss.str() + " username=" + session->getUserName(), session);
            }
        }
        else
        {
            LOG_DBG("Uploaded SaveAs docKey ["
                    << _docKey << "] to URI [" << COOLWSD::anonymizeUrl(url) << "] with name ["
                    << filenameAnonym << "] successfully, but the client session is closed.");
        }
    }

    broadcastLastModificationTime();

    if (_docState.isUnloadRequested())
    {
        // We just uploaded, flag to destroy if unload is requested.
        LOG_DBG("Unload requested after uploading, marking to destroy.");
        _docState.markToDestroy();
    }

    // If marked to destroy, and there are no late-arriving modifications, then stop.
    if ((_docState.isMarkedToDestroy() || _sessions.empty()) && !isPossiblyModified())
    {
        // Stop so we get cleaned up and removed.
        LOG_DBG("Stopping after uploading because "
                << (_sessions.empty() ? "there are no active sessions left."
                                      : "the document is marked to destroy."));
        stop("unloading");
    }

    // After uploading, disconnect the sessions pending disconnection.
    for (const auto& pair : _sessions)
    {
        if (pair.second->isCloseFrame() && !pair.second->inWaitDisconnected())
        {
            disconnectSessionInternal(pair.second);
        }
    }
}

void DocumentBroker::handleUploadToStorageResponse(const StorageBase::UploadResult& uploadResult)
{
    assert(_uploadRequest && "Expected to have a valid UploadRequest instance");

    // Storage upload is considered successful only when storage returns OK.
    const bool lastUploadSuccessful =
        uploadResult.getResult() == StorageBase::UploadResult::Result::OK;
    const bool previousUploadSuccessful = _storageManager.lastUploadSuccessful();
    LOG_TRC("lastUploadSuccessful: " << lastUploadSuccessful
                                     << ", previousUploadSuccessful: " << previousUploadSuccessful);
    _storageManager.setLastUploadResult(lastUploadSuccessful);

    UNITWSD_CALL_INSTANCE(_unitWsd, onDocumentUploaded(lastUploadSuccessful));

#if !MOBILEAPP
    if (lastUploadSuccessful && !isModified())
    {
        // Flag the document as uploaded in the admin console.
        // But only when isModified() == false because it might happen
        // by the time we finish uploading there is further modification
        // to the document.
        _admin.uploadedAlert(_docKey, getPid(), lastUploadSuccessful);
    }
    _admin.getModel().sendMigrateMsgAfterSave(lastUploadSuccessful, _docKey);
#endif

    if (uploadResult.getResult() == StorageBase::UploadResult::Result::OK)
    {
        return handleUploadToStorageSuccessful(uploadResult);
    }

    handleUploadToStorageFailed(uploadResult);
}

void DocumentBroker::reportUploadToStorageFailed()
{
    const auto session = _uploadRequest->session();
    if (session)
    {
        LOG_ERR("Failed to upload docKey [" << _docKey << "] to URI [" << _uploadRequest->uriAnonym()
                                            << "]. Notifying client.");
        const std::string msg = std::string("error: cmd=storage kind=")
                                + (_uploadRequest->isRename() ? "renamefailed" : "savefailed");
        session->sendTextFrame(msg);
    }
    else
    {
        LOG_ERR("Failed to upload docKey [" << _docKey << "] to URI [" << _uploadRequest->uriAnonym()
                                            << "]. The client session is closed.");
    }
}

void DocumentBroker::handleUploadToStorageFailed(const StorageBase::UploadResult& uploadResult)
{
    assert(uploadResult.getResult() != StorageBase::UploadResult::Result::OK &&
           "Expected upload failure");
    assert(_uploadRequest && "Expected to have a valid UploadRequest instance");

    if (_docState.activity() == DocumentState::Activity::Rename)
    {
        // Must end the renaming, as we've failed.
        LOG_DBG("Failed to renameFile because uploading pre-renaming failed");

        const auto it = _sessions.find(_renameSessionId);
        endRenameFileCommand();
        if (it == _sessions.end() || it->second == nullptr)
        {
            LOG_WRN("Session [" << _renameSessionId << "] not found to rename docKey [" << _docKey
                                << "]. The document will not be renamed.");
        }
        else
        {
            it->second->sendTextFrameAndLogError("error: cmd=renamefile kind=failed");
        }
    }

    if (uploadResult.getResult() == StorageBase::UploadResult::Result::TOO_LARGE)
    {
        LOG_WRN(
            "Got Entitity Too Large while uploading docKey ["
            << _docKey << "] to URI [" << _uploadRequest->uriAnonym() << "] of "
            << _storageManager.getSizeAsUploaded()
            << " bytes. If a reverse-proxy is used, it might be misconfigured. Alternatively, the "
               "WOPI host might be low on disk or hitting a quota limit. Making all sessions "
               "on doc read-only and notifying clients.");

        // Make everyone readonly and tell everyone that the file is too large for the storage.
        for (const auto& sessionIt : _sessions)
        {
            sessionIt.second->setWritable(false);
            sessionIt.second->sendTextFrameAndLogError("error: cmd=storage kind=savetoolarge");
        }

        broadcastSaveResult(false, "Too large", uploadResult.getReason());
    }
    else if (uploadResult.getResult() == StorageBase::UploadResult::Result::DISKFULL)
    {
        LOG_WRN("Disk full while uploading docKey ["
                << _docKey << "] to URI [" << _uploadRequest->uriAnonym() << "] of "
                << _storageManager.getSizeAsUploaded()
                << " bytes. Making all sessions on doc read-only and notifying clients.");

        // Make everyone readonly and tell everyone that storage is low on diskspace.
        for (const auto& sessionIt : _sessions)
        {
            sessionIt.second->setWritable(false);
            sessionIt.second->sendTextFrameAndLogError("error: cmd=storage kind=savediskfull");
        }

        broadcastSaveResult(false, "Disk full", uploadResult.getReason());
    }
    else if (uploadResult.getResult() == StorageBase::UploadResult::Result::UNAUTHORIZED)
    {
        LOG_DBG("Last upload result: UNAUTHORIZED");
        const auto session = _uploadRequest->session();
        if (session)
        {
            LOG_ERR(
                "Cannot upload docKey ["
                << _docKey << "] to storage URI [" << _uploadRequest->uriAnonym() << "] of "
                << _storageManager.getSizeAsUploaded()
                << " bytes. Invalid or expired access token. Notifying client and invalidating the "
                   "authorization token of session ["
                << session->getId() << ']');
            session->sendTextFrameAndLogError("error: cmd=storage kind=saveunauthorized");
            session->invalidateAuthorizationToken();
        }
        else
        {
            LOG_ERR("Cannot upload docKey ["
                    << _docKey << "] to storage URI [" << _uploadRequest->uriAnonym() << "] of "
                    << _storageManager.getSizeAsUploaded()
                    << " bytes. Invalid or expired access token. The client session is closed.");
        }

        broadcastSaveResult(false, "Invalid or expired access token");
    }
    else if (uploadResult.getResult() == StorageBase::UploadResult::Result::FAILED)
    {
        // Likely timed out.
        LOG_INF("Failed to upload docKey [" << _docKey << "] to storage URI ["
                                            << _uploadRequest->uriAnonym() << "] of "
                                            << _storageManager.getSizeAsUploaded()
                                            << " bytes with reason: " << uploadResult.getReason());

        // Since we've failed to get a response, we cannot know if the
        // Storage has been updated. As such, we need to re-sync the
        // document's last modified timestamp.
        endActivity(); // Probably in Activity::Upload.
        startActivity(DocumentState::Activity::SyncFileTimestamp);

        reportUploadToStorageFailed();

        // Notify all.
        broadcastSaveResult(false, "Save failed", uploadResult.getReason());
    }
    else if (uploadResult.getResult() == StorageBase::UploadResult::Result::DOC_CHANGED
             || uploadResult.getResult() == StorageBase::UploadResult::Result::CONFLICT)
    {
        LOG_ERR("PutFile failed for docKey ["
                << _docKey << "] to storage URI [" << _uploadRequest->uriAnonym() << "] of "
                << _storageManager.getSizeAsUploaded() << " bytes because it's changed in storage");
        broadcastSaveResult(false, "Conflict: Document changed in storage",
                            uploadResult.getReason());
        handleDocumentConflict();
    }

    // We failed to upload, merge the last attributes into the current one.
    _currentStorageAttrs.merge(_lastStorageAttrs);
    _lastStorageAttrs.reset();
}

void DocumentBroker::handleDocumentConflict()
{
    _documentChangedInStorage = true;

    // Do not reload the document ("close: documentconflict") if there are
    // any changes in the loaded document, either saved or unsaved.
    const std::string message = (_lastStorageAttrs.isUserModified() ||
                                 _currentStorageAttrs.isUserModified() || isPossiblyModified())
                                    ? "error: cmd=storage kind=documentconflict"
                                    : "close: documentconflict";

    const std::size_t activeClients = broadcastMessage(message);
    LOG_TRC("There are " << activeClients << " active clients after broadcasting documentconflict");
    if (activeClients == 0)
    {
        // No clients were contacted; we will never resolve this conflict.
        LOG_WRN(
            "The document [" << _docKey
                             << "] could not be uploaded to storage because there is a newer "
                                "version there, and no active clients exist to resolve the conflict"
#if !MOBILEAPP
                             << (_storage && _quarantine && _quarantine->isEnabled()
                                     ? ". The document should be recoverable from the quarantine. "
                                     : ", but Quarantine is disabled. ")
#else
                             << ". "
#endif // !MOBILEAPP
                             << "Stopping.");

        // Nothing more to do.
        stop("conflict");
    }
}

void DocumentBroker::broadcastSaveResult(bool success, const std::string_view result,
                                         const std::string& errorMsg) const
{
    const std::string_view resultstr = success ? "true" : "false";
    // Some sane limit, otherwise we get problems transferring this to the client with large strings (can be a whole webpage)
    std::string errorMsgFormatted = COOLProtocol::getAbbreviatedMessage(errorMsg);
    std::ostringstream oss;
    oss << "commandresult: { \"command\": \"save\", \"success\": " << resultstr
        << ", \"result\": \"" << result << "\", \"errorMsg\": \""
        << Util::replaceInPlace(errorMsgFormatted, '"', '\'') // Replace reserved characters
        << "\"}";
    broadcastMessage(oss.str());
}

void DocumentBroker::setLoaded()
{
    if (!isLoaded())
    {
        _docState.setLive();
        _loadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - _createTime);
        const auto minTimeoutSecs = ((_loadDuration * 4).count() + 500) / 1000;
        _saveManager.setSavingTimeout(
            std::max(std::chrono::seconds(minTimeoutSecs), std::chrono::seconds(5)));
        LOG_INF("Document [" << _docKey << "] loaded in " << _loadDuration
                             << ", saving-timeout set to " << _saveManager.getSavingTimeout());
        LOG_DBG("Document [" << _docKey
                             << "] PSS: " << ProcUtil::getMemoryUsagePSS(_childProcess->getPid())
                             << " KB, total PSS: "
                             << ProcUtil::getProcessTreePss(ProcUtil::getProcessId()) << " KB");

        UNITWSD_CALL_INSTANCE(_unitWsd, onPerfDocumentLoaded());
    }
}

void DocumentBroker::setInteractive(bool value)
{
    if (isInteractive() != value)
    {
        _docState.setInteractive(value);
        LOG_TRC("Document has interactive dialogs before load");
    }
}

void DocumentBroker::onViewLoaded(const std::shared_ptr<ClientSession>& session)
{
    // Only lock the document on storage for editing sessions.
    lockIfEditing(session);

    // A view loaded.
    if (UnitWSD::isUnitTesting())
    {
        UNITWSD_CALL(onDocBrokerViewLoaded(getDocKey(), session));
    }
}

std::shared_ptr<ClientSession> DocumentBroker::getFirstAuthorizedSession() const
{
    ASSERT_CORRECT_THREAD();

    for (const auto& sessionIt : _sessions)
    {
        const auto& session = sessionIt.second;
        if (!session->getAuthorization().isExpired())
        {
            return session;
        }
    }

    return std::shared_ptr<ClientSession>();
}

std::shared_ptr<ClientSession> DocumentBroker::getWriteableSession() const
{
    ASSERT_CORRECT_THREAD();

    std::shared_ptr<ClientSession> savingSession;
    for (const auto& sessionIt : _sessions)
    {
        const auto& session = sessionIt.second;

        // Save the document using a session that is loaded, editable, and
        // with a valid authorization token, or the first.
        // Note that isViewLoaded() precludes inWaitDisconnected().
        if (!savingSession || (session->isViewLoaded() && session->isEditable() &&
                               !session->getAuthorization().isExpired()))
        {
            savingSession = session;
        }

        // or if any of the sessions is document owner, use that.
        //FIXME: can the owner be read-only?
        if (session->isDocumentOwner())
        {
            savingSession = session;
            break;
        }
    }

    return savingSession;
}

void DocumentBroker::refreshLock()
{
    ASSERT_CORRECT_THREAD();

    const std::shared_ptr<ClientSession> session = getWriteableSession();
    if (!session)
    {
        LOG_ERR("No write-able session to refresh lock with");
        _lockCtx->bumpTimer();
    }
    else if (session->getAuthorization().isExpired())
    {
        LOG_ERR("No write-able session with valid authorization to refresh lock with");
        _lockCtx->bumpTimer();
    }
    else
    {
        const std::string savingSessionId = session->getId();
        LOG_TRC("Refresh lock " << _lockCtx->lockToken() << " with session [" << savingSessionId
                                << ']');
        std::string error;
        if (!updateStorageLockStateAsync(session, StorageBase::LockState::LOCK, error))
        {
            LOG_ERR("Failed to refresh lock of docKey [" << _docKey << "] with session ["
                                                         << savingSessionId << "]: " << error);
        }
    }
}

DocumentBroker::NeedToSave DocumentBroker::needToSaveToDisk() const
{
    if (!_saveManager.lastSaveSuccessful())
    {
        // When saving is attempted and fails, we have no file on disk.
        return NeedToSave::Yes_LastSaveFailed;
    }

    if (isModified())
    {
        // ViewFileExtensions do not update the ModifiedStatus, but,
        // we expect a successful save anyway (including unmodified).
        if (!_isViewFileExtension)
        {
            return NeedToSave::Yes_Modified;
        }

        assert(_isViewFileExtension && "Not a view-file");
        // Fallback to check for activity post-saving.
    }

    assert(_saveManager.lastSaveSuccessful() && "Last save failed");

    if (haveModifyActivityAfterSaveRequest())
    {
        return NeedToSave::Maybe;
    }

    return NeedToSave::No;
}

bool DocumentBroker::manualSave(const std::shared_ptr<ClientSession>& session,
                                bool dontTerminateEdit, bool dontSaveIfUnmodified,
                                const std::string& extendedData)
{
    // If we aren't saving already.
    if (_docState.activity() != DocumentState::Activity::Save)
    {
        LOG_DBG("Manual save by " << session->getName() << " on docKey [" << _docKey << ']');
        return sendUnoSave(session, dontTerminateEdit, dontSaveIfUnmodified,
                           /*isAutosave=*/false, /*finalWrite=*/false, extendedData);
    }

    LOG_DBG("Document [" << _docKey << "] is currently saving and cannot issue another save");
    return false;
}

bool DocumentBroker::autoSave(const bool force, const bool dontSaveIfUnmodified,
                              const bool finalWrite)
{
    ASSERT_CORRECT_THREAD();

    // If we aren't saving already.
    if (_docState.activity() == DocumentState::Activity::Save)
    {
        LOG_DBG("Document [" << _docKey
                             << "] is currently saving and cannot issue another save for autosave");
        return true; // We are saving, wait for the results.
    }

    _saveManager.autoSaveChecked();

    LOG_TRC("autoSave(): forceful? " << force <<
            ", dontSaveIfUnmodified: " << dontSaveIfUnmodified <<
            " finalWrite : " << finalWrite);

    const CanSave canSave = canSaveToDisk();
    if (canSave != CanSave::Yes)
    {
        LOG_DBG("Cannot save to disk: " << name(canSave));
        return false;
    }

    if (!isModified() && !force)
    {
        // Nothing to do.
        LOG_TRC("Nothing to autosave [" << _docKey << ']');
        return false;
    }

    // Which session to use when auto saving ?
    // Prefer the last editing view, if still valid, otherwise, find the first writable sessionId.
    // Note: a loaded view cannot be disconnecting.
    const auto itLastEditingSession = _sessions.find(_lastEditingSessionId);
    const std::shared_ptr<ClientSession> savingSession =
        (itLastEditingSession != _sessions.end() && itLastEditingSession->second->isEditable() &&
         itLastEditingSession->second->isViewLoaded())
            ? itLastEditingSession->second
            : getWriteableSession();

    if (!savingSession)
    {
        LOG_ERR("No session to use for saving");
        return false;
    }

    const std::string savingSessionId = savingSession->getId();

    // Remember the last save time, since this is the predicate.
    LOG_TRC("Checking to autosave [" << _docKey << "] using session [" << savingSessionId << ']');

    bool sent = false;
    if (force)
    {
        LOG_TRC("Sending forced save command for [" << _docKey << ']');
        // Don't terminate editing as this can be invoked by the admin OOM, but otherwise force saving anyway.
        // Flag isAutosave=false so the WOPI host wouldn't think this is a regular checkpoint and
        // potentially optimize it away. This is as good as user-issued save, since this is
        // triggered when the document is closed. In the case of network disconnection or browser crash
        // most users would want to have had the chance to hit save before the document unloaded.
        sent = sendUnoSave(savingSession, /*dontTerminateEdit=*/true, dontSaveIfUnmodified,
                           /*isAutosave=*/false, finalWrite);
    }
    else if (isModified())
    {
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        const std::chrono::milliseconds inactivityTime
            = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastActivityTime);
        const auto timeSinceLastSave = std::min(_saveManager.timeSinceLastSaveRequest(),
                                                _storageManager.timeSinceLastUploadResponse());
        LOG_TRC("DocKey [" << _docKey << "] is modified. It has been " << timeSinceLastSave
                           << " since last save and the most recent activity was " << inactivityTime
                           << " ago. Idle save is "
                           << (_saveManager.isIdleSaveEnabled() ? "" : "not ")
                           << "enabled, auto save is "
                           << (_saveManager.isAutoSaveEnabled() ? "" : "not ")
                           << "enabled with interval of " << _saveManager.autoSaveInterval());

        // Either we've been idle long enough, or it's auto-save time.
        bool save = _saveManager.isIdleSaveEnabled() &&
                    inactivityTime >= _saveManager.idleSaveInterval() &&
                    timeSinceLastSave >= _saveManager.idleSaveInterval();

        // Save if it's been long enough since the last save and/or upload.
        if (!save && _saveManager.isAutoSaveEnabled() &&
            timeSinceLastSave >= _saveManager.autoSaveInterval())
        {
            save = true;
        }

        if (save)
        {
            LOG_TRC("Sending timed save command for [" << _docKey << ']');
            sent = sendUnoSave(savingSession, /*dontTerminateEdit=*/true,
                               /*dontSaveIfUnmodified=*/true, /*isAutosave=*/true,
                               finalWrite);
        }
    }

    return sent;
}

void DocumentBroker::autoSaveAndStop(const std::string_view reason)
{
    if (_saveManager.isSaving() || isAsyncUploading())
    {
        LOG_TRC("autoSaveAndStop ["
                << reason << "] skipped for docKey [" << getDocKey() << "] because an async "
                << (_saveManager.isSaving() ? "saving" : "uploading") << " is in progress");
        return;
    }

    const NeedToSave needToSave = needToSaveToDisk();
    const NeedToUpload needToUpload = needToUploadToStorage();
    bool canStop = (needToSave == NeedToSave::No && needToUpload == NeedToUpload::No);
    LOG_TRC("autoSaveAndStop [" << reason << "] for docKey [" << getDocKey()
                                << "]: " << name(needToSave) << ", " << name(needToUpload)
                                << ", canStop: " << canStop);

    if (!canStop && needToSave == NeedToSave::No && !isStorageOutdated())
    {
        if (_alwaysSaveOnExit && !_storageManager.lastUploadSuccessful())
        {
            CONFIG_STATIC const auto limStoreFailures =
                ConfigUtil::getConfigValue<int>("per_document.limit_store_failures", 5);

            if (limStoreFailures > 0 &&
                _storageManager.uploadFailureCount() >= static_cast<std::size_t>(limStoreFailures))
            {
                LOG_TRC("Uploads for always-save-on-exit are failing. Will stop");
                canStop = true;
            }
            else
            {
                LOG_TRC("Always-save-on-exit is set but last upload failed. Cannot stop.");
                canStop = false;
            }
        }
        else
        {
            LOG_TRC("autoSaveAndStop for docKey ["
                    << getDocKey()
                    << "] has nothing to save and Storage is up-to-date, canStop: true");
            canStop = true;
        }
    }

    if (!canStop && needToSave != NeedToSave::No)
    {
        // Check that we *can* save, now that we know we need to.
        const CanSave canSave = canSaveToDisk();
        if (canSave != CanSave::Yes)
        {
            LOG_ERR("Cannot save because " << name(canSave) << " though " << name(needToSave)
                                           << ". May have data loss, but must stop");
            canStop = true;
        }
    }

    if (!canStop && needToUpload == NeedToUpload::No)
    {
        // Here we don't check for the modified flag because it can come in
        // very late, or not at all. We care that there is nothing to upload
        // and the last save succeeded, possibly because there was no
        // modifications, and there has been no activity since.
        LOG_ASSERT_MSG(_saveManager.lastSaveRequestTime() <= _saveManager.lastSaveResponseTime(),
                       "Unexpected active save in flight");
        LOG_ASSERT_MSG(!_saveManager.isSaving(), "Unexpected active save in flight");
        if (!haveModifyActivityAfterSaveRequest() && _saveManager.lastSaveSuccessful())
        {
            // We can stop, but the modified flag is set. Delayed ModifiedStatus?
            if (isModified())
            {
                if (_saveManager.timeSinceLastSaveResponse() < std::chrono::seconds(2))
                {
                    LOG_INF("Can stop " << reason << " DocumentBroker for docKey [" << getDocKey()
                                        << "] but will wait for isModified to clear.");
                    return;
                }

                LOG_WRN("Will stop " << reason << " DocumentBroker for docKey [" << getDocKey()
                                     << "] even with isModified, which is not clearing.");
            }

            // Nothing to upload and last save was successful; stop.
            canStop = true;
            LOG_TRC("autoSaveAndStop for docKey ["
                    << getDocKey() << "]: no modifications since last successful save. Stopping.");
        }
        else if (needToSave == NeedToSave::No)
        {
            // Nothing to upload and no modifications; stop.
            LOG_ASSERT_MSG(!isPossiblyModified(), "Unexpected isPossiblyModified with NeedToSave::No");
            canStop = true;
            LOG_TRC("autoSaveAndStop for docKey [" << getDocKey() << "]: not modified. Stopping.");
        }
    }

    // Don't hammer on saving.
    if (!canStop && _saveManager.canSaveNow(isUnloading()))
    {
        // Stop if there is nothing to save.
        const bool possiblyModified = isPossiblyModified();
        const bool lastSaveSuccessful = _saveManager.lastSaveSuccessful();
        LOG_INF("Autosaving " << reason << " DocumentBroker for docKey [" << getDocKey()
                              << "] before terminating. isPossiblyModified: "
                              << (possiblyModified ? "yes" : "no")
                              << ", lastSaveSuccessful: " << (lastSaveSuccessful ? "yes" : "no")
                              << ", conflict: " << (_documentChangedInStorage ? "yes" : "no"));
        if (!autoSave(/*force=*/possiblyModified || !lastSaveSuccessful,
                      /*dontSaveIfUnmodified=*/true, /*finalWrite=*/true))
        {
            // Nothing to save. Try to upload if necessary.
            const auto session = getWriteableSession();
            if (session && !session->getAuthorization().isExpired())
            {
                checkAndUploadToStorage(session, /*justSaved=*/false);
                if (isAsyncUploading())
                {
                    LOG_DBG("Uploading document before stopping.");
                    return;
                }
            }
            else
            {
                // There is nothing to do here except to detect data-loss and stop.
                if (isStorageOutdated())
                {
                    LOG_WRN("The document ["
                            << _docKey
                            << "] could not be uploaded to storage because there are no writable "
                               "sessions, or no authorization tokens, to upload. The document "
                               "should be recoverable from the quarantine. Stopping.");
                }

                canStop = true;
            }
        }
    }
    else if (!canStop)
    {
        LOG_TRC("Too soon to issue another save on ["
                << getDocKey() << "], need at least " << _saveManager.timeToNextSave(isUnloading())
                << ": " << _saveManager.timeSinceLastSaveRequest() << " since last save request, "
                << _saveManager.timeSinceLastSaveResponse()
                << " since last save response, and last save took "
                << _saveManager.lastSaveDuration()
                << ". Min time between saves: " << _saveManager.minTimeBetweenSaves());
    }

    if (canStop)
    {
        // Nothing to save, nothing to upload, and no modifications. Stop.
        LOG_INF("Nothing to save or upload. Terminating "
                << reason << " DocumentBroker for docKey [" << getDocKey() << ']');
        stop(reason);
    }
}

bool DocumentBroker::sendUnoSave(const std::shared_ptr<ClientSession>& session,
                                 bool dontTerminateEdit, bool dontSaveIfUnmodified,
                                 bool isAutosave, bool finalWrite,
                                 const std::string& extendedData)
{
    ASSERT_CORRECT_THREAD();

    LOG_ASSERT_MSG(session, "Got null ClientSession");
    const std::string sessionId = session->getId();

    LOG_INF("Saving doc [" << _docKey << "] using session [" << sessionId << ']');

    // Invalidate the timestamp to force persisting.
    _saveManager.setLastModifiedLocalTime(std::chrono::system_clock::time_point());

    static const bool forceBackgroundEnv = !!getenv("COOL_FORCE_BGSAVE");
    constexpr std::size_t MaxFailureCountForBackgroundSaving = 2; // Give only 1 extra chance.

    // Note: It's odd to capture these here, but this function is used from ClientSession too.
    const bool autosave = isAutosave || UNITWSD_CALL_INSTANCE(_unitWsd, isAutosave());
    const bool backgroundConfigured = (autosave && _backgroundAutoSave) || _backgroundManualSave;
    const bool canBackground = forceBackgroundEnv || (!finalWrite && backgroundConfigured);
    const bool background = canBackground && _saveManager.lastSaveSuccessful() &&
                            _saveManager.saveFailureCount() < MaxFailureCountForBackgroundSaving;

    std::ostringstream oss;
    // arguments init
    oss << '{';

    // We do not want save to terminate editing mode if we are in edit mode now.
    // We want to terminate if forced by the user, otherwise autosave doesn't terminate.
    // Note: We always force terminating edit when saving in the background.
    dontTerminateEdit = dontTerminateEdit && background == false;

    oss << "\"DontTerminateEdit\" : { \"type\":\"boolean\", \"value\":"
        << (dontTerminateEdit ? "true" : "false") << " }";

    if (dontSaveIfUnmodified)
    {
        oss << ",\"DontSaveIfUnmodified\" : { \"type\":\"boolean\", \"value\":true }";
    }

    // arguments end
    oss << '}';

    // At this point, if we have any potential modifications, we need to capture the fact.
    // If Core does report something different after saving, we'll update this flag.
    _nextStorageAttrs.setUserModified(isModified() || haveModifyActivityAfterSaveRequest());

    if (finalWrite)
        LOG_TRC("suspected final save: don't do background write");

    _nextStorageAttrs.setIsAutosave(autosave);
    _nextStorageAttrs.setExtendedData(extendedData);

    const std::string saveArgs = oss.str();

    // re-written to .uno:Save in the Kit.
    const auto command = std::string("save background=") + (background ? "true " : " ") + saveArgs;
    if (forwardToChild(session, command))
    {
        LOG_DBG("Saving [" << _docKey << "] using [" << sessionId << "]: " << command);

        _saveManager.markLastSaveRequestTime();
        if (_docState.activity() == DocumentState::Activity::None)
        {
            // If we aren't in the midst of any particular activity,
            // then this is a generic save on its own.
            startActivity(DocumentState::Activity::Save);
        }

        return true;
    }

    LOG_ERR("Failed to save doc ["
            << _docKey << "]: Failed to forward .uno:Save command to session [" << sessionId
            << ']');
    return false;
}

std::string DocumentBroker::getJailRoot() const
{
    if constexpr (!Util::isMobileApp())
    {
        if (!_jailId.empty())
        {
            return Poco::Path(COOLWSD::ChildRoot, _jailId).toString();
        }

        LOG_WRN("Trying to get the jail root of a not yet downloaded document (no jailId)");
    }

    return std::string();
}

std::size_t DocumentBroker::addSession(const std::shared_ptr<ClientSession>& session,
                                       std::unique_ptr<WopiStorage::WOPIFileInfo> wopiFileInfo)
{
    ASSERT_CORRECT_THREAD();

    const std::string id = session->getId();
    LOG_TRC("Adding " << (session->isReadOnly() ? "readonly" : "non-readonly") << " session [" << id
                      << "] to docKey [" << _docKey << ']');

    try
    {
        // First, download the document, since this can fail.
        if (!download(session, _childProcess->getJailId(), session->getPublicUri(),
                      session->getAdditionalFilePublicUri(),
                      std::move(wopiFileInfo)))
        {
            const auto msg = "Failed to load document with URI [" + session->getPublicUri().toString() + "].";
            LOG_ERR(msg);
            throw std::runtime_error(msg);
        }

        // Request a new session from the child kit.
        const std::string message = "session " + id + ' ' + _docKey + ' ' + _docId;
        _childProcess->sendTextFrame(message);

#if !MOBILEAPP
        // Tell the admin console about this new doc
        const Poco::URI& uri = _storage->getUri();
        // Create uri without query parameters
        const std::string wopiSrc(uri.getScheme() + "://" + uri.getAuthority() + uri.getPath());
        _admin.addDoc(_docKey, getPid(), getFilename(), id, session->getUserName(),
                      session->getUserId(), _childProcess->getSMapsFp(), wopiSrc, session->isReadOnly());
        _admin.setDocWopiDownloadDuration(_docKey, _wopiDownloadDuration);
#endif

        // Add and attach the session.
        _sessions.emplace(session->getId(), session);
        session->setState(ClientSession::SessionState::LOADING);

        const std::size_t count = _sessions.size();
        LOG_TRC("Added " << (session->isReadOnly() ? "readonly" : "non-readonly") <<
                " session [" << id << "] to docKey [" <<
                _docKey << "] to have " << count << " sessions.");

        UNITWSD_CALL_INSTANCE(_unitWsd, onDocBrokerAddSession(_docKey, session));

        return count;
    }
    catch (const StorageSpaceLowException&)
    {
        LOG_ERR("Out of storage while loading document with URI ["
                << session->getPublicUri().toString() << ']');

        // We use the same message as is sent when some of cool's own locations are full,
        // even if in this case it might be a totally different location (file system, or
        // some other type of storage somewhere). This message is not sent to all clients,
        // though, just to all sessions of this document.
        alertAllUsers("internal", "diskfull");
        throw;
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to add session to [" << _docKey << "] with URI [" << COOLWSD::anonymizeUrl(session->getPublicUri().toString()) << "]: " << exc.what());
        if (_sessions.empty())
        {
            LOG_INF("Doc [" << _docKey << "] has no more sessions. Marking to destroy.");
            _docState.markToDestroy();
        }
        throw;
    }
}

std::size_t DocumentBroker::removeSession(const std::shared_ptr<ClientSession>& session)
{
    ASSERT_CORRECT_THREAD();

    LOG_ASSERT_MSG(session, "Got null ClientSession");
    const std::string id = session->getId();
    try
    {
        const std::size_t activeSessionCount = countActiveSessions();

        const bool lastEditableSession = session->isEditable() && !haveAnotherEditableSession(id);
        // Forcing a save when always_save_on_exit=true creates a new
        // file on disk, with a new timestamp, which makes it hard to
        // avoid uploading when there really isn't any modifications.
        // Instead, we rely on always issuing a save through forced
        // auto-save and expect Core has the correct modified flag.
        constexpr bool dontSaveIfUnmodified = true;

        LOG_INF("Removing session [" << id << "] on docKey [" << _docKey << "]. Have "
                                     << _sessions.size() << " sessions (" << activeSessionCount
                                     << " active). IsLive: " << session->isLive()
                                     << ", IsReadOnly: " << session->isReadOnly()
                                     << ", IsAllowChangeComments: " << session->isAllowChangeComments()
                                     << ", IsAllowManageRedlines: " << session->isAllowManageRedlines()
                                     << ", IsEditable: " << session->isEditable()
                                     << ", Unloading: " << _docState.isUnloadRequested()
                                     << ", MarkToDestroy: " << _docState.isMarkedToDestroy()
                                     << ", LastEditableSession: " << lastEditableSession
                                     << ", DontSaveIfUnmodified: " << dontSaveIfUnmodified
                                     << ", IsPossiblyModified: " << isPossiblyModified());

#if !MOBILEAPP
        try
        {
            /// make sure to upload preset to WOPIHost
            uploadPresetsToWopiHost();
        }
        catch (const std::exception& exc)
        {
            LOG_WRN("Failed to upload presets for session [" << id << "]: " << exc.what());
        }
        if (activeSessionCount <= 1 && !isConvertTo())
        {
            // rescue clipboard before shutdown.
            // N.B. If the user selects then copies, most likely we will
            // mark the document as possibly-modified. This will issue
            // a save (below) before removing the session, guaranteeing
            // that we wait for the save to complete, which is after
            // rescuing the clipboard via getclipboard. Conversely,
            // if there is no reason to think the document is possibly-
            // modified, then it's unlikely there is anything in the clipboard.
            LOG_TRC("request/rescue clipboard on disconnect for " << session->getId());
            forwardToChild(session, "getclipboard name=shutdown");
        }
#endif

        // In theory, we almost could do this here:

        // #if MOBILEAPP
        // There is always just one "session" in a mobile app, and the same one process continues
        // running, so no need to delay the disconnectSessionInternal() call. Doing it like this
        // will also get rid of the docbroker and lokit_main thread for the document quicker.

        // But, in reality it has unintended side effects on iOS because if you have done changes to
        // the document, it does get saved, but that is only to the temporary copy. It is only in
        // the document callback handler for KIT_CALLBACK_UNO_COMMAND_RESULT that we then call the
        // system API to save that copy back to where it came from. See the
        // KIT_CALLBACK_UNO_COMMAND_RESULT case in ChildSession::loKitCallback() in
        // ChildSession.cpp. If we did use the below code snippet here, the document callback would
        // get unregistered right away in Document::onUnload in Kit.cpp.

        // autoSave(isPossiblyModified(), dontSaveIfUnmodified);
        // disconnectSessionInternal(id);
        // stop("stopped");

        // So just go down the same code path as for normal Online:

        // If last editable, save (if not saving already) and
        // don't remove until after uploading to storage.
        // If always_save_on_exit=true, issue a save to guarantee uploading if necessary.
        if (!lastEditableSession ||
            (!_saveManager.isSaving() &&
             !autoSave(/*force=*/_alwaysSaveOnExit || isPossiblyModified(),
                       dontSaveIfUnmodified, /*finalWrite=*/true)))
        {
            disconnectSessionInternal(session);
        }

        // Last view going away; can destroy?
        if (activeSessionCount <= 1)
        {
            if (_saveManager.isSaving() || isAsyncUploading())
            {
                // Don't destroy just yet, wait until save and upload are done.
                // Notice that the save and/or upload could have been triggered
                // earlier, and not necessarily here when removing this last session.
                _docState.setUnloadRequested();
                LOG_DBG("Removing last session and will unload after saving and uploading. Setting "
                        "UnloadRequested flag.");
            }
            else if (_sessions.empty())
            {
                // Nothing to save, and we were the last.
                _docState.markToDestroy();
                LOG_DBG("No more sessions after removing last. Setting MarkToDestroy flag.");
            }
        }
        else if (activeSessionCount > 0)
        {
            LOG_ASSERT_MSG(!_docState.isMarkedToDestroy(),
                           "Have active sessions while marked to destroy");
        }
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Error while removing session [" << id << "]: " << ex.what());
    }

    return _sessions.size();
}

void DocumentBroker::disconnectSessionInternal(const std::shared_ptr<ClientSession>& session)
{
    ASSERT_CORRECT_THREAD();

    LOG_ASSERT_MSG(session, "Got null ClientSession");
    const std::string id = session->getId();
    try
    {
#if !MOBILEAPP
        _admin.rmDoc(_docKey, id);
        COOLWSD::dumpEndSessionTrace(getJailId(), id, _uriOrig);
#endif
        if (_docState.isUnloadRequested())
        {
            // We must be the last session, flag to destroy if unload is requested.
            LOG_ASSERT_MSG(countActiveSessions() <= 1, "Unload-requested with multiple sessions");
            LOG_TRC("Unload requested while disconnecting session ["
                    << id << "], having " << _sessions.size() << " sessions, marking to destroy");
            _docState.markToDestroy();
        }

        const bool lastEditableSession = session->isEditable() && !haveAnotherEditableSession(id);

        LOG_TRC("Disconnect session internal "
                << id << ", LastEditableSession: " << lastEditableSession << " destroy? "
                << _docState.isMarkedToDestroy() << " locked? " << _lockCtx->isLocked() << ", have "
                << _sessions.size() << " sessions (inclusive)");

        // Unlock the document, if last editable sessions, before we lose a token that can unlock.
        std::string error;
        if (lastEditableSession && _lockCtx->isLocked() && _storage &&
            !updateStorageLockState(*session, StorageBase::LockState::UNLOCK, error))
        {
            LOG_ERR("Failed to unlock docKey [" << _docKey
                                                << "] before disconnecting last editable session ["
                                                << session->getName() << "]: " << error);
        }

        bool hardDisconnect;
        if (session->inWaitDisconnected())
        {
            LOG_TRC("Removing session [" << id << "] while waiting for disconnected handshake");
            hardDisconnect = true;
        }
        else
        {
            // Disconnect before potentially failing and removing (won't send to removed sessions).
            LOG_DBG("Disconnecting session [" << id << "] from Kit");
            hardDisconnect = session->disconnectFromKit();

            // Clean-up and close loading sessions, if necessary.
            const std::size_t loadingSessions = countLoadingSessions();
            if (_sessions.size() == loadingSessions + 1)
            {
                // This session is the last loaded one.
                // If we remove it, the loading one(s) will never load.
                // Instead, fail the loading ones and remove them.
                failLoadingSessions(/*remove=*/true);
            }

            if (!Util::isMobileApp() && !isLoaded() &&
                _sessions.size() <= 1) // We remove the session below, so we still have it here.
            {
                // We aren't even loaded and no other views--kill.
                // If we send disconnect, we risk hanging because we flag Core for
                // quiting via unipoll, but Core would still continue loading.
                // If at the end of loading it shows a dialog (such as the macro or
                // csv import dialogs), it will wait for their dismissal indefinitely.
                // Neither would our load-timeout kick in, since we would be gone.
                LOG_INF("Session [" << session->getName() << "] disconnected but DocKey ["
                                    << _docKey
                                    << "] isn't loaded yet. Terminating the child roughly");
                if (_childProcess)
                    _childProcess->terminate();

                stop("Disconnected before loading");
            }
        }

        if (hardDisconnect)
            finalRemoveSession(session);
        // else wait for disconnected.
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Error while disconnecting session [" << id << "]: " << ex.what());
    }
}

void DocumentBroker::finalRemoveSession(const std::shared_ptr<ClientSession>& session)
{
    ASSERT_CORRECT_THREAD();

    LOG_ASSERT_MSG(session, "Got null ClientSession");
    const std::string sessionId = session->getId();
    try
    {
        // Notify test code before removal.
        UNITWSD_CALL_INSTANCE(_unitWsd, onDocBrokerRemoveSession(_docKey, session));

        const bool readonly = session->isReadOnly();
        session->dispose();

        // Remove. The caller must have a reference to the session
        // in question, lest we destroy from underneath them.
        _sessions.erase(sessionId);

        LOG_TRC("Removed " << (readonly ? "" : "non-") << "readonly session [" << sessionId
                           << "] from docKey [" << _docKey << "] to have " << _sessions.size()
                           << " session(s): " <<
                [&](auto& log)
                {
                    for (const auto& pair : _sessions)
                        log << pair.second->getId() << ' ';
                });
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Error while removing session [" << sessionId << "]: " << ex.what());
    }
}

std::shared_ptr<ClientSession> DocumentBroker::createNewClientSession(
    const std::shared_ptr<ProtocolHandlerInterface> &ws,
    const std::string& id,
    const Poco::URI& uriPublic,
    const bool isReadOnly,
    const RequestDetails &requestDetails)
{
    ASSERT_CORRECT_THREAD();

    LOG_TRC("Creating new client session " << (isReadOnly ? "readonly" : "non-readonly")
                                           << " session [" << id << "] to docKey ["
                                           << requestDetails.getDocKey() << ']');

    try
    {
        if (isMarkedToDestroy() || _docState.isCloseRequested())
        {
            LOG_WRN("DocBroker [" << getDocKey()
                                  << "] is unloading. Rejecting client request to load session ["
                                  << id << ']');
            if (ws)
            {
                constexpr std::string_view msg("error: cmd=load kind=docunloading");
                ws->sendTextMessage(msg);
                ws->shutdown(true, msg);
            }

            return nullptr;
        }

        // Now we have a DocumentBroker and we're ready to process client commands.
        if (ws)
        {
            static constexpr std::string_view statusReady = "progress: { \"id\":\"ready\" }";
            LOG_TRC("Sending to Client [" << statusReady << ']');
            ws->sendTextMessage(statusReady);
        }

        // In case of WOPI, if this session is not set as readonly, it might be set so
        // later after making a call to WOPI host which tells us the permission on files
        // (UserCanWrite param).
        auto session = std::make_shared<ClientSession>(ws, id, shared_from_this(), uriPublic, isReadOnly, requestDetails);
        session->construct();

        if (_docState.isUnloadRequested())
        {
            // A new client has connected; recover.
            LOG_DBG(
                "Unload was requested after uploading, but new clients have joined. Recovering");
            _docState.resetUnloadRequested();
        }

        return session;
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Exception while preparing session [" << id << "]: " << exc.what());
    }

    if (ws)
    {
        constexpr std::string_view msg("error: cmd=internal kind=load");
        ws->sendTextMessage(msg);
        ws->shutdown(true, msg);
    }

    return nullptr;
}

void DocumentBroker::addCallback(const SocketPoll::CallbackFn& fn)
{
    _poll->addCallback(fn);
}

void DocumentBroker::addSocketToPoll(const std::shared_ptr<StreamSocket>& socket)
{
    _poll->insertNewSocket(socket);
}

std::weak_ptr<SocketPoll> DocumentBroker::getPoll() const
{
    return _poll;
}

void DocumentBroker::alertAllUsers(const std::string& msg)
{
    ASSERT_CORRECT_THREAD();

    if (UNITWSD_CALL_INSTANCE(_unitWsd, filterAlertAllusers(msg)))
        return;

    auto payload = std::make_shared<Message>(msg, Message::Dir::Out);

    LOG_DBG("Alerting all users of [" << _docKey << "]: " << msg);
    for (const auto& it : _sessions)
    {
        if (!it.second->inWaitDisconnected())
            it.second->enqueueSendMessage(payload);
    }
}

#if !MOBILEAPP
void DocumentBroker::syncBrowserSettings(const std::string& userId, const std::string& json)
{
    ASSERT_CORRECT_THREAD();
    LOG_DBG("Updating browsersetting with json[" << json
                                                 << "] for all sessions with userId [" << userId
                                                 << ']');

    for (const auto& it : _sessions)
    {
        if (it.second->getUserId() != userId)
            continue;

        try
        {
            LOG_TRC("Updating browsersetting with json[" << json << "] for session["
                                                         << it.second->getId() << ']');
            it.second->updateBrowserSettingsJSON(json);
        }
        catch (const std::exception& exc)
        {
            LOG_WRN("Failed to update browsersetting json for session["
                    << it.second->getId() << "] with error[" << exc.what()
                    << "], skipping the browsersetting upload step");
            return;
        }
    }
}

Poco::URI DocumentBroker::getPresetUploadBaseUrl(const Poco::URI& uriObject)
{
    std::string path = uriObject.getPath();
    size_t pos = path.find("/files/");
    if (pos != std::string::npos)
        path = path.substr(0, pos);
    path.append("/settings/upload");
    Poco::URI result(uriObject);
    result.setPath(path);
    return result;
}

void DocumentBroker::uploadPresetsToWopiHost()
{
    const std::string& jailPresetsPath = FileUtil::buildLocalPathToJail(
        COOLWSD::EnableMountNamespaces, getJailRoot(), JAILED_CONFIG_ROOT);

    Poco::URI uriObject = DocumentBroker::getPresetUploadBaseUrl(_uriPublic);
    LOG_DBG("Uploading presets from jailPath[" << jailPresetsPath << "] to wopiHost["
                                               << uriObject.toString() << ']');

    std::string searchDir = jailPresetsPath;
    searchDir.append("wordbook");
    const auto fileNames = FileUtil::getDirEntries(searchDir);
    std::error_code ec;
    for (const auto& fileName : fileNames)
    {
        std::string fileJailPath = searchDir;
        fileJailPath.push_back('/');
        fileJailPath.append(fileName);
        std::filesystem::file_time_type currentTimestamp =
            std::filesystem::last_write_time(fileJailPath, ec);

        auto it = _presetTimestamp.find(fileName);
        bool skipUpload = false;
        if (ec)
            skipUpload = true;
        else if (it != _presetTimestamp.end())
            skipUpload = (currentTimestamp <= it->second);
        else if (fileName != "standard.dic")
            skipUpload = true;

        if (skipUpload)
        {
            LOG_TRC("Skip uploading preset file [" << fileName << "] to wopiHost["
                                                   << uriObject.toString() << "], "
                                                   << (ec ? "missing" : "no modification"));
            continue;
        }

        std::string filePath = "/settings/userconfig/wordbook/";
        filePath.append(fileName);
        uriObject.addQueryParameter("fileId", filePath);

        auto httpRequest = StorageConnectionManager::createHttpRequest(
            uriObject, Authorization::create(_uriPublic));
        httpRequest.setVerb(http::Request::VERB_POST);

        LOG_TRC("Uploading file from jailPath[" << filePath << "] to wopiHost["
                                                << uriObject.toString() << ']');

        httpRequest.setBodyFile(fileJailPath);
        httpRequest.set("Content-Type", "application/octet-stream");

        auto httpSession = StorageConnectionManager::getHttpSession(uriObject);
        auto httpResponse = httpSession->syncRequest(httpRequest);

        http::StatusLine statusLine = httpResponse->statusLine();
        if (statusLine.statusCode() != http::StatusCode::OK)
        {
            LOG_ERR("Failed to upload file[" << fileName << "] to wopiHost["
                                             << uriObject.getAuthority() << " with status["
                                             << statusLine.reasonPhrase() << ']');
            continue;
        }

        LOG_DBG("Successfully uploaded presetFile[" << fileName << ']');
    }
}
#endif

void DocumentBroker::setKitLogLevel(const std::string& level)
{
    ASSERT_CORRECT_THREAD();
    _childProcess->sendTextFrame("setloglevel " + level);
}

std::string DocumentBroker::getDownloadURL(const std::string& downloadId)
{
    auto found = _registeredDownloadLinks.find(downloadId);
    if (found != _registeredDownloadLinks.end())
        return found->second;

    return std::string();
}

void DocumentBroker::unregisterDownloadId(const std::string& downloadId)
{
    auto found = _registeredDownloadLinks.find(downloadId);
    if (found != _registeredDownloadLinks.end())
        _registeredDownloadLinks.erase(found);
}

/// Handles input from the prisoner / child kit process
bool DocumentBroker::handleInput(const std::shared_ptr<Message>& message)
{
    LOG_TRC("DocumentBroker handling child message: [" << message->abbr() << ']');

    if constexpr (!Util::isMobileApp())
    {
        if (COOLWSD::TraceDumper)
            COOLWSD::dumpOutgoingTrace(getJailId(), "0", message->abbr());
    }

    if (UNITWSD_CALL_INSTANCE(_unitWsd, filterLOKitMessage(message)))
        return true;

    if (COOLProtocol::getFirstToken(message->forwardToken(), '-') == "client")
    {
        if (message->firstTokenMatches("slidelayer:") ||
            message->firstTokenMatches("zstdslidelayer:") ||
            message->firstTokenMatches("sliderenderingcomplete:"))
        {
            handleSlideLayerResponse(message);
        }
        else
            forwardToClient(message);
    }
    else
    {
        if (message->firstTokenMatches("tile:"))
        {
            handleTileResponse(message);
        }
        else if (message->firstTokenMatches("tilecombine:"))
        {
            handleTileCombinedResponse(message);
        }
        else if (message->firstTokenMatches("errortoall:"))
        {
            LOG_CHECK_RET(message->tokens().size() == 3, false);
            std::string cmd, kind;
            COOLProtocol::getTokenString((*message)[1], "cmd", cmd);
            LOG_CHECK_RET(cmd != "", false);
            COOLProtocol::getTokenString((*message)[2], "kind", kind);
            LOG_CHECK_RET(kind != "", false);
            Util::alertAllUsers(cmd, kind);
        }
        else if (message->firstTokenMatches("registerdownload:"))
        {
            LOG_CHECK_RET(message->tokens().size() == 4, false);
            std::string downloadid, url, clientId;
            COOLProtocol::getTokenString((*message)[1], "downloadid", downloadid);
            LOG_CHECK_RET(downloadid != "", false);
            COOLProtocol::getTokenString((*message)[2], "url", url);
            LOG_CHECK_RET(url != "", false);
            COOLProtocol::getTokenString((*message)[3], "clientid", clientId);
            LOG_CHECK_RET(!clientId.empty(), false);

            const std::string decoded = Uri::decode(url);
            const std::string filePath(FileUtil::buildLocalPathToJail(COOLWSD::EnableMountNamespaces,
                                                                      COOLWSD::ChildRoot + getJailId(),
                                                                      JAILED_DOCUMENT_ROOT + decoded));

            std::ifstream ifs(filePath);
            const std::string svg((std::istreambuf_iterator<char>(ifs)),
                                (std::istreambuf_iterator<char>()));
            ifs.close();

            if (svg.empty())
                LOG_WRN("Empty download: [id: " << downloadid << ", url: " << url << ']');

            const auto it = _sessions.find(clientId);
            if (it != _sessions.end())
            {
                std::ofstream ofs(filePath);
                ofs << it->second->processSVGContent(svg);
            }

            _registeredDownloadLinks[downloadid] = std::move(url);
        }
        else if (message->firstTokenMatches("traceevent:"))
        {
            LOG_CHECK_RET(message->tokens().size() == 1, false);
            if (COOLWSD::TraceEventFile != NULL && TraceEvent::isRecordingOn())
            {
                const auto& firstLine = message->firstLine();
                if (firstLine.size() < message->size())
                    COOLWSD::writeTraceEventRecording(message->data().data() + firstLine.size() + 1,
                                                      message->size() - firstLine.size() - 1);
            }
        }
        else if (message->firstTokenMatches("forcedtraceevent:"))
        {
            LOG_CHECK_RET(message->tokens().size() == 1, false);
            if (COOLWSD::TraceEventFile != NULL)
            {
                const auto& firstLine = message->firstLine();
                if (firstLine.size() < message->size())
                    COOLWSD::writeTraceEventRecording(message->data().data() + firstLine.size() + 1,
                                                      message->size() - firstLine.size() - 1);
            }
        }
        else if (message->firstTokenMatches("memorytrimmed:"))
        {
            clearCaches();
        }
        else if (Util::isDebugEnabled() && message->firstTokenMatches("unitresult:"))
        {
            UNITWSD_CALL(processUnitResult(message->tokens()));
        }
        else
        {
            LOG_ERR("Unexpected message: [" << message->abbr() << ']');
            return false;
        }
    }

    return true;
}

std::size_t DocumentBroker::getMemorySize() const
{
    return sizeof(DocumentBroker) +
        (!!_tileCache ? _tileCache->getMemorySize() : 0) +
        _sessions.size() * sizeof(ClientSession);
}

// Expected to be legacy, ~all new requests are tilecombinedRequests
void DocumentBroker::handleTileRequest(const StringVector &tokens, bool forceKeyframe,
                                       const std::shared_ptr<ClientSession>& session)
{
    ASSERT_CORRECT_THREAD();

    TileDesc tile = TileDesc::parse(tokens);
    tile.setCanonicalViewId(session->getCanonicalViewId());

    tile.setVersion(++_tileVersion);
    const std::string tileMsg = tile.serialize();
    LOG_TRC("Tile request for " << tileMsg);

    if (!hasTileCache())
    {
        LOG_WRN("Tile request without a loaded document?");
        return;
    }

    if (forceKeyframe)
    {
        LOG_TRC("forcing a keyframe for tilecombined tile");
        session->resetTileSeq(tile);
    }

    Tile cachedTile = _tileCache->lookupTile(tile);
    if (cachedTile && cachedTile->isValid())
    {
        if (tile.getWireId() == 0)
            tile.setWireId(cachedTile->_wids.back());

        session->sendTileNow(tile, cachedTile);
        return;
    }

    if (!cachedTile || cachedTile->tooLarge())
        tile.forceKeyframe();

    auto now = std::chrono::steady_clock::now();
    tileCache().subscribeToTileRendering(tile, session, now);

    // Forward to child to render.
    LOG_DBG("Sending render request for tile (" << tile.getPart() << ',' <<
            tile.getEditMode() << ',' << tile.getTilePosX() << ',' << tile.getTilePosY() << ").");
    const std::string request = "tile " + tileMsg;
    _childProcess->sendTextFrame(request);
    _debugRenderedTileCount++;
}

void DocumentBroker::sendTileCombine(const TileCombined& newTileCombined)
{
    assert(!newTileCombined.hasDuplicates());

    // Forward to child to render.
    const std::string req = newTileCombined.serialize("tilecombine");
    LOG_TRC("Some of the tiles were not prerendered. Sending residual tilecombine: " << req);
    _childProcess->sendTextFrame(req);
}

void DocumentBroker::handleTileCombinedRequest(TileCombined& tileCombined, bool canForceKeyframe,
                                               const std::shared_ptr<ClientSession>& session)
{
    ASSERT_CORRECT_THREAD();

    assert(!tileCombined.hasDuplicates());

    LOG_TRC("TileCombined request for " << tileCombined.serialize() << " from " <<
            (canForceKeyframe ? "client" : "wsd"));
    if (!hasTileCache())
    {
        LOG_WRN("Combined tile request without a loaded document?");
        return;
    }

    // Check which newly requested tiles need rendering.
    const auto now = std::chrono::steady_clock::now();
    std::vector<TileDesc> tilesNeedsRendering;
    bool hasOldWireId = false;
    ++_tileVersion; // bump only once
    for (auto& tile : tileCombined.getTiles())
    {
        tile.setVersion(_tileVersion);

        // client can force keyframe with an oldWid == 0 on tile
        if (canForceKeyframe && tile.isForcedKeyFrame())
        {
            // combinedtiles requests direct from the browser get flagged.
            // The browser may have dropped / cleaned its cache, so we can't
            // rely on what we think we have sent it to send a delta in this
            // case; so forget what we last sent.
            LOG_TRC("forcing a keyframe for tilecombined tile (" << tile.getPart() << ',' <<
                    tile.getEditMode() << ',' << tile.getTilePosX() << ',' << tile.getTilePosY() << ").");
            session->resetTileSeq(tile);
            // don't force a keyframe to be rendered, only to be sent.
            tile.setOldWireId(1);
            hasOldWireId = true;
        }

        Tile cachedTile = _tileCache->lookupTile(tile);
        bool tooLarge = cachedTile && cachedTile->tooLarge();
        if(!cachedTile || !cachedTile->isValid() || tooLarge)
        {
            bool forceKeyFrame = false;
            if (!cachedTile || tooLarge)
            {
                forceKeyFrame = true;
                tile.forceKeyframe();
            }

            requestTileRendering(tile, forceKeyFrame, _tileVersion, now, tilesNeedsRendering, session);
        }
    }
    if (hasOldWireId)
        tileCombined.setHasOldWireId();

    // Send rendering request, prerender before we actually send the tiles
    if (!tilesNeedsRendering.empty())
        sendTileCombine(TileCombined::create(tilesNeedsRendering));

    // Accumulate tiles
    std::deque<TileDesc>& requestedTiles = session->getRequestedTiles();
    if (requestedTiles.empty())
    {
        requestedTiles = std::deque<TileDesc>(tileCombined.getTiles().begin(), tileCombined.getTiles().end());
    }
    // Drop duplicated tiles, but use newer version number
    else
    {
        // Make sure that the old request has the same canonicalviewid with the new request.
        for (size_t i = 0; i < requestedTiles.size(); i++) {
            if (requestedTiles[i].getCanonicalViewId() != session->getCanonicalViewId())
                requestedTiles[i].setCanonicalViewId(session->getCanonicalViewId());
        }

        for (const auto& newTile : tileCombined.getTiles())
        {
            bool tileFound = false;
            for (auto& oldTile : requestedTiles)
            {
                if(oldTile.getTilePosX() == newTile.getTilePosX() &&
                   oldTile.getTilePosY() == newTile.getTilePosY() &&
                   oldTile.sameTileCombineParams(newTile))
                {
                    oldTile.setVersion(newTile.getVersion());
                    oldTile.setOldWireId(newTile.getOldWireId());
                    oldTile.setWireId(newTile.getWireId());
                    tileFound = true;
                    break;
                }
            }
            if(!tileFound)
                requestedTiles.push_back(newTile);
        }
    }

    sendRequestedTiles(session);
}

void DocumentBroker::handleGetSlideRequest(const StringVector& tokens,
                                           const std::shared_ptr<ClientSession>& session)
{
    // cacheKey example:
    // hash=108777063986320 part=0 width=1919 height=1080 renderBackground=1 renderMasterPage=1 devicePixelRatio=1 compressedLayers=0 uniqueID=324
    std::string cacheKey = tokens.substrFromToken(1);
    if (auto itr = _slideLayerCache.find(cacheKey); itr != _slideLayerCache.end())
    {
        LOG_INF("Slideshow: Cached slide layer reused by canonical view ID "
                << session->getCanonicalViewId());
        for (const auto& message : itr->second)
        {
            session->sendBinaryFrame(message->data().data(), message->size());
        }
        return;
    }
    LOG_INF("Slideshow: Cached slide layer not found, slides layer is freshely rendered by "
            "canonical view ID "
            << session->getCanonicalViewId());
    forwardToChild(session, tokens.substrFromToken(0));
}

void DocumentBroker::handleSlideLayerResponse(const std::shared_ptr<Message>& message)
{
    if (EnableExperimental)
    {
        size_t pos = Util::findInVector(message->data(), "\n");
        std::string msg(message->data().data(), pos == std::string::npos ? message->size() : pos);
        Poco::JSON::Object::Ptr jsonPtr;
        if (!JsonUtil::parseJSON(msg, jsonPtr))
        {
            LOG_ERR("Invalid slide layer response, could not parse JSON: " << msg);
            return;
        }
        const std::string key = JsonUtil::getJSONValue<std::string>(jsonPtr, "cacheKey");

        // This message has forwardToken which can cause issue if reused for forwardToClient when using cache.
        // But we ignore it because when reusing cache we only send data from the message and not entire message
        _slideLayerCache.insert(key, message);
        LOG_INF("Slideshow: Cached a slide layer with cache key: " << key);
    }
    forwardToClient(message);
}

/// lookup in global clipboard cache and send response, send error if missing if @sendError
bool DocumentBroker::handlePersistentClipboardRequest(ClipboardRequest type,
                                                      const std::shared_ptr<StreamSocket> &socket,
                                                      const std::string &tag, bool sendError)
{
    LOG_TRC("Clipboard request " << tag << " not for a live session - check cache.");
#if !MOBILEAPP
    std::shared_ptr<FileUtil::OwnedFile> clipFile;
    if (type != ClipboardRequest::CLIP_REQUEST_SET)
        clipFile = COOLWSD::SavedClipboards->getClipboard(tag);
    if (clipFile)
    {
        auto session = std::make_shared<http::ServerSession>();

        http::ServerSession::ResponseHeaders headers;
        headers.emplace_back("Last-Modified", Util::getHttpTimeNow());
        headers.emplace_back("Content-Type", "application/octet-stream");
        headers.emplace_back("X-Content-Type-Options", "nosniff");
        headers.emplace_back("X-COOL-Clipboard", "true");
        headers.emplace_back("Cache-Control", "no-cache");
        headers.emplace_back("Connection", "close");

        // hold save clipfile until session dtor to guarantee it persists until completion
        session->setFinishedHandler([clipFile](const std::shared_ptr<http::ServerSession>&) {});

        // Hand over socket to ServerSession which will async provide
        // clipboard content backed by clipFile
        session->asyncUpload(clipFile->_file, std::move(headers));
        socket->setHandler(std::static_pointer_cast<ProtocolHandlerInterface>(session));

        LOG_INF("Found and queued clipboard response for send of size " << FileUtil::Stat(clipFile->_file).size());
        return true;
    }
#else
    (void)type;
#endif

    if (!sendError)
        return false;

    if constexpr (!Util::isMobileApp())
    {
        // Bad request.
        HttpHelper::sendError(http::StatusCode::BadRequest, socket, "Failed to find this clipboard",
                              "Connection: close\r\n");
    }

    socket->asyncShutdown();
    socket->ignoreInput();

    return false;
}

#if !MOBILEAPP

std::shared_ptr<ClientSession> DocumentBroker::getSessionFromClipboardTag(const std::string &viewId, const std::string &tag)
{
    for (const auto& it : _sessions)
    {
        if (it.second->matchesClipboardKeys(viewId, tag))
        {
            return it.second;
        }
    }
    return nullptr;
}

void DocumentBroker::handleClipboardRequest(ClipboardRequest type, const std::shared_ptr<StreamSocket> &socket,
                                            const std::string &viewId, const std::string &tag,
                                            const std::string &clipFile)
{
    if (std::shared_ptr<ClientSession> session = getSessionFromClipboardTag(viewId, tag))
    {
        session->handleClipboardRequest(type, socket, tag, clipFile);
        return;
    }

    if (!handlePersistentClipboardRequest(type, socket, tag, true))
        LOG_ERR("Could not find matching session to handle clipboard request for " << viewId << " tag: " << tag);
}

void DocumentBroker::handleMediaRequest(const std::string_view range,
                                        const std::shared_ptr<Socket>& socket,
                                        const std::string& tag,
                                        const std::string& field)
{
    LOG_DBG("handleMediaRequest: " << tag);

    auto streamSocket = std::static_pointer_cast<StreamSocket>(socket);
    if (!streamSocket)
    {
        LOG_ERR("Invalid socket to handle media request in Doc [" << _docId << "] with tag [" << tag
                                                                  << ']');
        return;
    }

    const auto it = _embeddedMedia.find(tag);
    if (it == _embeddedMedia.end())
    {
        LOG_ERR("Invalid media request in Doc [" << _docId << "] with tag [" << tag << ']');
        return;
    }

    LOG_DBG("Media: " << it->second);
    Poco::JSON::Object::Ptr object;
    if (JsonUtil::parseJSON(it->second, object))
    {
        LOG_ASSERT(JsonUtil::getJSONValue<std::string>(object, "id") == tag);
        const std::string url = JsonUtil::getJSONValue<std::string>(object, field);
        LOG_ASSERT(!url.empty());
        if (Util::toLower(url).starts_with("file://"))
        {
            // For now, we only support file:// schemes.
            // In the future, we may/should support http.
            std::string localPath = url.substr(sizeof("file:///") - 1);
            std::string path = getAbsoluteMediaPath(std::move(localPath));

            auto session = std::make_shared<http::ServerSession>();
            http::ServerSession::ResponseHeaders responseHeaders;
            responseHeaders.emplace_back("Content-Type", (field == "url" ? "video/mp4" : "text/plain"));
            session->asyncUpload(std::move(path), std::move(responseHeaders), range);
            streamSocket->setHandler(std::static_pointer_cast<ProtocolHandlerInterface>(session));
        }
    }
}

#endif

bool DocumentBroker::requestTileRendering(TileDesc& tile, bool forceKeyframe, int version,
                                          const std::chrono::steady_clock::time_point now,
                                          std::vector<TileDesc>& tilesNeedsRendering,
                                          const std::shared_ptr<ClientSession>& session)
{
    bool allSamePartAndSize = true;
    if (!tileCache().hasTileBeingRendered(tile, &now) || // There is no in progress rendering of the given tile
        tileCache().getTileBeingRenderedVersion(tile) < tile.getVersion()) // We need a newer version
    {
        tile.setVersion(version);

        if (forceKeyframe)
        {
            LOG_TRC("Forcing keyframe for tile was oldwid " << tile.getOldWireId());
            tile.forceKeyframe();
        }
        allSamePartAndSize &= tilesNeedsRendering.empty() || tile.sameTileCombineParams(tilesNeedsRendering.back());
        tilesNeedsRendering.push_back(tile);
        _debugRenderedTileCount++;
    }

    tileCache().subscribeToTileRendering(tile, session, now);
    return allSamePartAndSize;
}

void DocumentBroker::sendRequestedTiles(const std::shared_ptr<ClientSession>& session)
{
    ASSERT_CORRECT_THREAD();

    size_t tilesOnFlyUpperLimit = session->getTilesOnFlyUpperLimit();

    auto now = std::chrono::steady_clock::now();

    // Drop tiles which we are waiting for too long
    session->removeOutdatedTilesOnFly(now);

    // All tiles were processed on client side that we sent last time, so we can send
    // a new batch of tiles which was invalidated / requested in the meantime
    std::deque<TileDesc>& requestedTiles = session->getRequestedTiles();
    bool bumpedVersion = false;
    if (!requestedTiles.empty() && hasTileCache())
    {
        std::vector<TileDesc> tilesNeedsRendering;
        bool allSamePartAndSize = true;
        while (!requestedTiles.empty() &&
               session->getTilesOnFlyCount() < tilesOnFlyUpperLimit)
        {
            TileDesc& tile = *(requestedTiles.begin());

            // Satisfy as many tiles from the cache.
            Tile cachedTile = _tileCache->lookupTile(tile);
            if (cachedTile && cachedTile->isValid())
            {
                // It is typical for a request not to have a wireId. If the result is generated
                // without using the cache then doRender will send a timecombine result with wireIds
                // set. But if we use the cache here we send a response using the wireId of the request.

                // With no wireId in the request the result will have wireId of 0 and
                // in CanvasTileLayer.js tile::needsFetch such a tile will always return true
                // for needsFetch and is wasted.
                if (tile.getWireId() == 0)
                    tile.setWireId(cachedTile->_wids.back());

                // TODO: Combine the response to reduce latency.
                session->sendTileNow(tile, cachedTile);
            }
            else
            {
                // Not cached, needs rendering.
                if (!bumpedVersion)
                {
                    ++_tileVersion; // only once
                    bumpedVersion = true;
                }
                bool forceKeyFrame = !cachedTile;
                allSamePartAndSize &= requestTileRendering(tile, forceKeyFrame, _tileVersion, now, tilesNeedsRendering, session);
            }
            requestedTiles.pop_front();
        }

        // Send rendering request for those tiles which were not prerendered
        if (!tilesNeedsRendering.empty())
        {
            if (allSamePartAndSize)
            {
                // typically all requests match sufficiently to form a single tilecombine
                sendTileCombine(TileCombined::create(tilesNeedsRendering));
            }
            else
            {
                // but if not, split them by matching groups of requests to send a separate
                // tilecombine for each group
                std::vector<std::vector<TileDesc>> groupsNeedsRendering(1);
                auto it = tilesNeedsRendering.begin();
                // start off with one group bucket
                groupsNeedsRendering[0].push_back(*it++);
                while (it != tilesNeedsRendering.end())
                {
                    bool inserted = false;
                    // check if tile should go into an existing group bucket
                    for (size_t i = 0; i < groupsNeedsRendering.size(); ++i)
                    {
                        if (it->sameTileCombineParams(groupsNeedsRendering[i][0]))
                        {
                            groupsNeedsRendering[i].push_back(*it);
                            inserted = true;
                            break;
                        }
                    }
                    // if not, add another and put it there
                    if (!inserted)
                    {
                        groupsNeedsRendering.emplace_back();
                        groupsNeedsRendering.back().push_back(*it);
                    }
                    ++it;
                }
                for (const auto& group : groupsNeedsRendering)
                    sendTileCombine(TileCombined::create(group));
            }
        }
    }
}

void DocumentBroker::handleTileResponse(const std::shared_ptr<Message>& message)
{
    ASSERT_CORRECT_THREAD();

    const std::string firstLine = message->firstLine();
    LOG_DBG("Handling tile: " << firstLine);

    try
    {
        const std::size_t length = message->size();
        if (firstLine.size() < static_cast<std::string::size_type>(length) - 1)
        {
            const TileDesc tile = TileDesc::parse(firstLine);
            const char* buffer = message->data().data();
            const std::size_t offset = firstLine.size() + 1;

            tileCache().saveTileAndNotify(tile, buffer + offset, length - offset);
        }
        else
        {
            LOG_WRN("Dropping empty tile response: " << firstLine);
            // They will get re-issued if we don't forget them.
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to process tile response [" << firstLine << "]: " << exc.what() << '.');
    }
}

void DocumentBroker::handleTileCombinedResponse(const std::shared_ptr<Message>& message)
{
    const std::string firstLine = message->firstLine();
    LOG_DBG("Handling tile combined: " << firstLine);

    ASSERT_CORRECT_THREAD();

    try
    {
        const std::size_t length = message->size();
        if (firstLine.size() <= static_cast<std::string::size_type>(length) - 1)
        {
            const TileCombined tileCombined = TileCombined::parse(firstLine);
            const char* buffer = message->data().data();
            std::size_t offset = firstLine.size() + 1;

            for (const auto& tile : tileCombined.getTiles())
            {
                tileCache().saveTileAndNotify(tile, buffer + offset, tile.getImgSize());
                offset += tile.getImgSize();
            }
        }
        else
        {
            LOG_INF("Dropping empty tilecombine response: " << firstLine);
            // They will get re-issued if we don't forget them.
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to process tile response [" << firstLine << "]: " << exc.what() << '.');
    }
}

bool DocumentBroker::haveAnotherEditableSession(const std::string& id) const
{
    ASSERT_CORRECT_THREAD();

    for (const auto& it : _sessions)
    {
        if (it.second->getId() != id && it.second->isViewLoaded() && it.second->isEditable())
        {
            // This is a loaded session that is non-readonly.
            return true;
        }
    }

    // None found.
    return false;
}

std::size_t DocumentBroker::countActiveSessions() const
{
    ASSERT_CORRECT_THREAD();

    std::size_t count = 0;
    for (const auto& it : _sessions)
    {
        if (it.second->isLive())
        {
            ++count;
        }
    }

    return count;
}

std::size_t DocumentBroker::countLoadingSessions() const
{
    std::size_t count = 0;
    for (const auto& session : _sessions)
    {
        if (!session.second->isLive() && !session.second->inWaitDisconnected())
        {
            ++count;
        }
    }

    return count;
}

void DocumentBroker::setModified(const bool value)
{
#if !MOBILEAPP
    // Flag the document as modified in the admin console.
    _admin.modificationAlert(_docKey, getPid(), value);

    // Flag the document as uploaded in the admin console.
    _admin.uploadedAlert(
        _docKey, getPid(), !isAsyncUploading() && needToUploadToStorage() == NeedToUpload::No);
#endif

    LOG_DBG("Modified state set to " << value << " for Doc [" << _docId << ']');
    _isModified = value;
}

bool DocumentBroker::isInitialSettingSet(const std::string& name) const
{
    return _isInitialStateSet.find(name) != _isInitialStateSet.end();
}

void DocumentBroker::setInitialSetting(const std::string& name)
{
    _isInitialStateSet.emplace(name);
}

bool DocumentBroker::forwardUrpToChild(const std::string& message)
{
    return _childProcess && _childProcess->sendUrpMessage(message);
}

std::string
DocumentBroker::applySignViewSettings(const std::string& message,
                                      const std::shared_ptr<ClientSession>& session) const
{
    std::string finalMsg = message;
    if (!_isViewSettingsUpdated)
    {
        return finalMsg;
    }

    Poco::JSON::Object::Ptr signatureJson = new Poco::JSON::Object();

    std::string signatureCert = session->getSignatureCertificate();
    std::string signatureKey = session->getSignatureKey();
    std::string signatureCa = session->getSignatureCa();

    signatureJson->set("SignatureCert", signatureCert);
    signatureJson->set("SignatureKey", signatureKey);
    signatureJson->set("SignatureCa", signatureCa);

    std::string jsonString = JsonUtil::jsonToString(signatureJson);

    std::string encodedJson;
    Poco::URI::encode(jsonString, "", encodedJson);
    finalMsg += " signatureconfig=" + encodedJson;

    LOG_INF("Sent signatureconfig with values");
    return finalMsg;
}

std::string DocumentBroker::applyBrowserAccessibility(const std::string& message,
                                                   const std::string& viewId)
{
    bool accessibilityEnabled = false;
    bool lockAccessibilityOn = false;
    const auto it = _sessions.find(viewId);
    if (it != _sessions.end())
    {
        auto session = it->second;
        auto json = session->getBrowserSettingJSON();
        JsonUtil::findJSONValue(json, "accessibilityState", accessibilityEnabled);
        JsonUtil::findJSONValue(json, "lockAccessibilityOn", lockAccessibilityOn);
    }
    else
        LOG_WRN("Cannot lock accessibility on for ClientSession [" << viewId << ']');

    if (!accessibilityEnabled)
        return message;

    if (lockAccessibilityOn && it != _sessions.end())
    {
        auto session = it->second;
        session->sendTextFrame("lockaccessibilityon");
    }

    // Ensure accessibilityState=true is enabled. Overwrite accessibilityState=
    // if it exists, append otherwise.
    bool accessibilityOverridden = false;
    std::string result;
    result.reserve(message.size());
    const StringVector tokens = StringVector::tokenize(message);
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (i)
            result.push_back(' ');
        if (tokens[i].starts_with("accessibilityState"))
        {
            result.append("accessibilityState=true");
            accessibilityOverridden = true;
        }
        else
            result.append(tokens[i]);
    }
    if (!accessibilityOverridden)
    {
        result.push_back(' ');
        result.append("accessibilityState=true");
    }
    return result;
}

std::string DocumentBroker::applyViewSetting(const std::string& message, const std::string& viewId,
                                             const std::shared_ptr<ClientSession>& session)
{
    std::string msgWithSignSettings = applySignViewSettings(message, session);
    return applyBrowserAccessibility(msgWithSignSettings, viewId);
}

bool DocumentBroker::forwardToChild(const std::shared_ptr<ClientSession>& session,
                                    const std::string& message, bool binary)
{
    if (message.starts_with("urp "))
        return forwardUrpToChild(message);

    ASSERT_CORRECT_THREAD();
    LOG_ASSERT_MSG(session, "Must have a valid ClientSession");
    if (_sessions.find(session->getId()) == _sessions.end())
    {
        LOG_WRN("Cannot forward to unknown ClientSession [" << session->getId()
                                                            << "]: " << message);
        return false;
    }

    // Ignore userinactive, useractive message until document is loaded
    if (!isLoaded() && (message == "userinactive" || message == "useractive"))
    {
        return true;
    }

    // Ignore textinput, mouse and key message when document is unloading
    if (isUnloading() && (message.starts_with("textinput ") || message.starts_with("mouse ") ||
                          message.starts_with("key ")))
    {
        return true;
    }

    std::string viewId = session->getId();

    // Should not get through; we have our own save command.
    // .uno:SaveGraphic is not a document save - it exports an image to a temp file.
    assert(!message.starts_with("uno .uno:Save") || message.starts_with("uno .uno:SaveGraphic"));

    LOG_TRC("Forwarding payload to child [" << viewId << "]: " << getAbbreviatedMessage(message));

#if 0 // extreme paste debugging - message can be giant and binary
    if (Log::traceEnabled() && message.starts_with("paste "))
        LOG_TRC("Logging paste payload (" << message.size() << " bytes) '" << message << "' end paste");
#endif

    std::string msg = "child-" + viewId + ' ';
    if (message.starts_with("load "))
    {
        // Special-case loading.
        const StringVector tokens = StringVector::tokenize(message);
        if (tokens.size() > 1 && tokens.equals(0, "load"))
        {
            LOG_ASSERT_MSG(!_uriJailed.empty(), "Must have valid _uriJailed");

            // The json options must come last.
            msg += "load " + tokens[1];
            msg += " jail=" + _uriJailed;
            msg += " xjail=" + _uriJailedAnonym;
            msg += ' ' + tokens.cat(' ', 2);
#if !MOBILEAPP
            if (_asyncInstallTask)
            {
                auto sendLoad = [selfWeak = weak_from_this(), this, viewId = std::move(viewId),
                                 msg = std::move(msg), binary, session](bool success) {
                    if (!success)
                        return;
                    std::shared_ptr<DocumentBroker> selfLifecycle = selfWeak.lock();
                    if (!selfLifecycle)
                        return;

                    _childProcess->sendFrame(applyViewSetting(msg, viewId, session), binary);
                };
                _asyncInstallTask->appendCallback(sendLoad);
                return true;
            }
#endif
            return _childProcess->sendFrame(applyViewSetting(msg, viewId, session), binary);
        }
    }

    // Forward message with prefix to the Kit.
    return _childProcess->sendFrame(msg + message, binary);
}

bool DocumentBroker::forwardToClient(const std::shared_ptr<Message>& payload)
{
    ASSERT_CORRECT_THREAD();

    const std::string& prefix = payload->forwardToken();
    LOG_TRC("Forwarding payload to [" << prefix << "]: " << payload->abbr());

    std::string name;
    std::string sid;
    if (COOLProtocol::parseNameValuePair(prefix, name, sid, '-') && name == "client")
    {
        if (sid == "all")
        {
            // Broadcast to all.
            // Events could cause the removal of sessions.
            std::map<std::string, std::shared_ptr<ClientSession>> sessions(_sessions);
            for (const auto& it : sessions)
            {
                if (!it.second->inWaitDisconnected())
                    it.second->handleKitToClientMessage(payload);
            }
        }
        else
        {
            const auto it = _sessions.find(sid);
            if (it != _sessions.end())
            {
                // Take a ref as the session could be removed from _sessions
                // if it's the save confirmation keeping a stopped session alive.
                std::shared_ptr<ClientSession> session = it->second;
                return session->handleKitToClientMessage(payload);
            }

            LOG_WRN("Client session [" << sid << "] not found to forward message: "
                                       << (COOLWSD::AnonymizeUserData ? "..." : payload->abbr()));
        }
    }
    else
    {
        LOG_ERR("Unexpected prefix of forward-to-client message: " << prefix);
    }

    return false;
}

void DocumentBroker::shutdownClients(const std::string_view closeReason)
{
    ASSERT_CORRECT_THREAD();
    LOG_INF("Terminating " << _sessions.size() << " clients of doc [" << _docKey << "] with reason: " << closeReason);

    // First copy into local container, since removeSession
    // will erase from _sessions, but will leave the last.
    std::map<std::string, std::shared_ptr<ClientSession>> sessions = _sessions;
    for (const auto& pair : sessions)
    {
        const std::shared_ptr<ClientSession>& session = pair.second;
        try
        {
            if (session->inWaitDisconnected())
                finalRemoveSession(session);
            else
            {
                // Notify the client and disconnect.
                session->shutdownGoingAway(closeReason);

                // Remove session, save, and mark to destroy.
                removeSession(session);
            }
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Error while shutting down client [" <<
                    session->getName() << "]: " << exc.what());
        }
    }
}

void DocumentBroker::terminateChild(const std::string_view closeReason)
{
    ASSERT_CORRECT_THREAD();

    LOG_INF("Terminating doc [" << _docKey << "] with reason: " << closeReason);

    // Close all running sessions first.
    shutdownClients(closeReason);

    if (_childProcess)
    {
        LOG_INF("Terminating child [" << getPid() << "] of doc [" << _docKey << ']');

        _childProcess->close();
    }
    stop(closeReason);
}

void DocumentBroker::closeDocument(const std::string& reason)
{
    ASSERT_CORRECT_THREAD();

#if !MOBILEAPP
    if (reason == "oom")
    {
        // This is an internal close request, coming from Admin::triggerMemoryCleanup().
        // Dump the state now, since it's unsafe to do it from outside our poll thread.

        // But first signal the Kit, because we might kill it soon after returning.
        ::kill(getPid(), SIGUSR1);

        std::ostringstream oss(Util::makeDumpStateStream());
        dumpState(oss);
        LOG_WRN("OOM-closing Document [" << _docId << "]: " << oss.str());
    }
#endif

    _docState.setCloseRequested();
    _closeReason = reason;
    if (_documentChangedInStorage)
    {
        // Discarding changes in the face of conflict in storage.
        LOG_DBG("Closing DocumentBroker for docKey ["
                << _docKey << "] and discarding changes with reason: " << reason);
        stop(reason);
    }
    else
    {
        LOG_DBG("Closing DocumentBroker for docKey [" << _docKey << "] with reason: " << reason);
    }
}

void DocumentBroker::disconnectedFromKit(bool unexpected)
{
    ASSERT_CORRECT_THREAD();

    // Always set the kit disconnected flag.
    _docState.setKitDisconnected(unexpected ? DocumentState::KitDisconnected::Unexpected
                                            : DocumentState::KitDisconnected::Normal);
    if (_closeReason.empty())
    {
        // If we have a reason to close, no advantage in clobbering it.
        LOG_INF("DocBroker [" << _docKey << "] Disconnected from Kit. Flagging to close");
        closeDocument("docdisconnected");
    }
    else
    {
        LOG_INF("DocBroker [" << _docKey << "] Disconnected from Kit while closing with reason ["
                              << _closeReason << ']');
    }

    failLoadingSessions(/*remove=*/false);
}

void DocumentBroker::failLoadingSessions(bool remove)
{
    // All the sessions waiting to load need to be notified and removed.
    LOG_TRC("Failing any loading sessions with" << (remove ? "" : "out") << " removal");
    for (auto it = _sessions.rbegin(); it != _sessions.rend();)
    {
        const auto& pair = *it;
        if (!pair.second->isLive() && !pair.second->inWaitDisconnected() &&
            !pair.second->isCloseFrame())
        {
            const std::string msg("error: cmd=load kind=docunloading");
            LOG_INF("Rejecting loading session [" << pair.first << "] with " << msg);
            pair.second->sendTextFrame(msg);
            if (remove)
            {
                it = decltype(it)(_sessions.erase(std::next(it).base()));
                continue;
            }
        }

        ++it;
    }
}

std::size_t DocumentBroker::broadcastMessage(const std::string& message) const
{
    ASSERT_CORRECT_THREAD();

    LOG_DBG("Broadcasting message [" << message << "] to all " << _sessions.size() << " sessions.");
    std::size_t count = 0;
    for (const auto& sessionIt : _sessions)
    {
        count += (!sessionIt.second->isCloseFrame() && sessionIt.second->sendTextFrame(message));
    }

    return count;
}

void DocumentBroker::broadcastMessageToOthers(const std::string& message,
                                              const std::shared_ptr<ClientSession>& session) const
{
    ASSERT_CORRECT_THREAD();

    LOG_DBG("Broadcasting message [" << message << "] to all " << _sessions.size()
                                     << " sessions, except for " << session->getId());
    for (const auto& sessionIt : _sessions)
    {
        if (sessionIt.second != session)
            sessionIt.second->sendTextFrame(message);
    }
}

void DocumentBroker::processBatchUpdates()
{
#if !MOBILEAPP
    if ((_lastActivityTime - _lastNotifiedActivityTime) > 250ms)
    {
        _admin.updateLastActivityTime(_docKey);
        _lastNotifiedActivityTime = _lastActivityTime;
    }
#endif
}

void DocumentBroker::getIOStats(uint64_t &sent, uint64_t &recv)
{
    sent = 0;
    recv = 0;
    ASSERT_CORRECT_THREAD();
    for (const auto& sessionIt : _sessions)
    {
        uint64_t s = 0, r = 0;
        sessionIt.second->getIOStats(s, r);
        sent += s;
        recv += r;
    }
}

#if !MOBILEAPP
void DocumentBroker::checkFileInfo(const std::shared_ptr<ClientSession>& session, int redirectLimit)
{
    assert(_docState.activity() == DocumentState::Activity::SyncFileTimestamp &&
           "Unexpected activity for CheckFileInfo");
    assert(_storage && "Unexpected to not have Storage instance duing SyncFileTimestamp");

    if (!session)
    {
        assert(session && "Expected a valid session to CheckFileInfo");
        return;
    }

    if (_checkFileInfo)
    {
        LOG_DBG("CheckFileInfo is in progress already");
        return;
    }

    std::weak_ptr<ClientSession> weakSession = session;
    auto cfiContinuation = [this, weakSession]([[maybe_unused]] CheckFileInfo& checkFileInfo)
    {
        assert(_docState.activity() == DocumentState::Activity::SyncFileTimestamp &&
               "Expected to be in SyncFileTimestamp activity");
        assert(&checkFileInfo == _checkFileInfo.get() && "Unknown CheckFileInfo instance");
        assert(checkFileInfo.completed() &&
               "Expected CheckFileInfo to be completed when calling the continuation");

        // End the SyncFileTimestamp activity, but don't reset _checkFileInfo yet (it's our caller).
        endActivity();

        if (checkFileInfo.state() == CheckFileInfo::State::Pass && checkFileInfo.wopiInfo())
        {
            Poco::JSON::Object::Ptr object = _checkFileInfo->wopiInfo();

            std::string lastModifiedTime;
            JsonUtil::findJSONValue(object, "LastModifiedTime", lastModifiedTime);
            std::size_t size = 0;
            JsonUtil::findJSONValue(object, "Size", size);

            // It's highly unlikely that the document has been clobbered externally,
            // yet the size matches exactly. Still, if we are paranoid, we can download
            // and compare the SHA256 with the one we uploaded. For now, this is an improvement.
            if (_storageManager.getSizeAsUploaded() == size || _storageManager.getSizeOnServer())
            {
                LOG_INF("After failing to upload ["
                        << _docKey << "], the size on WOPI host matches "
                        << (_storageManager.getSizeAsUploaded() == size ? "our uploaded"
                                                                        : "the old size before our")
                        << " last uploaded size: " << size
                        << " bytes. We will assume this is our last uploaded version and "
                           "synchronize the timestamp to: "
                        << lastModifiedTime
                        << "(from: " << _storageManager.getLastModifiedServerTimeString() << ')');

                _storage->setLastModifiedTime(lastModifiedTime);
                _storageManager.setLastModifiedServerTimeString(lastModifiedTime);
            }
            else
            {
                LOG_WRN("After failing to upload, the document size neither matches the original, "
                        "nor our last uploaded. The document is in conflict.");

                handleDocumentConflict();
            }
        }
        else
        {
            // We failed to get CheckFileInfo.
            _storage->setLastModifiedTimeUnSafe(); // We can't trust the LastModifiedTime.

            std::shared_ptr<ClientSession> failedSession = weakSession.lock();
            if (checkFileInfo.state() == CheckFileInfo::State::Unauthorized)
            {
                if (failedSession)
                {
                    // Got some response, but not positive. This is an expired session.
                    LOG_WRN("CheckFileInfo on ["
                            << failedSession->getId()
                            << "] failed because it has invalid access_token for [" << _docKey
                            << "], resetting the authorization token");
                    failedSession->invalidateAuthorizationToken();
                }
                else
                {
                    LOG_WRN("CheckFileInfo failed and its session is expired");
                }
            }
            else
            {
                assert(checkFileInfo.state() == CheckFileInfo::State::Timedout ||
                       checkFileInfo.state() == CheckFileInfo::State::Fail);
                LOG_INF("CheckFileInfo on ["
                        << _docKey << "] for session #"
                        << (failedSession ? failedSession->getId() : "<expired>") << " timed-out");
            }
        }
    };

    // CheckFileInfo asynchronously.
    assert(!_checkFileInfo && "Unexpected CheckFileInfo in progress");
    _checkFileInfo =
        std::make_shared<CheckFileInfo>(_poll, session->getPublicUri(), std::move(cfiContinuation));
    if (!_checkFileInfo->checkFileInfo(redirectLimit))
    {
        LOG_INF("Resetting async CheckFileInfo as it failed to start");
        _checkFileInfo.reset();
    }
}
#endif // !MOBILEAPP

std::vector<std::shared_ptr<ClientSession>> DocumentBroker::getSessionsTestOnlyUnsafe()
{
    std::vector<std::shared_ptr<ClientSession>> result;
    for (const auto& it : _sessions)
        result.push_back(it.second);
    return result;
}

void DocumentBroker::dumpState(std::ostream& os)
{
    uint64_t sent = 0, recv = 0;
    getIOStats(sent, recv);

    const auto now = std::chrono::steady_clock::now();

    os << "\nDocumentBroker [" << _docId << "] Dump: [" << getDocKey() << "], pid: " << getPid();
    if (_docState.isMarkedToDestroy())
        os << " *** Marked to destroy ***";
    else
        os << " has live sessions";
    if (isLoaded())
        os << "\n  loaded in: " << _loadDuration;
    else
        os << "\n  still loading... "
           << std::chrono::duration_cast<std::chrono::seconds>(now - _createTime);
    os << "\n  now: " << Util::getClockAsString(now);
    const int childPid = _childProcess ? _childProcess->getPid() : 0;
    os << "\n  child PID: " << childPid;
    os << "\n  sent: " << sent << " bytes";
    os << "\n  recv: " << recv << " bytes";
    os << "\n  jail id: " << _jailId;
    os << "\n  filename: " << COOLWSD::anonymizeUrl(_filename);
    os << "\n  public uri: " << _uriPublic.toString();
    os << "\n  jailed uri: " << COOLWSD::anonymizeUrl(_uriJailed);
    os << "\n  doc key: " << _docKey;
    os << "\n  doc id: " << _docId;
    os << "\n  num sessions: " << _sessions.size();
    os << "\n  createTime: " << Util::getTimeForLog(now, _createTime);
    os << "\n  stop: " << _stop;
    os << "\n  closeReason: " << _closeReason;
    os << "\n  modified?: " << isModified();
    os << "\n  possibly-modified: " << isPossiblyModified();
    os << "\n  canSave: " << name(canSaveToDisk());
    os << "\n  canUpload: " << name(canUploadToStorage());
    os << "\n  isStorageOutdated: " << isStorageOutdated();
    os << "\n  needToUpload: " << name(needToUploadToStorage());
    os << "\n  lastActivityTime: " << Util::getTimeForLog(now, _lastActivityTime);
    os << "\n  haveActivityAfterSaveRequest: " << haveActivityAfterSaveRequest();
    os << "\n  lastModifyActivityTime: " << Util::getTimeForLog(now, _lastModifyActivityTime);
    os << "\n  haveModifyActivityAfterSaveRequest: " << haveModifyActivityAfterSaveRequest();
    os << "\n  loadDuration (ms): " << _loadDuration.count();
    os << "\n  wopiDownloadDuration (ms): " << _wopiDownloadDuration.count();
    os << "\n  alwaysSaveOnExit: " << (_alwaysSaveOnExit?"true":"false");
    os << "\n  backgroundAutoSave: " << (_backgroundAutoSave?"true":"false");
    os << "\n  backgroundManualSave: " << (_backgroundManualSave?"true":"false");
    os << "\n  isViewFileExtension: " << _isViewFileExtension;
    os << "\n  Total PSS: " << ProcUtil::getProcessTreePss(ProcUtil::getProcessId()) << " KB";
    if (childPid)
        os << "\n  Doc PSS: " << ProcUtil::getProcessTreePss(childPid) << " KB";
    if constexpr (!Util::isMobileApp())
    {
        os << "\n  last quarantined version: "
           << (_quarantine && _quarantine->isEnabled() ? _quarantine->lastQuarantinedFilePath()
                                                       : "<unavailable>");
    }

    if (_limitLifeSeconds > std::chrono::seconds::zero())
        os << "\n  life limit in seconds: " << _limitLifeSeconds.count();
    os << "\n  idle time: " << getIdleTime();
    os << "\n  cursor X: " << _cursorPosX << ", Y: " << _cursorPosY << ", W: " << _cursorWidth
       << ", H: " << _cursorHeight;

    os << "\n  DocumentState:";
    _docState.dumpState(os, "\n    ");

    if (_docState.activity() == DocumentState::Activity::Rename)
        os << "\n  (new name: " << _renameFilename << ')';

    os << "\n  SaveManager:";
    _saveManager.dumpState(os, "\n    ");

    os << "\n  StorageManager:";
    _storageManager.dumpState(os, "\n    ");

    os << "\n    Last StorageAttributes:";
    _lastStorageAttrs.dumpState(os, "\n      ");
    os << "\n    Current StorageAttributes:";
    _currentStorageAttrs.dumpState(os, "\n      ");
    os << "\n    Next StorageAttributes:";
    _nextStorageAttrs.dumpState(os, "\n      ");

    os << "\n  Storage:";
    if (_storage)
        _storage->dumpState(os, "\n    ");
    else
        os << " none";

    if (_lockCtx)
    {
        os << '\n';
        _lockCtx->dumpState(os);
    }

    if (_tileCache)
    {
        os << '\n';
        _tileCache->dumpState(os);
    }

    os << '\n';
    _poll->dumpState(os);

    if constexpr (!Util::isMobileApp())
    {
        // Bit nasty - need a cleaner way to dump state.
        if (!_sessions.empty())
        {
            os << "\n  Document broker sessions [" << _sessions.size()
               << "], should duplicate the above:";
            for (const auto& it : _sessions)
            {
                auto proto = it.second->getProtocol();
                auto proxy = dynamic_cast<ProxyProtocolHandler*>(proto.get());
                if (proxy)
                    proxy->dumpProxyState(os);
                else
                    std::static_pointer_cast<MessageHandlerInterface>(it.second)->dumpState(os);
            }
        }
    }

    os << "\n End DocumentBroker [" << _docId << "] Dump\n";
}

bool DocumentBroker::isAsyncUploading() const
{
    if (!_storage)
        return false;

    StorageBase::AsyncUpload::State state = _storage->queryLocalFileToStorageAsyncUploadState().state();

    return state == StorageBase::AsyncUpload::State::Running;
}

void DocumentBroker::addEmbeddedMedia(const std::string& id, const std::string& json)
{
    LOG_TRC("Adding embeddedmedia with id [" << id << "]: " << json);

    // Store the original json with the internal, temporary, file URI.
    _embeddedMedia[id] = json;
}

void DocumentBroker::removeEmbeddedMedia(const std::string& json)
{
    Poco::JSON::Object::Ptr object;
    if (JsonUtil::parseJSON(json, object))
    {
        const std::string id = JsonUtil::getJSONValue<std::string>(object, "id");
        if (id.empty())
        {
            LOG_ERR("Invalid embeddedmedia json without id: " << json);
        }
        else
        {
            LOG_TRC("Removing embeddedmedia with id [" << id << "]: " << json);
            _embeddedMedia.erase(id);
        }
    }
}

// This is used on mobile to allow our custom URL handling to get the media path
//
// on iOS this works through CoolURLSchemeHandler.mm, which handles cool:/cool/media?Tag=... requests in much the same way as
// https://.../cool/media?Tag=... would be handled by COOLWSD on a server. As part of that, we need to get the media path from
// the tag using this function
std::string DocumentBroker::getEmbeddedMediaPath(const std::string& id)
{
    const auto it = _embeddedMedia.find(id);

    if (it == _embeddedMedia.end())
    {
        LOG_ERR("Invalid media request in Doc [" << _docId << "] with tag [" << id << ']');
        return std::string();
    }

    LOG_DBG("Media: " << it->second);
    Poco::JSON::Object::Ptr object;

    if (!JsonUtil::parseJSON(it->second, object))
    {
        LOG_ERR("Invalid media object in Doc [" << _docId << "] with tag [" << id << "] (could not parse JSON)");
        return std::string();
    }

    if (JsonUtil::getJSONValue<std::string>(object, "id") != id)
    {
        LOG_ERR("Invalid media object in Doc [" << _docId << "] with tag [" << id << "] (ID does not match search)");
        return std::string();
    }

    const std::string url = JsonUtil::getJSONValue<std::string>(object, "url");

    if (!Util::toLower(url).starts_with("file://"))
    {
        LOG_ERR("Invalid media object in Doc [" << _docId << "] with tag [" << id << "] (URL does not start with file://)");
        return std::string();
    }

    std::string localPath = url.substr(sizeof("file:///") - 1);
    return getAbsoluteMediaPath(std::move(localPath));
}

std::string DocumentBroker::getAbsoluteMediaPath(std::string localPath)
{
#if !MOBILEAPP
    // We always extract media files in /tmp. Normally, we are in jail (chroot),
    // and this would need to be accessed from WSD through the JailRoot path.
    // But, when we have NoCapsForKit there is no jail, so the media file ends
    // up in the host (AppImage) /tmp
    if (COOLWSD::NoCapsForKit)
        return "/" + localPath;

    return FileUtil::buildLocalPathToJail(COOLWSD::EnableMountNamespaces,
                                          COOLWSD::ChildRoot + _jailId, std::move(localPath));
#else // MOBILEAPP
    return getJailRoot() + "/" + localPath;
#endif // MOBILEAPP
}

void DocumentBroker::onUrpMessage(const char* data, size_t len)
{
    const auto session = getWriteableSession();
    if (session)
    {
        constexpr std::string_view header = "urp: ";
        const size_t responseSize = header.size() + len;
        std::vector<char> response(responseSize);
        std::memcpy(response.data(), header.data(), header.size());
        std::memcpy(response.data() + header.size(), data, len);
        session->sendBinaryFrame(response.data(), responseSize);
    }
}

#if !MOBILEAPP && !WASMAPP

void DocumentBroker::switchMode(const std::shared_ptr<ClientSession>& session,
                                const std::string& mode)
{
    if (mode == "online")
    {
        //TODO: Sanity check that we aren't running in WASM, otherwise we can't do anything.
        startSwitchingToOnline();
    }
    else if (mode == "offline")
    {
        // We must be in Collaborative mode.

        if (_sessions.size() > 1)
        {
            session->sendTextFrame("error: cmd=switch kind=multiviews");
            return;
        }

        startSwitchingToOffline(session);
    }
}

void DocumentBroker::startSwitchingToOffline(const std::shared_ptr<ClientSession>& session)
{
    LOG_DBG("Starting switching to Offline mode");

    if (_docState.activity() != DocumentState::Activity::None)
    {
        // It's not safe to call startActivity() while executing another.
        return;
    }

    // Transition.
    if (!startActivity(DocumentState::Activity::SwitchingToOffline))
    {
        // The issue is logged.
        return;
    }

    // Block the UI to prevent further changes and notify the user.
    blockUI("switchingtooffline");

    constexpr bool dontTerminateEdit = false; // We will save and reload: terminate.
    constexpr bool dontSaveIfUnmodified = true;
    constexpr bool isAutosave = false;
    constexpr bool finalWrite = true;
    sendUnoSave(session, dontTerminateEdit, dontSaveIfUnmodified, isAutosave, finalWrite);
}

void DocumentBroker::endSwitchingToOffline()
{
    LOG_DBG("Ending switching to Offline mode");

    unblockUI();

    endActivity();
}

void DocumentBroker::startSwitchingToOnline()
{
    LOG_DBG("Starting switching to Online mode");

    if (_docState.activity() != DocumentState::Activity::None)
    {
        // It's not safe to call startActivity() while executing another.
        return;
    }

    // Transition.
    if (!startActivity(DocumentState::Activity::SwitchingToOnline))
    {
        // The issue is logged.
        return;
    }

    // Block the UI to prevent further changes and notify the user.
    blockUI("switchingtoonline");
}

void DocumentBroker::switchToOffline()
{
    // We should end SwitchingToOffline if the conditions change.
    LOG_ASSERT_MSG(_sessions.size() == 1,
                   "Unexpected number of sessions for SwitchingToOffline in post-upload");
    LOG_DBG("Switch to Offline post uploading");

    std::shared_ptr<ClientSession> session = _sessions.begin()->second;

    RequestDetails details(session->getPublicUri().toString());
    std::string access_token;
    details.getParamByName("access_token", access_token);
    // const std::string wopiSrc = session->getPublicUri().getPath() + "?access_token=" + access_token;
    const std::string wopiSrc = session->getPublicUri().getPath();
    COOLWSD::Uri2WasmModeMap[wopiSrc] = std::chrono::steady_clock::now();

    // End activity to allow for unloading.
    endActivity();

    // We are done with this instance. The user will reconnect; don't reuse.
    _docState.markToDestroy();

    session->sendTextFrame("reload");
}

void DocumentBroker::endSwitchingToOnline()
{
    LOG_DBG("Ending switching to Online mode");

    unblockUI();

    endActivity();
}

#endif // !MOBILEAPP && !WASMAPP

// not beautiful - but neither is editing mobile project files.
#if MOBILEAPP
#  include "Exceptions.cpp"
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
