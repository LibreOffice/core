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
 * Test for HTTP crash scenarios and resilience.
 */

#include <config.h>

#include <sys/types.h>

#include <cstring>
#include <chrono>

#include <Poco/Dynamic/Var.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/InvalidCertificateHandler.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/PrivateKeyPassphraseHandler.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/Socket.h>
#include <Poco/Path.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <cppunit/extensions/HelperMacros.h>

#include <Common.hpp>
#include <UserMessages.hpp>
#include <common/Util.hpp>
#include <Protocol.hpp>
#include <test.hpp>
#include <helpers.hpp>
#include <KitPidHelpers.hpp>

using namespace std::literals;
using namespace helpers;

// The default is KIT_PID_TIMEOUT_MS, but we may have to wait for coolforkit to restart, which
// may take up to the default CHILD_SPAWN_TIMEOUT_MS which is larger than KIT_PID_TIMEOUT_MS
const std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(KIT_PID_TIMEOUT_MS + CHILD_SPAWN_TIMEOUT_MS);

/// Tests the HTTP WebSocket API of coolwsd. The server has to be started manually before running this test.
class HTTPCrashTest : public CPPUNIT_NS::TestFixture
{
    const Poco::URI _uri;
    Poco::Net::HTTPResponse _response;
    std::shared_ptr<SocketPoll> _socketPoll;

    CPPUNIT_TEST_SUITE(HTTPCrashTest);

    CPPUNIT_TEST(testBarren);
    CPPUNIT_TEST(testCrashKit);
    CPPUNIT_TEST(testRecoverAfterKitCrash);
    CPPUNIT_TEST(testCrashForkit);

    CPPUNIT_TEST_SUITE_END();

    void testBarren();
    void testCrashKit();
    void testRecoverAfterKitCrash();
    void testCrashForkit();

    void killDocKitProcesses(const std::string& testname);

public:
    HTTPCrashTest()
        : _uri(helpers::getTestServerURI())
        , _socketPoll(std::make_shared<SocketPoll>("HttpCrashPoll"))
    {
#if ENABLE_SSL
        Poco::Net::initializeSSL();
        // Just accept the certificate anyway for testing purposes
        Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> invalidCertHandler = new Poco::Net::AcceptCertificateHandler(false);
        Poco::Net::Context::Params sslParams;
        Poco::Net::Context::Ptr sslContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, sslParams);
        Poco::Net::SSLManager::instance().initializeClient(nullptr, std::move(invalidCertHandler), std::move(sslContext));
#endif
    }

    ~HTTPCrashTest()
    {
#if ENABLE_SSL
        Poco::Net::uninitializeSSL();
#endif
    }

    void setUp()
    {
        resetTestStartTime();
        waitForKitPidsReady("setUp");
        resetTestStartTime();
        _socketPoll->startThread();
    }

    void tearDown()
    {
        _socketPoll->joinThread();
        resetTestStartTime();
        waitForKitPidsReady("tearDown", timeoutMs);
        resetTestStartTime();
    }
};

void HTTPCrashTest::testBarren()
{
    // Kill all kit processes and try loading a document.
    const char* testname = "barren ";
    try
    {
        TST_LOG("Killing all kits");
        helpers::killAllKitProcesses(testname);

        // Do not wait for spare kit to start up here

        TST_LOG("Loading after kill.");
        // Load a document and get its status.
        std::shared_ptr<http::WebSocketSession> socket
            = loadDocAndGetSession(_socketPoll, "hello.odt", _uri, testname);

        sendTextFrame(socket, "status", testname);
        assertResponseString(socket, "status:", testname);

        socket->asyncShutdown();
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}

void HTTPCrashTest::testCrashKit()
{
    const char* testname = "crashKit ";
    try
    {
        TST_LOG("Loading document");
        std::shared_ptr<http::WebSocketSession> socket
            = loadDocAndGetSession(_socketPoll, "empty.odt", _uri, testname);

        TST_LOG("Allowing time for kit to connect to wsd to get cleanly killed");
        std::this_thread::sleep_for(1s);

        TST_LOG("Killing coolkit instances.");
        helpers::killAllKitProcesses(testname);

        // TST_LOG("Reading the error code from the socket.");
        //FIXME: implement in WebSocketSession.
        // std::string message;
        // const int statusCode = getErrorCode(socket, message, testname);
        // LOK_ASSERT_EQUAL(static_cast<int>(Poco::Net::WebSocket::WS_ENDPOINT_GOING_AWAY), statusCode);

        // respond close frame
        TST_LOG("Shutting down socket.");
        socket->asyncShutdown();

        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}

void HTTPCrashTest::testRecoverAfterKitCrash()
{
    const char* testname = "recoverAfterKitCrash ";
    try
    {
        TST_LOG("Loading document");
        std::shared_ptr<http::WebSocketSession> socket1
            = loadDocAndGetSession(_socketPoll, "empty.odt", _uri, testname);

        TST_LOG("Allowing time for kit to connect to wsd to get cleanly killed");
        std::this_thread::sleep_for(1s);

        TST_LOG("Killing coolkit instances.");
        killDocKitProcesses(testname);

        TST_LOG("Reconnect after kill.");
        std::shared_ptr<http::WebSocketSession> socket2 = loadDocAndGetSession(
            _socketPoll, "empty.odt", _uri, testname, /*isView=*/true, /*isAssert=*/false);

        sendTextFrame(socket2, "status", testname);
        assertResponseString(socket2, "status:", testname);

        socket2->asyncShutdown();
        socket1->asyncShutdown();

        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                           socket2->waitForDisconnection(5s));
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                           socket1->waitForDisconnection(5s));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}

void HTTPCrashTest::testCrashForkit()
{
    const char* testname = "crashForkit ";
    try
    {
        TST_LOG("Loading document");
        std::shared_ptr<http::WebSocketSession> socket
            = loadDocAndGetSession(_socketPoll, "empty.odt", _uri, testname);

        TST_LOG("Allowing time for kit to connect to wsd to get cleanly killed");
        std::this_thread::sleep_for(1s);

        TST_LOG("Killing forkit.");
        helpers::killPid(testname, getForKitPid());

        TST_LOG("Communicating after kill.");
        sendTextFrame(socket, "status", testname);
        assertResponseString(socket, "status:", testname);

        // respond close frame
        socket->asyncShutdown();
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));

        TST_LOG("Killing coolkit.");
        helpers::killAllKitProcesses(testname);

        // Forkit should restart
        waitForKitPidsReady(testname, timeoutMs);

        TST_LOG("Communicating after kill.");
        socket = loadDocAndGetSession(_socketPoll, "empty.odt", _uri, testname);
        sendTextFrame(socket, "status", testname);
        assertResponseString(socket, "status:", testname);

        socket->asyncShutdown();
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}

void HTTPCrashTest::killDocKitProcesses(const std::string& testname)
{
    for (pid_t pid : getDocKitPids())
    {
        helpers::killPid(testname, pid);
    }
    waitForKitPidsReady(testname);
}

CPPUNIT_TEST_SUITE_REGISTRATION(HTTPCrashTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
