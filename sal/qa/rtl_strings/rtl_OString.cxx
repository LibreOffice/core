/*************************************************************************
 *
 *  $RCSfile: rtl_OString.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 16:43:55 $
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
#include <string.h>

#ifndef _SAL_TYPES_H_
    #include <sal/types.h>
#endif

// #ifndef _RTL_TRES_H_
//     #include <rtl/tres.h>
// #endif

#include <testshl/tresstatewrapper.hxx>

#ifndef _RTL_STRING_HXX_
    #include <rtl/string.hxx>
#endif

#ifndef _RTL_STRING_CONST_H_
    #include <rtl_String_Const.h>
#endif

#ifndef _RTL_STRING_UTILS_HXX_
    #include <rtl_String_Utils.hxx>
#endif
#include <rtl/ustring.h>

using namespace rtl;

//------------------------------------------------------------------------
// test classes
//------------------------------------------------------------------------
const MAXBUFLENGTH = 255;
//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------


//------------------------------------------------------------------------
// testing constructors
//------------------------------------------------------------------------
static sal_Bool test_rtl_OString_ctor_001( hTestResult hRtlTestResult )
{
    ::rtl::OString aStr;
    rtl_String* pData = aStr.pData;


    return
    (
        c_rtl_tres_state
        (
            hRtlTestResult,
            pData->length == 0 &&
            ! *pData->buffer,
            "New OString containing no characters",
            "ctor_001"
        )
    );
}

//------------------------------------------------------------------------

static sal_Bool SAL_CALL test_rtl_OString_ctor_002(
                                         hTestResult hRtlTestResult )
{
    ::rtl::OString aStr(kTestStr1);
    rtl_String* pData = aStr.pData;

    return
    (
        c_rtl_tres_state
        (
            hRtlTestResult,
            pData->refCount == 1 &&
            pData->length == kTestStr1Len &&
            cmpstr( (const sal_Char*)pData->buffer, kTestStr1, kTestStr1Len ),
            "New OString from a character buffer array",
            "ctor_002"
        )
    );
}
//------------------------------------------------------------------------

static sal_Bool SAL_CALL test_rtl_OString_ctor_003(
                                                hTestResult hRtlTestResult  )
{
    ::rtl::OString aStr(kTestStr2, kTestStr1Len);
    rtl_String* pData = aStr.pData;

    return
    (
       c_rtl_tres_state
        (
            hRtlTestResult,
            pData->refCount == 1 &&
            pData->length == kTestStr1Len &&
            cmpstr( (const sal_Char*)pData->buffer, kTestStr2, kTestStr1Len ),
            "New OString from the first n chars of ascii string",
            "ctor_003"
        )
    );
}

//------------------------------------------------------------------------

static sal_Bool SAL_CALL test_rtl_OString_ctor_004(
                                                hTestResult hRtlTestResult)
{
    ::rtl::OString aStr1(kTestStr1);
    ::rtl::OString aStr2(aStr1);
    rtl_String* pData1 = aStr1.pData;
    rtl_String* pData2 = aStr2.pData;

    return
    (
        c_rtl_tres_state
        (
            hRtlTestResult,
            pData1->refCount == pData2->refCount &&
            pData1->length == kTestStr1Len &&
            pData1->buffer == pData2->buffer,
            "New OString from an OString",
            "ctor_004"
        )
    );
}
//------------------------------------------------------------------------

static sal_Bool test_rtl_OString_ctor_005( hTestResult hRtlTestResult )
{
    rtl_String *aStr1 = NULL;

    rtl_string_newFromStr( &aStr1, kTestStr1 );

    if ( aStr1 != NULL )
    {
        ::rtl::OString aStr2(aStr1);
        rtl_String* pData2 = aStr2.pData;

        sal_Bool bOK =  c_rtl_tres_state
                        (
                            hRtlTestResult,
                            aStr1->refCount == pData2->refCount &&
                            pData2->length == kTestStr1Len &&
                            aStr1->buffer == pData2->buffer,
                            "new OString from a RTL String",
                            "ctor_005"
                        );

        rtl_string_release( aStr1 );
        aStr1 = NULL;
        return ( bOK );
    }
    return
    (
        c_rtl_tres_state
        (
            hRtlTestResult,
            sal_False,
            "copying an ascii string to a RTL String!",
            "ctor_005"
        )
    );
}

//------------------------------------------------------------------------

static sal_Bool test_rtl_OString_ctor_006( hTestResult hRtlTestResult )
{

    sal_Unicode aStr1[kTestStr1Len+1];

    if ( AStringToUStringNCopy( aStr1, kTestStr1, kTestStr1Len ) )
    {
        if ( AStringToUStringNCompare( aStr1, kTestStr1, kTestStr1Len ) == 0 )
        {
            const sal_Char  *kTCMessage[2] = { "", "array." };

            ::rtl::OString aStr2
            (
                aStr1,
                kTestStr1Len,
                kEncodingRTLTextUSASCII,
                kConvertFlagsOUStringToOString
            );

            return
            (
                c_rtl_tres_state
                (
                    hRtlTestResult,
                    aStr2 == kTestStr1,
                    "new OString from a unicode character buffer",
                    "ctor_006"
                )
            );
        } /// end if AStringToUStringNCompare

        return
        (
             c_rtl_tres_state
            (
                hRtlTestResult,
                sal_False,
                "compare ascii string with unicode string!",
                "ctor_006"
            )
        );
    } /// end if AStringToUStringNCopy

    return
    (
        c_rtl_tres_state
        (
            hRtlTestResult,
            sal_False,
            "copy ascii string to unicode string!",
            "ctor_006"
        )
    );
}
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_ctors(
                    hTestResult hRtlTestResult )
{

    c_rtl_tres_state_start(hRtlTestResult, "ctor");
    sal_Bool bTSState = test_rtl_OString_ctor_001( hRtlTestResult );

    bTSState &= test_rtl_OString_ctor_002( hRtlTestResult);
    bTSState &= test_rtl_OString_ctor_003( hRtlTestResult);
    bTSState &= test_rtl_OString_ctor_004( hRtlTestResult);
    bTSState &= test_rtl_OString_ctor_005( hRtlTestResult);
    bTSState &= test_rtl_OString_ctor_006( hRtlTestResult);

    c_rtl_tres_state_end(hRtlTestResult, "ctor");

//    return( bTSState );
}



//------------------------------------------------------------------------
// testing the method getLength
//------------------------------------------------------------------------

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_getLength(
                                      hTestResult hRtlTestResult)
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "getLength");

typedef struct TestCase
{
    sal_Char*                    comments;
    sal_Int32                    expVal;
    OString*                     input;
    ~TestCase() { delete input;}
} TestCase;

TestCase arrTestCase[]={

    {"length of ascii string", kTestStr1Len, new OString(kTestStr1)},
    {"length of ascci string of size 1", 1, new OString("1")},
    {"length of empty string (default constructor)", 0, new OString()},
    {"length of empty string (empty ascii string arg)",0,new OString("")},
    {"length of empty string (string arg = '\\0')",0,new OString("\0")}
    };


    sal_Bool res = sal_True;
    sal_Int32 i;

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
    c_rtl_tres_state_end(hRtlTestResult, "getLength");
//    return ( res );
}



//------------------------------------------------------------------------
// testing the method equals( const OString & aStr )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_equals(
                                             hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "equals");

    typedef struct TestCase
{
    sal_Char*                    comments;
    sal_Bool                     expVal;
    OString*                     input1;
    OString*                     input2;
    ~TestCase() { delete input1;delete input2;}
} TestCase;

TestCase arrTestCase[]={

    {"same size", sal_True, new OString(kTestStr1),new OString(kTestStr1)},
    {"different size", sal_False, new OString(kTestStr1),
        new OString(kTestStr2)},
    {"same size, no case match", sal_False, new OString(kTestStr1),
        new OString(kTestStr3)},
    {"two empty strings(def. constructor)", sal_True, new OString(),
        new OString()},
    {"empty(def.constructor) and non empty", sal_False, new OString(),
        new OString(kTestStr2)},
    {"non empty and empty(def. constructor)", sal_False,
        new OString(kTestStr1),new OString()},
    {"two empty strings(string arg = '\\0')", sal_True,
        new OString(""),new OString("")},
    {"empty(string arg = '\\0') and non empty", sal_False,
        new OString(""),new OString(kTestStr2)},
    {"non empty and empty(string arg = '\\0')", sal_False,
        new OString(kTestStr1),new OString("")}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool lastRes =
                 ( arrTestCase[i].input1->equals(*(arrTestCase[i].input2)) ==
                    arrTestCase[i].expVal );

          c_rtl_tres_state
        (
              hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "equals", i )
        );

        res &= lastRes;
    }
    c_rtl_tres_state_end(hRtlTestResult, "equals");
//    return (res);
}

//------------------------------------------------------------------------
// testing the method equalsIgnoreAsciiCase( const OString & aStr )
//------------------------------------------------------------------------

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_equalsIgnoreAsciiCase(
                                             hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "equalsIgnoreAsciiCase");
    typedef struct TestCase
{
    sal_Char*                    comments;
    sal_Bool                     expVal;
    OString*                     input1;
    OString*                     input2;
    ~TestCase() { delete input1;delete input2;}
} TestCase;

TestCase arrTestCase[]={
        {"same strings but different cases",sal_True,new OString(kTestStr4),
            new OString(kTestStr5)},
        {"same strings",sal_True,new OString(kTestStr4),
            new OString(kTestStr4)},
        {"with equal beginning",sal_False,new OString(kTestStr2),
            new OString(kTestStr4)},
        {"empty(def.constructor) and non empty",sal_False,new OString(),
            new OString(kTestStr5)},
        {"non empty and empty(def.constructor)",sal_False,
            new OString(kTestStr4), new OString()},
        {"two empty strings(def.constructor)",sal_True,new OString(),
            new OString()},
        {"different strings with equal length",sal_False,
            new OString(kTestStr10), new OString(kTestStr11)}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool lastRes =
        (arrTestCase[i].input1->equalsIgnoreAsciiCase(*arrTestCase[i].input2)
            == arrTestCase[i].expVal);

         c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "equalsIgnoreAsciiCase", i )
        );

        res &= lastRes;
    }
    c_rtl_tres_state_end(hRtlTestResult, "equalsIgnoreAsciiCase");

//    return (res);
}

static sal_Bool SAL_CALL test_rtl_OString_compareTo_001(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
{
    sal_Char*                     comments;
    sal_Int32                     expVal;
    OString*                      input1;
    OString*                      input2;
    ~TestCase() { delete input1;delete input2;}
} TestCase;

TestCase arrTestCase[]={

        {"simple compare, str1 to str5",-1,new OString(kTestStr1),
            new OString(kTestStr5)},
        {"simple compare, str2 to str5",-1,new OString(kTestStr2),
            new OString(kTestStr5)},
        {"simple compare, str1 to str9",-1,new OString(kTestStr1),
            new OString(kTestStr9)},
        {"simple compare, str1 to str2",-1,new OString(kTestStr1),
            new OString(kTestStr2)},
        {"simple compare, str4 to str5",-1,new OString(kTestStr4),
            new OString(kTestStr5)},
        {"simple compare, str1 to str3",-1,new OString(kTestStr1),
            new OString(kTestStr3)},
        {"simple compare, str5 to str1",+1,new OString(kTestStr5),
            new OString(kTestStr1)},
        {"simple compare, str2 to str1",+1,new OString(kTestStr2),
            new OString(kTestStr1)},
        {"simple compare, str9 to str5",+1,new OString(kTestStr9),
            new OString(kTestStr5)},
        {"simple compare, str5 to str4",+1,new OString(kTestStr5),
            new OString(kTestStr4)},
        {"simple compare, str1 to str1",0,new OString(kTestStr1),
            new OString(kTestStr1)},
        {"simple compare, nullString to nullString",0,new OString(),
            new OString()},
        {"simple compare, nullString to str2",-1,new OString(),
            new OString(kTestStr2)},
        {"simple compare, str1 to nullString",+1,new OString(kTestStr1),
            new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Int32 cmpRes =
            arrTestCase[i].input1->compareTo(*arrTestCase[i].input2);
        cmpRes = ( cmpRes == 0 ) ? 0 : ( cmpRes > 0 ) ? +1 : -1 ;
        sal_Bool lastRes = ( cmpRes == arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "compareTo(const OString&)", i )
        );

        res &= lastRes;
    }

    return (res);
}


//------------------------------------------------------------------------
//  testing the method compareTo( const OString & rObj, sal_Int32 length )
//------------------------------------------------------------------------
static sal_Bool SAL_CALL test_rtl_OString_compareTo_002(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
    sal_Char*                    comments;
    sal_Int32                    expVal;
    sal_Int32                    maxLength;
    OString*                     input1;
    OString*                     input2;
    ~TestCase() { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"compare with maxlength, str1 to str9, 16",-1,16,
            new OString(kTestStr1), new OString(kTestStr9)},
        {"compare with maxlength, str2 to str9, 32",-1,32,
            new OString(kTestStr2), new OString(kTestStr9)},
        {"compare with maxlength, str9 to str4, 16",+1,16,
            new OString(kTestStr9), new OString(kTestStr4)},
        {"compare with maxlength, str9 to str22, 32",+1,32,
            new OString(kTestStr9), new OString(kTestStr22)},
        {"compare with maxlength, str9 to str5, 16",0,16,
            new OString(kTestStr9), new OString(kTestStr5)},
        {"compare with maxlength, str9 to str9, 32",0,32,
            new OString(kTestStr9), new OString(kTestStr9)},
        {"compare with maxlength, str1 to str2, 32",-1,32,
            new OString(kTestStr1), new OString(kTestStr2)},
        {"compare with maxlength, str1 to str2, 32",-1,32,
            new OString(kTestStr1), new OString(kTestStr2)}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Int32 cmpRes =
                    arrTestCase[i].input1->compareTo(*arrTestCase[i].input2,
                            arrTestCase[i].maxLength);
        cmpRes = (cmpRes == 0) ? 0 : (cmpRes > 0) ? +1 : -1 ;
        sal_Bool lastRes = (cmpRes == arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "compareTo(const OString&, sal_Int32)", i )
        );

        res &= lastRes;
    }

    return (res);
}

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_compareTo(
                                                hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start(hRtlTestResult, "compareTo");
    sal_Bool res = test_rtl_OString_compareTo_001(hRtlTestResult);
    res &= test_rtl_OString_compareTo_002(hRtlTestResult);
    c_rtl_tres_state_end(hRtlTestResult, "compareTo");
//    return (res);
}

//------------------------------------------------------------------------
// testing the operator == ( const OString& rStr1, const OString& rStr2 )
// testing the operator == ( const OString& rStr1, const sal_Char *rStr2 )
// testing the operator == ( const sal_Char *rStr1, const OString& rStr2 )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_op_cmp(
                                                hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start(hRtlTestResult, "op_cmp");
    const sal_Int16 NCASES = 7;
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;
    const sal_Char *arrOStr[NCASES][2] =
    {
        {kTestStr1, kTestStr1},
        {kTestStr1, kTestStr3},
        {kTestStr1, kTestStr2},
        {0, 0},
        {0, kTestStr2},
        {kTestStr1, 0},
        {"", ""}
    };

    sal_Bool arrExpVal[NCASES] =
    {
        sal_True,
        sal_False,
        sal_False,
        sal_True,
        sal_False,
        sal_False,
        sal_True
    };

    sal_Char *arrComments[NCASES] =
    {
        "'Sun Microsystems'=='Sun Microsystems'",
        "!('Sun Microsystems'=='Sun microsystems')",
        "!('Sun Microsystems'=='Sun Microsystems Java Technology')",
        "two empty strings(def.constructor)",
        "!(empty string=='Sun Microsystems Java Technology')",
        "!('Sun Microsystems Java Technology'==empty string)",
        "''==''"
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < NCASES; i++)
    {
        OString *str1, *str2;
        str1 = (arrOStr[i][0]) ? new OString(arrOStr[i][0]) : new OString() ;
        str2 = (arrOStr[i][1]) ? new OString(arrOStr[i][1]) : new OString() ;

        sal_Bool cmpRes = (*str1 == *str2);
        sal_Bool lastRes = (cmpRes == arrExpVal[i]);
        res &= lastRes;

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrComments[i],
            createName( pMeth, "operator ==(OString&, OString&)", i )
        );

        cmpRes = (*str1 == arrOStr[i][1]);
        lastRes = (cmpRes == arrExpVal[i]);
        res &= lastRes;
        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrComments[i],
            createName( pMeth, "operator ==(OString&, sal_Char *)", i )
        );

        cmpRes = (arrOStr[i][0] == *str2);
        lastRes = (cmpRes == arrExpVal[i]);
        res &= lastRes;
        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrComments[i],
            createName( pMeth, "operator ==(sal_Char *, OString&)", i )
        );

        delete str2;
        delete str1;
    }

    c_rtl_tres_state_end(hRtlTestResult, "op_cmp");
//    return ( res );
}

//------------------------------------------------------------------------
// testing the operator != (const OString& rStr1, const OString& rStr2)
// testing the operator != (const OString& rStr1, const sal_Char *rStr2)
// testing the operator != (const sal_Char *rStr1, const OString& rStr2)
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_op_neq(
                                             hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start(hRtlTestResult, "op_neq");
    const sal_Int16 NCASES = 6;
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    const sal_Char *arrOStr[NCASES][2] =
    {
        {kTestStr1, kTestStr3},
        {kTestStr1, kTestStr2},
        {kTestStr1, kTestStr1},
        {0, kTestStr2},
        {kTestStr1, 0},
        {0, 0}
    };

    sal_Bool arrExpVal[NCASES] =
    {
        sal_True,
        sal_True,
        sal_False,
        sal_True,
        sal_True,
        sal_False
    };

    sal_Char *arrComments[NCASES] =
    {
        "'Sun Microsystems'!='Sun microsystems'",
        "'Sun Microsystems'!='Sun Microsystems Java Technology'",
        "!('Sun Microsystems'!='Sun Microsystems')",
        "empty string!='Sun Microsystems Java Technology'",
        "'Sun Microsystems Java Technology'!=empty string", "!(''!='')"
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < NCASES; i++)
    {
        OString *str1, *str2;
        str1 = (arrOStr[i][0]) ? new OString(arrOStr[i][0]) : new OString() ;
        str2 = (arrOStr[i][1]) ? new OString(arrOStr[i][1]) : new OString() ;

        sal_Bool cmpRes = (*str1 != *str2);
        sal_Bool lastRes = (cmpRes == arrExpVal[i]);
        res &= lastRes;
        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrComments[i],
            createName( pMeth, "operator !=(OString&, OString&)", i )
        );

        cmpRes = (*str1 != arrOStr[i][1]);
        lastRes = (cmpRes == arrExpVal[i]);
        res &= lastRes;
        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrComments[i],
            createName( pMeth, "operator !=(OString&, sal_Char *)", i )
        );

        cmpRes = (arrOStr[i][0] != *str2);
        lastRes = (cmpRes == arrExpVal[i]);
        res &= lastRes;
        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrComments[i],
            createName( pMeth, "operator !=(sal_Char *, OString&)", i )
        );

        delete str2;
        delete str1;
    }

    c_rtl_tres_state_end(hRtlTestResult, "op_neq");
//   return ( res );
}


//------------------------------------------------------------------------
// testing the operator > (const OString& rStr1, const OString& rStr2)
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_op_g(
                                             hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "op_g");
    typedef struct TestCase
    {
    sal_Char*                    comments;
    sal_Bool                     expVal;
    OString*                     input1;
    OString*                     input2;
    ~TestCase() { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        { "'Sun microsystems'>'Sun Microsystems'",sal_True,
            new OString(kTestStr3), new OString(kTestStr1)},
        {"!('Sun Microsystems'>'Sun microsystems')",sal_False,
            new OString(kTestStr1), new OString(kTestStr3)},
        {"'Sun Microsystems Java Technology'>'Sun Microsystems'",sal_True,
            new OString(kTestStr2), new OString(kTestStr1)},
        {"!('Sun Microsystems'>'Sun Microsystems Java Technology')",sal_False,
            new OString(kTestStr1), new OString(kTestStr2)},
        {"!('Sun Microsystems'>'Sun Microsystems'",sal_False,
            new OString(kTestStr1), new OString(kTestStr1)},
        {"'Sun Microsystems'>''",sal_True,new OString(kTestStr1),
            new OString()},
        {"!(''>'Sun Microsystems')",sal_False,new OString(),
            new OString(kTestStr1)},
        {"!(''>'')",sal_False,new OString(), new OString()}
};

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool cmpRes = (*arrTestCase[i].input1 > *arrTestCase[i].input2);
        sal_Bool lastRes = (cmpRes == arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "operator >", i )
        );

        res &= lastRes;

    }

    c_rtl_tres_state_end(hRtlTestResult, "op_g");
//    return ( res );
}

//------------------------------------------------------------------------
// testing the operator < (const OString& rStr1, const OString& rStr2)
//------------------------------------------------------------------------

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_op_l(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "op_l");
    typedef struct TestCase
    {
    sal_Char*                    comments;
    sal_Bool                     expVal;
    OString*                     input1;
    OString*                     input2;
    ~TestCase() { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"!('Sun microsystems'<'Sun Microsystems')",sal_False,
            new OString(kTestStr3), new OString(kTestStr1)},
        {"'Sun Microsystems'<'Sun microsystems'",sal_True,
            new OString(kTestStr1), new OString(kTestStr3)},
        {"'Sun Microsystems'<'Sun Microsystems Java Technology'",sal_True,
            new OString(kTestStr1), new OString(kTestStr2)},
        {"!('Sun Microsystems Java Technology'<'Sun Microsystems')",sal_False,
            new OString(kTestStr2), new OString(kTestStr1)},
        {"!('Sun Microsystems'<'Sun Microsystems'", sal_False,
            new OString(kTestStr1), new OString(kTestStr1)},
        {"'Sun Microsystems'<''",sal_False,new OString(kTestStr1),
            new OString()},
        {"''<'Sun Microsystems Java Technology'",sal_True,new OString(),
            new OString(kTestStr2)},
        {"!(''<'')",sal_False,new OString(), new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool cmpRes = (*arrTestCase[i].input1 < *arrTestCase[i].input2);
        sal_Bool lastRes = (cmpRes == arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "operator <", i )
        );

        res &= lastRes;

    }

    c_rtl_tres_state_end(hRtlTestResult, "op_l");
//   return ( res );
}

//------------------------------------------------------------------------
// testing the operator >= (const OString& rStr1, const OString& rStr2)
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_op_ge(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "op_ge");
   typedef struct TestCase
    {
    sal_Char*                    comments;
    sal_Bool                     expVal;
    OString*                     input1;
    OString*                     input2;
    ~TestCase() { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"'Sun microsystems'>='Sun Microsystems'",sal_True,
            new OString(kTestStr3), new OString(kTestStr1)},
        {"!('Sun Microsystems'>='Sun microsystems')",sal_False,
            new OString(kTestStr1), new OString(kTestStr3)},
        {"!('Sun Microsystems'>='Sun Microsystems Java Technology')",sal_False,
            new OString(kTestStr1), new OString(kTestStr2)},
        {"'Sun Microsystems Java Technology'>='Sun Microsystems'",sal_True,
            new OString(kTestStr2), new OString(kTestStr1)},
        {"'Sun Microsystems'>='Sun Microsystems'", sal_True,
            new OString(kTestStr1), new OString(kTestStr1)},
        {"'Sun Microsystems'>=''",sal_True,new OString(kTestStr1),
            new OString()},
        { "''>='Sun microsystems'",sal_False,new OString(),
            new OString(kTestStr3)},
        {"''>=''",sal_True,new OString(), new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool cmpRes = (*arrTestCase[i].input1 >= *arrTestCase[i].input2);
        sal_Bool lastRes = (cmpRes == arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "operator >=", i )
        );

        res &= lastRes;

    }

    c_rtl_tres_state_end(hRtlTestResult, "op_ge");
//    return ( res );
}

//------------------------------------------------------------------------
// testing the operator <= (const OString& rStr1, const OString& rStr2)
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_op_le(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "op_le");
    typedef struct TestCase
    {
    sal_Char*                    comments;
    sal_Bool                     expVal;
    OString*                     input1;
    OString*                     input2;
    ~TestCase() { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"!('Sun microsystems'<='Sun Microsystems')",sal_False,
            new OString(kTestStr3), new OString(kTestStr1)},
        {"'Sun Microsystems'<='Sun microsystems'",sal_True,
            new OString(kTestStr1), new OString(kTestStr3)},
        {"'Sun Microsystems'<='Sun Microsystems Java Technology'",sal_True,
            new OString(kTestStr1),
            new OString(kTestStr2)},
        {"!('Sun Microsystems Java Technology'<='Sun Microsystems')",sal_False,
            new OString(kTestStr2),
            new OString(kTestStr1)},
        {"!('Sun Microsystems'<='Sun Microsystems'", sal_True,
            new OString(kTestStr1), new OString(kTestStr1)},
        {"'Sun Microsystems'<=''",sal_False,new OString(kTestStr1),
            new OString()},
        {"''<='Sun Microsystems Java Technology'",sal_True,new OString(),
            new OString(kTestStr2)},
        {"!(''<='')",sal_True,new OString(), new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool cmpRes = (*arrTestCase[i].input1 <= *arrTestCase[i].input2);
        sal_Bool lastRes = (cmpRes == arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "operator <=", i )
        );

        res &= lastRes;

    }

    c_rtl_tres_state_end(hRtlTestResult, "op_le");
//    return ( res );
}


//------------------------------------------------------------------------
// testing the operator =
//------------------------------------------------------------------------
static sal_Bool test_rtl_OString_op_eq_001( hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
    sal_Char*                    comments;
    sal_Bool                     expVal;
    OString*                     input1;
    OString*                     input2;
    ~TestCase() { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"'' = str1, str1 == str2",sal_True,new OString(kTestStr1),
            new OString()},
        {"str1 = str2, str1 == str2",sal_True,new OString(kTestStr1),
            new OString(kTestStr6)},
        {"str2 = '', str1 == str2",sal_True,new OString(),
            new OString(kTestStr2)},
        {"'' = '', str1 == str2",sal_True,new OString(),
            new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
       *(arrTestCase[i].input1) = *(arrTestCase[i].input2);

        sal_Bool cmpRes =
            (*(arrTestCase[i].input1) == *(arrTestCase[i].input2));
        sal_Bool lastRes = (cmpRes == arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "operator =", i )
        );

        res &= lastRes;
    }

    return ( res );
}

static sal_Bool test_rtl_OString_op_eq_002(
                    hTestResult hRtlTestResult )
{
    ::rtl::OString aStr;
    aStr = OString(kTestStr1);

    return
    (
        c_rtl_tres_state
        (
            hRtlTestResult,
            aStr == kTestStr1,
            "str = OString(\"%s\"), str == \"%s\"",
            "operator ="
        )
    );
}

static sal_Bool test_rtl_OString_op_eq_003(
                    hTestResult hRtlTestResult )
{
    sal_Bool bTCState = false;

    ::rtl::OString aStr1(kTestStr1);
    ::rtl::OString aStr2;
    ::rtl::OString aStr3;

    aStr3 = aStr2 = aStr1;

    bTCState =    ( aStr1 == aStr2 )
               && ( aStr1 == aStr3 )
               && ( aStr2 == aStr3 );

    c_rtl_tres_state
    (
        hRtlTestResult,
        bTCState,
        "str3=str2=str1,(str1 == str2)&&(str1 == str3)&&(str2 == str3)",
        "operator ="
    );

    return bTCState;
}

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_op_eq(
                                         hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start(hRtlTestResult, "op_eq");
    sal_Bool res = test_rtl_OString_op_eq_001( hRtlTestResult );
    res &= test_rtl_OString_op_eq_002( hRtlTestResult );
    res &= test_rtl_OString_op_eq_003( hRtlTestResult );
    c_rtl_tres_state_end(hRtlTestResult, "op_eq");

//    return ( res );
}

//------------------------------------------------------------------------
// testing the operator +
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_op_plus(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "op_plus");
    typedef struct TestCase
    {
    sal_Char*                    comments;
    OString*                     expVal;
    OString*                     input1;
    OString*                     input2;
    ~TestCase() { delete input1;delete input2; delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"str1 = str7 + str8",new OString(kTestStr1),
            new OString(kTestStr7), new OString(kTestStr8)},
        {"str1 = str1 + '' ",new OString(kTestStr1),
            new OString(kTestStr1), new OString("")},
        {"str1 = '' + str1", new OString(kTestStr1),
            new OString(""), new OString(kTestStr1)},
        {" '' = '' + '' ", new OString(""),new OString(""),
            new OString("")},
        {"str1 = str1 + def.constr", new OString(kTestStr1),
            new OString(kTestStr1), new OString()},
        {" str1 = def.constr + str1 ",new OString(kTestStr1),
            new OString(), new OString(kTestStr1)},
        {" def.constr= def.constr + def.constr", new OString(),
            new OString(), new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;
    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        OString str = (*arrTestCase[i].input1) + (*arrTestCase[i].input2);
        sal_Bool lastRes = (str == *arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "operator +", i )
        );

        res &= lastRes;

    }

    c_rtl_tres_state_end(hRtlTestResult, "op_plus");
//   return ( res );
}

//------------------------------------------------------------------------
// testing the operator +=
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_op_peq(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "op_peq");
    typedef struct TestCase
    {
    sal_Char*                    comments;
    OString*                     expVal;
    OString*                     input1;
    OString*                     input2;
    ~TestCase() { delete input1;delete input2; delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"str1 == (str7 += str8)",new OString(kTestStr1),
            new OString(kTestStr7), new OString(kTestStr8)},
        {"str1 == (str1 += '')",new OString(kTestStr1),
            new OString(kTestStr1), new OString("")},
        {"str1 == ('' += str1)", new OString(kTestStr1),
            new OString(""), new OString(kTestStr1)},
        {" '' == ('' += '')", new OString(""),
            new OString(""), new OString("")},
        {"str1 == (str1 += def.constr)", new OString(kTestStr1),
            new OString(kTestStr1), new OString()},
        {" str1 == (def.constr += str1)",new OString(kTestStr1),
            new OString(), new OString(kTestStr1)},
        {" def.constr== (def.constr += def.constr)",
            new OString(),new OString(), new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;
    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {   OString str;
        str += (*arrTestCase[i].input1); str += (*arrTestCase[i].input2);
        sal_Bool lastRes = (str == *arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "operator +", i )
        );

        res &= lastRes;

    }

    c_rtl_tres_state_end(hRtlTestResult, "op_peq");
//   return ( res );
}

//------------------------------------------------------------------------
// testing the operator const sal_Char * (cscs for short)
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_op_cscs(
                                                hTestResult hRtlTestResult )
{
sal_Char methName[MAXBUFLENGTH];
sal_Char* pMeth = methName;

 c_rtl_tres_state_start(hRtlTestResult, "op_cscs");
typedef struct TestCase
    {
        sal_Char*                   comments;
        const sal_Char*              expVal;
        sal_Int32                    cmpLen;
        OString*                    input1;
        ~TestCase() { delete input1;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"test normal string",kTestStr1,kTestStr1Len,new OString(kTestStr1)},
        {"test empty string","",1,new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;
    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        const sal_Char* pstr = (*arrTestCase[i].input1);

        res &= c_rtl_tres_state
                (
                    hRtlTestResult,
                    cmpstr((sal_Char*)pstr,(sal_Char*)arrTestCase[i].expVal,
                        arrTestCase[i].cmpLen),
                    arrTestCase[i].comments,
                    createName( pMeth, "const sal_Char*", i )
                );
    }
    c_rtl_tres_state_end(hRtlTestResult, "op_cscs");
//    return ( res );
}


//------------------------------------------------------------------------
// testing the method getStr()
//------------------------------------------------------------------------


extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_getStr(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "getStr");
    typedef struct TestCase
    {
        sal_Char*                    comments;
        const sal_Char*              expVal;
        sal_Int32                    cmpLen;
        OString*                     input1;
        ~TestCase() { delete input1;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"test normal string",kTestStr1,kTestStr1Len,new OString(kTestStr1)},
        {"test empty string","",0,new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;
    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        const sal_Char* pstr = arrTestCase[i].input1->getStr();
        res &= c_rtl_tres_state
                    (
                        hRtlTestResult,
                        cmpstr(pstr, arrTestCase[i].expVal,
                            arrTestCase[i].cmpLen),
                        arrTestCase[i].comments,
                        createName( pMeth, "getStr", i )
                    );
    }
    c_rtl_tres_state_end(hRtlTestResult, "getStr");
//    return ( res );
}



//------------------------------------------------------------------------
// testing the method copy( sal_Int32 beginIndex )
//------------------------------------------------------------------------
static sal_Bool SAL_CALL test_rtl_OString_copy_001(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
    sal_Char*                    comments;
    const sal_Char*              srcStr;
    const sal_Char*              arrExpStr;
                    // string for comparing with result
    sal_Int32                    beginIndex;
                    // beginIndex for the method copy
    sal_Int32                    lengthForCmp;
                    // number of symbols for comparing
    // (if value is equal to 0 then pointers to buffers must be equal)
    sal_Int32                    expLength;
                    //expected length of the result string
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"beginIndex == 0 ( whole string )", kTestStr2,kTestStr2,
         0, kTestStr2Len, kTestStr2Len},
        {"beginIndex == strlen-2 ( last two char )",  kTestStr2,"gy",
                    kTestStr2Len-2, 2, 2},
        {"beginIndex == strlen-1( last char )", kTestStr2, "y",
            kTestStr2Len-1, 1, 1}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i <(sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        OString src(arrTestCase[i].srcStr);
        OString dst;
        rtl_String* pDataSrc = src.pData;

        dst = src.copy(arrTestCase[i].beginIndex);

        rtl_String* pDataDst = dst.pData;

        sal_Bool lastRes;

        lastRes= (dst== arrTestCase[i].arrExpStr);


        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth,
                "copy_001(beginIndex)(check buffer and length)", i )
        );

        res &= lastRes;

    }

    return (res);
}


//------------------------------------------------------------------------
// testing the method copy( sal_Int32 beginIndex, sal_Int32 count )
//------------------------------------------------------------------------
static sal_Bool SAL_CALL test_rtl_OString_copy_002(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
    sal_Char*               comments;
    const sal_Char*          arrExpStr;
    sal_Int32                beginIndex;
    sal_Int32                count;
    sal_Int32                expLen;

    } TestCase;

    TestCase arrTestCase[] ={

        {"copy substring",                            kTestStr6, kTestStr11Len, kTestStr2Len - kTestStr11Len,kTestStr6Len},
        /* LLA: it is a bug, beginIndex + count > kTestStr2.getLength() */
        /* {"copy normal substring with incorrect count",kTestStr6, kTestStr11Len, 31, 15}, */
        {"copy whole string", kTestStr2, 0, kTestStr2Len, kTestStr2Len},
        /* {"copy whole string with incorrect count larger than len and index 0", kTestStr2, 0, 40, 32}, */
        /* LLA: bug beginIndex + count > kTestStr2 {"copy last character",                                               "y",kTestStr2Len - 1, 31,1}, */
        {"copy last character",                                               "y",kTestStr2Len - 1, 1,1},
        /* LLA: bug, beginIndex > kTestStr2 {"beginindex larger than len","",60, 0,0}, */
        {"beginindex exact as large as it's length","",kTestStr2Len, 0,0}
        /* LLA: bug, negative count is not allowed. {"count is nagative int","",3, -1,0} */
    };
    sal_Bool res = sal_True;

    sal_Int32 i;
    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++) {
        OString src(kTestStr2);
        OString dst;
        dst = src.copy(arrTestCase[i].beginIndex, arrTestCase[i].count);
        rtl_String* pDataSrc = src.pData;
        rtl_String* pDataDst = dst.pData;

        sal_Bool lastRes=sal_True;
        // checks buffer and length
        //printf("\n this is copy__002 #%d\n", i);
        //printf("dst buffer =%s\n", pDataDst->buffer);
        //printf("expStr =%s\n", arrTestCase[i].arrExpStr);
        //printf("dst length =%d\n", pDataDst->length);
        //printf("count =%d\n", arrTestCase[i].count);
        //printf("expLen =%d\n", arrTestCase[i].expLen);

        lastRes = (dst.equals(arrTestCase[i].arrExpStr)) ? sal_True : sal_False;

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth,
                "copy_002(beginIndex,count)(check buffer and length)", i)
        );
        res &= lastRes;


    }

    return (res);
}


static sal_Bool SAL_CALL test_rtl_OString_copy_003(
                                                hTestResult hRtlTestResult )
{
    sal_Bool res = sal_True;
    char comment[] = "copy whole short string to long string";

    OString src(kTestStr1);
    rtl_String* pDataSrc = src.pData;
    OString dst(kTestStr2);

    dst = src.copy(0);
    rtl_String* pDataDst = dst.pData;
    //check buffer and length
    sal_Bool lastRes =(dst==src);
    c_rtl_tres_state
    (
        hRtlTestResult,
        lastRes,
        comment,
        "copy_003(beginIndex)(check buffer and length)"
    );
    res &= lastRes;

    return (res);
}


static sal_Bool SAL_CALL test_rtl_OString_copy_004(
                                                hTestResult hRtlTestResult )
{
    sal_Bool res = sal_True;
    sal_Char comment[] = "copy whole long string to short string";

    OString src(kTestStr2);
    rtl_String* pDataSrc = src.pData;
    OString dst(kTestStr1);

    dst = src.copy(0);
    rtl_String* pDataDst = dst.pData;
    //check buffer and length
    sal_Bool lastRes =(dst==src);
    c_rtl_tres_state
    (
        hRtlTestResult,
        lastRes,
        comment,
        "copy_004(beginIndex)(check buffer and length)"
    );

    res &= lastRes;
    return (res);
}

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_copy(
                                                hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start(hRtlTestResult, "copy");
    sal_Bool res = test_rtl_OString_copy_001(hRtlTestResult);
    res &= test_rtl_OString_copy_002(hRtlTestResult);
    res &= test_rtl_OString_copy_003(hRtlTestResult);
    res &= test_rtl_OString_copy_004(hRtlTestResult);
    c_rtl_tres_state_end(hRtlTestResult, "copy");

//    return ( res );
}

//------------------------------------------------------------------------
// testing the method concat( const OString & aStr )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_concat(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth =methName;

    c_rtl_tres_state_start(hRtlTestResult, "concat");
    typedef struct TestCase
    {
    sal_Char*                    comments;
    OString*                     expVal;
    OString*                     input1;
    OString*                     input2;
    ~TestCase() { delete input1;delete input2; delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"concatenates two strings",new OString(kTestStr1),
            new OString(kTestStr7),
            new OString(kTestStr8)},
        {"concatenates empty string",new OString(kTestStr1),
            new OString(kTestStr1),
            new OString("")},
        {"concatenates to empty string",new OString(kTestStr1),
            new OString(""),
            new OString(kTestStr1)},
        {"concatenates two empty strings",new OString(""),new OString(""),
             new OString("")},
        {"concatenates string constructed by default constructor",
            new OString(kTestStr1),
            new OString(kTestStr1), new OString()},
        {"concatenates to string constructed by default constructor",
            new OString(kTestStr1),
            new OString(), new OString(kTestStr1)},
        {"concatenates two strings constructed by default constructor",
            new OString(),
            new OString(), new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;
    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        OString str =
            arrTestCase[i].input1->concat(*arrTestCase[i].input2);
        sal_Bool lastRes = (str == *arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "concat", i)
        );

        res &= lastRes;

     }

    c_rtl_tres_state_end(hRtlTestResult, "concat");
//    return ( res );
}


//------------------------------------------------------------------------
// testing the method toAsciiLowerCase()
//-----------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_toAsciiLowerCase(
                                      hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth =methName;

    c_rtl_tres_state_start(hRtlTestResult, "toAsciiLowerCase");
    typedef struct TestCase
    {
    sal_Char*                    comments;
    OString*                     expVal;
    OString*                     input1;
    ~TestCase() { delete input1; delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {

        {"only uppercase",new OString(kTestStr5),new OString(kTestStr4)},
        {"different cases",new OString(kTestStr5),new OString(kTestStr1)},
        {"different cases",new OString(kTestStr5),new OString(kTestStr3)},
        {"only lowercase",new OString(kTestStr5),new OString(kTestStr5)},
        {"empty string",new OString(""),new OString("")},
        {"string constructed by default constructor",
            new OString(),new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        OString str = arrTestCase[i].input1->toAsciiLowerCase();
        sal_Bool lastRes = (str ==* arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "toAsciiLowerCase", i)
        );

        res &= lastRes;
    }

    c_rtl_tres_state_end(hRtlTestResult, "toAsciiLowerCase");
//    return ( res );
}

//------------------------------------------------------------------------
// testing the method toAsciiUpperCase()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_toAsciiUpperCase(
                                            hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth =methName;

    c_rtl_tres_state_start(hRtlTestResult, "toAsciiUpperCase");
    typedef struct TestCase
    {
    sal_Char*                    comments;
    OString*                     expVal;
    OString*                     input1;
    ~TestCase() { delete input1; delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"only lowercase",new OString(kTestStr4),new OString(kTestStr5)},
        {"mixed cases",new OString(kTestStr4),new OString(kTestStr3)},
        {"miced cases",new OString(kTestStr4),new OString(kTestStr1)},
        {"only uppercase",new OString(kTestStr4),new OString(kTestStr4)},
        {"empty string",new OString(""),new OString("")},
        {"string constructed by default constructor",
            new OString(),new OString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        OString str = arrTestCase[i].input1->toAsciiUpperCase();
        sal_Bool lastRes = (str == *arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "toAsciiLowerCase", i)
        );

        res &= lastRes;
    }
    c_rtl_tres_state_end(hRtlTestResult, "toAsciiUpperCase");

//    return ( res );
}


//------------------------------------------------------------------------
// testing the method trim()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_trim(
                                           hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth =methName;

    c_rtl_tres_state_start(hRtlTestResult, "trim");
    typedef struct TestCase
    {
    sal_Char*                    comments;
    OString*                     expVal;
    OString*                     input1;
    ~TestCase() { delete input1; delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"removes space from the front",new OString(kTestStr1),
            new OString(kTestStr10)},
        {"removes space from the end",new OString(kTestStr1),
            new OString(kTestStr11)},
        {"removes space from the front and end",new OString(kTestStr1),
            new OString(kTestStr12)},
        {"removes several spaces from the end",new OString(kTestStr1),
            new OString(kTestStr13)},
        {"removes several spaces from the front",new OString(kTestStr1),
            new OString(kTestStr14)},
        {"removes several spaces from the front and one from the end",
            new OString(kTestStr1),
            new OString(kTestStr15)},
        {"removes one space from the front and several from the end",
            new OString(kTestStr1),
            new OString(kTestStr16)},
        {"removes several spaces from the front and end",
            new OString(kTestStr1),
            new OString(kTestStr17)},
        {"removes characters that have codes <= 32",new OString(kTestStr20),
            new OString("\1\3\5\7\11\13\15\17sun\21\23\25\27\31\33\40")},
        {"no spaces",new OString(kTestStr8),new OString(kTestStr8)}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        OString strRes = arrTestCase[i].input1->trim();
        sal_Bool lastRes = (strRes == *arrTestCase[i].expVal);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "trim", i)
        );

        res &= lastRes;

    }

    c_rtl_tres_state_end(hRtlTestResult, "trim");
//    return ( res );
}



//------------------------------------------------------------------------
// testing the method valueOf( sal_Bool b )
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OString_valueOf_sal_Bool(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth =methName;

    typedef struct TestCase
    {
    sal_Char*                    comments;
    sal_Bool                     input1;
    OString*                     expVal;
    ~TestCase() {delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"true",sal_True,new OString("true")},
        {"false",sal_False, new OString("false")}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        ::rtl::OString aStr1;
        aStr1 = aStr1.valueOf( arrTestCase[i].input1 );
        sal_Bool lastRes = (arrTestCase[i].expVal->compareTo(aStr1) == 0);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "valueof_bool", i)
        );

        res &= lastRes;

    }

    return ( res );
}

sal_Bool SAL_CALL test_rtl_OString_valueOf_sal_Char(
                    hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth =methName;

    typedef struct TestCase
    {
    sal_Char*                    comments;
    sal_Char                     input1;
    OString*                     expVal;
    ~TestCase() {delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"A",'A',new OString("A")},
        {"a",'a', new OString("a")},
        {"0",'0', new OString("0")},
        {"-",'-', new OString("-")},
        {"_",'_', new OString("_")},
        {"|",'|', new OString("|")},
        {"?",'?', new OString("?")},
        {"?",'?', new OString("?")},
        {"\n",'\n', new OString("\n")},
        {"\'",'\'', new OString("\'")},
        {"\"",'\"', new OString("\"")}

    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        ::rtl::OString aStr1;
        aStr1 = aStr1.valueOf( arrTestCase[i].input1 );
        sal_Bool lastRes = (arrTestCase[i].expVal->compareTo(aStr1) == 0);

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "valueof_char", i)
        );

        res &= lastRes;

    }

    return ( res );
}

/**
 * Calls the method valueOf(T, radix) and compares
 * returned strings with strings that passed in the array resArray.
 *
 * @param T, type of argument, passed to valueOf
 * @param resArray, array of result strings to compare to
 * @param n the number of elements in the array resArray (testcases)
 * @param pTestResult the instance of the class TestResult
 * @param inArray [optional], array of value that is passed as first argument
 *                            to valueOf
 *
 * @return true, if all returned strings are equal to corresponding string in
 *               resArray else, false.
 */
template <class T>
sal_Bool test_valueOf( const char** resArray, int n, sal_Int16 radix,
                            hTestResult hRtlTestResult, const T *inArray )
{
    sal_Bool bRes = sal_True;

    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;
    sal_Int32 i;

    for (i = 0; i < n; i++)
    {
        ::rtl::OString aStr1;
        ::rtl::OString aStr2( resArray[i] );

        if (inArray == 0)
            aStr1 = ::rtl::OString::valueOf((T)i, radix);
        else
        {
            if ( inArray[i] < 0 )
            {
                aStr2 = "-";
                aStr2 += resArray[i];
            }
            aStr1 = ::rtl::OString::valueOf((T)inArray[i], radix);
        }

        bRes &= c_rtl_tres_state
                (
                    hRtlTestResult,
                    aStr2.compareTo(aStr1) == 0,
                    (sal_Char*)resArray[i],
                    createName( pMeth, "valueOf", i )
                );
    }

    return (bRes);
}


#define test_valueOf_Int32      test_valueOf<sal_Int32>
#define test_valueOf_Int64      test_valueOf<sal_Int64>
// LLA: #define test_valueOf_float      test_valueOf<float>
// LLA: #define test_valueOf_double     test_valueOf<double>

//------------------------------------------------------------------------
// testing the method valueOf( sal_Int32 i, sal_Int16 radix=2 )
// testing the method valueOf( sal_Int32 i, sal_Int16 radix=8 )
// testing the method valueOf( sal_Int32 i, sal_Int16 radix=10 )
// testing the method valueOf( sal_Int32 i, sal_Int16 radix=16 )
// testing the method valueOf( sal_Int32 i, sal_Int16 radix=36 )
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OString_valueOf_Int32(
                                                hTestResult hRtlTestResult )
{
    sal_Bool bRes = sal_False;

    bRes = c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32((const char**)kBinaryNumsStr,
                        kBinaryNumsCount, kRadixBinary, hRtlTestResult, 0 ),
                "kRadixBinary",
                "valueOf(sal_Int32, radix 2)"
            );


    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32((const char**)kOctolNumsStr,
                        kOctolNumsCount, kRadixOctol, hRtlTestResult, 0),
                "kRadixOctol",
                "valueOf(sal_Int32, radix 8)"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32((const char**)kDecimalNumsStr,
                        kDecimalNumsCount, kRadixDecimal, hRtlTestResult, 0),
                "kRadixDecimal",
                "valueOf(sal_Int32, radix 10)"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32((const char**)kHexDecimalNumsStr,
                kHexDecimalNumsCount, kRadixHexdecimal, hRtlTestResult, 0),
                "kRadixHexdecimal",
                "valueOf(sal_Int32, radix 16)"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32((const char**)kBase36NumsStr,
                        kBase36NumsCount, kRadixBase36, hRtlTestResult, 0),
                "kRadixBase36",
                "valueOf(sal_Int32, radix 36)"
            );


    return ( bRes );
}

//------------------------------------------------------------------------
// testing the method valueOf( sal_Int32 l, sal_Int32 radix=2 )
// where l = large constants
// testing the method valueOf( sal_Int32 l, sal_Int32 radix=8 )
// where l = large constants
// testing the method valueOf( sal_Int32 l, sal_Int32 radix=10 )
// where l = large constants
// testing the method valueOf( sal_Int32 l, sal_Int32 radix=16 )
// where l = large constants
// testing the method valueOf( sal_Int32 l, sal_Int32 radix=36 )
// where l = large constants
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OString_valueOf_Int32_Bounderies(
                                                hTestResult hRtlTestResult )
{
    sal_Bool bRes = sal_False;

    bRes =  c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32((const char**)kBinaryMaxNumsStr,
                kInt32MaxNumsCount, kRadixBinary, hRtlTestResult,
                    kInt32MaxNums),
                "kRadixBinary",
                "valueOf(salInt32, radix 2) Bounderies"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32((const char**)kOctolMaxNumsStr,
                 kInt32MaxNumsCount, kRadixOctol, hRtlTestResult,
                    kInt32MaxNums),
                "kRadixOctol",
                "valueOf(salInt32, radix 8) Bounderies"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32((const char**)kDecimalMaxNumsStr,
                                    kInt32MaxNumsCount, kRadixDecimal,
                                    hRtlTestResult, kInt32MaxNums),
                "kRadixDecimal",
                "valueOf(salInt32, radix 10) Bounderies"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32((const char**)kHexDecimalMaxNumsStr,
                                kInt32MaxNumsCount, kRadixHexdecimal,
                                hRtlTestResult, kInt32MaxNums),
                "kRadixHexdecimal",
                "valueOf(salInt32, radix 16) Bounderies"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32((const char**)kBase36MaxNumsStr,
                                    kInt32MaxNumsCount, kRadixBase36,
                                    hRtlTestResult, kInt32MaxNums),
                "kRadixBase36",
                "valueOf(salInt32, radix 36) Bounderies"
            );

    return ( bRes );
}

//------------------------------------------------------------------------
// testing the method valueOf( sal_Int32 i, sal_Int16 radix=2 )
// for negative value
// testing the method valueOf( sal_Int32 i, sal_Int16 radix=8 )
// for negative value
// testing the method valueOf( sal_Int32 i, sal_Int16 radix=10 )
// for negative value
// testing the method valueOf( sal_Int32 i, sal_Int16 radix=16 )
// for negative value
// testing the method valueOf( sal_Int32 i, sal_Int16 radix=36 )
// for negative value
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OString_valueOf_Int32_Negative(
                                                hTestResult hRtlTestResult )
{
    sal_Bool bRes = sal_False;
    sal_Int32 inArr[kBase36NumsCount];
    sal_Int32 i;

    for (i = 0; i < kBase36NumsCount; i++ )
        inArr[i] = -i;

    bRes =  c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32( kBinaryNumsStr, kBinaryNumsCount,
                                    kRadixBinary, hRtlTestResult, inArr ),
                "negative Int32, kRadixBinary",
                "valueOf( negative Int32, radix 2 )"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32( kOctolNumsStr, kOctolNumsCount,
                                    kRadixOctol, hRtlTestResult, inArr ),
                "negative Int32, kRadixOctol",
                "valueOf( negative Int32, radix 8 )"
            );


    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32( kDecimalNumsStr, kDecimalNumsCount,
                             kRadixDecimal, hRtlTestResult, inArr ),
                "negative Int32, kRadixDecimal",
                "valueOf( negative Int32, radix 10 )"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32( kHexDecimalNumsStr, kHexDecimalNumsCount,
                                kRadixHexdecimal, hRtlTestResult, inArr ),
                "negative Int32, kRadixHexdecimal",
                "valueOf( negative Int32, radix 16 )"
            );


    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int32( kBase36NumsStr, kBase36NumsCount,
                                        kRadixBase36, hRtlTestResult, inArr ),
                "negative Int32, kRadixBase36",
                "valueOf( negative Int32, radix 36 )"
            );

    return ( bRes );
}
//------------------------------------------------------------------------
// testing the method valueOf( sal_Int32 l, sal_Int32 radix ) where radix = -5
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OString_valueOf_Int32_WrongRadix(
                                                hTestResult hRtlTestResult )
{
    sal_Bool bRes = sal_False;

    sal_Int32 intVal = 11;

    ::rtl::OString aStr1;
    ::rtl::OString aStr2("11");

    aStr1 = aStr1.valueOf( intVal, -5 );

    bRes = c_rtl_tres_state
            (
                hRtlTestResult,
                aStr2.compareTo( aStr1 ) == 0,
                "if radix not valid then radix must be 10",
                "valueOf(sal_Int32, sal_Int32 radix): radix = -5"
            );

    return (bRes);
}

//------------------------------------------------------------------------
// testing the method valueOf( sal_Int32 l, sal_Int32 radix )
// where l = -2147483648 (smallest negative value)
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OString_valueOf_Int32_SmallestNegativeValue(
    hTestResult  hRtlTestResult)
{
    // Standard-conforming way to assign -2147483648 to n:
    sal_Int32 n = -1;
    for (int i = 1; i < 32; ++i)
        n *= 2;
    return c_rtl_tres_state
            (
                hRtlTestResult,
                ::rtl::OString::valueOf(n) == "-2147483648",
                "-2147483648",
                "valueOf(sal_Int32 -2147483648)"
            );
}

//------------------------------------------------------------------------
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=2 )
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=8 )
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=10 )
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=16 )
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=36 )
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OString_valueOf_Int64(
                                                hTestResult hRtlTestResult )
{
    sal_Bool bRes = sal_False;

    bRes =  c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64((const char**)kBinaryNumsStr,
                        kBinaryNumsCount, kRadixBinary, hRtlTestResult, 0),
                "kRadixBinary",
                "valueOf(sal_Int64, radix 2)_"
            );

    bRes &=  c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64((const char**)kOctolNumsStr,
                            kOctolNumsCount, kRadixOctol, hRtlTestResult, 0),
                "kRadixOctol",
                "valueOf(sal_Int64, radix 8)_"
            );

    bRes &=  c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64((const char**)kDecimalNumsStr,
                        kDecimalNumsCount, kRadixDecimal, hRtlTestResult, 0),
                "kRadixDecimal",
                "valueOf(sal_Int64, radix 10)_"
            );
    bRes &=  c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64((const char**)kHexDecimalNumsStr,
                  kHexDecimalNumsCount, kRadixHexdecimal, hRtlTestResult, 0),
                "kRadixHexdecimal",
                "valueOf(sal_Int64, radix 16)_"
            );

    bRes &=  c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64((const char**)kBase36NumsStr,
                          kBase36NumsCount, kRadixBase36, hRtlTestResult, 0),
                "kRadixBase36",
                "valueOf(sal_Int64, radix 36)_"
            );

    return (bRes);
}

//------------------------------------------------------------------------
// testing the method valueOf( sal_Int64 l, sal_Int32 radix=2 )
// where l = large constants
// testing the method valueOf( sal_Int64 l, sal_Int32 radix=8 )
// where l = large constants
// testing the method valueOf( sal_Int64 l, sal_Int32 radix=10 )
// where l = large constants
// testing the method valueOf( sal_Int64 l, sal_Int32 radix=16 )
// where l = large constants
// testing the method valueOf( sal_Int64 l, sal_Int32 radix=36 )
// where l = large constants
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OString_valueOf_Int64_Bounderies(
                                                hTestResult hRtlTestResult )
{
    sal_Bool bRes = sal_False;

    bRes =  c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64((const char**)kBinaryMaxNumsStr,
                                    kInt64MaxNumsCount, kRadixBinary,
                                            hRtlTestResult, kInt64MaxNums),
                "kRadixBinary",
                "valueOf(salInt64, radix 2) Bounderies"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64((const char**)kOctolMaxNumsStr,
                                    kInt64MaxNumsCount, kRadixOctol,
                                            hRtlTestResult, kInt64MaxNums),
                "kRadixOctol",
                "valueOf(salInt64, radix 8) Bounderies"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64((const char**)kDecimalMaxNumsStr,
                                    kInt64MaxNumsCount, kRadixDecimal,
                                            hRtlTestResult, kInt64MaxNums),
                "kRadixDecimal",
                "valueOf(salInt64, radix 10) Bounderies"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64((const char**)kHexDecimalMaxNumsStr,
                                    kInt64MaxNumsCount, kRadixHexdecimal,
                                            hRtlTestResult, kInt64MaxNums),
                "kRadixHexdecimal",
                "valueOf(salInt64, radix 16) Bounderies"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64((const char**)kBase36MaxNumsStr,
                                    kInt64MaxNumsCount, kRadixBase36,
                                            hRtlTestResult, kInt64MaxNums),
                "kRadixBase36",
                "valueOf(salInt64, radix 36) Bounderies"
            );

    return ( bRes );
}

//------------------------------------------------------------------------
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=2 )
// for negative value
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=8 )
// for negative value
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=10 )
// for negative value
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=16 )
// for negative value
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=36 )
// for negative value
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OString_valueOf_Int64_Negative(
                                                hTestResult hRtlTestResult )
{
    sal_Bool bRes = sal_False;

    sal_Int64 inArr[36];
    sal_Int32 i;

    for (i = 0; i < 36; i++) {
        inArr[i] = -i;
    }


    bRes = c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64( kBinaryNumsStr, kBinaryNumsCount,
                                    kRadixBinary, hRtlTestResult, inArr ),
                "negative Int64, kRadixBinary",
                "valueOf( negative Int64, radix 2 )"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64( kOctolNumsStr, kOctolNumsCount,
                                        kRadixOctol, hRtlTestResult, inArr ),
                "negative Int64, kRadixOctol",
                "valueOf( negative Int64, radix 8 )"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64( kDecimalNumsStr, kDecimalNumsCount,
                                    kRadixDecimal, hRtlTestResult, inArr ),
                "negative Int64, kRadixDecimal",
                "valueOf( negative Int64, radix 10 )"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64( kHexDecimalNumsStr, kHexDecimalNumsCount,
                                    kRadixHexdecimal, hRtlTestResult, inArr ),
                "negative Int64, kRadixHexDecimal",
                "valueOf( negative Int64, radix 16 )"
            );

    bRes &= c_rtl_tres_state
            (
                hRtlTestResult,
                test_valueOf_Int64( kBase36NumsStr, kBase36NumsCount,
                                      kRadixBase36, hRtlTestResult, inArr),
                "negative Int64, kRadixBase36",
                "valueOf( negative Int64, radix 36 )"
            );

    return (bRes);
}
//------------------------------------------------------------------------
// testing the method valueOf( sal_Int64 l, sal_Int32 radix )
// where radix = -5
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OString_valueOf_Int64_WrongRadix(
                                                hTestResult hRtlTestResult )
{
    sal_Bool bRes = sal_False;

    sal_Int64 intVal = 11;

    ::rtl::OString aStr1;
    ::rtl::OString aStr2("11");

    aStr1 = aStr1.valueOf( intVal, -5 );

    bRes = c_rtl_tres_state
            (
                hRtlTestResult,
                aStr2.compareTo(aStr1) == 0,
                "if radix not valid then radix must be 10",
                "valueOf(sal_Int64, sal_Int32 radix): radix = -5"
            );

    return (bRes);
}

//------------------------------------------------------------------------
// testing the method valueOf( sal_Int64 l, sal_Int32 radix )
// where l = -9223372036854775808 (smallest negative value)
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OString_valueOf_Int64_SmallestNegativeValue(
    hTestResult hRtlTestResult)
{
    // Standard-conforming way to assign -9223372036854775808 to n:
    sal_Int64 n = -1;
    for (int i = 1; i < 64; ++i)
        n *= 2;
    return c_rtl_tres_state
            (
                hRtlTestResult,
                ::rtl::OString::valueOf(n) == "-9223372036854775808",
                "-9223372036854775808",
                "valueOf(sal_Int64 -9223372036854775808)"
            );
}

//------------------------------------------------------------------------
// testing the method valueOf( float f )
//------------------------------------------------------------------------
// LLA: sal_Bool SAL_CALL test_rtl_OString_valueOf_float(
// LLA:                         hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Char methName[MAXBUFLENGTH];
// LLA:     sal_Char* pMeth =methName;
// LLA:
// LLA:     typedef struct TestCase
// LLA:     {
// LLA:     sal_Char*                    comments;
// LLA:     float                         input1;
// LLA:     OString*                     expVal;
// LLA:
// LLA:     ~TestCase() {delete expVal;}
// LLA:     } TestCase;
// LLA:
// LLA:     TestCase arrTestCase[] =
// LLA:     {
// LLA:         { "3.0", 3.0, new OString("3.0") },
// LLA:         { "3.5", 3.5f, new OString("3.5")},
// LLA:         { "3.0625", 3.0625f, new OString("3.0625")},
// LLA:         { "3.502525", 3.502525f, new OString("3.502525") },
// LLA:         { "3.141592", 3.141592f, new OString("3.141592") },
// LLA:         { "3.5025255", 3.5025255f, new OString("3.5025255") },
// LLA:         { "3.0039062", 3.00390625f, new OString("3.0039062") }
// LLA:     };
// LLA:
// LLA:     sal_Bool res = sal_True;
// LLA:     sal_Int32 i;
// LLA:
// LLA:     for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
// LLA:     {
// LLA:         ::rtl::OString aStr1;
// LLA:         aStr1 = aStr1.valueOf( arrTestCase[i].input1 );
// LLA:         sal_Bool lastRes = (arrTestCase[i].expVal->compareTo(aStr1) == 0);
// LLA:
// LLA:         c_rtl_tres_state
// LLA:         (
// LLA:             hRtlTestResult,
// LLA:             lastRes,
// LLA:             arrTestCase[i].comments,
// LLA:             createName( pMeth, "valueof_float", i)
// LLA:         );
// LLA:
// LLA:         res &= lastRes;
// LLA:
// LLA:     }
// LLA:
// LLA:     return ( res );
// LLA: }




//------------------------------------------------------------------------
// testing the method valueOf( float f ) for negative value
//------------------------------------------------------------------------
// LLA: sal_Bool SAL_CALL test_rtl_OString_valueOf_Float_Negative(
// LLA:                                               hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Char methName[MAXBUFLENGTH];
// LLA:     sal_Char* pMeth =methName;
// LLA:
// LLA:     typedef struct TestCase
// LLA:     {
// LLA:     sal_Char*                    comments;
// LLA:     float                         input1;
// LLA:     OString*                     expVal;
// LLA:
// LLA:     ~TestCase() {delete expVal;}
// LLA:     } TestCase;
// LLA:
// LLA:     TestCase arrTestCase[] =
// LLA:     {
// LLA:         { "-3.0", -3.0, new OString("-3.0") },
// LLA:         { "-3.5", -3.5f, new OString("-3.5")},
// LLA:         { "-3.0625", -3.0625f, new OString("-3.0625")},
// LLA:         { "-3.502525", -3.502525f, new OString("-3.502525") },
// LLA:         { "-3.141592", -3.141592f, new OString("-3.141592") },
// LLA:         { "-3.5025255", -3.5025255f, new OString("-3.5025255") },
// LLA:         { "-3.0039062", -3.00390625f, new OString("-3.0039062") }
// LLA:     };
// LLA:
// LLA:     sal_Bool res = sal_True;
// LLA:     sal_Int32 i;
// LLA:
// LLA:     for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
// LLA:     {
// LLA:         ::rtl::OString aStr1;
// LLA:         aStr1 = aStr1.valueOf( arrTestCase[i].input1 );
// LLA:         sal_Bool lastRes = (arrTestCase[i].expVal->compareTo(aStr1) == 0);
// LLA:
// LLA:         c_rtl_tres_state
// LLA:         (
// LLA:             hRtlTestResult,
// LLA:             lastRes,
// LLA:             arrTestCase[i].comments,
// LLA:             createName( pMeth, "valueof_negative float", i)
// LLA:         );
// LLA:
// LLA:         res &= lastRes;
// LLA:
// LLA:     }
// LLA:
// LLA:     return ( res );
// LLA: }

//------------------------------------------------------------------------
// testing the method valueOf( double f )
//------------------------------------------------------------------------
// LLA: sal_Bool SAL_CALL test_rtl_OString_valueOf_double(
// LLA:                                                 hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Char methName[MAXBUFLENGTH];
// LLA:     sal_Char* pMeth =methName;
// LLA:
// LLA:     typedef struct TestCase
// LLA:     {
// LLA:     sal_Char*                    comments;
// LLA:     double                         input1;
// LLA:     OString*                     expVal;
// LLA:
// LLA:     ~TestCase() {delete expVal;}
// LLA:     } TestCase;
// LLA:
// LLA:     TestCase arrTestCase[] =
// LLA:     {
// LLA:         {"3.0", 3.0, new OString("3.0")},
// LLA:         {"3.5", 3.5, new OString("3.5")},
// LLA:         {"3.0625", 3.0625, new OString("3.0625")},
// LLA:         {"3.1415926535", 3.1415926535, new OString("3.1415926535")},
// LLA:         {"3.1415926535897931", 3.141592653589793,
// LLA:             new OString("3.1415926535897931")},
// LLA:         {"3.1415926535897931", 3.1415926535897932,
// LLA:             new OString("3.1415926535897931")},
// LLA:         {"3.1415926535897931", 3.14159265358979323,
// LLA:             new OString("3.1415926535897931")},
// LLA:         {"3.1415926535897931", 3.141592653589793238462643,
// LLA:             new OString("3.1415926535897931")}
// LLA:     };
// LLA:
// LLA:     sal_Bool res = sal_True;
// LLA:     sal_Int32 i;
// LLA:
// LLA:     for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
// LLA:     {
// LLA:         ::rtl::OString aStr1;
// LLA:         aStr1 = aStr1.valueOf( arrTestCase[i].input1 );
// LLA:         sal_Bool lastRes = (arrTestCase[i].expVal->compareTo(aStr1) == 0);
// LLA:
// LLA:         c_rtl_tres_state
// LLA:         (
// LLA:             hRtlTestResult,
// LLA:             lastRes,
// LLA:             arrTestCase[i].comments,
// LLA:             createName( pMeth, "valueof_double", i)
// LLA:         );
// LLA:
// LLA:         res &= lastRes;
// LLA:
// LLA:     }
// LLA:
// LLA:     return ( res );
// LLA: }


//------------------------------------------------------------------------
// testing the method valueOf( double f ) for negative value
//------------------------------------------------------------------------
// LLA: sal_Bool SAL_CALL test_rtl_OString_valueOf_Double_Negative(
// LLA:                                                 hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Char methName[MAXBUFLENGTH];
// LLA:     sal_Char* pMeth =methName;
// LLA:
// LLA:     typedef struct TestCase
// LLA:     {
// LLA:     sal_Char*                    comments;
// LLA:     double                         input1;
// LLA:     OString*                     expVal;
// LLA:
// LLA:     ~TestCase() {delete expVal;}
// LLA:     } TestCase;
// LLA:
// LLA:     TestCase arrTestCase[] =
// LLA:     {
// LLA:         {"-3.0", -3.0, new OString("-3.0")},
// LLA:         {"-3.5", -3.5, new OString("-3.5")},
// LLA:         {"-3.0625", -3.0625, new OString("-3.0625")},
// LLA:         {"-3.1415926535", -3.1415926535, new OString("-3.1415926535")},
// LLA:         {"-3.1415926535897931", -3.141592653589793,
// LLA:             new OString("-3.1415926535897931")},
// LLA:         {"-3.1415926535897931", -3.1415926535897932,
// LLA:             new OString("-3.1415926535897931")},
// LLA:         {"-3.1415926535897931", -3.14159265358979323,
// LLA:             new OString("-3.1415926535897931")},
// LLA:         {"-3.1415926535897931", -3.141592653589793238462643,
// LLA:             new OString("-3.1415926535897931")}
// LLA:     };
// LLA:
// LLA:     sal_Bool res = sal_True;
// LLA:     sal_Int32 i;
// LLA:
// LLA:     for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
// LLA:     {
// LLA:         ::rtl::OString aStr1;
// LLA:         aStr1 = aStr1.valueOf( arrTestCase[i].input1 );
// LLA:         sal_Bool lastRes = (arrTestCase[i].expVal->compareTo(aStr1) == 0);
// LLA:
// LLA:         c_rtl_tres_state
// LLA:         (
// LLA:             hRtlTestResult,
// LLA:             lastRes,
// LLA:             arrTestCase[i].comments,
// LLA:             createName( pMeth, "valueof_nagative double", i)
// LLA:         );
// LLA:
// LLA:         res &= lastRes;
// LLA:
// LLA:     }
// LLA:
// LLA:     return ( res );
// LLA: }

//------------------------------------------------------------------------
// testing the method valueOf()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_valueOf(
                                                hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start(hRtlTestResult, "valueOf");
    sal_Bool bTState = test_rtl_OString_valueOf_sal_Bool( hRtlTestResult );

    bTState &= test_rtl_OString_valueOf_sal_Char( hRtlTestResult );

    bTState &= test_rtl_OString_valueOf_Int32( hRtlTestResult );
    bTState &= test_rtl_OString_valueOf_Int32_Bounderies( hRtlTestResult );
    bTState &= test_rtl_OString_valueOf_Int32_Negative( hRtlTestResult );
    bTState &= test_rtl_OString_valueOf_Int32_WrongRadix( hRtlTestResult );
    bTState &= test_rtl_OString_valueOf_Int32_SmallestNegativeValue(
                                                            hRtlTestResult );

    bTState &= test_rtl_OString_valueOf_Int64( hRtlTestResult );
    bTState &= test_rtl_OString_valueOf_Int64_Bounderies( hRtlTestResult );
    bTState &= test_rtl_OString_valueOf_Int64_Negative( hRtlTestResult );
    bTState &= test_rtl_OString_valueOf_Int64_WrongRadix( hRtlTestResult );
    bTState &= test_rtl_OString_valueOf_Int64_SmallestNegativeValue(
                                                            hRtlTestResult );

    // LLA: the tests for valueOf(float) and valueOf(double) are moved to file
    //      sal/qa/rtl/ostring/rtl_OString2.cxx

    // LLA: bTState &= test_rtl_OString_valueOf_float( hRtlTestResult );
    // LLA: bTState &= test_rtl_OString_valueOf_Float_Negative( hRtlTestResult );

    // LLA: bTState &= test_rtl_OString_valueOf_double( hRtlTestResult );
    // LLA: bTState &= test_rtl_OString_valueOf_Double_Negative( hRtlTestResult );

    c_rtl_tres_state_end(hRtlTestResult, "valueOf");
//    return ( bTState );
}


//------------------------------------------------------------------------
// testing the method toChar()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_toChar(
                                                hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "toChar");
    typedef struct TestCase
    {
    sal_Char*                    comments;
    sal_Char                     expVal;
    OString*                     input1;
    ~TestCase() {delete input1;}
    } TestCase;


    TestCase arrTestCase[] =
    {
        {"A", 'A', new OString("A")},
        {"a", 'a', new OString("a")},
        {"0", '0',new OString("0")},
        {"-", '-',new OString("-")},
        {"_", '_',new OString("_")},
        {"¥", '¥',new OString("¥")},
        { "¦", '¦',new OString("¦")},
        {"ô", 'ô',new OString("ô")},
        {"†", '†',new OString("†")},
        {"\n", '\n',new OString("\n")},
        {"\'", '\'',new OString("\'")},
        {"\"", '\"',new OString("\"")},
        {"\0", '\0',new OString("\0")},
        {"", '\0',new OString("")},
        {"Sun Microsystems", 'S', new OString(kTestStr1)}
    };


    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++ )
    {
        sal_Char strRes = arrTestCase[i].input1->toChar();
        sal_Bool lastRes = ( strRes == arrTestCase[i].expVal );

        char com[MAXBUFLENGTH];
        com[0] = '\'';
        cpynstr(com + 1, (*arrTestCase[i].input1), MAXBUFLENGTH);
        int length = AStringLen( (*arrTestCase[i].input1) );
        com[length + 1] = '\'';
        com[length + 2] = 0;

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            com,
            createName( pMeth, "toChar", i )
        );

    }

    c_rtl_tres_state_end(hRtlTestResult, "toChar");
//    return (res);
}


//------------------------------------------------------------------------
// testing the method toFloat()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_toFloat(
                                            hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "toFloat");
    typedef struct TestCase
    {
    float                        expVal;
    OString*                     input1;
    ~TestCase() {delete input1;}
    } TestCase;


    TestCase arrTestCase[] =
    {
        {3.0f, new OString("3")},
        {3.1f, new OString("3.1")},
        {3.1415f, new OString("3.1415")},
        {3.14159f, new OString("3.14159")},
        {3.141592f, new OString("3.141592")},
        {3.1415926f, new OString("3.1415926")},
        {3.14159265f, new OString("3.14159265")},
        {3.141592653589793238462643f,
            new OString("3.141592653589793238462643")},
        {6.5822e-16f, new OString("6.5822e-16")},
        {9.1096e-31f, new OString("9.1096e-31")},
        {2.997925e8f, new OString("2.997925e8")},
        {6.241e18f, new OString("6.241e18")},
        {3.1f, new OString("03.1")},
        {3.1f, new OString(" 3.1")},
        {-3.1f, new OString("-3.1")},
        {3.1f, new OString("+3.1")},
        {0.0f, new OString("-0.0")}
    };


    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++ )
    {
        float fRes = arrTestCase[i].input1->toFloat();
        sal_Bool lastRes = ( fabs(fRes - arrTestCase[i].expVal) <= 1e-35 );

        char com[MAXBUFLENGTH];
        com[0] = '\'';
        cpynstr(com + 1, (*arrTestCase[i].input1), MAXBUFLENGTH);
        int length = AStringLen( (*arrTestCase[i].input1) );
        com[length + 1] = '\'';
        com[length + 2] = 0;

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            com,
            createName( pMeth, "toFloat", i )
        );

    }

    c_rtl_tres_state_end(hRtlTestResult, "toFloat");
//    return (res);
}


//------------------------------------------------------------------------
// testing the method toDouble()
//------------------------------------------------------------------------
// LLA: extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_toDouble(
// LLA:                                                 hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Char methName[MAXBUFLENGTH];
// LLA:     sal_Char* pMeth = methName;
// LLA:
// LLA:     c_rtl_tres_state_start(hRtlTestResult, "toDouble");
// LLA:     typedef struct TestCase
// LLA:     {
// LLA:     double                       expVal;
// LLA:     double                       expDiff;
// LLA:     OString*                     input1;
// LLA:     ~TestCase() {delete input1;}
// LLA:     } TestCase;
// LLA:
// LLA:
// LLA:     TestCase arrTestCase[] =
// LLA:     {
// LLA:         {3.0, 1e-35, new OString("3")},
// LLA:         {3.1, 1e-2, new OString("3.1")},
// LLA:         {3.1415, 1e-5, new OString("3.1415")},
// LLA:         {3.1415926535, 1e-11, new OString("3.1415926535")},
// LLA:         {3.141592653589793, 1e-15,
// LLA:             new OString("3.141592653589793")},
// LLA:         {3.1415926535897932, 1e-16,
// LLA:             new OString("3.1415926535897932")},
// LLA:         {3.14159265358979323, 1e-15,
// LLA:             new OString("3.14159265358979323")},
// LLA:         {3.141592653589793238462643, 1e-15,
// LLA:             new OString("3.141592653589793238462643")},
// LLA:         {6.5822e-16, 1e-20, new OString("6.5822e-16")},
// LLA:         {9.1096e-31, 1e-35, new OString("9.1096e-31")},
// LLA:         {2.997925e8, 10, new OString("2.997925e8")},
// LLA:         {6.241e18, 100, new OString("6.241e18")},
// LLA:         {1.7e-308, 1e-35, new OString("1.7e-308")},
// LLA:         {1.7e+308, 100, new OString("1.7e+308")},
// LLA:         {3.1, 1e-2, new OString("03.1")},
// LLA:         {3.1, 1e-2, new OString(" 3.1")},
// LLA:         {-3.1, 1e-2, new OString("-3.1")},
// LLA:         {3.1, 1e-2, new OString("+3.1")},
// LLA:         {0.0, 1e-2, new OString("-0.0")}
// LLA:     };
// LLA:
// LLA:     sal_Bool res = sal_True;
// LLA:     sal_Int32 i;
// LLA:
// LLA:     for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++ )
// LLA:     {
// LLA:         double dRes = arrTestCase[i].input1->toDouble();
// LLA:         double dErg = dRes - arrTestCase[i].expVal ;
// LLA:         double dComp = fabs( dErg );
// LLA:         sal_Bool lastRes = ( dComp <= arrTestCase[i].expDiff );
// LLA:
// LLA:         char com[MAXBUFLENGTH];
// LLA:         com[0] = '\'';
// LLA:         cpynstr(com + 1, (*arrTestCase[i].input1), MAXBUFLENGTH);
// LLA:         int length = AStringLen( (*arrTestCase[i].input1) );
// LLA:         com[length + 1] = '\'';
// LLA:         com[length + 2] = 0;
// LLA:
// LLA:         c_rtl_tres_state
// LLA:         (
// LLA:             hRtlTestResult,
// LLA:             lastRes,
// LLA:             com,
// LLA:             createName( pMeth, "toDouble", i )
// LLA:         );
// LLA:
// LLA:     }
// LLA:
// LLA:     c_rtl_tres_state_end(hRtlTestResult, "toDouble");
// LLA: //    return (res);
// LLA: }

//------------------------------------------------------------------------
// testing the method toBoolean()
//------------------------------------------------------------------------

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_toBoolean(
                                            hTestResult hRtlTestResult)
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "toBoolean");
    typedef struct TestCase
    {
    sal_Char*                    comments;
    sal_Bool                     expVal;
    OString*                     input;

    } TestCase;

    TestCase arrTestCase[]={

    {"expected true", sal_True, new OString("True")},
    {"expected false", sal_False, new OString("False")},
    {"expected true", sal_True, new OString("1")}
    };


    sal_Bool res = sal_True;
    sal_Int32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool bRes = arrTestCase[i].input->toBoolean();
        sal_Bool lastRes = (bRes == arrTestCase[i].expVal);
        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "toBoolean", i )

        );
        res &= lastRes;
    }
    c_rtl_tres_state_end(hRtlTestResult, "toBoolean");
//     return ( res );
}



//------------------------------------------------------------------------
// testing the method toInt32( sal_Int16 radix = 2,8,10,16,36 )
//------------------------------------------------------------------------
sal_Bool test_toInt32(  int num, const sal_Char** in,
const sal_Int32 *expVal,sal_Int16 radix, hTestResult hRtlTestResult )
{
    sal_Bool res = sal_True;
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;
    sal_Int32 i;

    for( i = 0; i < num; i++ )
    {
        OString str(in[i]);
        sal_Int32 intRes = str.toInt32(radix);
        sal_Bool lastRes = (intRes == expVal[i]);

        char buf[MAXBUFLENGTH];
        buf[0] = '\'';
        cpynstr( buf + 1, in[i], MAXBUFLENGTH );
        int length = AStringLen( in[i] );
        buf[length + 1] = '\'';
        buf[length + 2] = 0;

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            buf,
            createName( pMeth,"toInt32", i )
        );

        res &= lastRes;
    }

    return( res );
}

sal_Bool SAL_CALL test_rtl_OString_toInt32_wrongRadix(
                                                hTestResult hRtlTestResult )
{
    ::rtl::OString str("0");

    sal_Int32 iRes = str.toInt32(-1);

    return
    (
        c_rtl_tres_state
        (
            hRtlTestResult,
            iRes == 0,
            "wrong radix -1",
            "toInt32( 0, wrong radix -1 )"
        )
    );
}

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_toInt32(
                                                hTestResult hRtlTestResult )
{
    sal_Int32 expValues[kBase36NumsCount];
    sal_Int32 i;

    c_rtl_tres_state_start(hRtlTestResult, "toInt32");
    for ( i = 0; i < kBase36NumsCount; i++ )
            expValues[i] = i;

    sal_Bool res = c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt32( kBinaryNumsCount, kBinaryNumsStr,
                                expValues, kRadixBinary, hRtlTestResult ),
        "kBinaryNumsStr",
        "toInt32( radix 2 )"
    );
    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt32( kInt32MaxNumsCount, kBinaryMaxNumsStr,
                            kInt32MaxNums, kRadixBinary, hRtlTestResult ),
        "kBinaryMaxNumsStr",
        "toInt32_Boundaries( radix 2 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt32( kOctolNumsCount, kOctolNumsStr,
                            expValues, kRadixOctol, hRtlTestResult ),
        "kOctolNumsStr",
        "toInt32( radix 8 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt32( kInt32MaxNumsCount, kOctolMaxNumsStr,
                    (sal_Int32*)kInt32MaxNums, kRadixOctol, hRtlTestResult ),
        "kOctolMaxNumsStr",
        "toInt32_Boundaries( radix 8 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt32( kDecimalNumsCount, kDecimalNumsStr, expValues,
                                            kRadixDecimal, hRtlTestResult ),
        "kDecimalNumsStr",
        "toInt32( radix 10 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt32( kInt32MaxNumsCount, kDecimalMaxNumsStr,
                (sal_Int32*)kInt32MaxNums, kRadixDecimal, hRtlTestResult ),
        "kDecimalMaxNumsStr",
        "toInt32_Boundaries( radix 10 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt32( kHexDecimalNumsCount, kHexDecimalNumsStr, expValues,
                                        kRadixHexdecimal, hRtlTestResult ),
        "kHexDecimalNumsStr",
        "toInt32( radix 16 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt32( kInt32MaxNumsCount, kHexDecimalMaxNumsStr,
            (sal_Int32*)kInt32MaxNums, kRadixHexdecimal, hRtlTestResult ),
        "kHexDecimalMaxNumsStr",
        "toInt32_Boundaries( radix 16 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt32( kBase36NumsCount, kBase36NumsStr, expValues,
                                            kRadixBase36, hRtlTestResult ),
        "kBase36NumsStr",
        "toInt32( radix 36 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt32( kInt32MaxNumsCount, kBase36MaxNumsStr,
                (sal_Int32*)kInt32MaxNums, kRadixBase36, hRtlTestResult ),
        "kBase36MaxNumsStr",
        "toInt32_Boundaries( radix 36 )"
    );

    const sal_Int16 nSpecCases = 5;
    static const sal_Char *spString[nSpecCases] =
    {
        "-1",
        "+1",
        " 1",
        " -1",
        "001"
    };

    sal_Int32 expSpecVal[nSpecCases] =
    {
        -1,
        1,
        1,
        -1,
        1
    };

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt32( nSpecCases, spString, expSpecVal,
                                            kRadixDecimal, hRtlTestResult ),
        "special cases",
        "toInt32( specialcases )"
    );

    res &= test_rtl_OString_toInt32_wrongRadix( hRtlTestResult );

    c_rtl_tres_state_end(hRtlTestResult, "toInt32");
//    return ( res );
}

//------------------------------------------------------------------------
// testing the method toInt64( sal_Int16 radix = 2,8,10,16,36 )
//------------------------------------------------------------------------
sal_Bool test_toInt64( int num, const sal_Char** in,
const sal_Int64 *expVal,sal_Int16 radix, hTestResult hRtlTestResult )
{
    sal_Bool res = sal_True;
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;
    sal_Int32 i;

    for( i = 0; i < num; i++ )
    {
        OString str( in[i] );
        sal_Int64 intRes = str.toInt64( radix );
        sal_Bool lastRes = ( intRes == expVal[i] );

        char buf[MAXBUFLENGTH];
        buf[0] = '\'';
        cpynstr( buf + 1, in[i], MAXBUFLENGTH );
        int length = AStringLen(in[i]);
        buf[length + 1] = '\'';
        buf[length + 2] = 0;

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            buf,
            createName( pMeth, "toInt64", i )
        );

        res &= lastRes;
    }
    return (res);
}

sal_Bool SAL_CALL test_rtl_OString_toInt64_wrongRadix(
                                                hTestResult hRtlTestResult )
{
    ::rtl::OString str("0");

    sal_Int64 iRes = str.toInt64(-1);

    return (

        c_rtl_tres_state
        ( hRtlTestResult,
          iRes == 0,
          "wrong radix -1",
          "toInt64( wrong radix -1)"
        )
     );
}

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_toInt64(
                                                hTestResult hRtlTestResult )
{
    sal_Int64 expValues[kBase36NumsCount];
    sal_Int32 i;

    c_rtl_tres_state_start(hRtlTestResult, "toInt64");
    for (i = 0; i < kBase36NumsCount; expValues[i] = i, i++);

    sal_Bool res = c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt64( kBinaryNumsCount, kBinaryNumsStr, expValues,
                                            kRadixBinary, hRtlTestResult ),
        "kBinaryNumsStr",
        "toInt64( radix 2 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt64( kInt32MaxNumsCount, kBinaryMaxNumsStr,
                (sal_Int64*)kInt64MaxNums, kRadixBinary, hRtlTestResult ),
        "kBinaryMaxNumsStr",
        "toInt64_Boundaries( radix 2 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt64( kOctolNumsCount, kOctolNumsStr, expValues,
                                            kRadixOctol, hRtlTestResult ),
        "kOctolNumsStr",
        "toInt64( radix 8 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt64( kInt32MaxNumsCount, kOctolMaxNumsStr,
                    (sal_Int64*)kInt64MaxNums, kRadixOctol, hRtlTestResult ),
        "kOctolMaxNumsStr",
        "toInt64_Boundaries( radix 8 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt64( kDecimalNumsCount, kDecimalNumsStr, expValues,
                                            kRadixDecimal, hRtlTestResult ),
        "kDecimalNumsStr",
        "toInt64( radix 10 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt64( kInt32MaxNumsCount, kDecimalMaxNumsStr,
                (sal_Int64*)kInt64MaxNums, kRadixDecimal, hRtlTestResult ),
        "kDecimalMaxNumsStr",
        "toInt64_Boundaries( radix 10 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt64( kHexDecimalNumsCount, kHexDecimalNumsStr, expValues,
                                        kRadixHexdecimal, hRtlTestResult ),
        "kHexDecimalNumsStr",
        "toInt64( radix 16 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt64( kInt32MaxNumsCount, kHexDecimalMaxNumsStr,
            (sal_Int64*)kInt64MaxNums, kRadixHexdecimal, hRtlTestResult ),
        "kHexDecimalMaxNumsStr",
        "toInt64_Boundaries( radix 16 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt64( kBase36NumsCount, kBase36NumsStr, expValues,
                                           kRadixBase36, hRtlTestResult ),
        "kBase36NumsStr",
        "toInt64( radix 36 )"
    );

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt64( kInt32MaxNumsCount, kBase36MaxNumsStr,
            (sal_Int64*)kInt64MaxNums, kRadixBase36, hRtlTestResult ),
        "kBase36MaxNumsStr",
        "toInt64_Boundaries( radix 36 )"
    );



    const sal_Int16 nSpecCases = 5;
    static const sal_Char *spString[nSpecCases] =
    {
        "-1",
        "+1",
        " 1",
        " -1",
        "001"
    };

    sal_Int64 expSpecVal[nSpecCases] =
    {
        -1,
        1,
        1,
        -1,
        1
    };

    res &= c_rtl_tres_state
    (
        hRtlTestResult,
        test_toInt64( nSpecCases, spString, expSpecVal,
                                        kRadixDecimal, hRtlTestResult ),
        "special cases",
        "toInt64( specialcases )"
    );

    res &= test_rtl_OString_toInt64_wrongRadix( hRtlTestResult );

    c_rtl_tres_state_end(hRtlTestResult, "toInt64");
//    return (res);
}

//------------------------------------------------------------------------
// testing the method replace( sal_Char oldChar, sal_Char newChar )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_replace(
                                            hTestResult hRtlTestResult)
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "replace");
typedef struct TestCase
{
    sal_Char*                    comments;
    OString*                     expVal;
    OString*                     input;
    sal_Char                     oldChar;
    sal_Char                     newChar;

    ~TestCase() { delete input; delete expVal;}
} TestCase;

TestCase arrTestCase[]={

    {"string differs", new OString(kTestStr18),
        new OString(kTestStr4),'S','s'},
    {"string differs", new OString(kTestStr19),
        new OString(kTestStr17),(sal_Char)' ',(sal_Char)'-'},
    {"same string, no replace ", new OString(kTestStr22),
        new OString(kTestStr22),'*','8'}
    };


    sal_Bool res = sal_True;
    sal_Int32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        ::rtl::OString aStr1;
        aStr1= arrTestCase[i].input->replace(arrTestCase[i].oldChar,
                            arrTestCase[i].newChar);

        res &= c_rtl_tres_state
                        (
                            hRtlTestResult,
                            (arrTestCase[i].expVal->compareTo(aStr1) == 0),
                            arrTestCase[i].comments,
                            createName( pMeth, "replace", i )

                        );
    }
    c_rtl_tres_state_end(hRtlTestResult, "replace");
//     return ( res );
}



//------------------------------------------------------------------------
// testing the method replaceAt( sal_Int32 index, sal_Int32 count,
// const OString& newStr )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString_replaceAt(
                                                hTestResult hRtlTestResult)
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    c_rtl_tres_state_start(hRtlTestResult, "replaceAt");
    typedef struct TestCase
    {
        sal_Char*                    comments;
        OString*                     expVal;
        OString*                     input;
        OString*                     newStr;
        sal_Int32                    index;
        sal_Int32                    count;

        ~TestCase() { delete input; delete expVal; delete newStr;}
    } TestCase;

    TestCase arrTestCase[]=
    {

        { "string differs", new OString(kTestStr2), new OString(kTestStr22),
                                new OString(kTestStr2), 0, kTestStr22Len },

        { "larger index", new OString(kTestStr1), new OString(kTestStr7),
                                new OString(kTestStr8), 64, kTestStr8Len },

        { "larger count", new OString(kTestStr2), new OString(kTestStr22),
                                new OString(kTestStr2),0, 64 },

        { "navigate index", new OString(kTestStr2), new OString(kTestStr22),
                                new OString(kTestStr2), -64, 64 },

        { "null string", new OString(""),
                new OString(kTestStr14),new OString(""), 0, kTestStr14Len }
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        ::rtl::OString aStr1;
        aStr1= arrTestCase[i].input->replaceAt( arrTestCase[i].index,
                            arrTestCase[i].count, *arrTestCase[i].newStr );

        sal_Bool lastRes = ( arrTestCase[i].expVal->compareTo(aStr1) == 0 );

        c_rtl_tres_state
        (
            hRtlTestResult,
            lastRes,
            arrTestCase[i].comments,
            createName( pMeth, "replaceAt", i )

        );
        res &= lastRes;
    }
    c_rtl_tres_state_end(hRtlTestResult, "replaceAt");
//     return ( res );
}

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OString( hTestResult hRtlTestResult )
{

    c_rtl_tres_state_start(hRtlTestResult, "rtl_OString" );

    test_rtl_OString_ctors( hRtlTestResult );
    test_rtl_OString_getLength( hRtlTestResult );
    test_rtl_OString_equals( hRtlTestResult );
    test_rtl_OString_equalsIgnoreAsciiCase( hRtlTestResult );
    test_rtl_OString_compareTo( hRtlTestResult );
    test_rtl_OString_op_cmp( hRtlTestResult );
    test_rtl_OString_op_neq( hRtlTestResult );
    test_rtl_OString_op_g( hRtlTestResult );
    test_rtl_OString_op_l( hRtlTestResult );
    test_rtl_OString_op_ge( hRtlTestResult );
    test_rtl_OString_op_le( hRtlTestResult );
    test_rtl_OString_op_eq( hRtlTestResult );
    test_rtl_OString_op_plus( hRtlTestResult );
    test_rtl_OString_op_peq( hRtlTestResult );
    test_rtl_OString_op_cscs( hRtlTestResult );
    test_rtl_OString_getStr( hRtlTestResult );
    test_rtl_OString_copy( hRtlTestResult );
    test_rtl_OString_concat( hRtlTestResult );
    test_rtl_OString_toAsciiLowerCase( hRtlTestResult );
    test_rtl_OString_toAsciiUpperCase( hRtlTestResult );
    test_rtl_OString_trim( hRtlTestResult );
    test_rtl_OString_valueOf( hRtlTestResult );
    test_rtl_OString_toChar( hRtlTestResult );
    test_rtl_OString_toFloat( hRtlTestResult );
    // LLA: test_rtl_OString_toDouble( hRtlTestResult );
    test_rtl_OString_toBoolean( hRtlTestResult );
    test_rtl_OString_toInt32( hRtlTestResult );
    test_rtl_OString_toInt64( hRtlTestResult );
    test_rtl_OString_replace( hRtlTestResult );
    test_rtl_OString_replaceAt( hRtlTestResult );

    c_rtl_tres_state_end(hRtlTestResult, "rtl_OString");
}


// -----------------------------------------------------------------------------
void RegisterAdditionalFunctions(FktRegFuncPtr _pFunc)
{
    if (_pFunc)
    {
        (_pFunc)(&test_rtl_OString, "");

        //# (_pFunc)(&test_rtl_OString_ctors,                 "");
        //# (_pFunc)(&test_rtl_OString_getLength, "");
        //# (_pFunc)(&test_rtl_OString_equals, "");
        //# (_pFunc)(&test_rtl_OString_equalsIgnoreAsciiCase, "");
        //# (_pFunc)(&test_rtl_OString_compareTo, "");
        //# (_pFunc)(&test_rtl_OString_op_cmp, "");
        //# (_pFunc)(&test_rtl_OString_op_neq, "");
        //# (_pFunc)(&test_rtl_OString_op_g, "");
        //# (_pFunc)(&test_rtl_OString_op_l, "");
        //# (_pFunc)(&test_rtl_OString_op_ge, "");
        //# (_pFunc)(&test_rtl_OString_op_le, "");
        //# (_pFunc)(&test_rtl_OString_op_eq, "");
        //# (_pFunc)(&test_rtl_OString_op_plus, "");
        //# (_pFunc)(&test_rtl_OString_op_peq, "");
        //# (_pFunc)(&test_rtl_OString_op_cscs, "");
        //# (_pFunc)(&test_rtl_OString_getStr, "");
        //# (_pFunc)(&test_rtl_OString_copy, "");
        //# (_pFunc)(&test_rtl_OString_concat, "");
        //# (_pFunc)(&test_rtl_OString_toAsciiLowerCase, "");
        //# (_pFunc)(&test_rtl_OString_toAsciiUpperCase, "");
        //# (_pFunc)(&test_rtl_OString_trim, "");
        //# (_pFunc)(&test_rtl_OString_valueOf, "");
        //# (_pFunc)(&test_rtl_OString_toChar, "");
        //# (_pFunc)(&test_rtl_OString_toFloat, "");
        //# (_pFunc)(&test_rtl_OString_toDouble, "");
        //# (_pFunc)(&test_rtl_OString_toBoolean, "");
        //# (_pFunc)(&test_rtl_OString_toInt32, "");
        //# (_pFunc)(&test_rtl_OString_toInt64, "");
        //# (_pFunc)(&test_rtl_OString_replace, "");
        //# (_pFunc)(&test_rtl_OString_replaceAt, "");
    }
}

/*
D:\local\644\SRX644\sal\qa\rtl_OString.cxx(3559) : error C2664:
'unsigned char (void (__cdecl *)(void *),const char *)'
 : cannot convert parameter 1 from
'unsigned char (__cdecl *)(void *)' to 'void (__cdecl *)(void *)'

        This conversion requires a reinterpret_cast, a C-style cast or function-
style cast
*/
