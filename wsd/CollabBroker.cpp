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

#include "CollabBroker.hpp"
#include "CollabSocketHandler.hpp"

#include <Anonymizer.hpp>
#include <COOLWSD.hpp>
#include <JsonUtil.hpp>
#include <Log.hpp>
#include <SigUtil.hpp>
#include <Util.hpp>

#include <algorithm>
#include <atomic>
#include <sstream>

/// Global CollabBrokers map - keyed by docKey
std::map<std::string, std::shared_ptr<CollabBroker>> CollabBrokers;
std::mutex CollabBrokersMutex;

/// Global fetch requests map - keyed by token
std::map<std::string, CollabFetchRequest> CollabFetchRequests;
std::mutex CollabFetchRequestsMutex;

/// Global upload requests map - keyed by token
std::map<std::string, CollabUploadRequest> CollabUploadRequests;
std::mutex CollabUploadRequestsMutex;

namespace
{
std::atomic<uint64_t> HandlerIdCounter{0};
}

CollabBroker::CollabBroker(const std::string& docKey, const std::string& wopiSrc)
    : _docKey(docKey)
    , _wopiSrc(wopiSrc)
    , _accessToken(CollabAccessTokenLength, CollabAccessTokenRotation)
{
    LOG_INF("CollabBroker created for docKey [" << _docKey << ']');
}

CollabBroker::~CollabBroker()
{
    LOG_INF("CollabBroker destroyed for docKey [" << _docKey << ']');
}

std::string CollabBroker::generateHandlerId()
{
    return "collab_" + std::to_string(++HandlerIdCounter);
}

void CollabBroker::addHandler(const std::shared_ptr<CollabSocketHandler>& handler)
{
    std::lock_guard<std::mutex> lock(_mutex);

    const std::string handlerId = generateHandlerId();
    handler->setHandlerId(handlerId);
    _handlers.push_back(handler);

    LOG_INF("CollabBroker [" << _docKey << "]: added handler [" << handlerId
            << "], total handlers: " << _handlers.size());
}

void CollabBroker::removeHandler(const std::shared_ptr<CollabSocketHandler>& handler)
{
    std::lock_guard<std::mutex> lock(_mutex);

    const std::string handlerId = handler->getHandlerId();

    // Use erase-remove idiom to find and remove the specific handler
    auto it = std::find_if(_handlers.begin(), _handlers.end(),
                           [&handler](const std::weak_ptr<CollabSocketHandler>& h) {
                               auto locked = h.lock();
                               return locked && locked.get() == handler.get();
                           });

    if (it != _handlers.end())
    {
        LOG_INF("CollabBroker [" << _docKey << "]: removed handler [" << handlerId << ']');
        _handlers.erase(it);
    }

    cleanupExpiredHandlers();
    LOG_DBG("CollabBroker [" << _docKey << "]: remaining handlers: " << _handlers.size());
}

size_t CollabBroker::getHandlerCount() const
{
    std::lock_guard<std::mutex> lock(_mutex);

    size_t count = 0;
    for (const auto& handler : _handlers)
    {
        if (!handler.expired())
            ++count;
    }
    return count;
}

bool CollabBroker::isEmpty() const
{
    return getHandlerCount() == 0;
}

void CollabBroker::setWopiInfo(Poco::JSON::Object::Ptr wopiInfo)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_wopiInfo)
    {
        _wopiInfo = wopiInfo;
        LOG_DBG("CollabBroker [" << _docKey << "]: WOPI info set");
    }
}

Poco::JSON::Object::Ptr CollabBroker::getWopiInfo() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _wopiInfo;
}

void CollabBroker::broadcastMessage(const std::string& message)
{
    std::lock_guard<std::mutex> lock(_mutex);

    LOG_DBG("CollabBroker [" << _docKey << "]: broadcasting message to "
            << _handlers.size() << " handlers");

    for (auto& weakHandler : _handlers)
    {
        if (auto handler = weakHandler.lock())
        {
            handler->sendTextMessage(message);
        }
    }
}

std::string CollabBroker::getUserListJson(const std::shared_ptr<CollabSocketHandler>& exclude) const
{
    std::lock_guard<std::mutex> lock(_mutex);

    std::ostringstream oss;
    oss << "{\"type\":\"user_list\",\"users\":[";

    bool first = true;
    for (const auto& weakHandler : _handlers)
    {
        auto handler = weakHandler.lock();
        if (handler && handler.get() != exclude.get())
        {
            if (!first)
                oss << ',';
            first = false;

            oss << "{\"id\":\"" << JsonUtil::escapeJSONValue(handler->getUserId()) << "\""
                << ",\"name\":\"" << JsonUtil::escapeJSONValue(handler->getUsername()) << "\""
                << ",\"canWrite\":" << (handler->getUserCanWrite() ? "true" : "false")
                << '}';
        }
    }

    oss << "]}";
    return oss.str();
}

void CollabBroker::notifyUserJoined(const std::shared_ptr<CollabSocketHandler>& handler)
{
    std::ostringstream oss;
    oss << "{\"type\":\"user_joined\",\"user\":{"
        << "\"id\":\"" << JsonUtil::escapeJSONValue(handler->getUserId()) << "\""
        << ",\"name\":\"" << JsonUtil::escapeJSONValue(handler->getUsername()) << "\""
        << ",\"canWrite\":" << (handler->getUserCanWrite() ? "true" : "false")
        << "}}";
    const std::string message = oss.str();

    LOG_INF("CollabBroker [" << _docKey << "]: notifying user joined: "
            << Anonymizer::anonymize(handler->getUsername()));

    // Send to all handlers except the one that joined
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto& weakHandler : _handlers)
    {
        auto h = weakHandler.lock();
        if (h && h.get() != handler.get())
        {
            h->sendTextMessage(message);
        }
    }
}

void CollabBroker::notifyUserLeft(const std::shared_ptr<CollabSocketHandler>& handler)
{
    std::ostringstream oss;
    oss << "{\"type\":\"user_left\",\"user\":{"
        << "\"id\":\"" << JsonUtil::escapeJSONValue(handler->getUserId()) << "\""
        << ",\"name\":\"" << JsonUtil::escapeJSONValue(handler->getUsername()) << "\""
        << "}}";
    const std::string message = oss.str();

    LOG_INF("CollabBroker [" << _docKey << "]: notifying user left: "
            << Anonymizer::anonymize(handler->getUsername()));

    // Send to all remaining handlers
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto& weakHandler : _handlers)
    {
        auto h = weakHandler.lock();
        if (h && h.get() != handler.get())
        {
            h->sendTextMessage(message);
        }
    }
}

void CollabBroker::cleanupExpiredHandlers()
{
    // Called with _mutex held - use erase-remove idiom
    _handlers.erase(
        std::remove_if(_handlers.begin(), _handlers.end(),
                       [](const std::weak_ptr<CollabSocketHandler>& h) {
                           return h.expired();
                       }),
        _handlers.end());
}

std::string CollabBroker::getCurrentAccessToken() const
{
    // Rotate if needed (time-based)
    const_cast<CollabBroker*>(this)->_accessToken.rotateIfNeeded();
    return _accessToken.getCurrent();
}

void CollabBroker::rotateAccessToken()
{
    _accessToken.rotate();
    LOG_TRC("CollabBroker [" << _docKey << "]: access token rotated to "
            << _accessToken.getCurrent() << " (previous: "
            << _accessToken.getPrevious() << ')');
}

bool CollabBroker::matchesAccessToken(const std::string& tag) const
{
    if (tag.empty())
    {
        LOG_ERR("Invalid empty access token tag");
        return false;
    }

    // Rotate if needed before validation
    const_cast<CollabBroker*>(this)->_accessToken.rotateIfNeeded();

    if (_accessToken.matches(tag))
    {
        return true;
    }

    LOG_WRN("CollabBroker [" << _docKey << "]: access token mismatch - got ["
            << tag << "], expected [" << _accessToken.getCurrent() << "] or ["
            << _accessToken.getPrevious() << ']');
    return false;
}

std::shared_ptr<CollabBroker> findOrCreateCollabBroker(const std::string& docKey,
                                                        const std::string& wopiSrc)
{
    LOG_INF("Find or create CollabBroker for docKey [" << docKey << ']');

    std::unique_lock<std::mutex> lock(CollabBrokersMutex);

    // Check if shutting down
    if (SigUtil::getShutdownRequestFlag())
    {
        LOG_WRN("Shutdown requested, not creating new CollabBroker for docKey [" << docKey << ']');
        return nullptr;
    }

    // Look up existing broker
    auto it = CollabBrokers.find(docKey);
    if (it != CollabBrokers.end() && it->second)
    {
        LOG_DBG("Found existing CollabBroker for docKey [" << docKey << ']');
        return it->second;
    }

    // Create new broker
    LOG_DBG("Creating new CollabBroker for docKey [" << docKey << ']');
    auto broker = std::make_shared<CollabBroker>(docKey, wopiSrc);
    CollabBrokers.emplace(docKey, broker);

    LOG_TRC("Have " << CollabBrokers.size() << " CollabBrokers after inserting [" << docKey << ']');

    return broker;
}

void cleanupCollabBrokers()
{
    std::lock_guard<std::mutex> lock(CollabBrokersMutex);

    for (auto it = CollabBrokers.begin(); it != CollabBrokers.end(); )
    {
        if (it->second && it->second->isEmpty())
        {
            LOG_INF("Removing empty CollabBroker for docKey [" << it->first << ']');
            it = CollabBrokers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

std::string createCollabFetchRequest(const std::string& streamUrl,
                                      const std::string& accessToken,
                                      const std::string& wopiSrc,
                                      const std::string& docKey,
                                      const std::string& brokerTag,
                                      const std::string& requestId,
                                      const std::string& stream)
{
    // Generate a unique token
    const std::string token = Util::rng::getHexString(CollabAccessTokenLength);

    CollabFetchRequest request;
    request.streamUrl = streamUrl;
    request.accessToken = accessToken;
    request.wopiSrc = wopiSrc;
    request.docKey = docKey;
    request.brokerTag = brokerTag;
    request.requestId = requestId;
    request.stream = stream;
    // Token expires after configured duration
    request.expiry = std::chrono::steady_clock::now() + CollabFetchTokenExpiry;

    {
        std::lock_guard<std::mutex> lock(CollabFetchRequestsMutex);
        CollabFetchRequests[token] = std::move(request);
    }

    LOG_DBG("Created fetch request with token [" << token << "] for stream [" << stream
            << "] docKey [" << docKey << ']');
    return token;
}

std::shared_ptr<CollabBroker> findCollabBroker(const std::string& docKey)
{
    std::lock_guard<std::mutex> lock(CollabBrokersMutex);

    auto it = CollabBrokers.find(docKey);
    if (it != CollabBrokers.end() && it->second && !it->second->isEmpty())
    {
        return it->second;
    }
    return nullptr;
}

bool consumeCollabFetchRequest(const std::string& token, CollabFetchRequest& request)
{
    std::lock_guard<std::mutex> lock(CollabFetchRequestsMutex);

    auto it = CollabFetchRequests.find(token);
    if (it == CollabFetchRequests.end())
    {
        LOG_WRN("Fetch request not found for token [" << token << ']');
        return false;
    }

    // Check if expired
    if (std::chrono::steady_clock::now() > it->second.expiry)
    {
        LOG_WRN("Fetch request expired for token [" << token << ']');
        CollabFetchRequests.erase(it);
        return false;
    }

    request = std::move(it->second);
    CollabFetchRequests.erase(it);

    LOG_DBG("Consumed fetch request for token [" << token << "] stream [" << request.stream << ']');
    return true;
}

void cleanupCollabFetchRequests()
{
    std::lock_guard<std::mutex> lock(CollabFetchRequestsMutex);

    const auto now = std::chrono::steady_clock::now();
    for (auto it = CollabFetchRequests.begin(); it != CollabFetchRequests.end(); )
    {
        if (now > it->second.expiry)
        {
            LOG_DBG("Removing expired fetch request for token [" << it->first << ']');
            it = CollabFetchRequests.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

std::string createCollabUploadRequest(const std::string& targetUrl,
                                       const std::string& accessToken,
                                       const std::string& wopiSrc,
                                       const std::string& docKey,
                                       const std::string& brokerTag,
                                       const std::string& requestId)
{
    const std::string token = Util::rng::getHexString(CollabAccessTokenLength);

    CollabUploadRequest request;
    request.targetUrl = targetUrl;
    request.accessToken = accessToken;
    request.wopiSrc = wopiSrc;
    request.docKey = docKey;
    request.brokerTag = brokerTag;
    request.requestId = requestId;
    request.expiry = std::chrono::steady_clock::now() + CollabUploadTokenExpiry;

    {
        std::lock_guard<std::mutex> lock(CollabUploadRequestsMutex);
        CollabUploadRequests[token] = std::move(request);
    }

    LOG_DBG("Created upload request with token [" << token << "] for docKey [" << docKey << ']');
    return token;
}

bool consumeCollabUploadRequest(const std::string& token, CollabUploadRequest& request)
{
    std::lock_guard<std::mutex> lock(CollabUploadRequestsMutex);

    auto it = CollabUploadRequests.find(token);
    if (it == CollabUploadRequests.end())
    {
        LOG_WRN("Upload request not found for token [" << token << ']');
        return false;
    }

    if (std::chrono::steady_clock::now() > it->second.expiry)
    {
        LOG_WRN("Upload request expired for token [" << token << ']');
        CollabUploadRequests.erase(it);
        return false;
    }

    request = std::move(it->second);
    CollabUploadRequests.erase(it);

    LOG_DBG("Consumed upload request for token [" << token << ']');
    return true;
}

void cleanupCollabUploadRequests()
{
    std::lock_guard<std::mutex> lock(CollabUploadRequestsMutex);

    const auto now = std::chrono::steady_clock::now();
    for (auto it = CollabUploadRequests.begin(); it != CollabUploadRequests.end(); )
    {
        if (now > it->second.expiry)
        {
            LOG_DBG("Removing expired upload request for token [" << it->first << ']');
            it = CollabUploadRequests.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
