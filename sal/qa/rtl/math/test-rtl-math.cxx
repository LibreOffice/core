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
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <rtl/math.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <limits>

CPPUNIT_NS_BEGIN

template<> struct assertion_traits<rtl_math_ConversionStatus>
{
    static bool equal( const rtl_math_ConversionStatus& x, const rtl_math_ConversionStatus& y )
    {
        return x == y;
    }

    static std::string toString( const rtl_math_ConversionStatus& x )
    {
        OStringStream ost;
        ost << static_cast<unsigned int>(x);
        return ost.str();
    }
};

CPPUNIT_NS_END

namespace {

class Test: public CppUnit::TestFixture {
public:
    void test_stringToDouble_good() {
        rtl_math_ConversionStatus status;
        sal_Int32 end;
        double res = rtl::math::stringToDouble(
            rtl::OUString("  +1.E01foo"),
            '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(RTL_CONSTASCII_LENGTH("  +1.E01"), end);
        CPPUNIT_ASSERT_EQUAL(10.0, res);

        res = rtl::math::stringToDouble(
                rtl::OUString("NaN"),
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), end);
        CPPUNIT_ASSERT_EQUAL(rtl::math::isNan(res), true);

        res = rtl::math::stringToDouble(
                rtl::OUString("NaN1.23"),
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), end);
        CPPUNIT_ASSERT_EQUAL(rtl::math::isNan(res), true);

        res = rtl::math::stringToDouble(
                rtl::OUString("INF"),
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_OutOfRange, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), end);
        CPPUNIT_ASSERT_EQUAL(rtl::math::isInf(res), true);

        res = rtl::math::stringToDouble(
                rtl::OUString("INF1.23"),
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_OutOfRange, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), end);
        CPPUNIT_ASSERT_EQUAL(rtl::math::isInf(res), true);

        res = rtl::math::stringToDouble(
                rtl::OUString(".5"),
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), end);
        CPPUNIT_ASSERT_EQUAL(0.5, res);

        res = rtl::math::stringToDouble(
                rtl::OUString("5."),
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), end);
        CPPUNIT_ASSERT_EQUAL(5.0, res);
    }

    void test_stringToDouble_bad() {
        rtl_math_ConversionStatus status;
        sal_Int32 end;
        double res = rtl::math::stringToDouble(
            rtl::OUString("  +Efoo"),
            '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        res = rtl::math::stringToDouble(
                rtl::OUString("."),
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        res = rtl::math::stringToDouble(
                rtl::OUString(" +.Efoo"),
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        res = rtl::math::stringToDouble(
                rtl::OUString(" +,.Efoo"),
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);
    }

    void test_stringToDouble_exponent_without_digit() {
        rtl_math_ConversionStatus status;
        sal_Int32 end;
        double res = rtl::math::stringToDouble(
            rtl::OUString("1e"),
            '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(RTL_CONSTASCII_LENGTH("1"), end);
        CPPUNIT_ASSERT_EQUAL(1.0, res);
        res = rtl::math::stringToDouble(
            rtl::OUString("0e"),
            '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(RTL_CONSTASCII_LENGTH("1"), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);
    }

    void test_doubleToString() {
        double fVal = 999999999999999.0;
        sal_Int32 aGroups[3] = { 3, 2, 0 };
        rtl::OUString aRes( rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max,
                    '.', aGroups, ',', true));
        CPPUNIT_ASSERT_EQUAL( OUString("99,99,99,99,99,99,999"), aRes);

        fVal = 949.0;
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    -2,     // round before decimals
                    '.', aGroups, ',', true);
        CPPUNIT_ASSERT_EQUAL( OUString("900"), aRes);

        fVal = 950.0;
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    -2,     // round before decimals
                    '.', aGroups, ',', true);
        CPPUNIT_ASSERT_EQUAL( OUString("1,000"), aRes);

        fVal = 4503599627370495.0;
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.');
        CPPUNIT_ASSERT_EQUAL( OUString("4503599627370495"), aRes);

        fVal = 4503599627370496.0;
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    2, '.');
        CPPUNIT_ASSERT_EQUAL( OUString("4503599627370496.00"), aRes);

        fVal = 9007199254740991.0;  // (2^53)-1
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL( OUString("9007199254740991"), aRes);

        fVal = 9007199254740992.0;  // (2^53), algorithm switch
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL( OUString("9.00719925474099E+015"), aRes);

        fVal = 9007199254740993.0;  // (2^53)+1 would be but is 9007199254740992
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL( OUString("9.00719925474099E+015"), aRes);

        // Test rtl_math_StringFormat_G

        fVal = 0.001234567;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 3, '.', true);
        CPPUNIT_ASSERT_EQUAL( OUString("0.00123"), aRes);

        fVal = 123.4567;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 3, '.', true);
        CPPUNIT_ASSERT_EQUAL( OUString("123"), aRes);

        fVal = 123.4567;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 4, '.', true);
        CPPUNIT_ASSERT_EQUAL( OUString("123.5"), aRes);

        fVal = 99.6;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 3, '.', true);
        CPPUNIT_ASSERT_EQUAL( OUString("99.6"), aRes);

        // Expected could be 1E+03 (as 999.6 rounded to 3 significant digits
        // results in 1000 with an exponent equal to significant digits).
        // Currently we don't and output 1000 instead, negligible.
        fVal = 999.6;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 3, '.', true);
        CPPUNIT_ASSERT_EQUAL( OUString("1000"), aRes);

        fVal = 9999.6;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 3, '.', true);
        CPPUNIT_ASSERT_EQUAL( OUString("1E+004"), aRes);

        fVal = 12345.6789;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, -3, '.', true);
        CPPUNIT_ASSERT_EQUAL( OUString("1.2E+004"), aRes);
    }

    void test_approx() {
        // (2^53)-1 , (2^53)-3
        CPPUNIT_ASSERT_EQUAL( false, rtl::math::approxEqual( 9007199254740991.0, 9007199254740989.0));
        // (2^53)-1 , (2^53)-2
        CPPUNIT_ASSERT_EQUAL( false, rtl::math::approxEqual( 9007199254740991.0, 9007199254740990.0));
        // Note: the following are internally represented as 900719925474099.12
        // and 900719925474098.88 and the difference is 0.25 ...
        CPPUNIT_ASSERT_EQUAL( true, rtl::math::approxEqual( 900719925474099.1, 900719925474098.9));
        CPPUNIT_ASSERT_EQUAL( true, rtl::math::approxEqual( 72.944444444444443, 72.9444444444444));
        CPPUNIT_ASSERT_EQUAL( true, rtl::math::approxEqual( 359650.27322404372, 359650.27322404401));
        CPPUNIT_ASSERT_EQUAL( true, rtl::math::approxEqual( 5.3590326375710063e+238, 5.3590326375710109e+238));
        CPPUNIT_ASSERT_EQUAL( true, rtl::math::approxEqual( 7.4124095894894475e+158, 7.4124095894894514e+158));
        CPPUNIT_ASSERT_EQUAL( true, rtl::math::approxEqual( 1.2905754687023132e+79, 1.2905754687023098e+79));
        CPPUNIT_ASSERT_EQUAL( true, rtl::math::approxEqual( 3.5612905090455637e+38, 3.5612905090455599e+38));
        // 0.3 - 0.2 - 0.1 == 0.0
        CPPUNIT_ASSERT_EQUAL( 0.0, rtl::math::approxSub( rtl::math::approxSub( 0.3, 0.2), 0.1));
        // ((2^53)-1) - ((2^53)-2) == 1.0
        CPPUNIT_ASSERT_EQUAL( 1.0, rtl::math::approxSub( 9007199254740991.0, 9007199254740990.0));
        // (3^31) - ((3^31)-1) == 1.0
        CPPUNIT_ASSERT_EQUAL( 1.0, rtl::math::approxSub( 617673396283947.0, 617673396283946.0));
    }

    void test_erf() {
        double x, res;
        x =  0.0;
        res = rtl::math::erf(x);
        CPPUNIT_ASSERT_EQUAL(0.0,res);
        rtl::math::setInf( &x, false);
        res = rtl::math::erf(x);
        CPPUNIT_ASSERT_EQUAL(1.0,res);
        rtl::math::setInf( &x, true);
        res = rtl::math::erf(x);
        CPPUNIT_ASSERT_EQUAL(-1.0,res);
        rtl::math::setNan( &x);
        res = rtl::math::erf(x);
        CPPUNIT_ASSERT_EQUAL(true,rtl::math::isNan(res));
        x = 3.0;
        res = rtl::math::erf(-x);
        CPPUNIT_ASSERT_DOUBLES_EQUAL( -rtl::math::erf(x), res, 1E-12);
    }

    void test_erfc() {
        double x, res;
        x =  0.0;
        res = rtl::math::erfc(x);
        CPPUNIT_ASSERT_EQUAL(1.0,res);
        rtl::math::setInf( &x, false);
        res = rtl::math::erfc(x);
        CPPUNIT_ASSERT_EQUAL(0.0,res);
        rtl::math::setInf( &x, true);
        res = rtl::math::erfc(x);
        CPPUNIT_ASSERT_EQUAL(2.0,res);
        rtl::math::setNan( &x);
        res = rtl::math::erfc(x);
        CPPUNIT_ASSERT_EQUAL(true,rtl::math::isNan(res));
        x = 3.0;
        res = rtl::math::erfc(-x);
        CPPUNIT_ASSERT_DOUBLES_EQUAL( 2.0 - rtl::math::erfc(x), res, 1E-12);
    }

    void test_expm1() {
        double x, res;
        x =  0.0;
        res = rtl::math::expm1(x);
        CPPUNIT_ASSERT_EQUAL(0.0,res);
        x = -0.0;
        res = rtl::math::expm1(x);
        CPPUNIT_ASSERT_EQUAL(-0.0,res);
        CPPUNIT_ASSERT_EQUAL(true, rtl::math::isSignBitSet(res));
        rtl::math::setInf( &x, false);
        res = rtl::math::expm1(x);
        CPPUNIT_ASSERT_EQUAL(true, rtl::math::isInf(res) && !rtl::math::isSignBitSet(res));
        rtl::math::setInf( &x, true);
        res = rtl::math::expm1(x);
        CPPUNIT_ASSERT_EQUAL(-1.0,res);
        rtl::math::setNan( &x);
        res = rtl::math::expm1(x);
        CPPUNIT_ASSERT_EQUAL(true,rtl::math::isNan(res));
    }

    void test_log1p() {
        double x, res;
        x =  0.0;
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT_EQUAL(0.0,res);
        x = -0.0;
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT_EQUAL(-0.0,res);
        CPPUNIT_ASSERT_EQUAL(true, rtl::math::isSignBitSet(res));
        rtl::math::setInf( &x, false);
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT_EQUAL(true, rtl::math::isInf(res) && !rtl::math::isSignBitSet(res));
        x = -1.0;
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT_EQUAL(true, rtl::math::isInf(res) && rtl::math::isSignBitSet(res));
        x = -1.1;
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT_EQUAL(true, rtl::math::isNan(res));
        rtl::math::setInf( &x, true);
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT_EQUAL(true, rtl::math::isNan(res));
        rtl::math::setNan( &x);
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT_EQUAL(true,rtl::math::isNan(res));
    }

    void test_acosh() {
        double res;

        res = rtl::math::acosh(-1.0); // NaN
        CPPUNIT_ASSERT(rtl::math::isNan(res));

        res = rtl::math::acosh(0.0); // NaN
        CPPUNIT_ASSERT(rtl::math::isNan(res));

        res = rtl::math::acosh(0.5); // NaN
        CPPUNIT_ASSERT(rtl::math::isNan(res));

        CPPUNIT_ASSERT_EQUAL(0.0, rtl::math::acosh(1.0));

        res = rtl::math::acosh(std::numeric_limits<double>::infinity()); // +Inf
        CPPUNIT_ASSERT(!rtl::math::isSignBitSet(res));
        CPPUNIT_ASSERT(rtl::math::isInf(res));

        // #i97605
        CPPUNIT_ASSERT_DOUBLES_EQUAL(692.56728736744176, rtl::math::acosh(3e+300), 1e-15);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.014142017775252324, rtl::math::acosh(1.0001), 1e-15);
    }

    void test_asinh() {
        double res;

        res = rtl::math::asinh(-std::numeric_limits<double>::infinity()); // -Inf
        CPPUNIT_ASSERT(rtl::math::isSignBitSet(res));
        CPPUNIT_ASSERT(rtl::math::isInf(res));

        CPPUNIT_ASSERT_EQUAL(0.0, rtl::math::asinh(0.0));

        res = rtl::math::asinh(std::numeric_limits<double>::infinity()); // +Inf
        CPPUNIT_ASSERT(!rtl::math::isSignBitSet(res));
        CPPUNIT_ASSERT(rtl::math::isInf(res));

        // #i97605
        CPPUNIT_ASSERT_DOUBLES_EQUAL(691.67568924815798, rtl::math::asinh(1.23e+300), 1e-15);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0350378961923076, rtl::math::asinh(1.23), 1e-16);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.23e-300, rtl::math::asinh(1.23e-300), 1e-303);

        // asinh is an odd function
        CPPUNIT_ASSERT_EQUAL(-rtl::math::asinh(1.23e+300), rtl::math::asinh(-1.23e+300));
        CPPUNIT_ASSERT_EQUAL(-rtl::math::asinh(1.23), rtl::math::asinh(-1.23));
        CPPUNIT_ASSERT_EQUAL(-rtl::math::asinh(1.23e-300), rtl::math::asinh(-1.23e-300));
    }

    void test_atanh() {
        double res;

        res = rtl::math::atanh(-2.0); // NaN
        CPPUNIT_ASSERT(rtl::math::isNan(res));

        res = rtl::math::atanh(-1.0); // -Inf
        CPPUNIT_ASSERT(rtl::math::isSignBitSet(res));
        CPPUNIT_ASSERT(rtl::math::isInf(res));

        CPPUNIT_ASSERT_EQUAL(0.0, rtl::math::atanh(0.0));

        res = rtl::math::atanh(1.0); // +Inf
        CPPUNIT_ASSERT(!rtl::math::isSignBitSet(res));
        CPPUNIT_ASSERT(rtl::math::isInf(res));

        res = rtl::math::atanh(2.0); // NaN
        CPPUNIT_ASSERT(rtl::math::isNan(res));
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_stringToDouble_good);
    CPPUNIT_TEST(test_stringToDouble_bad);
    CPPUNIT_TEST(test_stringToDouble_exponent_without_digit);
    CPPUNIT_TEST(test_doubleToString);
    CPPUNIT_TEST(test_erf);
    CPPUNIT_TEST(test_erfc);
    CPPUNIT_TEST(test_expm1);
    CPPUNIT_TEST(test_log1p);
    CPPUNIT_TEST(test_approx);
    CPPUNIT_TEST(test_acosh);
    CPPUNIT_TEST(test_asinh);
    CPPUNIT_TEST(test_atanh);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
