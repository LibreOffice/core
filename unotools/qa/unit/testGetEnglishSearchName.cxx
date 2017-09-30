/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "cppunit/TestCase.h"
#include "cppunit/TestFixture.h"
#include "cppunit/TestSuite.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <unotools/fontdefs.hxx>

class Test: public CppUnit::TestFixture
{
public:
    void testSingleElement();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testSingleElement);

    CPPUNIT_TEST_SUITE_END();
};

void Test::testSingleElement()
{
    // lowercase
    OUString test1 = GetEnglishSearchFontName( "SYMBOL" );
    CPPUNIT_ASSERT_EQUAL( OUString("symbol"),test1);
    //trailing whitespaces
    test1 = GetEnglishSearchFontName( "Symbol    " );
    CPPUNIT_ASSERT_EQUAL(OUString("symbol"),test1);
    //no longer remove script suffixes
    test1 = GetEnglishSearchFontName( "Symbol(SIP)" );
    CPPUNIT_ASSERT_EQUAL(OUString("symbol(sip)"),test1);
    test1 = GetEnglishSearchFontName( "CM Roman CE" );
    CPPUNIT_ASSERT_EQUAL( OUString("cmromance"),test1);
    //remove special characters; leave semicolon, numbers
    test1 = GetEnglishSearchFontName( "sy;mb?=ol129" );
    CPPUNIT_ASSERT_EQUAL( OUString("sy;mbol129"),test1);

    //transformation

    sal_Unicode const transfor[] ={ 0x30D2, 0x30E9, 0x30AE, 0x30CE, 0x4E38, 0x30B4, 'p','r','o','n',0};

    test1 = GetEnglishSearchFontName(transfor );
    CPPUNIT_ASSERT_EQUAL( OUString("hiraginomarugothicpron"),test1);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
