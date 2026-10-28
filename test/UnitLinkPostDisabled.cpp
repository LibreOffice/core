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

/*
 * Unit test for the links endpoint of a server with remote links disabled. The test runs on
 * its own because the server reads remote_links.enable once, at startup.
 */

#include <config.h>

#include <common/Uri.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>

#include <WopiTestServer.hpp>

#include <net/HttpRequest.hpp>

#include <atomic>
#include <memory>
#include <string>
#include <thread>

/// With remote links disabled in the configuration, the links endpoint refuses the POST and
/// the OPTIONS a browser sends before it alike, with a status a page of any origin can read.
class UnitLinkPostDisabled : public WopiTestServer
{
    STATE_ENUM(Phase, Start, Posting, Done) _phase;

    std::thread _postThread;
    /// Whether both requests were answered and checked.
    std::atomic<bool> _refused = false;

    /// Sends the given method to the links endpoint, naming a document, and checks that the
    /// answer is a refusal a page of any origin can read.
    void checkRefused(const std::string& verb)
    {
        http::Request request(
            "/cool/links?WOPISrc=" + Uri::encode(getWopiHostURI() + "/wopi/files/1"), verb);
        if (verb == http::Request::VERB_POST)
            request.setBody("{\"Nonce\":\"none\",\"Links\":[]}", "application/json");

        auto session = http::Session::create(helpers::getTestServerURI());
        session->setTimeout(std::chrono::seconds(10));
        const std::shared_ptr<const http::Response> response = session->syncRequest(request);
        LOK_ASSERT_MESSAGE("The links endpoint must answer " + verb, response);
        if (!response)
            return;

        LOK_ASSERT_EQUAL(http::StatusCode::Forbidden, response->statusCode());
        LOK_ASSERT_EQUAL(std::string("*"), response->header().get("Access-Control-Allow-Origin"));
    }

public:
    UnitLinkPostDisabled()
        : WopiTestServer("UnitLinkPostDisabled")
        , _phase(Phase::Start)
    {
    }

    ~UnitLinkPostDisabled()
    {
        if (_postThread.joinable())
            _postThread.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", false);
    }

    void invokeWSDTest() override
    {
        if (_phase == Phase::Start)
        {
            TRANSITION_STATE(_phase, Phase::Posting);
            _postThread = std::thread(
                [this]
                {
                    checkRefused("OPTIONS");
                    checkRefused(http::Request::VERB_POST);
                    _refused = true;
                });
        }
        else if (_phase == Phase::Posting && _refused)
        {
            TRANSITION_STATE(_phase, Phase::Done);
            passTest("A disabled links endpoint refuses the preflight and the POST alike");
        }
    }
};

UnitBase* unit_create_wsd(void) { return new UnitLinkPostDisabled(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
