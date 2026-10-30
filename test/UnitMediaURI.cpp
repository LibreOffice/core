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

// A load balancer that picks the server from the WOPISrc query parameter sends a media request to
// the server that holds its document, and that server answers it.

#include <config.h>

#include <HttpRequest.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <Util.hpp>
#include <WebSocketSession.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>
#include <test.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/ClientSession.hpp>

#include <Poco/URI.h>

using namespace std::literals;

namespace
{
/// The raw value of a query parameter, read the way a load balancer reads it: the query is split
/// at each '&' and nothing is decoded.
std::string getRawQueryParameter(const std::string& uri, const std::string& name)
{
    const auto query = uri.find('?');
    if (query == std::string::npos)
        return std::string();

    for (const std::string& parameter : Util::splitStringToVector(uri.substr(query + 1), '&'))
    {
        if (parameter.starts_with(name + '='))
            return parameter.substr(name.size() + 1);
    }

    return std::string();
}
}

// Inside the WSD process
class UnitMediaURI : public UnitWSD
{
public:
    UnitMediaURI()
        : UnitWSD("UnitMediaURI")
    {
        setTimeout(30s);
    }

    void invokeWSDTest() override
    {
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("hello.odp", documentPath, documentURL, testname);

        std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
            socketPoll(), Poco::URI(helpers::getTestServerURI()), documentURL, testname);

        std::vector<std::shared_ptr<DocumentBroker>> brokers = COOLWSD::getBrokersTestOnly();
        LOK_ASSERT(!brokers.empty());
        auto sessions = brokers[0]->getSessionsTestOnlyUnsafe();
        LOK_ASSERT(!sessions.empty());

        const std::string mediaURI = sessions[0]->createMediaURI("media", "no-such-media");
        const std::string clipboardURI = sessions[0]->getClipboardURI(false);
        TST_LOG("Media URI [" << mediaURI << "], clipboard URI [" << clipboardURI << ']');

        const std::string wopiSrc = getRawQueryParameter(clipboardURI, "WOPISrc");
        LOK_ASSERT(!wopiSrc.empty());
        LOK_ASSERT_EQUAL(wopiSrc, getRawQueryParameter(mediaURI, "WOPISrc"));
        LOK_ASSERT_EQUAL(Util::getProcessIdentifier(),
                         getRawQueryParameter(mediaURI, "ServerId"));
        LOK_ASSERT_EQUAL("no-such-media"s, getRawQueryParameter(mediaURI, "Tag"));

        // The server finds the document and the view from the URI, and only the media is unknown.
        const auto path = mediaURI.find("/cool/media?");
        LOK_ASSERT(path != std::string::npos);
        auto httpSession = http::Session::create(helpers::getTestServerURI());
        const auto response = httpSession->syncRequest(http::Request(mediaURI.substr(path)));
        LOK_ASSERT(response);
        LOK_ASSERT_EQUAL(http::StatusCode::NotFound, response->statusCode());

        socket->asyncShutdown();
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));

        passTest("The media URI reaches the server that holds its document");
    }
};

UnitBase* unit_create_wsd(void) { return new UnitMediaURI(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
