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
// Callback.cpp : Implementation of CCallback
#include "stdafx.h"
#include "XCallback_Impl.h"
#include "Callback.h"


// CCallback


STDMETHODIMP CCallback::func1()
{
    MessageBox( NULL, _T("Callback::func1 called"),_T(""), MB_OK);
    return S_OK;
}

STDMETHODIMP CCallback::returnInterface(IDispatch **ppdisp)
{
    if( ! ppdisp)
        return E_POINTER;
    CComPtr<IDispatch> spDisp;
    spDisp.CoCreateInstance( L"XCallback_Impl.Simple");
    *ppdisp= spDisp;
    (*ppdisp)->AddRef();
    return S_OK;
}

STDMETHODIMP CCallback::outInterface(IDispatch **ppdisp)
{
//  return S_OK;
    if( ! ppdisp)
        return E_POINTER;
    CComPtr<IDispatch> spDisp;
    spDisp.CoCreateInstance( L"XCallback_Impl.Simple");
    *ppdisp= spDisp;
    (*ppdisp)->AddRef();

//  MessageBox( NULL, _T("CCallback::outInterface"), _T(""), MB_OK);
    return S_OK;
}

STDMETHODIMP CCallback::outValuesMixed(long val, long *pval, BSTR string)
{
    USES_CONVERSION;
    char buff[1024];
    *pval = val+1;
    sprintf( buff, "param1: %d, param2 out: %d, param3: %S", val, *pval, string);
    MessageBox( NULL, A2T(buff), A2T("XCallback_Impl.Callback"), MB_OK);
    return S_OK;
}


STDMETHODIMP CCallback::outValuesAll(
            /* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdisp,
            /* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppSimpleStruct,
            /* [out] */ long __RPC_FAR *aSimpleEnum,
            /* [out] */ SAFEARRAY __RPC_FAR * __RPC_FAR *outSeq,
            /* [out] */ VARIANT __RPC_FAR *varAny,
            /* [out] */ VARIANT_BOOL __RPC_FAR *aBool,
            /* [out] */ short __RPC_FAR *aChar,
            /* [out] */ BSTR __RPC_FAR *aString,
            /* [out] */ float __RPC_FAR *aFloat,
            /* [out] */ double __RPC_FAR *aDouble,
            /* [out] */  unsigned char __RPC_FAR *aByte,
            /* [out] */ short __RPC_FAR *aShort,
            /* [out] */ long __RPC_FAR *aLong)
//)
{
//  if( ! ppdisp || ! ppSimpleStruct || ! aSimpleEnum ||
//      ! outSeq || !varAny ||! aBool || ! aChar ||
//      ! aString || ! aFloat || ! aDouble || ! aByte ||
//      ! aShort || ! aLong || ! aUShort || ! aULong)
//      return E_POINTER;

    HRESULT hr=S_OK;
    hr= outInterface( ppdisp);
    hr= outStruct( ppSimpleStruct);
    hr= outEnum( aSimpleEnum);
    hr= outSeqAny( outSeq);
    hr= outAny( varAny);
    hr= outBool( aBool);
    hr= outChar( aChar);
    hr= outString( aString);
    hr= outFloat( aFloat);
    hr= outDouble( aDouble);
    hr= outByte( aByte);
    hr= outShort( aShort);
    hr= outLong( aLong);
    return hr;
}

STDMETHODIMP CCallback::outStruct(IDispatch **outStruct)
{
//  return S_OK;
    if( !outStruct)
        return E_POINTER;
    HRESULT hr= E_FAIL;
//  MessageBox( NULL, _T("CCallback::outStruct"), _T(""), MB_OK);

    CComPtr<IDispatch> _dispMgr;
    if( SUCCEEDED(hr= _dispMgr.CoCreateInstance(L"com.sun.star.ServiceManager")))
    {
        CComDispatchDriver manager( _dispMgr);
        CComVariant param1(L"com.sun.star.reflection.CoreReflection");
        CComVariant varRet;
        hr= manager.Invoke1( L"createInstance", &param1, &varRet);

        CComDispatchDriver reflection( varRet.pdispVal);
        param1= L"oletest.SimpleStruct";
        varRet.Clear();
        hr= reflection.Invoke1( L"forName", &param1, &varRet);

        CComDispatchDriver classSimpleStruct( varRet.pdispVal);

        CComPtr<IDispatch> dispStruct;
        param1.vt= VT_DISPATCH | VT_BYREF;
        param1.ppdispVal= &dispStruct;
        if( SUCCEEDED( hr= classSimpleStruct.Invoke1(L"createObject", &param1)))
        {
            // Set the value
            CComDispatchDriver simpleStruct( dispStruct);
            param1=L" this is a property string";
            hr= simpleStruct.PutPropertyByName(L"message", &param1);
            *outStruct= dispStruct;
            (*outStruct)->AddRef();
            hr= S_OK;
        }
    }
    return hr;
}

STDMETHODIMP CCallback::outEnum(long *outEnum)
{
    if( !outEnum)
        return E_POINTER;
    *outEnum= 1;
    return S_OK;
}

STDMETHODIMP CCallback::outSeqAny(LPSAFEARRAY* outSeq)
{
//  _CrtDbgBreak();
    SAFEARRAY* pArr= SafeArrayCreateVector( VT_VARIANT, 0, 3);
    CComVariant var[3];
    var[0]=L" variant 0";
    var[1]=L" variant 1";
    var[2]=L"variant 2";
    for( long i=0; i<3; i++)
    {
        SafeArrayPutElement( pArr, &i, (void*)&var[i]);
    }

    *outSeq= pArr;
    return S_OK;
}

// ATLASSERT //VT_EMPTY


STDMETHODIMP CCallback::outAny(VARIANT *outAny)
{
    if( ! outAny)
        return E_POINTER;
    outAny->vt= VT_BSTR;
    outAny->bstrVal= SysAllocString( L"This is a string in a VARIANT");

    return S_OK;
}

STDMETHODIMP CCallback::outBool(VARIANT_BOOL *outBool)
{
    if( ! outBool)
        return E_POINTER;
    *outBool= VARIANT_TRUE;
    return S_OK;
}

STDMETHODIMP CCallback::outChar(short *outChar)
{
    if( !outChar)
        return E_POINTER;
    *outChar= (short)L'A';
    return S_OK;
}

STDMETHODIMP CCallback::outString(BSTR *outString)
{
    if( !outString)
        return E_POINTER;
    *outString= SysAllocString(L"This is a BSTR");
    return S_OK;
}

STDMETHODIMP CCallback::outFloat(float *outFloat)
{
    if( !outFloat)
        return E_POINTER;
    *outFloat= 3.14f;
    return S_OK;
}

STDMETHODIMP CCallback::outDouble(double *outDouble)
{
    if(!outDouble)
        return E_POINTER;

    *outDouble= 3.145;
    return S_OK;
}



STDMETHODIMP CCallback::outShort(short *outShort)
{
    if(!outShort)
        return E_POINTER;
    *outShort= -1;
    return S_OK;
}

STDMETHODIMP CCallback::outLong(long *outLong)
{
    if(!outLong)
        return E_POINTER;
    *outLong= 0xffffffff;
    return S_OK;
}



STDMETHODIMP CCallback::outByte(unsigned char* outByte)
{
    if(!outByte)
        return E_POINTER;
    *outByte= 0xff;
    return S_OK;
}

STDMETHODIMP CCallback::inoutInterface(IDispatch **ppdisp)
{
    if( !ppdisp)
        return E_POINTER;
    CComDispatchDriver disp( *ppdisp);
    CComVariant param1(L"");
    disp.Invoke1(L"func", &param1);

    (*ppdisp)->Release();

    CComPtr<IDispatch> outDisp;
    outDisp.CoCreateInstance( L"XCallback_Impl.Simple");
    *ppdisp= outDisp;
    (*ppdisp)->AddRef();

    return S_OK;
}

STDMETHODIMP CCallback::inoutStruct(IDispatch **inoutVal)
{
    if( !inoutVal)
        return E_POINTER;
    HRESULT hr= S_OK;
    USES_CONVERSION;
    CComVariant var;
    CComDispatchDriver disp( *inoutVal);

    hr= disp.GetPropertyByName(L"message", &var);
    MessageBox( NULL, W2T(var.bstrVal), _T("XCallback_Impl.Callback"), MB_OK);

    (*inoutVal)->Release();

    CComDispatchDriver dispStruct;
    hr= outStruct( &dispStruct.p);
    var.Clear();
    var= L"This struct was created in XCallback_Imp.Callback";
    hr= dispStruct.PutPropertyByName(L"message", &var);

    *inoutVal= dispStruct;
    (*inoutVal)->AddRef();
    return hr;
}

STDMETHODIMP CCallback::inoutEnum(long *inoutVal)
{
    if( !inoutVal)
        return E_POINTER;
    *inoutVal= *inoutVal+1;

    return S_OK;
}

STDMETHODIMP CCallback::inoutSeqAny(LPSAFEARRAY *pArray)
{
    if( !pArray)
        return E_POINTER;
    HRESULT hr= S_OK;
    long lbound=0;
    long ubound=0;
    hr= SafeArrayGetLBound( *pArray, 1, &lbound);
    hr= SafeArrayGetUBound( *pArray, 1, &ubound);
    long count= ubound - lbound + 1;

    // the Array is supposet to contain variants
    CComVariant var;
    for( long i=0; i<count; i++)
    {
        var.Clear();
        hr= SafeArrayGetElement( *pArray, &i, (void*)&var);
    }

    SafeArrayDestroy( *pArray);

    outSeqAny( pArray);
    return S_OK;
}

STDMETHODIMP CCallback::inoutAny(VARIANT *inoutVal)
{
    if( !inoutVal)
        return E_POINTER;
    USES_CONVERSION;
    if( inoutVal->vt= VT_BSTR)
        MessageBox( NULL, W2T( inoutVal->bstrVal), _T("XCallback_Impl.Callback"), MB_OK);

    VariantClear( inoutVal);
    inoutVal->vt= VT_BSTR;
    inoutVal->bstrVal=SysAllocString( L" [string] XCallback_Impl.Callback inoutAny");
    return S_OK;
}

STDMETHODIMP CCallback::inoutBool(VARIANT_BOOL *inoutVal)
{
    if( !inoutVal)
        return E_POINTER;

    *inoutVal= *inoutVal == VARIANT_TRUE ? VARIANT_FALSE : VARIANT_TRUE;
    return S_OK;
}

STDMETHODIMP CCallback::inoutChar(short *inoutVal)
{
    if( !inoutVal)
        return E_POINTER;
    USES_CONVERSION;
    char buff[256];
    sprintf( buff, "character value: %C", *inoutVal);
    MessageBox( NULL, A2T(buff), _T("XCallback_Impl.Callback"), MB_OK);
    *inoutVal= L'B';
    return S_OK;
}

STDMETHODIMP CCallback::inoutString(BSTR *inoutVal)
{
    if( !inoutVal)
        return E_POINTER;
    USES_CONVERSION;
    MessageBox( NULL, W2T(*inoutVal), _T("XCallback_Impl.Callback"), MB_OK);
    SysFreeString(*inoutVal);
    *inoutVal= SysAllocString(L"a string from XCallback_Impl.Callback");

    return S_OK;
}

STDMETHODIMP CCallback::inoutFloat(float *inoutVal)
{
    if( !inoutVal)
        return E_POINTER;
    *inoutVal = *inoutVal+1;
    return S_OK;
}

STDMETHODIMP CCallback::inoutDouble(double *inoutVal)
{
    if( !inoutVal)
        return E_POINTER;
    *inoutVal= *inoutVal+1;
    return S_OK;
}

STDMETHODIMP CCallback::inoutByte(unsigned char *inoutVal)
{
    if( !inoutVal)
        return E_POINTER;
    *inoutVal= 0xff;
    return S_OK;
}

STDMETHODIMP CCallback::inoutShort(short *inoutVal)
{
    if( !inoutVal)
        return E_POINTER;
    *inoutVal= -1;
    return S_OK;
}

STDMETHODIMP CCallback::inoutLong(long* inoutVal)
{
    if( !inoutVal)
        return E_POINTER;
    *inoutVal= 0xffffffff;
    return S_OK;
}

STDMETHODIMP CCallback::inoutValuesAll(
            /* [out][in] */ IDispatch __RPC_FAR *__RPC_FAR *aXSimple,
            /* [out][in] */ IDispatch __RPC_FAR *__RPC_FAR *aStruct,
            /* [out][in] */ long __RPC_FAR *aEnum,
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *aSeq,
            /* [out][in] */ VARIANT __RPC_FAR *aAny,
            /* [out][in] */ VARIANT_BOOL __RPC_FAR *aBool,
            /* [out][in] */ short __RPC_FAR *aChar,
            /* [out][in] */ BSTR __RPC_FAR *aString,
            /* [out][in] */ float __RPC_FAR *aFloat,
            /* [out][in] */ double __RPC_FAR *aDouble,
            /* [out][in] */ unsigned char __RPC_FAR *aByte,
            /* [out][in] */ short __RPC_FAR *aShort,
            /* [out][in] */ long __RPC_FAR *aLong)
{
    inoutInterface( aXSimple);
    inoutStruct( aStruct);
    inoutEnum( aEnum);
    inoutSeqAny( aSeq);
    inoutAny( aAny);
    inoutBool( aBool);
    inoutChar( aChar);
    inoutString( aString);
    inoutFloat( aFloat);
    inoutDouble( aDouble);
    inoutByte( aByte);
    inoutShort( aShort);
    inoutLong( aLong);

    return S_OK;
}


STDMETHODIMP CCallback::inValues(short aChar, long aLong, BSTR aString)
{
    USES_CONVERSION;
    wchar_t _char= (wchar_t) aChar;
    char buff[1024];
    sprintf( buff, "Parameters: char= %C, long= %d, string= %s", _char, aLong, W2A(aString));
    MessageBox( NULL, A2T(buff), _T("XCallback_Impl.Callback"), MB_OK);
    return S_OK;
}

STDMETHODIMP CCallback::outSeqByte(LPSAFEARRAY * outVal)
{
    // TODO: Add your implementation code here

    return S_OK;
}

STDMETHODIMP CCallback::inSeqByte( LPSAFEARRAY listeners)
{

    return S_OK;
}

STDMETHODIMP CCallback::inSeqXEventListener( LPSAFEARRAY listeners, LPSAFEARRAY events)
{
    HRESULT hr= S_OK;
    long ubound= 0;
    long lbound= 0;
    long count= 0;
    hr= SafeArrayGetUBound( listeners, 1, &ubound);
    hr= SafeArrayGetLBound( listeners, 1, &lbound);
    count= ubound - lbound +1;

    // We assume thate the count of EventObjects in events is the same
    for( long i = 0; i < count; i++)
    {
        CComVariant varListener;
        CComVariant varEvent;
        hr= SafeArrayGetElement( listeners, &i, &varListener);
        hr= SafeArrayGetElement( events, &i, &varEvent);
        if( varListener.vt == VT_DISPATCH && varEvent.vt == VT_DISPATCH)
        {
            CComDispatchDriver disp( varListener.pdispVal);
            hr= disp.Invoke1(L"disposing", &varEvent);
        }

    }

    return S_OK;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
