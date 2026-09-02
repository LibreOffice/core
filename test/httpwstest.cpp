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
 * Test for HTTP and WebSocket functionality.
 */

#include <config.h>

#include <WebSocketSession.hpp>

#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/InvalidCertificateHandler.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/URI.h>

#include <cppunit/extensions/HelperMacros.h>

#include <Common.hpp>
#include <Protocol.hpp>

#include <lokassert.hpp>
#include <helpers.hpp>
#include <KitPidHelpers.hpp>

#include <chrono>

using namespace std::literals;
using namespace helpers;

/// Tests the HTTP WebSocket API of coolwsd. The server has to be started manually before running this test.
class HTTPWSTest : public CPPUNIT_NS::TestFixture
{
    const Poco::URI _uri;
    Poco::Net::HTTPResponse _response;
    std::shared_ptr<SocketPoll> _socketPoll;

    CPPUNIT_TEST_SUITE(HTTPWSTest);

    CPPUNIT_TEST(testExoticLang);
    CPPUNIT_TEST(testSaveOnDisconnect);
    CPPUNIT_TEST(testReloadWhileDisconnecting);
    CPPUNIT_TEST(testInactiveClient);
    CPPUNIT_TEST(testViewInfoMsg);
    CPPUNIT_TEST(testUndoConflict);

    CPPUNIT_TEST_SUITE_END();

    void testExoticLang();
    void testSaveOnDisconnect();
    void testReloadWhileDisconnecting();
    void testInactiveClient();
    void testViewInfoMsg();
    void testUndoConflict();

public:
    HTTPWSTest()
        : _uri(helpers::getTestServerURI())
        , _socketPoll(std::make_shared<SocketPoll>("HttpWsPoll"))
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

    ~HTTPWSTest()
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
        waitForKitPidsReady("tearDown");
        resetTestStartTime();
    }
};

void HTTPWSTest::testExoticLang()
{
    const std::string testname = "testExoticLang- ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    try
    {
        // es-419 locale takes significantly longer than the default.
        // 22+ seconds has been observed.
        const auto timeout = std::chrono::seconds(COMMAND_TIMEOUT_SECS * 6);
        std::shared_ptr<http::WebSocketSession> socket = loadDocAndGetSession(
            _socketPoll, _uri, documentURL, "exoticlocale", /*isView=*/true, /*isAssert=*/true,
            /*loadParams=*/" lang=es-419", timeout);
        socket->asyncShutdown();
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}

void HTTPWSTest::testSaveOnDisconnect()
{
    const std::string testname = "saveOnDisconnect- ";

    Util::rng::seedForTesting(200177);
    const std::string text = helpers::genRandomString(40);
    TST_LOG("Test string: [" << text << "].");

    std::string documentPath, documentURL;
    getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    try
    {
        std::shared_ptr<http::WebSocketSession> socket1
            = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname + "1 ");
        std::shared_ptr<http::WebSocketSession> socket2
            = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname + "2 ");

        sendTextFrame(socket2, "userinactive", testname);

        deleteAll(socket1, testname, 100ms, 1);
        sendTextFrame(socket1, "paste mimetype=text/plain;charset=utf-8\n" + text, testname);
        getResponseMessage(socket1, "pasteresult: success", testname);

        // Shutdown abruptly.
        TST_LOG("Closing connection after pasting.");

        socket1->asyncShutdown(); // Should trigger saving.
        socket2->asyncShutdown();

        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                           socket1->waitForDisconnection(5s));
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                           socket2->waitForDisconnection(5s));

        // Allow time to save and destroy before we connect again.
        waitForKitPidsReady(testname);

        TST_LOG("Loading again.");
        // Load the same document and check that the last changes (pasted text) is saved.
        std::shared_ptr<http::WebSocketSession> socket
            = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname + "3 ");

        // Check if the document contains the pasted text.
        std::string selection = getAllText(socket, testname);

        // FIXME: intermittently it seems we get an unexpected newline here ...
        if (!selection.empty() && selection.back() == '\n')
        {
            LOG_DBG("Unexpected redundant new-line at end of string: ' " << selection << "'");
            selection.pop_back();
        }

        // if 'selection' is [] empty - we mis-matched in getAllText
        LOK_ASSERT_EQUAL("textselectioncontent: " + text, selection);

        socket->asyncShutdown();

        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 3",
                           socket->waitForDisconnection(5s));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}

void HTTPWSTest::testReloadWhileDisconnecting()
{
    const char* testname = "reloadWhileDisconnecting ";
    try
    {
        std::string documentPath, documentURL;
        getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

        std::shared_ptr<http::WebSocketSession> socket
            = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

        // Drain the 'textselection:' issued upon loading, which confuses the following SelectAll.
        getResponseMessage(socket, "textselection:", testname);

        deleteAll(socket, testname);
        sendTextFrame(socket, "paste mimetype=text/plain;charset=utf-8\naaa bbb ccc", testname);

        // Closing connection too fast might not flush buffers.
        // Often nothing more than the SelectAll reaches the server before
        // the socket is closed, when the doc is not even modified yet.
        getResponseMessage(socket, "statechanged", testname);

        // Shutdown abruptly.
        TST_LOG("Closing connection after pasting.");
        socket->asyncShutdown();
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));

        // Do not wait here. Reconnect before disconnect finishes
        // TODO: Test fails because it is unable to reconnect

        // Load the same document and check that the last changes (pasted text) is saved.
        TST_LOG("Loading again.");
        socket = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

        TST_LOG("Checking if the document contains the pasted text.");
        const std::string expected = "aaa bbb ccc";
        const std::string selection = getAllText(socket, testname, expected);
        LOK_ASSERT_EQUAL_STR("textselectioncontent: " + expected, selection);

        TST_LOG("Closing connection after getting pasted text.");
        socket->asyncShutdown();

        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}

void HTTPWSTest::testInactiveClient()
{
    const char* testname = "inactiveClient ";
    try
    {
        std::string documentPath, documentURL;
        getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

        std::shared_ptr<http::WebSocketSession> socket1
            = loadDocAndGetSession(_socketPoll, _uri, documentURL, "inactiveClient-1 ");

        // Connect another and go inactive.
        TST_LOG_NAME("inactiveClient-2 ", "Connecting second client.");
        std::shared_ptr<http::WebSocketSession> socket2
            = loadDocAndGetSession(_socketPoll, _uri, documentURL, "inactiveClient-2 ", true);
        sendTextFrame(socket2, "userinactive", "inactiveClient-2 ");

        // While second is inactive, make some changes.
        sendTextFrame(socket1, "key type=input char=97 key=0", "inactiveClient-1 ");
        sendTextFrame(socket1, "key type=up char=0 key=512", "inactiveClient-1 ");

        // Activate second.
        sendTextFrame(socket2, "useractive", "inactiveClient-2 ");
        SocketProcessor("Second ", socket2, [&](const std::string& msg)
                {
                    const auto token = COOLProtocol::getFirstToken(msg);
                    // 'window:' is e.g. 'window: {"id":"4","action":"invalidate","rectangle":"0, 0,
                    // 0, 0"}', which is probably fine, given that other invalidations are also
                    // expected.
                    LOK_ASSERT_MESSAGE("unexpected message: " + msg,
                                            token == "a11yfocuschanged:" ||
                                            token == "corelog:" ||
                                            token == "cursorvisible:" ||
                                            token == "graphicselection:" ||
                                            token == "graphicviewselection:" ||
                                            token == "invalidatecursor:" ||
                                            token == "invalidatetiles:" ||
                                            token == "invalidateviewcursor:" ||
                                            token == "setpart:" ||
                                            token == "statechanged:" ||
                                            token == "textselection:" ||
                                            token == "textselectionend:" ||
                                            token == "textselectionstart:" ||
                                            token == "textviewselection:" ||
                                            token == "viewcursorvisible:" ||
                                            token == "viewinfo:" ||
                                            token == "editor:" ||
                                            token == "context:" ||
                                            token == "window:" ||
                                            token == "hrulerupdate:" ||
                                            token == "vrulerupdate:" ||
                                            token == "tableselected:" ||
                                            token == "colorpalettes:" ||
                                            token == "jsdialog:" ||
                                            token == "serveraudit:" ||
                                            token == "loaded:" ||
                                            token == "unloaded:" ||
                                            token == "adminuser:");

                    // End when we get state changed.
                    return (token != "statechanged:");
                });

        TST_LOG("Second client finished. Shutting down");
        socket2->asyncShutdown();
        TST_LOG("Shutting down first client");
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

void HTTPWSTest::testViewInfoMsg()
{
    // Load 2 documents, cross-check the viewid received by each of them in their status message
    // with the one sent in viewinfo message to itself as well as to other one

    const std::string testname = "testViewInfoMsg-";
    std::string docPath;
    std::string docURL;
    getDocumentPathAndURL("hello.odt", docPath, docURL, testname);

    std::shared_ptr<http::WebSocketSession> socket0
        = connectLOKit(_socketPoll, _uri, docURL, testname + "0 ");
    std::shared_ptr<http::WebSocketSession> socket1
        = connectLOKit(_socketPoll, _uri, docURL, testname + "1 ");

    std::string response;
    std::string part;
    int parts, width, height;
    int viewid[2] = { 0 };

    try
    {
        // Load first view and remember the viewid
        TST_LOG("Loading the first view");
        sendTextFrame(socket0, "load url=" + docURL, testname);
        response = getResponseString(socket0, "status:", testname + "0 ");
        LOK_ASSERT_MESSAGE("Expected status: message", !response.empty());
        parseDocSize(response.substr(7), "text", part, parts, width, height, viewid[0], testname);

        // Check if viewinfo message also mentions the same viewid
        TST_LOG("Waiting for [viewinfo:] from the first view");
        response = getResponseString(socket0, "viewinfo: ", testname + "0 ");
        Poco::JSON::Parser parser0;
        Poco::JSON::Array::Ptr array = parser0.parse(response.substr(9)).extract<Poco::JSON::Array::Ptr>();
        LOK_ASSERT_EQUAL(static_cast<size_t>(1), array->size());

        Poco::JSON::Object::Ptr viewInfoObj0 = array->getObject(0);
        int viewid0 = viewInfoObj0->get("id").convert<int>();
        LOK_ASSERT_EQUAL(viewid[0], viewid0);

        // Load second view and remember the viewid
        TST_LOG("Loading the second view");
        sendTextFrame(socket1, "load url=" + docURL, testname);
        response = getResponseString(socket1, "status:", testname + "1 ");
        parseDocSize(response.substr(7), "text", part, parts, width, height, viewid[1], testname);

        // Check if viewinfo message in this view mentions
        // viewid of both first loaded view and this view
        TST_LOG("Waiting for [viewinfo:] from the second view");
        response = getResponseString(socket1, "viewinfo: ", testname + "1 ");
        Poco::JSON::Parser parser1;
        array = parser1.parse(response.substr(9)).extract<Poco::JSON::Array::Ptr>();
        LOK_ASSERT_EQUAL(static_cast<size_t>(2), array->size());

        viewInfoObj0 = array->getObject(0);
        Poco::JSON::Object::Ptr viewInfoObj1 = array->getObject(1);
        viewid0 = viewInfoObj0->get("id").convert<int>();
        int viewid1 = viewInfoObj1->get("id").convert<int>();

        if (viewid[0] == viewid0)
            LOK_ASSERT_EQUAL(viewid[1], viewid1);
        else if (viewid[0] == viewid1)
            LOK_ASSERT_EQUAL(viewid[1], viewid0);
        else
            LOK_ASSERT_FAIL("Inconsistent viewid in viewinfo and status messages");

        // Check if first view also got the same viewinfo message
        TST_LOG("Waiting for [viewinfo:] from the first view, again");
        const auto response1 = getResponseString(socket0, "viewinfo: ", testname + "0 ");
        LOK_ASSERT_EQUAL(response, response1);

        socket1->asyncShutdown();
        socket0->asyncShutdown();

        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                           socket1->waitForDisconnection(5s));
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 0",
                           socket0->waitForDisconnection(5s));
    }
    catch(const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }

    TST_LOG("Done");
}

void HTTPWSTest::testUndoConflict()
{
    const std::string testname = "testUndoConflict-";
    Poco::JSON::Parser parser;
    std::string docPath;
    std::string docURL;
    int conflict = 0;

    getDocumentPathAndURL("empty.odt", docPath, docURL, testname);

    std::shared_ptr<http::WebSocketSession> socket0
        = connectLOKit(_socketPoll, _uri, docURL, testname + "0 ");
    std::shared_ptr<http::WebSocketSession> socket1
        = connectLOKit(_socketPoll, _uri, docURL, testname + "1 ");

    std::string response;
    try
    {
        // Load first view
        sendTextFrame(socket0, "load url=" + docURL, testname + "0 ");
        response = getResponseString(socket0, "invalidatecursor:", testname + "0 ");

        // Load second view
        sendTextFrame(socket1, "load url=" + docURL, testname + "1 ");
        response = getResponseString(socket1, "invalidatecursor:", testname + "1 ");

        // edit first view
        sendChar(socket0, 'A', skNone, testname + "0 ");
        response = getResponseString(socket0, "invalidateviewcursor: ", testname + "0 ");
        response = getResponseString(socket0, "invalidateviewcursor: ", testname + "0 ");

        // edit second view
        sendChar(socket1, 'B', skNone, testname + "1 ");
        response = getResponseString(socket1, "invalidateviewcursor: ", testname + "1 ");
        response = getResponseString(socket1, "invalidateviewcursor: ", testname + "1 ");

        // try to undo first view
        sendTextFrame(socket0, "uno .uno:Undo", testname + "0 ");

        // undo conflict
        response = getResponseString(socket0, "unocommandresult:", testname + "0 ");
        Poco::JSON::Object::Ptr objJSON = parser.parse(response.substr(17)).extract<Poco::JSON::Object::Ptr>();
        LOK_ASSERT_EQUAL_STR(".uno:Undo", objJSON->get("commandName").toString());
        LOK_ASSERT_EQUAL_STR("true", objJSON->get("success").toString());
        LOK_ASSERT(objJSON->has("result"));
        const Poco::Dynamic::Var parsedResultJSON = objJSON->get("result");
        const auto& resultObj = parsedResultJSON.extract<Poco::JSON::Object::Ptr>();
        LOK_ASSERT_EQUAL_STR("long", resultObj->get("type").toString());
        LOK_ASSERT(Poco::strToInt(resultObj->get("value").toString(), conflict, 10));
        LOK_ASSERT(conflict > 0); /*UNDO_CONFLICT*/

        socket1->asyncShutdown();
        socket0->asyncShutdown();

        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                           socket1->waitForDisconnection(5s));
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 0",
                           socket0->waitForDisconnection(5s));
    }
    catch(const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(HTTPWSTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
