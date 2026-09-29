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

#include "RemoteLinks.hpp"

#include <common/Anonymizer.hpp>
#include <common/ConfigUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/SigUtil.hpp>
#include <common/StringVector.hpp>
#include <common/Uri.hpp>
#include <net/HttpRequest.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/HostUtil.hpp>
#include <wsd/RemoteDocumentBroker.hpp>
#include <wsd/RequestDetails.hpp>
#include <wsd/wopi/StorageConnectionManager.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/URI.h>

#include <algorithm>
#include <sstream>

void RemoteLinks::setSource(DocumentBroker& docBroker, const std::string& wopiSrc,
                            const std::string& name, const std::string& lastModifiedTime,
                            const std::string& persistentLink)
{
    docBroker.assertCorrectThread();

    // Key by the docKey, so encoded and decoded spellings of the same
    // WOPISrc resolve to one source.
    try
    {
        const std::string docKey = RequestDetails::getDocKey(wopiSrc);

        // A view reaches a remote link by its persistent link alone, so a document that is
        // not recorded yet is recorded with one.
        if (persistentLink.empty() && _entries.find(docKey) == _entries.end())
        {
            LOG_WRN("Ignoring the remote link [" << Anonymizer::anonymizeUrl(wopiSrc) << "] of ["
                                                 << docBroker.getDocKey()
                                                 << "]: it names no persistent link");
            return;
        }

        Entry& entry = _entries[docKey];
        entry.wopiSrc = wopiSrc.substr(0, wopiSrc.find('?'));
        entry.lastModifiedTime = lastModifiedTime;
        // A report that names no document leaves the name it is known by as it is, so a later
        // report of the time alone keeps the name the integrator gave it.
        if (!name.empty())
            entry.name = name;

        // The pages that store a persistent link come from one document, so the link moves to
        // the remote link reported for it and leaves any other.
        if (!persistentLink.empty())
        {
            for (auto& it : _entries)
            {
                if (it.second.persistentLink == persistentLink)
                    it.second.persistentLink.clear();
            }
            entry.persistentLink = persistentLink;
        }

        refreshAllViews(docBroker);
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Ignoring the invalid remote link WOPISrc ["
                << Anonymizer::anonymizeUrl(wopiSrc) << "]: " << exc.what());
    }
}

bool RemoteLinks::removeSource(DocumentBroker& docBroker, const std::string& persistentLink)
{
    docBroker.assertCorrectThread();

    const std::string linkAnonym = Anonymizer::anonymize(persistentLink);
    const auto itEntry = persistentLink.empty() ? _entries.end() : findListed(persistentLink);
    if (itEntry == _entries.end())
    {
        LOG_INF("No remote link of [" << docBroker.getDocKey() << "] is bound to ["
                                      << linkAnonym << ']');
        return false;
    }

    const std::string docKey = itEntry->first;
    const std::string wopiSrc = itEntry->second.wopiSrc;
    _entries.erase(itEntry);

    // The document is gone for every view, so the live links to it go down and the tokens that
    // opened them go with it.
    for (auto& itView : _views)
    {
        itView.second.linkAccess.erase(persistentLink);
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

    LOG_INF("Dropped the remote link [" << Anonymizer::anonymizeUrl(wopiSrc) << "] bound to ["
                                        << linkAnonym << "] of [" << docBroker.getDocKey()
                                        << ']');
    refreshAllViews(docBroker);
    return true;
}

void RemoteLinks::setNamedSources(DocumentBroker& docBroker, std::vector<std::string> names)
{
    docBroker.assertCorrectThread();

    if (!RemoteDocumentBroker::isEnabled())
        return;

    if (names == _namedSources)
        return;

    _namedSources = std::move(names);

    for (auto& itView : _views)
    {
        std::map<std::string, LinkAccess>& linkAccess = itView.second.linkAccess;
        for (auto it = linkAccess.begin(); it != linkAccess.end();)
        {
            if (std::find(_namedSources.begin(), _namedSources.end(), it->first) ==
                _namedSources.end())
                it = linkAccess.erase(it);
            else
                ++it;
        }
    }

    resolveUnlistedSources(docBroker);
    refreshAllViews(docBroker);
}

void RemoteLinks::setViewToken(DocumentBroker& docBroker, const std::string& tag,
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
        LOG_ERR("Ignoring the access token for the invalid remote link WOPISrc ["
                << Anonymizer::anonymizeUrl(wopiSrc) << "]: " << exc.what());
    }
}

void RemoteLinks::enableViewLinkAccess(DocumentBroker& docBroker, const std::string& tag)
{
    docBroker.assertCorrectThread();

    if (!RemoteDocumentBroker::isEnabled())
        return;

    _views[tag].supportsLinkAccess = true;
    resolveUnlistedSources(docBroker, tag);
}

void RemoteLinks::resolveUnlistedSources(DocumentBroker& docBroker)
{
    std::vector<std::string> tags;
    tags.reserve(_views.size());
    for (const auto& itView : _views)
        tags.push_back(itView.first);

    for (const std::string& tag : tags)
        resolveUnlistedSources(docBroker, tag);
}

void RemoteLinks::resolveUnlistedSources(DocumentBroker& docBroker, const std::string& tag)
{
    const auto itView = _views.find(tag);
    if (itView == _views.end() || !itView->second.supportsLinkAccess)
        return;

    // The sources the view finds on the document are asked for once.
    if (itView->second.askedOnLoad || _namedSources.empty())
        return;

    itView->second.askedOnLoad = true;

    for (const std::string& name : _namedSources)
    {
        // A source this view can already read needs no question
        const auto itEntry = findListed(name);
        if (itEntry != _entries.end() &&
            itView->second.tokens.find(itEntry->first) != itView->second.tokens.end())
            continue;
        if (itView->second.linkAccess.find(name) != itView->second.linkAccess.end())
            continue;

        requestLinkAccess(docBroker, tag, name);
    }

    refreshView(docBroker, tag);
}

void RemoteLinks::resolveSource(DocumentBroker& docBroker, const std::string& tag,
                                const std::string& persistentLink)
{
    docBroker.assertCorrectThread();

    const auto itView = _views.find(tag);
    if (itView == _views.end() || !itView->second.supportsLinkAccess)
        return;

    // Only a source the document names is asked for, and only once at a time: the answer to
    // the question in flight is the answer to this one.
    if (std::find(_namedSources.begin(), _namedSources.end(), persistentLink) ==
        _namedSources.end())
    {
        LOG_WRN("Not asking view [" << tag << "] for a source of [" << docBroker.getDocKey()
                                    << "] that the document does not name");
        return;
    }

    const auto itAccess = itView->second.linkAccess.find(persistentLink);
    if (itAccess != itView->second.linkAccess.end() && itAccess->second == LinkAccess::Pending)
        return;

    requestLinkAccess(docBroker, tag, persistentLink);
    refreshView(docBroker, tag);
}

void RemoteLinks::requestLinkAccess(DocumentBroker& docBroker, const std::string& tag,
                                    const std::string& persistentLink)
{
    View& view = _views[tag];
    const std::shared_ptr<ClientSession> session = docBroker.findSession(tag);
    if (!session)
        return;

    // The question goes to the storage of this document.
    // Is authorized the way every other WOPI request of this view is.
    const Authorization& auth = session->getAuthorization();
    Poco::URI uri(session->getPublicUri());
    uri.setPath(uri.getPath() + "/linkaccess");
    auth.authorizeURI(uri);

    const std::string uriAnonym = Anonymizer::anonymizeUrl(uri.toString());
    const std::string linkAnonym = Anonymizer::anonymize(persistentLink);

    // A relative URI has no host to send the request to.
    if (uri.isRelative())
    {
        LOG_WRN("Not asking for the source [" << linkAnonym << "] of [" << docBroker.getDocKey()
                                              << "]: the WOPI URL [" << uriAnonym
                                              << "] is relative");
        view.linkAccess[persistentLink] = LinkAccess::Failed;
        return;
    }

    Poco::JSON::Object::Ptr body = new Poco::JSON::Object();
    body->set("PersistentLink", persistentLink);
    std::ostringstream bodyStream;
    body->stringify(bodyStream);

    http::Request request = StorageConnectionManager::createHttpRequest(uri, auth);
    request.setVerb(http::Request::VERB_POST);
    request.setBody(bodyStream.str(), "application/json; charset=utf-8");

    std::shared_ptr<http::Session> httpSession = StorageConnectionManager::getHttpSession(uri);
    if (!httpSession)
    {
        LOG_WRN("Not asking for the source [" << linkAnonym << "] of [" << docBroker.getDocKey()
                                              << "]: no HTTP session to [" << uriAnonym << ']');
        view.linkAccess[persistentLink] = LinkAccess::Failed;
        return;
    }

    std::weak_ptr<DocumentBroker> weakBroker = docBroker.weak_from_this();
    const auto complete = [weakBroker, tag, persistentLink](unsigned statusCode,
                                                             const std::string& answer)
    {
        if (SigUtil::getShutdownRequestFlag())
            return;

        const std::shared_ptr<DocumentBroker> broker = weakBroker.lock();
        if (!broker || broker->isMarkedToDestroy())
            return;

        broker->completeRemoteDocumentLinkAccess(tag, persistentLink, statusCode, answer);
    };
    httpSession->setFinishedHandler(
        [complete](const std::shared_ptr<http::Session>& finishedSession)
        {
            const std::shared_ptr<const http::Response> response = finishedSession->response();
            complete(response ? static_cast<unsigned>(response->statusLine().statusCode()) : 0,
                     response ? response->getBody() : std::string());
        });
    httpSession->setConnectFailHandler([complete](const std::shared_ptr<http::Session>&)
                                       { complete(0, std::string()); });

    view.linkAccess[persistentLink] = LinkAccess::Pending;
    LOG_INF("Asking [" << uriAnonym << "] for the source [" << linkAnonym << "] of ["
                       << docBroker.getDocKey() << "] for view [" << tag << ']');
    httpSession->asyncRequest(request, docBroker.getPoll());
}

void RemoteLinks::completeLinkAccess(DocumentBroker& docBroker, const std::string& tag,
                                     const std::string& persistentLink, const unsigned statusCode,
                                     const std::string& body)
{
    docBroker.assertCorrectThread();

    const auto itView = _views.find(tag);
    if (itView == _views.end())
        return;

    const auto itAccess = itView->second.linkAccess.find(persistentLink);
    if (itAccess == itView->second.linkAccess.end() || itAccess->second != LinkAccess::Pending)
        return;

    const std::string linkAnonym = Anonymizer::anonymize(persistentLink);

    if (statusCode == static_cast<unsigned>(http::StatusCode::NotFound))
    {
        LOG_INF("The storage knows no document for the source ["
                                << linkAnonym << "] of [" << docBroker.getDocKey() << ']');
        itAccess->second = LinkAccess::NotFound;
        refreshView(docBroker, tag);
        return;
    }

    if (statusCode == static_cast<unsigned>(http::StatusCode::Unauthorized) ||
        statusCode == static_cast<unsigned>(http::StatusCode::Forbidden))
    {
        LOG_INF("The storage denies view [" << tag << "] the source ["
                                << linkAnonym << "] of [" << docBroker.getDocKey() << ']');
        itAccess->second = LinkAccess::Denied;
        refreshView(docBroker, tag);
        return;
    }

    if (statusCode != static_cast<unsigned>(http::StatusCode::OK))
    {
        LOG_WRN("The storage answered the question for the source ["
                                << linkAnonym << "] of [" << docBroker.getDocKey()
                                << "] with status " << statusCode);
        itAccess->second = LinkAccess::Failed;
        refreshView(docBroker, tag);
        return;
    }

    constexpr std::size_t MaxAnswerSize = 64 * 1024;
    Poco::JSON::Object::Ptr object;
    if (body.size() > MaxAnswerSize || !JsonUtil::parseJSON(body, object))
    {
        LOG_ERR("The storage answered the question for the source ["
                                << linkAnonym << "] of [" << docBroker.getDocKey()
                                << "] with a body that is not a link");
        itAccess->second = LinkAccess::Failed;
        refreshView(docBroker, tag);
        return;
    }

    std::string wopiSrc;
    std::string accessToken;
    std::string name;
    std::string lastModifiedTime;
    std::string answeredPersistentLink;
    JsonUtil::findJSONValue(object, "WOPISrc", wopiSrc);
    JsonUtil::findJSONValue(object, "AccessToken", accessToken);
    JsonUtil::findJSONValue(object, "BaseFileName", name);
    JsonUtil::findJSONValue(object, "LastModifiedTime", lastModifiedTime);
    JsonUtil::findJSONValue(object, "PersistentLink", answeredPersistentLink);

    std::string reason;
    try
    {
        if (wopiSrc.empty() || accessToken.empty())
            reason = "names no document or no token";
        else if (!answeredPersistentLink.empty() && answeredPersistentLink != persistentLink)
            reason = "answers for another persistent link";
        else if (RequestDetails::getDocKey(wopiSrc) == docBroker.getDocKey())
            reason = "names this document itself";
        else if (!HostUtil::allowedWopiHost(Poco::URI(wopiSrc).getHost()))
            reason = "names a document on a host that is not allowed";
    }
    catch (const std::exception& exc)
    {
        reason = std::string("names an invalid document: ") + exc.what();
    }

    if (!reason.empty())
    {
        LOG_ERR("The storage answered the question for the source ["
                                << linkAnonym << "] of [" << docBroker.getDocKey()
                                << "] with a link that " << reason);
        itAccess->second = LinkAccess::Failed;
        refreshView(docBroker, tag);
        return;
    }

    itAccess->second = LinkAccess::Resolved;
    LOG_INF("The storage named [" << Anonymizer::anonymizeUrl(wopiSrc)
                            << "] for the source [" << linkAnonym << "] of ["
                            << docBroker.getDocKey() << "] for view [" << tag << ']');

    // The document stands for the source for every view, and the token is this view's own.
    setSource(docBroker, wopiSrc, name, lastModifiedTime, persistentLink);
    setViewToken(docBroker, tag, wopiSrc, accessToken);
}

void RemoteLinks::handleSubscribe(DocumentBroker& docBroker, const std::string& tag,
                                  const std::string& persistentLink, const bool subscribe)
{
    docBroker.assertCorrectThread();

    const std::string linkAnonym = Anonymizer::anonymize(persistentLink);

    LOG_INF("Remote document " << (subscribe ? "subscribe" : "unsubscribe") << " by view ["
                               << tag << "] to [" << linkAnonym << ']');

    if (!RemoteDocumentBroker::isEnabled() || !RemoteDocumentBroker::isInitialized())
    {
        LOG_ERR("Remote document subscribe by view [" << tag
                                                      << "] rejected: remote_links is disabled");
        sendError(docBroker, tag, persistentLink, "disabled");
        return;
    }

    // A view names the remote document by its persistent link, and the address it is reached
    // at is the one the remote link records.
    const auto itEntry = findListed(persistentLink);
    if (itEntry == _entries.end())
    {
        LOG_WRN("Remote document subscribe by view [" << tag << "] rejected: no remote link of ["
                                                      << docBroker.getDocKey() << "] is bound to ["
                                                      << linkAnonym << ']');
        sendError(docBroker, tag, persistentLink, "notfound");
        return;
    }

    const std::string remoteDocKey = itEntry->first;
    const std::string wopiSrc = itEntry->second.wopiSrc;

    if (subscribe)
    {
        try
        {
            const Poco::URI wopiSrcUri(wopiSrc);
            if (!HostUtil::allowedWopiHost(wopiSrcUri.getHost()))
            {
                LOG_WRN("Remote document [" << Anonymizer::anonymizeUrl(wopiSrc)
                                            << "] is not on an allowed WOPI host");
                sendError(docBroker, tag, persistentLink, "hostnotallowed");
                return;
            }
        }
        catch (const std::exception& exc)
        {
            LOG_ERR("Invalid remote document WOPISrc: " << exc.what());
            sendError(docBroker, tag, persistentLink, "syntax");
            return;
        }
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
        sendError(docBroker, tag, persistentLink, "notoken");
        return;
    }

    const std::string serverUrl = RemoteDocumentBroker::getServerUrl();
    if (serverUrl.empty())
    {
        LOG_ERR("Remote document subscribe by view ["
                << tag
                << "] rejected: no server URL to dial through; set remote_links.server_url "
                   "or server_name in the configuration");
        sendError(docBroker, tag, persistentLink, "noserver");
        return;
    }

    const auto isLive = [](const Subscription& subscription)
    { return subscription.state != "failed" && subscription.state != "missing"; };

    // A view already subscribed to the source has nothing to do.
    const auto itSubscription = view.subscriptions.find(remoteDocKey);
    if (itSubscription != view.subscriptions.end() && isLive(itSubscription->second))
        return;

    const size_t maxLinks =
        ConfigUtil::getConfigValue<int>("remote_links.max_links_per_document", 4);
    const size_t liveLinks =
        std::count_if(view.subscriptions.begin(), view.subscriptions.end(),
                      [&isLive](const auto& it) { return isLive(it.second); });
    if (liveLinks >= maxLinks)
    {
        LOG_ERR("Remote document subscribe by view ["
                << tag << "] rejected: the view already holds " << liveLinks
                << " subscriptions of the maximum " << maxLinks);
        sendError(docBroker, tag, persistentLink, "limitreached");
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

void RemoteLinks::onRemoteEvent(DocumentBroker& docBroker, const std::string& tag,
                                const std::string& encodedWopiSrc,
                                const std::string& eventArguments)
{
    docBroker.assertCorrectThread();

    // The broker names the remote document by its WOPISrc, and a view knows it by the
    // persistent link of the remote link recorded at that address.
    std::string docKey;
    try
    {
        docKey = RequestDetails::getDocKey(Uri::decode(encodedWopiSrc));
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Cannot mirror the remote document event: " << exc.what());
        return;
    }

    const auto itEntry = _entries.find(docKey);
    if (itEntry == _entries.end())
    {
        LOG_DBG("Ignoring an event of [" << docKey << "]: no remote link of ["
                                         << docBroker.getDocKey() << "] is recorded there");
        return;
    }

    const std::string persistentLink = itEntry->second.persistentLink;
    const std::string wopiSrc = itEntry->second.wopiSrc;

    const StringVector arguments = StringVector::tokenize(eventArguments);
    std::string event;
    if (arguments.size())
        COOLProtocol::getTokenString(arguments[0], "event", event);

    // Every event reaches the kit, which will drive the shared freshness of
    // linked slides once the engine reads these events.
    docBroker.sendTextFrameToKit("remotedocevent tag=" + tag + " source=" +
                                 Uri::encode(persistentLink) + ' ' + eventArguments);

    // A save on the source uploaded a new file to storage. The new last-modified
    // time is the same for every view, so refresh the shared source record.
    if (event == "saved")
    {
        std::string time;
        if (arguments.size() > 1)
            COOLProtocol::getTokenString(arguments[1], "time", time);
        setSource(docBroker, wopiSrc, std::string(), Uri::decode(time), std::string());
        return;
    }

    // Content events carry no per-view connection change.
    if (event == "modified" || event == "invalidated" || event == "structure" || event.empty())
        return;

    // Connection events belong to the one view that opened the subscription.
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
        itSub->second.state = std::move(event);

    refreshView(docBroker, tag);
}

void RemoteLinks::sendCommand(DocumentBroker& docBroker, const std::string& tag,
                              const std::string& persistentLink, const std::string& command)
{
    docBroker.assertCorrectThread();

    if (!RemoteDocumentBroker::isEnabled() || !RemoteDocumentBroker::isInitialized())
        return;

    const std::string linkAnonym = Anonymizer::anonymize(persistentLink);
    const auto itEntry = findListed(persistentLink);
    if (itEntry == _entries.end())
    {
        LOG_DBG("Ignoring a remote document command from view ["
                << tag << "] for [" << linkAnonym << "]: no remote link of ["
                << docBroker.getDocKey() << "] is bound to it");
        return;
    }

    // A command only reaches a remote the view itself is subscribed to.
    const auto itView = _views.find(tag);
    if (itView != _views.end())
    {
        const auto itSub = itView->second.subscriptions.find(itEntry->first);
        if (itSub != itView->second.subscriptions.end())
        {
            RemoteDocumentBroker::instance().sendCommandAsync(itSub->second.wopiSrc,
                                                              itSub->second.accessToken,
                                                              docBroker.getDocKey(), tag,
                                                              command);
            return;
        }
    }

    LOG_DBG("Ignoring a remote document command from view [" << tag << "] for [" << linkAnonym
                                                             << "]: no live subscription");
}

void RemoteLinks::removeSubscription(DocumentBroker& docBroker, const std::string& tag,
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

void RemoteLinks::removeView(DocumentBroker& docBroker, const std::string& tag)
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

void RemoteLinks::unsubscribeAll(DocumentBroker& docBroker)
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

void RemoteLinks::addToIncomingDocKeyChain(DocumentBroker& docBroker,
                                                const std::string& docKeyChain)
{
    docBroker.assertCorrectThread();

    if (!RemoteDocumentBroker::isEnabled())
        return;

    // A subscription reaches through at most this many documents, so a chain
    // holds that many docKeys and the record keeps that many.
    static const std::size_t maxChainDepth =
        std::max(0, ConfigUtil::getConfigValue<int>("remote_links.max_chain_depth", 3));

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
                const auto itEntry = _entries.find(docKey);
                sendError(docBroker, itView.first,
                          itEntry != _entries.end() ? itEntry->second.persistentLink
                                                    : std::string(),
                          "cycledetected");
                itView.second.subscriptions.erase(itSub);
                refreshView(docBroker, itView.first);
            }
        }

        _incomingDocKeyChain.push_back(std::move(docKey));
    }
}

void RemoteLinks::sendError(DocumentBroker& docBroker, const std::string& tag,
                            const std::string& persistentLink, const std::string& kind)
{
    docBroker.sendTextFrameToKit("remotedocevent tag=" + tag + " source=" +
                                 Uri::encode(persistentLink) + " event=error kind=" + kind);
}

std::string RemoteLinks::persistentLinkOf(const std::string& wopiSrc) const
{
    try
    {
        const auto itEntry = _entries.find(RequestDetails::getDocKey(wopiSrc));
        return itEntry != _entries.end() ? itEntry->second.persistentLink : std::string();
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("No remote link at the invalid WOPISrc [" << Anonymizer::anonymizeUrl(wopiSrc)
                                                          << "]: " << exc.what());
        return std::string();
    }
}

std::string RemoteLinks::documentName(const std::string& wopiSrc)
{
    const std::string path = wopiSrc.substr(0, wopiSrc.find('?'));
    const std::size_t lastSlash = path.rfind('/');
    const std::string name = lastSlash == std::string::npos ? path : path.substr(lastSlash + 1);
    return Uri::decode(name);
}

std::string RemoteLinks::entryName(const Entry& entry)
{
    return entry.name.empty() ? documentName(entry.wopiSrc) : entry.name;
}

std::map<std::string, RemoteLinks::Entry>::const_iterator
RemoteLinks::findListed(const std::string& persistentLink) const
{
    return std::find_if(_entries.begin(), _entries.end(),
                        [&persistentLink](const auto& it)
                        { return it.second.persistentLink == persistentLink; });
}

const char* RemoteLinks::linkAccessName(const LinkAccess access)
{
    switch (access)
    {
        case LinkAccess::Pending:
            return "pending";
        case LinkAccess::Resolved:
            return "resolved";
        case LinkAccess::NotFound:
            return "notfound";
        case LinkAccess::Denied:
            return "denied";
        case LinkAccess::Failed:
            return "failed";
    }
    return "unknown";
}

std::string RemoteLinks::buildJson(const std::string& tag) const
{
    const auto itView = _views.find(tag);
    const View* view = itView != _views.end() ? &itView->second : nullptr;

    // What asking this view's storage for a source came to, empty for one it never asked for.
    const auto access = [&view](const std::string& persistentLink) -> std::string
    {
        if (!view || persistentLink.empty())
            return std::string();

        const auto it = view->linkAccess.find(persistentLink);
        return it == view->linkAccess.end() ? std::string() : linkAccessName(it->second);
    };

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
        entry->set("name", entryName(it.second));
        entry->set("state", state);
        entry->set("lastModifiedTime", it.second.lastModifiedTime);
        entry->set("persistentLink", it.second.persistentLink);
        entry->set("access", access(it.second.persistentLink));
        documents->add(entry);
    }

    // A source the document names that no remote link stands for is reported under its name
    // alone: it says what this document was made from, and no remote link is bound to it that
    // a view could read.
    for (const std::string& name : _namedSources)
    {
        if (isListed(name))
            continue;

        Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
        entry->set("name", name);
        entry->set("state", "missing");
        entry->set("lastModifiedTime", std::string());
        entry->set("persistentLink", name);
        entry->set("access", access(name));
        documents->add(entry);
    }

    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("documents", documents);

    std::ostringstream oss;
    root->stringify(oss);
    return oss.str();
}

void RemoteLinks::sendTo(const std::shared_ptr<ClientSession>& session)
{
    const std::string tag = session->getId();
    const std::string message = "remotelinks: " + buildJson(tag);
    _views[tag].lastClientMessage = message;
    session->sendTextFrame(message);
}

void RemoteLinks::refreshView(DocumentBroker& docBroker, const std::string& tag)
{
    const std::shared_ptr<ClientSession> session = docBroker.findSession(tag);
    if (!session)
        return;

    const std::string message = "remotelinks: " + buildJson(tag);
    View& view = _views[tag];
    if (message == view.lastClientMessage)
        return;

    view.lastClientMessage = message;
    session->sendTextFrame(message);
}

void RemoteLinks::refreshAllViews(DocumentBroker& docBroker)
{
    for (const std::string& tag : docBroker.getSessionIds())
        refreshView(docBroker, tag);
}

void RemoteLinks::dumpState(std::ostream& os) const
{
    os << "\n  remote link sources: " << _entries.size();
    for (const auto& it : _entries)
        os << "\n    " << it.first << " name: " << Anonymizer::anonymize(it.second.name)
           << " last modified: " << it.second.lastModifiedTime
           << " persistent link: " << Anonymizer::anonymize(it.second.persistentLink);
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
           << " subscriptions: " << itView.second.subscriptions.size()
           << " link access: " << (itView.second.supportsLinkAccess ? "yes" : "no")
           << " asked on load: " << (itView.second.askedOnLoad ? "yes" : "no")
           << " link access asked: " << itView.second.linkAccess.size();
        for (const auto& it : itView.second.subscriptions)
            os << "\n      " << Anonymizer::anonymizeUrl(it.second.wopiSrc) << " state: "
               << it.second.state;
        for (const auto& it : itView.second.linkAccess)
            os << "\n      persistent link " << Anonymizer::anonymize(it.first) << " access: "
               << linkAccessName(it.second);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
