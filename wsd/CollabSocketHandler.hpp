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

#pragma once

#include <net/WebSocketHandler.hpp>
#include <wsd/wopi/CheckFileInfo.hpp>
#include <net/HttpRequest.hpp>

#include <Poco/JSON/Object.h>

#include <map>
#include <memory>
#include <string>

class CollabBroker;
class StreamSocket;

namespace Poco::Net
{
class HTTPRequest;
}

/// WebSocket handler for /co/collab endpoint.
/// Accepts WOPISrc from URL, access_token from first message,
/// then validates via CheckFileInfo before allowing authentication.
class CollabSocketHandler : public WebSocketHandler
{
    std::string _wopiSrc;
    std::string _accessToken;
    bool _isAuthenticated = false;
    bool _isValidating = false;
    std::shared_ptr<CheckFileInfo> _checkFileInfo;
    std::weak_ptr<StreamSocket> _socketWeak;

    // Stored from successful CheckFileInfo validation
    std::string _docKey;
    std::string _userId;
    std::string _username;
    bool _userCanWrite = false;
    Poco::JSON::Object::Ptr _wopiInfo;

    /// The CollabBroker managing this handler (set after authentication)
    std::weak_ptr<CollabBroker> _broker;

    /// Unique handler ID within the broker
    std::string _handlerId;

public:
    CollabSocketHandler(const std::shared_ptr<StreamSocket>& socket,
                        const Poco::Net::HTTPRequest& request,
                        bool allowedOrigin,
                        const std::string& wopiSrc);

    void handleMessage(const std::vector<char>& payload) override;

    const std::string& getWopiSrc() const { return _wopiSrc; }
    const std::string& getAccessToken() const { return _accessToken; }
    bool isAuthenticated() const { return _isAuthenticated; }

    /// Returns the document key (available after successful authentication)
    const std::string& getDocKey() const { return _docKey; }

    /// Returns the user ID from WOPI (available after successful authentication)
    const std::string& getUserId() const { return _userId; }

    /// Returns the username from WOPI (available after successful authentication)
    const std::string& getUsername() const { return _username; }

    /// Returns whether the user can write (available after successful authentication)
    bool getUserCanWrite() const { return _userCanWrite; }

    /// Returns the raw WOPI info JSON (available after successful authentication)
    Poco::JSON::Object::Ptr getWopiInfo() const { return _wopiInfo; }

    /// Returns the CollabBroker managing this handler
    std::shared_ptr<CollabBroker> getBroker() const { return _broker.lock(); }

    /// Returns the unique handler ID (set after authentication)
    const std::string& getHandlerId() const { return _handlerId; }

    /// Sets the handler ID (called by CollabBroker::addHandler)
    void setHandlerId(const std::string& id) { _handlerId = id; }

    /// Called when the connection is closed
    void onDisconnect() override;

private:
    void startValidation();
    void onCheckFileInfoFinished(CheckFileInfo& cfi);

    /// Handle authenticated messages
    void handleAuthenticatedMessage(const std::string& msg);

    /// Handle fetch request - downloads a stream and sends result via WebSocket
    void handleFetch(const std::string& stream, const std::string& requestId,
                     const std::string& ifNoneMatch, const std::string& ifModifiedSince);

    /// Handle upload request - returns a one-time upload URL for HTTP PUT
    void handleUpload(const std::string& stream, const std::string& requestId);

    /// Handle editing_started notification - broadcast to other users
    void handleEditingStarted();

    /// Callback when fetch completes
    void onFetchComplete(const std::string& requestId, const std::string& stream,
                         const std::shared_ptr<http::Session>& session);

    /// Active fetch sessions by requestId
    std::map<std::string, std::shared_ptr<http::Session>> _fetchSessions;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
