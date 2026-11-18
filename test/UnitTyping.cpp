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

// Runs client tests in their own thread inside a WSD process.

#include <config.h>

#include <COOLWSD.hpp>
#include <common/HexUtil.hpp>
#include <Exceptions.hpp>
#include <common/Log.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <WebSocketSession.hpp>
#include <helpers.hpp>
#include <wsd/TileDesc.hpp>
#include <kit/KitQueue.hpp>

#include <random>
#include <iostream>

using namespace std::literals;
using namespace ::helpers;

// Inside the WSD process
class UnitTyping : public UnitWSD
{
    bool _workerStarted;
    std::thread _worker;

public:
    UnitTyping()
        : UnitWSD("UnitTyping")
        , _workerStarted(false)
    {
        constexpr std::chrono::minutes timeout_minutes(5);
        setTimeout(timeout_minutes);
    }

    ~UnitTyping()
    {
        LOG_INF("Joining test worker thread\n");
        _worker.join();
    }

    bool filterAlertAllusers(const std::string & msg) override
    {
        std::cout << "Alert: " << msg << '\n';
        return false;
    }


    TestResult testWriterTyping()
    {
        Poco::URI uri(helpers::getTestServerURI());

        LOG_TRC("test writer typing");

        // Load a doc with the cursor saved at a top row.
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL(
            "empty.odt", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("TypingPoll");
        socketPoll->startThread();

        std::shared_ptr<http::WebSocketSession> socket =
            helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);

        // We input two Bopomofo (Mandarin Phonetic Symbols) characters and a grave accent using
        // textinput messages and then delete them and then input a fourth character. Apparently
        // this is supposed to mimic what happens when input is coming from an IME for Traditional
        // Chinese? Unclear whether the 'key' messages here really match what the JS generates in
        // such a case.
        static const char *commands[] = {
            "key type=up char=0 key=17",
            // BOPOMOFO LETTER C
            "textinput id=0 text=%E3%84%98",
            "key type=up char=0 key=519",

            // BOPOMOFO LETTER E
            "textinput id=0 text=%E3%84%9C",
            "key type=up char=0 key=522",

            // MODIFIER LETTER GRAVE ACCENT
            // Huh?
            "textinput id=0 text=%CB%8B",
            "key type=up char=0 key=260",

            // replace with the complete character
            "removetextcontext id=0 before=3 after=0",
            "textinput id=0 text=%E6%B8%AC",
            "key type=up char=0 key=259"
        };
        // CJK UNIFIED IDEOGRAPH-6E2C
        static const unsigned char correct[] = {
            0xe6, 0xb8, 0xac
        };

        // Feed the keystrokes ...
        for (const char *str : commands)
            sendTextFrame(socket, str, testname);

        // extract their text
        sendTextFrame(socket, "uno .uno:SelectAll", testname);
        sendTextFrame(socket, "gettextselection mimetype=text/plain;charset=utf-8", testname);

        LOG_TRC("Waiting for test selection:");
        const char response[] = "textselectioncontent:";
        const int responseLen = sizeof(response) - 1;
        const std::string result = getResponseString(socket, response, testname, 5s);

        // The result string should contain "textselectioncontent:\n" followed by the UTF-8 bytes
        LOG_TRC("length " << result.length() << " vs. " << (responseLen + 1 + sizeof(correct)));
        if (strncmp(result.c_str(), response, responseLen) ||
            result.length() != responseLen + 1 + sizeof(correct) ||
            memcmp(result.c_str() + responseLen + 1, (const char *)correct, sizeof(correct)))
        {
            LOK_ASSERT_FAIL("Error: wrong textselectioncontent:\n" << HexUtil::dumpHex(result));
            return TestResult::Failed;
        }

        return TestResult::Ok;
    }

    TestResult testKitQueueMerging()
    {
        TilePrioritizer dummy;
        KitQueue queue(dummy);

        queue.put("child-foo textinput id=0 text=a");
        queue.put("child-foo textinput id=0 text=b");

        auto v = queue.get();

        if (!queue.isEmpty())
        {
            LOG_ERR("Merge of textinput messages did not happen");
            return TestResult::Failed;
        }

        std::string s;
        s = std::string(v.data(), v.size());

        if (s != "child-foo textinput id=0 text=ab")
        {
            LOG_ERR("Merge of textinput messages produced unexpected result '" << s << "'");
            return TestResult::Failed;
        }

        queue.put("child-foo textinput id=0 text=a");
        queue.put("child-bar textinput id=0 text=b");
        queue.put("child-foo textinput id=0 text=c");

        v = queue.get();
        if (queue.isEmpty())
        {
            LOG_ERR("Merge of textinput messages for different clients that should not have happened");
            return TestResult::Failed;
        }

        // Verify that the earlier textinput was removed and its contents merged with the later one
        s = std::string(v.data(), v.size());
        if (s != "child-bar textinput id=0 text=b")
        {
            LOG_ERR("Merge of textinput messages done incorrectly");
            return TestResult::Failed;
        }

        v = queue.get();
        if (!queue.isEmpty())
        {
            LOG_ERR("Merge of textinput messages did not happen");
            return TestResult::Failed;
        }

        s = std::string(v.data(), v.size());
        if (s != "child-foo textinput id=0 text=ac")
        {
            LOG_ERR("Merge of textinput messages done incorrectly");
            return TestResult::Failed;
        }

        queue.put("child-foo textinput id=0 text=a");
        queue.put("child-foo key type=input char=97 key=0");
        queue.put("child-foo textinput id=0 text=b");

        v = queue.get();
        v = queue.get();
        if (queue.isEmpty())
        {
            LOG_ERR("Merge of textinput messages with a key message inbetween that should not have happened");
            return TestResult::Failed;
        }
        v = queue.get();
        if (!queue.isEmpty())
        {
            LOG_ERR("KitQueue contains more than was put into it");
            return TestResult::Failed;
        }

        queue.put("child-foo textinput id=0 text=abcdef");
        queue.put("child-foo removetextcontext id=0 before=1 after=0");
        queue.put("child-foo removetextcontext id=0 before=1 after=0");

        v = queue.get();
        v = queue.get();

        if (!queue.isEmpty())
        {
            LOG_ERR("Merge of removetextcontext messages did not happen");
            return TestResult::Failed;
        }

        s = std::string(v.data(), v.size());

        if (s != "child-foo removetextcontext id=0 before=2 after=0")
        {
            LOG_ERR("Merge of removetextcontext messages produced unexpected result '" << s << "'");
            return TestResult::Failed;
        }

        return TestResult::Ok;
    }

    TestResult testCalcTyping()
    {
        Poco::URI uri(helpers::getTestServerURI());

        // Load a doc with the cursor saved at a top row.
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL("empty.ods", documentPath, documentURL, testname);

        std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("TypingPoll");
        socketPoll->startThread();

        const int numRender = 2;
        const int numTyping = 6;
        const int numSocket = numRender + numTyping;
        std::vector<std::shared_ptr<http::WebSocketSession>> sockets;

        LOG_TRC("Connecting first client to " << uri.toString() << " doc: " << documentURL);
        sockets.emplace_back(helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname));

        for (int i = 1; i < numSocket; ++i)
        {
            LOG_TRC("Connecting client " << i);
            std::shared_ptr<http::WebSocketSession> socket =
                helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname);
            sockets.emplace_back(
                helpers::loadDocAndGetSession(socketPoll, uri, documentURL, testname));
            for (int j = 0; j < i * 3; ++j)
            {
                // cursor down some multiple of times
                sendTextFrame(socket, "key type=input char=0 key=1024", testname);
                sendTextFrame(socket, "key type=up char=0 key=1024", testname);
                assertResponseString(socket, "celladdress:", testname);
            }
        }

        int count = 100 * numTyping;
        std::vector<std::string> messages[numTyping];

        // setup.
        for (int i = 0; i < numTyping; ++i)
        {
            messages[i].push_back("clientvisiblearea x=0 y=0 width=27960 height=5160");
            messages[i].push_back("clientzoom tilepixelwidth=256 tilepixelheight=256 tiletwipwidth=3840 tiletwipheight=3840");
            messages[i].push_back("commandvalues command=.uno:ViewRowColumnHeaders?x=-15&y=3870&width=0&height=5160");
            messages[i].push_back("useractive");
        }

        // randomly queue 'a', 'b' etc. key-press / space to each socket.
        std::mt19937 randMt(0);
        for (int i = 0; i < count; ++i)
        {
            int which = i % numTyping;
            std::vector<std::string> &msgs = messages[which];
            int chr = 97 + which;
            int key = 512 + which * 2;

            bool space = !(randMt() & 0300); // send a space

            msgs.push_back("key type=input char=" + std::to_string(chr) + " key=0");
            msgs.push_back("key type=up char=0 key=" + std::to_string(key));
            if (space)
            {
                msgs.push_back("key type=input char=32 key=0");
                msgs.push_back("key type=up char=0 key=1284");
            }
        }

        constexpr std::chrono::milliseconds waitMS{ 5 };
        std::vector<std::thread> threads;
        std::atomic<bool> started(false);
        std::atomic<int> liveTyping(0);

        // First some rendering load
        threads.reserve(numRender);
        for (int i = 0; i < numRender; ++i)
            threads.emplace_back([&,i] {
                    std::mt19937 randDev(numRender * 257);
                    std::shared_ptr<http::WebSocketSession> sock = sockets[numTyping + i];
                    while (!started || liveTyping > 0)
                    {
                        std::ostringstream oss;
                        std::uniform_int_distribution<int> distribution(0,32);
                        oss << "tilecombine nviewid=0 part=0 width=256 height=256"
                            << " tileposx=" << 3840*distribution(randDev)
                            << " tileposy=" << 3840*distribution(randDev)
                            << " tilewidth=7680 tileheight=7680";
                        sendTextFrame(sock, oss.str(), testname);

                        const std::vector<char> tile =
                            getResponseMessage(sock, "tile:", testname, 5ms);

                        std::this_thread::sleep_for(25ms);
                    }
                });

        // Add some typing
        for (int which = 0; which < numTyping; ++which)
        {
            threads.emplace_back([&,which] {
                    std::mt19937 randDev(which * 16);
                    std::shared_ptr<http::WebSocketSession> sock = sockets[which];
                    liveTyping++;
                    started = true;
                    for (size_t i = 0; i < messages[which].size(); ++i)
                    {
                        std::string msg = messages[which][i];

                        std::uint_fast32_t num = randDev();
                        if (!(num & 0x30))
                            sendTextFrame(sock, "ping", testname);

                        // suck and dump replies down
                        std::vector<char> tile = getResponseMessage(sock, "tile:", testname, waitMS);
                        if (tile.size())
                        {
// 1544818858022 INCOMING: tile: nviewid=0 part=0 width=256 height=256 tileposx=15360 tileposy=38400 tilewidth=3840 tileheight=3840 oldwid=0 wid=232 ver=913 imgsize=1002
// Socket.js:123 1544818858027 OUTGOING: tileprocessed tile=0:15360:38400:3840:3840
                            TileDesc desc = TileDesc::parse(StringVector::tokenize(tile.data(), tile.size()));
                            sendTextFrame(sock,
                                          "tileprocessed wids=" + std::to_string(desc.getWireId()),
                                          testname);
                        }

                        if (!(num & 0x300)) // occasionally sleep some more - why not.
                            std::this_thread::sleep_for(std::chrono::milliseconds(waitMS*25));
                        LOG_TRC("Send to " << which << " message " << msg);
                        // std::cout << "Send to " << which << " message " << msg << '\n';
                        sendTextFrame(sock, msg, testname);
                    }
                    liveTyping--;
                });
        }

        for (auto& thread : threads)
        {
            thread.join();
        }

        // complete the cells with some 'enters'
        std::string results[numTyping];
        for (int i = 0; i < numTyping; ++i)
        {
            sendTextFrame(sockets[i], "key type=input char=13 key=0", testname);
            sendTextFrame(sockets[i], "key type=up char=0 key=1280", testname);

            // extract their text
            sendTextFrame(sockets[i], "uno .uno:SelectAll", testname);
            sendTextFrame(sockets[i], "gettextselection mimetype=text/plain;charset=utf-8", testname);

            LOG_TRC("Waiting for test selection:");
            const std::string result =
                getResponseString(sockets[i], "textselectioncontent:", testname, 20s);
            results[i] = result;

            char target = 'a'+i;
            LOG_TRC("Result [" << i << "] target " << target << " is '" << result << '\'');
            for (size_t j = sizeof("textselectioncontent:"); j < result.size(); ++j)
            {
                if (result[j] != ' ' && result[j] != target)
                {
                    LOG_TRC("Text contains incorrect char[" << j << "] = '" << result[j] << "' not " << target << " '" << result << '\'');
                    if (result[j] != target)
                        return TestResult::Failed;
                }
            }
        }

        return TestResult::Ok;
    }

    TestResult testTyping()
    {
        TestResult res;
        res = testWriterTyping();
        if (res != TestResult::Ok)
            return res;

        res = testKitQueueMerging();
        if (res != TestResult::Ok)
            return res;

//        res = testCalcTyping();
        return res;
    }

    void invokeWSDTest() override
    {
        // this method gets called every few seconds.
        if (_workerStarted)
            return;
        _workerStarted = true;

        _worker = std::thread([this]{
                exitTest (testTyping());
            });
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitTyping();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
