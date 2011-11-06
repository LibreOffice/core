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



#include "precompiled_comphelper.hxx"
#include "sal/config.h"

#include "comphelper/string.hxx"
#include "testshl/simpleheader.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace {

class Test: public CppUnit::TestFixture {
public:
    void test();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test() {
    rtl::OUString s1(RTL_CONSTASCII_USTRINGPARAM("foobarbar"));
    sal_Int32 n1;
    rtl::OUString s2(
        comphelper::string::searchAndReplace(
            s1, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("baaz")), 0, &n1));
    CPPUNIT_ASSERT(
        s2 == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foobaazbar")));
    CPPUNIT_ASSERT(n1 == 3);
    sal_Int32 n2;
    rtl::OUString s3(
        comphelper::string::searchAndReplace(
            s2, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bz")),
            n1 + RTL_CONSTASCII_LENGTH("baaz"), &n2));
    CPPUNIT_ASSERT(
        s3 == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("foobaazbz")));
    CPPUNIT_ASSERT(n2 == 7);
    sal_Int32 n3;
    rtl::OUString s4(
        comphelper::string::searchAndReplace(
            s3, RTL_CONSTASCII_STRINGPARAM("bar"),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("baz")),
            n2 + RTL_CONSTASCII_LENGTH("bz"), &n3));
    CPPUNIT_ASSERT(s4 == s3);
    CPPUNIT_ASSERT(n3 == -1);
}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

}

NOADDITIONAL;
