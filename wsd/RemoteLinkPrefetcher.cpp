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

#include "RemoteLinkPrefetcher.hpp"

#include <common/Anonymizer.hpp>
#include <common/ConfigUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/Uri.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/RemoteDocumentBroker.hpp>

#include <Poco/JSON/Array.h>

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string_view>

namespace
{
/// The presentation information the index of a source stands for: the document size, the
/// sections when the source has any, and one entry per slide with the keys a loaded document
/// reports.
std::string presentationInfoOf(const Poco::JSON::Object::Ptr& index)
{
    Poco::JSON::Object::Ptr info = new Poco::JSON::Object();

    const Poco::JSON::Object::Ptr file = index->getObject("file");
    info->set("docWidth", file ? file->optValue<int>("docWidth", 0) : 0);
    info->set("docHeight", file ? file->optValue<int>("docHeight", 0) : 0);

    const Poco::JSON::Array::Ptr sections = index->getArray("sections");
    if (sections && sections->size() > 0)
        info->set("sections", sections);

    Poco::JSON::Array::Ptr slides = new Poco::JSON::Array();
    if (const Poco::JSON::Array::Ptr indexSlides = index->getArray("slides"))
    {
        for (std::size_t i = 0; i < indexSlides->size(); ++i)
        {
            const Poco::JSON::Object::Ptr indexSlide = indexSlides->getObject(i);
            if (!indexSlide)
                continue;

            Poco::JSON::Object::Ptr slide = new Poco::JSON::Object();
            slide->set("hash", indexSlide->optValue<std::string>("hash", std::string()));
            slide->set("index", indexSlide->optValue<int>("index", static_cast<int>(i)));
            slide->set("part", indexSlide->optValue<std::string>("part", std::string()));
            slide->set("guid", indexSlide->optValue<std::string>("guid", std::string()));
            slide->set("name", indexSlide->optValue<std::string>("name", std::string()));
            slide->set("slideWidth", indexSlide->optValue<int>("width", 0));
            slide->set("slideHeight", indexSlide->optValue<int>("height", 0));
            if (indexSlide->optValue<bool>("hidden", false))
                slide->set("hidden", true);
            slides->add(slide);
        }
    }
    info->set("slides", slides);

    std::ostringstream oss;
    info->stringify(oss);
    return oss.str();
}

/// The sections of the index of a source, as the list a loaded document reports.
std::string slideSectionsOf(const Poco::JSON::Object::Ptr& index)
{
    const Poco::JSON::Array::Ptr sections = index->getArray("sections");
    if (!sections)
        return "[]";

    std::ostringstream oss;
    sections->stringify(oss);
    return oss.str();
}

/// The URL of the picture of the slide with the given part in the index of a source. Empty
/// when the index has no such slide.
std::string pictureUrlOf(const Poco::JSON::Object::Ptr& index, const std::string& part)
{
    const Poco::JSON::Array::Ptr slides = index->getArray("slides");
    if (!slides || part.empty())
        return std::string();

    for (std::size_t i = 0; i < slides->size(); ++i)
    {
        const Poco::JSON::Object::Ptr slide = slides->getObject(i);
        if (!slide || slide->optValue<std::string>("part", std::string()) != part)
            continue;

        const Poco::JSON::Object::Ptr preview = slide->getObject("preview");
        return preview ? preview->optValue<std::string>("url", std::string()) : std::string();
    }

    return std::string();
}

/// An HTTP session to the server remote documents are reached through, with the timeout of
/// a preview request. Null when there is no such server.
std::shared_ptr<http::Session> createPreviewSession()
{
    const std::string serverUrl = RemoteDocumentBroker::getHttpServerUrl();
    if (serverUrl.empty())
    {
        LOG_ERR("No server URL to fetch slide previews through; set remote_links.server_url "
                "or server_name in the configuration");
        return nullptr;
    }

    std::shared_ptr<http::Session> httpSession = http::Session::create(serverUrl);
    if (!httpSession)
    {
        LOG_ERR("Slide previews cannot be fetched: cannot reach [" << serverUrl << ']');
        return nullptr;
    }

    const std::chrono::seconds timeout(
        ConfigUtil::getConfigValue<int>("remote_links.preview_timeout_secs", 60));
    httpSession->setTimeout(timeout + std::chrono::seconds(5));
    return httpSession;
}
} // namespace

RemoteLinkPrefetcher::RemoteLinkPrefetcher(std::string wopiSrc, std::string accessToken,
                                           std::string tag, IndexHandler onIndexAnswered)
    : _wopiSrc(std::move(wopiSrc))
    , _accessToken(std::move(accessToken))
    , _tag(std::move(tag))
    , _onIndexAnswered(std::move(onIndexAnswered))
{
}

void RemoteLinkPrefetcher::handleCommand(DocumentBroker& docBroker, const std::string& command)
{
    docBroker.assertCorrectThread();

    if (!answerFromIndex(docBroker, command))
        forwardCommand(docBroker, command);
}

void RemoteLinkPrefetcher::forwardCommand(const DocumentBroker& docBroker,
                                          const std::string& command) const
{
    if (!RemoteDocumentBroker::isInitialized())
        return;

    RemoteDocumentBroker::instance().sendCommandAsync(_wopiSrc, _accessToken,
                                                      docBroker.getDocKey(), _tag, command);
}

bool RemoteLinkPrefetcher::answerFromIndex(DocumentBroker& docBroker, const std::string& command)
{
    const bool info = command == "getpresentationinfo";
    const bool sections = command == "getslidesections";
    const bool tile = command.starts_with("tile ");
    if (!info && !sections && !tile)
        return false;

    // The presentation information is the first thing asked for when the slides of a source
    // are read, and read again after the source changed, so that is the request a changed
    // source is fetched anew for. The sections and the pictures asked for right after it wait
    // for the same answer.
    if (!_index || _indexRequest || (info && _indexStale))
    {
        _waiting.push_back(command);
        fetchIndex(docBroker);
        return true;
    }

    if (info || sections)
    {
        const std::string answer = info ? "presentationinfo: " + presentationInfoOf(_index)
                                        : "slidesections: " + slideSectionsOf(_index);
        docBroker.sendRemoteDocumentCommandResult(_tag, Uri::encode(_wopiSrc),
                                                  std::vector<char>(answer.begin(), answer.end()));
        return true;
    }

    std::string part;
    COOLProtocol::getTokenStringFromMessage(command, "part", part);
    const std::string url = pictureUrlOf(_index, part);
    if (url.empty())
    {
        LOG_DBG("The index of [" << Anonymizer::anonymizeUrl(_wopiSrc) << "] has no slide ["
                                 << part << "], the live link answers");
        return false;
    }

    fetchPicture(docBroker, command, url);
    return true;
}

void RemoteLinkPrefetcher::fetchIndex(DocumentBroker& docBroker)
{
    docBroker.assertCorrectThread();

    if (_indexRequest)
        return;

    const std::string linkAnonym = Anonymizer::anonymizeUrl(_wopiSrc);
    std::shared_ptr<http::Session> httpSession = createPreviewSession();
    if (!httpSession)
    {
        onIndexFetched(docBroker, nullptr);
        return;
    }

    // The answer arrives on the broker's own thread, which the request runs on. A
    // subscription dropped while the request was out takes no answer.
    const std::weak_ptr<DocumentBroker> weakDocBroker = docBroker.weak_from_this();
    const std::weak_ptr<RemoteLinkPrefetcher> weakSelf = weak_from_this();
    const std::string tag = _tag;
    httpSession->setFinishedHandler(
        [weakSelf, weakDocBroker, tag, linkAnonym](const std::shared_ptr<http::Session>& session)
        {
            const std::shared_ptr<RemoteLinkPrefetcher> self = weakSelf.lock();
            const std::shared_ptr<DocumentBroker> broker = weakDocBroker.lock();
            if (!self || !broker)
                return;

            const std::shared_ptr<const http::Response> response = session->response();
            const http::StatusCode status =
                response ? response->statusLine().statusCode() : http::StatusCode::BadGateway;
            Poco::JSON::Object::Ptr index;
            if (status != http::StatusCode::OK)
                LOG_WRN("The preview index request of view [" << tag << "] for [" << linkAnonym
                                                              << "] answered " << status);
            else if (!JsonUtil::parseJSON(response->getBody(), index))
                LOG_WRN("The preview index of [" << linkAnonym << "] for view [" << tag
                                                 << "] is not JSON");

            self->onIndexFetched(*broker, index);
        });
    httpSession->setConnectFailHandler(
        [weakSelf, weakDocBroker, tag, linkAnonym](const std::shared_ptr<http::Session>&)
        {
            LOG_WRN("The preview index request of view [" << tag << "] for [" << linkAnonym
                                                          << "] could not connect");
            const std::shared_ptr<RemoteLinkPrefetcher> self = weakSelf.lock();
            const std::shared_ptr<DocumentBroker> broker = weakDocBroker.lock();
            if (self && broker)
                self->onIndexFetched(*broker, nullptr);
        });

    // The token travels in the query, as a browser's does, and is never logged.
    http::Request request("/cool/preview?WOPISrc=" + Uri::encode(_wopiSrc) +
                          "&access_token=" + Uri::encode(_accessToken));
    LOG_INF("Fetching the preview index of [" << linkAnonym << "] for view [" << _tag << "] of ["
                                              << docBroker.getDocKey() << ']');

    _indexRequest = httpSession;
    httpSession->asyncRequest(request, docBroker.getPoll());
}

void RemoteLinkPrefetcher::onIndexFetched(DocumentBroker& docBroker,
                                          const Poco::JSON::Object::Ptr& index)
{
    docBroker.assertCorrectThread();

    _indexRequest.reset();

    std::vector<std::string> waiting;
    waiting.swap(_waiting);

    if (index)
    {
        _index = index;
        _indexStale = false;
        LOG_INF("The preview index of [" << Anonymizer::anonymizeUrl(_wopiSrc)
                                         << "] reached view [" << _tag << "] of ["
                                         << docBroker.getDocKey() << ']');
    }
    else
    {
        LOG_INF("No preview index of [" << Anonymizer::anonymizeUrl(_wopiSrc) << "] for view ["
                                        << _tag << "], " << waiting.size()
                                        << " commands waiting for it go to the live link");
    }

    if (_onIndexAnswered)
        _onIndexAnswered(docBroker);

    for (const std::string& command : waiting)
    {
        if (!index || !answerFromIndex(docBroker, command))
            forwardCommand(docBroker, command);
    }
}

void RemoteLinkPrefetcher::fetchPicture(DocumentBroker& docBroker, const std::string& command,
                                        const std::string& url)
{
    docBroker.assertCorrectThread();

    std::shared_ptr<http::Session> httpSession = createPreviewSession();
    if (!httpSession)
    {
        forwardCommand(docBroker, command);
        return;
    }

    // The answer takes the request out of the ones in flight, and goes to the view as the
    // tile the command asked for, which the live link paints instead when the picture
    // cannot be read. A subscription dropped while the request was out takes no answer.
    const std::weak_ptr<DocumentBroker> weakDocBroker = docBroker.weak_from_this();
    const std::weak_ptr<RemoteLinkPrefetcher> weakSelf = weak_from_this();
    const auto finish = [weakSelf, weakDocBroker, command](
                            const std::shared_ptr<http::Session>& session,
                            const std::string_view png)
    {
        const std::shared_ptr<RemoteLinkPrefetcher> self = weakSelf.lock();
        const std::shared_ptr<DocumentBroker> broker = weakDocBroker.lock();
        if (!self || !broker)
            return;

        std::vector<std::shared_ptr<http::Session>>& requests = self->_pictureRequests;
        requests.erase(std::remove(requests.begin(), requests.end(), session), requests.end());

        if (png.empty())
        {
            self->forwardCommand(*broker, command);
            return;
        }

        static constexpr std::string_view commandName = "tile ";
        std::vector<char> payload;
        payload.reserve(commandName.size() + command.size() + 2 + png.size());
        const std::string header = "tile: " + command.substr(commandName.size()) + "\n";
        payload.insert(payload.end(), header.begin(), header.end());
        payload.insert(payload.end(), png.begin(), png.end());
        broker->sendRemoteDocumentCommandResult(self->_tag, Uri::encode(self->_wopiSrc),
                                                payload);
    };

    const std::string linkAnonym = Anonymizer::anonymizeUrl(_wopiSrc);
    const std::string tag = _tag;
    httpSession->setFinishedHandler(
        [finish, tag, linkAnonym](const std::shared_ptr<http::Session>& session)
        {
            const std::shared_ptr<const http::Response> response = session->response();
            const http::StatusCode status =
                response ? response->statusLine().statusCode() : http::StatusCode::BadGateway;
            if (status != http::StatusCode::OK)
            {
                LOG_WRN("A slide picture request of view [" << tag << "] for [" << linkAnonym
                                                            << "] answered " << status);
                finish(session, std::string_view());
                return;
            }

            finish(session, response->getBody());
        });
    httpSession->setConnectFailHandler(
        [finish, tag, linkAnonym](const std::shared_ptr<http::Session>& session)
        {
            LOG_WRN("A slide picture request of view [" << tag << "] for [" << linkAnonym
                                                        << "] could not connect");
            finish(session, std::string_view());
        });

    LOG_TRC("Fetching a slide picture of [" << linkAnonym << "] for view [" << _tag << ']');
    _pictureRequests.push_back(httpSession);
    httpSession->asyncRequest(http::Request(url), docBroker.getPoll());
}

void RemoteLinkPrefetcher::dumpState(std::ostream& os) const
{
    os << " index: " << (_index ? "yes" : "no") << " stale: " << (_indexStale ? "yes" : "no")
       << " index request: " << (_indexRequest ? "yes" : "no")
       << " waiting commands: " << _waiting.size()
       << " picture requests: " << _pictureRequests.size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
