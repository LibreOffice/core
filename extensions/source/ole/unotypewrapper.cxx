/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unotypewrapper.cxx,v $
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
#include "unotypewrapper.hxx"
#include "rtl/ustring.hxx"


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
#ifdef __MINGW32__
        pVar->pdispVal= CComQIPtr<IDispatch, &__uuidof(IDispatch)>(pObj->GetUnknown());
#else
        pVar->pdispVal= CComQIPtr<IDispatch>(pObj->GetUnknown());
#endif
        //now set the value, e.i. the name of the type
#ifdef __MINGW32__
        CComQIPtr<IUnoTypeWrapper, &__uuidof(IUnoTypeWrapper)> spType(pVar->pdispVal);
#else
        CComQIPtr<IUnoTypeWrapper> spType(pVar->pdispVal);
#endif
        OSL_ASSERT(spType);
        if (SUCCEEDED(spType->put_Name(sTypeName)))
        {
            ret = true;
        }
    }
    return ret;
}


bool createUnoTypeWrapper(const rtl::OUString& sTypeName, VARIANT * pVar)
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
    if (pDispParams == NULL)
        return DISP_E_EXCEPTION;

    if( pDispParams->cNamedArgs)
        return DISP_E_NONAMEDARGS;


    HRESULT ret= S_OK;
    switch( dispIdMember)
    {
    case DISPID_VALUE: // DISPID_VALUE
        if (wFlags & DISPATCH_PROPERTYGET)
        {
            if (pVarResult == NULL)
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



