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

#include <sal/types.h>
#include <testshl/tresstatewrapper.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.h>
#include <rtl/ustrbuf.hxx>
#include <osl/thread.h>
#include <rtl_String_Const.h>
#include <rtl_String_Utils.hxx>
#include "stdio.h"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

//------------------------------------------------------------------------
// test classes
//------------------------------------------------------------------------
const int MAXBUFLENGTH = 255;
//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------
static void unused()
{
    (void)kBinaryNumsStr;
    (void)kOctolNumsStr;
    (void)kDecimalNumsStr;
    (void)kHexDecimalNumsStr;
    (void)kBase36NumsStr;
    (void)inputChar;
    (void)input1StrDefault;
    (void)input1StrNormal;
    (void)input1StrLastDefault;
    (void)input1StrLastNormal;
    unused();
}

//------------------------------------------------------------------------
// testing constructors
//------------------------------------------------------------------------
static sal_Bool test_rtl_OUStringBuffer_ctor_001( hTestResult hRtlTestResult )
{

    ::rtl::OUStringBuffer aUStrBuf;

    bool b1 =
        aUStrBuf.getLength() == 0 &&
        ! *(aUStrBuf.getStr()) && aUStrBuf.getCapacity() == 16;

    ::rtl::OUStringBuffer aUStrBuf2(0);

    bool b2 =
        aUStrBuf2.getLength() == 0 &&
        ! *(aUStrBuf2.getStr()) && aUStrBuf2.getCapacity() == /* LLA: !!! */ 0;

    return
    (
        c_rtl_tres_state
        (
            hRtlTestResult,
            b1 && b2,
            "New OUStringBuffer containing no characters",
            "ctor_001"
        )
    );
}

//------------------------------------------------------------------------

static sal_Bool SAL_CALL test_rtl_OUStringBuffer_ctor_002(
                                               hTestResult hRtlTestResult )
{
    ::rtl::OUStringBuffer aUStrBuftmp( aUStr1 );
    ::rtl::OUStringBuffer aUStrBuf( aUStrBuftmp );
    sal_Bool res = cmpustr(aUStrBuftmp.getStr(),aUStrBuf.getStr());
    return
    (
        c_rtl_tres_state
        (
            hRtlTestResult,
            aUStrBuf.getLength()==aUStrBuftmp.getLength() &&
            aUStrBuf.getCapacity() == aUStrBuftmp.getCapacity() && res ,
            "New OUStringBuffer from another OUStringBuffer",
            "ctor_002"
        )
    );
}
//------------------------------------------------------------------------

/* static */
sal_Bool SAL_CALL test_rtl_OUStringBuffer_ctor_003(
                                               hTestResult hRtlTestResult )
{
    ::rtl::OUStringBuffer aUStrBuf1(kTestStr2Len);
    ::rtl::OUStringBuffer aUStrBuf2(0);
    ::rtl::OUStringBuffer aUStrBuf3(kNonSInt32Max);


    bool b1 =
        aUStrBuf1.getLength() == 0 &&
        ! *(aUStrBuf1.getStr()) && aUStrBuf1.getCapacity() == kTestStr2Len ;

    bool b2 =
        aUStrBuf2.getLength() == 0 &&
            ! *(aUStrBuf2.getStr()) && aUStrBuf2.getCapacity() == /* LLA: ??? 16 */ 0;

    bool b3 =
        aUStrBuf3.getLength() == 0 &&
        ! *(aUStrBuf3.getStr()) && aUStrBuf3.getCapacity() == kNonSInt32Max;

    return
        (
            c_rtl_tres_state
            (
                hRtlTestResult,
                b1 && b2 && b3,
                "New OUStringBuffer containing no characters and contain assigned capacity",
                "ctor_003( will core dump,because the kSInt32Max )"
                )
            );

}

//------------------------------------------------------------------------

static sal_Bool SAL_CALL test_rtl_OUStringBuffer_ctor_004(
                                               hTestResult hRtlTestResult)
{
    ::rtl::OUString aUStrtmp( aUStr1 );
    ::rtl::OUStringBuffer aUStrBuf( aUStrtmp );
    sal_Int32 leg = aUStrBuf.getLength();
    return
    (
        c_rtl_tres_state
        (
            hRtlTestResult,
            aUStrBuf.getStr() == aUStrtmp &&
            leg == aUStrtmp.pData->length &&
            aUStrBuf.getCapacity() == leg+16 ,
            "New OUStringBuffer from OUstring",
            "ctor_004"
        )
    );
}

static sal_Bool SAL_CALL test_rtl_OUStringBuffer_ctor_005(
                                               hTestResult hRtlTestResult)
{
    ::rtl::OUStringBuffer aUStrBuftmp( aUStr1 );
    ::rtl::OUString aUStrtmp = aUStrBuftmp.makeStringAndClear();
    ::rtl::OUStringBuffer aUStrBuf( aUStrBuftmp );
    sal_Bool res = cmpustr(aUStrBuftmp.getStr(),aUStrBuf.getStr());
    sal_Int32 leg = aUStrBuf.getLength();
    return
    (
        c_rtl_tres_state
        (
            hRtlTestResult,
            aUStrBuf.getLength()==aUStrBuftmp.getLength() &&
            aUStrBuf.getCapacity() == aUStrBuftmp.getCapacity() &&
            res && leg == 0,
            "New OUStringBuffer from another OUStringBuffer after makeClearFromString",
            "ctor_005"
        )
    );
}

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUStringBuffer_ctors(
                                              hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "ctors");
    sal_Bool DCState = test_ini_uString();
    (void)DCState;
    sal_Bool bTSState = test_rtl_OUStringBuffer_ctor_001( hRtlTestResult );
    bTSState &= test_rtl_OUStringBuffer_ctor_002( hRtlTestResult);
    bTSState &= test_rtl_OUStringBuffer_ctor_003( hRtlTestResult);
    bTSState &= test_rtl_OUStringBuffer_ctor_004( hRtlTestResult);
    bTSState &= test_rtl_OUStringBuffer_ctor_005( hRtlTestResult);

    c_rtl_tres_state_end( hRtlTestResult, "ctors");
//    return( bTSState );
}

//------------------------------------------------------------------------
// testing the method makeStringAndClear()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUStringBuffer_makeStringAndClear(
                                              hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "makeStringAndClear");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
    sal_Char*       comments;
    OUString*               expVal;
        OUStringBuffer*         input1;

    ~TestCase()     { delete input1;}
    } TestCase;

    OUString arrOUS[6]={
        OUString( aUStr1 ),
        OUString( aUStr14 ),
        OUString( aUStr25 ),
        OUString( aUStr27 ),
        OUString( aUStr29 ),
        OUString( "\0",1,
              kEncodingRTLTextUSASCII,
              kConvertFlagsOStringToOUString)
    };

    TestCase arrTestCase[]={

    {"two empty strings(def. constructor)", new OUString(),
                new OUStringBuffer()},
    {"two empty strings(with a argu)", new OUString(),
        new OUStringBuffer(26)},
    {"normal string", new OUString(arrOUS[0]),
                new OUStringBuffer(arrOUS[0])},
    {"string with space ", new OUString(arrOUS[1]),
            new OUStringBuffer(arrOUS[1])},
    {"empty string", new OUString(arrOUS[2]),
            new OUStringBuffer(arrOUS[2])},
    {"string with a character", new OUString(arrOUS[3]),
            new OUStringBuffer(arrOUS[3])},
    {"string with special characters", new OUString(arrOUS[4]),
                new OUStringBuffer(arrOUS[4])},
    {"string only with (\0)", new OUString(arrOUS[5]),
                new OUStringBuffer(arrOUS[5])}
    };

    sal_Bool res = sal_True;
    sal_uInt32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool lastRes =
                ( arrTestCase[i].input1->makeStringAndClear() ==
                                              *( arrTestCase[i].expVal ));
        lastRes = lastRes && ( arrTestCase[i].input1->getCapacity() == 0 );
        lastRes = lastRes && ( *(arrTestCase[i].input1->getStr()) == '\0' );

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "makeStringAndClear", i )
        );

        res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "makeStringAndClear");
//    return (res);
}
//------------------------------------------------------------------------
// testing the method getLength
//------------------------------------------------------------------------

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUStringBuffer_getLength(
                                              hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "getLength");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    OUString arrOUS[6]={OUString( aUStr1 ),
                        OUString( "1",1,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString(),
                        OUString( "",0,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( "\0",1,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( aUStr2 )};

    typedef struct TestCase
    {
    sal_Char*       comments;
    sal_Int32       expVal;
        OUStringBuffer*         input;
        ~TestCase()             { delete input;}
    } TestCase;

    TestCase arrTestCase[]={

    {"length of ascii string", kTestStr1Len,
            new OUStringBuffer(arrOUS[0]) },
        {"length of ascci string of size 1", 1,
                new OUStringBuffer(arrOUS[1])},
        {"length of empty string", 0,
                        new OUStringBuffer(arrOUS[2])},
    {"length of empty string (empty ascii string arg)",0,
            new OUStringBuffer(arrOUS[3])},
    {"length of empty string (string arg = \"\\0\")", 1,
            new OUStringBuffer(arrOUS[4])},
        {"length(>16) of ascii string", kTestStr2Len,
            new OUStringBuffer(arrOUS[5]) },
        {"length of empty string (default constructor)", 0,
                        new OUStringBuffer()},
        {"length of empty string (with capacity)", 0,
                        new OUStringBuffer(26)}
    };


    sal_Bool res = sal_True;
    sal_uInt32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
    sal_Int32 length = arrTestCase[i].input->getLength();
        sal_Bool lastRes = (length == arrTestCase[i].expVal);
        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "getLength", i )

        );
    res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "getLength");
//    return ( res );
}
//------------------------------------------------------------------------
// testing the method getCapacity()
//------------------------------------------------------------------------

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUStringBuffer_getCapacity(
                                              hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "getCapacity");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    OUString arrOUS[6]={OUString( aUStr1 ),
                        OUString( "1",1,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString(),
                        OUString( "",0,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( "\0",1,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( aUStr2 )};

    typedef struct TestCase
    {
    sal_Char*       comments;
    sal_Int32       expVal;
        OUStringBuffer*         input;
        ~TestCase()             { delete input;}
    } TestCase;

    TestCase arrTestCase[]={

    {"capacity of ascii string", kTestStr1Len+16,
            new OUStringBuffer(arrOUS[0]) },
        {"capacity of ascci string of size 1", 1+16,
                new OUStringBuffer(arrOUS[1]) },
        {"capacity of empty string", 0+16,
                        new OUStringBuffer(arrOUS[2]) },
    {"capacity of empty string (empty ascii string arg)",0+16,
            new OUStringBuffer(arrOUS[3]) },
    {"capacity of empty string (string arg = \"\\0\")", 1+16,
            new OUStringBuffer(arrOUS[4]) },
        {"capacity(>16) of ascii string", kTestStr2Len+16,
            new OUStringBuffer(arrOUS[5]) },
        {"capacity of empty string (default constructor)", 16,
                        new OUStringBuffer() },
        {"capacity of empty string (with capacity -2147483648)", kNonSInt32Max,
                        new OUStringBuffer(kNonSInt32Max) },
        {"capacity of empty string (with capacity 16)", 16,
                        new OUStringBuffer(16) },
        {"capacity of empty string (with capacity 6)", 6,
                        new OUStringBuffer(6) },
        {"capacity of empty string (with capacity 0)", 0,
                        new OUStringBuffer(0) },
        {"capacity of empty string (with capacity -2)", -2,
                        new OUStringBuffer(-2) }
    };


    sal_Bool res = sal_True;
    sal_uInt32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
    sal_Int32 length = arrTestCase[i].input->getCapacity();
        sal_Bool lastRes = (length == arrTestCase[i].expVal);
        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "getCapacity", i )

        );
    res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "getCapacity");
//    return ( res );
}
//------------------------------------------------------------------------
// testing the method ensureCapacity(sal_Int32 minimumCapacity)
//------------------------------------------------------------------------

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUStringBuffer_ensureCapacity(
                                              hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "ensureCapacity");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
    sal_Char*       comments;
    sal_Int32       expVal;
        OUStringBuffer*         input1;
        sal_Int32               input2;
        ~TestCase()             { delete input1;}
    } TestCase;

    TestCase arrTestCase[]={

    {"capacity equal to 16, minimum is 5 ", 16,
            new OUStringBuffer(), 5 },
        {"capacity equal to 16, minimum is -5", 16,
                new OUStringBuffer(), -5},
        {"capacity equal to 16, minimum is 0", 16,
                        new OUStringBuffer(), 0},
    {"capacity equal to 16, minimum is 20", 20, //the testcase is based on comments
            new OUStringBuffer(), 20},
    {"capacity equal to 16, minimum is 50", 50,
            new OUStringBuffer(), 50},
        {"capacity equal to 6, minimum is 20", 20,
            new OUStringBuffer(6), 20 },
        {"capacity equal to 6, minimum is 2", 6,
                        new OUStringBuffer(6), 2},
        {"capacity equal to 6, minimum is -6", 6,
                        new OUStringBuffer(6), -6},
        {"capacity equal to 6, minimum is -6", 10, //the testcase is based on comments
                        new OUStringBuffer(6), 10},
        {"capacity equal to 0, minimum is 6", 6,
                        new OUStringBuffer(0), 6},
        {"capacity equal to 0, minimum is 1", 2, //the testcase is based on comments
                        new OUStringBuffer(0), 1},
    /*
      {"capacity equal to 0, minimum is -1", 0,
                        new OUStringBuffer(0), -1},
    */
        {"capacity equal to -2147483648, minimum is 65535", 65535,
                        new OUStringBuffer(kNonSInt32Max), 65535},
        {"capacity equal to -2147483648, minimum is -1", 2,
                        new OUStringBuffer(kNonSInt32Max), -1},
        {"capacity equal to -2147483648, minimum is 0", 2,
                        new OUStringBuffer(kNonSInt32Max), 0},
        {"capacity equal to -2147483648, minimum is -2147483648", kNonSInt32Max,
                        new OUStringBuffer(kNonSInt32Max), kNonSInt32Max}
     };


    sal_Bool res = sal_True;
    sal_uInt32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
    arrTestCase[i].input1->ensureCapacity(arrTestCase[i].input2);
        sal_Int32 length = arrTestCase[i].input1->getCapacity();
        sal_Bool lastRes = (length == arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "ensureCapacity", i )

        );
    res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "ensureCapacity");
//    return ( res );
}
//------------------------------------------------------------------------
// testing the method setLength(sal_Int32 newLength)
//------------------------------------------------------------------------

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUStringBuffer_setLength(
                                              hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "setLength");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    OUString arrOUS[6]={OUString( aUStr1 ),
                        OUString( aUStr27),
                        OUString(),
                        OUString( "",0,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( "\0",1,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( aUStr2 )};

    typedef struct TestCase
    {
    sal_Char*       comments;
    sal_Int32       expVal1;
        OUString*               expVal2;
    sal_Int32       expVal3;
        OUStringBuffer*         input1;
        sal_Int32               input2;
        ~TestCase()             { delete input1; delete expVal2;}
    } TestCase;

    TestCase arrTestCase[]={

    {"newLength more than the capacity of OUStringBuffer(aUStr1)",
                50, new OUString(aUStr1), 50,
                new OUStringBuffer(arrOUS[0]), 50 },
    {"newLength more than the length of OUStringBuffer(aUStr1)",
                kTestStr13Len, new OUString(aUStr1), 32,
                new OUStringBuffer(arrOUS[0]), kTestStr13Len },
        {"newLength equal to the length of OUStringBuffer(aUStr1)",
                kTestStr1Len, new OUString(aUStr1), 32,
                new OUStringBuffer(arrOUS[0]), kTestStr1Len },
        {"newLength less than the length of OUStringBuffer(aUStr1)",
                kTestStr7Len, new OUString(aUStr7), 32,
                new OUStringBuffer(arrOUS[0]), kTestStr7Len},
        {"newLength equal to 0",
                0, new OUString(), 32,
                new OUStringBuffer(arrOUS[0]), 0},
    {"newLength more than the capacity of OUStringBuffer(1)",
                25, new OUString(arrOUS[1]), 25,
                new OUStringBuffer(arrOUS[1]), 25},
    {"newLength more than the length of OUStringBuffer(1)",
                5, new OUString(arrOUS[1]), 17,
                new OUStringBuffer(arrOUS[1]), 5},
    {"newLength equal to the length of OUStringBuffer(1)",
                kTestStr27Len, new OUString(arrOUS[1]), 17,
                new OUStringBuffer(arrOUS[1]), kTestStr27Len},
    {"newLength less than the length of OUStringBuffer(1)",
                0, new OUString(), 17,
                new OUStringBuffer(arrOUS[1]), 0},
    {"newLength more than the capacity of OUStringBuffer()",
                20, new OUString(), 20,
                new OUStringBuffer(arrOUS[2]), 20},
    {"newLength more than the length of OUStringBuffer()",
                3, new OUString(), 16,
                new OUStringBuffer(arrOUS[2]), 3},
    {"newLength less than the length of OUStringBuffer()",
                0, new OUString(), 16,
                new OUStringBuffer(arrOUS[2]), 0},
    {"newLength more than the capacity of OUStringBuffer("")",
                20, new OUString(), 20,
                new OUStringBuffer(arrOUS[3]), 20},
    {"newLength more than the length of OUStringBuffer("")",
                5, new OUString(), 16,
                new OUStringBuffer(arrOUS[3]), 5},
    {"newLength less than the length of OUStringBuffer("")",
                0, new OUString(), 16,
                new OUStringBuffer(arrOUS[3]), 0},
    {"newLength more than the length of OUStringBuffer(\0)",
                20, new OUString(), 20,
                new OUStringBuffer(arrOUS[4]), 20},
    {"newLength more than the length of OUStringBuffer(\0)",
                5, new OUString(), 16,
                new OUStringBuffer(arrOUS[4]), 5},
    {"newLength less than the length of OUStringBuffer(\0)",
                0, new OUString(), 16,
                new OUStringBuffer(arrOUS[4]), 0},
    {"newLength more than the capacity of OUStringBuffer(aUStr2)",
                50, new OUString(aUStr2), 66,
                new OUStringBuffer(arrOUS[5]), 50,},
    {"newLength more than the length of OUStringBuffer(aUStr2)",
                40, new OUString(aUStr2), 48,
                new OUStringBuffer(arrOUS[5]), 40,},
    {"newLength equal to the length of OUStringBuffer(aUStr2)",
                kTestStr2Len, new OUString(aUStr2), 48,
                new OUStringBuffer(arrOUS[5]), kTestStr2Len,},
        {"newLength less than the length of OUStringBuffer(aUStr2)",
                kTestStr7Len, new OUString(aUStr7), 48,
                new OUStringBuffer(arrOUS[5]), kTestStr7Len},
        {"newLength equal to 0",
                0, new OUString(), 48,
                new OUStringBuffer(arrOUS[5]), 0}

    };


    sal_Bool res = sal_True;
    sal_uInt32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
    arrTestCase[i].input1->setLength(arrTestCase[i].input2);
        sal_Bool lastRes =
            ( arrTestCase[i].input1->getStr() == *(arrTestCase[i].expVal2) &&
              arrTestCase[i].input1->getLength() == arrTestCase[i].expVal1 &&
              arrTestCase[i].input1->getCapacity() == arrTestCase[i].expVal3 );

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "setLength", i )

        );
    res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "setLength");
//    return ( res );
}
//------------------------------------------------------------------------
// testing the operator const sal_Unicode * (csuc for short)
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUStringBuffer_csuc(
                                              hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "csuc");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    const sal_Unicode tmpUC=0x0;
    rtl_uString* tmpUstring = NULL;
    const sal_Char *tmpStr=kTestStr1;
    sal_Int32 tmpLen=(sal_Int32) kTestStr1Len;
    //sal_Int32 cmpLen = 0;
        OUString tempString(aUStr1);

    rtl_string2UString( &tmpUstring, tmpStr,  tmpLen,
        osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
    OSL_ASSERT(tmpUstring != NULL);


    typedef struct TestCase
        {
            sal_Char*                  comments;
            const sal_Unicode*             expVal;
            sal_Int32                      cmpLen;
            OUStringBuffer*                input1;
            ~TestCase()                    {  delete input1; }
     } TestCase;

    TestCase arrTestCase[] =
    {
          {"test normal ustring",(*tmpUstring).buffer,kTestStr1Len,
                      new OUStringBuffer(tempString)},
        {"test empty ustring",&tmpUC, 1, new OUStringBuffer()}
    };

    sal_Bool res = sal_True;
    sal_uInt32 i;
    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
       const sal_Unicode* pstr = *arrTestCase[i].input1;

       res &= c_rtl_tres_state
       (
            hRtlTestResult,
            cmpustr( pstr, arrTestCase[i].expVal, arrTestCase[i].cmpLen ),
            arrTestCase[i].comments,
            createName( pMeth, "const sal_Unicode*", i )
       );
    }
    c_rtl_tres_state_end( hRtlTestResult, "csuc");
//    return ( res );
}
//------------------------------------------------------------------------
// testing the method const sal_Unicode * getStr()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUStringBuffer_getStr(
                                              hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "getStr");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    const sal_Unicode tmpUC=0x0;
    rtl_uString* tmpUstring = NULL;
    const sal_Char *tmpStr=kTestStr1;
    sal_Int32 tmpLen=(sal_Int32) kTestStr1Len;
    //sal_Int32 cmpLen = 0;
        OUString tempString(aUStr1);

    rtl_string2UString( &tmpUstring, tmpStr,  tmpLen,
        osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
    OSL_ASSERT(tmpUstring != NULL);


    typedef struct TestCase
    {
        sal_Char*           comments;
        const sal_Unicode*      expVal;
        sal_Int32           cmpLen;
        OUStringBuffer*                 input1;
        ~TestCase()                     {  delete input1;}
    } TestCase;

    TestCase arrTestCase[] =
    {
          {"test normal ustring",(*tmpUstring).buffer,kTestStr1Len,
                      new OUStringBuffer(tempString)},
        {"test empty ustring",&tmpUC, 1, new OUStringBuffer()}
    };

    sal_Bool res = sal_True;
    sal_uInt32 i;
    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
       const sal_Unicode* pstr = arrTestCase[i].input1->getStr();

       res &= c_rtl_tres_state
       (
            hRtlTestResult,
            cmpustr( pstr, arrTestCase[i].expVal, arrTestCase[i].cmpLen ),
            arrTestCase[i].comments,
            createName( pMeth, "getStr", i )
       );
    }
    c_rtl_tres_state_end( hRtlTestResult, "getStr");
//    return ( res );
}
//------------------------------------------------------------------------
// testing the method append(const OUString &str)
//------------------------------------------------------------------------

sal_Bool SAL_CALL test_rtl_OUStringBuffer_append_001(
                                              hTestResult hRtlTestResult)
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    OUString arrOUS[5]={OUString( aUStr7 ),
                        OUString(),
                        OUString( aUStr25 ),
                        OUString( "\0",1,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( aUStr28 )};

    typedef struct TestCase
    {
    sal_Char*       comments;
    OUString*               expVal;
        OUStringBuffer*         input1;
        OUString*               input2;

        ~TestCase()             { delete input1; delete input2; delete expVal; }
    } TestCase;

    TestCase arrTestCase[]={

    {"Appends the string(length less than 16) to the string buffer arrOUS[0]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[0]), new OUString(aUStr8) },
    {"Appends the string(length more than 16) to the string buffer arrOUS[0]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[0]), new OUString(aUStr36) },
        {"Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                new OUString(aUStr37),
                new OUStringBuffer(arrOUS[0]), new OUString(aUStr23) },
    {"Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[0]), new OUString()},
    {"Appends the string(length less than 16) to the string buffer arrOUS[1]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[1]), new OUString(aUStr7)},
        {"Appends the string(length more than 16) to the string buffer arrOUS[1]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[1]), new OUString(aUStr2)},
    {"Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[1]), new OUString(aUStr1) },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                new OUString(),
                new OUStringBuffer(arrOUS[1]), new OUString()},
        {"Appends the string(length less than 16) to the string buffer arrOUS[2]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[2]), new OUString(aUStr7)},
        {"Appends the string(length more than 16) to the string buffer arrOUS[2]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[2]), new OUString(aUStr2)},
    {"Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[2]), new OUString(aUStr1) },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                new OUString(),
                new OUStringBuffer(arrOUS[2]), new OUString()},
        {"Appends the string(length less than 16) to the string buffer arrOUS[3]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[3]), new OUString(aUStr7)},
        {"Appends the string(length more than 16) to the string buffer arrOUS[3]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[3]), new OUString(aUStr2)},
    {"Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[3]), new OUString(aUStr1) },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                new OUString(),
                new OUStringBuffer(arrOUS[3]), new OUString()},
        {"Appends the string(length less than 16) to the string buffer arrOUS[4]",
                new OUString(aUStr29),
                new OUStringBuffer(arrOUS[4]), new OUString(aUStr38)},
        {"Appends the string(length more than 16) to the string buffer arrOUS[4]",
                new OUString(aUStr39),
                new OUStringBuffer(arrOUS[4]), new OUString(aUStr17)},
    {"Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                new OUString(aUStr40),
                new OUStringBuffer(arrOUS[4]), new OUString(aUStr31) },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                new OUString(aUStr28),
                new OUStringBuffer(arrOUS[4]), new OUString()}
    };


    sal_Bool res = sal_True;
    sal_uInt32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        arrTestCase[i].input1->append( *(arrTestCase[i].input2) );
        sal_Bool lastRes =
            ( arrTestCase[i].input1->getStr()== *(arrTestCase[i].expVal) &&
              arrTestCase[i].input1->getLength() == arrTestCase[i].expVal->getLength()  );

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "append(const OUString &str)_001", i )

        );

        res &= lastRes;
    }

    return ( res );
}
//------------------------------------------------------------------------
// testing the method append( const sal_Unicode * str )
//------------------------------------------------------------------------

sal_Bool SAL_CALL test_rtl_OUStringBuffer_append_002(
                                              hTestResult hRtlTestResult)
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    OUString arrOUS[5]={OUString( aUStr7 ),
                        OUString(),
                        OUString( aUStr25 ),
                        OUString( "\0",1,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( aUStr28 )};

    typedef struct TestCase
    {
    sal_Char*       comments;
    OUString*               expVal;
        OUStringBuffer*         input1;
        sal_Unicode*            input2;

        ~TestCase()             { delete input1; delete expVal; }
    } TestCase;

    TestCase arrTestCase[]={

    {"Appends the string(length less than 16) to the string buffer arrOUS[0]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[0]), aUStr8 },
    {"Appends the string(length more than 16) to the string buffer arrOUS[0]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[0]), aUStr36 },
        {"Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                new OUString(aUStr37),
                new OUStringBuffer(arrOUS[0]), aUStr23 },
    {"Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[0]), aUStr25 },
    {"Appends the string(length less than 16) to the string buffer arrOUS[1]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[1]), aUStr7 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[1]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[1]), aUStr2 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[1]), aUStr1 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                new OUString(),
                new OUStringBuffer(arrOUS[1]), aUStr25 },
        {"Appends the string(length less than 16) to the string buffer arrOUS[2]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[2]), aUStr7 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[2]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[2]), aUStr2 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[2]), aUStr1 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                new OUString(),
                new OUStringBuffer(arrOUS[2]), aUStr25 },
        {"Appends the string(length less than 16) to the string buffer arrOUS[3]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[3]), aUStr7 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[3]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[3]), aUStr2 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[3]), aUStr1 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                new OUString(),
                new OUStringBuffer(arrOUS[3]), aUStr25 },
        {"Appends the string(length less than 16) to the string buffer arrOUS[4]",
                new OUString(aUStr29),
                new OUStringBuffer(arrOUS[4]), aUStr38 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[4]",
                new OUString(aUStr39),
                new OUStringBuffer(arrOUS[4]), aUStr17 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                new OUString(aUStr40),
                new OUStringBuffer(arrOUS[4]), aUStr31 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                new OUString(aUStr28),
                new OUStringBuffer(arrOUS[4]), aUStr25 }
    };


    sal_Bool res = sal_True;
    sal_uInt32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        arrTestCase[i].input1->append( arrTestCase[i].input2 );
        sal_Bool lastRes =
            ( arrTestCase[i].input1->getStr()== *(arrTestCase[i].expVal) &&
              arrTestCase[i].input1->getLength() == arrTestCase[i].expVal->getLength()  );

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "append( const sal_Unicode * str )_002", i )

        );

        res &= lastRes;
    }

    return ( res );
}
//------------------------------------------------------------------------
// testing the method append( const sal_Unicode * str, sal_Int32 len)
//------------------------------------------------------------------------

sal_Bool SAL_CALL test_rtl_OUStringBuffer_append_003(
                                              hTestResult hRtlTestResult)
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    OUString arrOUS[5]={OUString( aUStr7 ),
                        OUString(),
                        OUString( aUStr25 ),
                        OUString( "\0",1,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( aUStr28 )};

    typedef struct TestCase
    {
    sal_Char*       comments;
    OUString*               expVal;
        OUStringBuffer*         input1;
        sal_Unicode*            input2;
        sal_Int32               input3;

        ~TestCase()             { delete input1; delete expVal; }
    } TestCase;

    TestCase arrTestCase[]={

    {"Appends the string(length less than 16) to the string buffer arrOUS[0]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[0]), aUStr36, 12 },
    {"Appends the string(length more than 16) to the string buffer arrOUS[0]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[0]), aUStr36, 28 },
        {"Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                new OUString(aUStr37),
                new OUStringBuffer(arrOUS[0]), aUStr23, 16 },
    {"Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[0]), aUStr2, 0 },
    /* LLA: input3 must null < 0
        {"Appends the string(length less than 0) to the string buffer arrOUS[0]",
                new OUString(aUStr41),
                new OUStringBuffer(arrOUS[0]), aUStr2, -1 },
    */
    {"Appends the string(length less than 16) to the string buffer arrOUS[1]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[1]), aUStr2, 4 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[1]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[1]), aUStr2, 32 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[1]), aUStr2, 16 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                new OUString(),
                new OUStringBuffer(arrOUS[1]), aUStr2, 0 },
    /* LLA: input3 must null < 0
        {"Appends the string(length less than 0) to the string buffer arrOUS[1]",
                new OUString(),
                new OUStringBuffer(arrOUS[1]), aUStr2, -1 },
    */
        {"Appends the string(length less than 16) to the string buffer arrOUS[2]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[2]), aUStr2, 4 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[2]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[2]), aUStr2, 32 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[2]), aUStr2, 16 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                new OUString(),
                new OUStringBuffer(arrOUS[2]), aUStr2, 0 },
    /* LLA: input3 must null < 0
        {"Appends the string(length less than 0) to the string buffer arrOUS[2]",
                new OUString(),
                new OUStringBuffer(arrOUS[2]), aUStr2, -1 },
    */
        {"Appends the string(length less than 16) to the string buffer arrOUS[3]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[3]), aUStr2, 4 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[3]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[3]), aUStr2, 32 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[3]), aUStr2, 16 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                new OUString(),
                new OUStringBuffer(arrOUS[3]), aUStr2, 0 },
    /* LLA: input3 must null < 0
        {"Appends the string(length less than 0) to the string buffer arrOUS[3]",
                new OUString(),
                new OUStringBuffer(arrOUS[3]), aUStr2, -1 },
    */
        {"Appends the string(length less than 16) to the string buffer arrOUS[4]",
                new OUString(aUStr29),
                new OUStringBuffer(arrOUS[4]), aUStr38, 7 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[4]",
                new OUString(aUStr39),
                new OUStringBuffer(arrOUS[4]), aUStr17, 22 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                new OUString(aUStr40),
                new OUStringBuffer(arrOUS[4]), aUStr31, 16 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                new OUString(aUStr28),
                new OUStringBuffer(arrOUS[4]), aUStr2, 0 },
    };


    sal_Bool res = sal_True;
    sal_uInt32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        arrTestCase[i].input1->append(
                    arrTestCase[i].input2, arrTestCase[i].input3);
        sal_Bool lastRes =
            ( arrTestCase[i].input1->getStr()== *(arrTestCase[i].expVal) &&
              arrTestCase[i].input1->getLength() == arrTestCase[i].expVal->getLength()  );

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "append( const sal_Unicode * str, sal_Int32 len)_003", i )

        );

        res &= lastRes;
    }

    return ( res );
}
//------------------------------------------------------------------------
// testing the method append(sal_Bool b)
//------------------------------------------------------------------------

sal_Bool SAL_CALL test_rtl_OUStringBuffer_append_004(
                                              hTestResult hRtlTestResult)
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    OUString arrOUS[5]={OUString( aUStr7 ),
                        OUString(),
                        OUString( aUStr25 ),
                        OUString( "\0",1,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( aUStr28 )};

    typedef struct TestCase
    {
    sal_Char*       comments;
    OUString*               expVal;
        OUStringBuffer*         input1;
        sal_Bool                input2;

        ~TestCase()             { delete input1; delete expVal; }
    } TestCase;

    TestCase arrTestCase[]={

    {"Appends the sal_Bool(sal_True) to the string buffer arrOUS[0]",
                new OUString(aUStr45),
                new OUStringBuffer(arrOUS[0]), sal_True },
    {"Appends the sal_Bool(sal_False) to the string buffer arrOUS[0]",
                new OUString(aUStr46),
                new OUStringBuffer(arrOUS[0]), sal_False },
    {"Appends the sal_Bool(sal_True) to the string buffer arrOUS[1]",
                new OUString(aUStr47),
                new OUStringBuffer(arrOUS[1]), sal_True },
    {"Appends the sal_Bool(sal_False) to the string buffer arrOUS[1]",
                new OUString(aUStr48),
                new OUStringBuffer(arrOUS[1]), sal_False },
    {"Appends the sal_Bool(sal_True) to the string buffer arrOUS[2]",
                new OUString(aUStr47),
                new OUStringBuffer(arrOUS[2]), sal_True },
    {"Appends the sal_Bool(sal_False) to the string buffer arrOUS[2]",
                new OUString(aUStr48),
                new OUStringBuffer(arrOUS[2]), sal_False },
    {"Appends the sal_Bool(sal_True) to the string buffer arrOUS[3]",
                new OUString(aUStr47),
                new OUStringBuffer(arrOUS[3]), sal_True },
    {"Appends the sal_Bool(sal_False) to the string buffer arrOUS[3]",
                new OUString(aUStr48),
                new OUStringBuffer(arrOUS[3]), sal_False },
    {"Appends the sal_Bool(sal_True) to the string buffer arrOUS[4]",
                new OUString(aUStr49),
                new OUStringBuffer(arrOUS[4]), sal_True },
    {"Appends the sal_Bool(sal_False) to the string buffer arrOUS[4]",
                new OUString(aUStr50),
                new OUStringBuffer(arrOUS[4]), sal_False }
    };


    sal_Bool res = sal_True;
    sal_uInt32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        arrTestCase[i].input1->append(
                    arrTestCase[i].input2 );
        sal_Bool lastRes =
            ( arrTestCase[i].input1->getStr()== *(arrTestCase[i].expVal) &&
              arrTestCase[i].input1->getLength() == arrTestCase[i].expVal->getLength()  );

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "append( sal_Bool b)_004", i )

        );

        res &= lastRes;
    }

    return ( res );
}

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUStringBuffer_appends(
                                              hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "appends");
    sal_Bool bTSState = test_rtl_OUStringBuffer_append_001( hRtlTestResult );
    bTSState &= test_rtl_OUStringBuffer_append_002( hRtlTestResult);
    bTSState &= test_rtl_OUStringBuffer_append_003( hRtlTestResult);
    bTSState &= test_rtl_OUStringBuffer_append_004( hRtlTestResult);

    c_rtl_tres_state_end( hRtlTestResult, "appends");
//    return( bTSState );
}
//------------------------------------------------------------------------
// testing the method appendAscii( const sal_Char * str )
//------------------------------------------------------------------------

sal_Bool SAL_CALL test_rtl_OUStringBuffer_appendAscii_001(
                                              hTestResult hRtlTestResult)
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    OUString arrOUS[5]={OUString( aUStr7 ),
                        OUString(),
                        OUString( aUStr25 ),
                        OUString( "\0",1,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( aUStr28 )};

    typedef struct TestCase
    {
    sal_Char*       comments;
    OUString*               expVal;
        OUStringBuffer*         input1;
        const sal_Char*         input2;

        ~TestCase()             { delete input1; delete expVal; }
    } TestCase;

    TestCase arrTestCase[]={

        {"Appends the string(length less than 16) to the string buffer arrOUS[0]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[0]), kTestStr8 },
    {"Appends the string(length more than 16) to the string buffer arrOUS[0]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[0]), kTestStr36 },
        {"Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                new OUString(aUStr37),
                new OUStringBuffer(arrOUS[0]), kTestStr23 },
    {"Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[0]), kTestStr25 },
    {"Appends the string(length less than 16) to the string buffer arrOUS[1]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[1]), kTestStr7 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[1]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[1]), kTestStr2 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[1]), kTestStr1 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                new OUString(),
                new OUStringBuffer(arrOUS[1]), kTestStr25 },
        {"Appends the string(length less than 16) to the string buffer arrOUS[2]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[2]), kTestStr7 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[2]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[2]), kTestStr2 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[2]), kTestStr1 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                new OUString(),
                new OUStringBuffer(arrOUS[2]), kTestStr25 },
        {"Appends the string(length less than 16) to the string buffer arrOUS[3]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[3]), kTestStr7 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[3]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[3]), kTestStr2 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[3]), kTestStr1 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                new OUString(),
                new OUStringBuffer(arrOUS[3]), kTestStr25 },
        {"Appends the string(length less than 16) to the string buffer arrOUS[4]",
                new OUString(aUStr29),
                new OUStringBuffer(arrOUS[4]), kTestStr38 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[4]",
                new OUString(aUStr39),
                new OUStringBuffer(arrOUS[4]), kTestStr17 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                new OUString(aUStr40),
                new OUStringBuffer(arrOUS[4]), kTestStr31 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                new OUString(aUStr28),
                new OUStringBuffer(arrOUS[4]), kTestStr25 }
    };


    sal_Bool res = sal_True;
    sal_uInt32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        arrTestCase[i].input1->appendAscii( arrTestCase[i].input2 );
        sal_Bool lastRes =
            ( arrTestCase[i].input1->getStr()== *(arrTestCase[i].expVal) &&
              arrTestCase[i].input1->getLength() == arrTestCase[i].expVal->getLength()  );

            c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                arrTestCase[i].comments,
                createName( pMeth, "appendAscii_001", i )
            );

        res &= lastRes;
    }
    return ( res );
}
//------------------------------------------------------------------------
// testing the method appendAscii( const sal_Char * str, sal_Int32 len)
//------------------------------------------------------------------------

sal_Bool SAL_CALL test_rtl_OUStringBuffer_appendAscii_002(
                                              hTestResult hRtlTestResult)
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    OUString arrOUS[5]={OUString( aUStr7 ),
                        OUString(),
                        OUString( aUStr25 ),
                        OUString( "\0",1,
                    kEncodingRTLTextUSASCII,
                    kConvertFlagsOStringToOUString),
                        OUString( aUStr28 )};

    typedef struct TestCase
    {
    sal_Char*       comments;
    OUString*               expVal;
        OUStringBuffer*         input1;
        const sal_Char*         input2;
        sal_Int32               input3;

        ~TestCase()             { delete input1; delete expVal; }
    } TestCase;

    TestCase arrTestCase[]={

    {"Appends the string(length less than 16) to the string buffer arrOUS[0]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[0]), kTestStr36, 12 },
    {"Appends the string(length more than 16) to the string buffer arrOUS[0]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[0]), kTestStr36, 28 },
        {"Appends the string(length equal to 16) to the string buffer arrOUS[0]",
                new OUString(aUStr37),
                new OUStringBuffer(arrOUS[0]), kTestStr23, 16 },
    {"Appends the string(length equal to 0) to the string buffer arrOUS[0]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[0]), kTestStr2, 0 },
    /* LLA: input3 must null < 0
        {"Appends the string(length less than 0) to the string buffer arrOUS[0]",
                new OUString(aUStr41),
                new OUStringBuffer(arrOUS[0]), kTestStr2, -1 },
    */
    {"Appends the string(length less than 16) to the string buffer arrOUS[1]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[1]), kTestStr2, 4 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[1]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[1]), kTestStr2, 32 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[1]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[1]), kTestStr2, 16 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[1]",
                new OUString(),
                new OUStringBuffer(arrOUS[1]), kTestStr2, 0 },
    /* LLA: input3 must null < 0
        {"Appends the string(length less than 0) to the string buffer arrOUS[1]",
                new OUString(),
                new OUStringBuffer(arrOUS[1]), kTestStr2, -1 },
    */
        {"Appends the string(length less than 16) to the string buffer arrOUS[2]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[2]), kTestStr2, 4 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[2]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[2]), kTestStr2, 32 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[2]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[2]), kTestStr2, 16 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[2]",
                new OUString(),
                new OUStringBuffer(arrOUS[2]), kTestStr2, 0 },
    /* LLA: input3 must null < 0
       {"Appends the string(length less than 0) to the string buffer arrOUS[2]",
                new OUString(),
                new OUStringBuffer(arrOUS[2]), kTestStr2, -1 },
    */
        {"Appends the string(length less than 16) to the string buffer arrOUS[3]",
                new OUString(aUStr7),
                new OUStringBuffer(arrOUS[3]), kTestStr2, 4 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[3]",
                new OUString(aUStr2),
                new OUStringBuffer(arrOUS[3]), kTestStr2, 32 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[3]",
                new OUString(aUStr1),
                new OUStringBuffer(arrOUS[3]), kTestStr2, 16 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[3]",
                new OUString(),
                new OUStringBuffer(arrOUS[3]), kTestStr2, 0 },
    /* LLA: input3 must null < 0
        {"Appends the string(length less than 0) to the string buffer arrOUS[3]",
                new OUString(),
                new OUStringBuffer(arrOUS[3]), kTestStr2, -1 },
    */
        {"Appends the string(length less than 16) to the string buffer arrOUS[4]",
                new OUString(aUStr29),
                new OUStringBuffer(arrOUS[4]), kTestStr38, 7 },
        {"Appends the string(length more than 16) to the string buffer arrOUS[4]",
                new OUString(aUStr39),
                new OUStringBuffer(arrOUS[4]), kTestStr17, 22 },
    {"Appends the string(length equal to 16) to the string buffer arrOUS[4]",
                new OUString(aUStr40),
                new OUStringBuffer(arrOUS[4]), kTestStr31, 16 },
        {"Appends the string(length equal to 0) to the string buffer arrOUS[4]",
                new OUString(aUStr28),
                new OUStringBuffer(arrOUS[4]), kTestStr2, 0 },
    };


    sal_Bool res = sal_True;
    sal_uInt32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        arrTestCase[i].input1->appendAscii(
                    arrTestCase[i].input2, arrTestCase[i].input3);
        sal_Bool lastRes =
            ( arrTestCase[i].input1->getStr()== *(arrTestCase[i].expVal) &&
              arrTestCase[i].input1->getLength() == arrTestCase[i].expVal->getLength()  );

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "appendAscii_002", i )

        );

        res &= lastRes;
    }
    return ( res );
}
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUStringBuffer_appendAsciis(
                                              hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "appendAsciis");
    sal_Bool bTSState = test_rtl_OUStringBuffer_appendAscii_001( hRtlTestResult );
    bTSState &= test_rtl_OUStringBuffer_appendAscii_002( hRtlTestResult);

    c_rtl_tres_state_end( hRtlTestResult, "appendAsciis");
//    return( bTSState );
}
// -----------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUStringBuffer( hTestResult hRtlTestResult )
{

    c_rtl_tres_state_start(hRtlTestResult, "rtl_OUStringBuffer" );

    test_rtl_OUStringBuffer_ctors( hRtlTestResult );
    test_rtl_OUStringBuffer_makeStringAndClear( hRtlTestResult );
    test_rtl_OUStringBuffer_getLength( hRtlTestResult );
    test_rtl_OUStringBuffer_getCapacity( hRtlTestResult );
    test_rtl_OUStringBuffer_ensureCapacity( hRtlTestResult );
    test_rtl_OUStringBuffer_setLength( hRtlTestResult );
    test_rtl_OUStringBuffer_csuc( hRtlTestResult );
    test_rtl_OUStringBuffer_getStr( hRtlTestResult );
    test_rtl_OUStringBuffer_appends( hRtlTestResult );
    test_rtl_OUStringBuffer_appendAsciis( hRtlTestResult );

    c_rtl_tres_state_end(hRtlTestResult, "rtl_OUStringBuffer");
}

// -----------------------------------------------------------------------------
void RegisterAdditionalFunctions(FktRegFuncPtr _pFunc)
{
    if (_pFunc)
    {
        (_pFunc)(&test_rtl_OUStringBuffer, "");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
