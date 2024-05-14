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
#include <rtl/math.hxx>
#include <rtl/ustring.hxx>
#include <limits>

template<> inline std::string CPPUNIT_NS::assertion_traits<rtl_math_ConversionStatus>::toString(
    const rtl_math_ConversionStatus& x )
{
    OStringStream ost;
    ost << static_cast<unsigned int>(x);
    return ost.str();
}

namespace {

class Test: public CppUnit::TestFixture {
public:
    void test_stringToDouble_good() {
        rtl_math_ConversionStatus status;
        sal_Int32 end;
        double res = rtl::math::stringToDouble(
            "  +1.E01foo",
            '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(RTL_CONSTASCII_LENGTH("  +1.E01"), end);
        CPPUNIT_ASSERT_EQUAL(10.0, res);

        res = rtl::math::stringToDouble(
                "NaN",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), end);
        CPPUNIT_ASSERT(std::isnan(res));

        res = rtl::math::stringToDouble(
                "NaN1.23",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), end);
        CPPUNIT_ASSERT(std::isnan(res));

        res = rtl::math::stringToDouble(
                "+NaN",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        res = rtl::math::stringToDouble(
                "-NaN",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        res = rtl::math::stringToDouble(
                "+1.#NAN",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), end);
        CPPUNIT_ASSERT(std::isnan(res));
        CPPUNIT_ASSERT(!std::signbit(res));

        res = rtl::math::stringToDouble(
                "-1.#NAN",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), end);
        CPPUNIT_ASSERT(std::isnan(res));
        CPPUNIT_ASSERT(std::signbit(res));

        res = rtl::math::stringToDouble(
                "INF",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_OutOfRange, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), end);
        CPPUNIT_ASSERT(std::isinf(res));

        res = rtl::math::stringToDouble(
                "INF1.23",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_OutOfRange, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), end);
        CPPUNIT_ASSERT(std::isinf(res));

        res = rtl::math::stringToDouble(
                ".5",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), end);
        CPPUNIT_ASSERT_EQUAL(0.5, res);

        res = rtl::math::stringToDouble(
                "5.",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), end);
        CPPUNIT_ASSERT_EQUAL(5.0, res);

        // Leading 0 and group separator.
        res = rtl::math::stringToDouble(
                "0,123",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), end);
        CPPUNIT_ASSERT_EQUAL(123.0, res);

        // Leading 0 and two consecutive group separators are none.
        res = rtl::math::stringToDouble(
                "0,,1",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        // Leading 0 and group separator at end is none.
        res = rtl::math::stringToDouble(
                "0,",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        // Leading 0 and group separator before non-digit is none.
        res = rtl::math::stringToDouble(
                "0,x",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        // Trailing group separator is none.
        res = rtl::math::stringToDouble(
                "1,234,",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), end);
        CPPUNIT_ASSERT_EQUAL(1234.0, res);

        // Group separator followed by non-digit is none.
        res = rtl::math::stringToDouble(
                "1,234,x",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), end);
        CPPUNIT_ASSERT_EQUAL(1234.0, res);

        // Check that the value is the nearest double-precision representation of the decimal 0.0042
        // (it was 0.0042000000000000006 instead of 0.0041999999999999997)
        res = rtl::math::stringToDouble("0,0042", ',', ' ', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(0.0042, res);

        // "- 1" is nothing
        res = rtl::math::stringToDouble("- 1", '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        // "-1E+E" : no exponent
        res = rtl::math::stringToDouble("-1E+E", '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), end);
        CPPUNIT_ASSERT_EQUAL(-1.0, res);

        // "-0" is negative zero
        res = rtl::math::stringToDouble("-0", '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);
        CPPUNIT_ASSERT(std::signbit(res));

        // Compensating: "0.001E311" is 1E308, not overflow/inf
        res = rtl::math::stringToDouble("0.001E311", '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9), end);
        CPPUNIT_ASSERT_EQUAL(1E308, res);

        res = rtl::math::stringToDouble("1E8589934590", '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_OutOfRange, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(12), end);
        CPPUNIT_ASSERT_EQUAL(std::numeric_limits<double>::infinity(), res);

        // DBL_MAX and 4 nextafters
        double fValAfter = DBL_MAX;
        res = rtl::math::stringToDouble("1.7976931348623157e+308", '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(23), end);
        CPPUNIT_ASSERT_EQUAL(fValAfter, res);

        fValAfter = std::nextafter( fValAfter, 0);
        res = rtl::math::stringToDouble("1.7976931348623155e+308", '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(23), end);
        CPPUNIT_ASSERT_EQUAL(fValAfter, res);

        fValAfter = std::nextafter( fValAfter, 0);
        res = rtl::math::stringToDouble("1.7976931348623153e+308", '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(23), end);
        CPPUNIT_ASSERT_EQUAL(fValAfter, res);

        fValAfter = std::nextafter( fValAfter, 0);
        res = rtl::math::stringToDouble("1.7976931348623151e+308", '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(23), end);
        CPPUNIT_ASSERT_EQUAL(fValAfter, res);

        fValAfter = std::nextafter( fValAfter, 0);
        res = rtl::math::stringToDouble("1.7976931348623149e+308", '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(23), end);
        CPPUNIT_ASSERT_EQUAL(fValAfter, res);
    }

    void test_stringToDouble_bad() {
        rtl_math_ConversionStatus status;
        sal_Int32 end;
        double res = rtl::math::stringToDouble(
            "  +Efoo",
            '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        res = rtl::math::stringToDouble(
                ".",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        res = rtl::math::stringToDouble(
                " +.Efoo",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        res = rtl::math::stringToDouble(
                " +,.Efoo",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);

        // Leading group separator is none.
        res = rtl::math::stringToDouble(
                ",1234",
                '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);
    }

    void test_stringToDouble_exponent_without_digit() {
        rtl_math_ConversionStatus status;
        sal_Int32 end;
        double res = rtl::math::stringToDouble(
            "1e",
            '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(RTL_CONSTASCII_LENGTH("1"), end);
        CPPUNIT_ASSERT_EQUAL(1.0, res);
        res = rtl::math::stringToDouble(
            "0e",
            '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(RTL_CONSTASCII_LENGTH("1"), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);
    }

    void test_round() {
        double fVal = 5000000000000001.0;
        CPPUNIT_ASSERT_EQUAL( 5000000000000001.0, rtl::math::round( fVal, 9, rtl_math_RoundingMode_Corrected));

        fVal = 8796093022188.0;
        CPPUNIT_ASSERT_EQUAL( 6093022188.0 , rtl::math::round( fVal, 9, rtl_math_RoundingMode_Corrected) - 8790000000000);

        fVal = 4503599627370491.0;
        CPPUNIT_ASSERT_EQUAL( 4503599627370000.0, rtl::math::round( fVal, -3, rtl_math_RoundingMode_Corrected));

        // test #tdf158190: ROUND(16.83 * 650, 0)
        fVal = 16.83;
        CPPUNIT_ASSERT_EQUAL( 10940.0, rtl::math::round( fVal * 650, 0, rtl_math_RoundingMode_Corrected));
    }

    void test_doubleToString() {
        double fVal = 999999999999999.0;
        sal_Int32 aGroups[3] = { 3, 2, 0 };
        OUString aRes( rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max,
                    '.', aGroups, ',', true));
        CPPUNIT_ASSERT_EQUAL( u"99,99,99,99,99,99,999"_ustr, aRes);

        fVal = 949.0;
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    -2,     // round before decimals
                    '.', aGroups, ',', true);
        CPPUNIT_ASSERT_EQUAL( u"900"_ustr, aRes);

        fVal = 950.0;
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    -2,     // round before decimals
                    '.', aGroups, ',', true);
        CPPUNIT_ASSERT_EQUAL( u"1,000"_ustr, aRes);

        // Check non-ASCII separators: Arabic decimal separator U+066B, thousand separator U+066C
        fVal = 123456.78;
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    2,
                    u'٫', aGroups, u'٬', true);
        CPPUNIT_ASSERT_EQUAL( u"1٬23٬456٫78"_ustr, aRes);

        fVal = 4503599627370495.0;
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.');
        CPPUNIT_ASSERT_EQUAL( u"4503599627370495"_ustr, aRes);

        fVal = 4503599627370496.0;
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    2, '.');
        CPPUNIT_ASSERT_EQUAL( u"4503599627370496.00"_ustr, aRes);

        fVal = -4503599627370496.0;
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    2, '.');
        CPPUNIT_ASSERT_EQUAL( u"-4503599627370496.00"_ustr, aRes);

        fVal = 9007199254740991.0;  // (2^53)-1
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"9007199254740991"_ustr, aRes);

        fVal = 9007199254740992.0;  // (2^53), algorithm switch
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"9.00719925474099E+015"_ustr, aRes);

        fVal = 9007199254740993.0;  // (2^53)+1 would be but is 9007199254740992
        aRes = rtl::math::doubleToUString( fVal,
                    rtl_math_StringFormat_Automatic,
                    rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"9.00719925474099E+015"_ustr, aRes);

        // Test rtl_math_StringFormat_G

        fVal = 0.001234567;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 3, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"0.00123"_ustr, aRes);

        fVal = 123.4567;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 3, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"123"_ustr, aRes);

        fVal = 123.4567;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 4, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"123.5"_ustr, aRes);

        fVal = 99.6;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 3, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"99.6"_ustr, aRes);

        // Expected could be 1E+03 (as 999.6 rounded to 3 significant digits
        // results in 1000 with an exponent equal to significant digits).
        // Currently we don't and output 1000 instead, negligible.
        fVal = 999.6;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 3, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1000"_ustr, aRes);

        fVal = 9999.6;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 3, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1E+004"_ustr, aRes);

        fVal = 12345.6789;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, -3, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.2E+004"_ustr, aRes);

        fVal = 2000000528764739.0;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 15, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"2.00000052876474E+015"_ustr, aRes);

        // The original problem of tdf#160985 was nDecPlaces = 16 with
        // RTL_STR_MAX_VALUEOFDOUBLE - std::size("-x.E-xxx")
        fVal = 2000000528764739.0;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 16, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"2000000528764739"_ustr, aRes);

        fVal = 2000000528764739.0;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 18, '.', false);
        CPPUNIT_ASSERT_EQUAL( u"2000000528764739.00"_ustr, aRes);

        // DBL_MAX and 4 nextafters
        fVal = DBL_MAX;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.7976931348623157E+308"_ustr, aRes);

        fVal = std::nextafter( fVal, 0);
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.7976931348623155E+308"_ustr, aRes);

        fVal = std::nextafter( fVal, 0);
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.7976931348623153E+308"_ustr, aRes);

        fVal = std::nextafter( fVal, 0);
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.7976931348623151E+308"_ustr, aRes);

        fVal = std::nextafter( fVal, 0);
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.797693134862315E+308"_ustr, aRes);
        CPPUNIT_ASSERT_EQUAL(fVal, rtl::math::stringToDouble(aRes, '.', ',')); // Test roundtrip

        // DBL_MAX and 4 nextafters rounded to 15 decimals
        fVal = DBL_MAX;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic, 15, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.797693134862316E+308"_ustr, aRes);

        fVal = std::nextafter( fVal, 0);
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic, 15, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.797693134862316E+308"_ustr, aRes);

        fVal = std::nextafter( fVal, 0);
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic, 15, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.797693134862315E+308"_ustr, aRes);

        fVal = std::nextafter( fVal, 0);
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic, 15, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.797693134862315E+308"_ustr, aRes);

        fVal = std::nextafter( fVal, 0);
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic, 15, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.797693134862315E+308"_ustr, aRes);

        // DBL_MAX rounded to 14 decimals
        fVal = DBL_MAX;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic, 14, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.79769313486232E+308"_ustr, aRes);

        // DBL_MAX rounded to 2 decimals
        fVal = DBL_MAX;
        aRes = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_Automatic, 2, '.', true);
        CPPUNIT_ASSERT_EQUAL( u"1.8E+308"_ustr, aRes);

        // Crashed after commit eae24a9488814e77254d175c11fc4a138c1dbd30
        fVal = 123456.789;
        aRes = rtl::math::doubleToUString(fVal, rtl_math_StringFormat_E, 2, '.', false);
        CPPUNIT_ASSERT_EQUAL(u"1.23E+005"_ustr, aRes);

        fVal = 9.9999999999999929;
        aRes = rtl::math::doubleToUString(fVal, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL(u"9.99999999999999"_ustr, aRes);

        fVal = 0.99999999999999933;
        aRes = rtl::math::doubleToUString(fVal, rtl_math_StringFormat_F, rtl_math_DecimalPlaces_Max, '.', true);
        CPPUNIT_ASSERT_EQUAL(u"0.999999999999999"_ustr, aRes);
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
        CPPUNIT_ASSERT(std::isnan(res));
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
        CPPUNIT_ASSERT(std::isnan(res));
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
        CPPUNIT_ASSERT(std::signbit(res));
        rtl::math::setInf( &x, false);
        res = rtl::math::expm1(x);
        CPPUNIT_ASSERT_EQUAL(true, std::isinf(res) && !std::signbit(res));
        rtl::math::setInf( &x, true);
        res = rtl::math::expm1(x);
        CPPUNIT_ASSERT_EQUAL(-1.0,res);
        rtl::math::setNan( &x);
        res = rtl::math::expm1(x);
        CPPUNIT_ASSERT(std::isnan(res));
    }

    void test_log1p() {
        double x, res;
        x =  0.0;
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT_EQUAL(0.0,res);
        x = -0.0;
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT_EQUAL(-0.0,res);
        CPPUNIT_ASSERT(std::signbit(res));
        rtl::math::setInf( &x, false);
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT_EQUAL(true, std::isinf(res) && !std::signbit(res));
        x = -1.0;
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT_EQUAL(true, std::isinf(res) && std::signbit(res));
        x = -1.1;
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT(std::isnan(res));
        rtl::math::setInf( &x, true);
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT(std::isnan(res));
        rtl::math::setNan( &x);
        res = rtl::math::log1p(x);
        CPPUNIT_ASSERT(std::isnan(res));
    }

    void test_acosh() {
        double res;

        res = rtl::math::acosh(-1.0); // NaN
        CPPUNIT_ASSERT(std::isnan(res));

        res = rtl::math::acosh(0.0); // NaN
        CPPUNIT_ASSERT(std::isnan(res));

        res = rtl::math::acosh(0.5); // NaN
        CPPUNIT_ASSERT(std::isnan(res));

        CPPUNIT_ASSERT_EQUAL(0.0, rtl::math::acosh(1.0));

        res = rtl::math::acosh(std::numeric_limits<double>::infinity()); // +Inf
        CPPUNIT_ASSERT(!std::signbit(res));
        CPPUNIT_ASSERT(std::isinf(res));

        // #i97605
        CPPUNIT_ASSERT_DOUBLES_EQUAL(692.56728736744176, rtl::math::acosh(3e+300), 1e-15);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.014142017775252324, rtl::math::acosh(1.0001), 1e-15);
    }

    void test_asinh() {
        double res;

        res = rtl::math::asinh(-std::numeric_limits<double>::infinity()); // -Inf
        CPPUNIT_ASSERT(std::signbit(res));
        CPPUNIT_ASSERT(std::isinf(res));

        CPPUNIT_ASSERT_EQUAL(0.0, rtl::math::asinh(0.0));

        res = rtl::math::asinh(std::numeric_limits<double>::infinity()); // +Inf
        CPPUNIT_ASSERT(!std::signbit(res));
        CPPUNIT_ASSERT(std::isinf(res));

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
        CPPUNIT_ASSERT(std::isnan(res));

        res = rtl::math::atanh(-1.0); // -Inf
        CPPUNIT_ASSERT(std::signbit(res));
        CPPUNIT_ASSERT(std::isinf(res));

        CPPUNIT_ASSERT_EQUAL(0.0, rtl::math::atanh(0.0));

        res = rtl::math::atanh(1.0); // +Inf
        CPPUNIT_ASSERT(!std::signbit(res));
        CPPUNIT_ASSERT(std::isinf(res));

        res = rtl::math::atanh(2.0); // NaN
        CPPUNIT_ASSERT(std::isnan(res));
    }

    void test_payloadNaN() {
        // Test that a quiet NaN payload is propagated and behaves as we
        // expect. Ideally that could be done with a constexpr in
        // sal/rtl/math.cxx to fail already during compile time instead of make
        // check, but..
        // See
        // https://grouper.ieee.org/groups/msc/ANSI_IEEE-Std-754-2019/background/nan-propagation.pdf
        double fVal1 = std::numeric_limits<double>::quiet_NaN();
        reinterpret_cast<sal_math_Double*>(&fVal1)->nan_parts.fraction_lo = 0xbeef;
        const double fVal2 = 0 + fVal1;
        CPPUNIT_ASSERT(std::isnan(fVal2));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Your platform does not support propagation of NaN payloads.",
                static_cast<sal_uInt32>(0xbeef),
                static_cast<sal_uInt32>(reinterpret_cast<const sal_math_Double*>(&fVal2)->nan_parts.fraction_lo));
        reinterpret_cast<sal_math_Double*>(&fVal1)->nan_parts.fraction_lo = 0xdead;
        const double fVal3 = fVal1 + fVal2;
        // Result is one of the payloaded NaNs but the standard does not
        // specify which.
        CPPUNIT_ASSERT_MESSAGE("Your platform does not support propagation of two combined NaN payloads.",
                0xbeef == reinterpret_cast<const sal_math_Double*>(&fVal3)->nan_parts.fraction_lo ||
                0xdead == reinterpret_cast<const sal_math_Double*>(&fVal3)->nan_parts.fraction_lo);
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_stringToDouble_good);
    CPPUNIT_TEST(test_stringToDouble_bad);
    CPPUNIT_TEST(test_stringToDouble_exponent_without_digit);
    CPPUNIT_TEST(test_round);
    CPPUNIT_TEST(test_doubleToString);
    CPPUNIT_TEST(test_erf);
    CPPUNIT_TEST(test_erfc);
    CPPUNIT_TEST(test_expm1);
    CPPUNIT_TEST(test_log1p);
    CPPUNIT_TEST(test_approx);
    CPPUNIT_TEST(test_acosh);
    CPPUNIT_TEST(test_asinh);
    CPPUNIT_TEST(test_atanh);
    CPPUNIT_TEST(test_payloadNaN);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
