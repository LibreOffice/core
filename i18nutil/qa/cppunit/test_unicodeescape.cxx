/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <i18nutil/unicodeescape.hxx>
#include <o3tl/string_view.hxx>

using namespace o3tl;

class TestUnicodeEscape : public CppUnit::TestFixture
{
public:
    void testBasicUnicodeEscape();
    void testExtendedUnicodeEscape();
    void testEscapedBackslash();
    void testInvalidEscape();
    void testControlCharFiltering();
    void testSurrogatePair();

    CPPUNIT_TEST_SUITE(TestUnicodeEscape);
    CPPUNIT_TEST(testBasicUnicodeEscape);
    CPPUNIT_TEST(testExtendedUnicodeEscape);
    CPPUNIT_TEST(testEscapedBackslash);
    CPPUNIT_TEST(testInvalidEscape);
    CPPUNIT_TEST(testControlCharFiltering);
    CPPUNIT_TEST(testSurrogatePair);
    CPPUNIT_TEST_SUITE_END();
};

void TestUnicodeEscape::testBasicUnicodeEscape()
{
    // \u0041 should become 'A'
    OUString input(u"\\u0041"_ustr);
    OUString result = i18nutil::processUnicodeEscapes(input);
    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, result);

    // Multiple escapes
    input = u"\\u0041\\u0042\\u0043"_ustr;
    result = i18nutil::processUnicodeEscapes(input);
    CPPUNIT_ASSERT_EQUAL(u"ABC"_ustr, result);

    // Mixed with text
    input = u"Hello \\u0041 World"_ustr;
    result = i18nutil::processUnicodeEscapes(input);
    CPPUNIT_ASSERT_EQUAL(u"Hello A World"_ustr, result);
}

void TestUnicodeEscape::testExtendedUnicodeEscape()
{
    // \U00000041 should become 'A'
    OUString result = i18nutil::processUnicodeEscapes(u"\\U00000041");
    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, result);
}

void TestUnicodeEscape::testEscapedBackslash()
{
    // \\u0041 should become literal \u0041
    OUString result = i18nutil::processUnicodeEscapes(u"\\\\u0041");
    CPPUNIT_ASSERT_EQUAL(u"\\u0041"_ustr, result);
}

void TestUnicodeEscape::testInvalidEscape()
{
    // Invalid hex should keep literal
    OUString input(u"\\uXYZW"_ustr);
    OUString result = i18nutil::processUnicodeEscapes(input);
    CPPUNIT_ASSERT_EQUAL(input, result);

    // Incomplete escape should keep literal
    input = u"\\u004"_ustr;
    result = i18nutil::processUnicodeEscapes(input);
    CPPUNIT_ASSERT_EQUAL(input, result);
}

void TestUnicodeEscape::testControlCharFiltering()
{
    // \u000A (LF) should be allowed
    OUString input(u"\\u000A"_ustr);
    OUString result = i18nutil::processUnicodeEscapes(input);
    CPPUNIT_ASSERT_EQUAL(u"\n"_ustr, result);

    // \u0009 (TAB) should be allowed
    input = u"\\u0009"_ustr;
    result = i18nutil::processUnicodeEscapes(input);
    CPPUNIT_ASSERT_EQUAL(u"\t"_ustr, result);

    // \u0000 (NULL) should be filtered
    input = u"\\u0000"_ustr;
    result = i18nutil::processUnicodeEscapes(input);
    CPPUNIT_ASSERT_EQUAL(input, result); // Should remain literal

    input = u"\\\\uXYZW"_ustr;
    result = i18nutil::processUnicodeEscapes(input);

    CPPUNIT_ASSERT_EQUAL(u"\\uXYZW"_ustr, result);
}

void TestUnicodeEscape::testSurrogatePair()
{
    // \U0001F600 (emoji) should work with surrogate pairs
    OUString result = i18nutil::processUnicodeEscapes(u"\\U0001F600");

    // U+1F600 = UTF-16: 0xD83D 0xDE00
    sal_Unicode surrogates[2] = { 0xD83D, 0xDE00 };
    OUString expected(surrogates, 2);

    CPPUNIT_ASSERT_EQUAL(expected, result);
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestUnicodeEscape);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
