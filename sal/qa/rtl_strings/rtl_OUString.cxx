/*************************************************************************
 *
 *  $RCSfile: rtl_OUString.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 16:44:07 $
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

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

//# #ifndef _RTL_TRES_H_
//#     #include <rtl/tres.h>
//# #endif
#include <testshl/tresstatewrapper.hxx>
#include "stringhelper.hxx"

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _RTL_STRING_CONST_H_
#include <rtl_String_Const.h>
#endif

#ifndef _RTL_STRING_UTILS_HXX_
#include <rtl_String_Utils.hxx>
#endif



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
static sal_Bool test_rtl_OUString_ctor_001( hTestResult hRtlTestResult )
{

    ::rtl::OUString aUStr;
    rtl_uString * pData = aUStr.pData;

    return
        (
            c_rtl_tres_state
            (
                hRtlTestResult,
                pData->length == 0 &&
                ! *pData->buffer,
                "New OUString containing no characters",
                "ctor_001"
                )
            );
}

//------------------------------------------------------------------------

static sal_Bool SAL_CALL test_rtl_OUString_ctor_002(
    hTestResult hRtlTestResult )
{
    ::rtl::OUString aUStr( kTestStr1,
                           kTestStr1Len,
                           kEncodingRTLTextUSASCII,
                           kConvertFlagsOStringToOUString
                           );

    return
        (
            c_rtl_tres_state
            (
                hRtlTestResult,
                aUStr == aUStr1,
                "OUString from an ascii string",
                "ctor_002"
                )
            );
}
//------------------------------------------------------------------------

static sal_Bool SAL_CALL test_rtl_OUString_ctor_003(
    hTestResult hRtlTestResult )
{
    rtl_uString  *rtlUStr =NULL ;
    rtl_uString_newFromAscii( &rtlUStr, kTestStr1 );
    ::rtl::OUString aUStr( rtlUStr );

    return
        (
            c_rtl_tres_state
            (
                hRtlTestResult,
                aUStr == aUStr1,
                "New OUString from a rtl_uString",
                "ctor_003"
                )
            );
}

//------------------------------------------------------------------------

static sal_Bool SAL_CALL test_rtl_OUString_ctor_004(
    hTestResult hRtlTestResult)
{
    ::rtl::OUString aUStr( aUStr1 );

    return
        (
            c_rtl_tres_state
            (
                hRtlTestResult,
                aUStr == aUStr1,
                "New OUString from unicode string",
                "ctor_004"
                )
            );
}
//------------------------------------------------------------------------

static sal_Bool test_rtl_OUString_ctor_005( hTestResult hRtlTestResult )
{
    ::rtl::OUString aUStr( aUStr2, kTestStr1Len );

    return
        (
            c_rtl_tres_state
            (
                hRtlTestResult,
                aUStr == aUStr1,
                "New OUString from the first n characters of unicode string",
                "ctor_004"
                )
            );

}


//------------------------------------------------------------------------

static sal_Bool test_rtl_OUString_ctor_006( hTestResult hRtlTestResult )
{
    ::rtl::OUString aUStrtmp( aUStr1 );
    ::rtl::OUString aUStr( aUStrtmp );

    return
        (
            c_rtl_tres_state
            (
                hRtlTestResult,
                aUStr==aUStrtmp,
                "New OUString from another OUString",
                "ctor_006"
                )
            );
}

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_ctors(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "ctors");
    sal_Bool DCState = test_ini_uString();
    sal_Bool bTSState = test_rtl_OUString_ctor_001( hRtlTestResult );
    bTSState &= test_rtl_OUString_ctor_002( hRtlTestResult);
    bTSState &= test_rtl_OUString_ctor_003( hRtlTestResult);
    bTSState &= test_rtl_OUString_ctor_004( hRtlTestResult);
    bTSState &= test_rtl_OUString_ctor_005( hRtlTestResult);
    bTSState &= test_rtl_OUString_ctor_006( hRtlTestResult);
    c_rtl_tres_state_end( hRtlTestResult, "ctors");
//    return( bTSState );
}



//------------------------------------------------------------------------
// testing the method getLength
//------------------------------------------------------------------------

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_getLength(
    hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "getLength");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*       comments;
        sal_Int32       expVal;
        OUString*        input;
        ~TestCase()     { delete input;}
    } TestCase;

    TestCase arrTestCase[]={

        {"length of ascii string", kTestStr1Len,
         new OUString( kTestStr1,
                       kTestStr1Len,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString)},
        {"length of ascci string of size 1", 1,
         new OUString( "1",
                       1,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString)},
        {"length of empty string (default constructor)", 0, new OUString()},
        {"length of empty string (empty ascii string arg)",0,
         new OUString( "",
                       0,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString)},
        {"length of empty string (string arg = '\\0')", 0,
         new OUString( "\0",
                       0,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString)}
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
    c_rtl_tres_state_end( hRtlTestResult, "getLength");
//    return ( res );
}

//------------------------------------------------------------------------
// testing the method equals( const OString & aStr )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_equals(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "equals");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*       comments;
        sal_Bool         expVal;
        OUString*        input1;
        OUString*        input2;
        ~TestCase()     { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[]={

        {"same size", sal_True, new OUString(aUStr1), new OUString(aUStr1)},
        {"different size", sal_False, new OUString(aUStr1),
         new OUString(aUStr2)
        },
        {"same size, no case match", sal_False, new OUString(aUStr1),
         new OUString(aUStr3)
        },
        {"two empty strings(def. constructor)", sal_True, new OUString(),
         new OUString()
        },
        {"empty(def.constructor) and non empty", sal_False, new OUString(),
         new OUString(aUStr2)
        },
        {"non empty and empty(def. constructor)", sal_False,
         new OUString(aUStr1),
         new OUString()
        },
        {"two empty strings(string arg = '\\0')", sal_True,
         new OUString( "",
                       0,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString),
         new OUString( "",
                       0,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString)
        },
        {"empty(string arg = '\\0') and non empty", sal_False,
         new OUString( "",
                       0,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString),
         new OUString(aUStr2)
        },
        {"non empty and empty(string arg = '\\0')", sal_False,
         new OUString(aUStr1),
         new OUString( "",
                       0,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString)
        }
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
    c_rtl_tres_state_end( hRtlTestResult, "equals");
//    return (res);
}

//------------------------------------------------------------------------
// testing the method equalsIgnoreAsciiCase( const OString & aStr )
//------------------------------------------------------------------------

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_equalsIgnoreAsciiCase(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "equalsIgnoreAsciiCase");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*       comments;
        sal_Bool         expVal;
        OUString*        input1;
        OUString*        input2;
        ~TestCase()     { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[]={
        {"same strings but different cases",sal_True,new OUString(aUStr4),
         new OUString(aUStr5)
        },
        {"same strings",sal_True,new OUString(aUStr4),
         new OUString(aUStr4)},
        {"with equal beginning",sal_False,new OUString(aUStr2),
         new OUString(aUStr4)
        },
        {"empty(def.constructor) and non empty",sal_False,new OUString(),
         new OUString(aUStr5)
        },
        {"non empty and empty(def.constructor)",sal_False,
         new OUString(aUStr4),
         new OUString()
        },
        {"two empty strings(def.constructor)",sal_True,new OUString(),
         new OUString()
        },
        {"different strings with equal length",sal_False,
         new OUString(aUStr10),
         new OUString(aUStr11)
        }
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool lastRes =
            (arrTestCase[i].input1->equalsIgnoreAsciiCase(*arrTestCase[i].input2) ==
             arrTestCase[i].expVal);

        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                arrTestCase[i].comments,
                createName( pMeth, "equalsIgnoreAsciiCase", i )
                );

        res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "equalsIgnoreAsciiCase");
//    return (res);
}


static sal_Bool SAL_CALL test_rtl_OUString_compareTo_001(
    hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*       comments;
        sal_Int32         expVal;
        OUString*        input1;
        OUString*        input2;
        ~TestCase()     { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[]={

        {"simple compare, str1 to str5",-1,new OUString(aUStr1),
         new OUString(aUStr5)
        },
        {"simple compare, str2 to str5",-1,new OUString(aUStr2),
         new OUString(aUStr5)
        },
        {"simple compare, str1 to str9",-1,new OUString(aUStr1),
         new OUString(aUStr9)
        },
        {"simple compare, str1 to str2",-1,new OUString(aUStr1),
         new OUString(aUStr2)
        },
        {"simple compare, str4 to str5",-1,new OUString(aUStr4),
         new OUString(aUStr5)
        },
        {"simple compare, str1 to str3",-1,new OUString(aUStr1),
         new OUString(aUStr3)
        },
        {"simple compare, str5 to str1",+1,new OUString(aUStr5),
         new OUString(aUStr1)
        },
        {"simple compare, str2 to str1",+1,new OUString(aUStr2),
         new OUString(aUStr1)
        },
        {"simple compare, str9 to str5",+1,new OUString(aUStr9),
         new OUString(aUStr5)
        },
        {"simple compare, str5 to str4",+1,new OUString(aUStr5),
         new OUString(aUStr4)
        },
        {"simple compare, str1 to str1",0,new OUString(aUStr1),
         new OUString(aUStr1)
        },
        {"simple compare, nullString to nullString",0,new OUString(),
         new OUString()
        },
        {"simple compare, nullString to str2",-1,new OUString(),
         new OUString(aUStr2)
        },
        {"simple compare, str1 to nullString",+1,new OUString(aUStr1),
         new OUString()
        }
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Int32 cmpRes = arrTestCase[i].input1->compareTo
            (*arrTestCase[i].input2);
        cmpRes = ( cmpRes == 0 ) ? 0 : ( cmpRes > 0 ) ? +1 : -1 ;
        sal_Bool lastRes = ( cmpRes == arrTestCase[i].expVal);


        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                arrTestCase[i].comments,
                createName( pMeth, "compareTo_001(const OString&)", i )
                );

        res &= lastRes;
    }

    return (res);
}


//------------------------------------------------------------------------
//  testing the method compareTo( const OString & rObj, sal_Int32 length )
//------------------------------------------------------------------------
static sal_Bool SAL_CALL test_rtl_OUString_compareTo_002(
    hTestResult hRtlTestResult )
{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*       comments;
        sal_Int32       expVal;
        sal_Int32       maxLength;
           OUString*        input1;
        OUString*       input2;
        ~TestCase()     { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"compare with maxlength, str1 to str9, 16",-1,16,
         new OUString(aUStr1), new OUString(aUStr9)},
        {"compare with maxlength, str2 to str9, 32",-1,32,
         new OUString(aUStr2), new OUString(aUStr9)},
        {"compare with maxlength, str9 to str4, 16",+1,16,
         new OUString(aUStr9), new OUString(aUStr4)},
        {"compare with maxlength, str9 to str22, 32",+1,32,
         new OUString(aUStr9), new OUString(aUStr22)},
        {"compare with maxlength, str9 to str5, 16",0,16,
         new OUString(aUStr9), new OUString(aUStr5)},
        {"compare with maxlength, str9 to str9, 32",0,32,
         new OUString(aUStr9), new OUString(aUStr9)},
        {"compare with maxlength, str1 to str2, 32",-1,32,
         new OUString(aUStr1), new OUString(aUStr2)},
        {"compare with maxlength, str1 to str2, 32",-1,32,
         new OUString(aUStr1), new OUString(aUStr2)},
        {"compare with maxlength, str1 to str2", 0,-1,
         new OUString(aUStr1), new OUString(aUStr2)}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Int32 cmpRes = arrTestCase[i].input1->compareTo
            (*arrTestCase[i].input2, arrTestCase[i].maxLength);
        cmpRes = (cmpRes == 0) ? 0 : (cmpRes > 0) ? +1 : -1 ;
        sal_Bool lastRes = (cmpRes == arrTestCase[i].expVal);

        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                arrTestCase[i].comments,
                createName( pMeth, "compareTo_002(const OString&, sal_Int32)", i )
                );

        res &= lastRes;
    }

    return (res);
}

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_compareTo(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "compareTo");
    sal_Bool res = test_rtl_OUString_compareTo_001(hRtlTestResult);
    res &= test_rtl_OUString_compareTo_002(hRtlTestResult);
    c_rtl_tres_state_end( hRtlTestResult, "compareTo");
//    return (res);
}

//------------------------------------------------------------------------
// testing the method match( const OUString & str, sal_Int32 fromIndex = 0 )
//------------------------------------------------------------------------
static sal_Bool SAL_CALL test_rtl_OUString_match_001(
    hTestResult hRtlTestResult)

{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*       comments;
        sal_Bool         expVal;
        OUString*        input1;
        OUString*        input2;
        ~TestCase()     { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[]={

        {"aUStr2 and aUStr1", sal_True, new OUString(aUStr2),
         new OUString(aUStr1)},
        {"aUStr1 and aUStr2", sal_False, new OUString(aUStr1),
         new OUString(aUStr2)},
        {"aUStr5 and aUStr6", sal_False, new OUString(aUStr5),
         new OUString(aUStr6)},
        {"null and aUStr1", sal_False, new OUString( "",
                                                     0,
                                                     kEncodingRTLTextUSASCII,
                                                     kConvertFlagsOStringToOUString),
         new OUString(aUStr1)}

    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool lastRes = ( arrTestCase[i].input1->match(
            *(arrTestCase[i].input2)) == arrTestCase[i].expVal );

        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                arrTestCase[i].comments,
                createName( pMeth, "match(const OUString & str)", i  )
                );

        res &= lastRes;
    }
    return (res);
}

static sal_Bool SAL_CALL test_rtl_OUString_match_002(
    hTestResult hRtlTestResult )

{
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*       comments;
        sal_Bool         expVal;
        sal_Int32       fromIndex;
        OUString*       input1;
        OUString*       input2;
        ~TestCase()     { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[]={

        {"aUStr2 from 17 and aUStr6", sal_True, 17,
         new OUString(aUStr2),new OUString(aUStr6)},
        {"aUStr2 from 5 and aUStr6", sal_False, 5,
         new OUString(aUStr2),new OUString(aUStr6)},
        {"aUStr2 from 0 and aUStr1", sal_True, 0,
         new OUString(aUStr2),new OUString(aUStr1)},
        {"aUStr1 from 16 and null", sal_True, 16,
         new OUString(aUStr1),
         new OUString( "",
                       0,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString
                       )
        },
        {"aUStr1 from 5 and null", sal_True, 5,
         new OUString(aUStr1),
         new OUString( "",
                       0,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString
                       )
        },
        {"aUStr2 from -1 and aUStr1", sal_False, -1,
         new OUString(aUStr2),new OUString(aUStr1)},
        {"aUStr5 from 2 and aUStr4", sal_False, 2,
         new OUString(aUStr5),new OUString(aUStr4)},
        {"aUStr2 from 18 and aUStr1", sal_False, 18,
         new OUString(aUStr2),new OUString(aUStr1)}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool lastRes = ( arrTestCase[i].input1->match
                             (*(arrTestCase[i].input2),arrTestCase[i].fromIndex) ==
                             arrTestCase[i].expVal );

        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                arrTestCase[i].comments,
                createName( pMeth,
                            "match(const OUString & str,sal_Int32 fromIndex = 0)", i  )

                );

        res &= lastRes;
    }
    return (res);
}

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_match(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "match");
    sal_Bool res = test_rtl_OUString_match_001(hRtlTestResult);
    res &= test_rtl_OUString_match_002(hRtlTestResult);
    c_rtl_tres_state_end( hRtlTestResult, "match");
//    return (res);
}

//------------------------------------------------------------------------
// testing the operator +=
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_op_eq(
    hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "eq");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*       comments;
           OUString*        input1;
        OUString*        input2;
        ~TestCase()     { delete input1; delete input2;}
    } TestCase;

    TestCase arrTestCase[]={
        {"null and Ustr1", new OUString, new OUString(aUStr1)},
        {"Ustr2 and Ustr1", new OUString(aUStr2),
         new OUString(aUStr1)},
        {""" and Ustr1 from bit charactor buffer",
         new OUString(aUStr1),
         new OUString( "",
                       0,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString
                       )},
        {"Ustr1 and Ustr2 from value and length",
         new OUString( aUStr2, kTestStr2Len ),
         new OUString(aUStr1)}
    };


    sal_Bool res = sal_True;
    sal_Int32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        *(arrTestCase[i].input1) = *(arrTestCase[i].input2);
        sal_Bool lastRes = (*(arrTestCase[i].input1) ==
                            *(arrTestCase[i].input2));
        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                arrTestCase[i].comments,
                createName( pMeth, "op_eq", i )

                );
        res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "eq");
//    return ( res );
}

//------------------------------------------------------------------------
// testing the operator +=
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_op_peq(
    hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "peq");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*       comments;
        OUString*       expVal;
           OUString*        input1;
        OUString*        input2;
        ~TestCase()     { delete input1; delete input2;}
    } TestCase;

    TestCase arrTestCase[]={
        {" ' '= ''+='' ", new OUString( "",
                                        0,
                                        kEncodingRTLTextUSASCII,
                                        kConvertFlagsOStringToOUString
                                        ),
         new OUString( "",
                       0,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString
                       ),
         new OUString( "",
                       0,
                       kEncodingRTLTextUSASCII,
                       kConvertFlagsOStringToOUString
                       )},
        {"Ustr1= null += Ustr1", new OUString(aUStr1),
         new OUString(), new OUString(aUStr1)},
        {"Ustr1= '' += Ustr1", new OUString(aUStr1),
         /*new OUString( "",
           0,
           kEncodingRTLTextUSASCII,
           kConvertFlagsOStringToOUString
           )*/
         new OUString(),
         new OUString(aUStr1)},
        {"Ustr1PlusUStr6 = Ustr1 += Ustr6",
         new OUString(aUStr1PlusUStr6), new OUString(aUStr1),
         new OUString(aUStr6)},
    };


    sal_Bool res = sal_True;
    sal_Int32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        *(arrTestCase[i].input1) += *(arrTestCase[i].input2);
        sal_Bool lastRes = (*(arrTestCase[i].expVal) ==
                            *(arrTestCase[i].input1));
        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                arrTestCase[i].comments,
                createName( pMeth, "op_peq", i )

                );
        res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "peq");
//    return ( res );
}

//------------------------------------------------------------------------
// testing the operator const sal_Unicode * (csuc for short)
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_csuc(
    hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "csuc");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    const sal_Unicode tmpUC=0x0;
    rtl_uString* tmpUstring = NULL;
    const sal_Char *tmpStr=kTestStr1;
    sal_Int32 tmpLen=(sal_Int32) kTestStr1Len;
    sal_Int32 cmpLen = 0;

    rtl_string2UString( &tmpUstring, tmpStr,  tmpLen,
                        osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );


    typedef struct TestCase
    {
        sal_Char*               comments;
        const sal_Unicode*      expVal;
        sal_Int32               cmpLen;
        OUString*                input1;
        ~TestCase() {  delete input1;}
     } TestCase;

    TestCase arrTestCase[] =
    {
          {"test normal ustring",(*tmpUstring).buffer,kTestStr1Len,
         new OUString(aUStr1)},
        {"test empty ustring",&tmpUC, 1, new OUString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;
    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        const sal_Unicode* pstr = *arrTestCase[i].input1;

        res &= c_rtl_tres_state
            (
                hRtlTestResult,
                cmpstr((sal_Char*)pstr,
                       (sal_Char*)arrTestCase[i].expVal,
                       arrTestCase[i].cmpLen),
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
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_getStr(
    hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "getStr");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    const sal_Unicode tmpUC=0x0;
    rtl_uString* tmpUstring = NULL;
    const sal_Char *tmpStr=kTestStr1;
    sal_Int32 tmpLen=(sal_Int32) kTestStr1Len;
    sal_Int32 cmpLen = 0;

    rtl_string2UString( &tmpUstring, tmpStr,  tmpLen,
                        osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );


    typedef struct TestCase
    {
        sal_Char*               comments;
        const sal_Unicode*      expVal;
        sal_Int32               cmpLen;
        OUString*                input1;
        ~TestCase() {  delete input1;}
     } TestCase;

    TestCase arrTestCase[] =
    {
          {"test normal ustring",(*tmpUstring).buffer,kTestStr1Len,
         new OUString(aUStr1)},
        {"test empty ustring",&tmpUC, 1, new OUString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;
    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        const sal_Unicode* pstr = arrTestCase[i].input1->getStr();

        res &= c_rtl_tres_state
            (
                hRtlTestResult,
                cmpstr((sal_Char*)pstr,
                       (sal_Char*)arrTestCase[i].expVal,
                       arrTestCase[i].cmpLen),
                arrTestCase[i].comments,
                createName( pMeth, "getStr", i )
                );
    }
    c_rtl_tres_state_end( hRtlTestResult, "getStr");
//    return ( res );
}


//------------------------------------------------------------------------
// testing the method sal_Int32 reverseCompareTo( const OUString & str )
//-------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_reverseCompareTo(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "reverseCompareTo");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*       comments;
        sal_Int32         expVal;
        OUString*        input1;
        OUString*        input2;
        ~TestCase()     { delete input1;delete input2;}
    } TestCase;

    TestCase arrTestCase[]={

        {"simple compare, str1 to str5",-1,new OUString(aUStr1),
         new OUString(aUStr5)
        },
        {"simple compare, str2 to str5",1,new OUString(aUStr2),
         new OUString(aUStr5)
        },
        {"simple compare, str1 to str9",-1,new OUString(aUStr1),
         new OUString(aUStr9)
        },
        {"simple compare, str4 to str5",-1,new OUString(aUStr4),
         new OUString(aUStr5)
        },
        {"simple compare, str5 to str1",+1,new OUString(aUStr5),
         new OUString(aUStr1)
        },
        {"simple compare, str2 to str1",+1,new OUString(aUStr2),
         new OUString(aUStr1)
        },
        {"simple compare, str1 to str1",0,new OUString(aUStr1),
         new OUString(aUStr1)
        },
        {"simple compare, nullString to nullString",0,new OUString(),
         new OUString()
        },
        {"simple compare, nullString to str2",-1,new OUString(),
         new OUString(aUStr2)
        },
        {"simple compare, str1 to nullString",+1,new OUString(aUStr1),
         new OUString()
        }
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Int32 cmpRes = arrTestCase[i].input1->reverseCompareTo
            (*arrTestCase[i].input2);
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
    c_rtl_tres_state_end( hRtlTestResult, "reverseCompareTo");
//    return (res);
}

//------------------------------------------------------------------------
//       testing the method sal_Bool equalsAscii( const sal_Char* asciiStr )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_equalsAscii(
    hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "equalsAscii");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    const sal_Char *tmpAstr1="Sun Microsystems\0";
    const sal_Char *tmpAstr2="\0";



    typedef struct TestCase
    {
        sal_Char*        comments;
        sal_Bool         expVal;
           OUString*         input1;
        const sal_Char*    input2;
        ~TestCase()     { delete input1;}
    } TestCase;

    TestCase arrTestCase[]={
        {"str1 with str1 ", sal_True, new OUString( kTestStr1,
                                                    kTestStr1Len,
                                                    kEncodingRTLTextUSASCII,
                                                    kConvertFlagsOStringToOUString
                                                    ), tmpAstr1},
        {"str2 with str1 ", sal_False,new OUString( kTestStr2,
                                                    kTestStr2Len,
                                                    kEncodingRTLTextUSASCII,
                                                    kConvertFlagsOStringToOUString
                                                    ), tmpAstr1},
        {"null with str1  ", sal_False, new OUString(), tmpAstr1},
        {"null with '' ", sal_True, new OUString(), tmpAstr2},
        {"'' with ''", sal_True, new OUString( "",
                                               0,
                                               kEncodingRTLTextUSASCII,
                                               kConvertFlagsOStringToOUString
                                               ),
         tmpAstr2}

    };


    sal_Bool res = sal_True;
    sal_Int32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {

        sal_Bool lastRes = (arrTestCase[i].expVal ==
                            arrTestCase[i].input1->equalsAscii(arrTestCase[i].input2));
        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                arrTestCase[i].comments,
                createName( pMeth, "equalsAscii", i )

                );
        res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "equalsAscii");
//    return ( res );
}



//------------------------------------------------------------------------
//       testing the method sal_Bool equalsAsciiL(
//                          const sal_Char* asciiStr, sal_Int32 asciiStrLength )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_equalsAsciiL(
    hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "equalsAsciiL");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    const sal_Char *tmpAstr1="Sun Microsystems\0";
    const sal_Char *tmpAstr2="\0";
    const sal_Char *tmpAstr3="Sun Microsystems Java Technology\0";



    typedef struct TestCase
    {
        sal_Char*        comments;
        sal_Bool         expVal;
           OUString*         input1;
        const sal_Char*    input2;
        sal_Int32          cmpLen;
        ~TestCase()     { delete input1;}
    } TestCase;

    TestCase arrTestCase[]={
        {"str1 with str1,str1Len ", sal_True, new OUString( kTestStr1,
                                                            kTestStr1Len,
                                                            kEncodingRTLTextUSASCII,
                                                            kConvertFlagsOStringToOUString
                                                            ), tmpAstr1, kTestStr1Len},
        {"str2 with str1,str1Len", sal_False,new OUString( kTestStr2,
                                                           kTestStr2Len,
                                                           kEncodingRTLTextUSASCII,
                                                           kConvertFlagsOStringToOUString
                                                           ), tmpAstr1, kTestStr1Len},
        {"str1 with str2,str1Len", sal_True,new OUString( kTestStr1,
                                                          kTestStr1Len,
                                                          kEncodingRTLTextUSASCII,
                                                          kConvertFlagsOStringToOUString
                                                          ), tmpAstr3, kTestStr1Len},
        {"null with str1,1  ", sal_False, new OUString(), tmpAstr1, 1},
        {"null with '',1 ", sal_False, new OUString(), tmpAstr2, 1},
        {"'' with '', 1", sal_False, new OUString( "",
                                                   0,
                                                   kEncodingRTLTextUSASCII,
                                                   kConvertFlagsOStringToOUString
                                                   ),
         tmpAstr2, 1}

    };


    sal_Bool res = sal_True;
    sal_Int32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {

        sal_Bool lastRes = (arrTestCase[i].expVal ==
                            arrTestCase[i].input1->equalsAsciiL(arrTestCase[i].input2,
                                                                arrTestCase[i].cmpLen)
                            );
        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                arrTestCase[i].comments,
                createName( pMeth, "equalsAsciiL", i )

                );
        res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "equalsAsciiL");
//    return ( res );
}

//------------------------------------------------------------------------
//       testing the method sal_Int32 compareToAscii( const sal_Char* asciiStr )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_compareToAscii(
    hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "compareToAscii");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    const sal_Char *tmpAstr1="Sun Microsystems\0";
    const sal_Char *tmpAstr2="\0";
    const sal_Char *tmpAstr3="sun microsystems java technology\0";
    const sal_Char *tmpAstr4="Sun Microsystems Java Technology\0";


    typedef struct TestCase
    {
        sal_Char*        comments;
        sal_Int32        expVal;
           OUString*         input1;
        const sal_Char*    input2;
        ~TestCase()     { delete input1;}
    } TestCase;

    TestCase arrTestCase[]={
        {"str1 with str1 ", 0, new OUString( kTestStr1,
                                             kTestStr1Len,
                                             kEncodingRTLTextUSASCII,
                                             kConvertFlagsOStringToOUString
                                             ), tmpAstr1},
        {"str1 with '' ", 83, new OUString( kTestStr1,
                                            kTestStr1Len,
                                            kEncodingRTLTextUSASCII,
                                            kConvertFlagsOStringToOUString
                                            ), tmpAstr2},
        {"null with str1  ", -83 , new OUString(), tmpAstr1},
        {"null with '' ", 0, new OUString(), tmpAstr2},
        {"str1 with str9", -32, new OUString( kTestStr1,
                                              kTestStr1Len,
                                              kEncodingRTLTextUSASCII,
                                              kConvertFlagsOStringToOUString
                                              ),
         tmpAstr3},
        {"str1 with str2", -32, new OUString( kTestStr1,
                                              kTestStr1Len,
                                              kEncodingRTLTextUSASCII,
                                              kConvertFlagsOStringToOUString
                                              ),
         tmpAstr4}

    };


    sal_Bool res = sal_True;
    sal_Int32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        sal_Bool lastRes = (arrTestCase[i].expVal ==
                            arrTestCase[i].input1->compareToAscii(arrTestCase[i].input2));

        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                arrTestCase[i].comments,
                createName( pMeth, "equalsAscii", i )

                );
        res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "compareToAscii");
//    return ( res );
}


//------------------------------------------------------------------------
//       testing the method valueOf( sal_Bool b )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_valueOf_sal_Bool(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "Bool");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*       comments;
        OUString*       expVal;
        sal_Bool        input1;

        ~TestCase()     {delete expVal;}
    }TestCase;

    TestCase arrTestCase[]=
    {
        {"input Bool 'true' and return OUString 'true'",
         new OUString("true",4,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString),
         sal_True
        },
        {"input Bool 'false' and return OUString 'false'",
         new OUString("false",5,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString),
         sal_False
        }
    };

    sal_Bool    res;
    sal_Int32   i;

    for(i=0;i<(sizeof(arrTestCase))/(sizeof(TestCase));i++)
    {
        sal_Bool lastRes=(*arrTestCase[i].expVal==
                          OUString::valueOf(arrTestCase[i].input1)

                          );

        c_rtl_tres_state(hRtlTestResult,
                         lastRes,
                         arrTestCase[i].comments,
                         createName( pMeth, "valueOf( sal_Bool b )", i )
                         );

        res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "Bool");
//    return(res);
}

//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_valueOf_sal_Unicode(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "Unicode");
    sal_Char    methName[MAXBUFLENGTH];
    sal_Char*   pMeth = methName;

    sal_Unicode tmpchar1=97;
    sal_Unicode tmpchar2=53;
    sal_Unicode tmpchar3=0;
    sal_Unicode tmpchar4=32;
    sal_Unicode tmpchar5=47;

    typedef struct TestCase
    {
        sal_Char*       comments;
        OUString*       expVal;
        sal_Unicode     input1;

        ~TestCase()     {delete expVal;}
    }TestCase;

    TestCase arrTestCase[]=
    {
        {"input Unicode 'a' and return OUString 'a'",
         new OUString(&tmpchar1,1),tmpchar1
        },
        {"input Unicode '5' and return OUString '5'",
         new OUString(&tmpchar2,1), tmpchar2
        },
        {"input Unicode 0 and return OUString 0",
         new OUString(&tmpchar3,1),tmpchar3
        },
        {"input Unicode ' ' and return OUString ' '",
         new OUString(&tmpchar4,1),tmpchar4
        },
        {"input Unicode '/' and return OUString ' '",
         new OUString(&tmpchar5,1),tmpchar5
        }
    };

    sal_Bool    res=sal_True;
    sal_Int32   i;

    for(i=0;i<(sizeof(arrTestCase))/(sizeof(TestCase));i++)
    {
        sal_Bool lastRes=(*(arrTestCase[i].expVal)==
                          OUString::valueOf(arrTestCase[i].input1));

        c_rtl_tres_state(hRtlTestResult,
                         lastRes,
                         arrTestCase[i].comments,
                         createName( pMeth, "valueOf( sal_Unicode c )", i )
                         );

        res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "Unicode");
//    return(res);
}


/**
 * Calls the method valueOf(T, radix) and compares
 * returned ustrings with ustrings that passed in the array resArray.
 *
 * @param T, type of argument, passed to valueOf
 * @param resArray, array of result ustrings to compare to
 * @param n the number of elements in the array resArray (testcases)
 * @param pTestResult the instance of the class TestResult
 * @param inArray [optional], array of value that is passed as first argument
 *                            to valueOf
 *
 * @return true, if all returned ustrings are equal to corresponding ustring in
 *               resArray else, false.
 */
template <class T>
sal_Bool test_valueOf( const char** resArray, int n, sal_Int16 radix,
                       hTestResult hRtlTestResult, const T *inArray )
{
    sal_Bool    bRes = sal_True;

    sal_Char    methName[MAXBUFLENGTH];
    sal_Char*   pMeth = methName;
    sal_Int32   i;
    static sal_Unicode aUchar[50]={0x12};

    for (i = 0; i < n; i++)
    {
        ::rtl::OUString aStr1;

        OSL_ENSURE( i < 50, "ERROR: leave aUchar bound");

        AStringToUStringCopy(aUchar,resArray[i]);
        ::rtl::OUString aStr2(aUchar);
        ::rtl::OUString aStr3( "-",1,kEncodingRTLTextUSASCII,
                               kConvertFlagsOStringToOUString);

        if (inArray == 0)
        {
            aStr1 = ::rtl::OUString::valueOf((T)i, radix);
        }
        else
        {
            if ( inArray[i] < 0 )
            {
                sal_Unicode   aStr4[100];
                if(AStringToUStringCopy(aStr4,resArray[i]))
                {
                    aStr2 = aStr3;
                    aStr2 += aStr4;
                }

            }
            aStr1 = ::rtl::OUString::valueOf((T)inArray[i], radix);
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
sal_Bool SAL_CALL test_rtl_OUString_valueOf_Int32(
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
sal_Bool SAL_CALL test_rtl_OUString_valueOf_Int32_Bounderies(
    hTestResult hRtlTestResult )
{
    sal_Bool bRes = sal_False;

    bRes =  c_rtl_tres_state
        (
            hRtlTestResult,
            test_valueOf_Int32((const char**)kBinaryMaxNumsStr,
                               kInt32MaxNumsCount, kRadixBinary,
                               hRtlTestResult, kInt32MaxNums),
            "kRadixBinary",
            "valueOf(salInt32, radix 2) Bounderies"
            );

    bRes &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_valueOf_Int32((const char**)kOctolMaxNumsStr,
                               kInt32MaxNumsCount, kRadixOctol,
                               hRtlTestResult, kInt32MaxNums),
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
sal_Bool SAL_CALL test_rtl_OUString_valueOf_Int32_Negative(
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
sal_Bool SAL_CALL test_rtl_OUString_valueOf_Int32_WrongRadix(
    hTestResult hRtlTestResult )
{
    sal_Bool bRes = sal_False;

    sal_Int32 intVal = 11;

    ::rtl::OUString aStr1;
    ::rtl::OUString aStr2("11",2,kEncodingRTLTextUSASCII,
                          kConvertFlagsOStringToOUString);

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
static sal_Bool SAL_CALL test_rtl_OUString_valueOf_Int32_defaultParam(
    hTestResult hRtlTestResult )
{
    sal_Char     methName[MAXBUFLENGTH];
    sal_Char*    pMeth = methName;

    sal_Char*    newUChar1="15";
    sal_Char*    newUChar2="0";
    sal_Char*    newUChar3="-15";
    sal_Char*    newUChar4="2147483647";
    sal_Char*    newUChar5="-2147483648";

    typedef struct TestCase
    {
        sal_Char*       comments;
        sal_Int32       input1;
        OUString*       expVal;
        ~TestCase()     {delete expVal;}
    }TestCase;

    TestCase arrTestCase[]=
    {
        {"input Int32 15 and return OUString 15",15,
         new OUString(newUChar1,2,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)
        },
        {"input Int32 0 and return OUString 0",0,
         new OUString(newUChar2,1,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)
        },
        {"input Int32 -15 and return OUString -15",-15,
         new OUString(newUChar3,3,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)
        },
        {"input Int32 2147483647 and return OUString 2147483647",2147483647,
         new OUString(newUChar4,10,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)
        },
        {"input Int32 -2147483648 and return OUString -2147483648",
         -2147483648,
         new OUString(newUChar5,11,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)
        }
    };

    sal_Bool    res=sal_True;
    sal_Int32   i;

    for(i=0;i<(sizeof(arrTestCase))/(sizeof(TestCase));i++)
    {
        sal_Bool lastRes=(*(arrTestCase[i].expVal)==
                          OUString::valueOf(arrTestCase[i].input1));

        c_rtl_tres_state(hRtlTestResult,
                         lastRes,
                         arrTestCase[i].comments,
                         createName( pMeth,
                                     "valueOf( sal_Int32 i, sal_Int16 radix = 10 )", i )
                         );

        res &= lastRes;
    }

    return(res);

}
//------------------------------------------------------------------------
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=2 )
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=8 )
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=10 )
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=16 )
// testing the method valueOf( sal_Int64 l, sal_Int16 radix=36 )
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OUString_valueOf_Int64(
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
sal_Bool SAL_CALL test_rtl_OUString_valueOf_Int64_Bounderies(
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
sal_Bool SAL_CALL test_rtl_OUString_valueOf_Int64_Negative(
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
sal_Bool SAL_CALL test_rtl_OUString_valueOf_Int64_WrongRadix(
    hTestResult hRtlTestResult )
{
    sal_Bool bRes = sal_False;

    sal_Int64 intVal = 11;

    ::rtl::OUString aStr1;
    ::rtl::OUString aStr2("11",2,kEncodingRTLTextUSASCII,
                          kConvertFlagsOStringToOUString);

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
static sal_Bool SAL_CALL test_rtl_OUString_valueOf_Int64_defaultParam(
    hTestResult hRtlTestResult )
{
    sal_Char     methName[MAXBUFLENGTH];
    sal_Char*    pMeth = methName;

    sal_Char*    newUChar1="15";
    sal_Char*    newUChar2="0";
    sal_Char*    newUChar3="-15";
    sal_Char*    newUChar4="9223372036854775807";
    sal_Char*    newUChar5="-9223372036854775808";

    typedef struct TestCase
    {
        sal_Char*       comments;
        sal_Int64       input1;
        OUString*       expVal;
        ~TestCase()     {delete expVal;}
    }TestCase;

    TestCase arrTestCase[]=
    {
        {"input Int64 15 and return OUString 15",15,
         new OUString(newUChar1,2,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)
        },
        {"input Int64 0 and return OUString 0",0,
         new OUString(newUChar2,1,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)
        },
        {"input Int64 -15 and return OUString -15",-15,
         new OUString(newUChar3,3,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)
        },
        {"input Int64 9223372036854775807 and return 9223372036854775807",
         9223372036854775807,
         new OUString(newUChar4,19,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)
        },
        {"input Int64 -9223372036854775808 and return -9223372036854775808",
         9223372036854775808,
         new OUString(newUChar5,20,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)
        }
    };

    sal_Bool    res=sal_True;
    sal_Int32   i;

    for(i=0;i<(sizeof(arrTestCase))/(sizeof(TestCase));i++)
    {
        sal_Bool lastRes=(*(arrTestCase[i].expVal)==
                          OUString::valueOf(arrTestCase[i].input1));

        c_rtl_tres_state(hRtlTestResult,
                         lastRes,
                         arrTestCase[i].comments,
                         createName( pMeth,
                                     "valueOf( sal_Int64 i, sal_Int16 radix = 10 )", i )
                         );

        res &= lastRes;
    }

    return(res);

}
//------------------------------------------------------------------------
// testing the method valueOf( float f )
//------------------------------------------------------------------------
// LLA: sal_Bool SAL_CALL test_rtl_OUString_valueOf_float(
// LLA:     hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Char methName[MAXBUFLENGTH];
// LLA:     sal_Char* pMeth =methName;
// LLA:
// LLA:     typedef struct TestCase
// LLA:     {
// LLA:         sal_Char*                    comments;
// LLA:         float                         input1;
// LLA:         OUString*                     expVal;
// LLA:
// LLA:         ~TestCase() {delete expVal;}
// LLA:     } TestCase;
// LLA:
// LLA:     TestCase arrTestCase[] =
// LLA:     {
// LLA:         { "3.0", 3.0, new OUString("3.0",3,kEncodingRTLTextUSASCII,
// LLA:                                    kConvertFlagsOStringToOUString) },
// LLA:         { "3.5", 3.5f, new OUString("3.5",3,kEncodingRTLTextUSASCII,
// LLA:                                     kConvertFlagsOStringToOUString)},
// LLA:         { "3.0625", 3.0625f, new OUString("3.0625",6,kEncodingRTLTextUSASCII,
// LLA:                                           kConvertFlagsOStringToOUString)},
// LLA:         { "3.502525", 3.502525f, new OUString("3.502525",8,
// LLA:                                               kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString) },
// LLA:         { "3.141592", 3.141592f, new OUString("3.141592",8,
// LLA:                                               kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString) },
// LLA:         { "3.5025255", 3.5025255f, new OUString("3.5025255",9,
// LLA:                                                 kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString) },
// LLA:         { "3.0039062", 3.00390625f, new OUString("3.0039062",9,
// LLA:                                                  kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString) }
// LLA:     };
// LLA:
// LLA:     sal_Bool res = sal_True;
// LLA:     sal_Int32 i;
// LLA:
// LLA:     for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
// LLA:     {
// LLA:         ::rtl::OUString aStr1;
// LLA:         aStr1 = aStr1.valueOf( arrTestCase[i].input1 );
// LLA:         sal_Bool lastRes = (arrTestCase[i].expVal->compareTo(aStr1) == 0);
// LLA:
// LLA:         c_rtl_tres_state
// LLA:             (
// LLA:                 hRtlTestResult,
// LLA:                 lastRes,
// LLA:                 arrTestCase[i].comments,
// LLA:                 createName( pMeth, "valueof_float", i)
// LLA:                 );
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
// LLA: sal_Bool SAL_CALL test_rtl_OUString_valueOf_Float_Negative(
// LLA:     hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Char methName[MAXBUFLENGTH];
// LLA:     sal_Char* pMeth =methName;
// LLA:
// LLA:     typedef struct TestCase
// LLA:     {
// LLA:         sal_Char*                    comments;
// LLA:         float                         input1;
// LLA:         OUString*                     expVal;
// LLA:
// LLA:         ~TestCase() {delete expVal;}
// LLA:     } TestCase;
// LLA:
// LLA:     TestCase arrTestCase[] =
// LLA:     {
// LLA:         { "-3.0", -3.0, new OUString("-3.0",4,kEncodingRTLTextUSASCII,
// LLA:                                      kConvertFlagsOStringToOUString) },
// LLA:         { "-3.5", -3.5f, new OUString("-3.5",4,kEncodingRTLTextUSASCII,
// LLA:                                       kConvertFlagsOStringToOUString)},
// LLA:         { "-3.0625", -3.0625f, new OUString("-3.0625",7,
// LLA:                                             kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString)},
// LLA:         { "-3.502525", -3.502525f, new OUString("-3.502525",9,
// LLA:                                                 kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString) },
// LLA:         { "-3.141592", -3.141592f, new OUString("-3.141592",9,
// LLA:                                                 kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString) },
// LLA:         { "-3.5025255", -3.5025255f, new OUString("-3.5025255",10,
// LLA:                                                   kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString) },
// LLA:         { "-3.0039062", -3.00390625f, new OUString("-3.0039062",10,
// LLA:                                                    kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString) }
// LLA:     };
// LLA:
// LLA:     sal_Bool res = sal_True;
// LLA:     sal_Int32 i;
// LLA:
// LLA:     for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
// LLA:     {
// LLA:         ::rtl::OUString aStr1;
// LLA:         aStr1 = aStr1.valueOf( arrTestCase[i].input1 );
// LLA:         sal_Bool lastRes = (arrTestCase[i].expVal->compareTo(aStr1) == 0);
// LLA:
// LLA:         c_rtl_tres_state
// LLA:             (
// LLA:                 hRtlTestResult,
// LLA:                 lastRes,
// LLA:                 arrTestCase[i].comments,
// LLA:                 createName( pMeth, "valueof_negative float", i)
// LLA:                 );
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
// LLA: sal_Bool SAL_CALL test_rtl_OUString_valueOf_double(
// LLA:     hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Char methName[MAXBUFLENGTH];
// LLA:     sal_Char* pMeth =methName;
// LLA:
// LLA:     typedef struct TestCase
// LLA:     {
// LLA:         sal_Char*       comments;
// LLA:         double          input1;
// LLA:         OUString*       expVal;
// LLA:
// LLA:         ~TestCase() {delete expVal;}
// LLA:     } TestCase;
// LLA:
// LLA:     TestCase arrTestCase[] =
// LLA:     {
// LLA:         {"3.0", 3.0, new OUString("3.0",3,kEncodingRTLTextUSASCII,
// LLA:                                   kConvertFlagsOStringToOUString)},
// LLA:         {"3.5", 3.5, new OUString("3.5",3,kEncodingRTLTextUSASCII,
// LLA:                                   kConvertFlagsOStringToOUString)},
// LLA:         {"3.0625", 3.0625, new OUString("3.0625",6,kEncodingRTLTextUSASCII,
// LLA:                                         kConvertFlagsOStringToOUString)},
// LLA:         {"3.1415926535", 3.1415926535, new OUString("3.1415926535",12,
// LLA:                                                     kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString)},
// LLA:         {"3.1415926535897931", 3.141592653589793,
// LLA:          new OUString("3.1415926535897931",18,kEncodingRTLTextUSASCII,
// LLA:                       kConvertFlagsOStringToOUString)},
// LLA:         {"3.1415926535897931", 3.1415926535897932,
// LLA:          new OUString("3.1415926535897931",18,kEncodingRTLTextUSASCII,
// LLA:                       kConvertFlagsOStringToOUString)},
// LLA:         {"3.1415926535897931", 3.14159265358979323,
// LLA:          new OUString("3.1415926535897931",18,kEncodingRTLTextUSASCII,
// LLA:                       kConvertFlagsOStringToOUString)},
// LLA:         {"3.1415926535897931", 3.141592653589793238462643,
// LLA:          new OUString("3.1415926535897931",18,kEncodingRTLTextUSASCII,
// LLA:                       kConvertFlagsOStringToOUString)}
// LLA:     };
// LLA:
// LLA:     sal_Bool res = sal_True;
// LLA:     sal_Int32 i;
// LLA:
// LLA:     for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
// LLA:     {
// LLA:         ::rtl::OUString aStr1;
// LLA:         aStr1 = aStr1.valueOf( arrTestCase[i].input1 );
// LLA:         sal_Bool lastRes = (arrTestCase[i].expVal->compareTo(aStr1) == 0);
// LLA:
// LLA:         c_rtl_tres_state
// LLA:             (
// LLA:                 hRtlTestResult,
// LLA:                 lastRes,
// LLA:                 arrTestCase[i].comments,
// LLA:                 createName( pMeth, "valueof_double", i)
// LLA:                 );
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
// LLA: sal_Bool SAL_CALL test_rtl_OUString_valueOf_Double_Negative(
// LLA:     hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Char methName[MAXBUFLENGTH];
// LLA:     sal_Char* pMeth =methName;
// LLA:
// LLA:     typedef struct TestCase
// LLA:     {
// LLA:         sal_Char*        comments;
// LLA:         double           input1;
// LLA:         OUString*        expVal;
// LLA:
// LLA:         ~TestCase() {delete expVal;}
// LLA:     } TestCase;
// LLA:
// LLA:     TestCase arrTestCase[] =
// LLA:     {
// LLA:         {"-3.0", -3.0, new OUString("-3.0",4,kEncodingRTLTextUSASCII,
// LLA:                                     kConvertFlagsOStringToOUString)},
// LLA:         {"-3.5", -3.5, new OUString("-3.5",4,kEncodingRTLTextUSASCII,
// LLA:                                     kConvertFlagsOStringToOUString)},
// LLA:         {"-3.0625", -3.0625, new OUString("-3.0625",7,kEncodingRTLTextUSASCII,
// LLA:                                           kConvertFlagsOStringToOUString)},
// LLA:         {"-3.1415926535", -3.1415926535,
// LLA:          new OUString("-3.1415926535",13,kEncodingRTLTextUSASCII,
// LLA:                       kConvertFlagsOStringToOUString)},
// LLA:         {"-3.1415926535897931", -3.141592653589793,
// LLA:          new OUString("-3.1415926535897931",19,kEncodingRTLTextUSASCII,
// LLA:                       kConvertFlagsOStringToOUString)},
// LLA:         {"-3.1415926535897931", -3.1415926535897932,
// LLA:          new OUString("-3.1415926535897931",19,kEncodingRTLTextUSASCII,
// LLA:                       kConvertFlagsOStringToOUString)},
// LLA:         {"-3.1415926535897931", -3.14159265358979323,
// LLA:          new OUString("-3.1415926535897931",19,kEncodingRTLTextUSASCII,
// LLA:                       kConvertFlagsOStringToOUString)},
// LLA:         {"-3.1415926535897931", -3.141592653589793238462643,
// LLA:          new OUString("-3.1415926535897931",19,kEncodingRTLTextUSASCII,
// LLA:                       kConvertFlagsOStringToOUString)}
// LLA:     };
// LLA:
// LLA:     sal_Bool res = sal_True;
// LLA:     sal_Int32 i;
// LLA:
// LLA:     for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
// LLA:     {
// LLA:         ::rtl::OUString aStr1;
// LLA:         aStr1 = aStr1.valueOf( arrTestCase[i].input1 );
// LLA:         sal_Bool lastRes = (arrTestCase[i].expVal->compareTo(aStr1) == 0);
// LLA:
// LLA:         c_rtl_tres_state
// LLA:             (
// LLA:                 hRtlTestResult,
// LLA:                 lastRes,
// LLA:                 arrTestCase[i].comments,
// LLA:                 createName( pMeth, "valueof_nagative double", i)
// LLA:                 );
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
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_valueOf(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "valueOf");
    sal_Bool bTState = test_rtl_OUString_valueOf_Int32( hRtlTestResult );
    bTState &= test_rtl_OUString_valueOf_Int32_Bounderies( hRtlTestResult );
    bTState &= test_rtl_OUString_valueOf_Int32_Negative( hRtlTestResult );
    bTState &= test_rtl_OUString_valueOf_Int32_WrongRadix( hRtlTestResult );
    bTState &= test_rtl_OUString_valueOf_Int32_defaultParam(
        hRtlTestResult );
    bTState &= test_rtl_OUString_valueOf_Int64( hRtlTestResult );
    bTState &= test_rtl_OUString_valueOf_Int64_Bounderies( hRtlTestResult );
    bTState &= test_rtl_OUString_valueOf_Int64_Negative( hRtlTestResult );
    bTState &= test_rtl_OUString_valueOf_Int64_WrongRadix( hRtlTestResult );
    bTState &= test_rtl_OUString_valueOf_Int64_defaultParam(
        hRtlTestResult );
    // LLA: bTState &= test_rtl_OUString_valueOf_float( hRtlTestResult );
    // LLA: bTState &= test_rtl_OUString_valueOf_Float_Negative( hRtlTestResult );

    // LLA: bTState &= test_rtl_OUString_valueOf_double( hRtlTestResult );
    // LLA: bTState &= test_rtl_OUString_valueOf_Double_Negative( hRtlTestResult );
    c_rtl_tres_state_end( hRtlTestResult, "valueOf");
//    return ( bTState );
}
//------------------------------------------------------------------------
//    this is my testing code
//    testing the method createFromAscii( const sal_Char * value )
//------------------------------------------------------------------------

extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_createFromAscii(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "createFromAscii");
    sal_Char     methName[MAXBUFLENGTH];
    sal_Char*    pMeth = methName;


    typedef struct TestCase
    {
        sal_Char*       comments;
        const sal_Char*  input1;
        OUString*       expVal;
        ~TestCase()     {delete expVal;}

    }TestCase;

    TestCase arrTestCase[]=
    {

        {   "create OUString from sal_Char" ,kTestStr1,
            new OUString(kTestStr1,kTestStr1Len,kEncodingRTLTextUSASCII,
                         kConvertFlagsOStringToOUString)
        },
        {
            "create OUString from empty", "",
            new OUString()
        },
        {
            "create OUString from empty(string arg = '\\0')","",
            new OUString("",0,kEncodingRTLTextUSASCII,
                         kConvertFlagsOStringToOUString)
        }

    };

    sal_Bool    res;
    sal_Int32   i;

    for(i=0;i<(sizeof(arrTestCase))/(sizeof(TestCase));i++)
    {
        sal_Bool lastRes=(*(arrTestCase[i].expVal)==
                          OUString::createFromAscii(arrTestCase[i].input1));


    {
        c_rtl_tres_state(hRtlTestResult,
                         lastRes,
                         arrTestCase[i].comments,
                         createName( pMeth, "createFromAscii", i )
                         );
    }

    res&=lastRes;

    }

    c_rtl_tres_state_end( hRtlTestResult, "createFromAscii");
//    return(res);
}
//------------------------------------------------------------------------
// testing the method index(  )
//------------------------------------------------------------------------
template <class T>
sal_Bool test_index( const T* input1, int num,const sal_Int32* input2,
                     const sal_Int32* expVal,int base,rtlTestResult hRtlTestResult)
{
    sal_Bool    res=sal_True;
    sal_Char    methName[MAXBUFLENGTH];
    sal_Char    *meth;
    sal_Char*   pMeth=methName;
    sal_Int32   i;
    sal_Bool    lastRes=sal_False;

    for(i=0;i<num;i++)
    {
        OUString str(aUStr2);

        if(base==0)
        {
            lastRes=(str.indexOf(input1[i])==expVal[i]);
            meth="indexOf_001";
        }
        if(base==1)
        {
            lastRes=(str.indexOf(input1[i],input2[i])==expVal[i]);
            meth="indexOf_002";
        }
// LLA:         if(base==2)
// LLA:         {
// LLA:             lastRes=(str.lastIndexOf(input1[i])==expVal[i]);
// LLA:             meth="lastIndexOf_001(sal_Unicode)";
// LLA:         }
// LLA:         if(base==3)
// LLA:         {
// LLA: /*
// LLA:             OUString s4(&input1[i]);
// LLA:             rtl::OString sStr;
// LLA:             sStr <<= str;
// LLA:             printf("# str = %s\n", sStr.getStr());
// LLA:             rtl::OString sInput1;
// LLA:             sInput1 <<= s4; // rtl::OUString((sal_Unicode*)input1[i]);
// LLA:             printf("# %d = lastIndexOf(\"%s\", %d) =? %d\n", str.lastIndexOf(input1[i], input2[i]), sInput1.getStr(), input2[i], expVal[i]);
// LLA: */
// LLA:             lastRes=(str.lastIndexOf(input1[i],input2[i])==expVal[i]);
// LLA:             meth="lastIndexOf_002(sal_Unicode , sal_Int32 )";
// LLA:         }

        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                   "index",
                createName( pMeth,meth, i )
                );

        res &= lastRes;
    }

    return( res );
}
template <class T>
sal_Bool test_indexStr( const T** input1, int num,const sal_Int32* input2,
                        const sal_Int32* expVal,int base,rtlTestResult hRtlTestResult)
{
    sal_Bool    res=sal_True;
    sal_Char    methName[MAXBUFLENGTH];
    sal_Char    *meth;
    sal_Char*   pMeth=methName;
    sal_Int32   i;
    sal_Bool    lastRes=sal_False;

    for(i=0;i<num;i++)
    {
        OUString str(aUStr2);


        if(base==0)
        {
            OUString s1(input1[i]);
            lastRes=(str.indexOf(s1)==expVal[i]);
            meth="indexOf_003";
        }
        if(base==1)
        {
            OUString s2(input1[i]);
            lastRes=(str.indexOf(s2,input2[i])==expVal[i]);
            meth="indexOf_004";
        }
// LLA:         if(base==2)
// LLA:         {
// LLA:             OUString s3(input1[i]);
// LLA:             lastRes=(str.lastIndexOf(s3)==expVal[i]);
// LLA:             meth="lastIndexOf_003(const OUString)";
// LLA:         }
// LLA:         if(base==3)
// LLA:         {
// LLA:             OUString s4(input1[i]);
// LLA:
// LLA:             rtl::OString sStr;
// LLA:             sStr <<= str;
// LLA:             printf("# str = \"%s\"\n", sStr.getStr());
// LLA:             rtl::OString sInput1;
// LLA:             sInput1 <<= s4; // rtl::OUString((sal_Unicode*)input1[i]);
// LLA:             printf("# %d = lastIndexOf(\"%s\", %d) =? %d\n", str.lastIndexOf(input1[i], input2[i]), sInput1.getStr(), input2[i], expVal[i]);
// LLA:
// LLA:             lastRes=(str.lastIndexOf(s4,input2[i])==expVal[i]);
// LLA:             meth="lastIndexOf_004(const OUString,sal_Int32)";
// LLA:         }

        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                   "index",
                createName( pMeth,meth, i )
                );

        res &= lastRes;
    }

    return( res );
}
//------------------------------------------------------------------------
// testing the method indexOf(  )
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OUString_indexOf_001(
    hTestResult hRtlTestResult )
{
    sal_Bool bRes=sal_False;

    bRes=c_rtl_tres_state
        (
              hRtlTestResult,
              test_index<sal_Unicode>((const sal_Unicode*)input1Default,
                                    nDefaultCount,input2Default,
                                    expValDefault,0,hRtlTestResult),
              "index",
              "indexDefault(sal_Unicode ch, sal_Int32 fromIndex = 0)"
              );

    return ( bRes );
}
//------------------------------------------------------------------------
// testing the method indexOf(  )
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OUString_indexOf_002(
    hTestResult hRtlTestResult )
{
    sal_Bool bRes=sal_False;

    bRes=c_rtl_tres_state
        (
              hRtlTestResult,
              test_index<sal_Unicode>((const sal_Unicode*)input1Normal,
                                    nNormalCount,input2Normal,
                                    expValNormal,1,hRtlTestResult),
              "index",
              "indexNormal(sal_Unicode ch, sal_Int32 fromIndex)"
              );

    return ( bRes );
}
//------------------------------------------------------------------------
// testing the method indexOf(  OUString ch, sal_Int32 fromIndex = 0 )
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OUString_indexOf_003(
    hTestResult hRtlTestResult )
{
    sal_Bool bRes=sal_False;

    bRes=c_rtl_tres_state
        (
              hRtlTestResult,
              test_indexStr<sal_Unicode>((const sal_Unicode**)input1StrDefault,
                                       nStrDefaultCount,input2StrDefault,
                                       expValStrDefault,0,hRtlTestResult),
              "index",
              "indexDefault(OUString ch, sal_Int32 fromIndex = 0)"
              );

    return ( bRes );
}
//------------------------------------------------------------------------
// testing the method indexOf(  OUString ch, sal_Int32 fromIndex )
//------------------------------------------------------------------------
sal_Bool SAL_CALL test_rtl_OUString_indexOf_004(
    hTestResult hRtlTestResult )
{
    sal_Bool bRes=sal_False;

    bRes=c_rtl_tres_state
        (
              hRtlTestResult,
              test_indexStr<sal_Unicode>((const sal_Unicode**)input1StrNormal,
                                       nStrNormalCount,input2StrNormal,
                                       expValStrNormal,1,hRtlTestResult),
              "indexOf",
              "indexOf(OUString ch, sal_Int32 fromIndex)"
              );

    return ( bRes );
}
// LLA: //------------------------------------------------------------------------
// LLA: // testing the method lastIndexOf( sal_Unicode ch )
// LLA: //------------------------------------------------------------------------
// LLA: sal_Bool SAL_CALL test_rtl_OUString_lastIndexOf_001(
// LLA:     hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Bool bRes=sal_False;
// LLA:
// LLA:     bRes=c_rtl_tres_state
// LLA:         (
// LLA:             hRtlTestResult,
// LLA:             test_index<sal_Unicode>((const sal_Unicode*)input1lastDefault,
// LLA:                                     nlastDefaultCount,input2lastDefault,
// LLA:                                     expVallastDefault,2,hRtlTestResult),
// LLA:             "lastIndex",
// LLA:             "lastIndexDefault(sal_Unicode ch)"
// LLA:             );
// LLA:
// LLA:     return ( bRes );
// LLA: }
// LLA: //------------------------------------------------------------------------
// LLA: // testing the method lastIndexOf(  sal_Unicode ch, sal_Int32 fromIndex )
// LLA: //------------------------------------------------------------------------
// LLA: sal_Bool SAL_CALL test_rtl_OUString_lastIndexOf_002(
// LLA:     hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Bool bRes=sal_False;
// LLA:
// LLA:     bRes=c_rtl_tres_state
// LLA:         (
// LLA:             hRtlTestResult,
// LLA:             test_index<sal_Unicode>((const sal_Unicode*)input1lastNormal,
// LLA:                                     nlastNormalCount,input2lastNormal,
// LLA:                                     expVallastNormal,3,hRtlTestResult),
// LLA:             "lastIndex",
// LLA:             "lastIndexNormal(sal_Unicode ch, sal_Int32 fromIndex)"
// LLA:             );
// LLA:
// LLA:     return ( bRes );
// LLA: }
// LLA: //------------------------------------------------------------------------
// LLA: // testing the method lastIndexOf(  OUString ch )
// LLA: //------------------------------------------------------------------------
// LLA: sal_Bool SAL_CALL test_rtl_OUString_lastIndexOf_003(
// LLA:     hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Bool bRes=sal_False;
// LLA:
// LLA:     bRes=c_rtl_tres_state
// LLA:         (
// LLA:             hRtlTestResult,
// LLA:             test_indexStr<sal_Unicode>((const sal_Unicode**)input1StrLastDefault,
// LLA:                                        nStrLastDefaultCount,input2StrLastDefault,
// LLA:                                        expValStrLastDefault,2,hRtlTestResult),
// LLA:             "lastIndexOf",
// LLA:             "lastIndexOf(OUString ch)"
// LLA:             );
// LLA:
// LLA:     return ( bRes );
// LLA: }
// LLA: //------------------------------------------------------------------------
// LLA: // testing the method lastIndexOf(  OUString ch, sal_Int32 fromIndex )
// LLA: //------------------------------------------------------------------------
// LLA: sal_Bool SAL_CALL test_rtl_OUString_lastIndexOf_004(
// LLA:     hTestResult hRtlTestResult )
// LLA: {
// LLA:     sal_Bool bRes=sal_False;
// LLA:
// LLA:     for (int i=0;i<nStrLastNormalCount;i++)
// LLA:     {
// LLA:         rtl::OUString aStr = rtl::OUString(input1StrLastNormal[i]);
// LLA:         volatile int dummy = 0;
// LLA:     }
// LLA:
// LLA:     bRes=c_rtl_tres_state
// LLA:         (
// LLA:             hRtlTestResult,
// LLA:             test_indexStr<sal_Unicode>((const sal_Unicode**)input1StrLastNormal,
// LLA:                                        nStrLastNormalCount,input2StrLastNormal,
// LLA:                                        expValStrLastNormal,3,hRtlTestResult),
// LLA:             "lastIndexOf",
// LLA:             "lastIndexOf(OUString ch, sal_Int32 fromIndex)"
// LLA:             );
// LLA:
// LLA:     return ( bRes );
// LLA: }
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_indexOf(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "indexOf");
    sal_Bool res = test_rtl_OUString_indexOf_001(hRtlTestResult);
    res &= test_rtl_OUString_indexOf_002(hRtlTestResult);
    res &= test_rtl_OUString_indexOf_003(hRtlTestResult);
    res &= test_rtl_OUString_indexOf_004(hRtlTestResult);
    c_rtl_tres_state_end( hRtlTestResult, "indexOf");
//    return ( res );
}
//------------------------------------------------------------------------
// LLA: extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_lastIndexOf(
// LLA:     hTestResult hRtlTestResult )
// LLA: {
// LLA:     c_rtl_tres_state_start( hRtlTestResult, "lastIndexOf");
// LLA:     sal_Bool res = test_rtl_OUString_lastIndexOf_001(hRtlTestResult);
// LLA:     res &= test_rtl_OUString_lastIndexOf_002(hRtlTestResult);
// LLA:     res &= test_rtl_OUString_lastIndexOf_003(hRtlTestResult);
// LLA:     res &= test_rtl_OUString_lastIndexOf_004(hRtlTestResult);
// LLA:     c_rtl_tres_state_end( hRtlTestResult, "lastIndexOf");
// LLA: //    return ( res );
// LLA: }
//------------------------------------------------------------------------
// testing the method concat( const OString & aStr )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_concat(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "concat");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth =methName;

    typedef struct TestCase
    {
        sal_Char*                    comments;
        OUString*                     expVal;
        OUString*                     input1;
        OUString*                     input2;
        ~TestCase() { delete input1;delete input2; delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"concatenates two ustrings",new OUString(aUStr1),
         new OUString(aUStr7), new OUString(aUStr8)},
        {"concatenates empty ustring",new OUString(aUStr1),
         new OUString(aUStr1), new OUString("",0,
                                            kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString)},
        {"concatenates to empty ustring",new OUString(aUStr1),new OUString("",
                                                                           0,kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),
         new OUString(aUStr1)},
        {"concatenates two empty ustrings",new OUString("",0,
                                                        kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),
         new OUString("",0,
                      kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),
         new OUString("",0,
                      kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString)},
        {"concatenates ustring constructed by default constructor",
         new OUString(aUStr1),new OUString(aUStr1), new OUString()},
        {"concatenates to ustring constructed by default constructor",
         new OUString(aUStr1),new OUString(), new OUString(aUStr1)},
        {"concatenates two ustrings constructed by default constructor",
         new OUString(),new OUString(), new OUString()}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;
    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        OUString str = arrTestCase[i].input1->concat(*arrTestCase[i].input2);
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
    c_rtl_tres_state_end( hRtlTestResult, "concat");
//    return ( res );
}
//------------------------------------------------------------------------
// testing the method replaceAt( sal_Int32 index, sal_Int32 count,
// const OUString& newStr )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_replaceAt(
    rtlTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "replaceAt");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*                    comments;
        OUString*                    expVal;
        OUString*                    input;
        OUString*                    newStr;
        sal_Int32                    index;
        sal_Int32                    count;

        ~TestCase() { delete input; delete expVal; delete newStr;}
    } TestCase;

    TestCase arrTestCase[]=
    {

        { "string differs", new OUString(aUStr2), new OUString(aUStr22),
          new OUString(aUStr2), 0, kTestStr22Len },

        { "larger index", new OUString(aUStr1), new OUString(aUStr7),
          new OUString(aUStr8), 64, kTestStr8Len },

        { "larger count", new OUString(aUStr2), new OUString(aUStr22),
          new OUString(aUStr2),0, 64 },

        { "navigate index", new OUString(aUStr2), new OUString(aUStr22),
          new OUString(aUStr2), -64, 64 },

        { "null ustring",
          new OUString("",0,
                       kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),
          new OUString(aUStr14),
          new OUString("",0,
                       kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),
          0, kTestStr14Len }
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        ::rtl::OUString aStr1;
        aStr1 = arrTestCase[i].input->replaceAt( arrTestCase[i].index,
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

    c_rtl_tres_state_end( hRtlTestResult, "replaceAt");
//     return ( res );
}
//------------------------------------------------------------------------
// this is my testing code
// testing the method replace( sal_Unicode oldChar, sal_Unicode newChar )
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_replace(
    hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "replace");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*                    comments;
        OUString*                    expVal;
        OUString*                    input;
        sal_Unicode                  oldChar;
        sal_Unicode                  newChar;

        ~TestCase() { delete input; delete expVal;}
    } TestCase;

    TestCase arrTestCase[]=
    {
        {"ustring differs", new OUString(aUStr18), new OUString(aUStr4),83,115},
        {"ustring differs", new OUString(aUStr19), new OUString(aUStr17),32,45},
        {"ustring must be empty", new OUString("",0,
                                               kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),
         new OUString("",0,
                      kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),83,23},
        {"ustring must be empty", new OUString(),
         new OUString("",0,
                      kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),83,23},
        {"same ustring, no replace ", new OUString(aUStr22),
         new OUString(aUStr22),42,56}
    };


    sal_Bool res = sal_True;
    sal_Int32 i;

    for (i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        ::rtl::OUString aStr1;
        aStr1= arrTestCase[i].input->replace(arrTestCase[i].oldChar,arrTestCase[i].newChar);
        res &= c_rtl_tres_state
            (
                hRtlTestResult,
                (arrTestCase[i].expVal->compareTo(aStr1) == 0),
                arrTestCase[i].comments,
                createName( pMeth, "replace", i )

                );
    }
    c_rtl_tres_state_end( hRtlTestResult, "replace");
//     return ( res );
}
//------------------------------------------------------------------------
// testing the method toAsciiLowerCase()
//-----------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_toAsciiLowerCase(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "toAsciiLowerCase");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth =methName;

    typedef struct TestCase
    {
        sal_Char*                    comments;
        OUString*                     expVal;
        OUString*                     input1;
        ~TestCase() { delete input1; delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {

        {"only uppercase",new OUString(aUStr5),new OUString(aUStr4)},
        {"different cases",new OUString(aUStr5),new OUString(aUStr1)},
        {"different cases",new OUString(aUStr5),new OUString(aUStr3)},
        {"only lowercase",new OUString(aUStr5),new OUString(aUStr5)},
        {"empty ustring",new OUString("",0,
                                      kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),
         new OUString("",0,
                      kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString)},
        {"ustring constructed by default constructor",new OUString(),
         new OUString()},
        {"have special Unicode",new OUString("\23\12\34sun\13\45",6,
                                             kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),
         new OUString("\23\12\34sun\13\45",6,
                      kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString)}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;
    sal_Bool lastRes=sal_False;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        OUString str = arrTestCase[i].input1->toAsciiLowerCase();
        if(i<=5)
        {
            lastRes = (str ==* arrTestCase[i].expVal);

            c_rtl_tres_state
                (
                    hRtlTestResult,
                    lastRes,
                    arrTestCase[i].comments,
                    createName( pMeth, "toAsciiLowerCase", i)
                    );
        }
        else
        {
            c_rtl_tres_state
                (
                    hRtlTestResult,
                    sal_True,
                    arrTestCase[i].comments,
                    createName( pMeth, "toAsciiLowerCase", i)
                    );
        }
        res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "toAsciiLowerCase");
//    return ( res );
}
//------------------------------------------------------------------------
// testing the method toAsciiUpperCase()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_toAsciiUpperCase(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "toAsciiUpperCase");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth =methName;

    typedef struct TestCase
    {
        sal_Char*                    comments;
        OUString*                    expVal;
        OUString*                    input1;
        ~TestCase() { delete input1; delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"only lowercase",new OUString(aUStr4),new OUString(aUStr5)},
        {"mixed cases",new OUString(aUStr4),new OUString(aUStr3)},
        {"mixed cases",new OUString(aUStr4),new OUString(aUStr1)},
        {"only uppercase",new OUString(aUStr4),new OUString(aUStr4)},
        {"empty ustring",new OUString("",0,
                                      kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),
         new OUString("",0,
                      kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString)},
        {"ustring constructed by default constructor",new OUString(),
         new OUString()},
        {"have special Unicode",new OUString("\23\12\34SUN\13\45",6,
                                             kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString),
         new OUString("\23\12\34sun\13\45",6,
                      kEncodingRTLTextUSASCII,kConvertFlagsOStringToOUString)}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;
    sal_Bool lastRes=sal_False;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        OUString str = arrTestCase[i].input1->toAsciiUpperCase();
        if(i<=5)
        {
            lastRes = (str == *arrTestCase[i].expVal);

            c_rtl_tres_state
                (
                    hRtlTestResult,
                    lastRes,
                    arrTestCase[i].comments,
                    createName( pMeth, "toAsciiUpperCase", i)
                    );
        }
        else
        {
            c_rtl_tres_state
                (
                    hRtlTestResult,
                    sal_True,
                    arrTestCase[i].comments,
                    createName( pMeth, "toAsciiUpperCase", i)
                    );
        }

        res &= lastRes;
    }
    c_rtl_tres_state_end( hRtlTestResult, "toAsciiUpperCase");
//    return ( res );
}

//------------------------------------------------------------------------
// testing the method trim()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_trim(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "trim");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth =methName;

    typedef struct TestCase
    {
        sal_Char*                    comments;
        OUString*                    expVal;
        OUString*                    input1;
        ~TestCase() { delete input1; delete expVal;}
    } TestCase;

    TestCase arrTestCase[] =
    {
        {"removes space from the front",new OUString(aUStr1),
         new OUString(aUStr10)},
        {"removes space from the end",new OUString(aUStr1),
         new OUString(aUStr11)},
        {"removes space from the front and end",new OUString(aUStr1),
         new OUString(aUStr12)},
        {"removes several spaces from the end",new OUString(aUStr1),
         new OUString(aUStr13)},
        {"removes several spaces from the front",new OUString(aUStr1),
         new OUString(aUStr14)},
        {"removes several spaces from the front and one from the end",
         new OUString(aUStr1),
         new OUString(aUStr15)},
        {"removes one space from the front and several from the end",
         new OUString(aUStr1),
         new OUString(aUStr16)},
        {"removes several spaces from the front and end",
         new OUString(aUStr1),
         new OUString(aUStr17)},
        {"removes characters that have codes <= 32",new OUString(aUStr30),
         new OUString("\1\3\5\7\11\13\15\17sun\21\23\25\27\31\33\50",
                      18,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)},
        {"removes characters that have codes <= 32",new OUString(aUStr28),
         new OUString("\50\3\5\7\11\13\15\17sun\21\23\25\27\31\33\1",
                      18,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)},
        {"removes characters that have codes <= 32",new OUString(aUStr29),
         new OUString("\50\3\5\7\11\13\15\17sun\21\23\25\27\31\33\50",
                      18,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)},
        {"removes characters that have codes <= 32",new OUString(aUStr20),
         new OUString("\1\3\5\7\11\13\15\17sun\21\23\25\27\31\23\20",
                      18,kEncodingRTLTextUSASCII,
                      kConvertFlagsOStringToOUString)},
        {"no spaces",new OUString(aUStr8),
         new OUString(aUStr8)}
    };

    sal_Bool res = sal_True;
    sal_Int32 i;

    for(i = 0; i < (sizeof (arrTestCase))/(sizeof (TestCase)); i++)
    {
        OUString strRes = arrTestCase[i].input1->trim();
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
    c_rtl_tres_state_end( hRtlTestResult, "trim");
//    return ( res );
}
//------------------------------------------------------------------------
//    testing the method toData()
//------------------------------------------------------------------------

template <class T>
sal_Bool test_toData( const char** input, int num, sal_Int16 radix,
                      const T* expVal,int base,rtlTestResult hRtlTestResult)
{
    sal_Bool    res=sal_True;
    sal_Char    methName[MAXBUFLENGTH];
    sal_Char    *meth;
    sal_Char*   pMeth=methName;
    sal_Int32   i;
    static      sal_Unicode aUchar[60]={0x00};
    T           intRes;
    sal_Bool    lastRes=sal_False;

    for(i=0;i<num;i++)
    {
        OSL_ENSURE( i < 60, "ERROR: leave aUchar bound");

        AStringToUStringCopy(aUchar,input[i]);

        OUString str(aUchar);

        if(base==0)
        {
            intRes=str.toInt32();
            lastRes=(intRes==expVal[i]);
            meth="toInt32default";
        }
        if(base==1)
        {
            intRes=str.toInt32(radix);
            lastRes=(intRes==expVal[i]);
            meth="toInt32normal";
        }
        if(base==2)
        {
            intRes=str.toInt64();
            lastRes=(intRes==expVal[i]);
            meth="toInt64default";
        }
        if(base==3)
        {
            intRes=str.toInt64(radix);
            lastRes=(intRes==expVal[i]);
            meth="toInt64normal";
        }
        if(base==4)
        {
            intRes=str.toDouble();
            lastRes=(fabs(intRes-expVal[i])<=1e-35);
            meth="toDouble";
        }
        if(base==5)
        {
            intRes=str.toFloat();
            lastRes=(fabs(intRes-expVal[i])<=1e-35);
            meth="toFloat";
        }
        if(base==6)
        {
            intRes=str.toChar();
            lastRes=(intRes==expVal[i]);
            meth="toChar";
        }

        char buf[MAXBUFLENGTH];
        buf[0] = '\'';
        cpynstr( buf + 1, input[i], MAXBUFLENGTH );
        int length = AStringLen( input[i] );
        buf[length + 1] = '\'';
        buf[length + 2] = 0;

        c_rtl_tres_state
            (
                hRtlTestResult,
                lastRes,
                buf,
                createName( pMeth,meth, i )
                );

        res &= lastRes;
    }

    return( res );
}
//------------------------------------------------------------------------
//    testing the method toDouble()
//------------------------------------------------------------------------

// LLA: extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_toDouble(
// LLA:     hTestResult hRtlTestResult )
// LLA: {
// LLA:     c_rtl_tres_state_start( hRtlTestResult, "toDouble");
// LLA:     sal_Bool bRes=sal_False;
// LLA:
// LLA:     bRes=c_rtl_tres_state
// LLA:         (
// LLA:             hRtlTestResult,
// LLA:             test_toData<double>((const char**)inputDouble,nDoubleCount,10,
// LLA:                                 expValDouble,4,hRtlTestResult),
// LLA:             "toDouble",
// LLA:             "toDouble()"
// LLA:             );
// LLA:     c_rtl_tres_state_end( hRtlTestResult, "toDouble");
// LLA: //   return ( bRes );
// LLA: }

//------------------------------------------------------------------------
//    testing the method toFloat()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_toFloat(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "toFloat");
    sal_Bool bRes=sal_False;

    bRes=c_rtl_tres_state
        (
              hRtlTestResult,
              test_toData<float>((const char**)inputFloat,nFloatCount,10,
                               expValFloat,5,hRtlTestResult),
              "toFloat",
              "toFloat()"
              );

    c_rtl_tres_state_end( hRtlTestResult, "toFloat");
//   return ( bRes );

}
//------------------------------------------------------------------------
//    testing the method toChar()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_toChar(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "toChar");
    sal_Bool bRes=sal_False;

    bRes=c_rtl_tres_state
        (
              hRtlTestResult,
              test_toData<sal_Unicode>((const char**)inputChar,nCharCount,
                                     10,expValChar,6,hRtlTestResult),
              "toChar",
              "toChar()"
              );

    c_rtl_tres_state_end( hRtlTestResult, "toChar");
//   return ( bRes );

}
//------------------------------------------------------------------------
//    testing the method toBoolean()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_toBoolean(
    hTestResult hRtlTestResult)
{
    c_rtl_tres_state_start( hRtlTestResult, "toBoolean");
    sal_Char methName[MAXBUFLENGTH];
    sal_Char* pMeth = methName;

    typedef struct TestCase
    {
        sal_Char*          comments;
        sal_Bool           expVal;
        OUString*          input;

        ~TestCase()      {delete input;}
    }TestCase;

    TestCase arrTestCase[]={

        {"expected true", sal_True, new OUString("True",4,kEncodingRTLTextUSASCII,
                                                 kConvertFlagsOStringToOUString)},
        {"expected false", sal_False, new OUString("False",5,
                                                   kEncodingRTLTextUSASCII,
                                                   kConvertFlagsOStringToOUString)},
        {"expected true", sal_True, new OUString("1",1,kEncodingRTLTextUSASCII,
                                                 kConvertFlagsOStringToOUString)}
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
    c_rtl_tres_state_end( hRtlTestResult, "toBoolean");
//     return ( res );
}
//------------------------------------------------------------------------
//    testing the method toInt32()
//------------------------------------------------------------------------

sal_Bool SAL_CALL test_rtl_OUString_toInt32_normal(
    hTestResult hRtlTestResult )
{
    sal_Int32 expValues[kBase36NumsCount];
    sal_Int32 i;

    for ( i = 0; i < kBase36NumsCount; i++ )
        expValues[i] = i;

    sal_Bool res = c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>( kBinaryNumsStr,kBinaryNumsCount,
                                    kRadixBinary,expValues,1,hRtlTestResult ),
            "kBinaryNumsStr",
            "toInt32( radix 2 )"
            );
    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>( kBinaryMaxNumsStr,kInt32MaxNumsCount,
                                    kRadixBinary,kInt32MaxNums,1,hRtlTestResult ),
            "kBinaryMaxNumsStr",
            "toInt32_Boundaries( radix 2 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>( kOctolNumsStr,kOctolNumsCount,
                                    kRadixOctol,expValues,1,hRtlTestResult ),
            "kOctolNumsStr",
            "toInt32( radix 8 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>( kOctolMaxNumsStr,kInt32MaxNumsCount,
                                    kRadixOctol,(sal_Int32*)kInt32MaxNums,1,hRtlTestResult ),
            "kOctolMaxNumsStr",
            "toInt32_Boundaries( radix 8 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>( kDecimalNumsStr,kDecimalNumsCount,
                                    kRadixDecimal,expValues,1,hRtlTestResult ),
            "kDecimalNumsStr",
            "toInt32( radix 10 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>( kDecimalMaxNumsStr,kInt32MaxNumsCount,
                                    kRadixDecimal,(sal_Int32*)kInt32MaxNums,1,hRtlTestResult ),
            "kDecimalMaxNumsStr",
            "toInt32_Boundaries( radix 10 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>( kHexDecimalNumsStr,kHexDecimalNumsCount,
                                    kRadixHexdecimal,expValues,1,hRtlTestResult ),
            "kHexDecimalNumsStr",
            "toInt32( radix 16 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>(  kHexDecimalMaxNumsStr,kInt32MaxNumsCount,
                                     kRadixHexdecimal,(sal_Int32*)kInt32MaxNums,1,hRtlTestResult ),
            "kHexDecimalMaxNumsStr",
            "toInt32_Boundaries( radix 16 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>(  kBase36NumsStr,kBase36NumsCount,
                                     kRadixBase36, expValues,1,hRtlTestResult ),
            "kBase36NumsStr",
            "toInt32( radix 36 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>( kBase36MaxNumsStr,kInt32MaxNumsCount,
                                    kRadixBase36,(sal_Int32*)kInt32MaxNums,1,hRtlTestResult ),
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
            test_toData<sal_Int32>( spString,nSpecCases,
                                    kRadixDecimal,expSpecVal,1,hRtlTestResult ),
            "special cases",
            "toInt32( specialcases )"
            );

    return ( res );
}
sal_Bool SAL_CALL test_rtl_OUString_toInt32_wrongRadix(
    hTestResult hRtlTestResult )
{
    ::rtl::OUString str("0",1,kEncodingRTLTextUSASCII,
                        kConvertFlagsOStringToOUString);

    sal_Int32 iRes =str.toInt32(-1);

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
sal_Bool SAL_CALL test_rtl_OUString_toInt32_defaultParam(
    hTestResult hRtlTestResult )
{
    sal_Int32 expValues[kBase36NumsCount];
    sal_Int32 i;

    for ( i = 0; i < kBase36NumsCount; i++ )
        expValues[i] = i;

    sal_Bool res = c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>( kDecimalNumsStr,kDecimalNumsCount,
                                    kRadixDecimal,expValues,0,hRtlTestResult ),
            "kBinaryNumsStr",
            "toInt32( radix 2 )"
            );
    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int32>( kDecimalMaxNumsStr,kInt32MaxNumsCount,
                                    kRadixDecimal,(sal_Int32*)kInt32MaxNums,0,hRtlTestResult ),
            "kDecimalMaxNumsStr",
            "toInt32_Boundaries( radix 10 )"
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
            test_toData<sal_Int32>( spString,nSpecCases,
                                    kRadixDecimal,expSpecVal,0,hRtlTestResult ),
            "special cases",
            "toInt32( specialcases )"
            );

    return ( res );
}

//------------------------------------------------------------------------
// testing the method toInt32()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_toInt32(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "toInt32");
    sal_Bool bTState = test_rtl_OUString_toInt32_normal( hRtlTestResult );
    bTState &= test_rtl_OUString_toInt32_defaultParam( hRtlTestResult );
    bTState &= test_rtl_OUString_toInt32_wrongRadix( hRtlTestResult );
    c_rtl_tres_state_end( hRtlTestResult, "toInt32");
//    return ( bTState );
}
//------------------------------------------------------------------------
// testing the method toInt64( sal_Int16 radix = 2,8,10,16,36 )
//------------------------------------------------------------------------

sal_Bool SAL_CALL test_rtl_OUString_toInt64_normal(
    hTestResult hRtlTestResult )
{
    sal_Int64 expValues[kBase36NumsCount];
    sal_Int32 i;

    for (i = 0; i < kBase36NumsCount; expValues[i] = i, i++);

    sal_Bool res = c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int64>( kBinaryNumsStr,kBinaryNumsCount,
                                    kRadixBinary,expValues,3,hRtlTestResult ),
            "kBinaryNumsStr",
            "toInt64( radix 2 )"
            );

/* LLA: does not work within wntmsci8.pro
   res &= c_rtl_tres_state
   (
   hRtlTestResult,
   test_toData<sal_Int64>( kBinaryMaxNumsStr,kInt64MaxNumsCount,
   kRadixBinary,kInt64MaxNums,3,hRtlTestResult ),
   "kBinaryMaxNumsStr",
   "toInt64_Boundaries( radix 2 )"
   );
*/

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int64>( kOctolNumsStr,kOctolNumsCount,
                                    kRadixOctol,expValues,3,hRtlTestResult ),
            "kOctolNumsStr",
            "toInt64( radix 8 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int64>( kOctolMaxNumsStr,kInt64MaxNumsCount,
                                    kRadixOctol,(sal_Int64*)kInt64MaxNums,3,hRtlTestResult ),
            "kOctolMaxNumsStr",
            "toInt64_Boundaries( radix 8 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int64>( kDecimalNumsStr,kDecimalNumsCount,
                                    kRadixDecimal,expValues,3,hRtlTestResult ),
            "kDecimalNumsStr",
            "toInt64( radix 10 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int64>( kDecimalMaxNumsStr,kInt64MaxNumsCount,
                                    kRadixDecimal,(sal_Int64*)kInt64MaxNums,3,hRtlTestResult ),
            "kDecimalMaxNumsStr",
            "toInt64_Boundaries( radix 10 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int64>( kHexDecimalNumsStr,kHexDecimalNumsCount,
                                    kRadixHexdecimal,expValues,3,hRtlTestResult ),
            "kHexDecimalNumsStr",
            "toInt64( radix 16 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int64>(  kHexDecimalMaxNumsStr,kInt64MaxNumsCount,
                                     kRadixHexdecimal,(sal_Int64*)kInt64MaxNums,3,hRtlTestResult ),
            "kHexDecimalMaxNumsStr",
            "toInt64_Boundaries( radix 16 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int64>(  kBase36NumsStr,kBase36NumsCount,
                                     kRadixBase36, expValues,3,hRtlTestResult ),
            "kBase36NumsStr",
            "toInt64( radix 36 )"
            );

    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int64>( kBase36MaxNumsStr,kInt64MaxNumsCount,
                                    kRadixBase36,(sal_Int64*)kInt64MaxNums,3,hRtlTestResult ),
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
            test_toData<sal_Int64>( spString,nSpecCases,
                                    kRadixDecimal,expSpecVal,3,hRtlTestResult ),
            "special cases",
            "toInt64( specialcases )"
            );

    return (res);
}

sal_Bool SAL_CALL test_rtl_OUString_toInt64_wrongRadix(
    hTestResult hRtlTestResult )
{
    ::rtl::OUString str("0",1,kEncodingRTLTextUSASCII,
                        kConvertFlagsOStringToOUString);

    sal_Int64 iRes = str.toInt64(-1);

    return (
        c_rtl_tres_state
        (
            hRtlTestResult,
            iRes == 0,
            "wrong radix -1",
            "toInt64( wrong radix -1)"
            )
        );
}
sal_Bool SAL_CALL test_rtl_OUString_toInt64_defaultParam(
    hTestResult hRtlTestResult )
{
    sal_Int64 expValues[kBase36NumsCount];
    sal_Int32 i;

    for ( i = 0; i < kBase36NumsCount; i++ )
        expValues[i] = i;

    sal_Bool res = c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int64>( kDecimalNumsStr,kDecimalNumsCount,
                                    kRadixDecimal,expValues,2,hRtlTestResult ),
            "kBinaryNumsStr",
            "toInt64( radix 10 )"
            );
    res &= c_rtl_tres_state
        (
            hRtlTestResult,
            test_toData<sal_Int64>( kDecimalMaxNumsStr,kInt64MaxNumsCount,
                                    kRadixDecimal,(sal_Int64*)kInt64MaxNums,2,hRtlTestResult ),
            "kDecimalMaxNumsStr",
            "toInt64_Boundaries( radix 10 )"
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
            test_toData<sal_Int64>( spString,nSpecCases,
                                    kRadixDecimal,expSpecVal,2,hRtlTestResult ),
            "special cases",
            "toInt64( specialcases )"
            );

    return ( res );
}

//------------------------------------------------------------------------
// testing the method toInt64()
//------------------------------------------------------------------------
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString_toInt64(
    hTestResult hRtlTestResult )
{
    c_rtl_tres_state_start( hRtlTestResult, "toInt64");
    sal_Bool bTState = test_rtl_OUString_toInt64_normal( hRtlTestResult );
    bTState &= test_rtl_OUString_toInt64_defaultParam (hRtlTestResult );
    bTState &= test_rtl_OUString_toInt64_wrongRadix( hRtlTestResult );
    c_rtl_tres_state_end( hRtlTestResult, "toInt64");
//    return ( bTState );
}
extern "C" void /* sal_Bool */ SAL_CALL test_rtl_OUString( hTestResult hRtlTestResult )
{

    c_rtl_tres_state_start(hRtlTestResult, "rtl_OUString" );

    test_rtl_OUString_ctors( hRtlTestResult );
    test_rtl_OUString_getLength( hRtlTestResult );
    test_rtl_OUString_equals( hRtlTestResult );
    test_rtl_OUString_equalsIgnoreAsciiCase( hRtlTestResult );
    test_rtl_OUString_compareTo( hRtlTestResult );
    test_rtl_OUString_match( hRtlTestResult );
    test_rtl_OUString_op_eq( hRtlTestResult );
    test_rtl_OUString_op_peq( hRtlTestResult );
    test_rtl_OUString_csuc( hRtlTestResult );
    test_rtl_OUString_getStr( hRtlTestResult );
    test_rtl_OUString_reverseCompareTo( hRtlTestResult );
    test_rtl_OUString_equalsAscii( hRtlTestResult );
    test_rtl_OUString_equalsAsciiL( hRtlTestResult );
    test_rtl_OUString_compareToAscii( hRtlTestResult );
    test_rtl_OUString_valueOf_sal_Bool( hRtlTestResult );
    test_rtl_OUString_valueOf_sal_Unicode( hRtlTestResult );
    test_rtl_OUString_valueOf( hRtlTestResult );
    test_rtl_OUString_createFromAscii( hRtlTestResult );
    test_rtl_OUString_indexOf( hRtlTestResult );
// LLA: removed, it is in a new test in rtl/oustring. test_rtl_OUString_lastIndexOf( hRtlTestResult );
    test_rtl_OUString_concat( hRtlTestResult );
    test_rtl_OUString_replaceAt( hRtlTestResult );
    test_rtl_OUString_replace( hRtlTestResult );
    test_rtl_OUString_toAsciiLowerCase( hRtlTestResult );
    test_rtl_OUString_toAsciiUpperCase( hRtlTestResult );
    test_rtl_OUString_trim( hRtlTestResult );
// LLA: removed, it is in a new test in rtl/oustring. test_rtl_OUString_toDouble( hRtlTestResult );
    test_rtl_OUString_toFloat( hRtlTestResult );
    test_rtl_OUString_toChar( hRtlTestResult );
    test_rtl_OUString_toBoolean( hRtlTestResult );
    test_rtl_OUString_toInt32( hRtlTestResult );
    test_rtl_OUString_toInt64( hRtlTestResult );

    c_rtl_tres_state_end(hRtlTestResult, "rtl_OUString");
}
// -----------------------------------------------------------------------------
void RegisterAdditionalFunctions(FktRegFuncPtr _pFunc)
{
    if (_pFunc)
    {
        (_pFunc)(&test_rtl_OUString, "");
    }
}
