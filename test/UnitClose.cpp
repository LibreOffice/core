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
 * Unit test for document closing functionality.
 */

#include <config.h>

#include <chrono>
#include <memory>
#include <string>

#include <Poco/Exception.h>
#include <Poco/URI.h>
#include <test/lokassert.hpp>

#include <Unit.hpp>
#include <helpers.hpp>

using namespace std::literals;

namespace
{
std::string getFontList(const std::string& message, const std::string& testname)
{
    Poco::JSON::Parser parser;
    const Poco::Dynamic::Var result = parser.parse(message);
    const auto& command = result.extract<Poco::JSON::Object::Ptr>();
    std::string text = command->get("commandName").toString();
    LOK_ASSERT_EQUAL_STR(".uno:CharFontName", text);
    text = command->get("commandValues").toString();
    return text;
}
}

/// Test suite for closing, etc.
class UnitClose : public UnitWSD
{
    TestResult testCloseAfterClose();
    TestResult testFontList();
    TestResult testGraphicInvalidate();
    TestResult testAlertAllUsers();

public:
    UnitClose();
    void invokeWSDTest() override;
};

UnitBase::TestResult UnitClose::testCloseAfterClose()
{
    try
    {
        TST_LOG("Connecting and loading.");
        Poco::URI uri(helpers::getTestServerURI());

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("ClosePoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket =
            helpers::loadDocAndGetSession(socketPoll, "hello.odt", uri, testname);

        // send normal socket shutdown
        TST_LOG("Disconnecting gracefully.");
        socket->asyncShutdown();

        // 5 seconds timeout
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));

        // Verify that we get back a close frame.
        LOK_ASSERT_EQUAL(static_cast<int>(Poco::Net::WebSocket::FRAME_OP_CLOSE),
                         (socket->lastFlags() & Poco::Net::WebSocket::FRAME_OP_BITMASK));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitClose::testFontList()
{
    try
    {
        // Load a document
        Poco::URI uri(helpers::getTestServerURI());

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("ClosePoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket =
            helpers::loadDocAndGetSession(socketPoll, "setclientpart.odp", uri, testname);

        helpers::sendTextFrame(socket, "commandvalues command=.uno:CharFontName", testname);
        const std::vector<char> response
            = helpers::getResponseMessage(socket, "commandvalues:", testname);
        LOK_ASSERT_MESSAGE("did not receive a commandvalues: message as expected",
                               !response.empty());

        std::stringstream streamResponse;
        std::copy(response.begin() + std::string("commandvalues:").length() + 1, response.end(),
                  std::ostream_iterator<char>(streamResponse));
        LOK_ASSERT(!getFontList(streamResponse.str(), testname).empty());
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitClose::testGraphicInvalidate()
{
    try
    {
        // Load a document.
        Poco::URI uri(helpers::getTestServerURI());

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("ClosePoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket =
            helpers::loadDocAndGetSession(socketPoll, "shape.ods", uri, testname);

        // Send click message
        helpers::sendTextFrame(
            socket, "mouse type=buttondown x=1035 y=400 count=1 buttons=1 modifier=0", testname);
        helpers::sendTextFrame(
            socket, "mouse type=buttonup x=1035 y=400 count=1 buttons=1 modifier=0", testname);
        helpers::getResponseString(socket, "graphicselection:", testname);

        // Drag & drop graphic
        helpers::sendTextFrame(
            socket, "mouse type=buttondown x=1035 y=400 count=1 buttons=1 modifier=0", testname);
        helpers::sendTextFrame(socket, "mouse type=move x=1035 y=450 count=1 buttons=1 modifier=0",
                               testname);
        helpers::sendTextFrame(
            socket, "mouse type=buttonup x=1035 y=450 count=1 buttons=1 modifier=0", testname);

        const auto message = helpers::getResponseString(socket, "invalidatetiles:", testname);
        LOK_ASSERT_MESSAGE("Drag & Drop graphic invalidate all tiles",
                               message.find("EMPTY") == std::string::npos);
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitClose::testAlertAllUsers()
{
    // Load two documents, each in two sessions. Tell one session to fake a disk full
    // situation. Expect to get the corresponding error back in all sessions.
    static_assert(MAX_DOCUMENTS >= 2, "MAX_DOCUMENTS must be at least 2");
    try
    {

        Poco::URI uri(helpers::getTestServerURI());

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("ClosePoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket[4];
        socket[0] = helpers::loadDocAndGetSession(socketPoll, "hello.odt", uri, testname);
        socket[1] = helpers::loadDocAndGetSession(socketPoll, "Example.odt", uri, testname);

        // Simulate disk full.
        helpers::sendTextFrame(socket[0], "uno .uno:fakeDiskFull", testname);

        // Assert that both clients get the error.
        for (int i = 0; i < 2; i++)
        {
            const std::string response
                = helpers::assertResponseString(socket[i], "error:", testname);
            StringVector tokens(StringVector::tokenize(response.substr(6), ' '));
            std::string cmd;
            COOLProtocol::getTokenString(tokens, "cmd", cmd);
            LOK_ASSERT_EQUAL_STR("internal", cmd);
            std::string kind;
            COOLProtocol::getTokenString(tokens, "kind", kind);
            LOK_ASSERT_EQUAL_STR("diskfull", kind);
        }
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitClose::UnitClose()
    : UnitWSD("UnitClose")
{
    constexpr std::chrono::minutes timeout_minutes(2);
    setTimeout(timeout_minutes);
}

void UnitClose::invokeWSDTest()
{
    UnitBase::TestResult result = testCloseAfterClose();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testFontList();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testGraphicInvalidate();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testAlertAllUsers();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitClose(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
