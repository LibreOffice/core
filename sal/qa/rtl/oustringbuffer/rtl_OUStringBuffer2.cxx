/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rtl_OUStringBuffer2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 08:58:39 $
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
#include <cppunit/simpleheader.hxx>
#include "stringhelper.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>

namespace rtl_OUStringBuffer
{


class insertUtf32 : public CppUnit::TestFixture
{
public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void insertUtf32_001()
        {
            ::rtl::OUStringBuffer aUStrBuf(4);
            aUStrBuf.insertUtf32(0,0x10ffff);

            rtl::OUString suStr = aUStrBuf.makeStringAndClear();
            rtl::OUString suStr2 = rtl::Uri::encode(suStr, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);

            rtl::OString sStr;
            sStr <<= suStr2;
            t_print("%s\n", sStr.getStr());

            CPPUNIT_ASSERT_MESSAGE("Strings must be '%F4%8F%BF%BF'", sStr.equals(rtl::OString("%F4%8F%BF%BF")) == sal_True);
        }

    void insertUtf32_002()
        {
            ::rtl::OUStringBuffer aUStrBuf(4);
            aUStrBuf.insertUtf32(0,0x41);
            aUStrBuf.insertUtf32(1,0x42);
            aUStrBuf.insertUtf32(2,0x43);

            rtl::OUString suStr = aUStrBuf.makeStringAndClear();
            rtl::OUString suStr2 = rtl::Uri::encode(suStr, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);

            rtl::OString sStr;
            sStr <<= suStr2;
            t_print("%s\n", sStr.getStr());

            CPPUNIT_ASSERT_MESSAGE("Strings must be 'ABC'", sStr.equals(rtl::OString("ABC")) == sal_True);
        }

    CPPUNIT_TEST_SUITE(insertUtf32);
    CPPUNIT_TEST(insertUtf32_001);
    CPPUNIT_TEST(insertUtf32_002);
    CPPUNIT_TEST_SUITE_END();
}; // class getToken

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(rtl_OUStringBuffer::insertUtf32, "rtl_OUStringBuffer");

} // namespace rtl_OUStringBuffer


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;

