/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <cmath>

/*
In tdf#148430, we try to replace rtl math functions to std functions,
this unit test is to demonstrate this replacement will not change
the behavior of code and no other unexpected results.

You can see more discussions in https://gerrit.libreoffice.org/c/core/+/138294.
*/

class Test : public CppUnit::TestFixture
{
public:
    void test_erf()
    {
        double x, rtl_res, std_res;
        x = 0.0;
        rtl_res = rtl::math::erf(x);
        std_res = std::erf(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        rtl::math::setInf(&x, false);
        rtl_res = rtl::math::erf(x);
        std_res = std::erf(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        rtl::math::setInf(&x, true);
        rtl_res = rtl::math::erf(x);
        std_res = std::erf(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        rtl::math::setNan(&x);
        rtl_res = rtl::math::erf(x);
        std_res = std::erf(x);
        CPPUNIT_ASSERT_EQUAL(std::isnan(rtl_res), std::isnan(std_res));
        x = 3.0;
        rtl_res = rtl::math::erf(-x);
        std_res = std::erf(-x);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-std::erf(x), rtl_res, 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-rtl::math::erf(x), std_res, 1E-12);
    }

    void test_erfc()
    {
        double x, rtl_res, std_res;
        x = 0.0;
        rtl_res = rtl::math::erfc(x);
        std_res = std::erfc(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        rtl::math::setInf(&x, false);
        rtl_res = rtl::math::erfc(x);
        std_res = std::erfc(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        rtl::math::setInf(&x, true);
        rtl_res = rtl::math::erfc(x);
        std_res = std::erfc(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        rtl::math::setNan(&x);
        rtl_res = rtl::math::erfc(x);
        std_res = std::erfc(x);
        CPPUNIT_ASSERT_EQUAL(std::isnan(rtl_res), std::isnan(std_res));
        x = 3.0;
        rtl_res = rtl::math::erfc(-x);
        std_res = std::erfc(-x);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 - std::erfc(x), rtl_res, 1E-12);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 - rtl::math::erfc(x), std_res, 1E-12);
    }

    void test_expm1()
    {
        double x, rtl_res, std_res;
        x = 0.0;
        rtl_res = rtl::math::expm1(x);
        std_res = std::expm1(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        x = -0.0;
        rtl_res = rtl::math::expm1(x);
        std_res = std::expm1(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        CPPUNIT_ASSERT_EQUAL(std::signbit(rtl_res), std::signbit(std_res));
        rtl::math::setInf(&x, false);
        rtl_res = rtl::math::expm1(x);
        std_res = std::expm1(x);
        CPPUNIT_ASSERT_EQUAL(std::isinf(rtl_res) && !std::signbit(rtl_res),
                             std::isinf(std_res) && !std::signbit(std_res));
        rtl::math::setInf(&x, true);
        rtl_res = rtl::math::expm1(x);
        std_res = std::expm1(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        rtl::math::setNan(&x);
        rtl_res = rtl::math::expm1(x);
        std_res = std::expm1(x);
        CPPUNIT_ASSERT_EQUAL(std::isnan(rtl_res), std::isnan(std_res));
    }

    void test_log1p()
    {
        double x, rtl_res, std_res;
        x = 0.0;
        rtl_res = rtl::math::log1p(x);
        std_res = std::log1p(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        x = -0.0;
        rtl_res = rtl::math::log1p(x);
        std_res = std::log1p(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        CPPUNIT_ASSERT_EQUAL(std::signbit(rtl_res), std::signbit(std_res));
        rtl::math::setInf(&x, false);
        rtl_res = rtl::math::log1p(x);
        std_res = std::log1p(x);
        CPPUNIT_ASSERT_EQUAL(std::isinf(rtl_res) && !std::signbit(rtl_res),
                             std::isinf(std_res) && !std::signbit(std_res));
        x = -1.0;
        rtl_res = rtl::math::log1p(x);
        std_res = std::log1p(x);
        CPPUNIT_ASSERT_EQUAL(std::isinf(rtl_res) && std::signbit(rtl_res),
                             std::isinf(std_res) && std::signbit(std_res));
        x = -1.1;
        rtl_res = rtl::math::log1p(x);
        std_res = std::log1p(x);
        CPPUNIT_ASSERT_EQUAL(std::isnan(rtl_res), std::isnan(std_res));
        rtl::math::setInf(&x, true);
        rtl_res = rtl::math::log1p(x);
        std_res = std::log1p(x);
        CPPUNIT_ASSERT_EQUAL(std::isnan(rtl_res), std::isnan(std_res));
        rtl::math::setNan(&x);
        rtl_res = rtl::math::log1p(x);
        std_res = std::log1p(x);
        CPPUNIT_ASSERT_EQUAL(std::isnan(rtl_res), std::isnan(std_res));
    }

    void test_atanh()
    {
        double x, rtl_res, std_res;
        x = -2.0;
        rtl_res = rtl::math::atanh(x); // NaN
        std_res = std::atanh(x);
        CPPUNIT_ASSERT_EQUAL(std::isnan(rtl_res), std::isnan(std_res));
        x = -1.0;
        rtl_res = rtl::math::atanh(x); // -Inf
        std_res = std::atanh(x);
        CPPUNIT_ASSERT_EQUAL(std::signbit(rtl_res), std::signbit(std_res));
        CPPUNIT_ASSERT_EQUAL(std::isinf(rtl_res), std::isinf(std_res));
        x = 0.0;
        rtl_res = rtl::math::atanh(x);
        std_res = std::atanh(x);
        CPPUNIT_ASSERT_EQUAL(rtl_res, std_res);
        x = 1.0;
        rtl_res = rtl::math::atanh(1.0); // +Inf
        std_res = std::atanh(x);
        CPPUNIT_ASSERT_EQUAL(std::signbit(rtl_res), std::signbit(std_res));
        CPPUNIT_ASSERT_EQUAL(std::isinf(rtl_res), std::isinf(std_res));
        x = 2.0;
        rtl_res = rtl::math::atanh(2.0); // NaN
        std_res = std::atanh(x);
        CPPUNIT_ASSERT_EQUAL(std::isnan(rtl_res), std::isnan(std_res));
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_erf);
    CPPUNIT_TEST(test_erfc);
    CPPUNIT_TEST(test_expm1);
    CPPUNIT_TEST(test_log1p);
    CPPUNIT_TEST(test_atanh);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */