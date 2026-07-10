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
 * The Model Context Protocol endpoint served under a service root: the endpoint answers at the
 * address with the root in front, and other requests whose address mentions it are served as
 * before.
 */

#include <config.h>

#include <common/Unit.hpp>
#include <net/HttpRequest.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/Util/LayeredConfiguration.h>

#include <chrono>
#include <exception>
#include <memory>
#include <string>
#include <thread>

using namespace std::literals;

namespace
{
const std::string ApiKey = "secret123";
const std::string ServiceRoot = "/collabora";
}

class UnitMcpServiceRoot : public UnitWSD
{
    bool _workerStarted;
    std::thread _worker;

public:
    UnitMcpServiceRoot()
        : UnitWSD("UnitMcpServiceRoot")
        , _workerStarted(false)
    {
        setTimeout(5min);
    }

    ~UnitMcpServiceRoot()
    {
        if (_worker.joinable())
            _worker.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        config.setString("net.service_root", ServiceRoot);
        config.setBool("ai.mcp.enable", true);
        config.setString("ai.mcp.api_key", ApiKey);
    }

    void runTest(const std::shared_ptr<http::Session>& session)
    {
        // The handshake at the endpoint under the service root is answered.
        {
            http::Request request(ServiceRoot + "/cool/mcp", http::Request::VERB_POST);
            request.set("Authorization", "Bearer " + ApiKey);
            request.setBody(R"({"jsonrpc":"2.0","id":1,"method":"initialize",)"
                            R"("params":{"protocolVersion":"2025-06-18"}})",
                            "application/json");
            auto response = session->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusCode());
            LOK_ASSERT(response->getBody().find(R"("protocolVersion":"2025-06-18")") !=
                       std::string::npos);
        }

        // A request for another service whose query mentions the endpoint is served as usual.
        {
            http::Request request(ServiceRoot + "/hosting/discovery?from=/mcp",
                                  http::Request::VERB_GET);
            auto response = session->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusCode());
        }
    }

    void invokeWSDTest() override
    {
        if (_workerStarted)
            return;
        _workerStarted = true;

        _worker = std::thread(
            [this]
            {
                try
                {
                    auto session = http::Session::create(helpers::getTestServerURI());
                    session->setTimeout(std::chrono::seconds(30));
                    runTest(session);
                    exitTest(TestResult::Ok);
                }
                catch (const std::exception& exc)
                {
                    LOG_TST("UnitMcpServiceRoot failed: " << exc.what());
                    exitTest(TestResult::Failed);
                }
            });
    }
};

UnitBase* unit_create_wsd(void) { return new UnitMcpServiceRoot(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
