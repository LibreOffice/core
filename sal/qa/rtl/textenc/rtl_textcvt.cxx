/*************************************************************************
 *
 *  $RCSfile: rtl_textcvt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 11:38:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// LLA:
// this file is converted to use with testshl2
// original was placed in sal/test/textenc.cxx


// -----------------------------------------------------------------------------

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// #ifndef _OSL_DIAGNOSE_H_
// #include "osl/diagnose.h"
// #endif
#ifndef _RTL_STRING_H_
#include "rtl/string.h"
#endif
#ifndef _RTL_TENCINFO_H
#include "rtl/tencinfo.h"
#endif
#ifndef _RTL_TEXTENC_H
#include "rtl/textenc.h"
#endif
#ifndef _RTL_TEXTCVT_H
#include "rtl/textcvt.h"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#include <cppunit/simpleheader.hxx>

#define TEST_ENSURE(c, m) CPPUNIT_ASSERT_MESSAGE((m), (c))

// #if OSL_DEBUG_LEVEL > 0
// #define TEST_ENSURE(c, m) OSL_ENSURE((c), (m))
// #else // OSL_DEBUG_LEVEL
// #define TEST_ENSURE(c, m) if(!(c)) printf("Failed: %s\n", (m))
// #endif // OSL_DEBUG_LEVEL

// -----------------------------------------------------------------------------

namespace rtl_textenc
{

    struct SingleByteCharSet
    {
        rtl_TextEncoding m_nEncoding;
        sal_Unicode m_aMap[256];
    };

void testSingleByteCharSet(SingleByteCharSet const & rSet)
{
    sal_Char aText[256];
    sal_Unicode aUnicode[256];

    sal_Size nNumber = 0;
    {for (int i = 0; i < 256; ++i)
        if (rSet.m_aMap[i] != 0xFFFF)
            aText[nNumber++] = static_cast< sal_Char >(i);
    }

    {
        rtl_TextToUnicodeConverter aConverter
            = rtl_createTextToUnicodeConverter(rSet.m_nEncoding);
        rtl_TextToUnicodeContext aContext
            = rtl_createTextToUnicodeContext(aConverter);
        TEST_ENSURE(aConverter && aContext, "failure #1");

        sal_Size nSize;
        sal_uInt32 nInfo;
        sal_Size nConverted;
        nSize = rtl_convertTextToUnicode(
                    aConverter,
                    aContext,
                    aText,
                    nNumber,
                    aUnicode,
                    nNumber,
                    RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                        | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                        | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR,
                    &nInfo,
                    &nConverted);
        TEST_ENSURE(nSize == nNumber && nInfo == 0 && nConverted == nNumber,
                    "failure #2");

        rtl_destroyTextToUnicodeContext(aConverter, aContext);
        rtl_destroyTextToUnicodeConverter(aConverter);
    }

    {
        bool bSuccess = true;
        int j = 0;
        for (int i = 0; i < 256; ++i)
            if (rSet.m_aMap[i] != 0xFFFF && aUnicode[j++] != rSet.m_aMap[i])
            {
                bSuccess = false;
                break;
            }
        TEST_ENSURE(bSuccess, "failure #3");
    }

    if (rSet.m_nEncoding == RTL_TEXTENCODING_ASCII_US)
        nNumber = 128;

    {
        rtl_UnicodeToTextConverter aConverter
            = rtl_createUnicodeToTextConverter(rSet.m_nEncoding);
        rtl_UnicodeToTextContext aContext
            = rtl_createUnicodeToTextContext(aConverter);
        TEST_ENSURE(aConverter && aContext, "failure #4");

        sal_Size nSize;
        sal_uInt32 nInfo;
        sal_Size nConverted;
        nSize = rtl_convertUnicodeToText(
                    aConverter,
                    aContext,
                    aUnicode,
                    nNumber,
                    aText,
                    nNumber,
                    RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                        | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR,
                    &nInfo,
                    &nConverted);
        TEST_ENSURE(nSize == nNumber && nInfo == 0 && nConverted == nNumber,
                    "failure #5");

        rtl_destroyUnicodeToTextContext(aConverter, aContext);
        rtl_destroyUnicodeToTextConverter(aConverter);
    }

    {
        bool bSuccess = true;
        int j = 0;
        for (int i = 0; i < 256; ++i)
            if (rSet.m_aMap[i] != 0xFFFF
                && aText[j++] != static_cast< sal_Char >(i))
            {
                bSuccess = false;
                break;
            }
        TEST_ENSURE(bSuccess, "failure #6");
    }

    {for (int i = 0; i < 256; ++i)
        if (rSet.m_aMap[i] == 0xFFFF)
        {
            aText[0] = static_cast< sal_Char >(i);

            rtl_TextToUnicodeConverter aConverter
                = rtl_createTextToUnicodeConverter(rSet.m_nEncoding);
            rtl_TextToUnicodeContext aContext
                = rtl_createTextToUnicodeContext(aConverter);
            TEST_ENSURE(aConverter && aContext, "failure #7");

            sal_Size nSize;
            sal_uInt32 nInfo;
            sal_Size nConverted;
            nSize = rtl_convertTextToUnicode(
                        aConverter,
                        aContext,
                        aText,
                        1,
                        aUnicode,
                        1,
                        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                            | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                            | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR,
                        &nInfo,
                        &nConverted);
            TEST_ENSURE(nSize == 0
                        && nInfo == (RTL_TEXTTOUNICODE_INFO_ERROR
                                         | RTL_TEXTTOUNICODE_INFO_UNDEFINED)
                        && nConverted == 0,
                        "failure #9");

            rtl_destroyTextToUnicodeContext(aConverter, aContext);
            rtl_destroyTextToUnicodeConverter(aConverter);
        }
    }
}

int const TEST_STRING_SIZE = 1000;

struct ComplexCharSetTest
{
    rtl_TextEncoding m_nEncoding;
    char const * m_pText;
    sal_Size m_nTextSize;
    sal_Unicode m_aUnicode[TEST_STRING_SIZE];
    sal_Size m_nUnicodeSize;
    bool m_bNoContext;
    bool m_bForward;
    bool m_bReverse;
    bool m_bGlobalSignature;
    sal_uInt32 m_nReverseUndefined;
};

void doComplexCharSetTest(ComplexCharSetTest const & rTest)
{
    if (rTest.m_bForward)
    {
        sal_Unicode aUnicode[TEST_STRING_SIZE];

        rtl_TextToUnicodeConverter aConverter
            = rtl_createTextToUnicodeConverter(rTest.m_nEncoding);
        rtl_TextToUnicodeContext aContext
            = rtl_createTextToUnicodeContext(aConverter);
        TEST_ENSURE(aConverter && aContext, "failure #10");

        sal_Size nSize;
        sal_uInt32 nInfo;
        sal_Size nConverted;
        nSize = rtl_convertTextToUnicode(
                    aConverter,
                    aContext,
                    reinterpret_cast< sal_Char const * >(rTest.m_pText),
                    rTest.m_nTextSize,
                    aUnicode,
                    TEST_STRING_SIZE,
                    RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                        | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                        | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR
                        | RTL_TEXTTOUNICODE_FLAGS_FLUSH
                        | (rTest.m_bGlobalSignature ?
                               RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE : 0),
                    &nInfo,
                    &nConverted);
        TEST_ENSURE(nSize == rTest.m_nUnicodeSize
                    && nInfo == 0
                    && nConverted == rTest.m_nTextSize,
                    "failure #11");

        rtl_destroyTextToUnicodeContext(aConverter, aContext);
        rtl_destroyTextToUnicodeConverter(aConverter);

        bool bSuccess = true;
        for (sal_Size i = 0; i < rTest.m_nUnicodeSize; ++i)
            if (aUnicode[i] != rTest.m_aUnicode[i])
            {
                bSuccess = false;
                break;
            }
        TEST_ENSURE(bSuccess, "failure #12");
    }

    if (rTest.m_bForward)
    {
        sal_Unicode aUnicode[TEST_STRING_SIZE];

        rtl_TextToUnicodeConverter aConverter
            = rtl_createTextToUnicodeConverter(rTest.m_nEncoding);
        rtl_TextToUnicodeContext aContext
            = rtl_createTextToUnicodeContext(aConverter);
        TEST_ENSURE(aConverter && aContext, "failure #13");

        if (aContext != (rtl_TextToUnicodeContext) 1)
        {
            sal_Size nInput = 0;
            sal_Size nOutput = 0;
            for (bool bFlush = true; nInput < rTest.m_nTextSize || bFlush;)
            {
                sal_Size nSrcBytes = 1;
                sal_uInt32 nFlags
                    = RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                          | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                          | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR
                          | (rTest.m_bGlobalSignature ?
                                 RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE : 0);
                if (nInput >= rTest.m_nTextSize)
                {
                    nSrcBytes = 0;
                    nFlags |= RTL_TEXTTOUNICODE_FLAGS_FLUSH;
                    bFlush = false;
                }
                sal_uInt32 nInfo;
                sal_Size nConverted;
                sal_Size nSize = rtl_convertTextToUnicode(
                                     aConverter,
                                     aContext,
                                     reinterpret_cast< sal_Char const * >(
                                         rTest.m_pText + nInput),
                                     nSrcBytes,
                                     aUnicode + nOutput,
                                     TEST_STRING_SIZE - nOutput,
                                     nFlags,
                                     &nInfo,
                                     &nConverted);
                nOutput += nSize;
                nInput += nConverted;
                TEST_ENSURE((nInfo & ~RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL)
                                == 0,
                            "failure #14");
            }
            TEST_ENSURE(nOutput == rTest.m_nUnicodeSize
                        && nInput == rTest.m_nTextSize,
                        "failure #15");

            bool bSuccess = true;
            for (sal_Size i = 0; i < rTest.m_nUnicodeSize; ++i)
                if (aUnicode[i] != rTest.m_aUnicode[i])
                {
                    bSuccess = false;
                    break;
                }
            TEST_ENSURE(bSuccess, "failure #16");
        }

        rtl_destroyTextToUnicodeContext(aConverter, aContext);
        rtl_destroyTextToUnicodeConverter(aConverter);
    }

    if (rTest.m_bNoContext && rTest.m_bForward)
    {
        sal_Unicode aUnicode[TEST_STRING_SIZE];
        int nSize = 0;

        rtl_TextToUnicodeConverter aConverter
            = rtl_createTextToUnicodeConverter(rTest.m_nEncoding);
        TEST_ENSURE(aConverter, "failure #17");

        for (sal_Size i = 0;;)
        {
            if (i == rTest.m_nTextSize)
                goto done;
            sal_Char c1 = rTest.m_pText[i++];

            sal_Unicode aUC[2];
            sal_uInt32 nInfo = 0;
            sal_Size nCvtBytes;
            sal_Size nChars
                = rtl_convertTextToUnicode(
                      aConverter,
                      0,
                      &c1,
                      1,
                      aUC,
                      2,
                      RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                          | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                          | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR
                        /*| RTL_TEXTTOUNICODE_FLAGS_FLUSH*/
                          | (rTest.m_bGlobalSignature ?
                                 RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE : 0),
                      &nInfo,
                      &nCvtBytes);
            if ((nInfo & RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL) != 0)
            {
                sal_Char sBuffer[10];
                sBuffer[0] = c1;
                sal_uInt16 nLen = 1;
                while ((nInfo & RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL) != 0
                       && nLen < 10)
                {
                    if (i == rTest.m_nTextSize)
                        goto done;
                    c1 = rTest.m_pText[i++];

                    sBuffer[nLen++] = c1;
                    nChars
                        = rtl_convertTextToUnicode(
                              aConverter,
                              0,
                              sBuffer,
                              nLen,
                              aUC,
                              2,
                              RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                                  | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                                  | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR
                                /*| RTL_TEXTTOUNICODE_FLAGS_FLUSH*/
                                  | (rTest.m_bGlobalSignature ?
                                      RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE :
                                         0),
                              &nInfo,
                              &nCvtBytes);
                }
                if (nChars == 1 && nInfo == 0)
                {
                    OSL_ASSERT(nCvtBytes == nLen);
                    aUnicode[nSize++] = aUC[0];
                }
                else if (nChars == 2 && nInfo == 0)
                {
                    OSL_ASSERT(nCvtBytes == nLen);
                    aUnicode[nSize++] = aUC[0];
                    aUnicode[nSize++] = aUC[1];
                }
                else
                {
                    OSL_ASSERT(
                        (nInfo & RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL) == 0
                        && nChars == 0
                        && nInfo != 0);
                    aUnicode[nSize++] = sBuffer[0];
                    i -= nLen - 1;
                }
            }
            else if (nChars == 1 && nInfo == 0)
            {
                OSL_ASSERT(nCvtBytes == 1);
                aUnicode[nSize++] = aUC[0];
            }
            else if (nChars == 2 && nInfo == 0)
            {
                OSL_ASSERT(nCvtBytes == 1);
                aUnicode[nSize++] = aUC[0];
                aUnicode[nSize++] = aUC[1];
            }
            else
            {
                OSL_ASSERT(nChars == 0 && nInfo != 0);
                aUnicode[nSize++] = c1;
            }
        }
    done:

        rtl_destroyTextToUnicodeConverter(aConverter);

        bool bSuccess = true;
        {for (sal_Size i = 0; i < rTest.m_nUnicodeSize; ++i)
            if (aUnicode[i] != rTest.m_aUnicode[i])
            {
                bSuccess = false;
                break;
            }
        }
        TEST_ENSURE(bSuccess, "failure #18");
    }

    if (rTest.m_bReverse)
    {
        sal_Char aText[TEST_STRING_SIZE];

        rtl_UnicodeToTextConverter aConverter
            = rtl_createUnicodeToTextConverter(rTest.m_nEncoding);
        rtl_UnicodeToTextContext aContext
            = rtl_createUnicodeToTextContext(aConverter);
        TEST_ENSURE(aConverter && aContext, "failure #19");

        sal_Size nSize;
        sal_uInt32 nInfo;
        sal_Size nConverted;
        nSize = rtl_convertUnicodeToText(
                    aConverter,
                    aContext,
                    rTest.m_aUnicode,
                    rTest.m_nUnicodeSize,
                    aText,
                    TEST_STRING_SIZE,
                    rTest.m_nReverseUndefined
                        | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR
                        | RTL_UNICODETOTEXT_FLAGS_FLUSH
                        | (rTest.m_bGlobalSignature ?
                               RTL_UNICODETOTEXT_FLAGS_GLOBAL_SIGNATURE : 0),
                    &nInfo,
                    &nConverted);
        TEST_ENSURE(nSize == rTest.m_nTextSize
                    && (nInfo == 0
                        || (nInfo == RTL_UNICODETOTEXT_INFO_UNDEFINED
                            && (rTest.m_nReverseUndefined
                                != RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR)))
                    && nConverted == rTest.m_nUnicodeSize,
                    "failure #20");

        rtl_destroyUnicodeToTextContext(aConverter, aContext);
        rtl_destroyUnicodeToTextConverter(aConverter);

        bool bSuccess = true;
        for (sal_Size i = 0; i < rTest.m_nTextSize; ++i)
            if (aText[i] != rTest.m_pText[i])
            {
                bSuccess = false;
                break;
            }
        TEST_ENSURE(bSuccess, "failure #21");
    }
}

void doComplexCharSetCutTest(ComplexCharSetTest const & rTest)
{
    if (rTest.m_bNoContext)
    {
        sal_Unicode aUnicode[TEST_STRING_SIZE];

        rtl_TextToUnicodeConverter aConverter
            = rtl_createTextToUnicodeConverter(rTest.m_nEncoding);
        TEST_ENSURE(aConverter, "failure #22");

        sal_Size nSize;
        sal_uInt32 nInfo;
        sal_Size nConverted;
        nSize = rtl_convertTextToUnicode(
                    aConverter,
                    0,
                    reinterpret_cast< sal_Char const * >(rTest.m_pText),
                    rTest.m_nTextSize,
                    aUnicode,
                    TEST_STRING_SIZE,
                    RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                        | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                        | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR,
                    &nInfo,
                    &nConverted);
        TEST_ENSURE(nSize <= rTest.m_nUnicodeSize
                    && (nInfo == RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL
                        || nInfo
                               == (RTL_TEXTTOUNICODE_INFO_ERROR
                                   | RTL_TEXTTOUNICODE_INFO_SRCBUFFERTOSMALL))
                    && nConverted < rTest.m_nTextSize,
                    "failure #23");

        rtl_destroyTextToUnicodeConverter(aConverter);

        bool bSuccess = true;
        for (sal_Size i = 0; i < nSize; ++i)
            if (aUnicode[i] != rTest.m_aUnicode[i])
            {
                bSuccess = false;
                break;
            }
        TEST_ENSURE(bSuccess, "failure #24");
    }
}

// -----------------------------------------------------------------------------

class enc : public CppUnit::TestFixture
{
public:
    void test_textenc();

    CPPUNIT_TEST_SUITE( enc );
    CPPUNIT_TEST( test_textenc );
    CPPUNIT_TEST_SUITE_END( );
};

// -----------------------------------------------------------------------------

void enc::test_textenc()
{
    // printf("textenc test:\n");

    SingleByteCharSet aSingleByteCharSet[]
        = { { RTL_TEXTENCODING_MS_1250,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x20AC,0xFFFF,0x201A,0xFFFF,0x201E,0x2026,0x2020,0x2021,
                0xFFFF,0x2030,0x0160,0x2039,0x015A,0x0164,0x017D,0x0179,
                0xFFFF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
                0xFFFF,0x2122,0x0161,0x203A,0x015B,0x0165,0x017E,0x017A,
                0x00A0,0x02C7,0x02D8,0x0141,0x00A4,0x0104,0x00A6,0x00A7,
                0x00A8,0x00A9,0x015E,0x00AB,0x00AC,0x00AD,0x00AE,0x017B,
                0x00B0,0x00B1,0x02DB,0x0142,0x00B4,0x00B5,0x00B6,0x00B7,
                0x00B8,0x0105,0x015F,0x00BB,0x013D,0x02DD,0x013E,0x017C,
                0x0154,0x00C1,0x00C2,0x0102,0x00C4,0x0139,0x0106,0x00C7,
                0x010C,0x00C9,0x0118,0x00CB,0x011A,0x00CD,0x00CE,0x010E,
                0x0110,0x0143,0x0147,0x00D3,0x00D4,0x0150,0x00D6,0x00D7,
                0x0158,0x016E,0x00DA,0x0170,0x00DC,0x00DD,0x0162,0x00DF,
                0x0155,0x00E1,0x00E2,0x0103,0x00E4,0x013A,0x0107,0x00E7,
                0x010D,0x00E9,0x0119,0x00EB,0x011B,0x00ED,0x00EE,0x010F,
                0x0111,0x0144,0x0148,0x00F3,0x00F4,0x0151,0x00F6,0x00F7,
                0x0159,0x016F,0x00FA,0x0171,0x00FC,0x00FD,0x0163,0x02D9 } },
            { RTL_TEXTENCODING_MS_1251,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x0402,0x0403,0x201A,0x0453,0x201E,0x2026,0x2020,0x2021,
                0x20AC,0x2030,0x0409,0x2039,0x040A,0x040C,0x040B,0x040F,
                0x0452,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
                0xFFFF,0x2122,0x0459,0x203A,0x045A,0x045C,0x045B,0x045F,
                0x00A0,0x040E,0x045E,0x0408,0x00A4,0x0490,0x00A6,0x00A7,
                0x0401,0x00A9,0x0404,0x00AB,0x00AC,0x00AD,0x00AE,0x0407,
                0x00B0,0x00B1,0x0406,0x0456,0x0491,0x00B5,0x00B6,0x00B7,
                0x0451,0x2116,0x0454,0x00BB,0x0458,0x0405,0x0455,0x0457,
                0x0410,0x0411,0x0412,0x0413,0x0414,0x0415,0x0416,0x0417,
                0x0418,0x0419,0x041A,0x041B,0x041C,0x041D,0x041E,0x041F,
                0x0420,0x0421,0x0422,0x0423,0x0424,0x0425,0x0426,0x0427,
                0x0428,0x0429,0x042A,0x042B,0x042C,0x042D,0x042E,0x042F,
                0x0430,0x0431,0x0432,0x0433,0x0434,0x0435,0x0436,0x0437,
                0x0438,0x0439,0x043A,0x043B,0x043C,0x043D,0x043E,0x043F,
                0x0440,0x0441,0x0442,0x0443,0x0444,0x0445,0x0446,0x0447,
                0x0448,0x0449,0x044A,0x044B,0x044C,0x044D,0x044E,0x044F } },
            { RTL_TEXTENCODING_MS_1252,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x20AC,0xFFFF,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,
                0x02C6,0x2030,0x0160,0x2039,0x0152,0xFFFF,0x017D,0xFFFF,
                0xFFFF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
                0x02DC,0x2122,0x0161,0x203A,0x0153,0xFFFF,0x017E,0x0178,
                0x00A0,0x00A1,0x00A2,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,
                0x00A8,0x00A9,0x00AA,0x00AB,0x00AC,0x00AD,0x00AE,0x00AF,
                0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,
                0x00B8,0x00B9,0x00BA,0x00BB,0x00BC,0x00BD,0x00BE,0x00BF,
                0x00C0,0x00C1,0x00C2,0x00C3,0x00C4,0x00C5,0x00C6,0x00C7,
                0x00C8,0x00C9,0x00CA,0x00CB,0x00CC,0x00CD,0x00CE,0x00CF,
                0x00D0,0x00D1,0x00D2,0x00D3,0x00D4,0x00D5,0x00D6,0x00D7,
                0x00D8,0x00D9,0x00DA,0x00DB,0x00DC,0x00DD,0x00DE,0x00DF,
                0x00E0,0x00E1,0x00E2,0x00E3,0x00E4,0x00E5,0x00E6,0x00E7,
                0x00E8,0x00E9,0x00EA,0x00EB,0x00EC,0x00ED,0x00EE,0x00EF,
                0x00F0,0x00F1,0x00F2,0x00F3,0x00F4,0x00F5,0x00F6,0x00F7,
                0x00F8,0x00F9,0x00FA,0x00FB,0x00FC,0x00FD,0x00FE,0x00FF } },
            { RTL_TEXTENCODING_MS_1253,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x20AC,0xFFFF,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,
                0xFFFF,0x2030,0xFFFF,0x2039,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
                0xFFFF,0x2122,0xFFFF,0x203A,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0x00A0,0x0385,0x0386,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,
                0x00A8,0x00A9,0xFFFF,0x00AB,0x00AC,0x00AD,0x00AE,0x2015,
                0x00B0,0x00B1,0x00B2,0x00B3,0x0384,0x00B5,0x00B6,0x00B7,
                0x0388,0x0389,0x038A,0x00BB,0x038C,0x00BD,0x038E,0x038F,
                0x0390,0x0391,0x0392,0x0393,0x0394,0x0395,0x0396,0x0397,
                0x0398,0x0399,0x039A,0x039B,0x039C,0x039D,0x039E,0x039F,
                0x03A0,0x03A1,0xFFFF,0x03A3,0x03A4,0x03A5,0x03A6,0x03A7,
                0x03A8,0x03A9,0x03AA,0x03AB,0x03AC,0x03AD,0x03AE,0x03AF,
                0x03B0,0x03B1,0x03B2,0x03B3,0x03B4,0x03B5,0x03B6,0x03B7,
                0x03B8,0x03B9,0x03BA,0x03BB,0x03BC,0x03BD,0x03BE,0x03BF,
                0x03C0,0x03C1,0x03C2,0x03C3,0x03C4,0x03C5,0x03C6,0x03C7,
                0x03C8,0x03C9,0x03CA,0x03CB,0x03CC,0x03CD,0x03CE,0xFFFF } },
            { RTL_TEXTENCODING_MS_1254,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x20AC,0xFFFF,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,
                0x02C6,0x2030,0x0160,0x2039,0x0152,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
                0x02DC,0x2122,0x0161,0x203A,0x0153,0xFFFF,0xFFFF,0x0178,
                0x00A0,0x00A1,0x00A2,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,
                0x00A8,0x00A9,0x00AA,0x00AB,0x00AC,0x00AD,0x00AE,0x00AF,
                0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,
                0x00B8,0x00B9,0x00BA,0x00BB,0x00BC,0x00BD,0x00BE,0x00BF,
                0x00C0,0x00C1,0x00C2,0x00C3,0x00C4,0x00C5,0x00C6,0x00C7,
                0x00C8,0x00C9,0x00CA,0x00CB,0x00CC,0x00CD,0x00CE,0x00CF,
                0x011E,0x00D1,0x00D2,0x00D3,0x00D4,0x00D5,0x00D6,0x00D7,
                0x00D8,0x00D9,0x00DA,0x00DB,0x00DC,0x0130,0x015E,0x00DF,
                0x00E0,0x00E1,0x00E2,0x00E3,0x00E4,0x00E5,0x00E6,0x00E7,
                0x00E8,0x00E9,0x00EA,0x00EB,0x00EC,0x00ED,0x00EE,0x00EF,
                0x011F,0x00F1,0x00F2,0x00F3,0x00F4,0x00F5,0x00F6,0x00F7,
                0x00F8,0x00F9,0x00FA,0x00FB,0x00FC,0x0131,0x015F,0x00FF } },
            { RTL_TEXTENCODING_APPLE_ROMAN,
              {   0x00,  0x01,  0x02,  0x03,  0x04,  0x05,  0x06,  0x07,
                  0x08,  0x09,  0x0A,  0x0B,  0x0C,  0x0D,  0x0E,  0x0F,
                  0x10,  0x11,  0x12,  0x13,  0x14,  0x15,  0x16,  0x17,
                  0x18,  0x19,  0x1A,  0x1B,  0x1C,  0x1D,  0x1E,  0x1F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,  0x7F,
                0x00C4,0x00C5,0x00C7,0x00C9,0x00D1,0x00D6,0x00DC,0x00E1,
                0x00E0,0x00E2,0x00E4,0x00E3,0x00E5,0x00E7,0x00E9,0x00E8,
                0x00EA,0x00EB,0x00ED,0x00EC,0x00EE,0x00EF,0x00F1,0x00F3,
                0x00F2,0x00F4,0x00F6,0x00F5,0x00FA,0x00F9,0x00FB,0x00FC,
                0x2020,0x00B0,0x00A2,0x00A3,0x00A7,0x2022,0x00B6,0x00DF,
                0x00AE,0x00A9,0x2122,0x00B4,0x00A8,0x2260,0x00C6,0x00D8,
                0x221E,0x00B1,0x2264,0x2265,0x00A5,0x00B5,0x2202,0x2211,
                0x220F,0x03C0,0x222B,0x00AA,0x00BA,0x03A9,0x00E6,0x00F8,
                0x00BF,0x00A1,0x00AC,0x221A,0x0192,0x2248,0x2206,0x00AB,
                0x00BB,0x2026,0x00A0,0x00C0,0x00C3,0x00D5,0x0152,0x0153,
                0x2013,0x2014,0x201C,0x201D,0x2018,0x2019,0x00F7,0x25CA,
                0x00FF,0x0178,0x2044,0x20AC,0x2039,0x203A,0xFB01,0xFB02,
                0x2021,0x00B7,0x201A,0x201E,0x2030,0x00C2,0x00CA,0x00C1,
                0x00CB,0x00C8,0x00CD,0x00CE,0x00CF,0x00CC,0x00D3,0x00D4,
                0xF8FF,0x00D2,0x00DA,0x00DB,0x00D9,0x0131,0x02C6,0x02DC,
                0x00AF,0x02D8,0x02D9,0x02DA,0x00B8,0x02DD,0x02DB,0x02C7 } },
            { RTL_TEXTENCODING_IBM_437,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x00c7,0x00fc,0x00e9,0x00e2,0x00e4,0x00e0,0x00e5,0x00e7,
                0x00ea,0x00eb,0x00e8,0x00ef,0x00ee,0x00ec,0x00c4,0x00c5,
                0x00c9,0x00e6,0x00c6,0x00f4,0x00f6,0x00f2,0x00fb,0x00f9,
                0x00ff,0x00d6,0x00dc,0x00a2,0x00a3,0x00a5,0x20a7,0x0192,
                0x00e1,0x00ed,0x00f3,0x00fa,0x00f1,0x00d1,0x00aa,0x00ba,
                0x00bf,0x2310,0x00ac,0x00bd,0x00bc,0x00a1,0x00ab,0x00bb,
                0x2591,0x2592,0x2593,0x2502,0x2524,0x2561,0x2562,0x2556,
                0x2555,0x2563,0x2551,0x2557,0x255d,0x255c,0x255b,0x2510,
                0x2514,0x2534,0x252c,0x251c,0x2500,0x253c,0x255e,0x255f,
                0x255a,0x2554,0x2569,0x2566,0x2560,0x2550,0x256c,0x2567,
                0x2568,0x2564,0x2565,0x2559,0x2558,0x2552,0x2553,0x256b,
                0x256a,0x2518,0x250c,0x2588,0x2584,0x258c,0x2590,0x2580,
                0x03b1,0x00df,0x0393,0x03c0,0x03a3,0x03c3,0x00b5,0x03c4,
                0x03a6,0x0398,0x03a9,0x03b4,0x221e,0x03c6,0x03b5,0x2229,
                0x2261,0x00b1,0x2265,0x2264,0x2320,0x2321,0x00f7,0x2248,
                0x00b0,0x2219,0x00b7,0x221a,0x207f,0x00b2,0x25a0,0x00a0 } },
            // ...
            { RTL_TEXTENCODING_ASCII_US,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x20AC,0xFFFF,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021, // !
                0x02C6,0x2030,0x0160,0x2039,0x0152,0xFFFF,0x017D,0xFFFF, // !
                0xFFFF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014, // !
                0x02DC,0x2122,0x0161,0x203A,0x0153,0xFFFF,0x017E,0x0178, // !
                  0xA0,  0xA1,  0xA2,  0xA3,  0xA4,  0xA5,  0xA6,  0xA7,
                  0xA8,  0xA9,  0xAA,  0xAB,  0xAC,  0xAD,  0xAE,  0xAF,
                  0xB0,  0xB1,  0xB2,  0xB3,  0xB4,  0xB5,  0xB6,  0xB7,
                  0xB8,  0xB9,  0xBA,  0xBB,  0xBC,  0xBD,  0xBE,  0xBF,
                  0xC0,  0xC1,  0xC2,  0xC3,  0xC4,  0xC5,  0xC6,  0xC7,
                  0xC8,  0xC9,  0xCA,  0xCB,  0xCC,  0xCD,  0xCE,  0xCF,
                  0xD0,  0xD1,  0xD2,  0xD3,  0xD4,  0xD5,  0xD6,  0xD7,
                  0xD8,  0xD9,  0xDA,  0xDB,  0xDC,  0xDD,  0xDE,  0xDF,
                  0xE0,  0xE1,  0xE2,  0xE3,  0xE4,  0xE5,  0xE6,  0xE7,
                  0xE8,  0xE9,  0xEA,  0xEB,  0xEC,  0xED,  0xEE,  0xEF,
                  0xF0,  0xF1,  0xF2,  0xF3,  0xF4,  0xF5,  0xF6,  0xF7,
                  0xF8,  0xF9,  0xFA,  0xFB,  0xFC,  0xFD,  0xFE,  0xFF } },
            { RTL_TEXTENCODING_ISO_8859_1,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,
                0x0088,0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,
                0x0090,0x0091,0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,
                0x0098,0x0099,0x009A,0x009B,0x009C,0x009D,0x009E,0x009F,
                0x00A0,0x00A1,0x00A2,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,
                0x00A8,0x00A9,0x00AA,0x00AB,0x00AC,0x00AD,0x00AE,0x00AF,
                0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,
                0x00B8,0x00B9,0x00BA,0x00BB,0x00BC,0x00BD,0x00BE,0x00BF,
                0x00C0,0x00C1,0x00C2,0x00C3,0x00C4,0x00C5,0x00C6,0x00C7,
                0x00C8,0x00C9,0x00CA,0x00CB,0x00CC,0x00CD,0x00CE,0x00CF,
                0x00D0,0x00D1,0x00D2,0x00D3,0x00D4,0x00D5,0x00D6,0x00D7,
                0x00D8,0x00D9,0x00DA,0x00DB,0x00DC,0x00DD,0x00DE,0x00DF,
                0x00E0,0x00E1,0x00E2,0x00E3,0x00E4,0x00E5,0x00E6,0x00E7,
                0x00E8,0x00E9,0x00EA,0x00EB,0x00EC,0x00ED,0x00EE,0x00EF,
                0x00F0,0x00F1,0x00F2,0x00F3,0x00F4,0x00F5,0x00F6,0x00F7,
                0x00F8,0x00F9,0x00FA,0x00FB,0x00FC,0x00FD,0x00FE,0x00FF } },
            { RTL_TEXTENCODING_ISO_8859_2,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,
                0x0088,0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,
                0x0090,0x0091,0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,
                0x0098,0x0099,0x009A,0x009B,0x009C,0x009D,0x009E,0x009F,
                0x00A0,0x0104,0x02D8,0x0141,0x00A4,0x013D,0x015A,0x00A7,
                0x00A8,0x0160,0x015E,0x0164,0x0179,0x00AD,0x017D,0x017B,
                0x00B0,0x0105,0x02DB,0x0142,0x00B4,0x013E,0x015B,0x02C7,
                0x00B8,0x0161,0x015F,0x0165,0x017A,0x02DD,0x017E,0x017C,
                0x0154,0x00C1,0x00C2,0x0102,0x00C4,0x0139,0x0106,0x00C7,
                0x010C,0x00C9,0x0118,0x00CB,0x011A,0x00CD,0x00CE,0x010E,
                0x0110,0x0143,0x0147,0x00D3,0x00D4,0x0150,0x00D6,0x00D7,
                0x0158,0x016E,0x00DA,0x0170,0x00DC,0x00DD,0x0162,0x00DF,
                0x0155,0x00E1,0x00E2,0x0103,0x00E4,0x013A,0x0107,0x00E7,
                0x010D,0x00E9,0x0119,0x00EB,0x011B,0x00ED,0x00EE,0x010F,
                0x0111,0x0144,0x0148,0x00F3,0x00F4,0x0151,0x00F6,0x00F7,
                0x0159,0x016F,0x00FA,0x0171,0x00FC,0x00FD,0x0163,0x02D9 } },
            { RTL_TEXTENCODING_ISO_8859_3,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,
                0x0088,0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,
                0x0090,0x0091,0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,
                0x0098,0x0099,0x009A,0x009B,0x009C,0x009D,0x009E,0x009F,
                0x00A0,0x0126,0x02D8,0x00A3,0x00A4,0xFFFF,0x0124,0x00A7,
                0x00A8,0x0130,0x015E,0x011E,0x0134,0x00AD,0xFFFF,0x017B,
                0x00B0,0x0127,0x00B2,0x00B3,0x00B4,0x00B5,0x0125,0x00B7,
                0x00B8,0x0131,0x015F,0x011F,0x0135,0x00BD,0xFFFF,0x017C,
                0x00C0,0x00C1,0x00C2,0xFFFF,0x00C4,0x010A,0x0108,0x00C7,
                0x00C8,0x00C9,0x00CA,0x00CB,0x00CC,0x00CD,0x00CE,0x00CF,
                0xFFFF,0x00D1,0x00D2,0x00D3,0x00D4,0x0120,0x00D6,0x00D7,
                0x011C,0x00D9,0x00DA,0x00DB,0x00DC,0x016C,0x015C,0x00DF,
                0x00E0,0x00E1,0x00E2,0xFFFF,0x00E4,0x010B,0x0109,0x00E7,
                0x00E8,0x00E9,0x00EA,0x00EB,0x00EC,0x00ED,0x00EE,0x00EF,
                0xFFFF,0x00F1,0x00F2,0x00F3,0x00F4,0x0121,0x00F6,0x00F7,
                0x011D,0x00F9,0x00FA,0x00FB,0x00FC,0x016D,0x015D,0x02D9 } },
            // ...
            { RTL_TEXTENCODING_ISO_8859_6,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,
                0x0088,0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,
                0x0090,0x0091,0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,
                0x0098,0x0099,0x009A,0x009B,0x009C,0x009D,0x009E,0x009F,
                0x00A0,0xFFFF,0xFFFF,0xFFFF,0x00A4,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x060C,0x00AD,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0x061B,0xFFFF,0xFFFF,0xFFFF,0x061F,
                0xFFFF,0x0621,0x0622,0x0623,0x0624,0x0625,0x0626,0x0627,
                0x0628,0x0629,0x062A,0x062B,0x062C,0x062D,0x062E,0x062F,
                0x0630,0x0631,0x0632,0x0633,0x0634,0x0635,0x0636,0x0637,
                0x0638,0x0639,0x063A,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0x0640,0x0641,0x0642,0x0643,0x0644,0x0645,0x0646,0x0647,
                0x0648,0x0649,0x064A,0x064B,0x064C,0x064D,0x064E,0x064F,
                0x0650,0x0651,0x0652,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF } },
            // ...
            { RTL_TEXTENCODING_ISO_8859_8,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,
                0x0088,0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,
                0x0090,0x0091,0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,
                0x0098,0x0099,0x009A,0x009B,0x009C,0x009D,0x009E,0x009F,
                0x00A0,0xFFFF,0x00A2,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,
                0x00A8,0x00A9,0x00D7,0x00AB,0x00AC,0x00AD,0x00AE,0x00AF,
                0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,
                0x00B8,0x00B9,0x00F7,0x00BB,0x00BC,0x00BD,0x00BE,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x2017,
                0x05D0,0x05D1,0x05D2,0x05D3,0x05D4,0x05D5,0x05D6,0x05D7,
                0x05D8,0x05D9,0x05DA,0x05DB,0x05DC,0x05DD,0x05DE,0x05DF,
                0x05E0,0x05E1,0x05E2,0x05E3,0x05E4,0x05E5,0x05E6,0x05E7,
                0x05E8,0x05E9,0x05EA,0xFFFF,0xFFFF,0x200E,0x200F,0xFFFF } },
            // ...
            { RTL_TEXTENCODING_TIS_620,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,
                0x0088,0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,
                0x0090,0x0091,0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,
                0x0098,0x0099,0x009A,0x009B,0x009C,0x009D,0x009E,0x009F,
                0x00A0,0x0E01,0x0E02,0x0E03,0x0E04,0x0E05,0x0E06,0x0E07, // !
                0x0E08,0x0E09,0x0E0A,0x0E0B,0x0E0C,0x0E0D,0x0E0E,0x0E0F,
                0x0E10,0x0E11,0x0E12,0x0E13,0x0E14,0x0E15,0x0E16,0x0E17,
                0x0E18,0x0E19,0x0E1A,0x0E1B,0x0E1C,0x0E1D,0x0E1E,0x0E1F,
                0x0E20,0x0E21,0x0E22,0x0E23,0x0E24,0x0E25,0x0E26,0x0E27,
                0x0E28,0x0E29,0x0E2A,0x0E2B,0x0E2C,0x0E2D,0x0E2E,0x0E2F,
                0x0E30,0x0E31,0x0E32,0x0E33,0x0E34,0x0E35,0x0E36,0x0E37,
                0x0E38,0x0E39,0x0E3A,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0E3F,
                0x0E40,0x0E41,0x0E42,0x0E43,0x0E44,0x0E45,0x0E46,0x0E47,
                0x0E48,0x0E49,0x0E4A,0x0E4B,0x0E4C,0x0E4D,0x0E4E,0x0E4F,
                0x0E50,0x0E51,0x0E52,0x0E53,0x0E54,0x0E55,0x0E56,0x0E57,
                0x0E58,0x0E59,0x0E5A,0x0E5B,0xFFFF,0xFFFF,0xFFFF,0xFFFF } },
            { RTL_TEXTENCODING_MS_874,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x20AC,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x2026,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0x00A0,0x0E01,0x0E02,0x0E03,0x0E04,0x0E05,0x0E06,0x0E07,
                0x0E08,0x0E09,0x0E0A,0x0E0B,0x0E0C,0x0E0D,0x0E0E,0x0E0F,
                0x0E10,0x0E11,0x0E12,0x0E13,0x0E14,0x0E15,0x0E16,0x0E17,
                0x0E18,0x0E19,0x0E1A,0x0E1B,0x0E1C,0x0E1D,0x0E1E,0x0E1F,
                0x0E20,0x0E21,0x0E22,0x0E23,0x0E24,0x0E25,0x0E26,0x0E27,
                0x0E28,0x0E29,0x0E2A,0x0E2B,0x0E2C,0x0E2D,0x0E2E,0x0E2F,
                0x0E30,0x0E31,0x0E32,0x0E33,0x0E34,0x0E35,0x0E36,0x0E37,
                0x0E38,0x0E39,0x0E3A,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0E3F,
                0x0E40,0x0E41,0x0E42,0x0E43,0x0E44,0x0E45,0x0E46,0x0E47,
                0x0E48,0x0E49,0x0E4A,0x0E4B,0x0E4C,0x0E4D,0x0E4E,0x0E4F,
                0x0E50,0x0E51,0x0E52,0x0E53,0x0E54,0x0E55,0x0E56,0x0E57,
                0x0E58,0x0E59,0x0E5A,0x0E5B,0xFFFF,0xFFFF,0xFFFF,0xFFFF } },
            { RTL_TEXTENCODING_MS_1255,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x20AC,0xFFFF,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,
                0x02C6,0x2030,0xFFFF,0x2039,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
                0x02DC,0x2122,0xFFFF,0x203A,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0x00A0,0x00A1,0x00A2,0x00A3,0x20AA,0x00A5,0x00A6,0x00A7,
                0x00A8,0x00A9,0x00D7,0x00AB,0x00AC,0x00AD,0x00AE,0x00AF,
                0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,
                0x00B8,0x00B9,0x00F7,0x00BB,0x00BC,0x00BD,0x00BE,0x00BF,
                0x05B0,0x05B1,0x05B2,0x05B3,0x05B4,0x05B5,0x05B6,0x05B7,
                0x05B8,0x05B9,0xFFFF,0x05BB,0x05BC,0x05BD,0x05BE,0x05BF,
                0x05C0,0x05C1,0x05C2,0x05C3,0x05F0,0x05F1,0x05F2,0x05F3,
                0x05F4,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0x05D0,0x05D1,0x05D2,0x05D3,0x05D4,0x05D5,0x05D6,0x05D7,
                0x05D8,0x05D9,0x05DA,0x05DB,0x05DC,0x05DD,0x05DE,0x05DF,
                0x05E0,0x05E1,0x05E2,0x05E3,0x05E4,0x05E5,0x05E6,0x05E7,
                0x05E8,0x05E9,0x05EA,0xFFFF,0xFFFF,0x200E,0x200F,0xFFFF } },
            { RTL_TEXTENCODING_MS_1256,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x20AC,0x067E,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,
                0x02C6,0x2030,0x0679,0x2039,0x0152,0x0686,0x0698,0x0688,
                0x06AF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
                0x06A9,0x2122,0x0691,0x203A,0x0153,0x200C,0x200D,0x06BA,
                0x00A0,0x060C,0x00A2,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,
                0x00A8,0x00A9,0x06BE,0x00AB,0x00AC,0x00AD,0x00AE,0x00AF,
                0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,
                0x00B8,0x00B9,0x061B,0x00BB,0x00BC,0x00BD,0x00BE,0x061F,
                0x06C1,0x0621,0x0622,0x0623,0x0624,0x0625,0x0626,0x0627,
                0x0628,0x0629,0x062A,0x062B,0x062C,0x062D,0x062E,0x062F,
                0x0630,0x0631,0x0632,0x0633,0x0634,0x0635,0x0636,0x00D7,
                0x0637,0x0638,0x0639,0x063A,0x0640,0x0641,0x0642,0x0643,
                0x00E0,0x0644,0x00E2,0x0645,0x0646,0x0647,0x0648,0x00E7,
                0x00E8,0x00E9,0x00EA,0x00EB,0x0649,0x064A,0x00EE,0x00EF,
                0x064B,0x064C,0x064D,0x064E,0x00F4,0x064F,0x0650,0x00F7,
                0x0651,0x00F9,0x0652,0x00FB,0x00FC,0x200E,0x200F,0x06D2 } },
            { RTL_TEXTENCODING_MS_1257,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x20AC,0xFFFF,0x201A,0xFFFF,0x201E,0x2026,0x2020,0x2021,
                0xFFFF,0x2030,0xFFFF,0x2039,0xFFFF,0x00A8,0x02C7,0x00B8,
                0xFFFF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
                0xFFFF,0x2122,0xFFFF,0x203A,0xFFFF,0x00AF,0x02DB,0xFFFF,
                0x00A0,0xFFFF,0x00A2,0x00A3,0x00A4,0xFFFF,0x00A6,0x00A7,
                0x00D8,0x00A9,0x0156,0x00AB,0x00AC,0x00AD,0x00AE,0x00C6,
                0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,
                0x00F8,0x00B9,0x0157,0x00BB,0x00BC,0x00BD,0x00BE,0x00E6,
                0x0104,0x012E,0x0100,0x0106,0x00C4,0x00C5,0x0118,0x0112,
                0x010C,0x00C9,0x0179,0x0116,0x0122,0x0136,0x012A,0x013B,
                0x0160,0x0143,0x0145,0x00D3,0x014C,0x00D5,0x00D6,0x00D7,
                0x0172,0x0141,0x015A,0x016A,0x00DC,0x017B,0x017D,0x00DF,
                0x0105,0x012F,0x0101,0x0107,0x00E4,0x00E5,0x0119,0x0113,
                0x010D,0x00E9,0x017A,0x0117,0x0123,0x0137,0x012B,0x013C,
                0x0161,0x0144,0x0146,0x00F3,0x014D,0x00F5,0x00F6,0x00F7,
                0x0173,0x0142,0x015B,0x016B,0x00FC,0x017C,0x017E,0x02D9 } },
            { RTL_TEXTENCODING_MS_1258,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x20AC,0xFFFF,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,
                0x02C6,0x2030,0xFFFF,0x2039,0x0152,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
                0x02DC,0x2122,0xFFFF,0x203A,0x0153,0xFFFF,0xFFFF,0x0178,
                0x00A0,0x00A1,0x00A2,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,
                0x00A8,0x00A9,0x00AA,0x00AB,0x00AC,0x00AD,0x00AE,0x00AF,
                0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,
                0x00B8,0x00B9,0x00BA,0x00BB,0x00BC,0x00BD,0x00BE,0x00BF,
                0x00C0,0x00C1,0x00C2,0x0102,0x00C4,0x00C5,0x00C6,0x00C7,
                0x00C8,0x00C9,0x00CA,0x00CB,0x0300,0x00CD,0x00CE,0x00CF,
                0x0110,0x00D1,0x0309,0x00D3,0x00D4,0x01A0,0x00D6,0x00D7,
                0x00D8,0x00D9,0x00DA,0x00DB,0x00DC,0x01AF,0x0303,0x00DF,
                0x00E0,0x00E1,0x00E2,0x0103,0x00E4,0x00E5,0x00E6,0x00E7,
                0x00E8,0x00E9,0x00EA,0x00EB,0x0301,0x00ED,0x00EE,0x00EF,
                0x0111,0x00F1,0x0323,0x00F3,0x00F4,0x01A1,0x00F6,0x00F7,
                0x00F8,0x00F9,0x00FA,0x00FB,0x00FC,0x01B0,0x20AB,0x00FF } },
            { RTL_TEXTENCODING_KOI8_U, // RFC 2319
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0x2500,0x2502,0x250C,0x2510,0x2514,0x2518,0x251C,0x2524,
                0x252C,0x2534,0x253C,0x2580,0x2584,0x2588,0x258C,0x2590,
                0x2591,0x2592,0x2593,0x2320,0x25A0,0x2219,0x221A,0x2248,
                0x2264,0x2265,0x00A0,0x2321,0x00B0,0x00B2,0x00B7,0x00F7,
                0x2550,0x2551,0x2552,0x0451,0x0454,0x2554,0x0456,0x0457,
                0x2557,0x2558,0x2559,0x255A,0x255B,0x0491,0x255D,0x255E,
                0x255F,0x2560,0x2561,0x0401,0x0404,0x2563,0x0406,0x0407,
                0x2566,0x2567,0x2568,0x2569,0x256A,0x0490,0x256C,0x00A9,
                0x044E,0x0430,0x0431,0x0446,0x0434,0x0435,0x0444,0x0433,
                0x0445,0x0438,0x0439,0x043A,0x043B,0x043C,0x043D,0x043E,
                0x043F,0x044F,0x0440,0x0441,0x0442,0x0443,0x0436,0x0432,
                0x044C,0x044B,0x0437,0x0448,0x044D,0x0449,0x0447,0x044A,
                0x042E,0x0410,0x0411,0x0426,0x0414,0x0415,0x0424,0x0413,
                0x0425,0x0418,0x0419,0x041A,0x041B,0x041C,0x041D,0x041E,
                0x041F,0x042F,0x0420,0x0421,0x0422,0x0423,0x0416,0x0412,
                0x042C,0x042B,0x0417,0x0428,0x042D,0x0429,0x0427,0x042A } },
            { RTL_TEXTENCODING_ISCII_DEVANAGARI,
              { 0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
                0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
                0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
                0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0x0901,0x0902,0x0903,0x0905,0x0906,0x0907,0x0908,
                0x0909,0x090A,0x090B,0x090E,0x090F,0x0910,0x090D,0x0912,
                0x0913,0x0914,0x0911,0x0915,0x0916,0x0917,0x0918,0x0919,
                0x091A,0x091B,0x091C,0x091D,0x091E,0x091F,0x0920,0x0921,
                0x0922,0x0923,0x0924,0x0925,0x0926,0x0927,0x0928,0x0929,
                0x092A,0x092B,0x092C,0x092D,0x092E,0x092F,0x095F,0x0930,
                0x0931,0x0932,0x0933,0x0934,0x0935,0x0936,0x0937,0x0938,
                0x0939,0xFFFF,0x093E,0x093F,0x0940,0x0941,0x0942,0x0943,
                0x0946,0x0947,0x0948,0x0945,0x094A,0x094B,0x094C,0x0949,
                0x094D,0x093C,0x0964,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0x0966,0x0967,0x0968,0x0969,0x096A,0x096B,0x096C,
                0x096D,0x096E,0x096F,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF } },
            { RTL_TEXTENCODING_ADOBE_STANDARD,
              { 0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x2019,
                0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
                0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
                0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
                0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
                0x2018,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
                0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
                0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
                0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0x00A1,0x00A2,0x00A3,0x2215,0x00A5,0x0192,0x00A7,
                0x00A4,0x0027,0x201C,0x00AB,0x2039,0x203A,0xFB01,0xFB02,
                0xFFFF,0x2013,0x2020,0x2021,0x00B7,0xFFFF,0x00B6,0x2022,
                0x201A,0x201E,0x201D,0x00BB,0x2026,0x2030,0xFFFF,0x00BF,
                0xFFFF,0x0060,0x00B4,0x02C6,0x02DC,0x00AF,0x02D8,0x02D9,
                0x00A8,0xFFFF,0x02DA,0x00B8,0xFFFF,0x02DD,0x02DB,0x02C7,
                0x2014,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0x00C6,0xFFFF,0x00AA,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0x0141,0x00D8,0x0152,0x00BA,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0x00E6,0xFFFF,0xFFFF,0xFFFF,0x0131,0xFFFF,0xFFFF,
                0x0142,0x00F8,0x0153,0x00DF,0xFFFF,0xFFFF,0xFFFF,0xFFFF } },
            { RTL_TEXTENCODING_ADOBE_SYMBOL,
              { 0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0x0020,0x0021,0x2200,0x0023,0x2203,0x0025,0x0026,0x220B,
                0x0028,0x0029,0x2217,0x002B,0x002C,0x2212,0x002E,0x002F,
                0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
                0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
                0x2245,0x0391,0x0392,0x03A7,0x0394,0x0395,0x03A6,0x0393,
                0x0397,0x0399,0x03D1,0x039A,0x039B,0x039C,0x039D,0x039F,
                0x03A0,0x0398,0x03A1,0x03A3,0x03A4,0x03A5,0x03C2,0x03A9,
                0x039E,0x03A8,0x0396,0x005B,0x2234,0x005D,0x22A5,0x005F,
                0xF8E5,0x03B1,0x03B2,0x03C7,0x03B4,0x03B5,0x03C6,0x03B3,
                0x03B7,0x03B9,0x03D5,0x03BA,0x03BB,0x03BC,0x03BD,0x03BF,
                0x03C0,0x03B8,0x03C1,0x03C3,0x03C4,0x03C5,0x03D6,0x03C9,
                0x03BE,0x03C8,0x03B6,0x007B,0x007C,0x007D,0x223C,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
                0x20AC,0x03D2,0x2032,0x2264,0x2215,0x221E,0x0192,0x2663,
                0x2666,0x2665,0x2660,0x2194,0x2190,0x2191,0x2192,0x2193,
                0x00B0,0x00B1,0x2033,0x2265,0x00D7,0x221D,0x2202,0x2022,
                0x00F7,0x2260,0x2261,0x2248,0x2026,0x23AF,0x23D0,0x21B5,
                0x2135,0x2111,0x211C,0x2118,0x2297,0x2295,0x2205,0x2229,
                0x222A,0x2283,0x2287,0x2284,0x2282,0x2286,0x2208,0x2209,
                0x2220,0x2207,0xF6DA,0xF6D9,0xF6DB,0x220F,0x221A,0x22C5,
                0x00AC,0x2227,0x2228,0x21D4,0x21D0,0x21D1,0x21D2,0x21D3,
                0x25CA,0x2329,0xF8E8,0xF8E9,0xF8EA,0x2211,0x239B,0x239C,
                0x239D,0x23A1,0x23A2,0x23A3,0x23A7,0x23A8,0x23A9,0x23AA,
                0xFFFF,0x232A,0x222B,0x2320,0x23AE,0x2321,0x239E,0x239F,
                0x23A0,0x23A4,0x23A5,0x23A6,0x23AB,0x23AC,0x23AD,0xFFFF } } };
    {
        int nCount = sizeof aSingleByteCharSet / sizeof aSingleByteCharSet[0];
        for (int i = 0; i < nCount; ++i)
            testSingleByteCharSet(aSingleByteCharSet[i]);
    }

    ComplexCharSetTest aComplexCharSetTest[]
        = { { RTL_TEXTENCODING_ASCII_US,
              RTL_CONSTASCII_STRINGPARAM("\x01\"3De$~"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x007E },
              7,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_CN,
              RTL_CONSTASCII_STRINGPARAM("\x01\"3De$~\xA1\xB9\xF0\xC5"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x007E,
                0x300D,0x9E4B },
              9,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_TW,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x01\"3De$~\xC5\xF0\x8E\xA4\xDC\xD9"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x007E,
                0x4ED9,0xD87E,0xDD68 },
              10,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_GB_18030,
              RTL_CONSTASCII_STRINGPARAM("\x01\"3De$~"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x007E },
              7,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_GB_18030,
              RTL_CONSTASCII_STRINGPARAM("\x81\x40\xFE\xFE"),
              { 0x4E02,0xE4C5 },
              2,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_GB_18030,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x81\x30\xB1\x33\x81\x30\xD3\x30\x81\x36\xA5\x31"),
              { 0x028A,0x0452,0x200F },
              3,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_GB_18030,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xFE\x50\xFE\x51\xFE\x52\xFE\x53\xFE\x54\xFE\x55\xFE\x56"
                  "\xFE\x57\xFE\x58\xFE\x59\xFE\x5A\xFE\x5B\xFE\x5C\xFE\x5D"
                  "\xFE\x5E\xFE\x5F\xFE\x60\xFE\x61\xFE\x62\xFE\x63\xFE\x64"
                  "\xFE\x65\xFE\x66\xFE\x67\xFE\x68\xFE\x69\xFE\x6A\xFE\x6B"
                  "\xFE\x6C\xFE\x6D\xFE\x6E\xFE\x6F\xFE\x70\xFE\x71\xFE\x72"
                  "\xFE\x73\xFE\x74\xFE\x75\xFE\x76\xFE\x77\xFE\x78\xFE\x79"
                  "\xFE\x7A\xFE\x7B\xFE\x7C\xFE\x7D\xFE\x7E\xFE\x80\xFE\x81"
                  "\xFE\x82\xFE\x83\xFE\x84\xFE\x85\xFE\x86\xFE\x87\xFE\x88"
                  "\xFE\x89\xFE\x8A\xFE\x8B\xFE\x8C\xFE\x8D\xFE\x8E\xFE\x8F"
                  "\xFE\x90\xFE\x91\xFE\x92\xFE\x93\xFE\x94\xFE\x95\xFE\x96"
                  "\xFE\x97\xFE\x98\xFE\x99\xFE\x9A\xFE\x9B\xFE\x9C\xFE\x9D"
                  "\xFE\x9E\xFE\x9F\xFE\xA0"),
              { 0x2E81,0xE816,0xE817,0xE818,0x2E84,0x3473,0x3447,0x2E88,
                0x2E8B,0xE81E,0x359E,0x361A,0x360E,0x2E8C,0x2E97,0x396E,
                0x3918,0xE826,0x39CF,0x39DF,0x3A73,0x39D0,0xE82B,0xE82C,
                0x3B4E,0x3C6E,0x3CE0,0x2EA7,0xE831,0xE832,0x2EAA,0x4056,
                0x415F,0x2EAE,0x4337,0x2EB3,0x2EB6,0x2EB7,0xE83B,0x43B1,
                0x43AC,0x2EBB,0x43DD,0x44D6,0x4661,0x464C,0xE843,0x4723,
                0x4729,0x477C,0x478D,0x2ECA,0x4947,0x497A,0x497D,0x4982,
                0x4983,0x4985,0x4986,0x499F,0x499B,0x49B7,0x49B6,0xE854,
                0xE855,0x4CA3,0x4C9F,0x4CA0,0x4CA1,0x4C77,0x4CA2,0x4D13,
                0x4D14,0x4D15,0x4D16,0x4D17,0x4D18,0x4D19,0x4DAE,0xE864 },
              80,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_2022_JP,
              RTL_CONSTASCII_STRINGPARAM("\x01\"3De$\\~"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x005C,0x007E },
              8,
              false,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_2022_JP,
              RTL_CONSTASCII_STRINGPARAM("\x1B(B\x01\"3De$\\~"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x005C,0x007E },
              8,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_2022_JP,
              RTL_CONSTASCII_STRINGPARAM("\x1B(J\x01\"3De$\\~"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x00A5,0x00AF },
              8,
              false,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_2022_JP,
              RTL_CONSTASCII_STRINGPARAM("\x1B$B\x26\x21\x27\x71\x1B(B"),
              { 0x0391,0x044F },
              2,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_2022_KR,
              RTL_CONSTASCII_STRINGPARAM("\x1B$)C\x01\"3De$\\~"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x005C,0x007E },
              8,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_2022_KR,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x1B$)C\x0E\x25\x21\x0F\x0D\x0Ax\x0E\x48\x7E\x0F"),
              { 0x2170,0x000D,0x000A,0x0078,0xD79D },
              5,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_2022_CN,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x01\"3De$\\~\x1B$)G\x0E\x45\x70\x1B$*H\x1BN\x22\x22"
                      "\x45\x70\x0F\x1B$)A\x0E\x26\x21\x0F"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x005C,0x007E,
                0x4ED9,0x531F,0x4ED9,0x0391 },
              12,
              false,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_2022_CN,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x01\"3De$\\~\x1B$)A\x0E\x26\x21\x1B$*H\x1BN\x22\x22"
                      "\x26\x21\x0F\x0D\x0A\x1B$)A\x0E\x26\x21\x0F"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x005C,0x007E,
                0x0391,0x531F,0x0391,0x000D,0x000A,0x0391 },
              14,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            // The following does not work as long as Big5-HKSCS maps to
            // Unicode PUA instead of Plane 2.  Use the next two tests
            // instead:
//          { RTL_TEXTENCODING_BIG5_HKSCS,
//            RTL_CONSTASCII_STRINGPARAM(
//                "\x01\"3De$~\x88\x56\xF9\xFE\xFA\x5E\xA1\x40\xF9\xD5"),
//            { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x007E,0x0100,
//              0xFFED,0xD849,0xDD13,0x3000,0x9F98 },
//            13,
//            true,
//            true,
//            true,
//            false,
//            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5_HKSCS,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x01\"3De$~\x88\x56\xF9\xFE\xFA\x5E\xA1\x40\xF9\xD5"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x007E,0x0100,
                0xFFED,0xE01E,0x3000,0x9F98 },
              12,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5_HKSCS,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x01\"3De$~\x88\x56\xF9\xFE\xFA\x5E\xA1\x40\xF9\xD5"),
              { 0x0001,0x0022,0x0033,0x0044,0x0065,0x0024,0x007E,0x0100,
                0xFFED,0xD849,0xDD13,0x3000,0x9F98 },
              13,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5_HKSCS,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xC6\xA1\xC6\xCF\xC6\xD3\xC6\xD5\xC6\xD7\xC6\xDE\xC6\xDF"
                  "\xC6\xFE\xC7\x40\xC7\x7E\xC7\xA1\xC7\xFE"),
              { 0x2460,0xF6E0,0xF6E4,0xF6E6,0xF6E8,0xF6EF,0xF6F0,0x3058,
                0x3059,0x30A4,0x30A5,0x041A },
              12,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5_HKSCS,
              RTL_CONSTASCII_STRINGPARAM("\x81\x40\x84\xFE"),
              { 0xEEB8,0xF12B },
              2,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5_HKSCS,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x81\x40\x8D\xFE\x8E\x40\xA0\xFE\xC6\xA1\xC8\xFE\xFA\x40"
                  "\xFE\xFE"),
              { 0xEEB8,0xF6B0,0xE311,0xEEB7,0xF6B1,0xF848,0xE000,0xE310 },
              8,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5_HKSCS,
              RTL_CONSTASCII_STRINGPARAM("\xAD\xC5\x94\x55"),
              { 0x5029,0x7250 },
              2,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5_HKSCS,
              RTL_CONSTASCII_STRINGPARAM("\xFA\x5F\xA0\xE4"),
              { 0x5029,0x7250 },
              2,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5_HKSCS,
              RTL_CONSTASCII_STRINGPARAM("\xA0\x40\xA0\x7E\xA0\xA1\xA0\xFE"),
              { 0xEE1B,0xEE59,0xEE5A,0xEEB7 },
              4,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5,
              RTL_CONSTASCII_STRINGPARAM("\xA1\x45"),
              { 0x2027 },
              1,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xC6\xCF\xC6\xD3\xC6\xD5\xC6\xD7\xC6\xDE\xC6\xDF"),
              { 0x306B,0x306F,0x3071,0x3073,0x307A,0x307B },
              6,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xC7\xFD\xC7\xFE\xC8\x40\xC8\x7E\xC8\xA1\xC8\xFE"),
              { 0xF7AA,0xF7AB,0xF7AC,0xF7EA,0xF7EB,0xF848 },
              6,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5,
              RTL_CONSTASCII_STRINGPARAM("\xA0\x40\xA0\x7E\xA0\xA1\xA0\xFE"),
              { 0xEE1B,0xEE59,0xEE5A,0xEEB7 },
              4,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_MS_950,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xC6\xA1\xC6\xFE\xC7\x40\xC7\x7E\xC7\xA1\xC7\xFE\xC8\x40"
                  "\xC8\x7E\xC8\xA1\xC8\xFE"),
              { 0xF6B1,0xF70E,0xF70F,0xF74D,0xF74E,0xF7AB,0xF7AC,0xF7EA,
                0xF7EB,0xF848 },
              10,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_MS_950,
              RTL_CONSTASCII_STRINGPARAM("\xA0\x40\xA0\x7E\xA0\xA1\xA0\xFE"),
              { 0xEE1B,0xEE59,0xEE5A,0xEEB7 },
              4,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },

            // Test Unicode beyond BMP:

            // FIXME The second m_bForward test (requiring a context) does not
            // work for UTF7:
//          { RTL_TEXTENCODING_UTF7,
//            RTL_CONSTASCII_STRINGPARAM("+2EndEw-"),
//            { 0xD849,0xDD13 },
//            2,
//            true,
//            true,
//            true,
//            false,
//            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_UTF8,
              RTL_CONSTASCII_STRINGPARAM("\xF0\xA2\x94\x93"),
              { 0xD849,0xDD13 },
              2,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_GB_18030,
              RTL_CONSTASCII_STRINGPARAM("\x95\x39\xC5\x37"),
              { 0xD849,0xDD13 },
              2,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_BIG5_HKSCS,
              RTL_CONSTASCII_STRINGPARAM("\xFA\x5E"),
              { 0xD849,0xDD13 },
              2,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },

            // Test GBK (aka CP936):

            { RTL_TEXTENCODING_GBK,
              RTL_CONSTASCII_STRINGPARAM("\xFD\x7C\xC1\xFA\xFD\x9B"),
              { 0x9F76,0x9F99,0x9FA5 },
              3,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_MS_936,
              RTL_CONSTASCII_STRINGPARAM("\xFD\x7C\xC1\xFA\xFD\x9B"),
              { 0x9F76,0x9F99,0x9FA5 },
              3,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_GBK,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xFE\x50\xFE\x54\xFE\x55\xFE\x56"
                  "\xFE\x57\xFE\x58\xFE\x5A\xFE\x5B\xFE\x5C\xFE\x5D"
                  "\xFE\x5E\xFE\x5F\xFE\x60\xFE\x62\xFE\x63\xFE\x64"
                  "\xFE\x65\xFE\x68\xFE\x69\xFE\x6A\xFE\x6B"
                  "\xFE\x6E\xFE\x6F\xFE\x70\xFE\x71\xFE\x72"
                  "\xFE\x73\xFE\x74\xFE\x75\xFE\x77\xFE\x78\xFE\x79"
                  "\xFE\x7A\xFE\x7B\xFE\x7C\xFE\x7D\xFE\x80\xFE\x81"
                  "\xFE\x82\xFE\x83\xFE\x84\xFE\x85\xFE\x86\xFE\x87\xFE\x88"
                  "\xFE\x89\xFE\x8A\xFE\x8B\xFE\x8C\xFE\x8D\xFE\x8E\xFE\x8F"
                  "\xFE\x92\xFE\x93\xFE\x94\xFE\x95\xFE\x96"
                  "\xFE\x97\xFE\x98\xFE\x99\xFE\x9A\xFE\x9B\xFE\x9C\xFE\x9D"
                  "\xFE\x9E\xFE\x9F"),
              { 0x2E81,0x2E84,0x3473,0x3447,0x2E88,0x2E8B,0x359E,0x361A,
                0x360E,0x2E8C,0x2E97,0x396E,0x3918,0x39CF,0x39DF,0x3A73,
                0x39D0,0x3B4E,0x3C6E,0x3CE0,0x2EA7,0x2EAA,0x4056,0x415F,
                0x2EAE,0x4337,0x2EB3,0x2EB6,0x2EB7,0x43B1,0x43AC,0x2EBB,
                0x43DD,0x44D6,0x4661,0x464C,0x4723,0x4729,0x477C,0x478D,
                0x2ECA,0x4947,0x497A,0x497D,0x4982,0x4983,0x4985,0x4986,
                0x499F,0x499B,0x49B7,0x49B6,0x4CA3,0x4C9F,0x4CA0,0x4CA1,
                0x4C77,0x4CA2,0x4D13,0x4D14,0x4D15,0x4D16,0x4D17,0x4D18,
                0x4D19,0x4DAE },
              66,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_JP,
              RTL_CONSTASCII_STRINGPARAM("?"),
              { 0xFF0D },
              1,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK },

            // Test of "JIS X 0208 row 13" (taken from CP932; added to
            // ISO-2022-JP and EUC-JP; 74 of the 83 characters introduce
            // mappings to new Unicode characters):
            { RTL_TEXTENCODING_MS_932,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x87\x40\x87\x41\x87\x42\x87\x43\x87\x44\x87\x45\x87\x46"
                  "\x87\x47\x87\x48\x87\x49\x87\x4A\x87\x4B\x87\x4C\x87\x4D"
                  "\x87\x4E\x87\x4F\x87\x50\x87\x51\x87\x52\x87\x53\x87\x54"
                  "\x87\x55\x87\x56\x87\x57\x87\x58\x87\x59\x87\x5A\x87\x5B"
                  "\x87\x5C\x87\x5D\x87\x5F\x87\x60\x87\x61\x87\x62\x87\x63"
                  "\x87\x64\x87\x65\x87\x66\x87\x67\x87\x68\x87\x69\x87\x6A"
                  "\x87\x6B\x87\x6C\x87\x6D\x87\x6E\x87\x6F\x87\x70\x87\x71"
                  "\x87\x72\x87\x73\x87\x74\x87\x75\x87\x7E\x87\x80\x87\x81"
                  "\x87\x82\x87\x83\x87\x84\x87\x85\x87\x86\x87\x87\x87\x88"
                  "\x87\x89\x87\x8A\x87\x8B\x87\x8C\x87\x8D\x87\x8E\x87\x8F"
                  "\x87\x90\x87\x91\x87\x92\x87\x93\x87\x94\x87\x95\x87\x96"
                  "\x87\x97\x87\x98\x87\x99\x87\x9A\x87\x9B\x87\x9C"),
              { 0x2460,0x2461,0x2462,0x2463,0x2464,0x2465,0x2466,0x2467,0x2468,
                0x2469,0x246A,0x246B,0x246C,0x246D,0x246E,0x246F,0x2470,0x2471,
                0x2472,0x2473,0x2160,0x2161,0x2162,0x2163,0x2164,0x2165,0x2166,
                0x2167,0x2168,0x2169,0x3349,0x3314,0x3322,0x334D,0x3318,0x3327,
                0x3303,0x3336,0x3351,0x3357,0x330D,0x3326,0x3323,0x332B,0x334A,
                0x333B,0x339C,0x339D,0x339E,0x338E,0x338F,0x33C4,0x33A1,0x337B,
                0x301D,0x301F,0x2116,0x33CD,0x2121,0x32A4,0x32A5,0x32A6,0x32A7,
                0x32A8,0x3231,0x3232,0x3239,0x337E,0x337D,0x337C,0x2252,0x2261,
                0x222B,0x222E,0x2211,0x221A,0x22A5,0x2220,0x221F,0x22BF,0x2235,
                0x2229,0x222A },
              83,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_SHIFT_JIS,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x87\x40\x87\x41\x87\x42\x87\x43\x87\x44\x87\x45\x87\x46"
                  "\x87\x47\x87\x48\x87\x49\x87\x4A\x87\x4B\x87\x4C\x87\x4D"
                  "\x87\x4E\x87\x4F\x87\x50\x87\x51\x87\x52\x87\x53\x87\x54"
                  "\x87\x55\x87\x56\x87\x57\x87\x58\x87\x59\x87\x5A\x87\x5B"
                  "\x87\x5C\x87\x5D\x87\x5F\x87\x60\x87\x61\x87\x62\x87\x63"
                  "\x87\x64\x87\x65\x87\x66\x87\x67\x87\x68\x87\x69\x87\x6A"
                  "\x87\x6B\x87\x6C\x87\x6D\x87\x6E\x87\x6F\x87\x70\x87\x71"
                  "\x87\x72\x87\x73\x87\x74\x87\x75\x87\x7E\x87\x80\x87\x81"
                  "\x87\x82\x87\x83\x87\x84\x87\x85\x87\x86\x87\x87\x87\x88"
                  "\x87\x89\x87\x8A\x87\x8B\x87\x8C\x87\x8D\x87\x8E\x87\x8F"
                  "\x87\x90\x87\x91\x87\x92\x87\x93\x87\x94\x87\x95\x87\x96"
                  "\x87\x97\x87\x98\x87\x99\x87\x9A\x87\x9B\x87\x9C"),
              { 0x2460,0x2461,0x2462,0x2463,0x2464,0x2465,0x2466,0x2467,0x2468,
                0x2469,0x246A,0x246B,0x246C,0x246D,0x246E,0x246F,0x2470,0x2471,
                0x2472,0x2473,0x2160,0x2161,0x2162,0x2163,0x2164,0x2165,0x2166,
                0x2167,0x2168,0x2169,0x3349,0x3314,0x3322,0x334D,0x3318,0x3327,
                0x3303,0x3336,0x3351,0x3357,0x330D,0x3326,0x3323,0x332B,0x334A,
                0x333B,0x339C,0x339D,0x339E,0x338E,0x338F,0x33C4,0x33A1,0x337B,
                0x301D,0x301F,0x2116,0x33CD,0x2121,0x32A4,0x32A5,0x32A6,0x32A7,
                0x32A8,0x3231,0x3232,0x3239,0x337E,0x337D,0x337C,0x2252,0x2261,
                0x222B,0x222E,0x2211,0x221A,0x22A5,0x2220,0x221F,0x22BF,0x2235,
                0x2229,0x222A },
              83,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_2022_JP,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x1B$B\x2D\x21\x2D\x22\x2D\x23\x2D\x24\x2D\x25\x2D\x26"
                  "\x2D\x27\x2D\x28\x2D\x29\x2D\x2A\x2D\x2B\x2D\x2C\x2D\x2D"
                  "\x2D\x2E\x2D\x2F\x2D\x30\x2D\x31\x2D\x32\x2D\x33\x2D\x34"
                  "\x2D\x35\x2D\x36\x2D\x37\x2D\x38\x2D\x39\x2D\x3A\x2D\x3B"
                  "\x2D\x3C\x2D\x3D\x2D\x3E\x2D\x40\x2D\x41\x2D\x42\x2D\x43"
                  "\x2D\x44\x2D\x45\x2D\x46\x2D\x47\x2D\x48\x2D\x49\x2D\x4A"
                  "\x2D\x4B\x2D\x4C\x2D\x4D\x2D\x4E\x2D\x4F\x2D\x50\x2D\x51"
                  "\x2D\x52\x2D\x53\x2D\x54\x2D\x55\x2D\x56\x2D\x5F\x2D\x60"
                  "\x2D\x61\x2D\x62\x2D\x63\x2D\x64\x2D\x65\x2D\x66\x2D\x67"
                  "\x2D\x68\x2D\x69\x2D\x6A\x2D\x6B\x2D\x6C\x2D\x6D\x2D\x6E"
                  "\x2D\x6F\x2D\x70\x2D\x71\x2D\x72\x2D\x73\x2D\x74\x2D\x75"
                  "\x2D\x76\x2D\x77\x2D\x78\x2D\x79\x2D\x7A\x2D\x7B\x2D\x7C"
                  "\x1B(B"),
              { 0x2460,0x2461,0x2462,0x2463,0x2464,0x2465,0x2466,0x2467,0x2468,
                0x2469,0x246A,0x246B,0x246C,0x246D,0x246E,0x246F,0x2470,0x2471,
                0x2472,0x2473,0x2160,0x2161,0x2162,0x2163,0x2164,0x2165,0x2166,
                0x2167,0x2168,0x2169,0x3349,0x3314,0x3322,0x334D,0x3318,0x3327,
                0x3303,0x3336,0x3351,0x3357,0x330D,0x3326,0x3323,0x332B,0x334A,
                0x333B,0x339C,0x339D,0x339E,0x338E,0x338F,0x33C4,0x33A1,0x337B,
                0x301D,0x301F,0x2116,0x33CD,0x2121,0x32A4,0x32A5,0x32A6,0x32A7,
                0x32A8,0x3231,0x3232,0x3239,0x337E,0x337D,0x337C,0x2252,0x2261,
                0x222B,0x222E,0x2211,0x221A,0x22A5,0x2220,0x221F,0x22BF,0x2235,
                0x2229,0x222A },
              83,
              false,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_2022_JP,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x1B$B\x2D\x21\x2D\x22\x2D\x23\x2D\x24\x2D\x25\x2D\x26"
                  "\x2D\x27\x2D\x28\x2D\x29\x2D\x2A\x2D\x2B\x2D\x2C\x2D\x2D"
                  "\x2D\x2E\x2D\x2F\x2D\x30\x2D\x31\x2D\x32\x2D\x33\x2D\x34"
                  "\x2D\x35\x2D\x36\x2D\x37\x2D\x38\x2D\x39\x2D\x3A\x2D\x3B"
                  "\x2D\x3C\x2D\x3D\x2D\x3E\x2D\x40\x2D\x41\x2D\x42\x2D\x43"
                  "\x2D\x44\x2D\x45\x2D\x46\x2D\x47\x2D\x48\x2D\x49\x2D\x4A"
                  "\x2D\x4B\x2D\x4C\x2D\x4D\x2D\x4E\x2D\x4F\x2D\x50\x2D\x51"
                  "\x2D\x52\x2D\x53\x2D\x54\x2D\x55\x2D\x56\x2D\x5F\x2D\x60"
                  "\x2D\x61\x2D\x62\x2D\x63\x2D\x64\x2D\x65\x2D\x66\x2D\x67"
                  "\x2D\x68\x2D\x69\x2D\x6A\x2D\x6B\x2D\x6C\x2D\x6D\x2D\x6E"
                  "\x2D\x6F\x2D\x73\x2D\x74\x2D\x78\x2D\x79\x1B(B"),
              { 0x2460,0x2461,0x2462,0x2463,0x2464,0x2465,0x2466,0x2467,0x2468,
                0x2469,0x246A,0x246B,0x246C,0x246D,0x246E,0x246F,0x2470,0x2471,
                0x2472,0x2473,0x2160,0x2161,0x2162,0x2163,0x2164,0x2165,0x2166,
                0x2167,0x2168,0x2169,0x3349,0x3314,0x3322,0x334D,0x3318,0x3327,
                0x3303,0x3336,0x3351,0x3357,0x330D,0x3326,0x3323,0x332B,0x334A,
                0x333B,0x339C,0x339D,0x339E,0x338E,0x338F,0x33C4,0x33A1,0x337B,
                0x301D,0x301F,0x2116,0x33CD,0x2121,0x32A4,0x32A5,0x32A6,0x32A7,
                0x32A8,0x3231,0x3232,0x3239,0x337E,0x337D,0x337C,0x222E,0x2211,
                0x221F,0x22BF },
              74,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_JP,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xAD\xA1\xAD\xA2\xAD\xA3\xAD\xA4\xAD\xA5\xAD\xA6\xAD\xA7"
                  "\xAD\xA8\xAD\xA9\xAD\xAA\xAD\xAB\xAD\xAC\xAD\xAD\xAD\xAE"
                  "\xAD\xAF\xAD\xB0\xAD\xB1\xAD\xB2\xAD\xB3\xAD\xB4\xAD\xB5"
                  "\xAD\xB6\xAD\xB7\xAD\xB8\xAD\xB9\xAD\xBA\xAD\xBB\xAD\xBC"
                  "\xAD\xBD\xAD\xBE\xAD\xC0\xAD\xC1\xAD\xC2\xAD\xC3\xAD\xC4"
                  "\xAD\xC5\xAD\xC6\xAD\xC7\xAD\xC8\xAD\xC9\xAD\xCA\xAD\xCB"
                  "\xAD\xCC\xAD\xCD\xAD\xCE\xAD\xCF\xAD\xD0\xAD\xD1\xAD\xD2"
                  "\xAD\xD3\xAD\xD4\xAD\xD5\xAD\xD6\xAD\xDF\xAD\xE0\xAD\xE1"
                  "\xAD\xE2\xAD\xE3\xAD\xE4\xAD\xE5\xAD\xE6\xAD\xE7\xAD\xE8"
                  "\xAD\xE9\xAD\xEA\xAD\xEB\xAD\xEC\xAD\xED\xAD\xEE\xAD\xEF"
                  "\xAD\xF0\xAD\xF1\xAD\xF2\xAD\xF3\xAD\xF4\xAD\xF5\xAD\xF6"
                  "\xAD\xF7\xAD\xF8\xAD\xF9\xAD\xFA\xAD\xFB\xAD\xFC"),
              { 0x2460,0x2461,0x2462,0x2463,0x2464,0x2465,0x2466,0x2467,0x2468,
                0x2469,0x246A,0x246B,0x246C,0x246D,0x246E,0x246F,0x2470,0x2471,
                0x2472,0x2473,0x2160,0x2161,0x2162,0x2163,0x2164,0x2165,0x2166,
                0x2167,0x2168,0x2169,0x3349,0x3314,0x3322,0x334D,0x3318,0x3327,
                0x3303,0x3336,0x3351,0x3357,0x330D,0x3326,0x3323,0x332B,0x334A,
                0x333B,0x339C,0x339D,0x339E,0x338E,0x338F,0x33C4,0x33A1,0x337B,
                0x301D,0x301F,0x2116,0x33CD,0x2121,0x32A4,0x32A5,0x32A6,0x32A7,
                0x32A8,0x3231,0x3232,0x3239,0x337E,0x337D,0x337C,0x2252,0x2261,
                0x222B,0x222E,0x2211,0x221A,0x22A5,0x2220,0x221F,0x22BF,0x2235,
                0x2229,0x222A },
              83,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_JP,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xAD\xA1\xAD\xA2\xAD\xA3\xAD\xA4\xAD\xA5\xAD\xA6\xAD\xA7"
                  "\xAD\xA8\xAD\xA9\xAD\xAA\xAD\xAB\xAD\xAC\xAD\xAD\xAD\xAE"
                  "\xAD\xAF\xAD\xB0\xAD\xB1\xAD\xB2\xAD\xB3\xAD\xB4\xAD\xB5"
                  "\xAD\xB6\xAD\xB7\xAD\xB8\xAD\xB9\xAD\xBA\xAD\xBB\xAD\xBC"
                  "\xAD\xBD\xAD\xBE\xAD\xC0\xAD\xC1\xAD\xC2\xAD\xC3\xAD\xC4"
                  "\xAD\xC5\xAD\xC6\xAD\xC7\xAD\xC8\xAD\xC9\xAD\xCA\xAD\xCB"
                  "\xAD\xCC\xAD\xCD\xAD\xCE\xAD\xCF\xAD\xD0\xAD\xD1\xAD\xD2"
                  "\xAD\xD3\xAD\xD4\xAD\xD5\xAD\xD6\xAD\xDF\xAD\xE0\xAD\xE1"
                  "\xAD\xE2\xAD\xE3\xAD\xE4\xAD\xE5\xAD\xE6\xAD\xE7\xAD\xE8"
                  "\xAD\xE9\xAD\xEA\xAD\xEB\xAD\xEC\xAD\xED\xAD\xEE\xAD\xEF"
                  "\xAD\xF3\xAD\xF4\xAD\xF8\xAD\xF9"),
              { 0x2460,0x2461,0x2462,0x2463,0x2464,0x2465,0x2466,0x2467,0x2468,
                0x2469,0x246A,0x246B,0x246C,0x246D,0x246E,0x246F,0x2470,0x2471,
                0x2472,0x2473,0x2160,0x2161,0x2162,0x2163,0x2164,0x2165,0x2166,
                0x2167,0x2168,0x2169,0x3349,0x3314,0x3322,0x334D,0x3318,0x3327,
                0x3303,0x3336,0x3351,0x3357,0x330D,0x3326,0x3323,0x332B,0x334A,
                0x333B,0x339C,0x339D,0x339E,0x338E,0x338F,0x33C4,0x33A1,0x337B,
                0x301D,0x301F,0x2116,0x33CD,0x2121,0x32A4,0x32A5,0x32A6,0x32A7,
                0x32A8,0x3231,0x3232,0x3239,0x337E,0x337D,0x337C,0x222E,0x2211,
                0x221F,0x22BF },
              74,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },

            { RTL_TEXTENCODING_EUC_JP,
              RTL_CONSTASCII_STRINGPARAM("\xB9\xF5"),
              { 0x9ED2 },
              1,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },

            // Test ISO-8859-x/MS-125x range 0x80--9F:

            { RTL_TEXTENCODING_ISO_8859_1,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E"
                  "\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                  "\x9E\x9F"),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_8859_2,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E"
                  "\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                  "\x9E\x9F"),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_8859_3,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E"
                  "\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                  "\x9E\x9F"),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_8859_4,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E"
                  "\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                  "\x9E\x9F"),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_8859_5,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E"
                  "\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                  "\x9E\x9F"),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_8859_6,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E"
                  "\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                  "\x9E\x9F"),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_8859_7,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E"
                  "\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                  "\x9E\x9F"),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_8859_8,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E"
                  "\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                  "\x9E\x9F"),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_8859_9,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E"
                  "\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                  "\x9E\x9F"),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_8859_14,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E"
                  "\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                  "\x9E\x9F"),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ISO_8859_15,
              RTL_CONSTASCII_STRINGPARAM(
                  "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E"
                  "\x8F\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D"
                  "\x9E\x9F"),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_MS_874,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_MS_1250,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_MS_1251,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_MS_1252,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_MS_1253,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_MS_1254,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_MS_1255,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_MS_1256,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_MS_1257,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_MS_1258,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x0080,0x0081,0x0082,0x0083,0x0084,0x0085,0x0086,0x0087,0x0088,
                0x0089,0x008A,0x008B,0x008C,0x008D,0x008E,0x008F,0x0090,0x0091,
                0x0092,0x0093,0x0094,0x0095,0x0096,0x0097,0x0098,0x0099,0x009A,
                0x009B,0x009C,0x009D,0x009E,0x009F },
              32,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_MS_949,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xB0\xA1\xB0\xA2\x81\x41\x81\x42\xB0\xA3\x81\x43\x81\x44"
                  "\xB0\xA4\xB0\xA5\xB0\xA6\xB0\xA7\x81\x45\x81\x46\x81\x47"
                  "\x81\x48\x81\x49\xB0\xA8\xB0\xA9\xB0\xAA\xB0\xAB\xB0\xAC"
                  "\xB0\xAD\xB0\xAE\xB0\xAF\x81\x4A\xB0\xB0\xB0\xB1\xB0\xB2"),
              { 0xAC00,0xAC01,0xAC02,0xAC03,0xAC04,0xAC05,0xAC06,0xAC07,0xAC08,
                0xAC09,0xAC0A,0xAC0B,0xAC0C,0xAC0D,0xAC0E,0xAC0F,0xAC10,0xAC11,
                0xAC12,0xAC13,0xAC14,0xAC15,0xAC16,0xAC17,0xAC18,0xAC19,0xAC1A,
                0xAC1B },
              28,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_MS_949,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xC9\xA1\xC9\xA2\xC9\xA3\xC9\xFC\xC9\xFD\xC9\xFE"
                  "\xFE\xA1\xFE\xA2\xFE\xA3\xFE\xFC\xFE\xFD\xFE\xFE"),
              { 0xE000,0xE001,0xE002,0xE05B,0xE05C,0xE05D,
                0xE05E,0xE05F,0xE060,0xE0B9,0xE0BA,0xE0BB },
              12,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_KR,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xB0\xA1\xB0\xA2"              "\xB0\xA3"
                  "\xB0\xA4\xB0\xA5\xB0\xA6\xB0\xA7"
                                  "\xB0\xA8\xB0\xA9\xB0\xAA\xB0\xAB\xB0\xAC"
                  "\xB0\xAD\xB0\xAE\xB0\xAF"      "\xB0\xB0\xB0\xB1\xB0\xB2"),
              { 0xAC00,0xAC01,              0xAC04,              0xAC07,0xAC08,
                0xAC09,0xAC0A,                                   0xAC10,0xAC11,
                0xAC12,0xAC13,0xAC14,0xAC15,0xAC16,0xAC17,       0xAC19,0xAC1A,
                0xAC1B },
              18,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_KR,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xB0\xA1\xB0\xA2"              "\xB0\xA3"
                  "\xB0\xA4\xB0\xA5\xB0\xA6\xB0\xA7"
                                  "\xB0\xA8\xB0\xA9\xB0\xAA\xB0\xAB\xB0\xAC"
                  "\xB0\xAD\xB0\xAE\xB0\xAF"      "\xB0\xB0\xB0\xB1\xB0\xB2"),
              { 0xAC00,0xAC01,0xAC02,0xAC03,0xAC04,0xAC05,0xAC06,0xAC07,0xAC08,
                0xAC09,0xAC0A,0xAC0B,0xAC0C,0xAC0D,0xAC0E,0xAC0F,0xAC10,0xAC11,
                0xAC12,0xAC13,0xAC14,0xAC15,0xAC16,0xAC17,0xAC18,0xAC19,0xAC1A,
                0xAC1B },
              28,
              true,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_EUC_KR,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xC9\xA1\xC9\xA2\xC9\xA3\xC9\xFC\xC9\xFD\xC9\xFE"
                  "\xFE\xA1\xFE\xA2\xFE\xA3\xFE\xFC\xFE\xFD\xFE\xFE"),
              { 0xE000,0xE001,0xE002,0xE05B,0xE05C,0xE05D,
                0xE05E,0xE05F,0xE060,0xE0B9,0xE0BA,0xE0BB },
              12,
              true,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },

            // Test UTF-8:

            { RTL_TEXTENCODING_UTF8,
              RTL_CONSTASCII_STRINGPARAM("\x00"),
              { 0x0000 },
              1,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_UTF8,
              RTL_CONSTASCII_STRINGPARAM("\xEF\xBB\xBF"),
              { 0xFEFF },
              1,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_UTF8,
              RTL_CONSTASCII_STRINGPARAM("\xEF\xBB\xBF\xEF\xBB\xBF"),
              { 0xFEFF,0xFEFF },
              2,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_UTF8,
              RTL_CONSTASCII_STRINGPARAM("\xEF\xBB\xBF"),
              { 0 },
              0,
              false,
              true,
              true,
              true,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_UTF8,
              RTL_CONSTASCII_STRINGPARAM("\xEF\xBB\xBF\xEF\xBB\xBF"),
              { 0xFEFF },
              1,
              false,
              true,
              true,
              true,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_UTF8,
              RTL_CONSTASCII_STRINGPARAM("\x01\x02\x7E\x7F"),
              { 0x0001,0x0002,0x007E,0x007F },
              4,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_UTF8,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xC0\x80\xE0\x80\x81\xF0\x80\x80\x82\xF8\x80\x80\x80\x83"
                  "\xFC\x80\x80\x80\x80\x84"),
              { 0x0000,0x0001,0x0002,0x0003,0x0004 },
              5,
              false,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_UTF8,
              RTL_CONSTASCII_STRINGPARAM("\xED\xA1\x89\xED\xB4\x93"),
              { 0xD849,0xDD13 },
              2,
              false,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_UTF8,
              RTL_CONSTASCII_STRINGPARAM("\xED\xA1\x89\x41"),
              { 0xD849,0x0041 },
              2,
              false,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },

            // Test Java UTF-8:

            { RTL_TEXTENCODING_JAVA_UTF8,
              RTL_CONSTASCII_STRINGPARAM(
                  "\xEF\xBB\xBF\xC0\x80\x01\x20\x41\x7F\xED\xA0\x80"
                  "\xED\xB0\x80"),
              { 0xFEFF,0x0000,0x0001,0x0020,0x0041,0x007F,0xD800,0xDC00 },
              8,
              false,
              true,
              true,
              true,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },

            // Bug #112949#:

            { RTL_TEXTENCODING_SHIFT_JIS,
              RTL_CONSTASCII_STRINGPARAM("\x81\x63"),
              { 0x2026 },
              1,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_SHIFT_JIS,
              RTL_CONSTASCII_STRINGPARAM("\xA0\xFD\xFE\xFF"),
              { 0x00A0, 0x00A9, 0x2122, 0x2026 },
              4,
              false,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_SHIFT_JIS,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x00A0, 0x00A9, 0x2122 },
              3,
              false,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_MS_932,
              RTL_CONSTASCII_STRINGPARAM("\x81\x63"),
              { 0x2026 },
              1,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_MS_932,
              RTL_CONSTASCII_STRINGPARAM("\xA0\xFD\xFE\xFF"),
              { 0x00A0, 0x00A9, 0x2122, 0x2026 },
              4,
              false,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_MS_932,
              RTL_CONSTASCII_STRINGPARAM(""),
              { 0x00A0, 0x00A9, 0x2122 },
              3,
              false,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE },
            { RTL_TEXTENCODING_APPLE_JAPANESE,
              RTL_CONSTASCII_STRINGPARAM("\xA0\xFD\xFE\x81\x63"),
              { 0x00A0, 0x00A9, 0x2122, 0x2026 },
              4,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_APPLE_JAPANESE,
              RTL_CONSTASCII_STRINGPARAM("\xFF"),
              { 0x2026 },
              1,
              false,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },

            { RTL_TEXTENCODING_ADOBE_STANDARD,
              RTL_CONSTASCII_STRINGPARAM("\x20\x2D\xA4\xB4\xC5"),
              { 0x0020, 0x002D, 0x2215, 0x00B7, 0x00AF },
              5,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ADOBE_STANDARD,
              RTL_CONSTASCII_STRINGPARAM("\x20\x2D\xA4\xB4\xC5"),
              { 0x00A0, 0x00AD, 0x2044, 0x2219, 0x02C9 },
              5,
              false,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },

            { RTL_TEXTENCODING_ADOBE_SYMBOL,
              RTL_CONSTASCII_STRINGPARAM("\x20\x44\x57\x6D\xA4"),
              { 0x0020, 0x0394, 0x03A9, 0x03BC, 0x2215 },
              5,
              false,
              true,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_ADOBE_SYMBOL,
              RTL_CONSTASCII_STRINGPARAM("\x20\x44\x57\x6D\xA4"),
              { 0x00A0, 0x2206, 0x2126, 0x00B5, 0x2044 },
              5,
              false,
              false,
              true,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR }
        };
    {
        int nCount = sizeof aComplexCharSetTest
                         / sizeof aComplexCharSetTest[0];
        for (int i = 0; i < nCount; ++i)
            doComplexCharSetTest(aComplexCharSetTest[i]);
    }

    ComplexCharSetTest aComplexCharSetCutTest[]
        = { { RTL_TEXTENCODING_EUC_JP,
              RTL_CONSTASCII_STRINGPARAM("\xA1"),
              { 0 },
              0,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_JP,
              RTL_CONSTASCII_STRINGPARAM("\x8E"),
              { 0 },
              0,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_JP,
              RTL_CONSTASCII_STRINGPARAM("\x8F"),
              { 0 },
              0,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_JP,
              RTL_CONSTASCII_STRINGPARAM("\x8F\xA1"),
              { 0 },
              0,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_CN,
              RTL_CONSTASCII_STRINGPARAM("\xA1"),
              { 0 },
              0,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR } /* ,
            { RTL_TEXTENCODING_EUC_TW,
              RTL_CONSTASCII_STRINGPARAM("\xA1"),
              { 0 },
              0,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_TW,
              RTL_CONSTASCII_STRINGPARAM("\x8E"),
              { 0 },
              0,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_TW,
              RTL_CONSTASCII_STRINGPARAM("\x8E\xA1"),
              { 0 },
              0,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR },
            { RTL_TEXTENCODING_EUC_TW,
              RTL_CONSTASCII_STRINGPARAM("\x8E\xA1\xA1"),
              { 0 },
              0,
              true,
              true,
              false,
              false,
              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR } */ };
    {
        int nCount = sizeof aComplexCharSetCutTest
                         / sizeof aComplexCharSetCutTest[0];
        for (int i = 0; i < nCount; ++i)
            doComplexCharSetCutTest(aComplexCharSetCutTest[i]);
    }

    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("GBK")
                    == RTL_TEXTENCODING_GBK,
                "Detecting MIME charset name GBK");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("CP936")
                    == RTL_TEXTENCODING_GBK,
                "Detecting MIME charset name CP936");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("MS936")
                    == RTL_TEXTENCODING_GBK,
                "Detecting MIME charset name MS936");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("windows-936")
                    == RTL_TEXTENCODING_GBK,
                "Detecting MIME charset name windows-936");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("GB18030")
                    == RTL_TEXTENCODING_GB_18030,
                "Detecting MIME charset name GB18030");

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_TIS_620),
                       "TIS-620") == 0,
                "Returning MIME charset name TIS-620");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("TIS-620")
                    == RTL_TEXTENCODING_TIS_620,
                "Detecting MIME charset name TIS-620");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("ISO-8859-11")
                    == RTL_TEXTENCODING_TIS_620,
                "Detecting MIME charset name ISO-8859-11"); // not registered

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_MS_874),
                       "windows-874") == 0,
                "Returning MIME charset name windows-874"); // not registered
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("CP874")
                    == RTL_TEXTENCODING_MS_874,
                "Detecting MIME charset name CP874"); // not registered
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("MS874")
                    == RTL_TEXTENCODING_MS_874,
                "Detecting MIME charset name MS874"); // not registered
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("windows-874")
                    == RTL_TEXTENCODING_MS_874,
                "Detecting MIME charset name windows-874"); // not registered

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_ISO_8859_8),
                       "ISO-8859-8") == 0,
                "Returning MIME charset name ISO-8859-8");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("ISO_8859-8:1988")
                    == RTL_TEXTENCODING_ISO_8859_8,
                "Detecting MIME charset name ISO_8859-8:1988");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("iso-ir-138")
                    == RTL_TEXTENCODING_ISO_8859_8,
                "Detecting MIME charset name iso-ir-138");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("ISO_8859-8")
                    == RTL_TEXTENCODING_ISO_8859_8,
                "Detecting MIME charset name ISO_8859-8");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("ISO-8859-8")
                    == RTL_TEXTENCODING_ISO_8859_8,
                "Detecting MIME charset name ISO-8859-8");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("hebrew")
                    == RTL_TEXTENCODING_ISO_8859_8,
                "Detecting MIME charset name hebrew");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csISOLatinHebrew")
                    == RTL_TEXTENCODING_ISO_8859_8,
                "Detecting MIME charset name csISOLatinHebrew");

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_MS_1255),
                       "windows-1255") == 0,
                "Returning MIME charset name windows-1255");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("windows-1255")
                    == RTL_TEXTENCODING_MS_1255,
                "Detecting MIME charset name windows-1255");

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_IBM_862),
                       "IBM862") == 0,
                "Returning MIME charset name IBM862");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("IBM862")
                    == RTL_TEXTENCODING_IBM_862,
                "Detecting MIME charset name IBM862");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("cp862")
                    == RTL_TEXTENCODING_IBM_862,
                "Detecting MIME charset name cp862");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("862")
                    == RTL_TEXTENCODING_IBM_862,
                "Detecting MIME charset name 862");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csPC862LatinHebrew")
                    == RTL_TEXTENCODING_IBM_862,
                "Detecting MIME charset name csPC862LatinHebrew");

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_ISO_8859_6),
                       "ISO-8859-6") == 0,
                "Returning MIME charset name ISO_8859_6");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("ISO_8859-6:1987")
                    == RTL_TEXTENCODING_ISO_8859_6,
                "Detecting MIME charset name ISO_8859-6:1987");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("iso-ir-127")
                    == RTL_TEXTENCODING_ISO_8859_6,
                "Detecting MIME charset name iso-ir-127");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("ISO_8859-6")
                    == RTL_TEXTENCODING_ISO_8859_6,
                "Detecting MIME charset name ISO_8859-6");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("ISO-8859-6")
                    == RTL_TEXTENCODING_ISO_8859_6,
                "Detecting MIME charset name ISO-8859-6");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("ECMA-114")
                    == RTL_TEXTENCODING_ISO_8859_6,
                "Detecting MIME charset name ECMA-114");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("ASMO-708")
                    == RTL_TEXTENCODING_ISO_8859_6,
                "Detecting MIME charset name ASMO-708");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("arabic")
                    == RTL_TEXTENCODING_ISO_8859_6,
                "Detecting MIME charset name arabic");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csISOLatinArabic")
                    == RTL_TEXTENCODING_ISO_8859_6,
                "Detecting MIME charset name csISOLatinArabic");

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_MS_1256),
                       "windows-1256") == 0,
                "Returning MIME charset name windows-1256");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("windows-1256")
                    == RTL_TEXTENCODING_MS_1256,
                "Detecting MIME charset name windows-1256");

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_IBM_864),
                       "IBM864") == 0,
                "Returning MIME charset name IBM864");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("IBM864")
                    == RTL_TEXTENCODING_IBM_864,
                "Detecting MIME charset name IBM864");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("cp864")
                    == RTL_TEXTENCODING_IBM_864,
                "Detecting MIME charset name cp864");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csIBM864")
                    == RTL_TEXTENCODING_IBM_864,
                "Detecting MIME charset name csIBM864");

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_KOI8_R),
                       "koi8-r") == 0,
                "Returning MIME charset name koi8-r");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("KOI8-R")
                    == RTL_TEXTENCODING_KOI8_R,
                "Detecting MIME charset name KOI8-R");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csKOI8R")
                    == RTL_TEXTENCODING_KOI8_R,
                "Detecting MIME charset name csKOI8R");

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_KOI8_U),
                       "KOI8-U") == 0,
                "Returning MIME charset name KOI8-U");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("KOI8-U")
                    == RTL_TEXTENCODING_KOI8_U,
                "Detecting MIME charset name KOI8-U");
    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_IBM_860),
                       "IBM860") == 0,
                "Returning MIME charset name IBM860");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("IBM860")
                    == RTL_TEXTENCODING_IBM_860,
                "Detecting MIME charset name IBM860");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("cp860")
                    == RTL_TEXTENCODING_IBM_860,
                "Detecting MIME charset name cp860");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("860")
                    == RTL_TEXTENCODING_IBM_860,
                "Detecting MIME charset name 860");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csIBM860")
                    == RTL_TEXTENCODING_IBM_860,
                "Detecting MIME charset name csIBM860");

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_IBM_861),
                       "IBM861") == 0,
                "Returning MIME charset name IBM861");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("IBM861")
                    == RTL_TEXTENCODING_IBM_861,
                "Detecting MIME charset name IBM861");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("cp861")
                    == RTL_TEXTENCODING_IBM_861,
                "Detecting MIME charset name cp861");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("861")
                    == RTL_TEXTENCODING_IBM_861,
                "Detecting MIME charset name 861");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("cp-is")
                    == RTL_TEXTENCODING_IBM_861,
                "Detecting MIME charset name cp-is");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csIBM861")
                    == RTL_TEXTENCODING_IBM_861,
                "Detecting MIME charset name csIBM861");

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_IBM_863),
                       "IBM863") == 0,
                "Returning MIME charset name IBM863");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("IBM863")
                    == RTL_TEXTENCODING_IBM_863,
                "Detecting MIME charset name IBM863");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("cp863")
                    == RTL_TEXTENCODING_IBM_863,
                "Detecting MIME charset name cp863");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("863")
                    == RTL_TEXTENCODING_IBM_863,
                "Detecting MIME charset name 863");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csIBM863")
                    == RTL_TEXTENCODING_IBM_863,
                "Detecting MIME charset name csIBM863");

    TEST_ENSURE(strcmp(rtl_getMimeCharsetFromTextEncoding(
                           RTL_TEXTENCODING_IBM_865),
                       "IBM865") == 0,
                "Returning MIME charset name IBM865");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("IBM865")
                    == RTL_TEXTENCODING_IBM_865,
                "Detecting MIME charset name IBM865");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("cp865")
                    == RTL_TEXTENCODING_IBM_865,
                "Detecting MIME charset name cp865");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("865")
                    == RTL_TEXTENCODING_IBM_865,
                "Detecting MIME charset name 865");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csIBM865")
                    == RTL_TEXTENCODING_IBM_865,
                "Detecting MIME charset name csIBM865");

    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("Latin-9")
                    == RTL_TEXTENCODING_ISO_8859_15,
                "Detecting MIME charset name Latin-9");

    TEST_ENSURE(rtl_getMimeCharsetFromTextEncoding(RTL_TEXTENCODING_MS_949)
                    == 0,
                "Returning MIME charset name for MS 949");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("KS_C_5601-1987")
                    == RTL_TEXTENCODING_MS_949,
                "Detecting MIME charset name KS_C_5601-1987");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("iso-ir-149")
                    == RTL_TEXTENCODING_MS_949,
                "Detecting MIME charset name iso-ir-149");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("KS_C_5601-1989")
                    == RTL_TEXTENCODING_MS_949,
                "Detecting MIME charset name KS_C_5601-1989");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("KSC_5601")
                    == RTL_TEXTENCODING_MS_949,
                "Detecting MIME charset name KSC_5601");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("korean")
                    == RTL_TEXTENCODING_MS_949,
                "Detecting MIME charset name korean");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csKSC56011987")
                    == RTL_TEXTENCODING_MS_949,
                "Detecting MIME charset name csKSC56011987");

    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("Adobe-Standard-Encoding")
                    == RTL_TEXTENCODING_ADOBE_STANDARD,
                "Detecting MIME charset name Adobe-Standard-Encoding");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csAdobeStandardEncoding")
                    == RTL_TEXTENCODING_ADOBE_STANDARD,
                "Detecting MIME charset name csAdobeStandardEncoding");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("Adobe-Symbol-Encoding")
                    == RTL_TEXTENCODING_ADOBE_SYMBOL,
                "Detecting MIME charset name Adobe-Symbol-Encoding");
    TEST_ENSURE(rtl_getTextEncodingFromMimeCharset("csHPPSMath")
                    == RTL_TEXTENCODING_ADOBE_SYMBOL,
                "Detecting MIME charset name csHPPSMath");

    {
        struct Test
        {
            rtl_TextEncoding eEncoding;
            sal_uInt32 nFlag;
            bool bOn;
        };
        static Test const aTests[]
            = { { RTL_TEXTENCODING_APPLE_CHINTRAD,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_APPLE_JAPANESE,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_APPLE_KOREAN,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_BIG5,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_BIG5_HKSCS,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_EUC_CN,
                  RTL_TEXTENCODING_INFO_ASCII, true },
                { RTL_TEXTENCODING_EUC_JP,
                  RTL_TEXTENCODING_INFO_ASCII, true },
                { RTL_TEXTENCODING_EUC_KR,
                  RTL_TEXTENCODING_INFO_ASCII, true },
                { RTL_TEXTENCODING_EUC_TW,
                  RTL_TEXTENCODING_INFO_ASCII, true },
                { RTL_TEXTENCODING_GBK,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_GB_18030,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_GB_18030,
                  RTL_TEXTENCODING_INFO_UNICODE, true },
                { RTL_TEXTENCODING_ISO_2022_CN,
                  RTL_TEXTENCODING_INFO_CONTEXT, true },
                { RTL_TEXTENCODING_ISO_2022_CN,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_ISO_2022_JP,
                  RTL_TEXTENCODING_INFO_CONTEXT, true },
                { RTL_TEXTENCODING_ISO_2022_JP,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_ISO_2022_KR,
                  RTL_TEXTENCODING_INFO_CONTEXT, true },
                { RTL_TEXTENCODING_ISO_2022_KR,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_MS_1361,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_MS_874,
                  RTL_TEXTENCODING_INFO_ASCII, true },
                { RTL_TEXTENCODING_MS_932,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_MS_936,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_MS_949,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_MS_950,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_SHIFT_JIS,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_KOI8_R,
                  RTL_TEXTENCODING_INFO_ASCII, true },
                { RTL_TEXTENCODING_KOI8_R,
                  RTL_TEXTENCODING_INFO_MIME, true },
                { RTL_TEXTENCODING_KOI8_U,
                  RTL_TEXTENCODING_INFO_ASCII, true },
                { RTL_TEXTENCODING_KOI8_U,
                  RTL_TEXTENCODING_INFO_MIME, true },
                { RTL_TEXTENCODING_IBM_860, RTL_TEXTENCODING_INFO_MIME, true },
                { RTL_TEXTENCODING_IBM_861, RTL_TEXTENCODING_INFO_MIME, true },
                { RTL_TEXTENCODING_IBM_863, RTL_TEXTENCODING_INFO_MIME, true },
                { RTL_TEXTENCODING_IBM_865, RTL_TEXTENCODING_INFO_MIME, true },
                { RTL_TEXTENCODING_ISCII_DEVANAGARI,
                  RTL_TEXTENCODING_INFO_ASCII, true },
                { RTL_TEXTENCODING_ISCII_DEVANAGARI,
                  RTL_TEXTENCODING_INFO_MIME, false },
                { RTL_TEXTENCODING_ADOBE_STANDARD,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_ADOBE_STANDARD,
                  RTL_TEXTENCODING_INFO_MIME, true },
                { RTL_TEXTENCODING_ADOBE_STANDARD,
                  RTL_TEXTENCODING_INFO_SYMBOL, false },
                { RTL_TEXTENCODING_ADOBE_SYMBOL,
                  RTL_TEXTENCODING_INFO_ASCII, false },
                { RTL_TEXTENCODING_ADOBE_SYMBOL,
                  RTL_TEXTENCODING_INFO_MIME, true },
                { RTL_TEXTENCODING_ADOBE_SYMBOL,
                  RTL_TEXTENCODING_INFO_SYMBOL, true } };
        for (size_t i = 0; i < sizeof aTests / sizeof aTests[0]; ++i)
        {
            rtl_TextEncodingInfo aInfo;
            aInfo.StructSize = sizeof aInfo;
            if (!rtl_getTextEncodingInfo(aTests[i].eEncoding, &aInfo))
            {
                printf("rtl_getTextEncodingInfo(%d) FAILED\n",
                       static_cast< int >(aTests[i].eEncoding));
                continue;
            }
            if (((aInfo.Flags & aTests[i].nFlag) != 0) != aTests[i].bOn)
                printf("rtl_getTextEncodingInfo(%d): flag %d != %d\n",
                       static_cast< int >(aTests[i].eEncoding),
                       static_cast< int >(aTests[i].nFlag),
                       static_cast< int >(aTests[i].bOn));
        }
    }

    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(437)
                    == RTL_TEXTENCODING_IBM_437,
                "Code Page 437 -> IBM_437");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_437)
                    == 437,
                "IBM_437 -> Code Page 437");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(708)
                    == RTL_TEXTENCODING_ISO_8859_6,
                "Code Page 708 -> ISO_8859_6");
//   TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
//                       RTL_TEXTENCODING_ISO_8859_6)
//                   == 708,
//               "ISO_8859_6 -> Code Page 708");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(737)
                    == RTL_TEXTENCODING_IBM_737,
                "Code Page 737 -> IBM_737");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_737)
                    == 737,
                "IBM_737 -> Code Page 737");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(775)
                    == RTL_TEXTENCODING_IBM_775,
                "Code Page 775 -> IBM_775");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_775)
                    == 775,
                "IBM_775 -> Code Page 775");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(850)
                    == RTL_TEXTENCODING_IBM_850,
                "Code Page 850 -> IBM_850");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_850)
                    == 850,
                "IBM_850 -> Code Page 850");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(852)
                    == RTL_TEXTENCODING_IBM_852,
                "Code Page 852 -> IBM_852");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_852)
                    == 852,
                "IBM_852 -> Code Page 852");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(855)
                    == RTL_TEXTENCODING_IBM_855,
                "Code Page 855 -> IBM_855");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_855)
                    == 855,
                "IBM_855 -> Code Page 855");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(857)
                    == RTL_TEXTENCODING_IBM_857,
                "Code Page 857 -> IBM_857");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_857)
                    == 857,
                "IBM_857 -> Code Page 857");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(860)
                    == RTL_TEXTENCODING_IBM_860,
                "Code Page 860 -> IBM_860");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_860)
                    == 860,
                "IBM_860 -> Code Page 860");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(861)
                    == RTL_TEXTENCODING_IBM_861,
                "Code Page 861 -> IBM_861");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_861)
                    == 861,
                "IBM_861 -> Code Page 861");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(862)
                    == RTL_TEXTENCODING_IBM_862,
                "Code Page 862 -> IBM_862");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_862)
                    == 862,
                "IBM_862 -> Code Page 862");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(863)
                    == RTL_TEXTENCODING_IBM_863,
                "Code Page 863 -> IBM_863");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_863)
                    == 863,
                "IBM_863 -> Code Page 863");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(864)
                    == RTL_TEXTENCODING_IBM_864,
                "Code Page 864 -> IBM_864");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_864)
                    == 864,
                "IBM_864 -> Code Page 864");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(865)
                    == RTL_TEXTENCODING_IBM_865,
                "Code Page 865 -> IBM_865");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_865)
                    == 865,
                "IBM_865 -> Code Page 865");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(866)
                    == RTL_TEXTENCODING_IBM_866,
                "Code Page 866 -> IBM_866");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_866)
                    == 866,
                "IBM_866 -> Code Page 866");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(869)
                    == RTL_TEXTENCODING_IBM_869,
                "Code Page 869 -> IBM_869");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_IBM_869)
                    == 869,
                "IBM_869 -> Code Page 869");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(874)
                    == RTL_TEXTENCODING_MS_874,
                "Code Page 874 -> MS_874");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_874)
                    == 874,
                "MS_874 -> Code Page 874");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(932)
                    == RTL_TEXTENCODING_MS_932,
                "Code Page 932 -> MS_932");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_932)
                    == 932,
                "MS_932 -> Code Page 932");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(936)
                    == RTL_TEXTENCODING_MS_936,
                "Code Page 936 -> MS_936");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_936)
                    == 936,
                "MS_936 -> Code Page 936");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(949)
                    == RTL_TEXTENCODING_MS_949,
                "Code Page 949 -> MS_949");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_949)
                    == 949,
                "MS_949 -> Code Page 949");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(950)
                    == RTL_TEXTENCODING_MS_950,
                "Code Page 950 -> MS_950");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_950)
                    == 950,
                "MS_950 -> Code Page 950");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1250)
                    == RTL_TEXTENCODING_MS_1250,
                "Code Page 1250 -> MS_1250");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_1250)
                    == 1250,
                "MS_1250 -> Code Page 1250");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1251)
                    == RTL_TEXTENCODING_MS_1251,
                "Code Page 1251 -> MS_1251");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_1251)
                    == 1251,
                "MS_1251 -> Code Page 1251");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1252)
                    == RTL_TEXTENCODING_MS_1252,
                "Code Page 1252 -> MS_1252");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_1252)
                    == 1252,
                "MS_1252 -> Code Page 1252");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1253)
                    == RTL_TEXTENCODING_MS_1253,
                "Code Page 1253 -> MS_1253");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_1253)
                    == 1253,
                "MS_1253 -> Code Page 1253");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1254)
                    == RTL_TEXTENCODING_MS_1254,
                "Code Page 1254 -> MS_1254");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_1254)
                    == 1254,
                "MS_1254 -> Code Page 1254");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1255)
                    == RTL_TEXTENCODING_MS_1255,
                "Code Page 1255 -> MS_1255");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_1255)
                    == 1255,
                "MS_1255 -> Code Page 1255");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1256)
                    == RTL_TEXTENCODING_MS_1256,
                "Code Page 1256 -> MS_1256");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_1256)
                    == 1256,
                "MS_1256 -> Code Page 1256");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1257)
                    == RTL_TEXTENCODING_MS_1257,
                "Code Page 1257 -> MS_1257");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_1257)
                    == 1257,
                "MS_1257 -> Code Page 1257");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1258)
                    == RTL_TEXTENCODING_MS_1258,
                "Code Page 1258 -> MS_1258");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_1258)
                    == 1258,
                "MS_1258 -> Code Page 1258");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1361)
                    == RTL_TEXTENCODING_MS_1361,
                "Code Page 1361 -> MS_1361");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_MS_1361)
                    == 1361,
                "MS_1361 -> Code Page 1361");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10000)
                    == RTL_TEXTENCODING_APPLE_ROMAN,
                "Code Page 10000 -> APPLE_ROMAN");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_ROMAN)
                    == 10000,
                "APPLE_ROMAN -> Code Page 10000");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10001)
                    == RTL_TEXTENCODING_APPLE_JAPANESE,
                "Code Page 10001 -> APPLE_JAPANESE");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_JAPANESE)
                    == 10001,
                "APPLE_JAPANESE -> Code Page 10001");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10002)
                    == RTL_TEXTENCODING_APPLE_CHINTRAD,
                "Code Page 10002 -> APPLE_CHINTRAD");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_CHINTRAD)
                    == 10002,
                "APPLE_CHINTRAD -> Code Page 10002");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10003)
                    == RTL_TEXTENCODING_APPLE_KOREAN,
                "Code Page 10003 -> APPLE_KOREAN");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_KOREAN)
                    == 10003,
                "APPLE_KOREAN -> Code Page 10003");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10004)
                    == RTL_TEXTENCODING_APPLE_ARABIC,
                "Code Page 10004 -> APPLE_ARABIC");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_ARABIC)
                    == 10004,
                "APPLE_ARABIC -> Code Page 10004");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10005)
                    == RTL_TEXTENCODING_APPLE_HEBREW,
                "Code Page 10005 -> APPLE_HEBREW");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_HEBREW)
                    == 10005,
                "APPLE_HEBREW -> Code Page 10005");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10006)
                    == RTL_TEXTENCODING_APPLE_GREEK,
                "Code Page 10006 -> APPLE_GREEK");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_GREEK)
                    == 10006,
                "APPLE_GREEK -> Code Page 10006");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10007)
                    == RTL_TEXTENCODING_APPLE_CYRILLIC,
                "Code Page 10007 -> APPLE_CYRILLIC");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_CYRILLIC)
                    == 10007,
                "APPLE_CYRILLIC -> Code Page 10007");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10008)
                    == RTL_TEXTENCODING_APPLE_CHINSIMP,
                "Code Page 10008 -> APPLE_CHINSIMP");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_CHINSIMP)
                    == 10008,
                "APPLE_CHINSIMP -> Code Page 10008");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10010)
                    == RTL_TEXTENCODING_APPLE_ROMANIAN,
                "Code Page 10010 -> APPLE_ROMANIAN");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_ROMANIAN)
                    == 10010,
                "APPLE_ROMANIAN -> Code Page 10010");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10017)
                    == RTL_TEXTENCODING_APPLE_UKRAINIAN,
                "Code Page 10017 -> APPLE_UKRAINIAN");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_UKRAINIAN)
                    == 10017,
                "APPLE_UKRAINIAN -> Code Page 10017");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10029)
                    == RTL_TEXTENCODING_APPLE_CENTEURO,
                "Code Page 10029 -> APPLE_CENTEURO");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_CENTEURO)
                    == 10029,
                "APPLE_CENTEURO -> Code Page 10029");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10079)
                    == RTL_TEXTENCODING_APPLE_ICELAND,
                "Code Page 10079 -> APPLE_ICELAND");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_ICELAND)
                    == 10079,
                "APPLE_ICELAND -> Code Page 10079");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10081)
                    == RTL_TEXTENCODING_APPLE_TURKISH,
                "Code Page 10081 -> APPLE_TURKISH");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_TURKISH)
                    == 10081,
                "APPLE_TURKISH -> Code Page 10081");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(10082)
                    == RTL_TEXTENCODING_APPLE_CROATIAN,
                "Code Page 10082 -> APPLE_CROATIAN");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_APPLE_CROATIAN)
                    == 10082,
                "APPLE_CROATIAN -> Code Page 10082");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(20127)
                    == RTL_TEXTENCODING_ASCII_US,
                "Code Page 20127 -> ASCII_US");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ASCII_US)
                    == 20127,
                "ASCII_US -> Code Page 20127");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(20866)
                    == RTL_TEXTENCODING_KOI8_R,
                "Code Page 20866 -> KOI8_R");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_KOI8_R)
                    == 20866,
                "KOI8_R -> Code Page 20866");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(21866)
                    == RTL_TEXTENCODING_KOI8_U,
                "Code Page 21866 -> KOI8_U");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_KOI8_U)
                    == 21866,
                "KOI8_U -> Code Page 21866");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(28591)
                    == RTL_TEXTENCODING_ISO_8859_1,
                "Code Page 28591 -> ISO_8859_1");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_8859_1)
                    == 28591,
                "ISO_8859_1 -> Code Page 28591");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(28592)
                    == RTL_TEXTENCODING_ISO_8859_2,
                "Code Page 28592 -> ISO_8859_2");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_8859_2)
                    == 28592,
                "ISO_8859_2 -> Code Page 28592");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(28593)
                    == RTL_TEXTENCODING_ISO_8859_3,
                "Code Page 28593 -> ISO_8859_3");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_8859_3)
                    == 28593,
                "ISO_8859_3 -> Code Page 28593");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(28594)
                    == RTL_TEXTENCODING_ISO_8859_4,
                "Code Page 28594 -> ISO_8859_4");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_8859_4)
                    == 28594,
                "ISO_8859_4 -> Code Page 28594");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(28595)
                    == RTL_TEXTENCODING_ISO_8859_5,
                "Code Page 28595 -> ISO_8859_5");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_8859_5)
                    == 28595,
                "ISO_8859_5 -> Code Page 28595");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(28596)
                    == RTL_TEXTENCODING_ISO_8859_6,
                "Code Page 28596 -> ISO_8859_6");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_8859_6)
                    == 28596,
                "ISO_8859_6 -> Code Page 28596");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(28597)
                    == RTL_TEXTENCODING_ISO_8859_7,
                "Code Page 28597 -> ISO_8859_7");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_8859_7)
                    == 28597,
                "ISO_8859_7 -> Code Page 28597");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(28598)
                    == RTL_TEXTENCODING_ISO_8859_8,
                "Code Page 28598 -> ISO_8859_8");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_8859_8)
                    == 28598,
                "ISO_8859_8 -> Code Page 28598");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(28599)
                    == RTL_TEXTENCODING_ISO_8859_9,
                "Code Page 28599 -> ISO_8859_9");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_8859_9)
                    == 28599,
                "ISO_8859_9 -> Code Page 28599");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(28605)
                    == RTL_TEXTENCODING_ISO_8859_15,
                "Code Page 28605 -> ISO_8859_15");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_8859_15)
                    == 28605,
                "ISO_8859_15 -> Code Page 28605");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(50220)
                    == RTL_TEXTENCODING_ISO_2022_JP,
                "Code Page 50220 -> ISO_2022_JP");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_2022_JP)
                    == 50220,
                "ISO_2022_JP -> Code Page 50220");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(50225)
                    == RTL_TEXTENCODING_ISO_2022_KR,
                "Code Page 50225 -> ISO_2022_KR");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_ISO_2022_KR)
                    == 50225,
                "ISO_2022_KR -> Code Page 50225");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(51932)
                    == RTL_TEXTENCODING_EUC_JP,
                "Code Page 51932 -> EUC_JP");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_EUC_JP)
                    == 51932,
                "EUC_JP -> Code Page 51932");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(51936)
                    == RTL_TEXTENCODING_EUC_CN,
                "Code Page 51936 -> EUC_CN");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_EUC_CN)
                    == 51936,
                "EUC_CN -> Code Page 51936");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(51949)
                    == RTL_TEXTENCODING_EUC_KR,
                "Code Page 51949 -> EUC_KR");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_EUC_KR)
                    == 51949,
                "EUC_KR -> Code Page 51949");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(65000)
                    == RTL_TEXTENCODING_UTF7,
                "Code Page 65000 -> UTF7");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_UTF7)
                    == 65000,
                "UTF7 -> Code Page 65000");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(65001)
                    == RTL_TEXTENCODING_UTF8,
                "Code Page 65001 -> UTF8");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_UTF8)
                    == 65001,
                "UTF8 -> Code Page 65001");
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1200)
                    == RTL_TEXTENCODING_DONTKNOW,
                "Code Page 1200 -> DONTKNOW"); // UTF-16LE
    TEST_ENSURE(rtl_getTextEncodingFromWindowsCodePage(1201)
                    == RTL_TEXTENCODING_DONTKNOW,
                "Code Page 1201 -> DONTKNOW"); // UTF-16BE
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                        RTL_TEXTENCODING_DONTKNOW)
                    == 0,
                "DONTKNOW -> Code Page 0");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_UCS4)
                    == 0,
                "UCS4 -> Code Page 0");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(RTL_TEXTENCODING_UCS2)
                    == 0,
                "UCS2 -> Code Page 0");
    TEST_ENSURE(rtl_getWindowsCodePageFromTextEncoding(
                    RTL_TEXTENCODING_ISCII_DEVANAGARI) == 0,
                "ISCII Devanagari -> Code Page 0");

        // printf("textenc test done\n");
}


} // namespace rtl_text

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_textenc::enc, "rtl_textenc" );

// -----------------------------------------------------------------------------
NOADDITIONAL;
