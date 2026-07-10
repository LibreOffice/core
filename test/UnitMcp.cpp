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
 * End to end test for the Model Context Protocol endpoint: the handshake and the requests that are
 * turned away.
 */

#include <config.h>

#include <string>
#include <thread>

#include <Common.hpp>
#include <Protocol.hpp>
#include <Unit.hpp>
#include <helpers.hpp>

#include <Poco/Util/LayeredConfiguration.h>

using namespace std::literals;

namespace
{
const std::string ApiKey = "secret123";

bool contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}
}

/// Inside the WSD process.
class UnitMcp : public UnitWSD
{
    bool _workerStarted;
    std::thread _worker;

public:
    UnitMcp()
        : UnitWSD("UnitMcp")
        , _workerStarted(false)
    {
        setTimeout(1h);
    }

    ~UnitMcp()
    {
        if (_worker.joinable())
            _worker.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        config.setBool("ssl.enable", true);
        config.setBool("storage.filesystem[@allow]", false);

        // Switch the endpoint on and give it a key to require.
        config.setBool("ai.mcp.enable", true);
        config.setString("ai.mcp.api_key", ApiKey);
    }

    /// Send jsonBody to the message address path, with bearer as the Authorization header, or with
    /// no Authorization header when bearer is empty.
    std::shared_ptr<const http::Response>
    mcpCall(const std::shared_ptr<http::Session>& session, const std::string& jsonBody,
            const std::string& bearer = "Bearer " + ApiKey, const std::string& path = "/cool/mcp")
    {
        http::Request request(path, http::Request::VERB_POST);
        if (!bearer.empty())
            request.set("Authorization", bearer);
        request.setBody(jsonBody, "application/json");
        return session->syncRequest(request);
    }

    std::shared_ptr<const http::Response> mcpGet(const std::shared_ptr<http::Session>& session,
                                                 const std::string& path,
                                                 const std::string& bearer = "Bearer " + ApiKey)
    {
        http::Request request(path, http::Request::VERB_GET);
        request.set("Authorization", bearer);
        return session->syncRequest(request);
    }

    void runTest(const std::shared_ptr<http::Session>& session)
    {
        // The handshake negotiates a supported protocol version and returns the numeric request id
        // unchanged.
        {
            auto response = mcpCall(
                session, R"({"jsonrpc":"2.0","id":1,"method":"initialize",)"
                         R"("params":{"protocolVersion":"2025-06-18"}})");
            LOK_ASSERT(response->statusCode() == http::StatusCode::OK);
            LOK_ASSERT(contains(response->getBody(), R"("protocolVersion":"2025-06-18")"));
            LOK_ASSERT(contains(response->getBody(), R"("id":1)"));
        }

        const std::string initialize = R"({"jsonrpc":"2.0","id":30,"method":"initialize"})";

        // A request with the key from a web page, which carries an Origin header, is forbidden.
        {
            http::Request request("/cool/mcp", http::Request::VERB_POST);
            request.set("Authorization", "Bearer " + ApiKey);
            request.set("Origin", "https://example.com");
            request.setBody(initialize, "application/json");
            auto response = session->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::Forbidden, response->statusCode());
        }

        // A message without the key is refused and told to send a bearer token: with no
        // Authorization header, with the scheme and no key, with the key one character short or one
        // character long, and with the key under another scheme.
        const std::string shortKey = ApiKey.substr(0, ApiKey.size() - 1);
        for (const std::string& authorization :
             { std::string(), std::string("Bearer"), "Bearer " + shortKey, "Bearer " + ApiKey + 'x',
               "Basic " + ApiKey })
        {
            TST_LOG("Sending a message with authorization [" << authorization << ']');
            auto response = mcpCall(session, initialize, authorization);
            LOK_ASSERT_EQUAL(http::StatusCode::Unauthorized, response->statusCode());
            LOK_ASSERT_EQUAL_STR("Bearer", response->get("WWW-Authenticate"));
        }

        // The address written in hex is the same endpoint: it is refused without the key and
        // answered with it.
        {
            auto refused = mcpCall(session, initialize, std::string(), "/cool/0x6d6370");
            LOK_ASSERT_EQUAL(http::StatusCode::Unauthorized, refused->statusCode());
            LOK_ASSERT_EQUAL_STR("Bearer", refused->get("WWW-Authenticate"));

            auto answered = mcpCall(session, initialize, "Bearer " + ApiKey, "/cool/0x6d6370");
            LOK_ASSERT_EQUAL(http::StatusCode::OK, answered->statusCode());
            LOK_ASSERT(contains(answered->getBody(), R"("serverInfo")"));
        }

        // A POST must state the length of its body. One sent in chunks, or with no Content-Length,
        // is refused.
        {
            http::Request chunked("/cool/mcp", http::Request::VERB_POST);
            chunked.set("Authorization", "Bearer " + ApiKey);
            chunked.set("Transfer-Encoding", "chunked");
            auto response = session->syncRequest(chunked);
            LOK_ASSERT_EQUAL(http::StatusCode::LengthRequired, response->statusCode());

            http::Request unstated("/cool/mcp", http::Request::VERB_POST);
            unstated.set("Authorization", "Bearer " + ApiKey);
            response = session->syncRequest(unstated);
            LOK_ASSERT_EQUAL(http::StatusCode::LengthRequired, response->statusCode());
        }

        // The message address takes only POST.
        {
            auto response = mcpGet(session, "/cool/mcp");
            LOK_ASSERT_EQUAL(http::StatusCode::MethodNotAllowed, response->statusCode());
            LOK_ASSERT_EQUAL_STR("POST", response->get("Allow"));
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
                    LOG_TST("UnitMcp failed: " << exc.what());
                    exitTest(TestResult::Failed);
                }
            });
    }
};

UnitBase* unit_create_wsd(void) { return new UnitMcp(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
