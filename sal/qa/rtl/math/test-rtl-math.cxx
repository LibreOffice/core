/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "precompiled_sal.hxx"
#include "sal/config.h"

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
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
