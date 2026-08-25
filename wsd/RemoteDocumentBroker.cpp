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

#include "RemoteDocumentBroker.hpp"

#include <common/Anonymizer.hpp>
#include <common/ConfigUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/SigUtil.hpp>
#include <common/StringVector.hpp>
#include <common/Uri.hpp>
#include <net/HttpRequest.hpp>
#include <net/Uri.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/RequestDetails.hpp>

#include <sstream>

std::unique_ptr<RemoteDocumentBroker> RemoteDocumentBroker::Instance;

namespace
{
/// Joins the given docKeys with commas. docKeys are URI-encoded strings, so
/// they contain neither commas nor spaces.
std::string joinDocKeyChain(const std::vector<std::string>& docKeyChain)
{
    std::string result;
    for (const std::string& docKey : docKeyChain)
    {
        if (!result.empty())
            result += ',';
        result += docKey;
    }

    return result;
}
} // namespace

HeadlessClientSession::HeadlessClientSession(const std::weak_ptr<RemoteDocument>& remoteDocument,
                                             const std::string& loadUrl,
                                             const std::string& docKeyChain)
    : WebSocketHandler(/*isClient=*/true, /*isMasking=*/true)
    , _remoteDocument(remoteDocument)
    , _loadUrl(loadUrl)
    , _docKeyChain(docKeyChain)
    , _state(State::Connecting)
{
}

bool HeadlessClientSession::connect(const std::string& uri, SocketPoll& poll)
{
    std::string scheme;
    std::string host;
    std::string port;
    std::string pathAndQuery;
    if (!net::parseUri(uri, scheme, host, port, pathAndQuery))
    {
        LOG_ERR("RemoteDoc: cannot parse the remote document URI ["
                << Anonymizer::anonymizeUrl(uri) << ']');
        return false;
    }

    const bool secure = scheme == "wss://" || scheme == "https://";
    if (port.empty())
        port = secure ? "443" : "80";

    LOG_DBG("RemoteDoc: connecting to " << host << ':' << port << " for ["
                                        << Anonymizer::anonymizeUrl(pathAndQuery) << ']');

    http::Request request(pathAndQuery);
    return wsRequest(request, host, port, secure, poll);
}

void HeadlessClientSession::shutdownSession()
{
    if (_state != State::Closed)
    {
        _state = State::Closed;
        shutdown(true, "Unsubscribed");
    }
}

void HeadlessClientSession::handleIncomingMessage(SocketDisposition& disposition)
{
    WebSocketHandler::handleIncomingMessage(disposition);

    if (_state == State::Connecting)
    {
        const std::shared_ptr<StreamSocket> socket = getSocket().lock();
        if (socket && socket->isWebSocket())
        {
            _state = State::Loading;
            sendHandshake();
        }
    }
}

void HeadlessClientSession::sendHandshake()
{
    LOG_DBG("RemoteDoc: WebSocket upgraded, sending the handshake");
    sendTextMessage("coolclient " + COOLProtocol::GetProtocolVersion());

    // The chain travels in the load message: a URL query value would be
    // decoded once per RequestDetails::sanitizeURI pass and the docKeys'
    // own encoding would not survive that.
    std::string loadMessage = "load url=" + _loadUrl + " readonly=1";
    if (!_docKeyChain.empty())
        loadMessage += " remotechain=" + _docKeyChain;
    sendTextMessage(loadMessage);
}

bool HeadlessClientSession::sendCommand(const std::string& command)
{
    // Only commands that cannot modify the remote document.
    static const std::vector<std::string> allowedPrefixes = {
        "status", "commandvalues ", "setclientpart ", "selectclientpart ",
        "clientvisiblearea ", "clientzoom ", "ping"
    };

    bool allowed = false;
    for (const std::string& prefix : allowedPrefixes)
    {
        if (command.starts_with(prefix))
        {
            allowed = true;
            break;
        }
    }

    if (!allowed)
    {
        LOG_WRN("RemoteDoc: refusing to send the command ["
                << COOLProtocol::getAbbreviatedMessage(command)
                << "] to the remote document: not read-only");
        return false;
    }

    if (_state != State::Live)
    {
        LOG_DBG("RemoteDoc: not sending [" << command << "]: the session is not live");
        return false;
    }

    sendTextMessage(command);
    return true;
}

void HeadlessClientSession::handleMessage(const std::vector<char>& data)
{
    const std::shared_ptr<RemoteDocument> remoteDocument = _remoteDocument.lock();
    if (!remoteDocument || _state == State::Closed)
        return;

    const std::string firstLine = COOLProtocol::getFirstLine(data.data(), data.size());
    const StringVector tokens = StringVector::tokenize(firstLine);
    if (tokens.empty())
        return;

    if (tokens.equals(0, "status:") || tokens.equals(0, "statusupdate:"))
    {
        if (_state != State::Live)
        {
            _state = State::Live;
            remoteDocument->onLive();
        }
        else
        {
            remoteDocument->onStructureChanged();
        }
    }
    else if (tokens.equals(0, "statechanged:") && tokens.size() > 1)
    {
        static const std::string modifiedPrefix = ".uno:ModifiedStatus=";
        const std::string payload = tokens[1];
        if (payload.starts_with(modifiedPrefix))
            remoteDocument->onModified(payload.substr(modifiedPrefix.size()) == "true");
    }
    else if (tokens.equals(0, "invalidatetiles:"))
    {
        // Either "part=<p> mode=<m> x=..." or "EMPTY, <p>[, <m>]".
        int part = -1;
        if (tokens.size() > 1 && !COOLProtocol::getTokenInteger(tokens[1], "part", part) &&
            tokens.equals(1, "EMPTY,") && tokens.size() > 2)
        {
            std::string partToken = tokens[2];
            if (!partToken.empty() && partToken.back() == ',')
                partToken.pop_back();
            part = std::atoi(partToken.c_str());
        }

        remoteDocument->onInvalidated(part);
    }
    else if (tokens.equals(0, "perm:") && tokens.size() > 1 && !tokens.equals(1, "readonly"))
    {
        // The URL and load options requested read-only; the command filter in
        // sendCommand stays the guarantee if the remote node grants more.
        LOG_WRN("RemoteDoc: the remote session has permission [" << tokens[1]
                                                                 << "] instead of readonly");
    }
    else if (tokens.equals(0, "error:"))
    {
        std::string cmd;
        std::string kind;
        COOLProtocol::getTokenStringFromMessage(firstLine, "cmd", cmd);
        COOLProtocol::getTokenStringFromMessage(firstLine, "kind", kind);
        if (cmd == "load" || cmd == "internal" || _state != State::Live)
        {
            LOG_ERR("RemoteDoc: the remote document failed to load: cmd=" << cmd
                                                                          << " kind=" << kind);
            _state = State::Closed;
            remoteDocument->onLoadFailed(kind.empty() ? std::string("faileddocloading") : kind);
            shutdown(true, "Load failed");
        }
        else
        {
            LOG_WRN("RemoteDoc: error from the remote document: " << firstLine);
        }
    }
    else
    {
        LOG_TRC("RemoteDoc: dropping message [" << COOLProtocol::getAbbreviatedMessage(firstLine)
                                                << ']');
    }
}

void HeadlessClientSession::onDisconnect()
{
    WebSocketHandler::onDisconnect();

    if (_state == State::Closed)
        return;

    _state = State::Closed;
    if (const std::shared_ptr<RemoteDocument> remoteDocument = _remoteDocument.lock())
        remoteDocument->onSessionClosed();
}

RemoteDocument::RemoteDocument(std::string docKey, std::string wopiSrc, std::string accessToken,
                               std::string serverUrl, RemoteDocumentBroker& broker)
    : _docKey(std::move(docKey))
    , _wopiSrc(std::move(wopiSrc))
    , _accessToken(std::move(accessToken))
    , _serverUrl(std::move(serverUrl))
    , _broker(broker)
    , _lastInvalidationFlush(std::chrono::steady_clock::now())
    , _reconnectAttempts(0)
    , _modified(false)
    , _everConnected(false)
{
}

void RemoteDocument::addConsumer(const std::string& localDocKey, Consumer consumer)
{
    LOG_INF("RemoteDoc: adding consumer [" << localDocKey << "] tag=" << consumer.tag
                                           << " to the remote document [" << _docKey << "], have "
                                           << _consumers.size());

    // A live session serves the new consumer right away.
    if (_session && _session->state() == HeadlessClientSession::State::Live)
        sendEvent(consumer, _wopiSrc, "event=connected");

    const std::string tag = consumer.tag;
    _consumers[std::make_pair(localDocKey, tag)] = std::move(consumer);
}

bool RemoteDocument::removeConsumer(const std::string& localDocKey, const std::string& tag)
{
    LOG_INF("RemoteDoc: removing consumer [" << localDocKey << "] tag=" << tag
                                             << " from the remote document [" << _docKey
                                             << "], have " << _consumers.size());

    for (auto it = _consumers.begin(); it != _consumers.end();)
    {
        if (it->first.first == localDocKey && (tag == "0" || it->first.second == tag))
        {
            sendEvent(it->second, _wopiSrc, "event=unsubscribed");
            it = _consumers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return _consumers.empty();
}

std::vector<std::string> RemoteDocument::getDocKeyChains() const
{
    std::vector<std::string> result;
    for (const auto& it : _consumers)
    {
        for (const std::string& docKey : it.second.docKeyChain)
        {
            if (std::find(result.begin(), result.end(), docKey) == result.end())
                result.push_back(docKey);
        }

        if (std::find(result.begin(), result.end(), it.first.first) == result.end())
            result.push_back(it.first.first);
    }

    return result;
}

bool RemoteDocument::connect()
{
    const char separator = _wopiSrc.find('?') == std::string::npos ? '?' : '&';
    const std::string documentUri = _wopiSrc + separator + "access_token=" +
                                    Uri::encode(_accessToken) + "&permission=readonly";

    const std::string encodedDocumentUri = Uri::encode(documentUri);
    const std::string target =
        _serverUrl + "/cool/" + encodedDocumentUri + "/ws?WOPISrc=" + Uri::encode(_wopiSrc);

    LOG_INF("RemoteDoc: opening a headless session to ["
            << Anonymizer::anonymizeUrl(_wopiSrc) << "] as [" << Anonymizer::anonymizeUrl(target)
            << ']');

    _session = std::make_shared<HeadlessClientSession>(weak_from_this(), encodedDocumentUri,
                                                       joinDocKeyChain(getDocKeyChains()));
    if (!_session->connect(target, _broker))
    {
        _session.reset();
        return false;
    }

    return true;
}

void RemoteDocument::shutdown()
{
    _reconnectTime = std::chrono::steady_clock::time_point();
    if (_session)
    {
        _session->shutdownSession();
        _session.reset();
    }
}

void RemoteDocument::checkTimers(const std::chrono::steady_clock::time_point now)
{
    if (!_pendingInvalidatedParts.empty() &&
        now - _lastInvalidationFlush >= RemoteDocumentBroker::InvalidationCoalescePeriod)
    {
        for (const int part : _pendingInvalidatedParts)
            broadcastEvent("event=invalidated part=" + std::to_string(part));

        _pendingInvalidatedParts.clear();
        _lastInvalidationFlush = now;
    }

    if (_reconnectTime != std::chrono::steady_clock::time_point() && now >= _reconnectTime)
    {
        _reconnectTime = std::chrono::steady_clock::time_point();
        LOG_INF("RemoteDoc: reconnecting to [" << _docKey << "], attempt " << _reconnectAttempts);
        if (!connect())
            onSessionClosed();
    }
}

void RemoteDocument::onLive()
{
    LOG_INF("RemoteDoc: the remote document [" << _docKey << "] is live");
    _reconnectAttempts = 0;
    _everConnected = true;
    broadcastEvent("event=connected");
}

void RemoteDocument::onStructureChanged() { broadcastEvent("event=structure"); }

void RemoteDocument::onModified(const bool modified)
{
    if (modified == _modified)
        return;

    _modified = modified;
    broadcastEvent(std::string("event=modified value=") + (modified ? "true" : "false"));
}

void RemoteDocument::onInvalidated(const int part)
{
    if (_pendingInvalidatedParts.empty())
        _lastInvalidationFlush = std::chrono::steady_clock::now() -
                                 RemoteDocumentBroker::InvalidationCoalescePeriod;

    _pendingInvalidatedParts.insert(part);
}

void RemoteDocument::onLoadFailed(const std::string& kind)
{
    LOG_ERR("RemoteDoc: loading the remote document [" << _docKey << "] failed: " << kind);
    broadcastEvent("event=failed kind=" + kind);
    shutdown();
}

void RemoteDocument::onSessionClosed()
{
    _session.reset();

    if (_consumers.empty())
        return;

    ++_reconnectAttempts;
    if (_reconnectAttempts > _broker.getReconnectAttemptLimit())
    {
        LOG_ERR("RemoteDoc: giving up on the remote document ["
                << _docKey << "] after " << (_reconnectAttempts - 1) << " reconnect attempts");
        broadcastEvent("event=failed kind=disconnected");
        return;
    }

    broadcastEvent("event=disconnected");

    std::chrono::seconds delay = RemoteDocumentBroker::ReconnectBaseDelay * (1 << (_reconnectAttempts - 1));
    delay = std::min(delay, std::chrono::duration_cast<std::chrono::seconds>(
                                RemoteDocumentBroker::ReconnectMaxDelay));
    _reconnectTime = std::chrono::steady_clock::now() + delay;
    LOG_INF("RemoteDoc: lost the connection to [" << _docKey << "], reconnecting in " << delay);
}

void RemoteDocument::broadcastEvent(const std::string& eventArguments)
{
    for (const auto& it : _consumers)
        sendEvent(it.second, _wopiSrc, eventArguments);
}

void RemoteDocument::sendEvent(const Consumer& consumer, const std::string& wopiSrc,
                               const std::string& eventArguments)
{
    const std::shared_ptr<DocumentBroker> docBroker = consumer.docBroker.lock();
    if (!docBroker)
        return;

    const std::string message =
        "remotedocevent tag=" + consumer.tag + " wopisrc=" + Uri::encode(wopiSrc) + ' ' +
        eventArguments;

    docBroker->addCallback([docBroker, message]() { docBroker->sendTextFrameToKit(message); });
}

void RemoteDocument::dumpState(std::ostream& os) const
{
    os << "\n    docKey: " << _docKey
       << "\n    state: "
       << (_session ? HeadlessClientSession::name(_session->state()) : "no session")
       << "\n    consumers: " << _consumers.size();
    for (const auto& it : _consumers)
        os << "\n      " << it.first.first << " tag: " << it.first.second;
    os << "\n    everConnected: " << _everConnected
       << "\n    reconnectAttempts: " << _reconnectAttempts
       << "\n    modified: " << _modified
       << "\n    pendingInvalidatedParts: " << _pendingInvalidatedParts.size() << '\n';
}

RemoteDocumentBroker::RemoteDocumentBroker()
    : SocketPoll("remotedocbroker")
    , _maxRemoteDocuments(ConfigUtil::getConfigValue<int>("remote_documents.max_remote_docs", 16))
    , _maxChainDepth(ConfigUtil::getConfigValue<int>("remote_documents.max_chain_depth", 3))
    , _reconnectAttemptLimit(
          ConfigUtil::getConfigValue<int>("remote_documents.reconnect_attempts", 5))
    , _threadStarted(false)
{
}

bool RemoteDocumentBroker::isEnabled()
{
    static const bool enabled =
        ConfigUtil::getConfigValue<bool>("remote_documents.enable", false);
    return enabled;
}

std::string RemoteDocumentBroker::getServerUrl()
{
    static const std::string serverUrl = []() -> std::string
    {
        const bool secure = ConfigUtil::isSslEnabled();

        std::string url =
            ConfigUtil::getConfigValue<std::string>("remote_documents.server_url", "");
        if (url.empty())
        {
            std::string authority = COOLWSD::ServerName;
            if (authority.empty() && COOLWSD::getClientPortNumber() > 0)
            {
                // Dial this server itself; the setup has no cluster proxy.
                authority = "127.0.0.1:" + std::to_string(COOLWSD::getClientPortNumber());
            }

            if (authority.empty())
                return std::string();

            url = (secure ? "wss://" : "ws://") + authority + COOLWSD::ServiceRoot;
        }
        else if (url.starts_with("http://"))
        {
            url.replace(0, 4, "ws");
        }
        else if (url.starts_with("https://"))
        {
            url.replace(0, 5, "wss");
        }
        else if (!url.starts_with("ws://") && !url.starts_with("wss://"))
        {
            url = (secure ? "wss://" : "ws://") + url;
        }

        while (!url.empty() && url.back() == '/')
            url.pop_back();

        LOG_INF("RemoteDoc: remote documents are dialed through [" << url << ']');
        return url;
    }();

    return serverUrl;
}

void RemoteDocumentBroker::subscribeAsync(RemoteDocumentRequest request)
{
    if (!_threadStarted.exchange(true))
        startThread();

    addCallback([this, request = std::move(request)]() { subscribe(request); });
}

void RemoteDocumentBroker::unsubscribeAsync(std::string wopiSrc, std::string accessToken,
                                            std::string localDocKey, std::string tag)
{
    if (!_threadStarted)
        return;

    addCallback([this, wopiSrc = std::move(wopiSrc), accessToken = std::move(accessToken),
                 localDocKey = std::move(localDocKey), tag = std::move(tag)]()
                { unsubscribe(wopiSrc, accessToken, localDocKey, tag); });
}

void RemoteDocumentBroker::pollingThread()
{
    LOG_INF("RemoteDoc: the RemoteDocumentBroker poll started");

    while (continuePolling() && !SigUtil::getShutdownRequestFlag())
    {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

        // A short deadline drives the invalidation coalescing and reconnects.
        const auto timeout = _remoteDocuments.empty()
                                 ? std::chrono::milliseconds(std::chrono::seconds(5))
                                 : std::chrono::duration_cast<std::chrono::milliseconds>(
                                       InvalidationCoalescePeriod / 2);
        pollUntilDeadline(now + timeout);

        now = std::chrono::steady_clock::now();
        for (const auto& it : _remoteDocuments)
            it.second->checkTimers(now);
    }

    LOG_INF("RemoteDoc: the RemoteDocumentBroker poll finished");
}

bool RemoteDocumentBroker::formsSubscriptionCycle(const std::string& targetDocKey,
                                                  const std::string& consumerDocKey) const
{
    ASSERT_CORRECT_THREAD();

    // Walk the subscription edges of the registry: a remote document is
    // subscribed to by its consumers and by the docKeys on their chains.
    // A path from the new target back to the new consumer means the new
    // link would close a loop.
    std::vector<std::string> pending{ targetDocKey };
    std::set<std::string> visited;
    while (!pending.empty())
    {
        const std::string current = std::move(pending.back());
        pending.pop_back();

        if (current == consumerDocKey)
            return true;

        if (!visited.insert(current).second)
            continue;

        for (const auto& it : _remoteDocuments)
        {
            const std::vector<std::string> subscribers = it.second->getDocKeyChains();
            if (std::find(subscribers.begin(), subscribers.end(), current) != subscribers.end())
                pending.push_back(it.second->getDocKey());
        }
    }

    return false;
}

void RemoteDocumentBroker::subscribe(const RemoteDocumentRequest& request)
{
    ASSERT_CORRECT_THREAD();

    const std::string docKey = RequestDetails::getDocKey(request.wopiSrc);

    LOG_INF("RemoteDoc: subscribing [" << request.localDocKey << "] to [" << docKey << ']');

    if (docKey == request.localDocKey ||
        std::find(request.docKeyChain.begin(), request.docKeyChain.end(), docKey) !=
            request.docKeyChain.end())
    {
        LOG_WRN("RemoteDoc: rejecting the subscription of [" << request.localDocKey << "] to ["
                                                             << docKey << "]: a connection cycle");
        reject(request, "cycledetected");
        return;
    }

    if (request.docKeyChain.size() + 1 > _maxChainDepth)
    {
        LOG_WRN("RemoteDoc: rejecting the subscription of ["
                << request.localDocKey << "] to [" << docKey << "]: chain depth "
                << request.docKeyChain.size() + 1 << " over the limit " << _maxChainDepth);
        reject(request, "chaindepth");
        return;
    }

    // The registry serializes subscriptions on this thread, so a subscription
    // whose target already reaches the subscriber through registered links is
    // caught here even when two documents subscribe to each other at the same
    // moment, before either connection chain has traveled.
    if (formsSubscriptionCycle(docKey, request.localDocKey))
    {
        LOG_WRN("RemoteDoc: rejecting the subscription of ["
                << request.localDocKey << "] to [" << docKey
                << "]: the target already subscribes to the subscriber");
        reject(request, "cycledetected");
        return;
    }

    const std::pair<std::string, std::string> key = makeKey(docKey, request.accessToken);
    auto it = _remoteDocuments.find(key);
    if (it == _remoteDocuments.end())
    {
        if (_remoteDocuments.size() >= _maxRemoteDocuments)
        {
            LOG_WRN("RemoteDoc: rejecting the subscription of ["
                    << request.localDocKey << "] to [" << docKey << "]: already have "
                    << _remoteDocuments.size() << " remote documents");
            reject(request, "limitreached");
            return;
        }

        auto remoteDocument = std::make_shared<RemoteDocument>(
            docKey, request.wopiSrc, request.accessToken, request.serverUrl, *this);
        remoteDocument->addConsumer(
            request.localDocKey,
            RemoteDocument::Consumer{ request.consumer, request.tag, request.docKeyChain });

        if (!remoteDocument->connect())
        {
            LOG_ERR("RemoteDoc: failed to connect to [" << docKey << ']');
            reject(request, "connectfailed");
            return;
        }

        _remoteDocuments.emplace(key, std::move(remoteDocument));
    }
    else
    {
        it->second->addConsumer(
            request.localDocKey,
            RemoteDocument::Consumer{ request.consumer, request.tag, request.docKeyChain });
    }
}

void RemoteDocumentBroker::unsubscribe(const std::string& wopiSrc, const std::string& accessToken,
                                       const std::string& localDocKey, const std::string& tag)
{
    ASSERT_CORRECT_THREAD();

    const std::string docKey = RequestDetails::getDocKey(wopiSrc);
    const auto it = _remoteDocuments.find(makeKey(docKey, accessToken));
    if (it == _remoteDocuments.end())
    {
        LOG_DBG("RemoteDoc: no remote document [" << docKey << "] to unsubscribe ["
                                                  << localDocKey << "] from");
        return;
    }

    if (it->second->removeConsumer(localDocKey, tag))
    {
        LOG_INF("RemoteDoc: the last consumer of [" << docKey << "] left, disconnecting");
        it->second->shutdown();
        _remoteDocuments.erase(it);
    }
}

void RemoteDocumentBroker::reject(const RemoteDocumentRequest& request, const std::string& kind)
{
    RemoteDocument::Consumer consumer{ request.consumer, request.tag, {} };
    RemoteDocument::sendEvent(consumer, request.wopiSrc, "event=error kind=" + kind);

    // The consumer recorded the subscription when sending it; a rejection
    // removes that record again.
    if (std::shared_ptr<DocumentBroker> docBroker = request.consumer.lock())
    {
        docBroker->addCallback([docBroker, wopiSrc = request.wopiSrc, tag = request.tag]()
                               { docBroker->removeRemoteSubscription(wopiSrc, tag); });
    }
}

void RemoteDocumentBroker::dumpState(std::ostream& os) const
{
    os << "RemoteDocumentBroker: " << _remoteDocuments.size() << " remote documents:\n";
    for (const auto& it : _remoteDocuments)
        it.second->dumpState(os);

    SocketPoll::dumpState(os);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
