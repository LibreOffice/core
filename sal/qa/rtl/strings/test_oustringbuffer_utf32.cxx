/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/strbuf.hxx"
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

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustringbuffer::Utf32);

namespace {

void appendString(rtl::OStringBuffer & buffer, rtl::OUString const & string) {
    buffer.append('"');
    for (int i = 0; i < string.getLength(); ++i) {
        buffer.append(RTL_CONSTASCII_STRINGPARAM("\\u"));
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
    rtl::OStringBuffer & message, rtl::OUString const & string1,
    rtl::OUString const & string2)
{
    message.setLength(0);
    appendString(message, string1);
    message.append(RTL_CONSTASCII_STRINGPARAM(" vs. "));
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
    rtl::OStringBuffer message;
    rtl::OUStringBuffer buf1(rtl::OUString(str1, str1Len));
    buf1.appendUtf32('d');
    rtl::OUString res1(buf1.makeStringAndClear());
    createMessage(message, res1, rtl::OUString(str2, str2Len));
    CPPUNIT_ASSERT_MESSAGE(
        (const char *) message.getStr(), res1 == rtl::OUString(str2, str2Len));
    rtl::OUStringBuffer buf2(rtl::OUString(str2, str2Len));
    buf2.appendUtf32(0x10000);
    rtl::OUString res2(buf2.makeStringAndClear());
    createMessage(message, res2, rtl::OUString(str3, str3Len));
    CPPUNIT_ASSERT_MESSAGE(
        (const char *)message.getStr(), res2 == rtl::OUString(str3, str3Len));
}

void test::oustringbuffer::Utf32::insertUtf32() {
    int const str1Len = 3;
    sal_Unicode const str1[str1Len] = { 'a', 'b', 'c' };
    int const str2Len = 4;
    sal_Unicode const str2[str2Len] = { 'a', 'b', 'd', 'c' };
    int const str3Len = 6;
    sal_Unicode const str3[str3Len] = { 'a', 'b', 0xDBFF, 0xDFFF, 'd', 'c' };
    rtl::OStringBuffer message;
    rtl::OUStringBuffer buf1(rtl::OUString(str1, str1Len));
    buf1.insertUtf32(2, 'd');
    rtl::OUString res1(buf1.makeStringAndClear());
    createMessage(message, res1, rtl::OUString(str2, str2Len));
    CPPUNIT_ASSERT_MESSAGE(
        (const char *) message.getStr(), res1 == rtl::OUString(str2, str2Len));
    rtl::OUStringBuffer buf2(rtl::OUString(str2, str2Len));
    buf2.insertUtf32(2, 0x10FFFF);
    rtl::OUString res2(buf2.makeStringAndClear());
    createMessage(message, res2, rtl::OUString(str3, str3Len));
    CPPUNIT_ASSERT_MESSAGE(
        (const char *) message.getStr(), res2 == rtl::OUString(str3, str3Len));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
