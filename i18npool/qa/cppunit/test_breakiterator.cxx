/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <o3tl/cppunittraitshelper.hxx>
#include <unotest/bootstrapfixturebase.hxx>

#include <unicode/uvernum.h>

#include <string.h>

#include <stack>

using namespace ::com::sun::star;

class TestBreakIterator : public test::BootstrapFixtureBase
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void testLineBreaking();
    void testWordBoundaries();
    void testGraphemeIteration();
    void testWeak();
    void testAsian();
    void testThai();
#if (U_ICU_VERSION_MAJOR_NUM > 51)
    void testLao();
#ifdef TODO
    void testNorthernThai();
    void testKhmer();
#endif
#endif
    void testJapanese();
    void testChinese();

    CPPUNIT_TEST_SUITE(TestBreakIterator);
    CPPUNIT_TEST(testLineBreaking);
    CPPUNIT_TEST(testWordBoundaries);
    CPPUNIT_TEST(testGraphemeIteration);
    CPPUNIT_TEST(testWeak);
    CPPUNIT_TEST(testAsian);
    CPPUNIT_TEST(testThai);
#if (U_ICU_VERSION_MAJOR_NUM > 51)
    CPPUNIT_TEST(testLao);
#ifdef TODO
    CPPUNIT_TEST(testKhmer);
    CPPUNIT_TEST(testNorthernThai);
#endif
#endif
    CPPUNIT_TEST(testJapanese);
    CPPUNIT_TEST(testChinese);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<i18n::XBreakIterator> m_xBreak;
    void doTestJapanese(uno::Reference< i18n::XBreakIterator > const &xBreak);
};

void TestBreakIterator::testLineBreaking()
{
    i18n::LineBreakHyphenationOptions aHyphOptions;
    i18n::LineBreakUserOptions aUserOptions;
    lang::Locale aLocale;

    //See https://bugs.libreoffice.org/show_bug.cgi?id=31271
    {
        OUString aTest("(some text here)");

        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            //Here we want the line break to leave text here) on the next line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, strlen("(some tex"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected a break at the start of the word", static_cast<sal_Int32>(6), aResult.breakIndex);
        }

        {
            //Here we want the line break to leave "here)" on the next line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, strlen("(some text here"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected a break at the start of the word", static_cast<sal_Int32>(11), aResult.breakIndex);
        }
    }

    //See https://bugs.libreoffice.org/show_bug.cgi?id=49849
    {
        constexpr OUStringLiteral aWord = u"\u05DE\u05D9\u05DC\u05D9\u05DD";
        OUString aTest(aWord + " " + aWord);

        aLocale.Language = "he";
        aLocale.Country = "IL";

        {
            //Here we want the line break to happen at the whitespace
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, aTest.getLength()-1, aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected a break at the start of the word", aWord.getLength()+1, aResult.breakIndex);
        }
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=17155
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            //Here we want the line break to leave /bar/ba clumped together on the next line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak("foo /bar/baz", strlen("foo /bar/ba"), aLocale, 0,
                aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected a break at the first slash", static_cast<sal_Int32>(4), aResult.breakIndex);
        }
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=19716
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            OUString aTest("aaa]aaa");
            //Here we want the line break to move the whole lot to the next line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, aTest.getLength()-2, aLocale, 0,
                aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected a break at the start of the line, not at ]", static_cast<sal_Int32>(0), aResult.breakIndex);
        }
    }

    //this is an example sequence from tdf92993-1.docx caught by the load crashtesting
    {
        static constexpr OUStringLiteral aTest = u"\U0001f356\U0001f357\U0001f346"
                                       "\U0001f364\u2668\ufe0f\U0001f3c6";

        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            //This must not assert/crash
            (void)m_xBreak->getLineBreak(aTest, 0, aLocale, 0, aHyphOptions, aUserOptions);
        }
    }

    //See https://bugs.documentfoundation.org/show_bug.cgi?id=96197
    {
        static constexpr OUStringLiteral aTest = u"\uc560\uad6D\uac00\uc758 \uac00"
                                       "\uc0ac\ub294";

        aLocale.Language = "ko";
        aLocale.Country = "KR";

        {
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, aTest.getLength()-2, aLocale, 0,
                aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected a break don't split the Korean word!", static_cast<sal_Int32>(5), aResult.breakIndex);
        }
    }
}

//See https://bugs.libreoffice.org/show_bug.cgi?id=49629
void TestBreakIterator::testWordBoundaries()
{
    lang::Locale aLocale;
    aLocale.Language = "en";
    aLocale.Country = "US";

    i18n::Boundary aBounds;

    //See https://bz.apache.org/ooo/show_bug.cgi?id=11993
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

    //See https://bz.apache.org/ooo/show_bug.cgi?id=21907
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

    //See https://bz.apache.org/ooo/show_bug.cgi?id=14904
    {
        static constexpr OUStringLiteral aTest =
            u"Working \u201CWords"
            " starting wit"
            "h quotes\u201D Work"
            "ing \u2018Broken\u2019 "
            "?Spanish? doe"
            "sn\u2019t work. No"
            "t even \u00BFreal? "
            "Spanish";

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

    //See https://bugs.libreoffice.org/show_bug.cgi?id=49629
    sal_Unicode aBreakTests[] = { ' ', 1, 2, 3, 4, 5, 6, 7, 0x91, 0x92, 0x200B, 0xE8FF, 0xF8FF };
    for (int mode = i18n::WordType::ANY_WORD; mode <= i18n::WordType::WORD_COUNT; ++mode)
    {
        //make sure that in all cases isBeginWord and isEndWord matches getWordBoundary
        for (size_t i = 0; i < SAL_N_ELEMENTS(aBreakTests); ++i)
        {
#if (U_ICU_VERSION_MAJOR_NUM == 4) && (U_ICU_VERSION_MINOR_NUM <= 2)
            //Note the breakiterator test is known to fail on older icu
            //versions (4.2.1) for the 200B (ZWSP) Zero Width Space testcase.
            if (aBreakTests[i] == 0x200B)
                continue;
#endif
            OUString aTest = "Word" + OUStringChar(aBreakTests[i]) + "Word";
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
            OUString aTest = "Word" + OUStringChar(aJoinTests[i]) + "Word";
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

    //See https://bz.apache.org/ooo/show_bug.cgi?id=13494
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
                CPPUNIT_ASSERT_EQUAL(aDoublePositions[i], nPos);
                ++i;
            }
            while (nPos < aTest.getLength());
            nPos = aTest.getLength();
            i = SAL_N_ELEMENTS(aDoublePositions)-1;
            do
            {
                nPos = m_xBreak->previousWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                --i;
                CPPUNIT_ASSERT_EQUAL(aDoublePositions[i], nPos);
            }
            while (nPos > 0);
        }

        const sal_Int32 aSinglePositions[] = {0, 1, 3, 4, 6, 7, 9, 10};
        for (size_t j = 1; j < SAL_N_ELEMENTS(aTests); ++j)
        {
            OUString aTest = aBase.replaceAll("xx", OUStringChar(aTests[j]));
            sal_Int32 nPos = -1;
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aSinglePositions));
                nPos = m_xBreak->nextWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                CPPUNIT_ASSERT_EQUAL(aSinglePositions[i], nPos);
                ++i;
            }
            while (nPos < aTest.getLength());
            nPos = aTest.getLength();
            i = SAL_N_ELEMENTS(aSinglePositions)-1;
            do
            {
                nPos = m_xBreak->previousWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                --i;
                CPPUNIT_ASSERT_EQUAL(aSinglePositions[i], nPos);
            }
            while (nPos > 0);
        }

        const sal_Int32 aSingleQuotePositions[] = {0, 1, 9, 10};
        CPPUNIT_ASSERT_EQUAL(u'\'', aTests[0]);
        {
            OUString aTest = aBase.replaceAll("xx", OUStringChar(aTests[0]));
            sal_Int32 nPos = -1;
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aSingleQuotePositions));
                nPos = m_xBreak->nextWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                CPPUNIT_ASSERT_EQUAL(aSingleQuotePositions[i], nPos);
                ++i;
            }
            while (nPos < aTest.getLength());
            nPos = aTest.getLength();
            i = SAL_N_ELEMENTS(aSingleQuotePositions)-1;
            do
            {
                nPos = m_xBreak->previousWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                --i;
                CPPUNIT_ASSERT_EQUAL(aSingleQuotePositions[i], nPos);
            }
            while (nPos > 0);
        }
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=13451
    {
        aLocale.Language = "ca";
        aLocale.Country = "ES";

        OUString aTest("mirar-se comprar-vos donem-nos les mans aneu-vos-en!");

        sal_Int32 nPos = 0;
        sal_Int32 aExpected[] = {8, 20, 30, 34, 39, 51, 52};
        size_t i = 0;
        do
        {
            CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aExpected));
            nPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                i18n::WordType::DICTIONARY_WORD, true).endPos;
            CPPUNIT_ASSERT_EQUAL(aExpected[i], nPos);
            ++i;
        }
        while (nPos++ < aTest.getLength());
        CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(aExpected), i);
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=85411
    for (int j = 0; j < 3; ++j)
    {
        switch (j)
        {
            case 0:
                aLocale.Language = "en";
                aLocale.Country = "US";
                break;
            case 1:
                aLocale.Language = "ca";
                aLocale.Country = "ES";
                break;
            case 2:
                aLocale.Language = "fi";
                aLocale.Country = "FI";
                break;
            default:
                CPPUNIT_ASSERT(false);
                break;
        }

        static constexpr OUStringLiteral aTest =
            u"I\u200Bwant\u200Bto\u200Bgo";

        sal_Int32 nPos = 0;
        sal_Int32 aExpected[] = {1, 6, 9, 12};
        size_t i = 0;
        do
        {
            CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aExpected));
            nPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                i18n::WordType::DICTIONARY_WORD, true).endPos;
            CPPUNIT_ASSERT_EQUAL(aExpected[i], nPos);
            ++i;
        }
        while (nPos++ < aTest.getLength());
        CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(aExpected), i);
    }

    //https://bz.apache.org/ooo/show_bug.cgi?id=21290
    for (int j = 0; j < 2; ++j)
    {
        switch (j)
        {
            case 0:
                aLocale.Language = "en";
                aLocale.Country = "US";
                break;
            case 1:
                aLocale.Language = "grc";
                aLocale.Country.clear();
                break;
            default:
                CPPUNIT_ASSERT(false);
                break;
        }

        static constexpr OUStringLiteral aTest =
            u"\u1F0C\u03BD\u03B4\u03C1\u03B1 \u1F00"
            "\u03C1\u03BD\u1F7B\u03BC\u03B5\u03BD\u03BF"
            "\u03C2 \u1F00\u03BB\u03BB \u1F24"
            "\u03C3\u03B8\u03B9\u03BF\u03BD";

        sal_Int32 nPos = 0;
        sal_Int32 aExpected[] = {5, 15, 19, 26};
        size_t i = 0;
        do
        {
            CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aExpected));
            nPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                i18n::WordType::DICTIONARY_WORD, true).endPos;
            CPPUNIT_ASSERT_EQUAL(aExpected[i], nPos);
            ++i;
        }
        while (nPos++ < aTest.getLength());
        CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(aExpected), i);
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=58513
    //See https://bugs.libreoffice.org/show_bug.cgi?id=55707
    {
        aLocale.Language = "fi";
        aLocale.Country = "FI";

        OUString aTest("Kuorma-auto kaakkois- ja Keski-Suomi USA:n 90:n %:n");

        {
            sal_Int32 nPos = 0;
            sal_Int32 aExpected[] = {11, 21, 24, 36, 42, 47, 51};
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aExpected));
                nPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                    i18n::WordType::WORD_COUNT, true).endPos;
                CPPUNIT_ASSERT_EQUAL(aExpected[i], nPos);
                ++i;
            }
            while (nPos++ < aTest.getLength());
            CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(aExpected), i);
        }

        {
            sal_Int32 nPos = 0;
            sal_Int32 aExpected[] = {0, 11, 12, 20, 22, 24, 25, 36, 37,
                                    40, 41, 42, 43, 45, 46, 47, 50, 51};
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < SAL_N_ELEMENTS(aExpected));
                aBounds = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                    i18n::WordType::DICTIONARY_WORD, true);
                CPPUNIT_ASSERT_EQUAL(aExpected[i], aBounds.startPos);
                ++i;
                CPPUNIT_ASSERT_EQUAL(aExpected[i], aBounds.endPos);
                ++i;
                nPos = aBounds.endPos;
            }
            while (nPos++ < aTest.getLength());
            CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(aExpected), i);
        }
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=107843
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        static constexpr OUStringLiteral aTest =
            u"ru\uFB00le \uFB01sh";

        aBounds = m_xBreak->getWordBoundary(aTest, 1, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 5);

        aBounds = m_xBreak->getWordBoundary(aTest, 7, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT(aBounds.startPos == 6 && aBounds.endPos == 9);
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=113785
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        static constexpr OUStringLiteral aTest =
            u"a\u2013b\u2014c";

        aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 1);

        aBounds = m_xBreak->nextWord(aTest, 0, aLocale, i18n::WordType::DICTIONARY_WORD);
        CPPUNIT_ASSERT(aBounds.startPos == 2 && aBounds.endPos == 3);

        aBounds = m_xBreak->nextWord(aTest, aBounds.endPos, aLocale, i18n::WordType::DICTIONARY_WORD);
        CPPUNIT_ASSERT(aBounds.startPos == 4 && aBounds.endPos == 5);
    }
}

//See https://bugs.libreoffice.org/show_bug.cgi?id=40292
//See https://bz.apache.org/ooo/show_bug.cgi?id=80412
//See https://bz.apache.org/ooo/show_bug.cgi?id=111152
//See https://bz.apache.org/ooo/show_bug.cgi?id=50172
void TestBreakIterator::testGraphemeIteration()
{
    lang::Locale aLocale;
    aLocale.Language = "bn";
    aLocale.Country = "IN";

    {
        static constexpr OUStringLiteral aTest = u"\u09AC\u09CD\u09AF"; // BA HALANT LA

        sal_Int32 nDone=0;
        sal_Int32 nPos;
        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", aTest.getLength(), nPos);
        nPos = m_xBreak->previousCharacters(aTest, aTest.getLength(), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(0), nPos);
    }

    {
        static constexpr OUStringLiteral aTest = u"\u09B9\u09CD\u09A3\u09BF";
            // HA HALANT NA VOWELSIGNI

        sal_Int32 nDone=0;
        sal_Int32 nPos;
        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", aTest.getLength(), nPos);
        nPos = m_xBreak->previousCharacters(aTest, aTest.getLength(), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(0), nPos);
    }

    {
        static constexpr OUStringLiteral aTest = u"\u09A4\u09CD\u09AE\u09CD\u09AF";
            // TA HALANT MA HALANT YA

        sal_Int32 nDone=0;
        sal_Int32 nPos;
        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", aTest.getLength(), nPos);
        nPos = m_xBreak->previousCharacters(aTest, aTest.getLength(), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(0), nPos);
    }

    aLocale.Language = "ta";
    aLocale.Country = "IN";

    {
        static constexpr OUStringLiteral aTest = u"\u0B9A\u0BBF\u0BA4\u0BCD\u0BA4\u0BBF\u0BB0\u0BC8"; // CA VOWELSIGNI TA VIRAMA TA VOWELSIGNI RA VOWELSIGNAI

        sal_Int32 nDone=0;
        sal_Int32 nPos = 0;

        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale, i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(2), nPos);
        nPos = m_xBreak->nextCharacters(aTest, nPos, aLocale, i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(4), nPos);
        nPos = m_xBreak->nextCharacters(aTest, nPos, aLocale, i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(6), nPos);
        nPos = m_xBreak->nextCharacters(aTest, nPos, aLocale, i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", aTest.getLength(), nPos);
        nPos = m_xBreak->previousCharacters(aTest, aTest.getLength(), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(6), nPos);
        nPos = m_xBreak->previousCharacters(aTest, nPos, aLocale, i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(4), nPos);
        nPos = m_xBreak->previousCharacters(aTest, nPos, aLocale, i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(2), nPos);
        nPos = m_xBreak->previousCharacters(aTest, nPos, aLocale, i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(0), nPos);
    }

    {
        static constexpr OUStringLiteral aTest = u"\u0B95\u0BC1"; // KA VOWELSIGNU

        sal_Int32 nDone=0;
        sal_Int32 nPos = 0;

        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", aTest.getLength(), nPos);
        nPos = m_xBreak->previousCharacters(aTest, aTest.getLength(), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(0), nPos);
    }

    {
        static constexpr OUStringLiteral aTest =
            u"\u0B9A\u0BBF\u0BA4\u0BCD\u0BA4\u0BBF\u0BB0\u0BC8";
            // CA VOWELSIGNI TA VIRAMA TA VOWELSIGNI RA VOWELSIGNAI

        sal_Int32 nDone=0;
        sal_Int32 nPos=0;

        for (sal_Int32 i = 0; i < 4; ++i)
        {
            sal_Int32 nOldPos = nPos;
            nPos = m_xBreak->nextCharacters(aTest, nPos, aLocale,
                i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip 2 units", nOldPos+2, nPos);
        }

        for (sal_Int32 i = 0; i < 4; ++i)
        {
            sal_Int32 nOldPos = nPos;
            nPos = m_xBreak->previousCharacters(aTest, nPos, aLocale,
                i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip 2 units", nOldPos-2, nPos);
        }
    }

    {
        static constexpr OUStringLiteral aText = u"\u05D0\u05B8"; // ALEF QAMATS

        sal_Int32 nGraphemeCount = 0;

        sal_Int32 nCurPos = 0;
        while (nCurPos < aText.getLength())
        {
            sal_Int32 nCount2 = 1;
            nCurPos = m_xBreak->nextCharacters(aText, nCurPos, lang::Locale(),
                i18n::CharacterIteratorMode::SKIPCELL, nCount2, nCount2);
            ++nGraphemeCount;
        }

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should be considered 1 grapheme", static_cast<sal_Int32>(1), nGraphemeCount);
    }

    aLocale.Language = "hi";
    aLocale.Country = "IN";

    {
        static constexpr OUStringLiteral aTest = u"\u0936\u0940"; // SHA VOWELSIGNII

        sal_Int32 nDone=0;
        sal_Int32 nPos = 0;

        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", aTest.getLength(), nPos);
        nPos = m_xBreak->previousCharacters(aTest, aTest.getLength(), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(0), nPos);
    }
}

//A test to ensure that certain ranges and codepoints that are categorized as
//weak remain as weak, so that existing docs that depend on this don't silently
//change font for those weak chars
void TestBreakIterator::testWeak()
{
    lang::Locale aLocale;
    aLocale.Language = "en";
    aLocale.Country = "US";

    {
        static constexpr OUStringLiteral aWeaks =
            u"\u0001\u0002"
            " \u00A0"
            "\u0300\u036F"  //Combining Diacritical Marks
            "\u1AB0\u1AFF"  //Combining Diacritical Marks Extended
            "\u1DC0\u1DFF"  //Combining Diacritical Marks Supplement
            "\u20D0\u20FF"  //Combining Diacritical Marks for Symbols
            "\u2150\u215F"  //Number Forms, fractions
            "\u2160\u2180"  //Number Forms, roman numerals
            "\u2200\u22FF"  //Mathematical Operators
            "\u27C0\u27EF"  //Miscellaneous Mathematical Symbols-A
            "\u2980\u29FF"  //Miscellaneous Mathematical Symbols-B
            "\u2A00\u2AFF"  //Supplemental Mathematical Operators
            "\u2100\u214F"  //Letterlike Symbols
            "\u2308\u230B"  //Miscellaneous technical
            "\u25A0\u25FF"  //Geometric Shapes
            "\u2B30\u2B4C"; //Miscellaneous Symbols and Arrows

        for (sal_Int32 i = 0; i < aWeaks.getLength(); ++i)
        {
            sal_Int16 nScript = m_xBreak->getScriptType(aWeaks, i);
            OString aMsg =
                "Char 0x" +
                OString::number(static_cast<sal_Int32>(OUString(aWeaks)[i]), 16) +
                " should have been weak";
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(),
                i18n::ScriptType::WEAK, nScript);
        }
    }
}

//A test to ensure that certain ranges and codepoints that are categorized as
//asian remain as asian, so that existing docs that depend on this don't silently
//change font for those asian chars.
//See https://bugs.libreoffice.org/show_bug.cgi?id=38095
void TestBreakIterator::testAsian()
{
    lang::Locale aLocale;
    aLocale.Language = "en";
    aLocale.Country = "US";

    {
        static constexpr OUStringLiteral aAsians =
            //some typical CJK chars
            u"\u4E00\u62FF"
            //The full HalfWidth and FullWidth block has historically been
            //designated as taking the CJK font :-(
            //HalfWidth and FullWidth forms of ASCII 0-9, categorized under
            //UAX24 as "Common" i.e. by that logic WEAK
            "\uFF10\uFF19"
            //HalfWidth and FullWidth forms of ASCII A-z, categorized under
            //UAX25 as "Latin", i.e. by that logic LATIN
            "\uFF21\uFF5A";

        for (sal_Int32 i = 0; i < aAsians.getLength(); ++i)
        {
            sal_Int16 nScript = m_xBreak->getScriptType(aAsians, i);
            OString aMsg =
                "Char 0x" +
                OString::number(static_cast<sal_Int32>(OUString(aAsians)[i]), 16) +
                " should have been asian";
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(),
                i18n::ScriptType::ASIAN, nScript);
        }
    }
}

#if (U_ICU_VERSION_MAJOR_NUM > 51)
//A test to ensure that our Lao word boundary detection is useful
void TestBreakIterator::testLao()
{
    lang::Locale aLocale;
    aLocale.Language = "lo";
    aLocale.Country = "LA";

    static constexpr OUStringLiteral aTest = u"\u0e8d\u0eb4\u0e99\u0e94\u0eb5\u0e95\u0ec9\u0ead\u0e99\u0eae\u0eb1\u0e9a";
    i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

    aBounds = m_xBreak->getWordBoundary(aTest, aBounds.endPos, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.endPos);
}
#endif

//A test to ensure that our thai word boundary detection is useful
void TestBreakIterator::testThai()
{
    lang::Locale aLocale;
    aLocale.Language = "th";
    aLocale.Country = "TH";

    //See http://lists.freedesktop.org/archives/libreoffice/2012-February/025959.html
    {
        static constexpr OUStringLiteral aTest = u"\u0E01\u0E38\u0E2B\u0E25\u0E32\u0E1A";
        i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_MESSAGE("Should skip full word",
            aBounds.startPos == 0 && aBounds.endPos == aTest.getLength());
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=29548
    //make sure forwards and back are consistent
    {
        static constexpr OUStringLiteral aTest =
            u"\u0E2D\u0E38\u0E17\u0E22\u0E32\u0E19\u0E41"
            "\u0E2B\u0E48\u0E07\u0E0A\u0E32\u0E15\u0E34"
            "\u0E19\u0E49\u0E33\u0E2B\u0E19\u0E32\u0E27"
            "\u0E2D\u0E38\u0E17\u0E22\u0E32\u0E19\u0E41"
            "\u0E2B\u0E48\u0E07\u0E0A\u0E32\u0E15\u0E34"
            "\u0E19\u0E49\u0E33\u0E2B\u0E19\u0E32\u0E27";

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
            CPPUNIT_ASSERT_EQUAL(aPositions.top(), nPos);
            aPositions.pop();
        }
        while (nPos > 0);
    }

    // tdf#113694
    {
        static constexpr OUStringLiteral aTest = u"\U00010000";

        sal_Int32 nDone=0;
        sal_Int32 nPos;

        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full surrogate pair", aTest.getLength(), nPos);
        nPos = m_xBreak->previousCharacters(aTest, aTest.getLength(), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full surrogate pair", static_cast<sal_Int32>(0), nPos);

        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCHARACTER, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full surrogate pair", aTest.getLength(), nPos);
        nPos = m_xBreak->previousCharacters(aTest, aTest.getLength(), aLocale,
            i18n::CharacterIteratorMode::SKIPCHARACTER, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full surrogate pair", static_cast<sal_Int32>(0), nPos);
    }
}

#ifdef TODO
void TestBreakIterator::testNorthernThai()
{
    lang::Locale aLocale;
    aLocale.Language = "nod";
    aLocale.Country = "TH";

    const sal_Unicode NORTHERN_THAI1[] = { 0x0E01, 0x0E38, 0x0E4A, 0x0E2B, 0x0E25, 0x0E32, 0x0E1A };
    OUString aTest(NORTHERN_THAI1, SAL_N_ELEMENTS(NORTHERN_THAI1));
    i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);
    CPPUNIT_ASSERT_MESSAGE("Should skip full word",
        aBounds.startPos == 0 && aBounds.endPos == aTest.getLength());
}

// Not sure if any version earlier than 49 did have Khmer word boundary
// dictionaries, 4.6 does not.

// As of icu 54, word boundary detection for Khmer is still considered
// insufficient, so icu khmer stuff is disabled

//A test to ensure that our khmer word boundary detection is useful
//https://bugs.libreoffice.org/show_bug.cgi?id=52020
void TestBreakIterator::testKhmer()
{
    lang::Locale aLocale;
    aLocale.Language = "km";
    aLocale.Country = "KH";

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

void TestBreakIterator::doTestJapanese(uno::Reference< i18n::XBreakIterator > const &xBreak)
{
    lang::Locale aLocale;
    aLocale.Language = "ja";
    aLocale.Country = "JP";
    i18n::Boundary aBounds;

    {
        static constexpr OUStringLiteral aTest = u"\u30B7\u30E3\u30C3\u30C8\u30C0\u30A6\u30F3";

        aBounds = xBreak->getWordBoundary(aTest, 5, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);

        CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 7);
    }

    {
        static constexpr OUStringLiteral aTest = u"\u9EBB\u306E\u8449\u9EBB\u306E\u8449";

        aBounds = xBreak->getWordBoundary(aTest, 1, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);

        CPPUNIT_ASSERT(aBounds.startPos == 0 && aBounds.endPos == 3);

        aBounds = xBreak->getWordBoundary(aTest, 5, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);

        CPPUNIT_ASSERT(aBounds.startPos == 3 && aBounds.endPos == 6);
    }
}

void TestBreakIterator::testJapanese()
{
    doTestJapanese(m_xBreak);

    // fdo#78479 - test second / cached instantiation of xdictionary
    uno::Reference< i18n::XBreakIterator > xTmpBreak(m_xSFactory->createInstance(
        "com.sun.star.i18n.BreakIterator"), uno::UNO_QUERY_THROW);

    doTestJapanese(xTmpBreak);
}

void TestBreakIterator::testChinese()
{
    lang::Locale aLocale;
    aLocale.Language = "zh";
    aLocale.Country = "CN";

    {
        static constexpr OUStringLiteral aTest = u"\u6A35\u6A30\u69FE\u8919\U00029EDB";

        i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 4, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT(aBounds.startPos == 4 && aBounds.endPos == 6);
    }
}
void TestBreakIterator::setUp()
{
    BootstrapFixtureBase::setUp();
    m_xBreak.set(m_xSFactory->createInstance("com.sun.star.i18n.BreakIterator"), uno::UNO_QUERY_THROW);
}

void TestBreakIterator::tearDown()
{
    m_xBreak.clear();
    BootstrapFixtureBase::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestBreakIterator);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
