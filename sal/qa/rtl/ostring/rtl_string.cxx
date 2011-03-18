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
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/ustring.hxx>
#include <cstring>

namespace rtl_string
{

    class getLength : public CppUnit::TestFixture
    {
    public:

        void getLength_000()
            {
                rtl_string_getLength( NULL );
                // should not GPF
            }

        void getLength_001()
            {
                rtl::OString aStr("Test Length.");
                sal_Int32 nValue = rtl_string_getLength( aStr.pData );

                CPPUNIT_ASSERT_MESSAGE("Length must equal getLength()", aStr.getLength() == nValue);
                CPPUNIT_ASSERT_MESSAGE(
                    "Length must equal strlen()",
                    nValue >= 0
                    && (strlen(aStr.getStr())
                        == sal::static_int_cast< sal_uInt32 >(nValue)));
            }
        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(getLength);
        CPPUNIT_TEST(getLength_000);
        CPPUNIT_TEST(getLength_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class getLength

// -----------------------------------------------------------------------------

    class newFromString : public CppUnit::TestFixture
    {
    public:

        // void newFromString_000()
        //     {
        //         sal_Int32 nValue = rtl_string_newFromString( NULL, NULL );
        //         // should not GPF
        //     }

        void newFromString_001()
            {
                rtl::OString aStr("Test Length.");
                rtl_String *pStr = NULL;

                rtl_string_newFromString( &pStr, aStr.pData );

                rtl::OString aNewStr(pStr);
                CPPUNIT_ASSERT_MESSAGE("Strings must be equal", aStr.equals(aNewStr) == sal_True);

                rtl_string_release(pStr);
            }
        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(newFromString);
        // CPPUNIT_TEST(newFromString_000);
        CPPUNIT_TEST(newFromString_001);
        CPPUNIT_TEST_SUITE_END();
    }; // class newFromString

    // -----------------------------------------------------------------------------

    class convertUStringToString : public CppUnit::TestFixture
    {
    public:

        // void newFromString_000()
        //     {
        //         sal_Int32 nValue = rtl_string_newFromString( NULL, NULL );
        //         // should not GPF
        //     }

        void convertUStringToString_001()
            {
                rtl::OUString suString(RTL_CONSTASCII_USTRINGPARAM("Hello"));
                rtl::OString sString;
                sal_Bool bRet = rtl_convertUStringToString(&sString.pData, suString.getStr(), suString.getLength(), RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS);

                CPPUNIT_ASSERT_MESSAGE("Strings must be equal", bRet == sal_True && sString.equals(rtl::OString("Hello")) == sal_True);
            }

        void convertUStringToString_002()
            {
                rtl::OString sStr("H\xE4llo");
                rtl::OUString suString = rtl::OStringToOUString(sStr, RTL_TEXTENCODING_ISO_8859_15);

                rtl::OString sString;
                sal_Bool bRet = rtl_convertUStringToString(&sString.pData, suString.getStr(), suString.getLength(), RTL_TEXTENCODING_ISO_8859_15, OUSTRING_TO_OSTRING_CVTFLAGS);

                CPPUNIT_ASSERT_MESSAGE("Strings must be equal", bRet == sal_True && sString.equals(rtl::OString("H\xE4llo")) == sal_True);
            }

        void convertUStringToString_003()
            {
                rtl::OString sStr("H\xC3\xA4llo");
                rtl::OUString suString = rtl::OStringToOUString(sStr, RTL_TEXTENCODING_UTF8);

                rtl::OString sString;
                sal_Bool bRet = rtl_convertUStringToString(&sString.pData, suString.getStr(), suString.getLength(), RTL_TEXTENCODING_ISO_8859_15, OUSTRING_TO_OSTRING_CVTFLAGS);

                CPPUNIT_ASSERT_MESSAGE("Strings must be equal", bRet == sal_True && sString.equals(rtl::OString("H\xE4llo")) == sal_True);
            }

        void convertUStringToString_004()
            {
                rtl::OString sStr("Tsch\xFC\xDF");
                rtl::OUString suString = rtl::OStringToOUString(sStr, RTL_TEXTENCODING_ISO_8859_15);
                rtl::OString sString;

                sal_Bool       bRet = rtl_convertUStringToString(&sString.pData, suString.getStr(), suString.getLength(), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
                /* sal_Bool */ bRet = rtl_convertUStringToString(&sString.pData, suString.getStr(), suString.getLength(), RTL_TEXTENCODING_ISO_8859_15, OUSTRING_TO_OSTRING_CVTFLAGS);
                CPPUNIT_ASSERT_MESSAGE("Strings must be equal", bRet == sal_True && sString.equals(rtl::OString("Tsch\xFC\xDF")) == sal_True);
            }



        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(convertUStringToString);
        CPPUNIT_TEST(convertUStringToString_001);
        CPPUNIT_TEST(convertUStringToString_002);
        CPPUNIT_TEST(convertUStringToString_003);
        CPPUNIT_TEST(convertUStringToString_004);
        CPPUNIT_TEST_SUITE_END();
    }; // class convertUStringToString



} // namespace rtl_string

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_string::getLength);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_string::newFromString);
CPPUNIT_TEST_SUITE_REGISTRATION(rtl_string::convertUStringToString);

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
