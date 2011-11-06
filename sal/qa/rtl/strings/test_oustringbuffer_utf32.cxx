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
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

namespace test { namespace oustringbuffer {

class Utf32: public CppUnit::TestFixture {
private:
    void appendUtf32();

    void insertUtf32();

    CPPUNIT_TEST_SUITE(Utf32);
    CPPUNIT_TEST(appendUtf32);
    CPPUNIT_TEST(insertUtf32);
    CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test::oustringbuffer::Utf32, "alltest");

namespace {

void appendString(rtl::OUStringBuffer & buffer, rtl::OUString const & string) {
    buffer.append(static_cast< sal_Unicode >('"'));
    for (int i = 0; i < string.getLength(); ++i) {
        buffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("\\u"));
        sal_Unicode c = string[i];
        if (c < 0x1000) {
            buffer.append(static_cast< sal_Unicode >('0'));
            if (c < 0x100) {
                buffer.append(static_cast< sal_Unicode >('0'));
                if (c < 0x10) {
                    buffer.append(static_cast< sal_Unicode >('0'));
                }
            }
        }
        buffer.append(
            static_cast< sal_Int32 >(c), static_cast< sal_Int16 >(16));
    }
    buffer.append(static_cast< sal_Unicode >('"'));
}

void createMessage(
    rtl::OUStringBuffer & message, rtl::OUString const & string1,
    rtl::OUString const & string2)
{
    message.setLength(0);
    appendString(message, string1);
    message.appendAscii(RTL_CONSTASCII_STRINGPARAM(" vs. "));
    appendString(message, string2);
}

}

void test::oustringbuffer::Utf32::appendUtf32() {
    int const str1Len = 3;
    sal_Unicode const str1[str1Len] = { 'a', 'b', 'c' };
    int const str2Len = 4;
    sal_Unicode const str2[str2Len] = { 'a', 'b', 'c', 'd' };
    int const str3Len = 6;
    sal_Unicode const str3[str3Len] = { 'a', 'b', 'c', 'd', 0xD800, 0xDC00 };
    rtl::OUStringBuffer message;
    rtl::OUStringBuffer buf1(rtl::OUString(str1, str1Len));
    buf1.appendUtf32('d');
    rtl::OUString res1(buf1.makeStringAndClear());
    createMessage(message, res1, rtl::OUString(str2, str2Len));
    CPPUNIT_ASSERT_MESSAGE(
        message.getStr(), res1 == rtl::OUString(str2, str2Len));
    rtl::OUStringBuffer buf2(rtl::OUString(str2, str2Len));
    buf2.appendUtf32(0x10000);
    rtl::OUString res2(buf2.makeStringAndClear());
    createMessage(message, res2, rtl::OUString(str3, str3Len));
    CPPUNIT_ASSERT_MESSAGE(
        message.getStr(), res2 == rtl::OUString(str3, str3Len));
}

void test::oustringbuffer::Utf32::insertUtf32() {
    int const str1Len = 3;
    sal_Unicode const str1[str1Len] = { 'a', 'b', 'c' };
    int const str2Len = 4;
    sal_Unicode const str2[str2Len] = { 'a', 'b', 'd', 'c' };
    int const str3Len = 6;
    sal_Unicode const str3[str3Len] = { 'a', 'b', 0xDBFF, 0xDFFF, 'd', 'c' };
    rtl::OUStringBuffer message;
    rtl::OUStringBuffer buf1(rtl::OUString(str1, str1Len));
    buf1.insertUtf32(2, 'd');
    rtl::OUString res1(buf1.makeStringAndClear());
    createMessage(message, res1, rtl::OUString(str2, str2Len));
    CPPUNIT_ASSERT_MESSAGE(
        message.getStr(), res1 == rtl::OUString(str2, str2Len));
    rtl::OUStringBuffer buf2(rtl::OUString(str2, str2Len));
    buf2.insertUtf32(2, 0x10FFFF);
    rtl::OUString res2(buf2.makeStringAndClear());
    createMessage(message, res2, rtl::OUString(str3, str3Len));
    CPPUNIT_ASSERT_MESSAGE(
        message.getStr(), res2 == rtl::OUString(str3, str3Len));
}
