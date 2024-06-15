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

#include <sal/types.h>

#include <cassert>
#include <stdio.h>
#include <string_view>
#include "inprocembobj.h"
#include <embservconst.h>

#include <olectl.h> // declarations of DllRegisterServer/DllUnregisterServer

static const GUID* guidList[ SUPPORTED_FACTORIES_NUM ] = {
    &OID_WriterTextServer,
    &OID_WriterOASISTextServer,
    &OID_CalcServer,
    &OID_CalcOASISServer,
    &OID_DrawingServer,
    &OID_DrawingOASISServer,
    &OID_PresentationServer,
    &OID_PresentationOASISServer,
    &OID_MathServer,
    &OID_MathOASISServer
};

static HINSTANCE g_hInstance = nullptr;
static ULONG g_nObj = 0;
static ULONG g_nLock = 0;


namespace {
    HRESULT WriteLibraryToRegistry( const wchar_t* pLibrary, DWORD nLen )
    {
        HRESULT hRes = E_FAIL;
        if ( pLibrary && nLen )
        {
            hRes = S_OK;
            for ( int nInd = 0; nInd < SUPPORTED_FACTORIES_NUM; nInd++ )
            {
                constexpr std::wstring_view prefix(L"Software\\Classes\\CLSID\\");
                constexpr std::wstring_view suffix(L"\\InprocHandler32");
                constexpr size_t guidStringSize
                    = std::size(L"{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}");
                constexpr size_t bufsize = prefix.size() + guidStringSize + suffix.size();
                wchar_t pSubKey[bufsize];
                wchar_t *pos = pSubKey, *end = pSubKey + std::size(pSubKey);
                pos += prefix.copy(pos, prefix.size());
                int nGuidLen = StringFromGUID2(*guidList[nInd], pos, end - pos);

                bool bLocalSuccess = false;
                if (nGuidLen == guidStringSize)
                {
                    pos += nGuidLen - 1;
                    pos += suffix.copy(pos, suffix.size());
                    assert(pos == end - 1);
                    *pos = 0;
                    if (ERROR_SUCCESS == RegSetKeyValueW(HKEY_LOCAL_MACHINE, pSubKey, nullptr, REG_SZ, pLibrary, nLen*sizeof(wchar_t)))
                        bLocalSuccess = true;
                }

                if ( !bLocalSuccess )
                    hRes = E_FAIL;
            }
        }

        return hRes;
    }
};


// InprocEmbedProvider_Impl declaration


namespace inprocserv
{

namespace {

class InprocEmbedProvider_Impl : public IClassFactory, public InprocCountedObject_Impl
{
public:

    explicit InprocEmbedProvider_Impl( const GUID& guid );
    virtual ~InprocEmbedProvider_Impl();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

    /* IClassFactory methods */
    STDMETHOD(CreateInstance)(IUnknown* punkOuter, REFIID riid, void** ppv) override;
    STDMETHOD(LockServer)(BOOL fLock) override;

protected:

    ULONG               m_refCount;
    GUID                m_guid;
};

}

}; // namespace inprocserv


// Entry points


extern "C" BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hInstance;
    }

    return TRUE;    // ok
}


STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID* ppv )
{
    for( int nInd = 0; nInd < SUPPORTED_FACTORIES_NUM; nInd++ )
         if ( *guidList[nInd] == rclsid )
         {
            if ( !IsEqualIID( riid, IID_IUnknown ) && !IsEqualIID( riid, IID_IClassFactory ) )
                return E_NOINTERFACE;

            *ppv = new inprocserv::InprocEmbedProvider_Impl( rclsid );
            static_cast<LPUNKNOWN>(*ppv)->AddRef();
            return S_OK;
         }

    return E_FAIL;
}


STDAPI DllCanUnloadNow()
{
    if ( !g_nObj && !g_nLock )
        return S_OK;

    return S_FALSE;
}


STDAPI DllRegisterServer()
{
    HMODULE aCurModule{};
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                           | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       reinterpret_cast<LPCWSTR>(&DllRegisterServer), &aCurModule);
    if( aCurModule )
    {
        wchar_t aLibPath[1024];
        DWORD nLen = GetModuleFileNameW( aCurModule, aLibPath, 1019 );
        if ( nLen && nLen < 1019 )
        {
            aLibPath[nLen++] = 0;
            return WriteLibraryToRegistry( aLibPath, nLen );
        }
    }

    return E_FAIL;
}


STDAPI DllUnregisterServer()
{
    return WriteLibraryToRegistry( L"ole32.dll", 10 );
}


// End of entry points


namespace inprocserv
{


// InprocCountedObject_Impl implementation


InprocCountedObject_Impl::InprocCountedObject_Impl()
{
    g_nObj++;
}


InprocCountedObject_Impl::~InprocCountedObject_Impl()
{
    g_nObj--;
}


// InprocEmbedProvider_Impl implementation


InprocEmbedProvider_Impl::InprocEmbedProvider_Impl( const GUID& guid )
: m_refCount( 0 )
, m_guid( guid )
{
}


InprocEmbedProvider_Impl::~InprocEmbedProvider_Impl()
{
}

// IUnknown

COM_DECLSPEC_NOTHROW STDMETHODIMP InprocEmbedProvider_Impl::QueryInterface( REFIID riid, void ** ppv )
{
    if(IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppv = static_cast<IUnknown*>(this);
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IClassFactory))
    {
        AddRef();
        *ppv = static_cast<IClassFactory*>(this);
        return S_OK;
    }

    *ppv = nullptr;
    return E_NOINTERFACE;
}


COM_DECLSPEC_NOTHROW STDMETHODIMP_(ULONG) InprocEmbedProvider_Impl::AddRef()
{
    return ++m_refCount;
}


COM_DECLSPEC_NOTHROW STDMETHODIMP_(ULONG) InprocEmbedProvider_Impl::Release()
{
    sal_Int32 nCount = --m_refCount;
    if ( nCount == 0 )
        delete this;
    return nCount;
}


COM_DECLSPEC_NOTHROW STDMETHODIMP InprocEmbedProvider_Impl::CreateInstance(IUnknown* punkOuter,
                                                                           REFIID riid, void** ppv)
{
    // TODO/LATER: should the aggregation be supported?
    // if ( punkOuter != NULL && riid != IID_IUnknown )
    //     return E_NOINTERFACE;
    if ( punkOuter != nullptr )
        return CLASS_E_NOAGGREGATION;

    InprocEmbedDocument_Impl* pEmbedDocument = new InprocEmbedDocument_Impl( m_guid );
    pEmbedDocument->AddRef();
    HRESULT hr = pEmbedDocument->QueryInterface( riid, ppv );
    pEmbedDocument->Release();

    if ( !SUCCEEDED( hr ) )
        *ppv = nullptr;

    return hr;
}


COM_DECLSPEC_NOTHROW STDMETHODIMP InprocEmbedProvider_Impl::LockServer( BOOL fLock )
{
    if ( fLock )
        g_nLock++;
    else
        g_nLock--;

    return S_OK;
}

}; // namespace inprocserv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
