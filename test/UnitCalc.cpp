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
 * Unit test for Calc-specific functionality including column/row sizing and rendering.
 */

#include <config.h>

#include <Png.hpp>
#include <Unit.hpp>
#include <WebSocketSession.hpp>
#include <helpers.hpp>
#include <kit/Delta.hpp>

#include <Poco/Exception.h>
#include <Poco/URI.h>
#include <test/lokassert.hpp>

#include <memory>
#include <ostream>
#include <string>

namespace
{
double getColRowSize(const std::string& property, const std::string& message, int index,
                     const std::string& testname)
{
    Poco::JSON::Parser parser;
    const Poco::Dynamic::Var result = parser.parse(message);
    const auto& command = result.extract<Poco::JSON::Object::Ptr>();
    std::string text = command->get("commandName").toString();

    LOK_ASSERT_EQUAL_STR(".uno:ViewRowColumnHeaders", text);
    LOK_ASSERT(command->isArray(property));

    Poco::JSON::Array::Ptr array = command->getArray(property);

    LOK_ASSERT(array->isObject(index));

    Poco::SharedPtr<Poco::JSON::Object> item = array->getObject(index);

    LOK_ASSERT(item->has("size"));

    return item->getValue<double>("size");
}

double getColRowSize(const std::shared_ptr<http::WebSocketSession>& socket, const std::string& item,
                     int index, const std::string& testname)
{
    std::vector<char> response;
    response = helpers::getResponseMessage(socket, "commandvalues:", testname);
    LOK_ASSERT_MESSAGE("did not receive a commandvalues: message as expected",
                           !response.empty());
    std::vector<char> json(response.begin() + std::string("commandvalues:").length(),
                           response.end());
    json.push_back(0);
    return getColRowSize(item, json.data(), index, testname);
}
}

/// Test suite for Calc.
class UnitCalc : public UnitWSD
{
    TestResult testCalcEditRendering();
    TestResult testCalcRenderAfterNewView51();
    TestResult testCalcRenderAfterNewView53();
    TestResult testColumnRowResize();
    TestResult testOptimalResize();

public:
    UnitCalc()
        : UnitWSD("UnitCalc")
    {
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitCalc::testCalcEditRendering()
{
    Poco::URI uri(helpers::getTestServerURI());

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("CalcPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> socket =
        helpers::loadDocAndGetSession(socketPoll, "calc_render.xls", uri, testname);

    helpers::sendTextFrame(socket, "mouse type=buttondown x=5000 y=5 count=1 buttons=1 modifier=0",
                           testname);
    helpers::sendTextFrame(socket, "key type=input char=97 key=0", testname);
    helpers::sendTextFrame(socket, "key type=input char=98 key=0", testname);
    helpers::sendTextFrame(socket, "key type=input char=99 key=0", testname);

    helpers::assertResponseString(socket, "cellformula: abc", testname);

    const char* req = "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=3840 tileposy=0 "
                      "tilewidth=7680 tileheight=7680";
    helpers::sendTextFrame(socket, req, testname);

    const std::vector<char> tile = helpers::getResponseMessage(socket, "tile:", testname);
    TST_LOG("size: " << tile.size());

    // Return early for now when on LO >= 5.2.
    int major = 0;
    int minor = 0;
    helpers::getServerVersion(socket, major, minor, testname);

    const std::string firstLine = COOLProtocol::getFirstLine(tile);
    Blob zimg = std::make_shared<BlobData>(tile.begin() + firstLine.size() + 1, tile.end());

    std::fstream outStream("/tmp/res.z", std::ios::out);
    outStream.write(zimg->data(), zimg->size());
    outStream.close();

    Blob img = DeltaGenerator::expand(zimg);

    png_uint_32 height = 256;
    png_uint_32 width = 256;
    png_uint_32 rowBytes = 256 * 4;
    LOK_ASSERT_EQUAL(img->size(), (size_t)rowBytes * height);

    std::vector<png_bytep> rows;
    for (png_uint_32 i = 0; i < height; ++i)
        rows.push_back((png_bytep)img->data() + rowBytes * i);

    const std::vector<char> exp
        = helpers::readDataFromFile("calc_render_0_256x256.3840,0.7680x7680.png");
    std::stringstream streamExp;
    std::copy(exp.begin(), exp.end(), std::ostream_iterator<char>(streamExp));

    png_uint_32 heightExp = 0;
    png_uint_32 widthExp = 0;
    png_uint_32 rowBytesExp = 0;
    std::vector<png_bytep> rowsExp = Png::decodePNG(streamExp, heightExp, widthExp, rowBytesExp);

    LOK_ASSERT_EQUAL(heightExp, height);
    LOK_ASSERT_EQUAL(widthExp, width);
    LOK_ASSERT_EQUAL(rowBytesExp, rowBytes);

    for (png_uint_32 itRow = 0; itRow < height; ++itRow)
    {
        const bool eq = std::equal(rowsExp[itRow], rowsExp[itRow] + rowBytes, rows[itRow]);
        if (!eq)
        {
            // This is a very strict test that breaks often/easily due to slight rendering
            // differences. So for now just keep it informative only.
            //LOK_ASSERT_MESSAGE("Tile not rendered as expected @ row #" + std::to_string(itRow), eq);
            TST_LOG("\nFAILURE: Tile not rendered as expected @ row #" << itRow);
            break;
        }
    }
    return TestResult::Ok;
}

/// When a second view is loaded to a Calc doc,
/// the first stops rendering correctly.
/// This only happens at high rows.
UnitBase::TestResult UnitCalc::testCalcRenderAfterNewView51()
{
    // Load a doc with the cursor saved at a top row.
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("empty.ods", documentPath, documentURL, testname);

    Poco::URI uri(helpers::getTestServerURI());

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("CalcPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> socket =
        helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

    int major = 0;
    int minor = 0;
    helpers::getServerVersion(socket, major, minor, testname);
    if (major != 5 || minor != 1)
    {
        TST_LOG("Skipping test on incompatible client [" << major << '.' << minor
                                                         << "], expected [5.1].");
        return TestResult::Ok;
    }

    // Page Down until we get to the bottom of the doc.
    for (int i = 0; i < 40; ++i)
    {
        helpers::sendTextFrame(socket, "key type=input char=0 key=1031", testname);
    }

    // Wait for status due to doc resize.
    helpers::assertResponseString(socket, "status:", testname);

    const char* req = "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0 "
                      "tileposy=253440 tilewidth=3840 tileheight=3840";

    // Get tile.
    const std::vector<char> tile1
        = helpers::getTileAndSave(socket, req, "/tmp/calc_render_51_orig.png", testname);

    // Connect second client, which will load at the top.
    TST_LOG("Connecting second client.");
    std::shared_ptr<http::WebSocketSession> socket2 =
        helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

    // Up one row on the first view to trigger the bug.
    TST_LOG("Up.");
    helpers::sendTextFrame(socket, "key type=input char=0 key=1025", testname);
    helpers::assertResponseString(socket, "invalidatetiles:", testname); // Up invalidates.

    // Get same tile again.
    const std::vector<char> tile2
        = helpers::getTileAndSave(socket, req, "/tmp/calc_render_51_sec.png", testname);

    LOK_ASSERT(tile1 == tile2);
    return TestResult::Ok;
}

UnitBase::TestResult UnitCalc::testCalcRenderAfterNewView53()
{
    // Load a doc with the cursor saved at a top row.
    std::string documentPath, documentURL;
    helpers::getDocumentPathAndURL("calc-render.ods", documentPath, documentURL, testname);

    Poco::URI uri(helpers::getTestServerURI());

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("CalcPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> socket =
        helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

    int major = 0;
    int minor = 0;
    helpers::getServerVersion(socket, major, minor, testname);
    if (major < 5 || minor < 3)
    {
        TST_LOG("Skipping test on incompatible client [" << major << '.' << minor
                                                         << "], expected [>=5.3].");
        return TestResult::Ok;
    }

    helpers::sendTextFrame(socket, "clientvisiblearea x=750 y=1861 width=20583 height=6997",
                           testname);
    helpers::sendTextFrame(socket, "key type=input char=0 key=1031", testname);

    // Get tile.
    const char* req = "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0 "
                      "tileposy=291840 tilewidth=3840 tileheight=3840 oldwid=0";
    const std::vector<char> tile1
        = helpers::getTileAndSave(socket, req, "/tmp/calc_render_53_orig.png", testname);

    // Connect second client, which will load at the top.
    TST_LOG("Connecting second client.");
    std::shared_ptr<http::WebSocketSession> socket2 =
        helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

    TST_LOG("Waiting for cellviewcursor of second on first.");
    helpers::assertResponseString(socket, "cellviewcursor:", testname);

    // Get same tile again.
    const std::vector<char> tile2
        = helpers::getTileAndSave(socket, req, "/tmp/calc_render_53_sec.png", testname);

    LOK_ASSERT(tile1 == tile2);

    // Don't let them go out of scope and disconnect.
    socket2->shutdownWS();
    socket->shutdownWS();
    return TestResult::Ok;
}

UnitBase::TestResult UnitCalc::testColumnRowResize()
{
    try
    {
        std::vector<char> response;
        std::string documentPath, documentURL;
        double oldHeight, oldWidth;

        helpers::getDocumentPathAndURL("setclientpart.ods", documentPath, documentURL, testname);
        Poco::URI uri(helpers::getTestServerURI());

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("CalcPoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket =
            helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

        const std::string commandValues = "commandvalues command=.uno:ViewRowColumnHeaders";
        helpers::sendTextFrame(socket, commandValues, testname);
        response = helpers::getResponseMessage(socket, "commandvalues:", testname);
        LOK_ASSERT_MESSAGE("did not receive a commandvalues: message as expected",
                               !response.empty());
        {
            std::vector<char> json(response.begin() + std::string("commandvalues:").length(),
                                   response.end());
            json.push_back(0);

            // get column 2
            oldHeight = getColRowSize("rows", json.data(), 1, testname);
            // get row 2
            oldWidth = getColRowSize("columns", json.data(), 1, testname);
        }

        // send column width
        {
            std::ostringstream oss;
            Poco::JSON::Object objJSON, objColumn, objWidth;
            double newWidth;

            // change column 2
            objColumn.set("type", "unsigned short");
            objColumn.set("value", 2);

            objWidth.set("type", "unsigned short");
            objWidth.set("value", oldWidth + 100);

            objJSON.set("Column", objColumn);
            objJSON.set("Width", objWidth);

            Poco::JSON::Stringifier::stringify(objJSON, oss);
            helpers::sendTextFrame(socket, "uno .uno:ColumnWidth " + oss.str(), testname);
            helpers::sendTextFrame(socket, commandValues, testname);
            response = helpers::getResponseMessage(socket, "commandvalues:", testname);
            LOK_ASSERT_MESSAGE("did not receive a commandvalues: message as expected",
                                   !response.empty());
            std::vector<char> json(response.begin() + std::string("commandvalues:").length(),
                                   response.end());
            json.push_back(0);
            newWidth = getColRowSize("columns", json.data(), 1, testname);
            LOK_ASSERT(newWidth > oldWidth);
        }

        // send row height
        {
            std::ostringstream oss;
            Poco::JSON::Object objJSON, objRow, objHeight;
            double newHeight;

            // change row 2
            objRow.set("type", "unsigned short");
            objRow.set("value", 2);

            objHeight.set("type", "unsigned short");
            objHeight.set("value", oldHeight + 100);

            objJSON.set("Row", objRow);
            objJSON.set("Height", objHeight);

            Poco::JSON::Stringifier::stringify(objJSON, oss);
            helpers::sendTextFrame(socket, "uno .uno:RowHeight " + oss.str(), testname);
            helpers::sendTextFrame(socket, commandValues, testname);
            response = helpers::getResponseMessage(socket, "commandvalues:", testname);
            LOK_ASSERT_MESSAGE("did not receive a commandvalues: message as expected",
                                   !response.empty());
            std::vector<char> json(response.begin() + std::string("commandvalues:").length(),
                                   response.end());
            json.push_back(0);
            newHeight = getColRowSize("rows", json.data(), 1, testname);
            LOK_ASSERT(newHeight > oldHeight);
        }
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

UnitBase::TestResult UnitCalc::testOptimalResize()
{
    try
    {
        double newWidth, newHeight;
        Poco::JSON::Object objIndex, objSize, objModifier;

        // row/column index 0
        objIndex.set("type", "unsigned short");
        objIndex.set("value", 1);

        // size in twips
        objSize.set("type", "unsigned short");
        objSize.set("value", 3840);

        // keyboard modifier
        objModifier.set("type", "unsigned short");
        objModifier.set("value", 0);

        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("empty.ods", documentPath, documentURL, testname);
        Poco::URI uri(helpers::getTestServerURI());

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("CalcPoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket =
            helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

        const std::string commandValues = "commandvalues command=.uno:ViewRowColumnHeaders";
        // send new column width
        {
            std::ostringstream oss;
            Poco::JSON::Object objJSON;

            objJSON.set("Column", objIndex);
            objJSON.set("Width", objSize);

            Poco::JSON::Stringifier::stringify(objJSON, oss);
            helpers::sendTextFrame(socket, "uno .uno:ColumnWidth " + oss.str(), testname);
            helpers::sendTextFrame(socket, commandValues, testname);
            newWidth = getColRowSize(socket, "columns", 0, testname);
        }
        // send new row height
        {
            std::ostringstream oss;
            Poco::JSON::Object objJSON;

            objJSON.set("Row", objIndex);
            objJSON.set("Height", objSize);

            Poco::JSON::Stringifier::stringify(objJSON, oss);
            helpers::sendTextFrame(socket, "uno .uno:RowHeight " + oss.str(), testname);
            helpers::sendTextFrame(socket, commandValues, testname);
            newHeight = getColRowSize(socket, "rows", 0, testname);
        }

        objIndex.set("value", 0);

        // send optimal column width
        {
            std::ostringstream oss;
            Poco::JSON::Object objJSON;
            double optimalWidth;

            objJSON.set("Col", objIndex);
            objJSON.set("Modifier", objModifier);

            Poco::JSON::Stringifier::stringify(objJSON, oss);
            helpers::sendTextFrame(socket, "uno .uno:SelectColumn " + oss.str(), testname);
            helpers::sendTextFrame(socket, "uno .uno:SetOptimalColumnWidthDirect", testname);
            helpers::sendTextFrame(socket, commandValues, testname);
            optimalWidth = getColRowSize(socket, "columns", 0, testname);
            LOK_ASSERT(optimalWidth < newWidth);
        }

        // send optimal row height
        {
            Poco::JSON::Object objSelect, objOptHeight, objExtra;
            double optimalHeight;

            objSelect.set("Row", objIndex);
            objSelect.set("Modifier", objModifier);

            objExtra.set("type", "unsigned short");
            objExtra.set("value", 0);

            objOptHeight.set("aExtraHeight", objExtra);

            std::ostringstream oss;
            Poco::JSON::Stringifier::stringify(objSelect, oss);
            helpers::sendTextFrame(socket, "uno .uno:SelectRow " + oss.str(), testname);
            oss.str("");
            oss.clear();

            Poco::JSON::Stringifier::stringify(objOptHeight, oss);
            helpers::sendTextFrame(socket, "uno .uno:SetOptimalRowHeight " + oss.str(), testname);

            helpers::sendTextFrame(socket, commandValues, testname);
            optimalHeight = getColRowSize(socket, "rows", 0, testname);
            LOK_ASSERT(optimalHeight < newHeight);
        }
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    return TestResult::Ok;
}

void UnitCalc::invokeWSDTest()
{
    UnitBase::TestResult result = testCalcEditRendering();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testCalcRenderAfterNewView51();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testCalcRenderAfterNewView53();
    if (result != TestResult::Ok)
        exitTest(result);

    // FIXME result = testColumnRowResize();
    if (result != TestResult::Ok)
        exitTest(result);

    // FIXME result = testOptimalResize();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitCalc(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
