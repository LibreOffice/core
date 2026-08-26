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

#include <iosfwd>
#include <map>
#include <memory>
#include <string>

class ClientSession;
class DocumentBroker;

/// The client-facing view of the related documents of one document: the
/// WOPISrc and subscription state per entry, published to the clients as the
/// relateddocuments: message. Access tokens are never stored here.
class RelatedDocuments
{
public:
    /// Records a related document under its docKey. Query parameters of the
    /// WOPISrc are dropped.
    void add(DocumentBroker& docBroker, const std::string& docKey, const std::string& wopiSrc);

    /// Marks the entry with the given docKey as subscribed.
    void setSubscribed(DocumentBroker& docBroker, const std::string& docKey);

    /// Mirrors a remotedocevent into the entry states.
    void onEvent(DocumentBroker& docBroker, const std::string& encodedWopiSrc,
                 const std::string& eventArguments);

    /// Sends the list to every session when it changed.
    void refresh(DocumentBroker& docBroker);

    /// Sends the current list to the given session.
    void sendTo(const DocumentBroker& docBroker,
                const std::shared_ptr<ClientSession>& session) const;

    bool empty() const { return _entries.empty(); }

    void dumpState(const DocumentBroker& docBroker, std::ostream& os) const;

private:
    struct Entry
    {
        /// The remote document's WOPISrc, decoded, without query parameters.
        std::string wopiSrc;
        /// The last connection event of a subscription: subscribed,
        /// connected, disconnected or failed.
        std::string lastState;
    };

    /// The entries and their subscription states as JSON.
    std::string buildJson(const DocumentBroker& docBroker) const;

    /// The related documents the document knows, keyed by their docKey.
    std::map<std::string, Entry> _entries;

    /// The last relateddocuments: message sent to the clients.
    std::string _lastClientMessage;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
