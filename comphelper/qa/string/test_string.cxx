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
    s1 = u"\U0001D7FE\U0001D7F7"; // MATHEMATICAL MONOSPACE DIGIT EIGHT and ONE
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
    OString aIn("abc");
    OString aOut;

    aOut = ::comphelper::string::stripStart(aIn, 'b');
    CPPUNIT_ASSERT_EQUAL(OString("abc"), aOut);

    aOut = ::comphelper::string::stripStart(aIn, 'a');
    CPPUNIT_ASSERT_EQUAL(OString("bc"), aOut);

    aIn = "aaa";
    aOut = ::comphelper::string::stripStart(aIn, 'a');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aIn = "aba";
    aOut = ::comphelper::string::stripStart(aIn, 'a');
    CPPUNIT_ASSERT_EQUAL(OString("ba"), aOut);
}

void TestString::testStripEnd()
{
    OString aIn("abc");
    OString aOut;

    aOut = ::comphelper::string::stripEnd(aIn, 'b');
    CPPUNIT_ASSERT_EQUAL(OString("abc"), aOut);

    aOut = ::comphelper::string::stripEnd(aIn, 'c');
    CPPUNIT_ASSERT_EQUAL(OString("ab"), aOut);

    aIn = "aaa";
    aOut = ::comphelper::string::stripEnd(aIn, 'a');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aIn = "aba";
    aOut = ::comphelper::string::stripEnd(aIn, 'a');
    CPPUNIT_ASSERT_EQUAL(OString("ab"), aOut);
}

void TestString::testStrip()
{
    OString aIn("abc");
    OString aOut;

    aOut = ::comphelper::string::strip(aIn, 'b');
    CPPUNIT_ASSERT_EQUAL(OString("abc"), aOut);

    aOut = ::comphelper::string::strip(aIn, 'c');
    CPPUNIT_ASSERT_EQUAL(OString("ab"), aOut);

    aIn = "aaa";
    aOut = ::comphelper::string::strip(aIn, 'a');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aIn = "aba";
    aOut = ::comphelper::string::strip(aIn, 'a');
    CPPUNIT_ASSERT_EQUAL(OString("b"), aOut);
}

void TestString::testToken()
{
    OString aIn("10.11.12");
    OString aOut;

    aOut = aIn.getToken(-1, '.');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aOut = aIn.getToken(0, '.');
    CPPUNIT_ASSERT_EQUAL(OString("10"), aOut);

    aOut = aIn.getToken(1, '.');
    CPPUNIT_ASSERT_EQUAL(OString("11"), aOut);

    aOut = aIn.getToken(2, '.');
    CPPUNIT_ASSERT_EQUAL(OString("12"), aOut);

    aOut = aIn.getToken(3, '.');
    CPPUNIT_ASSERT(aOut.isEmpty());
}

void TestString::testTokenCount()
{
    OString aIn("10.11.12");
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
    OString aOut = ::comphelper::string::reverseString("ABC");

    CPPUNIT_ASSERT_EQUAL(OString("CBA"), aOut);
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
