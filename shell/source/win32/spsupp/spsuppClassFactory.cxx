/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <spsuppClassFactory.hpp>
#include <COMOpenDocuments.hpp>

LONG ClassFactory::m_nObjCount = 0;
LONG ClassFactory::m_nLockCount = 0;

ClassFactory::ClassFactory()
{
    ::InterlockedIncrement(&m_nObjCount);
};

ClassFactory::~ClassFactory()
{
    ::InterlockedDecrement(&m_nObjCount);
};

// IUnknown methods

STDMETHODIMP ClassFactory::QueryInterface(
    REFIID riid,
    void **ppvObject)
{
    *ppvObject = nullptr;
    if (IsEqualIID(riid, __uuidof(IUnknown)) ||
        IsEqualIID(riid, __uuidof(IClassFactory)))
    {
        *ppvObject = static_cast<IClassFactory*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

// IClassFactory methods

STDMETHODIMP ClassFactory::CreateInstance(
    IUnknown *pUnkOuter,
    REFIID riid,
    void **ppvObject)
{
    *ppvObject = nullptr;
    if (pUnkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    COMOpenDocuments* pObj;
    try {
        pObj = new COMOpenDocuments;
    }
    catch (const COMOpenDocuments::Error& e) {
        return e.val();
    }
    catch (...) {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pObj->QueryInterface(riid, ppvObject);
    pObj->Release();
    return hr;
}

STDMETHODIMP ClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        ::InterlockedIncrement(&m_nLockCount);
    else
        ::InterlockedDecrement(&m_nLockCount);
    return S_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
