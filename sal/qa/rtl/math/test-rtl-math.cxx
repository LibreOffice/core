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
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "rtl/math.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

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
        CPPUNIT_ASSERT_EQUAL(sal_Int32(RTL_CONSTASCII_LENGTH("  +1.E01")), end);
        CPPUNIT_ASSERT_EQUAL(10.0, res);
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
    }

    void test_stringToDouble_exponent_without_digit() {
        rtl_math_ConversionStatus status;
        sal_Int32 end;
        double res = rtl::math::stringToDouble(
            rtl::OUString("1e"),
            '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(RTL_CONSTASCII_LENGTH("1")), end);
        CPPUNIT_ASSERT_EQUAL(1.0, res);
        res = rtl::math::stringToDouble(
            rtl::OUString("0e"),
            '.', ',', &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(RTL_CONSTASCII_LENGTH("1")), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);
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
        CPPUNIT_ASSERT_EQUAL(true,rtl::math::isNan(x));
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
        CPPUNIT_ASSERT_EQUAL(true,rtl::math::isNan(x));
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
        CPPUNIT_ASSERT_EQUAL(true,rtl::math::isNan(x));
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
        CPPUNIT_ASSERT_EQUAL(true,rtl::math::isNan(x));
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_stringToDouble_good);
    CPPUNIT_TEST(test_stringToDouble_bad);
    CPPUNIT_TEST(test_stringToDouble_exponent_without_digit);
    CPPUNIT_TEST(test_erf);
    CPPUNIT_TEST(test_erfc);
    CPPUNIT_TEST(test_expm1);
    CPPUNIT_TEST(test_log1p);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
