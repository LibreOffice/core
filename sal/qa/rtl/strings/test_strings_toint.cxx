/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace {

template< typename T > class Test: public CppUnit::TestFixture {
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testToInt32Overflow);
    CPPUNIT_TEST(testToInt64Overflow);
    CPPUNIT_TEST(testToUInt64Overflow);
    CPPUNIT_TEST_SUITE_END();

    void testToInt32Overflow() {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), T("-2147483649").toInt32());
        CPPUNIT_ASSERT_EQUAL(SAL_MIN_INT32, T("-2147483648").toInt32());
        CPPUNIT_ASSERT_EQUAL(SAL_MIN_INT32 + 1, T("-2147483647").toInt32());
        CPPUNIT_ASSERT_EQUAL(SAL_MAX_INT32 - 1, T("2147483646").toInt32());
        CPPUNIT_ASSERT_EQUAL(SAL_MAX_INT32, T("2147483647").toInt32());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), T("2147483648").toInt32());
    }

    void testToInt64Overflow() {
        CPPUNIT_ASSERT_EQUAL(sal_Int64(0), T("-9223372036854775809").toInt64());
        CPPUNIT_ASSERT_EQUAL(
            SAL_MIN_INT64, T("-9223372036854775808").toInt64());
        CPPUNIT_ASSERT_EQUAL(
            SAL_MIN_INT64 + 1, T("-9223372036854775807").toInt64());
        CPPUNIT_ASSERT_EQUAL(
            SAL_MAX_INT64 - 1, T("9223372036854775806").toInt64());
        CPPUNIT_ASSERT_EQUAL(SAL_MAX_INT64, T("9223372036854775807").toInt64());
        CPPUNIT_ASSERT_EQUAL(sal_Int64(0), T("9223372036854775808").toInt64());
    }

    void testToUInt64Overflow() {
        CPPUNIT_ASSERT_EQUAL(
            SAL_MAX_UINT64 - 1, T("18446744073709551614").toUInt64());
        CPPUNIT_ASSERT_EQUAL(
            SAL_MAX_UINT64, T("18446744073709551615").toUInt64());
        CPPUNIT_ASSERT_EQUAL(
            sal_uInt64(0), T("18446744073709551616").toUInt64());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test< rtl::OString >);
CPPUNIT_TEST_SUITE_REGISTRATION(Test< rtl::OUString >);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
