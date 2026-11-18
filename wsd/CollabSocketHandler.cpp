/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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
#include "DocumentBroker.hpp"

#include <wsd/COOLWSD.hpp>
#include <wsd/Storage.hpp>
#include <common/Anonymizer.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/SettingsSecrets.hpp>
#include <Protocol.hpp>
#include <wsd/RequestDetails.hpp>
#include <SigUtil.hpp>
#include <wsd/SharedSettings.hpp>
#include <Socket.hpp>
#include <Util.hpp>
#include <wsd/wopi/StorageConnectionManager.hpp>
#include <wsd/wopi/WopiStorage.hpp>

#include <common/Uri.hpp>

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>

#include <charconv>
#include <optional>
#include <sstream>

namespace {

/// The name of the fetch stream that gives one file of a settings list.
const std::string SettingsFileStream = "settingsFile";

/// The group of a settings list whose files that stream gives.
const std::string TemplateGroup = "template";

// Whether this server can speak to a client, given the client's "protocol" query parameter (the
// number of the protocol that the client speaks, which only a broken client leaves out or garbles):
bool acceptsProtocol(Poco::Net::HTTPRequest const & request) {
    std::optional<std::string> parameter;
    for (auto const & [name, value]: Poco::URI(request.getURI()).getQueryParameters()) {
        if (name == "protocol") {
            parameter = value;
            break;
        }
    }
    if (!parameter) {
        LOG_WRN("Collab: the client gives no protocol number");
        return false;
    }
    int number;
    auto const [end, error]
        = std::from_chars(parameter->data(), parameter->data() + parameter->size(), number);
    if (error != std::errc() || end != parameter->data() + parameter->size()) {
        LOG_WRN("Collab: the client's protocol number [" << *parameter << "] is not readable");
        return false;
    }
    if (number < CollabSocketHandler::Protocol) {
        LOG_WRN("Collab: the client's protocol number " << number << " is too old");
        return false;
    }
    LOG_INF("Collab: the client's protocol number " << number << " gives protocol "
            << CollabSocketHandler::Protocol);
    return true;
}

}

CollabSocketHandler::CollabSocketHandler(const std::shared_ptr<StreamSocket>& socket,
                                         const Poco::Net::HTTPRequest& request,
                                         bool allowedOrigin,
                                         const std::string& wopiSrc)
    : WebSocketHandler(socket, request, allowedOrigin)
    , _wopiSrc(wopiSrc)
    , _socketWeak(socket)
    , _protocolAccepted(acceptsProtocol(request))
{
}

CollabSocketHandler::CollabSocketHandler(const std::string& wopiSrc, const std::string& docKey,
                                         const std::shared_ptr<CollabBroker>& broker)
    : WebSocketHandler(/*isClient=*/false, /*isMasking=*/false)
    , _wopiSrc(wopiSrc)
    , _protocolAccepted(true)
    , _docKey(docKey)
    , _broker(broker)
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
        // A client that the server cannot speak to gets the refusal in reply to its first message
        // (as a page's script cannot read the status of a refused upgrade):
        if (!_protocolAccepted) {
            sendTextMessage("error: cmd=collab kind=badprotocolversion");
            shutdown();
            return;
        }

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

    _wopiUri = RequestDetails::sanitizeURI(_wopiSrc);
    // The access token comes from the first message, so the WOPISrc keeps none of its own.
    Poco::URI::QueryParameters queryParams = _wopiUri.getQueryParameters();
    std::erase_if(queryParams, [](const auto& param) { return param.first == "access_token"; });
    _wopiUri.setQueryParameters(queryParams);

    // Validate the WOPI host so that subsequent connections
    // (e.g., switching from WASM to server mode) are authorized.
    if (StorageBase::validate(_wopiUri, /*takeOwnership=*/false) != StorageBase::StorageType::Wopi)
    {
        LOG_ERR("Collab: WOPISrc is not an allowed WOPI host: "
                << Anonymizer::anonymizeUrl(_wopiSrc));
        _isValidating = false;
        sendTextMessage("error: cmd=internal kind=unauthorized");
        shutdown();
        return;
    }

    // sanitizeURI decodes an access_token parameter, so decode this one the same way.
    Poco::URI wopiUri(_wopiUri);
    wopiUri.addQueryParameter("access_token", Uri::decode(_accessToken));

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

                takeRights(cfi);

                // WOPI's UserExtraInfo.avatar is a URL to the user's
                // avatar image, forwarded to collab peers via user
                // notifications (user_list, user_joined, ...).
                if (_wopiInfo->isObject("UserExtraInfo"))
                {
                    auto extra = _wopiInfo->getObject("UserExtraInfo");
                    if (extra)
                        _avatar = extra->optValue<std::string>(
                            "avatar", std::string());
                }
            }

            // Find or create the CollabBroker for this document
            auto broker = findOrCreateCollabBroker(_docKey, _wopiUri.toString());
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

                    // Register the user's avatar URL + token on
                    // the broker, so /cool/collab/avatar can proxy
                    // it for peers.
                    if (!_avatar.empty())
                        broker->registerAvatar(
                            _userId, _avatar, _accessToken);

                    // If a plain-COOL DocBroker is already editing
                    // the same docKey, surface its sessions as
                    // external entries in the broker.  Posts a
                    // callback onto the DocBroker's poll thread; the
                    // resulting user_joined messages arrive on this
                    // handler shortly after the initial user_list.
                    std::shared_ptr<DocumentBroker> docBroker;
                    {
                        std::lock_guard<std::mutex> lock(DocBrokersMutex);
                        auto it = DocBrokers.find(_docKey);
                        if (it != DocBrokers.end())
                            docBroker = it->second;
                    }
                    if (docBroker)
                    {
                        docBroker->addCallback([docBroker]
                        {
                            docBroker->pushSessionsToCollabBroker();
                        });
                    }
                }
            }

            _isAuthenticated = true;
            LOG_INF("Collab session authenticated for WOPISrc: "
                    << Anonymizer::anonymizeUrl(_wopiSrc)
                    << ", docKey: " << _docKey
                    << ", userId: " << Anonymizer::anonymize(_userId)
                    << ", username: " << Anonymizer::anonymize(_username)
                    << ", canWrite: " << _userCanWrite
                    << ", cleanExport: " << _allowsCleanExport);

            Poco::JSON::Object::Ptr authenticated = new Poco::JSON::Object();
            authenticated->set("type", "authenticated");
            authenticated->set("protocol", Protocol);
            const std::unique_ptr<WopiStorage::WOPIFileInfo> fileInfo = cfi.wopiFileInfo(cfi.url());
            if (fileInfo)
            {
                const std::string baseFileName = fileInfo->getFilename();
                const std::string watermarkText = fileInfo->getWatermarkText();
                const bool isOwner = !fileInfo->getUserId().empty()
                                     && fileInfo->getUserId() == fileInfo->getOwnerId();

                Poco::JSON::Object::Ptr properties =
                    DocumentBroker::makeEditorFileInfo(*fileInfo, baseFileName, watermarkText);

                properties->set("UserCanWrite", fileInfo->getUserCanWrite());
                properties->set("IsOwner", isOwner);

                // FileUrl downloads the document from the host. Fetch gives the document
                // only when the host allows export and copy and sets no watermark.
                properties->remove("FileUrl");
                authenticated->set("fileInfo", properties);
            }

            sendTextMessage(JsonUtil::jsonToString(authenticated));
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
        case CheckFileInfo::State::NoAnswer:
        case CheckFileInfo::State::Transient:
        {
            // The host either didn't answer in time or asked us to come back
            // later. Report it as a timeout rather than a load failure: nothing
            // is wrong with the document or the user's access to it.
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

void CollabSocketHandler::takeRights(CheckFileInfo & cfi) {
                const std::unique_ptr<WopiStorage::WOPIFileInfo> fileInfo =
                    cfi.wopiFileInfo(cfi.url());
                _userCanWrite = fileInfo && fileInfo->getUserCanWrite() &&
                                !fileInfo->getUserCanOnlyComment() &&
                                !fileInfo->getUserCanOnlyManageRedlines();
                _allowsCleanExport = fileInfo && fileInfo->allowsCleanExport();
}

void CollabSocketHandler::renewAccessToken(std::string const & token) {
    if (token.empty()) {
        sendTextMessage(R"({"type":"access_token","accepted":false})");
        return;
    }
    // The URI-encoded token is decoded as for the first CheckFileInfo:
    Poco::URI wopiUri(_wopiUri);
    wopiUri.addQueryParameter("access_token", Uri::decode(token));

    std::weak_ptr<ProtocolHandlerInterface> const handlerWeak = shared_from_this();
    _renewalCheckFileInfo = std::make_shared<CheckFileInfo>(
        COOLWSD::getWebServerPoll(), wopiUri, [handlerWeak, token](CheckFileInfo & cfi) {
            if (auto const handler = handlerWeak.lock()) {
                static_cast<CollabSocketHandler *>(handler.get())
                    ->onRenewalCheckFileInfoFinished(cfi, token);
            }
        });
    _renewalCheckFileInfo->checkFileInfo(/* redirectionLimit */ 5);
}

void CollabSocketHandler::onRenewalCheckFileInfoFinished(
    CheckFileInfo & cfi, std::string const & token)
{
    // A new token takes over only for the same user, together with the rights that it gives:
    bool const accepted = cfi.state() == CheckFileInfo::State::Pass && cfi.wopiInfo()
        && cfi.wopiInfo()->optValue<std::string>("UserId", std::string()) == _userId;
    if (accepted) {
        _accessToken = token;
        takeRights(cfi);
        if (auto const broker = _broker.lock(); broker && !_avatar.empty()) {
            broker->registerAvatar(_userId, _avatar, _accessToken);
        }
    }

    LOG_INF("Collab: a new access token was " << (accepted ? "accepted" : "refused"));
    sendTextMessage(
        std::string(R"({"type":"access_token","accepted":)") + (accepted ? "true" : "false")
        + '}');
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
        if (stream == SettingsFileStream)
            handleSettingsFileFetch(requestId, json->optValue<std::string>("list", std::string()),
                                    json->optValue<std::string>("group", std::string()),
                                    json->optValue<std::string>("uri", std::string()));
        else
            handleFetch(stream, requestId, ifNoneMatch, ifModifiedSince);
    }
    else if (type == "upload")
    {
        const std::string stream = json->optValue<std::string>("stream", "contents");
        handleUpload(
            stream, requestId, json->optValue<std::string>("lastModifiedTime", std::string()),
            json->optValue<bool>("force", false));
    }
    else if (type == "editing_started")
    {
        handleEditingStarted();
    }
    else if (type == "access_token")
    {
        renewAccessToken(json->optValue<std::string>("token", std::string()));
    }
    else if (type == "bye")
    {
        // Client is announcing an orderly departure (e.g. CODA-Q
        // closed the document, or is switching out of local-edit
        // mode).  Mark the handler so removeHandler() short-circuits
        // the broker's idle grace period when this socket
        // disconnects.
        _byeReceived = true;
        LOG_INF("Collab: bye received from " << _handlerId);
    }
    else if (type == "switch_to_collab" || type == "saved_and_switching")
    {
        // Forward to other handlers as-is, adding the sender's identity.
        auto broker = _broker.lock();
        if (broker)
        {
            auto self = std::dynamic_pointer_cast<CollabSocketHandler>(shared_from_this());
            if (self)
            {
                std::ostringstream oss;
                oss << "{\"type\":\"" << JsonUtil::escapeJSONValue(type)
                    << "\",\"user\":{"
                       "\"id\":\"" << JsonUtil::escapeJSONValue(_userId) << "\""
                       ",\"name\":\"" << JsonUtil::escapeJSONValue(_username) << "\""
                       "}}";
                const std::string message = oss.str();

                LOG_INF("Collab: forwarding " << type << " from "
                        << Anonymizer::anonymize(_username));

                // A switch_to_collab is for the users who edit a local copy:
                if (type == "switch_to_collab") {
                    broker->sendToEditingUsers(message, self);
                } else {
                    broker->broadcastExcluding(message, self);
                }
            }
        }
    }
    else
    {
        LOG_WRN("Collab: unknown message type: " << type);
        std::ostringstream oss;
        oss << "{\"type\":\"error\",\"error\":\"Unknown message type\""
               ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"}";
        sendTextMessage(oss.str());
    }
}

void CollabSocketHandler::sendFetchError(const std::string& stream, const std::string& requestId,
                                         const std::string& error)
{
    std::ostringstream oss;
    oss << "{\"type\":\"fetch_error\",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
    if (!requestId.empty())
        oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
    oss << ",\"error\":\"" << JsonUtil::escapeJSONValue(error) << "\"}";
    sendTextMessage(oss.str());
}

void CollabSocketHandler::sendFetchUrl(const std::string& stream, const std::string& requestId,
                                       const std::string& url)
{
    // Get the broker's access token for secure validation
    auto broker = _broker.lock();
    if (!broker)
    {
        LOG_ERR("Collab: broker no longer available for fetch");
        sendFetchError(stream, requestId, "Session expired");
        return;
    }

    const std::string brokerTag = broker->getCurrentAccessToken();

    // A settings list or one of its files is fetched with the uri that the host gives, as it is.
    const bool settings = stream == "userSettings" || stream == "sharedSettings" ||
                          stream == SettingsFileStream;

    // Create a fetch token that the client can use to download via HTTP
    const std::string token =
        createCollabFetchRequest(url, settings ? std::string() : _accessToken, _wopiSrc, _docKey,
                                 brokerTag, requestId, stream);

    // Build the download URL
    std::ostringstream downloadUrl;
    downloadUrl << "/cool/collab/fetch?token=" << token
                << collabRoutingParameters(_wopiUri.toString());

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
        // The version in storage of the bytes behind the URL:
        const std::string lastModifiedTime =
            _wopiInfo->optValue<std::string>("LastModifiedTime", std::string());
        if (!lastModifiedTime.empty()) {
            oss << ",\"lastModifiedTime\":\"" << JsonUtil::escapeJSONValue(lastModifiedTime)
                << "\"";
        }
    }
    oss << "}";

    sendTextMessage(oss.str());
    LOG_DBG("Collab: sent fetch URL for stream: " << stream << ", token: " << token);
}

std::string CollabSocketHandler::settingsListUri(const std::string& list) const
{
    if (!_wopiInfo)
        return std::string();

    if (list == "shared")
        return SharedSettings(_wopiInfo).getUri();

    std::string uri;
    if (list == "user")
    {
        if (auto settings = _wopiInfo->getObject("UserSettings"))
            JsonUtil::findJSONValue(settings, "uri", uri);
    }
    return uri;
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
        sendFetchError(stream, requestId, "Server shutting down");
        return;
    }

    // Build the URL for the requested stream
    std::string url;
    if (stream == "contents" && !_allowsCleanExport)
    {
        LOG_WRN("Collab: refusing " << stream << " fetch, the host allows no clean export");
        std::ostringstream oss;
        oss << "{\"type\":\"fetch_error\",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
        if (!requestId.empty()) {
            oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
        }
        oss << ",\"error\":\"Export not allowed\",\"cleanExport\":false}";
        sendTextMessage(oss.str());
        return;
    }
    else if (stream == "contents")
    {
        // Use FileUrl if available, otherwise construct /contents URL
        if (_wopiInfo)
        {
            url = _wopiInfo->optValue<std::string>("FileUrl", std::string());
        }
        if (url.empty())
        {
            // Construct WOPI /contents URL; _wopiUri may already contain
            // query parameters, so use Poco::URI to modify only the path.
            Poco::URI contentsUri(_wopiUri);
            contentsUri.setPath(contentsUri.getPath() + "/contents");
            contentsUri.addQueryParameter("access_token", _accessToken);
            url = contentsUri.toString();
        }
    }
    else if (stream == "userSettings")
    {
        url = settingsListUri("user");
    }
    else if (stream == "sharedSettings")
    {
        url = settingsListUri("shared");
    }

    if (url.empty())
    {
        LOG_WRN("Collab: stream not available: " << stream);
        sendFetchError(stream, requestId, "Stream not available");
        return;
    }

    sendFetchUrl(stream, requestId, url);
}

namespace
{
/// The file name that a settings uri gives: its file_name parameter, or else the last segment of
/// its path. Empty when that is not one plain file name.
std::string settingsFileName(const std::string& uri)
{
    std::string name;
    try
    {
        for (const auto& param : Poco::URI(uri).getQueryParameters())
        {
            if (param.first == "file_name")
                name = param.second;
        }

        if (name.empty())
            name = Uri::getFilenameWithExtFromURL(uri);
    }
    catch (const std::exception&)
    {
        return std::string();
    }

    return FileUtil::isPlainFileName(name) ? name : std::string();
}
} // namespace

std::span<const std::string_view>
CollabSocketHandler::settingsSecretFields(const std::string& stream, const std::string& url)
{
    if (stream != SettingsFileStream)
        return {};
    return SettingsSecrets::fieldsFor(settingsFileName(url));
}

std::string CollabSocketHandler::settingsListEntryUri(const Poco::JSON::Object::Ptr& settings,
                                                      const std::string& group,
                                                      const std::string& uri)
{
    const std::string name = settingsFileName(uri);
    if (name.empty() || !settings || !settings->isArray(group))
        return std::string();

    const Poco::JSON::Array::Ptr entries = settings->getArray(group);
    for (std::size_t i = 0, count = entries->size(); i < count; ++i)
    {
        const Poco::JSON::Object::Ptr entry = entries->getObject(i);
        if (!entry)
            continue;
        std::string entryUri = entry->optValue<std::string>("uri", std::string());
        if (settingsFileName(entryUri) == name)
            return entryUri;
    }
    return std::string();
}

void CollabSocketHandler::answerSettingsFileFetch(const Poco::JSON::Object::Ptr& settings,
                                                  const PendingSettingsFile& request)
{
    // The client names the entry, and the list this server fetched gives the uri to fetch.
    const std::string uri = settingsListEntryUri(settings, request.group, request.uri);
    if (uri.empty())
    {
        LOG_WRN("Collab: refusing settingsFile fetch, the settings list has no such entry");
        sendFetchError(SettingsFileStream, request.requestId, "Not in settings list");
        return;
    }

    sendFetchUrl(SettingsFileStream, request.requestId, uri);
}

void CollabSocketHandler::handleSettingsFileFetch(const std::string& requestId,
                                                  const std::string& list,
                                                  const std::string& group,
                                                  const std::string& uri)
{
    LOG_INF("Collab: handling settingsFile fetch from the " << list << " list, group: " << group
            << ", requestId: " << requestId);

    if (SigUtil::getShutdownRequestFlag())
    {
        LOG_WRN("Collab: shutdown in progress, rejecting fetch");
        sendFetchError(SettingsFileStream, requestId, "Server shutting down");
        return;
    }

    if (list != "shared" && list != "user")
    {
        sendFetchError(SettingsFileStream, requestId, "Unknown list");
        return;
    }

    if (group != TemplateGroup)
    {
        sendFetchError(SettingsFileStream, requestId, "Unknown group");
        return;
    }

    PendingSettingsFile request{ requestId, group, uri };

    const auto it = _settingsLists.find(list);
    if (it != _settingsLists.end())
    {
        answerSettingsFileFetch(it->second, request);
        return;
    }

    const std::string listUri = settingsListUri(list);
    if (listUri.empty())
    {
        sendFetchError(SettingsFileStream, requestId, "Stream not available");
        return;
    }

    Poco::URI settingsUri;
    try
    {
        settingsUri = Poco::URI(listUri);
    }
    catch (const Poco::Exception& exc)
    {
        LOG_ERR("Collab: the uri of the " << list << " settings list is not valid: "
                                           << exc.displayText());
        sendFetchError(SettingsFileStream, requestId, "Settings list not available");
        return;
    }

    // The first request for a list starts its download, and the later ones wait for it.
    std::vector<PendingSettingsFile>& pending = _pendingSettingsFiles[list];
    pending.push_back(std::move(request));
    if (pending.size() > 1)
        return;

    std::shared_ptr<http::Session> httpSession =
        StorageConnectionManager::getHttpSession(settingsUri);
    http::Request httpRequest(settingsUri.getPathAndQuery());

    std::weak_ptr<ProtocolHandlerInterface> handlerWeak = shared_from_this();
    http::Session::FinishedCallback finishedCallback =
        [handlerWeak, list](const std::shared_ptr<http::Session>& session)
    {
        if (auto handler = handlerWeak.lock())
            static_cast<CollabSocketHandler*>(handler.get())
                ->onSettingsListFetched(list, session->response());
    };
    httpSession->setFinishedHandler(std::move(finishedCallback));
    httpSession->setConnectFailHandler(
        [handlerWeak, list](const std::shared_ptr<http::Session>& /*session*/)
        {
            if (auto handler = handlerWeak.lock())
                static_cast<CollabSocketHandler*>(handler.get())
                    ->onSettingsListFetched(list, nullptr);
        });

    if (!httpSession->asyncRequest(httpRequest, COOLWSD::getWebServerPoll()))
        onSettingsListFetched(list, nullptr);
    else
        httpSession->response()->setBodySizeLimit(MaxHttpFetchSizeBytes);
}

void CollabSocketHandler::onSettingsListFetched(
    const std::string& list, const std::shared_ptr<const http::Response>& httpResponse)
{
    Poco::JSON::Object::Ptr settings;
    if (!httpResponse || httpResponse->statusLine().statusCode() != http::StatusCode::OK ||
        httpResponse->bodySizeLimitExceeded())
        LOG_ERR("Collab: failed to get the " << list << " settings list");
    else if (!JsonUtil::parseJSON(httpResponse->getBody(), settings))
    {
        LOG_ERR("Collab: the " << list << " settings list is no JSON object");
        settings.reset();
    }

    std::vector<PendingSettingsFile> pending;
    pending.swap(_pendingSettingsFiles[list]);
    _pendingSettingsFiles.erase(list);

    if (settings)
        _settingsLists[list] = settings;

    for (const PendingSettingsFile& request : pending)
    {
        if (settings)
            answerSettingsFileFetch(settings, request);
        else
            sendFetchError(SettingsFileStream, request.requestId, "Settings list not available");
    }
}

void CollabSocketHandler::handleUpload(const std::string& stream, const std::string& requestId,
                                       const std::string& lastModifiedTime, bool force)
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

    std::string error;
    if (stream != "contents")
        error = "Upload only supported for contents stream";
    else if (!_userCanWrite)
        error = "The host allows this user no writes";
    else if (lastModifiedTime.empty() && !force)
        error = "The upload has neither lastModifiedTime nor force";
    if (!error.empty())
    {
        LOG_WRN("Collab: refusing upload of stream " << stream << ": " << error);
        std::ostringstream oss;
        oss << "{\"type\":\"upload_error\",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
        if (!requestId.empty())
            oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
        oss << ",\"error\":\"" << error << "\"}";
        sendTextMessage(oss.str());
        return;
    }

    // Build the target URL by appending /contents to the WOPISrc path.
    // _wopiUri may already contain query parameters,
    // so use Poco::URI to modify only the path component.
    Poco::URI targetUri(_wopiUri);
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
    const std::string token = createCollabUploadRequest(
        targetUrl, _accessToken, _wopiSrc, _docKey, brokerTag, requestId,
        force ? "" : lastModifiedTime);

    // Send the upload URL to the client
    std::ostringstream oss;
    oss << "{\"type\":\"upload_url\"";
    oss << ",\"stream\":\"" << JsonUtil::escapeJSONValue(stream) << "\"";
    if (!requestId.empty())
        oss << ",\"requestId\":\"" << JsonUtil::escapeJSONValue(requestId) << "\"";
    oss << ",\"url\":\""
        << JsonUtil::escapeJSONValue(
               "/cool/collab/put?token=" + token + collabRoutingParameters(_wopiUri.toString()))
        << "\"";
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

void CollabSocketHandler::handleEditingStarted()
{
    LOG_INF("Collab: user [" << Anonymizer::anonymize(_username)
            << "] started editing, notifying other users");

    auto broker = _broker.lock();
    if (!broker)
    {
        LOG_ERR("Collab: broker no longer available for editing_started");
        return;
    }

    auto self = std::dynamic_pointer_cast<CollabSocketHandler>(shared_from_this());
    if (self)
        broker->notifyEditingStarted(self);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
