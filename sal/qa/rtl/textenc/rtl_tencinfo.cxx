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
#include <string.h>

#include <osl/thread.h>
#include <rtl/tencinfo.h>

#include "gtest/gtest.h"

// -----------------------------------------------------------------------------

namespace rtl_tencinfo
{
    class getBestMime : public ::testing::Test
    {
    public:
        void SetUp()
            {
            }

        void check( const sal_Char* _pRTL_TEXTENCODING, rtl_TextEncoding _aCurrentEncode )
            {
                const sal_Char *pCharSet = rtl_getBestMimeCharsetFromTextEncoding( _aCurrentEncode );
                if (pCharSet == 0)
                {
                    printf("rtl_getBestMimeCharsetFromTextEncoding(%s) (%d) doesn't seem to exist.\n\n", _pRTL_TEXTENCODING, _aCurrentEncode);
                }
                else
                {
                    printf("'%s' is charset: '%s'\n", _pRTL_TEXTENCODING, pCharSet);

                    rtl_TextEncoding eTextEnc = rtl_getTextEncodingFromMimeCharset( pCharSet );
                    if (_aCurrentEncode != eTextEnc &&
                        eTextEnc != RTL_TEXTENCODING_DONTKNOW)
                    {
                        printf("rtl_getBestMimeCharsetFromTextEncoding(%s) is charset: %s\n", _pRTL_TEXTENCODING, pCharSet);
                        printf("rtl_getTextEncodingFromMimeCharset() differ: %s %d -> %d\n\n", _pRTL_TEXTENCODING, _aCurrentEncode, eTextEnc );
                    }
                    // rtl::OString sError = "getTextEncodingFromMimeCharset(";
                    // sError += pCharSet;
                    // sError += ") returns null";
                    // ASSERT_TRUE(eTextEnc != RTL_TEXTENCODING_DONTKNOW) << sError.getStr();
                    // ASSERT_TRUE(_aCurrentEncode == eTextEnc) << "Does not realize itself";
                }
            }
    };

/* ATTENTION!  Whenever some encoding is added here, make sure to update
 * rtl_isOctetEncoding in tencinfo.c.
 */

// RTL_TEXTENCODING_USER_START
// RTL_TEXTENCODING_USER_END

//                check( "RTL_TEXTENCODING_UCS4", RTL_TEXTENCODING_UCS4 );
//                check( "RTL_TEXTENCODING_UCS2", RTL_TEXTENCODING_UCS2 );
//                check( "RTL_TEXTENCODING_UNICODE", RTL_TEXTENCODING_UNICODE /* RTL_TEXTENCODING_UCS2 */ );


// the defines for the follows test could be found in file inc/rtl/textenc.h

    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_1252)
    {
        check( "RTL_TEXTENCODING_MS_1252", RTL_TEXTENCODING_MS_1252 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_ROMAN)
    {
        check( "RTL_TEXTENCODING_APPLE_ROMAN", RTL_TEXTENCODING_APPLE_ROMAN );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_437)
    {
        check( "RTL_TEXTENCODING_IBM_437", RTL_TEXTENCODING_IBM_437 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_850)
    {
        check( "RTL_TEXTENCODING_IBM_850", RTL_TEXTENCODING_IBM_850 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_860)
    {
        check( "RTL_TEXTENCODING_IBM_860", RTL_TEXTENCODING_IBM_860 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_861)
    {
        check( "RTL_TEXTENCODING_IBM_861", RTL_TEXTENCODING_IBM_861 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_863)
    {
        check( "RTL_TEXTENCODING_IBM_863", RTL_TEXTENCODING_IBM_863 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_865)
    {
        check( "RTL_TEXTENCODING_IBM_865", RTL_TEXTENCODING_IBM_865 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_SYMBOL)
    {
        check( "RTL_TEXTENCODING_SYMBOL", RTL_TEXTENCODING_SYMBOL );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ASCII_US)
    {
        check( "RTL_TEXTENCODING_ASCII_US", RTL_TEXTENCODING_ASCII_US );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_1)
    {
        check( "RTL_TEXTENCODING_ISO_8859_1", RTL_TEXTENCODING_ISO_8859_1 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_2)
    {
        check( "RTL_TEXTENCODING_ISO_8859_2", RTL_TEXTENCODING_ISO_8859_2 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_3)
    {
        check( "RTL_TEXTENCODING_ISO_8859_3", RTL_TEXTENCODING_ISO_8859_3 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_4)
    {
        check( "RTL_TEXTENCODING_ISO_8859_4", RTL_TEXTENCODING_ISO_8859_4 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_5)
    {
        check( "RTL_TEXTENCODING_ISO_8859_5", RTL_TEXTENCODING_ISO_8859_5 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_6)
    {
        check( "RTL_TEXTENCODING_ISO_8859_6", RTL_TEXTENCODING_ISO_8859_6 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_7)
    {
        check( "RTL_TEXTENCODING_ISO_8859_7", RTL_TEXTENCODING_ISO_8859_7 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_8)
    {
        check( "RTL_TEXTENCODING_ISO_8859_8", RTL_TEXTENCODING_ISO_8859_8 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_9)
    {
        check( "RTL_TEXTENCODING_ISO_8859_9", RTL_TEXTENCODING_ISO_8859_9 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_14)
    {
        check( "RTL_TEXTENCODING_ISO_8859_14", RTL_TEXTENCODING_ISO_8859_14 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_15)
    {
        check( "RTL_TEXTENCODING_ISO_8859_15", RTL_TEXTENCODING_ISO_8859_15 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_737)
    {
        check( "RTL_TEXTENCODING_IBM_737", RTL_TEXTENCODING_IBM_737 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_775)
    {
        check( "RTL_TEXTENCODING_IBM_775", RTL_TEXTENCODING_IBM_775 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_852)
    {
        check( "RTL_TEXTENCODING_IBM_852", RTL_TEXTENCODING_IBM_852 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_855)
    {
        check( "RTL_TEXTENCODING_IBM_855", RTL_TEXTENCODING_IBM_855 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_857)
    {
        check( "RTL_TEXTENCODING_IBM_857", RTL_TEXTENCODING_IBM_857 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_862)
    {
        check( "RTL_TEXTENCODING_IBM_862", RTL_TEXTENCODING_IBM_862 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_864)
    {
        check( "RTL_TEXTENCODING_IBM_864", RTL_TEXTENCODING_IBM_864 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_866)
    {
        check( "RTL_TEXTENCODING_IBM_866", RTL_TEXTENCODING_IBM_866 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_IBM_869)
    {
        check( "RTL_TEXTENCODING_IBM_869", RTL_TEXTENCODING_IBM_869 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_874)
    {
        check( "RTL_TEXTENCODING_MS_874", RTL_TEXTENCODING_MS_874 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_1250)
    {
        check( "RTL_TEXTENCODING_MS_1250", RTL_TEXTENCODING_MS_1250 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_1251)
    {
        check( "RTL_TEXTENCODING_MS_1251", RTL_TEXTENCODING_MS_1251 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_1253)
    {
        check( "RTL_TEXTENCODING_MS_1253", RTL_TEXTENCODING_MS_1253 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_1254)
    {
        check( "RTL_TEXTENCODING_MS_1254", RTL_TEXTENCODING_MS_1254 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_1255)
    {
        check( "RTL_TEXTENCODING_MS_1255", RTL_TEXTENCODING_MS_1255 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_1256)
    {
        check( "RTL_TEXTENCODING_MS_1256", RTL_TEXTENCODING_MS_1256 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_1257)
    {
        check( "RTL_TEXTENCODING_MS_1257", RTL_TEXTENCODING_MS_1257 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_1258)
    {
        check( "RTL_TEXTENCODING_MS_1258", RTL_TEXTENCODING_MS_1258 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_ARABIC)
    {
        check( "RTL_TEXTENCODING_APPLE_ARABIC", RTL_TEXTENCODING_APPLE_ARABIC );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_CENTEURO)
    {
        check( "RTL_TEXTENCODING_APPLE_CENTEURO", RTL_TEXTENCODING_APPLE_CENTEURO );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_CROATIAN)
    {
        check( "RTL_TEXTENCODING_APPLE_CROATIAN", RTL_TEXTENCODING_APPLE_CROATIAN );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_CYRILLIC)
    {
        check( "RTL_TEXTENCODING_APPLE_CYRILLIC", RTL_TEXTENCODING_APPLE_CYRILLIC );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_DEVANAGARI)
    {
        check( "RTL_TEXTENCODING_APPLE_DEVANAGARI", RTL_TEXTENCODING_APPLE_DEVANAGARI );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_FARSI)
    {
        check( "RTL_TEXTENCODING_APPLE_FARSI", RTL_TEXTENCODING_APPLE_FARSI );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_GREEK)
    {
        check( "RTL_TEXTENCODING_APPLE_GREEK", RTL_TEXTENCODING_APPLE_GREEK );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_GUJARATI)
    {
        check( "RTL_TEXTENCODING_APPLE_GUJARATI", RTL_TEXTENCODING_APPLE_GUJARATI );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_GURMUKHI)
    {
        check( "RTL_TEXTENCODING_APPLE_GURMUKHI", RTL_TEXTENCODING_APPLE_GURMUKHI );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_HEBREW)
    {
        check( "RTL_TEXTENCODING_APPLE_HEBREW", RTL_TEXTENCODING_APPLE_HEBREW );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_ICELAND)
    {
        check( "RTL_TEXTENCODING_APPLE_ICELAND", RTL_TEXTENCODING_APPLE_ICELAND );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_ROMANIAN)
    {
        check( "RTL_TEXTENCODING_APPLE_ROMANIAN", RTL_TEXTENCODING_APPLE_ROMANIAN );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_THAI)
    {
        check( "RTL_TEXTENCODING_APPLE_THAI", RTL_TEXTENCODING_APPLE_THAI );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_TURKISH)
    {
        check( "RTL_TEXTENCODING_APPLE_TURKISH", RTL_TEXTENCODING_APPLE_TURKISH );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_UKRAINIAN)
    {
        check( "RTL_TEXTENCODING_APPLE_UKRAINIAN", RTL_TEXTENCODING_APPLE_UKRAINIAN );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_CHINSIMP)
    {
        check( "RTL_TEXTENCODING_APPLE_CHINSIMP", RTL_TEXTENCODING_APPLE_CHINSIMP );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_CHINTRAD)
    {
        check( "RTL_TEXTENCODING_APPLE_CHINTRAD", RTL_TEXTENCODING_APPLE_CHINTRAD );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_JAPANESE)
    {
        check( "RTL_TEXTENCODING_APPLE_JAPANESE", RTL_TEXTENCODING_APPLE_JAPANESE );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_APPLE_KOREAN)
    {
        check( "RTL_TEXTENCODING_APPLE_KOREAN", RTL_TEXTENCODING_APPLE_KOREAN );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_932)
    {
        check( "RTL_TEXTENCODING_MS_932", RTL_TEXTENCODING_MS_932 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_936)
    {
        check( "RTL_TEXTENCODING_MS_936", RTL_TEXTENCODING_MS_936 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_949)
    {
        check( "RTL_TEXTENCODING_MS_949", RTL_TEXTENCODING_MS_949 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_950)
    {
        check( "RTL_TEXTENCODING_MS_950", RTL_TEXTENCODING_MS_950 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_SHIFT_JIS)
    {
        check( "RTL_TEXTENCODING_SHIFT_JIS", RTL_TEXTENCODING_SHIFT_JIS );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_GB_2312)
    {
        check( "RTL_TEXTENCODING_GB_2312", RTL_TEXTENCODING_GB_2312 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_GBT_12345)
    {
        check( "RTL_TEXTENCODING_GBT_12345", RTL_TEXTENCODING_GBT_12345 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_GBK)
    {
        check( "RTL_TEXTENCODING_GBK", RTL_TEXTENCODING_GBK );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_BIG5)
    {
        check( "RTL_TEXTENCODING_BIG5", RTL_TEXTENCODING_BIG5 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_EUC_JP)
    {
        check( "RTL_TEXTENCODING_EUC_JP", RTL_TEXTENCODING_EUC_JP );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_EUC_CN)
    {
        check( "RTL_TEXTENCODING_EUC_CN", RTL_TEXTENCODING_EUC_CN );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_EUC_TW)
    {
        check( "RTL_TEXTENCODING_EUC_TW", RTL_TEXTENCODING_EUC_TW );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_2022_JP)
    {
        check( "RTL_TEXTENCODING_ISO_2022_JP", RTL_TEXTENCODING_ISO_2022_JP );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_2022_CN)
    {
        check( "RTL_TEXTENCODING_ISO_2022_CN", RTL_TEXTENCODING_ISO_2022_CN );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_KOI8_R)
    {
        check( "RTL_TEXTENCODING_KOI8_R", RTL_TEXTENCODING_KOI8_R );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_UTF7)
    {
        check( "RTL_TEXTENCODING_UTF7", RTL_TEXTENCODING_UTF7 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_UTF8)
    {
        check( "RTL_TEXTENCODING_UTF8", RTL_TEXTENCODING_UTF8 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_10)
    {
        check( "RTL_TEXTENCODING_ISO_8859_10", RTL_TEXTENCODING_ISO_8859_10 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_8859_13)
    {
        check( "RTL_TEXTENCODING_ISO_8859_13", RTL_TEXTENCODING_ISO_8859_13 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_EUC_KR)
    {
        check( "RTL_TEXTENCODING_EUC_KR", RTL_TEXTENCODING_EUC_KR );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISO_2022_KR)
    {
        check( "RTL_TEXTENCODING_ISO_2022_KR", RTL_TEXTENCODING_ISO_2022_KR );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_JIS_X_0201)
    {
        check( "RTL_TEXTENCODING_JIS_X_0201", RTL_TEXTENCODING_JIS_X_0201 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_JIS_X_0208)
    {
        check( "RTL_TEXTENCODING_JIS_X_0208", RTL_TEXTENCODING_JIS_X_0208 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_JIS_X_0212)
    {
        check( "RTL_TEXTENCODING_JIS_X_0212", RTL_TEXTENCODING_JIS_X_0212 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_MS_1361)
    {
        check( "RTL_TEXTENCODING_MS_1361", RTL_TEXTENCODING_MS_1361 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_GB_18030)
    {
        check( "RTL_TEXTENCODING_GB_18030", RTL_TEXTENCODING_GB_18030 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_BIG5_HKSCS)
    {
        check( "RTL_TEXTENCODING_BIG5_HKSCS", RTL_TEXTENCODING_BIG5_HKSCS );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_TIS_620)
    {
        check( "RTL_TEXTENCODING_TIS_620", RTL_TEXTENCODING_TIS_620 );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_KOI8_U)
    {
        check( "RTL_TEXTENCODING_KOI8_U", RTL_TEXTENCODING_KOI8_U );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_ISCII_DEVANAGARI)
    {
        check( "RTL_TEXTENCODING_ISCII_DEVANAGARI", RTL_TEXTENCODING_ISCII_DEVANAGARI );
    }
    // ----------------------------------------
    TEST_F(getBestMime, MimeCharsetFromTextEncoding_JAVA_UTF8)
    {
        check( "RTL_TEXTENCODING_JAVA_UTF8", RTL_TEXTENCODING_JAVA_UTF8 );
    }


    // -----------------------------------------------------------------------------

    class getBestUnix : public ::testing::Test
    {
    public:
        void SetUp()
            {
            }

        void check( const sal_Char* _pRTL_TEXTENCODING, rtl_TextEncoding _aCurrentEncode )
            {
                const sal_Char *pCharSet = rtl_getBestUnixCharsetFromTextEncoding( _aCurrentEncode );
                if (pCharSet == 0)
                {
                    printf("rtl_getBestUnixCharsetFromTextEncoding(%s) (%d) doesn't seem to exist.\n\n", _pRTL_TEXTENCODING, _aCurrentEncode);
                }
                else
                {
                    printf("'%s' is charset: '%s'\n", _pRTL_TEXTENCODING, pCharSet);

                    rtl_TextEncoding eTextEnc = rtl_getTextEncodingFromUnixCharset( pCharSet );
                    if (_aCurrentEncode != eTextEnc &&
                        eTextEnc != RTL_TEXTENCODING_DONTKNOW)
                    {
                        printf("rtl_getBestUnixCharsetFromTextEncoding(%s) is charset: %s\n", _pRTL_TEXTENCODING, pCharSet);
                        printf("rtl_getTextEncodingFromUnixCharset() differ: %s %d -> %d\n\n", _pRTL_TEXTENCODING, _aCurrentEncode, eTextEnc );
                    }
                    // rtl::OString sError = "getTextEncodingFromUnixCharset(";
                    // sError += pCharSet;
                    // sError += ") returns null";
                    // ASSERT_TRUE(eTextEnc != RTL_TEXTENCODING_DONTKNOW) << sError.getStr();
                    // ASSERT_TRUE(_aCurrentEncode == eTextEnc) << "Does not realize itself";
                }
            }
    };

    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_1252)
    {
        check( "RTL_TEXTENCODING_MS_1252", RTL_TEXTENCODING_MS_1252 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_ROMAN)
    {
        check( "RTL_TEXTENCODING_APPLE_ROMAN", RTL_TEXTENCODING_APPLE_ROMAN );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_437)
    {
        check( "RTL_TEXTENCODING_IBM_437", RTL_TEXTENCODING_IBM_437 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_850)
    {
        check( "RTL_TEXTENCODING_IBM_850", RTL_TEXTENCODING_IBM_850 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_860)
    {
        check( "RTL_TEXTENCODING_IBM_860", RTL_TEXTENCODING_IBM_860 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_861)
    {
        check( "RTL_TEXTENCODING_IBM_861", RTL_TEXTENCODING_IBM_861 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_863)
    {
        check( "RTL_TEXTENCODING_IBM_863", RTL_TEXTENCODING_IBM_863 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_865)
    {
        check( "RTL_TEXTENCODING_IBM_865", RTL_TEXTENCODING_IBM_865 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_SYMBOL)
    {
        check( "RTL_TEXTENCODING_SYMBOL", RTL_TEXTENCODING_SYMBOL );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ASCII_US)
    {
        check( "RTL_TEXTENCODING_ASCII_US", RTL_TEXTENCODING_ASCII_US );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_1)
    {
        check( "RTL_TEXTENCODING_ISO_8859_1", RTL_TEXTENCODING_ISO_8859_1 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_2)
    {
        check( "RTL_TEXTENCODING_ISO_8859_2", RTL_TEXTENCODING_ISO_8859_2 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_3)
    {
        check( "RTL_TEXTENCODING_ISO_8859_3", RTL_TEXTENCODING_ISO_8859_3 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_4)
    {
        check( "RTL_TEXTENCODING_ISO_8859_4", RTL_TEXTENCODING_ISO_8859_4 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_5)
    {
        check( "RTL_TEXTENCODING_ISO_8859_5", RTL_TEXTENCODING_ISO_8859_5 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_6)
    {
        check( "RTL_TEXTENCODING_ISO_8859_6", RTL_TEXTENCODING_ISO_8859_6 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_7)
    {
        check( "RTL_TEXTENCODING_ISO_8859_7", RTL_TEXTENCODING_ISO_8859_7 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_8)
    {
        check( "RTL_TEXTENCODING_ISO_8859_8", RTL_TEXTENCODING_ISO_8859_8 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_9)
    {
        check( "RTL_TEXTENCODING_ISO_8859_9", RTL_TEXTENCODING_ISO_8859_9 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_14)
    {
        check( "RTL_TEXTENCODING_ISO_8859_14", RTL_TEXTENCODING_ISO_8859_14 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_15)
    {
        check( "RTL_TEXTENCODING_ISO_8859_15", RTL_TEXTENCODING_ISO_8859_15 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_737)
    {
        check( "RTL_TEXTENCODING_IBM_737", RTL_TEXTENCODING_IBM_737 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_775)
    {
        check( "RTL_TEXTENCODING_IBM_775", RTL_TEXTENCODING_IBM_775 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_852)
    {
        check( "RTL_TEXTENCODING_IBM_852", RTL_TEXTENCODING_IBM_852 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_855)
    {
        check( "RTL_TEXTENCODING_IBM_855", RTL_TEXTENCODING_IBM_855 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_857)
    {
        check( "RTL_TEXTENCODING_IBM_857", RTL_TEXTENCODING_IBM_857 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_862)
    {
        check( "RTL_TEXTENCODING_IBM_862", RTL_TEXTENCODING_IBM_862 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_864)
    {
        check( "RTL_TEXTENCODING_IBM_864", RTL_TEXTENCODING_IBM_864 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_866)
    {
        check( "RTL_TEXTENCODING_IBM_866", RTL_TEXTENCODING_IBM_866 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_IBM_869)
    {
        check( "RTL_TEXTENCODING_IBM_869", RTL_TEXTENCODING_IBM_869 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_874)
    {
        check( "RTL_TEXTENCODING_MS_874", RTL_TEXTENCODING_MS_874 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_1250)
    {
        check( "RTL_TEXTENCODING_MS_1250", RTL_TEXTENCODING_MS_1250 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_1251)
    {
        check( "RTL_TEXTENCODING_MS_1251", RTL_TEXTENCODING_MS_1251 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_1253)
    {
        check( "RTL_TEXTENCODING_MS_1253", RTL_TEXTENCODING_MS_1253 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_1254)
    {
        check( "RTL_TEXTENCODING_MS_1254", RTL_TEXTENCODING_MS_1254 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_1255)
    {
        check( "RTL_TEXTENCODING_MS_1255", RTL_TEXTENCODING_MS_1255 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_1256)
    {
        check( "RTL_TEXTENCODING_MS_1256", RTL_TEXTENCODING_MS_1256 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_1257)
    {
        check( "RTL_TEXTENCODING_MS_1257", RTL_TEXTENCODING_MS_1257 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_1258)
    {
        check( "RTL_TEXTENCODING_MS_1258", RTL_TEXTENCODING_MS_1258 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_ARABIC)
    {
        check( "RTL_TEXTENCODING_APPLE_ARABIC", RTL_TEXTENCODING_APPLE_ARABIC );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_CENTEURO)
    {
        check( "RTL_TEXTENCODING_APPLE_CENTEURO", RTL_TEXTENCODING_APPLE_CENTEURO );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_CROATIAN)
    {
        check( "RTL_TEXTENCODING_APPLE_CROATIAN", RTL_TEXTENCODING_APPLE_CROATIAN );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_CYRILLIC)
    {
        check( "RTL_TEXTENCODING_APPLE_CYRILLIC", RTL_TEXTENCODING_APPLE_CYRILLIC );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_DEVANAGARI)
    {
        check( "RTL_TEXTENCODING_APPLE_DEVANAGARI", RTL_TEXTENCODING_APPLE_DEVANAGARI );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_FARSI)
    {
        check( "RTL_TEXTENCODING_APPLE_FARSI", RTL_TEXTENCODING_APPLE_FARSI );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_GREEK)
    {
        check( "RTL_TEXTENCODING_APPLE_GREEK", RTL_TEXTENCODING_APPLE_GREEK );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_GUJARATI)
    {
        check( "RTL_TEXTENCODING_APPLE_GUJARATI", RTL_TEXTENCODING_APPLE_GUJARATI );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_GURMUKHI)
    {
        check( "RTL_TEXTENCODING_APPLE_GURMUKHI", RTL_TEXTENCODING_APPLE_GURMUKHI );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_HEBREW)
    {
        check( "RTL_TEXTENCODING_APPLE_HEBREW", RTL_TEXTENCODING_APPLE_HEBREW );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_ICELAND)
    {
        check( "RTL_TEXTENCODING_APPLE_ICELAND", RTL_TEXTENCODING_APPLE_ICELAND );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_ROMANIAN)
    {
        check( "RTL_TEXTENCODING_APPLE_ROMANIAN", RTL_TEXTENCODING_APPLE_ROMANIAN );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_THAI)
    {
        check( "RTL_TEXTENCODING_APPLE_THAI", RTL_TEXTENCODING_APPLE_THAI );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_TURKISH)
    {
        check( "RTL_TEXTENCODING_APPLE_TURKISH", RTL_TEXTENCODING_APPLE_TURKISH );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_UKRAINIAN)
    {
        check( "RTL_TEXTENCODING_APPLE_UKRAINIAN", RTL_TEXTENCODING_APPLE_UKRAINIAN );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_CHINSIMP)
    {
        check( "RTL_TEXTENCODING_APPLE_CHINSIMP", RTL_TEXTENCODING_APPLE_CHINSIMP );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_CHINTRAD)
    {
        check( "RTL_TEXTENCODING_APPLE_CHINTRAD", RTL_TEXTENCODING_APPLE_CHINTRAD );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_JAPANESE)
    {
        check( "RTL_TEXTENCODING_APPLE_JAPANESE", RTL_TEXTENCODING_APPLE_JAPANESE );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_APPLE_KOREAN)
    {
        check( "RTL_TEXTENCODING_APPLE_KOREAN", RTL_TEXTENCODING_APPLE_KOREAN );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_932)
    {
        check( "RTL_TEXTENCODING_MS_932", RTL_TEXTENCODING_MS_932 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_936)
    {
        check( "RTL_TEXTENCODING_MS_936", RTL_TEXTENCODING_MS_936 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_949)
    {
        check( "RTL_TEXTENCODING_MS_949", RTL_TEXTENCODING_MS_949 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_950)
    {
        check( "RTL_TEXTENCODING_MS_950", RTL_TEXTENCODING_MS_950 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_SHIFT_JIS)
    {
        check( "RTL_TEXTENCODING_SHIFT_JIS", RTL_TEXTENCODING_SHIFT_JIS );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_GB_2312)
    {
        check( "RTL_TEXTENCODING_GB_2312", RTL_TEXTENCODING_GB_2312 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_GBT_12345)
    {
        check( "RTL_TEXTENCODING_GBT_12345", RTL_TEXTENCODING_GBT_12345 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_GBK)
    {
        check( "RTL_TEXTENCODING_GBK", RTL_TEXTENCODING_GBK );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_BIG5)
    {
        check( "RTL_TEXTENCODING_BIG5", RTL_TEXTENCODING_BIG5 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_EUC_JP)
    {
        check( "RTL_TEXTENCODING_EUC_JP", RTL_TEXTENCODING_EUC_JP );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_EUC_CN)
    {
        check( "RTL_TEXTENCODING_EUC_CN", RTL_TEXTENCODING_EUC_CN );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_EUC_TW)
    {
        check( "RTL_TEXTENCODING_EUC_TW", RTL_TEXTENCODING_EUC_TW );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_2022_JP)
    {
        check( "RTL_TEXTENCODING_ISO_2022_JP", RTL_TEXTENCODING_ISO_2022_JP );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_2022_CN)
    {
        check( "RTL_TEXTENCODING_ISO_2022_CN", RTL_TEXTENCODING_ISO_2022_CN );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_KOI8_R)
    {
        check( "RTL_TEXTENCODING_KOI8_R", RTL_TEXTENCODING_KOI8_R );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_UTF7)
    {
        check( "RTL_TEXTENCODING_UTF7", RTL_TEXTENCODING_UTF7 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_UTF8)
    {
        check( "RTL_TEXTENCODING_UTF8", RTL_TEXTENCODING_UTF8 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_10)
    {
        check( "RTL_TEXTENCODING_ISO_8859_10", RTL_TEXTENCODING_ISO_8859_10 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_8859_13)
    {
        check( "RTL_TEXTENCODING_ISO_8859_13", RTL_TEXTENCODING_ISO_8859_13 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_EUC_KR)
    {
        check( "RTL_TEXTENCODING_EUC_KR", RTL_TEXTENCODING_EUC_KR );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISO_2022_KR)
    {
        check( "RTL_TEXTENCODING_ISO_2022_KR", RTL_TEXTENCODING_ISO_2022_KR );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_JIS_X_0201)
    {
        check( "RTL_TEXTENCODING_JIS_X_0201", RTL_TEXTENCODING_JIS_X_0201 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_JIS_X_0208)
    {
        check( "RTL_TEXTENCODING_JIS_X_0208", RTL_TEXTENCODING_JIS_X_0208 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_JIS_X_0212)
    {
        check( "RTL_TEXTENCODING_JIS_X_0212", RTL_TEXTENCODING_JIS_X_0212 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_MS_1361)
    {
        check( "RTL_TEXTENCODING_MS_1361", RTL_TEXTENCODING_MS_1361 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_GB_18030)
    {
        check( "RTL_TEXTENCODING_GB_18030", RTL_TEXTENCODING_GB_18030 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_BIG5_HKSCS)
    {
        check( "RTL_TEXTENCODING_BIG5_HKSCS", RTL_TEXTENCODING_BIG5_HKSCS );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_TIS_620)
    {
        check( "RTL_TEXTENCODING_TIS_620", RTL_TEXTENCODING_TIS_620 );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_KOI8_U)
    {
        check( "RTL_TEXTENCODING_KOI8_U", RTL_TEXTENCODING_KOI8_U );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_ISCII_DEVANAGARI)
    {
        check( "RTL_TEXTENCODING_ISCII_DEVANAGARI", RTL_TEXTENCODING_ISCII_DEVANAGARI );
    }
    // ----------------------------------------
    TEST_F(getBestUnix, UnixCharsetFromTextEncoding_JAVA_UTF8)
    {
        check( "RTL_TEXTENCODING_JAVA_UTF8", RTL_TEXTENCODING_JAVA_UTF8 );
    }
    // ----------------------------------------

 // -----------------------------------------------------------------------------


    class getBestWindows : public ::testing::Test
    {
    public:
        void SetUp()
            {
            }

        void check( const sal_Char* _pRTL_TEXTENCODING, rtl_TextEncoding _aCurrentEncode )
            {
                const sal_uInt8 nCharSet = rtl_getBestWindowsCharsetFromTextEncoding( _aCurrentEncode );
                if (nCharSet == 1)
                {
                    printf("rtl_getBestWindowsCharsetFromTextEncoding(%s) (%d) doesn't seem to exist.\n\n", _pRTL_TEXTENCODING, _aCurrentEncode);
                }
                else
                {
                    printf("'%s' is charset: '%d'\n", _pRTL_TEXTENCODING, nCharSet);

                    rtl_TextEncoding eTextEnc = rtl_getTextEncodingFromWindowsCharset( nCharSet );
                    if (_aCurrentEncode != eTextEnc &&
                        eTextEnc != RTL_TEXTENCODING_DONTKNOW)
                    {
                        printf("rtl_getBestUnixCharsetFromTextEncoding(%s) is charset: %d\n", _pRTL_TEXTENCODING, nCharSet);
                        printf("rtl_getTextEncodingFromWindowsCharset() differ: %s %d -> %d\n\n", _pRTL_TEXTENCODING, _aCurrentEncode, eTextEnc );
                    }
                    // rtl::OString sError = "getTextEncodingFromWindowsCharset(";
                    // sError += rtl::OString::valueOf(nCharSet);
                    // sError += ") returns nul.";

                    // ASSERT_TRUE(eTextEnc != RTL_TEXTENCODING_DONTKNOW) << sError.getStr();
                    // ASSERT_TRUE(_aCurrentEncode == eTextEnc) << "Does not realize itself";
                }
            }
    };

    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_1252)
    {
        check( "RTL_TEXTENCODING_MS_1252", RTL_TEXTENCODING_MS_1252 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_ROMAN)
    {
        check( "RTL_TEXTENCODING_APPLE_ROMAN", RTL_TEXTENCODING_APPLE_ROMAN );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_437)
    {
        check( "RTL_TEXTENCODING_IBM_437", RTL_TEXTENCODING_IBM_437 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_850)
    {
        check( "RTL_TEXTENCODING_IBM_850", RTL_TEXTENCODING_IBM_850 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_860)
    {
        check( "RTL_TEXTENCODING_IBM_860", RTL_TEXTENCODING_IBM_860 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_861)
    {
        check( "RTL_TEXTENCODING_IBM_861", RTL_TEXTENCODING_IBM_861 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_863)
    {
        check( "RTL_TEXTENCODING_IBM_863", RTL_TEXTENCODING_IBM_863 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_865)
    {
        check( "RTL_TEXTENCODING_IBM_865", RTL_TEXTENCODING_IBM_865 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_SYMBOL)
    {
        check( "RTL_TEXTENCODING_SYMBOL", RTL_TEXTENCODING_SYMBOL );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ASCII_US)
    {
        check( "RTL_TEXTENCODING_ASCII_US", RTL_TEXTENCODING_ASCII_US );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_1)
    {
        check( "RTL_TEXTENCODING_ISO_8859_1", RTL_TEXTENCODING_ISO_8859_1 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_2)
    {
        check( "RTL_TEXTENCODING_ISO_8859_2", RTL_TEXTENCODING_ISO_8859_2 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_3)
    {
        check( "RTL_TEXTENCODING_ISO_8859_3", RTL_TEXTENCODING_ISO_8859_3 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_4)
    {
        check( "RTL_TEXTENCODING_ISO_8859_4", RTL_TEXTENCODING_ISO_8859_4 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_5)
    {
        check( "RTL_TEXTENCODING_ISO_8859_5", RTL_TEXTENCODING_ISO_8859_5 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_6)
    {
        check( "RTL_TEXTENCODING_ISO_8859_6", RTL_TEXTENCODING_ISO_8859_6 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_7)
    {
        check( "RTL_TEXTENCODING_ISO_8859_7", RTL_TEXTENCODING_ISO_8859_7 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_8)
    {
        check( "RTL_TEXTENCODING_ISO_8859_8", RTL_TEXTENCODING_ISO_8859_8 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_9)
    {
        check( "RTL_TEXTENCODING_ISO_8859_9", RTL_TEXTENCODING_ISO_8859_9 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_14)
    {
        check( "RTL_TEXTENCODING_ISO_8859_14", RTL_TEXTENCODING_ISO_8859_14 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_15)
    {
        check( "RTL_TEXTENCODING_ISO_8859_15", RTL_TEXTENCODING_ISO_8859_15 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_737)
    {
        check( "RTL_TEXTENCODING_IBM_737", RTL_TEXTENCODING_IBM_737 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_775)
    {
        check( "RTL_TEXTENCODING_IBM_775", RTL_TEXTENCODING_IBM_775 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_852)
    {
        check( "RTL_TEXTENCODING_IBM_852", RTL_TEXTENCODING_IBM_852 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_855)
    {
        check( "RTL_TEXTENCODING_IBM_855", RTL_TEXTENCODING_IBM_855 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_857)
    {
        check( "RTL_TEXTENCODING_IBM_857", RTL_TEXTENCODING_IBM_857 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_862)
    {
        check( "RTL_TEXTENCODING_IBM_862", RTL_TEXTENCODING_IBM_862 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_864)
    {
        check( "RTL_TEXTENCODING_IBM_864", RTL_TEXTENCODING_IBM_864 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_866)
    {
        check( "RTL_TEXTENCODING_IBM_866", RTL_TEXTENCODING_IBM_866 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_IBM_869)
    {
        check( "RTL_TEXTENCODING_IBM_869", RTL_TEXTENCODING_IBM_869 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_874)
    {
        check( "RTL_TEXTENCODING_MS_874", RTL_TEXTENCODING_MS_874 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_1250)
    {
        check( "RTL_TEXTENCODING_MS_1250", RTL_TEXTENCODING_MS_1250 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_1251)
    {
        check( "RTL_TEXTENCODING_MS_1251", RTL_TEXTENCODING_MS_1251 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_1253)
    {
        check( "RTL_TEXTENCODING_MS_1253", RTL_TEXTENCODING_MS_1253 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_1254)
    {
        check( "RTL_TEXTENCODING_MS_1254", RTL_TEXTENCODING_MS_1254 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_1255)
    {
        check( "RTL_TEXTENCODING_MS_1255", RTL_TEXTENCODING_MS_1255 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_1256)
    {
        check( "RTL_TEXTENCODING_MS_1256", RTL_TEXTENCODING_MS_1256 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_1257)
    {
        check( "RTL_TEXTENCODING_MS_1257", RTL_TEXTENCODING_MS_1257 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_1258)
    {
        check( "RTL_TEXTENCODING_MS_1258", RTL_TEXTENCODING_MS_1258 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_ARABIC)
    {
        check( "RTL_TEXTENCODING_APPLE_ARABIC", RTL_TEXTENCODING_APPLE_ARABIC );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_CENTEURO)
    {
        check( "RTL_TEXTENCODING_APPLE_CENTEURO", RTL_TEXTENCODING_APPLE_CENTEURO );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_CROATIAN)
    {
        check( "RTL_TEXTENCODING_APPLE_CROATIAN", RTL_TEXTENCODING_APPLE_CROATIAN );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_CYRILLIC)
    {
        check( "RTL_TEXTENCODING_APPLE_CYRILLIC", RTL_TEXTENCODING_APPLE_CYRILLIC );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_DEVANAGARI)
    {
        check( "RTL_TEXTENCODING_APPLE_DEVANAGARI", RTL_TEXTENCODING_APPLE_DEVANAGARI );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_FARSI)
    {
        check( "RTL_TEXTENCODING_APPLE_FARSI", RTL_TEXTENCODING_APPLE_FARSI );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_GREEK)
    {
        check( "RTL_TEXTENCODING_APPLE_GREEK", RTL_TEXTENCODING_APPLE_GREEK );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_GUJARATI)
    {
        check( "RTL_TEXTENCODING_APPLE_GUJARATI", RTL_TEXTENCODING_APPLE_GUJARATI );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_GURMUKHI)
    {
        check( "RTL_TEXTENCODING_APPLE_GURMUKHI", RTL_TEXTENCODING_APPLE_GURMUKHI );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_HEBREW)
    {
        check( "RTL_TEXTENCODING_APPLE_HEBREW", RTL_TEXTENCODING_APPLE_HEBREW );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_ICELAND)
    {
        check( "RTL_TEXTENCODING_APPLE_ICELAND", RTL_TEXTENCODING_APPLE_ICELAND );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_ROMANIAN)
    {
        check( "RTL_TEXTENCODING_APPLE_ROMANIAN", RTL_TEXTENCODING_APPLE_ROMANIAN );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_THAI)
    {
        check( "RTL_TEXTENCODING_APPLE_THAI", RTL_TEXTENCODING_APPLE_THAI );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_TURKISH)
    {
        check( "RTL_TEXTENCODING_APPLE_TURKISH", RTL_TEXTENCODING_APPLE_TURKISH );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_UKRAINIAN)
    {
        check( "RTL_TEXTENCODING_APPLE_UKRAINIAN", RTL_TEXTENCODING_APPLE_UKRAINIAN );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_CHINSIMP)
    {
        check( "RTL_TEXTENCODING_APPLE_CHINSIMP", RTL_TEXTENCODING_APPLE_CHINSIMP );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_CHINTRAD)
    {
        check( "RTL_TEXTENCODING_APPLE_CHINTRAD", RTL_TEXTENCODING_APPLE_CHINTRAD );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_JAPANESE)
    {
        check( "RTL_TEXTENCODING_APPLE_JAPANESE", RTL_TEXTENCODING_APPLE_JAPANESE );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_APPLE_KOREAN)
    {
        check( "RTL_TEXTENCODING_APPLE_KOREAN", RTL_TEXTENCODING_APPLE_KOREAN );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_932)
    {
        check( "RTL_TEXTENCODING_MS_932", RTL_TEXTENCODING_MS_932 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_936)
    {
        check( "RTL_TEXTENCODING_MS_936", RTL_TEXTENCODING_MS_936 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_949)
    {
        check( "RTL_TEXTENCODING_MS_949", RTL_TEXTENCODING_MS_949 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_950)
    {
        check( "RTL_TEXTENCODING_MS_950", RTL_TEXTENCODING_MS_950 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_SHIFT_JIS)
    {
        check( "RTL_TEXTENCODING_SHIFT_JIS", RTL_TEXTENCODING_SHIFT_JIS );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_GB_2312)
    {
        check( "RTL_TEXTENCODING_GB_2312", RTL_TEXTENCODING_GB_2312 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_GBT_12345)
    {
        check( "RTL_TEXTENCODING_GBT_12345", RTL_TEXTENCODING_GBT_12345 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_GBK)
    {
        check( "RTL_TEXTENCODING_GBK", RTL_TEXTENCODING_GBK );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_BIG5)
    {
        check( "RTL_TEXTENCODING_BIG5", RTL_TEXTENCODING_BIG5 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_EUC_JP)
    {
        check( "RTL_TEXTENCODING_EUC_JP", RTL_TEXTENCODING_EUC_JP );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_EUC_CN)
    {
        check( "RTL_TEXTENCODING_EUC_CN", RTL_TEXTENCODING_EUC_CN );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_EUC_TW)
    {
        check( "RTL_TEXTENCODING_EUC_TW", RTL_TEXTENCODING_EUC_TW );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_2022_JP)
    {
        check( "RTL_TEXTENCODING_ISO_2022_JP", RTL_TEXTENCODING_ISO_2022_JP );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_2022_CN)
    {
        check( "RTL_TEXTENCODING_ISO_2022_CN", RTL_TEXTENCODING_ISO_2022_CN );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_KOI8_R)
    {
        check( "RTL_TEXTENCODING_KOI8_R", RTL_TEXTENCODING_KOI8_R );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_UTF7)
    {
        check( "RTL_TEXTENCODING_UTF7", RTL_TEXTENCODING_UTF7 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_UTF8)
    {
        check( "RTL_TEXTENCODING_UTF8", RTL_TEXTENCODING_UTF8 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_10)
    {
        check( "RTL_TEXTENCODING_ISO_8859_10", RTL_TEXTENCODING_ISO_8859_10 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_8859_13)
    {
        check( "RTL_TEXTENCODING_ISO_8859_13", RTL_TEXTENCODING_ISO_8859_13 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_EUC_KR)
    {
        check( "RTL_TEXTENCODING_EUC_KR", RTL_TEXTENCODING_EUC_KR );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISO_2022_KR)
    {
        check( "RTL_TEXTENCODING_ISO_2022_KR", RTL_TEXTENCODING_ISO_2022_KR );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_JIS_X_0201)
    {
        check( "RTL_TEXTENCODING_JIS_X_0201", RTL_TEXTENCODING_JIS_X_0201 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_JIS_X_0208)
    {
        check( "RTL_TEXTENCODING_JIS_X_0208", RTL_TEXTENCODING_JIS_X_0208 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_JIS_X_0212)
    {
        check( "RTL_TEXTENCODING_JIS_X_0212", RTL_TEXTENCODING_JIS_X_0212 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_MS_1361)
    {
        check( "RTL_TEXTENCODING_MS_1361", RTL_TEXTENCODING_MS_1361 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_GB_18030)
    {
        check( "RTL_TEXTENCODING_GB_18030", RTL_TEXTENCODING_GB_18030 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_BIG5_HKSCS)
    {
        check( "RTL_TEXTENCODING_BIG5_HKSCS", RTL_TEXTENCODING_BIG5_HKSCS );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_TIS_620)
    {
        check( "RTL_TEXTENCODING_TIS_620", RTL_TEXTENCODING_TIS_620 );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_KOI8_U)
    {
        check( "RTL_TEXTENCODING_KOI8_U", RTL_TEXTENCODING_KOI8_U );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_ISCII_DEVANAGARI)
    {
        check( "RTL_TEXTENCODING_ISCII_DEVANAGARI", RTL_TEXTENCODING_ISCII_DEVANAGARI );
    }
    // ----------------------------------------
    TEST_F(getBestWindows, WindowsCharsetFromTextEncoding_JAVA_UTF8)
    {
        check( "RTL_TEXTENCODING_JAVA_UTF8", RTL_TEXTENCODING_JAVA_UTF8 );
    }
    // ----------------------------------------


    class getTextEncodingInfo: public ::testing::Test
    {
    public:
        // initialise your test code values here.
        void SetUp()
            {
            }

        void TearDown()
            {
            }
    };


    // not implemented encoding test
    TEST_F(getTextEncodingInfo, getTextEncodingInfo_001)
    {
        rtl_TextEncodingInfo aInfo1, aInfo2, aInfo3, aInfo4, aInfo5;
        aInfo1.StructSize = 4;
        // not implemented
        sal_Bool bRes1 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo1 );
        // implemented
        sal_Bool bRes11 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo1 );
        ASSERT_TRUE(bRes1 == sal_False && bRes11 == sal_False) << "should return sal_False.";

        aInfo2.StructSize = 5;
        sal_Bool bRes2 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo2 );
        sal_Bool bRes21 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo2 );
        ASSERT_TRUE(bRes2 == sal_True && bRes21 == sal_True && aInfo2.MinimumCharSize >=1) << "StructSize<6 should return sal_True";

        aInfo3.StructSize = 6;
        sal_Bool bRes3 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo3 );
        sal_Bool bRes31 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo3 );
        ASSERT_TRUE(bRes3 == sal_True && bRes31 == sal_True) << "StructSize<6 should return sal_True";
//&& aInfo2.MinimumCharSize >=1 );

        aInfo4.StructSize = 8;
        sal_Bool bRes4 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo4 );
        sal_Bool bRes41 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_CYRILLIC, &aInfo4);
        ASSERT_TRUE(bRes4 == sal_True && bRes41 == sal_True) << "StructSize<6 should return sal_True";
// && aInfo2.MinimumCharSize >=1 );

    aInfo5.StructSize = sizeof aInfo5;
        sal_Bool bRes5 = rtl_getTextEncodingInfo( RTL_TEXTENCODING_APPLE_ARABIC, &aInfo5 );
        ASSERT_TRUE(bRes5 == sal_False && aInfo5.Flags == 0) << "StructSize<6 should return sal_True";
    }
}

namespace {

class TestEncodingFromUnix: public ::testing::Test {
protected:
    void check(rtl_TextEncoding expected, char const * input) {
        ASSERT_EQ(
            expected, rtl_getTextEncodingFromUnixCharset(input)) << input;
    }
};

TEST_F(TestEncodingFromUnix, testIso8859) {
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

TEST_F(TestEncodingFromUnix, testTis620) {
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

}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
