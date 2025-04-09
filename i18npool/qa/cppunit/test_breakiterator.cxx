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
#include <string_view>

using namespace ::com::sun::star;

class TestBreakIterator : public test::BootstrapFixtureBase
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void testLineBreaking();
    void testWordBoundaries();
    void testSentenceBoundaries();
    void testGraphemeIteration();
    void testWeak();
    void testAsian();
    void testThai();
    void testLao();
#ifdef TODO
    void testNorthernThai();
    void testKhmer();
#endif
    void testJapanese();
    void testChinese();
    void testKorean();

    void testDictWordAbbreviation();
    void testDictWordPrepostDash();
    void testHebrewGereshGershaim();
    void testLegacySurrogatePairs();
    void testWordCount();
    void testDictionaryIteratorLanguages();

    CPPUNIT_TEST_SUITE(TestBreakIterator);
    CPPUNIT_TEST(testLineBreaking);
    CPPUNIT_TEST(testWordBoundaries);
    CPPUNIT_TEST(testSentenceBoundaries);
    CPPUNIT_TEST(testGraphemeIteration);
    CPPUNIT_TEST(testWeak);
    CPPUNIT_TEST(testAsian);
    CPPUNIT_TEST(testThai);
    CPPUNIT_TEST(testLao);
#ifdef TODO
    CPPUNIT_TEST(testKhmer);
    CPPUNIT_TEST(testNorthernThai);
#endif
    CPPUNIT_TEST(testJapanese);
    CPPUNIT_TEST(testChinese);
    CPPUNIT_TEST(testKorean);
    CPPUNIT_TEST(testDictWordAbbreviation);
    CPPUNIT_TEST(testDictWordPrepostDash);
    CPPUNIT_TEST(testHebrewGereshGershaim);
    CPPUNIT_TEST(testLegacySurrogatePairs);
    CPPUNIT_TEST(testWordCount);
    CPPUNIT_TEST(testDictionaryIteratorLanguages);
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
        OUString aTest(u"(some text here)"_ustr);

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
        static constexpr OUString aWord = u"\u05DE\u05D9\u05DC\u05D9\u05DD"_ustr;
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
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(u"foo /bar/baz"_ustr, strlen("foo /bar/ba"), aLocale, 0,
                aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected a break at the first slash", static_cast<sal_Int32>(4), aResult.breakIndex);
        }
    }

    // i#22602: writer breaks word after dot immediately followed by a letter
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            //Here we want the line break to leave ./bar/baz clumped together on the next line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                u"foo ./bar/baz"_ustr, strlen("foo ./bar/ba"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected a break at the first period",
                                         static_cast<sal_Int32>(4), aResult.breakIndex);
        }
    }

    // i#81448: slash and backslash make non-breaking spaces of preceding spaces
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            // Per the bug, the line break should leave ...BE clumped together on the next line.
            // However, the current behavior does not wrap the string at all. This test asserts the
            // current behavior as a point of reference.
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                u"THIS... ...BE"_ustr, strlen("THIS... ...B"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aResult.breakIndex);
        }
    }

    // i#81448: slash and backslash make non-breaking spaces of preceding spaces
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            // The line break should leave /BE clumped together on the next line.
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                u"THIS... /BE"_ustr, strlen("THIS... /B"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8), aResult.breakIndex);
        }
    }

    // i#80548: Bad word wrap between dash and word
    {
        aLocale.Language = "fi";
        aLocale.Country = "FI";

        {
            // Per the bug, the line break should leave -bar clumped together on the next line.
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                u"foo -bar"_ustr, strlen("foo -ba"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected a break at the first dash",
                                         static_cast<sal_Int32>(4), aResult.breakIndex);
        }
    }

    // i#80645: Line erroneously breaks at backslash
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            // Note that the current behavior deviates from the original fix for this bug.
            //
            // The original report was filed due to wrapping all of "\Program Files\aaaa" to the
            // next line, even though only "aaaa" overflowed. The original fix was to simply make
            // U+005C reverse solidus (backslash) a breaking character.
            //
            // However, the root cause for this bug was not the behavior of '\', but rather some
            // other bug making all of "\Program Files\" behave like a single token, despite it
            // even containing whitespace.
            //
            // Reverting to the ICU line rules fixes this root issue. Now, in the following,
            // "C:\Program" and "Files\LibreOffice" are treated as separate tokens. This is also
            // consistent with the behavior of other office programs.
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                u"C:\\Program Files\\LibreOffice"_ustr, strlen("C:\\Program Files\\Libre"), aLocale, 0,
                aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11), aResult.breakIndex);

            // An identical result should be generated for solidus.
            aResult = m_xBreak->getLineBreak(
                u"C:/Program Files/LibreOffice"_ustr, strlen("C:/Program Files/Libre"), aLocale, 0,
                aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11), aResult.breakIndex);
        }
    }

    // i#80841: Words separated by hyphens will always break to next line
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            // Here we want the line break to leave toll- on the first line
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                u"toll-free"_ustr, strlen("toll-fr"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), aResult.breakIndex);
        }
    }

    // i#83464: Line break between letter and $
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            // Here we want the line break to leave US$ clumped on the next line.
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                u"word US$ 123"_ustr, strlen("word U"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), aResult.breakIndex);
        }
    }

    // Unknown bug number: "fix line break problem of dot after letter and before number"
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            // Here we want the line break to leave US$ clumped on the next line.
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                u"word L.5 word"_ustr, strlen("word L"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5), aResult.breakIndex);
        }
    }

    // i#83229: Wrong line break when word contains a hyphen
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            // The root cause for this bug was the Unicode standard introducing special treatment
            // for '-' in a number range context. This change makes number ranges (e.g. "100-199")
            // behave as if they are single tokens for the purposes of line breaking. Unfortunately,
            // this caused a significant appearance change to existing documents.
            //
            // Despite being a user-visible layout change, this isn't exactly a bug. Wrapping
            // number ranges as a single token is consistent with other applications, including web
            // browsers, and other office suites as mentioned in the bug discussion. Removing this
            // customization seems like it would be a major change, however.
            //
            // Here we want the line break to leave 100- clumped on the first line.

            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                u"word 100-199 word"_ustr, strlen("word 100-1"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(9), aResult.breakIndex);
        }

        {
            // From the same bug: "the leading minus must stay with numbers and strings"

            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                    u"range of -100.000 to 100.000"_ustr, strlen("range of -1"), aLocale, 0,
                    aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(sal_Int32{9}, aResult.breakIndex);

            static constexpr OUString str = u"range of \u2212100.000 to 100.000"_ustr;
            aResult = m_xBreak->getLineBreak(
                    str, strlen("range of -"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(sal_Int32{9}, aResult.breakIndex);
        }

        aLocale.Language = "de";
        aLocale.Country = "DE";

        {
            // From the same bug: "the leading minus must stay with numbers and strings"

            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                    u"EURO is -10,50"_ustr, strlen("EURO is -1"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(sal_Int32{8}, aResult.breakIndex);

            // Also the mathematical minus sign:

            static constexpr OUString str = u"EURO is \u221210,50"_ustr;
            aResult = m_xBreak->getLineBreak(
                    str, strlen("EURO is -"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(sal_Int32{8}, aResult.breakIndex);
        }

        {
            // From the same bug: "the leading minus must stay with numbers and strings"

            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                    u"und -kosten"_ustr, strlen("und -ko"), aLocale, 0,
                    aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(sal_Int32{4}, aResult.breakIndex);

            // But not the non-breaking hyphen:

            static constexpr OUString str = u"und \u2011"_ustr;
            aResult = m_xBreak->getLineBreak(
                    str, strlen("und -ko"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(sal_Int32{5}, aResult.breakIndex);
        }
    }

    // i#83649: "Line break should be between typographical quote and left bracket"
    // - Actually: Spaces between quotation mark and opening punctuation not treated as a break.
    // - Note that per the Unicode standard, prohibiting breaks in this context is intentional
    // because it may cause issues in certain languages due to the various ways quotation
    // characters are used.
    // - We do it anyway by customizing the ICU line breaking rules.
    {
        {
            // This uses the sample text provided in the bug report. Based on usage, it is assumed
            // they were in the de_DE locale.

            aLocale.Language = "de";
            aLocale.Country = "DE";

            // Per the bug report, it is expected that »angetan werden« remains on the first line.
            const OUString str = u"»angetan werden« [Passiv]"_ustr;
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                str, str.getLength() - 4, aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(17), aResult.breakIndex);

            // The same result should be returned for this and the first case.
            const OUString str2 = u"»angetan werden« Passiv"_ustr;
            aResult = m_xBreak->getLineBreak(
                str2, str2.getLength() - 4, aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(17), aResult.breakIndex);

            // Under ICU rules, no amount of spaces would cause this to wrap.
            const OUString str3 = u"»angetan werden«    [Passiv]"_ustr;
            aResult = m_xBreak->getLineBreak(
                str3, str3.getLength() - 4, aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(20), aResult.breakIndex);

            // However, tabs will
            const OUString str4 = u"»angetan werden«\t[Passiv]"_ustr;
            aResult = m_xBreak->getLineBreak(
                str4, str4.getLength() - 4, aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(17), aResult.breakIndex);
        }

        {
            // The same behavior is seen in English

            aLocale.Language = "en";
            aLocale.Country = "US";

            const OUString str = u"\"angetan werden\" [Passiv]"_ustr;
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                str, str.getLength() - 4, aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(17), aResult.breakIndex);

            const OUString str2 = u"\"angetan werden\" Passiv"_ustr;
            aResult = m_xBreak->getLineBreak(
                str2, str2.getLength() - 4, aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(17), aResult.breakIndex);
        }
    }

    // i#72868: Writer/Impress line does not break after Chinese punctuation and Latin letters
    // tdf#130592: Fixed the regression. If this case fails, UI text will be laid out incorrectly.
    {
        aLocale.Language = "zh";
        aLocale.Country = "HK";

        {
            const OUString str = u"word word、word word"_ustr;
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(
                str, strlen("word wordXwor"), aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(10), aResult.breakIndex);
        }
    }

    // i#80891: Character in the forbidden list sometimes appears at the start of line
    {
        aLocale.Language = "zh";
        aLocale.Country = "HK";

        {
            // Per the bug, the ideographic two-dot leader should be a forbidden character. However,
            // this change seems to have been reverted or broken at some point.
            const OUString str = u"電話︰電話"_ustr;
            i18n::LineBreakResults aResult
                = m_xBreak->getLineBreak(str, 2, aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aResult.breakIndex);
        }
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=19716
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            OUString aTest(u"aaa]aaa"_ustr);
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
        static constexpr OUString aTest = u"\uc560\uad6D\uac00\uc758 \uac00"
                                       "\uc0ac\ub294"_ustr;

        aLocale.Language = "ko";
        aLocale.Country = "KR";

        {
            i18n::LineBreakResults aResult = m_xBreak->getLineBreak(aTest, aTest.getLength()-2, aLocale, 0,
                aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected a break don't split the Korean word!", static_cast<sal_Int32>(5), aResult.breakIndex);
        }
    }

    // i#65267: Comma is badly broken at end of line
    // - The word should be wrapped along with the comma
    {
        aLocale.Language = "de";
        aLocale.Country = "DE";

        {
            auto res = m_xBreak->getLineBreak(u"Wort -prinzessinnen, wort"_ustr,
                                              strlen("Wort -prinzessinnen,"), aLocale, 0,
                                              aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(sal_Int32{ 5 }, res.breakIndex);
        }
    }

    // tdf#114160: ZWJ shouldn't be treated as a breaking character
    {
        aLocale.Language = "mn";
        aLocale.Country = "MN";

        {
            auto res = m_xBreak->getLineBreak(u"\u1828\u1820\u200d\u00a0\u200d\u1873\u1873"_ustr, 6,
                                              aLocale, 0, aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), res.breakIndex);
        }

        aLocale.Language = "en";
        aLocale.Country = "US";

        {
            auto res = m_xBreak->getLineBreak(u"AB\u200d\u00a0\u200dCD"_ustr, 6, aLocale, 0,
                                              aHyphOptions, aUserOptions);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), res.breakIndex);
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
        OUString aTest(u"abcd ef  ghi??? KLM"_ustr);

        CPPUNIT_ASSERT(!m_xBreak->isBeginWord(aTest, 4, aLocale, i18n::WordType::DICTIONARY_WORD));
        CPPUNIT_ASSERT(m_xBreak->isEndWord(aTest, 4, aLocale, i18n::WordType::DICTIONARY_WORD));
        aBounds = m_xBreak->getWordBoundary(aTest, 4, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);

        CPPUNIT_ASSERT(!m_xBreak->isBeginWord(aTest, 8, aLocale, i18n::WordType::DICTIONARY_WORD));
        CPPUNIT_ASSERT(!m_xBreak->isEndWord(aTest, 8, aLocale, i18n::WordType::DICTIONARY_WORD));

        //next word
        aBounds = m_xBreak->getWordBoundary(aTest, 8, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(12), aBounds.endPos);

        //previous word
        aBounds = m_xBreak->getWordBoundary(aTest, 8, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aBounds.endPos);

        CPPUNIT_ASSERT(!m_xBreak->isBeginWord(aTest, 12, aLocale, i18n::WordType::DICTIONARY_WORD));
        CPPUNIT_ASSERT(m_xBreak->isEndWord(aTest, 12, aLocale, i18n::WordType::DICTIONARY_WORD));
        aBounds = m_xBreak->getWordBoundary(aTest, 12, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(12), aBounds.endPos);

        CPPUNIT_ASSERT(m_xBreak->isBeginWord(aTest, 16, aLocale, i18n::WordType::DICTIONARY_WORD));
        CPPUNIT_ASSERT(!m_xBreak->isEndWord(aTest, 16, aLocale, i18n::WordType::DICTIONARY_WORD));
        aBounds = m_xBreak->getWordBoundary(aTest, 16, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(19), aBounds.endPos);
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=21907
    {
        OUString aTest(u"b a?"_ustr);

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
        static constexpr OUString aTest =
            u"Working \u201CWords"
            " starting wit"
            "h quotes\u201D Work"
            "ing \u2018Broken\u2019 "
            "?Spanish? doe"
            "sn\u2019t work. No"
            "t even \u00BFreal? "
            "Spanish"_ustr;

        aBounds = m_xBreak->getWordBoundary(aTest, 4, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 12, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 40, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(37), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(44), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 49, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(46), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(52), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 58, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(55), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(62), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 67, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(64), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(71), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 90, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(88), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(92), aBounds.endPos);
    }

    //See https://bugs.libreoffice.org/show_bug.cgi?id=49629
    sal_Unicode aBreakTests[] = { ' ', 1, 2, 3, 4, 5, 6, 7, 0x91, 0x92, 0x200B, 0xE8FF, 0xF8FF };
    for (int mode = i18n::WordType::ANY_WORD; mode <= i18n::WordType::WORD_COUNT; ++mode)
    {
        //make sure that in all cases isBeginWord and isEndWord matches getWordBoundary
        for (auto const& i: aBreakTests)
        {
            OUString aTest = "Word" + OUStringChar(i) + "Word";
            aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale, mode, true);
            switch (mode)
            {
                case i18n::WordType::ANY_WORD:
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);
                    break;
                case i18n::WordType::ANYWORD_IGNOREWHITESPACES:
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);
                    break;
                case i18n::WordType::DICTIONARY_WORD:
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);
                    break;
                case i18n::WordType::WORD_COUNT:
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);
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
        for (auto const& p: aJoinTests)
        {
            OUString aTest = "Word" + OUStringChar(p) + "Word";
            aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale, mode, true);
            switch (mode)
            {
                case i18n::WordType::ANY_WORD:
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.endPos);
                    break;
                case i18n::WordType::ANYWORD_IGNOREWHITESPACES:
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.endPos);
                    break;
                case i18n::WordType::DICTIONARY_WORD:
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.endPos);
                    break;
                case i18n::WordType::WORD_COUNT:
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.endPos);
                    break;
            }

            CPPUNIT_ASSERT(m_xBreak->isBeginWord(aTest, aBounds.startPos, aLocale, mode));
            CPPUNIT_ASSERT(m_xBreak->isEndWord(aTest, aBounds.endPos, aLocale, mode));
        }
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=13494
    {
        static constexpr OUString aBase(u"xxAAxxBBxxCCxx"_ustr);
        const sal_Unicode aTests[] =
        {
            '\'', ';', ',', '.', '!', '@', '#', '%', '&', '*',
            '(', ')', '_', '-', '{', '}', '[', ']', '\"', '/',
            '\\', '?', '~', '$', '+', '^', '=', '<', '>', '|'
        };

        const sal_Int32 aDoublePositions[] = {0, 2, 4, 6, 8, 10, 12, 14};
        for (auto const& r: aTests)
        {
            OUString aTest = aBase.replace('x', r);
            sal_Int32 nPos = -1;
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < std::size(aDoublePositions));
                nPos = m_xBreak->nextWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                CPPUNIT_ASSERT_EQUAL(aDoublePositions[i], nPos);
                ++i;
            }
            while (nPos < aTest.getLength());
            nPos = aTest.getLength();
            i = std::size(aDoublePositions)-1;
            do
            {
                nPos = m_xBreak->previousWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                --i;
                CPPUNIT_ASSERT_EQUAL(aDoublePositions[i], nPos);
            }
            while (nPos > 0);
        }

        const sal_Int32 aSinglePositions[] = {0, 1, 3, 4, 6, 7, 9, 10};
        for (size_t j = 1; j < std::size(aTests); ++j)
        {
            OUString aTest = aBase.replaceAll("xx", OUStringChar(aTests[j]));
            sal_Int32 nPos = -1;
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < std::size(aSinglePositions));
                nPos = m_xBreak->nextWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                CPPUNIT_ASSERT_EQUAL(aSinglePositions[i], nPos);
                ++i;
            }
            while (nPos < aTest.getLength());
            nPos = aTest.getLength();
            i = std::size(aSinglePositions)-1;
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
                CPPUNIT_ASSERT(i < std::size(aSingleQuotePositions));
                nPos = m_xBreak->nextWord(aTest, nPos, aLocale, i18n::WordType::ANYWORD_IGNOREWHITESPACES).startPos;
                CPPUNIT_ASSERT_EQUAL(aSingleQuotePositions[i], nPos);
                ++i;
            }
            while (nPos < aTest.getLength());
            nPos = aTest.getLength();
            i = std::size(aSingleQuotePositions)-1;
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

        OUString aTest(u"mirar-se comprar-vos donem-nos les mans aneu-vos-en!"_ustr);

        sal_Int32 nPos = 0;
        sal_Int32 aExpected[] = {8, 20, 30, 34, 39, 51, 52};
        size_t i = 0;
        do
        {
            CPPUNIT_ASSERT(i < std::size(aExpected));
            nPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                i18n::WordType::DICTIONARY_WORD, true).endPos;
            CPPUNIT_ASSERT_EQUAL(aExpected[i], nPos);
            ++i;
        }
        while (nPos++ < aTest.getLength());
        CPPUNIT_ASSERT_EQUAL(std::size(aExpected), i);
    }

    // i#85411: ZWSP should be a word separator for spellchecking
    // - This fix was applied to both dict and edit customizations
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

        static constexpr OUString aTest = u"I\u200Bwant\u200Bto\u200Bgo"_ustr;

        sal_Int32 nPos = 0;
        sal_Int32 aExpected[] = { 1, 6, 9, 12 };
        size_t i = 0;
        do
        {
            CPPUNIT_ASSERT(i < std::size(aExpected));
            auto dwPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                                                   i18n::WordType::DICTIONARY_WORD, true);
            CPPUNIT_ASSERT_EQUAL(aExpected[i], dwPos.endPos);
            auto ewPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                                                   i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
            CPPUNIT_ASSERT_EQUAL(aExpected[i], ewPos.endPos);
            nPos = dwPos.endPos;
            ++i;
        } while (nPos++ < aTest.getLength());
        CPPUNIT_ASSERT_EQUAL(std::size(aExpected), i);
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

        static constexpr OUString aTest =
            u"\u1F0C\u03BD\u03B4\u03C1\u03B1 \u1F00"
            "\u03C1\u03BD\u1F7B\u03BC\u03B5\u03BD\u03BF"
            "\u03C2 \u1F00\u03BB\u03BB \u1F24"
            "\u03C3\u03B8\u03B9\u03BF\u03BD"_ustr;

        sal_Int32 nPos = 0;
        sal_Int32 aExpected[] = {5, 15, 19, 26};
        size_t i = 0;
        do
        {
            CPPUNIT_ASSERT(i < std::size(aExpected));
            nPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                i18n::WordType::DICTIONARY_WORD, true).endPos;
            CPPUNIT_ASSERT_EQUAL(aExpected[i], nPos);
            ++i;
        }
        while (nPos++ < aTest.getLength());
        CPPUNIT_ASSERT_EQUAL(std::size(aExpected), i);
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=58513
    //See https://bugs.libreoffice.org/show_bug.cgi?id=55707
    {
        aLocale.Language = "fi";
        aLocale.Country = "FI";

        OUString aTest(u"Kuorma-auto kaakkois- ja Keski-Suomi USA:n 90:n %:n"_ustr);

        {
            sal_Int32 nPos = 0;
            sal_Int32 aExpected[] = {11, 21, 24, 36, 42, 47, 51};
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < std::size(aExpected));
                nPos = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                    i18n::WordType::WORD_COUNT, true).endPos;
                CPPUNIT_ASSERT_EQUAL(aExpected[i], nPos);
                ++i;
            }
            while (nPos++ < aTest.getLength());
            CPPUNIT_ASSERT_EQUAL(std::size(aExpected), i);
        }

        {
            sal_Int32 nPos = 0;
            sal_Int32 aExpected[] = {0, 11, 12, 20, 22, 24, 25, 36, 37,
                                    40, 41, 42, 43, 45, 46, 47, 50, 51};
            size_t i = 0;
            do
            {
                CPPUNIT_ASSERT(i < std::size(aExpected));
                aBounds = m_xBreak->getWordBoundary(aTest, nPos, aLocale,
                    i18n::WordType::DICTIONARY_WORD, true);
                CPPUNIT_ASSERT_EQUAL(aExpected[i], aBounds.startPos);
                ++i;
                CPPUNIT_ASSERT_EQUAL(aExpected[i], aBounds.endPos);
                ++i;
                nPos = aBounds.endPos;
            }
            while (nPos++ < aTest.getLength());
            CPPUNIT_ASSERT_EQUAL(std::size(aExpected), i);
        }
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=107843
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        static constexpr OUString aTest =
            u"ru\uFB00le \uFB01sh"_ustr;

        aBounds = m_xBreak->getWordBoundary(aTest, 1, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 7, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.endPos);
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=113785
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        static constexpr OUString aTest =
            u"a\u2013b\u2014c"_ustr;

        aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aBounds.endPos);

        aBounds = m_xBreak->nextWord(aTest, 0, aLocale, i18n::WordType::DICTIONARY_WORD);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.endPos);

        aBounds = m_xBreak->nextWord(aTest, aBounds.endPos, aLocale, i18n::WordType::DICTIONARY_WORD);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);
    }

    // i#55778: Words containing numbers get broken up
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        static constexpr OUString aTest = u"first i18n third"_ustr;

        aBounds
            = m_xBreak->getWordBoundary(aTest, 8, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aBounds.endPos);
    }

    // i#56347: "BreakIterator patch for Hungarian"
    // i#56348: Special chars in first pos not handled by spell checking in Writer (Hungarian)
    // Rules for Hungarian affixes after numbers and certain symbols
    {
        aLocale.Language = "hu";
        aLocale.Country = "HU";

        OUString aTest = u"szavak 15 15-tel 15%-kal €-val szavak"_ustr;

        for (auto mode :
             { i18n::WordType::DICTIONARY_WORD, i18n::WordType::ANYWORD_IGNOREWHITESPACES })
        {
            aBounds = m_xBreak->getWordBoundary(aTest, 2, aLocale, mode, true);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);

            aBounds = m_xBreak->getWordBoundary(aTest, 7, aLocale, mode, true);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aBounds.startPos);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.endPos);

            aBounds = m_xBreak->getWordBoundary(aTest, 11, aLocale, mode, true);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aBounds.startPos);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(16), aBounds.endPos);

            aBounds = m_xBreak->getWordBoundary(aTest, 18, aLocale, mode, true);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(17), aBounds.startPos);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(24), aBounds.endPos);

            aBounds = m_xBreak->getWordBoundary(aTest, 25, aLocale, mode, true);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(25), aBounds.startPos);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(30), aBounds.endPos);

            aBounds = m_xBreak->getWordBoundary(aTest, 27, aLocale, mode, true);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(25), aBounds.startPos);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(30), aBounds.endPos);

            aBounds = m_xBreak->getWordBoundary(aTest, 34, aLocale, mode, true);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(31), aBounds.startPos);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(37), aBounds.endPos);
        }
    }

    // tdf#49885: Upgrade CJ word boundary analysis to ICU frequency-based analysis
    {
        aLocale.Language = "ja";
        aLocale.Country = "JP";

        static constexpr OUString aTest = u"通産省工業技術院北海道工業開発試験所"_ustr;

        aBounds
            = m_xBreak->getWordBoundary(aTest, 9, aLocale, i18n::WordType::DICTIONARY_WORD, false);

        // When using the old LO custom dictionaries, this will select the entire phrase.
        // When using ICU, it will select only 北海道.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aBounds.endPos);
    }

    //  tdf#161737: narrow no-break space at the end of words resulted spelling mistakes
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        OUString aTest(u"L’espace fine insécable\u202F!"_ustr);
        aBounds
            = m_xBreak->getWordBoundary(aTest, 14, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aBounds.startPos);
        // This was 24 (word + NNBSP)
        CPPUNIT_ASSERT_EQUAL(sal_Int32(23), aBounds.endPos);
    }

    //  tdf#161737: narrow no-break space between digits resulted spelling mistakes
    //  as a quick fix, limit NBSP as word-part character only for editing, and not for spell checking
    //  TODO: remove NBSP by the linguistic module or by the spell checking dictionaries to allow
    //  to check numbers with thousand separators and with correct suffix
    {
        aLocale.Language = "en";
        aLocale.Country = "US";

        OUString aTest(u"1\u202F000\u202F000"_ustr);
        aBounds
            = m_xBreak->getWordBoundary(aTest, 2, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        // This was 0 (word + NNBSP)
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.startPos);
        // This was 8 (word + NNBSP)
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);
    }

    //  tdf#161737: narrow no-break space at the end of words resulted spelling mistakes
    {
        aLocale.Language = "hu";
        aLocale.Country = "HU";

        OUString aTest(u"L’espace fine insécable\u202F!"_ustr);
        aBounds
            = m_xBreak->getWordBoundary(aTest, 14, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aBounds.startPos);
        // This was 24 (word + NNBSP)
        CPPUNIT_ASSERT_EQUAL(sal_Int32(23), aBounds.endPos);
    }

    //  tdf#161737: narrow no-break space between digits resulted spelling mistakes
    //  as a quick fix, limit NBSP as word-part character only for editing, and not for spell checking
    //  TODO: remove NBSP by the linguistic module or by the spell checking dictionaries to allow
    //  to check numbers with thousand separators and with correct suffix
    {
        aLocale.Language = "hu";
        aLocale.Country = "HU";

        OUString aTest(u"1\u202F000\u202F000"_ustr);
        aBounds
            = m_xBreak->getWordBoundary(aTest, 2, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        // This was 0 (word + NNBSP)
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.startPos);
        // This was 8 (word + NNBSP)
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);
    }
}

void TestBreakIterator::testSentenceBoundaries()
{
    lang::Locale aLocale;
    aLocale.Language = "en";
    aLocale.Country = "US";

    // Trivial characteristic test for sentence boundary detection
    {
        OUString aTest(u"This is a sentence. This is a different sentence."_ustr);

        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), m_xBreak->beginOfSentence(aTest, 5, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(19), m_xBreak->endOfSentence(aTest, 5, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(20), m_xBreak->beginOfSentence(aTest, 31, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(49), m_xBreak->endOfSentence(aTest, 31, aLocale));
    }

    // i#24098: i18n API beginOfSentence/endOfSentence
    // fix beginOfSentence, ... when cursor is on the beginning of the sentence
    {
        OUString aTest(u"This is a sentence. This is a different sentence."_ustr);

        CPPUNIT_ASSERT_EQUAL(sal_Int32(20), m_xBreak->beginOfSentence(aTest, 20, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(49), m_xBreak->endOfSentence(aTest, 20, aLocale));
    }

    // i#24098: i18n API beginOfSentence/endOfSentence
    // "skip preceding space for beginOfSentence"
    {
        OUString aTest(u"This is a sentence.     This is a different sentence."_ustr);

        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), m_xBreak->beginOfSentence(aTest, 20, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(19), m_xBreak->endOfSentence(aTest, 20, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(24), m_xBreak->beginOfSentence(aTest, 26, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(53), m_xBreak->endOfSentence(aTest, 26, aLocale));
    }

    // i#55063: Sentence selection in Thai should select a space-delimited phrase.
    // - This customization broke at some point. It works in an English locale in a synthetic test
    // like this one, but does not work in the Thai locale, nor on Thai text in practice.
    {
        static constexpr OUString aTest = u"ว้อย โหลยโท่ยคอร์รัปชันโอเพ่นฮอตดอก โปรโมเตอร์"_ustr;

        aLocale.Language = "en";
        aLocale.Country = "US";
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), m_xBreak->beginOfSentence(aTest, 23, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(46), m_xBreak->endOfSentence(aTest, 23, aLocale));

        aLocale.Language = "th";
        aLocale.Country = "TH";
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), m_xBreak->beginOfSentence(aTest, 23, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(46), m_xBreak->endOfSentence(aTest, 23, aLocale));
    }

    // i#55063: Thai phrases should delimit English sentence selection.
    // - This customization broke at some point. It works in an English locale in a synthetic test
    // like this one, but does not work in the Thai locale, nor on Thai text in practice.
    {
        static constexpr OUString aTest = u"ว้อย English usually ends with a period โปรโมเตอร์."_ustr;

        aLocale.Language = "en";
        aLocale.Country = "US";
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), m_xBreak->beginOfSentence(aTest, 23, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(51), m_xBreak->endOfSentence(aTest, 23, aLocale));

        aLocale.Language = "th";
        aLocale.Country = "TH";
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), m_xBreak->beginOfSentence(aTest, 23, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(51), m_xBreak->endOfSentence(aTest, 23, aLocale));
    }

    // i#55063: Characteristic test for English text delimiting Thai phrases (sentences)
    // - English text should not delimit Thai phrases.
    {
        static constexpr OUString aTest = u"Englishโหลยโท่ยคอร์รัปชันโอเพ่นฮอตดอกEnglish"_ustr;

        aLocale.Language = "en";
        aLocale.Country = "US";
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), m_xBreak->beginOfSentence(aTest, 23, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(44), m_xBreak->endOfSentence(aTest, 23, aLocale));

        aLocale.Language = "th";
        aLocale.Country = "TH";
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), m_xBreak->beginOfSentence(aTest, 23, aLocale));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(44), m_xBreak->endOfSentence(aTest, 23, aLocale));
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
        static constexpr OUString aTest = u"\u09AC\u09CD\u09AF"_ustr; // BA HALANT LA

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
        static constexpr OUString aTest = u"\u09B9\u09CD\u09A3\u09BF"_ustr;
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
        static constexpr OUString aTest = u"\u09A4\u09CD\u09AE\u09CD\u09AF"_ustr;
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
        static constexpr OUString aTest = u"\u0B9A\u0BBF\u0BA4\u0BCD\u0BA4\u0BBF\u0BB0\u0BC8"_ustr; // CA VOWELSIGNI TA VIRAMA TA VOWELSIGNI RA VOWELSIGNAI

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
        static constexpr OUString aTest = u"\u0B95\u0BC1"_ustr; // KA VOWELSIGNU

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
        static constexpr OUString aTest =
            u"\u0B9A\u0BBF\u0BA4\u0BCD\u0BA4\u0BBF\u0BB0\u0BC8"_ustr;
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
        static constexpr OUString aText = u"\u05D0\u05B8"_ustr; // ALEF QAMATS

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
        static constexpr OUString aTest = u"\u0936\u0940"_ustr; // SHA VOWELSIGNII

        sal_Int32 nDone=0;
        sal_Int32 nPos = 0;

        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", aTest.getLength(), nPos);
        nPos = m_xBreak->previousCharacters(aTest, aTest.getLength(), aLocale,
            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full grapheme", static_cast<sal_Int32>(0), nPos);
    }

    // tdf#49885: Replace custom Thai implementation with ICU
    {
        aLocale.Language = "th";
        aLocale.Country = "TH";

        static constexpr OUString aTest = u"กำ"_ustr;

        CPPUNIT_ASSERT_EQUAL(sal_Int32{ 2 }, aTest.getLength());

        sal_Int32 nDone = 0;
        sal_Int32 nPos = 0;

        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale, i18n::CharacterIteratorMode::SKIPCELL, 1,
                                        nDone);
        CPPUNIT_ASSERT_EQUAL(aTest.getLength(), nPos);

        nPos = m_xBreak->previousCharacters(aTest, aTest.getLength(), aLocale,
                                            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL(sal_Int32{ 0 }, nPos);
    }

    // Korean may also use grapheme clusters for character composition
    {
        aLocale.Language = "ko";
        aLocale.Country = "KR";

        static constexpr OUString aTest = u"각"_ustr;

        CPPUNIT_ASSERT_EQUAL(sal_Int32{ 3 }, aTest.getLength());

        sal_Int32 nDone = 0;
        sal_Int32 nPos = 0;

        nPos = m_xBreak->nextCharacters(aTest, 0, aLocale, i18n::CharacterIteratorMode::SKIPCELL, 1,
                                        nDone);
        CPPUNIT_ASSERT_EQUAL(aTest.getLength(), nPos);

        nPos = m_xBreak->previousCharacters(aTest, aTest.getLength(), aLocale,
                                            i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
        CPPUNIT_ASSERT_EQUAL(sal_Int32{ 0 }, nPos);
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
        static constexpr OUString aWeaks =
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
            "\u2B30\u2B4C"_ustr; //Miscellaneous Symbols and Arrows

        for (sal_Int32 i = 0; i < aWeaks.getLength(); ++i)
        {
            sal_Int16 nScript = m_xBreak->getScriptType(aWeaks, i);
            OString aMsg =
                "Char 0x" +
                OString::number(static_cast<sal_Int32>(std::u16string_view(aWeaks)[i]), 16) +
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
        static constexpr OUString aAsians =
            //some typical CJK chars
            u"\u4E00\u62FF"
            //The full HalfWidth and FullWidth block has historically been
            //designated as taking the CJK font :-(
            //HalfWidth and FullWidth forms of ASCII 0-9, categorized under
            //UAX24 as "Common" i.e. by that logic WEAK
            "\uFF10\uFF19"
            //HalfWidth and FullWidth forms of ASCII A-z, categorized under
            //UAX25 as "Latin", i.e. by that logic LATIN
            "\uFF21\uFF5A"_ustr;

        for (sal_Int32 i = 0; i < aAsians.getLength(); ++i)
        {
            sal_Int16 nScript = m_xBreak->getScriptType(aAsians, i);
            OString aMsg =
                "Char 0x" +
                OString::number(static_cast<sal_Int32>(std::u16string_view(aAsians)[i]), 16) +
                " should have been asian";
            CPPUNIT_ASSERT_EQUAL_MESSAGE(aMsg.getStr(),
                i18n::ScriptType::ASIAN, nScript);
        }
    }
}

//A test to ensure that our Lao word boundary detection is useful
void TestBreakIterator::testLao()
{
    lang::Locale aLocale;
    aLocale.Language = "lo";
    aLocale.Country = "LA";

    static constexpr OUString aTest = u"\u0e8d\u0eb4\u0e99\u0e94\u0eb5\u0e95\u0ec9\u0ead\u0e99\u0eae\u0eb1\u0e9a"_ustr;
    i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

    aBounds = m_xBreak->getWordBoundary(aTest, aBounds.endPos, aLocale,
        i18n::WordType::DICTIONARY_WORD, true);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
#if (U_ICU_VERSION_MAJOR_NUM < 70)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.endPos);
#else
    // FIXME:
    // In ICU 70/71 for yet unknown reason the word boundary 9 is not detected and
    // instead the length 12 is returned as endpos.
    // Deep in
    // icu_70::RuleBasedBreakIterator::BreakCache::next()
    // icu_70::RuleBasedBreakIterator::BreakCache::following()
    // icu_70::RuleBasedBreakIterator::following()
    // i18npool::BreakIterator_Unicode::getWordBoundary()
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12), aBounds.endPos);
#endif
}

//A test to ensure that our thai word boundary detection is useful
void TestBreakIterator::testThai()
{
    lang::Locale aLocale;
    aLocale.Language = "th";
    aLocale.Country = "TH";

    //See http://lists.freedesktop.org/archives/libreoffice/2012-February/025959.html
    {
        static constexpr OUString aTest = u"\u0E01\u0E38\u0E2B\u0E25\u0E32\u0E1A"_ustr;
        i18n::Boundary aBounds = m_xBreak->getWordBoundary(aTest, 0, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full word",
            sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Should skip full word",
            aTest.getLength(), aBounds.endPos);
    }

    //See https://bz.apache.org/ooo/show_bug.cgi?id=29548
    //make sure forwards and back are consistent
    {
        static constexpr OUString aTest =
            u"\u0E2D\u0E38\u0E17\u0E22\u0E32\u0E19\u0E41"
            "\u0E2B\u0E48\u0E07\u0E0A\u0E32\u0E15\u0E34"
            "\u0E19\u0E49\u0E33\u0E2B\u0E19\u0E32\u0E27"
            "\u0E2D\u0E38\u0E17\u0E22\u0E32\u0E19\u0E41"
            "\u0E2B\u0E48\u0E07\u0E0A\u0E32\u0E15\u0E34"
            "\u0E19\u0E49\u0E33\u0E2B\u0E19\u0E32\u0E27"_ustr;

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
        static constexpr OUString aTest = u"\U00010000"_ustr;

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
        static constexpr OUString aTest = u"シャットダウン"_ustr;

        aBounds = xBreak->getWordBoundary(aTest, 5, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);

        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aBounds.endPos);
    }

    {
        static constexpr OUString aTest = u"\u9EBB\u306E\u8449\u9EBB\u306E\u8449"_ustr;

        aBounds = xBreak->getWordBoundary(aTest, 1, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);

        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.endPos);

        aBounds = xBreak->getWordBoundary(aTest, 5, aLocale,
            i18n::WordType::DICTIONARY_WORD, true);

        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);
    }

    {
        // tdf#162912: Double-clicking should only select one Basic identifier
        static constexpr OUString aTest = u"ThisComponent.CurrentSelection"_ustr;

        aBounds = xBreak->getWordBoundary(aTest, 5, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(30), aBounds.endPos);

        aBounds = xBreak->getWordBoundary(aTest, 5, aLocale,
                                          i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(13), aBounds.endPos);

        aBounds = xBreak->getWordBoundary(aTest, 15, aLocale,
                                          i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(30), aBounds.endPos);
    }
}

void TestBreakIterator::testJapanese()
{
    doTestJapanese(m_xBreak);

    // fdo#78479 - test second / cached instantiation of xdictionary
    uno::Reference< i18n::XBreakIterator > xTmpBreak(m_xSFactory->createInstance(
        u"com.sun.star.i18n.BreakIterator"_ustr), uno::UNO_QUERY_THROW);

    doTestJapanese(xTmpBreak);
}

void TestBreakIterator::testChinese()
{
    lang::Locale stLocale;
    stLocale.Language = "zh";
    stLocale.Country = "CN";

    // Verify dictionary-based word breakiterator
    {
        static constexpr OUStringLiteral aTest = u"\u6A35\u6A30\u69FE\u8919\U00029EDB";

        i18n::Boundary aBounds
            = m_xBreak->getWordBoundary(aTest, 4, stLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);
    }

    // Chinese allows line breaking inside a word
    {
        i18n::LineBreakHyphenationOptions stHyphOptions;
        i18n::LineBreakUserOptions stUserOptions;

        auto aTest = u"手机"_ustr;
        auto stBreak = m_xBreak->getLineBreak(aTest, 1, stLocale, 0, stHyphOptions, stUserOptions);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), stBreak.breakIndex);
    }

    // Characteristic test including built-in forbidden rules
    {
        i18n::LineBreakHyphenationOptions stHyphOptions;
        i18n::LineBreakUserOptions stUserOptions;

        // Comma normally not allowed at start of line, quote normally not allowed at end
        auto aTest = u"水水水、水水水「水水水水水水水水水"_ustr;
        auto stBreak1 = m_xBreak->getLineBreak(aTest, 3, stLocale, 0, stHyphOptions, stUserOptions);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), stBreak1.breakIndex);
        auto stBreak2 = m_xBreak->getLineBreak(aTest, 8, stLocale, 0, stHyphOptions, stUserOptions);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), stBreak2.breakIndex);
    }

    // tdf#117554 Do not break at ZWNBSP
    {
        i18n::LineBreakHyphenationOptions stHyphOptions;
        i18n::LineBreakUserOptions stUserOptions;

        auto aTest = u"手\uFEFF机"_ustr;
        auto stBreak = m_xBreak->getLineBreak(aTest, 2, stLocale, 0, stHyphOptions, stUserOptions);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), stBreak.breakIndex);
    }

    // Characteristic test for hanging punctuation
    {
        i18n::LineBreakHyphenationOptions stHyphOptions;
        i18n::LineBreakUserOptions stUserOptions;
        stUserOptions.allowPunctuationOutsideMargin = true;

        auto aTest = u"水水水、水水水。"_ustr;

        // Comma normally not allowed at start of line. Usually this should wrap the preceding
        // character to the next line, but with hanging punctuation it can overflow the line.
        auto stBreak1 = m_xBreak->getLineBreak(aTest, 3, stLocale, 0, stHyphOptions, stUserOptions);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), stBreak1.breakIndex);

        // With hanging punctuation, the period should be allowed to carry over to the margin.
        auto stBreak2 = m_xBreak->getLineBreak(aTest, 7, stLocale, 0, stHyphOptions, stUserOptions);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), stBreak2.breakIndex);
    }

    // tdf#58604: Test for interaction between line breaks and hanging punctuation
    {
        i18n::LineBreakHyphenationOptions stHyphOptions;
        i18n::LineBreakUserOptions stUserOptions;
        stUserOptions.allowPunctuationOutsideMargin = true;

        auto aTest = u"水水水、\n水水水。"_ustr;

        // Lines should always break after any hanging punctuation
        auto stBreak3 = m_xBreak->getLineBreak(aTest, 3, stLocale, 0, stHyphOptions, stUserOptions);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), stBreak3.breakIndex);

        // Lines should also break after the first-seen hanging punctuation
        auto stBreak4 = m_xBreak->getLineBreak(aTest, 4, stLocale, 0, stHyphOptions, stUserOptions);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), stBreak4.breakIndex);
    }
}

void TestBreakIterator::testKorean()
{
    lang::Locale stLocale;
    stLocale.Language = "ko";
    stLocale.Country = "KR";

    {
        i18n::LineBreakHyphenationOptions stHyphOptions;
        i18n::LineBreakUserOptions stUserOptions;

        auto aTest = u"저는 한국에서 살고 있어요"_ustr;
        auto stBreak = m_xBreak->getLineBreak(aTest, 5, stLocale, 0, stHyphOptions, stUserOptions);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), stBreak.breakIndex);
    }
}

void TestBreakIterator::testDictWordPrepostDash()
{
    std::vector<lang::Locale> aLocale{ { "de", "DE", "" },
                                       { "nds", "DE", "" },
                                       { "nl", "NL", "" },
                                       { "sv", "SE", "" },
                                       { "da", "DK", "" } };

    for (const auto& rLocale : aLocale)
    {
        auto aTest = u"Arbeits- -nehmer"_ustr;

        i18n::Boundary aBounds
            = m_xBreak->getWordBoundary(aTest, 3, rLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aTest, 13, rLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(16), aBounds.endPos);
    }
}

void TestBreakIterator::testDictWordAbbreviation()
{
    std::vector<lang::Locale> aLocale{
        { "en", "US", "" }, // dict_word locale
        { "de", "DE", "" } // dict_word_prepostdash locale
    };

    for (const auto& rLocale : aLocale)
    {
        auto aTest = u"Examples: e.g. i.e. etc. and such"_ustr;

        i18n::Boundary aBounds
            = m_xBreak->getWordBoundary(aTest, 3, rLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aTest, 10, rLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(10), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aTest, 15, rLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(15), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(19), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aTest, 20, rLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(20), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(24), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aTest, 26, rLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(25), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(28), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aTest, 30, rLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(29), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(33), aBounds.endPos);
    }
}

void TestBreakIterator::testHebrewGereshGershaim()
{
    // In Hebrew documents, there are multiple valid ways to represent the geresh and gershaim
    // intra-word punctuation marks. This test exhaustively exercises them.
    //
    // See the following bugs:
    // i#51661: Add quotation mark as middle letter for Hebrew
    // tdf#46950: Spell-checking breaks Hebrew words at intra-word single and double quotes

    lang::Locale aLocale;

    aLocale.Language = "he";
    aLocale.Country = "IL";

    // Unicode U+05F3 HEBREW PUNCTUATION GERESH
    {
        auto aTest = u"ג׳ירפה"_ustr;

        auto aBounds
            = m_xBreak->getWordBoundary(aTest, 3, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 3, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);
    }

    // Apostrophe as geresh
    {
        auto aTest = u"ג'ירפה"_ustr;

        auto aBounds
            = m_xBreak->getWordBoundary(aTest, 3, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 3, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);
    }

    // Right single quote as geresh
    {
        auto aTest = u"ג’ירפה"_ustr;

        auto aBounds
            = m_xBreak->getWordBoundary(aTest, 3, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 3, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);
    }

    // Unicode U+05F4 HEBREW PUNCTUATION GERSHAYIM
    {
        auto aTest = u"דו״ח"_ustr;

        auto aBounds
            = m_xBreak->getWordBoundary(aTest, 2, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 2, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);
    }

    // Double quote as gershayim
    {
        auto aTest = u"דו\"ח"_ustr;

        auto aBounds
            = m_xBreak->getWordBoundary(aTest, 2, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 2, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);
    }

    // Right double quote as gershayim
    {
        auto aTest = u"דו”ח"_ustr;

        auto aBounds
            = m_xBreak->getWordBoundary(aTest, 2, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aTest, 2, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);
    }
}

void TestBreakIterator::testLegacySurrogatePairs()
{
    lang::Locale aLocale;

    aLocale.Language = "ja";
    aLocale.Country = "JP";

    // i#75632: [surrogate pair] Japanese word break does not work properly for surrogate pairs.
    // and many others to address bugs: i#75631 i#75633 i#75412 etc.
    //
    // BreakIterator supports surrogate pairs (UTF-16). This is a simple characteristic test.
    {
        static constexpr OUString aTest = u"X 𠮟 X"_ustr;

        auto aBounds
            = m_xBreak->getWordBoundary(aTest, 1, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aTest, 2, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aTest, 5, aLocale, i18n::WordType::DICTIONARY_WORD, false);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);
    }
}

void TestBreakIterator::testWordCount()
{
    auto fnCountWords = [&](const OUString& aStr, const lang::Locale& aLocale) -> int
    {
        int nWords = 0;
        sal_Int32 nNextPos = 0;
        int nIterGuard = 0;

        if (m_xBreak->isBeginWord(aStr, nNextPos, aLocale, i18n::WordType::WORD_COUNT))
        {
            ++nWords;
        }

        while (true)
        {
            CPPUNIT_ASSERT_MESSAGE("Tripped infinite loop check", ++nIterGuard < 100);

            auto aBounds = m_xBreak->nextWord(aStr, nNextPos, aLocale, i18n::WordType::WORD_COUNT);
            if (aBounds.endPos == aBounds.startPos)
            {
                break;
            }

            nNextPos = aBounds.endPos;
            ++nWords;
        }

        return nWords;
    };

    // i#80815: "Word count differs from MS Word"
    // This is a characteristic test for word count using test data from the linked bug.
    {
        lang::Locale aLocale;
        aLocale.Language = "en";
        aLocale.Country = "US";

        const OUString aStr = u""
                              "test data for word count issue #80815\n"
                              "fo\\\'sforos\n"
                              "archipi\\\'elago\n"
                              "do\\^me\n"
                              "f**k\n"
                              "\n"
                              "battery-driven\n"
                              "and/or\n"
                              "apple(s)\n"
                              "money+opportunity\n"
                              "Micro$oft\n"
                              "\n"
                              "300$\n"
                              "I(not you)\n"
                              "a****n\n"
                              "1+3=4\n"
                              "\n"
                              "aaaaaaa.aaaaaaa\n"
                              "aaaaaaa,aaaaaaa\n"
                              "aaaaaaa;aaaaaaa\n"_ustr;

        CPPUNIT_ASSERT_EQUAL(24, fnCountWords(aStr, aLocale));
    }

    // Test that the switch to upstream ICU for CJ word boundary analysis doesn't change word count.
    {
        lang::Locale aLocale;
        aLocale.Language = "ja";
        aLocale.Country = "JP";

        const OUString aStr = u"Wordの様にワード数をするのにTest\n植松町"_ustr;

        CPPUNIT_ASSERT_EQUAL(8, fnCountWords(aStr, aLocale));
    }

    // tdf#150621 Korean words should be counted individually, rather than by syllable.
    //
    // Per i#80815, the intention for the word count feature is to emulate the behavior of MS Word.
    {
        lang::Locale aLocale;
        aLocale.Language = "ko";
        aLocale.Country = "KR";

        // Basic case: Korean words are counted as space-delimited. In particular, grammatical
        // particles are treated as part of the previous word.
        CPPUNIT_ASSERT_EQUAL(3, fnCountWords(u"저는 영화를 봤어요"_ustr, aLocale));

        // Mixed script: Korean is mostly written in hangul, but hanja are still used in certain
        // situations (e.g. abbreviations in newspaper articles). For Chinese and Japanese, such
        // ideographs would be counted individually as words. In Korean, however, they are treated
        // no differently than hangul characters.
        CPPUNIT_ASSERT_EQUAL(1, fnCountWords(u"불렀다...與"_ustr, aLocale));
        CPPUNIT_ASSERT_EQUAL(2, fnCountWords(u"불렀다 ...與"_ustr, aLocale));
        CPPUNIT_ASSERT_EQUAL(3, fnCountWords(u"불렀다 ... 與"_ustr, aLocale));
        CPPUNIT_ASSERT_EQUAL(1, fnCountWords(u"尹탄핵"_ustr, aLocale));
        CPPUNIT_ASSERT_EQUAL(2, fnCountWords(u"尹 탄핵"_ustr, aLocale));
    }
}

void TestBreakIterator::testDictionaryIteratorLanguages()
{
    // Thai
    {
        lang::Locale aLocale{ "th", "TH", "" };

        const OUString aStr = u"รอนานหรือเปล่า"_ustr;

        i18n::Boundary aBounds;

        aBounds
            = m_xBreak->getWordBoundary(aStr, 1, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aStr, 3, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aStr, 6, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aStr, 10, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 1, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 3, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 6, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 10, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 1, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 3, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 6, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 10, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14), aBounds.endPos);
    }

    // Japanese
    {
        lang::Locale aLocale{ "ja", "JP", "" };

        const OUString aStr = u"通産省工業技術院北海道"_ustr;

        i18n::Boundary aBounds;

        aBounds
            = m_xBreak->getWordBoundary(aStr, 1, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aStr, 2, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aStr, 4, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aStr, 6, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aStr, 7, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aStr, 9, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 1, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 2, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 4, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 6, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 7, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 9, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 1, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 2, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 4, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 6, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 7, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 9, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aBounds.endPos);
    }

    // Chinese
    {
        lang::Locale aLocale{ "zh", "CN", "" };

        const OUString aStr = u"很高兴认识你"_ustr;

        i18n::Boundary aBounds;

        aBounds
            = m_xBreak->getWordBoundary(aStr, 0, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aStr, 1, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aStr, 3, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

        aBounds
            = m_xBreak->getWordBoundary(aStr, 5, aLocale, i18n::WordType::DICTIONARY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 0, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 1, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 3, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 5, aLocale, i18n::WordType::ANY_WORD, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 0, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 1, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 3, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.endPos);

        aBounds = m_xBreak->getWordBoundary(aStr, 5, aLocale,
                                            i18n::WordType::ANYWORD_IGNOREWHITESPACES, true);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aBounds.startPos);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aBounds.endPos);
    }
}

void TestBreakIterator::setUp()
{
    BootstrapFixtureBase::setUp();
    m_xBreak.set(m_xSFactory->createInstance(u"com.sun.star.i18n.BreakIterator"_ustr), uno::UNO_QUERY_THROW);
}

void TestBreakIterator::tearDown()
{
    m_xBreak.clear();
    BootstrapFixtureBase::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestBreakIterator);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
