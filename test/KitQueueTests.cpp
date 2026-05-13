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
 * Unit test for Kit queue and message handling functionality.
 */

#include <config.h>

#include <test/lokassert.hpp>

#include <Common.hpp>
#include <Protocol.hpp>
#include <Message.hpp>
#include <kit/KitQueue.hpp>
#include <SenderQueue.hpp>
#include <wsd/TileCache.hpp>
#include <common/Util.hpp>

#include <algorithm>
#include <sstream>

#include <cppunit/extensions/HelperMacros.h>

/// KitQueue unit-tests.
class KitQueueTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(KitQueueTests);

#if 0
    CPPUNIT_TEST(testKitQueuePriority);
    CPPUNIT_TEST(testViewOrder);
    CPPUNIT_TEST(testPreviewsDeprioritization);
#endif
    CPPUNIT_TEST(testTileCombinedRendering);
    CPPUNIT_TEST(testTileRecombining);
    CPPUNIT_TEST(testSenderQueue);
    CPPUNIT_TEST(testSenderQueueLog);
    CPPUNIT_TEST(testSenderQueueProgress);
    CPPUNIT_TEST(testSenderQueueTileDeduplication);
    CPPUNIT_TEST(testSenderQueueTileDedupReportsDroppedWireId);
    CPPUNIT_TEST(testTileCacheKitHangBecomesStale);
    CPPUNIT_TEST(testTileCacheStaleRenderIsReissued);
    CPPUNIT_TEST(testInvalidateViewCursorDeduplication);
    CPPUNIT_TEST(testCallbackModifiedStatusIsSkipped);
    CPPUNIT_TEST(testCallbackInvalidation);
    CPPUNIT_TEST(testCallbackIndicatorValue);
    CPPUNIT_TEST(testCallbackPageSize);
    CPPUNIT_TEST(testPutAndPop);
    CPPUNIT_TEST(testPopEmptyQueue);
    CPPUNIT_TEST(testClear);
    CPPUNIT_TEST(testPutEmptyPayload);
    CPPUNIT_TEST(testTextInputBlockedByKeyMessage);
    CPPUNIT_TEST(testTextInputBlockedByMouse);
    CPPUNIT_TEST(testTextInputBlockedByRemoveText);
    CPPUNIT_TEST(testTextInputDifferentId);
    CPPUNIT_TEST(testTextInputMissingTokens);
    CPPUNIT_TEST(testRemoveTextBlockedByKeyMessage);
    CPPUNIT_TEST(testRemoveTextBlockedByTextInput);
    CPPUNIT_TEST(testRemoveTextDifferentId);
    CPPUNIT_TEST(testCallbackStateChangedDedup);
    CPPUNIT_TEST(testCallbackStateChangedDifferentCommands);
    CPPUNIT_TEST(testCallbackStateChangedNoEquals);
    CPPUNIT_TEST(testCallbackInvalidationMergeSizeLimit);
    CPPUNIT_TEST(testCallbackInvalidationNoIntersection);
    CPPUNIT_TEST(testCallbackInvalidationDifferentModes);
    CPPUNIT_TEST(testCallbackCursorDedup);
    CPPUNIT_TEST(testCallbackViewCursorDedup);
    CPPUNIT_TEST(testPreviewTilesNoCombine);
    CPPUNIT_TEST(testTileDeduplicationOnPush);
    CPPUNIT_TEST(testMultiViewTileQueues);
    CPPUNIT_TEST(testGetCallbackBoolOverload);
    CPPUNIT_TEST(testCallbackInvalidationEmptyMode);

    CPPUNIT_TEST_SUITE_END();

#if 0
    void testKitQueuePriority();
    void testViewOrder();
    void testPreviewsDeprioritization();
#endif
    void testTileCombinedRendering();
    void testTileRecombining();
    void testSenderQueue();
    void testSenderQueueLog();
    void testSenderQueueProgress();
    void testSenderQueueTileDeduplication();
    void testSenderQueueTileDedupReportsDroppedWireId();
    void testTileCacheKitHangBecomesStale();
    void testTileCacheStaleRenderIsReissued();
    void testInvalidateViewCursorDeduplication();
    void testCallbackModifiedStatusIsSkipped();
    void testCallbackInvalidation();
    void testCallbackIndicatorValue();
    void testCallbackPageSize();
    void testPutAndPop();
    void testPopEmptyQueue();
    void testClear();
    void testPutEmptyPayload();
    void testTextInputBlockedByKeyMessage();
    void testTextInputBlockedByMouse();
    void testTextInputBlockedByRemoveText();
    void testTextInputDifferentId();
    void testTextInputMissingTokens();
    void testRemoveTextBlockedByKeyMessage();
    void testRemoveTextBlockedByTextInput();
    void testRemoveTextDifferentId();
    void testCallbackStateChangedDedup();
    void testCallbackStateChangedDifferentCommands();
    void testCallbackStateChangedNoEquals();
    void testCallbackInvalidationMergeSizeLimit();
    void testCallbackInvalidationNoIntersection();
    void testCallbackInvalidationDifferentModes();
    void testCallbackCursorDedup();
    void testCallbackViewCursorDedup();
    void testPreviewTilesNoCombine();
    void testTileDeduplicationOnPush();
    void testMultiViewTileQueues();
    void testGetCallbackBoolOverload();
    void testCallbackInvalidationEmptyMode();

    // Compat helper for tests
    std::string popHelper(KitQueue &queue)
    {
        TilePrioritizer::Priority dummy;

        TileCombined c = queue.popTileQueue(dummy);

        std::string result;
        if (c.getTiles().size() != 1)
            result = c.serialize("tilecombine");
        else
            result = c.getTiles()[0].serialize("tile");

        return result;
    }
};

#if 0
void KitQueueTests::testKitQueuePriority()
{
    constexpr std::string_view testname = __func__;

    const std::string reqHigh = "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840";
    const std::string resHigh = "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840 ver=-1";
    const KitQueue::Payload payloadHigh(resHigh.data(), resHigh.data() + resHigh.size());
    const std::string reqLow = "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=253440 tilewidth=3840 tileheight=3840";
    const std::string resLow = "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=253440 tilewidth=3840 tileheight=3840 ver=-1";
    const KitQueue::Payload payloadLow(resLow.data(), resLow.data() + resLow.size());

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // Request the tiles.
    queue.put(reqLow);
    queue.put(reqHigh);

    // Original order.
    LOK_ASSERT_EQUAL_STR(payloadLow, popHelper(queue));
    LOK_ASSERT_EQUAL_STR(payloadHigh, popHelper(queue));

    // Request the tiles.
    queue.put(reqLow);
    queue.put(reqHigh);
    queue.put(reqHigh);
    queue.put(reqLow);

    // Set cursor above reqHigh.
    queue.updateCursorPosition(0, 0, 0, 0, 10, 100);

    // Prioritized order.
    LOK_ASSERT_EQUAL_STR(payloadHigh, popHelper(queue));
    LOK_ASSERT_EQUAL_STR(payloadLow, popHelper(queue));

    // Repeat with cursor position set.
    queue.put(reqLow);
    queue.put(reqHigh);
    LOK_ASSERT_EQUAL_STR(payloadHigh, popHelper(queue));
    LOK_ASSERT_EQUAL_STR(payloadLow, popHelper(queue));

    // Repeat by changing cursor position.
    queue.put(reqLow);
    queue.put(reqHigh);
    queue.updateCursorPosition(0, 0, 0, 253450, 10, 100);
    LOK_ASSERT_EQUAL_STR(payloadLow, popHelper(queue));
    LOK_ASSERT_EQUAL_STR(payloadHigh, popHelper(queue));
}
#endif

void KitQueueTests::testTileCombinedRendering()
{
    constexpr std::string_view testname = __func__;

    const std::string req1 = "tile nviewid=0 nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840";
    const std::string req2 = "tile nviewid=0 part=0 width=256 height=256 tileposx=3840 tileposy=0 tilewidth=3840 tileheight=3840";
    const std::string req3 = "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=3840 tilewidth=3840 tileheight=3840";

    const std::string resHor = "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 tilewidth=3840 tileheight=3840 ver=-1,-1";
    const KitQueue::Payload payloadHor(resHor.data(), resHor.data() + resHor.size());
    const std::string resVer = "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,0 tileposy=0,3840 tilewidth=3840 tileheight=3840 ver=-1,-1";
    const KitQueue::Payload payloadVer(resVer.data(), resVer.data() + resVer.size());
    const std::string resFull = "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,0 tileposy=0,0,3840 tilewidth=3840 tileheight=3840 ver=-1,-1,-1";
    const KitQueue::Payload payloadFull(resFull.data(), resFull.data() + resFull.size());

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // Horizontal.
    queue.put(req1);
    queue.put(req2);
    LOK_ASSERT_EQUAL_STR(payloadHor, popHelper(queue));

    // Vertical.
    queue.put(req1);
    queue.put(req3);
    LOK_ASSERT_EQUAL_STR(payloadVer, popHelper(queue));

    // Vertical.
    queue.put(req1);
    queue.put(req2);
    queue.put(req3);
    LOK_ASSERT_EQUAL_STR(payloadFull, popHelper(queue));
}

void KitQueueTests::testTileRecombining()
{
    constexpr std::string_view testname = __func__;

    class TestPrioritizer : public TilePrioritizer {
        int _prioX = 0;
        int _prioY = 0;
    public:
        virtual Priority getTilePriority(const TileDesc& tile) const
        {
            if (tile.getTilePosX() == _prioX && tile.getTilePosY() == _prioY)
                return TilePrioritizer::Priority::ULTRAHIGH;
            return TilePrioritizer::Priority::NORMAL;
        }
        void setHighestPrio(int prioX, int prioY)
        {
            _prioX = prioX;
            _prioY = prioY;
        }
    };


    TestPrioritizer prio;
    KitQueue queue(prio);

    {
        queue.put("tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,7680 tileposy=0,0,0 tilewidth=3840 tileheight=3840");
        queue.put("tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 tilewidth=3840 tileheight=3840");

        // the tilecombine's get merged, resulting in 3 "tile" messages
        LOK_ASSERT_EQUAL(3, static_cast<int>(queue.getTileQueueSize()));

        // but when we later extract that, it is just one "tilecombine" message
        LOK_ASSERT_EQUAL_STR(
            "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,7680 tileposy=0,0,0 "
            "tilewidth=3840 tileheight=3840 ver=-1,-1,-1",
            popHelper(queue));

        // and nothing remains in the queue
        LOK_ASSERT_EQUAL(0, static_cast<int>(queue.getTileQueueSize()));
    }

    // Set the 2nd tile on the first row as the prio tile, with one candidate
    // on the same row relatively distant to the left.
    // The following row is adjacent to the first row, and has two candidates
    // relatively distant to the right of the prio tile, but very distant from
    // the first tile in the first row.
    {
        prio.setHighestPrio(23040, 268800);

        // notional grid positions of 0:140, 12:140, 26:141, 27:141
        queue.put("tilecombine nviewid=1000 part=0 width=256 height=256 "
                  "tileposx=0,23040,49920,51840 tileposy=268800,268800,270720,270720 "
                  "tilewidth=1920 tileheight=1920 ver=-1,-1,-1,-1");

        // rearrange this to avoid excessively large tile combines
        // 0:140, 12:140
        LOK_ASSERT_EQUAL_STR(
            "tilecombine nviewid=1000 part=0 width=256 height=256 "
            "tileposx=0,23040 tileposy=268800,268800 tilewidth=1920 "
            "tileheight=1920 ver=-1,-1",
            popHelper(queue));

        // 26:141, 27:141
        LOK_ASSERT_EQUAL_STR(
            "tilecombine nviewid=1000 part=0 width=256 height=256 "
            "tileposx=49920,51840 tileposy=270720,270720 "
            "tilewidth=1920 tileheight=1920 ver=-1,-1",
            popHelper(queue));

        // and nothing remains in the queue
        LOK_ASSERT_EQUAL(0, static_cast<int>(queue.getTileQueueSize()));
    }
}

#if 0
void KitQueueTests::testViewOrder()
{
    constexpr std::string_view testname = __func__;

    class TestPrioritizer : public TilePrioritizer {
    public:
        virtual Priority getTilePriority(const TileDesc &) const
        {
            // FIXME: implement cursor priority hooks.
            return Priority::NORMAL;
        }
    };
    TestPrioritizer dummy;
    KitQueue queue(dummy);

    // should result in the 3, 2, 1, 0 order of the views
    queue.updateCursorPosition(0, 0, 0, 0, 10, 100);
    queue.updateCursorPosition(2, 0, 0, 0, 10, 100);
    queue.updateCursorPosition(1, 0, 0, 7680, 10, 100);
    queue.updateCursorPosition(3, 0, 0, 0, 10, 100);
    queue.updateCursorPosition(2, 0, 0, 15360, 10, 100);
    queue.updateCursorPosition(3, 0, 0, 23040, 10, 100);

    const std::vector<std::string> tiles =
    {
        "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840 ver=-1",
        "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=7680 tilewidth=3840 tileheight=3840 ver=-1",
        "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=15360 tilewidth=3840 tileheight=3840 ver=-1",
        "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=23040 tilewidth=3840 tileheight=3840 ver=-1"
    };

    for (auto &tile : tiles)
        queue.put(tile);

    LOK_ASSERT_EQUAL(4, static_cast<int>(queue.getTileQueueSize()));

    // should result in the 3, 2, 1, 0 order of the tiles thanks to the cursor
    // positions
    for (size_t i = 0; i < tiles.size(); ++i)
    {
        LOK_ASSERT_EQUAL_STR(tiles[3 - i], popHelper(queue));
    }
}

void KitQueueTests::testPreviewsDeprioritization()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // simple case - put previews to the queue and get everything back again
    const std::vector<std::string> previews =
    {
        "tile nviewid=0 part=0 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 ver=-1 id=0",
        "tile nviewid=0 part=1 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 ver=-1 id=1",
        "tile nviewid=0 part=2 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 ver=-1 id=2",
        "tile nviewid=0 part=3 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 ver=-1 id=3"
    };

    for (auto &preview : previews)
        queue.put(preview);

    for (size_t i = 0; i < previews.size(); ++i)
    {
        LOK_ASSERT_EQUAL_STR(previews[i], popHelper(queue));
    }

    // stays empty after all is done
    LOK_ASSERT_EQUAL(0, static_cast<int>(queue.getTileQueueSize()));

    // re-ordering case - put previews and normal tiles to the queue and get
    // everything back again but this time the tiles have to interleave with
    // the previews
    const std::vector<std::string> tiles =
    {
        "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840 ver=-1",
        "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=7680 tilewidth=3840 tileheight=3840 ver=-1"
    };

    for (auto &preview : previews)
        queue.put(preview);

    queue.put(tiles[0]);

    LOK_ASSERT_EQUAL_STR(previews[0], popHelper(queue));
    LOK_ASSERT_EQUAL_STR(tiles[0], popHelper(queue));
    LOK_ASSERT_EQUAL_STR(previews[1], popHelper(queue));

    queue.put(tiles[1]);

    LOK_ASSERT_EQUAL_STR(previews[2], popHelper(queue));
    LOK_ASSERT_EQUAL_STR(tiles[1], popHelper(queue));
    LOK_ASSERT_EQUAL_STR(previews[3], popHelper(queue));

    // stays empty after all is done
    LOK_ASSERT_EQUAL(0, static_cast<int>(queue.getTileQueueSize()));

    // cursor positioning case - the cursor position should not prioritize the
    // previews
    queue.updateCursorPosition(0, 0, 0, 0, 10, 100);

    queue.put(tiles[1]);
    queue.put(previews[0]);

    LOK_ASSERT_EQUAL_STR(tiles[1], popHelper(queue));
    LOK_ASSERT_EQUAL_STR(previews[0], popHelper(queue));

    // stays empty after all is done
    LOK_ASSERT_EQUAL(0, static_cast<int>(queue.getTileQueueSize()));
}
#endif

namespace {
    std::string msgStr(const std::shared_ptr<Message> &item)
    {
        return std::string(item->data().data(), item->data().size());
    }
}

void KitQueueTests::testSenderQueue()
{
    constexpr std::string_view testname = __func__;

    SenderQueue<std::shared_ptr<Message>> queue;

    std::shared_ptr<Message> item;

    // Empty queue
    LOK_ASSERT_EQUAL_STR(false, queue.dequeue(item));
    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());

    const std::vector<std::string> messages =
    {
        "message 1",
        "message 2",
        "message 3"
    };

    for (const auto& msg : messages)
    {
        queue.enqueue(std::make_shared<Message>(msg, Message::Dir::Out));
    }

    LOK_ASSERT_EQUAL(static_cast<size_t>(3), queue.size());

    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.size());
    LOK_ASSERT(item);
    LOK_ASSERT_EQUAL(messages[0], msgStr(item));

    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.size());
    LOK_ASSERT(item);
    LOK_ASSERT_EQUAL(messages[1], msgStr(item));

    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());
    LOK_ASSERT(item);
    LOK_ASSERT_EQUAL(messages[2], msgStr(item));

    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());
}

void KitQueueTests::testSenderQueueLog()
{
    constexpr std::string_view testname = __func__;

    SenderQueue<std::shared_ptr<Message>> queue;

    std::shared_ptr<Message> item;

    const std::vector<std::string> messages =
    {
        "just one",
        "message",
        "message",
        "another",
        "another",
        "another",
        "single one",
        "last",
        "last"
    };

    for (const auto& msg : messages)
    {
        queue.enqueue(std::make_shared<Message>(msg, Message::Dir::Out));
    }

    std::ostringstream str(Util::makeDumpStateStream());
    queue.dumpState(str);

    std::string result = "\t\tqueue items: 9\n"
        "\t\t\ttype: text: o4 - just one\n"
        "\t\t\ttype: text: o5 - message\n"
        "\t\t\t<repeats 1 times>\n"
        "\t\t\ttype: text: o7 - another\n"
        "\t\t\t<repeats 2 times>\n"
        "\t\t\ttype: text: o10 - single one\n"
        "\t\t\ttype: text: o11 - last\n"
        "\t\t\t<repeats 1 times>\n"
        "\t\tqueue size: 61 bytes\n";

    LOK_ASSERT_EQUAL(result, str.str());
}

void KitQueueTests::testSenderQueueProgress()
{
    constexpr std::string_view testname = __func__;

    SenderQueue<std::shared_ptr<Message>> queue;

    std::shared_ptr<Message> item;

    const std::vector<std::string> messages =
    {
        "progress: { \"id\":\"start\", \"text\":\"hello world\" }",
        "progress: { \"id\":\"setvalue\", \"value\":1 }",
        "progress: { \"id\":\"setvalue\", \"value\":5 }",
        "progress: { \"id\":\"setvalue\", \"value\":25 }",
        "progress: { \"id\":\"finish\" }"
    };

    for (const auto& msg : messages)
        queue.enqueue(std::make_shared<Message>(msg, Message::Dir::Out));

    LOK_ASSERT_EQUAL(static_cast<size_t>(3), queue.size());

    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT(item);
    LOK_ASSERT_EQUAL(messages[0], msgStr(item));

    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT(item);
    LOK_ASSERT_EQUAL(messages[3], msgStr(item));

    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT(item);
    LOK_ASSERT_EQUAL(messages[4], msgStr(item));

    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());
}

void KitQueueTests::testSenderQueueTileDeduplication()
{
    constexpr std::string_view testname = __func__;

    SenderQueue<std::shared_ptr<Message>> queue;

    std::shared_ptr<Message> item;

    // Empty queue
    LOK_ASSERT_EQUAL_STR(false, queue.dequeue(item));
    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());

    const std::vector<std::string> part_messages =
    {
        "tile: nviewid=0 part=0 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 ver=0",
        "tile: nviewid=0 part=1 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 ver=1",
        "tile: nviewid=0 part=2 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 ver=-1"
    };

    for (const auto& msg : part_messages)
    {
        queue.enqueue(std::make_shared<Message>(msg, Message::Dir::Out));
    }

    LOK_ASSERT_EQUAL(static_cast<size_t>(3), queue.size());
    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));

    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());

    const std::vector<std::string> dup_messages =
    {
        "tile: nviewid=0 part=0 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 ver=-1",
        "tile: nviewid=0 part=0 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 ver=1",
        "tile: nviewid=0 part=0 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 ver=1"
    };

    for (const auto& msg : dup_messages)
    {
        queue.enqueue(std::make_shared<Message>(msg, Message::Dir::Out));
    }

    LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.size());
    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));

    LOK_ASSERT(item);

    // The last one should persist.
    LOK_ASSERT_EQUAL(dup_messages[2], msgStr(item));

    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());
}

// Reproduces the lost-tile / on-fly-leak scenario.
//
// When a queued tile message is replaced by a newer one for the same position,
// SenderQueue::deduplicate erases the older entry from the queue. The older
// tile never reaches the client, so the client never sends tileprocessed for
// its wireId. Without reporting which wireId was dropped, ClientSession's
// _tilesOnFly tracking would leak: the dropped wireId would stay there until
// the 10s round-trip timeout, eating slots in tilesOnFlyUpperLimit and
// stalling tile delivery.
void KitQueueTests::testSenderQueueTileDedupReportsDroppedWireId()
{
    constexpr std::string_view testname = __func__;

    SenderQueue<std::shared_ptr<Message>> queue;

    auto makeTile = [](TileWireId wid)
    {
        std::ostringstream oss;
        oss << "tile: nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0"
               " tilewidth=3840 tileheight=3840 oldwid=1 wid=" << wid << " ver=-1";
        return std::make_shared<Message>(oss.str(), Message::Dir::Out);
    };

    auto makeTileAt = [](int posX, int posY, TileWireId wid)
    {
        std::ostringstream oss;
        oss << "tile: nviewid=0 part=0 width=256 height=256 tileposx=" << posX
            << " tileposy=" << posY
            << " tilewidth=3840 tileheight=3840 oldwid=1 wid=" << wid << " ver=-1";
        return std::make_shared<Message>(oss.str(), Message::Dir::Out);
    };

    // 1. First enqueue: nothing to dedup. droppedTileWireId stays 0.
    {
        TileWireId dropped = 999; // sentinel: should be cleared by enqueue
        const bool enqueued = queue.enqueue(makeTile(100), &dropped);
        LOK_ASSERT_EQUAL_STR(true, enqueued);
        LOK_ASSERT_EQUAL(static_cast<TileWireId>(0), dropped);
        LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.size());
    }

    // 2. Same position, newer wireId: dedup happens, dropped wid is reported.
    {
        TileWireId dropped = 0;
        const bool enqueued = queue.enqueue(makeTile(120), &dropped);
        LOK_ASSERT_EQUAL_STR(true, enqueued);
        LOK_ASSERT_EQUAL(static_cast<TileWireId>(100), dropped);
        // queue still has just the newest one
        LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.size());
    }

    // 3. Different position: no dedup, dropped stays 0.
    {
        TileWireId dropped = 999;
        const bool enqueued = queue.enqueue(makeTileAt(3840, 0, 130), &dropped);
        LOK_ASSERT_EQUAL_STR(true, enqueued);
        LOK_ASSERT_EQUAL(static_cast<TileWireId>(0), dropped);
        LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.size());
    }

    // Drain the queue, only the surviving wireIds (120 and 130) come out.
    std::shared_ptr<Message> item;
    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT_EQUAL(static_cast<TileWireId>(120),
                     TileDesc::parse(item->firstLine()).getWireId());

    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT_EQUAL(static_cast<TileWireId>(130),
                     TileDesc::parse(item->firstLine()).getWireId());

    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());

    // Reproduce the full scenario:
    // a sender should end up tracking only the wireIds still in the queue,
    // never the ones removed by dedup.
    SenderQueue<std::shared_ptr<Message>> q2;
    std::vector<TileWireId> tilesOnFly;

    auto sendTile = [&](TileWireId wid, int posX = 0)
    {
        TileWireId dropped = 0;
        if (q2.enqueue(makeTileAt(posX, 0, wid), &dropped))
        {
            if (dropped != 0)
            {
                auto it = std::find(tilesOnFly.begin(), tilesOnFly.end(), dropped);
                LOK_ASSERT(it != tilesOnFly.end()); // the dropped wid was tracked
                tilesOnFly.erase(it);
            }
            tilesOnFly.push_back(wid);
        }
    };

    // Simulate fast re-enqueues at the same position before the websocket
    // gets to drain the queue: each new wireId dedups the previous.
    sendTile(200);
    sendTile(201);
    sendTile(202);
    sendTile(203);

    // And one tile at a different position which is not dedup'd.
    sendTile(204, 3840);

    // The tracker mirrors what is actually in the queue:
    // { 203 (latest at posX=0), 204 (at posX=3840) }
    // Without the dropped-wireId reporting, tilesOnFly would still contain
    // 200, 201, 202
    LOK_ASSERT_EQUAL(static_cast<size_t>(2), tilesOnFly.size());
    LOK_ASSERT_EQUAL(static_cast<size_t>(2), q2.size());
    LOK_ASSERT(std::find(tilesOnFly.begin(), tilesOnFly.end(),
                         static_cast<TileWireId>(203)) != tilesOnFly.end());
    LOK_ASSERT(std::find(tilesOnFly.begin(), tilesOnFly.end(),
                         static_cast<TileWireId>(204)) != tilesOnFly.end());
    for (TileWireId leakedWid : { 200, 201, 202 })
    {
        LOK_ASSERT(std::find(tilesOnFly.begin(), tilesOnFly.end(),
                             static_cast<TileWireId>(leakedWid)) == tilesOnFly.end());
    }

    // The default-argument overload (no out-param) must keep working too.
    SenderQueue<std::shared_ptr<Message>> q3;
    LOK_ASSERT_EQUAL_STR(true, q3.enqueue(makeTile(300)));
    LOK_ASSERT_EQUAL_STR(true, q3.enqueue(makeTile(301)));
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), q3.size());
}

// Reproduces the kit-hang scenario for an in-flight tile render.
//
// When a client sends a tilecombine, the server calls
// requestTileRendering → subscribeToTileRendering, which inserts a
// TileBeingRendered into _tilesBeingRendered with a start timestamp.
// If the kit doesn't reply in time (hang), no path removes the entry.
// Once the entry's age exceeds COMMAND_TIMEOUT_MS, isStale() flips
// and hasTileBeingRendered returns false. The next call to
// requestTileRendering sees "no in-progress render" and re-issues to
// kit. Without a re-issue trigger, the entry sits there orphaned.
// So in case throttled client tilecombine (5s limit) we would NOT re-issue
// and leave this state with missing tiles on the client.
void KitQueueTests::testTileCacheKitHangBecomesStale()
{
    constexpr std::string_view testname = __func__;

    TileCache cache("dummy://test-doc.odt", std::chrono::system_clock::now(),
                    /*dontCache=*/true);

    const TileDesc tile = TileDesc::parse(
        "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0 "
        "tilewidth=3840 tileheight=3840");

    // Simulate the server beginning to render this tile (kit request sent).
    const auto t0 = std::chrono::steady_clock::now();
    cache.injectTileBeingRenderedForTest(tile, t0);

    // Existence-only check (no `now`): entry is in the map.
    LOK_ASSERT_EQUAL_STR(true, cache.hasTileBeingRendered(tile));

    // Just after the request: not stale. requestTileRendering would NOT
    // re-issue here; it would just subscribe to the existing render.
    {
        const auto fresh = t0 + std::chrono::milliseconds(100);
        LOK_ASSERT_EQUAL_STR(true, cache.hasTileBeingRendered(tile, &fresh));
    }

    // Just under the timeout: still considered in-progress.
    {
        const auto almost = t0 + std::chrono::milliseconds(COMMAND_TIMEOUT_MS - 1);
        LOK_ASSERT_EQUAL_STR(true, cache.hasTileBeingRendered(tile, &almost));
    }

    // One step past the timeout: kit has hung. Staleness check now
    // returns false. The entry has NOT been removed from the map - no
    // automatic cleanup ever fires - so a client that retriggers will
    // re-issue to kit; a client that doesn't (e.g. blocked by the
    // 5s per-tile client-side rate limit) leaves the request lost.
    {
        const auto stale = t0 + std::chrono::milliseconds(COMMAND_TIMEOUT_MS + 1);
        LOK_ASSERT_EQUAL_STR(false, cache.hasTileBeingRendered(tile, &stale));
        // Entry still present:
        LOK_ASSERT_EQUAL_STR(true, cache.hasTileBeingRendered(tile));
    }

    // Far past the timeout: still stale, still no cleanup.
    {
        const auto wayPast = t0 + std::chrono::seconds(60);
        LOK_ASSERT_EQUAL_STR(false, cache.hasTileBeingRendered(tile, &wayPast));
        LOK_ASSERT_EQUAL_STR(true, cache.hasTileBeingRendered(tile));
    }
}

// Verifies the periodic stale-render sweep:
//   - returns stale entries for re-issue when live subscribers remain,
//   - resets the start time so the next sweep does not re-flag them,
//   - drops stale entries whose subscribers have all gone.
void KitQueueTests::testTileCacheStaleRenderIsReissued()
{
    constexpr std::string_view testname = __func__;

    TileCache cache("dummy://test-doc.odt", std::chrono::system_clock::now(),
                    /*dontCache=*/true);

    const TileDesc tileA = TileDesc::parse(
        "tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0 "
        "tilewidth=3840 tileheight=3840");
    const TileDesc tileB = TileDesc::parse(
        "tile nviewid=0 part=0 width=256 height=256 tileposx=3840 tileposy=0 "
        "tilewidth=3840 tileheight=3840");

    int sessionA = 0;
    auto liveSubscriber = std::shared_ptr<ClientSession>(
        reinterpret_cast<ClientSession*>(&sessionA), [](ClientSession*) {});

    const auto t0 = std::chrono::steady_clock::now();
    cache.injectTileBeingRenderedForTest(tileA, t0, liveSubscriber);

    // tileB has a subscriber that goes away before the sweep sees it.
    {
        int sessionB = 0;
        auto goneSubscriber = std::shared_ptr<ClientSession>(
            reinterpret_cast<ClientSession*>(&sessionB), [](ClientSession*) {});
        cache.injectTileBeingRenderedForTest(tileB, t0, goneSubscriber);
        // goneSubscriber drops out of scope here; its weak_ptr in the cache
        // is now expired.
    }

    // Not stale yet: sweep should return nothing.
    {
        const auto fresh = t0 + std::chrono::milliseconds(100);
        auto reissue = cache.takeStaleRendersForReissue(fresh);
        LOK_ASSERT_EQUAL(static_cast<size_t>(0), reissue.size());
        // Both entries still tracked.
        LOK_ASSERT_EQUAL_STR(true, cache.hasTileBeingRendered(tileA));
        LOK_ASSERT_EQUAL_STR(true, cache.hasTileBeingRendered(tileB));
    }

    // After the timeout: sweep returns tileA (live subscriber), and
    // silently drops tileB (no live subscribers left).
    const auto stale = t0 + std::chrono::milliseconds(COMMAND_TIMEOUT_MS + 1);
    {
        auto reissue = cache.takeStaleRendersForReissue(stale);
        LOK_ASSERT_EQUAL(static_cast<size_t>(1), reissue.size());
        LOK_ASSERT_EQUAL(tileA.getTilePosX(), reissue[0].getTilePosX());
        LOK_ASSERT_EQUAL(tileA.getTilePosY(), reissue[0].getTilePosY());

        // tileA's start time was reset to `stale`. The entry is still
        // present (it is waiting for a new kit reply).
        LOK_ASSERT_EQUAL_STR(true, cache.hasTileBeingRendered(tileA, &stale));

        // tileB's entry was dropped because all subscribers are gone.
        LOK_ASSERT_EQUAL_STR(false, cache.hasTileBeingRendered(tileB));
    }

    // Immediately calling the sweep again must not re-flag tileA: the
    // start time was just reset to `stale`, so it is now fresh again.
    {
        auto reissue = cache.takeStaleRendersForReissue(stale);
        LOK_ASSERT_EQUAL(static_cast<size_t>(0), reissue.size());
        LOK_ASSERT_EQUAL_STR(true, cache.hasTileBeingRendered(tileA));
    }

    // Advance far past the new start: tileA goes stale again, sweep
    // returns it once more. This is what would happen if the kit keeps
    // hanging across multiple sweep intervals.
    {
        const auto staleAgain = stale + std::chrono::milliseconds(COMMAND_TIMEOUT_MS + 1);
        auto reissue = cache.takeStaleRendersForReissue(staleAgain);
        LOK_ASSERT_EQUAL(static_cast<size_t>(1), reissue.size());
    }
}

void KitQueueTests::testInvalidateViewCursorDeduplication()
{
    constexpr std::string_view testname = __func__;

    SenderQueue<std::shared_ptr<Message>> queue;

    std::shared_ptr<Message> item;

    // Empty queue
    LOK_ASSERT_EQUAL_STR(false, queue.dequeue(item));
    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());

    const std::vector<std::string> view_messages =
    {
        "invalidateviewcursor: {    \"viewId\": \"1\",     \"rectangle\": \"3999, 1418, 0, 298\",     \"part\": \"0\" }",
        "invalidateviewcursor: {    \"viewId\": \"2\",     \"rectangle\": \"3999, 1418, 0, 298\",     \"part\": \"0\" }",
        "invalidateviewcursor: {    \"viewId\": \"3\",     \"rectangle\": \"3999, 1418, 0, 298\",     \"part\": \"0\" }",
    };

    for (const auto& msg : view_messages)
    {
        queue.enqueue(std::make_shared<Message>(msg, Message::Dir::Out));
    }

    LOK_ASSERT_EQUAL(static_cast<size_t>(3), queue.size());

    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.size());
    LOK_ASSERT(item);
    LOK_ASSERT_EQUAL(view_messages[0], msgStr(item));

    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.size());
    LOK_ASSERT(item);
    LOK_ASSERT_EQUAL(view_messages[1], msgStr(item));

    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));
    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());
    LOK_ASSERT(item);
    LOK_ASSERT_EQUAL(view_messages[2], msgStr(item));

    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());

    const std::vector<std::string> dup_messages =
    {
        "invalidateviewcursor: {    \"viewId\": \"1\",     \"rectangle\": \"3999, 1418, 0, 298\",     \"part\": \"0\" }",
        "invalidateviewcursor: {    \"viewId\": \"1\",     \"rectangle\": \"1000, 1418, 0, 298\",     \"part\": \"0\" }",
        "invalidateviewcursor: {    \"viewId\": \"1\",     \"rectangle\": \"2000, 1418, 0, 298\",     \"part\": \"0\" }",
    };

    for (const auto& msg : dup_messages)
    {
        queue.enqueue(std::make_shared<Message>(msg, Message::Dir::Out));
    }

    LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.size());
    LOK_ASSERT_EQUAL_STR(true, queue.dequeue(item));

    // The last one should persist.
    LOK_ASSERT_EQUAL(dup_messages[2], std::string(item->data().data(), item->data().size()));

    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());
}

// back-compatible method from before putCallback implementation
void putCallback(KitQueue &queue, const std::string &str)
{
    StringVector tokens = StringVector::tokenize(str);
    assert(tokens[0] == "callback");
    int view = std::atoi(tokens[1].c_str());
    if (tokens[1] == "all")
        view = -1;
    int type = std::atoi(tokens[2].c_str());
    queue.putCallback(view, type, tokens.cat(' ', 3));
}

void KitQueueTests::testCallbackInvalidation()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);
    KitQueue::Callback item;

    // join tiles
    putCallback(queue, "callback all 0 284, 1418, 11105, 275, 0");
    putCallback(queue, "callback all 0 4299, 1418, 7090, 275, 0");

    LOK_ASSERT_EQUAL(1, static_cast<int>(queue.callbackSize()));

    item = queue.getCallback();
    LOK_ASSERT_EQUAL_STR("284, 1418, 11105, 275, 0", item._payload);

    // invalidate everything with EMPTY, but keep the different part intact
    putCallback(queue, "callback all 0 284, 1418, 11105, 275, 0");
    putCallback(queue, "callback all 0 4299, 1418, 7090, 275, 1");
    putCallback(queue, "callback all 0 4299, 10418, 7090, 275, 0");
    putCallback(queue, "callback all 0 4299, 20418, 7090, 275, 0");

    LOK_ASSERT_EQUAL(4, static_cast<int>(queue.callbackSize()));

    putCallback(queue, "callback all 0 EMPTY, 0");

    LOK_ASSERT_EQUAL(2, static_cast<int>(queue.callbackSize()));

    item = queue.getCallback();
    LOK_ASSERT_EQUAL_STR("4299, 1418, 7090, 275, 1", item._payload);
    item = queue.getCallback();
    LOK_ASSERT_EQUAL_STR("EMPTY, 0", item._payload);
}

void KitQueueTests::testCallbackIndicatorValue()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);
    KitQueue::Callback item;

    // join tiles
    putCallback(queue, "callback all 10 25");
    putCallback(queue, "callback all 10 50");

    LOK_ASSERT_EQUAL(1, static_cast<int>(queue.callbackSize()));
    item = queue.getCallback();
    LOK_ASSERT_EQUAL(item._view, -1);
    LOK_ASSERT_EQUAL(item._type, 10);
    LOK_ASSERT_EQUAL_STR("50", item._payload);
}

void KitQueueTests::testCallbackPageSize()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);
    KitQueue::Callback item;

    // join tiles
    putCallback(queue, "callback all 13 12474, 188626");
    putCallback(queue, "callback all 13 12474, 205748");

    LOK_ASSERT_EQUAL(1, static_cast<int>(queue.callbackSize()));
    item = queue.getCallback();
    LOK_ASSERT_EQUAL(item._view, -1);
    LOK_ASSERT_EQUAL(item._type, 13);
    LOK_ASSERT_EQUAL_STR("12474, 205748", item._payload);
}

void KitQueueTests::testCallbackModifiedStatusIsSkipped()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);
    KitQueue::Callback item;

    std::stringstream ss;
    ss << "callback all " << KIT_CALLBACK_STATE_CHANGED;

    const std::vector<std::string> messages =
    {
        ss.str() + " .uno:ModifiedStatus=false",
        ss.str() + " .uno:ModifiedStatus=true",
        ss.str() + " .uno:ModifiedStatus=true",
        ss.str() + " .uno:ModifiedStatus=false"
    };

    for (const auto& msg : messages)
    {
        putCallback(queue, msg);
    }

    LOK_ASSERT_EQUAL(static_cast<size_t>(4), queue.callbackSize());

    for (size_t i = 0; i < std::size(messages); i++)
    {
        item = queue.getCallback();
        LOK_ASSERT_EQUAL_STR(messages[i].substr(ss.str().size() + 1), item._payload);
    }
}

void KitQueueTests::testPutAndPop()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    LOK_ASSERT(queue.isEmpty());
    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());

    queue.put("session1 key type=input char=0 key=0");
    queue.put("session1 mouse type=buttondown x=1 y=2");
    queue.put("session1 uno .uno:Bold");

    LOK_ASSERT(!queue.isEmpty());
    LOK_ASSERT_EQUAL(static_cast<size_t>(3), queue.size());

    KitQueue::Payload p;

    p = queue.pop();
    LOK_ASSERT_EQUAL_STR("session1 key type=input char=0 key=0",
                         std::string_view(p.data(), p.size()));

    p = queue.pop();
    LOK_ASSERT_EQUAL_STR("session1 mouse type=buttondown x=1 y=2",
                         std::string_view(p.data(), p.size()));

    p = queue.pop();
    LOK_ASSERT_EQUAL_STR("session1 uno .uno:Bold", std::string_view(p.data(), p.size()));

    LOK_ASSERT(queue.isEmpty());
    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.size());
}

void KitQueueTests::testPopEmptyQueue()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    LOK_ASSERT(queue.isEmpty());

    KitQueue::Payload p = queue.pop();
    LOK_ASSERT(p.empty());
}

void KitQueueTests::testClear()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    queue.put("session1 key type=input char=0 key=0");
    queue.put("session1 mouse type=buttondown x=1 y=2");
    putCallback(queue, "callback all 10 25");
    putCallback(queue, "callback all 10 50");

    LOK_ASSERT(!queue.isEmpty());
    LOK_ASSERT(queue.callbackSize() > 0);

    queue.clear();

    LOK_ASSERT(queue.isEmpty());
    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.callbackSize());
}

void KitQueueTests::testPutEmptyPayload()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    bool threw = false;
    try
    {
        queue.put(KitQueue::Payload{});
    }
    catch (const std::runtime_error&)
    {
        threw = true;
    }
    LOK_ASSERT(threw);
}

void KitQueueTests::testTextInputBlockedByKeyMessage()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    queue.put("sess textinput id=0 text=a");
    queue.put("sess key type=input char=0 key=0");
    queue.put("sess textinput id=0 text=b");

    // The key message blocks combining; all three remain separate.
    LOK_ASSERT_EQUAL(static_cast<size_t>(3), queue.size());

    KitQueue::Payload p;
    p = queue.pop();
    LOK_ASSERT_EQUAL_STR("sess textinput id=0 text=a", std::string_view(p.data(), p.size()));
    p = queue.pop();
    LOK_ASSERT_EQUAL_STR("sess key type=input char=0 key=0", std::string_view(p.data(), p.size()));
    p = queue.pop();
    LOK_ASSERT_EQUAL_STR("sess textinput id=0 text=b", std::string_view(p.data(), p.size()));
}

void KitQueueTests::testTextInputBlockedByMouse()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    queue.put("sess textinput id=0 text=a");
    queue.put("sess mouse type=buttondown x=1 y=2");
    queue.put("sess textinput id=0 text=b");

    LOK_ASSERT_EQUAL(static_cast<size_t>(3), queue.size());
}

void KitQueueTests::testTextInputBlockedByRemoveText()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    queue.put("sess textinput id=0 text=a");
    queue.put("sess removetextcontext id=0 before=1 after=0");
    queue.put("sess textinput id=0 text=b");

    LOK_ASSERT_EQUAL(static_cast<size_t>(3), queue.size());
}

void KitQueueTests::testTextInputDifferentId()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    queue.put("sess textinput id=0 text=a");
    queue.put("sess textinput id=1 text=b");

    // Different ids should not combine.
    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.size());
}

void KitQueueTests::testTextInputMissingTokens()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // Missing text= token — queued as-is, no crash.
    queue.put("sess textinput id=0");
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.size());

    // Missing id= token — queued as-is.
    queue.put("sess textinput text=a");
    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.size());
}

void KitQueueTests::testRemoveTextBlockedByKeyMessage()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    queue.put("sess removetextcontext id=0 before=3 after=0");
    queue.put("sess key type=input char=0 key=0");
    queue.put("sess removetextcontext id=0 before=2 after=0");

    // The key message blocks combining.
    LOK_ASSERT_EQUAL(static_cast<size_t>(3), queue.size());

    KitQueue::Payload p;
    p = queue.pop();
    LOK_ASSERT_EQUAL_STR("sess removetextcontext id=0 before=3 after=0",
                         std::string_view(p.data(), p.size()));
    p = queue.pop();
    LOK_ASSERT_EQUAL_STR("sess key type=input char=0 key=0", std::string_view(p.data(), p.size()));
    p = queue.pop();
    LOK_ASSERT_EQUAL_STR("sess removetextcontext id=0 before=2 after=0",
                         std::string_view(p.data(), p.size()));
}

void KitQueueTests::testRemoveTextBlockedByTextInput()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    queue.put("sess removetextcontext id=0 before=3 after=0");
    queue.put("sess textinput id=0 text=a");
    queue.put("sess removetextcontext id=0 before=2 after=0");

    LOK_ASSERT_EQUAL(static_cast<size_t>(3), queue.size());
}

void KitQueueTests::testRemoveTextDifferentId()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    queue.put("sess removetextcontext id=0 before=3 after=0");
    queue.put("sess removetextcontext id=1 before=2 after=0");

    // Different ids should not combine.
    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.size());
}

void KitQueueTests::testCallbackStateChangedDedup()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    std::stringstream ss;
    ss << "callback all " << KIT_CALLBACK_STATE_CHANGED;

    putCallback(queue, ss.str() + " .uno:Bold=true");
    putCallback(queue, ss.str() + " .uno:Bold=false");

    // The older .uno:Bold=true should be removed; only the latest remains.
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.callbackSize());

    KitQueue::Callback item = queue.getCallback();
    LOK_ASSERT_EQUAL_STR(".uno:Bold=false", item._payload);
}

void KitQueueTests::testCallbackStateChangedDifferentCommands()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    std::stringstream ss;
    ss << "callback all " << KIT_CALLBACK_STATE_CHANGED;

    putCallback(queue, ss.str() + " .uno:Bold=true");
    putCallback(queue, ss.str() + " .uno:Italic=true");

    // Different commands should both remain.
    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.callbackSize());
}

void KitQueueTests::testCallbackStateChangedNoEquals()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    std::stringstream ss;
    ss << "callback all " << KIT_CALLBACK_STATE_CHANGED;

    putCallback(queue, ss.str() + " .uno:Bold=true");
    putCallback(queue, ss.str() + " .uno:Bold");

    // A payload without '=' should not elide the earlier one with '='.
    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.callbackSize());
}

void KitQueueTests::testCallbackInvalidationMergeSizeLimit()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // Two intersecting rectangles whose union exceeds 4*3840 x 2*3840.
    // Rect1: x=0 y=0 w=10000 h=5000 part=0
    // Rect2: x=8000 y=4000 w=10000 h=5000 part=0
    // Union would be 18000 x 9000, exceeding limits (15360 x 7680).
    putCallback(queue, "callback all 0 0, 0, 10000, 5000, 0");
    putCallback(queue, "callback all 0 8000, 4000, 10000, 5000, 0");

    // Should NOT merge because union is too large.
    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.callbackSize());
}

void KitQueueTests::testCallbackInvalidationNoIntersection()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // Two non-intersecting rectangles on the same part.
    putCallback(queue, "callback all 0 100, 100, 50, 50, 0");
    putCallback(queue, "callback all 0 1000, 1000, 50, 50, 0");

    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.callbackSize());
}

void KitQueueTests::testCallbackInvalidationDifferentModes()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // Same rectangle, same part, different modes — should not merge.
    putCallback(queue, "callback all 0 100, 100, 50, 50, 0, 0");
    putCallback(queue, "callback all 0 100, 100, 50, 50, 0, 1");

    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.callbackSize());
}

void KitQueueTests::testCallbackCursorDedup()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR = 1
    putCallback(queue, "callback all 1 old_cursor_pos");
    putCallback(queue, "callback all 1 new_cursor_pos");

    // Only the latest should remain.
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.callbackSize());

    KitQueue::Callback item = queue.getCallback();
    LOK_ASSERT_EQUAL_STR("new_cursor_pos", item._payload);
}

void KitQueueTests::testCallbackViewCursorDedup()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // KIT_CALLBACK_CELL_VIEW_CURSOR = 26; payload requires JSON with viewId.
    putCallback(queue, "callback all 26 { \"viewId\": \"1\", \"rectangle\": \"0, 0, 100, 100\" }");
    putCallback(queue, "callback all 26 { \"viewId\": \"1\", \"rectangle\": \"50, 50, 100, 100\" }");

    // Same viewId — the older one is replaced.
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.callbackSize());

    KitQueue::Callback item = queue.getCallback();
    LOK_ASSERT_EQUAL_STR("{ \"viewId\": \"1\", \"rectangle\": \"50, 50, 100, 100\" }", item._payload);

    // Now add a different viewId — both should persist.
    putCallback(queue, "callback all 26 { \"viewId\": \"1\", \"rectangle\": \"10, 10, 100, 100\" }");
    putCallback(queue, "callback all 26 { \"viewId\": \"2\", \"rectangle\": \"200, 200, 100, 100\" }");

    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.callbackSize());
}

void KitQueueTests::testPreviewTilesNoCombine()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // Preview tiles have id= parameter (id >= 0 means preview).
    queue.put("tile nviewid=0 part=0 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 id=0");
    queue.put("tile nviewid=0 part=1 width=180 height=135 tileposx=0 tileposy=0 tilewidth=15875 tileheight=11906 id=1");

    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.getTileQueueSize());

    // Each preview should be popped individually — NOT combined into a tilecombine.
    TilePrioritizer::Priority prio;
    TileCombined c1 = queue.popTileQueue(prio);
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), c1.getTiles().size());

    TileCombined c2 = queue.popTileQueue(prio);
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), c2.getTiles().size());

    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.getTileQueueSize());
}

void KitQueueTests::testTileDeduplicationOnPush()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // Push the same tile twice — sortedInsert replaces the duplicate.
    queue.put("tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840");
    queue.put("tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840");

    LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.getTileQueueSize());

    // Push a tilecombine containing the same tile — no growth.
    queue.put("tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840");

    LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.getTileQueueSize());
}

void KitQueueTests::testMultiViewTileQueues()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);

    // Push tiles for two different viewIds.
    queue.put("tile nviewid=0 part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840");
    queue.put("tile nviewid=1000 part=0 width=256 height=256 tileposx=0 tileposy=0 tilewidth=3840 tileheight=3840");

    // Total size is the sum of both queues.
    LOK_ASSERT_EQUAL(static_cast<size_t>(2), queue.getTileQueueSize());

    // Pop both tiles.
    TilePrioritizer::Priority prio;
    TileCombined c1 = queue.popTileQueue(prio);
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), c1.getTiles().size());
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), queue.getTileQueueSize());

    TileCombined c2 = queue.popTileQueue(prio);
    LOK_ASSERT_EQUAL(static_cast<size_t>(1), c2.getTiles().size());
    LOK_ASSERT_EQUAL(static_cast<size_t>(0), queue.getTileQueueSize());
}

void KitQueueTests::testGetCallbackBoolOverload()
{
    constexpr std::string_view testname = __func__;

    TilePrioritizer dummy;
    KitQueue queue(dummy);
    KitQueue::Callback cb;

    // Empty queue — returns false.
    LOK_ASSERT(!queue.getCallback(cb));

    // Queue a callback and retrieve it.
    putCallback(queue, "callback all 10 test_payload");

    LOK_ASSERT(queue.getCallback(cb));
    LOK_ASSERT_EQUAL(-1, cb._view);
    LOK_ASSERT_EQUAL(10, cb._type);
    LOK_ASSERT_EQUAL_STR("test_payload", cb._payload);

    // Queue is empty again.
    LOK_ASSERT(!queue.getCallback(cb));
}

void KitQueueTests::testCallbackInvalidationEmptyMode()
{
    // Given a mode=1 and a mode=2 partial invalidation in the queue:
    constexpr std::string_view testname = __func__;
    TilePrioritizer dummy;
    KitQueue queue(dummy);
    KitQueue::Callback item;
    putCallback(queue, "callback all 0 284, 1418, 11105, 275, 0, 1");
    putCallback(queue, "callback all 0 284, 1418, 11105, 275, 0, 2");
    LOK_ASSERT_EQUAL(2, static_cast<int>(queue.callbackSize()));

    // When putting mode=1 and mode=2 full invalidations in the queue:
    putCallback(queue, "callback all 0 EMPTY, 0, 1");
    putCallback(queue, "callback all 0 EMPTY, 0, 2");

    // Then make sure deduplication results in two full invalidations:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. the queue had "284, 1418, 11105, 275, 0, 1", "284, 1418, 11105, 275, 0, 2" and "EMPTY,
    // 0, 2", which means the EMPTY invalidate for mode=1 was lost.
    LOK_ASSERT_EQUAL(2, static_cast<int>(queue.callbackSize()));
    item = queue.getCallback();
    LOK_ASSERT_EQUAL_STR("EMPTY, 0, 1", item._payload);
    item = queue.getCallback();
    LOK_ASSERT_EQUAL_STR("EMPTY, 0, 2", item._payload);
}

CPPUNIT_TEST_SUITE_REGISTRATION(KitQueueTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
