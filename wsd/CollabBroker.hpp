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

#include <common/RotatingToken.hpp>

#include <Poco/JSON/Object.h>

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class CollabSocketHandler;
class StreamSocket;
class SocketDisposition;
class TerminatingPoll;

/// Token length for secure access tokens (32 bytes = 64 hex chars)
constexpr size_t CollabAccessTokenLength = 32;

/// How long before collab access tokens are rotated (5 minutes)
constexpr auto CollabAccessTokenRotation = std::chrono::minutes(5);

/// How long fetch tokens are valid (5 minutes)
constexpr auto CollabFetchTokenExpiry = std::chrono::minutes(5);

/// How long upload tokens are valid (5 minutes)
constexpr auto CollabUploadTokenExpiry = std::chrono::minutes(5);

/// Details for a pending fetch request (for HTTP download)
struct CollabFetchRequest
{
    std::string streamUrl;       ///< URL to fetch from
    std::string accessToken;     ///< WOPI access token
    std::string wopiSrc;         ///< Original WOPISrc for validation
    std::string docKey;          ///< Document key to verify live CollabBroker
    std::string brokerTag;       ///< Broker access token at time of request
    std::string requestId;       ///< Client-provided request ID
    std::string stream;          ///< Stream name (contents, userSettings, etc.)
    std::chrono::steady_clock::time_point expiry;  ///< When this token expires
};

/// Details for a pending upload request (for HTTP upload via PutFile)
struct CollabUploadRequest
{
    std::string targetUrl;       ///< WOPI /contents URL to upload to
    std::string accessToken;     ///< WOPI access token
    std::string wopiSrc;         ///< Original WOPISrc for validation
    std::string docKey;          ///< Document key to verify live CollabBroker
    std::string brokerTag;       ///< Broker access token at time of request
    std::string requestId;       ///< Client-provided request ID
    std::chrono::steady_clock::time_point expiry;  ///< When this token expires
};

/// Manages all CollabSocketHandler instances for a single document (docKey).
/// Similar to DocumentBroker, this class groups all collaboration WebSocket
/// connections that share the same WOPI source URL.
class CollabBroker : public std::enable_shared_from_this<CollabBroker>
{
    const std::string _docKey;
    const std::string _wopiSrc;

    /// Mutex protecting _handlers and other mutable state
    mutable std::mutex _mutex;

    /// Connected handlers
    std::vector<std::weak_ptr<CollabSocketHandler>> _handlers;

    /// Set when a user sends editing_started via the collab WebSocket.
    bool _editingStarted = false;

    /// WOPI info from the first authenticated handler (shared by all)
    Poco::JSON::Object::Ptr _wopiInfo;

    /// Rotating access tokens for secure download URLs
    RotatingToken _accessToken;

public:
    CollabBroker(const std::string& docKey, const std::string& wopiSrc);
    ~CollabBroker();

    const std::string& getDocKey() const { return _docKey; }
    const std::string& getWopiSrc() const { return _wopiSrc; }

    /// Add a handler to this broker. Called when handler authenticates.
    void addHandler(const std::shared_ptr<CollabSocketHandler>& handler);

    /// Remove a handler from this broker. Called when handler disconnects.
    void removeHandler(const std::shared_ptr<CollabSocketHandler>& handler);

    /// Returns the number of active handlers
    size_t getHandlerCount() const;

    /// Returns true if there are no active handlers
    bool isEmpty() const;

    /// Returns true if there are no handlers and no editing was started.
    /// Idle brokers can be cleaned up; brokers with _editingStarted set
    /// are kept so late joiners see editingActive in the user_list.
    bool isIdle() const;

    /// Set WOPI info (from first authenticated handler)
    void setWopiInfo(Poco::JSON::Object::Ptr wopiInfo);

    /// Get WOPI info
    Poco::JSON::Object::Ptr getWopiInfo() const;

    /// Broadcast a message to all handlers
    void broadcastMessage(const std::string& message);

    /// Broadcast a message to all handlers except the specified one.
    void broadcastExcluding(const std::string& message,
                            const std::shared_ptr<CollabSocketHandler>& exclude);

    /// Get JSON array of current users (excluding the specified handler)
    /// Returns JSON like: {"users": [{"id": "...", "name": "...", "canWrite": true}, ...]}
    std::string getUserListJson(const std::shared_ptr<CollabSocketHandler>& exclude) const;

    /// Notify all handlers that a user joined
    /// Sends: {"type": "user_joined", "user": {"id": "...", "name": "...", "canWrite": true}}
    void notifyUserJoined(const std::shared_ptr<CollabSocketHandler>& handler);

    /// Notify all handlers that a user left
    /// Sends: {"type": "user_left", "user": {"id": "...", "name": "..."}}
    void notifyUserLeft(const std::shared_ptr<CollabSocketHandler>& handler);

    /// Notify all handlers that a user started editing
    /// Sends: {"type": "editing_started", "user": {"id": "...", "name": "..."}}
    void notifyEditingStarted(const std::shared_ptr<CollabSocketHandler>& handler);

    /// Get the current access token for secure download URLs
    std::string getCurrentAccessToken() const;

    /// Rotate the access token (old current becomes previous, new current is generated)
    void rotateAccessToken();

    /// Check if a tag matches either the current or previous access token
    bool matchesAccessToken(const std::string& tag) const;

private:
    /// Generate a unique ID for a handler
    static std::string generateHandlerId();

    /// Clean up expired weak pointers
    void cleanupExpiredHandlers();
};

/// Global CollabBrokers map and mutex - follows same pattern as DocBrokers
extern std::map<std::string, std::shared_ptr<CollabBroker>> CollabBrokers;
extern std::mutex CollabBrokersMutex;

/// Find or create a CollabBroker for the given docKey.
/// Returns nullptr if shutting down.
std::shared_ptr<CollabBroker> findOrCreateCollabBroker(const std::string& docKey,
                                                        const std::string& wopiSrc);

/// Remove empty CollabBrokers (called during cleanup)
void cleanupCollabBrokers();

/// Global fetch requests map and mutex
extern std::map<std::string, CollabFetchRequest> CollabFetchRequests;
extern std::mutex CollabFetchRequestsMutex;

/// Create a fetch request and return the token.
/// The token can be used with /co/collab/fetch?token=... endpoint.
/// The brokerTag is the broker's current access token for validation.
std::string createCollabFetchRequest(const std::string& streamUrl,
                                      const std::string& accessToken,
                                      const std::string& wopiSrc,
                                      const std::string& docKey,
                                      const std::string& brokerTag,
                                      const std::string& requestId,
                                      const std::string& stream);

/// Find an existing CollabBroker by docKey (returns nullptr if not found or empty)
std::shared_ptr<CollabBroker> findCollabBroker(const std::string& docKey);

/// Look up and consume a fetch request by token.
/// Returns true if found and valid, fills in the request details.
bool consumeCollabFetchRequest(const std::string& token, CollabFetchRequest& request);

/// Clean up expired fetch requests
void cleanupCollabFetchRequests();

/// Global upload requests map and mutex
extern std::map<std::string, CollabUploadRequest> CollabUploadRequests;
extern std::mutex CollabUploadRequestsMutex;

/// Create an upload request and return the token.
/// The token can be used with /co/collab/put?token=... endpoint.
std::string createCollabUploadRequest(const std::string& targetUrl,
                                       const std::string& accessToken,
                                       const std::string& wopiSrc,
                                       const std::string& docKey,
                                       const std::string& brokerTag,
                                       const std::string& requestId);

/// Look up and consume an upload request by token.
/// Returns true if found and valid, fills in the request details.
bool consumeCollabUploadRequest(const std::string& token, CollabUploadRequest& request);

/// Clean up expired upload requests
void cleanupCollabUploadRequests();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
