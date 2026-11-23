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

#pragma once

#include <net/WebSocketHandler.hpp>
#include <wsd/wopi/CheckFileInfo.hpp>
#include <net/HttpRequest.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/URI.h>

#include <map>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

class CollabBroker;
class StreamSocket;

namespace Poco::Net
{
class HTTPRequest;
}

/// WebSocket handler for /cool/collab/ws endpoint.
/// Accepts WOPISrc from URL, access_token from first message,
/// then validates via CheckFileInfo before allowing authentication.
class CollabSocketHandler : public WebSocketHandler
{
    std::string _wopiSrc;
    /// The WOPISrc as it passed the WOPI host check, without the access token.
    Poco::URI _wopiUri;
    // The access token, without the URI encoding in which the client sends it:
    std::string _accessToken;
    bool _isAuthenticated = false;
    bool _isValidating = false;
    std::shared_ptr<CheckFileInfo> _checkFileInfo;
    // Whether the sign-in's CheckFileInfo was tried again after a temporary error or no answer:
    bool _checkFileInfoRetried = false;
    // The CheckFileInfo that a new access token from the client has to pass:
    std::shared_ptr<CheckFileInfo> _renewalCheckFileInfo;
    std::weak_ptr<StreamSocket> _socketWeak;
    // Whether the server can speak to the client, given the client's protocol number:
    bool _protocolAccepted;

    // Stored from successful CheckFileInfo validation
    std::string _docKey;
    std::string _userId;
    std::string _username;
    std::string _avatar;
    bool _userCanWrite = false;
    bool _allowsCleanExport = false;
    Poco::JSON::Object::Ptr _wopiInfo;

    /// The CollabBroker managing this handler (set after authentication)
    std::weak_ptr<CollabBroker> _broker;

    /// Unique handler ID within the broker
    std::string _handlerId;

    /// Set when the client sends a {"type":"bye"} message announcing
    /// it is intentionally leaving (e.g. the user closed the doc, or
    /// CODA-Q is switching out of local-edit mode).  Lets the broker
    /// distinguish an orderly disconnect (reclaim immediately - no
    /// reconnect expected) from a transient network drop (start the
    /// idle grace clock and wait).
    bool _byeReceived = false;

public:
    // The /cool/collab protocol number that this server speaks (see wsd/protocol.txt):
    static constexpr int Protocol = 1;

    CollabSocketHandler(const std::shared_ptr<StreamSocket>& socket,
                        const Poco::Net::HTTPRequest& request,
                        bool allowedOrigin,
                        const std::string& wopiSrc);

    /// Socket-free constructor for tests.
    CollabSocketHandler(const std::string& wopiSrc, const std::string& docKey,
                        const std::shared_ptr<CollabBroker>& broker);

    void handleMessage(const std::vector<char>& payload) override;

    /// Handle authenticated messages
    void handleAuthenticatedMessage(const std::string& msg);

    const std::string& getWopiSrc() const { return _wopiSrc; }
    const std::string& getAccessToken() const { return _accessToken; }
    bool isAuthenticated() const { return _isAuthenticated; }

    /// Returns the document key (available after successful authentication)
    const std::string& getDocKey() const { return _docKey; }

    /// Returns the user ID from WOPI (available after successful authentication)
    const std::string& getUserId() const { return _userId; }

    /// Returns the username from WOPI (available after successful authentication)
    const std::string& getUsername() const { return _username; }

    /// Returns the user's avatar URL from WOPI UserExtraInfo
    /// (may be empty)
    const std::string& getAvatar() const { return _avatar; }

    /// Returns the raw WOPI info JSON (available after successful authentication)
    Poco::JSON::Object::Ptr getWopiInfo() const { return _wopiInfo; }

    /// Returns the CollabBroker managing this handler
    std::shared_ptr<CollabBroker> getBroker() const { return _broker.lock(); }

    /// Returns the unique handler ID (set after authentication)
    const std::string& getHandlerId() const { return _handlerId; }

    /// Sets the handler ID (called by CollabBroker::addHandler)
    void setHandlerId(const std::string& id) { _handlerId = id; }

    /// True iff the client has announced via {"type":"bye"} that it
    /// is leaving voluntarily; CollabBroker::removeHandler uses this
    /// to decide between immediate reclaim and the idle grace
    /// period.
    bool isGracefulClose() const { return _byeReceived; }

    /// Called when the connection is closed
    void onDisconnect() override;

    /// The fields that hold a stored secret in the file at url, when stream is the fetch stream
    /// of a settings file. Empty for every other stream, and for a file that holds no secret.
    static std::span<const std::string_view> settingsSecretFields(const std::string& stream,
                                                                  const std::string& url);

private:
    void startValidation();
    void onCheckFileInfoFinished(CheckFileInfo& cfi);

    /// Handle fetch request - downloads a stream and sends result via WebSocket
    void handleFetch(const std::string& stream, const std::string& requestId,
                     const std::string& ifNoneMatch, const std::string& ifModifiedSince);

    /// Answers a fetch with fetch_error and the reason.
    void sendFetchError(const std::string& stream, const std::string& requestId,
                        const std::string& error);

    /// Answers a fetch with a one-time URL that downloads url.
    void sendFetchUrl(const std::string& stream, const std::string& requestId,
                      const std::string& url, bool hostChecked);

    /// The URI of the "shared" or the "user" settings list in the CheckFileInfo answer, or
    /// empty when the answer names none.
    std::string settingsListUri(const std::string& list) const;

    /// A settingsFile fetch that waits for this server's copy of its settings list.
    struct PendingSettingsFile
    {
        std::string requestId;
        std::string group;
        std::string uri;
    };

    /// Handle a fetch of one file that the "shared" or the "user" settings list names in group.
    void handleSettingsFileFetch(const std::string& requestId, const std::string& list,
                                 const std::string& group, const std::string& uri);

    /// Answers a settingsFile fetch from settings: a one-time URL when settings names the uri
    /// in the group, an error otherwise.
    void answerSettingsFileFetch(const Poco::JSON::Object::Ptr& settings,
                                 const PendingSettingsFile& request);

    /// Keeps the settings list that httpResponse holds and answers the fetches that wait for it.
    /// A null httpResponse means that the request could not start.
    void onSettingsListFetched(const std::string& list,
                               const std::shared_ptr<const http::Response>& httpResponse);

    /// The uri of the entry of group in settings that has the file name of uri. Empty when the
    /// group has no such entry.
    static std::string settingsListEntryUri(const Poco::JSON::Object::Ptr& settings,
                                            const std::string& group, const std::string& uri);

    /// Handle upload request - returns a one-time upload URL for HTTP PUT
    /// (one that is refused if storage no longer has lastModifiedTime, unless force)
    void handleUpload(const std::string& stream, const std::string& requestId,
                      const std::string& lastModifiedTime, bool force);

    /// Handle editing_started notification - broadcast to other users
    void handleEditingStarted();

    void renewAccessToken(std::string const & token);
    void onRenewalCheckFileInfoFinished(CheckFileInfo & cfi, std::string const & token);
    void takeRights(CheckFileInfo & cfi);

    /// Callback when fetch completes
    static void onFetchComplete(const std::string& requestId, const std::string& stream,
                         const std::shared_ptr<http::Session>& session);

    /// Active fetch sessions by requestId
    std::map<std::string, std::shared_ptr<http::Session>> _fetchSessions;

    /// The settings lists this server downloaded itself, by list name ("shared" or "user").
    std::map<std::string, Poco::JSON::Object::Ptr> _settingsLists;

    /// The settingsFile fetches that wait for the download of a settings list, by list name.
    std::map<std::string, std::vector<PendingSettingsFile>> _pendingSettingsFiles;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
