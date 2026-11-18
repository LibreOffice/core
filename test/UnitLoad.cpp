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
 * Unit test for document loading functionality.
 */

#include <config.h>

#include <common/Util.hpp>
#include <Unit.hpp>
#include <WebSocketSession.hpp>
#include <helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/Exception.h>
#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <chrono>
#include <memory>
#include <ostream>
#include <string>

using namespace std::literals;

namespace
{
void loadDoc(const std::string& documentURL, const std::string& testname)
{
    try
    {
        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname + "Poll");
        socketPoll->startThread();

        // Load a document and wait for the status.
        // Don't replace with helpers, so we catch status.
        Poco::URI uri(helpers::getTestServerURI());
        std::shared_ptr<http::WebSocketSession> socket =
            helpers::connectLOKit(socketPoll, uri, documentURL, testname);

        helpers::sendTextFrame(socket, "load url=" + documentURL, testname);

        helpers::assertResponseString(socket, "status:", testname);
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}
}

/// Test suite for loading.
class UnitLoad : public UnitWSD
{
    TestResult testConnectNoLoad();
    TestResult testLoadSimple();
    TestResult testBadLoad();
    TestResult testExcelLoad();
    TestResult testReload();
    TestResult testLoad();
    TestResult testLoadUserPrivateInfo();
    TestResult testLoadSessionRace();

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        config.setBool("ssl.enable", true);
    }

public:
    UnitLoad()
        : UnitWSD("UnitLoad")
    {
        setTimeout(60s);
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitLoad::testConnectNoLoad()
{
    const char* testname1 = "connectNoLoad-1 ";
    const char* testname2 = "connectNoLoad-2 ";
    const char* testname3 = "connectNoLoad-3 ";

    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, "connectNoLoad ");

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname + "Poll");
    socketPoll->startThread();

    Poco::URI uri(helpers::getTestServerURI());

    // Connect and disconnect without loading.
    TST_LOG_NAME(testname1, "Connecting first to disconnect without loading.");
    std::shared_ptr<http::WebSocketSession> socket =
        helpers::connectLOKit(socketPoll, uri, documentURL, testname1);
    LOK_ASSERT_MESSAGE("Failed to connect.", socket);
    TST_LOG_NAME(testname1, "Disconnecting first.");
    socket.reset();

    sleep(1); // paranoia.

    // Connect and load first view.
    TST_LOG_NAME(testname2, "Connecting second to load first view.");
    std::shared_ptr<http::WebSocketSession> socket1 =
        helpers::connectLOKit(socketPoll, uri, documentURL, testname2);
    LOK_ASSERT_MESSAGE("Failed to connect.", socket1);
    helpers::sendTextFrame(socket1, "load url=" + documentURL, testname2);
    LOK_ASSERT_MESSAGE("cannot load the document " + documentURL,
                           helpers::isDocumentLoaded(socket1, testname2));

    // Connect but don't load second view.
    TST_LOG_NAME(testname3, "Connecting third to disconnect without loading.");
    std::shared_ptr<http::WebSocketSession> socket2 =
        helpers::connectLOKit(socketPoll, uri, documentURL, testname3);
    LOK_ASSERT_MESSAGE("Failed to connect.", socket2);
    TST_LOG_NAME(testname3, "Disconnecting third.");
    socket2.reset();

    TST_LOG_NAME(testname2, "Getting status from first view.");
    helpers::sendTextFrame(socket1, "status", testname2);
    helpers::assertResponseString(socket1, "status:", testname2);

    TST_LOG_NAME(testname2, "Disconnecting second.");
    socket1.reset();
    return TestResult::Ok;
}

UnitBase::TestResult UnitLoad::testLoadSimple()
{
    testname = __func__;
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
    loadDoc(documentURL, "load ");
    return TestResult::Ok;
}

UnitBase::TestResult UnitLoad::testBadLoad()
{
    testname = __func__;
    try
    {
        // Load a document and get its status.
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname + "Poll");
        socketPoll->startThread();

        Poco::URI uri(helpers::getTestServerURI());
        std::shared_ptr<http::WebSocketSession> socket =
            helpers::connectLOKit(socketPoll, uri, documentURL, testname);

        // Before loading request status.
        helpers::sendTextFrame(socket, "status", testname);

        const auto line = helpers::assertResponseString(socket, "error:", testname);
        LOK_ASSERT_EQUAL_STR("error: cmd=status kind=nodocloaded", line);
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitLoad::testExcelLoad()
{
    testname = __func__;
    try
    {
        // Load a document and get status.
        Poco::URI uri(helpers::getTestServerURI());

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("ExcelLoadPoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket =
            helpers::loadDocAndGetSession(socketPoll, "timeline.xlsx", uri, testname);

        helpers::sendTextFrame(socket, "status", testname);
        const auto status = helpers::assertResponseString(socket, "status:", testname);

        // Expected format is something like 'status: type=spreadsheet parts=1 current=0 width=20685 height=24885 viewid=0 lastcolumn=31 lastrow=12 readonly=0'
        StringVector tokens(StringVector::tokenize(status, ' '));
        LOK_ASSERT(tokens.size() >= 9);
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitLoad::testReload()
{
    testname = __func__;
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
    for (int i = 0; i < 3; ++i)
    {
        TST_LOG("Loading #" << (i + 1));
        Util::Stopwatch sw;
        loadDoc(documentURL, testname);
        TST_LOG("Loaded #" << (i + 1) << " in " << sw.elapsed());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitLoad::testLoad()
{
    testname = __func__;
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    std::shared_ptr<SocketPoll> socketPollPtr = std::make_shared<SocketPoll>("LoadPoll");
    socketPollPtr->startThread();

    auto wsSession
        = http::WebSocketSession::create(socketPollPtr, helpers::getTestServerURI(), documentURL);

    TST_LOG("Loading " << documentURL);
    wsSession->sendMessage("load url=" + documentURL);

    std::vector<char> message = wsSession->waitForMessage("status:", 10s);
    LOK_ASSERT_MESSAGE("Failed to load the document", !message.empty());

    wsSession->asyncShutdown();

    LOK_ASSERT_MESSAGE("Expected success disconnection of the WebSocket",
                       wsSession->waitForDisconnection(10s));

    return TestResult::Ok;
}

UnitBase::TestResult UnitLoad::testLoadUserPrivateInfo()
{
    // Given a document to be loaded:
    testname = __func__;
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
    std::shared_ptr<SocketPoll> socketPollPtr = std::make_shared<SocketPoll>("LoadPoll");
    socketPollPtr->startThread();
    auto wsSession
        = http::WebSocketSession::create(socketPollPtr, helpers::getTestServerURI(), documentURL);

    // When the user private info is valid JSON, but is not a dictionary:
    wsSession->sendMessage("load url=" + documentURL + " authorprivateinfo=%5B%5D");

    // Then make sure we can still load the document:
    std::vector<char> message = wsSession->waitForMessage("status:", std::chrono::seconds(COMMAND_TIMEOUT_SECS));
    // This failed, invalid user private info resulted in a failed load.
    LOK_ASSERT(!message.empty());
    wsSession->asyncShutdown();
    LOK_ASSERT(wsSession->waitForDisconnection(std::chrono::seconds(COMMAND_TIMEOUT_SECS)));
    return TestResult::Ok;
}

UnitBase::TestResult UnitLoad::testLoadSessionRace()
{
    testname = __func__;
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    try
    {
        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname + "Poll");
        socketPoll->startThread();

        Poco::URI uri(helpers::getTestServerURI());

        // Load a document and wait for the status.
        std::shared_ptr<http::WebSocketSession> socket =
            helpers::connectLOKit(socketPoll, uri, documentURL, testname);
        helpers::sendTextFrame(socket, "load url=" + documentURL, testname);
        LOK_ASSERT_MESSAGE("cannot load the document " + documentURL,
                           helpers::isDocumentLoaded(socket, testname, false));

        std::shared_ptr<http::WebSocketSession> socket2 =
            helpers::connectLOKit(socketPoll, uri, documentURL, testname);
        helpers::sendTextFrame(socket, "load url=" + documentURL, testname);
        LOK_ASSERT_MESSAGE("cannot get a 2nd active view " + documentURL,
                           helpers::isDocumentLoaded(socket, testname, true));

        // Now we should have two active views ...
        TST_LOG("We now have two fully-loaded views");

        std::shared_ptr<http::WebSocketSession> socketslowload =
            helpers::connectLOKit(socketPoll, uri, documentURL, testname);

        // FIXME: do we get this ? ...
//        helpers::assertResponseString(socket, "status:", testname);

        // This is the race - notice we don't send the 'load url' at all from this view.
        // We should have a 'session' command already sent to the Kit, but no COKit view yet.

        TST_LOG("Shutting down after second session partially setup");
        socket2->asyncShutdown();
        socket->asyncShutdown();

        // Somehow - we want both these messages to get there fast [!] ...

        // We should have sent a "disconnect" to the kit for session 1
        TST_LOG("Send load after initial view shutdown");
        helpers::sendTextFrame(socketslowload, "load url=" + documentURL, testname);

        // Expect either a successful load or a docunloading error.
        auto response = helpers::getResponseStringAny(
            socketslowload, { "error:", "commandresult: { \"command\": \"load\"" }, testname);
        if (response.starts_with("error:"))
            LOK_ASSERT_EQUAL_STR("error: cmd=load kind=docunloading", response);
        else
            LOK_ASSERT_EQUAL_STR(
                R"xx(commandresult: { "command": "load", "success": true, "result": "", "errorMsg": ""})xx",
                response);
        //    FIXME: somewhere we should be failing / asserting on something here ...
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }

    return TestResult::Ok;
}

void UnitLoad::invokeWSDTest()
{
    UnitBase::TestResult result;

    result = testLoad();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testConnectNoLoad();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testLoadSimple();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testExcelLoad();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testLoadUserPrivateInfo();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testLoadSessionRace();
    if (result != TestResult::Ok)
        exitTest(result);

    // Disabling because it doesn't handle 'error: cmd=load kind=docunloading'
    // when the document unloads. This leads to unreliable results.
    // result = testReload();
    // if (result != TestResult::Ok)
    //     exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitLoad(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
