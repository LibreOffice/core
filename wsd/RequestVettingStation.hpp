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
 * Request validation and authorization gateway.
 * Classes: RequestVettingStation
 */

#pragma once

#include <common/Util.hpp>
#include <net/WebSocketHandler.hpp>
#include <wsd/RequestDetails.hpp>
#include <wsd/Storage.hpp>

#include <Poco/URI.h>

#include <string>
#include <string_view>

class CheckFileInfo;
class PresetsInstallTask;

/// RequestVettingStation is used to vet the request in the background.
/// Vetting for a WOPI request is performed through CheckFileInfo.
/// Once the request checks out, we can proceed to creating a
/// DocBroker and a Kit process.
/// There are two ways to use this class. One is to create it when
/// serving cool.html, the other when the WebSocket is created
/// (by upgrading the socket).
/// Unfortunately, when serving cool.html the connection is not the one
/// used for the WebSocket. As such, it cannot be used to create
/// DocBroker. Therefore, we work in two modes: we do the CheckFileInfo
/// as soon as we serve cool.html, but then we need to wait for the
/// WebSocket to create DocBroker.
/// A small complication is that CheckFileInfo might not be done by
/// then. Or, it might have timed out. Alternatively, the WebSocket
/// might never arrive (say, because the user clicked away).
/// We take these possibilities into account and support them here.
class RequestVettingStation final : public std::enable_shared_from_this<RequestVettingStation>
{
public:
    /// Create an instance with a SocketPoll and a RequestDetails instance.
    RequestVettingStation(const std::shared_ptr<TerminatingPoll>& poll,
                          const RequestDetails& requestDetails)
        : _requestDetails(requestDetails)
        , _poll(poll)
        , _wopiPostReceived(std::chrono::steady_clock::now())
        , _mobileAppDocId(0)
    {
    }

    void logPrefix(std::ostream& os) const
    {
        auto socket = _socket.lock();
        int logContextFD = socket ? socket->getFD() : -1;
        os << '#' << logContextFD << ": ";
    }

    /// Called when cool.html is served, to start the vetting as early as possible.
    void handleRequest(const std::string& id);

    /// Called when the WebSocket is connected (i.e. after cool.html is loaded in the browser).
    void handleRequest(const std::string& id, const RequestDetails& requestDetails,
                       const std::shared_ptr<WebSocketHandler>& ws,
                       const std::shared_ptr<StreamSocket>& socket, unsigned mobileAppDocId,
                       SocketDisposition& disposition);

#if !MOBILEAPP
    /// Attempt to create a DocBroker and setup a transfer via disposition
    /// of disposition's Socket to the DocBrokers SocketPoll
    void transferToDocBroker(const std::string& url,
                             const std::string& configId,
                             const std::string& sslVerifyResult);
#endif

    /// Returns true iff we are older than the given age.
    template <typename T>
    bool aged(T minAge,
              std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now()) const
    {
        return _birthday.elapsed<T>(minAge, now);
    }

private:
    std::shared_ptr<DocumentBroker> createDocBroker(const std::string& docKey,
                                                    const std::string& configId,
                                                    const std::string& url,
                                                    const Poco::URI& uriPublic);

    void createClientSession(const std::shared_ptr<DocumentBroker>& docBroker,
                             const std::string& docKey, const std::string& url,
                             const Poco::URI& uriPublic);

    /// Send unauthorized error to the client and disconnect the socket.
    /// Includes SSL verification status, if available, as the error code.
    void sendUnauthorizedErrorAndShutdown();

    /// Send an error to the client and disconnect the socket.
    void sendErrorAndShutdown(std::string_view msg, WebSocketHandler::StatusCodes statusCode);

#if !MOBILEAPP
    void launchInstallPresets();

    void checkFileInfo(const Poco::URI& uri, int redirectionLimit);
    std::shared_ptr<CheckFileInfo> _checkFileInfo;
    std::shared_ptr<PresetsInstallTask> _asyncInstallTask;
#endif // !MOBILEAPP

    RequestDetails _requestDetails;
    std::string _id;
    std::shared_ptr<TerminatingPoll> _poll;
    std::shared_ptr<WebSocketHandler> _ws;
    std::weak_ptr<StreamSocket> _socket;
    Util::Stopwatch _birthday;
    std::chrono::steady_clock::time_point _wopiPostReceived;
    std::chrono::steady_clock::time_point _checkFileInfoStart;
    std::chrono::steady_clock::time_point _checkFileInfoEnd;
    unsigned _mobileAppDocId;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
