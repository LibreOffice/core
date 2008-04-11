/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: classfactory.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_shell.hxx"
#include "internal/global.hxx"
#include "classfactory.hxx"
#include "internal/infotips.hxx"
#include "internal/propsheets.hxx"
#include "internal/columninfo.hxx"
#include "internal/thumbviewer.hxx"
#include "internal/shlxthdl.hxx"

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
