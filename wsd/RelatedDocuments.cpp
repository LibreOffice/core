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

void RelatedDocuments::setSource(DocumentBroker& docBroker, const std::string& wopiSrc,
                                 const std::string& name, const std::string& lastModifiedTime)
{
    docBroker.assertCorrectThread();

    // Key by the docKey, so encoded and decoded spellings of the same
    // WOPISrc resolve to one source.
    try
    {
        const std::string docKey = RequestDetails::getDocKey(wopiSrc);
        _entries[docKey].wopiSrc = wopiSrc.substr(0, wopiSrc.find('?'));
        _entries[docKey].lastModifiedTime = lastModifiedTime;
        // A report that names no document leaves the name it is known by as it is, so a later
        // report of the time alone keeps the name the integrator gave it.
        if (!name.empty())
            _entries[docKey].name = name;
        refreshAllViews(docBroker);
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Ignoring the invalid related document WOPISrc ["
                << Anonymizer::anonymizeUrl(wopiSrc) << "]: " << exc.what());
    }
}

bool RelatedDocuments::removeSource(DocumentBroker& docBroker, const std::string& wopiSrc)
{
    docBroker.assertCorrectThread();

    std::string docKey;
    try
    {
        docKey = RequestDetails::getDocKey(wopiSrc);
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Cannot drop the invalid related document WOPISrc ["
                << Anonymizer::anonymizeUrl(wopiSrc) << "]: " << exc.what());
        return false;
    }

    if (_entries.erase(docKey) == 0)
    {
        LOG_INF("No related document of [" << docBroker.getDocKey() << "] at ["
                                           << Anonymizer::anonymizeUrl(wopiSrc) << ']');
        return false;
    }

    // The document is gone for every view, so the live links to it go down and the tokens that
    // opened them go with it.
    for (auto& itView : _views)
    {
        const auto itSubscription = itView.second.subscriptions.find(docKey);
        if (itSubscription != itView.second.subscriptions.end())
        {
            if (RemoteDocumentBroker::isInitialized())
                RemoteDocumentBroker::instance().unsubscribeAsync(
                    itSubscription->second.wopiSrc, itSubscription->second.accessToken,
                    docBroker.getDocKey(), itView.first);
            itView.second.subscriptions.erase(itSubscription);
        }
        itView.second.tokens.erase(docKey);
    }

    LOG_INF("Dropped the related document [" << Anonymizer::anonymizeUrl(wopiSrc) << "] of ["
                                             << docBroker.getDocKey() << ']');
    refreshAllViews(docBroker);
    return true;
}

void RelatedDocuments::setNamedSources(DocumentBroker& docBroker, std::vector<std::string> names)
{
    docBroker.assertCorrectThread();

    // A source nothing can reach is reported where a document reads related documents at all, and
    // left alone on a server that serves none.
    if (!RemoteDocumentBroker::isEnabled())
        return;

    if (names == _namedSources)
        return;

    _namedSources = std::move(names);
    refreshAllViews(docBroker);
}

void RelatedDocuments::setViewToken(DocumentBroker& docBroker, const std::string& tag,
                                    const std::string& wopiSrc, const std::string& accessToken)
{
    docBroker.assertCorrectThread();

    try
    {
        const std::string docKey = RequestDetails::getDocKey(wopiSrc);
        _views[tag].tokens[docKey] = accessToken;
        refreshView(docBroker, tag);
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Ignoring the access token for the invalid related document WOPISrc ["
                << Anonymizer::anonymizeUrl(wopiSrc) << "]: " << exc.what());
    }
}

void RelatedDocuments::handleSubscribe(DocumentBroker& docBroker, const std::string& tag,
                                       const std::string& encodedWopiSrc, const bool subscribe)
{
    docBroker.assertCorrectThread();

    const std::string wopiSrc = Uri::decode(encodedWopiSrc);

    LOG_INF("Remote document " << (subscribe ? "subscribe" : "unsubscribe") << " by view ["
                               << tag << "] to [" << Anonymizer::anonymizeUrl(wopiSrc)
                               << ']');

    if (!RemoteDocumentBroker::isEnabled() || !RemoteDocumentBroker::isInitialized())
    {
        LOG_ERR("Remote document subscribe by view [" << tag
                                                      << "] rejected: remote_documents is disabled");
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

    View& view = _views[tag];

    if (!subscribe)
    {
        const auto it = view.subscriptions.find(remoteDocKey);
        if (it != view.subscriptions.end())
        {
            RemoteDocumentBroker::instance().unsubscribeAsync(
                it->second.wopiSrc, it->second.accessToken, docBroker.getDocKey(), tag);
            view.subscriptions.erase(it);
            refreshView(docBroker, tag);
        }

        return;
    }

    const auto itToken = view.tokens.find(remoteDocKey);
    if (itToken == view.tokens.end())
    {
        LOG_ERR("Remote document subscribe by view ["
                << tag << "] rejected: the view holds no access token for [" << remoteDocKey
                << ']');
        sendError(docBroker, tag, encodedWopiSrc, "notoken");
        return;
    }

    const std::string serverUrl = RemoteDocumentBroker::getServerUrl();
    if (serverUrl.empty())
    {
        LOG_ERR("Remote document subscribe by view ["
                << tag
                << "] rejected: no server URL to dial through; set remote_documents.server_url "
                   "or server_name in the configuration");
        sendError(docBroker, tag, encodedWopiSrc, "noserver");
        return;
    }

    // A view already subscribed to the source has nothing to do.
    if (view.subscriptions.find(remoteDocKey) != view.subscriptions.end())
        return;

    static const size_t maxLinks =
        ConfigUtil::getConfigValue<int>("remote_documents.max_links_per_document", 4);
    if (view.subscriptions.size() >= maxLinks)
    {
        LOG_ERR("Remote document subscribe by view ["
                << tag << "] rejected: the view already holds " << view.subscriptions.size()
                << " subscriptions of the maximum " << maxLinks);
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

    view.subscriptions[remoteDocKey] = { wopiSrc, itToken->second, "subscribed" };

    RemoteDocumentBroker::instance().subscribeAsync(std::move(request));
    refreshView(docBroker, tag);
}

void RelatedDocuments::onRemoteEvent(DocumentBroker& docBroker, const std::string& tag,
                                     const std::string& encodedWopiSrc,
                                     const std::string& eventArguments)
{
    docBroker.assertCorrectThread();

    const StringVector arguments = StringVector::tokenize(eventArguments);
    std::string event;
    if (arguments.size())
        COOLProtocol::getTokenString(arguments[0], "event", event);

    // Every event reaches the kit, which will drive the shared freshness of
    // linked slides once the engine reads these events.
    docBroker.sendTextFrameToKit("remotedocevent tag=" + tag + " wopisrc=" + encodedWopiSrc +
                                 ' ' + eventArguments);

    // A save on the source uploaded a new file to storage. The new last-modified
    // time is the same for every view, so refresh the shared source record.
    if (event == "saved")
    {
        std::string time;
        if (arguments.size() > 1)
            COOLProtocol::getTokenString(arguments[1], "time", time);
        setSource(docBroker, Uri::decode(encodedWopiSrc), std::string(), Uri::decode(time));
        return;
    }

    // Content events carry no per-view connection change.
    if (event == "modified" || event == "invalidated" || event == "structure" || event.empty())
        return;

    // Connection events belong to the one view that opened the subscription.
    try
    {
        const std::string docKey = RequestDetails::getDocKey(Uri::decode(encodedWopiSrc));
        const auto itView = _views.find(tag);
        if (itView == _views.end())
            return;

        const auto itSub = itView->second.subscriptions.find(docKey);
        if (itSub == itView->second.subscriptions.end())
            return;

        if (event == "unsubscribed")
            itView->second.subscriptions.erase(itSub);
        else if (event == "connected" || event == "disconnected" || event == "failed" ||
                 event == "missing")
            itSub->second.state = event;

        refreshView(docBroker, tag);
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Cannot mirror the remote document event: " << exc.what());
    }
}

void RelatedDocuments::sendCommand(DocumentBroker& docBroker, const std::string& tag,
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

    // A command only reaches a remote the view itself is subscribed to.
    const auto itView = _views.find(tag);
    if (itView != _views.end())
    {
        const auto itSub = itView->second.subscriptions.find(remoteDocKey);
        if (itSub != itView->second.subscriptions.end())
        {
            RemoteDocumentBroker::instance().sendCommandAsync(itSub->second.wopiSrc,
                                                              itSub->second.accessToken,
                                                              docBroker.getDocKey(), tag,
                                                              command);
            return;
        }
    }

    LOG_DBG("Ignoring a remote document command from view [" << tag << "] for ["
                                                             << remoteDocKey
                                                             << "]: no live subscription");
}

void RelatedDocuments::removeSubscription(DocumentBroker& docBroker, const std::string& tag,
                                          const std::string& wopiSrc)
{
    docBroker.assertCorrectThread();

    const auto itView = _views.find(tag);
    if (itView == _views.end())
        return;

    try
    {
        const std::string docKey = RequestDetails::getDocKey(wopiSrc);
        if (itView->second.subscriptions.erase(docKey))
            refreshView(docBroker, tag);
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Cannot drop the remote document subscription: " << exc.what());
    }
}

void RelatedDocuments::removeView(DocumentBroker& docBroker, const std::string& tag)
{
    docBroker.assertCorrectThread();

    const auto itView = _views.find(tag);
    if (itView == _views.end())
        return;

    if (RemoteDocumentBroker::isInitialized())
    {
        for (const auto& it : itView->second.subscriptions)
            RemoteDocumentBroker::instance().unsubscribeAsync(
                it.second.wopiSrc, it.second.accessToken, docBroker.getDocKey(), tag);
    }

    _views.erase(itView);
}

void RelatedDocuments::unsubscribeAll(DocumentBroker& docBroker)
{
    if (RemoteDocumentBroker::isInitialized())
    {
        for (const auto& itView : _views)
        {
            for (const auto& it : itView.second.subscriptions)
                RemoteDocumentBroker::instance().unsubscribeAsync(
                    it.second.wopiSrc, it.second.accessToken, docBroker.getDocKey(), itView.first);
        }
    }

    for (auto& itView : _views)
        itView.second.subscriptions.clear();
}

void RelatedDocuments::addToIncomingDocKeyChain(DocumentBroker& docBroker,
                                                const std::string& docKeyChain)
{
    docBroker.assertCorrectThread();

    if (!RemoteDocumentBroker::isEnabled())
        return;

    // A subscription reaches through at most this many documents, so a chain
    // holds that many docKeys and the record keeps that many.
    static const std::size_t maxChainDepth =
        std::max(0, ConfigUtil::getConfigValue<int>("remote_documents.max_chain_depth", 3));

    const StringVector docKeys = StringVector::tokenize(docKeyChain, ',');
    for (std::size_t i = 0; i < docKeys.size(); ++i)
    {
        if (_incomingDocKeyChain.size() >= maxChainDepth)
        {
            LOG_WRN("The connection chain of ["
                    << docBroker.getDocKey() << "] holds " << _incomingDocKeyChain.size()
                    << " docKeys, as many as a chain reaches, so the remaining "
                    << docKeys.size() - i << " are ignored");
            break;
        }

        std::string docKey = docKeys[i];
        if (docKey.empty() ||
            std::find(_incomingDocKeyChain.begin(), _incomingDocKeyChain.end(), docKey) !=
                _incomingDocKeyChain.end())
            continue;

        LOG_DBG("The docKey [" << docKey << "] is on the connection chain of ["
                               << docBroker.getDocKey() << ']');

        // The chain proves this docKey (transitively) subscribes to this
        // document. A live subscription of ours back to it closes a loop that
        // would keep both documents loaded forever. When documents subscribed
        // to each other at the same moment, each end sees the other's chain;
        // the greater docKey yields so one link survives.
        if (docBroker.getDocKey() > docKey && RemoteDocumentBroker::isInitialized())
        {
            for (auto& itView : _views)
            {
                const auto itSub = itView.second.subscriptions.find(docKey);
                if (itSub == itView.second.subscriptions.end())
                    continue;

                LOG_WRN("Dropping the remote document link of view ["
                        << itView.first << "] in [" << docBroker.getDocKey() << "] to [" << docKey
                        << "]: it subscribes back to this document");
                RemoteDocumentBroker::instance().unsubscribeAsync(itSub->second.wopiSrc,
                                                                  itSub->second.accessToken,
                                                                  docBroker.getDocKey(),
                                                                  itView.first);
                sendError(docBroker, itView.first, Uri::encode(itSub->second.wopiSrc),
                          "cycledetected");
                itView.second.subscriptions.erase(itSub);
                refreshView(docBroker, itView.first);
            }
        }

        _incomingDocKeyChain.push_back(std::move(docKey));
    }
}

void RelatedDocuments::sendError(DocumentBroker& docBroker, const std::string& tag,
                                 const std::string& encodedWopiSrc, const std::string& kind)
{
    docBroker.sendTextFrameToKit("remotedocevent tag=" + tag + " wopisrc=" + encodedWopiSrc +
                                 " event=error kind=" + kind);
}

std::string RelatedDocuments::documentName(const std::string& wopiSrc)
{
    const std::string path = wopiSrc.substr(0, wopiSrc.find('?'));
    const std::size_t lastSlash = path.rfind('/');
    const std::string name = lastSlash == std::string::npos ? path : path.substr(lastSlash + 1);
    return Uri::decode(name);
}

std::string RelatedDocuments::entryName(const Entry& entry)
{
    return entry.name.empty() ? documentName(entry.wopiSrc) : entry.name;
}

std::string RelatedDocuments::buildJson(const std::string& tag) const
{
    const auto itView = _views.find(tag);
    const View* view = itView != _views.end() ? &itView->second : nullptr;

    Poco::JSON::Array::Ptr documents = new Poco::JSON::Array();
    for (const auto& it : _entries)
    {
        // The source and its last-modified time are the same for every view;
        // the state is this view's own: its live connection when subscribed,
        // otherwise whether it holds a token to reach the source at all.
        std::string state = "noaccess";
        if (view)
        {
            const auto itSub = view->subscriptions.find(it.first);
            if (itSub != view->subscriptions.end())
                state = itSub->second.state;
            else if (view->tokens.find(it.first) != view->tokens.end())
                state = "available";
        }

        Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
        entry->set("wopiSrc", it.second.wopiSrc);
        entry->set("name", entryName(it.second));
        entry->set("state", state);
        entry->set("lastModifiedTime", it.second.lastModifiedTime);
        documents->add(entry);
    }

    // A source the document names that the storage listed no related document for is reported
    // under its name alone: it says what this document was made from, and there is no address to
    // reach it at and no token to read it with.
    for (const std::string& name : _namedSources)
    {
        // A source is one the storage listed when either spelling of that document's name is the
        // one recorded, so slides imported before the integrator named a document stay with it.
        const bool listed = std::any_of(
            _entries.begin(), _entries.end(), [&name](const auto& it)
            { return entryName(it.second) == name || documentName(it.second.wopiSrc) == name; });
        if (listed)
            continue;

        Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
        entry->set("wopiSrc", std::string());
        entry->set("name", name);
        entry->set("state", "missing");
        entry->set("lastModifiedTime", std::string());
        documents->add(entry);
    }

    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("documents", documents);

    std::ostringstream oss;
    root->stringify(oss);
    return oss.str();
}

void RelatedDocuments::sendTo(const std::shared_ptr<ClientSession>& session)
{
    const std::string tag = session->getId();
    const std::string message = "relateddocuments: " + buildJson(tag);
    _views[tag].lastClientMessage = message;
    session->sendTextFrame(message);
}

void RelatedDocuments::refreshView(DocumentBroker& docBroker, const std::string& tag)
{
    const std::shared_ptr<ClientSession> session = docBroker.findSession(tag);
    if (!session)
        return;

    const std::string message = "relateddocuments: " + buildJson(tag);
    View& view = _views[tag];
    if (message == view.lastClientMessage)
        return;

    view.lastClientMessage = message;
    session->sendTextFrame(message);
}

void RelatedDocuments::refreshAllViews(DocumentBroker& docBroker)
{
    for (const std::string& tag : docBroker.getSessionIds())
        refreshView(docBroker, tag);
}

void RelatedDocuments::dumpState(std::ostream& os) const
{
    os << "\n  related document sources: " << _entries.size();
    for (const auto& it : _entries)
        os << "\n    " << it.first << " name: " << Anonymizer::anonymize(it.second.name)
           << " last modified: " << it.second.lastModifiedTime;
    os << "\n  sources the document names: " << _namedSources.size();
    for (const std::string& name : _namedSources)
        os << "\n    " << Anonymizer::anonymize(name);
    os << "\n  incoming docKey chain: " << _incomingDocKeyChain.size();
    for (const std::string& docKey : _incomingDocKeyChain)
        os << "\n    " << docKey;
    os << "\n  views: " << _views.size();
    for (const auto& itView : _views)
    {
        os << "\n    view " << itView.first << " tokens: " << itView.second.tokens.size()
           << " subscriptions: " << itView.second.subscriptions.size();
        for (const auto& it : itView.second.subscriptions)
            os << "\n      " << Anonymizer::anonymizeUrl(it.second.wopiSrc) << " state: "
               << it.second.state;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
