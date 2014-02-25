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
// Test.cpp : Defines the entry point for the console application.


#include "stdafx.h"
#include "../XCallback_Impl/XCallback_Impl.h"
#include "../XCallback_Impl/XCallback_Impl_i.c"

CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

HRESULT doTest();

int main(int argc, char* argv[])
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


    _Module.Term();
    CoUninitialize();


    return 0;
}


HRESULT doTest()
{
    HRESULT hr= S_OK;

    CComPtr<IUnknown> spUnk;
    hr= spUnk.CoCreateInstance(L"com.sun.star.ServiceManager");
    if( FAILED( hr))
        return hr;

    CComDispatchDriver manager( spUnk);
    CComVariant param( L"oletest.OleTest");
    CComVariant retVal;
    hr= manager.Invoke1((LPCOLESTR)L"createInstance", &param, &retVal );

    CComDispatchDriver oletest( retVal.punkVal);

    spUnk.Release();

    hr= spUnk.CoCreateInstance(L"XCallback_Impl.Callback");
    if( FAILED( hr))
        return hr;

    CComQIPtr<IDispatch> paramDisp(spUnk);



    //  out parameters

    CComVariant param1( paramDisp);
    CComVariant param2(1);

    // oletest calls XCallback::func1
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::returnInterface
    param2= 2;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outInterface
    param2= 3;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outStruct
    param2= 4;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outEnum
    param2= 5;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outSeqAny
    param2= 6;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outAny
    param2= 7;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outBool
    param2= 8;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outChar
    param2= 9;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outString
    param2= 10;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outFloat
    param2= 11;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outDouble
    param2= 12;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outByte
    param2= 13;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outShort
    param2= 14;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outLong
    param2= 15;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outValuesMixed
    param2= 30;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::outValuesAll
    param2= 31;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);

    // XCallback::outSeqByte
    // Does not work currently because Sequences are always converted to
    // SAFEARRAY( VARIANT)
    //  param2= 32;
    //  hr= oletest.Invoke2(L"testInterface", &param1, &param2);


    //  in / out parameters

    // XCallback::inoutInterface
    param2= 100;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutStruct
    param2= 101;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutEnum
    param2= 102;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutSeqAny
    param2= 103;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutAny
    param2= 104;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutBool
    param2= 105;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutChar
    param2= 106;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutString
    param2= 107;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutFloat
    param2= 108;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutDouble
    param2= 109;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutByte
    param2= 110;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutShort
    param2= 111;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutLong
    param2= 112;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutValuesAll
    param2=120;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);

    //  in  parameters

    // XCallback::inValues
    param2= 200;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inSeqByte
    // SAFEARRAY( VARIANT)
    param2= 201;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    //XCallback::inSeqXEventListener
    param2= 202;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);

    //  The UNO test component OleTest calls on XCallback_Impl.Callback directly
    // that is the COM object has not been past a parameter but rather OleTest
    // creates the COM object itself

    // XCallback::outValuesAll
    // does not work currently
    param2= 300;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutValuesAll
    param2= 301;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);
    // XCallback::inoutValues
    param2= 302;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);

    // XCallback::inValues
    param2= 303;
    hr= oletest.Invoke2(L"testInterface", &param1, &param2);

    // Test a COM object which implements several interfaces.


    CComQIPtr<IDispatch> dispSimple;
    hr= dispSimple.CoCreateInstance(L"XCallback_Impl.Simple");
    CComVariant varSimple( dispSimple);
    param2= 0;
    hr= oletest.Invoke2(L"testInterface2", &varSimple, &param2);

    return hr;
}
// VARIANT CComVariant VT_UNKNOWN VT_DISPATCH V_UI1 CComDispatchDriver WINAPI


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
