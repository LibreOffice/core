/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Caolán McNamara <caolanm@redhat.com>
 *
 * Contributor(s):
 *   Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "sal/config.h"
#include "sal/precppunit.hxx"

#ifdef IOS
#define CPPUNIT_PLUGIN_EXPORTED_NAME cppunitTest_i18npool_breakiterator
#endif

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <unotest/bootstrapfixturebase.hxx>

#include <unicode/uvernum.h>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <string.h>

using namespace ::com::sun::star;

class TestBreakIterator : public test::BootstrapFixtureBase
{
public:
    virtual void setUp();
    virtual void tearDown();

    void testLineBreaking();
    void testGraphemeIteration();
    void testWeak();
    void testAsian();
    void testThai();
#if TODO
    void testNorthernThai();
#endif
#if (U_ICU_VERSION_MAJOR_NUM > 4)
    void testKhmer();
#endif

    CPPUNIT_TEST_SUITE(TestBreakIterator);
    CPPUNIT_TEST(testLineBreaking);
    CPPUNIT_TEST(testGraphemeIteration);
    CPPUNIT_TEST(testWeak);
    CPPUNIT_TEST(testAsian);
    CPPUNIT_TEST(testThai);
#if TODO
    CPPUNIT_TEST(testNorthernThai);
#endif
#if (U_ICU_VERSION_MAJOR_NUM > 4)
    CPPUNIT_TEST(testKhmer);
#endif
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference<i18n::XBreakIterator> m_xBreak;
};

void TestBreakIterator::testLineBreaking()
{
    i18n::LineBreakHyphenationOptions aHyphOptions;
    i18n::LineBreakUserOptions aUserOptions;
    lang::Locale aLocale;

    //See https://bugs.freedesktop.org/show_bug.cgi?id=31271
    {
        ::rtl::OUString aTest(RTL_CONSTASCII_USTRINGPARAM("(some text here)"));

        aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("en"));
        aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("US"));

        {
            //Here we want the line break to leave text here) on the next line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, strlen("(some tex"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_MESSAGE("Expected a break at the the start of the word", aResult.breakIndex == 6);
        }

        {
            //Here we want the line break to leave "here)" on the next line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, strlen("(some text here"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_MESSAGE("Expected a break at the the start of the word", aResult.breakIndex == 11);
        }
    }

    //See https://bugs.freedesktop.org/show_bug.cgi?id=49849
    {
        const sal_Unicode HEBREW1[] = { 0x05DE, 0x05D9, 0x05DC, 0x05D9, 0x5DD };
        ::rtl::OUString aWord(HEBREW1, SAL_N_ELEMENTS(HEBREW1));
        ::rtl::OUString aTest(rtl::OUStringBuffer(aWord).append(' ').append(aWord).makeStringAndClear());

        aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("he"));
        aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IL"));

        {
            //Here we want the line break to happen at the whitespace
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, aTest.getLength()-1, aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_MESSAGE("Expected a break at the the start of the word", aResult.breakIndex == aWord.getLength()+1);
        }
    }
}

//See http://qa.openoffice.org/issues/show_bug.cgi?id=111152
//See https://bugs.freedesktop.org/show_bug.cgi?id=40292
void TestBreakIterator::testGraphemeIteration()
{
    lang::Locale aLocale;
    aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bn"));
    aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IN"));

    {
        const sal_Unicode BA_HALANT_LA[] = { 0x09AC, 0x09CD, 0x09AF };
        ::rtl::OUString aTest(BA_HALANT_LA, SAL_N_ELEMENTS(BA_HALANT_LA));

        sal_Int32 nDone=0;
        sal_Int32 nPos;
        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == SAL_N_ELEMENTS(BA_HALANT_LA));
        nPos = m_xBreak->previousCharacters(aTest, SAL_N_ELEMENTS(BA_HALANT_LA), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == 0);
    }

    {
        const sal_Unicode HA_HALANT_NA_VOWELSIGNI[] = { 0x09B9, 0x09CD, 0x09A3, 0x09BF };
        ::rtl::OUString aTest(HA_HALANT_NA_VOWELSIGNI, SAL_N_ELEMENTS(HA_HALANT_NA_VOWELSIGNI));

        sal_Int32 nDone=0;
        sal_Int32 nPos;
        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == SAL_N_ELEMENTS(HA_HALANT_NA_VOWELSIGNI));
        nPos = m_xBreak->previousCharacters(aTest, SAL_N_ELEMENTS(HA_HALANT_NA_VOWELSIGNI), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == 0);
    }

    {
        const sal_Unicode TA_HALANT_MA_HALANT_YA  [] = { 0x09A4, 0x09CD, 0x09AE, 0x09CD, 0x09AF };
        ::rtl::OUString aTest(TA_HALANT_MA_HALANT_YA, SAL_N_ELEMENTS(TA_HALANT_MA_HALANT_YA));

        sal_Int32 nDone=0;
        sal_Int32 nPos;
        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == SAL_N_ELEMENTS(TA_HALANT_MA_HALANT_YA));
        nPos = m_xBreak->previousCharacters(aTest, SAL_N_ELEMENTS(TA_HALANT_MA_HALANT_YA), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == 0);
    }

    aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ta"));
    aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IN"));

    {
        const sal_Unicode KA_VIRAMA_SSA[] = { 0x0B95, 0x0BCD, 0x0BB7 };
        ::rtl::OUString aTest(KA_VIRAMA_SSA, SAL_N_ELEMENTS(KA_VIRAMA_SSA));

        sal_Int32 nDone=0;
        sal_Int32 nPos = 0;

        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == SAL_N_ELEMENTS(KA_VIRAMA_SSA));
        nPos = m_xBreak->previousCharacters(aTest, SAL_N_ELEMENTS(KA_VIRAMA_SSA), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == 0);
    }

    {
        const sal_Unicode CA_VOWELSIGNI_TA_VIRAMA_TA_VOWELSIGNI_RA_VOWELSIGNAI[] =
            { 0x0B9A, 0x0BBF, 0x0BA4, 0x0BCD, 0x0BA4, 0x0BBF, 0x0BB0, 0x0BC8 };
        ::rtl::OUString aTest(CA_VOWELSIGNI_TA_VIRAMA_TA_VOWELSIGNI_RA_VOWELSIGNAI,
            SAL_N_ELEMENTS(CA_VOWELSIGNI_TA_VIRAMA_TA_VOWELSIGNI_RA_VOWELSIGNAI));

        sal_Int32 nDone=0;
        sal_Int32 nPos=0;

        for (sal_Int32 i = 0; i < 4; ++i)
        {
            sal_Int32 nOldPos = nPos;
            nPos = m_xBreak->nextCharacters(aTest, nPos, aLocale,
                i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
            CPPUNIT_ASSERT_MESSAGE("Should skip 2 units", nPos == nOldPos+2);
        }

        for (sal_Int32 i = 0; i < 4; ++i)
        {
            sal_Int32 nOldPos = nPos;
            nPos = m_xBreak->previousCharacters(aTest, nPos, aLocale,
                i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
            CPPUNIT_ASSERT_MESSAGE("Should skip 2 units", nPos == nOldPos-2);
        }
    }

    {
        const sal_Unicode ALEF_QAMATS [] = { 0x05D0, 0x05B8 };
        ::rtl::OUString aText(ALEF_QAMATS, SAL_N_ELEMENTS(ALEF_QAMATS));

        sal_Int32 nGraphemeCount = 0;

        sal_Int32 nCurPos = 0;
        while (nCurPos < aText.getLength())
        {
            sal_Int32 nCount2 = 1;
            nCurPos = m_xBreak->nextCharacters(aText, nCurPos, lang::Locale(),
                i18n::CharacterIteratorMode::SKIPCELL, nCount2, nCount2);
            ++nGraphemeCount;
        }

        CPPUNIT_ASSERT_MESSAGE("Should be considered 1 grapheme", nGraphemeCount == 1);
    }
}

//A test to ensure that certain ranges and codepoints that are categorized as
//weak remain as weak, so that existing docs that depend on this don't silently
//change font for those weak chars
void TestBreakIterator::testWeak()
{
    lang::Locale aLocale;
    aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("en"));
    aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("US"));

    {
        const sal_Unicode WEAKS[] =
        {
            0x0001, 0x0002,
            0x0020, 0x00A0,
            0x2150, 0x215F, //Number Forms, fractions
            0x2160, 0x2180, //Number Forms, roman numerals
            0x2200, 0x22FF, //Mathematical Operators
            0x27C0, 0x27EF, //Miscellaneous Mathematical Symbols-A
            0x2980, 0x29FF, //Miscellaneous Mathematical Symbols-B
            0x2A00, 0x2AFF, //Supplemental Mathematical Operators
            0x2100, 0x214F, //Letterlike Symbols
            0x2308, 0x230B, //Miscellaneous technical
            0x25A0, 0x25FF, //Geometric Shapes
            0x2B30, 0x2B4C  //Miscellaneous Symbols and Arrows
        };
        ::rtl::OUString aWeaks(WEAKS, SAL_N_ELEMENTS(WEAKS));

        for (sal_Int32 i = 0; i < aWeaks.getLength(); ++i)
        {
            sal_Int16 nScript = m_xBreak->getScriptType(aWeaks, i);
            rtl::OStringBuffer aMsg;
            aMsg.append(RTL_CONSTASCII_STRINGPARAM("Char 0x"));
            aMsg.append(static_cast<sal_Int32>(aWeaks.getStr()[i]), 16);
            aMsg.append(RTL_CONSTASCII_STRINGPARAM(" should have been weak"));
            CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(),
                nScript == i18n::ScriptType::WEAK);
        }
    }
}

//A test to ensure that certain ranges and codepoints that are categorized as
//asian remain as asian, so that existing docs that depend on this don't silently
//change font for those asian chars.
//See https://bugs.freedesktop.org/show_bug.cgi?id=38095
void TestBreakIterator::testAsian()
{
    lang::Locale aLocale;
    aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("en"));
    aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("US"));

    {
        const sal_Unicode ASIANS[] =
        {
            //some typical CJK chars
            0x4E00, 0x62FF,
            //The full HalfWidth and FullWidth block has historically been
            //designated as taking the CJK font :-(
            //HalfWidth and FullWidth forms of ASCII 0-9, categorized under
            //UAX24 as "Common" i.e. by that logic WEAK
            0xFF10, 0xFF19,
            //HalfWidth and FullWidth forms of ASCII A-z, categorized under
            //UAX25 as "Latin", i.e. by that logic LATIN
            0xFF21, 0xFF5A
        };
        ::rtl::OUString aAsians(ASIANS, SAL_N_ELEMENTS(ASIANS));

        for (sal_Int32 i = 0; i < aAsians.getLength(); ++i)
        {
            sal_Int16 nScript = m_xBreak->getScriptType(aAsians, i);
            rtl::OStringBuffer aMsg;
            aMsg.append(RTL_CONSTASCII_STRINGPARAM("Char 0x"));
            aMsg.append(static_cast<sal_Int32>(aAsians.getStr()[i]), 16);
            aMsg.append(RTL_CONSTASCII_STRINGPARAM(" should have been asian"));
            CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(),
                nScript == i18n::ScriptType::ASIAN);
        }
    }
}

//A test to ensure that our thai word boundary detection is useful
//http://lists.freedesktop.org/archives/libreoffice/2012-February/025959.html
void TestBreakIterator::testThai()
{
    lang::Locale aLocale;
    aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("th"));
    aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TH"));

    const sal_Unicode THAI1[] = { 0x0E01, 0x0E38, 0x0E2B, 0x0E25, 0x0E32, 0x0E1A };
    ::rtl::OUString aTest(THAI1, SAL_N_ELEMENTS(THAI1));
    i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);
    CPPUNIT_ASSERT_MESSAGE("Should skip full word",
        aBounds.startPos == 0 && aBounds.endPos == aTest.getLength());
}

#if TODO
void TestBreakIterator::testNorthernThai()
{
    lang::Locale aLocale;
    aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("nod"));
    aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TH"));

    const sal_Unicode NORTHERN_THAI1[] = { 0x0E01, 0x0E38, 0x0E4A, 0x0E2B, 0x0E25, 0x0E32, 0x0E1A };
    ::rtl::OUString aTest(NORTHERN_THAI1, SAL_N_ELEMENTS(NORTHERN_THAI1));
    i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);
    CPPUNIT_ASSERT_MESSAGE("Should skip full word",
        aBounds.startPos == 0 && aBounds.endPos == aTest.getLength());
}
#endif

#if (U_ICU_VERSION_MAJOR_NUM > 4)
//A test to ensure that our khmer word boundary detection is useful
//https://bugs.freedesktop.org/show_bug.cgi?id=52020
//
//icu doesn't have the Khmer word boundary dictionaries in <= 4.0.0 but does in
//the current 49.x.y . Not sure which version first had them introduced.
void TestBreakIterator::testKhmer()
{
    lang::Locale aLocale;
    aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("km"));
    aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("KH"));

    const sal_Unicode KHMER1[] = { 0x17B2, 0x17D2, 0x1799, 0x1782, 0x17C1 };

    ::rtl::OUString aTest(KHMER1, SAL_N_ELEMENTS(KHMER1));
    i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);

    CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 3);

    aBounds = m_xBreak->getWordBoundary(aTest, aBounds.endPos, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);

    CPPUNIT_ASSERT(aBounds.startPos == 3 && aBounds.endPos == 5);
}
#endif

void TestBreakIterator::setUp()
{
    BootstrapFixtureBase::setUp();
    m_xBreak = uno::Reference< i18n::XBreakIterator >(m_xSFactory->createInstance(
        "com.sun.star.i18n.BreakIterator"), uno::UNO_QUERY_THROW);
}

void TestBreakIterator::tearDown()
{
    BootstrapFixtureBase::tearDown();
    m_xBreak.clear();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestBreakIterator);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
