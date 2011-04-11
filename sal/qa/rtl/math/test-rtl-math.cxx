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

#include "sal/config.h"

#include <sal/cppunit.h>
#include "rtl/math.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace {

class Test: public CppUnit::TestFixture {
public:
    void test_stringToDouble_good() {
        rtl_math_ConversionStatus status;
        sal_Int32 end;
        double res = rtl::math::stringToDouble(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("  +1.E01foo")),
            sal_Unicode('.'), sal_Unicode(','), &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(RTL_CONSTASCII_LENGTH("  +1.E01")), end);
        CPPUNIT_ASSERT_EQUAL(10.0, res);
    }

    void test_stringToDouble_bad() {
        rtl_math_ConversionStatus status;
        sal_Int32 end;
        double res = rtl::math::stringToDouble(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("  +Efoo")),
            sal_Unicode('.'), sal_Unicode(','), &status, &end);
        CPPUNIT_ASSERT_EQUAL(rtl_math_ConversionStatus_Ok, status);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), end);
        CPPUNIT_ASSERT_EQUAL(0.0, res);
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_stringToDouble_good);
    CPPUNIT_TEST(test_stringToDouble_bad);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();
