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
 * The limit on the Model Context Protocol tool calls that run at one time. With the one call
 * allowed under way, another call is told that the server is busy, and once the first call has
 * finished a new call runs.
 */

#include <config.h>

#include <common/Message.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/Util/LayeredConfiguration.h>

#include <atomic>
#include <chrono>
#include <exception>
#include <future>
#include <memory>
#include <string>
#include <thread>

using namespace std::literals;

namespace
{
const std::string ApiKey = "secret123";

/// The name of the document whose converted result the test holds back until it lets it go.
const std::string HeldName = "heldcall";

/// The longest the test waits for one step of the server.
constexpr std::chrono::seconds StepTimeout(60);
}

class UnitMcpBusy : public UnitWSD
{
    bool _workerStarted;
    std::thread _worker;

    /// Set when the result of the held call has reached this process and is being held.
    std::promise<void> _resultHeld;
    /// Set by the test to let the held result go on.
    std::promise<void> _releaseResult;
    std::shared_future<void> _resultReleased;
    std::atomic<bool> _holding;

    /// Set when the document of the held call is closed and gone.
    std::promise<void> _heldDocumentGone;
    std::atomic<bool> _heldDocumentClosed;

public:
    UnitMcpBusy()
        : UnitWSD("UnitMcpBusy")
        , _workerStarted(false)
        , _resultReleased(_releaseResult.get_future().share())
        , _holding(false)
        , _heldDocumentClosed(false)
    {
        setTimeout(5min);
    }

    ~UnitMcpBusy()
    {
        if (_worker.joinable())
            _worker.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        config.setBool("ai.mcp.enable", true);
        config.setString("ai.mcp.api_key", ApiKey);
        config.setUInt("ai.mcp.max_running_tools", 1);
    }

    /// The converted result of the held call waits here, on the thread of its document, until the
    /// test lets it go. Its call keeps running meanwhile.
    bool onFilterLOKitMessage(const std::shared_ptr<Message>& message) override
    {
        if (message->contains("saveas:") && message->contains(HeldName) && !_holding.exchange(true))
        {
            TST_LOG("Holding the result [" << message->abbr() << ']');
            _resultHeld.set_value();
            if (_resultReleased.wait_for(StepTimeout) != std::future_status::ready)
                TST_LOG("The held result was not let go in time");
        }

        return false;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        if (docKey.find(HeldName) != std::string::npos && !_heldDocumentClosed.exchange(true))
            _heldDocumentGone.set_value();
    }

    static std::shared_ptr<const http::Response>
    toolCall(const std::shared_ptr<http::Session>& session, const std::string& body)
    {
        http::Request request("/cool/mcp", http::Request::VERB_POST);
        request.set("Authorization", "Bearer " + ApiKey);
        request.setBody(body, "application/json");
        return session->syncRequest(request);
    }

    static std::shared_ptr<http::Session> makeSession()
    {
        auto session = http::Session::create(helpers::getTestServerURI());
        session->setTimeout(StepTimeout);
        return session;
    }

    void runTest()
    {
        const std::string listDesigns = R"({"jsonrpc":"2.0","id":2,"method":"tools/call",)"
                                        R"("params":{"name":"list_design_templates"}})";

        // The one call allowed runs on its own connection, and its result is held back.
        std::shared_ptr<const http::Response> heldResponse;
        std::thread heldCall(
            [&heldResponse]
            {
                try
                {
                    const std::string data = Util::base64Encode(std::string("hello world"));
                    heldResponse = toolCall(
                        makeSession(), R"({"jsonrpc":"2.0","id":1,"method":"tools/call","params":)"
                                       R"({"name":"convert_document","arguments":{"data":")" +
                                           data + R"(","filename":")" + HeldName +
                                           R"(.txt","format":"pdf"}}})");
                }
                catch (const std::exception& exc)
                {
                    LOG_TST("The first call failed: " << exc.what());
                }
            });

        const bool held =
            _resultHeld.get_future().wait_for(StepTimeout) == std::future_status::ready;

        // While it runs, another call is told the server is busy.
        std::shared_ptr<const http::Response> busy;
        if (held)
            busy = toolCall(makeSession(), listDesigns);

        _releaseResult.set_value();
        heldCall.join();

        LOK_ASSERT_MESSAGE("the result of the first call reached the server", held);
        LOK_ASSERT_EQUAL(http::StatusCode::OK, busy->statusCode());
        LOK_ASSERT(busy->getBody().find(R"("isError":true)") != std::string::npos);
        LOK_ASSERT(busy->getBody().find("The server is busy with other tool calls.") !=
                   std::string::npos);

        // The first call finishes with its document.
        LOK_ASSERT(heldResponse);
        LOK_ASSERT_EQUAL(http::StatusCode::OK, heldResponse->statusCode());
        LOK_ASSERT(heldResponse->getBody().find(R"("isError":false)") != std::string::npos);

        // Once the first call and its document are gone, a new call runs.
        LOK_ASSERT(_heldDocumentGone.get_future().wait_for(StepTimeout) ==
                   std::future_status::ready);
        auto again = toolCall(makeSession(), listDesigns);
        LOK_ASSERT_EQUAL(http::StatusCode::OK, again->statusCode());
        LOK_ASSERT(again->getBody().find(R"("isError":false)") != std::string::npos);
        LOK_ASSERT(again->getBody().find("designs") != std::string::npos);
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
                    runTest();
                    exitTest(TestResult::Ok);
                }
                catch (const std::exception& exc)
                {
                    LOG_TST("UnitMcpBusy failed: " << exc.what());
                    exitTest(TestResult::Failed);
                }
            });
    }
};

UnitBase* unit_create_wsd(void) { return new UnitMcpBusy(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
