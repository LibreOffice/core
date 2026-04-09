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
 * Unit test for tile caching functionality.
 */

#include <config.h>

#include <common/Common.hpp>
#include <common/HexUtil.hpp>
#include <common/Png.hpp>
#include <common/Protocol.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <kit/Delta.hpp>
#include <wsd/TileCache.hpp>
#include <wsd/TileDesc.hpp>

#include <test/KitPidHelpers.hpp>
#include <test/WebSocketSession.hpp>
#include <test/helpers.hpp>
#include <test/test.hpp>

#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/InvalidCertificateHandler.h>
#include <Poco/Net/SSLManager.h>

#include <cppunit/extensions/HelperMacros.h>

#include <sstream>
#include <random>

using namespace std::literals;
using namespace helpers;

namespace CPPUNIT_NS
{
template<>
struct assertion_traits<std::vector<char>>
{
    static bool equal(const std::vector<char>& x, const std::vector<char>& y)
    {
        return x == y;
    }

    static std::string toString(const std::vector<char>& x)
    {
        const std::string text = '"' + (!x.empty() ? std::string(x.data(), x.size()) : "<empty>") + '"';
        return text;
    }
};
}

/// TileCache unit-tests.
class TileCacheTests : public CPPUNIT_NS::TestFixture
{
    const Poco::URI _uri;
    Poco::Net::HTTPResponse _response;
    std::shared_ptr<SocketPoll> _socketPoll;

    CPPUNIT_TEST_SUITE(TileCacheTests);

    CPPUNIT_TEST(testDesc);
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testSimpleCombine);
    CPPUNIT_TEST(testTileSubscription);
    CPPUNIT_TEST(testSize);
    CPPUNIT_TEST(testDisconnectMultiView);
    CPPUNIT_TEST(testUnresponsiveClient);
    CPPUNIT_TEST(testImpressTiles);
    CPPUNIT_TEST(testClientPartImpress);
    CPPUNIT_TEST(testClientPartCalc);
    // CPPUNIT_TEST(testTilesRenderedJustOnce); // unreliable
    // CPPUNIT_TEST(testTilesRenderedJustOnceMultiClient); // always fails, seems complicated to fix
#if ENABLE_DEBUG
    CPPUNIT_TEST(testSimultaneousTilesRenderedJustOnce);
#endif
    CPPUNIT_TEST(testLoad12ods);
    CPPUNIT_TEST(testTileInvalidateWriter);
    CPPUNIT_TEST(testTileInvalidateWriterPage);
    CPPUNIT_TEST(testTileInvalidateCalc);
    // temporarily disable
    //CPPUNIT_TEST(testTileInvalidatePartCalc);
    //CPPUNIT_TEST(testTileInvalidatePartImpress);
    CPPUNIT_TEST(testTileRequestByInvalidation);
    CPPUNIT_TEST(testTileRequestByZoom);
#if 0
    CPPUNIT_TEST(testTileWireIDHandling);
#endif
    CPPUNIT_TEST(testTileProcessed);
    // CPPUNIT_TEST(testTileInvalidatedOutside); // Disabled as it's failing locally on even very old commits.
#if 0
    CPPUNIT_TEST(testTileBeingRenderedHandling);
    CPPUNIT_TEST(testWireIDFilteringOnWSDSide);
#endif
    // unstable
    //CPPUNIT_TEST(testLimitTileVersionsOnFly);

    CPPUNIT_TEST_SUITE_END();

    void testDesc();
    void testSimple();
    void testSimpleCombine();
    void testTileSubscription();
    void testSize();
    void testDisconnectMultiView();
    void testUnresponsiveClient();
    void testImpressTiles();
    void testClientPartImpress();
    void testClientPartCalc();
    void testTilesRenderedJustOnce();
    void testTilesRenderedJustOnceMultiClient();
    void testSimultaneousTilesRenderedJustOnce();
    void testLoad12ods();
    void testTileInvalidateWriter();
    void testTileInvalidateWriterPage();
    void testWriterAnyKey();
    void testTileInvalidateCalc();
    void testTileInvalidatePartCalc();
    void testTileInvalidatePartImpress();
    void testTileRequestByInvalidation();
    void testTileRequestByZoom();
    void testTileWireIDHandling();
    void testTileProcessed();
    void testTileInvalidatedOutside();
    void testTileBeingRenderedHandling();
    void testWireIDFilteringOnWSDSide();
    void testLimitTileVersionsOnFly();

    void checkTiles(std::shared_ptr<http::WebSocketSession>& socket, const std::string& docType,
                      const std::string& testname);

    void requestTiles(std::shared_ptr<http::WebSocketSession>& socket, const std::string& docType,
                      const int part, const int docWidth, const int docHeight,
                      const std::string& testname);

    void checkBlackTiles(std::shared_ptr<http::WebSocketSession>& socket, const int /*part*/,
                         const int /*docWidth*/, const int /*docHeight*/,
                         const std::string& testname);

    void checkBlackTile(BlobData::const_iterator start, BlobData::const_iterator end);

    bool getPartFromInvalidateMessage(const std::string& message, int& part);

public:
    TileCacheTests()
        : _uri(helpers::getTestServerURI())
        , _socketPoll(std::make_shared<SocketPoll>("TileCachePoll"))
    {
#if ENABLE_SSL
        Poco::Net::initializeSSL();
        // Just accept the certificate anyway for testing purposes
        Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> invalidCertHandler = new Poco::Net::AcceptCertificateHandler(false);
        Poco::Net::Context::Params sslParams;
        Poco::Net::Context::Ptr sslContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, sslParams);
        Poco::Net::SSLManager::instance().initializeClient(nullptr, std::move(invalidCertHandler), std::move(sslContext));
#endif
    }

#if ENABLE_SSL
    ~TileCacheTests()
    {
        Poco::Net::uninitializeSSL();
    }
#endif

    void setUp()
    {
        resetTestStartTime();
        waitForKitPidsReady("setUp");
        resetTestStartTime();
        _socketPoll->startThread();
    }

    void tearDown()
    {
        _socketPoll->joinThread();
        resetTestStartTime();
        waitForKitPidsReady("tearDown");
        resetTestStartTime();
    }
};


bool TileCacheTests::getPartFromInvalidateMessage(const std::string& message, int& part)
{
    StringVector tokens = StringVector::tokenize(message);
    if (tokens.size() == 2 && tokens.equals(1, "EMPTY"))
    {
        part = -1;
        return true;
    }
    if (tokens.size() > 2 && tokens.equals(1, "EMPTY,"))
        return COOLProtocol::stringToInteger(tokens[2], part);
    return COOLProtocol::getTokenInteger(tokens, "part", part);
}

void TileCacheTests::testDesc()
{
    constexpr std::string_view testname = __func__;

    TileDesc descA = TileDesc(CanonicalViewId::None, 0, 0, 256, 256, 0, 0, 3200, 3200, /* ignored in cache */ 0, 1234, 1);
    TileDesc descB = TileDesc(CanonicalViewId::None, 0, 0, 256, 256, 0, 0, 3200, 3200, /* ignored in cache */ 1, 1235, 2);

    TileDescCacheCompareEq pred;
    LOK_ASSERT_MESSAGE("TileDesc versions do match", descA.getVersion() != descB.getVersion());
    LOK_ASSERT_MESSAGE("TileDesc should match, ignoring unimportant fields", pred(descA, descB));
}

void TileCacheTests::testSimple()
{
    constexpr std::string_view testname = __func__;

    if (isStandalone())
    {
        if (!UnitWSD::init(UnitWSD::UnitType::Wsd, ""))
            throw std::runtime_error("Failed to load wsd unit test library.");
    }

    // Create TileCache and pretend the file was modified as recently as
    // now, so it discards the cached data.
    TileCache tc("doc.ods", std::chrono::system_clock::time_point());

    CanonicalViewId nviewid(CanonicalViewId::None);
    int part = 0;
    int mode = 0;
    int width = 256;
    int height = 256;
    int tilePosX = 0;
    int tilePosY = 0;
    int tileWidth = 3840;
    int tileHeight = 3840;
    TileDesc tile(nviewid, part, mode, width, height, tilePosX, tilePosY, tileWidth, tileHeight, -1, 0, -1);

    // No Cache
    Tile tileData = tc.lookupTile(tile);
    LOK_ASSERT_MESSAGE("found tile when none was expected", !tileData || !tileData->isValid());

    // Cache Tile
    const int size = 1024;
    std::vector<char> data = genRandomData(size);
    data[0] = 'Z'; // compressed pixels.
    tc.saveTileAndNotify(tile, data.data(), size);

    // Find Tile
    tileData = tc.lookupTile(tile);
    LOK_ASSERT_MESSAGE("tile not found when expected", tileData && tileData->isValid());
    const BlobData &keyframe = tileData->data();
    LOK_ASSERT_MESSAGE("cached tile corrupted", keyframe.size() == data.size() - 1 /* dropped Z */);
    for (size_t i = 0; i < data.size() - 1; ++i)
        LOK_ASSERT_MESSAGE("cached tile data", data[i+1] == keyframe[i]);

    // Invalidate Tiles
    tc.invalidateTiles("invalidatetiles: EMPTY", nviewid);

    // No Cache
    tileData = tc.lookupTile(tile);
    LOK_ASSERT_MESSAGE("found tile when none was expected", !tileData || !tileData->isValid());
}

void TileCacheTests::testSimpleCombine()
{
    const std::string testname = "simpleCombine-";
    std::string documentPath, documentURL;
    getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    // First.
    std::shared_ptr<http::WebSocketSession> socket1
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname + "1 ");

    sendTextFrame(socket1,
                  "tilecombine nviewid=0  part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 "
                  "tilewidth=3840 tileheight=3840",
                  testname);

    std::vector<char> tile1a = getResponseMessage(socket1, "tile:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !tile1a.empty());
    std::vector<char> tile1b = getResponseMessage(socket1, "tile:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !tile1b.empty());

    sendTextFrame(socket1,
                  "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 "
                  "oldwid=42,42 tilewidth=3840 tileheight=3840",
                  testname);
    tile1a = getResponseMessage(socket1, "delta:", testname + "1 ", 10s);
    //  TST_LOG("Response is: " + HexUtil::dumpHex(tile1a) << "\n");
    // no content in an update delta: - so ends with a '\n'
    LOK_ASSERT_MESSAGE("did not receive an update delta: message as expected", !tile1a.empty() && tile1a.back() == '\n');
    tile1b = getResponseMessage(socket1, "delta:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive an update delta: message as expected", !tile1b.empty() && tile1b.back() == '\n');

    // Second.
    TST_LOG("Connecting second client.");
    std::shared_ptr<http::WebSocketSession> socket2
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname + "2 ");

    sendTextFrame(socket2,
                  "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 "
                  "tilewidth=3840 tileheight=3840",
                  testname);

    std::vector<char> tile2a = getResponseMessage(socket2, "tile:", testname + "2 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !tile2a.empty());
    std::vector<char> tile2b = getResponseMessage(socket2, "tile:", testname + "2 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !tile2b.empty());

    // First - check force keyframe
    sendTextFrame(socket1,
                  "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 "
                  "oldwid=0,0 tilewidth=3840 tileheight=3840",
                  testname);
    tile1a = getResponseMessage(socket1, "tile:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !tile1a.empty());
    tile1b = getResponseMessage(socket1, "tile:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !tile1b.empty());

    socket1->asyncShutdown();
    socket2->asyncShutdown();

    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                       socket1->waitForDisconnection(5s));
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                       socket2->waitForDisconnection(5s));
}

void TileCacheTests::testTileSubscription()
{
    const std::string testname = "tileSubscription-";
    std::string documentPath, documentURL;
    getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    // First.
    std::shared_ptr<http::WebSocketSession> socket1
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname + "1 ");

    sendTextFrame(socket1,
                  "tilecombine nviewid=0  part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 "
                  "tilewidth=3840 tileheight=3840",
                  testname);

    // std::shared_ptr<TileDesc>
    auto tile1a = getResponseDesc(socket1, "tile:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !!tile1a);
    auto tile1b = getResponseDesc(socket1, "tile:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !!tile1b);

    // type a period
    sendChar(socket1, '.', skNone, testname);

    // no viewport set so we have to re-request:
    sendTextFrame(socket1,
                  "tilecombine nviewid=0  part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 "
                  "oldwid=42,42 tilewidth=3840 tileheight=3840",
                  testname);

    auto delta1a = getResponseDesc(socket1, "delta:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a delta: message as expected", !!delta1a);
    auto delta1b = getResponseDesc(socket1, "delta:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a delta: message as expected", !!delta1b);

    // ordering is undefined tiles arrive in so swap if needed:
    if (tile1a->getTilePosX() != delta1a->getTilePosX())
    {
        std::swap(delta1a, delta1b);
        TST_LOG("tiles re-ordered for once");
    }

    // check WIDs variously
    LOK_ASSERT_EQUAL(tile1a->getWireId(), delta1a->getWireId());
    LOK_ASSERT_EQUAL(tile1b->getWireId(), delta1b->getWireId());

    // Second.
    TST_LOG("Connecting second client.");
    std::shared_ptr<http::WebSocketSession> socket2
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname + "2 ");

    // type a space - get an invalidate - but no change
    sendChar(socket1, ' ', skNone, testname);

    // we need to wait for the invalidation and message to get to the kit ->
    // wsd and back - otherwise when we re-fetch tiles we get un-changed ones
    // from the cache, since wsd has no idea it has changed yet.
    // so wait for an invalidation
    assertResponseString(socket1, "invalidatetiles:", testname);

    // two subscriptions on a tile we hope from three requests
    sendTextFrame(socket1,
                  "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840 oldwid=42,42 "
                  "tileposy=0,0 tilewidth=3840 tileheight=3840",
                  testname);
    sendTextFrame(socket1,
                  "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840 oldwid=42,42 "
                  "tileposy=0,0 tilewidth=3840 tileheight=3840",
                  testname);
    sendTextFrame(socket2,
                  "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 "
                  "tilewidth=3840 tileheight=3840",
                  testname);

    // User 2 should get tiles
    auto tile2a = getResponseDesc(socket2, "tile:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !!tile2a);
    auto tile2b = getResponseDesc(socket2, "tile:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !!tile2b);

    // User 1 should get deltas
    auto delta1c = getResponseDesc(socket1, "delta:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !!delta1c);
    auto delta1d = getResponseDesc(socket1, "delta:", testname + "1 ");
    LOK_ASSERT_MESSAGE("did not receive a tile: message as expected", !!delta1d);

    // ordering is undefined tiles arrive in so swap if needed:
    if (tile2a->getTilePosX() != delta1c->getTilePosX())
    {
        std::swap(delta1c, delta1d);
        TST_LOG("tiles re-ordered for once");
    }
    // are they the right tiles ?
    LOK_ASSERT_EQUAL(tile2a->getTilePosX(), delta1c->getTilePosX());
    LOK_ASSERT_EQUAL(tile2a->getTilePosY(), delta1c->getTilePosY());
    LOK_ASSERT_EQUAL(tile2b->getTilePosX(), delta1d->getTilePosX());
    LOK_ASSERT_EQUAL(tile2b->getTilePosY(), delta1d->getTilePosY());

    // WIDs should match
    LOK_ASSERT_EQUAL(tile2a->getWireId(), delta1c->getWireId());
    LOK_ASSERT_EQUAL(tile2b->getWireId(), delta1d->getWireId());

    socket1->asyncShutdown();
    socket2->asyncShutdown();

    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                       socket1->waitForDisconnection(5s));
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                       socket2->waitForDisconnection(5s));
}

void TileCacheTests::testSize()
{
    constexpr std::string_view testname = __func__;

    // Create TileCache and pretend the file was modified as recently as
    // now, so it discards the cached data.
    TileCache tc("doc.ods", std::chrono::system_clock::time_point());

    CanonicalViewId nviewid(CanonicalViewId::None);
    int part = 0;
    int mode = 0;
    int width = 256;
    int height = 256;
    int tilePosX = 0;
    int tileWidth = 3840;
    int tileHeight = 3840;
    TileWireId id = 0;
    const std::vector<char> data = genRandomData(4096);

    // Churn the cache somewhat
    size_t maxSize = (data.size() + sizeof (TileDesc)) * 10;
    tc.setMaxCacheSize(maxSize);
    for (int tilePosY = 0; tilePosY < 20; tilePosY++)
    {
        TileDesc tile(nviewid, part, mode, width, height, tilePosX, tilePosY * tileHeight,
                      tileWidth, tileHeight, -1, 0, -1);
        tile.setWireId(id++);
        tc.saveTileAndNotify(tile, data.data(), data.size());
    }
    LOK_ASSERT_MESSAGE("tile cache too big", tc.getMemorySize() < maxSize);
}


void TileCacheTests::testDisconnectMultiView()
{
    const char* testname = "testDisconnectMultiView";

    constexpr size_t repeat = 2;
    for (size_t j = 1; j <= repeat; ++j)
    {
        // Make sure previous sessions have closed
        waitForKitPidsReady(testname);

        TST_LOG("disconnectMultiView try #" << j);

        std::string documentPath, documentURL;
        getDocumentPathAndURL("setclientpart.ods", documentPath, documentURL, "disconnectMultiView ");


        // Request a huge tile, and cancel immediately.
        std::shared_ptr<http::WebSocketSession> socket1
            = loadDocAndGetSession(_socketPoll, _uri, documentURL, "disconnectMultiView-1 ");
        std::shared_ptr<http::WebSocketSession> socket2
            = loadDocAndGetSession(_socketPoll, _uri, documentURL, "disconnectMultiView-2 ", true);

        sendTextFrame(socket1,
                      "tilecombine nviewid=0 part=0 width=256 height=256 "
                      "tileposx=0,3840,7680,11520,0,3840,7680,11520 "
                      "tileposy=0,0,0,0,3840,3840,3840,3840 tilewidth=3840 tileheight=3840",
                      "cancelTilesMultiView-1 ");
        sendTextFrame(socket2,
                      "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,7680,0 "
                      "tileposy=0,0,0,22520 tilewidth=3840 tileheight=3840",
                      "cancelTilesMultiView-2 ");

        socket1->asyncShutdown();

        for (int i = 0; i < 4; ++i)
        {
            getTileMessage(socket2, "disconnectMultiView-2 ");
        }

        // Should never get more than 4 tiles on socket2.
        getResponseString(socket2, "tile:", "disconnectMultiView-2 ", 500ms);

        socket2->asyncShutdown();

        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                           socket1->waitForDisconnection(5s));
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                           socket2->waitForDisconnection(5s));
    }
}

void TileCacheTests::testUnresponsiveClient()
{
    const std::string testname = "unresponsiveClient-";

    TST_LOG("testUnresponsiveClient.");

    std::string documentPath, documentURL;
    getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    TST_LOG("Connecting first client.");
    std::shared_ptr<http::WebSocketSession> socket1
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname + "1 ");

    TST_LOG("Connecting second client.");
    std::shared_ptr<http::WebSocketSession> socket2
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname + "2 ");

    // Pathologically request tiles and fail to read (say slow connection).
    // Meanwhile, verify that others can get all tiles fine.
    // TODO: Track memory consumption to verify we don't buffer too much.
    std::ostringstream oss;
    for (int i = 0; i < 1000; ++i)
    {
        Util::encodeId(oss, Util::rng::getNext(), 6);
    }

    const std::string documentContents = oss.str();

    // Request tiles before expecting an invalidate.
    sendTextFrame(socket2, "tilecombine nviewid=0 part=0 width=256 height=256 "
                           "tileposx=0,3840,7680,11520,0,3840,7680,11520 "
                           "tileposy=0,0,0,0,3840,3840,3840,3840 tilewidth=3840 "
                           "tileheight=3840",
                  testname + "2 ");
    for (int i = 0; i < 8; ++i)
    {
        std::vector<char> tile = getResponseMessage(socket2, "tile:", testname + "2 ");
        LOK_ASSERT_MESSAGE("Did not receive tile #" + std::to_string(i+1) + " of 8: message as expected", !tile.empty());
    }

    for (int x = 0; x < 8; ++x)
    {
        // Invalidate to force re-rendering.
        sendTextFrame(socket2, "uno .uno:SelectAll", testname);
        sendTextFrame(socket2, "uno .uno:Delete", testname);
        assertResponseString(socket2, "invalidatetiles:", testname + "2 ");
        sendTextFrame(socket2, "paste mimetype=text/html\n" + documentContents, testname + "2 ");
        assertResponseString(socket2, "invalidatetiles:", testname + "2 ");

        // Ask for tiles and don't read!
        sendTextFrame(socket1, "tilecombine nviewid=0 part=0 width=256 height=256 "
                               "tileposx=0,3840,7680,11520,0,3840,7680,11520 "
                               "tileposy=0,0,0,0,3840,3840,3840,3840 tilewidth=3840 "
                               "tileheight=3840",
                      testname + "1 ");

        // Verify that we get all 8 tiles.
        sendTextFrame(socket2, "tilecombine nviewid=0 part=0 width=256 height=256 "
                               "tileposx=0,3840,7680,11520,0,3840,7680,11520 "
                               "tileposy=0,0,0,0,3840,3840,3840,3840 tilewidth=3840 "
                               "tileheight=3840",
                      testname + "2 ");
        for (int i = 0; i < 8; ++i)
        {
            std::vector<char> tile = getResponseMessage(socket2, "tile:", testname + "2 ");
            LOK_ASSERT_MESSAGE("Did not receive tile #" + std::to_string(i+1) + " of 8: message as expected", !tile.empty());
        }

        // FIXME: removed canceltiles here ...
    }

    socket1->asyncShutdown();
    socket2->asyncShutdown();

    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                       socket1->waitForDisconnection(5s));
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                       socket2->waitForDisconnection(5s));
}

void TileCacheTests::testImpressTiles()
{
    const std::string testname = "impressTiles ";
    try
    {
        std::shared_ptr<http::WebSocketSession> socket
            = loadDocAndGetSession(_socketPoll, "setclientpart.odp", _uri, testname);

        sendTextFrame(socket,
                      "tile nviewid=0 part=0 width=180 height=135 tileposx=0 tileposy=0 "
                      "tilewidth=15875 tileheight=11906 id=0",
                      testname);
        getTileMessage(socket, testname);

        socket->asyncShutdown();
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}

void TileCacheTests::testClientPartImpress()
{
    const std::string testname = "clientPartImpress ";
    try
    {
        std::shared_ptr<http::WebSocketSession> socket
            = loadDocAndGetSession(_socketPoll, "setclientpart.odp", _uri, testname);

        checkTiles(socket, "presentation", testname);

        socket->asyncShutdown();
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}

void TileCacheTests::testClientPartCalc()
{
    const std::string testname = "clientPartCalc ";
    try
    {
        std::shared_ptr<http::WebSocketSession> socket
            = loadDocAndGetSession(_socketPoll, "setclientpart.ods", _uri, testname);

        checkTiles(socket, "spreadsheet", testname);

        socket->asyncShutdown();
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
}

void TileCacheTests::testTilesRenderedJustOnce()
{
    const char* testname = "tilesRenderedJustOnce ";

    std::shared_ptr<http::WebSocketSession> socket
        = loadDocAndGetSession(_socketPoll, "with_comment.odt", _uri, testname);

    assertResponseString(socket, "statechanged: .uno:AcceptTrackedChange=", testname);

    for (int i = 0; i < 10; ++i)
    {
        // Get initial rendercount.
        sendTextFrame(socket, "ping", testname);
        const auto ping1 = assertResponseString(socket, "pong", testname);
        int renderCount1 = 0;
        LOK_ASSERT(COOLProtocol::getTokenIntegerFromMessage(ping1, "rendercount", renderCount1));
        LOK_ASSERT_EQUAL(i * 3, renderCount1);

        // Modify.
        sendText(socket, "a", testname);
        assertResponseString(socket, "invalidatetiles:", testname);

        // Get 3 tiles.
        sendTextFrame(socket, "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,7680 tileposy=0,0,0 tilewidth=3840 tileheight=3840", testname);
        assertResponseString(socket, "tile:", testname);
        assertResponseString(socket, "tile:", testname);
        assertResponseString(socket, "tile:", testname);

        // Get new rendercount.
        sendTextFrame(socket, "ping", testname);
        const auto ping2 = assertResponseString(socket, "pong", testname);
        int renderCount2 = 0;
        LOK_ASSERT(COOLProtocol::getTokenIntegerFromMessage(ping2, "rendercount", renderCount2));
        LOK_ASSERT_EQUAL((i+1) * 3, renderCount2);

        // Get same 3 tiles.
        sendTextFrame(socket, "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,7680 tileposy=0,0,0 tilewidth=3840 tileheight=3840", testname);
        const auto tile1 = assertResponseString(socket, "tile:", testname);

        // monotonically increasing id.
        std::string wid1;
        COOLProtocol::getTokenStringFromMessage(tile1, "wid", wid1);

        const auto tile2 = assertResponseString(socket, "tile:", testname);

        std::string wid2;
        COOLProtocol::getTokenStringFromMessage(tile2, "wid", wid2);
        LOK_ASSERT_EQUAL(wid1, wid2); // shouldn't have changed

        const auto tile3 = assertResponseString(socket, "tile:", testname);
        std::string wid3;
        COOLProtocol::getTokenStringFromMessage(tile3, "wid", wid3);
        LOK_ASSERT_EQUAL(wid3, wid2);

        // Get new rendercount.
        sendTextFrame(socket, "ping", testname);
        const auto ping3 = assertResponseString(socket, "pong", testname);
        int renderCount3 = 0;
        LOK_ASSERT(COOLProtocol::getTokenIntegerFromMessage(ping3, "rendercount", renderCount3));
        LOK_ASSERT_EQUAL(renderCount2, renderCount3);
    }

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

void TileCacheTests::testTilesRenderedJustOnceMultiClient()
{
    const std::string testname = "tilesRenderdJustOnceMultiClient";
    const auto testname1 = testname + "-1 ";
    const auto testname2 = testname + "-2 ";
    const auto testname3 = testname + "-3 ";
    const auto testname4 = testname + "-4 ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL("with_comment.odt", documentPath, documentURL, testname);

    TST_LOG("Connecting first client.");
    std::shared_ptr<http::WebSocketSession> socket1
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname1);
    TST_LOG("Connecting second client.");
    std::shared_ptr<http::WebSocketSession> socket2
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname2);
    TST_LOG("Connecting third client.");
    std::shared_ptr<http::WebSocketSession> socket3
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname3);
    TST_LOG("Connecting fourth client.");
    std::shared_ptr<http::WebSocketSession> socket4
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname4);

    for (int i = 0; i < 10; ++i)
    {
        // No tiles at this point.
        assertNotInResponse(socket1, "tile:", testname1);
        assertNotInResponse(socket2, "tile:", testname2);
        assertNotInResponse(socket3, "tile:", testname3);
        assertNotInResponse(socket4, "tile:", testname4);

        // Get initial rendercount.
        sendTextFrame(socket1, "ping", testname1);
        const auto ping1 = assertResponseString(socket1, "pong", testname1);
        int renderCount1 = 0;
        LOK_ASSERT(COOLProtocol::getTokenIntegerFromMessage(ping1, "rendercount", renderCount1));
        LOK_ASSERT_EQUAL(i * 3, renderCount1);

        // Modify.
        sendText(socket1, "a", testname1);
        assertResponseString(socket1, "invalidatetiles:", testname1);

        // Get 3 tiles.
        sendTextFrame(socket1, "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,7680 tileposy=0,0,0 tilewidth=3840 tileheight=3840", testname1);
        assertResponseString(socket1, "tile:", testname1);
        assertResponseString(socket1, "tile:", testname1);
        assertResponseString(socket1, "tile:", testname1);

        assertResponseString(socket2, "invalidatetiles:", testname2);
        sendTextFrame(socket2, "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,7680 tileposy=0,0,0 tilewidth=3840 tileheight=3840", testname2);
        assertResponseString(socket2, "tile:", testname2);
        assertResponseString(socket2, "tile:", testname2);
        assertResponseString(socket2, "tile:", testname2);

        assertResponseString(socket3, "invalidatetiles:", testname3);
        sendTextFrame(socket3, "tilecombine nviewid=0 part=0 nviewid=0 width=256 height=256 tileposx=0,3840,7680 tileposy=0,0,0 tilewidth=3840 tileheight=3840", testname3);
        assertResponseString(socket3, "tile:", testname3);
        assertResponseString(socket3, "tile:", testname3);
        assertResponseString(socket3, "tile:", testname3);

        assertResponseString(socket4, "invalidatetiles:", testname4);
        sendTextFrame(socket4, "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,7680 tileposy=0,0,0 tilewidth=3840 tileheight=3840", testname4);
        assertResponseString(socket4, "tile:", testname4);
        assertResponseString(socket4, "tile:", testname4);
        assertResponseString(socket4, "tile:", testname4);

        // Get new rendercount.
        sendTextFrame(socket1, "ping", testname1);
        const auto ping2 = assertResponseString(socket1, "pong", testname1);
        int renderCount2 = 0;
        LOK_ASSERT(COOLProtocol::getTokenIntegerFromMessage(ping2, "rendercount", renderCount2));
        LOK_ASSERT_EQUAL((i+1) * 3, renderCount2);

        // Get same 3 tiles.
        sendTextFrame(socket1, "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,7680 tileposy=0,0,0 tilewidth=3840 tileheight=3840", testname1);
        const auto tile1 = assertResponseString(socket1, "tile:", testname1);
        std::string renderId1;
        COOLProtocol::getTokenStringFromMessage(tile1, "renderid", renderId1);
        LOK_ASSERT_EQUAL_STR("cached", renderId1);

        const auto tile2 = assertResponseString(socket1, "tile:", testname1);
        std::string renderId2;
        COOLProtocol::getTokenStringFromMessage(tile2, "renderid", renderId2);
        LOK_ASSERT_EQUAL_STR("cached", renderId2);

        const auto tile3 = assertResponseString(socket1, "tile:", testname1);
        std::string renderId3;
        COOLProtocol::getTokenStringFromMessage(tile3, "renderid", renderId3);
        LOK_ASSERT_EQUAL_STR("cached", renderId3);

        // Get new rendercount.
        sendTextFrame(socket1, "ping", testname1);
        const auto ping3 = assertResponseString(socket1, "pong", testname1);
        int renderCount3 = 0;
        LOK_ASSERT(COOLProtocol::getTokenIntegerFromMessage(ping3, "rendercount", renderCount3));
        LOK_ASSERT_EQUAL(renderCount2, renderCount3);
    }

    socket1->asyncShutdown();
    socket2->asyncShutdown();
    socket3->asyncShutdown();
    socket4->asyncShutdown();

    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                       socket1->waitForDisconnection(5s));
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                       socket2->waitForDisconnection(5s));
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 3",
                       socket3->waitForDisconnection(5s));
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 4",
                       socket4->waitForDisconnection(5s));
}

void TileCacheTests::testSimultaneousTilesRenderedJustOnce()
{
    const std::string testname = "testSimultaneousTilesRenderedJustOnce-";
    std::string documentPath, documentURL;
    getDocumentPathAndURL("hello.odt", documentPath, documentURL, testname);

    TST_LOG("Connecting first client.");
    std::shared_ptr<http::WebSocketSession> socket1
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname + "1 ");
    TST_LOG("Connecting second client.");
    std::shared_ptr<http::WebSocketSession> socket2
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname + "2 ");

    // Wait for the invalidatetile events to pass, otherwise they
    // remove our tile subscription.
    assertResponseString(socket1, "statechanged:", "client1 ");
    assertResponseString(socket2, "statechanged:", "client2 ");

    sendTextFrame(socket1,
                  "tile nviewid=0 part=42 width=256 height=256 tileposx=1000 tileposy=2000 "
                  "tilewidth=3000 tileheight=3000",
                  testname);
    sendTextFrame(socket2,
                  "tile nviewid=0 part=42 width=256 height=256 tileposx=1000 tileposy=2000 "
                  "tilewidth=3000 tileheight=3000",
                  testname);

    const auto response1 = assertResponseString(socket1, "tile:", "client1 ");
    const auto response2 = assertResponseString(socket2, "tile:", "client2 ");

    if (!response1.empty() && !response2.empty())
    {
        std::string renderId1;
        COOLProtocol::getTokenStringFromMessage(response1, "renderid", renderId1);
        std::string renderId2;
        COOLProtocol::getTokenStringFromMessage(response2, "renderid", renderId2);

        LOK_ASSERT(renderId1 == renderId2 ||
                       (renderId1 == "cached" && renderId2 != "cached") ||
                       (renderId1 != "cached" && renderId2 == "cached"));
    }

    socket1->asyncShutdown();
    socket2->asyncShutdown();

    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                       socket1->waitForDisconnection(5s));
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                       socket2->waitForDisconnection(5s));
}

void TileCacheTests::testLoad12ods()
{
    const char* testname = "load12ods ";
    try
    {
        std::shared_ptr<http::WebSocketSession> socket
            = loadDocAndGetSession(_socketPoll, "load12.ods", _uri, testname);

        int docSheet = -1;
        int docSheets = 0;
        int docHeight = 0;
        int docWidth = 0;
        int docViewId = -1;

        // check document size
        sendTextFrame(socket, "status", testname);

        const auto response = assertResponseString(socket, "status:", testname);
        parseDocSize(response.substr(7), "spreadsheet", docSheet, docSheets, docWidth, docHeight,
                     docViewId, testname);

        checkBlackTiles(socket, docSheet, docWidth, docWidth, testname);

        socket->asyncShutdown();
        LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                           socket->waitForDisconnection(5s));
    }
    catch (const Poco::Exception& exc)
    {
        LOK_ASSERT_FAIL(exc.displayText());
    }
    catch (...)
    {
        LOK_ASSERT_FAIL("Unexpected exception thrown during ods load");
    }
}

void TileCacheTests::checkBlackTile(BlobData::const_iterator start, BlobData::const_iterator end)
{
    constexpr std::string_view testname = __func__;

    size_t width = 256, height = 256, black = 0;

    Blob zimg = std::make_shared<BlobData>(start, end);
    Blob img = DeltaGenerator::expand(zimg);

    png_bytep rows = (png_bytep)img->data();

    for (size_t i = 0; i < img->size(); i += 4)
    {
        png_byte R = rows[i + 0];
        png_byte G = rows[i + 1];
        png_byte B = rows[i + 2];
        png_byte A = rows[i + 3];
        if (R == 0x00 && G == 0x00 && B == 0x00 && A == 0xff)
            ++black;
    }

    LOK_ASSERT_MESSAGE("The tile is 100% black", black != height * width);
    LOK_ASSERT(height * width != 0);
    LOK_ASSERT_MESSAGE("The tile is 90% black", (black * 100) / (height * width) < 90);
}

void TileCacheTests::checkBlackTiles(std::shared_ptr<http::WebSocketSession>& socket,
                                     const int /*part*/, const int /*docWidth*/,
                                     const int /*docHeight*/, const std::string& testname)
{
    // Check the last row of tiles to verify that the tiles
    // render correctly and there are no black tiles.
    // Current cap of table size ends at 257280 twips (for load12.ods),
    // otherwise 2035200 should be rendered successfully.
    const char* req = "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=253440 "
                      "tilewidth=3840 tileheight=3840";
    sendTextFrame(socket, req, testname);

    const std::vector<char> tile = getResponseMessage(socket, "tile:", testname);
    if (!tile.size())
    {
        LOK_ASSERT_FAIL("No tile returned to checkBlackTiles - failed load ?");
        return;
    }

    const std::string firstLine = COOLProtocol::getFirstLine(tile);

#if 0
    std::fstream outStream("/tmp/black.z", std::ios::out);
    outStream.write(tile.data() + firstLine.size() + 1, tile.size() - firstLine.size() - 1);
    outStream.close();
#endif

    checkBlackTile(tile.begin() + firstLine.size() + 1, tile.end());
}

void TileCacheTests::testTileInvalidateWriter()
{
    const char* testname = "tileInvalidateWriter ";
    std::string documentPath, documentURL;
    getDocumentPathAndURL("empty.odt", documentPath, documentURL, testname);

    std::shared_ptr<http::WebSocketSession> socket
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

    // Request a tile before expecting an invalidate.
    sendTextFrame(socket, "tilecombine nviewid=0 part=0 width=256 height=256 "
                           "tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840",
                  testname);
    std::vector<char> tile = getResponseMessage(socket, "tile:", testname);
    LOK_ASSERT_MESSAGE("Did not receive tile message as expected", !tile.empty());

    std::string text = "abcde";
    for (char ch : text)
    {
        sendChar(socket, ch, skNone, testname); // Send ordinary characters and wait for response -> one tile invalidation for each
        assertResponseString(socket, "invalidatetiles:", testname);
    }

    // While extra invalidates are not desirable, they are inevitable at the moment.
    //LOK_ASSERT_MESSAGE("received unexpected invalidatetiles: message", getResponseMessage(socket, "invalidatetiles:").empty());

    // TODO: implement a random-sequence "monkey test"

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

void TileCacheTests::testTileInvalidateWriterPage()
{
    const char* testname = "tileInvalidateWriterPage ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL("empty.odt", documentPath, documentURL, testname);

    std::shared_ptr<http::WebSocketSession> socket
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

    // Request a tile before expecting an invalidate.
    sendTextFrame(socket, "tilecombine nviewid=0 part=0 width=256 height=256 "
                           "tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840",
                  testname);
    std::vector<char> tile = getResponseMessage(socket, "tile:", testname);
    LOK_ASSERT_MESSAGE("Did not receive tile message as expected", !tile.empty());

    sendChar(socket, '\n', skCtrl, testname); // Send Ctrl+Enter (page break).
    assertResponseString(socket, "invalidatetiles:", testname);

    sendTextFrame(socket, "uno .uno:InsertTable { \"Columns\": { \"type\": \"long\",\"value\": 3 }, \"Rows\": { \"type\": \"long\",\"value\": 2 }}", testname);

    const auto res = assertResponseString(socket, "invalidatetiles:", testname);
    int part = -1;
    LOK_ASSERT_MESSAGE("No part# in invalidatetiles message.",
                           getPartFromInvalidateMessage(res, part));
    LOK_ASSERT_EQUAL(0, part);

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

// This isn't yet used
void TileCacheTests::testWriterAnyKey()
{
    const char* testname = "writerAnyKey ";
    std::string documentPath, documentURL;
    getDocumentPathAndURL("empty.odt", documentPath, documentURL, testname);

    std::shared_ptr<http::WebSocketSession> socket
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

    // Now test "usual" keycodes (TODO: whole 32-bit range)
    for (int i=0; i<0x1000; ++i)
    {
        std::stringstream ss("Keycode ");
        ss << i;
        std::string s = ss.str();
        std::stringstream fn("saveas url=");
        fn << documentURL << i << ".odt format= options=";
        std::string f = fn.str();

        const int istart = 474;
        sendText(socket, "\n"+s+"\n", testname);
        sendKeyEvent(socket, "input", 0, i, testname);
        sendKeyEvent(socket, "up", 0, i, testname);
        sendText(socket, "\nEnd "+s+"\n", testname);
        if (i>=istart)
            sendTextFrame(socket, f, testname);

        sendText(socket, "\n"+s+" With Shift:\n", testname);
        sendKeyEvent(socket, "input", 0, i|skShift, testname);
        sendKeyEvent(socket, "up", 0, i|skShift, testname);
        sendText(socket, "\nEnd "+s+" With Shift\n", testname);
        if (i>=istart)
            sendTextFrame(socket, f, testname);

        sendText(socket, "\n"+s+" With Ctrl:\n", testname);
        sendKeyEvent(socket, "input", 0, i|skCtrl, testname);
        sendKeyEvent(socket, "up", 0, i|skCtrl, testname);
        sendText(socket, "\nEnd "+s+" With Ctrl\n", testname);
        if (i>=istart)
            sendTextFrame(socket, f, testname);

        sendText(socket, "\n"+s+" With Alt:\n", testname);
        sendKeyEvent(socket, "input", 0, i|skAlt, testname);
        sendKeyEvent(socket, "up", 0, i|skAlt, testname);
        sendText(socket, "\nEnd "+s+" With Alt\n", testname);
        if (i>=istart)
            sendTextFrame(socket, f, testname);

        sendText(socket, "\n"+s+" With Shift+Ctrl:\n", testname);
        sendKeyEvent(socket, "input", 0, i|skShift|skCtrl, testname);
        sendKeyEvent(socket, "up", 0, i|skShift|skCtrl, testname);
        sendText(socket, "\nEnd "+s+" With Shift+Ctrl\n", testname);
        if (i>=istart)
            sendTextFrame(socket, f, testname);

        sendText(socket, "\n"+s+" With Shift+Alt:\n", testname);
        sendKeyEvent(socket, "input", 0, i|skShift|skAlt, testname);
        sendKeyEvent(socket, "up", 0, i|skShift|skAlt, testname);
        sendText(socket, "\nEnd "+s+" With Shift+Alt\n", testname);
        if (i>=istart)
            sendTextFrame(socket, f, testname);

        sendText(socket, "\n"+s+" With Ctrl+Alt:\n", testname);
        sendKeyEvent(socket, "input", 0, i|skCtrl|skAlt, testname);
        sendKeyEvent(socket, "up", 0, i|skCtrl|skAlt, testname);
        sendText(socket, "\nEnd "+s+" With Ctrl+Alt\n", testname);
        if (i>=istart)
            sendTextFrame(socket, f, testname);

        sendText(socket, "\n"+s+" With Shift+Ctrl+Alt:\n", testname);
        sendKeyEvent(socket, "input", 0, i|skShift|skCtrl|skAlt, testname);
        sendKeyEvent(socket, "up", 0, i|skShift|skCtrl|skAlt, testname);
        sendText(socket, "\nEnd "+s+" With Shift+Ctrl+Alt\n", testname);

        if (i>=istart)
            sendTextFrame(socket, f, testname);

        // This is to allow server to process the input, and check that everything is still OK
        sendTextFrame(socket, "status", testname);
        getResponseMessage(socket, "status:", testname);
    }
    //    sendTextFrame(socket, "saveas url=file:///tmp/emptyempty.odt format= options=");

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

void TileCacheTests::testTileInvalidateCalc()
{
    const std::string testname = "tileInvalidateCalc ";
        std::shared_ptr<http::WebSocketSession> socket
            = loadDocAndGetSession(_socketPoll, "empty.ods", _uri, testname);
    helpers::sendTextFrame(socket, "uno .uno:GoToStart", testname);

    // Request a tile before expecting an invalidate.
    sendTextFrame(socket, "tilecombine nviewid=0 part=0 width=256 height=256 "
                           "tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840",
                  testname);
    std::vector<char> tile = getResponseMessage(socket, "tile:", testname);
    LOK_ASSERT_MESSAGE("Did not receive tile message as expected", !tile.empty());

    std::string text = "abcde";
    for (char ch : text)
    {
        sendChar(socket, ch, skNone, testname); // Send ordinary characters -> one tile invalidation for each
        assertResponseString(socket, "invalidatetiles:", testname);
    }

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

void TileCacheTests::testTileInvalidatePartCalc()
{
    const std::string filename = "setclientpart.ods";
    const std::string testname = "tileInvalidatePartCalc";
    const std::string testname1 = testname + "-1 ";
    const std::string testname2 = testname + "-2 ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL(filename, documentPath, documentURL, testname);
    std::shared_ptr<http::WebSocketSession> socket1
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname1);

    sendTextFrame(socket1, "setclientpart part=2", testname1);
    assertResponseString(socket1, "setpart:", testname1);
    sendTextFrame(socket1, "mouse type=buttondown x=1500 y=1500 count=1 buttons=1 modifier=0", testname1);

    std::shared_ptr<http::WebSocketSession> socket2
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname2);
    sendTextFrame(socket2, "setclientpart part=5", testname2);
    assertResponseString(socket2, "setpart:", testname2);
    sendTextFrame(socket2, "mouse type=buttondown x=1500 y=1500 count=1 buttons=1 modifier=0", testname2);

    constexpr std::string_view text = "Some test";
    for (char ch : text)
    {
        sendChar(socket1, ch, skNone, testname);
        sendChar(socket2, ch, skNone, testname);

        const auto response1 = assertResponseString(socket1, "invalidatetiles:", testname1);
        int value1;
        getPartFromInvalidateMessage(response1, value1);
        LOK_ASSERT_EQUAL(2, value1);

        const auto response2 = assertResponseString(socket2, "invalidatetiles:", testname2);
        int value2;
        getPartFromInvalidateMessage(response2, value2);
        LOK_ASSERT_EQUAL(5, value2);
    }

    socket1->asyncShutdown();
    socket2->asyncShutdown();

    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                       socket1->waitForDisconnection(5s));
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                       socket2->waitForDisconnection(5s));
}

void TileCacheTests::testTileInvalidatePartImpress()
{
    const std::string filename = "setclientpart.odp";
    const std::string testname = "tileInvalidatePartImpress";
    const std::string testname1 = testname + "-1 ";
    const std::string testname2 = testname + "-2 ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL(filename, documentPath, documentURL, testname);
    std::shared_ptr<http::WebSocketSession> socket1
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname1);

    sendTextFrame(socket1, "setclientpart part=2", testname1);
    assertResponseString(socket1, "setpart:", testname1);
    sendTextFrame(socket1, "mouse type=buttondown x=1500 y=1500 count=1 buttons=1 modifier=0", testname1);

    std::shared_ptr<http::WebSocketSession> socket2
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname2);
    sendTextFrame(socket2, "setclientpart part=5", testname2);
    assertResponseString(socket2, "setpart:", testname2);
    sendTextFrame(socket2, "mouse type=buttondown x=1500 y=1500 count=1 buttons=1 modifier=0", testname2);

    // This should be short, as in odp the font is large and we leave the page otherwise.
    constexpr std::string_view text = "Some test";
    for (char ch : text)
    {
        sendChar(socket1, ch, skNone, testname);
        sendChar(socket2, ch, skNone, testname);

        const auto response1 = assertResponseString(socket1, "invalidatetiles:", testname1);
        int value1;
        getPartFromInvalidateMessage(response1, value1);
        LOK_ASSERT_EQUAL(2, value1);

        const auto response2 = assertResponseString(socket2, "invalidatetiles:", testname2);
        int value2;
        getPartFromInvalidateMessage(response2, value2);
        LOK_ASSERT_EQUAL(5, value2);
    }

    socket1->asyncShutdown();
    socket2->asyncShutdown();

    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                       socket1->waitForDisconnection(5s));
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                       socket2->waitForDisconnection(5s));
}

void TileCacheTests::checkTiles(std::shared_ptr<http::WebSocketSession>& socket,
                                const std::string& docType, const std::string& testname)
{
    int currentPart = -1;
    int totalParts = 0;
    int docHeight = 0;
    int docWidth = 0;
    int docViewId = -1;

    // check total slides 10
    sendTextFrame(socket, "status", testname);
    const auto response = assertResponseString(socket, "status:", testname);
    {
        std::string text = docType;

        parseDocSize(response.substr(7), docType, currentPart, totalParts, docWidth, docHeight,
                     docViewId, testname);

        LOK_ASSERT_EQUAL(docType, text);
        LOK_ASSERT_EQUAL(10, totalParts);
        LOK_ASSERT(currentPart > -1);
        LOK_ASSERT(docWidth > 0);
        LOK_ASSERT(docHeight > 0);
    }

    if (docType == "presentation")
    {
        // request tiles
        TST_LOG("Requesting Impress tiles.");
        requestTiles(socket, docType, currentPart, docWidth, docHeight, testname);
    }

    // random setclientpart
    std::vector<int> parts = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    std::mt19937 random;
    random.seed(std::time(nullptr));
    std::shuffle(parts.begin(), parts.end(), random);
    int requests = 0;
    for (int it : parts)
    {
        if (currentPart != it)
        {
            // change part
            std::ostringstream oss;
            oss << "setclientpart part=" << it;
            sendTextFrame(socket, oss.str(), testname);
            // Wait for the change to take effect otherwise we get invalidatetile
            // which removes our next tile request subscription (expecting us to
            // issue a new tile request as a response, which a real client would do).
            assertResponseString(socket, "setpart:", testname);

            requestTiles(socket, docType, it, docWidth, docHeight, testname);

            if (++requests >= 3)
            {
                // No need to test all parts.
                TST_LOG("Breaking checkTiles for " << testname);
                break;
            }
        }

        currentPart = it;
    }
}

void TileCacheTests::requestTiles(std::shared_ptr<http::WebSocketSession>& socket,
                                  const std::string&, const int part, const int docWidth,
                                  const int docHeight, const std::string& testname)
{
    // twips
    const int tileSize = 3840;
    // pixel
    const int pixTileSize = 256;

    int rows;
    int cols;
    int tileX;
    int tileY;
    int tileWidth;
    int tileHeight;

    std::string text;
    std::string tile;

    rows = docHeight / tileSize;
    cols = docWidth / tileSize;
    TST_LOG("requestTiles for " << testname << " will request " << rows << " rows and " << cols
                                << " cols.");

    // Note: this code tests tile requests in the wrong way.

    // This code does NOT match what was the idea how the COOL protocol should/could be used. The
    // intent was never that the protocol would need to be, or should be, used in a strict
    // request/reply fashion. If a client needs n tiles, it should just send the requests, one after
    // another. There is no need to do n roundtrips. A client should all the time be reading
    // incoming messages, and handle incoming tiles as appropriate. There should be no expectation
    // that tiles arrive at the client in the same order that they were requested.

    // But, whatever.

    for (int itRow = 0; itRow < rows; ++itRow)
    {
        for (int itCol = 0; itCol < cols; ++itCol)
        {
            tileWidth = tileSize;
            tileHeight = tileSize;
            tileX = tileSize * itCol;
            tileY = tileSize * itRow;
            std::ostringstream oss;
            oss << "tile nviewid=0 part=" << part << " width=" << pixTileSize
                << " height=" << pixTileSize << " tileposx=" << tileX << " tileposy=" << tileY
                << " tilewidth=" << tileWidth << " tileheight=" << tileHeight;
            text = oss.str();

            sendTextFrame(socket, text, testname);
            tile = assertResponseString(socket, "tile:", testname);
            // expected tile: part= width= height= tileposx= tileposy= tilewidth= tileheight=
            StringVector tokens(StringVector::tokenize(tile, ' '));
            LOK_ASSERT_EQUAL_STR("tile:", tokens[0]);
            LOK_ASSERT_EQUAL(1000, NumUtil::stoi(tokens[1].substr(std::string_view("nviewid=").size())));
            LOK_ASSERT_EQUAL(part, NumUtil::stoi(tokens[2].substr(std::string_view("part=").size())));
            LOK_ASSERT_EQUAL(pixTileSize,
                             NumUtil::stoi(tokens[3].substr(std::string_view("width=").size())));
            LOK_ASSERT_EQUAL(pixTileSize,
                             NumUtil::stoi(tokens[4].substr(std::string_view("height=").size())));
            LOK_ASSERT_EQUAL(tileX,
                             NumUtil::stoi(tokens[5].substr(std::string_view("tileposx=").size())));
            LOK_ASSERT_EQUAL(tileY,
                             NumUtil::stoi(tokens[6].substr(std::string_view("tileposy=").size())));
            LOK_ASSERT_EQUAL(tileWidth,
                             NumUtil::stoi(tokens[7].substr(std::string_view("tileWidth=").size())));
            LOK_ASSERT_EQUAL(tileHeight,
                             NumUtil::stoi(tokens[8].substr(std::string_view("tileHeight=").size())));
        }
    }

    TST_LOG("requestTiles for " << testname << " finished.");
}

void TileCacheTests::testTileRequestByInvalidation()
{
    const char* testname = "tileRequestByInvalidation ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL("empty.odt", documentPath, documentURL, testname);

    std::shared_ptr<http::WebSocketSession> socket
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

    // Request a tile before expecting an invalidate.
    sendTextFrame(socket, "tilecombine nviewid=0 part=0 width=256 height=256 "
                           "tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840",
                  testname);
    std::vector<char> tile = getResponseMessage(socket, "tile:", testname);
    LOK_ASSERT_MESSAGE("Did not receive tile message as expected", !tile.empty());

    // 1. use case: invalidation without having a valid visible area in wsd
    // Type one character to trigger invalidation
    sendChar(socket, 'x', skNone, testname);

    // First wsd forwards the invalidation
    assertResponseString(socket, "invalidatetiles:", testname);

    // Since we did not set client visible area wsd won't send tile
    tile = getResponseMessage(socket, "tile:", testname);
    LOK_ASSERT_MESSAGE("Not expected tile message arrived!", tile.empty());

    // 2. use case: invalidation of one tile inside the client visible area
    // Now set the client visible area
    sendTextFrame(socket, "clientvisiblearea x=-4005 y=0 width=50490 height=72300", testname);
    sendTextFrame(
        socket,
        "clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=3840 tiletwipheight=3840",
        testname);

    // Type one character to trigger invalidation
    sendChar(socket, 'x', skNone, testname);

    // First wsd forwards the invalidation
    assertResponseString(socket, "invalidatetiles:", testname);

    // Then sends the new tile which was invalidated inside the visible area
    assertResponseString(socket, "delta:", testname);

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

void TileCacheTests::testTileRequestByZoom()
{
    // By zoom the client requests all the tile of the visible area
    // Server should push all these tiles to the network, so tiles-on-fly should be bigger than this count

    const char* testname = "testTileRequestByZoom ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL("empty.odt", documentPath, documentURL, testname);
    std::shared_ptr<http::WebSocketSession> socket
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

    // Set the client visible area
    sendTextFrame(socket, "clientvisiblearea x=-2662 y=0 width=16000 height=9875", testname);
    sendTextFrame(
        socket,
        "clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=3200 tiletwipheight=3200",
        testname);

    // Request all tile of the visible area (it happens by zoom)
    sendTextFrame(socket,
                  "tilecombine nviewid=0 part=0 width=256 height=256 "
                  "tileposx=0,3200,6400,9600,12800,0,3200,6400,9600,12800,0,3200,6400,9600,12800,0,"
                  "3200,6400,9600,12800 "
                  "tileposy=0,0,0,0,0,3200,3200,3200,3200,3200,6400,6400,6400,6400,6400,9600,9600,"
                  "9600,9600,9600 tilewidth=3200 tileheight=3200",
                  testname);

    // Check that we get all the tiles without we send back the tileprocessed message
    for (int i = 0; i < 20; ++i)
    {
        std::vector<char> tile = getResponseMessage(socket, "tile:", testname);
        LOK_ASSERT_MESSAGE("Did not get tile as expected!", !tile.empty());
    }

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

void TileCacheTests::testTileWireIDHandling()
{
    const char* testname = "testTileWireIDHandling ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL("empty.odt", documentPath, documentURL, testname);
    std::shared_ptr<http::WebSocketSession> socket
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

    // Set the client visible area
    sendTextFrame(socket, "clientvisiblearea x=-4005 y=0 width=50490 height=72300", testname);
    sendTextFrame(
        socket,
        "clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=3840 tiletwipheight=3840",
        testname);

    // Type one character to trigger invalidation
    sendChar(socket, 'x', skNone, testname);

    // First wsd forwards the invalidation
    assertResponseString(socket, "invalidatetiles:", testname);

    // For the first input wsd will send all invalidated tiles
    LOK_ASSERT_MESSAGE("Expected at least two tiles.",
                       countMessages(socket, "tile:", testname, 500ms) > 1);

    // Let WSD know we got these so it wouldn't stop sending us modified tiles automatically.
    for (;;)
    {
        const auto tile = getResponseDesc(socket, "tile:", testname, 500ms);
        if (!tile)
            break;

        sendTextFrame(socket, "tileprocessed wids=" + std::to_string(tile->getWireId()), testname);
    }

    // Type another character
    sendChar(socket, 'y', skNone, testname);
    assertResponseString(socket, "invalidatetiles:", testname);

    // For the second input wsd will send one tile, since some of them are identical.
    const int arrivedTiles = countMessages(socket, "delta:", testname, 500ms);
    if (arrivedTiles == 1)
        return;

    // Or, at most 2. The reason is that sometimes we get line antialiasing differences that
    // are sub-pixel different, and that results in a different hash.
    LOK_ASSERT_EQUAL(2, arrivedTiles);

    // The third time, however, we shouldn't see anything but the tile we change.
    sendChar(socket, 'z', skNone, testname);
    assertResponseString(socket, "invalidatetiles:", testname);

    LOK_ASSERT_MESSAGE("Expected exactly one tile.",
                       countMessages(socket, "delta:", testname, 500ms) == 1);

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

void TileCacheTests::testTileProcessed()
{
    // Test whether tileprocessed message removes the tiles from the internal tiles-on-fly list
    const char* testname = "testTileProcessed ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL("empty.odt", documentPath, documentURL, testname);
    std::shared_ptr<http::WebSocketSession> socket
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

    // Set the client visible area
    sendTextFrame(socket, "clientvisiblearea x=-2662 y=0 width=10000 height=9000", testname);
    sendTextFrame(
        socket,
        "clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=3200 tiletwipheight=3200",
        testname);

    for (int i = 0; i < 100; ++i)
        getResponseMessage(socket, "spinandwait:", testname, 10ms);

    // Request a lots of tiles ~25 ie. more than wsd can send back at once.
    sendTextFrame(socket,
                  "tilecombine nviewid=0 part=0 width=256 height=256 "
                  "tileposx=0,3200,6400,9600,12800,0,3200,6400,9600,12800,0,3200,6400,9600,12800,0,"
                  "3200,6400,9600,12800,0,3200,6400,9600,12800 "
                  "tileposy=0,0,0,0,0,3200,3200,3200,3200,3200,6400,6400,6400,6400,6400,9600,9600,"
                  "9600,9600,9600,12800,12800,12800,12800,12800 tilewidth=3200 tileheight=3200",
                  testname);

    std::vector<int> wids;
    int arrivedTile = 0;
    bool gotTile = false;
    do
    {
        std::string tile = getResponseString(socket, "tile:", testname);
        gotTile = !tile.empty();
        if(gotTile)
        {
            ++arrivedTile;

            // Store tileID, so we can send it back
            StringVector tokens(StringVector::tokenize(tile, ' '));
            TileDesc desc = TileDesc::parse(tokens);

            wids.push_back(desc.getWireId());
        }

    } while(gotTile);

    LOK_ASSERT_EQUAL_MESSAGE("Expected exactly the requested number of tiles", 25, arrivedTile);

    std::ostringstream oss;
    for (int wid : wids)
    {
        oss << wid << ',';
    }

    sendTextFrame(socket, "tileprocessed wids=" + oss.str(), testname);

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

void TileCacheTests::testTileInvalidatedOutside()
{
    // Test whether wsd sends us the tiles which are hanging out the visible area
    const char* testname = "testTileInvalidatedOutside ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL("empty.odt", documentPath, documentURL, testname);
    std::shared_ptr<http::WebSocketSession> socket
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

    // Type one character to trigger invalidation and get the invalidation rectangle
    sendChar(socket, 'x', skNone, testname);

    // First wsd forwards the invalidation
    const std::string sInvalidate = assertResponseString(socket, "invalidatetiles:", testname);
    LOK_ASSERT_MESSAGE("Expected invalidatetiles message.", !sInvalidate.empty());
    StringVector tokens(StringVector::tokenize(sInvalidate, ' '));
    LOK_ASSERT_MESSAGE("Expected at least 6 tokens.", tokens.size() >= 6);
    const int y = NumUtil::stoi(tokens[3].substr(std::string_view("y=").size()));
    const int height = NumUtil::stoi(tokens[5].substr(std::string_view("height=").size()));

    // Set client visible area to make it not having intersection with the invalidate rectangle, but having shared tiles
    std::ostringstream oss;
    oss << "clientvisiblearea x=0 y=" << (y + height + 100) << " width=50490 height=72300";
    sendTextFrame(socket, oss.str(), testname);
    sendTextFrame(
        socket,
        "clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=3840 tiletwipheight=3840",
        testname);

    // Type one character to trigger invalidation
    sendChar(socket, 'x', skNone, testname);

    // First wsd forwards the invalidation
    assertResponseString(socket, "invalidatetiles:", testname);

    // Since the invalidation rectangle is outside the visible area
    // wsd does not send a new tile even if some of the invalidated tiles
    // are partly visible.
    std::vector<char> tile = getResponseMessage(socket, "tile:", testname);
    LOK_ASSERT_MESSAGE("Not expected tile message arrived!", tile.empty());

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

void TileCacheTests::testTileBeingRenderedHandling()
{
    // The issue here was that we requested the tile of the same tile twice
    // and so sometimes we got the same tile message twice from wsd.
    const char* testname = "testTileBeingRenderedHandling ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL("empty.odt", documentPath, documentURL, testname);
    std::shared_ptr<http::WebSocketSession> socket
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

    // Set the client visible area
    sendTextFrame(socket, "clientvisiblearea x=-2662 y=0 width=16000 height=9875", testname);
    sendTextFrame(
        socket,
        "clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=3200 tiletwipheight=3200",
        testname);

    // Type one character to trigger invalidation
    sendChar(socket, 'x', skNone, testname);

    // First wsd forwards the invalidation
    assertResponseString(socket, "invalidatetiles:", testname);

    // For the first input wsd will send all invalidated tiles
    LOK_ASSERT_MESSAGE("Expected at least two tiles.",
                       countMessages(socket, "tile:", testname, 500ms) > 1);

    // For the later inputs wsd will send one tile, since other ones are identical
    for(int i = 0; i < 5; ++i)
    {
        sendTextFrame(socket, "tileprocessed tile=0:0:0:3200:3200:0", testname);

        // Type another character
        sendChar(socket, 'y', skNone, testname);
        assertResponseString(socket, "invalidatetiles:", testname);

        const int arrivedTiles = countMessages(socket, "delta:", testname, 500ms);
        if (arrivedTiles != 1)
        {
            // Or, at most 2. The reason is that sometimes we get line antialiasing differences that
            // are sub-pixel different, and that results in a different hash.
            LOK_ASSERT_MESSAGE("Expected at most 3 tiles--though really there should be only 1", 3 <= arrivedTiles);

            sendTextFrame(socket, "tileprocessed tile=0:0:0:3200:3200:0", testname);

            // The third time, however, we shouldn't see anything but the tile we change.
            sendChar(socket, 'z', skNone, testname);
            assertResponseString(socket, "invalidatetiles:", testname);

            LOK_ASSERT_MESSAGE("Expected exactly one tile.",
                               countMessages(socket, "delta:", testname, 500ms) == 1);
        }
    }

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

void TileCacheTests::testWireIDFilteringOnWSDSide()
{
    const char* testname = "testWireIDFilteringOnWSDSide ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL("empty.odt", documentPath, documentURL, testname);
    std::shared_ptr<http::WebSocketSession> socket1
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);
    // Set the client visible area
    sendTextFrame(socket1, "clientvisiblearea x=-4005 y=0 width=50490 height=72300", testname);
    sendTextFrame(
        socket1,
        "clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=3840 tiletwipheight=3840",
        testname);

    std::shared_ptr<http::WebSocketSession> socket2
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname, true);
    // Set the client visible area
    sendTextFrame(socket1, "clientvisiblearea x=-4005 y=0 width=50490 height=72300", testname);
    sendTextFrame(
        socket1,
        "clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=3840 tiletwipheight=3840",
        testname);

    //1. First make the first client to trigger the kit to filter out tiles based on identical wireIDs

    // Type one character to trigger invalidation
    sendChar(socket1, 'x', skNone, testname);

    // First wsd forwards the invalidation
    assertResponseString(socket1, "invalidatetiles:", testname);

    // For the first input wsd will send all invalidated tiles
    LOK_ASSERT_MESSAGE("Expected at least two tiles.",
                       countMessages(socket1, "tile:", testname, 1s) > 1);

    // Let WSD know we got these so it wouldn't stop sending us modified tiles automatically.
    sendTextFrame(socket1, "tileprocessed tile=0:0:0:3840:3840:0", testname);
    sendTextFrame(socket1, "tileprocessed tile=0:3840:0:3840:3840:0", testname);
    sendTextFrame(socket1, "tileprocessed tile=0:7680:0:3840:3840:0", testname);

    // Type another character
    sendChar(socket1, 'y', skNone, testname);
    assertResponseString(socket1, "invalidatetiles:", testname);

    // For the second input wsd will send one tile, since some of them are identical.
    const int arrivedTiles = countMessages(socket1, "tile:", testname, 1s);
    if (arrivedTiles == 1)
        return;

    // Or, at most 2. The reason is that sometimes we get line antialiasing differences that
    // are sub-pixel different, and that results in a different hash.
    LOK_ASSERT_MESSAGE("Expected at most 3 tiles.", arrivedTiles <= 3);

    // The third time, however, we shouldn't see anything but the tile we change.
    sendChar(socket1, 'z', skNone, testname);
    assertResponseString(socket1, "invalidatetiles:", testname);

    LOK_ASSERT_MESSAGE("Expected exactly one tile.",
                       countMessages(socket1, "delta:", testname, 1s) == 1);

    //2. Now request the same tiles by the other client (e.g. scroll to the same view)

    sendTextFrame(socket2,
                  "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,7680 "
                  "tileposy=0,0,0 tilewidth=3840 tileheight=3840",
                  testname);

    // We expect three tiles sent to the second client
    LOK_ASSERT_EQUAL(3, countMessages(socket2, "tile:", testname, 1s));

    // wsd should not send tiles messages for the first client
    const std::vector<char> tile = getResponseMessage(socket1, "tile:", testname, 1s);
    LOK_ASSERT_MESSAGE("Not expected tile message arrived!", tile.empty());

    socket1->asyncShutdown();
    socket2->asyncShutdown();

    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 1",
                       socket1->waitForDisconnection(5s));
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket 2",
                       socket2->waitForDisconnection(5s));
}

void TileCacheTests::testLimitTileVersionsOnFly()
{
    // We have an upper limit (2) for the versions of the same tile wsd send out
    // without getting the tileprocessed message for the first tile message.
    const char* testname = "testLimitTileVersionsOnFly ";

    std::string documentPath, documentURL;
    getDocumentPathAndURL("empty.odt", documentPath, documentURL, testname);
    std::shared_ptr<http::WebSocketSession> socket
        = loadDocAndGetSession(_socketPoll, _uri, documentURL, testname);

    // Set the client visible area
    sendTextFrame(socket, "clientvisiblearea x=-2662 y=0 width=16000 height=9875", testname);
    sendTextFrame(
        socket,
        "clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=3200 tiletwipheight=3200",
        testname);

    // Type one character to trigger sending tiles
    sendChar(socket, 'x', skNone, testname);

    // Handle all tiles send by wsd
    bool getTileResp = false;
    do
    {
        const std::string tile = getResponseString(socket, "tile:", testname, 1000ms);
        getTileResp = !tile.empty();
    } while(getTileResp);

    // Type another character to trigger sending tiles
    sendChar(socket, 'x', skNone, testname);

    // Handle all tiles sent by wsd
    do
    {
        const std::string tile = getResponseString(socket, "tile:", testname, 1000ms);
        getTileResp = !tile.empty();
    } while(getTileResp);

    // For the third invalidation wsd does not send the new tile since
    // two versions of the same tile were already sent.
    sendChar(socket, 'x', skNone, testname);

    const std::vector<char> tile1 = getResponseMessage(socket, "tile:", testname, 1000ms);
    LOK_ASSERT_MESSAGE("Not expected tile message arrived!", tile1.empty());

    // When the next tileprocessed message arrive with correct tileID
    // wsd sends the delayed tile
    sendTextFrame(socket, "tileprocessed tile=0:0:0:3200:3200:0", testname);

    int arrivedTiles = 0;
    bool gotTile = false;
    do
    {
        const std::vector<char> tile = getResponseMessage(socket, "tile:", testname, 1000ms);
        gotTile = !tile.empty();
        if(gotTile)
            ++arrivedTiles;
    } while(gotTile);

    LOK_ASSERT_EQUAL(1, arrivedTiles);

    socket->asyncShutdown();
    LOK_ASSERT_MESSAGE("Expected successful disconnection of the WebSocket",
                       socket->waitForDisconnection(5s));
}

CPPUNIT_TEST_SUITE_REGISTRATION(TileCacheTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
