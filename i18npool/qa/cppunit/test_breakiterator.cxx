/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <unotest/bootstrapfixturebase.hxx>

#include <unicode/uversion.h>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <string.h>

#include <stack>

using namespace ::com::sun::star;

class TestBreakIterator : public test::BootstrapFixtureBase
{
public:
    virtual void setUp();
    virtual void tearDown();

    void testLineBreaking();
    void testWordBoundaries();
    void testGraphemeIteration();
    void testWeak();
    void testAsian();
    void testThai();
#ifdef TODO
    void testNorthernThai();
#endif
    void testKhmer();
    void testJapanese();

    CPPUNIT_TEST_SUITE(TestBreakIterator);
    CPPUNIT_TEST(testLineBreaking);
    CPPUNIT_TEST(testGraphemeIteration);
    CPPUNIT_TEST(testWeak);
    CPPUNIT_TEST(testAsian);
    CPPUNIT_TEST(testThai);
#ifdef TODO
    CPPUNIT_TEST(testNorthernThai);
#endif

    CPPUNIT_TEST(testWordBoundaries);
#if (U_ICU_VERSION_MAJOR_NUM > 4)
    CPPUNIT_TEST(testKhmer);
#endif
    CPPUNIT_TEST(testJapanese);
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
        OUString aTest("(some text here)");

        aLocale.Language = OUString("en");
        aLocale.Country = OUString("US");

        {
            //Here we want the line break to leave text here) on the next line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, strlen("(some tex"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_MESSAGE("Expected a break at the start of the word", aResult.breakIndex == 6);
        }

        {
            //Here we want the line break to leave "here)" on the next line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, strlen("(some text here"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_MESSAGE("Expected a break at the start of the word", aResult.breakIndex == 11);
        }
    }

    //See https://bugs.freedesktop.org/show_bug.cgi?id=49849
    {
        const sal_Unicode HEBREW1[] = { 0x05DE, 0x05D9, 0x05DC, 0x05D9, 0x5DD };
        OUString aWord(HEBREW1, SAL_N_ELEMENTS(HEBREW1));
        OUString aTest(OUStringBuffer(aWord).append(' ').append(aWord).makeStringAndClear());

        aLocale.Language = OUString("he");
        aLocale.Country = OUString("IL");

        {
            //Here we want the line break to happen at the whitespace
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, aTest.getLength()-1, aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_MESSAGE("Expected a break at the start of the word", aResult.breakIndex == aWord.getLength()+1);
        }
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=17155
    {
        OUString aTest("foo /bar/baz");

        aLocale.Language = OUString("en");
        aLocale.Country = OUString("US");

        {
            //Here we want the line break to leave /bar/ba clumped together on the next line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, strlen("foo /bar/ba"), aLocale, 0,
                aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_MESSAGE("Expected a break at the first slash", aResult.breakIndex == 4);
        }
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=19716
    {
        OUString aTest("aaa]aaa");

        aLocale.Language = OUString("en");
        aLocale.Country = OUString("US");

        {
            //Here we want the line break to move the whole lot to the next line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, aTest.getLength()-2, aLocale, 0,
                aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_MESSAGE("Expected a break at the start of the line, not at ]", aResult.breakIndex == 0);
        }
    }
}

//See https://bugs.freedesktop.org/show_bug.cgi?id=49629
void TestBreakIterator::testWordBoundaries()
{
    lang::Locale aLocale;
    aLocale.Language = OUString("en");
    aLocale.Country = OUString("US");

    i18n::Boundary aBounds;

    //See https://issues.apache.org/ooo/show_bug.cgi?id=11993
    {
        OUString aTest("abcd ef  ghi??? KLM");

        CPPUNIT_ASSERT(!m_xBreak->isBeginWord(aTest, 4, aLocale, i18n::WordType::DICTIONARY_WORD));
        CPPUNIT_ASSERT(m_xBreak->isEndWord(aTest, 4, aLocale, i18n::WordType::DICTIONARY_WORD));
        aBounds = m_xBreak->getWordBoundary(aTest, 4, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 4);

        CPPUNIT_ASSERT(!m_xBreak->isBeginWord(aTest, 8, aLocale, i18n::WordType::DICTIONARY_WORD));
        CPPUNIT_ASSERT(!m_xBreak->isEndWord(aTest, 8, aLocale, i18n::WordType::DICTIONARY_WORD));

        //next word
        aBounds = m_xBreak->getWordBoundary(aTest, 8, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT(aBounds.startPos == 9 && aBounds.endPos == 12);

        //previous word
        aBounds = m_xBreak->getWordBoundary(aTest, 8, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 5 && aBounds.endPos == 7);

        CPPUNIT_ASSERT(!m_xBreak->isBeginWord(aTest, 12, aLocale, i18n::WordType::DICTIONARY_WORD));
        CPPUNIT_ASSERT(m_xBreak->isEndWord(aTest, 12, aLocale, i18n::WordType::DICTIONARY_WORD));
        aBounds = m_xBreak->getWordBoundary(aTest, 12, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT(aBounds.startPos == 9 && aBounds.endPos == 12);

        CPPUNIT_ASSERT(m_xBreak->isBeginWord(aTest, 16, aLocale, i18n::WordType::DICTIONARY_WORD));
        CPPUNIT_ASSERT(!m_xBreak->isEndWord(aTest, 16, aLocale, i18n::WordType::DICTIONARY_WORD));
        aBounds = m_xBreak->getWordBoundary(aTest, 16, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT(aBounds.startPos == 16 && aBounds.endPos == 19);
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=21907
    {
        OUString aTest("b a?");

        CPPUNIT_ASSERT(m_xBreak->isBeginWord(aTest, 1, aLocale, i18n::WordType::ANY_WORD));
        CPPUNIT_ASSERT(m_xBreak->isBeginWord(aTest, 2, aLocale, i18n::WordType::ANY_WORD));
        CPPUNIT_ASSERT(m_xBreak->isBeginWord(aTest, 3, aLocale, i18n::WordType::ANY_WORD));

        CPPUNIT_ASSERT(m_xBreak->isBeginWord(aTest, 3, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES));

        CPPUNIT_ASSERT(m_xBreak->isEndWord(aTest, 1, aLocale, i18n::WordType::ANY_WORD));
        CPPUNIT_ASSERT(m_xBreak->isEndWord(aTest, 2, aLocale, i18n::WordType::ANY_WORD));
        CPPUNIT_ASSERT(m_xBreak->isEndWord(aTest, 3, aLocale, i18n::WordType::ANY_WORD));

        CPPUNIT_ASSERT(m_xBreak->isEndWord(aTest, 3, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES));
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=14904
    {
        const sal_Unicode TEST[] =
        {
            'W', 'o', 'r', 'k', 'i', 'n', 'g', ' ', 0x201C, 'W', 'o', 'r', 'd', 's',
            ' ', 's', 't', 'a', 'r', 't', 'i', 'n', 'g', ' ', 'w', 'i', 't',
            'h', ' ', 'q', 'u', 'o', 't', 'e', 's', 0x201D, ' ', 'W', 'o', 'r', 'k',
            'i', 'n', 'g', ' ', 0x2018, 'B', 'r', 'o', 'k', 'e', 'n', 0x2019, ' ',
            '?', 'S', 'p', 'a', 'n', 'i', 's', 'h', '?', ' ', 'd', 'o', 'e',
            's', 'n', 0x2019, 't', ' ', 'w', 'o', 'r', 'k', '.', ' ', 'N', 'o',
            't', ' ', 'e', 'v', 'e', 'n', ' ' , 0x00BF, 'r', 'e', 'a', 'l', '?', ' ',
            'S', 'p', 'a', 'n', 'i', 's', 'h'
        };
        OUString aTest(TEST, SAL_N_ELEMENTS(TEST));

        aBounds = m_xBreak->getWordBoundary(aTest, 4, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 7);

        aBounds = m_xBreak->getWordBoundary(aTest, 12, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 9 && aBounds.endPos == 14);

        aBounds = m_xBreak->getWordBoundary(aTest, 40, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 37 && aBounds.endPos == 44);

        aBounds = m_xBreak->getWordBoundary(aTest, 49, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 46 && aBounds.endPos == 52);

        aBounds = m_xBreak->getWordBoundary(aTest, 58, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 55 && aBounds.endPos == 62);

        aBounds = m_xBreak->getWordBoundary(aTest, 67, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 64 && aBounds.endPos == 71);

        aBounds = m_xBreak->getWordBoundary(aTest, 90, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 88 && aBounds.endPos == 92);
    }

    //See https://bugs.freedesktop.org/show_bug.cgi?id=49629
    //Note that the breakiterator test will fail on older icu versions
    //(4.2.1) for the 200B (ZWSP) Zero Width Space testcase.
    sal_Unicode aBreakTests[] = { ' ', 1, 2, 3, 4, 5, 6, 7, 0x91, 0x92, 0x200B, 0xE8FF, 0xF8FF };
    for (int mode = i18n::WordType::ANY_WORD; mode <= i18n::WordType::WORD_COUNT; ++mode)
    {
        //make sure that in all cases isBeginWord and isEndWord matches getWordBoundary
        for (size_t i = 0; i < SAL_N_ELEMENTS(aBreakTests); ++i)
        {
            OUString aTest("Word");
            aTest += OUString(aBreakTests[i]) + OUString("Word");
            aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale, mode, true);
            switch (mode)
            {
                case i18n::WordType::ANY_WORD:
                    CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 4);
                    break;
                case i18n::WordType::ANYWORD_IGNOREWHITESPACES:
                    CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 4);
                    break;
                case i18n::WordType::DICTIONARY_WORD:
                    CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 4);
                    break;
                case i18n::WordType::WORD_COUNT:
                    CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 4);
                    break;
            }

            CPPUNIT_ASSERT(m_xBreak->isBeginWord(aTest, aBounds.startPos, aLocale, mode));
            CPPUNIT_ASSERT(m_xBreak->isEndWord(aTest, aBounds.endPos, aLocale, mode));
        }
    }

    sal_Unicode aJoinTests[] = { 'X', 0x200C, 0x200D, 0x2060, 0xFEFF, 0xFFF9, 0xFFFA, 0xFFFB };
    for (int mode = i18n::WordType::ANY_WORD; mode <= i18n::WordType::WORD_COUNT; ++mode)
    {
        //make sure that in all cases isBeginWord and isEndWord matches getWordBoundary
        for (size_t i = 0; i < SAL_N_ELEMENTS(aJoinTests); ++i)
        {
            OUString aTest("Word");
            aTest += OUString(aJoinTests[i]) + OUString("Word");
            aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale, mode, true);
            switch (mode)
            {
                case i18n::WordType::ANY_WORD:
                    CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 9);
                    break;
                case i18n::WordType::ANYWORD_IGNOREWHITESPACES:
                    CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 9);
                    break;
                case i18n::WordType::DICTIONARY_WORD:
                    CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 9);
                    break;
                case i18n::WordType::WORD_COUNT:
                    CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 9);
                    break;
            }

            CPPUNIT_ASSERT(m_xBreak->isBeginWord(aTest, aBounds.startPos, aLocale, mode));
            CPPUNIT_ASSERT(m_xBreak->isEndWord(aTest, aBounds.endPos, aLocale, mode));
        }
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=13494
    {
        const OUString aBase("xxAAxxBBxxCCxx");
        const sal_Unicode aTests[] =
        {
            '\'', ';', ',', '.', '!', '@', '#', '%', '&', '*',
            '(', ')', '_', '-', '{', '}', '[', ']', '\"', '/',
            '\\', '?', '~', '$', '+', '^', '=', '<', '>', '|'
        };

        const sal_Int32 aDoublePositions[] = {0, 2, 4, 6, 8, 10, 12, 14};
        for (size_t j = 0; j < SAL_N_ELEMENTS(aTests); ++j)
        {
            OUString aTest = aBase.replace('x', aTests[j]);
            sal_Int32 nPos = -1;
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aDoublePositions));
                nPos = m_xBreak->nextWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                CPPUNIT_ASSERT(nPos == aDoublePositions[i++]);
            }
            while (nPos < aTest.getLength());
            nPos = aTest.getLength();
            i = SAL_N_ELEMENTS(aDoublePositions)-1;
            do
            {
                nPos = m_xBreak->previousWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                CPPUNIT_ASSERT(nPos == aDoublePositions[--i]);
            }
            while (nPos > 0);
        }

        const sal_Int32 aSinglePositions[] = {0, 1, 3, 4, 6, 7, 9, 10};
        for (size_t j = 1; j < SAL_N_ELEMENTS(aTests); ++j)
        {
            OUString aTest = aBase.replaceAll(OUString("xx"), OUString(aTests[j]));
            sal_Int32 nPos = -1;
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aSinglePositions));
                nPos = m_xBreak->nextWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                CPPUNIT_ASSERT(nPos == aSinglePositions[i++]);
            }
            while (nPos < aTest.getLength());
            nPos = aTest.getLength();
            i = SAL_N_ELEMENTS(aSinglePositions)-1;
            do
            {
                nPos = m_xBreak->previousWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                CPPUNIT_ASSERT(nPos == aSinglePositions[--i]);
            }
            while (nPos > 0);
        }

        const sal_Int32 aSingleQuotePositions[] = {0, 1, 9, 10};
        CPPUNIT_ASSERT(aTests[0] == '\'');
        {
            OUString aTest = aBase.replaceAll(OUString("xx"), OUString(aTests[0]));
            sal_Int32 nPos = -1;
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aSingleQuotePositions));
                nPos = m_xBreak->nextWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                CPPUNIT_ASSERT(nPos == aSingleQuotePositions[i++]);
            }
            while (nPos < aTest.getLength());
            nPos = aTest.getLength();
            i = SAL_N_ELEMENTS(aSingleQuotePositions)-1;
            do
            {
                nPos = m_xBreak->previousWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                CPPUNIT_ASSERT(nPos == aSingleQuotePositions[--i]);
            }
            while (nPos > 0);
        }
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=13451
    {
        aLocale.Language = OUString("ca");
        aLocale.Country = OUString("ES");

        OUString aTest("mirar-se comprar-vos donem-nos les mans aneu-vos-en!");

        sal_Int32 nPos = 0;
        sal_Int32 aExpected[] = {8, 20, 30, 34, 39, 51, 52};
        size_t i = 0;
        do
        {
            CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aExpected));
            nPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                i18n::WordType::DICTIONARY_WORD, true).endPos;
            CPPUNIT_ASSERT(aExpected[i++] == nPos);
        }
        while (nPos++ < aTest.getLength());
        CPPUNIT_ASSERT(i == SAL_N_ELEMENTS(aExpected));
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=85411
    for (int j = 0; j < 2; ++j)
    {
        switch (j)
        {
            case 0:
                aLocale.Language = OUString("en");
                aLocale.Country = OUString("US");
                break;
            case 1:
                aLocale.Language = OUString("ca");
                aLocale.Country = OUString("ES");
                break;
            default:
                CPPUNIT_ASSERT(false);
                break;
        }

        const sal_Unicode TEST[] =
        {
            'I', 0x200B, 'w', 'a', 'n', 't', 0x200B, 't', 'o', 0x200B, 'g', 'o'
        };
        OUString aTest(TEST, SAL_N_ELEMENTS(TEST));

        sal_Int32 nPos = 0;
        sal_Int32 aExpected[] = {1, 6, 9, 12};
        size_t i = 0;
        do
        {
            CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aExpected));
            nPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                i18n::WordType::DICTIONARY_WORD, true).endPos;
            CPPUNIT_ASSERT(aExpected[i++] == nPos);
        }
        while (nPos++ < aTest.getLength());
        CPPUNIT_ASSERT(i == SAL_N_ELEMENTS(aExpected));
    }

    //https://issues.apache.org/ooo/show_bug.cgi?id=21290
    for (int j = 0; j < 2; ++j)
    {
        switch (j)
        {
            case 0:
                aLocale.Language = OUString("en");
                aLocale.Country = OUString("US");
                break;
            case 1:
                aLocale.Language = OUString("grc");
                aLocale.Country = OUString();
                break;
            default:
                CPPUNIT_ASSERT(false);
                break;
        }

        const sal_Unicode TEST[] =
        {
            0x1F0C, 0x03BD, 0x03B4, 0x03C1, 0x03B1, 0x0020, 0x1F00,
            0x03C1, 0x03BD, 0x1F7B, 0x03BC, 0x03B5, 0x03BD, 0x03BF,
            0x03C2, 0x0020, 0x1F00, 0x03BB, 0x03BB, 0x0020, 0x1F24,
            0x03C3, 0x03B8, 0x03B9, 0x03BF, 0x03BD
        };
        OUString aTest(TEST, SAL_N_ELEMENTS(TEST));

        sal_Int32 nPos = 0;
        sal_Int32 aExpected[] = {5, 15, 19, 26};
        size_t i = 0;
        do
        {
            CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aExpected));
            nPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                i18n::WordType::DICTIONARY_WORD, true).endPos;
            CPPUNIT_ASSERT(aExpected[i++] == nPos);
        }
        while (nPos++ < aTest.getLength());
        CPPUNIT_ASSERT(i == SAL_N_ELEMENTS(aExpected));
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=58513
    {
        aLocale.Language = "fi";
        aLocale.Country = "FI";

        OUString aTest("Kuorma-auto kaakkois- ja Keski-Suomi");

        {
            sal_Int32 nPos = 0;
            sal_Int32 aExpected[] = {12, 22, 25, 36};
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aExpected));
                nPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                    i18n::WordType::WORD_COUNT, true).endPos;
                CPPUNIT_ASSERT(aExpected[i++] == nPos);
            }
            while (nPos++ < aTest.getLength());
            CPPUNIT_ASSERT(i == SAL_N_ELEMENTS(aExpected));
        }

        {
            sal_Int32 nPos = 0;
            sal_Int32 aExpected[] = {0, 11, 12, 21, 22, 24, 25, 36};
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aExpected));
                aBounds = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                    i18n::WordType::DICTIONARY_WORD, true);
                CPPUNIT_ASSERT(aExpected[i++] == aBounds.startPos);
                CPPUNIT_ASSERT(aExpected[i++] == aBounds.endPos);
                nPos = aBounds.endPos;
            }
            while (nPos++ < aTest.getLength());
            CPPUNIT_ASSERT(i == SAL_N_ELEMENTS(aExpected));
        }
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=107843
    {
        aLocale.Language = OUString("en");
        aLocale.Country = OUString("US");

        const sal_Unicode TEST[] =
        {
            'r', 'u', 0xFB00, 'l', 'e', ' ', 0xFB01, 's', 'h'
        };
        OUString aTest(TEST, SAL_N_ELEMENTS(TEST));

        aBounds = m_xBreak->getWordBoundary(aTest, 1, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 5);

        aBounds = m_xBreak->getWordBoundary(aTest, 7, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 6 && aBounds.endPos == 9);
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=113785
    {
        aLocale.Language = OUString("en");
        aLocale.Country = OUString("US");

        const sal_Unicode TEST[] =
        {
            'a', 0x2013, 'b', 0x2014, 'c'
        };
        OUString aTest(TEST, SAL_N_ELEMENTS(TEST));

        aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 1);

        aBounds = m_xBreak->nextWord(aTest, 0, aLocale, i18n::WordType::DICTIONARY_WORD);
        CPPUNIT_ASSERT(aBounds.startPos == 2 && aBounds.endPos == 3);

        aBounds = m_xBreak->nextWord(aTest, aBounds.endPos, aLocale, i18n::WordType::DICTIONARY_WORD);
        CPPUNIT_ASSERT(aBounds.startPos == 4 && aBounds.endPos == 5);
    }
}

//See https://bugs.freedesktop.org/show_bug.cgi?id=40292
//See https://issues.apache.org/ooo/show_bug.cgi?id=80412
//See https://issues.apache.org/ooo/show_bug.cgi?id=111152
//See https://issues.apache.org/ooo/show_bug.cgi?id=50172
void TestBreakIterator::testGraphemeIteration()
{
    lang::Locale aLocale;
    aLocale.Language = OUString("bn");
    aLocale.Country = OUString("IN");

    {
        const sal_Unicode BA_HALANT_LA[] = { 0x09AC, 0x09CD, 0x09AF };
        OUString aTest(BA_HALANT_LA, SAL_N_ELEMENTS(BA_HALANT_LA));

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
        OUString aTest(HA_HALANT_NA_VOWELSIGNI, SAL_N_ELEMENTS(HA_HALANT_NA_VOWELSIGNI));

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
        OUString aTest(TA_HALANT_MA_HALANT_YA, SAL_N_ELEMENTS(TA_HALANT_MA_HALANT_YA));

        sal_Int32 nDone=0;
        sal_Int32 nPos;
        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == SAL_N_ELEMENTS(TA_HALANT_MA_HALANT_YA));
        nPos = m_xBreak->previousCharacters(aTest, SAL_N_ELEMENTS(TA_HALANT_MA_HALANT_YA), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == 0);
    }

    aLocale.Language = OUString("ta");
    aLocale.Country = OUString("IN");

    {
        const sal_Unicode KA_VIRAMA_SSA[] = { 0x0B95, 0x0BCD, 0x0BB7 };
        OUString aTest(KA_VIRAMA_SSA, SAL_N_ELEMENTS(KA_VIRAMA_SSA));

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
        const sal_Unicode KA_VOWELSIGNU[] = { 0x0B95, 0x0BC1 };
        OUString aTest(KA_VOWELSIGNU, SAL_N_ELEMENTS(KA_VOWELSIGNU));

        sal_Int32 nDone=0;
        sal_Int32 nPos = 0;

        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == SAL_N_ELEMENTS(KA_VOWELSIGNU));
        nPos = m_xBreak->previousCharacters(aTest, SAL_N_ELEMENTS(KA_VOWELSIGNU), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == 0);
    }

    {
        const sal_Unicode CA_VOWELSIGNI_TA_VIRAMA_TA_VOWELSIGNI_RA_VOWELSIGNAI[] =
            { 0x0B9A, 0x0BBF, 0x0BA4, 0x0BCD, 0x0BA4, 0x0BBF, 0x0BB0, 0x0BC8 };
        OUString aTest(CA_VOWELSIGNI_TA_VIRAMA_TA_VOWELSIGNI_RA_VOWELSIGNAI,
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
        OUString aText(ALEF_QAMATS, SAL_N_ELEMENTS(ALEF_QAMATS));

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

    aLocale.Language = OUString("hi");
    aLocale.Country = OUString("IN");

    {
        const sal_Unicode SHA_VOWELSIGNII[] = { 0x936, 0x940 };
        OUString aTest(SHA_VOWELSIGNII, SAL_N_ELEMENTS(SHA_VOWELSIGNII));

        sal_Int32 nDone=0;
        sal_Int32 nPos = 0;

        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == SAL_N_ELEMENTS(SHA_VOWELSIGNII));
        nPos = m_xBreak->previousCharacters(aTest, SAL_N_ELEMENTS(SHA_VOWELSIGNII), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_MESSAGE("Should skip full grapheme", nPos == 0);
    }
}

//A test to ensure that certain ranges and codepoints that are categorized as
//weak remain as weak, so that existing docs that depend on this don't silently
//change font for those weak chars
void TestBreakIterator::testWeak()
{
    lang::Locale aLocale;
    aLocale.Language = OUString("en");
    aLocale.Country = OUString("US");

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
        OUString aWeaks(WEAKS, SAL_N_ELEMENTS(WEAKS));

        for (sal_Int32 i = 0; i < aWeaks.getLength(); ++i)
        {
            sal_Int16 nScript = m_xBreak->getScriptType(aWeaks, i);
            OStringBuffer aMsg;
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
    aLocale.Language = OUString("en");
    aLocale.Country = OUString("US");

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
        OUString aAsians(ASIANS, SAL_N_ELEMENTS(ASIANS));

        for (sal_Int32 i = 0; i < aAsians.getLength(); ++i)
        {
            sal_Int16 nScript = m_xBreak->getScriptType(aAsians, i);
            OStringBuffer aMsg;
            aMsg.append(RTL_CONSTASCII_STRINGPARAM("Char 0x"));
            aMsg.append(static_cast<sal_Int32>(aAsians.getStr()[i]), 16);
            aMsg.append(RTL_CONSTASCII_STRINGPARAM(" should have been asian"));
            CPPUNIT_ASSERT_MESSAGE(aMsg.getStr(),
                nScript == i18n::ScriptType::ASIAN);
        }
    }
}

//A test to ensure that our thai word boundary detection is useful
void TestBreakIterator::testThai()
{
    lang::Locale aLocale;
    aLocale.Language = OUString("th");
    aLocale.Country = OUString("TH");

    //See http://lists.freedesktop.org/archives/libreoffice/2012-February/025959.html
    {
        const sal_Unicode THAI[] = { 0x0E01, 0x0E38, 0x0E2B, 0x0E25, 0x0E32, 0x0E1A };
        OUString aTest(THAI, SAL_N_ELEMENTS(THAI));
        i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_MESSAGE("Should skip full word",
            aBounds.startPos == 0 && aBounds.endPos == aTest.getLength());
    }

    //See https://issues.apache.org/ooo/show_bug.cgi?id=29548
    //make sure forwards and back are consistent
    {
        const sal_Unicode THAI[] =
        {
            0x0E2D, 0x0E38, 0x0E17, 0x0E22, 0x0E32, 0x0E19, 0x0E41,
            0x0E2B, 0x0E48, 0x0E07, 0x0E0A, 0x0E32, 0x0E15, 0x0E34,
            0x0E19, 0x0E49, 0x0E33, 0x0E2B, 0x0E19, 0x0E32, 0x0E27,
            0x0E2D, 0x0E38, 0x0E17, 0x0E22, 0x0E32, 0x0E19, 0x0E41,
            0x0E2B, 0x0E48, 0x0E07, 0x0E0A, 0x0E32, 0x0E15, 0x0E34,
            0x0E19, 0x0E49, 0x0E33, 0x0E2B, 0x0E19, 0x0E32, 0x0E27
        };
        OUString aTest(THAI, SAL_N_ELEMENTS(THAI));

        std::stack<sal_Int32> aPositions;
        sal_Int32 nPos = -1;
        do
        {
            nPos = m_xBreak->nextWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
            aPositions.push(nPos);
        }
        while (nPos < aTest.getLength());
        nPos = aTest.getLength();
        CPPUNIT_ASSERT(!aPositions.empty());
        aPositions.pop();
        do
        {
            CPPUNIT_ASSERT(!aPositions.empty());
            nPos = m_xBreak->previousWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
            CPPUNIT_ASSERT(nPos == aPositions.top());
            aPositions.pop();
        }
        while (nPos > 0);
    }
}

#ifdef TODO
void TestBreakIterator::testNorthernThai()
{
    lang::Locale aLocale;
    aLocale.Language = OUString("nod");
    aLocale.Country = OUString("TH");

    const sal_Unicode NORTHERN_THAI1[] = { 0x0E01, 0x0E38, 0x0E4A, 0x0E2B, 0x0E25, 0x0E32, 0x0E1A };
    OUString aTest(NORTHERN_THAI1, SAL_N_ELEMENTS(NORTHERN_THAI1));
    i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);
    CPPUNIT_ASSERT_MESSAGE("Should skip full word",
        aBounds.startPos == 0 && aBounds.endPos == aTest.getLength());
}
#endif

#if (U_ICU_VERSION_MAJOR_NUM > 4)
// Not sure if any version earlier than 49 did have Khmer word boundary
// dictionaries, 4.6 does not.

//A test to ensure that our khmer word boundary detection is useful
//https://bugs.freedesktop.org/show_bug.cgi?id=52020
void TestBreakIterator::testKhmer()
{
    lang::Locale aLocale;
    aLocale.Language = OUString("km");
    aLocale.Country = OUString("KH");

    const sal_Unicode KHMER[] = { 0x17B2, 0x17D2, 0x1799, 0x1782, 0x17C1 };

    OUString aTest(KHMER, SAL_N_ELEMENTS(KHMER));
    i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);

    CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 3);

    aBounds = m_xBreak->getWordBoundary(aTest, aBounds.endPos, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);

    CPPUNIT_ASSERT(aBounds.startPos == 3 && aBounds.endPos == 5);
}
#endif

void TestBreakIterator::testJapanese()
{
    lang::Locale aLocale;
    aLocale.Language = OUString("ja");
    aLocale.Country = OUString("JP");
    i18n::Boundary aBounds;

    {
        const sal_Unicode JAPANESE[] = { 0x30B7, 0x30E3, 0x30C3, 0x30C8, 0x30C0, 0x30A6, 0x30F3 };

        OUString aTest(JAPANESE, SAL_N_ELEMENTS(JAPANESE));
        aBounds = m_xBreak->getWordBoundary(aTest, 5, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);

        CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 7);
    }

    {
        const sal_Unicode JAPANESE[] = { 0x9EBB, 0x306E, 0x8449, 0x9EBB, 0x306E, 0x8449 };

        OUString aTest(JAPANESE, SAL_N_ELEMENTS(JAPANESE));
        aBounds = m_xBreak->getWordBoundary(aTest, 1, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);

        CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 3);

        aBounds = m_xBreak->getWordBoundary(aTest, 5, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);

        CPPUNIT_ASSERT(aBounds.startPos == 3 && aBounds.endPos == 6);
    }
}

void TestBreakIterator::setUp()
{
    BootstrapFixtureBase::setUp();
    m_xBreak = uno::Reference< i18n::XBreakIterator >(m_xSFactory->createInstance(
        "com.sun.star.i18n.BreakIterator"), uno::UNO_QUERY_THROW);
}

void TestBreakIterator::tearDown()
{
    m_xBreak.clear();
    BootstrapFixtureBase::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestBreakIterator);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
