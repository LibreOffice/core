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

#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/XTextSearch2.hpp>
#include <unotest/bootstrapfixturebase.hxx>
#include <i18nutil/transliteration.hxx>

#include <unicode/regex.h>

using namespace ::com::sun::star;
typedef U_ICU_NAMESPACE::UnicodeString IcuUniString;

class TestTextSearch : public test::BootstrapFixtureBase
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void testICU();
    void testSearches();
    void testWildcardSearch();
    void testApostropheSearch();

    CPPUNIT_TEST_SUITE(TestTextSearch);
    CPPUNIT_TEST(testICU);
    CPPUNIT_TEST(testSearches);
    CPPUNIT_TEST(testWildcardSearch);
    CPPUNIT_TEST(testApostropheSearch);
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference<util::XTextSearch> m_xSearch;
    uno::Reference<util::XTextSearch2> m_xSearch2;
};

// Sanity check our ICU first ...
void TestTextSearch::testICU()
{
    UErrorCode nErr = U_ZERO_ERROR;
    sal_uInt32 nSearchFlags = UREGEX_UWORD | UREGEX_CASE_INSENSITIVE;

    OUString aString( "abcdefgh" );
    OUString aPattern( "e" );
    IcuUniString aSearchPat( reinterpret_cast<const UChar*>(aPattern.getStr()), aPattern.getLength() );

    std::unique_ptr<icu::RegexMatcher> pRegexMatcher(new icu::RegexMatcher( aSearchPat, nSearchFlags, nErr ));

    IcuUniString aSource( reinterpret_cast<const UChar*>(aString.getStr()), aString.getLength() );
    pRegexMatcher->reset( aSource );

    CPPUNIT_ASSERT( pRegexMatcher->find( 0, nErr ) );
    CPPUNIT_ASSERT_EQUAL( U_ZERO_ERROR, nErr );
    CPPUNIT_ASSERT_EQUAL( static_cast<int32_t>(4), pRegexMatcher->start( nErr ) );
    CPPUNIT_ASSERT_EQUAL( U_ZERO_ERROR, nErr );
    CPPUNIT_ASSERT_EQUAL( static_cast<int32_t>(5), pRegexMatcher->end( nErr ) );
    CPPUNIT_ASSERT_EQUAL( U_ZERO_ERROR, nErr );

    OUString aString2( "acababaabcababadcdaa" );
    OUString aPattern2( "a" );

    IcuUniString aSearchPat2( reinterpret_cast<const UChar*>(aPattern2.getStr()), aPattern2.getLength() );
    pRegexMatcher.reset(new icu::RegexMatcher( aSearchPat2, nSearchFlags, nErr ));

    IcuUniString aSource2( reinterpret_cast<const UChar*>(aString2.getStr()), aString2.getLength() );
    pRegexMatcher->reset( aSource2 );

    CPPUNIT_ASSERT( pRegexMatcher->find( 0, nErr ) );
    CPPUNIT_ASSERT_EQUAL( U_ZERO_ERROR, nErr );
    CPPUNIT_ASSERT_EQUAL( static_cast<int32_t>(0), pRegexMatcher->start( nErr ) );
    CPPUNIT_ASSERT_EQUAL( U_ZERO_ERROR, nErr );
    CPPUNIT_ASSERT_EQUAL( static_cast<int32_t>(1), pRegexMatcher->end( nErr ) );
    CPPUNIT_ASSERT_EQUAL( U_ZERO_ERROR, nErr );
}

void TestTextSearch::testSearches()
{
    OUString str( "acababaabcababadcdaa" );
    sal_Int32 startPos = 2, endPos = 20 ;
    sal_Int32 const fStartRes = 10, fEndRes = 18 ;
    sal_Int32 const bStartRes = 18, bEndRes = 10 ;

    // set options
    util::SearchOptions aOptions;
    aOptions.algorithmType = util::SearchAlgorithms_REGEXP ;
    aOptions.searchFlag = util::SearchFlags::ALL_IGNORE_CASE;
    aOptions.searchString = "(ab)*a(c|d)+";
    m_xSearch->setOptions( aOptions );

    util::SearchResult aRes;

    // search forward
    aRes = m_xSearch->searchForward( str, startPos, endPos );
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT_EQUAL( fStartRes, aRes.startOffset[0] );
    CPPUNIT_ASSERT_EQUAL( fEndRes, aRes.endOffset[0] );

    // search backwards
    aRes = m_xSearch->searchBackward( str, endPos, startPos );
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT_EQUAL( bStartRes, aRes.startOffset[0] );
    CPPUNIT_ASSERT_EQUAL( bEndRes, aRes.endOffset[0] );

    aOptions.transliterateFlags = static_cast<int>(TransliterationFlags::IGNORE_CASE
                                | TransliterationFlags::IGNORE_WIDTH);
    aOptions.searchString = "([^ ]*)[ ]*([^ ]*)";
    m_xSearch->setOptions(aOptions);
    aRes = m_xSearch->searchForward("11 22 33", 2, 7);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aRes.endOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRes.startOffset[1]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRes.endOffset[1]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aRes.startOffset[2]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aRes.endOffset[2]);
}

void TestTextSearch::testWildcardSearch()
{
    util::SearchOptions2 aOptions;
    OUString aText;
    util::SearchResult aRes;

    aOptions.AlgorithmType2 = util::SearchAlgorithms2::WILDCARD ;
    aOptions.WildcardEscapeCharacter = '~';
    // aOptions.searchFlag = ::css::util::SearchFlags::WILD_MATCH_SELECTION;
    // is not set, so substring match is allowed.
    aOptions.transliterateFlags = sal_Int32(::css::i18n::TransliterationModules::TransliterationModules_IGNORE_CASE);
    aText = "abAca";

    aOptions.searchString = "a";
    m_xSearch2->setOptions2( aOptions );
    // match first "a", [0,1)
    aRes = m_xSearch2->searchForward( aText, 0, aText.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRes.endOffset[0]);
    // match last "a", (5,4]
    aRes = m_xSearch2->searchBackward( aText, aText.getLength(), 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRes.endOffset[0]);

    aOptions.searchString = "a?";
    m_xSearch2->setOptions2( aOptions );
    // match "ab", [0,2)
    aRes = m_xSearch2->searchForward( aText, 0, aText.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRes.endOffset[0]);
    // match "ac", (4,2]
    aRes = m_xSearch2->searchBackward( aText, aText.getLength(), 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRes.endOffset[0]);

    aOptions.searchString = "a*c";
    m_xSearch2->setOptions2( aOptions );
    // match "abac", [0,4) XXX NOTE: first match forward
    aRes = m_xSearch2->searchForward( aText, 0, aText.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRes.endOffset[0]);
    // match "ac", (4,2] XXX NOTE: first match backward, not greedy
    aRes = m_xSearch2->searchBackward( aText, aText.getLength(), 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRes.endOffset[0]);

    aOptions.searchString = "b*a";
    m_xSearch2->setOptions2( aOptions );
    // match "ba", [1,3) XXX NOTE: first match forward, not greedy
    aRes = m_xSearch2->searchForward( aText, 0, aText.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aRes.endOffset[0]);
    // match "baca", (5,1] XXX NOTE: first match backward
    aRes = m_xSearch2->searchBackward( aText, aText.getLength(), 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRes.endOffset[0]);

    aText = "ab?ca";

    aOptions.searchString = "?~??";
    m_xSearch2->setOptions2( aOptions );
    // match "b?c", [1,4)
    aRes = m_xSearch2->searchForward( aText, 0, aText.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRes.endOffset[0]);
    // match "b?c", (4,1]
    aRes = m_xSearch2->searchBackward( aText, aText.getLength(), 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRes.endOffset[0]);

    aText = "ab*ca";

    aOptions.searchString = "?~*?";
    m_xSearch2->setOptions2( aOptions );
    // match "b?c", [1,4)
    aRes = m_xSearch2->searchForward( aText, 0, aText.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRes.endOffset[0]);
    // match "b?c", (4,1]
    aRes = m_xSearch2->searchBackward( aText, aText.getLength(), 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRes.endOffset[0]);

    aOptions.searchString = "ca?";
    m_xSearch2->setOptions2( aOptions );
    // no match
    aRes = m_xSearch2->searchForward( aText, 0, aText.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aRes.subRegExpressions);
    // no match
    aRes = m_xSearch2->searchBackward( aText, aText.getLength(), 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aRes.subRegExpressions);

    aOptions.searchString = "ca*";
    m_xSearch2->setOptions2( aOptions );
    // match "ca", [3,5)
    aRes = m_xSearch2->searchForward( aText, 0, aText.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aRes.endOffset[0]);
    // match "ca", (5,3]
    aRes = m_xSearch2->searchBackward( aText, aText.getLength(), 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aRes.endOffset[0]);

    aOptions.searchString = "*ca*";
    m_xSearch2->setOptions2( aOptions );
    // match "abaca", [0,5)
    aRes = m_xSearch2->searchForward( aText, 0, aText.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aRes.endOffset[0]);
    // match "abaca", (5,0]
    aRes = m_xSearch2->searchBackward( aText, aText.getLength(), 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRes.endOffset[0]);

    aText = "123123";
    aOptions.searchString = "*2?";
    m_xSearch2->setOptions2( aOptions );
    // match first "123", [0,3)
    aRes = m_xSearch2->searchForward( aText, 0, aText.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aRes.endOffset[0]);
    // match "123123", (6,0]    Yes this looks odd, but it is as searching "?2*" forward.
    aRes = m_xSearch2->searchBackward( aText, aText.getLength(), 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRes.endOffset[0]);

    aOptions.searchFlag |= util::SearchFlags::WILD_MATCH_SELECTION;
    m_xSearch2->setOptions2( aOptions );
    // match "123123", [0,6) with greedy '*'
    aRes = m_xSearch2->searchForward( aText, 0, aText.getLength());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aRes.endOffset[0]);
    // match "123123", (6,0]
    aRes = m_xSearch2->searchBackward( aText, aText.getLength(), 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aRes.subRegExpressions);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), aRes.startOffset[0]);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aRes.endOffset[0]);
}

void TestTextSearch::testApostropheSearch()
{
    // A) find typographic apostrophes also by using ASCII apostrophe in searchString
    OUString str( u"It\u2019s an apostrophe." );
    sal_Int32 startPos = 0, endPos = str.getLength();

    // set options
    util::SearchOptions aOptions;
    aOptions.algorithmType = util::SearchAlgorithms_ABSOLUTE;
    aOptions.searchFlag = util::SearchFlags::ALL_IGNORE_CASE;
    aOptions.searchString = "'";
    m_xSearch->setOptions( aOptions );

    util::SearchResult aRes;

    // search forward
    aRes = m_xSearch->searchForward( str, startPos, endPos );
    // This was 0.
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(2), aRes.startOffset[0] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(3), aRes.endOffset[0] );

    // search backwards
    aRes = m_xSearch->searchBackward( str, endPos, startPos );
    // This was 0.
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(3), aRes.startOffset[0] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(2), aRes.endOffset[0] );

    // check with transliteration
    aOptions.transliterateFlags = static_cast<int>(TransliterationFlags::IGNORE_CASE
                                | TransliterationFlags::IGNORE_WIDTH);
    m_xSearch->setOptions(aOptions);

    // search forward
    aRes = m_xSearch->searchForward( str, startPos, endPos );
    // This was 0.
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(2), aRes.startOffset[0] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(3), aRes.endOffset[0] );

    // search backwards
    aRes = m_xSearch->searchBackward( str, endPos, startPos );
    // This was 0.
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(3), aRes.startOffset[0] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(2), aRes.endOffset[0] );

    // B) search ASCII apostrophe in a text with ASCII apostrophes
    str = str.replace(u'\u2019', '\'');

    // search forward
    aRes = m_xSearch->searchForward( str, startPos, endPos );
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(2), aRes.startOffset[0] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(3), aRes.endOffset[0] );

    // search backwards
    aRes = m_xSearch->searchBackward( str, endPos, startPos );
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(3), aRes.startOffset[0] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(2), aRes.endOffset[0] );

    // C) search typographic apostrophe in a text with ASCII apostrophes (no result)
    aOptions.searchString = OUString(u"\u2019");
    m_xSearch->setOptions( aOptions );

    aRes = m_xSearch->searchForward( str, startPos, endPos );
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aRes.subRegExpressions);

    aRes = m_xSearch->searchBackward( str, endPos, startPos );
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aRes.subRegExpressions);

    // D) search typographic apostrophe in a text with typographic apostrophes
    str = str.replace('\'', u'\u2019');

    // search forward
    aRes = m_xSearch->searchForward( str, startPos, endPos );
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(2), aRes.startOffset[0] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(3), aRes.endOffset[0] );

    // search backwards
    aRes = m_xSearch->searchBackward( str, endPos, startPos );
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(3), aRes.startOffset[0] );
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int32>(2), aRes.endOffset[0] );

    // E) search mixed apostrophes in a text with mixed apostrophes:
    aOptions.searchString = OUString(u"'\u2019");
    m_xSearch->setOptions( aOptions );
    str = u"test: \u2019'";

    // search forward
    aRes = m_xSearch->searchForward( str, startPos, str.getLength());
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );

    // search backwards
    aRes = m_xSearch->searchBackward( str, str.getLength(), startPos );
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );

    // F) search mixed apostrophes in a text with ASCII apostrophes:
    str = u"test: ''";

    // search forward
    aRes = m_xSearch->searchForward( str, startPos, str.getLength());
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );

    // search backwards
    aRes = m_xSearch->searchBackward( str, str.getLength(), startPos );
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
}

void TestTextSearch::setUp()
{
    BootstrapFixtureBase::setUp();
    m_xSearch.set(m_xSFactory->createInstance("com.sun.star.util.TextSearch"), uno::UNO_QUERY_THROW);
    m_xSearch2.set(m_xSFactory->createInstance("com.sun.star.util.TextSearch2"), uno::UNO_QUERY_THROW);
}

void TestTextSearch::tearDown()
{
    m_xSearch.clear();
    m_xSearch2.clear();
    BootstrapFixtureBase::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestTextSearch);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
