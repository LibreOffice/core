/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Test various copy/paste pieces ...

#include <config.h>

#include <HttpRequest.hpp>
#include <Unit.hpp>
#include <WebSocketSession.hpp>
#include <common/JsonUtil.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/ClientSession.hpp>

using namespace std::literals;
using namespace Poco::Net;

std::shared_ptr<ClientSession> getChildSession(size_t session)
{
    std::shared_ptr<DocumentBroker> broker;
    std::shared_ptr<ClientSession> clientSession;

    std::vector<std::shared_ptr<DocumentBroker>> brokers = COOLWSD::getBrokersTestOnly();
    assert(brokers.size() > 0);
    broker = brokers[0];
    auto sessions = broker->getSessionsTestOnlyUnsafe();
    assert(sessions.size() > 0 && session < sessions.size());
    return sessions[session];
}

std::string getSessionClipboardURI(size_t session)
{
    std::shared_ptr<ClientSession> clientSession = getChildSession(session);

    std::string tag = clientSession->getClipboardURI(false); // nominally thread unsafe
    return tag;
}

// Inside the WSD process
class UnitCopyPasteWriter : public UnitWSD
{
    STATE_ENUM(Phase, RunTest, WaitDocClose, PostCloseTest, Done) _phase;

public:
    UnitCopyPasteWriter()
        : UnitWSD("UnitCopyPasteWriter")
        , _phase(Phase::RunTest)
    {
    }

    void onDocBrokerDestroy(const std::string& /*docKey*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);
        TRANSITION_STATE(_phase, Phase::PostCloseTest);
    }

    void runTest()
    {
        // Given a Writer document with bullets:
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("bullets.odt", documentPath, documentURL, testname);
        std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
            socketPoll(), Poco::URI(helpers::getTestServerURI()), documentURL, testname);
        helpers::sendTextFrame(socket, "uno .uno:SelectAll", testname);

        // When copying the content of the document:
        helpers::sendAndDrain(socket, testname, "uno .uno:Copy", "statechanged:");

        // Then make sure asking for multiple, specific formats results in a JSON answer, it's what
        // JS expects:
        {
            std::string clipURI = getSessionClipboardURI(0);
            clipURI += "&MimeType=text/html,text/plain;charset=utf-8";
            std::shared_ptr<http::Session> httpSession = http::Session::create(clipURI);
            std::shared_ptr<const http::Response> httpResponse =
                httpSession->syncRequest(http::Request(Poco::URI(clipURI).getPathAndQuery()));
            LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusLine().statusCode());
            const std::string& body = httpResponse->getBody();
            Poco::JSON::Object::Ptr object;
            // This failed, we didn't return JSON.
            LOK_ASSERT(JsonUtil::parseJSON(body, object));
            LOK_ASSERT(object->has("text/html"));
            std::string expectedPlainText("    • first\n    • second\n    • third");
            std::string actualPlainText = object->get("text/plain;charset=utf-8").toString();
            LOK_ASSERT_EQUAL(actualPlainText, expectedPlainText);
        }

        // Now also test HTML only:
        {
            std::string clipURI = getSessionClipboardURI(0);
            clipURI += "&MimeType=text/html";
            std::shared_ptr<http::Session> httpSession = http::Session::create(clipURI);
            std::shared_ptr<const http::Response> httpResponse =
                httpSession->syncRequest(http::Request(Poco::URI(clipURI).getPathAndQuery()));
            LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusLine().statusCode());
            const std::string& body = httpResponse->getBody();
            Poco::JSON::Object::Ptr object;
            LOK_ASSERT(JsonUtil::parseJSON(body, object));
            LOK_ASSERT(object->has("text/html"));
        }

        TRANSITION_STATE(_phase, Phase::WaitDocClose);
        socket->asyncShutdown();
        LOK_ASSERT(socket->waitForDisconnection(5s));
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::RunTest:
            {
                runTest();
                break;
            }
            case Phase::WaitDocClose:
                break;
            case Phase::PostCloseTest:
                TRANSITION_STATE(_phase, Phase::Done);
                passTest();
                break;
            case Phase::Done:
                break;
        }
    }
};

UnitBase* unit_create_wsd(void) { return new UnitCopyPasteWriter(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
