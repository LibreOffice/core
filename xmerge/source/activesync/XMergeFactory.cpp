/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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

    if (pUnkOuter != NULL)  // cannot aggregate
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


