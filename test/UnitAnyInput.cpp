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

/*
 * Unit test for handling arbitrary input events.
 */

#include <config.h>

#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <kit/Kit.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/URI.h>

#include <memory>
#include <string>

/// Tests the anyInput callback.
class UnitAnyInput : public UnitWSD
{
public:
    UnitAnyInput();

    void invokeWSDTest() override;
};

UnitAnyInput::UnitAnyInput()
    : UnitWSD("UnitAnyInput")
{
    setHasKitHooks();
}

void UnitAnyInput::invokeWSDTest()
{
    // Given a document of 3 pages:
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("3pages.odt", documentPath, documentURL, testname);
    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("AnyInputPoll");
    socketPoll->startThread();
    std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);
    helpers::sendTextFrame(socket, "clientvisiblearea x=0 y=0 width=12240 height=15840", testname);

    // When inserting a page break (Ctrl+Enter) and instantly asking for the state of the layout:
    helpers::sendTextFrame(socket, "key type=input char=0 key=9472", testname);

    // Then make sure some of the pages are still invalid:
    std::string content = helpers::assertResponseString(socket, "commandvalues:", testname);
    LOK_ASSERT(!content.empty());
    Poco::JSON::Object::Ptr commandValues = new Poco::JSON::Object();
    LOK_ASSERT(JsonUtil::parseJSON(content, commandValues));
    auto values = commandValues->get("commandValues").extract<Poco::JSON::Object::Ptr>();
    auto pages = values->get("pages").extract<Poco::JSON::Array::Ptr>();
    bool hasInvalidPage = false;
    for (const auto& child : *pages)
    {
        const auto& page = child.extract<Poco::JSON::Object::Ptr>();
        bool isInvalidContent{};
        JsonUtil::findJSONValue(page, "isInvalidContent", isInvalidContent);
        if (isInvalidContent)
        {
            hasInvalidPage = true;
        }
    }
    // Without the accompanying fix in place, this test would have failed, not only the visible area
    // but all pages were valid by the time we could send our first tile request.
    LOK_ASSERT(hasInvalidPage);
    exitTest(TestResult::Ok);
}

class UnitKitAnyInput : public UnitKit
{
    bool _keyInput = false;
    Session* _session = nullptr;
public:
    UnitKitAnyInput() : UnitKit("UnitKitAnyInput")
    {
    }

    void preKitPollCallback() override
    {
        if (!_keyInput || !_session)
        {
            return;
        }

        _keyInput = false;
        std::string message("commandvalues command=.uno:Layout");
        std::vector<char> messageVec(message.begin(), message.end());
        _session->handleMessage(messageVec);
        return;
    }

    void postKitSessionCreated(Session* session) override
    {
        _session = session;
    }

    bool filterKitMessage(WebSocketHandler *, std::string& message ) override
    {
        if (message.find("key type=input") != std::string::npos)
        {
            _keyInput = true;
        }
        return false;
    }
};

UnitBase* unit_create_wsd(void) { return new UnitAnyInput(); }

UnitBase *unit_create_kit(void) { return new UnitKitAnyInput(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
