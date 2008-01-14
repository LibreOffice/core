/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unotypewrapper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:48:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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



