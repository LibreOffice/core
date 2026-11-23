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

#include "CollabFileProxy.hpp"

#include <Anonymizer.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/ClientRequestDispatcher.hpp>
#include <Common.hpp>
#include <common/SettingsSecrets.hpp>
#include <wsd/DocumentBroker.hpp>
#include <HttpHelper.hpp>
#include <JsonUtil.hpp>
#include <Log.hpp>
#include <Protocol.hpp>
#include <wsd/RequestDetails.hpp>
#include <SigUtil.hpp>
#include <Util.hpp>
#include <wsd/wopi/CheckFileInfo.hpp>
#include <wsd/wopi/StorageConnectionManager.hpp>

#include <Poco/JSON/Parser.h>
#include <Poco/SHA2Engine.h>
#include <Poco/URI.h>

namespace {

// How often a download is tried again after a temporary error or no answer from the host:
constexpr int DownloadRetries = 2;

// The handler that answers the client with 502 when no connection to the host comes about (as when
// its name does not resolve, or when none of its addresses is allowed):
http::Session::ConnectFailCallback badGatewayOnConnectFail(
    std::weak_ptr<StreamSocket> const & socket, std::string const & uriAnonym)
{
    return [socket, uriAnonym](std::shared_ptr<http::Session> const &) {
        LOG_ERR("CollabFileProxy: no connection to [" << uriAnonym << ']');
        if (std::shared_ptr<StreamSocket> const client = socket.lock()) {
            HttpHelper::sendErrorAndShutdown(http::StatusCode::BadGateway, client);
        }
    };
}

}

CollabFileProxy::CollabFileProxy(std::string id, const RequestDetails& requestDetails,
                                 const std::shared_ptr<StreamSocket>& socket,
                                 const std::string& wopiSrc, const std::string& accessToken)
    : _id(std::move(id))
    , _requestDetails(requestDetails)
    , _socket(socket)
    , _wopiSrc(wopiSrc)
    , _accessToken(accessToken)
    , _logFD(socket->getFD())
{
}

void CollabFileProxy::handleFetchRequest(const std::string& streamUrl, bool hostChecked,
                                         const std::shared_ptr<TerminatingPoll>& poll,
                                         SocketDisposition& disposition,
                                         std::span<const std::string_view> secretFields)
{
    _secretFields = secretFields;

    LOG_INF("CollabFileProxy: handling fetch request for stream ["
            << Anonymizer::anonymizeUrl(streamUrl) << ']');

    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (!socket)
    {
        LOG_ERR("CollabFileProxy: invalid socket for fetch request");
        return;
    }

    // Parse the stream URL and add access token; the URL may already
    // contain query parameters, so use Poco::URI to add it properly.
    // A request that has no access token fetches the URL as it is.
    Poco::URI uri(streamUrl);
    if (!_accessToken.empty())
        uri.addQueryParameter("access_token", _accessToken);

    // Transfer to poll and start download directly (bypassing CheckFileInfo)
    disposition.setTransfer(*poll,
        [this, poll, uri = std::move(uri), hostChecked, keepalive = shared_from_this()](
            const std::shared_ptr<Socket>& moveSocket)
        {
            LOG_TRC('#' << moveSocket->getFD()
                        << ": CollabFileProxy: starting direct fetch for ["
                        << Anonymizer::anonymizeUrl(uri.toString()) << ']');

            doDownload(poll, uri, HTTP_REDIRECTION_LIMIT, hostChecked, DownloadRetries);
        });
}

void CollabFileProxy::handleUploadRequest(const std::string& targetUrl,
                                          std::string const & lastModifiedTime,
                                          std::istream& message,
                                          const std::shared_ptr<TerminatingPoll>& poll,
                                          SocketDisposition& disposition)
{
    LOG_INF("CollabFileProxy: handling upload request to ["
            << Anonymizer::anonymizeUrl(targetUrl) << ']');

    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (!socket)
    {
        LOG_ERR("CollabFileProxy: invalid socket for upload request");
        return;
    }

    // Read the POST body
    _uploadBody = std::string(std::istreambuf_iterator<char>(message), {});
    _uploadLastModifiedTime = lastModifiedTime;

    // Parse the target URL and add access token; the URL may already
    // contain query parameters, so use Poco::URI to add it properly.
    Poco::URI uri(targetUrl);
    uri.addQueryParameter("access_token", _accessToken);

    // Transfer to poll and start upload directly (bypassing CheckFileInfo)
    disposition.setTransfer(*poll,
        [this, poll, uri = std::move(uri), keepalive = shared_from_this()](
            const std::shared_ptr<Socket>& moveSocket)
        {
            LOG_TRC('#' << moveSocket->getFD()
                        << ": CollabFileProxy: starting direct upload to ["
                        << Anonymizer::anonymizeUrl(uri.toString()) << ']');

            doUpload(poll, uri, _uploadBody);
        });
}

void CollabFileProxy::doDownload(const std::shared_ptr<TerminatingPoll>& poll,
                                 const Poco::URI& uri, int redirectLimit, bool hostChecked,
                                 int retriesLeft)
{
    const std::string uriAnonym = Anonymizer::anonymizeUrl(uri.toString());
    LOG_DBG("CollabFileProxy: downloading from [" << uriAnonym << ']');

    // The poll keeps the session alive until it finishes, and the session's callback keeps this
    // proxy alive:
    std::shared_ptr<http::Session> const httpSession
        = hostChecked ? StorageConnectionManager::getWopiHttpSession(uri)
                      : StorageConnectionManager::getHttpSession(uri);
    Authorization auth = Authorization::create(uri);
    http::Request httpRequest = StorageConnectionManager::createHttpRequest(uri, auth);

    const auto startTime = std::chrono::steady_clock::now();

    http::Session::FinishedCallback finishedCallback =
        [this, poll, uri, startTime, uriAnonym, redirectLimit, hostChecked, retriesLeft,
         keepalive = shared_from_this()](const std::shared_ptr<http::Session>& session)
    {
        if (SigUtil::getShutdownRequestFlag())
        {
            LOG_DBG("CollabFileProxy: shutdown flagged, aborting download");
            return;
        }

        // A failed connection got its answer from the connect-fail handler, which runs before this
        // one (as after a failed TLS handshake):
        if (session->connectionResult() != net::AsyncConnectResult::Connected) {
            return;
        }

        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (!socket)
        {
            LOG_ERR("CollabFileProxy: invalid socket while downloading ["
                    << uriAnonym << ']');
            return;
        }

        const std::shared_ptr<const http::Response> httpResponse = session->response();
        const http::StatusCode statusCode = httpResponse->statusLine().statusCode();

        LOG_TRC("CollabFileProxy: GetFile returned " << statusCode);

        // Handle redirects
        if (statusCode == http::StatusCode::MovedPermanently ||
            statusCode == http::StatusCode::Found ||
            statusCode == http::StatusCode::TemporaryRedirect ||
            statusCode == http::StatusCode::PermanentRedirect)
        {
            if (redirectLimit > 0)
            {
                const std::string location = httpResponse->get("Location");
                LOG_TRC("CollabFileProxy: redirect to ["
                        << Anonymizer::anonymizeUrl(location) << ']');
                doDownload(poll, Poco::URI(location), redirectLimit - 1, false, retriesLeft);
                return;
            }
            else
            {
                LOG_WRN("CollabFileProxy: too many redirects for ["
                        << uriAnonym << ']');
                HttpHelper::sendErrorAndShutdown(http::StatusCode::BadGateway, socket);
                return;
            }
        }

        // A temporary error or no answer from the host leads to another try (as a GET changes
        // nothing in storage):
        bool const answered = httpResponse->statusLine().statusCategory()
            != http::StatusLine::StatusCodeClass::Invalid;
        if ((!answered || http::isTransientStatusCode(statusCode)) && retriesLeft > 0) {
            LOG_WRN("CollabFileProxy: trying GetFile from [" << uriAnonym << "] again after "
                    << (answered ? "a temporary error" : "no answer") << " (" << statusCode
                    << ')');
            doDownload(poll, uri, redirectLimit, hostChecked, retriesLeft - 1);
            return;
        }

        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime);

        if (statusCode != http::StatusCode::OK)
        {
            LOG_ERR("CollabFileProxy: GetFile failed with " << statusCode
                    << " for [" << uriAnonym << ']');
            HttpHelper::sendErrorAndShutdown(
                statusCode == http::StatusCode::Forbidden
                    ? http::StatusCode::Forbidden
                    : http::StatusCode::BadGateway,
                socket);
            return;
        }

        LOG_INF("CollabFileProxy: downloaded " << httpResponse->getBody().size()
                << " bytes from [" << uriAnonym << "] in " << duration);

        // Send the file to the client.  Preserve the upstream's
        // Content-Type so images etc. are rendered correctly
        // (avatars need image/*).
        http::Response response(http::StatusCode::OK);
        std::string contentType = httpResponse->header().get("Content-Type");
        if (contentType.empty())
            contentType = "application/octet-stream";
        response.setBody(_secretFields.empty()
                             ? httpResponse->getBody()
                             : SettingsSecrets::redactSettingSecrets(httpResponse->getBody(),
                                                                     _secretFields),
                         std::move(contentType));
        response.set("X-Content-Type-Options", "nosniff");
        response.set("Content-Security-Policy", "sandbox");
        response.set("Content-Disposition", "attachment");
        socket->sendAndShutdown(response);
    };

    httpSession->setFinishedHandler(std::move(finishedCallback));
    httpSession->setConnectFailHandler(badGatewayOnConnectFail(_socket, uriAnonym));
    httpSession->asyncRequest(httpRequest, poll);
}

void CollabFileProxy::doUpload(const std::shared_ptr<TerminatingPoll>& poll,
                               const Poco::URI& uri, const std::string& body)
{
    const std::string uriAnonym = Anonymizer::anonymizeUrl(uri.toString());
    LOG_DBG("CollabFileProxy: uploading " << body.size()
            << " bytes to [" << uriAnonym << ']');

    // The poll keeps the session alive until it finishes, and the session's callback keeps this
    // proxy alive:
    std::shared_ptr<http::Session> const httpSession
        = StorageConnectionManager::getWopiHttpSession(uri);
    Authorization auth = Authorization::create(uri);
    http::Request httpRequest = StorageConnectionManager::createHttpRequest(uri, auth);

    httpRequest.setVerb(http::Request::VERB_POST);
    httpRequest.set("X-WOPI-Override", "PUT");
    httpRequest.setContentType("application/octet-stream");
    httpRequest.setContentLength(body.size());
    httpRequest.setBody(body);
    // Have the WOPI host refuse the upload if storage no longer has the version that the client
    // edited:
    if (!_uploadLastModifiedTime.empty()) {
        httpRequest.set("X-COOL-WOPI-Timestamp", _uploadLastModifiedTime);
    }

    const auto startTime = std::chrono::steady_clock::now();

    http::Session::FinishedCallback finishedCallback =
        [this, poll, uri, startTime, uriAnonym, bodySize = body.size(),
         keepalive = shared_from_this()](const std::shared_ptr<http::Session>& session)
    {
        if (SigUtil::getShutdownRequestFlag())
        {
            LOG_DBG("CollabFileProxy: shutdown flagged, aborting upload");
            return;
        }

        // A failed connection got its answer from the connect-fail handler, which runs before this
        // one (as after a failed TLS handshake):
        if (session->connectionResult() != net::AsyncConnectResult::Connected) {
            return;
        }

        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (!socket)
        {
            LOG_ERR("CollabFileProxy: invalid socket while uploading ["
                    << uriAnonym << ']');
            return;
        }

        const std::shared_ptr<const http::Response> httpResponse = session->response();
        const http::StatusCode statusCode = httpResponse->statusLine().statusCode();

        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime);

        LOG_INF("CollabFileProxy: PutFile returned " << statusCode
                << " for " << bodySize << " bytes to [" << uriAnonym
                << "] in " << duration);

        if (statusCode == http::StatusCode::OK)
        {
            answerUploaded(socket, httpResponse->getBody());
        }
        else if (statusCode == http::StatusCode::Conflict)
        {
            // Document was modified externally
            http::Response response(http::StatusCode::Conflict);
            response.setBody(httpResponse->getBody(), "application/json; charset=utf-8");
            socket->sendAndShutdown(response);
        }
        else if (statusCode == http::StatusCode::Unauthorized ||
                 statusCode == http::StatusCode::Forbidden)
        {
            HttpHelper::sendErrorAndShutdown(http::StatusCode::Forbidden, socket);
        }
        else if ((httpResponse->statusLine().statusCategory()
                      == http::StatusLine::StatusCodeClass::Invalid
                  || http::isTransientStatusCode(statusCode))
                 && !_uploadRetried)
        {
            // The host may or may not have stored the upload (so a forced one is just repeated,
            // and else storage tells which, as for /cool/ws):
            _uploadRetried = true;
            if (_uploadLastModifiedTime.empty()) {
                LOG_WRN("CollabFileProxy: repeating the forced PutFile after " << statusCode);
                doUpload(poll, uri, _uploadBody);
            } else {
                checkUploadOutcome(poll, uri);
            }
        }
        else
        {
            LOG_ERR("CollabFileProxy: PutFile failed with " << statusCode);
            HttpHelper::sendErrorAndShutdown(http::StatusCode::BadGateway, socket);
        }
    };

    httpSession->setFinishedHandler(std::move(finishedCallback));
    httpSession->setConnectFailHandler(badGatewayOnConnectFail(_socket, uriAnonym));
    httpSession->asyncRequest(httpRequest, poll);
}

void CollabFileProxy::answerUploaded(
    std::shared_ptr<StreamSocket> const & socket, std::string const & wopiResponse)
{
    // The CheckFileInfo kept for this document's editor form still has the version from
    // before this upload, and a session that used it would get 409 on every save.
    ClientRequestDispatcher::DropRequestVettingStations(
        RequestDetails::getDocKey(_wopiSrc));

    // Update any existing DocumentBroker's stored timestamp
    // so it matches the new version we just uploaded.  Without
    // this, a subsequent CheckFileInfo (when another user joins)
    // would see a timestamp mismatch and trigger a spurious
    // "Document has been changed" conflict dialog.
    try
    {
        Poco::JSON::Parser parser;
        auto json = parser.parse(wopiResponse).extract<Poco::JSON::Object::Ptr>();
        const std::string lastModifiedTime =
            json->optValue<std::string>("LastModifiedTime", std::string());
        if (!lastModifiedTime.empty())
        {
            const std::string docKey = RequestDetails::getDocKey(_wopiSrc);
            std::unique_lock<std::mutex> lock(DocBrokersMutex);
            auto it = DocBrokers.find(docKey);
            if (it != DocBrokers.end() && it->second)
            {
                auto broker = it->second;
                broker->addCallback(
                    [broker, lastModifiedTime]()
                    {
                        broker->updateLastModifiedTime(lastModifiedTime);
                    });
            }
        }
    }
    catch (const std::exception& ex)
    {
        LOG_DBG("CollabFileProxy: failed to parse PutFile response for "
                "timestamp update: " << ex.what());
    }

    // Return the WOPI response (contains LastModifiedTime etc.)
    http::Response response(http::StatusCode::OK);
    response.setBody(wopiResponse, "application/json; charset=utf-8");
    socket->sendAndShutdown(response);
}

void CollabFileProxy::checkUploadOutcome(
    std::shared_ptr<TerminatingPoll> const & poll, Poco::URI const & uri)
{
    // The address of the upload is that of the file with "/contents" appended:
    Poco::URI fileUri(uri);
    std::string path = fileUri.getPath();
    path.erase(path.size() - std::string_view("/contents").size());
    fileUri.setPath(path);

    LOG_WRN("CollabFileProxy: asking storage whether the PutFile without a clear answer landed");
    _uploadCheck = std::make_shared<CheckFileInfo>(
        poll, fileUri,
        [this, poll, uri, keepalive = shared_from_this()](CheckFileInfo & check) {
            // The CheckFileInfo stays alive until this callback returns:
            Poco::JSON::Object::Ptr const info = check.wopiInfo();
            bool const passed = check.state() == CheckFileInfo::State::Pass && info;
            _uploadCheck.reset();

            std::shared_ptr<StreamSocket> const socket = _socket.lock();
            if (!socket) {
                return;
            }
            if (!passed) {
                LOG_ERR("CollabFileProxy: storage cannot tell whether the PutFile landed");
                HttpHelper::sendErrorAndShutdown(http::StatusCode::BadGateway, socket);
                return;
            }

            std::string lastModifiedTime;
            JsonUtil::findJSONValue(info, "LastModifiedTime", lastModifiedTime);
            if (lastModifiedTime == _uploadLastModifiedTime) {
                LOG_WRN("CollabFileProxy: the PutFile did not land, repeating it");
                doUpload(poll, uri, _uploadBody);
                return;
            }

            // A hash settles whether the document in storage is the uploaded one, else the size
            // tells, as for /cool/ws:
            std::string storageHash;
            JsonUtil::findJSONValue(info, "SHA256", storageHash);
            bool isUploaded;
            if (storageHash.empty()) {
                std::size_t size = 0;
                JsonUtil::findJSONValue(info, "Size", size);
                isUploaded = size == _uploadBody.size();
            } else {
                Poco::SHA2Engine engine(Poco::SHA2Engine::SHA_256);
                engine.update(_uploadBody.data(), _uploadBody.size());
                isUploaded = storageHash == Util::base64Encode(engine.digest());
            }

            if (isUploaded) {
                LOG_INF("CollabFileProxy: the PutFile landed, as version [" << lastModifiedTime
                        << ']');
                Poco::JSON::Object::Ptr result = new Poco::JSON::Object();
                result->set("LastModifiedTime", lastModifiedTime);
                answerUploaded(socket, JsonUtil::jsonToString(result));
            } else {
                LOG_WRN("CollabFileProxy: storage has another version [" << lastModifiedTime
                        << "] than the PutFile");
                http::Response response(http::StatusCode::Conflict);
                response.setBody("{}", "application/json; charset=utf-8");
                socket->sendAndShutdown(response);
            }
        });
    _uploadCheck->checkFileInfo(HTTP_REDIRECTION_LIMIT);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
