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
#include <vector>

class ClientSession;
class DocumentBroker;

/// The related documents of one document, split into a shared part and a
/// private part per browser view.
///
/// The shared part, the same for every view, is the list of related documents
/// this document may open.
///
/// The private part belongs to one view, keyed by that view's tag.
///
/// A view's tag is its ClientSession id. It is the tag reported in
/// remotedocevent messages, so a remote document's reply reaches the one view
/// that opened the subscription.
class RelatedDocuments
{
public:
    /// Records the public part of a related document: its source, the document as the user knows
    /// it and the time it was last modified. The same for every view. The latest report per
    /// WOPISrc wins, and a report that carries no name keeps the recorded one.
    void setSource(DocumentBroker& docBroker, const std::string& wopiSrc, const std::string& name,
                   const std::string& lastModifiedTime);

    /// Records the source documents the open document's own content names, which is the whole of
    /// what it names: a name reported here and nowhere else stands for a document the storage
    /// listed no related document for, so nothing can reach it. The same for every view.
    void setNamedSources(DocumentBroker& docBroker, std::vector<std::string> names);

    /// Records the access token one view holds for a related document. Private
    /// to that view.
    void setViewToken(DocumentBroker& docBroker, const std::string& tag,
                      const std::string& wopiSrc, const std::string& accessToken);

    /// Opens or drops one view's subscription to a remote document. On
    /// subscribe the view's own token is used; a view without a token for the
    /// source is refused.
    void handleSubscribe(DocumentBroker& docBroker, const std::string& tag,
                         const std::string& encodedWopiSrc, bool subscribe);

    /// Handles a remote document event addressed to one view: updates that
    /// view's connection state and re-sends its related documents list.
    /// Content events, which are the same for every view, are passed to the
    /// kit instead.
    void onRemoteEvent(DocumentBroker& docBroker, const std::string& tag,
                       const std::string& encodedWopiSrc, const std::string& eventArguments);

    /// Routes a read-only client command from one view to a remote document
    /// that view is subscribed to.
    void sendCommand(DocumentBroker& docBroker, const std::string& tag,
                     const std::string& wopiSrc, const std::string& command);

    /// Drops the record of one view's subscription that was not accepted.
    void removeSubscription(DocumentBroker& docBroker, const std::string& tag,
                            const std::string& wopiSrc);

    /// Drops everything one view held, its tokens and its subscriptions, when
    /// the view leaves.
    void removeView(DocumentBroker& docBroker, const std::string& tag);

    /// Drops every view's subscriptions when the document unloads.
    void unsubscribeAll(DocumentBroker& docBroker);

    /// Records the docKeys from a comma-separated chain as connected into this
    /// document through headless sessions, and drops any view's subscription
    /// back to one of them that would close a loop.
    void addToIncomingDocKeyChain(DocumentBroker& docBroker, const std::string& docKeyChain);

    /// Sends the given view its own related documents list.
    void sendTo(const std::shared_ptr<ClientSession>& session);

    /// True when no related document source is known.
    bool empty() const { return _entries.empty() && _namedSources.empty(); }

    void dumpState(std::ostream& os) const;

private:
    /// The public part of one related document, the same for every view.
    struct Entry
    {
        /// The remote document's WOPISrc, decoded, without query parameters.
        std::string wopiSrc;
        /// The remote document as the user knows it, as the integrator named
        /// it. Empty when the integrator named none.
        std::string name;
        /// The time the remote document was last modified, as the integrator
        /// reported it. Empty when the integrator did not provide one.
        std::string lastModifiedTime;
    };

    /// One view's subscription to a remote document.
    struct Subscription
    {
        /// The remote document's WOPISrc, decoded.
        std::string wopiSrc;
        /// The token the subscription was opened with.
        std::string accessToken;
        /// The last connection event: subscribed, connected, disconnected,
        /// failed or missing.
        std::string state;
    };

    /// The private state of one view.
    struct View
    {
        /// Access tokens the view holds, keyed by the remote document's docKey.
        std::map<std::string, std::string> tokens;
        /// The view's subscriptions, keyed by the remote document's docKey.
        std::map<std::string, Subscription> subscriptions;
        /// The last relateddocuments: message sent to the view.
        std::string lastClientMessage;
    };

    /// The document a WOPISrc names, as the user knows it: the last part of its path, decoded.
    static std::string documentName(const std::string& wopiSrc);

    /// The document one entry stands for, as the user knows it: the name the integrator gave it,
    /// or the one its WOPISrc names when the integrator gave none.
    static std::string entryName(const Entry& entry);

    /// Answers one view's remote document subscription with an error event.
    static void sendError(DocumentBroker& docBroker, const std::string& tag,
                          const std::string& encodedWopiSrc, const std::string& kind);

    /// Builds the relateddocuments: list for one view: every known source,
    /// stamped with that view's access and connection state.
    std::string buildJson(const std::string& tag) const;

    /// Re-sends the list to one view when it changed.
    void refreshView(DocumentBroker& docBroker, const std::string& tag);

    /// Re-sends the list to every view when the shared part changed.
    void refreshAllViews(DocumentBroker& docBroker);

    /// The related document sources the document knows, keyed by their docKey.
    std::map<std::string, Entry> _entries;

    /// The source documents the open document's own content names, as the user knows them, in the
    /// order the document names them. One the storage listed as a related document is reported as
    /// that document; any other is reported as a document nothing can reach.
    std::vector<std::string> _namedSources;

    /// docKeys of the documents connected into this document through headless
    /// sessions, from the remotechain option of the sessions' load messages.
    std::vector<std::string> _incomingDocKeyChain;

    /// The private state of each view, keyed by the view's tag.
    std::map<std::string, View> _views;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
