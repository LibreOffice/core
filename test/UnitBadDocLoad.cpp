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
 * Unit test for document loading error handling and bad document scenarios.
 */

#include <config.h>

#include <Png.hpp>
#include <Unit.hpp>
#include <WebSocketSession.hpp>
#include <helpers.hpp>

#include <Poco/Exception.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/RegularExpression.h>
#include <Poco/URI.h>
#include <test/lokassert.hpp>

#include <memory>
#include <ostream>
#include <string>

// Include config.h last, so the test server URI is still HTTP, even in SSL builds.
#include <config.h>

class COOLWebSocket;

/// Test suite for bad document loading, etc.
class UnitBadDocLoad : public UnitWSD
{
    TestResult testBadDocLoadFail();
    TestResult testMaxDocuments();
    TestResult testMaxConnections();
    TestResult testMaxViews();

public:
    UnitBadDocLoad()
        : UnitWSD("UnitBadDocLoad")
    {
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitBadDocLoad::testBadDocLoadFail()
{
    // Load corrupted document and validate error.
    try
    {
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("corrupted.odt", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>(testname);
        socketPoll->startThread();

        Poco::URI uri(helpers::getTestServerURI());
        std::shared_ptr<http::WebSocketSession> socket =
            helpers::connectLOKit(socketPoll, uri, documentURL, testname);

        // Send a load request with incorrect password
        helpers::sendTextFrame(socket, "load url=" + documentURL, testname);

        std::map<std::string, std::string> items;

        // We receive jsdialog with question about repair
        const auto dialog = helpers::getResponseString(socket, "jsdialog:", testname);
        LOK_ASSERT_EQUAL(true, dialog.size() > 0);

        // Extract all json entries into a map.
        items = JsonUtil::jsonToMap(dialog.substr(sizeof("jsdialog:")));
        const std::string firstId = items["id"];

        // Click "Yes" in a dialog
        helpers::sendTextFrame(socket, "dialogevent " + firstId + " {\"id\":\"yes\", \"cmd\": \"click\", \"data\": \"2\", \"type\": \"responsebutton\"}", testname);

        // we can potentially receive multiple jsdialog update messages
        std::string dialog2;
        do {
            dialog2 = helpers::getResponseString(socket, "jsdialog:", testname);
            LOK_ASSERT_EQUAL(true, dialog2.size() > 0);
            items = JsonUtil::jsonToMap(dialog2.substr(sizeof("jsdialog:")));
        } while(items["id"] == firstId); // a duplicate update of existing dialog

        // Now we received jsdialog with warning that repair failed

        // Click "Ok"
        helpers::sendTextFrame(socket, "dialogevent " + items["id"] + " {\"id\":\"ok\", \"cmd\": \"click\", \"data\": \"1\", \"type\": \"responsebutton\"}", testname);

        auto response = helpers::getResponseString(socket, "error:", testname);

        StringVector tokens(StringVector::tokenize(response, ' '));
        LOK_ASSERT_EQUAL(static_cast<size_t>(4), tokens.size());

        std::string errorCommand;
        std::string errorKind;
        std::string errorDetail;
        COOLProtocol::getTokenString(tokens[1], "cmd", errorCommand);
        COOLProtocol::getTokenString(tokens[2], "kind", errorKind);
        COOLProtocol::getTokenString(tokens[3], "errordetail", errorDetail);
        LOK_ASSERT_EQUAL_STR("load", errorCommand);
        LOK_ASSERT_EQUAL_STR("faileddocloading", errorKind);
        LOK_ASSERT(!errorDetail.empty());
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitBadDocLoad::testMaxDocuments()
{
    static_assert(MAX_DOCUMENTS >= 2, "MAX_DOCUMENTS must be at least 2");

    if (MAX_DOCUMENTS > 20)
    {
        std::cerr << "skipping test since MAX_DOCUMENTS (" << MAX_DOCUMENTS
                  << ") is too high to test. Set to a more sensible number, ideally a dozen or so."
                  << std::endl;
        return TestResult::Ok;
    }

    try
    {
        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("BadDocLoadPoll");
        socketPoll->startThread();

        // Load a document.
        std::vector<std::shared_ptr<http::WebSocketSession>> docs;

        std::cerr << "Loading max number of documents: " << MAX_DOCUMENTS << std::endl;
        Poco::URI uri(helpers::getTestServerURI());
        for (int it = 1; it <= MAX_DOCUMENTS; ++it)
        {
            docs.emplace_back(
                helpers::loadDocAndGetSession(socketPoll, "empty.odt", uri, testname));
            std::cerr << "Loaded document #" << it << " of " << MAX_DOCUMENTS << std::endl;
        }

        std::cerr << "Loading one more document beyond the limit." << std::endl;

        // try to open MAX_DOCUMENTS + 1
        std::string docPath;
        std::string documentURL;
        helpers::getDocumentPathAndURL("empty.odt", docPath, documentURL, testname);

        std::shared_ptr<http::WebSocketSession> socket =
            helpers::connectLOKit(socketPoll, uri, documentURL, testname);

        // Send load request, which will fail.
        helpers::sendTextFrame(socket, "load url=" + documentURL, testname);

        helpers::assertResponseString(socket, "error:", testname);

        //FIXME: Implement in http::WebSocketSession.
        // std::string message;
        // const int statusCode = helpers::getErrorCode(socket, message, testname);
        // LOK_ASSERT_EQUAL(static_cast<int>(Poco::Net::WebSocket::WS_POLICY_VIOLATION),
        //                      statusCode);

        socket->shutdownWS();
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitBadDocLoad::testMaxConnections()
{
    static_assert(MAX_CONNECTIONS >= 3, "MAX_CONNECTIONS must be at least 3");

    if (MAX_CONNECTIONS > 40)
    {
        std::cerr << "skipping test since MAX_CONNECTION (" << MAX_CONNECTIONS
                  << ") is too high to test. Set to a more sensible number, ideally a dozen or so."
                  << std::endl;
        return TestResult::Ok;
    }

    try
    {
        std::cerr << "Opening max number of connections: " << MAX_CONNECTIONS << std::endl;

        // Load a document.
        std::string docPath;
        std::string documentURL;

        helpers::getDocumentPathAndURL("empty.odt", docPath, documentURL, testname);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, documentURL);
        Poco::URI uri(helpers::getTestServerURI());

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("BadDocLoadPoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket =
            helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

        std::cerr << "Opened connection #1 of " << MAX_CONNECTIONS << std::endl;

        std::vector<std::shared_ptr<COOLWebSocket>> views;
        for (int it = 1; it < MAX_CONNECTIONS; ++it)
        {
            std::unique_ptr<Poco::Net::HTTPClientSession> session(helpers::createSession(uri));
            Poco::Net::HTTPResponse httpResponse;
            auto ws = std::make_shared<COOLWebSocket>(*session, request, httpResponse);
            views.emplace_back(ws);
            std::cerr << "Opened connection #" << (it + 1) << " of " << MAX_CONNECTIONS
                      << std::endl;
        }

        std::cerr << "Opening one more connection beyond the limit." << std::endl;

        // try to connect MAX_CONNECTIONS + 1
        std::unique_ptr<Poco::Net::HTTPClientSession> session(helpers::createSession(uri));
        Poco::Net::HTTPResponse httpResponse;
        auto socketN = std::make_shared<COOLWebSocket>(*session, request, httpResponse);

        // Send load request, which will fail.
        helpers::sendTextFrame(socketN, "load url=" + documentURL, testname);

        helpers::assertResponseString(socket, "error:", testname);

        //FIXME: Implement in http::WebSocketSession.
        // std::string message;
        // const int statusCode = helpers::getErrorCode(socketN, message, testname);
        // LOK_ASSERT_EQUAL(static_cast<int>(Poco::Net::WebSocket::WS_POLICY_VIOLATION),
        //                      statusCode);

        socketN->shutdown();
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitBadDocLoad::testMaxViews()
{
    static_assert(MAX_CONNECTIONS >= 3, "MAX_CONNECTIONS must be at least 3");

    if (MAX_CONNECTIONS > 40)
    {
        std::cerr << "skipping test since MAX_CONNECTION (" << MAX_CONNECTIONS
                  << ") is too high to test. Set to a more sensible number, ideally a dozen or so."
                  << std::endl;
        return TestResult::Ok;
    }

    try
    {
        std::cerr << "Opening max number of views: " << MAX_CONNECTIONS << std::endl;

        // Load a document.
        std::string docPath;
        std::string documentURL;

        helpers::getDocumentPathAndURL("empty.odt", docPath, documentURL, testname);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, documentURL);
        Poco::URI uri(helpers::getTestServerURI());

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("BadDocLoadPoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket =
            helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

        std::cerr << "Opened view #1 of " << MAX_CONNECTIONS << std::endl;

        std::vector<std::shared_ptr<http::WebSocketSession>> views;
        for (int it = 1; it < MAX_CONNECTIONS; ++it)
        {
            views.emplace_back(
                helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname));
            std::cerr << "Opened view #" << (it + 1) << " of " << MAX_CONNECTIONS << std::endl;
        }

        std::cerr << "Opening one more connection beyond the limit." << std::endl;

        // try to connect MAX_CONNECTIONS + 1
        std::unique_ptr<Poco::Net::HTTPClientSession> session(helpers::createSession(uri));
        Poco::Net::HTTPResponse httpResponse;
        auto socketN = std::make_shared<COOLWebSocket>(*session, request, httpResponse);

        // Send load request, which will fail.
        helpers::sendTextFrame(socketN, "load url=" + documentURL, testname);

        helpers::assertResponseString(socket, "error:", testname);

        //FIXME: Implement in http::WebSocketSession.
        // std::string message;
        // const int statusCode = helpers::getErrorCode(socketN, message, testname);
        // LOK_ASSERT_EQUAL(static_cast<int>(Poco::Net::WebSocket::WS_POLICY_VIOLATION),
        //                      statusCode);
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

void UnitBadDocLoad::invokeWSDTest()
{
    UnitBase::TestResult result = testBadDocLoadFail();
    if (result != TestResult::Ok)
        exitTest(result);

// FIXME: Disabled recently - breaking the tests - should
//        check for the warning popup instead.
#if 0
    result = testMaxDocuments();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testMaxConnections();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testMaxViews();
    if (result != TestResult::Ok)
        exitTest(result);
#endif

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitBadDocLoad(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
