/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>

namespace test::oustringbuffer {

class Utf32: public CppUnit::TestFixture {
private:
    void appendUtf32();

    void insertUtf32();

    CPPUNIT_TEST_SUITE(Utf32);
    CPPUNIT_TEST(appendUtf32);
    CPPUNIT_TEST(insertUtf32);
    CPPUNIT_TEST_SUITE_END();
};

}

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustringbuffer::Utf32);

namespace {

void appendString(OStringBuffer & buffer, OUString const & string) {
    buffer.append('"');
    for (int i = 0; i < string.getLength(); ++i) {
        buffer.append("\\u");
        sal_Unicode c = string[i];
        if (c < 0x1000) {
            buffer.append('0');
            if (c < 0x100) {
                buffer.append('0');
                if (c < 0x10) {
                    buffer.append('0');
                }
            }
        }
        buffer.append(
            static_cast< sal_Int32 >(c), static_cast< sal_Int16 >(16));
    }
    buffer.append('"');
}

void createMessage(
    OStringBuffer & message, OUString const & string1,
    OUString const & string2)
{
    message.setLength(0);
    appendString(message, string1);
    message.append(" vs. ");
    appendString(message, string2);
}

}

void test::oustringbuffer::Utf32::appendUtf32() {
    int const str1Len = 3;
    sal_Unicode const str1[str1Len] = { 'a', 'b', 'c' };
    static constexpr OUStringLiteral str2 = u"abcd";
    static constexpr OUStringLiteral str3 = u"abcd\U00010000";
    OStringBuffer message;
    OUStringBuffer buf1(std::u16string_view(str1, str1Len));
    buf1.appendUtf32('d');
    OUString res1(buf1.makeStringAndClear());
    createMessage(message, res1, str2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        message.getStr(), OUString(str2), res1);
    OUStringBuffer buf2(str2);
    buf2.appendUtf32(0x10000);
    OUString res2(buf2.makeStringAndClear());
    createMessage(message, res2, str3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        message.getStr(), OUString(str3), res2);
}

void test::oustringbuffer::Utf32::insertUtf32() {
    int const str1Len = 3;
    sal_Unicode const str1[str1Len] = { 'a', 'b', 'c' };
    static constexpr OUStringLiteral str2 = u"abdc";
    static constexpr OUStringLiteral str3 = u"ab\U0010FFFFdc";
    OStringBuffer message;
    OUStringBuffer buf1(std::u16string_view(str1, str1Len));
    buf1.insertUtf32(2, 'd');
    OUString res1(buf1.makeStringAndClear());
    createMessage(message, res1, str2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        message.getStr(), OUString(str2), res1);
    OUStringBuffer buf2(str2);
    buf2.insertUtf32(2, 0x10FFFF);
    OUString res2(buf2.makeStringAndClear());
    createMessage(message, res2, str3);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        message.getStr(), OUString(str3), res2);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
