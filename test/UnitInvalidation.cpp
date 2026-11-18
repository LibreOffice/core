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
 * Unit test for tile invalidation functionality.
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

/// Rendering options testcase.
class UnitInvalidation : public UnitWSD
{
    void setupSession(const std::shared_ptr<http::WebSocketSession> &session);
    void renderArea(const std::shared_ptr<http::WebSocketSession> &session, int part);
public:
    UnitInvalidation() : UnitWSD("UnitInvalidation")
    {
    }

    void invokeWSDTest() override;
};


void UnitInvalidation::setupSession(const std::shared_ptr<http::WebSocketSession> &session)
{
    // Setup a small calc window
    helpers::sendTextFrame(session, "useractive", testname);
    helpers::sendTextFrame(session, "a11ystate false", testname);
    helpers::sendTextFrame(session, "uno .uno:ToolbarMode?Mode:string=notebookbar_online.ui", testname);
    helpers::sendTextFrame(session, "clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=2560 tiletwipheight=2560 dpiscale=1.5 zoom=10", testname);
    helpers::sendTextFrame(session, "clientvisiblearea x=0 y=0 width=5120 height=5120 splitx=0 splity=0", testname);
    helpers::sendTextFrame(session, "commandvalues command=.uno:CellCursor?outputHeight=256&outputWidth=256&tileHeight=2560&tileWidth=2560", testname);

    // Let the app get somewhat setup ...
    TST_LOG("settling ...");
    helpers::drain(session, testname, 2s);
}

void UnitInvalidation::renderArea(const std::shared_ptr<http::WebSocketSession> &session, int part)
{
        // render a small area
    helpers::sendTextFrame(session, "tilecombine nviewid=0 part=" + std::to_string(part) +
                           " width=256 height=256 "
                           "tileposx=0,2560,5120,0,2560,5120,0,2560,5120 "
                           "tileposy=0,0,0,2560,2560,2560,5120,5120,5120 "
                           "oldwid=0,0,0,0,0,0,0,0,0 tilewidth=2560 tileheight=2560", testname);

        std::vector<std::string> wids;
        for (int n = 0; n < 9; ++n)
        {
            std::string msg = helpers::getResponseString(session, "tile:", testname);
            LOK_ASSERT(msg.length() > 0);
            auto vec = StringVector::tokenize(msg);
            for (size_t i = 0; i < vec.size(); ++i)
            {
                if (vec.startsWith(i, "wid="))
                    wids.push_back(vec[i].substr(4, vec[i].length() - 4));
            }
        }

        helpers::sendTextFrame(session, "tileprocessed wids=" + Util::joinPair(wids, ","),
                               testname);
}

void UnitInvalidation::invokeWSDTest()
{
    try
    {
        // Load a document with two sheets/parts.
        // Window one renders part 0.
        // Window two gets created.
        // Window one switches to part 1.
        // Window one changes part 1 B2, part 0 refers to this cell.
        // Window two should get an invalidate because of the reference.
        std::string documentPath;
        std::string documentURL;
        helpers::getDocumentPathAndURL("invalidate.fods", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("MainPoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> windowOne = helpers::loadDocAndGetSession(
            socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

        TST_LOG("Rendering window one");

        setupSession(windowOne);
        // ensure all tiles hit the cache and made it to us.
        renderArea(windowOne, 0);

        TST_LOG("Rendering window two");

        std::shared_ptr<http::WebSocketSession> windowTwo = helpers::loadDocAndGetSession(
            socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);
        setupSession(windowTwo);
        // ensure all the same tiles are served from the cache, and not the COKit view.
        renderArea(windowTwo, 0);

        TST_LOG("Tab switch and edit: window one");

        // Switch to sheet two in window one
        helpers::sendTextFrame(windowOne, "setclientpart part=1", testname);
        setupSession(windowOne); // for good measure

        // Get some tiles here too for good measure
        renderArea(windowOne, 1);

        helpers::sendTextFrame(windowOne, "mouse type=move x=1830 y=475 count=1 buttons=0 modifier=0", testname);
        helpers::sendTextFrame(windowOne, "mouse type=buttondown x=1830 y=475 count=1 buttons=1 modifier=0", testname);
        helpers::sendTextFrame(windowOne, "mouse type=buttonup   x=1830 y=475 count=1 buttons=1 modifier=0", testname);
        helpers::sendTextFrame(windowOne, "mouse type=buttondown x=1830 y=475 count=2 buttons=1 modifier=0", testname);
        helpers::sendTextFrame(windowOne, "mouse type=buttonup   x=1830 y=475 count=2 buttons=1 modifier=0", testname);

        std::string message;

        // Select cell B2 and start editing
        message = helpers::getResponseString(windowOne, "celladdress:", testname);
        LOK_ASSERT_EQUAL(message, std::string("celladdress: B2"));

        message = helpers::getResponseString(windowOne, "cursorvisible:", testname);
        LOK_ASSERT_EQUAL(message, std::string("cursorvisible: true"));

        // Enter a number of '1's + enter
        helpers::sendTextFrame(windowOne, "textinput id=0 text=11111111", testname);
        helpers::sendTextFrame(windowOne, "key type=input char=13 key=1280", testname);
        helpers::sendTextFrame(windowOne, "key type=up char=0 key=1280", testname);

        TST_LOG("Wait for window one invalidations");
        std::vector<std::string> invalidates;

        invalidates = helpers::getAllResponsesTimed(windowOne, "invalidatetiles:", testname, 2s);
        LOK_ASSERT(invalidates.size() > 0);

        bool invalidated = false;
        for (auto &i : invalidates)
        {
            if (i.starts_with("invalidatetiles: part=1"))
            {
                invalidated = true;
                break;
            }
        }
        LOK_ASSERT(invalidated);
        // We expect invalidates on both part 1 (current) and 0 (dependent)

        TST_LOG("Wait for window two invalidations");

        // Notice no invalidation on window two
        invalidates = helpers::getAllResponsesTimed(windowTwo, "invalidatetiles:", testname, 2s);

        // We expect to get an invalidate for our current view, due to the dependency updating.
        LOK_ASSERT(invalidates.size() > 0);
        invalidated = false;
        for (auto &i : invalidates)
        {
            if (i.starts_with("invalidatetiles: part=0"))
            {
                invalidated = true;
                break;
            }
        }
        LOK_ASSERT(invalidated);
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitInvalidation(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
