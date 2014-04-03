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
        rtl::OUString m_aStr;
    public:
        rtl::OUString fromUTF8(rtl::OString const& _suStr)
            {
                rtl::OUString suStr = rtl::OStringToOUString(_suStr, RTL_TEXTENCODING_UTF8);
                return suStr;
            }

        rtl::OString convertToOString(rtl::OUString const& _suStr)
            {
                return rtl::OUStringToOString(_suStr, osl_getThreadTextEncoding()/*RTL_TEXTENCODING_ASCII_US*/);
            }

        void showContent(rtl::OUString const& _suStr)
            {
                rtl::OString sStr = convertToOString(_suStr);
                printf("%s\n", sStr.getStr());
            }

        void toUTF8_mech(rtl::OUString const& _suStr, rtl_UriEncodeMechanism _eMechanism)
            {
                rtl::OUString suStr;
                suStr = rtl::Uri::encode(_suStr, rtl_UriCharClassNone,          _eMechanism, RTL_TEXTENCODING_UTF8);
                showContent(suStr);
                suStr = rtl::Uri::encode(_suStr, rtl_UriCharClassUric,          _eMechanism, RTL_TEXTENCODING_UTF8);
                showContent(suStr);
                suStr = rtl::Uri::encode(_suStr, rtl_UriCharClassUricNoSlash,   _eMechanism, RTL_TEXTENCODING_UTF8);
                showContent(suStr);
                suStr = rtl::Uri::encode(_suStr, rtl_UriCharClassRelSegment,    _eMechanism, RTL_TEXTENCODING_UTF8);
                showContent(suStr);
                suStr = rtl::Uri::encode(_suStr, rtl_UriCharClassRegName,       _eMechanism, RTL_TEXTENCODING_UTF8);
                showContent(suStr);
                suStr = rtl::Uri::encode(_suStr, rtl_UriCharClassUserinfo,      _eMechanism, RTL_TEXTENCODING_UTF8);
                showContent(suStr);
                suStr = rtl::Uri::encode(_suStr, rtl_UriCharClassPchar,         _eMechanism, RTL_TEXTENCODING_UTF8);
                showContent(suStr);
                suStr = rtl::Uri::encode(_suStr, rtl_UriCharClassUnoParamValue, _eMechanism, RTL_TEXTENCODING_UTF8);
                showContent(suStr);
            }

        void toUTF8(rtl::OUString const& _suStr)
            {
                printf("rtl_UriEncodeIgnoreEscapes \n");
                toUTF8_mech(_suStr, rtl_UriEncodeIgnoreEscapes);
                printf("\n");
                printf("# rtl_UriEncodeKeepEscapes\n");
                toUTF8_mech(_suStr, rtl_UriEncodeKeepEscapes);
                printf("\n");
                printf("# rtl_UriEncodeCheckEscapes\n");
                toUTF8_mech(_suStr, rtl_UriEncodeCheckEscapes);
                printf("\n");
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

                CPPUNIT_ASSERT_MESSAGE("Strings must be equal", suString.equals(suStr_UriDecodeToIuri));

                // ustring --> ustring (UTF8)
                rtl::OUString suStr2 = rtl::Uri::encode(suStr_UriDecodeToIuri, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
                showContent(suStr2);

                CPPUNIT_ASSERT_MESSAGE("Strings must be equal", suStr2.equals(suStrUTF8));
                // suStr should be equal to suStr2
            }

        // "%C3%84qypten";
        // testshl2 ../../../unxlngi4.pro/lib/libConvert.so "-onlyerrors"
        // # Type: 'Directory' file name '%E6%89%8B%E6%9C%BA%E5%8F%B7%E7%A0%81'
        // # Type: 'Directory' file name '%E6%9C%AA%E5%91%BD%E5%90%8Dzhgb18030'
        // # Type: 'Regular file' file name '%E5%B7%A5%E4%BD%9C'
        // # Type: 'Regular file' file name '%E4%BA%8C%E6%89%8B%E6%88%BF%E4%B9%B0%E5%8D%96%E5%90%88%E5%90%8C%E8%8D%89%E7%A8%BF.doc'
        // ls
        rtl::OString getFileTypeName(osl::FileStatus const& _aStatus)
            {
                rtl::OString sType;
                if (_aStatus.isValid(osl_FileStatus_Mask_Type))
                {
                    osl::FileStatus::Type aType = _aStatus.getFileType();
                    if (aType == osl::FileStatus::Directory)
                    {
                        sType = "Directory";
                    }
                    else if (aType == osl::FileStatus::Regular)
                    {
                        sType = "Regular file";
                    }
                    else if (aType == osl::FileStatus::Volume)
                    {
                        sType = "Volume";
                    }
                    else if (aType == osl::FileStatus::Fifo)
                    {
                        sType = "Fifo";
                    }
                    else if (aType == osl::FileStatus::Socket)
                    {
                        sType = "Socket";
                    }
                    else if (aType == osl::FileStatus::Link)
                    {
                        sType = "Link";
                    }
                    else if (aType == osl::FileStatus::Special)
                    {
                        sType = "Special";
                    }
                    else if (aType == osl::FileStatus::Unknown)
                    {
                        sType = "Unknown";
                    }
                    else
                    {
                        sType = "Not handled yet";
                    }
                }
                else
                {
                    sType = "ERROR: osl_FileStatus_Mask_Type not set for FileStatus!";
                }
                return sType;
            }

        void test_UTF8_files()
            {
#ifdef UNX
                rtl::OUString suDirURL(rtl::OUString("file:///tmp/atestdir"));
#else /* Windows */
                rtl::OUString suDirURL(rtl::OUString("file:///c:/temp/atestdir"));
#endif
                osl::Directory aDir(suDirURL);
                aDir.open();
                if (aDir.isOpen())
                {
                    osl::DirectoryItem aItem;
                    osl::FileStatus aStatus(osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_Attributes | osl_FileStatus_Mask_Type);
                    while (aDir.getNextItem(aItem) == ::osl::FileBase::E_None)
                    {
                        if (osl::FileBase::E_None == aItem.getFileStatus(aStatus) &&
                            aStatus.isValid(osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_Attributes))
                        {
                            rtl::OString sType = getFileTypeName(aStatus);

                            rtl::OUString suFilename = aStatus.getFileName();
                            // rtl::OUString suFullFileURL;

                            rtl::OUString suStrUTF8 = rtl::Uri::encode(suFilename, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
                            rtl::OString sStrUTF8 = convertToOString(suStrUTF8);
                            printf("Type: '%s' file name '%s'\n", sType.getStr(), sStrUTF8.getStr());
                        }
                    }
                    aDir.close();
                }
                else
                {
                    rtl::OString sStr;
                    sStr = rtl::OUStringToOString(suDirURL, osl_getThreadTextEncoding());
                    printf("can't open dir:'%s'\n", sStr.getStr());
                }
            }

        void test_FromUTF8()
            {
                rtl::OString sStr("h%C3%A4llo");
                rtl::OUString suStr = rtl::OStringToOUString(sStr, osl_getThreadTextEncoding());

//    rtl_UriEncodeIgnoreEscapes,
//    rtl_UriEncodeKeepEscapes,
//     rtl_UriEncodeCheckEscapes,
//                rtl::OUString suStr2 = rtl::Uri::encode(suStr, rtl_UriCharClassRegName, rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8);
                rtl::OUString suStr_UriDecodeNone        = rtl::Uri::decode(suStr, rtl_UriDecodeNone, RTL_TEXTENCODING_UTF8);
                showContent(suStr_UriDecodeNone);
                toUTF8(suStr_UriDecodeNone);

                rtl::OUString suStr_UriDecodeToIuri      = rtl::Uri::decode(suStr, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8);
                showContent(suStr_UriDecodeToIuri);
                toUTF8(suStr_UriDecodeToIuri);

                rtl::OUString suStr_UriDecodeWithCharset = rtl::Uri::decode(suStr, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                showContent(suStr_UriDecodeWithCharset);
                toUTF8(suStr_UriDecodeWithCharset);
            }

        CPPUNIT_TEST_SUITE( Convert );
        CPPUNIT_TEST( test_FromUTF8_001 );
//        CPPUNIT_TEST( test_UTF8_files );
//      CPPUNIT_TEST( test_FromUTF8 );
        CPPUNIT_TEST_SUITE_END( );
    };

}

CPPUNIT_TEST_SUITE_REGISTRATION( Stringtest::Convert );

// LLA: doku anpassen!!!

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
