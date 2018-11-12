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

#include <config_locales.h>

#include <string.h>

#include <rtl/tencinfo.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

namespace
{
    class testBestMime : public CppUnit::TestFixture
    {
    public:
        void check(rtl_TextEncoding eIn, rtl_TextEncoding eOut)
        {
            const sal_Char *pCharSet = rtl_getBestMimeCharsetFromTextEncoding(eIn);
            rtl_TextEncoding eTextEnc = rtl_getTextEncodingFromMimeCharset(pCharSet);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("rtl_getBestMimeCharsetFromTextEncoding && rtl_getTextEncodingFromMimeCharset differdiffer", eOut, eTextEnc );
        }

        // the defines for the follows test could be found in file inc/rtl/textenc.h

        void MimeCharsetFromTextEncoding_MS_1252()
        {
            check( RTL_TEXTENCODING_MS_1252, RTL_TEXTENCODING_MS_1252 );
        }

        void MimeCharsetFromTextEncoding_APPLE_ROMAN()
        {
            check( RTL_TEXTENCODING_APPLE_ROMAN, RTL_TEXTENCODING_APPLE_ROMAN);
        }

        void MimeCharsetFromTextEncoding_IBM_437()
        {
            check( RTL_TEXTENCODING_IBM_437, RTL_TEXTENCODING_IBM_437 );
        }

        void MimeCharsetFromTextEncoding_IBM_850()
        {
            check( RTL_TEXTENCODING_IBM_850, RTL_TEXTENCODING_IBM_850 );
        }

        void MimeCharsetFromTextEncoding_IBM_860()
        {
            check( RTL_TEXTENCODING_IBM_860, RTL_TEXTENCODING_IBM_860 );
        }

        void MimeCharsetFromTextEncoding_IBM_861()
        {
            check( RTL_TEXTENCODING_IBM_861, RTL_TEXTENCODING_IBM_861 );
        }

        void MimeCharsetFromTextEncoding_IBM_863()
        {
            check( RTL_TEXTENCODING_IBM_863, RTL_TEXTENCODING_IBM_863 );
        }

        void MimeCharsetFromTextEncoding_IBM_865()
        {
            check( RTL_TEXTENCODING_IBM_865, RTL_TEXTENCODING_IBM_865 );
        }

        void MimeCharsetFromTextEncoding_SYMBOL()
        {
            check( RTL_TEXTENCODING_SYMBOL, RTL_TEXTENCODING_DONTKNOW );
        }

        void MimeCharsetFromTextEncoding_ASCII_US()
        {
            check( RTL_TEXTENCODING_ASCII_US, RTL_TEXTENCODING_ASCII_US );
        }

        void MimeCharsetFromTextEncoding_ISO_8859_1()
        {
            check( RTL_TEXTENCODING_ISO_8859_1, RTL_TEXTENCODING_ISO_8859_1 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_2()
        {
            check( RTL_TEXTENCODING_ISO_8859_2, RTL_TEXTENCODING_ISO_8859_2 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_3()
        {
            check( RTL_TEXTENCODING_ISO_8859_3, RTL_TEXTENCODING_ISO_8859_3 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_4()
        {
            check( RTL_TEXTENCODING_ISO_8859_4, RTL_TEXTENCODING_ISO_8859_4 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_5()
        {
            check( RTL_TEXTENCODING_ISO_8859_5, RTL_TEXTENCODING_ISO_8859_5 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_6()
        {
            check( RTL_TEXTENCODING_ISO_8859_6, RTL_TEXTENCODING_ISO_8859_6 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_7()
        {
            check( RTL_TEXTENCODING_ISO_8859_7, RTL_TEXTENCODING_ISO_8859_7 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_8()
        {
            check( RTL_TEXTENCODING_ISO_8859_8, RTL_TEXTENCODING_ISO_8859_8 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_9()
        {
            check( RTL_TEXTENCODING_ISO_8859_9, RTL_TEXTENCODING_ISO_8859_9 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_14()
        {
            check( RTL_TEXTENCODING_ISO_8859_14, RTL_TEXTENCODING_ISO_8859_14 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_15()
        {
            check( RTL_TEXTENCODING_ISO_8859_15, RTL_TEXTENCODING_ISO_8859_15 );
        }
        void MimeCharsetFromTextEncoding_IBM_737()
        {
            check( RTL_TEXTENCODING_IBM_737, RTL_TEXTENCODING_ISO_8859_7 );
        }
        void MimeCharsetFromTextEncoding_IBM_775()
        {
            check( RTL_TEXTENCODING_IBM_775, RTL_TEXTENCODING_ISO_8859_4 );
        }
        void MimeCharsetFromTextEncoding_IBM_852()
        {
            check( RTL_TEXTENCODING_IBM_852, RTL_TEXTENCODING_IBM_852 );
        }
        void MimeCharsetFromTextEncoding_IBM_855()
        {
            check( RTL_TEXTENCODING_IBM_855, RTL_TEXTENCODING_ISO_8859_5 );
        }
        void MimeCharsetFromTextEncoding_IBM_857()
        {
            check( RTL_TEXTENCODING_IBM_857, RTL_TEXTENCODING_ISO_8859_9 );
        }
        void MimeCharsetFromTextEncoding_IBM_862()
        {
            check( RTL_TEXTENCODING_IBM_862, RTL_TEXTENCODING_IBM_862 );
        }
        void MimeCharsetFromTextEncoding_IBM_864()
        {
            check( RTL_TEXTENCODING_IBM_864, RTL_TEXTENCODING_IBM_864 );
        }
        void MimeCharsetFromTextEncoding_IBM_866()
        {
            check( RTL_TEXTENCODING_IBM_866, RTL_TEXTENCODING_IBM_866 );
        }
        void MimeCharsetFromTextEncoding_IBM_869()
        {
            check( RTL_TEXTENCODING_IBM_869, RTL_TEXTENCODING_ISO_8859_7 );
        }
        void MimeCharsetFromTextEncoding_MS_874()
        {
            check( RTL_TEXTENCODING_MS_874, RTL_TEXTENCODING_MS_874 );
        }
        void MimeCharsetFromTextEncoding_MS_1250()
        {
            check( RTL_TEXTENCODING_MS_1250, RTL_TEXTENCODING_MS_1250 );
        }
        void MimeCharsetFromTextEncoding_MS_1251()
        {
            check( RTL_TEXTENCODING_MS_1251, RTL_TEXTENCODING_MS_1251 );
        }
        void MimeCharsetFromTextEncoding_MS_1253()
        {
            check( RTL_TEXTENCODING_MS_1253, RTL_TEXTENCODING_MS_1253 );
        }
        void MimeCharsetFromTextEncoding_MS_1254()
        {
            check( RTL_TEXTENCODING_MS_1254, RTL_TEXTENCODING_MS_1254 );
        }
        void MimeCharsetFromTextEncoding_MS_1255()
        {
            check( RTL_TEXTENCODING_MS_1255, RTL_TEXTENCODING_MS_1255 );
        }
        void MimeCharsetFromTextEncoding_MS_1256()
        {
            check( RTL_TEXTENCODING_MS_1256, RTL_TEXTENCODING_MS_1256 );
        }
        void MimeCharsetFromTextEncoding_MS_1257()
        {
            check( RTL_TEXTENCODING_MS_1257, RTL_TEXTENCODING_MS_1257 );
        }
        void MimeCharsetFromTextEncoding_MS_1258()
        {
            check( RTL_TEXTENCODING_MS_1258, RTL_TEXTENCODING_MS_1258 );
        }
        void MimeCharsetFromTextEncoding_APPLE_CENTEURO()
        {
            check( RTL_TEXTENCODING_APPLE_CENTEURO, RTL_TEXTENCODING_ISO_8859_2 );
        }
        void MimeCharsetFromTextEncoding_APPLE_CROATIAN()
        {
            check( RTL_TEXTENCODING_APPLE_CROATIAN, RTL_TEXTENCODING_ISO_8859_2 );
        }
        void MimeCharsetFromTextEncoding_APPLE_CYRILLIC()
        {
            check( RTL_TEXTENCODING_APPLE_CYRILLIC, RTL_TEXTENCODING_ISO_8859_5 );
        }
        void MimeCharsetFromTextEncoding_APPLE_GREEK()
        {
            check( RTL_TEXTENCODING_APPLE_GREEK, RTL_TEXTENCODING_ISO_8859_7 );
        }
        void MimeCharsetFromTextEncoding_APPLE_ICELAND()
        {
            check( RTL_TEXTENCODING_APPLE_ICELAND, RTL_TEXTENCODING_ISO_8859_1 );
        }
        void MimeCharsetFromTextEncoding_APPLE_ROMANIAN()
        {
            check( RTL_TEXTENCODING_APPLE_ROMANIAN, RTL_TEXTENCODING_ISO_8859_2 );
        }
        void MimeCharsetFromTextEncoding_APPLE_TURKISH()
        {
            check( RTL_TEXTENCODING_APPLE_TURKISH, RTL_TEXTENCODING_ISO_8859_9 );
        }
        void MimeCharsetFromTextEncoding_APPLE_UKRAINIAN()
        {
            check( RTL_TEXTENCODING_APPLE_UKRAINIAN, RTL_TEXTENCODING_ISO_8859_5 );
        }
        void MimeCharsetFromTextEncoding_MS_932()
        {
            check( RTL_TEXTENCODING_MS_932, RTL_TEXTENCODING_SHIFT_JIS );
        }
        void MimeCharsetFromTextEncoding_MS_936()
        {
            check( RTL_TEXTENCODING_MS_936, RTL_TEXTENCODING_GB_2312 );
        }
        void MimeCharsetFromTextEncoding_MS_949()
        {
            check( RTL_TEXTENCODING_MS_949, RTL_TEXTENCODING_EUC_KR );
        }
        void MimeCharsetFromTextEncoding_MS_950()
        {
            check( RTL_TEXTENCODING_MS_950, RTL_TEXTENCODING_BIG5 );
        }
        void MimeCharsetFromTextEncoding_KOI8_R()
        {
            check( RTL_TEXTENCODING_KOI8_R, RTL_TEXTENCODING_KOI8_R );
        }
        void MimeCharsetFromTextEncoding_UTF7()
        {
            check( RTL_TEXTENCODING_UTF7, RTL_TEXTENCODING_UTF7 );
        }
        void MimeCharsetFromTextEncoding_UTF8()
        {
            check( RTL_TEXTENCODING_UTF8, RTL_TEXTENCODING_UTF8 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_10()
        {
            check( RTL_TEXTENCODING_ISO_8859_10, RTL_TEXTENCODING_ISO_8859_10 );
        }
        void MimeCharsetFromTextEncoding_ISO_8859_13()
        {
            check( RTL_TEXTENCODING_ISO_8859_13, RTL_TEXTENCODING_ISO_8859_13 );
        }
        void MimeCharsetFromTextEncoding_MS_1361()
        {
            check( RTL_TEXTENCODING_MS_1361, RTL_TEXTENCODING_EUC_KR );
        }
        void MimeCharsetFromTextEncoding_TIS_620()
        {
            check( RTL_TEXTENCODING_TIS_620, RTL_TEXTENCODING_TIS_620 );
        }
        void MimeCharsetFromTextEncoding_KOI8_U()
        {
            check( RTL_TEXTENCODING_KOI8_U, RTL_TEXTENCODING_KOI8_U );
        }
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        void MimeCharsetFromTextEncoding_APPLE_JAPANESE()
        {
            check( RTL_TEXTENCODING_APPLE_JAPANESE, RTL_TEXTENCODING_SHIFT_JIS );
        }
        void MimeCharsetFromTextEncoding_SHIFT_JIS()
        {
            check( RTL_TEXTENCODING_SHIFT_JIS, RTL_TEXTENCODING_SHIFT_JIS );
        }
        void MimeCharsetFromTextEncoding_EUC_JP()
        {
            check( RTL_TEXTENCODING_EUC_JP, RTL_TEXTENCODING_EUC_JP );
        }
        void MimeCharsetFromTextEncoding_ISO_2022_JP()
        {
            check( RTL_TEXTENCODING_ISO_2022_JP, RTL_TEXTENCODING_ISO_2022_JP );
        }
        void MimeCharsetFromTextEncoding_JIS_X_0201()
        {
            check( RTL_TEXTENCODING_JIS_X_0201, RTL_TEXTENCODING_EUC_JP );
        }
        void MimeCharsetFromTextEncoding_JIS_X_0208()
        {
            check( RTL_TEXTENCODING_JIS_X_0208, RTL_TEXTENCODING_EUC_JP );
        }
        void MimeCharsetFromTextEncoding_JIS_X_0212()
        {
            check( RTL_TEXTENCODING_JIS_X_0212, RTL_TEXTENCODING_EUC_JP );
        }
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ko
        void MimeCharsetFromTextEncoding_APPLE_KOREAN()
        {
            check( RTL_TEXTENCODING_APPLE_KOREAN, RTL_TEXTENCODING_EUC_KR );
        }
        void MimeCharsetFromTextEncoding_EUC_KR()
        {
            check( RTL_TEXTENCODING_EUC_KR, RTL_TEXTENCODING_EUC_KR );
        }
        void MimeCharsetFromTextEncoding_ISO_2022_KR()
        {
            check( RTL_TEXTENCODING_ISO_2022_KR, RTL_TEXTENCODING_ISO_2022_KR );
        }
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        void MimeCharsetFromTextEncoding_APPLE_CHINSIMP()
        {
            check( RTL_TEXTENCODING_APPLE_CHINSIMP, RTL_TEXTENCODING_GB_2312 );
        }
        void MimeCharsetFromTextEncoding_APPLE_CHINTRAD()
        {
            check( RTL_TEXTENCODING_APPLE_CHINTRAD, RTL_TEXTENCODING_BIG5 );
        }
        void MimeCharsetFromTextEncoding_GB_2312()
        {
            check( RTL_TEXTENCODING_GB_2312, RTL_TEXTENCODING_GB_2312 );
        }
        void MimeCharsetFromTextEncoding_GBT_12345()
        {
            check( RTL_TEXTENCODING_GBT_12345, RTL_TEXTENCODING_GBT_12345 );
        }
        void MimeCharsetFromTextEncoding_GBK()
        {
            check( RTL_TEXTENCODING_GBK, RTL_TEXTENCODING_GBK );
        }
        void MimeCharsetFromTextEncoding_BIG5()
        {
            check( RTL_TEXTENCODING_BIG5, RTL_TEXTENCODING_BIG5 );
        }
        void MimeCharsetFromTextEncoding_EUC_CN()
        {
            check( RTL_TEXTENCODING_EUC_CN, RTL_TEXTENCODING_GB_2312 );
        }
        void MimeCharsetFromTextEncoding_EUC_TW()
        {
            check( RTL_TEXTENCODING_EUC_TW, RTL_TEXTENCODING_BIG5 );
        }
        void MimeCharsetFromTextEncoding_ISO_2022_CN()
        {
            check( RTL_TEXTENCODING_ISO_2022_CN, RTL_TEXTENCODING_ISO_2022_CN );
        }
        void MimeCharsetFromTextEncoding_GB_18030()
        {
            check( RTL_TEXTENCODING_GB_18030, RTL_TEXTENCODING_GB_18030 );
        }
        void MimeCharsetFromTextEncoding_BIG5_HKSCS()
        {
            check( RTL_TEXTENCODING_BIG5_HKSCS, RTL_TEXTENCODING_BIG5_HKSCS );
        }
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_FOR_SCRIPT_Deva
        void MimeCharsetFromTextEncoding_ISCII_DEVANAGARI()
        {
            check( RTL_TEXTENCODING_ISCII_DEVANAGARI, RTL_TEXTENCODING_ISCII_DEVANAGARI );
        }
#endif
        CPPUNIT_TEST_SUITE( testBestMime );

        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_1252 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_ROMAN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_437 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_850 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_860 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_861 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_863 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_865 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_SYMBOL );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ASCII_US );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_1 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_2 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_3 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_4 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_5 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_6 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_7 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_8 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_9 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_14 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_15 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_737 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_775 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_852 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_855 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_857 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_862 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_864 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_866 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_IBM_869 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_874 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_1250 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_1251 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_1253 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_1254 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_1255 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_1256 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_1257 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_1258 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_CENTEURO );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_CROATIAN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_CYRILLIC );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_GREEK );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_ICELAND );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_ROMANIAN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_TURKISH );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_UKRAINIAN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_932 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_936 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_949 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_950 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_KOI8_R );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_UTF7 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_UTF8 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_10 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_13 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_1361 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_TIS_620 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_KOI8_U );
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_JAPANESE );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_SHIFT_JIS );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_EUC_JP );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_2022_JP );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_JIS_X_0201 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_JIS_X_0208 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_JIS_X_0212 );
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ko
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_KOREAN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_EUC_KR );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_2022_KR );
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_CHINSIMP );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_CHINTRAD );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_GB_2312 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_GBT_12345 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_GBK );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_BIG5 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_EUC_CN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_EUC_TW );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_2022_CN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_GB_18030 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_BIG5_HKSCS );
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_FOR_SCRIPT_Deva
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISCII_DEVANAGARI );
#endif
        CPPUNIT_TEST_SUITE_END( );
    };

    class testBestUnix : public CppUnit::TestFixture
    {
    public:
        void check(rtl_TextEncoding eIn, rtl_TextEncoding eOut)
        {
            const sal_Char *pCharSet = rtl_getBestUnixCharsetFromTextEncoding(eIn);
            rtl_TextEncoding eTextEnc = rtl_getTextEncodingFromUnixCharset(pCharSet);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("rtl_getBestUnixCharsetFromTextEncoding && rtl_getTextEncodingFromUnixCharset", eOut, eTextEnc );
        }

        void UnixCharsetFromTextEncoding_MS_1252()
        {
            check( RTL_TEXTENCODING_MS_1252, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_APPLE_ROMAN()
        {
            check( RTL_TEXTENCODING_APPLE_ROMAN, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_IBM_437()
        {
            check( RTL_TEXTENCODING_IBM_437, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_IBM_850()
        {
            check( RTL_TEXTENCODING_IBM_850, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_IBM_860()
        {
            check( RTL_TEXTENCODING_IBM_860, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_IBM_861()
        {
            check( RTL_TEXTENCODING_IBM_861, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_IBM_863()
        {
            check( RTL_TEXTENCODING_IBM_863, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_IBM_865()
        {
            check( RTL_TEXTENCODING_IBM_865, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_SYMBOL()
        {
            check( RTL_TEXTENCODING_SYMBOL, RTL_TEXTENCODING_SYMBOL );
        }

        void UnixCharsetFromTextEncoding_ASCII_US()
        {
            check( RTL_TEXTENCODING_ASCII_US, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_1()
        {
            check( RTL_TEXTENCODING_ISO_8859_1, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_2()
        {
            check( RTL_TEXTENCODING_ISO_8859_2, RTL_TEXTENCODING_ISO_8859_2 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_3()
        {
            check( RTL_TEXTENCODING_ISO_8859_3, RTL_TEXTENCODING_ISO_8859_3 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_4()
        {
            check( RTL_TEXTENCODING_ISO_8859_4, RTL_TEXTENCODING_ISO_8859_4 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_5()
        {
            check( RTL_TEXTENCODING_ISO_8859_5, RTL_TEXTENCODING_ISO_8859_5 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_6()
        {
            check( RTL_TEXTENCODING_ISO_8859_6, RTL_TEXTENCODING_ISO_8859_6 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_7()
        {
            check( RTL_TEXTENCODING_ISO_8859_7, RTL_TEXTENCODING_ISO_8859_7 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_8()
        {
            check( RTL_TEXTENCODING_ISO_8859_8, RTL_TEXTENCODING_ISO_8859_8 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_9()
        {
            check( RTL_TEXTENCODING_ISO_8859_9, RTL_TEXTENCODING_ISO_8859_9 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_14()
        {
            check( RTL_TEXTENCODING_ISO_8859_14, RTL_TEXTENCODING_ISO_8859_14 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_15()
        {
            check( RTL_TEXTENCODING_ISO_8859_15, RTL_TEXTENCODING_ISO_8859_15 );
        }

        void UnixCharsetFromTextEncoding_IBM_737()
        {
            check( RTL_TEXTENCODING_IBM_737, RTL_TEXTENCODING_ISO_8859_7 );
        }

        void UnixCharsetFromTextEncoding_IBM_775()
        {
            check( RTL_TEXTENCODING_IBM_775, RTL_TEXTENCODING_ISO_8859_4 );
        }

        void UnixCharsetFromTextEncoding_IBM_852()
        {
            check( RTL_TEXTENCODING_IBM_852, RTL_TEXTENCODING_ISO_8859_2 );
        }

        void UnixCharsetFromTextEncoding_IBM_855()
        {
            check( RTL_TEXTENCODING_IBM_855, RTL_TEXTENCODING_ISO_8859_5 );
        }

        void UnixCharsetFromTextEncoding_IBM_857()
        {
            check( RTL_TEXTENCODING_IBM_857, RTL_TEXTENCODING_ISO_8859_9 );
        }

        void UnixCharsetFromTextEncoding_IBM_862()
        {
            check( RTL_TEXTENCODING_IBM_862, RTL_TEXTENCODING_ISO_8859_8 );
        }

        void UnixCharsetFromTextEncoding_IBM_864()
        {
            check( RTL_TEXTENCODING_IBM_864, RTL_TEXTENCODING_ISO_8859_6 );
        }

        void UnixCharsetFromTextEncoding_IBM_866()
        {
            check( RTL_TEXTENCODING_IBM_866, RTL_TEXTENCODING_ISO_8859_5 );
        }

        void UnixCharsetFromTextEncoding_IBM_869()
        {
            check( RTL_TEXTENCODING_IBM_869, RTL_TEXTENCODING_ISO_8859_7 );
        }

        void UnixCharsetFromTextEncoding_MS_874()
        {
            check( RTL_TEXTENCODING_MS_874, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_MS_1250()
        {
            check( RTL_TEXTENCODING_MS_1250, RTL_TEXTENCODING_ISO_8859_2 );
        }

        void UnixCharsetFromTextEncoding_MS_1251()
        {
            check( RTL_TEXTENCODING_MS_1251, RTL_TEXTENCODING_ISO_8859_5 );
        }

        void UnixCharsetFromTextEncoding_MS_1253()
        {
            check( RTL_TEXTENCODING_MS_1253, RTL_TEXTENCODING_ISO_8859_7 );
        }

        void UnixCharsetFromTextEncoding_MS_1254()
        {
            check( RTL_TEXTENCODING_MS_1254, RTL_TEXTENCODING_ISO_8859_9 );
        }

        void UnixCharsetFromTextEncoding_MS_1255()
        {
            check( RTL_TEXTENCODING_MS_1255, RTL_TEXTENCODING_ISO_8859_8 );
        }

        void UnixCharsetFromTextEncoding_MS_1256()
        {
            check( RTL_TEXTENCODING_MS_1256, RTL_TEXTENCODING_ISO_8859_6 );
        }

        void UnixCharsetFromTextEncoding_MS_1257()
        {
            check( RTL_TEXTENCODING_MS_1257, RTL_TEXTENCODING_ISO_8859_4 );
        }

        void UnixCharsetFromTextEncoding_MS_1258()
        {
            check( RTL_TEXTENCODING_MS_1258, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_APPLE_CENTEURO()
        {
            check( RTL_TEXTENCODING_APPLE_CENTEURO, RTL_TEXTENCODING_ISO_8859_2 );
        }

        void UnixCharsetFromTextEncoding_APPLE_CROATIAN()
        {
            check( RTL_TEXTENCODING_APPLE_CROATIAN, RTL_TEXTENCODING_ISO_8859_2 );
        }

        void UnixCharsetFromTextEncoding_APPLE_CYRILLIC()
        {
            check( RTL_TEXTENCODING_APPLE_CYRILLIC, RTL_TEXTENCODING_ISO_8859_5 );
        }

        void UnixCharsetFromTextEncoding_APPLE_GREEK()
        {
            check( RTL_TEXTENCODING_APPLE_GREEK, RTL_TEXTENCODING_ISO_8859_7 );
        }

        void UnixCharsetFromTextEncoding_APPLE_ICELAND()
        {
            check( RTL_TEXTENCODING_APPLE_ICELAND, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_APPLE_ROMANIAN()
        {
            check( RTL_TEXTENCODING_APPLE_ROMANIAN, RTL_TEXTENCODING_ISO_8859_2 );
        }

        void UnixCharsetFromTextEncoding_APPLE_TURKISH()
        {
            check( RTL_TEXTENCODING_APPLE_TURKISH, RTL_TEXTENCODING_ISO_8859_9 );
        }

        void UnixCharsetFromTextEncoding_APPLE_UKRAINIAN()
        {
            check( RTL_TEXTENCODING_APPLE_UKRAINIAN, RTL_TEXTENCODING_ISO_8859_5 );
        }
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        void UnixCharsetFromTextEncoding_APPLE_CHINSIMP()
        {
            check( RTL_TEXTENCODING_APPLE_CHINSIMP, RTL_TEXTENCODING_DONTKNOW );
        }

        void UnixCharsetFromTextEncoding_APPLE_CHINTRAD()
        {
            check( RTL_TEXTENCODING_APPLE_CHINTRAD, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        void UnixCharsetFromTextEncoding_APPLE_JAPANESE()
        {
            check( RTL_TEXTENCODING_APPLE_JAPANESE, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ko
        void UnixCharsetFromTextEncoding_APPLE_KOREAN()
        {
            check( RTL_TEXTENCODING_APPLE_KOREAN, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
        void UnixCharsetFromTextEncoding_MS_932()
        {
            check( RTL_TEXTENCODING_MS_932, RTL_TEXTENCODING_DONTKNOW );
        }

        void UnixCharsetFromTextEncoding_MS_936()
        {
            check( RTL_TEXTENCODING_MS_936, RTL_TEXTENCODING_DONTKNOW );
        }

        void UnixCharsetFromTextEncoding_MS_949()
        {
            check( RTL_TEXTENCODING_MS_949, RTL_TEXTENCODING_DONTKNOW );
        }

        void UnixCharsetFromTextEncoding_MS_950()
        {
            check( RTL_TEXTENCODING_MS_950, RTL_TEXTENCODING_DONTKNOW );
        }
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        void UnixCharsetFromTextEncoding_SHIFT_JIS()
        {
            check( RTL_TEXTENCODING_SHIFT_JIS, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        void UnixCharsetFromTextEncoding_GB_2312()
        {
            check( RTL_TEXTENCODING_GB_2312, RTL_TEXTENCODING_DONTKNOW );
        }

        void UnixCharsetFromTextEncoding_GBT_12345()
        {
            check( RTL_TEXTENCODING_GBT_12345, RTL_TEXTENCODING_DONTKNOW );
        }

        void UnixCharsetFromTextEncoding_GBK()
        {
            check( RTL_TEXTENCODING_GBK, RTL_TEXTENCODING_DONTKNOW );
        }

        void UnixCharsetFromTextEncoding_BIG5()
        {
            check( RTL_TEXTENCODING_BIG5, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        void UnixCharsetFromTextEncoding_EUC_JP()
        {
            check( RTL_TEXTENCODING_EUC_JP, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        void UnixCharsetFromTextEncoding_EUC_CN()
        {
            check( RTL_TEXTENCODING_EUC_CN, RTL_TEXTENCODING_DONTKNOW );
        }

        void UnixCharsetFromTextEncoding_EUC_TW()
        {
            check( RTL_TEXTENCODING_EUC_TW, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        void UnixCharsetFromTextEncoding_ISO_2022_JP()
        {
            check( RTL_TEXTENCODING_ISO_2022_JP, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        void UnixCharsetFromTextEncoding_ISO_2022_CN()
        {
            check( RTL_TEXTENCODING_ISO_2022_CN, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
        void UnixCharsetFromTextEncoding_KOI8_R()
        {
            check( RTL_TEXTENCODING_KOI8_R, RTL_TEXTENCODING_KOI8_R );
        }

        void UnixCharsetFromTextEncoding_UTF7()
        {
            check( RTL_TEXTENCODING_UTF7, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_UTF8()
        {
            check( RTL_TEXTENCODING_UTF8, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_10()
        {
            check( RTL_TEXTENCODING_ISO_8859_10, RTL_TEXTENCODING_ISO_8859_10 );
        }

        void UnixCharsetFromTextEncoding_ISO_8859_13()
        {
            check( RTL_TEXTENCODING_ISO_8859_13, RTL_TEXTENCODING_ISO_8859_13 );
        }
#if WITH_LOCALE_ALL || WITH_LOCALE_ko
        void UnixCharsetFromTextEncoding_EUC_KR()
        {
            check( RTL_TEXTENCODING_EUC_KR, RTL_TEXTENCODING_DONTKNOW );
        }

        void UnixCharsetFromTextEncoding_ISO_2022_KR()
        {
            check( RTL_TEXTENCODING_ISO_2022_KR, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        void UnixCharsetFromTextEncoding_JIS_X_0201()
        {
            check( RTL_TEXTENCODING_JIS_X_0201, RTL_TEXTENCODING_DONTKNOW );
        }

        void UnixCharsetFromTextEncoding_JIS_X_0208()
        {
            check( RTL_TEXTENCODING_JIS_X_0208, RTL_TEXTENCODING_DONTKNOW );
        }

        void UnixCharsetFromTextEncoding_JIS_X_0212()
        {
            check( RTL_TEXTENCODING_JIS_X_0212, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
        void UnixCharsetFromTextEncoding_MS_1361()
        {
            check( RTL_TEXTENCODING_MS_1361, RTL_TEXTENCODING_DONTKNOW );
        }
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        void UnixCharsetFromTextEncoding_GB_18030()
        {
            check( RTL_TEXTENCODING_GB_18030, RTL_TEXTENCODING_GBK );
        }

        void UnixCharsetFromTextEncoding_BIG5_HKSCS()
        {
            check( RTL_TEXTENCODING_BIG5_HKSCS, RTL_TEXTENCODING_DONTKNOW );
        }
#endif
        void UnixCharsetFromTextEncoding_TIS_620()
        {
            check( RTL_TEXTENCODING_TIS_620, RTL_TEXTENCODING_ISO_8859_1 );
        }

        void UnixCharsetFromTextEncoding_KOI8_U()
        {
            check( RTL_TEXTENCODING_KOI8_U, RTL_TEXTENCODING_KOI8_U );
        }

        CPPUNIT_TEST_SUITE( testBestUnix );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_1252 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_ROMAN );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_437 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_850 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_860 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_861 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_863 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_865 );

        CPPUNIT_TEST( UnixCharsetFromTextEncoding_SYMBOL );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ASCII_US );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_1 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_2 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_3 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_4 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_5 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_6 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_7 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_8 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_9 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_14 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_15 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_737 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_775 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_852 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_855 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_857 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_862 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_864 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_866 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_IBM_869 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_874 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_1250 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_1251 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_1253 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_1254 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_1255 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_1256 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_1257 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_1258 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_CENTEURO );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_CROATIAN );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_CYRILLIC );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_GREEK );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_ICELAND );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_ROMANIAN );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_TURKISH );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_UKRAINIAN );
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_CHINSIMP );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_CHINTRAD );
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_JAPANESE );
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ko
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_KOREAN );
#endif
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_932 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_936 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_949 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_950 );
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_SHIFT_JIS );
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_GB_2312 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_GBT_12345 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_GBK );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_BIG5 );
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_EUC_JP );
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_EUC_CN );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_EUC_TW );
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_2022_JP );
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_2022_CN );
#endif
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_KOI8_R );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_UTF7 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_UTF8 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_10 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_13 );
#if WITH_LOCALE_ALL || WITH_LOCALE_ko
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_EUC_KR );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_2022_KR );
#endif
#if WITH_LOCALE_ALL || WITH_LOCALE_ja
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_JIS_X_0201 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_JIS_X_0208 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_JIS_X_0212 );
#endif
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_1361 );
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_GB_18030 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_BIG5_HKSCS );
#endif
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_TIS_620 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_KOI8_U );

        CPPUNIT_TEST_SUITE_END( );
    };

    class testBestWindows : public CppUnit::TestFixture
    {
    public:
        void check(rtl_TextEncoding nIn, rtl_TextEncoding nOut)
        {
            const sal_uInt8 nCharSet = rtl_getBestWindowsCharsetFromTextEncoding(nIn);
            rtl_TextEncoding eTextEnc = rtl_getTextEncodingFromWindowsCharset(nCharSet);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("rtl_getBestWindowsCharsetFromTextEncoding && rtl_getTextEncodingFromWindowsCharset differ", nOut, eTextEnc );
        }

        void WindowsCharsetFromTextEncoding_MS_1252()
        {
            check( RTL_TEXTENCODING_MS_1252, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_ROMAN()
        {
            check( RTL_TEXTENCODING_APPLE_ROMAN, RTL_TEXTENCODING_APPLE_ROMAN );
        }

        void WindowsCharsetFromTextEncoding_IBM_437()
        {
            check( RTL_TEXTENCODING_IBM_437, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_850()
        {
            check( RTL_TEXTENCODING_IBM_850, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_860()
        {
            check( RTL_TEXTENCODING_IBM_860, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_861()
        {
            check( RTL_TEXTENCODING_IBM_861, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_863()
        {
            check( RTL_TEXTENCODING_IBM_863, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_865()
        {
            check( RTL_TEXTENCODING_IBM_865, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_SYMBOL()
        {
            check( RTL_TEXTENCODING_SYMBOL, RTL_TEXTENCODING_SYMBOL );
        }

        void WindowsCharsetFromTextEncoding_ASCII_US()
        {
            check( RTL_TEXTENCODING_ASCII_US, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISO_8859_1()
        {
            check( RTL_TEXTENCODING_ISO_8859_1, RTL_TEXTENCODING_MS_1252 );
        }
#if 0
        void WindowsCharsetFromTextEncoding_ISO_8859_2()
        {
            check( RTL_TEXTENCODING_ISO_8859_2, RTL_TEXTENCODING_MS_1252 );
        }
#endif
        void WindowsCharsetFromTextEncoding_ISO_8859_3()
        {
            check( RTL_TEXTENCODING_ISO_8859_3, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISO_8859_4()
        {
            check( RTL_TEXTENCODING_ISO_8859_4, RTL_TEXTENCODING_MS_1257 );
        }
#if 0
        void WindowsCharsetFromTextEncoding_ISO_8859_5()
        {
            check( RTL_TEXTENCODING_ISO_8859_5, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISO_8859_6()
        {
            check( RTL_TEXTENCODING_ISO_8859_6, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISO_8859_7()
        {
            check( RTL_TEXTENCODING_ISO_8859_7, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISO_8859_8()
        {
            check( RTL_TEXTENCODING_ISO_8859_8, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISO_8859_9()
        {
            check( RTL_TEXTENCODING_ISO_8859_9, RTL_TEXTENCODING_MS_1252 );
        }
#endif
        void WindowsCharsetFromTextEncoding_ISO_8859_14()
        {
            check( RTL_TEXTENCODING_ISO_8859_14, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISO_8859_15()
        {
            check( RTL_TEXTENCODING_ISO_8859_15, RTL_TEXTENCODING_MS_1252 );
        }
#if 0
        void WindowsCharsetFromTextEncoding_IBM_737()
        {
            check( RTL_TEXTENCODING_IBM_737, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_775()
        {
            check( RTL_TEXTENCODING_IBM_775, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_852()
        {
            check( RTL_TEXTENCODING_IBM_852, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_855()
        {
            check( RTL_TEXTENCODING_IBM_855, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_857()
        {
            check( RTL_TEXTENCODING_IBM_857, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_862()
        {
            check( RTL_TEXTENCODING_IBM_862, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_864()
        {
            check( RTL_TEXTENCODING_IBM_864, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_866()
        {
            check( RTL_TEXTENCODING_IBM_866, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_IBM_869()
        {
            check( RTL_TEXTENCODING_IBM_869, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_874()
        {
            check( RTL_TEXTENCODING_MS_874, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_1250()
        {
            check( RTL_TEXTENCODING_MS_1250, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_1251()
        {
            check( RTL_TEXTENCODING_MS_1251, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_1253()
        {
            check( RTL_TEXTENCODING_MS_1253, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_1254()
        {
            check( RTL_TEXTENCODING_MS_1254, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_1255()
        {
            check( RTL_TEXTENCODING_MS_1255, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_1256()
        {
            check( RTL_TEXTENCODING_MS_1256, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_1257()
        {
            check( RTL_TEXTENCODING_MS_1257, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_1258()
        {
            check( RTL_TEXTENCODING_MS_1258, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_ARABIC()
        {
            check( RTL_TEXTENCODING_APPLE_ARABIC, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_CENTEURO()
        {
            check( RTL_TEXTENCODING_APPLE_CENTEURO, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_CROATIAN()
        {
            check( RTL_TEXTENCODING_APPLE_CROATIAN, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_CYRILLIC()
        {
            check( RTL_TEXTENCODING_APPLE_CYRILLIC, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_DEVANAGARI()
        {
            check( RTL_TEXTENCODING_APPLE_DEVANAGARI, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_FARSI()
        {
            check( RTL_TEXTENCODING_APPLE_FARSI, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_GREEK()
        {
            check( RTL_TEXTENCODING_APPLE_GREEK, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_GUJARATI()
        {
            check( RTL_TEXTENCODING_APPLE_GUJARATI, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_GURMUKHI()
        {
            check( RTL_TEXTENCODING_APPLE_GURMUKHI, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_HEBREW()
        {
            check( RTL_TEXTENCODING_APPLE_HEBREW, RTL_TEXTENCODING_MS_1252 );
        }
#endif
        void WindowsCharsetFromTextEncoding_APPLE_ICELAND()
        {
            check( RTL_TEXTENCODING_APPLE_ICELAND, RTL_TEXTENCODING_MS_1252 );
        }
#if 0
        void WindowsCharsetFromTextEncoding_APPLE_ROMANIAN()
        {
            check( RTL_TEXTENCODING_APPLE_ROMANIAN, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_THAI()
        {
            check( RTL_TEXTENCODING_APPLE_THAI, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_TURKISH()
        {
            check( RTL_TEXTENCODING_APPLE_TURKISH, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_UKRAINIAN()
        {
            check( RTL_TEXTENCODING_APPLE_UKRAINIAN, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_CHINSIMP()
        {
            check( RTL_TEXTENCODING_APPLE_CHINSIMP, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_CHINTRAD()
        {
            check( RTL_TEXTENCODING_APPLE_CHINTRAD, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_JAPANESE()
        {
            check( RTL_TEXTENCODING_APPLE_JAPANESE, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_APPLE_KOREAN()
        {
            check( RTL_TEXTENCODING_APPLE_KOREAN, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_932()
        {
            check( RTL_TEXTENCODING_MS_932, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_936()
        {
            check( RTL_TEXTENCODING_MS_936, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_949()
        {
            check( RTL_TEXTENCODING_MS_949, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_950()
        {
            check( RTL_TEXTENCODING_MS_950, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_SHIFT_JIS()
        {
            check( RTL_TEXTENCODING_SHIFT_JIS, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_GB_2312()
        {
            check( RTL_TEXTENCODING_GB_2312, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_GBT_12345()
        {
            check( RTL_TEXTENCODING_GBT_12345, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_GBK()
        {
            check( RTL_TEXTENCODING_GBK, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_BIG5()
        {
            check( RTL_TEXTENCODING_BIG5, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_EUC_JP()
        {
            check( RTL_TEXTENCODING_EUC_JP, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_EUC_CN()
        {
            check( RTL_TEXTENCODING_EUC_CN, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_EUC_TW()
        {
            check( RTL_TEXTENCODING_EUC_TW, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISO_2022_JP()
        {
            check( RTL_TEXTENCODING_ISO_2022_JP, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISO_2022_CN()
        {
            check( RTL_TEXTENCODING_ISO_2022_CN, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_KOI8_R()
        {
            check( RTL_TEXTENCODING_KOI8_R, RTL_TEXTENCODING_MS_1252 );
        }
#endif
        void WindowsCharsetFromTextEncoding_UTF7()
        {
            check( RTL_TEXTENCODING_UTF7, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_UTF8()
        {
            check( RTL_TEXTENCODING_UTF8, RTL_TEXTENCODING_MS_1252 );
        }
#if 0
        void WindowsCharsetFromTextEncoding_ISO_8859_10()
        {
            check( RTL_TEXTENCODING_ISO_8859_10, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISO_8859_13()
        {
            check( RTL_TEXTENCODING_ISO_8859_13, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_EUC_KR()
        {
            check( RTL_TEXTENCODING_EUC_KR, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISO_2022_KR()
        {
            check( RTL_TEXTENCODING_ISO_2022_KR, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_JIS_X_0201()
        {
            check( RTL_TEXTENCODING_JIS_X_0201, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_JIS_X_0208()
        {
            check( RTL_TEXTENCODING_JIS_X_0208, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_JIS_X_0212()
        {
            check( RTL_TEXTENCODING_JIS_X_0212, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_MS_1361()
        {
            check( RTL_TEXTENCODING_MS_1361, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_GB_18030()
        {
            check( RTL_TEXTENCODING_GB_18030, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_BIG5_HKSCS()
        {
            check( RTL_TEXTENCODING_BIG5_HKSCS, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_TIS_620()
        {
            check( RTL_TEXTENCODING_TIS_620, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_KOI8_U()
        {
            check( RTL_TEXTENCODING_KOI8_U, RTL_TEXTENCODING_MS_1252 );
        }

        void WindowsCharsetFromTextEncoding_ISCII_DEVANAGARI()
        {
            check( RTL_TEXTENCODING_ISCII_DEVANAGARI, RTL_TEXTENCODING_MS_1252 );
        }
#endif
        void WindowsCharsetFromTextEncoding_JAVA_UTF8()
        {
            check( RTL_TEXTENCODING_JAVA_UTF8, RTL_TEXTENCODING_MS_1252 );
        }

        CPPUNIT_TEST_SUITE( testBestWindows );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_1252 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_ROMAN );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_437 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_850 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_860 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_861 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_863 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_865 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_SYMBOL );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ASCII_US );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_1 );
#if 0
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_2 );
#endif
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_3 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_4 );
#if 0
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_5 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_6 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_7 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_8 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_9 );
#endif
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_14 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_15 );
#if 0
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_737 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_775 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_852 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_855 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_857 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_862 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_864 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_866 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_IBM_869 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_874 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_1250 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_1251 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_1253 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_1254 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_1255 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_1256 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_1257 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_1258 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_ARABIC );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_CENTEURO );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_CROATIAN );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_CYRILLIC );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_DEVANAGARI );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_FARSI );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_GREEK );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_GUJARATI );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_GURMUKHI );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_HEBREW );
#endif
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_ICELAND );
#if 0
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_ROMANIAN );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_THAI );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_TURKISH );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_UKRAINIAN );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_CHINSIMP );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_CHINTRAD );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_JAPANESE );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_KOREAN );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_932 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_936 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_949 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_950 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_SHIFT_JIS );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_GB_2312 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_GBT_12345 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_GBK );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_BIG5 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_EUC_JP );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_EUC_CN );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_EUC_TW );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_2022_JP );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_2022_CN );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_KOI8_R );
#endif
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_UTF7 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_UTF8 );
#if 0
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_10 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_13 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_EUC_KR );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_2022_KR );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_JIS_X_0201 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_JIS_X_0208 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_JIS_X_0212 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_MS_1361 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_GB_18030 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_BIG5_HKSCS );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_TIS_620 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_KOI8_U );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISCII_DEVANAGARI );
#endif
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_JAVA_UTF8 );

        CPPUNIT_TEST_SUITE_END( );
    };

    class testTextEncodingInfo: public CppUnit::TestFixture
    {
    public:
        // not implemented encoding test
        void testTextEncodingInfo_001()
            {
                rtl_TextEncodingInfo aInfo1, aInfo2, aInfo3, aInfo4, aInfo5;
                aInfo1.StructSize = 4;
                // not implemented
                bool bRes1 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo1 );
                // implemented
                bool bRes11 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo1 );
                CPPUNIT_ASSERT_MESSAGE("should return sal_False.", !bRes1 && !bRes11);

                aInfo2.StructSize = 5;
                bool bRes2 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo2 );
                bool bRes21 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo2 );
                CPPUNIT_ASSERT_MESSAGE("StructSize<6 should return sal_True", bRes2 && bRes21 && aInfo2.MinimumCharSize >=1 );

                aInfo3.StructSize = 6;
                bool bRes3 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo3 );
                bool bRes31 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo3 );
                CPPUNIT_ASSERT_MESSAGE("StructSize<6 should return sal_True", bRes3 && bRes31);
//&& aInfo2.MinimumCharSize >=1 );

                aInfo4.StructSize = 8;
                bool bRes4 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo4 );
                bool bRes41 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo4);
                CPPUNIT_ASSERT_MESSAGE("StructSize<6 should return sal_True", bRes4 && bRes41);
// && aInfo2.MinimumCharSize >=1 );

                aInfo5.StructSize = sizeof aInfo5;
                bool bRes5 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo5 );
                CPPUNIT_ASSERT_MESSAGE("StructSize<6 should return sal_True", !bRes5 && aInfo5.Flags == 0);

            }
        CPPUNIT_TEST_SUITE(testTextEncodingInfo);
        CPPUNIT_TEST(testTextEncodingInfo_001);
        CPPUNIT_TEST_SUITE_END();
    };

    class testEncodingFromUnix: public CppUnit::TestFixture
    {
    public:
        void testIso8859() {
            check(RTL_TEXTENCODING_DONTKNOW, "ISO8859");
            check(RTL_TEXTENCODING_DONTKNOW, "ISO8859-0");
            check(RTL_TEXTENCODING_DONTKNOW, "ISO8859-01");
            check(RTL_TEXTENCODING_DONTKNOW, "ISO8859_1");
            check(RTL_TEXTENCODING_DONTKNOW, "ISO88591");
            check(RTL_TEXTENCODING_ISO_8859_1, "ISO8859-1");
            check(RTL_TEXTENCODING_ISO_8859_2, "ISO8859-2");
            check(RTL_TEXTENCODING_ISO_8859_3, "ISO8859-3");
            check(RTL_TEXTENCODING_ISO_8859_4, "ISO8859-4");
            check(RTL_TEXTENCODING_ISO_8859_5, "ISO8859-5");
            check(RTL_TEXTENCODING_ISO_8859_6, "ISO8859-6");
            check(RTL_TEXTENCODING_ISO_8859_7, "ISO8859-7");
            check(RTL_TEXTENCODING_ISO_8859_8, "ISO8859-8");
            check(RTL_TEXTENCODING_ISO_8859_9, "ISO8859-9");
            check(RTL_TEXTENCODING_ISO_8859_10, "ISO8859-10");
            check(RTL_TEXTENCODING_TIS_620, "ISO8859-11");
            check(RTL_TEXTENCODING_ISO_8859_13, "ISO8859-13");
            check(RTL_TEXTENCODING_ISO_8859_14, "ISO8859-14");
            check(RTL_TEXTENCODING_ISO_8859_15, "ISO8859-15");
        }

        void testTis620() {
            check(RTL_TEXTENCODING_DONTKNOW, "TIS620");
            check(RTL_TEXTENCODING_TIS_620, "TIS620-0");
            check(RTL_TEXTENCODING_DONTKNOW, "TIS620-1");
            check(RTL_TEXTENCODING_TIS_620, "TIS620-2529");
            check(RTL_TEXTENCODING_TIS_620, "TIS620-2533");
            check(RTL_TEXTENCODING_DONTKNOW, "TIS620.2529-0");
            check(RTL_TEXTENCODING_TIS_620, "TIS620.2529-1");
            check(RTL_TEXTENCODING_DONTKNOW, "TIS620.2529-2");
            check(RTL_TEXTENCODING_TIS_620, "TIS620.2533-0");
            check(RTL_TEXTENCODING_TIS_620, "TIS620.2533-1");
            check(RTL_TEXTENCODING_DONTKNOW, "TIS620.2533-2");
        }

        CPPUNIT_TEST_SUITE(testEncodingFromUnix);
        CPPUNIT_TEST(testIso8859);
        CPPUNIT_TEST(testTis620);
        CPPUNIT_TEST_SUITE_END();

    private:
        void check(rtl_TextEncoding expected, char const * input) {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                input, expected, rtl_getTextEncodingFromUnixCharset(input));
        }
    };

}

CPPUNIT_TEST_SUITE_REGISTRATION(testBestMime);
CPPUNIT_TEST_SUITE_REGISTRATION(testBestUnix);
CPPUNIT_TEST_SUITE_REGISTRATION(testBestWindows);
CPPUNIT_TEST_SUITE_REGISTRATION(testTextEncodingInfo);
CPPUNIT_TEST_SUITE_REGISTRATION(testEncodingFromUnix);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
