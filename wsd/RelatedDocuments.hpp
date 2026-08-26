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
#include <set>
#include <string>
#include <tuple>
#include <vector>

class ClientSession;
class DocumentBroker;
class Message;

/// The related documents of one document: the access tokens for the entries
/// the document may subscribe to (from the RelatedDocuments field of
/// CheckFileInfo and from POST /cool/relateddocument), the live
/// subscriptions, the docKey chain of the incoming headless sessions, and
/// the relateddocuments: view of it all for the clients. The view never
/// carries the tokens.
class RelatedDocuments
{
public:
    /// Stores the access token to use for a subscription to the given remote
    /// document. The latest token per WOPISrc wins.
    void setToken(DocumentBroker& docBroker, const std::string& wopiSrc,
                  const std::string& accessToken);

    /// Handles a remotedocsubscribe: or remotedocunsubscribe: message from the kit.
    void handleMessage(DocumentBroker& docBroker, const std::shared_ptr<Message>& message,
                       bool subscribe);

    /// Sends a remote document event to the kit and mirrors the subscription
    /// state to the clients.
    void sendEvent(DocumentBroker& docBroker, const std::string& tag,
                   const std::string& encodedWopiSrc, const std::string& eventArguments);

    /// Removes the record of a remote document subscription that was not
    /// accepted.
    void removeSubscription(DocumentBroker& docBroker, const std::string& wopiSrc,
                            const std::string& tag);

    /// Drops every remote document subscription the document holds.
    void unsubscribeAll(DocumentBroker& docBroker);

    /// Records the docKeys from a comma-separated chain as linked to the
    /// document through headless sessions.
    void addToIncomingDocKeyChain(DocumentBroker& docBroker, const std::string& docKeyChain);

    /// Sends the current related documents list to the given session.
    void sendTo(const std::shared_ptr<ClientSession>& session) const;

    bool empty() const { return _entries.empty(); }

    void dumpState(std::ostream& os) const;

private:
    struct Entry
    {
        /// The remote document's WOPISrc, decoded, without query parameters.
        std::string wopiSrc;
        /// The last connection event of a subscription: subscribed,
        /// connected, disconnected or failed.
        std::string lastState;
    };

    /// Mirrors a remotedocevent into the entry states.
    void onEvent(DocumentBroker& docBroker, const std::string& encodedWopiSrc,
                 const std::string& eventArguments);

    /// Marks the entry with the given docKey as subscribed.
    void setSubscribed(DocumentBroker& docBroker, const std::string& docKey);

    /// Sends the list to every session when it changed.
    void refresh(DocumentBroker& docBroker);

    /// Answers a remote document subscription with an error event.
    void sendError(DocumentBroker& docBroker, const std::string& tag,
                   const std::string& encodedWopiSrc, const std::string& kind);

    /// Returns true when the document holds a subscription to the remote
    /// document with the given docKey.
    bool hasSubscription(const std::string& remoteDocKey) const;

    /// The entries and their subscription states as JSON.
    std::string buildJson() const;

    /// The related documents the document knows, keyed by their docKey.
    std::map<std::string, Entry> _entries;

    /// Access tokens for the remote documents, keyed by their docKey.
    std::map<std::string, std::string> _tokens;

    /// The remote documents the document is subscribed to, as
    /// (WOPISrc, access token, link tag) tuples.
    std::set<std::tuple<std::string, std::string, std::string>> _subscriptions;

    /// docKeys of the documents connected to the document through headless
    /// sessions, from the remotechain option of the sessions' load messages.
    std::vector<std::string> _incomingDocKeyChain;

    /// The last relateddocuments: message sent to the clients.
    std::string _lastClientMessage;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
