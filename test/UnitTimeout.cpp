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
 * Unit test for timeout functionality.
 */

#include <config.h>

#include <common/Log.hpp>
#include <common/Unit.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>
#include <wsd/UserMessages.hpp>

#include <Poco/Util/LayeredConfiguration.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

class UnitTimeout : public UnitWSD
{
public:
    UnitTimeout()
        : UnitWSD("UnitTimeout")
    {
        setTimeout(1s);
    }

    virtual void timeout() override { passTest("Timed out as expected"); }
};

/// Base test suite class for timeout and connection limit using HTTP and WS sessions.
class UnitTimeoutBase0 : public UnitWSD
{
public:
    bool assertMessage(http::WebSocketSession& session, const std::string_view expectedPrefix,
                       const std::string_view expectedId, LOG_CAPTURE_CALLER_DECLARATION)
    {
        std::vector<char> res = session.poll(
            [&](const std::vector<char>& message) -> bool
            {
                const std::string_view msg(message.begin(), message.end());
                TST_LOG("Got WS response: " << msg);
                if (!msg.starts_with("error:"))
                {
                    if (expectedPrefix == "progress:")
                    {
                        LOK_ASSERT_EQUAL(true, COOLProtocol::matchPrefix(expectedPrefix, msg));
                        LOK_ASSERT(helpers::getProgressWithIdValue(msg, expectedId));
                        TST_LOG("Good WS response(0): " << msg);
                        return true;
                    }
                    else if (msg.find(expectedId) != std::string::npos)
                    {
                        // simple match
                        TST_LOG("Good WS response(1): " << msg);
                        return true;
                    }
                    else
                    {
                        const bool c = session.isConnected();
                        TST_LOG("Some WS response(2): " << msg << ", connected " << c);
                        return !c; // continue waiting for 'it' if still connected
                    }
                }
                else
                {
                    // check error message
                    LOK_ASSERT_EQUAL_STR(SERVICE_UNAVAILABLE_INTERNAL_ERROR, msg);

                    // close frame message
                    return true;
                }
            },
            std::chrono::seconds(10), testname);
        return !res.empty();
    }

    template <typename SessionType>
    bool pollDisconnected(std::chrono::microseconds timeout, SessionType& session)
    {
        std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point t1 = t0;
        while (t1 - t0 < timeout && session.isConnected())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            t1 = std::chrono::steady_clock::now();
        }
        return !session.isConnected();
    }

    std::shared_ptr<SocketPoll> createPoller(std::vector<std::shared_ptr<TerminatingPoll>>& pollers,
                                             bool useOwnPoller, bool pollerOnClientThread)
    {
        if (useOwnPoller)
        {
            auto poller = std::make_shared<TerminatingPoll>(testname);
            if (pollerOnClientThread)
            {
                poller->runOnClientThread();
            }
            else
            {
                poller->startThread();
            }

            pollers.push_back(poller);
            return poller;
        }

        return UnitBase::socketPoll();
    }

    void assertHttpResponse(const http::Session& session, const http::Response& response,
                            LOG_CAPTURE_CALLER_DECLARATION)
    {
        if (session.isConnected())
        {
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response.statusCode());
            LOK_ASSERT(http::Header::ConnectionToken::None ==
                       response.header().getConnectionToken());
            LOK_ASSERT(0 < response.header().getContentLength());
        }
        else
        {
            LOK_ASSERT_EQUAL(http::StatusCode::None, response.statusCode());
        }
    }

    static void shutdownSession(std::shared_ptr<http::Session>& session)
    {
        session->asyncShutdown();
    }

    static void shutdownSession(std::shared_ptr<http::WebSocketSession>& session)
    {
        session->shutdownWS();
    }

    template <typename SessionType>
    TestResult shutdownAndCleanup(std::vector<std::shared_ptr<SessionType>>& sessions,
                                  std::vector<std::shared_ptr<TerminatingPoll>>& socketPollers,
                                  size_t maxConnections, size_t connectionsCount,
                                  size_t connectionLimit, bool useOwnPoller,
                                  bool pollerOnClientThread, LOG_CAPTURE_CALLER_DECLARATION)
    {
        size_t connected = 0;
        for (size_t sockIdx = 0; sockIdx < connectionsCount; ++sockIdx)
        {
            std::shared_ptr<SessionType>& session = sessions[sockIdx];
            TST_LOG("SessionA " << sockIdx << ": connected " << session->isConnected());
            if (session->isConnected())
            {
                ++connected;
                shutdownSession(session);
            }

            if (useOwnPoller)
            {
                const std::shared_ptr<TerminatingPoll>& socketPoller = socketPollers[sockIdx];
                if (pollerOnClientThread)
                {
                    socketPoller->closeAllSockets();
                }
                else
                {
                    socketPoller->joinThread();
                }
            }
        }
        TST_LOG("Test: Connected: " << connected << " / " << connectionsCount << ", limit "
                                    << connectionLimit);
        LOK_ASSERT_MESSAGE("connected: " << connected << ", maxConnections: " << maxConnections,
                           maxConnections - 1 <= connected && connected <= maxConnections + 1);

        TST_LOG("Clearing Sessions: " << testname);
        sessions.clear();
        TST_LOG("Clearing Poller: " << testname);
        socketPollers.clear();
        // TCP Connection Count: Just an estimation, no locking on server side
        TST_LOG("TCP Connection Count: " << StreamSocket::getExternalConnectionCount() << " / "
                                         << net::Defaults.maxExtConnections);
        TST_LOG("Ending Test: " << testname);
        return TestResult::Ok;
    }

    UnitTimeoutBase0(const std::string& testname_)
        : UnitWSD(testname_)
    {
    }
};

/// Base test suite class for timeout and connection limit using HTTP and WS sessions.
class UnitTimeoutBase1 : public UnitTimeoutBase0
{
    const size_t _connectionLimit;
    const size_t _connectionCount;

public:
    TestResult testHttp();
    TestResult testWSPing();
    TestResult testWSDChatPing();

    UnitTimeoutBase1(const std::string& testname_, size_t connectionLimit, size_t connectionCount)
        : UnitTimeoutBase0(testname_)
        , _connectionLimit(connectionLimit)
        , _connectionCount(connectionCount)
    {
    }

    size_t connectionLimit() const { return _connectionLimit; }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitTimeoutBase1::testHttp()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    const size_t MaxConnections = std::min(_connectionCount, _connectionLimit);
    const std::string documentURL = "/favicon.ico";

    constexpr bool UseOwnPoller = true;
    constexpr bool PollerOnClientThread = true;
    std::vector<std::shared_ptr<TerminatingPoll>> socketPollers;
    std::vector<std::shared_ptr<http::Session>> sessions;

    try
    {
        for (size_t sockIdx = 0; sockIdx < _connectionCount; ++sockIdx)
        {
            std::shared_ptr<SocketPoll> socketPoller =
                createPoller(socketPollers, UseOwnPoller, PollerOnClientThread);

            std::shared_ptr<http::Session> session =
                http::Session::create(helpers::getTestServerURI());
            sessions.push_back(session);
            TST_LOG("Test: " << testname << '[' << sockIdx << "]: `" << documentURL << '`');
            http::Request request(documentURL, http::Request::VERB_GET);
            const std::shared_ptr<const http::Response> response =
                session->syncRequest(request, *socketPoller);
            TST_LOG("Response: " << response->header().toString());
            TST_LOG("Response size: " << testname << '[' << sockIdx << "]: `" << documentURL
                                      << "`: " << response->header().getContentLength());
            assertHttpResponse(*session, *response);
        }
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }

    return shutdownAndCleanup(sessions, socketPollers, MaxConnections, _connectionCount,
                              _connectionLimit, UseOwnPoller, PollerOnClientThread);
}

/// Test the native WebSocket control-frame ping/pong facility -> No Timeout!
UnitBase::TestResult UnitTimeoutBase1::testWSPing()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    const size_t maxConnections = std::min(_connectionCount, _connectionLimit);
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    constexpr bool UseOwnPoller = true;
    constexpr bool PollerOnClientThread = false;
    std::vector<std::shared_ptr<TerminatingPoll>> socketPollers;
    std::vector<std::shared_ptr<http::WebSocketSession>> sessions;

    size_t connected0 = 0;
    for (size_t sockIdx = 0; sockIdx < _connectionCount; ++sockIdx)
    {
        std::shared_ptr<SocketPoll> socketPoller =
            createPoller(socketPollers, UseOwnPoller, PollerOnClientThread);

        std::shared_ptr<http::WebSocketSession> session =
            http::WebSocketSession::create(helpers::getTestServerURI());
        sessions.push_back(session);
        TST_LOG("Test: " << testname << '[' << sockIdx << "]: `" << documentURL << '`');
        http::Request req(documentURL);
        session->asyncRequest(req, socketPoller);
        session->sendMessage("load url=" + documentURL);

        TST_LOG("Test: XX0 " << testname << '[' << sockIdx << "]: connected "
                             << session->isConnected());
        if (sockIdx < maxConnections)
        {
            LOK_ASSERT_EQUAL(true, session->isConnected());

            assertMessage(*session, "progress:", "find");
            assertMessage(*session, "progress:", "connect");
            assertMessage(*session, "progress:", "ready");

            TST_LOG("Test: XX1 " << testname << '[' << sockIdx << "]: connected "
                                 << session->isConnected());
            LOK_ASSERT_EQUAL(true, session->isConnected());
            ++connected0;
        }
        else
        {
            // Perform actual communication attempt, required to fail (disconnect)
            TST_LOG("Test: XX2 " << testname << '[' << sockIdx << "]: connected "
                                 << session->isConnected());
            bool comRes = false;
            if (session->isConnected())
            {
                comRes = assertMessage(*session, "progress:", "find");
                if (session->isConnected())
                    ++connected0;
            }
            TST_LOG("Test: XX3 " << testname << '[' << sockIdx << "]: connected "
                                 << session->isConnected() << '/' << connected0 << ", com "
                                 << comRes);
            LOK_ASSERT_EQUAL(false, comRes);
            LOK_ASSERT_EQUAL(false, session->isConnected());
        }
    }
    TST_LOG("Test: X01 Connected: " << connected0 << " / " << _connectionCount << ", limit "
                                    << _connectionLimit);

    return shutdownAndCleanup(sessions, socketPollers, maxConnections, _connectionCount,
                              _connectionLimit, UseOwnPoller, PollerOnClientThread);
}

/// Tests the WSD chat ping/pong facility, where client sends the ping.
/// See: https://github.com/CollaboraOnline/online/blob/master/wsd/protocol.txt/
UnitBase::TestResult UnitTimeoutBase1::testWSDChatPing()
{
    setTestname(__func__);
    TST_LOG("Starting Test: " << testname);

    const size_t maxConnections = std::min(_connectionCount, _connectionLimit);
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    constexpr bool UseOwnPoller = true;
    constexpr bool PollerOnClientThread = false;
    std::vector<std::shared_ptr<TerminatingPoll>> socketPollers;
    std::vector<std::shared_ptr<http::WebSocketSession>> sessions;

    for (size_t sockIdx = 0; sockIdx < _connectionCount; ++sockIdx)
    {
        std::shared_ptr<SocketPoll> socketPoller =
            createPoller(socketPollers, UseOwnPoller, PollerOnClientThread);

        std::shared_ptr<http::WebSocketSession> session =
            http::WebSocketSession::create(helpers::getTestServerURI());
        sessions.push_back(session);
        TST_LOG("Test: " << testname << '[' << sockIdx << "]: `" << documentURL << '`');
        http::Request req(documentURL);
        session->asyncRequest(req, socketPoller);
        session->sendMessage("load url=" + documentURL);

        TST_LOG("Test: XX0 " << testname << '[' << sockIdx << "]: connected "
                             << session->isConnected());
        if (sockIdx < maxConnections)
        {
            LOK_ASSERT_EQUAL(true, session->isConnected());

            assertMessage(*session, "progress:", "find");
            assertMessage(*session, "progress:", "connect");
            assertMessage(*session, "progress:", "ready");

            TST_LOG("Test: XX1 " << testname << '[' << sockIdx << "]: connected "
                                 << session->isConnected());
            // LOK_ASSERT_EQUAL(true, session->isConnected());
        }
        else
        {
            TST_LOG("Test: XX2 " << testname << '[' << sockIdx << "]: connected "
                                 << session->isConnected());
            // LOK_ASSERT_EQUAL(false, session->isConnected());
        }
    }

    for (size_t sockIdx = 0; sockIdx < _connectionCount; ++sockIdx)
    {
        const std::shared_ptr<http::WebSocketSession>& wsSession = sessions[sockIdx];
        TST_LOG("Test: XX3a " << testname << '[' << sockIdx << "]: connected "
                              << wsSession->isConnected());
        if (wsSession->isConnected())
        {
            wsSession->sendMessage("ping");
            TST_LOG("Test: XX3b " << testname << '[' << sockIdx << "]: connected "
                                  << wsSession->isConnected());
            assertMessage(*wsSession, "", "pong");
            TST_LOG("Test: XX3c " << testname << '[' << sockIdx << "]: connected "
                                  << wsSession->isConnected());
        }
    }

    return shutdownAndCleanup(sessions, socketPollers, maxConnections, _connectionCount,
                              _connectionLimit, UseOwnPoller, PollerOnClientThread);
}

void UnitTimeoutBase1::invokeWSDTest()
{
    UnitBase::TestResult result;

    result = testHttp();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testWSPing();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testWSDChatPing();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

/// Test suite class for inactivity across WS and Http.
class UnitTimeoutInactivity : public UnitTimeoutBase0
{
    TestResult testHttp(bool forceInactivityTO);
    TestResult testWS(bool forceInactivityTO);

    void configure(Poco::Util::LayeredConfiguration& /* config */) override
    {
        // Set every net::Defaults field this suite cares about: tests share global state,
        // so each test must establish the values it needs rather than inherit them.
        net::Defaults.inactivityTimeout = 360ms;
        net::Defaults.maxExtConnections = net::DefaultMaxExtConnections;
    }

public:
    UnitTimeoutInactivity()
        : UnitTimeoutBase0("UnitTimeoutInactivity")
    {
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitTimeoutInactivity::testHttp(bool forceInactivityTO)
{
    setTestname(__func__);
    TST_LOG("Starting Test: forceInactivityTO " << forceInactivityTO);

    const std::string documentURL = "/favicon.ico";

    constexpr bool UseOwnPoller = true;
    constexpr bool PollerOnClientThread = true;
    std::shared_ptr<SocketPoll> socketPoller;
    std::shared_ptr<http::Session> session;
    std::vector<std::shared_ptr<TerminatingPoll>> socketPollers;

    try
    {
        socketPoller = createPoller(socketPollers, UseOwnPoller, PollerOnClientThread);

        session = http::Session::create(helpers::getTestServerURI());
        {
            TST_LOG("Test Req1: `" << documentURL << "`");
            http::Request request(documentURL, http::Request::VERB_GET);
            const std::shared_ptr<const http::Response> response =
                session->syncRequest(request, *socketPoller);
            TST_LOG("Response1: " << response->header().toString());
            TST_LOG("Response1 size: `" << documentURL
                                        << "`: " << response->header().getContentLength());
            assertHttpResponse(*session, *response);
        }

        if (session->isConnected())
        {
            if (forceInactivityTO)
            {
                std::this_thread::sleep_for(net::Defaults.inactivityTimeout * 2);
            }

            TST_LOG("Test Req2: `" << documentURL << "`");
            http::Request request(documentURL, http::Request::VERB_GET);
            const std::shared_ptr<const http::Response> response =
                session->syncRequest(request, *socketPoller);
            TST_LOG("Response2: " << response->header().toString());
            TST_LOG("Response2 size: `" << documentURL
                                        << "`: " << response->header().getContentLength());
            assertHttpResponse(*session, *response);
        }
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }

    size_t connected = 0;
    {
        TST_LOG("SessionA " << ": connected " << session->isConnected());
        if (session->isConnected())
        {
            ++connected;
            session->asyncShutdown();
        }

        if constexpr (UseOwnPoller)
        {
            if (PollerOnClientThread)
            {
                socketPoller->closeAllSockets();
            }
            else
            {
                socketPoller->joinThread();
            }
        }
    }
    TST_LOG("Test: X01 Connected: " << connected);
    if (forceInactivityTO)
    {
        LOK_ASSERT_EQUAL(size_t(0), connected);
    }
    else
    {
        LOK_ASSERT_EQUAL(size_t(1), connected);
    }

    TST_LOG("Ending Test: " << testname);
    return TestResult::Ok;
}

UnitBase::TestResult UnitTimeoutInactivity::testWS(bool forceInactivityTO)
{
    setTestname(__func__);
    TST_LOG("Starting Test: forceInactivityTO " << forceInactivityTO);

    std::shared_ptr<http::WebSocketSession> session;

    try
    {
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

        // NOTE: Do not replace with wrappers. This has to be explicit.
        session = http::WebSocketSession::create(helpers::getTestServerURI());
        http::Request req(documentURL);
        session->asyncRequest(req, socketPoll());
        session->sendMessage("load url=" + documentURL);
        TST_LOG("Test: XX0: connected " << session->isConnected());

        assertMessage(*session, "progress:", "find");
        assertMessage(*session, "progress:", "connect");
        assertMessage(*session, "progress:", "ready");
        TST_LOG("Test: XX1: connected " << session->isConnected());
        LOK_ASSERT_EQUAL(true, session->isConnected());

        if (forceInactivityTO)
        {
            pollDisconnected(net::Defaults.inactivityTimeout * 20, *session);
        }
        TST_LOG("Test: XX2: connected " << session->isConnected());
        session->sendMessage("ping");
        TST_LOG("Test: XX3b: connected " << session->isConnected());
        assertMessage(*session, "", "pong");
        TST_LOG("Test: XX3c: connected " << session->isConnected());
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }

    size_t connected = 0;
    TST_LOG("SessionA " << ": connected " << session->isConnected());
    if (session->isConnected())
    {
        ++connected;
        session->shutdownWS();
    }

    TST_LOG("Test: X01 Connected: " << connected);
    if (forceInactivityTO)
    {
        LOK_ASSERT_EQUAL(0UL, connected);
    }
    else
    {
        LOK_ASSERT_EQUAL(1UL, connected);
    }

    TST_LOG("Ending Test: " << testname);
    return TestResult::Ok;
}

void UnitTimeoutInactivity::invokeWSDTest()
{
    UnitBase::TestResult result;

    result = testHttp(/*forceInactivityTO=*/false);
    if (result != TestResult::Ok)
        exitTest(result);
    result = testHttp(/*forceInactivityTO=*/true);
    if (result != TestResult::Ok)
        exitTest(result);

    result = testWS(/*forceInactivityTO=*/false);
    if (result != TestResult::Ok)
        exitTest(result);
    result = testWS(/*forceInactivityTO=*/true);
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

/// Test suite class for connection limit (limited) using HTTP and WS sessions.
class UnitTimeoutConnections : public UnitTimeoutBase1
{
    void configure(Poco::Util::LayeredConfiguration& /* config */) override
    {
        // Set every net::Defaults field this suite cares about: tests share global state,
        // so each test must establish the values it needs rather than inherit them.
        net::Defaults.inactivityTimeout = net::DefaultInactivityTimeout;
        net::Defaults.maxExtConnections = connectionLimit();
    }

public:
    UnitTimeoutConnections(size_t connectionLimit, size_t connectionCount)
        : UnitTimeoutBase1("UnitTimeoutConnections", connectionLimit, connectionCount)
    {
    }
};

/// Test suite class for connection limit (no limits) using HTTP and WS sessions.
class UnitTimeoutNone : public UnitTimeoutBase1
{
    void configure(Poco::Util::LayeredConfiguration& /* config */) override
    {
        // Set every net::Defaults field this suite cares about: tests share global state,
        // so each test must establish the values it needs rather than inherit them.
        net::Defaults.inactivityTimeout = net::DefaultInactivityTimeout;
        net::Defaults.maxExtConnections = net::DefaultMaxExtConnections;
    }

public:
    UnitTimeoutNone(size_t connectionLimit, size_t connectionCount)
        : UnitTimeoutBase1("UnitTimeoutNone", connectionLimit, connectionCount)
    {
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* []
    {
        new UnitTimeout(), new UnitTimeoutInactivity(), new UnitTimeoutNone(9999, 9),
            new UnitTimeoutConnections(5, 9), nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
