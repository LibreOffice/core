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

#include <stdlib.h>
#include <rtl/strbuf.hxx>
#include <rtl/uri.hxx>
#include <osl/thread.h>
#include <osl/file.hxx>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

namespace Stringtest
{

    class Convert : public CppUnit::TestFixture
    {
    public:
        rtl::OString convertToOString(rtl::OUString const& _suStr)
            {
                return rtl::OUStringToOString(_suStr, osl_getThreadTextEncoding()/*RTL_TEXTENCODING_ASCII_US*/);
            }

        void showContent(rtl::OUString const& _suStr)
            {
                rtl::OString sStr = convertToOString(_suStr);
                printf("%s\n", sStr.getStr());
            }

        void test_FromUTF8_001()
            {
                // string --> ustring
                rtl::OString sStrUTF8("h%C3%A4llo");
                rtl::OUString suStrUTF8 = rtl::OStringToOUString(sStrUTF8, RTL_TEXTENCODING_ASCII_US);

                // UTF8 --> real ustring
                rtl::OUString suStr_UriDecodeToIuri      = rtl::Uri::decode(suStrUTF8, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8);
                showContent(suStr_UriDecodeToIuri);

                // string --> ustring
                rtl::OString sStr("h\xE4llo", strlen("h\xE4llo"));
                rtl::OUString suString = rtl::OStringToOUString(sStr, RTL_TEXTENCODING_ISO_8859_15);

                CPPUNIT_ASSERT_EQUAL_MESSAGE("Strings must be equal", suString, suStr_UriDecodeToIuri);

                // ustring --> ustring (UTF8)
                rtl::OUString suStr2 = rtl::Uri::encode(suStr_UriDecodeToIuri, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
                showContent(suStr2);

                CPPUNIT_ASSERT_EQUAL_MESSAGE("Strings must be equal", suStr2, suStrUTF8);
                // suStr should be equal to suStr2
            }

        CPPUNIT_TEST_SUITE( Convert );
        CPPUNIT_TEST( test_FromUTF8_001 );
        CPPUNIT_TEST_SUITE_END( );
    };

}

CPPUNIT_TEST_SUITE_REGISTRATION( Stringtest::Convert );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
