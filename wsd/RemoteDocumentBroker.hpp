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

#include <net/Socket.hpp>
#include <net/WebSocketHandler.hpp>

#include <atomic>
#include <cassert>
#include <chrono>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

class DocumentBroker;
class RemoteDocument;
class RemoteDocumentBroker;

/// A request from a local document to subscribe to a remote document.
struct RemoteDocumentRequest
{
    /// The remote document's WOPISrc, decoded.
    std::string wopiSrc;
    /// Access token valid for the remote WOPISrc.
    std::string accessToken;
    /// The originating view's session id, echoed back in every event so the
    /// reply reaches the one view that opened the subscription.
    std::string tag;
    /// docKey of the consumer document.
    std::string localDocKey;
    /// docKeys of the documents already on the connection chain leading to the consumer.
    std::vector<std::string> docKeyChain;
    /// Base URL to dial: scheme://authority plus the service root, no trailing slash.
    std::string serverUrl;
    /// The consumer document's broker, target of the event messages.
    std::weak_ptr<DocumentBroker> consumer;
};

/// An outbound WebSocket connection to a remote coolwsd node, joined to one
/// document there as a read-only client speaking the browser protocol.
class HeadlessClientSession final : public WebSocketHandler
{
public:
    enum class State : char
    {
        Connecting, ///< TCP/TLS connect and WebSocket upgrade in progress.
        Loading, ///< Handshake sent, waiting for the first status: message.
        Live, ///< The remote document is loaded and events are flowing.
        Closed ///< Disconnected; a new session object is needed to reconnect.
    };

    HeadlessClientSession(const std::weak_ptr<RemoteDocument>& remoteDocument,
                          const std::string& loadUrl, const std::string& docKeyChain);

    /// Connect to the given ws(s):// URI and add the socket to the poll.
    /// Returns false when the connection could not be established.
    bool connect(const std::string& uri, SocketPoll& poll);

    /// Close the WebSocket. The remote node then reaps its session normally.
    void shutdownSession();

    /// Sends a command to the remote document. Only commands that cannot
    /// modify the document are allowed; anything else is dropped with a log.
    /// Returns false when refused or when the session is not live.
    bool sendCommand(const std::string& command);

    State state() const { return _state; }

    static std::string_view name(State state)
    {
        switch (state)
        {
            case State::Connecting:
                return "Connecting";
            case State::Loading:
                return "Loading";
            case State::Live:
                return "Live";
            case State::Closed:
                return "Closed";
        }
        return "Unknown";
    }

private:
    /// Completes the client upgrade in the base class, then sends the
    /// coolclient/load handshake once the socket has switched to WebSocket mode.
    void handleIncomingMessage(SocketDisposition& disposition) override;

    void handleMessage(const std::vector<char>& data) override;

    void onDisconnect() override;

    void sendHandshake();

    std::weak_ptr<RemoteDocument> _remoteDocument;

    /// The value of the url= option in the load message.
    const std::string _loadUrl;

    /// The value of the remotechain= option in the load message: a comma
    /// list of the docKeys already on the connection chain.
    const std::string _docKeyChain;

    State _state;
};

/// One headless connection to a remote document, private to the one view that
/// opened it: the connection is keyed by the remote document, the access
/// token, the consumer document and the view's tag, so two views never share
/// it. Owns the headless session and sends its events to that view.
class RemoteDocument final : public std::enable_shared_from_this<RemoteDocument>
{
public:
    struct Consumer
    {
        std::weak_ptr<DocumentBroker> docBroker;
        std::string tag;
        std::vector<std::string> docKeyChain;
    };

    RemoteDocument(std::string docKey, std::string wopiSrc, std::string accessToken,
                   std::string serverUrl, RemoteDocumentBroker& broker);

    const std::string& getDocKey() const { return _docKey; }
    const std::string& getWopiSrc() const { return _wopiSrc; }
    const std::string& getAccessToken() const { return _accessToken; }

    void addConsumer(const std::string& localDocKey, Consumer consumer);

    /// Removes the consumer with the given docKey and tag, or every consumer
    /// of that docKey when the tag is "0", sending each an unsubscribed
    /// event. Returns true when the last consumer was removed.
    bool removeConsumer(const std::string& localDocKey, const std::string& tag);

    /// Sends a read-only client command to the remote document on behalf of
    /// one browser view, and remembers that view so the remote's replies are
    /// routed back to it.
    void sendCommand(const std::string& localDocKey, const std::string& tag,
                     const std::string& command);

    /// Forwards a raw frame from the remote document to every view that has
    /// issued a command, wrapped as a remotedoccommandresult.
    void forwardCommandResult(const std::vector<char>& data);

    size_t getConsumerCount() const { return _consumers.size(); }

    /// The docKeys any further connection made on behalf of this remote
    /// document would carry: the union of the consumers' chains plus the
    /// consumers' own docKeys.
    std::vector<std::string> getDocKeyChains() const;

    /// Opens the headless session. Returns false when dialing failed
    /// outright; a reconnect is then scheduled by the caller.
    bool connect();

    /// Closes the headless session and stops any reconnect.
    void shutdown();

    /// Flushes coalesced invalidations and performs a due reconnect.
    void checkTimers(std::chrono::steady_clock::time_point now);

    // Notifications from the headless session, on the broker poll thread.
    void onLive();
    void onStructureChanged();
    void onModified(bool modified);
    void onInvalidated(int part);
    void onLoadFailed(const std::string& kind);
    void onSessionClosed();

    void dumpState(std::ostream& os) const;

    /// Sends "remotedocevent tag=<t> wopisrc=<enc> <eventArguments>" to a single consumer.
    static void sendEvent(const Consumer& consumer, const std::string& wopiSrc,
                          const std::string& eventArguments);

private:
    /// Sends the event to every consumer.
    void broadcastEvent(const std::string& eventArguments);

    const std::string _docKey;
    const std::string _wopiSrc;
    const std::string _accessToken;
    const std::string _serverUrl;
    RemoteDocumentBroker& _broker;

    /// The one view this connection belongs to, keyed by (consumer's docKey,
    /// view tag). A connection is not shared, so this holds a single entry.
    std::map<std::pair<std::string, std::string>, Consumer> _consumers;

    /// The views that have sent a command to this remote document, as
    /// (consumer's docKey, view tag) pairs. The remote's replies go back to
    /// these views.
    std::set<std::pair<std::string, std::string>> _commandSubscribers;

    std::shared_ptr<HeadlessClientSession> _session;

    /// Parts with invalidations not yet forwarded. Part -1 covers documents
    /// without parts.
    std::set<int> _pendingInvalidatedParts;
    std::chrono::steady_clock::time_point _lastInvalidationFlush;

    /// When to attempt the next reconnect; zero when none is due.
    std::chrono::steady_clock::time_point _reconnectTime;
    unsigned _reconnectAttempts;

    /// The last modified state seen, to forward only changes.
    bool _modified;

    bool _everConnected;
};

/// Manages the headless sessions of this coolwsd process on its own poll
/// thread. One instance per process, like Admin.
class RemoteDocumentBroker final : public SocketPoll
{
    RemoteDocumentBroker(const RemoteDocumentBroker&) = delete;
    RemoteDocumentBroker& operator=(const RemoteDocumentBroker&) = delete;
    RemoteDocumentBroker();

    static std::unique_ptr<RemoteDocumentBroker> Instance;

public:
    /// The remote_documents.enable configuration value.
    static bool isEnabled();

    /// The base URL to dial remote documents through, in the ws(s) scheme
    /// with the service root appended, no trailing slash. Built from the
    /// remote_documents.server_url configuration, the server_name
    /// configuration, or this server's own address, in that order; never
    /// from request data. Empty when none of the sources is available.
    static std::string getServerUrl();

    static void initialize()
    {
        assert(Instance == nullptr && "Unexpected double initialization of RemoteDocumentBroker");
        Instance.reset(new RemoteDocumentBroker);
    }

    static void uninitialize()
    {
        if (Instance)
        {
            Instance->stop();
            Instance->joinThread();
            Instance.reset();
        }
    }

    static bool isInitialized() { return Instance != nullptr; }

    static RemoteDocumentBroker& instance()
    {
        assert(Instance && "Expected a valid RemoteDocumentBroker instance");
        return *Instance;
    }

    /// Subscribes a local document to a remote one. Callable from any thread.
    /// The outcome arrives at the consumer as a remotedocevent message.
    void subscribeAsync(RemoteDocumentRequest request);

    /// Drops a consumer of a remote document: the link with the given tag,
    /// or every link of the document when the tag is "0". Callable from any
    /// thread.
    void unsubscribeAsync(std::string wopiSrc, std::string accessToken, std::string localDocKey,
                          std::string tag);

    /// Routes a read-only client command from one browser view to the remote
    /// document identified by the WOPISrc and access token. Callable from any
    /// thread.
    void sendCommandAsync(std::string wopiSrc, std::string accessToken, std::string localDocKey,
                          std::string tag, std::string command);

    void dumpState(std::ostream& os) const override;

    /// How long invalidations are coalesced before one event is sent.
    static constexpr std::chrono::milliseconds InvalidationCoalescePeriod{ 500 };

    /// The base delay before the first reconnect attempt; doubled per attempt.
    static constexpr std::chrono::seconds ReconnectBaseDelay{ 2 };
    static constexpr std::chrono::seconds ReconnectMaxDelay{ 60 };

    /// The configured maximum number of reconnect attempts.
    unsigned getReconnectAttemptLimit() const { return _reconnectAttemptLimit; }

private:
    void pollingThread() override;

    /// All private methods below run on this poll's thread.
    void subscribe(const RemoteDocumentRequest& request);
    void unsubscribe(const std::string& wopiSrc, const std::string& accessToken,
                     const std::string& localDocKey, const std::string& tag);
    void sendCommand(const std::string& wopiSrc, const std::string& accessToken,
                     const std::string& localDocKey, const std::string& tag,
                     const std::string& command);

    /// Returns true when a subscription of the given consumer to the given
    /// target would close a loop through the links already in the registry.
    bool formsSubscriptionCycle(const std::string& targetDocKey,
                                const std::string& consumerDocKey) const;

    /// Rejects the request with "event=error kind=<kind>" sent to the consumer.
    static void reject(const RemoteDocumentRequest& request, const std::string& kind);

    /// The key of one headless connection. A connection is private to one
    /// view: the remote docKey and access token, plus the consumer document's
    /// docKey and the view's tag. Two views never share a connection, even
    /// with the same token.
    using Key = std::tuple<std::string, std::string, std::string, std::string>;

    static Key makeKey(const std::string& docKey, const std::string& accessToken,
                       const std::string& localDocKey, const std::string& tag)
    {
        return std::make_tuple(docKey, accessToken, localDocKey, tag);
    }

    /// Keyed by (remote docKey, access token, consumer docKey, view tag).
    std::map<Key, std::shared_ptr<RemoteDocument>> _remoteDocuments;

    const size_t _maxRemoteDocuments;
    const unsigned _maxChainDepth;
    const unsigned _reconnectAttemptLimit;

    std::atomic<bool> _threadStarted;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
