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

#include "gtest/gtest.h"
#include "rtl/math.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace {

class Test: public ::testing::Test {
};

TEST_F(Test, test_stringToDouble_good) {
    rtl_math_ConversionStatus status;
    sal_Int32 end;
    double res = rtl::math::stringToDouble(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("  +1.E01foo")),
        sal_Unicode('.'), sal_Unicode(','), &status, &end);
    ASSERT_EQ(rtl_math_ConversionStatus_Ok, status);
    ASSERT_EQ(sal_Int32(RTL_CONSTASCII_LENGTH("  +1.E01")), end);
    ASSERT_EQ(10.0, res);
}

TEST_F(Test, test_stringToDouble_bad) {
    rtl_math_ConversionStatus status;
    sal_Int32 end;
    double res = rtl::math::stringToDouble(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("  +Efoo")),
        sal_Unicode('.'), sal_Unicode(','), &status, &end);
    ASSERT_EQ(rtl_math_ConversionStatus_Ok, status);
    ASSERT_EQ(sal_Int32(0), end);
    ASSERT_EQ(0.0, res);
}

}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
