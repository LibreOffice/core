/*************************************************************************
 *
 *  $RCSfile: unotypewrapper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-02 09:47:01 $
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


bool createUnoTypeWrapper(const rtl::OUString& sTypeName, VARIANT * pVar)
{
    CComBSTR bstr(sTypeName.getStr());
    return createUnoTypeWrapper(bstr, pVar);
}

UnoTypeWrapper::UnoTypeWrapper()
{
}

UnoTypeWrapper::~UnoTypeWrapper()
{
}


// UnoTypeWrapper, IDispatch --------------------------------------------
STDMETHODIMP UnoTypeWrapper::GetTypeInfoCount(UINT *pctinfo)
{
    return E_NOTIMPL;
}

// UnoTypeWrapper, IDispatch --------------------------------------------
STDMETHODIMP UnoTypeWrapper::GetTypeInfo( UINT iTInfo,
                                          LCID lcid,
                                          ITypeInfo **ppTInfo)
{
    return E_NOTIMPL;
}

// UnoTypeWrapper, IDispatch --------------------------------------------
STDMETHODIMP UnoTypeWrapper::GetIDsOfNames( REFIID riid,
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

    if( name == CComBSTR( L"name") )
        *rgDispId= DISPID_VALUE;
    else
        ret= DISP_E_UNKNOWNNAME;

    return ret;
}

// UnoTypeWrapper, IDispatch --------------------------------------------
STDMETHODIMP UnoTypeWrapper::Invoke( DISPID dispIdMember,
                         REFIID riid,
                         LCID lcid,
                         WORD wFlags,
                         DISPPARAMS *pDispParams,
                         VARIANT *pVarResult,
                         EXCEPINFO *pExcepInfo,
                         UINT *puArgErr)
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



