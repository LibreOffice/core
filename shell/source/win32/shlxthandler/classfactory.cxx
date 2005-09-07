/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: classfactory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:58:07 $
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

#ifndef GLOBAL_HXX_INCLUDED
#include "internal/global.hxx"
#endif

#ifndef CLASSFACTORY_HXX_INCLUDED
#include "classfactory.hxx"
#endif

#ifndef INFOTIPS_HXX_INCLUDED
#include "internal/infotips.hxx"
#endif

#ifndef PROPSHEETS_HXX_INCLUDED
#include "internal/propsheets.hxx"
#endif

#ifndef COLUMNINFO_HXX_INCLUDED
#include "internal/columninfo.hxx"
#endif

#ifndef THUMBVIEWER_HXX_INCLUDED
#include "internal/thumbviewer.hxx"
#endif

#ifndef SHLXTHDL_HXX_INCLUDED
#include "internal/shlxthdl.hxx"
#endif

//-----------------------------
//
//-----------------------------

long CClassFactory::s_ServerLocks = 0;

//-----------------------------
//
//-----------------------------

CClassFactory::CClassFactory(const CLSID& clsid) :
    m_RefCnt(1),
    m_Clsid(clsid)
{
    InterlockedIncrement(&g_DllRefCnt);
}

//-----------------------------
//
//-----------------------------

CClassFactory::~CClassFactory()
{
    InterlockedDecrement(&g_DllRefCnt);
}

//-----------------------------
// IUnknown methods
//-----------------------------

HRESULT STDMETHODCALLTYPE CClassFactory::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = 0;

    if (IID_IUnknown == riid || IID_IClassFactory == riid)
    {
        IUnknown* pUnk = this;
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }

    return E_NOINTERFACE;
}

//-----------------------------
//
//-----------------------------

ULONG STDMETHODCALLTYPE CClassFactory::AddRef(void)
{
    return InterlockedIncrement(&m_RefCnt);
}

//-----------------------------
//
//-----------------------------

ULONG STDMETHODCALLTYPE CClassFactory::Release(void)
{
    long refcnt = InterlockedDecrement(&m_RefCnt);

    if (0 == refcnt)
        delete this;

    return refcnt;
}

//-----------------------------
// IClassFactory methods
//-----------------------------

HRESULT STDMETHODCALLTYPE CClassFactory::CreateInstance(
            IUnknown __RPC_FAR *pUnkOuter,
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if ((pUnkOuter != NULL))
        return CLASS_E_NOAGGREGATION;

    IUnknown* pUnk = 0;

    if (CLSID_PROPERTYSHEET_HANDLER == m_Clsid)
        pUnk = static_cast<IShellExtInit*>(new CPropertySheet());

    else if (CLSID_INFOTIP_HANDLER == m_Clsid)
        pUnk = static_cast<IQueryInfo*>(new CInfoTip());

    else if (CLSID_COLUMN_HANDLER == m_Clsid)
        pUnk = static_cast<IColumnProvider*>(new CColumnInfo());

    else if (CLSID_THUMBVIEWER_HANDLER == m_Clsid)
        pUnk = static_cast<IExtractImage*>(new CThumbviewer());

    POST_CONDITION(pUnk != 0, "Could not create COM object");

    if (0 == pUnk)
        return E_OUTOFMEMORY;

    HRESULT hr = pUnk->QueryInterface(riid, ppvObject);

    // if QueryInterface failed the component will destroy itself
    pUnk->Release();

    return hr;
}

//-----------------------------
//
//-----------------------------

HRESULT STDMETHODCALLTYPE CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        InterlockedIncrement(&s_ServerLocks);
    else
        InterlockedDecrement(&s_ServerLocks);

    return S_OK;
}

//-----------------------------
//
//-----------------------------

bool CClassFactory::IsLocked()
{
    return (s_ServerLocks > 0);
}
