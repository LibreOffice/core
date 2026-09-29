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

/// The remote links of one document, split into a shared part and a
/// private part per browser view.
///
/// The shared part, the same for every view, is the list of remote links
/// this document may open.
///
/// The private part belongs to one view, keyed by that view's tag.
///
/// A view's tag is its ClientSession id. It is the tag reported in
/// remotedocevent messages, so a remote document's reply reaches the one view
/// that opened the subscription.
class RemoteLinks
{
public:
    /// Records the public part of a remote link: its address, the document as the user knows
    /// it, the time it was last modified and the persistent link the open document's pages
    /// store for it. The same for every view. The latest report per WOPISrc wins, and a report
    /// that carries no name or no persistent link keeps the recorded one. A persistent link
    /// belongs to one remote link, so recording it here takes it off any other. A view
    /// reaches a remote link by its persistent link alone, so a report of an address not
    /// recorded yet that names none is ignored.
    void setSource(DocumentBroker& docBroker, const std::string& wopiSrc, const std::string& name,
                   const std::string& lastModifiedTime, const std::string& persistentLink);

    /// Records the source documents the open document's own content names, which is the whole of
    /// what it names: a name reported here and nowhere else stands for a document the storage
    /// listed no remote link for, so nothing can reach it. The same for every view.
    void setNamedSources(DocumentBroker& docBroker, std::vector<std::string> names);

    /// Drops the remote link bound to the given persistent link: the record of it, which is
    /// the same for every view, and in every view the token, the live link and the link
    /// access answer that reach it. Reports whether one was bound to it.
    bool removeSource(DocumentBroker& docBroker, const std::string& persistentLink);

    /// Records the access token one view holds for a remote link. Private
    /// to that view.
    void setViewToken(DocumentBroker& docBroker, const std::string& tag,
                      const std::string& wopiSrc, const std::string& accessToken);

    /// Records that storage answers link access requests, then asks it for every
    /// source the document names that the view holds no remote link for. Private to that view.
    void enableViewLinkAccess(DocumentBroker& docBroker, const std::string& tag);

    /// Takes the storage's answer to one view's link access request for one source.
    void completeLinkAccess(DocumentBroker& docBroker, const std::string& tag,
                            const std::string& persistentLink, unsigned statusCode,
                            const std::string& body);

    /// Asks one view's storage again for the document behind one source the document names,
    /// at the request of the user of that view. Private to that view.
    void resolveSource(DocumentBroker& docBroker, const std::string& tag,
                       const std::string& persistentLink);

    /// Opens or drops one view's subscription to the remote document bound to the given
    /// persistent link. On subscribe the view's own token is used; a view without a token for
    /// the source is refused, and so is a persistent link no remote link is bound to. A
    /// subscription in the failed or missing state holds no connection, so subscribing to its
    /// source again opens a fresh attempt.
    void handleSubscribe(DocumentBroker& docBroker, const std::string& tag,
                         const std::string& persistentLink, bool subscribe);

    /// Handles a remote document event addressed to one view: updates that
    /// view's connection state and re-sends its remote links list.
    /// Content events, which are the same for every view, are passed to the
    /// kit instead.
    void onRemoteEvent(DocumentBroker& docBroker, const std::string& tag,
                       const std::string& encodedWopiSrc, const std::string& eventArguments);

    /// Routes a read-only client command from one view to the remote document bound to the
    /// given persistent link, when that view is subscribed to it.
    void sendCommand(DocumentBroker& docBroker, const std::string& tag,
                     const std::string& persistentLink, const std::string& command);

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

    /// Sends the given view its own remote links list.
    void sendTo(const std::shared_ptr<ClientSession>& session);

    /// The persistent link the remote link at the given WOPISrc is bound to. Empty when no
    /// remote link is recorded at that address.
    std::string persistentLinkOf(const std::string& wopiSrc) const;

    /// True when no remote link source is known.
    bool empty() const { return _entries.empty() && _namedSources.empty(); }

    void dumpState(std::ostream& os) const;

private:
    /// The public part of one remote link, the same for every view.
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
        /// The persistent link to the remote document, which is the name after
        /// vnd.collabora.slide-source:. It is not the WOPISrc, it is the reference the
        /// storage resolves to it, and the one name a view knows the remote link by.
        std::string persistentLink;
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

    /// What asking the storage for the document behind one persistent link came to.
    enum class LinkAccess
    {
        Pending, ///< The request is in flight.
        Resolved, ///< The storage named a document and gave this view a token for it.
        NotFound, ///< The storage knows no document for the reference.
        Denied, ///< This view's user may not reach the document.
        Failed, ///< Any other answer, or none.
    };

    /// The private state of one view.
    struct View
    {
        /// Access tokens the view holds, keyed by the remote document's docKey.
        std::map<std::string, std::string> tokens;
        /// The view's subscriptions, keyed by the remote document's docKey.
        std::map<std::string, Subscription> subscriptions;
        /// The last remotelinks: message sent to the view.
        std::string lastClientMessage;
        /// Whether this view's storage answers a POST to <WOPISrc>/linkaccess, from the
        /// SupportsLinkAccess of its CheckFileInfo.
        bool supportsLinkAccess = false;
        /// Whether the view has asked for the sources the document named when it found them.
        bool askedOnLoad = false;
        /// What asking the storage for each source came to, keyed by the persistent link. A
        /// persistent link with no record has not been asked for by this view.
        std::map<std::string, LinkAccess> linkAccess;
    };

    /// The document a WOPISrc names, as the user knows it: the last part of its path, decoded.
    static std::string documentName(const std::string& wopiSrc);

    /// The document one entry stands for, as the user knows it: the name the integrator gave it,
    /// or the one its WOPISrc names when the integrator gave none.
    static std::string entryName(const Entry& entry);

    /// The remote link the given persistent link is bound to. The end of the entries when
    /// none is.
    std::map<std::string, Entry>::const_iterator
    findListed(const std::string& persistentLink) const;

    /// True when a remote link stands for the source of the given persistent link.
    bool isListed(const std::string& persistentLink) const
    {
        return findListed(persistentLink) != _entries.end();
    }

    /// Asks the storage of every view, or of the given view alone, for the document behind
    /// each source the document names that the view holds no token for and has not asked
    /// for yet. A view asks for the sources it finds on the document once, so the sources
    /// editing names are left to the user to ask for.
    void resolveUnlistedSources(DocumentBroker& docBroker);
    void resolveUnlistedSources(DocumentBroker& docBroker, const std::string& tag);

    /// Sends one view's storage the question for one source, authorized the way every WOPI
    /// request of that view is, and records the question as pending.
    void requestLinkAccess(DocumentBroker& docBroker, const std::string& tag,
                           const std::string& persistentLink);

    /// The name of one link access outcome, for logs and the state dump.
    static const char* linkAccessName(LinkAccess access);

    /// Answers one view's remote document subscription with an error event.
    static void sendError(DocumentBroker& docBroker, const std::string& tag,
                          const std::string& persistentLink, const std::string& kind);

    /// Builds the remotelinks: list for one view: every known source,
    /// stamped with that view's access and connection state.
    std::string buildJson(const std::string& tag) const;

    /// Re-sends the list to one view when it changed.
    void refreshView(DocumentBroker& docBroker, const std::string& tag);

    /// Re-sends the list to every view when the shared part changed.
    void refreshAllViews(DocumentBroker& docBroker);

    /// The remote link sources the document knows, keyed by their docKey.
    std::map<std::string, Entry> _entries;

    /// The source documents the open document's own content names, by the persistent link its
    /// pages record, in the order the document names them. One a remote link is bound to is
    /// reported as that document; any other is reported as a document nothing can reach.
    std::vector<std::string> _namedSources;

    /// docKeys of the documents connected into this document through headless
    /// sessions, from the remotechain option of the sessions' load messages.
    std::vector<std::string> _incomingDocKeyChain;

    /// The private state of each view, keyed by the view's tag.
    std::map<std::string, View> _views;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
