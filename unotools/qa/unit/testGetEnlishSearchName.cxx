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
    virtual void setUp() SAL_OVERRIDE;
    void testSingleElement();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testSingleElement);

    CPPUNIT_TEST_SUITE_END();
};

void Test::setUp()
{};

void Test::testSingleElement()
{
    {   //lowercase
        printf("GetEnglishSearchFontName( \"SYMBOL\" )");
        OUString test1 = GetEnglishSearchFontName( "SYMBOL" );
        CPPUNIT_ASSERT_EQUAL(test1, OUString("symbol"));
        printf("return %s",test1.getStr());
        //trailingWhitespaces
        printf("GetEnglishSearchFontName( \"SYMBOL    \" )");
        test1 = GetEnglishSearchFontName( "Symbol    " );
        CPPUNIT_ASSERT_EQUAL(test1, OUString("symbol"));
        printf("return %s",test1.getStr());
        //removing Skripts
        printf("GetEnglishSearchFontName( \"SYMBOL(skript)\" )");
        test1 = GetEnglishSearchFontName( "Symbol(skript)" );
        CPPUNIT_ASSERT_EQUAL(test1, OUString("symbol"));
        printf("return %s",test1.getStr());
        //remove Whitespaces between
        printf("GetEnglishSearchFontName( \"SYMBOL (skript)\" )");
        test1 = GetEnglishSearchFontName( "Symbol (skript)" );
        CPPUNIT_ASSERT_EQUAL(test1, OUString("symbol"));
        printf("return %s",test1.getStr());
        //trailingWhitespaces

    }


}




CPPUNIT_TEST_SUITE_REGISTRATION(Test);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
