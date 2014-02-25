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

#include "jscriptclasses.hxx"


// JScriptValue

JScriptValue::JScriptValue(): m_bOutParam(0), m_bInOutParam(0)
{
}

JScriptValue::~JScriptValue()
{
}


// JScriptValue, IDispatch --------------------------------------------
STDMETHODIMP JScriptValue::GetTypeInfoCount(UINT* /*pctinfo*/)
{
    return E_NOTIMPL;
}

// JScriptValue, IDispatch --------------------------------------------
STDMETHODIMP JScriptValue::GetTypeInfo( UINT /*iTInfo*/,
                                           LCID /*lcid*/,
                                           ITypeInfo** /*ppTInfo*/)
{
    return E_NOTIMPL;
}

// JScriptValue, IDispatch --------------------------------------------
STDMETHODIMP JScriptValue::GetIDsOfNames( REFIID /*riid*/,
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

    if( name == CComBSTR( L"set") )
        *rgDispId= 1;
    else if( name == CComBSTR( L"get") )
        *rgDispId= 2;
    else if( name == CComBSTR( L"initoutparam") )
        *rgDispId= 3;
    else if( name == CComBSTR( L"initinoutparam") )
        *rgDispId= 4;
    else
        ret= DISP_E_UNKNOWNNAME;

    return ret;
}

// JScriptValue, IDispatch --------------------------------------------
STDMETHODIMP JScriptValue::Invoke( DISPID dispIdMember,
                         REFIID /*riid*/,
                         LCID /*lcid*/,
                         WORD wFlags,
                         DISPPARAMS *pDispParams,
                         VARIANT *pVarResult,
                         EXCEPINFO* /*pExcepInfo*/,
                         UINT* /*puArgErr*/)
{
    if( pDispParams->cNamedArgs)
        return DISP_E_NONAMEDARGS;


    HRESULT ret= S_OK;
    switch( dispIdMember)
    {
    case 0: // DISPID_VALUE
        if( wFlags & DISPATCH_PROPERTYGET && pVarResult)
        {
            if( FAILED( VariantCopy( pVarResult, &m_varValue)))
                ret= E_FAIL;
        }
        else
            ret= E_POINTER;
        break;
    case 1:
        if( wFlags & DISPATCH_METHOD)
            ret= Set( pDispParams->rgvarg[1], pDispParams->rgvarg[0]);
        if( FAILED( ret))
            ret= DISP_E_EXCEPTION;
        break;
    case 2:
        if( wFlags & DISPATCH_METHOD)
            ret= Get( pVarResult);
        if( FAILED( ret))
            ret= DISP_E_EXCEPTION;
        break;
    case 3:
        if( wFlags & DISPATCH_METHOD)
            ret= InitOutParam();
        if( FAILED( ret))
            ret= DISP_E_EXCEPTION;
        break;
    case 4:
        if( wFlags & DISPATCH_METHOD)
            ret= InitInOutParam( pDispParams->rgvarg[1], pDispParams->rgvarg[0]);
        if( FAILED( ret))
            ret= DISP_E_EXCEPTION;
        break;
    default:
        ret= DISP_E_MEMBERNOTFOUND;
        break;
    }

    return ret;
}

// JScriptValue, IScriptOutParam-----------------------
STDMETHODIMP JScriptValue::Set( VARIANT type, VARIANT value)
{
    Lock();
    m_varValue.Clear();
    HRESULT hr= VariantCopyInd( &m_varValue, &value);
    VARIANT var;
    VariantInit( &var);
    if( SUCCEEDED( hr= VariantChangeType( &var, &type, 0, VT_BSTR)))
        m_bstrType= var.bstrVal;
    Unlock();
    return hr;
}
// JScriptValue, IScriptOutParam-----------------------
STDMETHODIMP JScriptValue::Get( VARIANT *val)
{
    Lock();
    if( !val)
        return E_POINTER;
    HRESULT hr= VariantCopy( val, &m_varValue);
    Unlock();
    return hr;
}

STDMETHODIMP JScriptValue::InitOutParam()
{
    Lock();
    m_varValue.Clear();
    m_bOutParam= true;
    m_bInOutParam= false;
    Unlock();
    return S_OK;
}

STDMETHODIMP JScriptValue::InitInOutParam( VARIANT type, VARIANT value)
{
    Lock();
    m_bInOutParam= true;
    m_bOutParam= false;
    Unlock();
    return Set( type, value);
}

STDMETHODIMP JScriptValue::IsOutParam( VARIANT_BOOL * flag)
{
    Lock();
    if( !flag)
        return E_POINTER;
    *flag= m_bOutParam ? VARIANT_TRUE : VARIANT_FALSE;
    Unlock();
    return S_OK;
}

STDMETHODIMP JScriptValue::IsInOutParam( VARIANT_BOOL * flag)
{
    Lock();
    if( !flag)
        return E_POINTER;
    *flag= m_bInOutParam ? VARIANT_TRUE : VARIANT_FALSE;
    Unlock();
    return S_OK;
}

STDMETHODIMP JScriptValue::GetValue( BSTR* type, VARIANT *value)
{
    Lock();
    if( !type || !value)
        return E_POINTER;
    HRESULT hr;
    if( SUCCEEDED(  hr= m_bstrType.CopyTo( type)))
        hr= VariantCopy( value, &m_varValue);
    Unlock();
    return hr;
}


// JScriptOutValue


JScriptOutParam::JScriptOutParam()
{
}

JScriptOutParam::~JScriptOutParam()
{
}


// JScriptOutParam, IDispatch --------------------------------------------
STDMETHODIMP JScriptOutParam::GetTypeInfoCount(UINT* /*pctinfo*/)
{
    return E_NOTIMPL;
}

// JScriptOutParam, IDispatch --------------------------------------------
STDMETHODIMP JScriptOutParam::GetTypeInfo( UINT /*iTInfo*/,
                                           LCID /*lcid*/,
                                           ITypeInfo** /*ppTInfo*/)
{
    return E_NOTIMPL;
}

// JScriptOutParam, IDispatch --------------------------------------------
STDMETHODIMP JScriptOutParam::GetIDsOfNames( REFIID /*riid*/,
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

    if( name == CComBSTR( L"0") )
        *rgDispId= 1;
    else
        ret= DISP_E_UNKNOWNNAME;

    return ret;
}

// JScriptOutParam, IDispatch --------------------------------------------
STDMETHODIMP JScriptOutParam::Invoke( DISPID dispIdMember,
                         REFIID /*riid*/,
                         LCID /*lcid*/,
                         WORD wFlags,
                         DISPPARAMS *pDispParams,
                         VARIANT *pVarResult,
                         EXCEPINFO* /*pExcepInfo*/,
                         UINT* /*puArgErr*/)
{
    HRESULT ret= S_OK;
    switch( dispIdMember)
    {
    case 0: // DISPID_VALUE
        if( wFlags & DISPATCH_PROPERTYGET && pVarResult)
        {
            if( FAILED( VariantCopy( pVarResult, &m_varValue)))
                ret= E_FAIL;
        }
        else if( wFlags & DISPATCH_PROPERTYPUT || wFlags & DISPATCH_PROPERTYPUTREF)
        {
            m_varValue.Clear();
            if( FAILED( VariantCopyInd( &m_varValue, &pDispParams->rgvarg[0])))
                ret= E_FAIL;
        }
        else
            ret= E_POINTER;
        break;
    case 1:
        if( wFlags & DISPATCH_PROPERTYGET && pVarResult)
        {
            if( FAILED( VariantCopy( pVarResult, &m_varValue)))
                ret= E_FAIL;
        }
        else if( wFlags & DISPATCH_PROPERTYPUT || wFlags & DISPATCH_PROPERTYPUTREF)
        {
            m_varValue.Clear();
            if( FAILED( VariantCopyInd( &m_varValue, &pDispParams->rgvarg[0])))
                ret= E_FAIL;
        }
        else
            ret= E_POINTER;
        break;

    default:
        ret= DISP_E_MEMBERNOTFOUND;
        break;
    }

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
