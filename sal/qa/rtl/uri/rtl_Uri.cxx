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
#include <stdlib.h>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/uri.hxx>
#include <osl/thread.h>
#include <osl/file.hxx>

#include "gtest/gtest.h"

// -----------------------------------------------------------------------------

namespace Stringtest
{
    rtl::OString toHex(unsigned char _c)
    {
        rtl::OStringBuffer sStrBuf;
        static char cHex[] = "0123456789ABCDEF";

        int nhigh = int(_c) >> 4 & 0xf;
        int nlow  = int(_c) & 0xf;
        sStrBuf.append( cHex[nhigh] );
        sStrBuf.append( cHex[nlow] );
        return sStrBuf.makeStringAndClear();
    }

    rtl::OString escapeString(rtl::OString const& _sStr)
    {
        rtl::OStringBuffer sStrBuf;
        sal_Int32 nLength = _sStr.getLength();
        for(int i=0;i<nLength;++i)
        {
            unsigned char c = (unsigned char)_sStr[i];
            if (c > 127)
            {
                sStrBuf.append("%");
                sStrBuf.append(toHex(c));
            }
            else
            {
                sStrBuf.append((char)c);
            }
        }
        return sStrBuf.makeStringAndClear();
    }

    // -----------------------------------------------------------------------------

    class Convert : public ::testing::Test
    {
    protected:
        rtl::OUString m_aStr;
    public:
        /*
          rtl::OString toUTF8(rtl::OUString const& _suStr)
            {
                rtl::OString sStrAsUTF8 = rtl::OUStringToOString(_suStr, RTL_TEXTENCODING_UTF8);
                printf("%s\n", escapeString(sStrAsUTF8).getStr());
                return sStrAsUTF8;
            }
        */
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
    };

    TEST_F(Convert, test_FromUTF8_001)
    {
        // string --> ustring
        rtl::OString sStrUTF8("h%C3%A4llo");
        rtl::OUString suStrUTF8 = rtl::OStringToOUString(sStrUTF8, RTL_TEXTENCODING_ASCII_US);

        // UTF8 --> real ustring
        rtl::OUString suStr_UriDecodeToIuri      = rtl::Uri::decode(suStrUTF8, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8);
        showContent(suStr_UriDecodeToIuri);

        // string --> ustring
        rtl::OString sStr("h\xE4llo");
        rtl::OUString suString = rtl::OStringToOUString(sStr, RTL_TEXTENCODING_ISO_8859_15);

        ASSERT_TRUE(suString.equals(suStr_UriDecodeToIuri) == sal_True) << "Strings must be equal";

        // ustring --> ustring (UTF8)
        rtl::OUString suStr2 = rtl::Uri::encode(suStr_UriDecodeToIuri, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
        showContent(suStr2);

        ASSERT_TRUE(suStr2.equals(suStrUTF8) == sal_True) << "Strings must be equal";
        // suStr should be equal to suStr2
    }

// These tests were commented out in the pre-gtest code:
//    TEST_F(Convert, test_UTF8_files)
//    {
//#ifdef UNX
//        rtl::OUString suDirURL(rtl::OUString::createFromAscii("file:///tmp/atestdir"));
//#else /* Windows */
//        rtl::OUString suDirURL(rtl::OUString::createFromAscii("file:///c:/temp/atestdir"));
//#endif
//        osl::Directory aDir(suDirURL);
//        aDir.open();
//        if (aDir.isOpen())
//        {
//            osl::DirectoryItem aItem;
//            osl::FileStatus aStatus(osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_Attributes | osl_FileStatus_Mask_Type);
//            while (aDir.getNextItem(aItem) == ::osl::FileBase::E_None)
//            {
//                if (osl::FileBase::E_None == aItem.getFileStatus(aStatus) &&
//                    aStatus.isValid(osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_Attributes))
//                {
//                    rtl::OString sType = getFileTypeName(aStatus);
//
//                    rtl::OUString suFilename = aStatus.getFileName();
//                    // rtl::OUString suFullFileURL;
//
//                    rtl::OUString suStrUTF8 = rtl::Uri::encode(suFilename, rtl_UriCharClassUnoParamValue, rtl_UriEncodeKeepEscapes, RTL_TEXTENCODING_UTF8);
//                    rtl::OString sStrUTF8 = convertToOString(suStrUTF8);
//                    printf("Type: '%s' file name '%s'\n", sType.getStr(), sStrUTF8.getStr());
//                }
//            }
//            aDir.close();
//        }
//        else
//        {
//            rtl::OString sStr;
//            sStr = rtl::OUStringToOString(suDirURL, osl_getThreadTextEncoding());
//            printf("can't open dir:'%s'\n", sStr.getStr());
//        }
//    }
//
//    TEST_F(Convert, test_FromUTF8)
//    {
//        rtl::OString sStr("h%C3%A4llo");
//        rtl::OUString suStr = rtl::OStringToOUString(sStr, osl_getThreadTextEncoding());
//
////    rtl_UriEncodeIgnoreEscapes,
////    rtl_UriEncodeKeepEscapes,
////     rtl_UriEncodeCheckEscapes,
////                rtl::OUString suStr2 = rtl::Uri::encode(suStr, rtl_UriCharClassRegName, rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8);
//        rtl::OUString suStr_UriDecodeNone        = rtl::Uri::decode(suStr, rtl_UriDecodeNone, RTL_TEXTENCODING_UTF8);
//        showContent(suStr_UriDecodeNone);
//        toUTF8(suStr_UriDecodeNone);
//
//        rtl::OUString suStr_UriDecodeToIuri      = rtl::Uri::decode(suStr, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8);
//        showContent(suStr_UriDecodeToIuri);
//        toUTF8(suStr_UriDecodeToIuri);
//
//        rtl::OUString suStr_UriDecodeWithCharset = rtl::Uri::decode(suStr, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
//        showContent(suStr_UriDecodeWithCharset);
//        toUTF8(suStr_UriDecodeWithCharset);
//    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
