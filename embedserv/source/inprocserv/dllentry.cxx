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

#include <stdio.h>
#include "inprocembobj.h"
#include <embservconst.h>

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
    void FillCharFromInt( int nValue, wchar_t* pBuf, int nLen )
    {
        int nInd = 0;
        while( nInd < nLen )
        {
            char nSign = ( nValue / ( 1 << ( ( nLen - nInd - 1 ) * 4 ) ) ) % 16;
            if ( nSign >= 0 && nSign <= 9 )
                pBuf[nInd] = nSign + L'0';
            else if ( nSign >= 10 && nSign <= 15 )
                pBuf[nInd] = nSign - 10 + L'a';

            nInd++;
        }
    }

    int GetStringFromClassID( const GUID& guid, wchar_t* pBuf, int nLen )
    {
        // is not allowed to insert
        if ( nLen < 38 )
            return 0;

        pBuf[0] = L'{';
        FillCharFromInt( guid.Data1, &pBuf[1], 8 );
        pBuf[9] = L'-';
        FillCharFromInt( guid.Data2, &pBuf[10], 4 );
        pBuf[14] = L'-';
        FillCharFromInt( guid.Data3, &pBuf[15], 4 );
        pBuf[19] = L'-';

        int nInd = 0;
        for ( nInd = 0; nInd < 2 ; nInd++ )
            FillCharFromInt( guid.Data4[nInd], &pBuf[20 + 2*nInd], 2 );
        pBuf[24] = L'-';
        for ( nInd = 2; nInd < 8 ; nInd++ )
            FillCharFromInt( guid.Data4[nInd], &pBuf[20 + 1 + 2*nInd], 2 );
        pBuf[37] = L'}';

        return 38;
    }

    HRESULT WriteLibraryToRegistry( const wchar_t* pLibrary, DWORD nLen )
    {
        HRESULT hRes = E_FAIL;
        if ( pLibrary && nLen )
        {
            HKEY hKey = nullptr;

            hRes = S_OK;
            for ( int nInd = 0; nInd < SUPPORTED_FACTORIES_NUM; nInd++ )
            {
                const wchar_t pSubKeyTemplate[] = L"Software\\Classes\\CLSID\\.....................................\\InprocHandler32";
                wchar_t pSubKey[SAL_N_ELEMENTS(pSubKeyTemplate)];
                wcsncpy(pSubKey, pSubKeyTemplate, SAL_N_ELEMENTS(pSubKeyTemplate));

                int nGuidLen = GetStringFromClassID( *guidList[nInd], &pSubKey[23], 38 );

                BOOL bLocalSuccess = FALSE;
                if ( nGuidLen == 38 )
                {
                    if ( ERROR_SUCCESS == RegOpenKeyW( HKEY_LOCAL_MACHINE, pSubKey, &hKey ) )
                    {
                        if ( ERROR_SUCCESS == RegSetValueExW( hKey, L"", 0, REG_SZ, reinterpret_cast<const BYTE*>(pLibrary), nLen*sizeof(wchar_t) ) )
                            bLocalSuccess = TRUE;
                    }

                    if ( hKey )
                    {
                        RegCloseKey( hKey );
                        hKey = nullptr;
                    }
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

class InprocEmbedProvider_Impl : public IClassFactory, public InprocCountedObject_Impl
{
public:

    explicit InprocEmbedProvider_Impl( const GUID& guid );
    virtual ~InprocEmbedProvider_Impl();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppvObj) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

    /* IClassFactory methods */
    STDMETHOD(CreateInstance)(IUnknown FAR* punkOuter, REFIID riid, void FAR* FAR* ppv) override;
    STDMETHOD(LockServer)(int fLock) override;

protected:

    ULONG               m_refCount;
    GUID                m_guid;
};
}; // namespace inprocserv


// Entry points


extern "C" BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hInstance;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
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
    HMODULE aCurModule = GetModuleHandleW( L"inprocserv.dll" );
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

STDMETHODIMP InprocEmbedProvider_Impl::QueryInterface( REFIID riid, void FAR* FAR* ppv )
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


STDMETHODIMP_(ULONG) InprocEmbedProvider_Impl::AddRef()
{
    return ++m_refCount;
}


STDMETHODIMP_(ULONG) InprocEmbedProvider_Impl::Release()
{
    sal_Int32 nCount = --m_refCount;
    if ( nCount == 0 )
        delete this;
    return nCount;
}


STDMETHODIMP InprocEmbedProvider_Impl::CreateInstance(IUnknown FAR* punkOuter,
                                                       REFIID riid,
                                                       void FAR* FAR* ppv)
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


STDMETHODIMP InprocEmbedProvider_Impl::LockServer( int fLock )
{
    if ( fLock )
        g_nLock++;
    else
        g_nLock--;

    return S_OK;
}

}; // namespace inprocserv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
