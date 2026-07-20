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
 * White box unit tests for various internal components.
 */

#include <config.h>

#include <common/Anonymizer.hpp>
#include <common/Common.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Message.hpp>
#include <common/Protocol.hpp>
#include <common/RegexUtil.hpp>
#include <common/StateEnum.hpp>
#include <common/ThreadPool.hpp>
#include <common/Util.hpp>
#include <wsd/TileCache.hpp>
#include <wsd/TileDesc.hpp>

#include <test/lokassert.hpp>
#include <test/testlog.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace std::literals;

/// WhiteBox unit-tests.
class WhiteBoxTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(WhiteBoxTests);
    CPPUNIT_TEST(testCOOLProtocolFunctions);
    CPPUNIT_TEST(testSplitting);
    CPPUNIT_TEST(testMessage);
    CPPUNIT_TEST(testPathPrefixTrimming);
    CPPUNIT_TEST(testMessageAbbreviation);
    CPPUNIT_TEST(testReplace);
    CPPUNIT_TEST(testReplaceChar);
    CPPUNIT_TEST(testReplaceCharInPlace);
    CPPUNIT_TEST(testReplaceAllOf);
    CPPUNIT_TEST(testRegexListMatcher);
    CPPUNIT_TEST(testRegexListMatcher_Init);
    CPPUNIT_TEST(testTileDesc);
    CPPUNIT_TEST(testTileData);
    CPPUNIT_TEST(testRectanglesIntersect);
    CPPUNIT_TEST(testJson);
    CPPUNIT_TEST(testAnonymization);
    CPPUNIT_TEST(testStat);
    CPPUNIT_TEST(testStringCompare);
    CPPUNIT_TEST(testJsonUtilEscapeJSONValue);
    CPPUNIT_TEST(testStateEnum);
    CPPUNIT_TEST(testFindInVector);
    CPPUNIT_TEST(testJoinPair);
    CPPUNIT_TEST(testThreadPool);
    CPPUNIT_TEST(testLogCaptureCaller);
    CPPUNIT_TEST(testIsIso8601);
    CPPUNIT_TEST_SUITE_END();

    void testCOOLProtocolFunctions();
    void testSplitting();
    void testMessage();
    void testPathPrefixTrimming();
    void testMessageAbbreviation();
    void testReplace();
    void testReplaceChar();
    void testReplaceCharInPlace();
    void testReplaceAllOf();
    void testRegexListMatcher();
    void testRegexListMatcher_Init();
    void testTileDesc();
    void testTileData();
    void testRectanglesIntersect();
    void testJson();
    void testAnonymization();
    void testStat();
    void testStringCompare();
    void testJsonUtilEscapeJSONValue();
    void testStateEnum();
    void testFindInVector();
    void testJoinPair();
    void testThreadPool();
    void testLogCaptureCaller();
    void testIsIso8601();

    size_t waitForThreads(size_t count);
};

void WhiteBoxTests::testCOOLProtocolFunctions()
{
    constexpr std::string_view testname = __func__;

    int foo;
    LOK_ASSERT(COOLProtocol::getTokenInteger("foo=42", "foo", foo));
    LOK_ASSERT_EQUAL(42, foo);

    std::string bar;
    LOK_ASSERT(COOLProtocol::getTokenString("bar=hello-sailor", "bar", bar));
    LOK_ASSERT_EQUAL_STR("hello-sailor", bar);

    LOK_ASSERT(COOLProtocol::getTokenString("bar=", "bar", bar));
    LOK_ASSERT_EQUAL_STR("", bar);

    int mumble;
    std::map<std::string, int> map { { "hello", 1 }, { "goodbye", 2 }, { "adieu", 3 } };

    LOK_ASSERT(COOLProtocol::getTokenKeyword("mumble=goodbye", "mumble", map, mumble));
    LOK_ASSERT_EQUAL(2, mumble);

    std::string message("hello x=1 y=2 foo=42 bar=hello-sailor mumble='goodbye' zip zap");
    StringVector tokens(StringVector::tokenize(message));

    LOK_ASSERT(COOLProtocol::getTokenInteger(tokens, "foo", foo));
    LOK_ASSERT_EQUAL(42, foo);

    LOK_ASSERT(COOLProtocol::getTokenString(tokens, "bar", bar));
    LOK_ASSERT_EQUAL_STR("hello-sailor", bar);

    LOK_ASSERT(COOLProtocol::getTokenKeyword(tokens, "mumble", map, mumble));
    LOK_ASSERT_EQUAL(2, mumble);

    LOK_ASSERT(COOLProtocol::getTokenIntegerFromMessage(message, "foo", foo));
    LOK_ASSERT_EQUAL(42, foo);

    LOK_ASSERT(COOLProtocol::getTokenStringFromMessage(message, "bar", bar));
    LOK_ASSERT_EQUAL_STR("hello-sailor", bar);

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trimmed("A").size());
    LOK_ASSERT_EQUAL_STR("A", Util::trimmed("A"));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trimmed(" X").size());
    LOK_ASSERT_EQUAL_STR("X", Util::trimmed(" X"));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trimmed("Y ").size());
    LOK_ASSERT_EQUAL_STR("Y", Util::trimmed("Y "));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trimmed(" Z ").size());
    LOK_ASSERT_EQUAL_STR("Z", Util::trimmed(" Z "));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), Util::trimmed(" ").size());
    LOK_ASSERT_EQUAL_STR("", Util::trimmed(" "));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), Util::trimmed("   ").size());
    LOK_ASSERT_EQUAL_STR("", Util::trimmed("   "));

    std::string s;

    s = "A";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trim(s).size());
    s = "A";
    LOK_ASSERT_EQUAL_STR("A", Util::trim(s));

    s = " X";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trim(s).size());
    s = " X";
    LOK_ASSERT_EQUAL_STR("X", Util::trim(s));

    s = "Y ";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trim(s).size());
    s = "Y ";
    LOK_ASSERT_EQUAL_STR("Y", Util::trim(s));

    s = " Z ";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), Util::trim(s).size());
    s = " Z ";
    LOK_ASSERT_EQUAL_STR("Z", Util::trim(s));

    s = " ";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), Util::trim(s).size());
    s = " ";
    LOK_ASSERT_EQUAL_STR("", Util::trim(s));

    s = "   ";
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), Util::trim(s).size());
    s = "   ";
    LOK_ASSERT_EQUAL_STR("", Util::trim(s));

    // Integer lists.
    std::vector<int> ints;

    ints = COOLProtocol::tokenizeInts(std::string("-1"));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), ints.size());
    LOK_ASSERT_EQUAL(-1, ints[0]);

    ints = COOLProtocol::tokenizeInts(std::string("1,2,3,4"));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(4), ints.size());
    LOK_ASSERT_EQUAL(1, ints[0]);
    LOK_ASSERT_EQUAL(2, ints[1]);
    LOK_ASSERT_EQUAL(3, ints[2]);
    LOK_ASSERT_EQUAL(4, ints[3]);

    ints = COOLProtocol::tokenizeInts("");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), ints.size());

    ints = COOLProtocol::tokenizeInts(std::string(",,,"));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), ints.size());
}

void WhiteBoxTests::testSplitting()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring(nullptr, 5, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring(nullptr, -1, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring("abc", 0, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring("abc", -1, '\n'));
    LOK_ASSERT_EQUAL_STR("ab", Util::getDelimitedInitialSubstring("abc", 2, '\n'));

    std::string first;
    std::string second;

    std::tie(first, second) = Util::split(std::string(""), '.', true);
    std::tie(first, second) = Util::split(std::string(""), '.', false);

    std::tie(first, second) = Util::splitLast(std::string(""), '.', true);
    std::tie(first, second) = Util::splitLast(std::string(""), '.', false);

    // Split first, remove delim.
    std::tie(first, second) = Util::split(std::string("a"), '.', true);
    LOK_ASSERT_EQUAL_STR("a", first);
    LOK_ASSERT_EQUAL_STR("", second);

    // Split first, keep delim.
    std::tie(first, second) = Util::split(std::string("a"), '.', false);
    LOK_ASSERT_EQUAL_STR("a", first);
    LOK_ASSERT_EQUAL_STR("", second);

    // Split first, remove delim.
    std::tie(first, second) = Util::splitLast(std::string("a"), '.', true);
    LOK_ASSERT_EQUAL_STR("a", first);
    LOK_ASSERT_EQUAL_STR("", second);

    // Split first, keep delim.
    std::tie(first, second) = Util::splitLast(std::string("a"), '.', false);
    LOK_ASSERT_EQUAL_STR("a", first);
    LOK_ASSERT_EQUAL_STR("", second);

    // Split first, remove delim.
    std::tie(first, second) = Util::split(std::string("a."), '.', true);
    LOK_ASSERT_EQUAL_STR("a", first);
    LOK_ASSERT_EQUAL_STR("", second);

    // Split first, keep delim.
    std::tie(first, second) = Util::split(std::string("a."), '.', false);
    LOK_ASSERT_EQUAL_STR("a", first);
    LOK_ASSERT_EQUAL_STR(".", second);

    // Split first, remove delim.
    std::tie(first, second) = Util::splitLast(std::string("a."), '.', true);
    LOK_ASSERT_EQUAL_STR("a", first);
    LOK_ASSERT_EQUAL_STR("", second);

    // Split first, keep delim.
    std::tie(first, second) = Util::splitLast(std::string("a."), '.', false);
    LOK_ASSERT_EQUAL_STR("a", first);
    LOK_ASSERT_EQUAL_STR(".", second);

    // Split first, remove delim.
    std::tie(first, second) = Util::split(std::string("aa.bb"), '.', true);
    LOK_ASSERT_EQUAL_STR("aa", first);
    LOK_ASSERT_EQUAL_STR("bb", second);

    // Split first, keep delim.
    std::tie(first, second) = Util::split(std::string("aa.bb"), '.', false);
    LOK_ASSERT_EQUAL_STR("aa", first);
    LOK_ASSERT_EQUAL_STR(".bb", second);

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(5), Util::getLastDelimiterPosition("aa.bb.cc", 8, '.'));

    // Split last, remove delim.
    std::tie(first, second) = Util::splitLast(std::string("aa.bb.cc"), '.', true);
    LOK_ASSERT_EQUAL_STR("aa.bb", first);
    LOK_ASSERT_EQUAL_STR("cc", second);

    // Split last, keep delim.
    std::tie(first, second) = Util::splitLast(std::string("aa.bb.cc"), '.', false);
    LOK_ASSERT_EQUAL_STR("aa.bb", first);
    LOK_ASSERT_EQUAL_STR(".cc", second);

    // Split last, remove delim.
    std::tie(first, second) = Util::splitLast(std::string("/owncloud/index.php/apps/richdocuments/wopi/files/13_ocgdpzbkm39u"), '/', true);
    LOK_ASSERT_EQUAL_STR("/owncloud/index.php/apps/richdocuments/wopi/files", first);
    LOK_ASSERT_EQUAL_STR("13_ocgdpzbkm39u", second);

    // Split last, keep delim.
    std::tie(first, second) = Util::splitLast(std::string("/owncloud/index.php/apps/richdocuments/wopi/files/13_ocgdpzbkm39u"), '/', false);
    LOK_ASSERT_EQUAL_STR("/owncloud/index.php/apps/richdocuments/wopi/files", first);
    LOK_ASSERT_EQUAL_STR("/13_ocgdpzbkm39u", second);

    std::string third;
    std::string fourth;

    std::tie(first, second, third, fourth) = Util::splitUrl("filename");
    LOK_ASSERT_EQUAL_STR("", first);
    LOK_ASSERT_EQUAL_STR("filename", second);
    LOK_ASSERT_EQUAL_STR("", third);
    LOK_ASSERT_EQUAL_STR("", fourth);

    std::tie(first, second, third, fourth) = Util::splitUrl("filename.ext");
    LOK_ASSERT_EQUAL_STR("", first);
    LOK_ASSERT_EQUAL_STR("filename", second);
    LOK_ASSERT_EQUAL_STR(".ext", third);
    LOK_ASSERT_EQUAL_STR("", fourth);

    std::tie(first, second, third, fourth) = Util::splitUrl("/path/to/filename");
    LOK_ASSERT_EQUAL_STR("/path/to/", first);
    LOK_ASSERT_EQUAL_STR("filename", second);
    LOK_ASSERT_EQUAL_STR("", third);
    LOK_ASSERT_EQUAL_STR("", fourth);

    std::tie(first, second, third, fourth) = Util::splitUrl("http://domain.com/path/filename");
    LOK_ASSERT_EQUAL_STR("http://domain.com/path/", first);
    LOK_ASSERT_EQUAL_STR("filename", second);
    LOK_ASSERT_EQUAL_STR("", third);
    LOK_ASSERT_EQUAL_STR("", fourth);

    std::tie(first, second, third, fourth) = Util::splitUrl("http://domain.com/path/filename.ext");
    LOK_ASSERT_EQUAL_STR("http://domain.com/path/", first);
    LOK_ASSERT_EQUAL_STR("filename", second);
    LOK_ASSERT_EQUAL_STR(".ext", third);
    LOK_ASSERT_EQUAL_STR("", fourth);

    std::tie(first, second, third, fourth) = Util::splitUrl("http://domain.com/path/filename.ext?params=3&command=5");
    LOK_ASSERT_EQUAL_STR("http://domain.com/path/", first);
    LOK_ASSERT_EQUAL_STR("filename", second);
    LOK_ASSERT_EQUAL_STR(".ext", third);
    LOK_ASSERT_EQUAL_STR("?params=3&command=5", fourth);
}

void WhiteBoxTests::testMessage()
{
    // try to force an isolated page alloc, likely to have
    // an invalid, electrified fence page after it.
    size_t sz = 4096*128;
    char *big = static_cast<char *>(malloc(sz));
    const char msg[] = "bogus-forward";
    char *dest = big + sz - (sizeof(msg) - 1);
    memcpy(dest, msg, sizeof (msg) - 1);
    Message overrun(dest, sizeof (msg) - 1, Message::Dir::Out);
    free(big);
}

void WhiteBoxTests::testPathPrefixTrimming()
{
    constexpr std::string_view testname = __func__;

    // These helpers are used by the logging macros.
    // See Log.hpp for details.

#ifdef IOS

    LOK_ASSERT_EQUAL(23UL, skipPathToFilename("./path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(21UL, skipPathToFilename("path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(22UL, skipPathToFilename("/path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(24UL, skipPathToFilename("../path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(0UL, skipPathToFilename(""));
    LOK_ASSERT_EQUAL(0UL, skipPathToFilename("/"));
    LOK_ASSERT_EQUAL(0UL, skipPathToFilename("."));

    LOK_ASSERT_EQUAL_STR("filename.cpp",
                         std::string(LOG_FILE_NAME("./path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL_STR("filename.cpp",
                         std::string(LOG_FILE_NAME("path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL_STR("filename.cpp",
                         std::string(LOG_FILE_NAME("/path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME("")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME("/")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME(".")));

#else

    LOK_ASSERT_EQUAL(2UL, skipPathPrefix("./path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(0UL, skipPathPrefix("path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(1UL, skipPathPrefix("/path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(3UL, skipPathPrefix("../path/to/a/looooooong/filename.cpp"));
    LOK_ASSERT_EQUAL(0UL, skipPathPrefix(""));
    LOK_ASSERT_EQUAL(1UL, skipPathPrefix("/"));
    LOK_ASSERT_EQUAL(1UL, skipPathPrefix("."));

    LOK_ASSERT_EQUAL_STR("path/to/a/looooooong/filename.cpp",
                         std::string(LOG_FILE_NAME("./path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL_STR("path/to/a/looooooong/filename.cpp",
                         std::string(LOG_FILE_NAME("path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL_STR("path/to/a/looooooong/filename.cpp",
                         std::string(LOG_FILE_NAME("/path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL_STR("path/to/a/looooooong/filename.cpp",
                         std::string(LOG_FILE_NAME("../path/to/a/looooooong/filename.cpp")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME("")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME("/")));
    LOK_ASSERT_EQUAL(std::string(), std::string(LOG_FILE_NAME(".")));

#endif
}

void WhiteBoxTests::testMessageAbbreviation()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring(nullptr, 5, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring(nullptr, -1, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring("abc", 0, '\n'));
    LOK_ASSERT_EQUAL(std::string(), Util::getDelimitedInitialSubstring("abc", -1, '\n'));
    LOK_ASSERT_EQUAL_STR("ab", Util::getDelimitedInitialSubstring("abc", 2, '\n'));

    // The end arg of getAbbreviatedMessage is the length of the first argument, not
    // the point at which it should be abbreviated. Abbreviation appends ... to the
    // result
    LOK_ASSERT_EQUAL(std::string(), COOLProtocol::getAbbreviatedMessage(nullptr, 5));
    LOK_ASSERT_EQUAL(std::string(), COOLProtocol::getAbbreviatedMessage(nullptr, -1));
    LOK_ASSERT_EQUAL(std::string(), COOLProtocol::getAbbreviatedMessage("abc", 0));
    LOK_ASSERT_EQUAL(std::string(), COOLProtocol::getAbbreviatedMessage("abc", -1));
    LOK_ASSERT_EQUAL_STR("ab", COOLProtocol::getAbbreviatedMessage("abc", 2));

    std::string s;
    std::string abbr;

    s = "abcdefg";
    LOK_ASSERT_EQUAL(s, COOLProtocol::getAbbreviatedMessage(s));

    s = "1234567890123\n45678901234567890123456789012345678901234567890123";
    abbr = "1234567890123...";
    LOK_ASSERT_EQUAL(abbr, COOLProtocol::getAbbreviatedMessage(s.data(), s.size()));
    LOK_ASSERT_EQUAL(abbr, COOLProtocol::getAbbreviatedMessage(s));

    std::string long_utf8_str_a(COOLProtocol::maxNonAbbreviatedMsgLen - 3, 'a');
    LOK_ASSERT_EQUAL(long_utf8_str_a + std::string("mü..."),
                     COOLProtocol::getAbbreviatedMessage(long_utf8_str_a + "müsli"));

    // don't allow the ü sequence to be broken
    std::string long_utf8_str_b(COOLProtocol::maxNonAbbreviatedMsgLen - 2, 'a');
    LOK_ASSERT_EQUAL(long_utf8_str_b + std::string("mü..."),
                     COOLProtocol::getAbbreviatedMessage(long_utf8_str_b + "müsli"));
}

void WhiteBoxTests::testReplace()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR("zesz one zwo flee", Util::replace("test one two flee", "t", "z"));
    LOK_ASSERT_EQUAL_STR("testt one two flee", Util::replace("test one two flee", "tes", "test"));
    LOK_ASSERT_EQUAL_STR("testest one two flee",
                         Util::replace("test one two flee", "tes", "testes"));
    LOK_ASSERT_EQUAL_STR("tete one two flee", Util::replace("tettet one two flee", "tet", "te"));
    LOK_ASSERT_EQUAL_STR("t one two flee", Util::replace("test one two flee", "tes", ""));
    LOK_ASSERT_EQUAL_STR("test one two flee", Util::replace("test one two flee", "", "X"));
}

void WhiteBoxTests::testReplaceChar()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR("zesz one zwo flee", Util::replace("test one two flee", 't', 'z'));
    LOK_ASSERT_EQUAL_STR("test one two flee", Util::replace("test one two flee", ' ', ' '));
}

void WhiteBoxTests::testReplaceCharInPlace()
{
    constexpr std::string_view testname = __func__;

    // Can't compile, because the argument is a temporary.
    // LOK_ASSERT_EQUAL_STR("zesz one zwo flee", Util::replaceInPlace("test one two flee", 't', 'z'));
    std::string s = "test one two flee";
    LOK_ASSERT_EQUAL_STR("zesz one zwo flee", Util::replaceInPlace(s, 't', 'z'));
    LOK_ASSERT_EQUAL_STR("zesz one zwo flee", Util::replaceInPlace(s, ' ', ' '));
}

void WhiteBoxTests::testReplaceAllOf()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR("humvee", Util::replaceAllOf("humans", "san", "eve"));
    LOK_ASSERT_EQUAL_STR("simple.odt", Util::replaceAllOf("s#&-le.odt", "#&-", "imp"));
}

void WhiteBoxTests::testRegexListMatcher()
{
    constexpr std::string_view testname = __func__;

    RegexUtil::RegexListMatcher matcher;

    matcher.allow("localhost");
    LOK_ASSERT(matcher.match("localhost"));
    LOK_ASSERT(!matcher.match(""));
    LOK_ASSERT(!matcher.match("localhost2"));
    LOK_ASSERT(!matcher.match("xlocalhost"));
    LOK_ASSERT(!matcher.match("192.168.1.1"));

    matcher.deny("localhost");
    LOK_ASSERT(!matcher.match("localhost"));

    matcher.allow("www[0-9].*");
    LOK_ASSERT(matcher.match("www1example"));

    matcher.allow(R"(192\.168\..*\..*)");
    LOK_ASSERT(matcher.match("192.168.1.1"));
    LOK_ASSERT(matcher.match("192.168.159.1"));
    LOK_ASSERT(matcher.match("192.168.1.134"));
    LOK_ASSERT(!matcher.match("192.169.1.1"));
    LOK_ASSERT(matcher.match("192.168.."));

    matcher.deny(R"(192\.168\.1\..*)");
    LOK_ASSERT(!matcher.match("192.168.1.1"));

    matcher.allow("staging\\.collaboracloudsuite\\.com.*");
    matcher.deny(".*collaboracloudsuite.*");
    LOK_ASSERT(!matcher.match("staging.collaboracloudsuite"));
    LOK_ASSERT(!matcher.match("web.collaboracloudsuite"));
    LOK_ASSERT(!matcher.match("staging.collaboracloudsuite.com"));

    matcher.allow(R"(10\.10\.[0-9]{1,3}\.[0-9]{1,3})");
    matcher.deny(R"(10\.10\.10\.10)");
    LOK_ASSERT(matcher.match("10.10.001.001"));
    LOK_ASSERT(!matcher.match("10.10.10.10"));
    LOK_ASSERT(matcher.match("10.10.250.254"));
}

void WhiteBoxTests::testRegexListMatcher_Init()
{
    constexpr std::string_view testname = __func__;

    RegexUtil::RegexListMatcher matcher;
    matcher.allow("localhost");
    matcher.allow("192\\..*");
    matcher.deny("192\\.168\\..*");

    LOK_ASSERT(matcher.match("localhost"));
    LOK_ASSERT(!matcher.match(""));
    LOK_ASSERT(!matcher.match("localhost2"));
    LOK_ASSERT(!matcher.match("xlocalhost"));
    LOK_ASSERT(!matcher.match("192.168.1.1"));
    LOK_ASSERT(matcher.match("192.172.10.122"));

    matcher.deny("localhost");
    LOK_ASSERT(!matcher.match("localhost"));

    matcher.allow("www[0-9].*");
    LOK_ASSERT(matcher.match("www1example"));

    matcher.allow(R"(192\.168\..*\..*)");
    LOK_ASSERT(!matcher.match("192.168.1.1"));
    LOK_ASSERT(!matcher.match("192.168.159.1"));
    LOK_ASSERT(!matcher.match("192.168.1.134"));
    LOK_ASSERT(matcher.match("192.169.1.1"));
    LOK_ASSERT(!matcher.match("192.168.."));

    matcher.clear();

    matcher.allow(R"(192\.168\..*\..*)");
    LOK_ASSERT(matcher.match("192.168.1.1"));
    LOK_ASSERT(matcher.match("192.168.159.1"));
    LOK_ASSERT(matcher.match("192.168.1.134"));
    LOK_ASSERT(!matcher.match("192.169.1.1"));
    LOK_ASSERT(matcher.match("192.168.."));
}

void WhiteBoxTests::testTileDesc()
{
    constexpr std::string_view testname = __func__;

    // simulate a previous overflow
    errno = ERANGE;
    TileDesc desc = TileDesc::parse(
        "tile nviewid=0 part=5 width=256 height=256 tileposx=0 tileposy=12288 tilewidth=3072 tileheight=3072 oldwid=0 wid=0 ver=33");
    (void)desc; // exception in parse if we have problems.

    // A preview tile names the slide it shows. The unique id survives a parse
    // and serialize round trip.
    TileDesc preview = TileDesc::parse(
        "tile nviewid=0 part=5 width=256 height=256 tileposx=0 tileposy=12288 tilewidth=3072 tileheight=3072 ver=33 id=3 uniqueid=42");
    LOK_ASSERT_EQUAL(static_cast<uint64_t>(42), preview.getUniqueId());
    LOK_ASSERT(preview.serialize().find(" uniqueid=42") != std::string::npos);
    TileCombined combined = TileCombined::parse(
        "tilecombine nviewid=0 part=5 width=256 height=256 tileposx=0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504,0,3072,6144,9216,12288,15360,18432,21504 tileposy=0,0,0,0,0,0,0,0,3072,3072,3072,3072,3072,3072,3072,3072,6144,6144,6144,6144,6144,6144,6144,6144,9216,9216,9216,9216,9216,9216,9216,9216,12288,12288,12288,12288,12288,12288,12288,12288,15360,15360,15360,15360,15360,15360,15360,15360,18432,18432,18432,18432,18432,18432,18432,18432 oldwid=2,3,4,5,6,7,8,8,9,10,11,12,13,14,15,16,17,18,19,20,21,0,0,0,24,25,26,27,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 tilewidth=3072 tileheight=3072");
    (void)combined; // exception in parse if we have problems.

    // Test parsing removing un-used pieces
    std::string base = "tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840 tileposy=0,0 ";
    struct {
        std::string inp;
        std::string outp;
    } tests[] = {
        { "imgsize=0,0 tilewidth=3840 tileheight=3840 ver=-1,-1",
          "tilewidth=3840 tileheight=3840 ver=-1,-1" },
        { "imgsize=1,0 tilewidth=3840 tileheight=3840 ver=-1,-1",
          "imgsize=1,0 tilewidth=3840 tileheight=3840 ver=-1,-1" },
        { "wid=0,0 tilewidth=3840 tileheight=3840 ver=-1,-1",
          "tilewidth=3840 tileheight=3840 ver=-1,-1" },
        { "tilewidth=3840 tileheight=3840 ver=-1,-1 wid=0,1",
          "tilewidth=3840 tileheight=3840 ver=-1,-1 wid=0,1" },
        { "oldwid=0,0 tilewidth=3840 tileheight=3840 ver=-1,-1",
          "tilewidth=3840 tileheight=3840 ver=-1,-1" },
        { "tilewidth=3840 tileheight=3840 ver=-1,-1 oldwid=0,1",
          "tilewidth=3840 tileheight=3840 ver=-1,-1 oldwid=0,1" },
    };
    for (auto &s : tests)
    {
        combined = TileCombined::parse(base + s.inp);
        LOK_ASSERT_EQUAL(combined.serialize("tilecombine"), base + s.outp);
    }
}

void WhiteBoxTests::testTileData()
{
    constexpr std::string_view testname = __func__;

    TileData data(42, "Zfoo", 4);

    // replace keyframe
    data.appendBlob(43, "Zfoo", 4);
    LOK_ASSERT_EQUAL(size_t(3), data.size());

    // append a delta
    data.appendBlob(44, "Dbaa", 4);
    LOK_ASSERT_EQUAL(size_t(6), data.size());

    LOK_ASSERT_EQUAL(false, data.isPng());

    // validation.
    LOK_ASSERT_EQUAL(true, data.isValid());
    data.invalidate();
    LOK_ASSERT_EQUAL(false, data.isValid());

    std::vector<char> out;
    LOK_ASSERT_EQUAL(false, data.appendChangesSince(out, 128));
    LOK_ASSERT_EQUAL(size_t(0), out.size());

    LOK_ASSERT_EQUAL(true, data.appendChangesSince(out, 42));
    LOK_ASSERT_EQUAL_STR("foobaa", Util::toString(out));

    out.clear();
    LOK_ASSERT_EQUAL(true, data.appendChangesSince(out, 43));
    LOK_ASSERT_EQUAL_STR("baa", Util::toString(out));

    // append another delta
    data.appendBlob(47, "Dbaz", 4);
    LOK_ASSERT_EQUAL(size_t(9), data.size());

    out.clear();
    LOK_ASSERT_EQUAL(true, data.appendChangesSince(out, 1));
    LOK_ASSERT_EQUAL_STR("foobaabaz", Util::toString(out));

    out.clear();
    LOK_ASSERT_EQUAL(true, data.appendChangesSince(out, 43));
    LOK_ASSERT_EQUAL_STR("baabaz", Util::toString(out));

    // append an empty delta
    data.appendBlob(52, "D", 1);
    LOK_ASSERT_EQUAL(size_t(9), data.size());
    LOK_ASSERT_EQUAL(size_t(4), data._wids.size());
    LOK_ASSERT_EQUAL(unsigned(52), data._wids.back());

    // the next empty delta should pack into the last one
    data.appendBlob(54, "D", 1);
    LOK_ASSERT_EQUAL(size_t(9), data.size());
    LOK_ASSERT_EQUAL(size_t(4), data._wids.size());
    LOK_ASSERT_EQUAL(unsigned(54), data._wids.back());
}

void WhiteBoxTests::testRectanglesIntersect()
{
    constexpr std::string_view testname = __func__;

    // these intersect
    LOK_ASSERT(TileDesc::rectanglesIntersect(1000, 1000, 2000, 1000,
                                                 2000, 1000, 2000, 1000));
    LOK_ASSERT(TileDesc::rectanglesIntersect(2000, 1000, 2000, 1000,
                                                 1000, 1000, 2000, 1000));

    LOK_ASSERT(TileDesc::rectanglesIntersect(1000, 1000, 2000, 1000,
                                                 3000, 2000, 1000, 1000));
    LOK_ASSERT(TileDesc::rectanglesIntersect(3000, 2000, 1000, 1000,
                                                 1000, 1000, 2000, 1000));

    // these don't
    LOK_ASSERT(!TileDesc::rectanglesIntersect(1000, 1000, 2000, 1000,
                                                  2000, 3000, 2000, 1000));
    LOK_ASSERT(!TileDesc::rectanglesIntersect(2000, 3000, 2000, 1000,
                                                  1000, 1000, 2000, 1000));

    LOK_ASSERT(!TileDesc::rectanglesIntersect(1000, 1000, 2000, 1000,
                                                  2000, 3000, 1000, 1000));
    LOK_ASSERT(!TileDesc::rectanglesIntersect(2000, 3000, 1000, 1000,
                                                  1000, 1000, 2000, 1000));
}

void WhiteBoxTests::testJson()
{
    constexpr std::string_view testname = __func__;

    static const char* testString =
        R"({"BaseFileName":"SomeFile.pdf","DisableCopy":true,"DisableExport":true,"DisableInactiveMessages":true,"DisablePrint":true,"EnableOwnerTermination":true,"HideExportOption":true,"HidePrintOption":true,"OwnerId":"id@owner.com","PostMessageOrigin":"*","Size":193551,"UserCanWrite":true,"UserFriendlyName":"Owning user","UserId":"user@user.com","WatermarkText":null})";

    Poco::JSON::Object::Ptr object;
    LOK_ASSERT(JsonUtil::parseJSON(testString, object));

    std::size_t intValue = 0;
    JsonUtil::findJSONValue(object, "Size", intValue);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(193551), intValue);

    bool boolValue = false;
    JsonUtil::findJSONValue(object, "DisableCopy", boolValue);
    LOK_ASSERT_EQUAL(true, boolValue);

    std::string stringValue;
    JsonUtil::findJSONValue(object, "BaseFileName", stringValue);
    LOK_ASSERT_EQUAL_STR("SomeFile.pdf", stringValue);

    // Don't accept inexact key names.
    stringValue.clear();
    JsonUtil::findJSONValue(object, "basefilename", stringValue);
    LOK_ASSERT_EQUAL(std::string(), stringValue);

    JsonUtil::findJSONValue(object, "invalid", stringValue);
    LOK_ASSERT_EQUAL(std::string(), stringValue);

    JsonUtil::findJSONValue(object, "UserId", stringValue);
    LOK_ASSERT_EQUAL_STR("user@user.com", stringValue);
}

void WhiteBoxTests::testAnonymization()
{
    constexpr std::string_view testname = __func__;

    static const std::string name = "some name with space";
    static const std::string filename = "filename.ext";
    static const std::string filenameTestx = "testx (6).odt";
    static const std::string path = "/path/to/filename.ext";
    static const std::string plainUrl
        = "http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/"
          "736_ocgdpzbkm39u?access_token=Hn0zttjbwkvGWb5BHbDa5ArgTykJAyBl&access_token_ttl=0&"
          "permission=edit";
    static const std::string fileUrl = "http://localhost/owncloud/index.php/apps/richdocuments/"
                                       "wopi/files/736_ocgdpzbkm39u/"
                                       "secret.odt?access_token=Hn0zttjbwkvGWb5BHbDa5ArgTykJAyBl&"
                                       "access_token_ttl=0&permission=edit";

    std::uint64_t anonymizationSalt = 1111111111182589933;
    Anonymizer::initialize(true, anonymizationSalt);

    LOK_ASSERT_EQUAL_STR("#0#5e45aef91248a8aa#", Anonymizer::anonymizeUrl(name));
    LOK_ASSERT_EQUAL_STR("#1#8f8d95bd2a202d00#.odt", Anonymizer::anonymizeUrl(filenameTestx));
    LOK_ASSERT_EQUAL_STR("/path/to/#2#5c872b2d82ecc8a0#.ext", Anonymizer::anonymizeUrl(path));
    LOK_ASSERT_EQUAL_STR("http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/"
                         "#3#22c6f0caad277666#?access_token=Hn0zttjbwkvGWb5BHbDa5ArgTykJAyBl&"
                         "access_token_ttl=0&permission=edit",
                         Anonymizer::anonymizeUrl(plainUrl));
    LOK_ASSERT_EQUAL_STR(
        "http://localhost/owncloud/index.php/apps/richdocuments/wopi/files/736_ocgdpzbkm39u/"
        "#4#294f0dfb18f6a80b#.odt?access_token=Hn0zttjbwkvGWb5BHbDa5ArgTykJAyBl&access_token_ttl=0&"
        "permission=edit",
        Anonymizer::anonymizeUrl(fileUrl));

    anonymizationSalt = 0;
    Anonymizer::initialize(true, anonymizationSalt);

    LOK_ASSERT_EQUAL_STR("#0#42027f9b6df09510#", Anonymizer::anonymizeUrl(name));
    Anonymizer::mapAnonymized(name, name);
    LOK_ASSERT_EQUAL(name, Anonymizer::anonymizeUrl(name));

    LOK_ASSERT_EQUAL_STR("#1#366ab9ebe19ea09e#.ext", Anonymizer::anonymizeUrl(filename));
    Anonymizer::mapAnonymized("filename",
                              "filename"); // Identity map of the filename without extension.
    LOK_ASSERT_EQUAL(filename, Anonymizer::anonymizeUrl(filename));

    LOK_ASSERT_EQUAL_STR("#2#eac31ed57854de54#.odt", Anonymizer::anonymizeUrl(filenameTestx));
    Anonymizer::mapAnonymized("testx (6)",
                              "testx (6)"); // Identity map of the filename without extension.
    LOK_ASSERT_EQUAL(filenameTestx, Anonymizer::anonymizeUrl(filenameTestx));

    LOK_ASSERT_EQUAL(path, Anonymizer::anonymizeUrl(path));

    const std::string urlAnonymized =
        Util::replace(plainUrl, "736_ocgdpzbkm39u", "#3#f64fbe55134cd5f0#");
    LOK_ASSERT_EQUAL(urlAnonymized, Anonymizer::anonymizeUrl(plainUrl));
    Anonymizer::mapAnonymized("736_ocgdpzbkm39u", "736_ocgdpzbkm39u");
    LOK_ASSERT_EQUAL(plainUrl, Anonymizer::anonymizeUrl(plainUrl));

    const std::string urlAnonymized2 = Util::replace(fileUrl, "secret", "#4#dcac6c9cae1b3b95#");
    LOK_ASSERT_EQUAL(urlAnonymized2, Anonymizer::anonymizeUrl(fileUrl));
    Anonymizer::mapAnonymized("secret", "736_ocgdpzbkm39u");
    const std::string urlAnonymized3 = Util::replace(fileUrl, "secret", "736_ocgdpzbkm39u");
    LOK_ASSERT_EQUAL(urlAnonymized3, Anonymizer::anonymizeUrl(fileUrl));
}

void WhiteBoxTests::testStat()
{
    constexpr std::string_view testname = __func__;

    FileUtil::Stat invalid("/missing/file/path");
    LOK_ASSERT(!invalid.good());
    LOK_ASSERT(invalid.bad());
    LOK_ASSERT(!invalid.exists());

    const std::string tmpFile = FileUtil::getSysTempDirectoryPath() + "/test_stat";
    std::ofstream ofs(tmpFile);
    FileUtil::Stat st(tmpFile);
    LOK_ASSERT(st.good());
    LOK_ASSERT(!st.bad());
    LOK_ASSERT(st.exists());
    LOK_ASSERT(!st.isDirectory());
    LOK_ASSERT(st.isFile());
    LOK_ASSERT(!st.isLink());

    // Modified-time tests.
    // Some test might fail when the system has a different resolution for file timestamps
    // and time_point. Specifically, if the filesystem has microsecond precision but time_point
    // has lower resolution (milliseconds or seconds, f.e.), modifiedTimepoint() will not match
    // modifiedTimeUs(), and the checks will fail.
    // So far, microseconds seem to be the lower common denominator. At least on Android and
    // iOS that's the precision of time_point (as of late 2020), but Linux servers have
    // nanosecond precision.

    LOK_ASSERT(std::chrono::time_point_cast<std::chrono::microseconds>(st.modifiedTimepoint())
                   .time_since_epoch()
                   .count()
               == static_cast<long>(st.modifiedTimeUs()));
    LOK_ASSERT(std::chrono::time_point_cast<std::chrono::milliseconds>(st.modifiedTimepoint())
                   .time_since_epoch()
                   .count()
               == static_cast<long>(st.modifiedTimeMs()));
    LOK_ASSERT(std::chrono::time_point_cast<std::chrono::seconds>(st.modifiedTimepoint())
                   .time_since_epoch()
                   .count()
               == static_cast<long>(st.modifiedTimeMs() / 1000));
    LOK_ASSERT_EQUAL(static_cast<long>(st.modifiedTimeMs() / 1000), st.modifiedTime().tv_sec);
    LOK_ASSERT(st.modifiedTime().tv_nsec / 1000
               == static_cast<long>(st.modifiedTimeUs())
                      - (st.modifiedTime().tv_sec * 1000 * 1000));

    ofs.close();
    FileUtil::removeFile(tmpFile);
}

void WhiteBoxTests::testStringCompare()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT(Util::iequal("abcd", "abcd"));
    LOK_ASSERT(Util::iequal("aBcd", "abCd"));
    LOK_ASSERT(Util::iequal("", ""));

    LOK_ASSERT(!Util::iequal("abcd", "abc"));
    LOK_ASSERT(!Util::iequal("abc", "abcd"));
    LOK_ASSERT(!Util::iequal("abc", "abcd"));

    LOK_ASSERT(!Util::iequal("abc", 3, "abcd", 4));
}

void WhiteBoxTests::testJsonUtilEscapeJSONValue()
{
    constexpr std::string_view testname = __func__;

    constexpr std::string_view in = "domain\\username";
    const std::string expected = "domain\\\\username";
    LOK_ASSERT_EQUAL(JsonUtil::escapeJSONValue(in), expected);
}

namespace
{
STATE_ENUM(TestState, First, Second, Last);
}

void WhiteBoxTests::testStateEnum()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR("TestState::First", name(TestState::First));
    LOK_ASSERT_EQUAL_STR("TestState::Second", name(TestState::Second));
    LOK_ASSERT_EQUAL_STR("TestState::Last", name(TestState::Last));

    LOK_ASSERT_EQUAL_STR("First", nameShort(TestState::First));
    LOK_ASSERT_EQUAL_STR("Second", nameShort(TestState::Second));
    LOK_ASSERT_EQUAL_STR("Last", nameShort(TestState::Last));

    TestState e = TestState::First;

    e = TestState::First;
    LOK_ASSERT_EQUAL_STR("TestState::First", name(e));
    e = TestState::Second;
    LOK_ASSERT_EQUAL_STR("TestState::Second", name(e));
    e = TestState::Last;
    LOK_ASSERT_EQUAL_STR("TestState::Last", name(e));

    e = TestState::First;
    LOK_ASSERT_EQUAL_STR("First", nameShort(e));
    e = TestState::Second;
    LOK_ASSERT_EQUAL_STR("Second", nameShort(e));
    e = TestState::Last;
    LOK_ASSERT_EQUAL_STR("Last", nameShort(e));

    std::ostringstream oss;

    e = TestState::First;
    oss << e;
    LOK_ASSERT_EQUAL_STR("TestState::First", oss.str());
    oss.str("");

    e = TestState::Second;
    oss << e;
    LOK_ASSERT_EQUAL_STR("TestState::Second", oss.str());
    oss.str("");

    e = TestState::Last;
    oss << e;
    LOK_ASSERT_EQUAL_STR("TestState::Last", oss.str());
    oss.str("");
}

void WhiteBoxTests::testFindInVector()
{
    constexpr std::string_view testname = __func__;
    std::string s("fooBarfooBaz");
    std::vector<char> v(s.begin(), s.end());

    // Normal case, we find the first "foo".
    std::size_t ret = Util::findInVector(v, "foo");
    std::size_t expected = 0;
    LOK_ASSERT_EQUAL(expected, ret);

    // Offset, so we find the second "foo".
    ret = Util::findInVector(v, "foo", 1);
    expected = 6;
    LOK_ASSERT_EQUAL(expected, ret);

    // Negative testing.
    ret = Util::findInVector(v, "blah");
    expected = std::string::npos;
    LOK_ASSERT_EQUAL(expected, ret);
}

void WhiteBoxTests::testJoinPair()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR(std::string(), Util::joinPair(std::vector<int>()));
    LOK_ASSERT_EQUAL_STR(std::string(), Util::joinPair(std::vector<int>(), "bazinga"));
    LOK_ASSERT_EQUAL_STR(std::string(), Util::joinPair(std::vector<int>(), "bazinga", "more"));

    LOK_ASSERT_EQUAL_STR("1", Util::joinPair<std::vector<int>>({ 1 }));
    LOK_ASSERT_EQUAL_STR("1", Util::joinPair<std::vector<int>>({ 1 }, "bazinga"));
    LOK_ASSERT_EQUAL_STR("1", Util::joinPair<std::vector<int>>({ 1 }, "bazinga", "more"));

    LOK_ASSERT_EQUAL_STR("1 / 2", Util::joinPair<std::vector<int>>({ 1, 2 }));
    LOK_ASSERT_EQUAL_STR("1bazinga2", Util::joinPair<std::vector<int>>({ 1, 2 }, "bazinga"));
    LOK_ASSERT_EQUAL_STR("1bazingamore2",
                         Util::joinPair<std::vector<int>>({ 1, 2 }, "bazinga", "more"));

    LOK_ASSERT_EQUAL_STR("1 / 2", Util::joinPair<std::vector<int>>({ 1, 2 }));
    LOK_ASSERT_EQUAL_STR("132", Util::joinPair<std::vector<int>>({ 1, 2 }, 3));
    LOK_ASSERT_EQUAL_STR("1342", Util::joinPair<std::vector<int>>({ 1, 2 }, 3, 4));

    LOK_ASSERT_EQUAL_STR("1 / 2 / 3 / 4 / 5 / 6 / 7",
                         Util::joinPair<std::vector<int>>({ 1, 2, 3, 4, 5, 6, 7 }));
    LOK_ASSERT_EQUAL_STR("1323334353637",
                         Util::joinPair<std::vector<int>>({ 1, 2, 3, 4, 5, 6, 7 }, 3));
    LOK_ASSERT_EQUAL_STR("1342343344345346347",
                         Util::joinPair<std::vector<int>>({ 1, 2, 3, 4, 5, 6, 7 }, 3, 4));
}

#if 0
size_t WhiteBoxTests::waitForThreads(size_t count)
{
    auto start = std::chrono::steady_clock::now();
    while (Util::getCurrentThreadCount() != count)
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count() >= 250)
        {
            std::cerr << "Failed to get correct thread count " << count <<
                " instead we have " << Util::getCurrentThreadCount() << "\n";
            break;
        }
        std::this_thread::sleep_for(10ms);
    }
    return Util::getCurrentThreadCount();
}
#endif

void WhiteBoxTests::testThreadPool()
{
    constexpr std::string_view testname = __func__;
    //    const size_t existingUnrelatedThreads = Util::getCurrentThreadCount();
    // coverity[tainted_data_argument : FALSE] - we trust this variable in tests
    setenv("MAX_CONCURRENCY","8",1);
    // coverity[tainted_argument] : don't warn that getenv("MAX_CONCURRENCY") is tainted
    ThreadPool pool;
    LOK_ASSERT_EQUAL(int(8), pool._maxConcurrency);
    LOK_ASSERT_EQUAL(size_t(7), pool._threads.size());
//    LOK_ASSERT_EQUAL(size_t(7 + existingUnrelatedThreads), waitForThreads(8 + existingUnrelatedThreads));

    pool.stop();
    LOK_ASSERT_EQUAL(size_t(0), pool._threads.size());
//    LOK_ASSERT_EQUAL(size_t(existingUnrelatedThreads), waitForThreads(existingUnrelatedThreads));

    pool.start();
    LOK_ASSERT_EQUAL(size_t(7), pool._threads.size());
//    LOK_ASSERT_EQUAL(size_t(7 + existingUnrelatedThreads), waitForThreads(8 + existingUnrelatedThreads));
}

void WhiteBoxTests::testLogCaptureCaller()
{
    constexpr std::string_view testname = __func__;

    const auto logWithoutCaller = []() -> std::string
    {
        std::ostringstream oss;
        LOG_END(oss);
        return oss.str();
    };
    // Should return an empty string.
    const std::string withoutCaller = logWithoutCaller();
    TST_LOG("Without caller: [" << withoutCaller << ']');
    LOK_ASSERT_MESSAGE("Unexpected to find the parent's source location",
                       withoutCaller.find("(from") == std::string::npos);

#line __LINE__ "WhiteBoxTests.cpp"
    const auto logWithCaller = [](LOG_CAPTURE_CALLER_DECLARATION) -> std::string
    {
        std::ostringstream oss;
        LOG_END(oss);
        return oss.str();
    };
    // Should return something like "(from WhiteBoxTests.cpp:1031)|"
    const std::string withCaller = logWithCaller();
    TST_LOG("With caller: [" << withCaller << ']');
    LOK_ASSERT_MESSAGE("Expected to find the parent's source location",
                       withCaller.find("|(from WhiteBoxTests.cpp:") != std::string::npos);
}

void WhiteBoxTests::testIsIso8601()
{
    constexpr auto testname = __func__;

    // Second-precision and fractional-second forms are accepted, including a
    // trailing zone marker after the fraction (the form coolwsd itself emits).
    LOK_ASSERT(Util::isIso8601("2021-03-04T05:06:07"));
    LOK_ASSERT(Util::isIso8601("2021-03-04T05:06:07.1"));
    LOK_ASSERT(Util::isIso8601("2021-03-04T05:06:07.123456"));
    LOK_ASSERT(Util::isIso8601("2021-03-04T05:06:07.123456Z"));

    // Empty, non-timestamp text, and a date with no time are rejected.
    LOK_ASSERT(!Util::isIso8601(""));
    LOK_ASSERT(!Util::isIso8601("not a timestamp"));
    LOK_ASSERT(!Util::isIso8601("2021-03-04"));

    // A dot with no following digit is rejected.
    LOK_ASSERT(!Util::isIso8601("2021-03-04T05:06:07."));
    LOK_ASSERT(!Util::isIso8601("2021-03-04T05:06:07.Z"));

    // Forms iso8601ToTimestamp does not parse (bare zone marker or numeric
    // offset without a fraction) are rejected.
    LOK_ASSERT(!Util::isIso8601("2021-03-04T05:06:07Z"));
    LOK_ASSERT(!Util::isIso8601("2021-03-04T05:06:07+01:00"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(WhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
