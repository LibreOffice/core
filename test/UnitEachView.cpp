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
 * Unit test for multi-view functionality.
 */

#include <config.h>

#include <memory>
#include <sstream>
#include <string>

#include <Poco/URI.h>
#include <test/lokassert.hpp>

#include <Unit.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>

using namespace std::literals;

namespace
{
void testEachView(const std::string& doc, const std::string& type, const std::string& protocol,
                  const std::string& protocolView, const std::string& testname)
{
    TST_LOG("testEachView for " << testname);

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("UnitEachView");
    socketPoll->startThread();

    try
    {
        // Load a document
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL(doc, documentPath, documentURL, testname);

        TST_LOG("Loading " << documentURL);
        int itView = 0;

        std::ostringstream ossView;
        ossView << testname << "view " << itView << " -> ";
        const std::string view = ossView.str();

        std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
            socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, view);

        // Check document size
        helpers::sendTextFrame(socket, "status", view);
        auto response = helpers::assertResponseString(socket, "status:", view);
        std::string docPart;
        int docParts = 0;
        int docHeight = 0;
        int docWidth = 0;
        int docViewId = -1;
        helpers::parseDocSize(response.substr(7), type, docPart, docParts, docWidth, docHeight,
                              docViewId, testname);

        // Send click message
        std::ostringstream ossButtondown;
        ossButtondown << "mouse type=buttondown x=" << docWidth / 2 << " y=" << docHeight / 6
                      << " count=1 buttons=1 modifier=0";
        helpers::sendTextFrame(socket, ossButtondown.str(), view);

        std::ostringstream ossButtonup;
        ossButtonup << "mouse type=buttonup x=" << docWidth / 2 << " y=" << docHeight / 6
                    << " count=1 buttons=1 modifier=0";
        helpers::sendTextFrame(socket, ossButtonup.str(), view);
        // Double of the default.
        constexpr std::chrono::milliseconds timeoutMs{ 20000 };
        response = helpers::getResponseString(socket, protocol, view, timeoutMs);

        std::ostringstream ossError;
        ossError << testname << "view " << itView << ", did not receive a " << protocol
                 << " message as expected";
        LOK_ASSERT_MESSAGE(ossError.str(), !response.empty());

        // Connect and load 0..N Views, where N<=limit
        std::vector<std::shared_ptr<http::WebSocketSession>> views;
        static_assert(MAX_DOCUMENTS >= 2, "MAX_DOCUMENTS must be at least 2");
        const int limit = std::min(4, MAX_DOCUMENTS - 1); // +1 connection above
        for (itView = 0; itView < limit; ++itView)
        {
            TST_LOG("loadDocAndGetSession #" << (itView + 1) << ": " << documentURL);
            std::ostringstream oss;
            oss << testname << "view " << itView << " -> ";
            views.emplace_back(helpers::loadDocAndGetSession(
                socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, oss.str()));
        }

        // main view should receive response each view
        if (protocolView == "invalidateviewcursor:" || protocolView == "viewcursorvisible:"
            || protocolView == "cellviewcursor:" || protocolView == "graphicviewselection:")
        {
            return;
        }
        itView = 0;
        for (const auto& socketView : views)
        {
            TST_LOG("getResponse #" << (itView + 1) << ": " << protocolView);
            std::ostringstream oss;
            oss << testname << "view " << itView << " -> ";
            response = helpers::getResponseString(socket, protocolView, oss.str(), timeoutMs);

            std::ostringstream ossSocketViewError;
            ossSocketViewError << testname << "view " << itView << ", did not receive a "
                               << protocol << " message as expected";
            LOK_ASSERT_MESSAGE(ossSocketViewError.str(), !response.empty());
            ++itView;
            (void)socketView;
        }
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    catch (const std::exception& exc)
    {
        LOK_ASSERT_FAIL(exc.what());
    }

    TST_LOG("Done testEachView for " << testname);
}
}

/// Test suite that asserts a state for each view.
class UnitEachView : public UnitWSD
{
    TestResult testInvalidateViewCursor();
    TestResult testViewCursorVisible();
    TestResult testCellViewCursor();
    TestResult testGraphicViewSelectionWriter();
    TestResult testGraphicViewSelectionCalc();
    TestResult testGraphicViewSelectionImpress();

public:
    UnitEachView();
    void invokeWSDTest() override;
};

UnitBase::TestResult UnitEachView::testInvalidateViewCursor()
{
    testEachView("viewcursor.odp", "presentation",
                 "invalidatecursor:", "invalidateviewcursor:", "invalidateViewCursor ");
    return TestResult::Ok;
}

UnitBase::TestResult UnitEachView::testViewCursorVisible()
{
    testEachView("viewcursor.odp", "presentation",
                 "cursorvisible:", "viewcursorvisible:", "viewCursorVisible ");
    return TestResult::Ok;
}

UnitBase::TestResult UnitEachView::testCellViewCursor()
{
    testEachView("empty.ods", "spreadsheet", "cellcursor:", "cellviewcursor:", "cellViewCursor ");
    return TestResult::Ok;
}

UnitBase::TestResult UnitEachView::testGraphicViewSelectionWriter()
{
    testEachView("graphicviewselection.odt", "text",
                 "graphicselection:", "graphicviewselection:", "graphicViewSelection-odt ");
    return TestResult::Ok;
}

UnitBase::TestResult UnitEachView::testGraphicViewSelectionCalc()
{
    testEachView("graphicviewselection.ods", "spreadsheet",
                 "graphicselection:", "graphicviewselection:", "graphicViewSelection-ods ");
    return TestResult::Ok;
}

UnitBase::TestResult UnitEachView::testGraphicViewSelectionImpress()
{
    testEachView("graphicviewselection.odp", "presentation",
                 "graphicselection:", "graphicviewselection:", "graphicViewSelection-odp ");
    return TestResult::Ok;
}

UnitEachView::UnitEachView()
    : UnitWSD("UnitEachView")
{
    // 8 times larger then the default.
    setTimeout(240s);
}

void UnitEachView::invokeWSDTest()
{
    UnitBase::TestResult result = testInvalidateViewCursor();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testViewCursorVisible();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testCellViewCursor();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testGraphicViewSelectionWriter();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testGraphicViewSelectionCalc();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testGraphicViewSelectionImpress();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitEachView(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
