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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include <cstddef>

#include "gtest/gtest.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "../../source/deployment/inc/dp_version.hxx"

namespace {

class Test: public ::testing::Test {
public:
};

TEST_F(Test, test) {
    struct Data {
        rtl::OUString version1;
        rtl::OUString version2;
        ::dp_misc::Order order;
    };
    static Data const data[] = {
        { rtl::OUString(),
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0.0000.00.0")),
          ::dp_misc::EQUAL },
        { rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".01")),
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0.1")),
          ::dp_misc::EQUAL },
        { rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("10")),
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("2")),
          ::dp_misc::GREATER },
        { rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("9223372036854775808")),
              // 2^63
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("9223372036854775807")),
          ::dp_misc::GREATER }
    };
    for (::std::size_t i = 0; i < sizeof data / sizeof (Data); ++i) {
        ASSERT_EQ(
            data[i].order,
            ::dp_misc::compareVersions(data[i].version1, data[i].version2));
        static ::dp_misc::Order const reverse[3] = {
            ::dp_misc::GREATER, ::dp_misc::EQUAL, ::dp_misc::LESS
        };
        ASSERT_EQ(
            reverse[data[i].order],
            ::dp_misc::compareVersions(data[i].version2, data[i].version1));
    }
}


}
