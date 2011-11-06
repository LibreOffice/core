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
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace test { namespace oustring {

class EndsWith: public CppUnit::TestFixture
{
private:
    void endsWith();

    CPPUNIT_TEST_SUITE(EndsWith);
    CPPUNIT_TEST(endsWith);
    CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test::oustring::EndsWith, "alltest");

namespace {

void appendString(rtl::OStringBuffer & buffer, rtl::OString const & string)
{
    buffer.append('"');
    for (int i = 0; i < string.getLength(); ++i) {
        char c = string[i];
        if (c < ' ' || c == '"' || c == '\\' || c > '~') {
            buffer.append('\\');
            sal_Int32 n = static_cast< sal_Int32 >(
                static_cast< unsigned char >(c));
            if (n < 16) {
                buffer.append('0');
            }
            buffer.append(n, 16);
        } else {
            buffer.append(c);
        }
    }
    buffer.append('"');
}

}

void test::oustring::EndsWith::endsWith()
{
    struct Data {
        char const * str1;
        sal_Int32 str1Len;
        char const * str2;
        sal_Int32 str2Len;
        bool endsWith;
    };
    Data const data[] = {
        { RTL_CONSTASCII_STRINGPARAM(""), RTL_CONSTASCII_STRINGPARAM(""),
          true },
        { RTL_CONSTASCII_STRINGPARAM("abc"), RTL_CONSTASCII_STRINGPARAM(""),
          true },
        { RTL_CONSTASCII_STRINGPARAM(""), RTL_CONSTASCII_STRINGPARAM("abc"),
          false },
        { RTL_CONSTASCII_STRINGPARAM("ABC"), RTL_CONSTASCII_STRINGPARAM("abc"),
          true },
        { RTL_CONSTASCII_STRINGPARAM("abcd"), RTL_CONSTASCII_STRINGPARAM("bcd"),
          true },
        { RTL_CONSTASCII_STRINGPARAM("bcd"), RTL_CONSTASCII_STRINGPARAM("abcd"),
          false },
        { RTL_CONSTASCII_STRINGPARAM("a\0b\0c"),
          RTL_CONSTASCII_STRINGPARAM("b\0c"), true },
        { RTL_CONSTASCII_STRINGPARAM("a\0b\0c"),
          RTL_CONSTASCII_STRINGPARAM("b"), false } };
    for (int i = 0; i < sizeof data / sizeof data[0]; ++i) {
        rtl::OStringBuffer msg;
        appendString(msg, rtl::OString(data[i].str1, data[i].str1Len));
        msg.append(
            RTL_CONSTASCII_STRINGPARAM(".endsWithIgnoreAsciiCaseAsciiL("));
        appendString(msg, rtl::OString(data[i].str2, data[i].str2Len));
        msg.append(RTL_CONSTASCII_STRINGPARAM(") == "));
        msg.append(static_cast< sal_Bool >(data[i].endsWith));
        CPPUNIT_ASSERT_MESSAGE(
            msg.getStr(),
            rtl::OUString(
                data[i].str1, data[i].str1Len,
                RTL_TEXTENCODING_ASCII_US).endsWithIgnoreAsciiCaseAsciiL(
                    data[i].str2, data[i].str2Len)
            == data[i].endsWith);
    }
}
