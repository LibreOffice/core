/*************************************************************************
 *
 *  $RCSfile: jscriptclasses.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:40 $
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
STDMETHODIMP JScriptValue::GetTypeInfoCount(UINT *pctinfo)
{
    return E_NOTIMPL;
}

// JScriptValue, IDispatch --------------------------------------------
STDMETHODIMP JScriptValue::GetTypeInfo( UINT iTInfo,
                                           LCID lcid,
                                           ITypeInfo **ppTInfo)
{
    return E_NOTIMPL;
}

// JScriptValue, IDispatch --------------------------------------------
STDMETHODIMP JScriptValue::GetIDsOfNames( REFIID riid,
                                             LPOLESTR *rgszNames,
                                             UINT cNames,
                                             LCID lcid,
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
                         REFIID riid,
                         LCID lcid,
                         WORD wFlags,
                         DISPPARAMS *pDispParams,
                         VARIANT *pVarResult,
                         EXCEPINFO *pExcepInfo,
                         UINT *puArgErr)
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
    m_varValue= value;
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
STDMETHODIMP JScriptOutParam::GetTypeInfoCount(UINT *pctinfo)
{
    return E_NOTIMPL;
}

// JScriptOutParam, IDispatch --------------------------------------------
STDMETHODIMP JScriptOutParam::GetTypeInfo( UINT iTInfo,
                                           LCID lcid,
                                           ITypeInfo **ppTInfo)
{
    return E_NOTIMPL;
}

// JScriptOutParam, IDispatch --------------------------------------------
STDMETHODIMP JScriptOutParam::GetIDsOfNames( REFIID riid,
                                             LPOLESTR *rgszNames,
                                             UINT cNames,
                                             LCID lcid,
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
                         REFIID riid,
                         LCID lcid,
                         WORD wFlags,
                         DISPPARAMS *pDispParams,
                         VARIANT *pVarResult,
                         EXCEPINFO *pExcepInfo,
                         UINT *puArgErr)
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
