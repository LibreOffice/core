/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"
#ifdef WIN32
// LLA: take a look into Microsofts math.h implementation, why this define is need
#define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <rtl/math.h>
#include <rtl/string.hxx>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "valueequal.hxx"

namespace rtl_math
{

class test : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }


    void equalCheck(double _nResult, double _nExpect) /* throws Exception */
        {
            bool bEqualResult = is_double_equal(_nResult, _nExpect);

            rtl::OString sError = "rtl_math_round expected result is wrong should:(";
            sError += rtl::OString::valueOf(_nExpect);
            sError += ") but is:(";
            sError += rtl::OString::valueOf(_nResult);
            sError += ")";

            CPPUNIT_ASSERT_MESSAGE(sError.getStr(), bEqualResult == true);
        }

    // insert your test code here.
    void round_000()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nValue = M_PI;
        double nResult = 0.0;

        nResult = rtl_math_round(nValue, 0, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(3.0));

        nResult = rtl_math_round(nValue, 2, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(3.14));

        nResult = rtl_math_round(nValue, 3, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(3.142));

        nResult = rtl_math_round(nValue, 10, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(3.1415926536));
    }

    // insert your test code here.
    void round_001_positiv()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(0.1, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.11, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.13, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.14, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.1499999, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.15, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.151, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.16, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.199999999, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(0.2));
    }

    void round_001_negativ()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(-0.1, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.11, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.13, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.14, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.1499999, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.15, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.151, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.16, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.19999999999, 1, rtl_math_RoundingMode_Corrected);
        equalCheck(nResult, double(-0.2));
    }
// -----------------------------------------------------------------------------
    void round_002_positiv()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(0.1, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.11, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.13, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.14, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.1499999, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.15, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.151, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.16, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.199999999, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(0.1));
    }

    void round_002_negativ()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(-0.1, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.11, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.13, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.14, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.1499999, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.15, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.151, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.16, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.19999999999, 1, rtl_math_RoundingMode_Down);
        equalCheck(nResult, double(-0.1));
    }
// -----------------------------------------------------------------------------
    void round_003_positiv()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(0.1, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.11, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.13, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.14, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.1499999, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.15, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.151, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.16, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.199999999, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(0.2));
    }

    void round_003_negativ()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(-0.1, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.11, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.13, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.14, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.1499999, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.15, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.151, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.16, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.19999999999, 1, rtl_math_RoundingMode_Up);
        equalCheck(nResult, double(-0.2));
    }
// -----------------------------------------------------------------------------
    void round_004_positiv()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(0.1, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.11, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.13, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.14, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.1499999, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.15, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.151, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.16, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.199999999, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(0.1));
    }

    void round_004_negativ()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(-0.1, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.11, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.13, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.14, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.1499999, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.15, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.151, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.16, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.19999999999, 1, rtl_math_RoundingMode_Floor);
        equalCheck(nResult, double(-0.2));
    }
// -----------------------------------------------------------------------------
    void round_005_positiv()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(0.1, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.11, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.13, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.14, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.1499999, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.15, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.151, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.16, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.199999999, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(0.2));
    }

    void round_005_negativ()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(-0.1, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.11, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.13, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.14, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.1499999, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.15, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.151, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.16, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.19999999999, 1, rtl_math_RoundingMode_Ceiling);
        equalCheck(nResult, double(-0.1));
    }
// -----------------------------------------------------------------------------
    void round_006_positiv()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(0.1, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.11, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.13, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.14, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.1499999, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.15, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.151, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.16, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.199999999, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(0.2));
    }

    void round_006_negativ()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(-0.1, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.11, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.13, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.14, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.1499999, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.15, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.151, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.16, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.19999999999, 1, rtl_math_RoundingMode_HalfDown);
        equalCheck(nResult, double(-0.2));
    }
// -----------------------------------------------------------------------------
    void round_007_positiv()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(0.1, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.11, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.13, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.14, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.1499999, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(0.1));

        nResult = rtl_math_round(0.15, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.151, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.16, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(0.2));

        nResult = rtl_math_round(0.199999999, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(0.2));
    }

    void round_007_negativ()
    {
        // this is demonstration code
        // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);

        double nResult = 0.0;

        nResult = rtl_math_round(-0.1, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.11, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.13, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.14, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.1499999, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(-0.1));

        nResult = rtl_math_round(-0.15, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.151, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.16, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(-0.2));

        nResult = rtl_math_round(-0.19999999999, 1, rtl_math_RoundingMode_HalfUp);
        equalCheck(nResult, double(-0.2));
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(test);
    CPPUNIT_TEST(round_000);

    CPPUNIT_TEST(round_001_positiv);
    CPPUNIT_TEST(round_001_negativ);

    CPPUNIT_TEST(round_002_positiv);
    CPPUNIT_TEST(round_002_negativ);

    CPPUNIT_TEST(round_003_positiv);
    CPPUNIT_TEST(round_003_negativ);

    CPPUNIT_TEST(round_004_positiv);
    CPPUNIT_TEST(round_004_negativ);

    CPPUNIT_TEST(round_005_positiv);
    CPPUNIT_TEST(round_005_negativ);

    CPPUNIT_TEST(round_006_positiv);
    CPPUNIT_TEST(round_006_negativ);

    CPPUNIT_TEST(round_007_positiv);
    CPPUNIT_TEST(round_007_negativ);

    CPPUNIT_TEST_SUITE_END();
}; // class test

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_math::test);
} // namespace rtl_math


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
