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
#include <cppunit/extensions/HelperMacros.h>
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include <sal/macros.h>

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

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::EndsWith);

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
    for (int i = 0; i < SAL_N_ELEMENTS(data); ++i) {
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
