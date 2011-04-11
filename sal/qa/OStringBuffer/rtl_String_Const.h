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

#ifndef _RTL_STRING_CONST_H_
#define _RTL_STRING_CONST_H_

#ifndef _RTL_STRING_UTILS_HXX_
    #include <rtl_String_Utils.hxx>
#endif

#include <limits.h>

#ifndef _SAL_TYPES_H_
    #include <sal/types.h>
#endif

#ifndef _RTL_TEXTENC_H
    #include <rtl/textenc.h>
#endif

#ifndef _RTL_USTRING_H_
    #include <rtl/ustring.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

static const rtl_TextEncoding kEncodingRTLTextUSASCII = RTL_TEXTENCODING_ASCII_US;

static const sal_uInt32 kConvertFlagsOUStringToOString = OUSTRING_TO_OSTRING_CVTFLAGS;
static const sal_uInt32 kConvertFlagsOStringToOUString = OSTRING_TO_OUSTRING_CVTFLAGS;

static const sal_Char *kTestStr1  = "Sun Microsystems";
static const sal_Char *kTestStr2  = "Sun Microsystems Java Technology";
static const sal_Char *kTestStr3  = "Sun microsystems";
static const sal_Char *kTestStr7  = "Sun ";
static const sal_Char *kTestStr8  = "Microsystems";
static const sal_Char *kTestStr14 = "   Sun Microsystems";
static const sal_Char *kTestStr17 = "   Sun Microsystems   ";
static const sal_Char *kTestStr23  = " Java Technology";
static const sal_Char *kTestStr25 = "";
static const sal_Char *kTestStr27 = "s";
static const sal_Char *kTestStr28 = "\50\3\5\7\11\13\15\17sun";
static const sal_Char *kTestStr29 = "\50\3\5\7\11\13\15\17sun\21\23\25\27\31\33\50";
static const sal_Char *kTestStr31 = "sun Microsystems";
static const sal_Char *kTestStr32 = "Sun Microsystem ";
static const sal_Char *kTestStr33 = " ";
static const sal_Char *kTestStr34 = "\50\5\5\7\11\13\15\17sun";
static const sal_Char *kTestStr35 = "\50\373\5\7\11\13\15\17sun";
static const sal_Char *kTestStr36 = "Microsystems Java Technology";
static const sal_Char *kTestStr37 = "Sun  Java Technology";
static const sal_Char *kTestStr38 = "\21\23\25\27\31\33\50";
static const sal_Char *kTestStr39 = "\50\3\5\7\11\13\15\17sun   Sun Microsystems   ";
static const sal_Char *kTestStr40 = "\50\3\5\7\11\13\15\17sunsun Microsystems";
static const sal_Char *kTestStr45  = "Sun true";
static const sal_Char *kTestStr46  = "Sun false";
static const sal_Char *kTestStr47  = "true";
static const sal_Char *kTestStr48  = "false";
static const sal_Char *kTestStr49 = "\50\3\5\7\11\13\15\17suntrue";
static const sal_Char *kTestStr50 = "\50\3\5\7\11\13\15\17sunfalse";
static const sal_Char *kTestStr51  = "Sun M";
//static const sal_Char *kTestStr52  = "Sun \077777";
//static const sal_Char *kTestStr53  = "Sun \100000";
//static const sal_Char *kTestStr54  = "\77777";
//static const sal_Char *kTestStr55  = "\100000";
static const sal_Char *kTestStr56 = "\50\3\5\7\11\13\15\17suns";
//static const sal_Char *kTestStr57 = "\50\3\5\7\11\13\15\17sun\77777";
//static const sal_Char *kTestStr58 = "\50\3\5\7\11\13\15\17sun\10000";
static const sal_Char *kTestStr59  = "Sun 11";
static const sal_Char *kTestStr60  = "11";
static const sal_Char *kTestStr61  = "\50\3\5\7\11\13\15\17sun11";
static const sal_Char *kTestStr62  = "Sun 0";
static const sal_Char *kTestStr63  = "Sun -11";
static const sal_Char *kTestStr64  = "Sun 2147483647";
static const sal_Char *kTestStr65  = "Sun -2147483648";
static const sal_Char *kTestStr66  = "0";
static const sal_Char *kTestStr67  = "-11";
static const sal_Char *kTestStr68  = "2147483647";
static const sal_Char *kTestStr69  = "-2147483648";
static const sal_Char *kTestStr70  = "\50\3\5\7\11\13\15\17sun0";
static const sal_Char *kTestStr71  = "\50\3\5\7\11\13\15\17sun-11";
static const sal_Char *kTestStr72  = "\50\3\5\7\11\13\15\17sun2147483647";
static const sal_Char *kTestStr73  = "\50\3\5\7\11\13\15\17sun-2147483648";
static const sal_Char *kTestStr116  = "Sun 9223372036854775807";
static const sal_Char *kTestStr117  = "Sun -9223372036854775808";
static const sal_Char *kTestStr118  = "9223372036854775807";
static const sal_Char *kTestStr119  = "-9223372036854775808";
static const sal_Char *kTestStr120  = "\50\3\5\7\11\13\15\17sun9223372036854775807";
static const sal_Char *kTestStr121  = "\50\3\5\7\11\13\15\17sun-9223372036854775808";
static const sal_Char *kTestStr143  = "Sun \377";
static const sal_Char *kTestStr144  = "\377";
static const sal_Char *kTestStr145 = "\50\3\5\7\11\13\15\17sun\377";

static const sal_Int32 kTestStr1Len  = 16;
static const sal_Int32 kTestStr2Len  = 32;
static const sal_Int32 kTestStr3Len  = 16;
static const sal_Int32 kTestStr4Len  = 16;
static const sal_Int32 kTestStr5Len  = 16;
static const sal_Int32 kTestStr6Len  = 15;
static const sal_Int32 kTestStr7Len  = 4;
static const sal_Int32 kTestStr8Len  = 12;
static const sal_Int32 kTestStr9Len  = 32;
static const sal_Int32 kTestStr10Len = 17;
static const sal_Int32 kTestStr11Len = 17;
static const sal_Int32 kTestStr12Len = 18;
static const sal_Int32 kTestStr13Len = 19;
static const sal_Int32 kTestStr14Len = 19;
static const sal_Int32 kTestStr15Len = 20;
static const sal_Int32 kTestStr16Len = 20;
static const sal_Int32 kTestStr17Len = 22;
static const sal_Int32 kTestStr18Len = 16;
static const sal_Int32 kTestStr19Len = 22;
static const sal_Int32 kTestStr20Len = 3;
static const sal_Int32 kTestStr21Len = 3;
static const sal_Int32 kTestStr22Len = 32;
static const sal_Int32 kTestStr23Len = 16;
static const sal_Int32 kTestStr24Len = 31;
static const sal_Int32 kTestStr25Len = 0;
static const sal_Int32 kTestStr26Len = 4;
static const sal_Int32 kTestStr27Len = 1;
static const sal_Int32 kTestStr28Len = 11;
static const sal_Int32 kTestStr29Len = 18;
static const sal_Int32 kTestStr30Len = 10;
static const sal_Int32 kTestStr31Len = 16;
static const sal_Int32 kTestStr32Len = 16;
static const sal_Int32 kTestStr33Len = 1;
static const sal_Int32 kTestStr34Len = 11;
static const sal_Int32 kTestStr35Len = 11;
static const sal_Int32 kTestStr36Len = 28;
static const sal_Int32 kTestStr37Len = 20;
static const sal_Int32 kTestStr38Len = 7;
static const sal_Int32 kTestStr39Len = 33;
static const sal_Int32 kTestStr40Len = 27;
static const sal_Int32 kTestStr41Len = 3;
static const sal_Int32 kTestStr42Len = 10;
static const sal_Int32 kTestStr43Len = 13;
static const sal_Int32 kTestStr44Len = 2;
static const sal_Int32 kTestStr45Len = 8;
static const sal_Int32 kTestStr46Len = 9;
static const sal_Int32 kTestStr47Len = 4;
static const sal_Int32 kTestStr48Len = 5;
static const sal_Int32 kTestStr49Len = 15;
static const sal_Int32 kTestStr50Len = 16;
static const sal_Int32 kTestStr51Len = 5;
static const sal_Int32 kTestStr52Len = 5;
static const sal_Int32 kTestStr53Len = 5;
static const sal_Int32 kTestStr54Len = 1;
static const sal_Int32 kTestStr55Len = 1;
static const sal_Int32 kTestStr56Len = 12;
static const sal_Int32 kTestStr57Len = 12;
static const sal_Int32 kTestStr58Len = 12;
static const sal_Int32 kTestStr59Len = 6;
static const sal_Int32 kTestStr60Len = 2;
static const sal_Int32 kTestStr61Len = 13;
static const sal_Int32 kTestStr62Len = 5;
static const sal_Int32 kTestStr63Len = 7;
static const sal_Int32 kTestStr64Len = 14;
static const sal_Int32 kTestStr65Len = 15;
static const sal_Int32 kTestStr66Len = 1;
static const sal_Int32 kTestStr67Len = 3;
static const sal_Int32 kTestStr68Len = 10;
static const sal_Int32 kTestStr69Len = 11;
static const sal_Int32 kTestStr70Len = 12;
static const sal_Int32 kTestStr71Len = 14;
static const sal_Int32 kTestStr72Len = 21;
static const sal_Int32 kTestStr73Len = 22;
static const sal_Int32 kTestStr74Len = 7;
static const sal_Int32 kTestStr75Len = 7;
static const sal_Int32 kTestStr76Len = 10;
static const sal_Int32 kTestStr77Len = 12;
static const sal_Int32 kTestStr78Len = 12;
static const sal_Int32 kTestStr79Len = 13;
static const sal_Int32 kTestStr80Len = 13;
static const sal_Int32 kTestStr81Len = 3;
static const sal_Int32 kTestStr82Len = 3;
static const sal_Int32 kTestStr83Len = 6;
static const sal_Int32 kTestStr84Len = 8;
static const sal_Int32 kTestStr85Len = 8;
static const sal_Int32 kTestStr86Len = 9;
static const sal_Int32 kTestStr87Len = 9;
static const sal_Int32 kTestStr88Len = 14;
static const sal_Int32 kTestStr89Len = 14;
static const sal_Int32 kTestStr90Len = 17;
static const sal_Int32 kTestStr91Len = 19;
static const sal_Int32 kTestStr92Len = 19;
static const sal_Int32 kTestStr93Len = 20;
static const sal_Int32 kTestStr94Len = 20;
static const sal_Int32 kTestStr95Len = 8;
static const sal_Int32 kTestStr96Len = 8;
static const sal_Int32 kTestStr97Len = 11;
static const sal_Int32 kTestStr98Len = 13;
static const sal_Int32 kTestStr99Len = 13;
static const sal_Int32 kTestStr100Len = 14;
static const sal_Int32 kTestStr101Len = 14;
static const sal_Int32 kTestStr102Len = 4;
static const sal_Int32 kTestStr103Len = 4;
static const sal_Int32 kTestStr104Len = 7;
static const sal_Int32 kTestStr105Len = 9;
static const sal_Int32 kTestStr106Len = 9;
static const sal_Int32 kTestStr107Len = 10;
static const sal_Int32 kTestStr108Len = 10;
static const sal_Int32 kTestStr109Len = 15;
static const sal_Int32 kTestStr110Len = 15;
static const sal_Int32 kTestStr111Len = 18;
static const sal_Int32 kTestStr112Len = 20;
static const sal_Int32 kTestStr113Len = 20;
static const sal_Int32 kTestStr114Len = 21;
static const sal_Int32 kTestStr115Len = 21;
static const sal_Int32 kTestStr116Len = 23;
static const sal_Int32 kTestStr117Len = 24;
static const sal_Int32 kTestStr118Len = 19;
static const sal_Int32 kTestStr119Len = 20;
static const sal_Int32 kTestStr120Len = 30;
static const sal_Int32 kTestStr121Len = 31;
static const sal_Int32 kTestStr122Len = 16;
static const sal_Int32 kTestStr123Len = 21;
static const sal_Int32 kTestStr124Len = 23;
static const sal_Int32 kTestStr125Len = 30;
static const sal_Int32 kTestStr126Len = 12;
static const sal_Int32 kTestStr127Len = 17;
static const sal_Int32 kTestStr128Len = 19;
static const sal_Int32 kTestStr129Len = 26;
static const sal_Int32 kTestStr130Len = 23;
static const sal_Int32 kTestStr131Len = 28;
static const sal_Int32 kTestStr132Len = 30;
static const sal_Int32 kTestStr133Len = 37;
static const sal_Int32 kTestStr134Len = 22;
static const sal_Int32 kTestStr135Len = 24;
static const sal_Int32 kTestStr136Len = 31;
static const sal_Int32 kTestStr137Len = 18;
static const sal_Int32 kTestStr138Len = 20;
static const sal_Int32 kTestStr139Len = 27;
static const sal_Int32 kTestStr140Len = 29;
static const sal_Int32 kTestStr141Len = 31;
static const sal_Int32 kTestStr142Len = 38;
static const sal_Int32 kTestStr143Len = 5;
static const sal_Int32 kTestStr144Len = 1;
static const sal_Int32 kTestStr145Len = 12;
static const sal_Int32 kTestStr146Len = 19;
static const sal_Int32 kTestStr147Len = 19;
static const sal_Int32 kTestStr148Len = 19;
static const sal_Int32 kTestStr149Len = 32;
static const sal_Int32 kTestStr150Len = 32;
static const sal_Int32 kTestStr151Len = 31;
static const sal_Int32 kTestStr152Len = 31;
static const sal_Int32 kTestStr153Len = 31;
static const sal_Int32 kTestStr154Len = 36;
static const sal_Int32 kTestStr155Len = 36;
static const sal_Int32 kTestStr156Len = 36;
static const sal_Int32 kTestStr157Len = 49;
static const sal_Int32 kTestStr158Len = 49;
static const sal_Int32 kTestStr159Len = 49;
static const sal_Int32 kTestStr160Len = 48;
static const sal_Int32 kTestStr161Len = 48;
static const sal_Int32 kTestStr162Len = 48;
static const sal_Int32 kTestStr163Len = 15;
static const sal_Int32 kTestStr164Len = 15;
static const sal_Int32 kTestStr165Len = 15;
static const sal_Int32 kTestStr166Len = 28;
static const sal_Int32 kTestStr167Len = 28;
static const sal_Int32 kTestStr168Len = 28;
static const sal_Int32 kTestStr169Len = 27;
static const sal_Int32 kTestStr170Len = 27;
static const sal_Int32 kTestStr171Len = 27;
static const sal_Int32 kTestStr1PlusStr6Len = kTestStr1Len + kTestStr6Len;

static const sal_Int32 uTestStr1Len  = 16;
static const sal_Int32 uTestStr2Len  = 32;
static const sal_Int32 uTestStr3Len  = 16;
static const sal_Int32 uTestStr4Len  = 16;
static const sal_Int32 uTestStr5Len  = 16;
static const sal_Int32 uTestStr9Len  = 32;
static const sal_Int32 uTestStr22Len = 32;

const sal_Unicode uTestStr31[]= {0x400,0x410,0x4DF};
const sal_Unicode uTestStr32[]= {0x9F9F,0xA000,0x8F80,0x9AD9};

static const sal_Int32 uTestStr31Len  = 3;
static const sal_Int32 uTestStr32Len  = 4;

static const sal_Int16 kRadixBinary     = 2;
static const sal_Int16 kRadixOctol      = 8;
static const sal_Int16 kRadixDecimal    = 10;
static const sal_Int16 kRadixHexdecimal = 16;
static const sal_Int16 kRadixBase36     = 36;

static const sal_Int8  kSInt8Max  = SCHAR_MAX;
static const sal_Int16 kUInt8Max  = UCHAR_MAX;
static const sal_Int16 kSInt16Max = SHRT_MAX;
static const sal_Int32 kUInt16Max = USHRT_MAX;
static const sal_Int32 kSInt32Max = INT_MAX;
static const sal_Int64 kUInt32Max = UINT_MAX;
static const sal_Int64 kSInt64Max = SAL_CONST_INT64(9223372036854775807);

//------------------------------------------------------------------------

static const sal_Int32 kInt32MaxNumsCount = 5;

static const sal_Int32 kInt32MaxNums[kInt32MaxNumsCount] =
                        {
                            kSInt8Max,  kUInt8Max,
                            kSInt16Max, kUInt16Max,
                            kSInt32Max
                        };

static const sal_Int32 kInt64MaxNumsCount = 7;

static const sal_Int64 kInt64MaxNums[kInt64MaxNumsCount] =
                        {
                            kSInt8Max,  kUInt8Max,
                            kSInt16Max, kUInt16Max,
                            kSInt32Max, kUInt32Max,
                            kSInt64Max
                        };

static const sal_Int32 kBinaryNumsCount = 16;

static const sal_Int32 kBinaryMaxNumsCount = 7;

//------------------------------------------------------------------------

static const sal_Int32 kOctolNumsCount = 16;

static const sal_Int32 kOctolMaxNumsCount = 7;

//------------------------------------------------------------------------

static const sal_Int32 kDecimalNumsCount = 16;

static const sal_Int32 kDecimalMaxNumsCount = 7;

//------------------------------------------------------------------------

static const sal_Int32 kHexDecimalNumsCount = 16;

static const sal_Int32 kHexDecimalMaxNumsCount = 7;

//------------------------------------------------------------------------

static const sal_Int32 kBase36NumsCount = 36;

static const sal_Int32 kBase36MaxNumsCount = 7;

static const sal_Int32 nDoubleCount=24;
static const double   expValDouble[nDoubleCount]=
    {
            3.0,3.1,3.1415,3.1415926535,3.141592653589793,
            3.1415926535897932,3.14159265358979323,3.1,
            3.141592653589793238462643,9.1096e-31,2.997925e8,6.241e18,5.381e18,
            1.7e-309,6.5822e-16,1.7e+307,2.2e30,3.1,3.1,-3.1,
            0.0,0.0,0.0,1.00e+308
    };
//------------------------------------------------------------------------
static const sal_Int32 nFloatCount=22;
static const float  expValFloat[nFloatCount] =
        {
            3.0f,3.1f,3.1415f,3.14159f,3.141592f,
            3.1415926f,3.14159265f,3.141592653589793238462643f,
            6.5822e-16f,9.1096e-31f,2.997925e8f,6.241e18f,
            1.00e38f,6.241e-37f,6.241e37f,3.1f,3.1f,-3.1f,
            3.1f,0.0f,0.0f,0.0f
        };
//------------------------------------------------------------------------
static const sal_Int32 nCharCount=15;
static const sal_Unicode  expValChar[nCharCount] =
        {
            65,97,48,45,95,
            21,27,29,
            64,10,39,34,
            0,0,83
        };
//------------------------------------------------------------------------
static const sal_Int32 nDefaultCount=6;
static const sal_Unicode input1Default[nDefaultCount] =
        {
            77,115,85,119,32,0
        };
static const sal_Int32 input2Default[nDefaultCount] =
        {
            0,0,0,0,0,0
        };
static const sal_Int32  expValDefault[nDefaultCount] =
        {
            4,9,-1,-1,3,-1
        };
//------------------------------------------------------------------------
static const sal_Int32 nNormalCount=10;
static const sal_Unicode input1Normal[nNormalCount] =
        {
            77,77,77,115,115,115,119,119,0,0
        };
static const sal_Int32 input2Normal[nNormalCount] =
        {
            0,32,80,0,13,20,0,80,0,32
        };
static const sal_Int32  expValNormal[nNormalCount] =
        {
            4,-1,-1,9,15,-1,-1,-1,-1,-1
        };
//------------------------------------------------------------------------
static const sal_Int32 nlastDefaultCount=5;
static const sal_Unicode input1lastDefault[nlastDefaultCount] =
        {
            77,115,119,32,0
        };
static const sal_Int32 input2lastDefault[nlastDefaultCount] =
        {
            31,31,31,31,31
        };
static const sal_Int32  expVallastDefault[nlastDefaultCount] =
        {
            4,15,-1,21,-1
        };
//------------------------------------------------------------------------
static const sal_Int32 nlastNormalCount=8;
static const sal_Unicode input1lastNormal[nlastNormalCount] =
        {
            77,77,77,115,115,119,119,0
        };
static const sal_Int32 input2lastNormal[nlastNormalCount] =
        {
            29,0,80,31,3,31,80,31
        };
static const sal_Int32  expVallastNormal[nlastNormalCount] =
        {
            4,-1,4,15,-1,-1,-1,-1
        };
//------------------------------------------------------------------------
static const sal_Int32 nStrDefaultCount=6;
static const sal_Int32 input2StrDefault[nStrDefaultCount] =
        {
            0,0,0,0,0,0
        };
static const sal_Int32 expValStrDefault[nStrDefaultCount] =
        {
            0,4,-1,-1,-1,3
        };
//------------------------------------------------------------------------
static const sal_Int32 nStrNormalCount=9;
static const sal_Int32 input2StrNormal[nStrNormalCount] =
        {
            0,32,0,30,0,0,0,32,0
        };
static const sal_Int32 expValStrNormal[nStrNormalCount] =
        {
            0,-1,4,-1,-1,-1,-1,-1,3
        };
//------------------------------------------------------------------------
static const sal_Int32 nStrLastDefaultCount=6;
static const sal_Int32 input2StrLastDefault[nStrLastDefaultCount] =
        {
            31,31,31,31,31,31
        };
static const sal_Int32  expValStrLastDefault[nStrLastDefaultCount] =
        {
            0,4,-1,-1,-1,3
        };
//------------------------------------------------------------------------
static const sal_Int32 nStrLastNormalCount=12;
static const sal_Int32 input2StrLastNormal[nStrLastNormalCount] =
        {
            31,0,80,31,2,31,31,31,0,31,31,14
        };
static const sal_Int32  expValStrLastNormal[nStrLastNormalCount] =
        {
            0,-1,0,4,-1,-1,-1,-1,-1,3,15,11
        };
//------------------------------------------------------------------------
static const sal_Int32 kNonSInt32Max = INT_MIN;
static const sal_Int32 kNonSInt16Max = SHRT_MIN;
//------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif /* _RTL_STRING_CONST_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
