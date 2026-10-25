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

#include <config.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <Poco/URI.h>

#include <common/Unit.hpp>
#include <net/Socket.hpp>

#include <lokassert.hpp>
#include <helpers.hpp>

using namespace std::literals;

namespace
{
/// The bytes of a frame after its first line.
std::vector<char> bodyOf(const std::vector<char>& frame)
{
    const auto newline = std::find(frame.begin(), frame.end(), '\n');
    if (newline == frame.end())
        return {};
    return std::vector<char>(newline + 1, frame.end());
}
}

/// Covers how the kit hands the vector primitives of a part to the sessions that hold it.
class UnitVectorPrimitives : public UnitWSD
{
    TestResult testDeltaReachesEverySessionHoldingThePart();
    TestResult testMousePointerIsSentOnlyWhenWanted();

public:
    UnitVectorPrimitives();
    void invokeWSDTest() override;

    bool onDataLoss(const std::string& reason) override
    {
        // The slide is changed to bring a delta about, and never saved.
        TST_LOG("onDataLoss (expected): " << reason);
        return false;
    }
};

UnitVectorPrimitives::UnitVectorPrimitives()
    : UnitWSD("UnitVectorPrimitives")
{
}

// A change to a slide is pushed once as a delta, and every session that asked for the slide's
// primitives gets the same compressed bytes. A session that draws from tiles never asked for
// them and gets no delta.
UnitBase::TestResult UnitVectorPrimitives::testDeltaReachesEverySessionHoldingThePart()
{
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("hello.odp", documentPath, documentURL, testname);
    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("VectorPrimitivesPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> editor = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);
    std::shared_ptr<http::WebSocketSession> reader = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);
    std::shared_ptr<http::WebSocketSession> tiles = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

    // The first slide, named by its page id as the status carries it.
    helpers::sendTextFrame(editor, "status", testname);
    const std::string status = helpers::assertResponseString(editor, "status:", testname);
    const std::vector<std::string> partIds = helpers::parsePartUniqueIds(status.substr(7));
    LOK_ASSERT(!partIds.empty());
    const std::string partId = partIds[0];

    // Two sessions ask for the primitives of the slide and hold it from then on.
    const std::string request = "commandvalues command=.uno:VectorPrimitives?partid=" + partId
                                + "&mode=0";
    for (const auto& session : { editor, reader })
    {
        helpers::sendTextFrame(session, request, testname);
        const std::vector<char> answer =
            helpers::getResponseMessage(session, "zstdvectorprimitives:", testname);
        LOK_ASSERT_MESSAGE("no primitives were answered", !answer.empty());
    }

    // Removing every object of the slide changes it, so a delta is pushed.
    helpers::sendTextFrame(editor, "uno .uno:SelectAll", testname);
    helpers::sendTextFrame(editor, "uno .uno:Delete", testname);

    const std::vector<char> toEditor =
        helpers::getResponseMessage(editor, "zstdvectorprimitivesdelta:", testname);
    LOK_ASSERT_MESSAGE("the session that changed the slide got no delta", !toEditor.empty());
    const std::vector<char> toReader =
        helpers::getResponseMessage(reader, "zstdvectorprimitivesdelta:", testname);
    LOK_ASSERT_MESSAGE("the other session holding the slide got no delta", !toReader.empty());

    // The header line names each session's own input sequence, the bytes after it are the one
    // delta compressed once.
    LOK_ASSERT(!bodyOf(toEditor).empty());
    LOK_ASSERT(bodyOf(toEditor) == bodyOf(toReader));

    const std::vector<char> toTiles =
        helpers::getResponseMessage(tiles, "zstdvectorprimitivesdelta:", testname, 2s);
    LOK_ASSERT_MESSAGE("a session drawing from tiles got a delta", toTiles.empty());

    // Each view flushes the invalidation of the same slide, but the change is one delta. The
    // view that flushes after it finds the slide unchanged since the push and writes no frame.
    const std::vector<char> again =
        helpers::getResponseMessage(editor, "zstdvectorprimitivesdelta:", testname, 2s);
    LOK_ASSERT_MESSAGE("one change brought a second delta", again.empty());

    socketPoll->joinThread();
    return TestResult::Ok;
}

// A session that draws the slide from the geometry it holds works out the pointer itself and
// asks for no mousepointer frames. Asking for them again brings them back.
UnitBase::TestResult UnitVectorPrimitives::testMousePointerIsSentOnlyWhenWanted()
{
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("hello.odp", documentPath, documentURL, testname);
    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("MousePointerPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> session = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

    // The title of the first slide spans 1.4cm to 26.6cm across and 0.8cm to 4.3cm down, and
    // the slide is 21cm high, so one point is in the title and the other is below the subtitle.
    const std::string overTitle = "mouse type=move x=7937 y=1467 count=1 buttons=0 modifier=0";
    const std::string belowAll = "mouse type=move x=7937 y=10772 count=1 buttons=0 modifier=0";

    // Frames are wanted by default. Loading reports the default pointer, and over the text of
    // the title the pointer becomes the text pointer.
    helpers::sendTextFrame(session, overTitle, testname);
    std::string overText;
    for (int frames = 0; frames < 3 && overText != "mousepointer: text"; ++frames)
        overText = helpers::getResponseString(session, "mousepointer:", testname);
    LOK_ASSERT_EQUAL(std::string("mousepointer: text"), overText);

    // Pointer frames are turned off, and leaving the object brings none.
    helpers::sendTextFrame(session, "reportmousepointer wanted=false", testname);
    helpers::sendTextFrame(session, belowAll, testname);
    const std::string whileOff =
        helpers::getResponseString(session, "mousepointer:", testname, 2s);
    LOK_ASSERT_MESSAGE("a pointer frame came while they were not wanted", whileOff.empty());

    // Turned on again, the next move reports the pointer that the client last missed.
    helpers::sendTextFrame(session, "reportmousepointer wanted=true", testname);
    helpers::sendTextFrame(session, belowAll, testname);
    const std::string whenOn =
        helpers::getResponseString(session, "mousepointer:", testname);
    LOK_ASSERT_EQUAL(std::string("mousepointer: default"), whenOn);

    socketPoll->joinThread();
    return TestResult::Ok;
}

void UnitVectorPrimitives::invokeWSDTest()
{
    UnitBase::TestResult result = testDeltaReachesEverySessionHoldingThePart();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testMousePointerIsSentOnlyWhenWanted();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitVectorPrimitives(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
