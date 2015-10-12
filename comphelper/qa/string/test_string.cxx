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

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

namespace {

class TestString: public CppUnit::TestFixture
{
public:
    void testNatural();
    void testRemove();
    void testStripStart();
    void testStripEnd();
    void testStrip();
    void testToken();
    void testTokenCount();
    void testDecimalStringToNumber();
    void testIsdigitAsciiString();
    void testReverseString();
    void testEqualsString();
    void testCompareVersionStrings();

    CPPUNIT_TEST_SUITE(TestString);
    CPPUNIT_TEST(testNatural);
    CPPUNIT_TEST(testRemove);
    CPPUNIT_TEST(testStripStart);
    CPPUNIT_TEST(testStripEnd);
    CPPUNIT_TEST(testStrip);
    CPPUNIT_TEST(testToken);
    CPPUNIT_TEST(testTokenCount);
    CPPUNIT_TEST(testDecimalStringToNumber);
    CPPUNIT_TEST(testIsdigitAsciiString);
    CPPUNIT_TEST(testReverseString);
    CPPUNIT_TEST(testEqualsString);
    CPPUNIT_TEST(testCompareVersionStrings);
    CPPUNIT_TEST_SUITE_END();
};

void TestString::testDecimalStringToNumber()
{
    OUString s1("1234");
    CPPUNIT_ASSERT_EQUAL((sal_uInt32)1234, comphelper::string::decimalStringToNumber(s1));
    s1 += OUString(static_cast<sal_Unicode>(0x07C6));
    CPPUNIT_ASSERT_EQUAL((sal_uInt32)12346, comphelper::string::decimalStringToNumber(s1));
    // Codepoints on 2 16bits words
    sal_uInt32 utf16String[] = { 0x1D7FE /* 8 */, 0x1D7F7 /* 1 */};
    s1 = OUString(utf16String, 2);
    CPPUNIT_ASSERT_EQUAL((sal_uInt32)81, comphelper::string::decimalStringToNumber(s1));
}

void TestString::testIsdigitAsciiString()
{
    OString s1("1234");
    CPPUNIT_ASSERT_EQUAL(comphelper::string::isdigitAsciiString(s1), true);

    OString s2("1A34");
    CPPUNIT_ASSERT_EQUAL(comphelper::string::isdigitAsciiString(s2), false);

    OString s3;
    CPPUNIT_ASSERT_EQUAL(comphelper::string::isdigitAsciiString(s3), true);
}

using namespace ::com::sun::star;

class testCollator : public cppu::WeakImplHelper< i18n::XCollator >
{
public:
    virtual sal_Int32 SAL_CALL compareSubstring(
        const OUString& str1, sal_Int32 off1, sal_Int32 len1,
        const OUString& str2, sal_Int32 off2, sal_Int32 len2) throw(uno::RuntimeException, std::exception) override
    {
        return str1.copy(off1, len1).compareTo(str2.copy(off2, len2));
    }
    virtual sal_Int32 SAL_CALL compareString(
        const OUString& str1,
        const OUString& str2) throw(uno::RuntimeException, std::exception) override
    {
        return str1.compareTo(str2);
    }
    virtual sal_Int32 SAL_CALL loadDefaultCollator(const lang::Locale&, sal_Int32)
        throw(uno::RuntimeException, std::exception) override {return 0;}
    virtual sal_Int32 SAL_CALL loadCollatorAlgorithm(const OUString&,
        const lang::Locale&, sal_Int32) throw(uno::RuntimeException, std::exception) override {return 0;}
    virtual void SAL_CALL loadCollatorAlgorithmWithEndUserOption(const OUString&,
        const lang::Locale&, const uno::Sequence< sal_Int32 >&) throw(uno::RuntimeException, std::exception) override {}
    virtual uno::Sequence< OUString > SAL_CALL listCollatorAlgorithms(const lang::Locale&)
        throw(uno::RuntimeException, std::exception) override
    {
        return uno::Sequence< OUString >();
    }
    virtual uno::Sequence< sal_Int32 > SAL_CALL listCollatorOptions(const OUString&)
        throw(uno::RuntimeException, std::exception) override
    {
        return uno::Sequence< sal_Int32 >();
    }
};

#define IS_DIGIT(CHAR) (((CHAR) >= 48) && ((CHAR <= 57)))

class testBreakIterator : public cppu::WeakImplHelper< i18n::XBreakIterator >
{
public:
    virtual sal_Int32 SAL_CALL nextCharacters( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16, sal_Int32, sal_Int32& )
            throw(uno::RuntimeException, std::exception) override {return -1;}
    virtual sal_Int32 SAL_CALL previousCharacters( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16, sal_Int32, sal_Int32& )
            throw(uno::RuntimeException, std::exception) override {return -1;}

    virtual i18n::Boundary SAL_CALL previousWord( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16) throw(uno::RuntimeException, std::exception) override
        { return i18n::Boundary(); }
    virtual i18n::Boundary SAL_CALL nextWord( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16) throw(uno::RuntimeException, std::exception) override
        { return i18n::Boundary(); }
    virtual i18n::Boundary SAL_CALL getWordBoundary( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16, sal_Bool )
        throw(uno::RuntimeException, std::exception) override
        { return i18n::Boundary(); }

    virtual sal_Bool SAL_CALL isBeginWord( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16 ) throw(uno::RuntimeException, std::exception) override
        { return false; }
    virtual sal_Bool SAL_CALL isEndWord( const OUString&, sal_Int32,
        const lang::Locale& , sal_Int16 ) throw(uno::RuntimeException, std::exception) override
        { return false; }
    virtual sal_Int16 SAL_CALL getWordType( const OUString&, sal_Int32,
        const lang::Locale& ) throw(uno::RuntimeException, std::exception) override
        { return 0; }

    virtual sal_Int32 SAL_CALL beginOfSentence( const OUString&, sal_Int32,
        const lang::Locale& ) throw(uno::RuntimeException, std::exception) override
        { return 0; }
    virtual sal_Int32 SAL_CALL endOfSentence( const OUString& rText, sal_Int32,
        const lang::Locale& ) throw(uno::RuntimeException, std::exception) override
        { return rText.getLength(); }

    virtual i18n::LineBreakResults SAL_CALL getLineBreak( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int32,
        const i18n::LineBreakHyphenationOptions&,
        const i18n::LineBreakUserOptions&)
        throw(uno::RuntimeException, std::exception) override
    {
        return i18n::LineBreakResults();
    }

    virtual sal_Int16 SAL_CALL getScriptType( const OUString&, sal_Int32 )
        throw(uno::RuntimeException, std::exception) override { return -1; }
    virtual sal_Int32 SAL_CALL beginOfScript( const OUString&, sal_Int32,
        sal_Int16 ) throw(uno::RuntimeException, std::exception) override { return -1; }
    virtual sal_Int32 SAL_CALL endOfScript( const OUString&, sal_Int32,
        sal_Int16 ) throw(uno::RuntimeException, std::exception) override { return -1; }
    virtual sal_Int32 SAL_CALL previousScript( const OUString&, sal_Int32,
        sal_Int16 ) throw(uno::RuntimeException, std::exception) override { return -1; }
    virtual sal_Int32 SAL_CALL nextScript( const OUString&, sal_Int32,
        sal_Int16 ) throw(uno::RuntimeException, std::exception) override { return -1; }

    virtual sal_Int32 SAL_CALL beginOfCharBlock( const OUString&, sal_Int32,
        const lang::Locale&, sal_Int16 ) throw(uno::RuntimeException, std::exception) override { return -1; }
    virtual sal_Int32 SAL_CALL endOfCharBlock( const OUString& rText, sal_Int32 nStartPos,
        const lang::Locale&, sal_Int16 CharType ) throw(uno::RuntimeException, std::exception) override
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
        const lang::Locale&, sal_Int16 ) throw(uno::RuntimeException, std::exception) override { return -1; }
    virtual sal_Int32 SAL_CALL nextCharBlock( const OUString& rText, sal_Int32 nStartPos,
        const lang::Locale&, sal_Int16 CharType ) throw(uno::RuntimeException, std::exception) override
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
    CPPUNIT_ASSERT(
        compareNatural("ABC", "ABC", xCollator, xBI, lang::Locale()) == 0
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
    CPPUNIT_ASSERT(
        compareNatural("apple10apple", "apple10apple", xCollator, xBI, lang::Locale()) == 0
    );
}

void TestString::testRemove()
{
    OString aIn("abc");
    OString aOut;

    aOut = ::comphelper::string::remove(aIn, 'b');
    CPPUNIT_ASSERT(aOut == "ac");

    aIn = "aaa";

    aOut = ::comphelper::string::remove(aIn, 'a');
    CPPUNIT_ASSERT(aOut.isEmpty());
}

void TestString::testStripStart()
{
    OString aIn("abc");
    OString aOut;

    aOut = ::comphelper::string::stripStart(aIn, 'b');
    CPPUNIT_ASSERT(aOut == "abc");

    aOut = ::comphelper::string::stripStart(aIn, 'a');
    CPPUNIT_ASSERT(aOut == "bc");

    aIn = "aaa";
    aOut = ::comphelper::string::stripStart(aIn, 'a');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aIn = "aba";
    aOut = ::comphelper::string::stripStart(aIn, 'a');
    CPPUNIT_ASSERT(aOut == "ba");
}

void TestString::testStripEnd()
{
    OString aIn("abc");
    OString aOut;

    aOut = ::comphelper::string::stripEnd(aIn, 'b');
    CPPUNIT_ASSERT(aOut == "abc");

    aOut = ::comphelper::string::stripEnd(aIn, 'c');
    CPPUNIT_ASSERT(aOut == "ab");

    aIn = "aaa";
    aOut = ::comphelper::string::stripEnd(aIn, 'a');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aIn = "aba";
    aOut = ::comphelper::string::stripEnd(aIn, 'a');
    CPPUNIT_ASSERT(aOut == "ab");
}

void TestString::testStrip()
{
    OString aIn("abc");
    OString aOut;

    aOut = ::comphelper::string::strip(aIn, 'b');
    CPPUNIT_ASSERT(aOut == "abc");

    aOut = ::comphelper::string::strip(aIn, 'c');
    CPPUNIT_ASSERT(aOut == "ab");

    aIn = "aaa";
    aOut = ::comphelper::string::strip(aIn, 'a');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aIn = "aba";
    aOut = ::comphelper::string::strip(aIn, 'a');
    CPPUNIT_ASSERT(aOut == "b");
}

void TestString::testToken()
{
    OString aIn("10.11.12");
    OString aOut;

    aOut = aIn.getToken(-1, '.');
    CPPUNIT_ASSERT(aOut.isEmpty());

    aOut = aIn.getToken(0, '.');
    CPPUNIT_ASSERT(aOut == "10");

    aOut = aIn.getToken(1, '.');
    CPPUNIT_ASSERT(aOut == "11");

    aOut = aIn.getToken(2, '.');
    CPPUNIT_ASSERT(aOut == "12");

    aOut = aIn.getToken(3, '.');
    CPPUNIT_ASSERT(aOut.isEmpty());
}

void TestString::testTokenCount()
{
    OString aIn("10.11.12");
    sal_Int32 nOut;

    nOut = ::comphelper::string::getTokenCount(aIn, '.');
    CPPUNIT_ASSERT(nOut == 3);

    nOut = ::comphelper::string::getTokenCount(aIn, 'X');
    CPPUNIT_ASSERT(nOut == 1);

    nOut = ::comphelper::string::getTokenCount(OString(), 'X');
    CPPUNIT_ASSERT(nOut == 0);
}

void TestString::testReverseString()
{
    OString aIn("ABC");
    OString aOut = ::comphelper::string::reverseString(aIn);

    CPPUNIT_ASSERT(aOut == "CBA");
}

void TestString::testEqualsString()
{
    OString aIn("A");
    CPPUNIT_ASSERT(::comphelper::string::equals(aIn, 'A'));
    CPPUNIT_ASSERT(!::comphelper::string::equals(aIn, 'B'));
    aIn = OString("AA");
    CPPUNIT_ASSERT(!::comphelper::string::equals(aIn, 'A'));
    aIn.clear();
    CPPUNIT_ASSERT(!::comphelper::string::equals(aIn, 'A'));
}

int sign(int n)
{
    if (n == 0)
        return 0;
    if (n < 0)
        return -1;
    else
        return 1;
}

void TestString::testCompareVersionStrings()
{
#ifdef TEST
#error TEST already defined
#endif
#define TEST(a,b,result) \
    CPPUNIT_ASSERT(sign(::comphelper::string::compareVersionStrings(a, b)) == result); \
    if ( result != 0 ) \
        CPPUNIT_ASSERT(sign(::comphelper::string::compareVersionStrings(b, a)) == -(result))

    TEST("", "", 0);
    TEST("", "0", -1);
    TEST("", "a", -1);
    TEST("0", "1", -1);
    TEST("1", "2", -1);
    TEST("2", "10", -1);
    TEST("01", "1", -1);
    TEST("01", "001", 1);
    TEST("1.00", "1", 1);
    TEST("1.2", "1", 1);
    TEST("1.01", "1.1", -1);
    TEST("1.001", "1.1", -1);
    TEST("1.001", "1.010", -1);
    TEST("1.2.a", "1.2.b", -1);
    TEST("1.2.3 (foo,bar)", "1.2.9", -1);
    TEST("1.2.3 (foo,bar)", "1.2.4 (foo,bar)", -1);
    TEST("1.2.3 (foo,bar)", "1.2.3 (foo)", 1); // Neither ordering makes any more sense than the other here, as long as they compare unequal
    TEST("1.2.3 (foo,bar)", "1.2.2 (foo,bar)", 1);

#undef TEST
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestString);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
