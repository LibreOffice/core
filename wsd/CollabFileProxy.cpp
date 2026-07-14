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

#include "CollabFileProxy.hpp"

#include <Anonymizer.hpp>
#include <COOLWSD.hpp>
#include <Common.hpp>
#include <DocumentBroker.hpp>
#include <HttpHelper.hpp>
#include <Log.hpp>
#include <Protocol.hpp>
#include <RequestDetails.hpp>
#include <SigUtil.hpp>
#include <wopi/StorageConnectionManager.hpp>

#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>

extern std::map<std::string, std::shared_ptr<DocumentBroker>> DocBrokers;
extern std::mutex DocBrokersMutex;

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

void CollabFileProxy::handleFetchRequest(const std::string& streamUrl,
                                         const std::shared_ptr<TerminatingPoll>& poll,
                                         SocketDisposition& disposition)
{
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
    Poco::URI uri(streamUrl);
    uri.addQueryParameter("access_token", _accessToken);

    // Transfer to poll and start download directly (bypassing CheckFileInfo)
    disposition.setTransfer(*poll,
        [this, poll, uri, keepalive = shared_from_this()](
            const std::shared_ptr<Socket>& moveSocket)
        {
            LOG_TRC('#' << moveSocket->getFD()
                        << ": CollabFileProxy: starting direct fetch for ["
                        << Anonymizer::anonymizeUrl(uri.toString()) << ']');

            doDownload(poll, uri, HTTP_REDIRECTION_LIMIT);
        });
}

void CollabFileProxy::handleUploadRequest(const std::string& targetUrl, std::istream& message,
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

    // Parse the target URL and add access token; the URL may already
    // contain query parameters, so use Poco::URI to add it properly.
    Poco::URI uri(targetUrl);
    uri.addQueryParameter("access_token", _accessToken);

    // Transfer to poll and start upload directly (bypassing CheckFileInfo)
    disposition.setTransfer(*poll,
        [this, poll, uri, keepalive = shared_from_this()](
            const std::shared_ptr<Socket>& moveSocket)
        {
            LOG_TRC('#' << moveSocket->getFD()
                        << ": CollabFileProxy: starting direct upload to ["
                        << Anonymizer::anonymizeUrl(uri.toString()) << ']');

            doUpload(poll, uri, _uploadBody);
        });
}

void CollabFileProxy::doDownload(const std::shared_ptr<TerminatingPoll>& poll,
                                 const Poco::URI& uri, int redirectLimit)
{
    const std::string uriAnonym = Anonymizer::anonymizeUrl(uri.toString());
    LOG_DBG("CollabFileProxy: downloading from [" << uriAnonym << ']');

    _httpSession = StorageConnectionManager::getHttpSession(uri);
    Authorization auth = Authorization::create(uri);
    http::Request httpRequest = StorageConnectionManager::createHttpRequest(uri, auth);

    const auto startTime = std::chrono::steady_clock::now();

    http::Session::FinishedCallback finishedCallback =
        [this, &poll, startTime, uriAnonym, redirectLimit, keepalive = shared_from_this()](
            const std::shared_ptr<http::Session>& session)
    {
        if (SigUtil::getShutdownRequestFlag())
        {
            LOG_DBG("CollabFileProxy: shutdown flagged, aborting download");
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

        // CORS allow-origin for cross-origin JS callers (see success
        // path below).  Applied uniformly to error responses too so
        // the browser surfaces the actual status code to JS instead
        // of an opaque "CORS failure".
        const std::string corsHeader = "Access-Control-Allow-Origin: *\r\n";

        // Handle redirects
        if (statusCode == http::StatusCode::MovedPermanently ||
            statusCode == http::StatusCode::Found ||
            statusCode == http::StatusCode::TemporaryRedirect ||
            statusCode == http::StatusCode::PermanentRedirect)
        {
            if (redirectLimit > 0)
            {
                const std::string& location = httpResponse->get("Location");
                LOG_TRC("CollabFileProxy: redirect to ["
                        << Anonymizer::anonymizeUrl(location) << ']');
                doDownload(poll, Poco::URI(location), redirectLimit - 1);
                return;
            }
            else
            {
                LOG_WRN("CollabFileProxy: too many redirects for ["
                        << uriAnonym << ']');
                HttpHelper::sendErrorAndShutdown(http::StatusCode::BadGateway, socket,
                                                 std::string_view(), corsHeader);
                return;
            }
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
                socket, std::string_view(), corsHeader);
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
        response.setBody(httpResponse->getBody(), std::move(contentType));
        // The /co/collab/fetch endpoint is the byte-stream side of
        // the token-gated download protocol.  CODA's page JS calls
        // it from cool.html's origin (file:// or https://localhost),
        // which is cross-origin to the cool server, so the response
        // needs to advertise CORS access.  The token check above is
        // what enforces authorization; the Origin is not a security
        // boundary here.
        response.set("Access-Control-Allow-Origin", "*");
        socket->sendAndShutdown(response);
    };

    _httpSession->setFinishedHandler(std::move(finishedCallback));
    _httpSession->asyncRequest(httpRequest, poll, false);
}

void CollabFileProxy::doUpload(const std::shared_ptr<TerminatingPoll>& poll,
                               const Poco::URI& uri, const std::string& body)
{
    const std::string uriAnonym = Anonymizer::anonymizeUrl(uri.toString());
    LOG_DBG("CollabFileProxy: uploading " << body.size()
            << " bytes to [" << uriAnonym << ']');

    _httpSession = StorageConnectionManager::getHttpSession(uri);
    Authorization auth = Authorization::create(uri);
    http::Request httpRequest = StorageConnectionManager::createHttpRequest(uri, auth);

    httpRequest.setVerb(http::Request::VERB_POST);
    httpRequest.set("X-WOPI-Override", "PUT");
    httpRequest.setContentType("application/octet-stream");
    httpRequest.setContentLength(body.size());
    httpRequest.setBody(body);

    const auto startTime = std::chrono::steady_clock::now();

    http::Session::FinishedCallback finishedCallback =
        [this, startTime, uriAnonym, bodySize = body.size(), keepalive = shared_from_this()](
            const std::shared_ptr<http::Session>& session)
    {
        if (SigUtil::getShutdownRequestFlag())
        {
            LOG_DBG("CollabFileProxy: shutdown flagged, aborting upload");
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

        // CORS allow-origin for cross-origin JS callers of
        // /co/collab/put (CODA's page JS lives on cool.html's origin,
        // which is cross-origin to the cool server).  Same rationale
        // as the GET side in doDownload.
        const std::string corsHeader = "Access-Control-Allow-Origin: *\r\n";
        if (statusCode == http::StatusCode::OK)
        {
            // Update any existing DocumentBroker's stored timestamp
            // so it matches the new version we just uploaded.  Without
            // this, a subsequent CheckFileInfo (when another user joins)
            // would see a timestamp mismatch and trigger a spurious
            // "Document has been changed" conflict dialog.
            const std::string& wopiResponse = httpResponse->getBody();
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
            response.setBody(httpResponse->getBody(), "application/json; charset=utf-8");
            response.set("Access-Control-Allow-Origin", "*");
            socket->sendAndShutdown(response);
        }
        else if (statusCode == http::StatusCode::Conflict)
        {
            // Document was modified externally
            http::Response response(http::StatusCode::Conflict);
            response.setBody(httpResponse->getBody(), "application/json; charset=utf-8");
            response.set("Access-Control-Allow-Origin", "*");
            socket->sendAndShutdown(response);
        }
        else if (statusCode == http::StatusCode::Unauthorized ||
                 statusCode == http::StatusCode::Forbidden)
        {
            HttpHelper::sendErrorAndShutdown(http::StatusCode::Forbidden, socket,
                                             std::string_view(), corsHeader);
        }
        else
        {
            LOG_ERR("CollabFileProxy: PutFile failed with " << statusCode);
            HttpHelper::sendErrorAndShutdown(http::StatusCode::BadGateway, socket,
                                             std::string_view(), corsHeader);
        }
    };

    _httpSession->setFinishedHandler(std::move(finishedCallback));
    _httpSession->asyncRequest(httpRequest, poll, false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
