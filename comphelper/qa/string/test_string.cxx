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
    void testNatural();
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
    CPPUNIT_TEST(testNatural);
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
    OString s1("1234");
    CPPUNIT_ASSERT_EQUAL(true, comphelper::string::isdigitAsciiString(s1));

    OString s2("1A34");
    CPPUNIT_ASSERT_EQUAL(false, comphelper::string::isdigitAsciiString(s2));

    OString s3;
    CPPUNIT_ASSERT_EQUAL(true, comphelper::string::isdigitAsciiString(s3));
}

using namespace ::com::sun::star;

class testCollator : public cppu::WeakImplHelper< i18n::XCollator >
{
public:
    virtual sal_Int32 SAL_CALL compareSubstring(
        const OUString& str1, sal_Int32 off1, sal_Int32 len1,
        const OUString& str2, sal_Int32 off2, sal_Int32 len2) override
    {
        return str1.copy(off1, len1).compareTo(str2.copy(off2, len2));
    }
    virtual sal_Int32 SAL_CALL compareString(
        const OUString& str1,
        const OUString& str2) override
    {
        return str1.compareTo(str2);
    }
    virtual sal_Int32 SAL_CALL loadDefaultCollator(const lang::Locale&, sal_Int32) override {return 0;}
    virtual sal_Int32 SAL_CALL loadCollatorAlgorithm(const OUString&,
        const lang::Locale&, sal_Int32) override {return 0;}
    virtual void SAL_CALL loadCollatorAlgorithmWithEndUserOption(const OUString&,
        const lang::Locale&, const uno::Sequence< sal_Int32 >&) override {}
    virtual uno::Sequence< OUString > SAL_CALL listCollatorAlgorithms(const lang::Locale&) override
    {
        return uno::Sequence< OUString >();
    }
    virtual uno::Sequence< sal_Int32 > SAL_CALL listCollatorOptions(const OUString&) override
    {
        return uno::Sequence< sal_Int32 >();
    }
};

#define IS_DIGIT(CHAR) (((CHAR) >= 48) && ((CHAR <= 57)))

class testBreakIterator : public cppu::WeakImplHelper< i18n::XBreakIterator >
{
public:
    virtual sal_Int32 SAL_CALL nextCharacters( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16, sal_Int32, sal_Int32& ) override {return -1;}
    virtual sal_Int32 SAL_CALL previousCharacters( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16, sal_Int32, sal_Int32& ) override {return -1;}

    virtual i18n::Boundary SAL_CALL previousWord( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16) override
        { return i18n::Boundary(); }
    virtual i18n::Boundary SAL_CALL nextWord( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16) override
        { return i18n::Boundary(); }
    virtual i18n::Boundary SAL_CALL getWordBoundary( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16, sal_Bool ) override
        { return i18n::Boundary(); }

    virtual sal_Bool SAL_CALL isBeginWord( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16 ) override
        { return false; }
    virtual sal_Bool SAL_CALL isEndWord( const OUString&, sal_Int32,
        const lang::Locale& , sal_Int16 ) override
        { return false; }
    virtual sal_Int16 SAL_CALL getWordType( const OUString&, sal_Int32,
        const lang::Locale& ) override
        { return 0; }

    virtual sal_Int32 SAL_CALL beginOfSentence( const OUString&, sal_Int32,
        const lang::Locale& ) override
        { return 0; }
    virtual sal_Int32 SAL_CALL endOfSentence( const OUString& rText, sal_Int32,
        const lang::Locale& ) override
        { return rText.getLength(); }

    virtual i18n::LineBreakResults SAL_CALL getLineBreak( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int32,
        const i18n::LineBreakHyphenationOptions&,
        const i18n::LineBreakUserOptions&) override
    {
        return i18n::LineBreakResults();
    }

    virtual sal_Int16 SAL_CALL getScriptType( const OUString&, sal_Int32 ) override { return -1; }
    virtual sal_Int32 SAL_CALL beginOfScript( const OUString&, sal_Int32,
        sal_Int16 ) override { return -1; }
    virtual sal_Int32 SAL_CALL endOfScript( const OUString&, sal_Int32,
        sal_Int16 ) override { return -1; }
    virtual sal_Int32 SAL_CALL previousScript( const OUString&, sal_Int32,
        sal_Int16 ) override { return -1; }
    virtual sal_Int32 SAL_CALL nextScript( const OUString&, sal_Int32,
        sal_Int16 ) override { return -1; }

    virtual sal_Int32 SAL_CALL beginOfCharBlock( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16 ) override { return -1; }
    virtual sal_Int32 SAL_CALL endOfCharBlock( const OUString& rText, sal_Int32 nStartPos,
        const lang::Locale&, sal_Int16 CharType ) override
    {
        const sal_Unicode *pStr = rText.getStr()+nStartPos;
        for (sal_Int32 nI = nStartPos; nI < rText.getLength(); ++nI)
        {
            if (CharType == i18n::CharType::DECIMAL_DIGIT_NUMBER && !IS_DIGIT(*pStr))
                return nI;
            else if (CharType != i18n::CharType::DECIMAL_DIGIT_NUMBER && IS_DIGIT(*pStr))
                return nI;
            ++pStr;
        }
        return -1;
    }
    virtual sal_Int32 SAL_CALL previousCharBlock( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16 ) override { return -1; }
    virtual sal_Int32 SAL_CALL nextCharBlock( const OUString& rText, sal_Int32 nStartPos,
        const lang::Locale&, sal_Int16 CharType ) override
    {
        const sal_Unicode *pStr = rText.getStr()+nStartPos;
        for (sal_Int32 nI = nStartPos; nI < rText.getLength(); ++nI)
        {
            if (CharType == i18n::CharType::DECIMAL_DIGIT_NUMBER && IS_DIGIT(*pStr))
                return nI;
            else if (CharType != i18n::CharType::DECIMAL_DIGIT_NUMBER && !IS_DIGIT(*pStr))
                return nI;
            ++pStr;
        }
        return -1;
    }
};

void TestString::testNatural()
{
    using namespace comphelper::string;

    uno::Reference< i18n::XCollator > xCollator(new testCollator);
    uno::Reference< i18n::XBreakIterator > xBI(new testBreakIterator);

// --- Some generic tests to ensure we do not alter original behavior
// outside what we want
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0), compareNatural("ABC", "ABC", xCollator, xBI, lang::Locale())
    );
    // Case sensitivity
    CPPUNIT_ASSERT(
        compareNatural("ABC", "abc", xCollator, xBI, lang::Locale()) < 0
    );
    // Reverse
    CPPUNIT_ASSERT(
        compareNatural("abc", "ABC", xCollator, xBI, lang::Locale()) > 0
    );
    // First shorter
    CPPUNIT_ASSERT(
        compareNatural("alongstring", "alongerstring", xCollator, xBI, lang::Locale()) > 0
    );
    // Second shorter
    CPPUNIT_ASSERT(
        compareNatural("alongerstring", "alongstring", xCollator, xBI, lang::Locale()) < 0
    );
// -- Here we go on natural order, each one is followed by classic compare and the reverse comparison
    // That's why we originally made the patch
    CPPUNIT_ASSERT(
        compareNatural("Heading 9", "Heading 10", xCollator, xBI, lang::Locale()) < 0
    );
    // Original behavior
    CPPUNIT_ASSERT(
        OUString("Heading 9").compareTo("Heading 10") > 0
    );
    CPPUNIT_ASSERT(
        compareNatural("Heading 10", "Heading 9", xCollator, xBI, lang::Locale()) > 0
    );
    // Harder
    CPPUNIT_ASSERT(
        compareNatural("July, the 4th", "July, the 10th", xCollator, xBI, lang::Locale()) < 0
    );
    CPPUNIT_ASSERT(
        OUString("July, the 4th").compareTo("July, the 10th") > 0
    );
    CPPUNIT_ASSERT(
        compareNatural("July, the 10th", "July, the 4th", xCollator, xBI, lang::Locale()) > 0
    );
    // Hardest
    CPPUNIT_ASSERT(
        compareNatural("abc08", "abc010", xCollator, xBI, lang::Locale()) < 0
    );
    CPPUNIT_ASSERT(
        OUString("abc08").compareTo("abc010") > 0
    );
    CPPUNIT_ASSERT(
        compareNatural("abc010", "abc08", xCollator, xBI, lang::Locale()) > 0
    );
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0), compareNatural("apple10apple", "apple10apple", xCollator, xBI, lang::Locale())
    );
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

    nOut = ::comphelper::string::getTokenCount(OString(), 'X');
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nOut);
}

void TestString::testReverseString()
{
    OString aIn("ABC");
    OString aOut = ::comphelper::string::reverseString(aIn);

    CPPUNIT_ASSERT_EQUAL(OString("CBA"), aOut);
}

void TestString::testSplit()
{
    std::vector<OUString> aRet = ::comphelper::string::split("CTRL+ALT+F1", '+');
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
