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
#include <string.h>

#include <osl/thread.h>
#include <rtl/tencinfo.h>

#include <testshl/simpleheader.hxx>

// -----------------------------------------------------------------------------

namespace rtl_tencinfo
{
    class getBestMime : public CppUnit::TestFixture
    {
    public:
        void setUp()
            {
            }

        void check( const sal_Char* _pRTL_TEXTENCODING, rtl_TextEncoding _aCurrentEncode )
            {
                const sal_Char *pCharSet = rtl_getBestMimeCharsetFromTextEncoding( _aCurrentEncode );
                if (pCharSet == 0)
                {
                    t_print("rtl_getBestMimeCharsetFromTextEncoding(%s) (%d) doesn't seem to exist.\n\n", _pRTL_TEXTENCODING, _aCurrentEncode);
                }
                else
                {
                    t_print(T_VERBOSE,     "'%s' is charset: '%s'\n", _pRTL_TEXTENCODING, pCharSet);

                    rtl_TextEncoding eTextEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );
                    if (_aCurrentEncode != eTextEnc &&
                        eTextEnc != RTL_TEXTENCODING_DONTKNOW)
                    {
                        t_print("rtl_getBestMimeCharsetFromTextEncoding(%s) is charset: %s\n", _pRTL_TEXTENCODING, pCharSet);
                        t_print("rtl_getTextEncodingFromMimeCharset() differ: %s %d -> %d\n\n", _pRTL_TEXTENCODING, _aCurrentEncode, eTextEnc );
                    }
                    // rtl::OString sError = "getTextEncodingFromMimeCharset(";
                    // sError += pCharSet;
                    // sError += ") returns null";
                    // CPPUNIT_ASSERT_MESSAGE(sError.getStr(), eTextEnc != RTL_TEXTENCODING_DONTKNOW);
                    // CPPUNIT_ASSERT_MESSAGE("Does not realize itself", _aCurrentEncode == eTextEnc );
                }
            }

// the defines for the follows test could be found in file inc/rtl/textenc.h

        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_1252()
            {
                check( "RTL_TEXTENCODING_MS_1252", RTL_TEXTENCODING_MS_1252 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_ROMAN()
            {
                check( "RTL_TEXTENCODING_APPLE_ROMAN", RTL_TEXTENCODING_APPLE_ROMAN );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_437()
            {
                check( "RTL_TEXTENCODING_IBM_437", RTL_TEXTENCODING_IBM_437 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_850()
            {
                check( "RTL_TEXTENCODING_IBM_850", RTL_TEXTENCODING_IBM_850 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_860()
            {
                check( "RTL_TEXTENCODING_IBM_860", RTL_TEXTENCODING_IBM_860 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_861()
            {
                check( "RTL_TEXTENCODING_IBM_861", RTL_TEXTENCODING_IBM_861 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_863()
            {
                check( "RTL_TEXTENCODING_IBM_863", RTL_TEXTENCODING_IBM_863 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_865()
            {
                check( "RTL_TEXTENCODING_IBM_865", RTL_TEXTENCODING_IBM_865 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_SYMBOL()
            {
                check( "RTL_TEXTENCODING_SYMBOL", RTL_TEXTENCODING_SYMBOL );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ASCII_US()
            {
                check( "RTL_TEXTENCODING_ASCII_US", RTL_TEXTENCODING_ASCII_US );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_1()
            {
                check( "RTL_TEXTENCODING_ISO_8859_1", RTL_TEXTENCODING_ISO_8859_1 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_2()
            {
                check( "RTL_TEXTENCODING_ISO_8859_2", RTL_TEXTENCODING_ISO_8859_2 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_3()
            {
                check( "RTL_TEXTENCODING_ISO_8859_3", RTL_TEXTENCODING_ISO_8859_3 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_4()
            {
                check( "RTL_TEXTENCODING_ISO_8859_4", RTL_TEXTENCODING_ISO_8859_4 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_5()
            {
                check( "RTL_TEXTENCODING_ISO_8859_5", RTL_TEXTENCODING_ISO_8859_5 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_6()
            {
                check( "RTL_TEXTENCODING_ISO_8859_6", RTL_TEXTENCODING_ISO_8859_6 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_7()
            {
                check( "RTL_TEXTENCODING_ISO_8859_7", RTL_TEXTENCODING_ISO_8859_7 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_8()
            {
                check( "RTL_TEXTENCODING_ISO_8859_8", RTL_TEXTENCODING_ISO_8859_8 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_9()
            {
                check( "RTL_TEXTENCODING_ISO_8859_9", RTL_TEXTENCODING_ISO_8859_9 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_14()
            {
                check( "RTL_TEXTENCODING_ISO_8859_14", RTL_TEXTENCODING_ISO_8859_14 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_15()
            {
                check( "RTL_TEXTENCODING_ISO_8859_15", RTL_TEXTENCODING_ISO_8859_15 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_737()
            {
                check( "RTL_TEXTENCODING_IBM_737", RTL_TEXTENCODING_IBM_737 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_775()
            {
                check( "RTL_TEXTENCODING_IBM_775", RTL_TEXTENCODING_IBM_775 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_852()
            {
                check( "RTL_TEXTENCODING_IBM_852", RTL_TEXTENCODING_IBM_852 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_855()
            {
                check( "RTL_TEXTENCODING_IBM_855", RTL_TEXTENCODING_IBM_855 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_857()
            {
                check( "RTL_TEXTENCODING_IBM_857", RTL_TEXTENCODING_IBM_857 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_862()
            {
                check( "RTL_TEXTENCODING_IBM_862", RTL_TEXTENCODING_IBM_862 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_864()
            {
                check( "RTL_TEXTENCODING_IBM_864", RTL_TEXTENCODING_IBM_864 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_866()
            {
                check( "RTL_TEXTENCODING_IBM_866", RTL_TEXTENCODING_IBM_866 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_IBM_869()
            {
                check( "RTL_TEXTENCODING_IBM_869", RTL_TEXTENCODING_IBM_869 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_874()
            {
                check( "RTL_TEXTENCODING_MS_874", RTL_TEXTENCODING_MS_874 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_1250()
            {
                check( "RTL_TEXTENCODING_MS_1250", RTL_TEXTENCODING_MS_1250 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_1251()
            {
                check( "RTL_TEXTENCODING_MS_1251", RTL_TEXTENCODING_MS_1251 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_1253()
            {
                check( "RTL_TEXTENCODING_MS_1253", RTL_TEXTENCODING_MS_1253 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_1254()
            {
                check( "RTL_TEXTENCODING_MS_1254", RTL_TEXTENCODING_MS_1254 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_1255()
            {
                check( "RTL_TEXTENCODING_MS_1255", RTL_TEXTENCODING_MS_1255 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_1256()
            {
                check( "RTL_TEXTENCODING_MS_1256", RTL_TEXTENCODING_MS_1256 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_1257()
            {
                check( "RTL_TEXTENCODING_MS_1257", RTL_TEXTENCODING_MS_1257 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_1258()
            {
                check( "RTL_TEXTENCODING_MS_1258", RTL_TEXTENCODING_MS_1258 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_ARABIC()
            {
                check( "RTL_TEXTENCODING_APPLE_ARABIC", RTL_TEXTENCODING_APPLE_ARABIC );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_CENTEURO()
            {
                check( "RTL_TEXTENCODING_APPLE_CENTEURO", RTL_TEXTENCODING_APPLE_CENTEURO );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_CROATIAN()
            {
                check( "RTL_TEXTENCODING_APPLE_CROATIAN", RTL_TEXTENCODING_APPLE_CROATIAN );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_CYRILLIC()
            {
                check( "RTL_TEXTENCODING_APPLE_CYRILLIC", RTL_TEXTENCODING_APPLE_CYRILLIC );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_DEVANAGARI()
            {
                check( "RTL_TEXTENCODING_APPLE_DEVANAGARI", RTL_TEXTENCODING_APPLE_DEVANAGARI );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_FARSI()
            {
                check( "RTL_TEXTENCODING_APPLE_FARSI", RTL_TEXTENCODING_APPLE_FARSI );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_GREEK()
            {
                check( "RTL_TEXTENCODING_APPLE_GREEK", RTL_TEXTENCODING_APPLE_GREEK );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_GUJARATI()
            {
                check( "RTL_TEXTENCODING_APPLE_GUJARATI", RTL_TEXTENCODING_APPLE_GUJARATI );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_GURMUKHI()
            {
                check( "RTL_TEXTENCODING_APPLE_GURMUKHI", RTL_TEXTENCODING_APPLE_GURMUKHI );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_HEBREW()
            {
                check( "RTL_TEXTENCODING_APPLE_HEBREW", RTL_TEXTENCODING_APPLE_HEBREW );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_ICELAND()
            {
                check( "RTL_TEXTENCODING_APPLE_ICELAND", RTL_TEXTENCODING_APPLE_ICELAND );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_ROMANIAN()
            {
                check( "RTL_TEXTENCODING_APPLE_ROMANIAN", RTL_TEXTENCODING_APPLE_ROMANIAN );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_THAI()
            {
                check( "RTL_TEXTENCODING_APPLE_THAI", RTL_TEXTENCODING_APPLE_THAI );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_TURKISH()
            {
                check( "RTL_TEXTENCODING_APPLE_TURKISH", RTL_TEXTENCODING_APPLE_TURKISH );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_UKRAINIAN()
            {
                check( "RTL_TEXTENCODING_APPLE_UKRAINIAN", RTL_TEXTENCODING_APPLE_UKRAINIAN );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_CHINSIMP()
            {
                check( "RTL_TEXTENCODING_APPLE_CHINSIMP", RTL_TEXTENCODING_APPLE_CHINSIMP );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_CHINTRAD()
            {
                check( "RTL_TEXTENCODING_APPLE_CHINTRAD", RTL_TEXTENCODING_APPLE_CHINTRAD );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_JAPANESE()
            {
                check( "RTL_TEXTENCODING_APPLE_JAPANESE", RTL_TEXTENCODING_APPLE_JAPANESE );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_APPLE_KOREAN()
            {
                check( "RTL_TEXTENCODING_APPLE_KOREAN", RTL_TEXTENCODING_APPLE_KOREAN );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_932()
            {
                check( "RTL_TEXTENCODING_MS_932", RTL_TEXTENCODING_MS_932 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_936()
            {
                check( "RTL_TEXTENCODING_MS_936", RTL_TEXTENCODING_MS_936 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_949()
            {
                check( "RTL_TEXTENCODING_MS_949", RTL_TEXTENCODING_MS_949 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_950()
            {
                check( "RTL_TEXTENCODING_MS_950", RTL_TEXTENCODING_MS_950 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_SHIFT_JIS()
            {
                check( "RTL_TEXTENCODING_SHIFT_JIS", RTL_TEXTENCODING_SHIFT_JIS );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_GB_2312()
            {
                check( "RTL_TEXTENCODING_GB_2312", RTL_TEXTENCODING_GB_2312 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_GBT_12345()
            {
                check( "RTL_TEXTENCODING_GBT_12345", RTL_TEXTENCODING_GBT_12345 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_GBK()
            {
                check( "RTL_TEXTENCODING_GBK", RTL_TEXTENCODING_GBK );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_BIG5()
            {
                check( "RTL_TEXTENCODING_BIG5", RTL_TEXTENCODING_BIG5 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_EUC_JP()
            {
                check( "RTL_TEXTENCODING_EUC_JP", RTL_TEXTENCODING_EUC_JP );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_EUC_CN()
            {
                check( "RTL_TEXTENCODING_EUC_CN", RTL_TEXTENCODING_EUC_CN );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_EUC_TW()
            {
                check( "RTL_TEXTENCODING_EUC_TW", RTL_TEXTENCODING_EUC_TW );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_2022_JP()
            {
                check( "RTL_TEXTENCODING_ISO_2022_JP", RTL_TEXTENCODING_ISO_2022_JP );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_2022_CN()
            {
                check( "RTL_TEXTENCODING_ISO_2022_CN", RTL_TEXTENCODING_ISO_2022_CN );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_KOI8_R()
            {
                check( "RTL_TEXTENCODING_KOI8_R", RTL_TEXTENCODING_KOI8_R );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_UTF7()
            {
                check( "RTL_TEXTENCODING_UTF7", RTL_TEXTENCODING_UTF7 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_UTF8()
            {
                check( "RTL_TEXTENCODING_UTF8", RTL_TEXTENCODING_UTF8 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_10()
            {
                check( "RTL_TEXTENCODING_ISO_8859_10", RTL_TEXTENCODING_ISO_8859_10 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_8859_13()
            {
                check( "RTL_TEXTENCODING_ISO_8859_13", RTL_TEXTENCODING_ISO_8859_13 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_EUC_KR()
            {
                check( "RTL_TEXTENCODING_EUC_KR", RTL_TEXTENCODING_EUC_KR );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISO_2022_KR()
            {
                check( "RTL_TEXTENCODING_ISO_2022_KR", RTL_TEXTENCODING_ISO_2022_KR );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_JIS_X_0201()
            {
                check( "RTL_TEXTENCODING_JIS_X_0201", RTL_TEXTENCODING_JIS_X_0201 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_JIS_X_0208()
            {
                check( "RTL_TEXTENCODING_JIS_X_0208", RTL_TEXTENCODING_JIS_X_0208 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_JIS_X_0212()
            {
                check( "RTL_TEXTENCODING_JIS_X_0212", RTL_TEXTENCODING_JIS_X_0212 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_MS_1361()
            {
                check( "RTL_TEXTENCODING_MS_1361", RTL_TEXTENCODING_MS_1361 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_GB_18030()
            {
                check( "RTL_TEXTENCODING_GB_18030", RTL_TEXTENCODING_GB_18030 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_BIG5_HKSCS()
            {
                check( "RTL_TEXTENCODING_BIG5_HKSCS", RTL_TEXTENCODING_BIG5_HKSCS );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_TIS_620()
            {
                check( "RTL_TEXTENCODING_TIS_620", RTL_TEXTENCODING_TIS_620 );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_KOI8_U()
            {
                check( "RTL_TEXTENCODING_KOI8_U", RTL_TEXTENCODING_KOI8_U );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_ISCII_DEVANAGARI()
            {
                check( "RTL_TEXTENCODING_ISCII_DEVANAGARI", RTL_TEXTENCODING_ISCII_DEVANAGARI );
            }
        // ----------------------------------------
        void MimeCharsetFromTextEncoding_JAVA_UTF8()
            {
                check( "RTL_TEXTENCODING_JAVA_UTF8", RTL_TEXTENCODING_JAVA_UTF8 );
            }

/* ATTENTION!  Whenever some encoding is added here, make sure to update
 * rtl_isOctetEncoding in tencinfo.c.
 */

// RTL_TEXTENCODING_USER_START
// RTL_TEXTENCODING_USER_END

//                check( "RTL_TEXTENCODING_UCS4", RTL_TEXTENCODING_UCS4 );
//                check( "RTL_TEXTENCODING_UCS2", RTL_TEXTENCODING_UCS2 );
//                check( "RTL_TEXTENCODING_UNICODE", RTL_TEXTENCODING_UNICODE /* RTL_TEXTENCODING_UCS2 */ );

        CPPUNIT_TEST_SUITE( getBestMime );

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
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_ARABIC );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_CENTEURO );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_CROATIAN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_CYRILLIC );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_DEVANAGARI );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_FARSI );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_GREEK );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_GUJARATI );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_GURMUKHI );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_HEBREW );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_ICELAND );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_ROMANIAN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_THAI );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_TURKISH );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_UKRAINIAN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_CHINSIMP );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_CHINTRAD );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_JAPANESE );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_APPLE_KOREAN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_932 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_936 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_949 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_950 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_SHIFT_JIS );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_GB_2312 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_GBT_12345 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_GBK );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_BIG5 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_EUC_JP );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_EUC_CN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_EUC_TW );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_2022_JP );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_2022_CN );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_KOI8_R );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_UTF7 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_UTF8 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_10 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_8859_13 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_EUC_KR );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISO_2022_KR );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_JIS_X_0201 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_JIS_X_0208 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_JIS_X_0212 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_MS_1361 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_GB_18030 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_BIG5_HKSCS );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_TIS_620 );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_KOI8_U );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_ISCII_DEVANAGARI );
        CPPUNIT_TEST( MimeCharsetFromTextEncoding_JAVA_UTF8 );

        CPPUNIT_TEST_SUITE_END( );
    };

    // -----------------------------------------------------------------------------

    class getBestUnix : public CppUnit::TestFixture
    {
    public:
        void setUp()
            {
            }

        void check( const sal_Char* _pRTL_TEXTENCODING, rtl_TextEncoding _aCurrentEncode )
            {
                const sal_Char *pCharSet = rtl_getBestUnixCharsetFromTextEncoding( _aCurrentEncode );
                if (pCharSet == 0)
                {
                    t_print("rtl_getBestUnixCharsetFromTextEncoding(%s) (%d) doesn't seem to exist.\n\n", _pRTL_TEXTENCODING, _aCurrentEncode);
                }
                else
                {
                    t_print(T_VERBOSE, "'%s' is charset: '%s'\n", _pRTL_TEXTENCODING, pCharSet);

                    rtl_TextEncoding eTextEnc = rtl_getTextEncodingFromUnixCharset( pCharSet );
                    if (_aCurrentEncode != eTextEnc &&
                        eTextEnc != RTL_TEXTENCODING_DONTKNOW)
                    {
                        t_print("rtl_getBestUnixCharsetFromTextEncoding(%s) is charset: %s\n", _pRTL_TEXTENCODING, pCharSet);
                        t_print("rtl_getTextEncodingFromUnixCharset() differ: %s %d -> %d\n\n", _pRTL_TEXTENCODING, _aCurrentEncode, eTextEnc );
                    }
                    // rtl::OString sError = "getTextEncodingFromUnixCharset(";
                    // sError += pCharSet;
                    // sError += ") returns null";
                    // CPPUNIT_ASSERT_MESSAGE(sError.getStr(), eTextEnc != RTL_TEXTENCODING_DONTKNOW);
                    // CPPUNIT_ASSERT_MESSAGE("Does not realize itself", _aCurrentEncode == eTextEnc );
                }
            }


        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_1252()
            {
                check( "RTL_TEXTENCODING_MS_1252", RTL_TEXTENCODING_MS_1252 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_ROMAN()
            {
                check( "RTL_TEXTENCODING_APPLE_ROMAN", RTL_TEXTENCODING_APPLE_ROMAN );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_437()
            {
                check( "RTL_TEXTENCODING_IBM_437", RTL_TEXTENCODING_IBM_437 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_850()
            {
                check( "RTL_TEXTENCODING_IBM_850", RTL_TEXTENCODING_IBM_850 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_860()
            {
                check( "RTL_TEXTENCODING_IBM_860", RTL_TEXTENCODING_IBM_860 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_861()
            {
                check( "RTL_TEXTENCODING_IBM_861", RTL_TEXTENCODING_IBM_861 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_863()
            {
                check( "RTL_TEXTENCODING_IBM_863", RTL_TEXTENCODING_IBM_863 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_865()
            {
                check( "RTL_TEXTENCODING_IBM_865", RTL_TEXTENCODING_IBM_865 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_SYMBOL()
            {
                check( "RTL_TEXTENCODING_SYMBOL", RTL_TEXTENCODING_SYMBOL );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ASCII_US()
            {
                check( "RTL_TEXTENCODING_ASCII_US", RTL_TEXTENCODING_ASCII_US );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_1()
            {
                check( "RTL_TEXTENCODING_ISO_8859_1", RTL_TEXTENCODING_ISO_8859_1 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_2()
            {
                check( "RTL_TEXTENCODING_ISO_8859_2", RTL_TEXTENCODING_ISO_8859_2 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_3()
            {
                check( "RTL_TEXTENCODING_ISO_8859_3", RTL_TEXTENCODING_ISO_8859_3 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_4()
            {
                check( "RTL_TEXTENCODING_ISO_8859_4", RTL_TEXTENCODING_ISO_8859_4 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_5()
            {
                check( "RTL_TEXTENCODING_ISO_8859_5", RTL_TEXTENCODING_ISO_8859_5 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_6()
            {
                check( "RTL_TEXTENCODING_ISO_8859_6", RTL_TEXTENCODING_ISO_8859_6 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_7()
            {
                check( "RTL_TEXTENCODING_ISO_8859_7", RTL_TEXTENCODING_ISO_8859_7 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_8()
            {
                check( "RTL_TEXTENCODING_ISO_8859_8", RTL_TEXTENCODING_ISO_8859_8 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_9()
            {
                check( "RTL_TEXTENCODING_ISO_8859_9", RTL_TEXTENCODING_ISO_8859_9 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_14()
            {
                check( "RTL_TEXTENCODING_ISO_8859_14", RTL_TEXTENCODING_ISO_8859_14 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_15()
            {
                check( "RTL_TEXTENCODING_ISO_8859_15", RTL_TEXTENCODING_ISO_8859_15 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_737()
            {
                check( "RTL_TEXTENCODING_IBM_737", RTL_TEXTENCODING_IBM_737 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_775()
            {
                check( "RTL_TEXTENCODING_IBM_775", RTL_TEXTENCODING_IBM_775 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_852()
            {
                check( "RTL_TEXTENCODING_IBM_852", RTL_TEXTENCODING_IBM_852 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_855()
            {
                check( "RTL_TEXTENCODING_IBM_855", RTL_TEXTENCODING_IBM_855 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_857()
            {
                check( "RTL_TEXTENCODING_IBM_857", RTL_TEXTENCODING_IBM_857 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_862()
            {
                check( "RTL_TEXTENCODING_IBM_862", RTL_TEXTENCODING_IBM_862 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_864()
            {
                check( "RTL_TEXTENCODING_IBM_864", RTL_TEXTENCODING_IBM_864 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_866()
            {
                check( "RTL_TEXTENCODING_IBM_866", RTL_TEXTENCODING_IBM_866 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_IBM_869()
            {
                check( "RTL_TEXTENCODING_IBM_869", RTL_TEXTENCODING_IBM_869 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_874()
            {
                check( "RTL_TEXTENCODING_MS_874", RTL_TEXTENCODING_MS_874 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_1250()
            {
                check( "RTL_TEXTENCODING_MS_1250", RTL_TEXTENCODING_MS_1250 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_1251()
            {
                check( "RTL_TEXTENCODING_MS_1251", RTL_TEXTENCODING_MS_1251 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_1253()
            {
                check( "RTL_TEXTENCODING_MS_1253", RTL_TEXTENCODING_MS_1253 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_1254()
            {
                check( "RTL_TEXTENCODING_MS_1254", RTL_TEXTENCODING_MS_1254 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_1255()
            {
                check( "RTL_TEXTENCODING_MS_1255", RTL_TEXTENCODING_MS_1255 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_1256()
            {
                check( "RTL_TEXTENCODING_MS_1256", RTL_TEXTENCODING_MS_1256 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_1257()
            {
                check( "RTL_TEXTENCODING_MS_1257", RTL_TEXTENCODING_MS_1257 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_1258()
            {
                check( "RTL_TEXTENCODING_MS_1258", RTL_TEXTENCODING_MS_1258 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_ARABIC()
            {
                check( "RTL_TEXTENCODING_APPLE_ARABIC", RTL_TEXTENCODING_APPLE_ARABIC );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_CENTEURO()
            {
                check( "RTL_TEXTENCODING_APPLE_CENTEURO", RTL_TEXTENCODING_APPLE_CENTEURO );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_CROATIAN()
            {
                check( "RTL_TEXTENCODING_APPLE_CROATIAN", RTL_TEXTENCODING_APPLE_CROATIAN );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_CYRILLIC()
            {
                check( "RTL_TEXTENCODING_APPLE_CYRILLIC", RTL_TEXTENCODING_APPLE_CYRILLIC );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_DEVANAGARI()
            {
                check( "RTL_TEXTENCODING_APPLE_DEVANAGARI", RTL_TEXTENCODING_APPLE_DEVANAGARI );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_FARSI()
            {
                check( "RTL_TEXTENCODING_APPLE_FARSI", RTL_TEXTENCODING_APPLE_FARSI );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_GREEK()
            {
                check( "RTL_TEXTENCODING_APPLE_GREEK", RTL_TEXTENCODING_APPLE_GREEK );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_GUJARATI()
            {
                check( "RTL_TEXTENCODING_APPLE_GUJARATI", RTL_TEXTENCODING_APPLE_GUJARATI );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_GURMUKHI()
            {
                check( "RTL_TEXTENCODING_APPLE_GURMUKHI", RTL_TEXTENCODING_APPLE_GURMUKHI );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_HEBREW()
            {
                check( "RTL_TEXTENCODING_APPLE_HEBREW", RTL_TEXTENCODING_APPLE_HEBREW );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_ICELAND()
            {
                check( "RTL_TEXTENCODING_APPLE_ICELAND", RTL_TEXTENCODING_APPLE_ICELAND );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_ROMANIAN()
            {
                check( "RTL_TEXTENCODING_APPLE_ROMANIAN", RTL_TEXTENCODING_APPLE_ROMANIAN );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_THAI()
            {
                check( "RTL_TEXTENCODING_APPLE_THAI", RTL_TEXTENCODING_APPLE_THAI );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_TURKISH()
            {
                check( "RTL_TEXTENCODING_APPLE_TURKISH", RTL_TEXTENCODING_APPLE_TURKISH );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_UKRAINIAN()
            {
                check( "RTL_TEXTENCODING_APPLE_UKRAINIAN", RTL_TEXTENCODING_APPLE_UKRAINIAN );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_CHINSIMP()
            {
                check( "RTL_TEXTENCODING_APPLE_CHINSIMP", RTL_TEXTENCODING_APPLE_CHINSIMP );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_CHINTRAD()
            {
                check( "RTL_TEXTENCODING_APPLE_CHINTRAD", RTL_TEXTENCODING_APPLE_CHINTRAD );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_JAPANESE()
            {
                check( "RTL_TEXTENCODING_APPLE_JAPANESE", RTL_TEXTENCODING_APPLE_JAPANESE );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_APPLE_KOREAN()
            {
                check( "RTL_TEXTENCODING_APPLE_KOREAN", RTL_TEXTENCODING_APPLE_KOREAN );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_932()
            {
                check( "RTL_TEXTENCODING_MS_932", RTL_TEXTENCODING_MS_932 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_936()
            {
                check( "RTL_TEXTENCODING_MS_936", RTL_TEXTENCODING_MS_936 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_949()
            {
                check( "RTL_TEXTENCODING_MS_949", RTL_TEXTENCODING_MS_949 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_950()
            {
                check( "RTL_TEXTENCODING_MS_950", RTL_TEXTENCODING_MS_950 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_SHIFT_JIS()
            {
                check( "RTL_TEXTENCODING_SHIFT_JIS", RTL_TEXTENCODING_SHIFT_JIS );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_GB_2312()
            {
                check( "RTL_TEXTENCODING_GB_2312", RTL_TEXTENCODING_GB_2312 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_GBT_12345()
            {
                check( "RTL_TEXTENCODING_GBT_12345", RTL_TEXTENCODING_GBT_12345 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_GBK()
            {
                check( "RTL_TEXTENCODING_GBK", RTL_TEXTENCODING_GBK );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_BIG5()
            {
                check( "RTL_TEXTENCODING_BIG5", RTL_TEXTENCODING_BIG5 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_EUC_JP()
            {
                check( "RTL_TEXTENCODING_EUC_JP", RTL_TEXTENCODING_EUC_JP );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_EUC_CN()
            {
                check( "RTL_TEXTENCODING_EUC_CN", RTL_TEXTENCODING_EUC_CN );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_EUC_TW()
            {
                check( "RTL_TEXTENCODING_EUC_TW", RTL_TEXTENCODING_EUC_TW );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_2022_JP()
            {
                check( "RTL_TEXTENCODING_ISO_2022_JP", RTL_TEXTENCODING_ISO_2022_JP );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_2022_CN()
            {
                check( "RTL_TEXTENCODING_ISO_2022_CN", RTL_TEXTENCODING_ISO_2022_CN );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_KOI8_R()
            {
                check( "RTL_TEXTENCODING_KOI8_R", RTL_TEXTENCODING_KOI8_R );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_UTF7()
            {
                check( "RTL_TEXTENCODING_UTF7", RTL_TEXTENCODING_UTF7 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_UTF8()
            {
                check( "RTL_TEXTENCODING_UTF8", RTL_TEXTENCODING_UTF8 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_10()
            {
                check( "RTL_TEXTENCODING_ISO_8859_10", RTL_TEXTENCODING_ISO_8859_10 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_8859_13()
            {
                check( "RTL_TEXTENCODING_ISO_8859_13", RTL_TEXTENCODING_ISO_8859_13 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_EUC_KR()
            {
                check( "RTL_TEXTENCODING_EUC_KR", RTL_TEXTENCODING_EUC_KR );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISO_2022_KR()
            {
                check( "RTL_TEXTENCODING_ISO_2022_KR", RTL_TEXTENCODING_ISO_2022_KR );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_JIS_X_0201()
            {
                check( "RTL_TEXTENCODING_JIS_X_0201", RTL_TEXTENCODING_JIS_X_0201 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_JIS_X_0208()
            {
                check( "RTL_TEXTENCODING_JIS_X_0208", RTL_TEXTENCODING_JIS_X_0208 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_JIS_X_0212()
            {
                check( "RTL_TEXTENCODING_JIS_X_0212", RTL_TEXTENCODING_JIS_X_0212 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_MS_1361()
            {
                check( "RTL_TEXTENCODING_MS_1361", RTL_TEXTENCODING_MS_1361 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_GB_18030()
            {
                check( "RTL_TEXTENCODING_GB_18030", RTL_TEXTENCODING_GB_18030 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_BIG5_HKSCS()
            {
                check( "RTL_TEXTENCODING_BIG5_HKSCS", RTL_TEXTENCODING_BIG5_HKSCS );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_TIS_620()
            {
                check( "RTL_TEXTENCODING_TIS_620", RTL_TEXTENCODING_TIS_620 );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_KOI8_U()
            {
                check( "RTL_TEXTENCODING_KOI8_U", RTL_TEXTENCODING_KOI8_U );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_ISCII_DEVANAGARI()
            {
                check( "RTL_TEXTENCODING_ISCII_DEVANAGARI", RTL_TEXTENCODING_ISCII_DEVANAGARI );
            }
        // ----------------------------------------
        void UnixCharsetFromTextEncoding_JAVA_UTF8()
            {
                check( "RTL_TEXTENCODING_JAVA_UTF8", RTL_TEXTENCODING_JAVA_UTF8 );
            }
        // ----------------------------------------

        CPPUNIT_TEST_SUITE( getBestUnix );
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
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_ARABIC );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_CENTEURO );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_CROATIAN );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_CYRILLIC );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_DEVANAGARI );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_FARSI );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_GREEK );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_GUJARATI );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_GURMUKHI );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_HEBREW );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_ICELAND );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_ROMANIAN );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_THAI );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_TURKISH );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_UKRAINIAN );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_CHINSIMP );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_CHINTRAD );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_JAPANESE );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_APPLE_KOREAN );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_932 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_936 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_949 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_950 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_SHIFT_JIS );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_GB_2312 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_GBT_12345 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_GBK );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_BIG5 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_EUC_JP );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_EUC_CN );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_EUC_TW );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_2022_JP );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_2022_CN );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_KOI8_R );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_UTF7 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_UTF8 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_10 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_8859_13 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_EUC_KR );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISO_2022_KR );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_JIS_X_0201 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_JIS_X_0208 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_JIS_X_0212 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_MS_1361 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_GB_18030 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_BIG5_HKSCS );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_TIS_620 );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_KOI8_U );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_ISCII_DEVANAGARI );
        CPPUNIT_TEST( UnixCharsetFromTextEncoding_JAVA_UTF8 );

        CPPUNIT_TEST_SUITE_END( );
    };

 // -----------------------------------------------------------------------------


    class getBestWindows : public CppUnit::TestFixture
    {
    public:
        void setUp()
            {
            }

        void check( const sal_Char* _pRTL_TEXTENCODING, rtl_TextEncoding _aCurrentEncode )
            {
                const sal_uInt8 nCharSet = rtl_getBestWindowsCharsetFromTextEncoding( _aCurrentEncode );
                if (nCharSet == 1)
                {
                    t_print("rtl_getBestWindowsCharsetFromTextEncoding(%s) (%d) doesn't seem to exist.\n\n", _pRTL_TEXTENCODING, _aCurrentEncode);
                }
                else
                {
                    t_print(T_VERBOSE, "'%s' is charset: '%d'\n", _pRTL_TEXTENCODING, nCharSet);

                    rtl_TextEncoding eTextEnc = rtl_getTextEncodingFromWindowsCharset( nCharSet );
                    if (_aCurrentEncode != eTextEnc &&
                        eTextEnc != RTL_TEXTENCODING_DONTKNOW)
                    {
                        t_print("rtl_getBestUnixCharsetFromTextEncoding(%s) is charset: %d\n", _pRTL_TEXTENCODING, nCharSet);
                        t_print("rtl_getTextEncodingFromWindowsCharset() differ: %s %d -> %d\n\n", _pRTL_TEXTENCODING, _aCurrentEncode, eTextEnc );
                    }
                    // rtl::OString sError = "getTextEncodingFromWindowsCharset(";
                    // sError += rtl::OString::valueOf(nCharSet);
                    // sError += ") returns nul.";

                    // CPPUNIT_ASSERT_MESSAGE(sError.getStr(), eTextEnc != RTL_TEXTENCODING_DONTKNOW);
                    // CPPUNIT_ASSERT_MESSAGE("Does not realize itself", _aCurrentEncode == eTextEnc );
                }
            }

        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_1252()
            {
                check( "RTL_TEXTENCODING_MS_1252", RTL_TEXTENCODING_MS_1252 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_ROMAN()
            {
                check( "RTL_TEXTENCODING_APPLE_ROMAN", RTL_TEXTENCODING_APPLE_ROMAN );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_437()
            {
                check( "RTL_TEXTENCODING_IBM_437", RTL_TEXTENCODING_IBM_437 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_850()
            {
                check( "RTL_TEXTENCODING_IBM_850", RTL_TEXTENCODING_IBM_850 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_860()
            {
                check( "RTL_TEXTENCODING_IBM_860", RTL_TEXTENCODING_IBM_860 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_861()
            {
                check( "RTL_TEXTENCODING_IBM_861", RTL_TEXTENCODING_IBM_861 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_863()
            {
                check( "RTL_TEXTENCODING_IBM_863", RTL_TEXTENCODING_IBM_863 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_865()
            {
                check( "RTL_TEXTENCODING_IBM_865", RTL_TEXTENCODING_IBM_865 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_SYMBOL()
            {
                check( "RTL_TEXTENCODING_SYMBOL", RTL_TEXTENCODING_SYMBOL );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ASCII_US()
            {
                check( "RTL_TEXTENCODING_ASCII_US", RTL_TEXTENCODING_ASCII_US );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_1()
            {
                check( "RTL_TEXTENCODING_ISO_8859_1", RTL_TEXTENCODING_ISO_8859_1 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_2()
            {
                check( "RTL_TEXTENCODING_ISO_8859_2", RTL_TEXTENCODING_ISO_8859_2 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_3()
            {
                check( "RTL_TEXTENCODING_ISO_8859_3", RTL_TEXTENCODING_ISO_8859_3 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_4()
            {
                check( "RTL_TEXTENCODING_ISO_8859_4", RTL_TEXTENCODING_ISO_8859_4 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_5()
            {
                check( "RTL_TEXTENCODING_ISO_8859_5", RTL_TEXTENCODING_ISO_8859_5 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_6()
            {
                check( "RTL_TEXTENCODING_ISO_8859_6", RTL_TEXTENCODING_ISO_8859_6 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_7()
            {
                check( "RTL_TEXTENCODING_ISO_8859_7", RTL_TEXTENCODING_ISO_8859_7 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_8()
            {
                check( "RTL_TEXTENCODING_ISO_8859_8", RTL_TEXTENCODING_ISO_8859_8 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_9()
            {
                check( "RTL_TEXTENCODING_ISO_8859_9", RTL_TEXTENCODING_ISO_8859_9 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_14()
            {
                check( "RTL_TEXTENCODING_ISO_8859_14", RTL_TEXTENCODING_ISO_8859_14 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_15()
            {
                check( "RTL_TEXTENCODING_ISO_8859_15", RTL_TEXTENCODING_ISO_8859_15 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_737()
            {
                check( "RTL_TEXTENCODING_IBM_737", RTL_TEXTENCODING_IBM_737 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_775()
            {
                check( "RTL_TEXTENCODING_IBM_775", RTL_TEXTENCODING_IBM_775 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_852()
            {
                check( "RTL_TEXTENCODING_IBM_852", RTL_TEXTENCODING_IBM_852 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_855()
            {
                check( "RTL_TEXTENCODING_IBM_855", RTL_TEXTENCODING_IBM_855 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_857()
            {
                check( "RTL_TEXTENCODING_IBM_857", RTL_TEXTENCODING_IBM_857 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_862()
            {
                check( "RTL_TEXTENCODING_IBM_862", RTL_TEXTENCODING_IBM_862 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_864()
            {
                check( "RTL_TEXTENCODING_IBM_864", RTL_TEXTENCODING_IBM_864 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_866()
            {
                check( "RTL_TEXTENCODING_IBM_866", RTL_TEXTENCODING_IBM_866 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_IBM_869()
            {
                check( "RTL_TEXTENCODING_IBM_869", RTL_TEXTENCODING_IBM_869 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_874()
            {
                check( "RTL_TEXTENCODING_MS_874", RTL_TEXTENCODING_MS_874 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_1250()
            {
                check( "RTL_TEXTENCODING_MS_1250", RTL_TEXTENCODING_MS_1250 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_1251()
            {
                check( "RTL_TEXTENCODING_MS_1251", RTL_TEXTENCODING_MS_1251 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_1253()
            {
                check( "RTL_TEXTENCODING_MS_1253", RTL_TEXTENCODING_MS_1253 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_1254()
            {
                check( "RTL_TEXTENCODING_MS_1254", RTL_TEXTENCODING_MS_1254 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_1255()
            {
                check( "RTL_TEXTENCODING_MS_1255", RTL_TEXTENCODING_MS_1255 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_1256()
            {
                check( "RTL_TEXTENCODING_MS_1256", RTL_TEXTENCODING_MS_1256 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_1257()
            {
                check( "RTL_TEXTENCODING_MS_1257", RTL_TEXTENCODING_MS_1257 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_1258()
            {
                check( "RTL_TEXTENCODING_MS_1258", RTL_TEXTENCODING_MS_1258 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_ARABIC()
            {
                check( "RTL_TEXTENCODING_APPLE_ARABIC", RTL_TEXTENCODING_APPLE_ARABIC );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_CENTEURO()
            {
                check( "RTL_TEXTENCODING_APPLE_CENTEURO", RTL_TEXTENCODING_APPLE_CENTEURO );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_CROATIAN()
            {
                check( "RTL_TEXTENCODING_APPLE_CROATIAN", RTL_TEXTENCODING_APPLE_CROATIAN );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_CYRILLIC()
            {
                check( "RTL_TEXTENCODING_APPLE_CYRILLIC", RTL_TEXTENCODING_APPLE_CYRILLIC );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_DEVANAGARI()
            {
                check( "RTL_TEXTENCODING_APPLE_DEVANAGARI", RTL_TEXTENCODING_APPLE_DEVANAGARI );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_FARSI()
            {
                check( "RTL_TEXTENCODING_APPLE_FARSI", RTL_TEXTENCODING_APPLE_FARSI );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_GREEK()
            {
                check( "RTL_TEXTENCODING_APPLE_GREEK", RTL_TEXTENCODING_APPLE_GREEK );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_GUJARATI()
            {
                check( "RTL_TEXTENCODING_APPLE_GUJARATI", RTL_TEXTENCODING_APPLE_GUJARATI );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_GURMUKHI()
            {
                check( "RTL_TEXTENCODING_APPLE_GURMUKHI", RTL_TEXTENCODING_APPLE_GURMUKHI );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_HEBREW()
            {
                check( "RTL_TEXTENCODING_APPLE_HEBREW", RTL_TEXTENCODING_APPLE_HEBREW );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_ICELAND()
            {
                check( "RTL_TEXTENCODING_APPLE_ICELAND", RTL_TEXTENCODING_APPLE_ICELAND );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_ROMANIAN()
            {
                check( "RTL_TEXTENCODING_APPLE_ROMANIAN", RTL_TEXTENCODING_APPLE_ROMANIAN );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_THAI()
            {
                check( "RTL_TEXTENCODING_APPLE_THAI", RTL_TEXTENCODING_APPLE_THAI );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_TURKISH()
            {
                check( "RTL_TEXTENCODING_APPLE_TURKISH", RTL_TEXTENCODING_APPLE_TURKISH );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_UKRAINIAN()
            {
                check( "RTL_TEXTENCODING_APPLE_UKRAINIAN", RTL_TEXTENCODING_APPLE_UKRAINIAN );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_CHINSIMP()
            {
                check( "RTL_TEXTENCODING_APPLE_CHINSIMP", RTL_TEXTENCODING_APPLE_CHINSIMP );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_CHINTRAD()
            {
                check( "RTL_TEXTENCODING_APPLE_CHINTRAD", RTL_TEXTENCODING_APPLE_CHINTRAD );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_JAPANESE()
            {
                check( "RTL_TEXTENCODING_APPLE_JAPANESE", RTL_TEXTENCODING_APPLE_JAPANESE );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_APPLE_KOREAN()
            {
                check( "RTL_TEXTENCODING_APPLE_KOREAN", RTL_TEXTENCODING_APPLE_KOREAN );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_932()
            {
                check( "RTL_TEXTENCODING_MS_932", RTL_TEXTENCODING_MS_932 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_936()
            {
                check( "RTL_TEXTENCODING_MS_936", RTL_TEXTENCODING_MS_936 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_949()
            {
                check( "RTL_TEXTENCODING_MS_949", RTL_TEXTENCODING_MS_949 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_950()
            {
                check( "RTL_TEXTENCODING_MS_950", RTL_TEXTENCODING_MS_950 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_SHIFT_JIS()
            {
                check( "RTL_TEXTENCODING_SHIFT_JIS", RTL_TEXTENCODING_SHIFT_JIS );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_GB_2312()
            {
                check( "RTL_TEXTENCODING_GB_2312", RTL_TEXTENCODING_GB_2312 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_GBT_12345()
            {
                check( "RTL_TEXTENCODING_GBT_12345", RTL_TEXTENCODING_GBT_12345 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_GBK()
            {
                check( "RTL_TEXTENCODING_GBK", RTL_TEXTENCODING_GBK );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_BIG5()
            {
                check( "RTL_TEXTENCODING_BIG5", RTL_TEXTENCODING_BIG5 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_EUC_JP()
            {
                check( "RTL_TEXTENCODING_EUC_JP", RTL_TEXTENCODING_EUC_JP );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_EUC_CN()
            {
                check( "RTL_TEXTENCODING_EUC_CN", RTL_TEXTENCODING_EUC_CN );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_EUC_TW()
            {
                check( "RTL_TEXTENCODING_EUC_TW", RTL_TEXTENCODING_EUC_TW );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_2022_JP()
            {
                check( "RTL_TEXTENCODING_ISO_2022_JP", RTL_TEXTENCODING_ISO_2022_JP );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_2022_CN()
            {
                check( "RTL_TEXTENCODING_ISO_2022_CN", RTL_TEXTENCODING_ISO_2022_CN );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_KOI8_R()
            {
                check( "RTL_TEXTENCODING_KOI8_R", RTL_TEXTENCODING_KOI8_R );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_UTF7()
            {
                check( "RTL_TEXTENCODING_UTF7", RTL_TEXTENCODING_UTF7 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_UTF8()
            {
                check( "RTL_TEXTENCODING_UTF8", RTL_TEXTENCODING_UTF8 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_10()
            {
                check( "RTL_TEXTENCODING_ISO_8859_10", RTL_TEXTENCODING_ISO_8859_10 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_8859_13()
            {
                check( "RTL_TEXTENCODING_ISO_8859_13", RTL_TEXTENCODING_ISO_8859_13 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_EUC_KR()
            {
                check( "RTL_TEXTENCODING_EUC_KR", RTL_TEXTENCODING_EUC_KR );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISO_2022_KR()
            {
                check( "RTL_TEXTENCODING_ISO_2022_KR", RTL_TEXTENCODING_ISO_2022_KR );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_JIS_X_0201()
            {
                check( "RTL_TEXTENCODING_JIS_X_0201", RTL_TEXTENCODING_JIS_X_0201 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_JIS_X_0208()
            {
                check( "RTL_TEXTENCODING_JIS_X_0208", RTL_TEXTENCODING_JIS_X_0208 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_JIS_X_0212()
            {
                check( "RTL_TEXTENCODING_JIS_X_0212", RTL_TEXTENCODING_JIS_X_0212 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_MS_1361()
            {
                check( "RTL_TEXTENCODING_MS_1361", RTL_TEXTENCODING_MS_1361 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_GB_18030()
            {
                check( "RTL_TEXTENCODING_GB_18030", RTL_TEXTENCODING_GB_18030 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_BIG5_HKSCS()
            {
                check( "RTL_TEXTENCODING_BIG5_HKSCS", RTL_TEXTENCODING_BIG5_HKSCS );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_TIS_620()
            {
                check( "RTL_TEXTENCODING_TIS_620", RTL_TEXTENCODING_TIS_620 );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_KOI8_U()
            {
                check( "RTL_TEXTENCODING_KOI8_U", RTL_TEXTENCODING_KOI8_U );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_ISCII_DEVANAGARI()
            {
                check( "RTL_TEXTENCODING_ISCII_DEVANAGARI", RTL_TEXTENCODING_ISCII_DEVANAGARI );
            }
        // ----------------------------------------
        void WindowsCharsetFromTextEncoding_JAVA_UTF8()
            {
                check( "RTL_TEXTENCODING_JAVA_UTF8", RTL_TEXTENCODING_JAVA_UTF8 );
            }
        // ----------------------------------------

        CPPUNIT_TEST_SUITE( getBestWindows );
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
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_2 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_3 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_4 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_5 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_6 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_7 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_8 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_9 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_14 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_ISO_8859_15 );
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
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_APPLE_ICELAND );
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
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_UTF7 );
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_UTF8 );
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
        CPPUNIT_TEST( WindowsCharsetFromTextEncoding_JAVA_UTF8 );

        CPPUNIT_TEST_SUITE_END( );
    };
    class getTextEncodingInfo: public CppUnit::TestFixture
    {
         public:
        // initialise your test code values here.
        void setUp()
            {
            }

        void tearDown()
            {
            }

        // not implemented encoding test
        void getTextEncodingInfo_001()
            {
                rtl_TextEncodingInfo aInfo1, aInfo2, aInfo3, aInfo4, aInfo5;
                aInfo1.StructSize = 4;
                // not implemented
                sal_Bool bRes1 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo1 );
                // implemented
                sal_Bool bRes11 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo1 );
                CPPUNIT_ASSERT_MESSAGE("should return sal_False.", bRes1 == sal_False && bRes11 == sal_False );

                aInfo2.StructSize = 5;
                sal_Bool bRes2 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo2 );
                sal_Bool bRes21 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo2 );
                CPPUNIT_ASSERT_MESSAGE("StructSize<6 should return sal_True", bRes2 == sal_True && bRes21 == sal_True && aInfo2.MinimumCharSize >=1 );

                aInfo3.StructSize = 6;
                sal_Bool bRes3 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo3 );
                sal_Bool bRes31 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo3 );
                CPPUNIT_ASSERT_MESSAGE("StructSize<6 should return sal_True", bRes3 == sal_True && bRes31 == sal_True );
//&& aInfo2.MinimumCharSize >=1 );

                aInfo4.StructSize = 8;
                sal_Bool bRes4 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo4 );
                sal_Bool bRes41 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo4);
               CPPUNIT_ASSERT_MESSAGE("StructSize<6 should return sal_True", bRes4 == sal_True && bRes41 == sal_True);
// && aInfo2.MinimumCharSize >=1 );

        aInfo5.StructSize = sizeof aInfo5;
                sal_Bool bRes5 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo5 );
                CPPUNIT_ASSERT_MESSAGE("StructSize<6 should return sal_True", bRes5 == sal_False && aInfo5.Flags == 0);

            }
        CPPUNIT_TEST_SUITE(getTextEncodingInfo);
        CPPUNIT_TEST(getTextEncodingInfo_001);
        //CPPUNIT_TEST(getTextEncodingInfo_002);
        CPPUNIT_TEST_SUITE_END();
    };
}

namespace {

class TestEncodingFromUnix: public CppUnit::TestFixture {
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

    CPPUNIT_TEST_SUITE(TestEncodingFromUnix);
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

// -----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_tencinfo::getBestMime,    "rtl_tencinfo" );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_tencinfo::getBestUnix,    "rtl_tencinfo" );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_tencinfo::getBestWindows, "rtl_tencinfo" );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_tencinfo::getTextEncodingInfo, "rtl_tencinfo" );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(
    TestEncodingFromUnix, "TestEncodingFromUnix");

// -----------------------------------------------------------------------------

NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
