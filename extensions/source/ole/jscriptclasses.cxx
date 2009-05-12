/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: jscriptclasses.cxx,v $
 * $Revision: 1.7 $
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
#include "jscriptclasses.hxx"

//========================================================================
// JScriptValue
//========================================================================
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
    HRESULT hr= S_OK;
    m_varValue.Clear();
    hr= VariantCopyInd( &m_varValue, &value);
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

//##########################################################################################
// JScriptOutValue
//##########################################################################################

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
    case 1: //
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
