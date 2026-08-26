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

#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/StringVector.hpp>
#include <common/Uri.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/RequestDetails.hpp>

#include <Poco/JSON/Object.h>

#include <sstream>

void RelatedDocuments::add(DocumentBroker& docBroker, const std::string& docKey,
                           const std::string& wopiSrc)
{
    _entries[docKey].wopiSrc = wopiSrc.substr(0, wopiSrc.find('?'));
    refresh(docBroker);
}

void RelatedDocuments::setSubscribed(DocumentBroker& docBroker, const std::string& docKey)
{
    const auto it = _entries.find(docKey);
    if (it != _entries.end())
    {
        it->second.lastState = "subscribed";
        refresh(docBroker);
    }
}

void RelatedDocuments::onEvent(DocumentBroker& docBroker, const std::string& encodedWopiSrc,
                               const std::string& eventArguments)
{
    const StringVector arguments = StringVector::tokenize(eventArguments);
    std::string event;
    if (arguments.size())
        COOLProtocol::getTokenString(arguments[0], "event", event);

    // Content events carry no subscription-state change.
    if (event == "modified" || event == "invalidated" || event == "structure" || event.empty())
        return;

    try
    {
        const std::string docKey = RequestDetails::getDocKey(Uri::decode(encodedWopiSrc));
        const auto it = _entries.find(docKey);
        if (it != _entries.end())
        {
            if (event == "connected" || event == "disconnected" || event == "failed")
                it->second.lastState = event;
            refresh(docBroker);
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Cannot mirror the remote document event: " << exc.what());
    }
}

void RelatedDocuments::refresh(DocumentBroker& docBroker)
{
    const std::string message = "relateddocuments: " + buildJson(docBroker);
    if (message == _lastClientMessage)
        return;

    _lastClientMessage = message;
    docBroker.broadcastMessage(message);
}

void RelatedDocuments::sendTo(const DocumentBroker& docBroker,
                              const std::shared_ptr<ClientSession>& session) const
{
    session->sendTextFrame("relateddocuments: " + buildJson(docBroker));
}

std::string RelatedDocuments::buildJson(const DocumentBroker& docBroker) const
{
    Poco::JSON::Array::Ptr documents = new Poco::JSON::Array();
    for (const auto& it : _entries)
    {
        Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
        entry->set("wopiSrc", it.second.wopiSrc);
        entry->set("state", docBroker.hasRemoteSubscription(it.first)
                                ? it.second.lastState
                                : std::string("available"));
        documents->add(entry);
    }

    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
    root->set("documents", documents);

    std::ostringstream oss;
    root->stringify(oss);
    return oss.str();
}

void RelatedDocuments::dumpState(const DocumentBroker& docBroker, std::ostream& os) const
{
    os << "\n  related documents: " << _entries.size();
    for (const auto& it : _entries)
        os << "\n    " << it.first << " state: "
           << (docBroker.hasRemoteSubscription(it.first) ? it.second.lastState
                                                         : std::string("available"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
