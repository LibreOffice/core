/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// XMergeFactory.cpp: implementation of the CXMergeFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "XMergeFilter.h"
#include "XMergeFactory.h"

//////////////////////////////////////////////////////////////////////
// IUnknown implementation 
//////////////////////////////////////////////////////////////////////																		 
STDMETHODIMP CXMergeFactory::QueryInterface(REFIID riid, void **ppvObject)  
{
    if(ppvObject == NULL)
        return E_INVALIDARG;

    if(::IsEqualIID(riid, IID_IUnknown) || ::IsEqualIID(riid, IID_IClassFactory)) 
    {
        *ppvObject = static_cast<IClassFactory*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG) CXMergeFactory::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CXMergeFactory::Release() 
{
    if(::InterlockedDecrement(&m_cRef) == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}


//////////////////////////////////////////////////////////////////////
// IUnknown implementation 
//////////////////////////////////////////////////////////////////////																		 
STDMETHODIMP CXMergeFactory::CreateInstance(IUnknown *pUnkOuter, REFIID iid, void **ppvObject)
{
    if (ppvObject == NULL)
        return E_INVALIDARG;
    
    if (pUnkOuter != NULL)	// cannot aggregate
    {
        *ppvObject = NULL;
        return CLASS_E_NOAGGREGATION;
    }

    if (iid == IID_ICeFileFilter) 
    {
        CXMergeFilter *pFilter = new CXMergeFilter();
        if(pFilter == NULL)
        {
             *ppvObject = NULL;
            return E_OUTOFMEMORY;
        }

        HRESULT hr = pFilter->QueryInterface(iid, ppvObject);
        pFilter->Release();

        return hr;
    }

    return E_INVALIDARG;
}


STDMETHODIMP CXMergeFactory::LockServer(BOOL fLock)
{
    _Module.LockServer(fLock);
    return S_OK;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
