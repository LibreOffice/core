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

#include <net/HttpRequest.hpp>

#include <Poco/JSON/Object.h>

#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class DocumentBroker;

/// The slides of a remote presentation for one view's subscription to it, without its live
/// link: the preview index of the source, fetched over HTTP from the server that holds the
/// source with the token the view holds. The presentation information and the slide sections
/// are answered from the index. Every other command goes to the live link of the
/// subscription. Every call is made on the thread of the document the view belongs to.
class RemoteLinkPrefetcher : public std::enable_shared_from_this<RemoteLinkPrefetcher>
{
public:
    /// Called once an index request answered, with or without an index.
    using IndexHandler = std::function<void(DocumentBroker& docBroker)>;

    RemoteLinkPrefetcher(std::string wopiSrc, std::string accessToken, std::string tag,
                         IndexHandler onIndexAnswered);

    /// Fetches the preview index of the source. Nothing is done while a request is in flight.
    void fetchIndex(DocumentBroker& docBroker);

    /// Answers one command of the view from the index, or queues it when the index is not
    /// there yet. A command the index cannot answer goes to the live link.
    void handleCommand(DocumentBroker& docBroker, const std::string& command);

    /// Records that the source changed, so the next request for the presentation information
    /// fetches the index again.
    void markStale() { _indexStale = true; }

    /// True once an index arrived.
    bool hasIndex() const { return _index; }

    void dumpState(std::ostream& os) const;

private:
    /// Answers one command from the index, or queues it when the index is not there yet.
    /// False for a command the index cannot answer.
    bool answerFromIndex(DocumentBroker& docBroker, const std::string& command);

    /// Takes the answer to the index request: the index on success, null otherwise. The
    /// commands waiting for the index are answered from it, or go to the live link when there
    /// is none.
    void onIndexFetched(DocumentBroker& docBroker, const Poco::JSON::Object::Ptr& index);

    /// Sends one command to the live link of the subscription.
    void forwardCommand(const DocumentBroker& docBroker, const std::string& command) const;

    /// The remote document's WOPISrc, decoded.
    const std::string _wopiSrc;
    /// The token the subscription was opened with.
    const std::string _accessToken;
    /// The tag of the view that holds the subscription.
    const std::string _tag;
    const IndexHandler _onIndexAnswered;
    /// The preview index of the source: the file, its sections and its slides, each with the
    /// URL of its picture. Null until the first index arrived.
    Poco::JSON::Object::Ptr _index;
    /// Whether the source changed since the index was fetched, so the next request for the
    /// presentation information fetches it again.
    bool _indexStale = false;
    /// The index request in flight, null while none is.
    std::shared_ptr<http::Session> _indexRequest;
    /// The commands that wait for the index to arrive.
    std::vector<std::string> _waiting;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
