/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "global.hxx"
#include "propertyhdl.hxx"
#include "fileextensions.hxx"
#include "metainforeader.hxx"
#include "utilities.hxx"
#include "config.hxx"

#include <propkey.h>
#include <propvarutil.h>
#include <sal/macros.h>

#include <malloc.h>
#include <strsafe.h>

#include "stream_helper.hxx"


// Module global

long g_DllRefCnt = 0;
static HINSTANCE g_hModule = nullptr;

// Map of property keys to the locations of their value(s) in the .??? XML schema
struct PROPERTYMAP
{
    PROPERTYKEY key;
    PCWSTR pszXPathParent;
    PCWSTR pszValueNodeName;
};

const PROPERTYMAP g_rgPROPERTYMAP[] =
{
    { PKEY_Title,          L"LibreOffice",          L"Title" },
    { PKEY_Author,         L"LibreOffice",          L"Author" },
    { PKEY_Subject,        L"LibreOffice",          L"Subject" },
    { PKEY_Keywords,       L"LibreOffice",          L"Keyword" },
    { PKEY_Comment,        L"LibreOffice",          L"Comments" },
};

size_t const gPropertyMapTableSize = SAL_N_ELEMENTS(g_rgPROPERTYMAP);


CPropertyHdl::CPropertyHdl( long nRefCnt ) :
    m_RefCnt( nRefCnt ),
    m_pCache( nullptr )
{
    OutputDebugStringFormatA( "CPropertyHdl: CTOR\n" );
    InterlockedIncrement( &g_DllRefCnt );
}


CPropertyHdl::~CPropertyHdl()
{
    if ( m_pCache )
    {
        m_pCache->Release();
        m_pCache = nullptr;
    }
    InterlockedDecrement( &g_DllRefCnt );
}


// IUnknown methods

HRESULT STDMETHODCALLTYPE CPropertyHdl::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = nullptr;

    if (IID_IUnknown == riid || IID_IPropertyStore == riid)
    {
        OutputDebugStringFormatA( "CPropertyHdl: QueryInterface (IID_IPropertyStore)\n" );
        IUnknown* pUnk = static_cast<IPropertyStore*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }
    else if (IID_IPropertyStoreCapabilities == riid)
    {
        OutputDebugStringFormatA( "CPropertyHdl: QueryInterface (IID_IPropertyStoreCapabilities)\n" );
        IUnknown* pUnk = static_cast<IPropertyStore*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }
    else if (IID_IInitializeWithStream == riid)
    {
        OutputDebugStringFormatA( "CPropertyHdl: QueryInterface (IID_IInitializeWithStream)\n" );
        IUnknown* pUnk = static_cast<IInitializeWithStream*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }
    OutputDebugStringFormatA( "CPropertyHdl: QueryInterface (something different)\n" );

    return E_NOINTERFACE;
}


ULONG STDMETHODCALLTYPE CPropertyHdl::AddRef()
{
    return InterlockedIncrement( &m_RefCnt );
}


ULONG STDMETHODCALLTYPE CPropertyHdl::Release()
{
    long refcnt = InterlockedDecrement( &m_RefCnt );

    if ( 0 == m_RefCnt )
        delete this;

    return refcnt;
}


// IPropertyStore

HRESULT STDMETHODCALLTYPE CPropertyHdl::GetCount( DWORD *pcProps )
{
    HRESULT hr = E_UNEXPECTED;
    if ( m_pCache && pcProps )
    {
        hr = m_pCache->GetCount( pcProps );
    }

    return hr;
}


HRESULT STDMETHODCALLTYPE CPropertyHdl::GetAt( DWORD iProp, PROPERTYKEY *pKey )
{
    HRESULT hr = E_UNEXPECTED;
    if ( m_pCache )
    {
        hr = m_pCache->GetAt( iProp, pKey );
    }

    return hr;
}


HRESULT STDMETHODCALLTYPE CPropertyHdl::GetValue( REFPROPERTYKEY key, PROPVARIANT *pPropVar )
{
    HRESULT hr = E_UNEXPECTED;
    if ( m_pCache )
    {
        hr = m_pCache->GetValue( key, pPropVar );
    }

    return hr;
}


HRESULT STDMETHODCALLTYPE
CPropertyHdl::SetValue(REFPROPERTYKEY /*key*/, REFPROPVARIANT /*propVar*/)
{
    HRESULT hr = E_UNEXPECTED;
    if ( m_pCache )
    {
        hr = STG_E_ACCESSDENIED;
    }
    return hr;
}


HRESULT STDMETHODCALLTYPE CPropertyHdl::Commit()
{
    return S_OK;
}


// IPropertyStore

HRESULT STDMETHODCALLTYPE
CPropertyHdl::IsPropertyWritable(REFPROPERTYKEY /*key*/)
{
    // We start with read only properties only
    return S_FALSE;
}


// IInitializeWithStream

HRESULT STDMETHODCALLTYPE CPropertyHdl::Initialize( IStream *pStream, DWORD grfMode )
{
    if ( grfMode & STGM_READWRITE )
        return STG_E_ACCESSDENIED;

    if ( !m_pCache )
    {
        if ( FAILED( PSCreateMemoryPropertyStore( IID_PPV_ARGS( &m_pCache ) ) ) )
            OutputDebugStringFormatA( "CPropertyHdl::Initialize: PSCreateMemoryPropertyStore failed" );

        BufferStream tmpStream(pStream);

        CMetaInfoReader *pMetaInfoReader = nullptr;

        try
        {
            pMetaInfoReader = new CMetaInfoReader( &tmpStream );
            LoadProperties( pMetaInfoReader );
            delete pMetaInfoReader;
        }
        catch (const std::exception& e)
        {
            OutputDebugStringFormatA( "CPropertyHdl::Initialize: Caught exception [%s]", e.what() );
            return E_FAIL;
        }
    }

    return S_OK;
}

namespace {

HRESULT GetItemData( CMetaInfoReader *pMetaInfoReader, UINT nIndex, PROPVARIANT *pVarData )
{
    switch (nIndex) {
    case 0: {
            pVarData->vt = VT_BSTR;
            pVarData->bstrVal = SysAllocString( pMetaInfoReader->getTagData( META_INFO_TITLE ).c_str() );
            OutputDebugStringFormatA( "CPropertyHdl::GetItemData: Title=%S.\n", pMetaInfoReader->getTagData( META_INFO_TITLE ).c_str() );
            return S_OK;
    }
    case 1: {
            pVarData->vt = VT_BSTR;
            pVarData->bstrVal = SysAllocString( pMetaInfoReader->getTagData( META_INFO_AUTHOR ).c_str() );
            OutputDebugStringFormatA( "CPropertyHdl::GetItemData: Author=%S.\n", pMetaInfoReader->getTagData( META_INFO_AUTHOR ).c_str() );
            return S_OK;
    }
    case 2: {
            pVarData->vt = VT_BSTR;
            pVarData->bstrVal = SysAllocString( pMetaInfoReader->getTagData( META_INFO_SUBJECT ).c_str() );
            OutputDebugStringFormatA( "CPropertyHdl::GetItemData: Subject=%S.\n", pMetaInfoReader->getTagData( META_INFO_SUBJECT ).c_str() );
            return S_OK;
    }
    case 3: {
            pVarData->vt = VT_BSTR;
            pVarData->bstrVal = SysAllocString( pMetaInfoReader->getTagData( META_INFO_KEYWORDS ).c_str() );
            OutputDebugStringFormatA( "CPropertyHdl::GetItemData: Keywords=%S.\n", pMetaInfoReader->getTagData( META_INFO_KEYWORDS ).c_str() );
            return S_OK;
    }
    case 4: {
            pVarData->vt = VT_BSTR;
            pVarData->bstrVal = SysAllocString( pMetaInfoReader->getTagData( META_INFO_DESCRIPTION ).c_str() );
            OutputDebugStringFormatA( "CPropertyHdl::GetItemData: Description=%S.\n", pMetaInfoReader->getTagData( META_INFO_DESCRIPTION ).c_str() );
            return S_OK;
    }
    case 5: {
            pVarData->vt = VT_BSTR;
            pVarData->bstrVal = SysAllocString( pMetaInfoReader->getTagAttribute( META_INFO_DOCUMENT_STATISTIC, META_INFO_PAGES ).c_str() );
            OutputDebugStringFormatA( "CPropertyHdl::GetItemData: Pages=%S.\n", pMetaInfoReader->getTagAttribute( META_INFO_DOCUMENT_STATISTIC, META_INFO_PAGES ).c_str() );
            return S_OK;
    }
    }

    return S_FALSE;
}

}

void CPropertyHdl::LoadProperties( CMetaInfoReader *pMetaInfoReader )
{
    OutputDebugStringFormatA( "CPropertyHdl: LoadProperties\n" );
    PROPVARIANT propvarValues;

    for ( UINT i = 0; i < (UINT)gPropertyMapTableSize; ++i )
    {
        PropVariantClear( &propvarValues );
        HRESULT hr = GetItemData( pMetaInfoReader, i, &propvarValues);
        if (hr == S_OK)
        {
            // coerce the value(s) to the appropriate type for the property key
            hr = PSCoerceToCanonicalValue( g_rgPROPERTYMAP[i].key, &propvarValues );
            if (SUCCEEDED(hr))
            {
                // cache the value(s) loaded
                hr = m_pCache->SetValueAndState( g_rgPROPERTYMAP[i].key, &propvarValues, PSC_NORMAL );
            }
        }
    }
}

//                              CClassFactory


long CClassFactory::s_ServerLocks = 0;


CClassFactory::CClassFactory( const CLSID& clsid ) :
    m_RefCnt(1),
    m_Clsid(clsid)
{
    InterlockedIncrement( &g_DllRefCnt );
}


CClassFactory::~CClassFactory()
{
    InterlockedDecrement( &g_DllRefCnt );
}


//                              IUnknown methods

HRESULT STDMETHODCALLTYPE CClassFactory::QueryInterface( REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject )
{
    *ppvObject = nullptr;

    if ( IID_IUnknown == riid || IID_IClassFactory == riid )
    {
        IUnknown* pUnk = this;
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }

    return E_NOINTERFACE;
}


ULONG STDMETHODCALLTYPE CClassFactory::AddRef()
{
    return InterlockedIncrement( &m_RefCnt );
}


ULONG STDMETHODCALLTYPE CClassFactory::Release()
{
    long refcnt = InterlockedDecrement( &m_RefCnt );

    if (0 == refcnt)
        delete this;

    return refcnt;
}


//                          IClassFactory methods

HRESULT STDMETHODCALLTYPE CClassFactory::CreateInstance(
            IUnknown __RPC_FAR *pUnkOuter,
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if ( pUnkOuter != nullptr )
        return CLASS_E_NOAGGREGATION;

    IUnknown* pUnk = nullptr;

    if ( CLSID_PROPERTY_HANDLER == m_Clsid )
        pUnk = static_cast<IPropertyStore*>( new CPropertyHdl() );

    if (nullptr == pUnk)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pUnk->QueryInterface( riid, ppvObject );

    // if QueryInterface failed the component will destroy itself
    pUnk->Release();

    return hr;
}


HRESULT STDMETHODCALLTYPE CClassFactory::LockServer( BOOL fLock )
{
    if ( fLock )
        InterlockedIncrement( &s_ServerLocks );
    else
        InterlockedDecrement( &s_ServerLocks );

    return S_OK;
}


bool CClassFactory::IsLocked()
{
    return ( s_ServerLocks > 0 );
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    OutputDebugStringFormatA( "DllGetClassObject.\n" );
    *ppv = nullptr;

    if ( rclsid != CLSID_PROPERTY_HANDLER )
        return CLASS_E_CLASSNOTAVAILABLE;

    if ( (riid != IID_IUnknown) && (riid != IID_IClassFactory) )
        return E_NOINTERFACE;

    IUnknown* pUnk = new CClassFactory( rclsid );
    *ppv = pUnk;
    return S_OK;
}


STDAPI DllCanUnloadNow()
{
    OutputDebugStringFormatA( "DllCanUnloadNow.\n" );
    if (CClassFactory::IsLocked() || g_DllRefCnt > 0)
        return S_FALSE;

    return S_OK;
}


BOOL WINAPI DllMain( HINSTANCE hInst, ULONG /*ul_reason_for_call*/, LPVOID /*lpReserved*/ )
{
    OutputDebugStringFormatA( "DllMain.\n" );
    g_hModule = hInst;
    return TRUE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
