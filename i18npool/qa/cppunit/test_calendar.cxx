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

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>
#include <com/sun/star/util/XTextSearch.hpp>
#include <com/sun/star/i18n/XCalendar3.hpp>
#include <com/sun/star/i18n/CalendarFieldIndex.hpp>
#include <com/sun/star/i18n/LocaleCalendar.hpp>
#include <unotest/bootstrapfixturebase.hxx>

#include <boost/scoped_ptr.hpp>
#include <unicode/regex.h>
#include <unicode/calendar.h>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;
using namespace U_ICU_NAMESPACE;
typedef U_ICU_NAMESPACE::UnicodeString IcuUniString;

class TestCalendar : public test::BootstrapFixtureBase
{
public:
    TestCalendar();
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    void testI102294();
    void testICU();
    void testSearches();

    CPPUNIT_TEST_SUITE(TestCalendar);
    CPPUNIT_TEST(testI102294);
    //CPPUNIT_TEST(testICU);
    //CPPUNIT_TEST(testSearches);
    CPPUNIT_TEST_SUITE_END();
private:
    uno::Reference<i18n::XCalendar3> m_xCalendar;
    boost::scoped_ptr<icu::TimeZone> m_pDefaultTimeZone;
};

TestCalendar::TestCalendar()
{
    m_pDefaultTimeZone.reset(icu::TimeZone::createDefault());
}

void TestCalendar::testI102294() {
    icu::TimeZone::adoptDefault(icu::TimeZone::createTimeZone("America/Sao_Paulo"));
    m_xCalendar = i18n::LocaleCalendar::create(m_xContext);

    UErrorCode nErr = U_ZERO_ERROR;
    RegexMatcher* pRegexMatcher;
    sal_uInt32 nSearchFlags = UREGEX_UWORD | UREGEX_CASE_INSENSITIVE;

    OUString aString( "abcdefgh" );
    OUString aPattern( "e" );
    IcuUniString aSearchPat( (const UChar*)aPattern.getStr(), aPattern.getLength() );

    pRegexMatcher = new RegexMatcher( aSearchPat, nSearchFlags, nErr );

    IcuUniString aSource( (const UChar*)aString.getStr(), aString.getLength() );
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

    IcuUniString aSearchPat2( (const UChar*)aPattern2.getStr(), aPattern2.getLength() );
    pRegexMatcher = new RegexMatcher( aSearchPat2, nSearchFlags, nErr );

    IcuUniString aSource2( (const UChar*)aString2.getStr(), aString2.getLength() );
    pRegexMatcher->reset( aSource2 );

    CPPUNIT_ASSERT( pRegexMatcher->find( 0, nErr ) );
    CPPUNIT_ASSERT( nErr == U_ZERO_ERROR );
    CPPUNIT_ASSERT( pRegexMatcher->start( nErr ) == 0 );
    CPPUNIT_ASSERT( nErr == U_ZERO_ERROR );
    CPPUNIT_ASSERT( pRegexMatcher->end( nErr ) == 1 );
    CPPUNIT_ASSERT( nErr == U_ZERO_ERROR );
    delete pRegexMatcher;
}

void TestCalendar::setUp()
{
    BootstrapFixtureBase::setUp();
}

void TestCalendar::tearDown()
{
    m_xCalendar.clear();
    icu::TimeZone::setDefault(*m_pDefaultTimeZone);
    BootstrapFixtureBase::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestCalendar);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
