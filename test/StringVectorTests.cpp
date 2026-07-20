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

#include <test/lokassert.hpp>
#include <cppunit/extensions/HelperMacros.h>

#include <common/StringVector.hpp>

/// StringVector unit-tests.
class StringVectorTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(StringVectorTests);
    CPPUNIT_TEST(testTokenizer);
    CPPUNIT_TEST(testTokenizerTokenizeAnyOf);
    CPPUNIT_TEST(testStringVector);
    CPPUNIT_TEST(testCat);
    CPPUNIT_TEST_SUITE_END();

    void testTokenizer();
    void testTokenizerTokenizeAnyOf();
    void testStringVector();
    void testCat();
    void testSubstrFromToken();
};

void StringVectorTests::testTokenizer()
{
    constexpr std::string_view testname = __func__;

    StringVector tokens;

    tokens = StringVector::tokenize("");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), tokens.size());

    tokens = StringVector::tokenize("  ");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), tokens.size());

    tokens = StringVector::tokenize("A");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);

    tokens = StringVector::tokenize("  A");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);

    tokens = StringVector::tokenize("A  ");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);

    tokens = StringVector::tokenize(" A ");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);

    tokens = StringVector::tokenize(" A  Z ");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);
    LOK_ASSERT_EQUAL_STR("Z", tokens[1]);

    tokens = StringVector::tokenize("\n");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), tokens.size());

    tokens = StringVector::tokenize(" A  \nZ ");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);

    tokens = StringVector::tokenize(" A  Z\n ");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);
    LOK_ASSERT_EQUAL_STR("Z", tokens[1]);

    tokens = StringVector::tokenize(" A  Z  \n ");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);
    LOK_ASSERT_EQUAL_STR("Z", tokens[1]);

    tokens = StringVector::tokenize("tile nviewid=0 part=0 width=256 height=256 tileposx=0 "
                                    "tileposy=0 tilewidth=3840 tileheight=3840 ver=-1");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(10), tokens.size());
    LOK_ASSERT_EQUAL_STR("tile", tokens[0]);
    LOK_ASSERT_EQUAL_STR("nviewid=0", tokens[1]);
    LOK_ASSERT_EQUAL_STR("part=0", tokens[2]);
    LOK_ASSERT_EQUAL_STR("width=256", tokens[3]);
    LOK_ASSERT_EQUAL_STR("height=256", tokens[4]);
    LOK_ASSERT_EQUAL_STR("tileposx=0", tokens[5]);
    LOK_ASSERT_EQUAL_STR("tileposy=0", tokens[6]);
    LOK_ASSERT_EQUAL_STR("tilewidth=3840", tokens[7]);
    LOK_ASSERT_EQUAL_STR("tileheight=3840", tokens[8]);
    LOK_ASSERT_EQUAL_STR("ver=-1", tokens[9]);

    // With custom delimiters
    tokens = StringVector::tokenize(std::string("ABC:DEF"), ':');
    LOK_ASSERT_EQUAL_STR("ABC", tokens[0]);
    LOK_ASSERT_EQUAL_STR("DEF", tokens[1]);

    tokens = StringVector::tokenize(std::string("ABC,DEF,XYZ"), ',');
    LOK_ASSERT_EQUAL_STR("ABC", tokens[0]);
    LOK_ASSERT_EQUAL_STR("DEF", tokens[1]);
    LOK_ASSERT_EQUAL_STR("XYZ", tokens[2]);

    static const std::string URI
        = "/cool/"
          "http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%2Ffiles%"
          "2F593_ocqiesh0cngs%3Faccess_token%3DMN0KXXDv9GJ1wCCLnQcjVQT2T7WrfYpA%26access_token_ttl%"
          "3D0%26reuse_cookies%3Doc_sessionPassphrase%"
          "253D8nFRqycbs7bP97yxCuJviBbVKdCXmuiXp6ZYH0DfUoy5UZDCTQgLwluvbgRbKrdKodJteG3uNE19KNUAoE5t"
          "ypf4oBGwJdFY%25252F5W9RNST8wEHWkUVIjZy7vmY0ZX38PlS%253Anc_sameSiteCookielax%253Dtrue%"
          "253Anc_sameSiteCookiestrict%253Dtrue%253Aocqiesh0cngs%253Dr5ujg4tpvgu9paaf5bguiokgjl%"
          "253AXCookieName%253DXCookieValue%253ASuperCookieName%253DBAZINGA/"
          "ws?WOPISrc=http%3A%2F%2Flocalhost%2Fnextcloud%2Findex.php%2Fapps%2Frichdocuments%2Fwopi%"
          "2Ffiles%2F593_ocqiesh0cngs&compat=/ws/b26112ab1b6f2ed98ce1329f0f344791/close/31";

    tokens = StringVector::tokenize(URI, '/');
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(7), tokens.size());
    LOK_ASSERT_EQUAL_STR("31", tokens[6]);
}

void StringVectorTests::testTokenizerTokenizeAnyOf()
{
    constexpr std::string_view testname = __func__;

    StringVector tokens;
    const char delimiters[] = "\n\r"; // any of these delimits; and we trim whitespace

    tokens = StringVector::tokenizeAnyOf("", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), tokens.size());

    tokens = StringVector::tokenizeAnyOf("  ", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), tokens.size());

    tokens = StringVector::tokenizeAnyOf("A", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);

    tokens = StringVector::tokenizeAnyOf("  A", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);

    tokens = StringVector::tokenizeAnyOf("A  ", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);

    tokens = StringVector::tokenizeAnyOf(" A ", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);

    tokens = StringVector::tokenizeAnyOf(" A  Z ", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A  Z", tokens[0]);

    tokens = StringVector::tokenizeAnyOf("\n", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), tokens.size());

    tokens = StringVector::tokenizeAnyOf("\n\r\r\n", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), tokens.size());

    tokens = StringVector::tokenizeAnyOf(" A  \nZ ", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);
    LOK_ASSERT_EQUAL_STR("Z", tokens[1]);

    tokens = StringVector::tokenizeAnyOf(" A  Z\n ", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A  Z", tokens[0]);

    tokens = StringVector::tokenizeAnyOf(" A  Z  \n\r\r\n ", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), tokens.size());
    LOK_ASSERT_EQUAL_STR("A  Z", tokens[0]);

    tokens = StringVector::tokenizeAnyOf(" A  \n\r\r\n  \r  \n  Z  \n ", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);
    LOK_ASSERT_EQUAL_STR("Z", tokens[1]);

    tokens = StringVector::tokenizeAnyOf("  \r A  \n  \r  \n  Z  \n ", delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);
    LOK_ASSERT_EQUAL_STR("Z", tokens[1]);

    tokens = StringVector::tokenizeAnyOf(std::string("A\rB\nC\n\rD\r\nE\r\rF\n\nG\r\r\n\nH"),
                                         delimiters);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(8), tokens.size());
    LOK_ASSERT_EQUAL_STR("A", tokens[0]);
    LOK_ASSERT_EQUAL_STR("B", tokens[1]);
    LOK_ASSERT_EQUAL_STR("C", tokens[2]);
    LOK_ASSERT_EQUAL_STR("D", tokens[3]);
    LOK_ASSERT_EQUAL_STR("E", tokens[4]);
    LOK_ASSERT_EQUAL_STR("F", tokens[5]);
    LOK_ASSERT_EQUAL_STR("G", tokens[6]);
    LOK_ASSERT_EQUAL_STR("H", tokens[7]);
}

void StringVectorTests::testStringVector()
{
    constexpr std::string_view testname = __func__;

    // Test push_back() and getParam().
    StringVector vector;
    vector.push_back("a");
    vector.push_back("b");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), vector.size());
    auto it = vector.begin();
    LOK_ASSERT_EQUAL_STR("a", vector.getParam(*it));
    ++it;
    LOK_ASSERT_EQUAL_STR("b", vector.getParam(*it));

    // Test operator []().
    LOK_ASSERT_EQUAL_STR("a", vector[0]);
    LOK_ASSERT_EQUAL_STR("", vector[2]);

    // Test equals().
    LOK_ASSERT(vector.equals(0, "a"));
    LOK_ASSERT(!vector.equals(0, "A"));
    LOK_ASSERT(vector.equals(1, "b"));
    LOK_ASSERT(!vector.equals(1, "B"));
    LOK_ASSERT(!vector.equals(2, ""));

    // Test equals(), StringVector argument version.
    StringVector vector2;
    vector2.push_back("a");
    vector2.push_back("B");

    LOK_ASSERT(vector.equals(0, vector2, 0));
    LOK_ASSERT(!vector.equals(0, vector2, 1));

    // Test startsWith().
    StringVector vector3;
    vector3.push_back("hello, world");
    vector3.push_back("goodbye, world");

    LOK_ASSERT(vector3.startsWith(0, "hello"));
    LOK_ASSERT(vector3.startsWith(0, "hello, world"));
    LOK_ASSERT(!vector3.startsWith(0, "hello, world!"));
    LOK_ASSERT(!vector3.startsWith(0, "hello, world! super long text"));
    LOK_ASSERT(vector3.startsWith(1, "goodbye"));
    LOK_ASSERT(!vector3.startsWith(1, "hello"));

    // Test startsWith(), StringToken argument version
    StringToken hello = *vector3.begin();
    StringToken goodbye = *std::next(vector3.begin());
    StringToken unrelated(50, 10); // out of vector3 range

    LOK_ASSERT(vector3.startsWith(hello, "hello"));
    LOK_ASSERT(vector3.startsWith(hello, "hello, world"));
    LOK_ASSERT(!vector3.startsWith(hello, "hello, world!"));
    LOK_ASSERT(!vector3.startsWith(hello, "hello, world! super long text"));
    LOK_ASSERT(vector3.startsWith(goodbye, "goodbye"));
    LOK_ASSERT(!vector3.startsWith(goodbye, "hello"));
    LOK_ASSERT(!vector3.startsWith(unrelated, "hello"));

    {
        StringVector tokens;
        tokens.push_back("a=1");
        uint32_t value{};
        LOK_ASSERT(tokens.getUInt32(0, "a", value));
        LOK_ASSERT_EQUAL(static_cast<uint32_t>(1), value);

        // Prefix does not match.
        LOK_ASSERT(!tokens.getUInt32(0, "b", value));

        // Index is out of bounds.
        LOK_ASSERT(!tokens.getUInt32(1, "a", value));

        // Expected key is prefix of actual key.
        tokens.push_back("bb=1");
        LOK_ASSERT(!tokens.getUInt32(1, "b", value));

        // Actual key is prefix of expected key.
        tokens.push_back("c=1");
        LOK_ASSERT(!tokens.getUInt32(1, "cc", value));
    }

    {
        StringVector tokens;
        // A value too large for 32 bits is returned whole.
        tokens.push_back("a=5000000000");
        uint64_t value{};
        LOK_ASSERT(tokens.getUInt64(0, "a", value));
        LOK_ASSERT_EQUAL(static_cast<uint64_t>(5000000000), value);

        // Prefix does not match.
        LOK_ASSERT(!tokens.getUInt64(0, "b", value));

        // Index is out of bounds.
        LOK_ASSERT(!tokens.getUInt64(1, "a", value));

        // Expected key is prefix of actual key.
        tokens.push_back("bb=1");
        LOK_ASSERT(!tokens.getUInt64(1, "b", value));

        // Actual key is prefix of expected key.
        tokens.push_back("c=1");
        LOK_ASSERT(!tokens.getUInt64(1, "cc", value));

        // A key with nothing after the equals sign has no value.
        tokens.push_back("d=");
        LOK_ASSERT(!tokens.getUInt64(3, "d", value));
    }

    {
        StringVector tokens;
        tokens.push_back("a=1");
        std::string name;
        int value{};
        LOK_ASSERT(tokens.getNameIntegerPair(0, name, value));
        LOK_ASSERT_EQUAL_STR("a", name);
        LOK_ASSERT_EQUAL(1, value);

        tokens.push_back("aa=1");
        LOK_ASSERT(tokens.getNameIntegerPair(1, name, value));
        LOK_ASSERT_EQUAL_STR("aa", name);
        LOK_ASSERT_EQUAL(1, value);

        tokens.push_back("a=11");
        LOK_ASSERT(tokens.getNameIntegerPair(2, name, value));
        LOK_ASSERT_EQUAL_STR("a", name);
        LOK_ASSERT_EQUAL(11, value);
    }
}

void StringVectorTests::testCat()
{
    constexpr std::string_view testname = __func__;

    // Test push_back() and getParam().
    StringVector vector;
    vector.push_back("a");
    vector.push_back("b");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), vector.size());

    // Test cat().
    LOK_ASSERT_EQUAL_STR("a b", vector.cat(" ", 0));
    LOK_ASSERT_EQUAL_STR("a b", vector.cat(' ', 0));
    LOK_ASSERT_EQUAL_STR("a*b", vector.cat('*', 0));
    LOK_ASSERT_EQUAL_STR("a blah mlah b", vector.cat(" blah mlah ", 0));
    LOK_ASSERT_EQUAL(std::string(), vector.cat(" ", 3));
    LOK_ASSERT_EQUAL(std::string(), vector.cat(" ", 42));

    // Test equals().
    LOK_ASSERT(vector.equals(0, "a"));
    LOK_ASSERT(!vector.equals(0, "A"));
    LOK_ASSERT(vector.equals(1, "b"));
    LOK_ASSERT(!vector.equals(1, "B"));
    LOK_ASSERT(!vector.equals(2, ""));

    // Test cat() with more tokens.
    vector.push_back("c");
    vector.push_back("d");
    vector.push_back("e");
    LOK_ASSERT_EQUAL_STR("a/b/c/d/e", vector.cat('/', 0));
    LOK_ASSERT_EQUAL_STR("b/c/d/e", vector.cat('/', 1));
    LOK_ASSERT_EQUAL_STR("c/d/e", vector.cat('/', 2));
    LOK_ASSERT_EQUAL_STR("d/e", vector.cat('/', 3));
    LOK_ASSERT_EQUAL_STR("e", vector.cat('/', 4));
    LOK_ASSERT_EQUAL(std::string(), vector.cat('/', 5));

    LOK_ASSERT_EQUAL_STR("a/b/c/d/e", vector.cat('/', 0, 7));
    LOK_ASSERT_EQUAL_STR("b/c/d/e", vector.cat('/', 1, 7));
    LOK_ASSERT_EQUAL_STR("c/d/e", vector.cat('/', 2, 7));
    LOK_ASSERT_EQUAL_STR("d/e", vector.cat('/', 3, 7));
    LOK_ASSERT_EQUAL_STR("e", vector.cat('/', 4, 7));
    LOK_ASSERT_EQUAL(std::string(), vector.cat('/', 5, 7));
    LOK_ASSERT_EQUAL(std::string(), vector.cat('/', 3, 0));

    LOK_ASSERT_EQUAL_STR("a/b/c/d/e", vector.cat('/', 0, 4));
    LOK_ASSERT_EQUAL_STR("b/c/d/e", vector.cat('/', 1, 4));
    LOK_ASSERT_EQUAL_STR("c/d/e", vector.cat('/', 2, 4));
    LOK_ASSERT_EQUAL_STR("d/e", vector.cat('/', 3, 4));
    LOK_ASSERT_EQUAL_STR("e", vector.cat('/', 4, 4));
    LOK_ASSERT_EQUAL(std::string(), vector.cat('/', 5, 4));

    vector = StringVector::tokenize("first second third forth fifth");
    LOK_ASSERT_EQUAL_STR("first second third forth fifth", vector.cat(' ', 0, 5));
    LOK_ASSERT_EQUAL_STR("first second third forth fifth", vector.cat(' ', 0, 4));
    LOK_ASSERT_EQUAL_STR("first second third forth", vector.cat(' ', 0, 3));
    LOK_ASSERT_EQUAL_STR("first second third", vector.cat(' ', 0, 2));
    LOK_ASSERT_EQUAL_STR("first second", vector.cat(' ', 0, 1));
    LOK_ASSERT_EQUAL_STR("first", vector.cat(' ', 0, 0));
    LOK_ASSERT_EQUAL(std::string(), vector.cat(' ', 1, 0));
    LOK_ASSERT_EQUAL_STR("second third forth fifth", vector.cat(' ', 1, 5));
    LOK_ASSERT_EQUAL_STR("third forth fifth", vector.cat(' ', 2, 5));
    LOK_ASSERT_EQUAL_STR("forth fifth", vector.cat(' ', 3, 5));
    LOK_ASSERT_EQUAL_STR("fifth", vector.cat(' ', 4, 5));
    LOK_ASSERT_EQUAL(std::string(), vector.cat(' ', 5, 5));
    LOK_ASSERT_EQUAL_STR("second third forth fifth", vector.cat(' ', 1, 4));
    LOK_ASSERT_EQUAL_STR("third forth", vector.cat(' ', 2, 3));
    LOK_ASSERT_EQUAL_STR("third", vector.cat(' ', 2, 2));
    LOK_ASSERT_EQUAL_STR("forth", vector.cat(' ', 3, 3));
    LOK_ASSERT_EQUAL_STR("fifth", vector.cat(' ', 4, 4));
    LOK_ASSERT_EQUAL(std::string(), vector.cat(' ', 5, 5));
}

void StringVectorTests::testSubstrFromToken()
{
    constexpr std::string_view testname = __func__;

    // Test push_back() and getParam().
    StringVector vector;
    vector.push_back("a");
    vector.push_back("b");
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), vector.size());

    // Test substrFromToken().
    LOK_ASSERT_EQUAL_STR("ab", vector.substrFromToken(0));
    LOK_ASSERT_EQUAL_STR("b", vector.substrFromToken(1));
    LOK_ASSERT_EQUAL(std::string(), vector.substrFromToken(3));
    LOK_ASSERT_EQUAL(std::string(), vector.substrFromToken(42));

    vector.push_back("c");
    vector.push_back("d");
    vector.push_back("e");

    // Test substrFromToken() with more tokens.
    LOK_ASSERT_EQUAL_STR("abcde", vector.substrFromToken(0));
    LOK_ASSERT_EQUAL_STR("bcde", vector.substrFromToken(1));
    LOK_ASSERT_EQUAL_STR("cde", vector.substrFromToken(2));
    LOK_ASSERT_EQUAL_STR("de", vector.substrFromToken(3));
    LOK_ASSERT_EQUAL_STR("e", vector.substrFromToken(4));
    LOK_ASSERT_EQUAL(std::string(), vector.substrFromToken(5));

    // Test substrFromToken() with lastOffset.
    LOK_ASSERT_EQUAL_STR("abcde", vector.substrFromToken(0, 5));
    LOK_ASSERT_EQUAL_STR("abcde", vector.substrFromToken(0, 4));
    LOK_ASSERT_EQUAL_STR("abcd", vector.substrFromToken(0, 3));
    LOK_ASSERT_EQUAL_STR("abc", vector.substrFromToken(0, 2));
    LOK_ASSERT_EQUAL_STR("ab", vector.substrFromToken(0, 1));
    LOK_ASSERT_EQUAL_STR("a", vector.substrFromToken(0, 0));
    LOK_ASSERT_EQUAL(std::string(), vector.substrFromToken(1, 0));

    // Test substrFromToken() with more tokens.
    LOK_ASSERT_EQUAL_STR("abcde", vector.substrFromToken(0));
    LOK_ASSERT_EQUAL_STR("bcde", vector.substrFromToken(1));
    LOK_ASSERT_EQUAL_STR("cde", vector.substrFromToken(2));
    LOK_ASSERT_EQUAL_STR("de", vector.substrFromToken(3));
    LOK_ASSERT_EQUAL_STR("e", vector.substrFromToken(4));
    LOK_ASSERT_EQUAL(std::string(), vector.substrFromToken(5));

    // Test substrFromToken() with lastOffset.
    LOK_ASSERT_EQUAL_STR("abcde", vector.substrFromToken(0, 5));
    LOK_ASSERT_EQUAL_STR("abcde", vector.substrFromToken(0, 4));
    LOK_ASSERT_EQUAL_STR("abcd", vector.substrFromToken(0, 3));
    LOK_ASSERT_EQUAL_STR("abc", vector.substrFromToken(0, 2));
    LOK_ASSERT_EQUAL_STR("ab", vector.substrFromToken(0, 1));
    LOK_ASSERT_EQUAL_STR("a", vector.substrFromToken(0, 0));
    LOK_ASSERT_EQUAL(std::string(), vector.substrFromToken(1, 0));

    vector = StringVector::tokenize("first second third forth fifth");
    LOK_ASSERT_EQUAL_STR("first second third forth fifth", vector.substrFromToken(0, 5));
    LOK_ASSERT_EQUAL_STR("first second third forth fifth", vector.substrFromToken(0, 4));
    LOK_ASSERT_EQUAL_STR("first second third forth", vector.substrFromToken(0, 3));
    LOK_ASSERT_EQUAL_STR("first second third", vector.substrFromToken(0, 2));
    LOK_ASSERT_EQUAL_STR("first second", vector.substrFromToken(0, 1));
    LOK_ASSERT_EQUAL_STR("first", vector.substrFromToken(0, 0));
    LOK_ASSERT_EQUAL(std::string(), vector.substrFromToken(1, 0));
    LOK_ASSERT_EQUAL_STR("second third forth fifth", vector.substrFromToken(1, 5));
    LOK_ASSERT_EQUAL_STR("third forth fifth", vector.substrFromToken(2, 5));
    LOK_ASSERT_EQUAL_STR("forth fifth", vector.substrFromToken(3, 5));
    LOK_ASSERT_EQUAL_STR("fifth", vector.substrFromToken(4, 5));
    LOK_ASSERT_EQUAL(std::string(), vector.substrFromToken(5, 5));
    LOK_ASSERT_EQUAL_STR("second third forth fifth", vector.substrFromToken(1, 4));
    LOK_ASSERT_EQUAL_STR("third forth", vector.substrFromToken(2, 3));
    LOK_ASSERT_EQUAL_STR("third", vector.substrFromToken(2, 2));
    LOK_ASSERT_EQUAL_STR("forth", vector.substrFromToken(3, 3));
    LOK_ASSERT_EQUAL_STR("fifth", vector.substrFromToken(4, 4));
    LOK_ASSERT_EQUAL(std::string(), vector.substrFromToken(5, 5));
}

CPPUNIT_TEST_SUITE_REGISTRATION(StringVectorTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
