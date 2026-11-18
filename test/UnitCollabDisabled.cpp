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

#include <chrono>
#include <memory>
#include <string>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>

#include <WebSocketSession.hpp>
#include <WopiTestServer.hpp>
#include <common/Uri.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>
#include <net/HttpRequest.hpp>

// Without client document loading enabled in the configuration, the server says so in its
// capabilities and refuses every /co/collab request:
class UnitCollabDisabled: public WopiTestServer {
public:
    UnitCollabDisabled(): WopiTestServer("UnitCollabDisabled") {}

    void invokeWSDTest() override {
        if (done_) {
            return;
        }
        done_ = true;

        // The capabilities say that client document loading is not available:
        {
            http::Request request("/hosting/capabilities");
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            Poco::JSON::Object::Ptr const capabilities = Poco::JSON::Parser()
                .parse(response->getBody()).extract<Poco::JSON::Object::Ptr>();
            LOK_ASSERT_EQUAL(
                false, capabilities->getValue<bool>("hasClientDocumentLoadingSupport"));
        }

        // A fetch is refused before its token is looked at (an unknown token would get
        // Unauthorized):
        {
            http::Request request("/co/collab/fetch?token=anything");
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::BadRequest, response->statusLine().statusCode());
        }

        // The WebSocket closes without a user list:
        {
            Poco::URI const wopiURL(
                getWopiHostURI()
                + "/wopi/files/0?access_token=anything&testname=UnitCollabDisabled");
            std::shared_ptr<http::WebSocketSession> const ws
                = http::WebSocketSession::create(helpers::getTestServerURI());
            http::Request request("/co/collab?WOPISrc=" + Uri::encode(wopiURL.toString()));
            LOK_ASSERT(ws->asyncRequest(request, socketPoll()));
            ws->sendMessage("access_token anything");
            LOK_ASSERT(ws->waitForDisconnection(std::chrono::seconds(10)));
            LOK_ASSERT(
                ws->waitForMessage("{\"type\":\"user_list\"", std::chrono::milliseconds(1),
                                   "collab")
                    .empty());
        }

        passTest("Client document loading is off by default");
    }

private:
    bool done_ = false;
};

UnitBase* unit_create_wsd(void) { return new UnitCollabDisabled(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
