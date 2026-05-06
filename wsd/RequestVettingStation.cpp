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
 * Implementation of request vetting and validation.
 * Classes: RequestVettingStation
 */

#include <config.h>

#include "RequestVettingStation.hpp"

#include <common/Anonymizer.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/TraceEvent.hpp>
#include <common/Util.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/CacheUtil.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/Exceptions.hpp>
#include <wsd/RequestDetails.hpp>
#include <wsd/ServerAuditUtil.hpp>
#include <wsd/Storage.hpp>

#if !MOBILEAPP
#include <common/JailUtil.hpp>
#include <wopi/CheckFileInfo.hpp>
#endif // !MOBILEAPP

extern std::pair<std::shared_ptr<DocumentBroker>, std::string>
findOrCreateDocBroker(DocumentBroker::ChildType type, const std::string& uri,
                      const std::string& docKey, const std::string& configId,
                      const std::string& id, const Poco::URI& uriPublic,
                      unsigned mobileAppDocId);

namespace
{
void sendLoadResult(const std::shared_ptr<ClientSession>& clientSession, bool success,
                    const std::string& errorMsg)
{
    const std::string result = success ? "" : "Error while loading document";
    const std::string resultstr = success ? "true" : "false";
    // Some sane limit, otherwise we get problems transferring this
    // to the client with large strings (can be a whole webpage)
    // Replace reserved characters before sending.
    std::string errorMsgFormatted = COOLProtocol::getAbbreviatedMessage(errorMsg);
    Util::replaceInPlace(errorMsgFormatted, '"', '\'');
    clientSession->sendTextFrame(
        "commandresult: { \"command\": \"load\", \"success\": " + resultstr + ", \"result\": \"" +
        result + "\", \"errorMsg\": \"" + errorMsgFormatted + "\"}");
}

} // anonymous namespace

void RequestVettingStation::handleRequest(const std::string& id)
{
    _id = id;

    const std::string url = _requestDetails.getDocumentURI();

    const auto uriPublic = RequestDetails::sanitizeURI(url);
    const auto docKey = RequestDetails::getDocKey(uriPublic);
    const std::string fileId = Uri::getFilenameFromURL(Uri::decode(docKey));
    Anonymizer::mapAnonymized(fileId,
                              fileId); // Identity mapping, since fileId is already obfuscated

    // Check if readonly session is required.
    const bool isReadOnly = Uri::hasReadonlyPermission(uriPublic.toString());

    LOG_INF("URL [" << COOLWSD::anonymizeUrl(url)
                    << "] will be proactively vetted. Sanitized uriPublic: ["
                    << COOLWSD::anonymizeUrl(uriPublic.toString()) << "], docKey: [" << docKey
                    << "], session: [" << _id << "], fileId: [" << fileId << "] "
                    << (isReadOnly ? "(readonly)" : "(writable)"));

    // Before we create DocBroker with a SocketPoll thread, a ClientSession, and a Kit process,
    // we need to vet this request by invoking CheckFileInfo.
    // For that, we need the storage settings to create a connection.
    const StorageBase::StorageType storageType =
        StorageBase::validate(uriPublic, /*takeOwnership=*/false);
    switch (storageType)
    {
        case StorageBase::StorageType::Unsupported:
            LOG_ERR("Unsupported URI [" << COOLWSD::anonymizeUrl(uriPublic.toString())
                                        << "] or no storage configured");
            throw BadRequestException("No Storage configured or invalid URI " +
                                      COOLWSD::anonymizeUrl(uriPublic.toString()) + ']');

            break;
        case StorageBase::StorageType::Unauthorized:
            LOG_ERR("No authorized hosts found matching the target host [" << uriPublic.getHost()
                                                                           << "] in config");
            sendUnauthorizedErrorAndShutdown();
            break;

        case StorageBase::StorageType::Conversion:
            LOG_INF("URI [" << COOLWSD::anonymizeUrl(uriPublic.toString()) << "] on docKey ["
                            << docKey << "] is for a document conversion");
            break;

#if ENABLE_LOCAL_FILESYSTEM
        case StorageBase::StorageType::FileSystem:
            LOG_INF("URI [" << COOLWSD::anonymizeUrl(uriPublic.toString()) << "] on docKey ["
                            << docKey << "] is for a FileSystem document");
            break;
#endif // ENABLE_LOCAL_FILESYSTEM

#if !MOBILEAPP
        case StorageBase::StorageType::Wopi:
            LOG_INF("URI [" << COOLWSD::anonymizeUrl(uriPublic.toString()) << "] on docKey ["
                            << docKey << "] is for a WOPI document");

            // CheckFileInfo asynchronously.
            checkFileInfo(uriPublic, HTTP_REDIRECTION_LIMIT);
            break;
#endif //!MOBILEAPP
    }
}

void RequestVettingStation::sendUnauthorizedErrorAndShutdown()
{
    std::string error = "error: cmd=internal kind=unauthorized";

#if !MOBILEAPP
    if (_checkFileInfo)
    {
        std::string sslVerifyResult = _checkFileInfo->getSslVerifyMessage();
        if (!sslVerifyResult.empty())
            error += " code=" + Util::base64Encode(sslVerifyResult);
    }
#endif
    sendErrorAndShutdown(error, WebSocketHandler::StatusCodes::POLICY_VIOLATION);
}

#if !MOBILEAPP

namespace
{

class SharedSettings
{
public:
    explicit SharedSettings(const Poco::JSON::Object::Ptr& wopiInfo)
    {
        if (auto settingsJSON = wopiInfo->getObject("SharedSettings"))
        {
            JsonUtil::findJSONValue(settingsJSON, "uri", _uri);
            _configId = "shared-" + Cache::getConfigId(_uri);

            std::string stamp;
            JsonUtil::findJSONValue(settingsJSON, "stamp", stamp);
            if (!stamp.empty())
                _configId.append("-").append(stamp);
        }
    }

    const std::string& getConfigId() const
    {
        return _configId;
    }

    const std::string& getUri() const
    {
        return _uri;
    }

private:
    std::string _uri;
    std::string _configId;
};

}

void RequestVettingStation::launchInstallPresets()
{
    SharedSettings sharedSettings(_checkFileInfo->wopiInfo());
    if (sharedSettings.getUri().empty())
        return;

    const std::string& configId = sharedSettings.getConfigId();

    auto finishedCallback = [selfWeak = weak_from_this(), this, configId](bool success)
    {
        std::shared_ptr<RequestVettingStation> selfLifecycle = selfWeak.lock();
        if (!selfLifecycle)
            return;

        if (!success)
        {
            LOG_ERR("Failed to install config [" << configId << "]");
            sendErrorAndShutdown("shared config install failed",
                                 WebSocketHandler::StatusCodes::UNEXPECTED_CONDITION);
        }
        else
        {
            COOLWSD::ensureSubForKit(configId);
        }
        _asyncInstallTask.reset();
    };

    // if this wopi server has some shared settings we want to have a subForKit for those settings
    std::string presetsPath = Poco::Path(COOLWSD::ChildRoot, JailUtil::CHILDROOT_TMP_SHARED_PRESETS_PATH).toString();
    std::string configIdPresets = Poco::Path(presetsPath, Uri::encode(configId)).toString();
    Poco::File(Poco::Path(configIdPresets, "autotext")).createDirectories();
    Poco::File(Poco::Path(configIdPresets, "wordbook")).createDirectories();
    Poco::File(Poco::Path(configIdPresets, "template")).createDirectories();
    // ensure the server config is downloaded and populate a subforkit when config is available
    _asyncInstallTask = DocumentBroker::asyncInstallPresets(_poll, configId, sharedSettings.getUri(), configIdPresets,
                                                            nullptr, finishedCallback);
}

#endif

void RequestVettingStation::handleRequest(const std::string& id,
                                          const RequestDetails& requestDetails,
                                          const std::shared_ptr<WebSocketHandler>& ws,
                                          const std::shared_ptr<StreamSocket>& socket,
                                          unsigned mobileAppDocId, SocketDisposition& /*disposition*/)
{
    _id = id;
    _requestDetails = requestDetails;
    _ws = ws;
    _socket = socket;
    _mobileAppDocId = mobileAppDocId;

    std::string url = _requestDetails.getDocumentURI();

    const auto uriPublic = RequestDetails::sanitizeURI(url);
    std::string docKey = RequestDetails::getDocKey(uriPublic);
    const std::string fileId = Uri::getFilenameFromURL(Uri::decode(docKey));
    Anonymizer::mapAnonymized(fileId,
                              fileId); // Identity mapping, since fileId is already obfuscated

    // Check if readonly session is required.
    const bool isReadOnly = Uri::hasReadonlyPermission(uriPublic.toString());

    LOG_INF("URL [" << COOLWSD::anonymizeUrl(url) << "] for WS Request. Sanitized uriPublic: ["
                    << COOLWSD::anonymizeUrl(uriPublic.toString()) << "], docKey: [" << docKey
                    << "], session: [" << _id << "], fileId: [" << fileId << "] "
                    << (isReadOnly ? "(readonly)" : "(writable)"));

    // Before we create DocBroker with a SocketPoll thread, a ClientSession, and a Kit process,
    // we need to vet this request by invoking CheckFileInfo.
    // For that, we need the storage settings to create a connection.
    const StorageBase::StorageType storageType =
        StorageBase::validate(uriPublic, /*takeOwnership=*/false);
    switch (storageType)
    {
        case StorageBase::StorageType::Unsupported:
            LOG_ERR("Unsupported URI [" << COOLWSD::anonymizeUrl(uriPublic.toString())
                                        << "] or no storage configured");
            throw BadRequestException("No Storage configured or invalid URI " +
                                      COOLWSD::anonymizeUrl(uriPublic.toString()) + ']');

            break;
        case StorageBase::StorageType::Unauthorized:
            LOG_ERR("No authorized hosts found matching the target host [" << uriPublic.getHost()
                                                                           << "] in config");
            sendUnauthorizedErrorAndShutdown();
            break;

        case StorageBase::StorageType::Conversion:
            LOG_INF("URI [" << COOLWSD::anonymizeUrl(uriPublic.toString()) << "] on docKey ["
                            << docKey << "] is for a document conversion");

            LOG_TRC("Dissociating client socket from "
                    "ClientRequestDispatcher and creating DocBroker for ["
                    << docKey << ']');

            // Create the DocBroker.
            if (std::shared_ptr<DocumentBroker> docBroker =
                    createDocBroker(docKey, "", url, uriPublic))
            {
                createClientSession(docBroker, docKey, url, uriPublic);
            }
            break;

#if ENABLE_LOCAL_FILESYSTEM
        case StorageBase::StorageType::FileSystem:
            LOG_INF("URI [" << COOLWSD::anonymizeUrl(uriPublic.toString()) << "] on docKey ["
                            << docKey << "] is for a FileSystem document");

            LOG_TRC("Dissociating client socket from "
                             "ClientRequestDispatcher and creating DocBroker for ["
                          << docKey << ']');

            // Create the DocBroker.
            if (std::shared_ptr<DocumentBroker> docBroker = createDocBroker(docKey, "",
                        url, uriPublic))
            {
                createClientSession(docBroker, docKey, url, uriPublic);
            }
            break;
#endif // ENABLE_LOCAL_FILESYSTEM

#if !MOBILEAPP
        case StorageBase::StorageType::Wopi:
            LOG_INF("URI [" << COOLWSD::anonymizeUrl(uriPublic.toString()) << "] on docKey ["
                            << docKey << "] is for a WOPI document");
            // Remove from the current poll and transfer.
            LOG_TRC("Dissociating client socket from "
                             "ClientRequestDispatcher and invoking CheckFileInfo for ["
                          << docKey << "], "
                          << (_checkFileInfo ? CheckFileInfo::name(_checkFileInfo->state())
                                             : "no CheckFileInfo"));

            // CheckFileInfo and only when it's good create DocBroker.
            if (_checkFileInfo && _checkFileInfo->state() == CheckFileInfo::State::Active)
            {
                // Wait for CheckFileInfo result.
                LOG_DBG("CheckFileInfo request is in progress. Will resume when done");
            }
            else if (_checkFileInfo &&
                     _checkFileInfo->state() == CheckFileInfo::State::Pass &&
                     _checkFileInfo->wopiInfo())
            {
                SharedSettings sharedSettings(_checkFileInfo->wopiInfo());
                transferToDocBroker(_checkFileInfo->url().toString(),
                                    sharedSettings.getConfigId(),
                                    _checkFileInfo->getSslVerifyMessage());
            }
            else if (_checkFileInfo == nullptr ||
                     _checkFileInfo->state() == CheckFileInfo::State::None ||
                     _checkFileInfo->state() == CheckFileInfo::State::Timedout)
            {
                // We haven't tried or we timed-out. Retry.
                _checkFileInfo.reset();
                checkFileInfo(uriPublic, HTTP_REDIRECTION_LIMIT);
            }
            else
            {
                // We had a response, but it was empty/error. Meaning the user is unauthorized.
                assert(_checkFileInfo && !_checkFileInfo->wopiInfo() &&
                       "Unexpected to have wopiInfo");

                LOG_ERR("CheckFileInfo failed for [" << docKey
                          << "], "
                          << (_checkFileInfo ? CheckFileInfo::name(_checkFileInfo->state())
                                             : "no CheckFileInfo"));

                sendUnauthorizedErrorAndShutdown();
            }
            break;
#endif //!MOBILEAPP
    }
}

#if !MOBILEAPP
void RequestVettingStation::transferToDocBroker(const std::string& url,
                                                const std::string& configId,
                                                const std::string& sslVerifyResult)
{
    // The final URL might be different due to redirection.
    const auto uriPublic = RequestDetails::sanitizeURI(url);
    const auto docKey = RequestDetails::getDocKey(uriPublic);
    LOG_DBG("WOPI::CheckFileInfo succeeded and will create DocBroker ["
            << docKey << "] now with URL: [" << url << ']');
    if (std::shared_ptr<DocumentBroker> docBroker = createDocBroker(docKey, configId, url, uriPublic))
    {
        launchInstallPresets();
        if (_ws)
        {
            // If we don't have the WebSocket, defer creating the client session.
            createClientSession(docBroker, docKey, url, uriPublic);
        }
        else
        {
            LOG_DBG("WOPI::CheckFileInfo succeeded but we don't have the client's "
                    "WebSocket yet. Deferring the ClientSession creation.");
        }

        // If there is anything dubious about the ssl connection provide a
        // warning about that.
        if (!sslVerifyResult.empty())
        {
            LOG_WRN("SSL verification warning: '" << sslVerifyResult << "' seen on CheckFileInfo for ["
                    << docKey << "]");
#if !WASMAPP
            docBroker->setCertAuditWarning();
#endif
        }
    }
}

void RequestVettingStation::checkFileInfo(const Poco::URI& uri, int redirectLimit)
{
    auto cfiContinuation = [this](CheckFileInfo& checkFileInfo)
    {
        _checkFileInfoEnd = std::chrono::steady_clock::now();
        assert(&checkFileInfo == _checkFileInfo.get() && "Unknown CheckFileInfo instance");
        if (_checkFileInfo && _checkFileInfo->state() == CheckFileInfo::State::Pass &&
            _checkFileInfo->wopiInfo())
        {
            SharedSettings sharedSettings(_checkFileInfo->wopiInfo());
            transferToDocBroker(checkFileInfo.url().toString(),
                                sharedSettings.getConfigId(),
                                checkFileInfo.getSslVerifyMessage());
        }
        else
        {
            if (_ws)
            {
                LOG_DBG("WOPI::CheckFileInfo failed, sending error and closing connection now");
                sendUnauthorizedErrorAndShutdown();
            }
            else
            {
                LOG_DBG("WOPI::CheckFileInfo failed but no client WebSocket to send error to");
            }
        }
    };

    // CheckFileInfo asynchronously.
    assert(_checkFileInfo == nullptr);
    _checkFileInfo = std::make_shared<CheckFileInfo>(_poll, uri, std::move(cfiContinuation));
    _checkFileInfoStart = std::chrono::steady_clock::now();
    _checkFileInfo->checkFileInfo(redirectLimit);
}
#endif //!MOBILEAPP

std::shared_ptr<DocumentBroker> RequestVettingStation::createDocBroker(
        const std::string& docKey, const std::string& configId,
        const std::string& url, const Poco::URI& uriPublic)
{
    // Request a kit process for this doc.
    auto [docBroker, error] =
        findOrCreateDocBroker(DocumentBroker::ChildType::Interactive, url, docKey,
                              configId, _id, uriPublic, _mobileAppDocId);

    if (docBroker)
    {
        docBroker->loadTimings().record("wopiPostReceived", _wopiPostReceived);
        if (_checkFileInfoStart.time_since_epoch().count() != 0)
            docBroker->loadTimings().record("checkFileInfoStart", _checkFileInfoStart);
        if (_checkFileInfoEnd.time_since_epoch().count() != 0)
            docBroker->loadTimings().record("checkFileInfoEnd", _checkFileInfoEnd);

        // Indicate to the client that we're connecting to the docbroker.
        if (_ws)
        {
            static constexpr std::string_view statusConnect = "progress: { \"id\":\"connect\" }";
            LOG_TRC("Sending to Client [" << statusConnect << ']');
            _ws->sendTextMessage(statusConnect);
        }

        LOG_DBG("DocBroker [" << docKey << "] acquired for [" << url << ']');
        return docBroker;
    }

    // Failed.
    LOG_ERR("Failed to create DocBroker [" << docKey << "]: " << error);
    sendErrorAndShutdown(error, WebSocketHandler::StatusCodes::UNEXPECTED_CONDITION);

    return nullptr;
}

namespace
{
void sendErrorAndShutdownWS(const std::shared_ptr<WebSocketHandler>& ws, const std::string_view msg,
                            WebSocketHandler::StatusCodes statusCode)
{
    if (ws)
    {
        ws->sendTextMessage(msg);
        ws->shutdown(statusCode, msg); // And ignore input (done in shutdown()).
    }
}
} // namespace

void RequestVettingStation::createClientSession(const std::shared_ptr<DocumentBroker>& docBroker,
                                                const std::string& docKey, const std::string& url,
                                                const Poco::URI& uriPublic)
{
    assert(docBroker && "Must have DocBroker");
    assert(_ws && "Must have WebSocket");

    if (docBroker->isUnloadingUnrecoverably())
    {
        LOG_INF("Cannot create client session to DocBroker ["
                << docKey << "] while it's unloading unrecoverably");
        sendErrorAndShutdown("error: cmd=load kind=docunloading",
                             WebSocketHandler::StatusCodes::UNEXPECTED_CONDITION);
        return;
    }

    std::unique_ptr<WopiStorage::WOPIFileInfo> realWopiFileInfo;
#if !MOBILEAPP
    assert((!_checkFileInfo || _checkFileInfo->wopiInfo()) &&
           "Must have WopiInfo when CheckFileInfo exists");
    realWopiFileInfo = _checkFileInfo ? _checkFileInfo->wopiFileInfo(uriPublic) : nullptr;
#endif // !MOBILEAPP

    // std::unique_ptr is not copyable, so cannot be captured in a std::function-wrapped lambda.
    std::shared_ptr<std::unique_ptr<WopiStorage::WOPIFileInfo>> wopiFileInfo =
        std::make_shared<std::unique_ptr<WopiStorage::WOPIFileInfo>>(std::move(realWopiFileInfo));

    std::weak_ptr<StreamSocket> socket = _socket;
    _socket.reset();

    const auto docBrokerPoll = docBroker->getPoll().lock();
    assert(docBrokerPoll && "Must have DocBroker SocketPoll");

    LOG_TRC("Transfering DocBroker [" << docKey << "] from vetting station to own thread ["
                                      << docBrokerPoll->name() << ']');

    // Transfer the client socket to the DocumentBroker when we get back to the poll:
    std::shared_ptr<WebSocketHandler> ws = _ws;
    docBroker->setupTransfer(*_poll, socket,
        [wopiFileInfo = std::move(wopiFileInfo), ws = std::move(ws), id = _id,
         requestDetails = _requestDetails, docBroker, docKey, url, uriPublic,
         selfLifecycle = shared_from_this()](const std::shared_ptr<Socket>& moveSocket)
        {
            try
            {
                LOG_DBG_S("Transfering docBroker [" << docBroker->getDocKey() << ']');

                auto streamSocket = std::static_pointer_cast<StreamSocket>(moveSocket);

                // Set WebSocketHandler's socket after its construction for shared_ptr goodness.
                // Note: this replaces ClientRequestDispatcher, which owns us.
                streamSocket->setHandler(ws);

                std::string logPrefix = '#' + std::to_string(moveSocket->getFD()) + ' ';

                const bool isReadOnly = Uri::hasReadonlyPermission(uriPublic.toString());
                std::shared_ptr<ClientSession> clientSession = docBroker->createNewClientSession(
                    ws, id, uriPublic, isReadOnly, requestDetails);
                if (!clientSession)
                {
                    // createNewClientSession() has sent the error to the client WebSocket.
                    LOG_ERR_S(logPrefix << "Failed to create Client Session [" << id
                                        << "] on docKey [" << docKey << ']');
                    return;
                }

                LOG_DBG_S(logPrefix << "handler is " << clientSession->getName());

                LOG_DBG_S(logPrefix << "ClientSession [" << clientSession->getName() << "] for ["
                                    << docKey << "] acquired for [" << url << ']');

                // Add and load the session.
                // Will download synchronously, but in own docBroker thread.
                docBroker->addSession(clientSession, std::move(*wopiFileInfo));

                COOLWSD::checkDiskSpaceAndWarnClients(true);
                // Users of development versions get just an info
                // when reaching max documents or connections
                COOLWSD::checkSessionLimitsAndWarnClients();

                sendLoadResult(clientSession, /*success=*/true, /*errorMsg=*/std::string());
            }
            catch (const UnauthorizedRequestException& exc)
            {
                LOG_ERR_S("Unauthorized Request while starting session on "
                          << docBroker->getDocKey() << " for socket #" << moveSocket->getFD()
                          << ". Terminating connection. Error: " << exc.what());
                sendErrorAndShutdownWS(ws, "error: cmd=internal kind=unauthorized",
                                       WebSocketHandler::StatusCodes::POLICY_VIOLATION);
            }
            catch (const StorageConnectionException& exc)
            {
                LOG_ERR_S("Storage error while starting session on "
                          << docBroker->getDocKey() << " for socket #" << moveSocket->getFD()
                          << ". Terminating connection. Error: " << exc.what());
                sendErrorAndShutdownWS(ws, "error: cmd=storage kind=loadfailed",
                                       WebSocketHandler::StatusCodes::POLICY_VIOLATION);
            }
            catch (const StorageSpaceLowException& exc)
            {
                LOG_ERR_S("Disk-Full error while starting session on "
                          << docBroker->getDocKey() << " for socket #" << moveSocket->getFD()
                          << ". Terminating connection. Error: " << exc.what());
                sendErrorAndShutdownWS(ws, "error: cmd=internal kind=diskfull",
                                       WebSocketHandler::StatusCodes::UNEXPECTED_CONDITION);
            }
            catch (const std::exception& exc)
            {
                LOG_ERR_S("Error while starting session on "
                          << docBroker->getDocKey() << " for socket #" << moveSocket->getFD()
                          << ". Terminating connection. Error: " << exc.what());
                sendErrorAndShutdownWS(ws, "error: cmd=storage kind=loadfailed",
                                       WebSocketHandler::StatusCodes::POLICY_VIOLATION);
            }
        });
}

void RequestVettingStation::sendErrorAndShutdown(const std::string_view msg,
                                                 WebSocketHandler::StatusCodes statusCode)
{
    sendErrorAndShutdownWS(_ws, msg, statusCode);
    // abandon responsibility for _socket now
    _socket.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
