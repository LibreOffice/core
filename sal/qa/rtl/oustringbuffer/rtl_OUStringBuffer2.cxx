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
#include <testshl/simpleheader.hxx>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
