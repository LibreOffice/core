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
 * Unit test for shape rendering functionality.
 */

#include <config.h>

#include <memory>
#include <string>

#include <Poco/URI.h>
#include <test/lokassert.hpp>

#include <Unit.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>

using namespace std::literals;

namespace
{
/**
 * Strips <desc>...</desc> strings from an SVG, some of which are only in debug builds, so breaks
 * comparison with a fixed reference.
 */
void stripDescriptions(std::vector<char>& svg)
{
    static const std::string startDesc("<desc>");
    static const std::string endDesc("</desc>");
    static const std::string selfClose("/>");

    while (true)
    {
        const auto itStart
            = std::search(svg.begin(), svg.end(), startDesc.begin(), startDesc.end());
        if (itStart == svg.end())
            return;

        const auto itClose
            = std::search(itStart + 1, svg.end(), selfClose.begin(), selfClose.end());

        const auto itEnd = std::search(itStart + 1, svg.end(), endDesc.begin(), endDesc.end());

        if (itEnd != svg.end() && itClose != svg.end())
        {
            if (itEnd < itClose)
                svg.erase(itStart, itEnd + endDesc.size());
            else
                svg.erase(itStart, itClose + selfClose.size());
        }
        else if (itEnd != svg.end())
        {
            svg.erase(itStart, itEnd + endDesc.size());
        }
        else if (itClose != svg.end())
        {
            svg.erase(itStart, itClose + selfClose.size());
        }
        else
        {
            // No more closing tags; possibly broken, as we found an opening tag.
            return;
        }
    }
}
}

/// Render shape testcase.
class UnitRenderShape : public UnitWSD
{
    TestResult testRenderShapeSelectionImpress();
    TestResult testRenderShapeSelectionWriterImage();

public:
    UnitRenderShape()
        : UnitWSD("UnitRenderShape")
    {
    }

    void invokeWSDTest() override;
};

UnitBase::TestResult UnitRenderShape::testRenderShapeSelectionImpress()
{
    try
    {
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("shapes.odp", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("RenderShapePoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
            socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

        int major = 0;
        int minor = 0;
        helpers::getServerVersion(socket, major, minor, testname);
        if (major != 6 || minor != 0)
        {
            TST_LOG("Skipping test on incompatible client [" << major << '.' << minor
                                                             << "], expected [6.0].");
            return TestResult::Ok;
        }

        helpers::selectAll(socket, testname);
        std::this_thread::sleep_for(250ms);
        helpers::sendTextFrame(socket, "rendershapeselection mimetype=image/svg+xml", testname);
        std::vector<char> responseSVG
            = helpers::getResponseMessage(socket, "shapeselectioncontent:", testname);
        LOK_ASSERT(!responseSVG.empty());
        auto it = std::find(responseSVG.begin(), responseSVG.end(), '\n');
        if (it != responseSVG.end())
            responseSVG.erase(responseSVG.begin(), ++it);

        stripDescriptions(responseSVG);

        LOK_ASSERT(helpers::svgMatch(testname, responseSVG, "shapes_impress.svg"));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }

    return TestResult::Ok;
}

UnitBase::TestResult UnitRenderShape::testRenderShapeSelectionWriterImage()
{
    try
    {
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("non-shape-image.odt", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("RenderShapePoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
            socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

        // Select the shape with SHIFT + F4
        helpers::sendKeyPress(socket, 0, 771 | helpers::SpecialKey::skShift, testname);
        std::this_thread::sleep_for(250ms);
        helpers::sendTextFrame(socket, "rendershapeselection mimetype=image/svg+xml", testname);
        std::vector<char> responseSVG
            = helpers::getResponseMessage(socket, "shapeselectioncontent:", testname);
        LOK_ASSERT(!responseSVG.empty());
        auto it = std::find(responseSVG.begin(), responseSVG.end(), '\n');
        if (it != responseSVG.end())
            responseSVG.erase(responseSVG.begin(), ++it);

        stripDescriptions(responseSVG);

        LOK_ASSERT(helpers::svgMatch(testname, responseSVG, "non_shape_writer_image.svg"));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }

    return TestResult::Ok;
}

void UnitRenderShape::invokeWSDTest()
{
    UnitBase::TestResult result = testRenderShapeSelectionImpress();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testRenderShapeSelectionWriterImage();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitRenderShape(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
