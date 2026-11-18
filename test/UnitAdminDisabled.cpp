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
 * Unit test for a server with the admin console disabled. The test runs on its own because the
 * server reads admin_console.enable once, at startup.
 */

#include <config.h>

#include <common/Unit.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>

#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <memory>
#include <string>

/// With the admin console disabled, the console pages stay closed, while the metrics endpoint
/// still answers an authenticated caller and the script of the Options dialog is still served.
class UnitAdminDisabled : public UnitWSD
{
    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);
        config.setBool("admin_console.enable", false);
    }

    /// Sends a GET for the given path, with the test admin credentials when asked, and returns
    /// the status of the answer.
    http::StatusCode get(const std::string& path, bool withCredentials)
    {
        std::shared_ptr<http::Session> session = http::Session::create(helpers::getTestServerURI());
        std::shared_ptr<TerminatingPoll> socketPoller = std::make_shared<TerminatingPoll>(testname);
        socketPoller->runOnClientThread();

        http::Request request(path, http::Request::VERB_GET);
        if (withCredentials)
            request.setBasicAuth("admin", "admin");

        // A refusal closes the connection without a Content-Length, so the response can end in
        // the error state with its status line already parsed. Only the status is compared.
        const std::shared_ptr<const http::Response> response =
            session->syncRequest(request, *socketPoller);
        TST_LOG("GET " << path << ": " << static_cast<int>(response->statusCode()));
        return response->statusCode();
    }

public:
    UnitAdminDisabled()
        : UnitWSD("UnitAdminDisabled")
    {
    }

    void invokeWSDTest() override
    {
        LOK_ASSERT_EQUAL(http::StatusCode::OK, get("/cool/getMetrics", true));
        LOK_ASSERT_EQUAL(http::StatusCode::Unauthorized, get("/cool/getMetrics", false));
        LOK_ASSERT_EQUAL(http::StatusCode::OK, get("/browser/dist/admin-bundle.js", false));
        LOK_ASSERT_EQUAL(http::StatusCode::Forbidden,
                         get("/browser/dist/admin/admin.html", true));

        passTest("Metrics and the Options dialog script work with the admin console disabled");
    }
};

UnitBase* unit_create_wsd(void) { return new UnitAdminDisabled(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
