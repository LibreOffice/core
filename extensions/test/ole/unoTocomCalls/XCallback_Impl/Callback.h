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
// Callback.h : Declaration of the CCallback

#pragma once

#include "resource.h"


// CCallback
class ATL_NO_VTABLE CCallback :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CCallback, &CLSID_Callback>,
    public IDispatchImpl<ICallback, &IID_ICallback, &LIBID_XCALLBACK_IMPLLib>
{
public:
    CCallback()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_CALLBACK)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCallback)
    COM_INTERFACE_ENTRY(ICallback)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// ICallback
public:
    STDMETHOD(inSeqByte)(/*[in]*/ LPSAFEARRAY val);
    STDMETHOD(inSeqXEventListener)(/*[in]*/ LPSAFEARRAY listener, LPSAFEARRAY event);
    STDMETHOD(outSeqByte)(/*[out]*/ LPSAFEARRAY* outVal);
    STDMETHOD(inValues)(/*[in]*/short aChar, /*[in]*/ long aLong, /*[in]*/ BSTR aString);
    STDMETHOD(inoutLong)(/*[in,out]*/ long* inoutVal);
    STDMETHOD(inoutShort)(/*[in,out]*/ short* inoutVal);
    STDMETHOD(inoutByte)(/*[in,out]*/ unsigned char* inoutVal);
    STDMETHOD(inoutDouble)(/*[in,out]*/ double* inoutVal);
    STDMETHOD(inoutFloat)(/*[in,out]*/ float* inoutVal);
    STDMETHOD(inoutString)(/*[in,out]*/ BSTR *inoutVal);
    STDMETHOD(inoutChar)(/*[in,out]*/ short* inoutVal);
    STDMETHOD(inoutBool)(/*[in,out]*/ VARIANT_BOOL * inoutVal);
    STDMETHOD(inoutAny)(/*[in,out]*/ VARIANT* inoutVal);
    STDMETHOD(inoutSeqAny)(/*[in,out]*/ LPSAFEARRAY* pArray);
    STDMETHOD(inoutEnum)(/*[in,out]*/ long * inoutVal);
    STDMETHOD(inoutStruct)(/*[in,out]*/ IDispatch** inoutVal);
    STDMETHOD(inoutInterface)(/*[in,out]*/ IDispatch** ppdisp);
    STDMETHOD(inoutValuesAll)(
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
            /* [out][in] */ long __RPC_FAR *aLong);

    STDMETHOD(outByte)( unsigned char* outByte);
    STDMETHOD(outLong)(/*[out]*/ long* outLong);
    STDMETHOD(outShort)(/*[out]*/ short *outShort);
    STDMETHOD(outDouble)(/*[out]*/ double* outDouble);
    STDMETHOD(outFloat)(/*[out]*/ float* outFloat);
    STDMETHOD(outString)(/*[out]*/ BSTR * outString);
    STDMETHOD(outChar)(short* outChar);
    STDMETHOD(outBool)(VARIANT_BOOL* outBool);
    STDMETHOD(outAny)(VARIANT* outAny);
    STDMETHOD(outSeqAny)(/*[out]*/LPSAFEARRAY* outSeq);
    STDMETHOD(outEnum)(/*[out]*/ long* outEnum);
    STDMETHOD(outStruct)(/*[out]*/ IDispatch** outStruct);
            virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE outValuesAll(
            /* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdisp,
            /* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppSimpleStruct,
            /* [out] */ long __RPC_FAR *aSimpleEnum,
            /* [out] */ LPSAFEARRAY* outSeq,
            /* [out] */ VARIANT __RPC_FAR *varAny,
            /* [out] */ VARIANT_BOOL __RPC_FAR *aBool,
            /* [out] */ short __RPC_FAR *aChar,
            /* [out] */ BSTR __RPC_FAR *aString,
            /* [out] */ float __RPC_FAR *aFloat,
            /* [out] */ double __RPC_FAR *aDouble,
            /* [out] */ unsigned char __RPC_FAR *aByte,
            /* [out] */ short __RPC_FAR *aShort,
            /* [out] */ long __RPC_FAR *aLong);
//              );

//  STDMETHOD(outValuesAll)(
//          /*[out]*/ IDispatch** ppdisp,
//          /*[out]*/ IUnknown** ppSimpleStruct,
//          /*[out]*/ long* aSimpleEnum,
//          /*[out]*/ VARIANT* ArrayAny,
//          /*[out]*/ VARIANT* varAny,
//          /*[out]*/ VARIANT_BOOL * aBool,
//          /*[out]*/ unsigned short* aChar,
//          /*[out]*/ BSTR* aString, /*[out]*/ float* aFloat,
//          /*[out]*/ double* aDouble,
//          /*[out]*/ signed char* aByte, /*[out]*/ short* aShort, /*[out]*/long* aLong, /*[out]*/ unsigned short* aUShort, /*[out]*/ unsigned long* aULong);
    STDMETHOD(outValuesMixed)(/*[in]*/ long val, /*[out]*/ long* pval, /*[in]*/ BSTR string);
    STDMETHOD(outInterface)(/*[out]*/ IDispatch** ppdisp);
    STDMETHOD(returnInterface)(/*[out, retval]*/ IDispatch** ppdisp);
    STDMETHOD(func1)();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
