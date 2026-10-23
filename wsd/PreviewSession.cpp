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

#include "PreviewSession.hpp"

#include <common/Anonymizer.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/StringVector.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <net/HttpHelper.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/Exceptions.hpp>
#include <wsd/RequestDetails.hpp>

#include <Poco/JSON/Array.h>

#include <algorithm>
#include <limits>
#include <cctype>
#include <cmath>
#include <sstream>

namespace
{
/// The HTTP status a failed load of the view maps to, by the kind of the load error.
http::StatusCode statusForLoadFailure(const std::string& kind)
{
    if (kind == "unauthorized")
        return http::StatusCode::Unauthorized;
    if (kind == "docunloading" || kind == "recycling")
        return http::StatusCode::ServiceUnavailable;
    if (kind == "passwordrequired" || kind == "wrongpassword")
        return http::StatusCode::Forbidden;
    return http::StatusCode::BadGateway;
}

std::vector<char> toFrame(const std::string& command)
{
    return std::vector<char>(command.begin(), command.end());
}
} // namespace

int PreviewProtocolHandler::sendTextMessage(const std::string_view msg, bool /*flush*/) const
{
    if (const std::shared_ptr<PreviewSession> session = _session.lock())
        session->onFrame(msg);
    return static_cast<int>(msg.size());
}

int PreviewProtocolHandler::sendBinaryMessage(const std::string_view data, bool /*flush*/) const
{
    if (const std::shared_ptr<PreviewSession> session = _session.lock())
        session->onFrame(data);
    return static_cast<int>(data.size());
}

void PreviewProtocolHandler::shutdown(bool goingAway, const std::string_view statusMessage)
{
    LOG_TRC("Preview view shut down, going away: " << goingAway << ": " << statusMessage);
    if (const std::shared_ptr<PreviewSession> session = _session.lock())
        session->onViewShutdown();
}

void PreviewProtocolHandler::dumpState(std::ostream& os, const std::string& indent) const
{
    os << indent << "PreviewProtocolHandler of tag [";
    if (const std::shared_ptr<PreviewSession> session = _session.lock())
        os << session->getTag();
    os << ']';
}

PreviewSession::PreviewSession(std::string tag, Poco::URI uriPublic, std::string fileName,
                               std::string lastModifiedTime, const std::size_t fileSize,
                               const std::chrono::seconds ttl, const std::chrono::seconds timeout)
    : _tag(std::move(tag))
    , _uriPublic(std::move(uriPublic))
    , _fileName(std::move(fileName))
    , _lastModifiedTime(std::move(lastModifiedTime))
    , _fileSize(fileSize)
    , _ttl(ttl)
    , _timeout(timeout)
    , _state(State::Loading)
    , _expiresAt(std::chrono::steady_clock::now() + ttl)
    , _docWidth(0)
    , _docHeight(0)
    , _infoRequested(false)
    , _nextContentId(ContentIdBase)
{
}

std::string PreviewSession::generateTag() { return Util::rng::getHexString(TagLength); }

bool PreviewSession::isWellFormedTag(const std::string_view tag)
{
    return tag.size() == TagLength &&
           std::all_of(tag.begin(), tag.end(),
                       [](unsigned char c) { return std::isxdigit(c) != 0; });
}

void PreviewSession::attach(DocumentBroker& docBroker, const std::string& id,
                            std::unique_ptr<WopiStorage::WOPIFileInfo> wopiFileInfo)
{
    docBroker.assertCorrectThread();

    try
    {
        _handler = std::make_shared<PreviewProtocolHandler>(weak_from_this());
        _clientSession = docBroker.createNewClientSession(_handler, id, _uriPublic,
                                                          /*isReadOnly=*/true,
                                                          RequestDetails("preview"));
        if (!_clientSession)
        {
            fail(http::StatusCode::ServiceUnavailable, "the document is unloading");
            return;
        }

        docBroker.addSession(_clientSession, std::move(wopiFileInfo));

        // The view speaks the browser protocol: the handshake, then the load.
        sendCommand("coolclient " + COOLProtocol::GetProtocolVersion());
        sendCommand("load url=" + Uri::encode(_uriPublic.toString()));
    }
    catch (const UnauthorizedRequestException& exc)
    {
        LOG_ERR("Preview view of [" << docBroker.getDocKey()
                                    << "] refused by the storage: " << exc.what());
        fail(http::StatusCode::Unauthorized, "unauthorized");
    }
    catch (const StorageConnectionException& exc)
    {
        LOG_ERR("Preview view of [" << docBroker.getDocKey()
                                    << "] failed on the storage: " << exc.what());
        fail(http::StatusCode::BadGateway, "storage error");
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Preview view of [" << docBroker.getDocKey() << "] failed: " << exc.what());
        fail(http::StatusCode::BadGateway, "the document could not be loaded");
    }
}

void PreviewSession::sendCommand(const std::string& command)
{
    if (!_clientSession)
        return;

    LOG_TRC("Preview [" << _tag << "] sends [" << COOLProtocol::getAbbreviatedMessage(command)
                        << ']');
    _clientSession->handleMessage(toFrame(command));
    pumpView();
}

void PreviewSession::pumpView()
{
    // The view queues what it sends its client and writes the queue out when the protocol
    // handler is asked to. The frames are written to this session through the handler.
    if (!_clientSession)
        return;

    // The queue is reached through the handler interface the view implements for its socket.
    MessageHandlerInterface& view = *_clientSession;
    if (view.hasQueuedMessages())
        view.writeQueuedMessages(std::numeric_limits<std::size_t>::max());
}

void PreviewSession::requestIndex(const std::shared_ptr<StreamSocket>& socket,
                                  const std::chrono::steady_clock::time_point now)
{
    if (_state == State::Failed)
    {
        answerError(socket, http::StatusCode::BadGateway, "the document could not be loaded");
        return;
    }

    touch(now);
    _indexRequests.push_back(IndexRequest{ socket, now + _timeout });

    // The index is built from a fresh answer, so a slide added since the last one is listed.
    if (_state == State::Live)
        askPresentationInfo();
}

void PreviewSession::requestContent(const std::shared_ptr<StreamSocket>& socket,
                                    const std::string& guid,
                                    const std::chrono::steady_clock::time_point now)
{
    if (_state == State::Failed)
    {
        answerError(socket, http::StatusCode::BadGateway, "the document could not be loaded");
        return;
    }

    touch(now);
    const int id = _nextContentId++;
    _contentRequests.emplace(id, ContentRequest{ socket, guid, now + _timeout, false });

    if (_presentationInfo)
        sendPendingContentRequests();
    else if (_state == State::Live)
        askPresentationInfo();
}

void PreviewSession::askPresentationInfo()
{
    if (_infoRequested)
        return;

    _infoRequested = true;
    sendCommand("getpresentationinfo");
}

std::chrono::steady_clock::time_point PreviewSession::nextDeadline() const
{
    std::chrono::steady_clock::time_point deadline = _expiresAt;
    for (const IndexRequest& request : _indexRequests)
        deadline = std::min(deadline, request.deadline);
    for (const auto& it : _contentRequests)
        deadline = std::min(deadline, it.second.deadline);
    return deadline;
}

void PreviewSession::checkTimers(const std::chrono::steady_clock::time_point now)
{
    for (auto it = _indexRequests.begin(); it != _indexRequests.end();)
    {
        const std::shared_ptr<StreamSocket> socket = it->socket.lock();
        if (!socket || !socket->isOpen())
        {
            it = _indexRequests.erase(it);
            continue;
        }

        if (now >= it->deadline)
        {
            answerError(socket, http::StatusCode::GatewayTimeout,
                        "the document did not answer in time");
            it = _indexRequests.erase(it);
            continue;
        }

        ++it;
    }

    for (auto it = _contentRequests.begin(); it != _contentRequests.end();)
    {
        const std::shared_ptr<StreamSocket> socket = it->second.socket.lock();
        if (!socket || !socket->isOpen())
        {
            it = _contentRequests.erase(it);
            continue;
        }

        if (now >= it->second.deadline)
        {
            answerError(socket, http::StatusCode::GatewayTimeout,
                        "the document did not answer in time");
            it = _contentRequests.erase(it);
            continue;
        }

        ++it;
    }
}

void PreviewSession::close(DocumentBroker& docBroker, const http::StatusCode status)
{
    docBroker.assertCorrectThread();

    answerAllPending(status, "the preview session is closed");

    if (_clientSession)
    {
        LOG_DBG("Preview [" << _tag << "] leaves [" << docBroker.getDocKey() << ']');
        docBroker.removeSession(_clientSession);
        _clientSession.reset();
    }

    _state = State::Failed;
}

void PreviewSession::abandon(const http::StatusCode status)
{
    answerAllPending(status, "the document is closing");
    _clientSession.reset();
    _state = State::Failed;
}

void PreviewSession::onFrame(const std::string_view frame)
{
    const std::string firstLine = COOLProtocol::getFirstLine(frame.data(), frame.size());
    const StringVector tokens = StringVector::tokenize(firstLine);
    if (tokens.empty())
        return;

    if (tokens.equals(0, "status:"))
        onStatus(firstLine, frame);
    else if (tokens.equals(0, "presentationinfo:"))
        onPresentationInfo(frame);
    else if (tokens.equals(0, "tile:"))
        onTile(tokens, frame, /*gone=*/false);
    else if (tokens.equals(0, "tilegone:"))
        onTile(tokens, frame, /*gone=*/true);
    else if (tokens.equals(0, "error:"))
        onError(tokens);
}

void PreviewSession::onStatus(const std::string_view /*firstLine*/, const std::string_view frame)
{
    if (_state == State::Loading)
    {
        // The first status says what kind of document the view shows.
        Poco::JSON::Object::Ptr status;
        std::string type;
        if (JsonUtil::parseJSON(frame, status))
            JsonUtil::findJSONValue(status, "type", type);

        if (type != "presentation")
        {
            LOG_WRN("Preview [" << _tag << "] refused: the document is a [" << type
                                << "], not a presentation");
            fail(http::StatusCode::UnsupportedMediaType, "the document is not a presentation");
            return;
        }

        LOG_DBG("Preview [" << _tag << "] is live");
        _state = State::Live;
    }

    if (hasPendingRequests())
        askPresentationInfo();
}

void PreviewSession::onPresentationInfo(const std::string_view frame)
{
    _infoRequested = false;

    Poco::JSON::Object::Ptr info;
    if (!JsonUtil::parseJSON(frame, info))
    {
        LOG_ERR("Preview [" << _tag << "] got presentation information it cannot read");
        answerAllPending(http::StatusCode::BadGateway, "the document answered badly");
        return;
    }

    _presentationInfo = info;
    _slidesByGuid.clear();
    _docWidth = info->optValue<int>("docWidth", 0);
    _docHeight = info->optValue<int>("docHeight", 0);

    if (Poco::JSON::Array::Ptr slides = info->getArray("slides"))
    {
        for (std::size_t i = 0; i < slides->size(); ++i)
        {
            Poco::JSON::Object::Ptr slide = slides->getObject(i);
            if (!slide)
                continue;

            const std::string guid = slide->optValue<std::string>("part", std::string());
            if (!guid.empty())
                _slidesByGuid.emplace(guid, slide);
        }
    }

    std::vector<IndexRequest> requests;
    requests.swap(_indexRequests);
    for (const IndexRequest& request : requests)
    {
        if (const std::shared_ptr<StreamSocket> socket = request.socket.lock())
            answerIndex(socket);
    }

    sendPendingContentRequests();
}

void PreviewSession::sendPendingContentRequests()
{
    if (!_presentationInfo)
        return;

    // A tile the tile cache holds is answered while its request is sent, and the answer
    // removes the request from the map, so the commands are sent once the walk is over.
    std::vector<std::string> commands;
    for (auto it = _contentRequests.begin(); it != _contentRequests.end();)
    {
        ContentRequest& request = it->second;
        if (request.sent)
        {
            ++it;
            continue;
        }

        if (_slidesByGuid.find(request.guid) == _slidesByGuid.end() || _docWidth <= 0 ||
            _docHeight <= 0)
        {
            if (const std::shared_ptr<StreamSocket> socket = request.socket.lock())
                answerError(socket, http::StatusCode::NotFound, "no such slide");
            it = _contentRequests.erase(it);
            continue;
        }

        // The whole slide as one preview tile, which the kit answers with the id echoed back.
        const int height = std::max(
            1, static_cast<int>(std::lround(static_cast<double>(PreviewWidth) * _docHeight /
                                            _docWidth)));
        std::ostringstream command;
        command << "tile nviewid=0 part=" << request.guid << " mode=0 width=" << PreviewWidth
                << " height=" << height << " tileposx=0 tileposy=0 tilewidth=" << _docWidth
                << " tileheight=" << _docHeight << " id=" << it->first;
        request.sent = true;
        commands.push_back(command.str());
        ++it;
    }

    for (const std::string& command : commands)
        sendCommand(command);
}

void PreviewSession::onTile(const StringVector& tokens, const std::string_view frame,
                            const bool gone)
{
    int id = -1;
    if (!COOLProtocol::getTokenInteger(tokens, "id", id) || id < ContentIdBase)
    {
        // A preview a browser view asked for, which the kit sends every view.
        return;
    }

    const auto it = _contentRequests.find(id);
    if (it == _contentRequests.end())
        return;

    std::string part;
    COOLProtocol::getTokenString(tokens, "part", part);
    if (part != it->second.guid)
    {
        LOG_WRN("Preview [" << _tag << "] got a tile of slide [" << part << "] for a request of ["
                            << it->second.guid << "], dropped");
        return;
    }

    const std::shared_ptr<StreamSocket> socket = it->second.socket.lock();
    _contentRequests.erase(it);
    if (!socket)
        return;

    if (gone)
    {
        answerError(socket, http::StatusCode::NotFound, "the slide is gone");
        return;
    }

    const std::size_t newline = frame.find('\n');
    if (newline == std::string_view::npos || newline + 1 >= frame.size())
    {
        answerError(socket, http::StatusCode::BadGateway, "the slide picture is empty");
        return;
    }

    // The tile store keeps a picture without its first byte, which is the format marker of
    // the frame it came in: for a PNG that is the first byte of the PNG signature, so the
    // signature is completed here.
    static constexpr std::string_view PngSignatureTail = "PNG\r\n\x1a\n";
    const std::string_view payload = frame.substr(newline + 1);
    std::string png;
    png.reserve(payload.size() + 1);
    if (payload.starts_with(PngSignatureTail))
        png.push_back('\x89');
    png.append(payload);
    answerContent(socket, png);
}

void PreviewSession::onError(const StringVector& tokens)
{
    std::string cmd;
    std::string kind;
    COOLProtocol::getTokenString(tokens, "cmd", cmd);
    COOLProtocol::getTokenString(tokens, "kind", kind);

    if (cmd == "load" || cmd == "internal" || cmd == "storage")
    {
        LOG_ERR("Preview [" << _tag << "] failed to load its view: cmd=" << cmd
                            << " kind=" << kind);
        fail(statusForLoadFailure(kind), "the document could not be loaded: " + kind);
    }
    else
    {
        LOG_DBG("Preview [" << _tag << "] ignores an error: cmd=" << cmd << " kind=" << kind);
    }
}

void PreviewSession::onViewShutdown()
{
    if (_state == State::Failed)
        return;

    LOG_DBG("Preview [" << _tag << "] lost its view");
    fail(http::StatusCode::ServiceUnavailable, "the document closed");
}

void PreviewSession::fail(const http::StatusCode status, const std::string& reason)
{
    _state = State::Failed;
    answerAllPending(status, reason);
}

void PreviewSession::answerAllPending(const http::StatusCode status, const std::string& reason)
{
    std::vector<IndexRequest> indexRequests;
    indexRequests.swap(_indexRequests);
    for (const IndexRequest& request : indexRequests)
    {
        if (const std::shared_ptr<StreamSocket> socket = request.socket.lock())
            answerError(socket, status, reason);
    }

    std::map<int, ContentRequest> contentRequests;
    contentRequests.swap(_contentRequests);
    for (const auto& it : contentRequests)
    {
        if (const std::shared_ptr<StreamSocket> socket = it.second.socket.lock())
            answerError(socket, status, reason);
    }
}

void PreviewSession::answerError(const std::shared_ptr<StreamSocket>& socket,
                                 const http::StatusCode status, const std::string& reason)
{
    HttpHelper::sendErrorAndShutdown(status, socket, reason);
}

void PreviewSession::answerIndex(const std::shared_ptr<StreamSocket>& socket)
{
    // The ttl counts from the last answer, so a slow load does not eat into it.
    touch(std::chrono::steady_clock::now());

    http::Response response(http::StatusCode::OK);
    response.add("X-Content-Type-Options", "nosniff");
    response.set("Cache-Control", "no-store");
    response.setBody(buildIndexJson(), "application/json");
    socket->sendAndShutdown(response);
}

void PreviewSession::answerContent(const std::shared_ptr<StreamSocket>& socket,
                                   const std::string_view png)
{
    touch(std::chrono::steady_clock::now());

    http::Response response(http::StatusCode::OK);
    response.add("X-Content-Type-Options", "nosniff");
    response.set("Cache-Control", "private, max-age=" + std::to_string(remainingTtlSeconds()));
    response.setBody(std::string(png), "image/png");
    socket->sendAndShutdown(response);
}

long PreviewSession::remainingTtlSeconds() const
{
    const auto now = std::chrono::steady_clock::now();
    if (now >= _expiresAt)
        return 0;
    return std::chrono::duration_cast<std::chrono::seconds>(_expiresAt - now).count();
}

std::string PreviewSession::buildContentUrl(const std::string& guid) const
{
    Poco::URI wopiSrc(_uriPublic);
    wopiSrc.setQueryParameters(Poco::URI::QueryParameters());

    // The tag is hexadecimal, so it goes into the path as it is.
    std::string url = "/cool/preview/" + _tag +
                      "/content?WOPISrc=" + Uri::encode(wopiSrc.toString()) +
                      "&ServerId=" + Util::getProcessIdentifier() + "&slide=" + Uri::encode(guid);
    if (!COOLWSD::RouteToken.empty())
        url += "&RouteToken=" + COOLWSD::RouteToken;
    return url;
}

std::string PreviewSession::buildIndexJson() const
{
    const long ttl = remainingTtlSeconds();
    const auto expiresAt = std::chrono::system_clock::now() + std::chrono::seconds(ttl);

    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("version", 1);
    root->set("tag", _tag);
    root->set("expiresAt", Util::getIso8601FracformatTime(expiresAt));
    root->set("ttlSecs", ttl);

    Poco::JSON::Object::Ptr file = new Poco::JSON::Object();
    file->set("name", _fileName);
    file->set("lastModifiedTime", _lastModifiedTime);
    file->set("size", static_cast<Poco::UInt64>(_fileSize));
    file->set("docWidth", _docWidth);
    file->set("docHeight", _docHeight);

    Poco::JSON::Array::Ptr sections = new Poco::JSON::Array();
    Poco::JSON::Array::Ptr slides = new Poco::JSON::Array();
    if (_presentationInfo)
    {
        if (Poco::JSON::Array::Ptr infoSections = _presentationInfo->getArray("sections"))
            sections = infoSections;

        if (Poco::JSON::Array::Ptr infoSlides = _presentationInfo->getArray("slides"))
        {
            for (std::size_t i = 0; i < infoSlides->size(); ++i)
            {
                Poco::JSON::Object::Ptr infoSlide = infoSlides->getObject(i);
                if (!infoSlide)
                    continue;

                const std::string guid = infoSlide->optValue<std::string>("part", std::string());
                const int width = infoSlide->optValue<int>("slideWidth", _docWidth);
                const int height = infoSlide->optValue<int>("slideHeight", _docHeight);

                Poco::JSON::Object::Ptr slide = new Poco::JSON::Object();
                slide->set("index", infoSlide->optValue<int>("index", static_cast<int>(i)));
                slide->set("part", guid);
                slide->set("guid", infoSlide->optValue<std::string>("guid", guid));
                slide->set("hash", infoSlide->optValue<std::string>("hash", std::string()));
                slide->set("name", infoSlide->optValue<std::string>("name", std::string()));
                slide->set("hidden", infoSlide->optValue<bool>("hidden", false));
                slide->set("empty", infoSlide->optValue<bool>("empty", false));
                slide->set("width", width);
                slide->set("height", height);

                Poco::JSON::Object::Ptr preview = new Poco::JSON::Object();
                preview->set("mime", "image/png");
                preview->set("width", PreviewWidth);
                preview->set("height",
                             width > 0 ? static_cast<int>(std::lround(
                                             static_cast<double>(PreviewWidth) * height / width))
                                       : 0);
                preview->set("url", buildContentUrl(guid));
                slide->set("preview", preview);

                slides->add(slide);
            }
        }
    }

    file->set("slideCount", static_cast<int>(slides->size()));
    root->set("file", file);
    root->set("sections", sections);
    root->set("slides", slides);

    std::ostringstream oss;
    root->stringify(oss);
    return oss.str();
}

void PreviewSession::dumpState(std::ostream& os) const
{
    os << "\n    PreviewSession [" << _tag;
    os << "]:\n      uri: " << Anonymizer::anonymizeUrl(_uriPublic.toString());
    os << "\n      fileName: " << _fileName;
    os << "\n      lastModifiedTime: " << _lastModifiedTime;
    os << "\n      fileSize: " << _fileSize;
    os << "\n      ttl: " << _ttl.count();
    os << "s\n      timeout: " << _timeout.count();
    os << "s\n      state: " << name(_state);
    os << "\n      expires in: " << remainingTtlSeconds();
    os << "s\n      view: " << (_clientSession ? _clientSession->getId() : std::string("none"));
    os << "\n      presentationInfo: " << (_presentationInfo ? "yes" : "no");
    os << "\n      slides: " << _slidesByGuid.size();
    os << "\n      docWidth: " << _docWidth;
    os << "\n      docHeight: " << _docHeight;
    os << "\n      infoRequested: " << _infoRequested;
    os << "\n      indexRequests: " << _indexRequests.size();
    os << "\n      contentRequests: " << _contentRequests.size();
    os << "\n      nextContentId: " << _nextContentId;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
