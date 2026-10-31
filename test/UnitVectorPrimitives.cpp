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

#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>

#include <zstd.h>

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

/// The JSON object that a zstd-compressed vector primitives frame carries, or nullptr when the
/// frame holds none.
Poco::JSON::Object::Ptr jsonOf(const std::vector<char>& frame)
{
    const std::vector<char> body = bodyOf(frame);
    const unsigned long long size = ZSTD_getFrameContentSize(body.data(), body.size());
    if (size == ZSTD_CONTENTSIZE_ERROR || size == ZSTD_CONTENTSIZE_UNKNOWN)
        return nullptr;
    std::string json(size, '\0');
    const std::size_t length = ZSTD_decompress(json.data(), json.size(), body.data(), body.size());
    if (ZSTD_isError(length))
        return nullptr;
    json.resize(length);
    Poco::JSON::Parser parser;
    return parser.parse(json).extract<Poco::JSON::Object::Ptr>();
}

/// The ids of the pages of a text document, as its status names them.
std::vector<std::string> pageIdsOf(const std::string& status)
{
    Poco::JSON::Parser parser;
    const Poco::JSON::Object::Ptr object
        = parser.parse(status).extract<Poco::JSON::Object::Ptr>();
    std::vector<std::string> ids;
    if (const Poco::JSON::Array::Ptr pageIds = object->getArray("pageids"))
    {
        for (std::size_t i = 0; i < pageIds->size(); ++i)
            ids.push_back(pageIds->get(i).toString());
    }
    return ids;
}
}

/// Covers how the kit hands the vector primitives of a part to the sessions that hold it.
class UnitVectorPrimitives : public UnitWSD
{
    TestResult testDeltaReachesEverySessionHoldingThePart();
    TestResult testMousePointerIsSentOnlyWhenWanted();
    TestResult testTextPageDeltaStopsWhenForgotten();
    TestResult testTextPageDeltaReachesTheSessionThatKeepsIt();
    TestResult testTextPageIsForgottenWhenItsLastHolderLeaves();
    TestResult testRequestForPrimitivesNamesOnePart();
    TestResult testRequestForPrimitivesHasTheWholeCommandName();
    TestResult testSessionHoldsThePartsTheDocumentServed();
    TestResult testDrawingFromTilesForgetsTheParts();

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

// A session that asked for a page of a text document is pushed a delta when typing changes the
// page. Once it says it forgot the page, typing brings it no more deltas.
UnitBase::TestResult UnitVectorPrimitives::testTextPageDeltaStopsWhenForgotten()
{
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("TextPagePoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> session = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

    helpers::sendTextFrame(session, "status", testname);
    const std::string status = helpers::assertResponseString(session, "status:", testname);
    const std::vector<std::string> pageIds = pageIdsOf(status.substr(7));
    LOK_ASSERT(!pageIds.empty());

    helpers::sendTextFrame(session,
                           "commandvalues command=.uno:VectorPrimitives?partid=" + pageIds[0],
                           testname);
    const std::vector<char> answer =
        helpers::getResponseMessage(session, "zstdvectorprimitives:", testname);
    LOK_ASSERT_MESSAGE("no primitives were answered for the page", !answer.empty());

    helpers::sendChar(session, 'x', helpers::SpecialKey::skNone, testname);
    const std::vector<char> delta =
        helpers::getResponseMessage(session, "zstdvectorprimitivesdelta:", testname);
    LOK_ASSERT_MESSAGE("typing brought no delta for the page", !delta.empty());

    helpers::sendTextFrame(session,
                           "commandvalues command=.uno:VectorForgetPart?partid=" + pageIds[0],
                           testname);
    helpers::sendChar(session, 'y', helpers::SpecialKey::skNone, testname);
    const std::vector<char> afterForget =
        helpers::getResponseMessage(session, "zstdvectorprimitivesdelta:", testname, 2s);
    LOK_ASSERT_MESSAGE("a forgotten page brought a delta", afterForget.empty());

    socketPoll->joinThread();
    return TestResult::Ok;
}

// Two sessions hold a page of a text document. When one forgets the page or leaves, the other
// still gets the deltas of typing on it.
UnitBase::TestResult UnitVectorPrimitives::testTextPageDeltaReachesTheSessionThatKeepsIt()
{
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("KeptPagePoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> keeper = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);
    std::shared_ptr<http::WebSocketSession> leaver = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

    helpers::sendTextFrame(keeper, "status", testname);
    const std::string status = helpers::assertResponseString(keeper, "status:", testname);
    const std::vector<std::string> pageIds = pageIdsOf(status.substr(7));
    LOK_ASSERT(!pageIds.empty());

    const std::string request = "commandvalues command=.uno:VectorPrimitives?partid=" + pageIds[0];
    for (const auto& session : { keeper, leaver })
    {
        helpers::sendTextFrame(session, request, testname);
        const std::vector<char> answer =
            helpers::getResponseMessage(session, "zstdvectorprimitives:", testname);
        LOK_ASSERT_MESSAGE("no primitives were answered for the page", !answer.empty());
    }

    // The page stays held by the keeper after the other session forgets it.
    helpers::sendTextFrame(leaver,
                           "commandvalues command=.uno:VectorForgetPart?partid=" + pageIds[0],
                           testname);
    helpers::sendChar(keeper, 'x', helpers::SpecialKey::skNone, testname);
    const std::vector<char> afterForget =
        helpers::getResponseMessage(keeper, "zstdvectorprimitivesdelta:", testname);
    LOK_ASSERT_MESSAGE("the session that kept the page got no delta", !afterForget.empty());
    const std::vector<char> toForgetter =
        helpers::getResponseMessage(leaver, "zstdvectorprimitivesdelta:", testname, 2s);
    LOK_ASSERT_MESSAGE("the session that forgot the page got a delta", toForgetter.empty());

    // The other session holds the page again and then leaves the document.
    helpers::sendTextFrame(leaver, request, testname);
    const std::vector<char> again =
        helpers::getResponseMessage(leaver, "zstdvectorprimitives:", testname);
    LOK_ASSERT_MESSAGE("no primitives were answered for the page again", !again.empty());
    leaver->asyncShutdown();
    LOK_ASSERT(leaver->waitForDisconnection(10s));

    helpers::sendChar(keeper, 'y', helpers::SpecialKey::skNone, testname);
    const std::vector<char> afterLeave =
        helpers::getResponseMessage(keeper, "zstdvectorprimitivesdelta:", testname);
    LOK_ASSERT_MESSAGE("the session that stayed got no delta", !afterLeave.empty());

    socketPoll->joinThread();
    return TestResult::Ok;
}

// When the last session that holds a page of a text document leaves, the document forgets the
// page. The version that session held is then unknown to the document, and a request since that
// version is answered with the whole page.
UnitBase::TestResult UnitVectorPrimitives::testTextPageIsForgottenWhenItsLastHolderLeaves()
{
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("LeftPagePoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> stayer = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);
    std::shared_ptr<http::WebSocketSession> leaver = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

    helpers::sendTextFrame(stayer, "status", testname);
    const std::string status = helpers::assertResponseString(stayer, "status:", testname);
    const std::vector<std::string> pageIds = pageIdsOf(status.substr(7));
    LOK_ASSERT(!pageIds.empty());

    const std::string request = "commandvalues command=.uno:VectorPrimitives?partid=" + pageIds[0];
    helpers::sendTextFrame(leaver, request, testname);
    const Poco::JSON::Object::Ptr served =
        jsonOf(helpers::getResponseMessage(leaver, "zstdvectorprimitives:", testname));
    LOK_ASSERT_MESSAGE("no primitives were answered for the page", !served.isNull());
    const std::string version = served->get("version").toString();
    const std::string epoch = served->get("epoch").toString();

    // The view lists sent so far are taken, so the next one is the list without the leaver.
    helpers::getAllResponsesTimed(stayer, "viewinfo:", testname, 1s);
    leaver->asyncShutdown();
    LOK_ASSERT(leaver->waitForDisconnection(10s));
    LOK_ASSERT(!helpers::getResponseString(stayer, "viewinfo:", testname).empty());

    helpers::sendTextFrame(stayer, request + "&since=" + version + "&epoch=" + epoch, testname);
    const Poco::JSON::Object::Ptr answer =
        jsonOf(helpers::getResponseMessage(stayer, "zstdvectorprimitives:", testname));
    LOK_ASSERT_MESSAGE("no primitives were answered for the page", !answer.isNull());
    LOK_ASSERT_EQUAL(std::string("vectorprimitives"), answer->get("type").toString());

    socketPoll->joinThread();
    return TestResult::Ok;
}

// A request for the primitives of a part names one part. A request of any other form is answered
// with a syntax error and serves no primitives.
UnitBase::TestResult UnitVectorPrimitives::testRequestForPrimitivesNamesOnePart()
{
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("OnePartPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> session = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

    helpers::sendTextFrame(session, "status", testname);
    const std::string status = helpers::assertResponseString(session, "status:", testname);
    const std::vector<std::string> pageIds = pageIdsOf(status.substr(7));
    LOK_ASSERT(!pageIds.empty());

    helpers::sendTextFrame(session,
                           "commandvalues command=.uno:VectorPrimitives?partid=" + pageIds[0]
                               + "&partid=" + pageIds[0],
                           testname);
    const std::string error = helpers::getResponseString(session, "error:", testname);
    LOK_ASSERT_EQUAL(std::string("error: cmd=commandvalues kind=syntax"), error);
    const std::vector<char> answer =
        helpers::getResponseMessage(session, "zstdvectorprimitives:", testname, 2s);
    LOK_ASSERT_MESSAGE("primitives were answered for the request", answer.empty());

    socketPoll->joinThread();
    return TestResult::Ok;
}

// A request for the primitives of a part has the whole command name before its parameters. A
// command with a longer name is another command: it serves no primitives, and the session draws
// from tiles as before, so typing brings it the tile that changed.
UnitBase::TestResult UnitVectorPrimitives::testRequestForPrimitivesHasTheWholeCommandName()
{
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("WholeNamePoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> session = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

    helpers::sendTextFrame(session, "status", testname);
    const std::string status = helpers::assertResponseString(session, "status:", testname);
    const std::vector<std::string> pageIds = pageIdsOf(status.substr(7));
    LOK_ASSERT(!pageIds.empty());

    // The session holds the tile at the top left of the document and says what it shows.
    helpers::sendTextFrame(session,
                           "tilecombine nviewid=0 part=0 width=256 height=256 "
                           "tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840",
                           testname);
    const std::vector<char> tile = helpers::getResponseMessage(session, "tile:", testname);
    LOK_ASSERT_MESSAGE("no tile was answered", !tile.empty());
    helpers::sendTextFrame(session, "clientvisiblearea x=-4005 y=0 width=50490 height=72300",
                           testname);
    helpers::sendTextFrame(
        session,
        "clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=3840 tiletwipheight=3840",
        testname);

    helpers::sendTextFrame(session,
                           "commandvalues command=.uno:VectorPrimitivesOfPage?partid="
                               + pageIds[0],
                           testname);
    const std::vector<char> answer =
        helpers::getResponseMessage(session, "zstdvectorprimitives:", testname, 2s);
    LOK_ASSERT_MESSAGE("primitives were answered for another command", answer.empty());

    // Typing changes the tile, and the session is sent the change.
    helpers::sendChar(session, 'x', helpers::SpecialKey::skNone, testname);
    helpers::assertResponseString(session, "invalidatetiles:", testname);
    helpers::assertResponseString(session, "delta:", testname);

    helpers::sendChar(session, 'y', helpers::SpecialKey::skNone, testname);
    const std::vector<char> delta =
        helpers::getResponseMessage(session, "zstdvectorprimitivesdelta:", testname, 2s);
    LOK_ASSERT_MESSAGE("a session that holds no page got a delta", delta.empty());

    socketPoll->joinThread();
    return TestResult::Ok;
}

// A session holds the parts the document served it. A request with an id that names no page is
// answered with the header alone, without a version, and typing then brings the session no delta.
UnitBase::TestResult UnitVectorPrimitives::testSessionHoldsThePartsTheDocumentServed()
{
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("ServedPartsPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> session = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

    helpers::sendTextFrame(session,
                           "commandvalues command=.uno:VectorPrimitives?partid=nopage",
                           testname);
    const Poco::JSON::Object::Ptr answer =
        jsonOf(helpers::getResponseMessage(session, "zstdvectorprimitives:", testname));
    LOK_ASSERT_MESSAGE("no answer came for the request", !answer.isNull());
    LOK_ASSERT_MESSAGE("the answer for no page carries a version", !answer->has("version"));

    helpers::sendChar(session, 'x', helpers::SpecialKey::skNone, testname);
    const std::vector<char> delta =
        helpers::getResponseMessage(session, "zstdvectorprimitivesdelta:", testname, 2s);
    LOK_ASSERT_MESSAGE("a session that holds no page got a delta", delta.empty());

    socketPoll->joinThread();
    return TestResult::Ok;
}

// A session that draws from tiles again forgets the pages it held. Typing brings it no more
// deltas, and the document forgets a page no other session holds: the version the session held is
// then unknown to the document, and a request since that version is answered with the whole page.
UnitBase::TestResult UnitVectorPrimitives::testDrawingFromTilesForgetsTheParts()
{
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);
    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("FromTilesPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> session = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

    helpers::sendTextFrame(session, "status", testname);
    const std::string status = helpers::assertResponseString(session, "status:", testname);
    const std::vector<std::string> pageIds = pageIdsOf(status.substr(7));
    LOK_ASSERT(!pageIds.empty());

    const std::string request = "commandvalues command=.uno:VectorPrimitives?partid=" + pageIds[0];
    helpers::sendTextFrame(session, request, testname);
    const Poco::JSON::Object::Ptr served =
        jsonOf(helpers::getResponseMessage(session, "zstdvectorprimitives:", testname));
    LOK_ASSERT_MESSAGE("no primitives were answered for the page", !served.isNull());
    const std::string version = served->get("version").toString();
    const std::string epoch = served->get("epoch").toString();

    helpers::sendTextFrame(session, "commandvalues command=.uno:DrawFromTiles", testname);
    helpers::sendChar(session, 'x', helpers::SpecialKey::skNone, testname);
    const std::vector<char> delta =
        helpers::getResponseMessage(session, "zstdvectorprimitivesdelta:", testname, 2s);
    LOK_ASSERT_MESSAGE("a session that draws from tiles got a delta", delta.empty());

    helpers::sendTextFrame(session, request + "&since=" + version + "&epoch=" + epoch, testname);
    const Poco::JSON::Object::Ptr answer =
        jsonOf(helpers::getResponseMessage(session, "zstdvectorprimitives:", testname));
    LOK_ASSERT_MESSAGE("no primitives were answered for the page", !answer.isNull());
    LOK_ASSERT_EQUAL(std::string("vectorprimitives"), answer->get("type").toString());

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

    result = testTextPageDeltaStopsWhenForgotten();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testTextPageDeltaReachesTheSessionThatKeepsIt();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testTextPageIsForgottenWhenItsLastHolderLeaves();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testRequestForPrimitivesNamesOnePart();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testRequestForPrimitivesHasTheWholeCommandName();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testSessionHoldsThePartsTheDocumentServed();
    if (result != TestResult::Ok)
        exitTest(result);

    result = testDrawingFromTilesForgetsTheParts();
    if (result != TestResult::Ok)
        exitTest(result);

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitVectorPrimitives(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
