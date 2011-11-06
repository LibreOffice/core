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
#include "precompiled_sal.hxx"

#include "testshl/simpleheader.hxx"
#include "rtl/string.h"
#include "rtl/ustring.hxx"

namespace test { namespace oustring {

class Compare: public CppUnit::TestFixture
{
private:
    void equalsIgnoreAsciiCaseAscii();

    CPPUNIT_TEST_SUITE(Compare);
    CPPUNIT_TEST(equalsIgnoreAsciiCaseAscii);
    CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test::oustring::Compare, "alltest");

void test::oustring::Compare::equalsIgnoreAsciiCaseAscii()
{
    CPPUNIT_ASSERT(!rtl::OUString().equalsIgnoreAsciiCaseAscii("abc"));
    CPPUNIT_ASSERT(!rtl::OUString().equalsIgnoreAsciiCaseAsciiL(
                       RTL_CONSTASCII_STRINGPARAM("abc")));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAscii(""));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("")));

    CPPUNIT_ASSERT(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAscii("abc"));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abcd")).
                   equalsIgnoreAsciiCaseAscii("abc"));
    CPPUNIT_ASSERT(!rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("abc")).
                   equalsIgnoreAsciiCaseAscii("abcd"));
}
