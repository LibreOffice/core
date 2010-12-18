/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <stdio.h>
#include <inprocembobj.h>
#ifdef __MINGW32__
#define INITGUID
#endif
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

static HINSTANCE g_hInstance = NULL;
static ULONG g_nObj = 0;
static ULONG g_nLock = 0;


namespace {
    void FillCharFromInt( int nValue, char* pBuf, int nLen )
    {
        int nInd = 0;
        while( nInd < nLen )
        {
            char nSign = ( nValue / ( 1 << ( ( nLen - nInd ) * 4 ) ) ) % 16;
            if ( nSign >= 0 && nSign <= 9 )
                pBuf[nInd] = nSign + '0';
            else if ( nSign >= 10 && nSign <= 15 )
                pBuf[nInd] = nSign - 10 + 'a';

            nInd++;
        }
    }

    int GetStringFromClassID( const GUID& guid, char* pBuf, int nLen )
    {
        // is not allowed to insert
        if ( nLen < 38 )
            return 0;

        pBuf[0] = '{';
        FillCharFromInt( guid.Data1, &pBuf[1], 8 );
        pBuf[9] = '-';
        FillCharFromInt( guid.Data2, &pBuf[10], 4 );
        pBuf[14] = '-';
        FillCharFromInt( guid.Data3, &pBuf[15], 4 );
        pBuf[19] = '-';

        int nInd = 0;
        for ( nInd = 0; nInd < 2 ; nInd++ )
            FillCharFromInt( guid.Data4[nInd], &pBuf[20 + 2*nInd], 2 );
        pBuf[24] = '-';
        for ( nInd = 2; nInd < 8 ; nInd++ )
            FillCharFromInt( guid.Data4[nInd], &pBuf[20 + 1 + 2*nInd], 2 );
        pBuf[37] = '}';

        return 38;
    }

    HRESULT WriteLibraryToRegistry( char* pLibrary, DWORD nLen )
    {
        HRESULT hRes = E_FAIL;
        if ( pLibrary && nLen )
        {
            HKEY hKey = NULL;

            hRes = S_OK;
            for ( int nInd = 0; nInd < SUPPORTED_FACTORIES_NUM; nInd++ )
            {
                char* pSubKey = "Software\\Classes\\CLSID\\.....................................\\InprocHandler32";

                int nGuidLen = GetStringFromClassID( *guidList[nInd], &pSubKey[23], 38 );

                BOOL bLocalSuccess = FALSE;
                if ( nGuidLen && nGuidLen == 38 )
                {
                    if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE, pSubKey, &hKey ) )
                    {
                        if ( ERROR_SUCCESS == RegSetValueEx( hKey, "", 0, REG_SZ, (const BYTE*)pLibrary, nLen ) )
                            bLocalSuccess = TRUE;
                    }

                    if ( hKey )
                    {
                        RegCloseKey( hKey );
                        hKey = NULL;
                    }
                }

                if ( !bLocalSuccess )
                    hRes = E_FAIL;
            }
        }

        return hRes;
    }
};

// ===========================
// InprocEmbedProvider_Impl declaration
// ===========================

namespace inprocserv
{

class InprocEmbedProvider_Impl : public IClassFactory, public InprocCountedObject_Impl
{
public:

    InprocEmbedProvider_Impl( const GUID& guid );
    virtual ~InprocEmbedProvider_Impl();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    /* IClassFactory methods */
    STDMETHOD(CreateInstance)(IUnknown FAR* punkOuter, REFIID riid, void FAR* FAR* ppv);
    STDMETHOD(LockServer)(int fLock);

protected:

    ULONG               m_refCount;
    GUID                m_guid;
};
}; // namespace inprocserv


// ===========================
// Entry points
// ===========================

// -------------------------------------------------------------------------------
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

// -------------------------------------------------------------------------------
extern "C" STDAPI DllGetClassObject( REFCLSID rclsid, REFIID riid, LPVOID* ppv )
{
    for( int nInd = 0; nInd < SUPPORTED_FACTORIES_NUM; nInd++ )
         if ( *guidList[nInd] == rclsid )
         {
            if ( !IsEqualIID( riid, IID_IUnknown ) && !IsEqualIID( riid, IID_IClassFactory ) )
                return E_NOINTERFACE;

            *ppv = new inprocserv::InprocEmbedProvider_Impl( rclsid );
            if ( *ppv == NULL )
                return E_OUTOFMEMORY;

            ((LPUNKNOWN)*ppv)->AddRef();
            return S_OK;
         }

    return E_FAIL;
}

// -------------------------------------------------------------------------------
extern "C" STDAPI DllCanUnloadNow()
{
    if ( !g_nObj && !g_nLock )
        return S_OK;

    return S_FALSE;
}

// -------------------------------------------------------------------------------
STDAPI DllRegisterServer( void )
{
    char aLibPath[1024];
    HMODULE aCurModule = GetModuleHandleA( "inprocserv.dll" );
    if( aCurModule )
    {
        DWORD nLen = GetModuleFileNameA( aCurModule, aLibPath, 1019 );
        if ( nLen && nLen < 1019 )
        {
            aLibPath[nLen++] = 0;
            return WriteLibraryToRegistry( aLibPath, nLen );
        }
    }

    return E_FAIL;
}

// -------------------------------------------------------------------------------
STDAPI DllUnregisterServer( void )
{
    return WriteLibraryToRegistry( "ole32.dll", 10 );
}

// ===========================
// End of entry points
// ===========================

namespace inprocserv
{

// ===========================
// InprocCountedObject_Impl implementation
// ===========================

// -------------------------------------------------------------------------------
InprocCountedObject_Impl::InprocCountedObject_Impl()
{
    g_nObj++;
}

// -------------------------------------------------------------------------------
InprocCountedObject_Impl::~InprocCountedObject_Impl()
{
    g_nObj--;
}

// ===========================
// InprocEmbedProvider_Impl implementation
// ===========================

// -------------------------------------------------------------------------------
InprocEmbedProvider_Impl::InprocEmbedProvider_Impl( const GUID& guid )
: m_refCount( 0 )
, m_guid( guid )
{
}

// -------------------------------------------------------------------------------
InprocEmbedProvider_Impl::~InprocEmbedProvider_Impl()
{
}

// IUnknown
// -------------------------------------------------------------------------------
STDMETHODIMP InprocEmbedProvider_Impl::QueryInterface( REFIID riid, void FAR* FAR* ppv )
{
    if(IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppv = (IUnknown*) this;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IClassFactory))
    {
        AddRef();
        *ppv = (IClassFactory*) this;
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
}

// -------------------------------------------------------------------------------
STDMETHODIMP_(ULONG) InprocEmbedProvider_Impl::AddRef()
{
    return ++m_refCount;
}

// -------------------------------------------------------------------------------
STDMETHODIMP_(ULONG) InprocEmbedProvider_Impl::Release()
{
    sal_Int32 nCount = --m_refCount;
    if ( nCount == 0 )
        delete this;
    return nCount;
}

// -------------------------------------------------------------------------------
STDMETHODIMP InprocEmbedProvider_Impl::CreateInstance(IUnknown FAR* punkOuter,
                                                       REFIID riid,
                                                       void FAR* FAR* ppv)
{
    // TODO/LATER: should the aggregation be supported?
    // if ( punkOuter != NULL && riid != IID_IUnknown )
    //     return E_NOINTERFACE;
    if ( punkOuter != NULL )
        return CLASS_E_NOAGGREGATION;

    InprocEmbedDocument_Impl* pEmbedDocument = new InprocEmbedDocument_Impl( m_guid );
    if ( !pEmbedDocument )
        return E_OUTOFMEMORY;

    pEmbedDocument->AddRef();
    HRESULT hr = pEmbedDocument->Init();
    if ( SUCCEEDED( hr ) )
        hr = pEmbedDocument->QueryInterface( riid, ppv );
    pEmbedDocument->Release();

    if ( !SUCCEEDED( hr ) )
        *ppv = NULL;

    return hr;
}

// -------------------------------------------------------------------------------
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
