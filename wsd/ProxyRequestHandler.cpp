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
 * Implementation of proxy request handling.
 * Classes: ProxyRequestHandler
 */

#include <config.h>

#include "ProxyRequestHandler.hpp"

#include <common/Anonymizer.hpp>
#include <net/HttpHelper.hpp>
#include <net/HttpRequest.hpp>
#include <wsd/COOLWSD.hpp>

#include <Poco/URI.h>

Util::UnorderedStringMap<std::shared_ptr<http::Response>> ProxyRequestHandler::CacheFileHash;
std::chrono::system_clock::time_point ProxyRequestHandler::MaxAge;

void ProxyRequestHandler::handleRequest(const std::string& relPath,
                                        const std::shared_ptr<StreamSocket>& socket,
                                        const std::string& serverUri)
{

    Poco::URI uriProxy(serverUri);
    constexpr const auto zero = std::chrono::system_clock::time_point();
    const auto timeNow = std::chrono::system_clock::now();

    if (MaxAge > zero && timeNow > MaxAge)
    {
        CacheFileHash.clear();
        MaxAge = zero;
    }

    const auto cacheEntry = CacheFileHash.find(relPath);
    if (cacheEntry != CacheFileHash.end())
    {
        socket->sendAndShutdown(*cacheEntry->second);
        return;
    }

    uriProxy.setPath(relPath);
    auto sessionProxy = http::Session::create(uriProxy.getHost(),
                                              http::Session::Protocol::HttpSsl,
                                              uriProxy.getPort());
    sessionProxy->setTimeout(std::chrono::seconds(10));
    http::Request requestProxy(uriProxy.getPathAndQuery());
    std::weak_ptr<StreamSocket> socketWeak(socket);
    http::Session::FinishedCallback proxyCallback =
        [socketWeak, zero](const std::shared_ptr<http::Session>& httpSession)
            {
                std::shared_ptr<StreamSocket> destSocket = socketWeak.lock();
                if (!destSocket)
                {
                    const std::string uriAnonym = Anonymizer::anonymizeUrl(httpSession->getUrl());
                    LOG_ERR("Invalid socket ProxyRequestHandler while handling [" << uriAnonym << ']');
                    return;
                }

                try
                {
                    const auto callbackNow = std::chrono::system_clock::now();
                    std::shared_ptr<http::Response> httpResponse = httpSession->response();
                    if (httpResponse->statusLine().statusCode() == http::StatusCode::OK)
                    {
                        if (MaxAge == zero)
                        {
                            MaxAge = callbackNow + std::chrono::hours(10);
                        }

                        CacheFileHash[httpSession->getUrl()] = httpResponse;

                        // We're proxying, we take responsibility.
                        httpResponse->set("Server", http::getServerString());

                        destSocket->sendAndShutdown(*httpResponse);
                    }
                    else
                    {
                        HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, destSocket);
                    }
                }
                catch(std::exception& exc)
                {
                    LOG_ERR("ProxyCallback: " << exc.what());
                    HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, destSocket);
                }
                catch(...)
                {
                    LOG_ERR("ProxyCallback: Unknown exception");
                    HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, destSocket);
                }
            };

    sessionProxy->setFinishedHandler(std::move(proxyCallback));

    http::Session::ConnectFailCallback connectFailCallback =
        [socketWeak](const std::shared_ptr<http::Session>& session) {
            std::shared_ptr<StreamSocket> destSocket = socketWeak.lock();
            if (!destSocket)
            {
                const std::string uriAnonym = Anonymizer::anonymizeUrl(session->getUrl());
                LOG_ERR("Invalid socket ProxyRequestHandler while handling [" << uriAnonym << ']');
                return;
            }
            HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, destSocket);
    };
    sessionProxy->setConnectFailHandler(std::move(connectFailCallback));

    sessionProxy->asyncRequest(requestProxy, COOLWSD::getWebServerPoll(), false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
