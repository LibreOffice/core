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
#include "precompiled_extensions.hxx"

#include <atlbase.h>

CComModule _Module;
#include<atlcom.h>
#include <comdef.h>

#include "axhost.hxx"

#include <stdio.h>
#include "typelib/typedescription.hxx"
#include <com/sun/star/bridge/oleautomation/Date.hpp>
#include <com/sun/star/bridge/oleautomation/Currency.hpp>
#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#include <com/sun/star/bridge/oleautomation/SCode.hpp>
#include <com/sun/star/bridge/oleautomation/NamedArgument.hpp>
#include <com/sun/star/bridge/oleautomation/PropertyPutArgument.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/servicefactory.hxx>
#include <rtl/string.h>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::script;
using namespace com::sun::star::bridge::oleautomation;
using namespace cppu;

using ::rtl::OUString;

Reference<XInvocation> convertComObject( IUnknown* pUnk);
Reference<XInvocation> getComObject( OUString progId);
bool checkOutArgs(const Sequence<Any> & outArgs,
                  const Sequence<sal_Int16> & indices, const Sequence<Any> & values);

bool doSimpleTest(const Reference<XInvocation> & inv);
bool doSimpleSequenceTest(const Reference<XInvocation> & inv);
bool doParameterTest(const Reference<XInvocation> & inv);
bool doPropertyWithArgumentTest(const Reference<XInvocation> & inv);
bool equalSequences(const Any& orig, const Any& returned);
HRESULT doTest();
HRESULT doTest2( Reference<XInvocation> &);
Reference<XInvocation> getComObject(OUString& );

HRESULT InitializeParameter();
void printResultVariantArray( VARIANT & var);
void printVariant( VARIANT & var);
void printSequence( Sequence<Any>& val);

extern "C" int __cdecl main( int , char **)
{
    HRESULT hr;
    if( FAILED( hr=CoInitialize(NULL)))
    {
        _tprintf(_T("CoInitialize failed \n"));
        return -1;
    }


    _Module.Init( ObjectMap, GetModuleHandle( NULL));

    if( FAILED(hr=doTest()))
    {
        _com_error err( hr);
        const TCHAR * errMsg= err.ErrorMessage();
        MessageBox( NULL, errMsg, "Test failed", MB_ICONERROR);
    }
    else
    {
        MessageBox( NULL,NULL , "Test succeeded", MB_ICONINFORMATION);
    }


    _Module.Term();
    CoUninitialize();
    return 0;
}
//Also supports named args

bool doParameterTest(const Reference<XInvocation> & inv)
{
    Sequence< sal_Int16> seqIndices;
    Sequence<Any> seqOut;

    Any arArgs[2];
    Any arValue[2];
    Any arArgs1[4];

    NamedArgument arg1(OUString(L"val1"), makeAny((sal_Int32) 123));
    NamedArgument arg2(OUString(L"val2"), makeAny((sal_Int32) 456));
    NamedArgument arg3(OUString(L"val3"), makeAny((sal_Int32) 0xff));
    NamedArgument arg4(OUString(L"val4"), makeAny((sal_Int32) 0xffff));

    NamedArgument argOut1(OUString(L"val1"), Any());
    NamedArgument argOut2(OUString(L"val2"), Any());
    Sequence<Any> seqNoArgs;
    arArgs[0] <<= (sal_Int32) 0;
    arArgs[1] <<= (sal_Int32) 0;
    Sequence<Any> seqPositional0(arArgs, 2);


    arArgs[0] <<= arg1;
    arArgs[1] <<= arg2;
    Sequence<Any> seqNamed(arArgs, 2);
    arArgs[0] <<= arg2;
    arArgs[1] <<= arg1;
    Sequence<Any> seqNamed2(arArgs, 2);
    arArgs[0] <<= argOut1;
    arArgs[1] <<= argOut2;
    Sequence<Any> seqNamed3(arArgs, 2);
    arArgs[0] <<= argOut2;
    arArgs[1] <<= argOut1;
    Sequence<Any> seqNamed4(arArgs, 2);

    arArgs[0] <<= arg1;
    Sequence<Any> seqNamed5(arArgs, 1);
    arArgs[0] <<= arg2;
    Sequence<Any> seqNamed6(arArgs, 1);

    arArgs[0] <<= (sal_Int32) 123;
    arArgs[1] <<= (sal_Int32) 456;
    Sequence<Any> seqPositional(arArgs, 2);
    arArgs[0] <<= (sal_Int32) 123;
    Sequence<Any> seqPositional2(arArgs, 1);

    arArgs[0] <<= Any();
    arArgs[1] <<= Any();
    Sequence<Any> seqPositional3(arArgs, 2);

    arArgs[0] <<= (sal_Int32) 123;
    arArgs[1] <<= SCode(DISP_E_PARAMNOTFOUND);
    Sequence<Any> seqOutOpt1(arArgs, 2);

    arArgs[0] <<= SCode(DISP_E_PARAMNOTFOUND);
    arArgs[1] <<= SCode(DISP_E_PARAMNOTFOUND);
    Sequence<Any> seqOutOpt2(arArgs, 2);

    arArgs[0] <<= SCode(DISP_E_PARAMNOTFOUND);
    arArgs[1] <<= (sal_Int32) 456;
    Sequence<Any> seqOutOpt3(arArgs, 2);

    arArgs1[0] <<= (sal_Int32) 0;
    arArgs1[1] <<= (sal_Int32) 0;
    arArgs1[2] <<= (sal_Int32) 0;
    arArgs1[3] <<= (sal_Int32) 0;
    Sequence<Any> seqMix0(arArgs1, 4);

    arArgs1[0] <<= (sal_Int32) 123;
    arArgs1[1] <<= (sal_Int32) 456;
    arArgs1[2] <<= arg3;
    arArgs1[3] <<= arg4;
    Sequence<Any> seqMix(arArgs1, 4);

    arArgs1[0] <<= Any();
    arArgs1[1] <<= (sal_Int32) 456;
    arArgs1[2] <<= arg4;
    Sequence<Any> seqMix2(arArgs1, 3);

    arArgs1[0] <<= SCode(DISP_E_PARAMNOTFOUND);
    arArgs1[1] <<= (sal_Int32) 456;
    arArgs1[2] <<= SCode(DISP_E_PARAMNOTFOUND);
    arArgs1[3] <<= arg4.Value;
    Sequence<Any> seqMixOut(arArgs1, 4);

    arArgs1[0] <<= SCode(DISP_E_PARAMNOTFOUND);
    arArgs1[1] <<= Any();
    arArgs1[2] <<= arg4;
    Sequence<Any> seqMix2Out(arArgs1, 3);



    //in args + out, optional, positional-----------------------------------------
    //first general test
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqPositional, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional5"), seqPositional, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqPositional))
        return false;

    //2 optional args, 1 provided
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqPositional0, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqPositional2, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional5"), seqPositional, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqOutOpt1))
        return false;

    //2 optional args, 0 provided
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqPositional0, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqNoArgs, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional5"), seqPositional3, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqOutOpt2))
        return false;

    //named args --------------------------------------------

    // 2 named args, correct position
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqPositional0, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqNamed, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional5"), seqPositional0, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqPositional))
        return false;

    // 2named args, position differs
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqPositional0, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqNamed2, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional5"), seqPositional, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqPositional))
        return false;

    //named out args, 2 named args with correct position
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqNamed, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqIndices.realloc(0);
    inv->invoke(OUString(L"optional5"), seqNamed3, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqNamed ))
        return false;

    //named out args, 2 named args with different position
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqNamed, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqIndices.realloc(0);
    inv->invoke(OUString(L"optional5"), seqNamed4, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqNamed2 ))
        return false;


    //2 args, 1 provided (correct order)
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqPositional0, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqNamed5, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional5"), seqPositional, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqOutOpt1))
        return false;
    //2 args, 1 provided (incorrect order)
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqPositional0, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional3"), seqNamed6, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional5"), seqPositional, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqOutOpt3))
        return false;

    //2position + 2 2named args, correct order
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional6"), seqMix0, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional6"), seqMix, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional7"), seqMix, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqMix))
         return false;

    // 4 in args, 1 positional, 1 named, 1 positional omitted
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional6"), seqMix0, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional6"), seqMix2, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional7"), seqMix0, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqMixOut))
         return false;

    // 4 out args, 1 positional, 1 named, 1 positional omitted
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional6"), seqMix2, seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"optional7"), seqMix2Out, seqIndices, seqOut);
    if ( ! checkOutArgs(seqOut, seqIndices, seqMix2Out))
         return false;


    return true;
}
bool doPropertyTest(const Reference<XInvocation> & inv)
{
    Sequence< sal_Int16> seqIndices;
    Sequence<Any> seqOut;

    Any inBool, outBool;
    Any inByte, outByte;
    Any inShort, outShort;
    Any inLong,  outLong;
    Any inString,  outString;
    Any inFloat, outFloat;
    Any inDouble, outDouble;
    Any inVariant, outVariant;
    Any inObject, outObject;
    Any inUnknown, outUnknown;
    Any inCY, outCY;
    Any inDate, outDate;
    Any inDecimal, outDecimal;
    Any inSCode, outSCode;
    Any inrefLong, outrefLong;
    Any inrefVariant, outrefVariant;
    Any inrefDecimal, outrefDecimal;
    Any inParamsLong, outParamsLong;
    Reference<XInterface> xintUnknown(getComObject(L"AxTestComponents.Foo"));

    inBool <<= (sal_Bool) sal_True;
    inv->setValue(OUString(L"prpBool"), inBool);
    outBool = inv->getValue(OUString(L"prpBool"));

    inByte <<= (sal_Int8) 100;
    inv->setValue(OUString(L"prpByte"), inByte);
    outByte = inv->getValue(OUString(L"prpByte"));

    inShort <<= static_cast<sal_Int16>( -1);
    inv->setValue(OUString(L"prpShort"), inShort);
    outShort = inv->getValue(OUString(L"prpShort"));

    inLong <<= ( sal_Int32) 1234567;
    inv->setValue(OUString(L"prpLong"), inLong  // TODO: Add your implementation code here
);
    outLong = inv->getValue(OUString(L"prpLong"));

    inString <<= OUString(L" this is clientTest.exe");
    inv->setValue(OUString(L"prpString"), inString);
    outString = inv->getValue(OUString(L"prpString"));

    inFloat <<=  3.14f;
    inv->setValue(OUString(L"prpFloat"), inFloat);
    outFloat = inv->getValue(OUString(L"prpFloat"));

    inDouble <<= ( double) 3.145;
    inv->setValue(OUString(L"prpDouble"), inDouble);
    outDouble = inv->getValue(OUString(L"prpDouble"));

    inVariant <<= OUString(L"A string in an any");
    inv->setValue(OUString(L"prpVariant"), inVariant);
    outVariant = inv->getValue(OUString(L"prpVariant"));

    inObject <<= inv;
    inv->setValue(OUString(L"prpObject"), inObject);
    outObject = inv->getValue(OUString(L"prpObject"));

    inUnknown <<= xintUnknown;
    inv->setValue(OUString(L"prpUnknown"), inUnknown);
    outUnknown = inv->getValue(OUString(L"prpUnknown"));

    Currency cur(99999);
    inCY <<= cur;
    inv->setValue(OUString(L"prpCurrency"), inCY);
    outCY = inv->getValue(OUString(L"prpCurrency"));

    Date d(37889.0);
    inDate <<= d;
    inv->setValue(OUString(L"prpDate"), inDate);
    outDate = inv->getValue(OUString(L"prpDate"));

    Decimal dec(20, 0, 0xffffffff, 0xffffffff, 0x0fffffff);
    inDecimal <<= dec;
    inv->setValue(OUString(L"prpDecimal"), inDecimal);
    outDecimal = inv->getValue(OUString(L"prpDecimal"));

    SCode code(DISP_E_BADVARTYPE);
    inSCode <<= code;
    inv->setValue(OUString(L"prpSCode"), inSCode);
    outSCode = inv->getValue(OUString(L"prpSCode"));

    inrefLong <<= (sal_Int32) 123456;
    inv->setValue(OUString(L"prprefLong"), inrefLong);
    outrefLong = inv->getValue(OUString(L"prprefLong"));

    inrefVariant <<= OUString(L"A string in an any");
    inv->setValue(OUString(L"prprefVariant"), inrefVariant);
    outrefVariant = inv->getValue(OUString(L"prprefVariant"));

    Decimal decref(20, 0, 0xffffffff, 0xffffffff, 0x0fffffff);
    inrefDecimal <<= decref;
    inv->setValue(OUString(L"prprefDecimal"), inrefDecimal);
    outrefDecimal = inv->getValue(OUString(L"prprefDecimal"));

    if (inBool != outBool || inByte != outByte || inShort != outShort || inLong != outLong
         || inFloat != outFloat || inDouble != outDouble || inString != outString
         || inVariant != outVariant || inObject != outObject
        || inUnknown != outUnknown || inCY != outCY
        || inDate != outDate || inDecimal != outDecimal || inSCode != outSCode
        || inrefLong != outrefLong ||inrefVariant != outrefVariant
        || inrefDecimal != outrefDecimal)
        return false;
    return true;
}

bool doPropertyWithArgumentTest(const Reference<XInvocation> & inv)
{
    Sequence< sal_Int16> seqIndices;
    Sequence<Any> seqOut;

    Any arMultiArgs[3];
    arMultiArgs[0] <<= makeAny((sal_Int32) 0);
    arMultiArgs[1] <<= makeAny((sal_Int32) 0);
    arMultiArgs[2] <<= PropertyPutArgument(makeAny((sal_Int32) 0));
    Sequence<Any> seqMultiArgPut0(arMultiArgs, 3);

    arMultiArgs[0] <<= makeAny((sal_Int32) 1);
    arMultiArgs[1] <<= makeAny((sal_Int32) 2);
    arMultiArgs[2] <<= PropertyPutArgument(makeAny((sal_Int32) 3));
    Sequence<Any> seqMultiArgPut1(arMultiArgs, 3);

    arMultiArgs[0] <<= makeAny((sal_Int32) 1);
    arMultiArgs[1] <<= PropertyPutArgument(makeAny((sal_Int32) 3));
    Sequence<Any> seqMultiArgPut2(arMultiArgs, 2);

    arMultiArgs[0] <<= NamedArgument(OUString(L"val2"), makeAny((sal_Int32) 1));
    arMultiArgs[1] <<= PropertyPutArgument(makeAny((sal_Int32) 3));
    Sequence<Any> seqMultiArgPut3(arMultiArgs, 2);

    arMultiArgs[0] <<= NamedArgument(OUString(L"val2"), makeAny((sal_Int32) 1));
    arMultiArgs[1] <<= NamedArgument(OUString(L"val3"), makeAny((sal_Int32) 3));
    Sequence<Any> seqMultiArgPut4(arMultiArgs, 2);

    arMultiArgs[0] <<= makeAny((sal_Int32) 0);
    arMultiArgs[1] <<= makeAny((sal_Int32) 0);
    Sequence<Any> seqMultiArgGet0(arMultiArgs, 2);

    arMultiArgs[0] <<= makeAny((sal_Int32) 1);
    arMultiArgs[1] <<= makeAny((sal_Int32) 2);
    Sequence<Any> seqMultiArgGet1(arMultiArgs, 2);
    Sequence<Any> seqMultiArgGet2(arMultiArgs, 1);


    arMultiArgs[0] <<= makeAny((sal_Int32) 0);
    arMultiArgs[1] <<= PropertyPutArgument(makeAny((sal_Int32) 0));
    Sequence<Any> seqMultiArgPut5(arMultiArgs, 2);

    arMultiArgs[0] <<= makeAny((sal_Int32) 1);
    arMultiArgs[1] <<= PropertyPutArgument(makeAny((sal_Int32) 2));
    Sequence<Any> seqMultiArgPut6(arMultiArgs, 2);

    arMultiArgs[0] <<= Any();
    arMultiArgs[1] <<= Any();
    Sequence<Any> seqMultiVoid(arMultiArgs, 2);

    arMultiArgs[0] = makeAny((sal_Int32) 0);
    arMultiArgs[1] = makeAny((sal_Int32) 0);
    Sequence<Any> seqMultiVoid2(arMultiArgs, 2);

    //[propput, ...] HRESULT prpMultiArg1([in,out,optional] VARIANT* val1, [in,out,optional] VARIANT* val2, [in] VARIANT* newVal);
    //[propget, ...] HRESULT prpMultiArg1([in,out,optional] VARIANT* val1, [in,out,optional] VARIANT* val2, [out, optional, retval] VARIANT* pVal);
    seqIndices.realloc( 0);
     seqOut.realloc(0);
    inv->invoke(OUString(L"prpMultiArg1"), seqMultiArgPut0, seqIndices, seqOut);
     seqIndices.realloc( 0);
     seqOut.realloc(0);
    inv->invoke(OUString(L"prpMultiArg1"), seqMultiArgPut1, seqIndices, seqOut);
    //check in/out args
       seqIndices.realloc( 0);
    seqOut.realloc(0);
     Any anyRet = inv->invoke(OUString(L"prpMultiArg1"), seqMultiArgGet0,
                             seqIndices, seqOut);

    if (anyRet != ((PropertyPutArgument const *)seqMultiArgPut1[2].getValue())->Value
        || ! checkOutArgs(seqOut, seqIndices, Sequence<Any>(seqMultiArgPut1.getArray(), 2)))
    {
       return false;
    }
    // test optional (one arg omitted
    seqIndices.realloc( 0);
     seqOut.realloc(0);
    inv->invoke(OUString(L"prpMultiArg1"), seqMultiArgPut0, seqIndices, seqOut);
     seqIndices.realloc( 0);
     seqOut.realloc(0);
    inv->invoke(OUString(L"prpMultiArg1"), seqMultiArgPut2, seqIndices, seqOut);
       seqIndices.realloc( 0);
    seqOut.realloc(0);
     anyRet = inv->invoke(OUString(L"prpMultiArg1"), seqMultiArgGet0,
                             seqIndices, seqOut);

    arMultiArgs[0] = makeAny((sal_Int32) 1);
    arMultiArgs[1] = makeAny((SCode(DISP_E_PARAMNOTFOUND)));

    if (anyRet != ((PropertyPutArgument const *) seqMultiArgPut2[1].getValue())->Value
        || ! checkOutArgs(seqOut, seqIndices, Sequence<Any>(arMultiArgs, 2)))
    {
       return false;
    }

    //test one named arg and one omitted
    seqIndices.realloc( 0);
     seqOut.realloc(0);
    inv->invoke(OUString(L"prpMultiArg1"), seqMultiArgPut0, seqIndices, seqOut);
     seqIndices.realloc( 0);
     seqOut.realloc(0);
    inv->invoke(OUString(L"prpMultiArg1"), seqMultiArgPut3, seqIndices, seqOut);
       seqIndices.realloc( 0);
    seqOut.realloc(0);
     anyRet = inv->invoke(OUString(L"prpMultiArg1"), seqMultiArgGet0,
                             seqIndices, seqOut);

    arMultiArgs[0] = makeAny((SCode(DISP_E_PARAMNOTFOUND)));
    arMultiArgs[1] = ((NamedArgument const*) seqMultiArgPut3[0].getValue())->Value;
    if (anyRet !=  ((PropertyPutArgument const*) seqMultiArgPut3[1].getValue())->Value
        || ! checkOutArgs(seqOut, seqIndices, Sequence<Any>(arMultiArgs, 2)))
    {
       return false;
    }

//    [propget,...] HRESULT prpMultiArg2([in] VARIANT val1, [out, retval] VARIANT* pVal);
//    [propput,...] HRESULT prpMultiArg2([in] VARIANT val1, [in] VARIANT newVal);
    seqIndices.realloc( 0);
     seqOut.realloc(0);
    inv->invoke(OUString(L"prpMultiArg2"), seqMultiArgPut5, seqIndices, seqOut);
     seqIndices.realloc( 0);
     seqOut.realloc(0);
    inv->invoke(OUString(L"prpMultiArg2GetValues"), seqMultiVoid, seqIndices, seqOut);

    if ( ! checkOutArgs(seqOut, seqIndices, seqMultiVoid2))
        return false;
       seqIndices.realloc( 0);
    seqOut.realloc(0);
     anyRet = inv->invoke(OUString(L"prpMultiArg2"), seqMultiArgPut6,
                             seqIndices, seqOut);
       seqIndices.realloc( 0);
    seqOut.realloc(0);
     anyRet = inv->invoke(OUString(L"prpMultiArg2GetValues"), seqMultiVoid,
                             seqIndices, seqOut);

// [propget,...] HRESULT prpMultiArg3([in,out] LONG* val1, [out, retval] LONG* pVal);
// [propput,...] HRESULT prpMultiArg3([in,out] LONG* val1, [in] LONG newVal);

    if ( ! checkOutArgs(seqOut, seqIndices, seqMultiArgGet1 ))
         return false;
    seqIndices.realloc( 0);
     seqOut.realloc(0);
    inv->invoke(OUString(L"prpMultiArg3"), seqMultiArgPut5, seqIndices, seqOut);
     seqIndices.realloc( 0);
     seqOut.realloc(0);
    inv->invoke(OUString(L"prpMultiArg3"), seqMultiArgPut6, seqIndices, seqOut);
     seqIndices.realloc( 0);
     seqOut.realloc(0);
    anyRet = inv->invoke(OUString(L"prpMultiArg3"), seqMultiArgGet2, seqIndices, seqOut);

    if ( anyRet !=  ((PropertyPutArgument const*) seqMultiArgPut6[1].getValue())->Value
         || !checkOutArgs(seqOut, seqIndices, seqMultiArgGet2))
        return false;


    //hasProperty, hasMethod
    if (inv->hasProperty(OUSTR("prpMultiArg1")))
        return false;
    if ( ! inv->hasMethod(OUSTR("prpMultiArg1")))
        return false;
    if ( ! inv->hasProperty(OUSTR("prprefLong")))
        return false;
    if (inv->hasMethod(OUSTR("prprefLong")))
        return false;
    if ( ! inv->hasMethod(OUSTR("inLong")))
        return false;

    return true;
}
bool doSimpleTest(const Reference<XInvocation> & inv)
{
    Sequence< sal_Int16> seqIndices;
    Sequence<Any> seqOut;

    Any inBool, outBool;
    Any inByte, outByte;
    Any inShort, outShort;
    Any inLong,  outLong;
    Any inString,  outString;
    Any inFloat, outFloat;
    Any inDouble, outDouble;
    Any inVariant, outVariant;
    Any inObject, outObject;
    Any inUnknown, outUnknown;
    Any inCY, outCY;
    Any inDate, outDate;
    Any inDecimal, outDecimal;
    Any inSCode, outSCode;
    Any inrefLong, outrefLong;
    Any inrefVariant, outrefVariant;
    Any inrefDecimal, outrefDecimal;

    Reference<XInterface> xIntFoo(getComObject(L"AxTestComponents.Foo"));
    //###################################################################################
    //  in and out parameter
    //###################################################################################
    sal_Bool aBool = sal_True;
    inBool.setValue(&aBool, getCppuBooleanType());
    inv->invoke(OUString(L"inBool"), Sequence< Any > ( &inBool, 1), seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outBool"), Sequence< Any > ( & outBool, 1), seqIndices, seqOut);
    outBool <<= seqOut[0];

    inByte <<= (sal_Int8) 127;
    inv->invoke(OUString(L"inByte"), Sequence< Any > ( & inByte, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outByte"), Sequence< Any > ( & outByte, 1), seqIndices, seqOut);
    outByte <<= seqOut[0];

    inShort <<= static_cast<sal_Int16>(-1);
    inv->invoke(OUString(L"inShort"), Sequence< Any > ( & inShort, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outShort"), Sequence< Any > ( & outShort, 1), seqIndices, seqOut);
    outShort <<= seqOut[0];

    inLong <<= ( sal_Int32) 1234567;
    inv->invoke(OUString(L"inLong"), Sequence< Any > ( & inLong, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outLong"), Sequence< Any > ( & outLong, 1 ), seqIndices, seqOut);
    outLong <<= seqOut[0];

    inString <<= OUString(L" this is clientTest.exe");
    inv->invoke(OUString(L"inString"), Sequence< Any > ( & inString, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outString"), Sequence< Any > ( & outString, 1 ), seqIndices, seqOut);
    outString <<= seqOut[0];

    inFloat <<=  3.14f;
    inv->invoke(OUString(L"inFloat"), Sequence< Any > ( & inFloat, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outFloat"), Sequence< Any > ( & outFloat, 1 ), seqIndices, seqOut);
    outFloat <<= seqOut[0];

    inDouble <<= ( double) 3.145;
    inv->invoke(OUString(L"inDouble"), Sequence< Any > ( & inDouble, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outDouble"), Sequence< Any > ( & outDouble, 1 ), seqIndices, seqOut);
    outDouble <<= seqOut[0];

    inVariant <<= OUString(L" A string in an any");
    inv->invoke(OUString(L"inVariant"), Sequence< Any > ( & inVariant, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outVariant"), Sequence< Any > (&outVariant, 1), seqIndices, seqOut);
    outVariant <<= seqOut[0];

    inObject <<= inv;
    inv->invoke(OUString(L"inObject"), Sequence< Any > ( & inObject, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outObject"), Sequence< Any > (& outObject, 1), seqIndices, seqOut);
    outObject <<= seqOut[0];

    inUnknown <<= xIntFoo;
    inv->invoke(OUString(L"inUnknown"), Sequence< Any > ( & inUnknown, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outUnknown"), Sequence< Any > (& outUnknown, 1), seqIndices, seqOut);
    outUnknown <<= seqOut[0];

    Currency cur(999999);
    inCY <<= cur;
    inv->invoke(OUString(L"inCurrency"), Sequence<Any>( & inCY, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outCurrency"), Sequence< Any > (& outCY, 1), seqIndices, seqOut);
    outCY <<= seqOut[0];

    Date dDate(37889.0);
    inDate <<= dDate;
    inv->invoke(OUString(L"inDate"), Sequence<Any>( & inDate, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outDate"), Sequence< Any > (& outDate, 1), seqIndices, seqOut);
    outDate <<= seqOut[0];

    Decimal dec(3, 0, 0xffffffff, 0xffffffff, 0xfffffff0);
    inDecimal <<= dec;
    inv->invoke(OUString(L"inDecimal"), Sequence<Any>( & inDecimal, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outDecimal"), Sequence< Any > (& outDecimal, 1), seqIndices, seqOut);
    outDecimal <<= seqOut[0];

    SCode code(DISP_E_BADVARTYPE);
    inSCode <<= code;
    inv->invoke(OUString(L"inSCode"), Sequence<Any>( & inSCode, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outSCode"), Sequence< Any > (& outSCode, 1), seqIndices, seqOut);
    outSCode <<= seqOut[0];

    if (inBool != outBool || inByte != outByte || inShort != outShort || inLong != outLong
        || inFloat != outFloat || inDouble != outDouble || inString != outString
        || inVariant != outVariant || inObject != outObject || inUnknown != outUnknown
        || inCY != outCY
        || inDate != outDate || inDecimal != outDecimal || inSCode != outSCode)
        return false;
    //###################################################################################
    //  in/out parameter
    //###################################################################################
    outBool = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutBool"), Sequence< Any > ( & inBool, 1), seqIndices, seqOut);
    outBool <<= seqOut[0];

    outByte = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutByte"), Sequence< Any > ( & inByte, 1), seqIndices, seqOut);
    outByte <<= seqOut[0];

    outShort = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutShort"), Sequence< Any > ( & inShort, 1), seqIndices, seqOut);
    outShort <<= seqOut[0];

    outLong = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutLong"), Sequence< Any > ( & inLong, 1), seqIndices, seqOut);
    outLong <<= seqOut[0];

    outString = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutString"), Sequence< Any > ( & inString, 1), seqIndices, seqOut);
    outString <<= seqOut[0];

    outFloat = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutFloat"), Sequence< Any > ( & inFloat, 1), seqIndices, seqOut);
    outFloat <<= seqOut[0];

    outDouble = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutDouble"), Sequence< Any > ( &inDouble, 1), seqIndices, seqOut);
    outDouble <<= seqOut[0];

    outVariant = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutVariant"), Sequence< Any > ( & inVariant, 1), seqIndices, seqOut);
    outVariant <<= seqOut[0];

    outObject = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutObject"), Sequence< Any > ( & inObject, 1), seqIndices, seqOut);
    outObject <<= seqOut[0];

    outCY = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutCurrency"), Sequence< Any > ( & inCY, 1), seqIndices, seqOut);
    outCY <<= seqOut[0];

    outDate = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutDate"), Sequence< Any > ( & inDate, 1), seqIndices, seqOut);
    outDate <<= seqOut[0];

    outDecimal = Any();
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutDecimal"), Sequence< Any > (& inDecimal, 1), seqIndices, seqOut);
    outDecimal <<= seqOut[0];

    outSCode = Any();
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutSCode"), Sequence< Any > (& inSCode, 1), seqIndices, seqOut);
    outSCode <<= seqOut[0];

    if (inBool != outBool || inByte != outByte || inShort != outShort || inLong != outLong
        || inFloat != outFloat || inDouble != outDouble || inString != outString
        || inVariant != outVariant || inObject != outObject || inCY != outCY
        || inDate != outDate || inDecimal != outDecimal || inSCode != outSCode)
        return false;

    //###################################################################################
    //  in byref parameters
    //###################################################################################

    inrefLong <<= (sal_Int32) 1234;
    inv->invoke(OUString(L"inrefLong"), Sequence<Any>( & inrefLong, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outLong"), Sequence< Any > (& outrefLong, 1), seqIndices, seqOut);
    outrefLong <<= seqOut[0];

    inrefVariant <<= OUString(L" A string in an any");
    inv->invoke(OUString(L"inrefVariant"), Sequence< Any > ( & inrefVariant, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outVariant"), Sequence< Any > (&outrefVariant, 1), seqIndices, seqOut);
    outrefVariant <<= seqOut[0];

    Decimal refdec(5, 1, 0xffff, 0xff, 0x1);
    inrefDecimal <<= refdec;
    inv->invoke(OUString(L"inrefDecimal"), Sequence< Any > ( & inrefDecimal, 1), seqIndices, seqOut);
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outDecimal"), Sequence< Any > (&outrefDecimal, 1), seqIndices, seqOut);
    outrefDecimal <<= seqOut[0];

    if (inrefLong != outrefLong || inrefVariant != outrefVariant
        || inrefDecimal != outrefDecimal)
        return false;


    //###################################################################################
    //  mixed parameter
    //###################################################################################
    // mixed1
    seqIndices.realloc( 0);
    seqOut.realloc(0);
    Any param[3];
    param[0] = inByte;
    param[1] = inFloat;
    param[2] = inVariant;
    inv->invoke(OUString(L"mixed1"), Sequence< Any >(param, 3), seqIndices, seqOut);

    if (seqOut.getLength() != 3 || inByte != seqOut[0] || inFloat != seqOut[1]
        || inVariant != seqOut[2])
            return false;
    return true;
}

bool doSimpleSequenceTest(const Reference<XInvocation> & inv)
{
    bool ret = true;
    Sequence<sal_Int16> seqIndices;
    Sequence<Any> seqOut;
    Any voidAny;
    Any inArAny;
    Any outArray;
    Any inArBool, outArBool;
    Any inArByte, outArByte;
    Any inArShort, outArShort;
    Any inArLong, outArLong;
    Any inArString, outArString;
    Any inArFloat, outArFloat;
    Any inArDouble, outArDouble;
    Any inArObject, outArObject;
    Any outVariant;

    //Initialize arrays
    OUString arStr[]= {L"string0", L"string1", L"string2"};
    Sequence<OUString> seq( arStr, 3);
    inArString <<= seq;

    Any arAnyStrTmp[3];
    arAnyStrTmp[0]<<= arStr[0];
    arAnyStrTmp[1]<<= arStr[1];
    arAnyStrTmp[2]<<= arStr[2];
    Sequence<Any> seq_1( arAnyStrTmp, 3);
    inArAny <<= seq_1;
    //###################################################################################
    //  in, out Sequences
    //###################################################################################
    //Test sequence containing Anys of Strings
    inv->invoke(OUString(L"inArray"), Sequence< Any > ( & inArAny, 1), seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outArray"), Sequence<Any>( & voidAny, 1), seqIndices, seqOut);
    if (inArAny != seqOut[0])
        return false;

    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inArray"), Sequence< Any >( & inArString, 1), seqIndices, seqOut);
    inv->invoke(OUString(L"outArray"), Sequence< Any >( & voidAny, 1), seqIndices, seqOut);
    outArray <<= seqOut[0];

    sal_Int8 arByte[]={1,2,3};
    Sequence<sal_Int8> seqByte(arByte, 3);
    inArByte <<= seqByte;
    inv->invoke(OUString(L"inSequenceByte"),Sequence<Any>( & inArByte, 1), seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outSequenceByte"), Sequence< Any >( & voidAny, 1), seqIndices, seqOut);
    outArByte <<= seqOut[0];

    sal_Int16 arShort[]={4,5,6};
    Sequence<sal_Int16> seqShort(arShort, 3);
    inArShort<<= seqShort;
    inv->invoke(OUString(L"inSequenceShort"),Sequence< Any >( & inArShort, 1), seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outSequenceShort"), Sequence< Any >( & voidAny, 1), seqIndices, seqOut);
    outArShort <<= seqOut[0];

    sal_Int32 arLong[] = {7,8,9};
    Sequence<sal_Int32> seqLong(arLong, 3);
    inArLong <<= seqLong;
    inv->invoke(OUString(L"inSequenceLong"),Sequence< Any > ( & inArLong, 1), seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outSequenceLong"), Sequence< Any >( & voidAny, 1), seqIndices, seqOut);
    outArLong <<= seqOut[0];

    inv->invoke(OUString(L"inSequenceLong"),Sequence< Any > ( & inArLong, 1), seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outSequenceLong"), Sequence< Any >( & voidAny, 1), seqIndices, seqOut);
    outArLong <<= seqOut[0];

    inv->invoke( OUString(L"inSequenceString"),Sequence< Any > ( & inArString, 1), seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outSequenceString"), Sequence< Any >( & voidAny, 1), seqIndices, seqOut);
    outArString <<= seqOut[0];

    float arFloat[]={3.14f, 31.4f, 314.f};
    Sequence<float> seqFloat( arFloat, 3);
    inArFloat <<= seqFloat;
    inv->invoke( OUString(L"inSequenceFloat"),Sequence< Any > ( & inArFloat, 1), seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outSequenceFloat"), Sequence< Any >( & voidAny, 1), seqIndices, seqOut);
    outArFloat <<= seqOut[0];

    double arDouble[]={3.145, 31.45, 3145.};
    Sequence<double> seqDouble( arDouble, 3);
    inArDouble <<= seqDouble;
    inv->invoke(OUString(L"inSequenceDouble"),Sequence< Any >( & inArDouble, 1), seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outSequenceDouble"), Sequence< Any >( & voidAny, 1), seqIndices, seqOut);
    outArDouble <<= seqOut[0];

    Sequence<Reference<XInvocation> > seqObj(2);
    seqObj[0]=  getComObject(L"AxTestComponents.Basic");
    seqObj[1]=  getComObject(L"AxTestComponents.Basic");
    inArObject <<= seqObj;
    inv->invoke(OUString(L"inSequenceObject"),Sequence< Any >( & inArObject, 1), seqIndices, seqOut);
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"outSequenceObject"), Sequence< Any >( & voidAny, 1), seqIndices, seqOut);
    outArObject <<= seqOut[0];

    if ( ! equalSequences(inArByte, outArByte) || ! equalSequences(inArShort, outArShort)
        || ! equalSequences(inArLong, outArLong) || ! equalSequences(inArString, outArray)
        || ! equalSequences(inArFloat, outArFloat) || ! equalSequences(inArDouble, outArDouble)
        || ! equalSequences(inArString, outArString)  || ! equalSequences(inArObject, outArObject))
        return false;

    //###################################################################################
    //  in/out Sequences
    //###################################################################################
    seqIndices.realloc(0);
    seqOut.realloc(0);
    inv->invoke(OUString(L"inoutArray"), Sequence< Any >( & inArString, 1), seqIndices, seqOut);
    inv->invoke(OUString(L"inoutArray"), Sequence< Any >( & inArString, 1), seqIndices, seqOut);
    outArray <<= seqOut[0];

    inv->invoke(OUString(L"inoutSequenceByte"), Sequence<Any>( & inArByte, 1), seqIndices, seqOut);
    inv->invoke(OUString(L"inoutSequenceByte"), Sequence<Any>( & inArByte, 1), seqIndices, seqOut);
    outArByte <<= seqOut[0];

    inv->invoke(OUString(L"inoutSequenceShort"), Sequence<Any>( & inArShort, 1), seqIndices, seqOut);
    inv->invoke(OUString(L"inoutSequenceShort"), Sequence<Any>( & inArShort, 1), seqIndices, seqOut);
    outArShort <<= seqOut[0];

    inv->invoke(OUString(L"inoutSequenceLong"), Sequence<Any>( & inArLong, 1), seqIndices, seqOut);
    inv->invoke(OUString(L"inoutSequenceLong"), Sequence< Any >( & inArLong, 1), seqIndices, seqOut);
    outArLong <<= seqOut[0];

    inv->invoke(OUString(L"inoutSequenceString"), Sequence<Any>( & inArString, 1), seqIndices, seqOut);
    inv->invoke(OUString(L"inoutSequenceString"), Sequence<Any>( & inArString, 1), seqIndices, seqOut);
    outArString <<= seqOut[0];

    inv->invoke(OUString(L"inoutSequenceFloat"), Sequence<Any>( & inArFloat, 1), seqIndices, seqOut);
    inv->invoke(OUString(L"inoutSequenceFloat"), Sequence<Any>( & inArFloat, 1), seqIndices, seqOut);
    outArFloat <<= seqOut[0];

    inv->invoke(OUString(L"inoutSequenceDouble"), Sequence<Any>( & inArDouble, 1), seqIndices, seqOut);
    inv->invoke(OUString(L"inoutSequenceDouble"), Sequence<Any>( & inArDouble, 1), seqIndices, seqOut);
    outArDouble <<= seqOut[0];

    inv->invoke(OUString(L"inoutSequenceObject"), Sequence<Any>( & inArObject, 1), seqIndices, seqOut);
    inv->invoke(OUString(L"inoutSequenceObject"), Sequence<Any>( & inArObject, 1), seqIndices, seqOut);
    outArObject <<= seqOut[0];

    if ( ! equalSequences(inArByte, outArByte) || ! equalSequences(inArShort, outArShort)
        || ! equalSequences(inArLong, outArLong) || ! equalSequences(inArString, outArray)
        || ! equalSequences(inArFloat, outArFloat) || ! equalSequences(inArDouble, outArDouble)
        || ! equalSequences(inArString, outArString)  || ! equalSequences(inArObject, outArObject))
        return false;

    return ret;
}

HRESULT doTest()
{
    HRESULT hr= S_OK;
    USES_CONVERSION;
    Reference<XInvocation> inv= getComObject( L"AxTestComponents.Basic");

    HostWin* pWin= new HostWin( L"MFCCONTROL.MfcControlCtrl.1");
    CComPtr<IUnknown> spUnk= pWin->GetHostedControl();
    Reference<XInvocation> invMfc= convertComObject( spUnk.p);

    Sequence< sal_Int16> seqIndices;
    Sequence<Any> seqOut;

    Any aAny;
    Any anyOut;
    char buff[1024];
    Any seqAny;

    if (! doSimpleTest(inv))
    {
        fprintf(stdout, "### Test failed!\n");
        return E_FAIL;
    }

    if (! doPropertyTest(inv))
    {
        fprintf(stdout, "### Test failed!\n");
        return E_FAIL;
    }

    if ( ! doSimpleSequenceTest(inv))
    {
        fprintf(stdout, "### Test failed! \n");
        return E_FAIL;
    }

    if ( ! doParameterTest(inv))
    {
        fprintf(stdout, "### Test failed! \n");
        return E_FAIL;
    }

    if ( ! doPropertyWithArgumentTest(inv))
    {
        fprintf(stdout, "### Test failed! \n");
        return E_FAIL;
    }





//
//  //###################################################################################
//  //  in multi Sequences
//  //###################################################################################
//  // inMulDimArrayLong
    sal_Int32 arLongi[]={1,2,3};
    sal_Int32 arLongi2[]={4,5,6,7};
    sal_Int32 arLongi3[]={8,9,10,11,12};

    Sequence<sal_Int32> seqLongi1( arLongi, 3);
    Sequence<sal_Int32> seqLongi2( arLongi2, 4);
    Sequence<sal_Int32> seqLongi3( arLongi3, 5);

    Sequence< Sequence< sal_Int32 > > seq2i(3);
    seq2i[0]= seqLongi1;
    seq2i[1]= seqLongi2;
    seq2i[2]= seqLongi3;
    seqAny<<= seq2i;
    // dimension length 3,5
    inv->invoke( OUString(L"inMulDimArrayLong"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);
//
    //inMulDimArrayVariant
    inv->invoke( OUString(L"inMulDimArrayVariant"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);

    //inMulDimArrayLong2
    sal_Int32 arLongii1[]={1,2,3};
    sal_Int32 arLongii2[]={4,5,6,7};
    sal_Int32 arLongii3[]={8,9,10,11,12};
    sal_Int32 arLongii4[]={13,14,15,16};
    sal_Int32 arLongii5[]={17,18,19};

    Sequence<sal_Int32> seqLongii1( arLongii1, 3);
    Sequence<sal_Int32> seqLongii2( arLongii2, 4);
    Sequence<sal_Int32> seqLongii3( arLongii3, 5);
    Sequence<sal_Int32> seqLongii4( arLongii4, 4);
    Sequence<sal_Int32> seqLongii5( arLongii5, 3);

    Sequence< Sequence< sal_Int32 > > seq2ii(3);
    Sequence< Sequence< sal_Int32> > seq2ii2(2);
    seq2ii[0]= seqLongii1;
    seq2ii[1]= seqLongii2;
    seq2ii[2]= seqLongii3;

    seq2ii2[0]= seqLongii4;
    seq2ii2[1]= seqLongii5;

    Sequence< Sequence< Sequence< sal_Int32> > >  seq3ii(2);
    seq3ii[0]=seq2ii;
    seq3ii[1]=seq2ii2;
    seqAny<<= seq3ii;
    inv->invoke( OUString(L"inMulDimArrayLong2"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);

    // inMulDimArrayByte2
    sal_Int8 arByteii1[]={1,2,3};
    sal_Int8 arByteii2[]={4,5,6,7};
    sal_Int8 arByteii3[]={8,9,10,11,12};
    sal_Int8 arByteii4[]={13,14,15,16};
    sal_Int8 arByteii5[]={17,18,19};

    Sequence<sal_Int8> seqByteii1( arByteii1, 3);
    Sequence<sal_Int8> seqByteii2( arByteii2, 4);
    Sequence<sal_Int8> seqByteii3( arByteii3, 5);
    Sequence<sal_Int8> seqByteii4( arByteii4, 4);
    Sequence<sal_Int8> seqByteii5( arByteii5, 3);

    Sequence< Sequence< sal_Int8 > > seq2Byteii(3);
    Sequence< Sequence< sal_Int8> > seq2Byteii2(2);
    seq2Byteii[0]= seqByteii1;
    seq2Byteii[1]= seqByteii2;
    seq2Byteii[2]= seqByteii3;

    seq2Byteii2[0]= seqByteii4;
    seq2Byteii2[1]= seqByteii5;

    Sequence< Sequence< Sequence< sal_Int8> > >  seq3Byteii(2);
    seq3Byteii[0]=seq2Byteii;
    seq3Byteii[1]=seq2Byteii2;
    seqAny<<= seq3Byteii;
    inv->invoke( OUString(L"inMulDimArrayByte2"),Sequence< Any > ( &seqAny, 1), seqIndices, seqOut);



    //###################################################################################
    //###################################################################################
    //###################################################################################
    //  Tests with a MFC ActiveX control, ( pure dispinterface)
    //###################################################################################

    //###################################################################################
    //  in parameter MFC ActiveX
    //###################################################################################
    // unsigned char is not supported by MFC
    //  aAny <<= ( sal_Int8) 127;
    //  invMfc->invoke( OUString(L"inByte"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    if ( ! invMfc.is())
        return hr;
    aAny <<= static_cast<sal_Int16>(-1);
    aAny= invMfc->invoke( OUString(L"inShort"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);

//
    aAny <<= ( sal_Int32) 1234567;
    aAny=invMfc->invoke( OUString(L"inLong"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    sal_Int32 retLong= *(sal_Int32*)aAny.getValue();

    OUString str_1(L" this is clientTest.exe");
    aAny <<= str_1;
    aAny=invMfc->invoke( OUString(L"inString"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    aAny>>= str_1;

    aAny <<= ( float) 3.14;
    aAny=invMfc->invoke( OUString(L"inFloat"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);

    aAny <<= ( double) 3.145;
    aAny=invMfc->invoke( OUString(L"inDouble"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);

    aAny <<= OUString( L" A string in an any");
    aAny=invMfc->invoke( OUString(L"inVariant"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);


    Reference < XInvocation > inv5= getComObject(L"AxTestComponents.Basic");
    Any anyVal4;
    anyVal4 <<= OUString(L"this is the value of prpString");
    inv5->setValue( OUString(L"prpString"), anyVal4);
    aAny <<= inv5;
    aAny=invMfc->invoke( OUString(L"inObject"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
//
//  //###################################################################################
//  //  out parameter MFC ActiveX
//  //###################################################################################
//
//  // outShort
    aAny= invMfc->invoke( OUString(L"outShort"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    anyOut<<= seqOut[0];
    sprintf(buff, "MFC outShort %d",  *(sal_Int16*)anyOut.getValue());
    MessageBox( NULL, buff, _T("clientTest"), MB_OK);

    // outLong
    aAny= invMfc->invoke( OUString(L"outLong"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    anyOut<<= seqOut[0];
    sprintf(buff, "MFC outLong %d",  *(sal_Int32*)anyOut.getValue());
    MessageBox( NULL, buff, _T("clientTest"), MB_OK);

    // outString
    aAny= invMfc->invoke( OUString(L"outString"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    anyOut<<= seqOut[0];
    sprintf(buff, "MFC outString %S",  ((OUString*)anyOut.getValue())->getStr());
    MessageBox( NULL, buff, _T("clientTest"), MB_OK);

    // outFloat
    aAny= invMfc->invoke( OUString(L"outFloat"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    anyOut<<= seqOut[0];
    sprintf(buff, "MFC outFloat %f",  *(float*)anyOut.getValue());
    MessageBox( NULL, buff, _T("clientTest"), MB_OK);

    // outDouble
    aAny= invMfc->invoke( OUString(L"outDouble"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    anyOut<<= seqOut[0];
    sprintf(buff, "MFC outFloat %f",  *(double*)anyOut.getValue());
    MessageBox( NULL, buff, _T("clientTest"), MB_OK);

    // outVariant
    // we expect a string!! ( VT_BSTR)
    aAny= invMfc->invoke( OUString(L"outVariant"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    anyOut<<= seqOut[0];
    sprintf(buff, "MFC outVariant %S", ((OUString*)anyOut.getValue())->getStr());
    MessageBox( NULL, buff, _T("clientTest"), MB_OK);

    // outDouble
    aAny= invMfc->invoke( OUString(L"outObject"), Sequence< Any > ( &aAny, 1), seqIndices, seqOut);
    Reference<XInvocation> invOut5;
    seqOut[0]>>= invOut5;
    // we assume that an object of AxTestControls.Basic is being returned.
    anyOut= invOut5->getValue( OUString(L"prpString"));
    OUString tmpStr;
    anyOut>>=tmpStr;
    sprintf(buff, "MFC outObject, property:  %S",  tmpStr.getStr());
    MessageBox( NULL, buff, _T("clientTest"), MB_OK);


    //###################################################################################
    //  Sequence parameter MFC ActiveX
    //###################################################################################
    // Sequences are not directly supported.


    delete pWin;
    return hr;

}



//VARIANT_TRUE VT_UI1


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
