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

#include "WopiProxy.hpp"

#include <common/Anonymizer.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/Util.hpp>
#include <net/HttpHelper.hpp>
#include <net/HttpRequest.hpp>
#include <wopi/StorageConnectionManager.hpp>
#include <wopi/WopiStorage.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/Exceptions.hpp>

#include <iterator>
#include <optional>

void WopiProxy::handleRequest(std::istream & message,
                              [[maybe_unused]] const std::shared_ptr<TerminatingPoll>& poll,
                              SocketDisposition& disposition)
{
    std::string url = _requestDetails.getDocumentURI();
    if (url.starts_with("/wasm/"))
    {
        url = url.substr(6);
    }

    LOG_INF("URL [" << url << "] for WS Request.");

    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (!socket)
    {
        LOG_ERR("Invalid socket while handling wopi proxy request for ["
                << Anonymizer::anonymizeUrl(url) << ']');
        return;
    }

    const auto uriPublic = RequestDetails::sanitizeURI(url);
    std::string docKey = RequestDetails::getDocKey(uriPublic);
    const std::string fileId = Uri::getFilenameFromURL(Uri::decode(docKey));
    Anonymizer::mapAnonymized(fileId,
                              fileId); // Identity mapping, since fileId is already obfuscated

    LOG_INF("Starting GET request handler for session [" << _id << "] on url ["
                                                         << Anonymizer::anonymizeUrl(url) << "].");

    LOG_INF("Sanitized URI [" << Anonymizer::anonymizeUrl(url) << "] to ["
                              << Anonymizer::anonymizeUrl(uriPublic.toString())
                              << "] and mapped to docKey [" << docKey << "] for session [" << _id
                              << "].");

    // Before we create DocBroker with a SocketPoll thread, a ClientSession, and a Kit process,
    // we need to vet this request by invoking CheckFileInfo.
    // For that, we need the storage settings to create a connection.
    const StorageBase::StorageType storageType =
        StorageBase::validate(uriPublic, /*takeOwnership=*/false);
    switch (storageType)
    {
        case StorageBase::StorageType::Unsupported:
            LOG_ERR("Unsupported URI [" << Anonymizer::anonymizeUrl(uriPublic.toString())
                                        << "] or no storage configured");
            throw BadRequestException("No Storage configured or invalid URI [" +
                                      Anonymizer::anonymizeUrl(uriPublic.toString()) + ']');

        case StorageBase::StorageType::Unauthorized:
            LOG_ERR("No authorized hosts found matching the target host [" << uriPublic.getHost()
                                                                           << "] in config");
            HttpHelper::sendErrorAndShutdown(http::StatusCode::Unauthorized, socket);
            break;

        case StorageBase::StorageType::Conversion:
            // We don't expect conversion requests.
            LOG_ERR("Unsupported URI [" << Anonymizer::anonymizeUrl(uriPublic.toString())
                                        << "] for conversion");
            throw BadRequestException("Invalid URI for conversion [" +
                                      Anonymizer::anonymizeUrl(uriPublic.toString()) + ']');

#if ENABLE_LOCAL_FILESYSTEM
        case StorageBase::StorageType::FileSystem:
        {
            LOG_INF("URI [" << Anonymizer::anonymizeUrl(uriPublic.toString()) << "] on docKey ["
                            << docKey << "] is for a FileSystem document");

            // Send the file contents.
            std::unique_ptr<std::vector<char>> data = FileUtil::readFile(uriPublic.getPath());
            if (data)
            {
                http::Response response(http::StatusCode::OK);
                response.setBody(std::string(data->data(), data->size()),
                                 "application/octet-stream");
                socket->sendAndShutdown(response);
            }
            else
            {
                HttpHelper::sendErrorAndShutdown(http::StatusCode::NotFound, socket);
            }
            break;
        }
#endif // ENABLE_LOCAL_FILESYSTEM

#if !MOBILEAPP
        case StorageBase::StorageType::Wopi:
            LOG_INF("URI [" << Anonymizer::anonymizeUrl(uriPublic.toString()) << "] on docKey ["
                            << docKey << "] is for a WOPI document");
            std::optional<std::string> postBody;
            if (_requestDetails.isPost()) {
                postBody = std::string(std::istreambuf_iterator<char>(message), {});
            }
            // Remove from the current poll and transfer.
            disposition.setTransfer(*poll,
                [this, poll, docKey = std::move(docKey), url = std::move(url),
                 uriPublic, postBody](const std::shared_ptr<Socket>& moveSocket)
                {
                    LOG_TRC_S('#' << moveSocket->getFD()
                                  << ": Dissociating client socket from "
                                     "ClientRequestDispatcher and invoking CheckFileInfo for ["
                                  << docKey << ']');

                    // CheckFileInfo and only when it's good create DocBroker.
                    checkFileInfo(poll, uriPublic, postBody, HTTP_REDIRECTION_LIMIT);
                });
            break;
#endif //!MOBILEAPP
    }
}

#if !MOBILEAPP
void WopiProxy::checkFileInfo(const std::shared_ptr<TerminatingPoll>& poll, const Poco::URI& uri,
                              std::optional<std::string> const & postBody, int redirectLimit)
{
    auto cfiContinuation =
        [this, poll, uri, postBody]([[maybe_unused]] CheckFileInfo& checkFileInfo)
    {
        const std::string uriAnonym = Anonymizer::anonymizeUrl(uri.toString());

        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (!socket)
        {
            LOG_ERR("Invalid socket while handling wopi CheckFileInfo for [" << uriAnonym << ']');
            return;
        }

        assert(&checkFileInfo == _checkFileInfo.get() && "Unknown CheckFileInfo instance");
        if (_checkFileInfo && _checkFileInfo->state() == CheckFileInfo::State::Pass &&
            _checkFileInfo->wopiInfo())
        {
            Poco::JSON::Object::Ptr object = _checkFileInfo->wopiInfo();

            std::size_t size = 0;
            std::string filename, ownerId, lastModifiedTime;
            JsonUtil::findJSONValue(object, "Size", size);
            JsonUtil::findJSONValue(object, "OwnerId", ownerId);
            JsonUtil::findJSONValue(object, "BaseFileName", filename);
            JsonUtil::findJSONValue(object, "LastModifiedTime", lastModifiedTime);

            const StorageBase::FileInfo fileInfo(size, std::move(filename), std::move(ownerId),
                                                 std::move(lastModifiedTime));

            auto wopiInfo = std::make_unique<WopiStorage::WOPIFileInfo>(fileInfo, object, uri);

            std::string url = checkFileInfo.url().toString();

            // If FileUrl is set, we use it for GetFile.
            const std::string fileUrl = wopiInfo->getFileUrl();

            // First try the FileUrl, if provided.
            if (!fileUrl.empty())
            {
                const std::string fileUrlAnonym = Anonymizer::anonymizeUrl(fileUrl);
                try
                {
                    LOG_INF("WOPI::GetFile using FileUrl: " << fileUrlAnonym);
                    return transfer(
                        poll, url, postBody, Poco::URI(fileUrl), HTTP_REDIRECTION_LIMIT);
                }
                catch (const std::exception& ex)
                {
                    LOG_ERR("Could not download document from WOPI FileUrl [" << fileUrlAnonym <<
                                "]. Will use default URL. Error: "
                            << ex.what());
                    // Fall-through.
                }
            }

            // Try the default URL, we either don't have FileUrl, or it failed.
            // WOPI URI to download files ends in '/contents'.
            // Add it here to get the payload instead of file info.
            Poco::URI uriObject(uri);
            uriObject.setPath(uriObject.getPath() + "/contents");
            url = uriObject.toString();

            try
            {
                LOG_INF("WOPI::GetFile using default URI: " << uriAnonym);
                return transfer(poll, url, postBody, uriObject, HTTP_REDIRECTION_LIMIT);
            }
            catch (const std::exception& ex)
            {
                LOG_ERR(
                    "Cannot download document from WOPI storage uri [" << uriAnonym << "]. Error: "
                    << ex.what());
                // Fall-through.
            }
        }

        LOG_ERR("Invalid URI or access denied to [" << uriAnonym << ']');
        HttpHelper::sendErrorAndShutdown(http::StatusCode::Unauthorized, socket);
    };

    // CheckFileInfo asynchronously.
    _checkFileInfo = std::make_shared<CheckFileInfo>(poll, uri, std::move(cfiContinuation));
    _checkFileInfo->checkFileInfo(redirectLimit);
}

void WopiProxy::transfer(const std::shared_ptr<TerminatingPoll>& poll, const std::string& url,
                         std::optional<std::string> const & postBody,
                         const Poco::URI& uriPublic, int redirectLimit)
{
    std::string uriAnonym = Anonymizer::anonymizeUrl(uriPublic.toString());

    LOG_DBG("Getting info for wopi uri [" << uriAnonym << ']');
    _httpSession = StorageConnectionManager::getHttpSession(uriPublic);
    Authorization auth = Authorization::create(uriPublic);
    http::Request httpRequest = StorageConnectionManager::createHttpRequest(uriPublic, auth);
    if (postBody) {
        httpRequest.setVerb(http::Request::VERB_POST);
        httpRequest.setBody(*postBody);
    }

    LOG_TRC("WOPI::GetFile request header for URI [" << uriAnonym << "]:\n"
                                                     << httpRequest.header());

    http::Session::FinishedCallback finishedCallback =
        [this, poll, url, postBody, uriAnonym=std::move(uriAnonym),
         redirectLimit](const std::shared_ptr<http::Session>& session)
    {
        if (SigUtil::getShutdownRequestFlag())
        {
            LOG_DBG("Shutdown flagged, giving up on in-flight requests");
            return;
        }

        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (!socket)
        {
            LOG_ERR("Invalid socket while downloading [" << uriAnonym << ']');
            return;
        }

        const std::shared_ptr<const http::Response> httpResponse = session->response();
        LOG_TRC("WOPI::GetFile returned " << httpResponse->statusLine().statusCode());

        const http::StatusCode statusCode = httpResponse->statusLine().statusCode();
        if (http::isRedirectStatusCode(statusCode))
        {
            if (redirectLimit)
            {
                const std::string& location = httpResponse->get("Location");
                LOG_TRC("WOPI::GetFile redirect to URI [" << Anonymizer::anonymizeUrl(location)
                                                          << "]");

                transfer(poll, location, postBody, Poco::URI(location), redirectLimit - 1);
                return;
            }
            else
            {
                LOG_WRN("WOPI::GetFile redirected too many times. Giving up on URI [" << uriAnonym
                                                                                      << ']');
            }
        }

        // Note: we don't log the response if obfuscation is enabled, except for failures.
        const bool failed = (httpResponse->statusLine().statusCode() != http::StatusCode::OK);
        if (Log::isEnabled(failed ? Log::Level::ERR : Log::Level::TRC))
        {
            const std::string& wopiResponse = httpResponse->getBody();

            std::ostringstream oss;
            oss << "WOPI::GetFile " << (failed ? "failed" : "returned") << " for URI [" << uriAnonym
                << "]: " << httpResponse->statusLine().statusCode() << ' '
                << httpResponse->statusLine().reasonPhrase()
                << ". Headers: " << httpResponse->header()
                << (failed ? "\tBody: [" + COOLProtocol::getAbbreviatedMessage(wopiResponse) + ']'
                           : std::string());

            if (failed)
            {
                LOG_ERR(oss.str());
            }
            else
            {
                LOG_TRC(oss.str());
            }
        }

        if (failed)
        {
            if (httpResponse->statusLine().statusCode() == http::StatusCode::Forbidden)
            {
                LOG_ERR("Access denied to [" << uriAnonym << ']');
                HttpHelper::sendErrorAndShutdown(http::StatusCode::Forbidden, socket);
                return;
            }

            LOG_ERR("Invalid URI or access denied to [" << uriAnonym << ']');
            HttpHelper::sendErrorAndShutdown(http::StatusCode::Unauthorized, socket);
            return;
        }

        http::Response response(http::StatusCode::OK);
        response.setBody(httpResponse->getBody(), "application/octet-stream");
        socket->sendAndShutdown(response);
    };

    _httpSession->setFinishedHandler(std::move(finishedCallback));

    // Run the GET request on the WebServer Poll.
    _httpSession->asyncRequest(httpRequest, poll, false);
}
#endif //!MOBILEAPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
