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
 * Unit test for cursor positioning and movement functionality.
 */

#include <config.h>

#include <common/Unit.hpp>

#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/Exception.h>
#include <Poco/URI.h>

#include <memory>
#include <sstream>
#include <string>

using namespace std::literals;

namespace
{
void getCursor(const std::string& message, int& cursorX, int& cursorY, int& cursorWidth,
               int& cursorHeight, const std::string& testname)
{
    Poco::JSON::Parser parser;
    const Poco::Dynamic::Var result = parser.parse(message);
    const auto& command = result.extract<Poco::JSON::Object::Ptr>();
    std::string text = command->get("commandName").toString();
    LOK_ASSERT_EQUAL_STR(".uno:CellCursor", text);
    text = command->get("commandValues").toString();
    LOK_ASSERT(!text.empty());
    StringVector position(StringVector::tokenize(text, ','));
    cursorX = NumUtil::stoi(position[0]);
    cursorY = NumUtil::stoi(position[1]);
    cursorWidth = NumUtil::stoi(position[2]);
    cursorHeight = NumUtil::stoi(position[3]);
    LOK_ASSERT(cursorX >= 0);
    LOK_ASSERT(cursorY >= 0);
    LOK_ASSERT(cursorWidth >= 0);
    LOK_ASSERT(cursorHeight >= 0);
}

void limitCursor(
    const std::function<void(const std::shared_ptr<http::WebSocketSession>& socket, int cursorX,
                             int cursorY, int cursorWidth, int cursorHeight, int docWidth,
                             int docHeight)>& keyhandler,
    const std::function<void(int docWidth, int docHeight, int newWidth, int newHeight)>&
        checkhandler,
    const std::string& testname)
{
    std::string docSheet;
    int docSheets = 0;
    int docHeight = 0;
    int docWidth = 0;
    int docViewId = -1;
    std::string newSheet;
    int newSheets = 0;
    int newHeight = 0;
    int newWidth = 0;
    int cursorX = 0;
    int cursorY = 0;
    int cursorWidth = 0;
    int cursorHeight = 0;

    std::string response;

    Poco::URI uri(helpers::getTestServerURI());

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("CursorPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> socket =
        helpers::loadDocAndGetSession(socketPoll, "empty.ods", uri, testname);

    // check document size
    helpers::sendTextFrame(socket, "status", testname);
    response = helpers::assertResponseString(socket, "status:", testname);
    helpers::parseDocSize(response.substr(7), "spreadsheet", docSheet, docSheets, docWidth,
                          docHeight, docViewId, testname);

    // Send an arrow key to initialize the CellCursor, otherwise we get "EMPTY".
    helpers::sendTextFrame(socket, "key type=input char=0 key=1027", testname);

    std::ostringstream text;
    text << "commandvalues "
         << "command=.uno:CellCursor?outputHeight=" << 256 << "&outputWidth=" << 256
         << "&tileHeight=" << 3840 << "&tileWidth=" << 3840;
    helpers::sendTextFrame(socket, text.str(), testname);
    const auto cursor = helpers::getResponseString(socket, "commandvalues:", testname);
    getCursor(cursor.substr(14), cursorX, cursorY, cursorWidth, cursorHeight, testname);

    // move cursor
    keyhandler(socket, cursorX, cursorY, cursorWidth, cursorHeight, docWidth, docHeight);

    // filter messages, and expect to receive new document size
    response = helpers::assertResponseString(socket, "status:", testname);
    helpers::parseDocSize(response.substr(7), "spreadsheet", newSheet, newSheets, newWidth,
                          newHeight, docViewId, testname);

    LOK_ASSERT_EQUAL(docSheets, newSheets);
    LOK_ASSERT_EQUAL(docSheet, newSheet);

    // check new document size
    checkhandler(docWidth, docHeight, newWidth, newHeight);
}
}

/// Test suite for cursor handling.
class UnitCursor : public UnitWSD
{
    TestResult testMaxColumn();
    TestResult testMaxRow();
    TestResult testInsertAnnotationWriter();
    TestResult testEditAnnotationWriter();
    TestResult testInsertAnnotationCalc();

public:
    UnitCursor()
        : UnitWSD("UnitCursor")
    {
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitCursor::testMaxColumn()
{
    try
    {
        limitCursor(
            // move cursor to last column
            [&](const std::shared_ptr<http::WebSocketSession>& socket, int cursorX, int cursorY,
                int cursorWidth, int cursorHeight, int docWidth, int docHeight)
            {
                LOK_ASSERT(cursorX >= 0);
                LOK_ASSERT(cursorY >= 0);
                LOK_ASSERT(cursorWidth >= 0);
                LOK_ASSERT(cursorHeight >= 0);
                LOK_ASSERT(docWidth >= 0);
                LOK_ASSERT(docHeight >= 0);

                const std::string text = "key type=input char=0 key=1027";
                while (cursorX <= docWidth)
                {
                    helpers::sendTextFrame(socket, text, testname);
                    cursorX += cursorWidth;
                }
            },
            // check new document width
            [&](int docWidth, int docHeight, int newWidth, int newHeight)
            {
                LOK_ASSERT_EQUAL(docHeight, newHeight);
                LOK_ASSERT(newWidth > docWidth);
            },
            "maxColumn");
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitCursor::testMaxRow()
{
    try
    {
        limitCursor(
            // move cursor to last row
            [&](const std::shared_ptr<http::WebSocketSession>& socket, int cursorX, int cursorY,
                int cursorWidth, int cursorHeight, int docWidth, int docHeight)
            {
                LOK_ASSERT(cursorX >= 0);
                LOK_ASSERT(cursorY >= 0);
                LOK_ASSERT(cursorWidth >= 0);
                LOK_ASSERT(cursorHeight >= 0);
                LOK_ASSERT(docWidth >= 0);
                LOK_ASSERT(docHeight >= 0);

                const std::string text = "key type=input char=0 key=1024";
                while (cursorY <= docHeight)
                {
                    helpers::sendTextFrame(socket, text, testname);
                    cursorY += cursorHeight;
                }
            },
            // check new document height
            [&](int docWidth, int docHeight, int newWidth, int newHeight)
            {
                LOK_ASSERT_EQUAL(docWidth, newWidth);
                LOK_ASSERT(newHeight > docHeight);
            },
            "maxRow");
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitCursor::testInsertAnnotationWriter()
{
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    Poco::URI uri(helpers::getTestServerURI());

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("CursorPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> socket =
        helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

    // Insert comment.
    helpers::sendTextFrame(socket, "uno .uno:InsertAnnotation", testname);
    helpers::assertResponseString(socket, "invalidatetiles:", testname);

    // Paste some text.
    helpers::sendTextFrame(socket, "paste mimetype=text/plain;charset=utf-8\nxxx yyy zzzz",
                           testname);

    // Read it back.
    std::string res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: xxx yyy zzzz", res);
    // Can we edit the comment?
    helpers::sendTextFrame(socket, "paste mimetype=text/plain;charset=utf-8\naaa bbb ccc",
                           testname);
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: aaa bbb ccc", res);

    // Confirm that the text is in the comment and not doc body.
    // Click in the body.
    helpers::sendTextFrame(
        socket, "mouse type=buttondown x=1600 y=1600 count=1 buttons=1 modifier=0", testname);
    helpers::sendTextFrame(socket, "mouse type=buttonup x=1600 y=1600 count=1 buttons=1 modifier=0",
                           testname);
    // Read body text.
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: Hello world", res);

    // Confirm that the comment is still intact.
    helpers::sendTextFrame(
        socket, "mouse type=buttondown x=13855 y=1893 count=1 buttons=1 modifier=0", testname);
    helpers::sendTextFrame(
        socket, "mouse type=buttonup x=13855 y=1893 count=1 buttons=1 modifier=0", testname);
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: aaa bbb ccc", res);

    // Can we still edit the comment?
    helpers::sendTextFrame(
        socket,
        "paste mimetype=text/plain;charset=utf-8\nand now for something completely different",
        testname);
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: and now for something completely different", res);

    // Close and reopen the same document and test again.
    socket->shutdownWS();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));

    TST_LOG("Reloading ");
    socket = helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

    // Confirm that the text is in the comment and not doc body.
    // Click in the body.
    helpers::sendTextFrame(
        socket, "mouse type=buttondown x=1600 y=1600 count=1 buttons=1 modifier=0", testname);
    helpers::sendTextFrame(socket, "mouse type=buttonup x=1600 y=1600 count=1 buttons=1 modifier=0",
                           testname);
    // Read body text.
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: Hello world", res);

    // Confirm that the comment is still intact.
    helpers::sendTextFrame(
        socket, "mouse type=buttondown x=13855 y=1893 count=1 buttons=1 modifier=0", testname);
    helpers::sendTextFrame(
        socket, "mouse type=buttonup x=13855 y=1893 count=1 buttons=1 modifier=0", testname);
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: and now for something completely different", res);

    // Can we still edit the comment?
    helpers::sendTextFrame(socket, "paste mimetype=text/plain;charset=utf-8\nblah blah xyz",
                           testname);
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: blah blah xyz", res);
    return TestResult::Ok;
}

UnitBase::TestResult UnitCursor::testEditAnnotationWriter()
{
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("with_comment.odt", documentPath, documentURL, testname);

    Poco::URI uri(helpers::getTestServerURI());

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("CursorPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> socket =
        helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

    // Click in the body.
    helpers::sendTextFrame(
        socket, "mouse type=buttondown x=1600 y=1600 count=1 buttons=1 modifier=0", testname);
    helpers::sendTextFrame(socket, "mouse type=buttonup x=1600 y=1600 count=1 buttons=1 modifier=0",
                           testname);
    // Read body text.
    std::string res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: Hello world", res);

    // Confirm that the comment is intact.
    helpers::sendTextFrame(
        socket, "mouse type=buttondown x=13855 y=1893 count=1 buttons=1 modifier=0", testname);
    helpers::sendTextFrame(
        socket, "mouse type=buttonup x=13855 y=1893 count=1 buttons=1 modifier=0", testname);
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: blah blah xyz", res);

    // Can we still edit the comment?
    helpers::sendTextFrame(
        socket,
        "paste mimetype=text/plain;charset=utf-8\nand now for something completely different",
        testname);
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: and now for something completely different", res);

    // Close and reopen the same document and test again.
    TST_LOG("Closing connection after pasting.");
    socket->shutdownWS();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));

    TST_LOG("Reloading ");
    socket = helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

    // Confirm that the text is in the comment and not doc body.
    // Click in the body.
    helpers::sendTextFrame(
        socket, "mouse type=buttondown x=1600 y=1600 count=1 buttons=1 modifier=0", testname);
    helpers::sendTextFrame(socket, "mouse type=buttonup x=1600 y=1600 count=1 buttons=1 modifier=0",
                           testname);
    // Read body text.
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: Hello world", res);

    // Confirm that the comment is still intact.
    helpers::sendTextFrame(
        socket, "mouse type=buttondown x=13855 y=1893 count=1 buttons=1 modifier=0", testname);
    helpers::sendTextFrame(
        socket, "mouse type=buttonup x=13855 y=1893 count=1 buttons=1 modifier=0", testname);
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: and now for something completely different", res);

    // Can we still edit the comment?
    helpers::sendTextFrame(socket, "paste mimetype=text/plain;charset=utf-8\nnew text different",
                           testname);
    res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: new text different", res);
    return TestResult::Ok;
}

UnitBase::TestResult UnitCursor::testInsertAnnotationCalc()
{
    Poco::URI uri(helpers::getTestServerURI());

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("CursorPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> socket =
        helpers::loadDocAndGetSession(socketPoll, "setclientpart.ods", uri, testname);

    // Insert comment.
    helpers::sendTextFrame(socket, "uno .uno:InsertAnnotation", testname);

    // Paste some text.
    helpers::sendTextFrame(socket, "paste mimetype=text/plain;charset=utf-8\naaa bbb ccc",
                           testname);

    // Read it back.
    std::string res = helpers::getAllText(socket, testname);
    LOK_ASSERT_EQUAL_STR("textselectioncontent: aaa bbb ccc", res);
    return TestResult::Ok;
}

void UnitCursor::invokeWSDTest()
{
    UnitBase::TestResult result = testMaxColumn();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testMaxRow();
    if (result != TestResult::Ok)
        exitTest(result);

    // result = testInsertAnnotationWriter();
    // if (result != TestResult::Ok)
    //     exitTest(result);

    // result = testEditAnnotationWriter();
    // if (result != TestResult::Ok)
    //     exitTest(result);

    // FIXME result = testInsertAnnotationCalc();
    // if (result != TestResult::Ok)
    //     exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitCursor(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
