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

#include "unotypewrapper.hxx"
#include "rtl/ustring.hxx"
#include <osl/diagnose.h>


bool createUnoTypeWrapper(BSTR sTypeName, VARIANT * pVar)
{
    bool ret = false;
    OSL_ASSERT(sTypeName && pVar);
    CComObject< UnoTypeWrapper>* pObj;
    VariantClear(pVar);
    if( SUCCEEDED( CComObject<UnoTypeWrapper>::CreateInstance( &pObj)))
    {
        pObj->AddRef();
        pVar->vt= VT_DISPATCH;
        pVar->pdispVal= CComQIPtr<IDispatch>(pObj->GetUnknown());
        //now set the value, e.i. the name of the type
        CComQIPtr<IUnoTypeWrapper> spType(pVar->pdispVal);
        OSL_ASSERT(spType);
        if (SUCCEEDED(spType->put_Name(sTypeName)))
        {
            ret = true;
        }
    }
    return ret;
}


bool createUnoTypeWrapper(const OUString& sTypeName, VARIANT * pVar)
{
    CComBSTR bstr(reinterpret_cast<LPCOLESTR>(sTypeName.getStr()));
    return createUnoTypeWrapper(bstr, pVar);
}

UnoTypeWrapper::UnoTypeWrapper()
{
}

UnoTypeWrapper::~UnoTypeWrapper()
{
}


// UnoTypeWrapper, IDispatch --------------------------------------------
STDMETHODIMP UnoTypeWrapper::GetTypeInfoCount(UINT* /*pctinfo*/)
{
    return E_NOTIMPL;
}

// UnoTypeWrapper, IDispatch --------------------------------------------
STDMETHODIMP UnoTypeWrapper::GetTypeInfo( UINT /*iTInfo*/,
                                          LCID /*lcid*/,
                                          ITypeInfo** /*ppTInfo*/)
{
    return E_NOTIMPL;
}

// UnoTypeWrapper, IDispatch --------------------------------------------
STDMETHODIMP UnoTypeWrapper::GetIDsOfNames( REFIID /*riid*/,
                                            LPOLESTR *rgszNames,
                                            UINT /*cNames*/,
                                            LCID /*lcid*/,
                                            DISPID *rgDispId)
{
    if( !rgDispId)
        return E_POINTER;

    HRESULT ret= S_OK;
    CComBSTR name(*rgszNames);
    name.ToLower();

    if( name == CComBSTR( L"name") )
        *rgDispId= DISPID_VALUE;
    else
        ret= DISP_E_UNKNOWNNAME;

    return ret;
}

// UnoTypeWrapper, IDispatch --------------------------------------------
STDMETHODIMP UnoTypeWrapper::Invoke( DISPID dispIdMember,
                         REFIID /*riid*/,
                         LCID /*lcid*/,
                         WORD wFlags,
                         DISPPARAMS *pDispParams,
                         VARIANT *pVarResult,
                         EXCEPINFO* /*pExcepInfo*/,
                         UINT* /*puArgErr*/)
{
    if (pDispParams == nullptr)
        return DISP_E_EXCEPTION;

    if( pDispParams->cNamedArgs)
        return DISP_E_NONAMEDARGS;


    HRESULT ret= S_OK;
    switch( dispIdMember)
    {
    case DISPID_VALUE: // DISPID_VALUE
        if (wFlags & DISPATCH_PROPERTYGET)
        {
            if (pVarResult == nullptr)
            {
                ret = E_POINTER;
                break;
            }
            get_Name( & pVarResult->bstrVal);
            pVarResult->vt = VT_BSTR;
        }
        break;
    default:
        ret= DISP_E_MEMBERNOTFOUND;
        break;
     }

    return ret;
}

// IUnoTypeWrapper-----------------------
STDMETHODIMP UnoTypeWrapper::put_Name(BSTR  val)
{
     Lock();
     HRESULT hr = S_OK;
    m_sName = val;
     Unlock();
     return hr;
}

// (UnoTypeWrapper-----------------------
STDMETHODIMP UnoTypeWrapper::get_Name(BSTR  *pVal)
{
     Lock();
    HRESULT hr = S_OK;
     if( !pVal)
         return E_POINTER;
    *pVal = m_sName.Copy();
     Unlock();
     return hr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
