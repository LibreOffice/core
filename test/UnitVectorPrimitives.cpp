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

    socketPoll->joinThread();
    return TestResult::Ok;
}

void UnitVectorPrimitives::invokeWSDTest()
{
    UnitBase::TestResult result = testDeltaReachesEverySessionHoldingThePart();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitVectorPrimitives(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
