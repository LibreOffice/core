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

#include <config.h>

#include "CollabSocketHandler.hpp"
#include "CollabBroker.hpp"

#include <COOLWSD.hpp>
#include <common/Anonymizer.hpp>
#include <common/JsonUtil.hpp>
#include <Protocol.hpp>
#include <RequestDetails.hpp>
#include <SigUtil.hpp>
#include <Socket.hpp>
#include <Util.hpp>
#include <wopi/StorageConnectionManager.hpp>

#include <common/Uri.hpp>

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>

#include <sstream>

CollabSocketHandler::CollabSocketHandler(const std::shared_ptr<StreamSocket>& socket,
                                         const Poco::Net::HTTPRequest& request,
                                         bool allowedOrigin,
                                         const std::string& wopiSrc)
    : WebSocketHandler(socket, request, allowedOrigin)
    , _wopiSrc(wopiSrc)
    , _socketWeak(socket)
{
}

void CollabSocketHandler::handleMessage(const std::vector<char>& payload)
{
    if (_isValidating)
    {
        // Still validating, ignore messages
        LOG_DBG("Collab: validation in progress, ignoring message");
        return;
    }

    if (!_isAuthenticated)
    {
        const std::string msg(payload.data(), payload.size());
        static const std::string prefix = "access_token ";
        if (msg.size() > prefix.size() &&
            msg.compare(0, prefix.size(), prefix) == 0)
        {
            _accessToken = msg.substr(prefix.size());
            startValidation();
            return;
        }
        LOG_ERR("First message must be access_token, got: "
                << COOLProtocol::getAbbreviatedMessage(payload));
        sendTextMessage("error: cmd=collab kind=accesstoken");
        shutdown();
        return;
    }

    // Handle authenticated messages
    const std::string msg(payload.data(), payload.size());
    handleAuthenticatedMessage(msg);
}

void CollabSocketHandler::startValidation()
{
    _isValidating = true;

    // Send progress message
    sendTextMessage("progress: validating");

    // Build the WOPI URL with access_token
    Poco::URI wopiUrl(_wopiSrc);
    wopiUrl.addQueryParameter("access_token", _accessToken);
    const Poco::URI wopiUri = RequestDetails::sanitizeURI(wopiUrl.toString());
    LOG_INF("Collab: starting CheckFileInfo validation for: "
            << Anonymizer::anonymizeUrl(wopiUri.toString()));

    // Store weak reference to this handler via ProtocolHandlerInterface base
    std::weak_ptr<ProtocolHandlerInterface> handlerWeak = shared_from_this();

    // Create callback to handle validation result
    auto onFinish = [handlerWeak](CheckFileInfo& cfi) {
        auto handlerBase = handlerWeak.lock();
        if (!handlerBase)
            return; // Handler was destroyed during validation

        // Safe downcast since we know the type
        CollabSocketHandler* handler = static_cast<CollabSocketHandler*>(handlerBase.get());
        handler->onCheckFileInfoFinished(cfi);
    };

    // Create and start CheckFileInfo request
    _checkFileInfo = std::make_shared<CheckFileInfo>(
        COOLWSD::getWebServerPoll(), wopiUri, std::move(onFinish));
    _checkFileInfo->checkFileInfo(/* redirectionLimit */ 5);
}

void CollabSocketHandler::onCheckFileInfoFinished(CheckFileInfo& cfi)
{
    _isValidating = false;

    const CheckFileInfo::State state = cfi.state();
    LOG_INF("Collab: CheckFileInfo finished with state: " << CheckFileInfo::name(state)
            << " for: " << Anonymizer::anonymizeUrl(_wopiSrc));

    switch (state)
    {
        case CheckFileInfo::State::Pass:
        {
            // Store the CheckFileInfo data
            _docKey = cfi.docKey();
            _wopiInfo = cfi.wopiInfo();

            // Extract commonly-needed fields from WOPI info
            if (_wopiInfo)
            {
                _userId = _wopiInfo->optValue<std::string>("UserId", std::string());
                _username = _wopiInfo->optValue<std::string>("UserFriendlyName", std::string());
                _userCanWrite = _wopiInfo->optValue<bool>("UserCanWrite", false);
            }

            // Find or create the CollabBroker for this document
            auto broker = findOrCreateCollabBroker(_docKey, _wopiSrc);
            if (broker)
            {
                _broker = broker;

                // Share WOPI info with the broker (first one wins)
                broker->setWopiInfo(_wopiInfo);

                // Get shared_ptr to this handler
                auto self = std::dynamic_pointer_cast<CollabSocketHandler>(shared_from_this());
                if (self)
                {
                    // Send list of existing users to the new user (before adding)
                    const std::string userList = broker->getUserListJson(self);
                    sendTextMessage(userList);

                    // Register this handler with the broker
                    broker->addHandler(self);

                    // Notify other users that this user joined
                    broker->notifyUserJoined(self);
                }
            }

            _isAuthenticated = true;
            LOG_INF("Collab session authenticated for WOPISrc: "
                    << Anonymizer::anonymizeUrl(_wopiSrc)
                    << ", docKey: " << _docKey
                    << ", userId: " << Anonymizer::anonymize(_userId)
                    << ", username: " << Anonymizer::anonymize(_username)
                    << ", canWrite: " << _userCanWrite);
            sendTextMessage("{\"type\":\"authenticated\"}");
            break;
        }
        case CheckFileInfo::State::Unauthorized:
        {
            LOG_ERR("Collab: access denied for WOPISrc: "
                    << Anonymizer::anonymizeUrl(_wopiSrc));
            std::string error = "error: cmd=internal kind=unauthorized";
            std::string sslMsg = cfi.getSslVerifyMessage();
            if (!sslMsg.empty())
                error += " code=" + Util::base64Encode(sslMsg);
            sendTextMessage(error);
            shutdown();
            break;
        }
        case CheckFileInfo::State::Timedout:
        {
            LOG_ERR("Collab: CheckFileInfo timed out for WOPISrc: "
                    << Anonymizer::anonymizeUrl(_wopiSrc));
            sendTextMessage("error: cmd=internal kind=timeout");
            shutdown();
            break;
        }
        case CheckFileInfo::State::Fail:
        default:
        {
            LOG_ERR("Collab: CheckFileInfo failed for WOPISrc: "
                    << Anonymizer::anonymizeUrl(_wopiSrc));
            sendTextMessage("error: cmd=storage kind=loadfailed");
            shutdown();
            break;
        }
    }

    // Clear the CheckFileInfo reference
    _checkFileInfo.reset();
}

void CollabSocketHandler::onDisconnect()
{
    LOG_INF("Collab: handler disconnected for WOPISrc: " << Anonymizer::anonymizeUrl(_wopiSrc));

    // Unregister from the broker and notify other users
    if (auto broker = _broker.lock())
    {
        auto self = std::dynamic_pointer_cast<CollabSocketHandler>(shared_from_this());
        if (self)
        {
            // Notify other users before removing
            broker->notifyUserLeft(self);
            broker->removeHandler(self);
        }
    }

    // Call base class
    WebSocketHandler::onDisconnect();
}

void CollabSocketHandler::handleAuthenticatedMessage(const std::string& msg)
{
    LOG_DBG("Collab: handling authenticated message: "
            << COOLProtocol::getAbbreviatedMessage(msg));

    // Parse JSON message
    Poco::JSON::Object::Ptr json;
    try
    {
        Poco::JSON::Parser parser;
        auto result = parser.parse(msg);
        json = result.extract<Poco::JSON::Object::Ptr>();
    }
    catch (const std::exception& ex)
    {
        LOG_ERR("Collab: failed to parse JSON message: " << ex.what());
        sendTextMessage("{\"type\":\"error\",\"error\":\"Invalid JSON\"}");
        return;
    }

    if (!json)
    {
        sendTextMessage("{\"type\":\"error\",\"error\":\"Invalid JSON object\"}");
        return;
    }

    const std::string type = json->optValue<std::string>("type", std::string());
    const std::string requestId = json->optValue<std::string>("requestId", std::string());

    if (type == "fetch")
    {
        const std::string stream = json->optValue<std::string>("stream", "contents");
        const std::string ifNoneMatch = json->optValue<std::string>("ifNoneMatch", std::string());
        const std::string ifModifiedSince = json->optValue<std::string>("ifModifiedSince", std::string());
        handleFetch(stream, requestId, ifNoneMatch, ifModifiedSince);
    }
    else if (type == "upload")
    {
        const std::string stream = json->optValue<std::string>("stream", "contents");
        handleUpload(stream, requestId);
    }
    else
    {
        LOG_WRN("Collab: unknown message type: " << type);
        std::ostringstream oss;
        oss << "{\"type\":\"error\",\"error\":\"Unknown message type\""
            << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"}";
        sendTextMessage(oss.str());
    }
}

void CollabSocketHandler::handleFetch(const std::string& stream, const std::string& requestId,
                                       const std::string& /* ifNoneMatch */,
                                       const std::string& /* ifModifiedSince */)
{
    LOG_INF("Collab: handling fetch request for stream: " << stream
            << ", requestId: " << requestId);

    if (SigUtil::getShutdownRequestFlag())
    {
        LOG_WRN("Collab: shutdown in progress, rejecting fetch");
        std::ostringstream oss;
        oss << "{\"type\":\"fetch_error\",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
        if (!requestId.empty())
            oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
        oss << ",\"error\":\"Server shutting down\"}";
        sendTextMessage(oss.str());
        return;
    }

    // Build the URL for the requested stream
    std::string url;
    if (stream == "contents")
    {
        // Use FileUrl if available, otherwise construct /contents URL
        if (_wopiInfo)
        {
            url = _wopiInfo->optValue<std::string>("FileUrl", std::string());
        }
        if (url.empty())
        {
            // Construct WOPI /contents URL; _wopiSrc may already contain
            // query parameters, so use Poco::URI to modify only the path.
            Poco::URI contentsUri(_wopiSrc);
            contentsUri.setPath(contentsUri.getPath() + "/contents");
            contentsUri.addQueryParameter("access_token", _accessToken);
            url = contentsUri.toString();
        }
    }
    else if (stream == "userSettings" && _wopiInfo)
    {
        url = _wopiInfo->optValue<std::string>("UserSettingsUri", std::string());
    }
    else if (stream == "templateSource" && _wopiInfo)
    {
        url = _wopiInfo->optValue<std::string>("TemplateSource", std::string());
    }
    else if (stream == "fileUrl" && _wopiInfo)
    {
        url = _wopiInfo->optValue<std::string>("FileUrl", std::string());
    }

    if (url.empty())
    {
        LOG_WRN("Collab: stream not available: " << stream);
        std::ostringstream oss;
        oss << "{\"type\":\"fetch_error\",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
        if (!requestId.empty())
            oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
        oss << ",\"error\":\"Stream not available\"}";
        sendTextMessage(oss.str());
        return;
    }

    // Get the broker's access token for secure validation
    auto broker = _broker.lock();
    if (!broker)
    {
        LOG_ERR("Collab: broker no longer available for fetch");
        std::ostringstream oss;
        oss << "{\"type\":\"fetch_error\",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
        if (!requestId.empty())
            oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
        oss << ",\"error\":\"Session expired\"}";
        sendTextMessage(oss.str());
        return;
    }

    const std::string brokerTag = broker->getCurrentAccessToken();

    // Create a fetch token that the client can use to download via HTTP
    const std::string token = createCollabFetchRequest(url, _accessToken, _wopiSrc, _docKey,
                                                        brokerTag, requestId, stream);

    // Build the download URL
    std::ostringstream downloadUrl;
    downloadUrl << "/co/collab/fetch?token=" << token;

    // Send the download URL to the client
    std::ostringstream oss;
    oss << "{\"type\":\"fetch_url\"";
    oss << ",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
    if (!requestId.empty())
        oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
    oss << ",\"url\":\"" << JsonUtil::escapeJSONValue(downloadUrl.str()) << "\"";
    if (stream == "contents" && _wopiInfo)
    {
        const std::string filename =
            _wopiInfo->optValue<std::string>("BaseFileName", std::string());
        if (!filename.empty())
            oss << ",\"filename\":\"" << JsonUtil::escapeJSONValue(filename) << "\"";
    }
    oss << "}";

    sendTextMessage(oss.str());
    LOG_DBG("Collab: sent fetch URL for stream: " << stream << ", token: " << token);
}

void CollabSocketHandler::handleUpload(const std::string& stream, const std::string& requestId)
{
    LOG_INF("Collab: handling upload request for stream: " << stream
            << ", requestId: " << requestId);

    if (SigUtil::getShutdownRequestFlag())
    {
        LOG_WRN("Collab: shutdown in progress, rejecting upload");
        std::ostringstream oss;
        oss << "{\"type\":\"upload_error\",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
        if (!requestId.empty())
            oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
        oss << ",\"error\":\"Server shutting down\"}";
        sendTextMessage(oss.str());
        return;
    }

    if (stream != "contents")
    {
        LOG_WRN("Collab: upload only supported for 'contents' stream, got: " << stream);
        std::ostringstream oss;
        oss << "{\"type\":\"upload_error\",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
        if (!requestId.empty())
            oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
        oss << ",\"error\":\"Upload only supported for contents stream\"}";
        sendTextMessage(oss.str());
        return;
    }

    // Build the target URL by appending /contents to the WOPISrc path.
    // _wopiSrc may already contain query parameters (access_token etc.),
    // so use Poco::URI to modify only the path component.
    Poco::URI targetUri(_wopiSrc);
    targetUri.setPath(targetUri.getPath() + "/contents");
    const std::string targetUrl = targetUri.toString();

    auto broker = _broker.lock();
    if (!broker)
    {
        LOG_ERR("Collab: broker no longer available for upload");
        std::ostringstream oss;
        oss << "{\"type\":\"upload_error\",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
        if (!requestId.empty())
            oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
        oss << ",\"error\":\"Session expired\"}";
        sendTextMessage(oss.str());
        return;
    }

    const std::string brokerTag = broker->getCurrentAccessToken();

    // Create an upload token that the client can use to POST via HTTP
    const std::string token = createCollabUploadRequest(targetUrl, _accessToken, _wopiSrc,
                                                         _docKey, brokerTag, requestId);

    // Send the upload URL to the client
    std::ostringstream oss;
    oss << "{\"type\":\"upload_url\"";
    oss << ",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
    if (!requestId.empty())
        oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
    oss << ",\"url\":\"/co/collab/put?token=" << token << "\"";
    oss << "}";

    sendTextMessage(oss.str());
    LOG_DBG("Collab: sent upload URL for stream: " << stream << ", token: " << token);
}

void CollabSocketHandler::onFetchComplete(const std::string& /* requestId */,
                                           const std::string& /* stream */,
                                           const std::shared_ptr<http::Session>& /* session */)
{
    // No longer used - fetch is now via HTTP download URL
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
