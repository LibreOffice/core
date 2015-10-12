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

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>
#include <com/sun/star/util/XTextSearch.hpp>
#include <unotest/bootstrapfixturebase.hxx>

#include <unicode/regex.h>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;
using namespace U_ICU_NAMESPACE;
typedef U_ICU_NAMESPACE::UnicodeString IcuUniString;

class TestTextSearch : public test::BootstrapFixtureBase
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void testICU();
    void testSearches();

    CPPUNIT_TEST_SUITE(TestTextSearch);
    CPPUNIT_TEST(testICU);
    CPPUNIT_TEST(testSearches);
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference<util::XTextSearch> m_xSearch;
};

// Sanity check our ICU first ...
void TestTextSearch::testICU()
{
    UErrorCode nErr = U_ZERO_ERROR;
    RegexMatcher* pRegexMatcher;
    sal_uInt32 nSearchFlags = UREGEX_UWORD | UREGEX_CASE_INSENSITIVE;

    OUString aString( "abcdefgh" );
    OUString aPattern( "e" );
    IcuUniString aSearchPat( reinterpret_cast<const UChar*>(aPattern.getStr()), aPattern.getLength() );

    pRegexMatcher = new RegexMatcher( aSearchPat, nSearchFlags, nErr );

    IcuUniString aSource( reinterpret_cast<const UChar*>(aString.getStr()), aString.getLength() );
    pRegexMatcher->reset( aSource );

    CPPUNIT_ASSERT( pRegexMatcher->find( 0, nErr ) );
    CPPUNIT_ASSERT( nErr == U_ZERO_ERROR );
    CPPUNIT_ASSERT( pRegexMatcher->start( nErr ) == 4 );
    CPPUNIT_ASSERT( nErr == U_ZERO_ERROR );
    CPPUNIT_ASSERT( pRegexMatcher->end( nErr ) == 5 );
    CPPUNIT_ASSERT( nErr == U_ZERO_ERROR );

    delete pRegexMatcher;

    OUString aString2( "acababaabcababadcdaa" );
    OUString aPattern2( "a" );

    IcuUniString aSearchPat2( reinterpret_cast<const UChar*>(aPattern2.getStr()), aPattern2.getLength() );
    pRegexMatcher = new RegexMatcher( aSearchPat2, nSearchFlags, nErr );

    IcuUniString aSource2( reinterpret_cast<const UChar*>(aString2.getStr()), aString2.getLength() );
    pRegexMatcher->reset( aSource2 );

    CPPUNIT_ASSERT( pRegexMatcher->find( 0, nErr ) );
    CPPUNIT_ASSERT( nErr == U_ZERO_ERROR );
    CPPUNIT_ASSERT( pRegexMatcher->start( nErr ) == 0 );
    CPPUNIT_ASSERT( nErr == U_ZERO_ERROR );
    CPPUNIT_ASSERT( pRegexMatcher->end( nErr ) == 1 );
    CPPUNIT_ASSERT( nErr == U_ZERO_ERROR );
    delete pRegexMatcher;
}

void TestTextSearch::testSearches()
{
    OUString str( "acababaabcababadcdaa" );
    sal_Int32 startPos = 2, endPos = 20 ;
    OUString searchStr( "(ab)*a(c|d)+" );
    sal_Int32 fStartRes = 10, fEndRes = 18 ;
    sal_Int32 bStartRes = 18, bEndRes = 10 ;

    // set options
    util::SearchOptions aOptions;
    aOptions.algorithmType = util::SearchAlgorithms_REGEXP ;
    aOptions.searchFlag = util::SearchFlags::ALL_IGNORE_CASE;
    aOptions.searchString = searchStr;
    m_xSearch->setOptions( aOptions );

    util::SearchResult aRes;

    // search forward
    aRes = m_xSearch->searchForward( str, startPos, endPos );
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT( aRes.startOffset[0] == fStartRes );
    CPPUNIT_ASSERT( aRes.endOffset[0] == fEndRes );

    // search backwards
    aRes = m_xSearch->searchBackward( str, endPos, startPos );
    CPPUNIT_ASSERT( aRes.subRegExpressions > 0 );
    CPPUNIT_ASSERT( aRes.startOffset[0] == bStartRes );
    CPPUNIT_ASSERT( aRes.endOffset[0] == bEndRes );
}

void TestTextSearch::setUp()
{
    BootstrapFixtureBase::setUp();
    m_xSearch = uno::Reference< util::XTextSearch >(m_xSFactory->createInstance(
        "com.sun.star.util.TextSearch"), uno::UNO_QUERY_THROW);
}

void TestTextSearch::tearDown()
{
    m_xSearch.clear();
    BootstrapFixtureBase::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestTextSearch);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
