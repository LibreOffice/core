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

#include <common/StateEnum.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>
#include <wsd/wopi/WopiStorage.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/URI.h>

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class ClientSession;
class DocumentBroker;
class PreviewSession;

/// The protocol of a preview view, which is a view of a document that has no socket of its
/// own. The frames the kit sends that view are handed to the preview session that owns the
/// view, and nothing is ever read.
class PreviewProtocolHandler final : public ProtocolHandlerInterface
{
public:
    explicit PreviewProtocolHandler(std::weak_ptr<PreviewSession> session)
        : _session(std::move(session))
    {
    }

    void onConnect(const std::shared_ptr<StreamSocket>& /*socket*/) override {}

    void handleIncomingMessage(SocketDisposition& /*disposition*/) override {}

    int getPollEvents(std::chrono::steady_clock::time_point /*now*/,
                      int64_t& /*timeoutMaxMicroS*/) override
    {
        return 0;
    }

    void performWrites(std::size_t /*capacity*/) override {}

    void onDisconnect() override {}

    int sendTextMessage(std::string_view msg, bool flush = false) const override;

    int sendBinaryMessage(std::string_view data, bool flush = false) const override;

    void shutdown(bool goingAway = false,
                  std::string_view statusMessage = std::string_view()) override;

    void getIOStats(uint64_t& sent, uint64_t& recv) override { sent = recv = 0; }

    void dumpState(std::ostream& os, const std::string& indent) const override;

private:
    std::weak_ptr<PreviewSession> _session;
};

/// The previews of the slides of one document for one access token, reached through a tag.
///
/// The session holds a read-only view of the document, created without a socket, and
/// answers two kinds of HTTP request through it: the index, which describes the document and
/// its slides, and the content of one slide, which is a picture of it. The tag stands for the
/// token the index was asked for with, so a content request carries the tag and no token.
class PreviewSession final : public std::enable_shared_from_this<PreviewSession>
{
public:
    static constexpr int PreviewWidth = 320;

    static constexpr std::size_t TagLength = 64;

    /// How long a tag, and the view behind it, stay after their last use. The pictures are
    /// asked for right after the index, so a short time covers an answer with room to spare.
    static constexpr std::chrono::seconds TagTtl{ 15 };

    /// The first tile id a content request uses. The ids below it belong to the previews of
    /// a browser view, which the kit answers with the same frames.
    static constexpr int ContentIdBase = 1 << 20;

    STATE_ENUM(State,
               Loading, ///< The view is being created, no status has arrived yet.
               Live, ///< The view is up and takes requests.
               Failed ///< The view could not be created or was lost; the session is done.
    );

    PreviewSession(std::string tag, Poco::URI uriPublic, std::string fileName,
                   std::string lastModifiedTime, std::size_t fileSize,
                   std::chrono::seconds ttl, std::chrono::seconds timeout);

    static std::string generateTag();

    static bool isWellFormedTag(std::string_view tag);

    const std::string& getTag() const { return _tag; }

    State getState() const { return _state; }

    /// Whether this session was opened for the given document and token.
    bool matches(const Poco::URI& uriPublic) const
    {
        return _uriPublic.toString() == uriPublic.toString();
    }

    /// Creates the view on the given broker and starts loading the document into it. On
    /// failure every request queued so far is answered and the session is Failed.
    void attach(DocumentBroker& docBroker, const std::string& id,
                std::unique_ptr<WopiStorage::WOPIFileInfo> wopiFileInfo);

    /// Answers the given socket with the index, once the view has the presentation
    /// information.
    void requestIndex(const std::shared_ptr<StreamSocket>& socket,
                      std::chrono::steady_clock::time_point now);

    /// Answers the given socket with the picture of the slide with the given GUID.
    void requestContent(const std::shared_ptr<StreamSocket>& socket, const std::string& guid,
                        std::chrono::steady_clock::time_point now);

    /// Moves the expiry of the session one ttl past now.
    void touch(std::chrono::steady_clock::time_point now) { _expiresAt = now + _ttl; }

    /// Whether the ttl passed. A session with requests waiting, or one that is filling the
    /// cache, stays until they are answered or reach their deadline.
    bool isExpired(std::chrono::steady_clock::time_point now) const
    {
        return now >= _expiresAt && !_fillingCache && !hasPendingRequests();
    }

    /// The earliest moment something is due: the expiry of the session or the deadline of a
    /// pending request.
    std::chrono::steady_clock::time_point nextDeadline() const;

    /// Whether the session has requests it has not answered yet.
    bool hasPendingRequests() const
    {
        return !_indexRequests.empty() || !_contentRequests.empty();
    }

    /// Answers the requests whose deadline passed, and drops the ones whose socket closed.
    void checkTimers(std::chrono::steady_clock::time_point now);

    /// Answers every pending request with the given status and removes the view from the
    /// broker.
    void close(DocumentBroker& docBroker, http::StatusCode status);

    /// Answers every pending request with the given status and lets go of the view without
    /// removing it, for when the broker is dropping every view itself.
    void abandon(http::StatusCode status);

    /// Hands over the frames the view has queued for its client, which this session is.
    void pumpView();

    /// A frame the kit sent the view.
    void onFrame(std::string_view frame);

    /// The view was shut down from the broker's side.
    void onViewShutdown();

    void dumpState(std::ostream& os) const;

private:
    struct IndexRequest
    {
        std::weak_ptr<StreamSocket> socket;
        std::chrono::steady_clock::time_point deadline;
    };

    struct ContentRequest
    {
        std::weak_ptr<StreamSocket> socket;
        std::string guid;
        std::chrono::steady_clock::time_point deadline;
        /// Whether the tile was asked for. False while the presentation information the
        /// request needs has not arrived.
        bool sent = false;
        /// Whether the session asked for the picture itself, to fill the cache. Such a request
        /// has no socket.
        bool forCache = false;
    };

    /// Sends a client command to the view, as a browser would.
    void sendCommand(const std::string& command);

    /// Asks the view for the presentation information, once per answer.
    void askPresentationInfo();

    void onStatus(std::string_view firstLine, std::string_view frame);
    void onPresentationInfo(std::string_view json);
    void onTile(const StringVector& tokens, std::string_view frame, bool gone);
    void onError(const StringVector& tokens);

    /// Sends the tile request of a content request whose slide the presentation information
    /// lists, and answers the ones it does not list.
    void sendPendingContentRequests();

    /// Asks for the picture of every slide, once, so the cache gets the whole document.
    void startFillingCache();

    /// Stores the index and the pictures in the cache once every slide has its picture, and
    /// makes the tag of this session a name for the stored entry.
    void storeInCacheWhenComplete();

    /// Marks the session Failed and answers every pending request with the given status.
    void fail(http::StatusCode status, const std::string& reason);

    void answerIndex(const std::shared_ptr<StreamSocket>& socket);
    void answerContent(const std::shared_ptr<StreamSocket>& socket, std::string_view png);
    void answerAllPending(http::StatusCode status, const std::string& reason);
    static void answerError(const std::shared_ptr<StreamSocket>& socket, http::StatusCode status,
                            const std::string& reason);

    /// The index without the parts that belong to one answer: the shape the cache stores.
    Poco::JSON::Object::Ptr buildIndex() const;

    /// The document URI without its query, which names the document in the picture URLs.
    Poco::URI documentUri() const;

    /// The seconds left before the session expires, zero when it already has.
    long remainingTtlSeconds() const;

    const std::string _tag;
    /// The document URI with the access token and the read-only permission.
    const Poco::URI _uriPublic;
    const std::string _fileName;
    const std::string _lastModifiedTime;
    const std::size_t _fileSize;
    const std::chrono::seconds _ttl;
    const std::chrono::seconds _timeout;

    std::shared_ptr<PreviewProtocolHandler> _handler;
    std::shared_ptr<ClientSession> _clientSession;
    /// The broker the view belongs to, which says whether the document has unsaved changes.
    std::weak_ptr<DocumentBroker> _docBroker;

    State _state;
    std::chrono::steady_clock::time_point _expiresAt;

    /// The last presentationinfo: answer, parsed. Null until the first answer.
    Poco::JSON::Object::Ptr _presentationInfo;
    /// The slides the presentation information lists, by GUID.
    std::map<std::string, Poco::JSON::Object::Ptr> _slidesByGuid;
    int _docWidth;
    int _docHeight;
    /// Whether a presentationinfo request is out and not answered yet.
    bool _infoRequested;

    std::vector<IndexRequest> _indexRequests;
    /// The content requests in flight, by tile id.
    std::map<int, ContentRequest> _contentRequests;
    int _nextContentId;

    /// Whether the session is collecting the picture of every slide for the cache.
    bool _fillingCache;
    /// Whether the cache got the pictures of this session.
    bool _storedInCache;
    /// The pictures collected for the cache, by slide GUID.
    std::map<std::string, std::string> _cachePictures;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
