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

#include "RelatedDocuments.hpp"

#include <common/Anonymizer.hpp>
#include <common/ConfigUtil.hpp>
#include <common/Log.hpp>
#include <common/Message.hpp>
#include <common/Protocol.hpp>
#include <common/StringVector.hpp>
#include <common/Uri.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/HostUtil.hpp>
#include <wsd/RemoteDocumentBroker.hpp>
#include <wsd/RequestDetails.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/URI.h>

#include <algorithm>
#include <sstream>

void RelatedDocuments::setToken(DocumentBroker& docBroker, const std::string& wopiSrc,
                                const std::string& accessToken)
{
    docBroker.assertCorrectThread();

    // Key by the docKey, so encoded and decoded spellings of the same
    // WOPISrc resolve to one entry.
    try
    {
        const std::string docKey = RequestDetails::getDocKey(wopiSrc);
        _tokens[docKey] = accessToken;
        _entries[docKey].wopiSrc = wopiSrc.substr(0, wopiSrc.find('?'));
        refresh(docBroker);
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Ignoring the access token for the invalid remote document WOPISrc ["
                << Anonymizer::anonymizeUrl(wopiSrc) << "]: " << exc.what());
    }
}

void RelatedDocuments::handleMessage(DocumentBroker& docBroker,
                                     const std::shared_ptr<Message>& message,
                                     const bool subscribe)
{
    docBroker.assertCorrectThread();

    std::string tag;
    std::string encodedWopiSrc;
    COOLProtocol::getTokenString((*message)[1], "tag", tag);
    COOLProtocol::getTokenString((*message)[2], "wopisrc", encodedWopiSrc);
    if (tag.empty() || encodedWopiSrc.empty())
    {
        LOG_ERR("Missing tag or wopisrc in [" << message->abbr() << ']');
        return;
    }

    const std::string wopiSrc = Uri::decode(encodedWopiSrc);

    LOG_INF("Remote document " << (subscribe ? "subscribe" : "unsubscribe") << " tag=" << tag
                               << " to [" << Anonymizer::anonymizeUrl(wopiSrc) << ']');

    if (!RemoteDocumentBroker::isEnabled() || !RemoteDocumentBroker::isInitialized())
    {
        LOG_ERR("Remote document subscribe tag=" << tag
                                                 << " rejected: remote_documents is disabled in "
                                                    "the configuration");
        sendError(docBroker, tag, encodedWopiSrc, "disabled");
        return;
    }

    // Match subscriptions and tokens on the canonical docKey, so encoded and
    // decoded spellings of the same WOPISrc refer to one document.
    std::string remoteDocKey;
    try
    {
        remoteDocKey = RequestDetails::getDocKey(wopiSrc);

        const Poco::URI wopiSrcUri(wopiSrc);
        if (subscribe && !HostUtil::allowedWopiHost(wopiSrcUri.getHost()))
        {
            LOG_WRN("Remote document [" << Anonymizer::anonymizeUrl(wopiSrc)
                                        << "] is not on an allowed WOPI host");
            sendError(docBroker, tag, encodedWopiSrc, "hostnotallowed");
            return;
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Invalid remote document WOPISrc: " << exc.what());
        sendError(docBroker, tag, encodedWopiSrc, "syntax");
        return;
    }

    if (!subscribe)
    {
        // Tag 0 drops every link of this document to the given WOPISrc; an
        // explicit tag drops only that link.
        for (auto it = _subscriptions.begin(); it != _subscriptions.end();)
        {
            if (RequestDetails::getDocKey(std::get<0>(*it)) == remoteDocKey &&
                (tag == "0" || std::get<2>(*it) == tag))
            {
                RemoteDocumentBroker::instance().unsubscribeAsync(
                    std::get<0>(*it), std::get<1>(*it), docBroker.getDocKey(), std::get<2>(*it));
                it = _subscriptions.erase(it);
            }
            else
            {
                ++it;
            }
        }

        refresh(docBroker);
        return;
    }

    const auto itToken = _tokens.find(remoteDocKey);
    if (itToken == _tokens.end())
    {
        std::ostringstream registered;
        for (const auto& it : _tokens)
            registered << " [" << it.first << ']';
        LOG_ERR("Remote document subscribe tag="
                << tag << " rejected: no access token registered for [" << remoteDocKey
                << "], have " << _tokens.size()
                << " from RelatedDocuments or POST /cool/relateddocument:" << registered.str());
        sendError(docBroker, tag, encodedWopiSrc, "notoken");
        return;
    }

    const std::string serverUrl = RemoteDocumentBroker::getServerUrl();
    if (serverUrl.empty())
    {
        LOG_ERR("Remote document subscribe tag="
                << tag
                << " rejected: no server URL to dial through; set remote_documents.server_url "
                   "or server_name in the configuration");
        sendError(docBroker, tag, encodedWopiSrc, "noserver");
        return;
    }

    static const size_t maxLinks =
        ConfigUtil::getConfigValue<int>("remote_documents.max_links_per_document", 4);
    if (_subscriptions.size() >= maxLinks)
    {
        LOG_ERR("Remote document subscribe tag="
                << tag << " rejected: [" << docBroker.getDocKey() << "] already holds "
                << _subscriptions.size() << " subscriptions of the maximum " << maxLinks);
        sendError(docBroker, tag, encodedWopiSrc, "limitreached");
        return;
    }

    RemoteDocumentRequest request;
    request.wopiSrc = wopiSrc;
    request.accessToken = itToken->second;
    request.tag = tag;
    request.localDocKey = docBroker.getDocKey();
    request.docKeyChain = _incomingDocKeyChain;
    request.serverUrl = serverUrl;
    request.consumer = docBroker.shared_from_this();

    _subscriptions.emplace(wopiSrc, itToken->second, tag);
    setSubscribed(docBroker, remoteDocKey);

    RemoteDocumentBroker::instance().subscribeAsync(std::move(request));
}

void RelatedDocuments::sendEvent(DocumentBroker& docBroker, const std::string& tag,
                                 const std::string& encodedWopiSrc,
                                 const std::string& eventArguments)
{
    docBroker.assertCorrectThread();

    docBroker.sendTextFrameToKit("remotedocevent tag=" + tag + " wopisrc=" + encodedWopiSrc +
                                 ' ' + eventArguments);

    onEvent(docBroker, encodedWopiSrc, eventArguments);
}

void RelatedDocuments::sendError(DocumentBroker& docBroker, const std::string& tag,
                                 const std::string& encodedWopiSrc, const std::string& kind)
{
    sendEvent(docBroker, tag, encodedWopiSrc, "event=error kind=" + kind);
}

void RelatedDocuments::removeSubscription(DocumentBroker& docBroker, const std::string& wopiSrc,
                                          const std::string& tag)
{
    docBroker.assertCorrectThread();

    for (auto it = _subscriptions.begin(); it != _subscriptions.end();)
    {
        if (std::get<0>(*it) == wopiSrc && std::get<2>(*it) == tag)
            it = _subscriptions.erase(it);
        else
            ++it;
    }

    refresh(docBroker);
}

void RelatedDocuments::unsubscribeAll(DocumentBroker& docBroker)
{
    if (_subscriptions.empty() || !RemoteDocumentBroker::isInitialized())
        return;

    LOG_DBG("Dropping " << _subscriptions.size() << " remote document subscriptions of ["
                        << docBroker.getDocKey() << ']');

    for (const auto& it : _subscriptions)
        RemoteDocumentBroker::instance().unsubscribeAsync(std::get<0>(it), std::get<1>(it),
                                                          docBroker.getDocKey(), std::get<2>(it));

    _subscriptions.clear();
}

void RelatedDocuments::sendCommand(DocumentBroker& docBroker, const std::string& sessionId,
                                   const std::string& wopiSrc, const std::string& command)
{
    docBroker.assertCorrectThread();

    if (!RemoteDocumentBroker::isEnabled() || !RemoteDocumentBroker::isInitialized())
        return;

    std::string remoteDocKey;
    try
    {
        remoteDocKey = RequestDetails::getDocKey(wopiSrc);
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Invalid remote document WOPISrc for a command: " << exc.what());
        return;
    }

    // A command is only routed to a remote the document is subscribed to.
    for (const auto& it : _subscriptions)
    {
        if (RequestDetails::getDocKey(std::get<0>(it)) == remoteDocKey)
        {
            RemoteDocumentBroker::instance().sendCommandAsync(std::get<0>(it), std::get<1>(it),
                                                              docBroker.getDocKey(), sessionId,
                                                              command);
            return;
        }
    }

    LOG_DBG("Ignoring a remote document command for [" << remoteDocKey
                                                       << "]: no live subscription");
}

void RelatedDocuments::addToIncomingDocKeyChain(DocumentBroker& docBroker,
                                                const std::string& docKeyChain)
{
    docBroker.assertCorrectThread();

    const StringVector docKeys = StringVector::tokenize(docKeyChain, ',');
    for (std::size_t i = 0; i < docKeys.size(); ++i)
    {
        std::string docKey = docKeys[i];
        if (!docKey.empty() &&
            std::find(_incomingDocKeyChain.begin(), _incomingDocKeyChain.end(), docKey) ==
                _incomingDocKeyChain.end())
        {
            LOG_DBG("The docKey [" << docKey << "] is on the connection chain of ["
                                   << docBroker.getDocKey() << ']');

            // The chain proves this docKey (transitively) subscribes to this
            // document. A live subscription of ours back to it closes a loop
            // that would keep both documents loaded forever. When documents
            // subscribed to each other at the same moment, each end sees the
            // other's chain; the greater docKey yields so one link survives.
            if (docBroker.getDocKey() > docKey && RemoteDocumentBroker::isInitialized())
            {
                for (auto it = _subscriptions.begin(); it != _subscriptions.end();)
                {
                    if (RequestDetails::getDocKey(std::get<0>(*it)) == docKey)
                    {
                        LOG_WRN("Dropping the remote document link tag="
                                << std::get<2>(*it) << " of [" << docBroker.getDocKey() << "] to ["
                                << docKey << "]: it subscribes back to this document");
                        RemoteDocumentBroker::instance().unsubscribeAsync(
                            std::get<0>(*it), std::get<1>(*it), docBroker.getDocKey(),
                            std::get<2>(*it));
                        sendError(docBroker, std::get<2>(*it), Uri::encode(std::get<0>(*it)),
                                  "cycledetected");
                        it = _subscriptions.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            _incomingDocKeyChain.push_back(std::move(docKey));
        }
    }
}

void RelatedDocuments::onEvent(DocumentBroker& docBroker, const std::string& encodedWopiSrc,
                               const std::string& eventArguments)
{
    const StringVector arguments = StringVector::tokenize(eventArguments);
    std::string event;
    if (arguments.size())
        COOLProtocol::getTokenString(arguments[0], "event", event);

    // Content events carry no subscription-state change.
    if (event == "modified" || event == "invalidated" || event == "structure" || event.empty())
        return;

    try
    {
        const std::string docKey = RequestDetails::getDocKey(Uri::decode(encodedWopiSrc));
        const auto it = _entries.find(docKey);
        if (it != _entries.end())
        {
            if (event == "connected" || event == "disconnected" || event == "failed")
                it->second.lastState = event;
            refresh(docBroker);
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Cannot mirror the remote document event: " << exc.what());
    }
}

void RelatedDocuments::setSubscribed(DocumentBroker& docBroker, const std::string& docKey)
{
    const auto it = _entries.find(docKey);
    if (it != _entries.end())
    {
        it->second.lastState = "subscribed";
        refresh(docBroker);
    }
}

void RelatedDocuments::refresh(DocumentBroker& docBroker)
{
    const std::string message = "relateddocuments: " + buildJson();
    if (message == _lastClientMessage)
        return;

    _lastClientMessage = message;
    docBroker.broadcastMessage(message);
}

void RelatedDocuments::sendTo(const std::shared_ptr<ClientSession>& session) const
{
    session->sendTextFrame("relateddocuments: " + buildJson());
}

bool RelatedDocuments::hasSubscription(const std::string& remoteDocKey) const
{
    for (const auto& it : _subscriptions)
    {
        if (RequestDetails::getDocKey(std::get<0>(it)) == remoteDocKey)
            return true;
    }

    return false;
}

std::string RelatedDocuments::buildJson() const
{
    Poco::JSON::Array::Ptr documents = new Poco::JSON::Array();
    for (const auto& it : _entries)
    {
        Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
        entry->set("wopiSrc", it.second.wopiSrc);
        entry->set("state", hasSubscription(it.first) ? it.second.lastState
                                                      : std::string("available"));
        documents->add(entry);
    }

    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("documents", documents);

    std::ostringstream oss;
    root->stringify(oss);
    return oss.str();
}

void RelatedDocuments::dumpState(std::ostream& os) const
{
    os << "\n  remote subscriptions: " << _subscriptions.size();
    for (const auto& it : _subscriptions)
        os << "\n    " << Anonymizer::anonymizeUrl(std::get<0>(it))
           << " tag: " << std::get<2>(it);
    os << "\n  related documents: " << _entries.size();
    for (const auto& it : _entries)
        os << "\n    " << it.first << " state: "
           << (hasSubscription(it.first) ? it.second.lastState : std::string("available"));
    os << "\n  incoming docKey chain: " << _incomingDocKeyChain.size();
    for (const std::string& docKey : _incomingDocKeyChain)
        os << "\n    " << docKey;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
