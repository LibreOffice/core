/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <iterator>

#include <comphelper/string.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XCollator.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

namespace {

class TestString: public CppUnit::TestFixture
{
public:
    void testStripStart();
    void testStripEnd();
    void testStrip();
    void testToken();
    void testTokenCount();
    void testDecimalStringToNumber();
    void testIsdigitAsciiString();
    void testReverseString();
    void testReverseCodePoints();
    void testSplit();
    void testRemoveAny();

    CPPUNIT_TEST_SUITE(TestString);
    CPPUNIT_TEST(testStripStart);
    CPPUNIT_TEST(testStripEnd);
    CPPUNIT_TEST(testStrip);
    CPPUNIT_TEST(testToken);
    CPPUNIT_TEST(testTokenCount);
    CPPUNIT_TEST(testDecimalStringToNumber);
    CPPUNIT_TEST(testIsdigitAsciiString);
    CPPUNIT_TEST(testReverseString);
    CPPUNIT_TEST(testReverseCodePoints);
    CPPUNIT_TEST(testSplit);
    CPPUNIT_TEST(testRemoveAny);
    CPPUNIT_TEST_SUITE_END();
};

void TestString::testDecimalStringToNumber()
{
    OUString s1("1234");
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(1234), comphelper::string::decimalStringToNumber(s1));
    s1 += u"\u07C6";
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(12346), comphelper::string::decimalStringToNumber(s1));
    // Codepoints on 2 16bits words
    s1 = u"\U0001D7FE\U0001D7F7"_ustr; // MATHEMATICAL MONOSPACE DIGIT EIGHT and ONE
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(81), comphelper::string::decimalStringToNumber(s1));
}

void TestString::testIsdigitAsciiString()
{
    CPPUNIT_ASSERT_EQUAL(true, comphelper::string::isdigitAsciiString("1234"));

    CPPUNIT_ASSERT_EQUAL(false, comphelper::string::isdigitAsciiString("1A34"));

    CPPUNIT_ASSERT_EQUAL(true, comphelper::string::isdigitAsciiString(""));
}

void TestString::testStripStart()
{
    OString aIn("abc"_ostr);
    OString aOut;

    aOut = ::comphelper::string::stripStart(aIn, 'b');
    CPPUNIT_ASSERT_EQUAL("abc"_ostr, aOut);

    aOut = ::comphelper::string::stripStart(aIn, 'a');
    CPPUNIT_ASSERT_EQUAL("bc"_ostr, aOut);

    aIn = "aaa"_ostr;
    aOut = ::comphelper::string::stripStart(aIn, 'a');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aIn = "aba"_ostr;
    aOut = ::comphelper::string::stripStart(aIn, 'a');
    CPPUNIT_ASSERT_EQUAL("ba"_ostr, aOut);
}

void TestString::testStripEnd()
{
    OString aIn("abc"_ostr);
    OString aOut;

    aOut = ::comphelper::string::stripEnd(aIn, 'b');
    CPPUNIT_ASSERT_EQUAL("abc"_ostr, aOut);

    aOut = ::comphelper::string::stripEnd(aIn, 'c');
    CPPUNIT_ASSERT_EQUAL("ab"_ostr, aOut);

    aIn = "aaa"_ostr;
    aOut = ::comphelper::string::stripEnd(aIn, 'a');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aIn = "aba"_ostr;
    aOut = ::comphelper::string::stripEnd(aIn, 'a');
    CPPUNIT_ASSERT_EQUAL("ab"_ostr, aOut);
}

void TestString::testStrip()
{
    OString aIn("abc"_ostr);
    OString aOut;

    aOut = ::comphelper::string::strip(aIn, 'b');
    CPPUNIT_ASSERT_EQUAL("abc"_ostr, aOut);

    aOut = ::comphelper::string::strip(aIn, 'c');
    CPPUNIT_ASSERT_EQUAL("ab"_ostr, aOut);

    aIn = "aaa"_ostr;
    aOut = ::comphelper::string::strip(aIn, 'a');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aIn = "aba"_ostr;
    aOut = ::comphelper::string::strip(aIn, 'a');
    CPPUNIT_ASSERT_EQUAL("b"_ostr, aOut);
}

void TestString::testToken()
{
    OString aIn("10.11.12"_ostr);
    OString aOut;

    aOut = aIn.getToken(-1, '.');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aOut = aIn.getToken(0, '.');
    CPPUNIT_ASSERT_EQUAL("10"_ostr, aOut);

    aOut = aIn.getToken(1, '.');
    CPPUNIT_ASSERT_EQUAL("11"_ostr, aOut);

    aOut = aIn.getToken(2, '.');
    CPPUNIT_ASSERT_EQUAL("12"_ostr, aOut);

    aOut = aIn.getToken(3, '.');
    CPPUNIT_ASSERT(aOut.isEmpty());
}

void TestString::testTokenCount()
{
    OString aIn("10.11.12"_ostr);
    sal_Int32 nOut;

    nOut = ::comphelper::string::getTokenCount(aIn, '.');
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), nOut);

    nOut = ::comphelper::string::getTokenCount(aIn, 'X');
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nOut);

    nOut = ::comphelper::string::getTokenCount("", 'X');
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nOut);
}

void TestString::testReverseString()
{
    CPPUNIT_ASSERT_EQUAL(OUString(), comphelper::string::reverseString(u""));
    CPPUNIT_ASSERT_EQUAL(OUString("cba"), comphelper::string::reverseString(u"abc"));
    static sal_Unicode const rev[] = {'w', 0xDFFF, 0xDBFF, 'v', 0xDC00, 0xD800, 'u'};
    CPPUNIT_ASSERT_EQUAL(
        OUString(rev, std::size(rev)),
        comphelper::string::reverseString(u"u\U00010000v\U0010FFFFw"));
    static sal_Unicode const malformed[] = {0xDC00, 0xD800};
    CPPUNIT_ASSERT_EQUAL(
        u"\U00010000"_ustr,
        comphelper::string::reverseString(std::u16string_view(malformed, std::size(malformed))));
}

void TestString::testReverseCodePoints() {
    CPPUNIT_ASSERT_EQUAL(OUString(), comphelper::string::reverseCodePoints(u""));
    CPPUNIT_ASSERT_EQUAL(OUString("cba"), comphelper::string::reverseCodePoints(u"abc"));
    CPPUNIT_ASSERT_EQUAL(
        u"w\U0010FFFFv\U00010000u"_ustr,
        comphelper::string::reverseCodePoints(u"u\U00010000v\U0010FFFFw"));
    static sal_Unicode const malformed[] = {0xDC00, 0xD800};
    CPPUNIT_ASSERT_EQUAL(
        u"\U00010000"_ustr,
        comphelper::string::reverseCodePoints(std::u16string_view(malformed, std::size(malformed))));
}

void TestString::testSplit()
{
    std::vector<OUString> aRet = ::comphelper::string::split(u"CTRL+ALT+F1", '+');
    CPPUNIT_ASSERT_EQUAL(size_t(3), aRet.size());
    CPPUNIT_ASSERT_EQUAL(OUString("CTRL"), aRet[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("ALT"), aRet[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("F1"), aRet[2]);
}

void TestString::testRemoveAny()
{
    using namespace ::comphelper::string;
    OUString in("abcAAAbbC");
    sal_Unicode const test1 [] = { 'a', 0 };
    CPPUNIT_ASSERT_EQUAL(OUString("bcAAAbbC"), removeAny(in, test1));
    sal_Unicode const test2 [] = { 0 };
    CPPUNIT_ASSERT_EQUAL(in, removeAny(in, test2));
    sal_Unicode const test3 [] = { 'A', 0 };
    CPPUNIT_ASSERT_EQUAL(OUString("abcbbC"), removeAny(in, test3));
    sal_Unicode const test4 [] = { 'A', 'a', 0 };
    CPPUNIT_ASSERT_EQUAL(OUString("bcbbC"), removeAny(in, test4));
    sal_Unicode const test5 [] = { 'C', 0 };
    CPPUNIT_ASSERT_EQUAL(OUString("abcAAAbb"), removeAny(in, test5));
    sal_Unicode const test6 [] = { 'X', 0 };
    CPPUNIT_ASSERT_EQUAL(in, removeAny(in, test6));
    sal_Unicode const test7 [] = { 'A', 'B', 'C', 'a', 'b', 'c', 0 };
    CPPUNIT_ASSERT_EQUAL(OUString(), removeAny(in, test7));
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestString);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
