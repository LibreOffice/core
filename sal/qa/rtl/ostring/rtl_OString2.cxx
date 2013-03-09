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

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/string.hxx>

#include "valueequal.hxx"

namespace rtl_OString
{

class valueOf : public CppUnit::TestFixture
{
    void valueOf_float_test_impl(float _nValue)
        {
            rtl::OString sValue;
            sValue = rtl::OString::valueOf( _nValue );
            printf("nFloat := %.9f  sValue := %s\n", _nValue, sValue.getStr());

            float nValueATOF = static_cast<float>(atof( sValue.getStr() ));

            bool bEqualResult = is_float_equal(_nValue, nValueATOF);
            CPPUNIT_ASSERT_MESSAGE("Values are not equal.", bEqualResult == true);
        }

    void valueOf_float_test(float _nValue)
        {
            valueOf_float_test_impl(_nValue);

            // test also the negative part.
            float nNegativeValue = -_nValue;
            valueOf_float_test_impl(nNegativeValue);
        }

public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    // insert your test code here.
    void valueOf_float_test_001()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);
        float nValue = 3.0f;
        valueOf_float_test(nValue);
    }

    void valueOf_float_test_002()
    {
        float nValue = 3.5f;
        valueOf_float_test(nValue);
    }

    void valueOf_float_test_003()
    {
        float nValue = 3.0625f;
        valueOf_float_test(nValue);
    }

    void valueOf_float_test_004()
    {
        float nValue = 3.502525f;
        valueOf_float_test(nValue);
    }

    void valueOf_float_test_005()
    {
        float nValue = 3.141592f;
        valueOf_float_test(nValue);
    }

    void valueOf_float_test_006()
    {
        float nValue = 3.5025255f;
        valueOf_float_test(nValue);
    }

    void valueOf_float_test_007()
    {
        float nValue = 3.0039062f;
        valueOf_float_test(nValue);
    }

private:

    void valueOf_double_test_impl(double _nValue)
        {
            rtl::OString sValue;
            sValue = rtl::OString::valueOf( _nValue );
            printf("nDouble := %.20f  sValue := %s\n", _nValue, sValue.getStr());

            double nValueATOF = atof( sValue.getStr() );

            bool bEqualResult = is_double_equal(_nValue, nValueATOF);
            CPPUNIT_ASSERT_MESSAGE("Values are not equal.", bEqualResult == true);
        }

    void valueOf_double_test(double _nValue)
        {
            valueOf_double_test_impl(_nValue);

            // test also the negative part.
            double nNegativeValue = -_nValue;
            valueOf_double_test_impl(nNegativeValue);
        }
public:

    // valueOf double
    void valueOf_double_test_001()
        {
            double nValue = 3.0;
            valueOf_double_test(nValue);
        }
    void valueOf_double_test_002()
        {
            double nValue = 3.5;
            valueOf_double_test(nValue);
        }
    void valueOf_double_test_003()
        {
            double nValue = 3.0625;
            valueOf_double_test(nValue);
        }
    void valueOf_double_test_004()
        {
            double nValue = 3.1415926535;
            valueOf_double_test(nValue);
        }
    void valueOf_double_test_005()
        {
            double nValue = 3.141592653589793;
            valueOf_double_test(nValue);
        }
    void valueOf_double_test_006()
        {
            double nValue = 3.1415926535897932;
            valueOf_double_test(nValue);
        }
    void valueOf_double_test_007()
        {
            double nValue = 3.14159265358979323;
            valueOf_double_test(nValue);
        }
    void valueOf_double_test_008()
        {
            double nValue = 3.141592653589793238462643;
            valueOf_double_test(nValue);
        }


    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(valueOf);
    CPPUNIT_TEST(valueOf_float_test_001);
    CPPUNIT_TEST(valueOf_float_test_002);
    CPPUNIT_TEST(valueOf_float_test_003);
    CPPUNIT_TEST(valueOf_float_test_004);
    CPPUNIT_TEST(valueOf_float_test_005);
    CPPUNIT_TEST(valueOf_float_test_006);
    CPPUNIT_TEST(valueOf_float_test_007);

    CPPUNIT_TEST(valueOf_double_test_001);
    CPPUNIT_TEST(valueOf_double_test_002);
    CPPUNIT_TEST(valueOf_double_test_003);
    CPPUNIT_TEST(valueOf_double_test_004);
    CPPUNIT_TEST(valueOf_double_test_005);
    CPPUNIT_TEST(valueOf_double_test_006);
    CPPUNIT_TEST(valueOf_double_test_007);
    CPPUNIT_TEST(valueOf_double_test_008);
    CPPUNIT_TEST_SUITE_END();
}; // class valueOf

// -----------------------------------------------------------------------------
// - toDouble (tests)
// -----------------------------------------------------------------------------
class toDouble : public CppUnit::TestFixture
{

public:

    toDouble()
        {
            // testPrecision a;
        }



    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }

    void toDouble_test_impl(rtl::OString const& _sValue)
        {
            double nValueATOF = atof( _sValue.getStr() );

            // rtl::OUString suValue = rtl::OUString::createFromAscii( _sValue.getStr() );
            double nValueToDouble = _sValue.toDouble();

            bool bEqualResult = is_double_equal(nValueToDouble, nValueATOF);
            CPPUNIT_ASSERT_MESSAGE("Values are not equal.", bEqualResult == true);
        }

    void toDouble_test(rtl::OString const& _sValue)
        {
            toDouble_test_impl(_sValue);

            // test also the negativ part.
            rtl::OString sNegativValue("-");
            sNegativValue += _sValue;
            toDouble_test_impl(sNegativValue);
        }

    // insert your test code here.
    void toDouble_selftest()
        {
            printf("Start selftest:\n");
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.01) == false);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.001) == false);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.0001) == false);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.00001) == false);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.000001) == false);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.0000001) == false);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.00000001) == false);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.000000001) == false);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.0000000001) == false);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.00000000001) == false);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.000000000001) == false);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.0000000000001) == false);
            // we check til 14 values after comma
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.00000000000001) == true);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.000000000000001) == true);
            CPPUNIT_ASSERT (is_double_equal(1.0, 1.0000000000000001) == true);
            printf("Selftest done.\n");
        }

    void toDouble_test_3()
        {
            rtl::OString sValue("3");
            toDouble_test(sValue);
        }
    void toDouble_test_3_5()
        {
            rtl::OString sValue("3.5");
            toDouble_test(sValue);
        }
    void toDouble_test_3_0625()
        {
            rtl::OString sValue("3.0625");
            toDouble_test(sValue);
        }
    void toDouble_test_pi()
        {
            // value from http://www.angio.net/pi/digits/50.txt
            rtl::OString sValue("3.141592653589793238462643383279502884197169399375");
            toDouble_test(sValue);
        }

    void toDouble_test_1()
        {
            rtl::OString sValue("1");
            toDouble_test(sValue);
        }
    void toDouble_test_10()
        {
            rtl::OString sValue("10");
            toDouble_test(sValue);
        }
    void toDouble_test_100()
        {
            rtl::OString sValue("100");
            toDouble_test(sValue);
        }
    void toDouble_test_1000()
        {
            rtl::OString sValue("1000");
            toDouble_test(sValue);
        }
    void toDouble_test_10000()
        {
            rtl::OString sValue("10000");
            toDouble_test(sValue);
        }
    void toDouble_test_1e99()
        {
            rtl::OString sValue("1e99");
            toDouble_test(sValue);
        }
    void toDouble_test_1e_n99()
        {
            rtl::OString sValue("1e-99");
            toDouble_test(sValue);
        }
    void toDouble_test_1e308()
        {
            rtl::OString sValue("1e308");
            toDouble_test(sValue);
        }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(toDouble);
    CPPUNIT_TEST(toDouble_selftest);

    CPPUNIT_TEST(toDouble_test_3);
    CPPUNIT_TEST(toDouble_test_3_5);
    CPPUNIT_TEST(toDouble_test_3_0625);
    CPPUNIT_TEST(toDouble_test_pi);
    CPPUNIT_TEST(toDouble_test_1);
    CPPUNIT_TEST(toDouble_test_10);
    CPPUNIT_TEST(toDouble_test_100);
    CPPUNIT_TEST(toDouble_test_1000);
    CPPUNIT_TEST(toDouble_test_10000);
    CPPUNIT_TEST(toDouble_test_1e99);
    CPPUNIT_TEST(toDouble_test_1e_n99);
    CPPUNIT_TEST(toDouble_test_1e308);
    CPPUNIT_TEST_SUITE_END();
}; // class toDouble

// -----------------------------------------------------------------------------
// - getToken (tests)
// -----------------------------------------------------------------------------
class getToken : public CppUnit::TestFixture
{

public:

    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }

    // -----------------------------------------------------------------------------

    void getToken_000()
        {
            rtl::OString sTokenStr;

            sal_Int32 nIndex = 0;
            do
            {
                rtl::OString sToken = sTokenStr.getToken( 0, ';', nIndex );
            }
            while ( nIndex >= 0 );
            // printf("Index %d\n", nIndex);
            // should not GPF
        }

    void getToken_001()
        {
            rtl::OString sTokenStr = "a;b";

            sal_Int32 nIndex = 0;

            rtl::OString sToken = sTokenStr.getToken( 0, ';', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be a 'a'", sToken.equals("a") == sal_True);

            /* rtl::OString */ sToken = sTokenStr.getToken( 0, ';', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be a 'b'", sToken.equals("b") == sal_True);
            CPPUNIT_ASSERT_MESSAGE("index should be negative", nIndex == -1);
        }

    void getToken_002()
        {
            rtl::OString sTokenStr = "a;b.c";

            sal_Int32 nIndex = 0;

            rtl::OString sToken = sTokenStr.getToken( 0, ';', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be a 'a'", sToken.equals("a") == sal_True);

            /* rtl::OString */ sToken = sTokenStr.getToken( 0, '.', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be a 'b'", sToken.equals("b") == sal_True);

            /* rtl::OString */ sToken = sTokenStr.getToken( 0, '.', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be a 'c'", sToken.equals("c") == sal_True);
            CPPUNIT_ASSERT_MESSAGE("index should be negative", nIndex == -1);
        }

    void getToken_003()
        {
            rtl::OString sTokenStr = "a;;b";

            sal_Int32 nIndex = 0;

            rtl::OString sToken = sTokenStr.getToken( 0, ';', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be a 'a'", sToken.equals("a") == sal_True);

            /* rtl::OString */ sToken = sTokenStr.getToken( 0, ';', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be empty", sToken.isEmpty());

            /* rtl::OString */ sToken = sTokenStr.getToken( 0, ';', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be a 'b'", sToken.equals("b") == sal_True);
            CPPUNIT_ASSERT_MESSAGE("index should be negative", nIndex == -1);
        }

    void getToken_004()
        {
            rtl::OString sTokenStr = "longer.then.ever.";

            sal_Int32 nIndex = 0;

            rtl::OString sToken = sTokenStr.getToken( 0, '.', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be 'longer'", sToken.equals("longer") == sal_True);

            /* rtl::OString */ sToken = sTokenStr.getToken( 0, '.', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be 'then'", sToken.equals("then") == sal_True);

            /* rtl::OString */ sToken = sTokenStr.getToken( 0, '.', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be 'ever'", sToken.equals("ever") == sal_True);

            /* rtl::OString */ sToken = sTokenStr.getToken( 0, '.', nIndex );
            CPPUNIT_ASSERT_MESSAGE("Token should be empty", sToken.isEmpty());

            CPPUNIT_ASSERT_MESSAGE("index should be negative", nIndex == -1);
        }


    CPPUNIT_TEST_SUITE(getToken);
    CPPUNIT_TEST(getToken_000);
    CPPUNIT_TEST(getToken_001);
    CPPUNIT_TEST(getToken_002);
    CPPUNIT_TEST(getToken_003);
    CPPUNIT_TEST(getToken_004);
    CPPUNIT_TEST_SUITE_END();
}; // class getToken

// -----------------------------------------------------------------------------
// testing the method replaceAt( sal_Int32 index, sal_Int32 count,
// const OString& newStr )
// -----------------------------------------------------------------------------

// Developer note: Mindy Liu, 2004-04-23
// stollen from sal/qa/rtl_strings/rtl_OString.cxx

class replaceAt : public CppUnit::TestFixture
{

public:
    // initialise your test code values here.
    void setUp()
        {
        }

    void tearDown()
        {
        }
    sal_Bool check_replaceAt( const rtl::OString* expVal, const rtl::OString* input,
        const rtl::OString* newStr, sal_Int32  index, sal_Int32 count)
    {
         ::rtl::OString aStr1;
        aStr1= input->replaceAt( index, count, *newStr );

        printf("the result OString is %s#\n", aStr1.getStr() );

        sal_Bool bRes = ( expVal->compareTo(aStr1) == 0 );
        return bRes;
    }
    // -----------------------------------------------------------------------------

   void replaceAt_001()
        {
        sal_Bool bRes = check_replaceAt(new rtl::OString("Java@Sun"),
            new rtl::OString("Sun java"), new rtl::OString("Java@Sun"), 0, 8 );
            CPPUNIT_ASSERT_MESSAGE("string differs, replace whole string", bRes == sal_True);
        }

    void replaceAt_002()
        {
            sal_Bool bRes = check_replaceAt(new rtl::OString("Sun Java desktop system"),
            new rtl::OString("Sun "), new rtl::OString("Java desktop system"), 10, 8 );
            CPPUNIT_ASSERT_MESSAGE("index > length of input string", bRes == sal_True);
        }

    void replaceAt_003()
        {
            sal_Bool bRes = check_replaceAt(new rtl::OString("SuJava desktop system"),
            new rtl::OString("Sun "), new rtl::OString("Java desktop system"), 2, 64 );
            CPPUNIT_ASSERT_MESSAGE("larger count", bRes == sal_True);
        }

    void replaceAt_004()
        {

            sal_Bool bRes = check_replaceAt(new rtl::OString("Java desktop system"),
            new rtl::OString("Sun "), new rtl::OString("Java desktop system"), -4, 8 );
            CPPUNIT_ASSERT_MESSAGE("navigate index", bRes == sal_True);
        }
    void replaceAt_005()
        {

            sal_Bool bRes = check_replaceAt(new rtl::OString("Sun Jesktop System"),
            new rtl::OString("Sun Java Desktop System"), new rtl::OString(""), 5, 5 );
            CPPUNIT_ASSERT_MESSAGE("replace with null string", bRes == sal_True);
        }


    CPPUNIT_TEST_SUITE(replaceAt);
    CPPUNIT_TEST(replaceAt_001);
    CPPUNIT_TEST(replaceAt_002);
    CPPUNIT_TEST(replaceAt_003);
    CPPUNIT_TEST(replaceAt_004);
    CPPUNIT_TEST(replaceAt_005);
    CPPUNIT_TEST_SUITE_END();
}; // class replaceAt


// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OString::valueOf);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OString::toDouble);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OString::getToken);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OString::replaceAt);

} // namespace rtl_OString


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
