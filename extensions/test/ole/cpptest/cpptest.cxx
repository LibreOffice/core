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

// cpptest.cpp : Defines the entry point for the console application.


#ifdef _MSC_VER
#pragma once
#endif
#pragma warning(disable: 4917)
#include <comdef.h>
#include <tchar.h>
#include<atlbase.h>
#include<atlcom.h>

HRESULT doTest();

int main(int /*argc*/, char** /*argv*/)
{
    HRESULT hr;
    if( FAILED( hr=CoInitialize(NULL)))
    {
        _tprintf(_T("CoInitialize failed \n"));
        return -1;
    }

    if( FAILED(hr=doTest()))
    {
        _com_error err( hr);
        const TCHAR * errMsg= err.ErrorMessage();
        MessageBox( NULL, errMsg, "Test failed", MB_ICONERROR);
    }

    CoUninitialize();
    return 0;
}

HRESULT doTest()
{
    HRESULT hr;
    CComPtr<IUnknown> spUnkMgr;


    if( FAILED(hr= spUnkMgr.CoCreateInstance(L"com.sun.star.ServiceManager")))
        return hr;

    IDispatchPtr starManager;
    //    var starManager=new ActiveXObject("com.sun.star.ServiceManager");
    if (FAILED(hr= starManager.CreateInstance(_T("com.sun.star.ServiceManager"))))
    {
        fprintf(stderr, "creating ServiceManager failed\n");
        return hr;
    }
    //    var starDesktop=starManager.createInstance("com.sun.star.frame.Desktop");
    _variant_t varP1(L"com.sun.star.frame.Desktop");
    _variant_t varRet;
    CComDispatchDriver dispMgr(starManager);
    if (FAILED(hr=  dispMgr.Invoke1(L"createInstance", &varP1, &varRet)))
    {
        fprintf(stderr,"createInstance of Desktop failed\n");
        return hr;
    }
    CComDispatchDriver dispDesk(varRet.pdispVal);
    varP1.Clear();
    varRet.Clear();
    //    var bOK=new Boolean(true);

    //    var noArgs=new Array();
    //    var oDoc=starDesktop.loadComponentFromURL("private:factory/swriter", "Test", 40, noArgs);
    IDispatchPtr oDoc;
    SAFEARRAY* ar= SafeArrayCreateVector(VT_DISPATCH, 0, 0);
    _variant_t args[4];
    args[3]= _variant_t(L"private:factory/swriter");
    args[2]= _variant_t(L"Test");
    args[1]= _variant_t((long) 40);
    args[0].vt= VT_ARRAY | VT_DISPATCH;
    args[0].parray= ar;
    if (FAILED(hr= dispDesk.InvokeN(L"loadComponentFromURL", args, 4, &varRet)))
    {
        fprintf(stderr,"loadComponentFromURL failed\n");
        return hr;
    }
    CComDispatchDriver dispDoc(varRet.pdispVal);
    varRet.Clear();
    return S_OK;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
