/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_oustring_convert.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:00:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include "cppunit/simpleheader.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"

namespace test { namespace oustring {

class Convert: public CppUnit::TestFixture
{
private:
    void convertToString();

    CPPUNIT_TEST_SUITE(Convert);
    CPPUNIT_TEST(convertToString);
    CPPUNIT_TEST_SUITE_END();
};

} }

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test::oustring::Convert, "alltest");

namespace {

struct TestConvertToString
{
    sal_Unicode aSource[100];
    sal_Int32 nLength;
    rtl_TextEncoding nEncoding;
    sal_uInt32 nFlags;
    char const * pStrict;
    char const * pRelaxed;
};

void testConvertToString(TestConvertToString const & rTest)
{
    const rtl::OUString aSource(rTest.aSource, rTest.nLength);
    rtl::OString aStrict(RTL_CONSTASCII_STRINGPARAM("12345"));
    bool bSuccess = aSource.convertToString(&aStrict, rTest.nEncoding,
                                            rTest.nFlags);
    rtl::OString aRelaxed(rtl::OUStringToOString(aSource, rTest.nEncoding,
                                                 rTest.nFlags));

    rtl::OStringBuffer aPrefix;
    aPrefix.append(RTL_CONSTASCII_STRINGPARAM("{"));
    for (sal_Int32 i = 0; i < rTest.nLength; ++i)
    {
        aPrefix.append(RTL_CONSTASCII_STRINGPARAM("U+"));
        aPrefix.append(static_cast< sal_Int32 >(rTest.aSource[i]), 16);
        if (i + 1 < rTest.nLength)
            aPrefix.append(RTL_CONSTASCII_STRINGPARAM(","));
    }
    aPrefix.append(RTL_CONSTASCII_STRINGPARAM("}, "));
    aPrefix.append(static_cast< sal_Int32 >(rTest.nEncoding));
    aPrefix.append(RTL_CONSTASCII_STRINGPARAM(", 0x"));
    aPrefix.append(static_cast< sal_Int32 >(rTest.nFlags), 16);
    aPrefix.append(RTL_CONSTASCII_STRINGPARAM(" -> "));

    if (bSuccess)
    {
        if (rTest.pStrict == 0 || !aStrict.equals(rTest.pStrict))
        {
            rtl::OStringBuffer aMessage(aPrefix);
            aMessage.append(RTL_CONSTASCII_STRINGPARAM("strict = \""));
            aMessage.append(aStrict);
            aMessage.append(RTL_CONSTASCII_STRINGPARAM("\""));
            CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(), false);
        }
    }
    else
    {
        if (!aStrict.equals(rtl::OString(RTL_CONSTASCII_STRINGPARAM("12345"))))
        {
            rtl::OStringBuffer aMessage(aPrefix);
            aMessage.append(RTL_CONSTASCII_STRINGPARAM("modified output"));
            CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(), false);
        }
        if (rTest.pStrict != 0)
        {
            rtl::OStringBuffer aMessage(aPrefix);
            aMessage.append(RTL_CONSTASCII_STRINGPARAM("failed"));
            CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(), false);
        }
    }
    if (!aRelaxed.equals(rTest.pRelaxed))
    {
        rtl::OStringBuffer aMessage(aPrefix);
        aMessage.append(RTL_CONSTASCII_STRINGPARAM("relaxed = \""));
        aMessage.append(aRelaxed);
        aMessage.append(RTL_CONSTASCII_STRINGPARAM("\""));
        CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(), false);
    }
}

}

void test::oustring::Convert::convertToString()
{
    TestConvertToString const aTests[]
        = { { { 0 },
              0,
              RTL_TEXTENCODING_ASCII_US,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                  | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR,
              "",
              "" },
            { { 0 },
              0,
              RTL_TEXTENCODING_ASCII_US,
              OUSTRING_TO_OSTRING_CVTFLAGS,
              "",
              "" },
            { { 0x0041,0x0042,0x0043 },
              3,
              RTL_TEXTENCODING_ASCII_US,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                  | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR,
              "ABC",
              "ABC" },
            { { 0x0041,0x0042,0x0043 },
              3,
              RTL_TEXTENCODING_ASCII_US,
              OUSTRING_TO_OSTRING_CVTFLAGS,
              "ABC",
              "ABC" },
            { { 0xB800 },
              1,
              RTL_TEXTENCODING_ISO_2022_JP,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                  | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR,
              0,
              "" },
            // the next also tests that a short source produces a long target:
            { { 0xB800 },
              1,
              RTL_TEXTENCODING_ISO_2022_JP,
              OUSTRING_TO_OSTRING_CVTFLAGS,
              "\x1B(B?",
              "\x1B(B?" },
            { { 0x0041,0x0100,0x0042 },
              3,
              RTL_TEXTENCODING_ISO_8859_1,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                  | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR,
              0,
              "A" },
            { { 0x0041,0x0100,0x0042 },
              3,
              RTL_TEXTENCODING_ISO_8859_1,
              OUSTRING_TO_OSTRING_CVTFLAGS,
              "A?B",
              "A?B" } };
    for (unsigned int i = 0; i < sizeof aTests / sizeof aTests[0]; ++i)
        testConvertToString(aTests[i]);
}
