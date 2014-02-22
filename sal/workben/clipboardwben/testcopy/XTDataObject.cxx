/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <osl/diagnose.h>

#include "XTDataObject.hxx"

#include <windows.h>
#include <ole2.h>














CXTDataObject::CXTDataObject( LONG nRefCntInitVal ) :
    m_nRefCnt( nRefCntInitVal )
{
}





CXTDataObject::~CXTDataObject( )
{
}





STDMETHODIMP CXTDataObject::QueryInterface( REFIID iid, LPVOID* ppvObject )
{
    OSL_ASSERT( NULL != ppvObject );

    if ( NULL == ppvObject )
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;

    *ppvObject = NULL;

    if ( ( __uuidof( IUnknown ) == iid ) || ( __uuidof( IDataObject ) == iid ) )
    {
        *ppvObject = static_cast< IUnknown* >( this );
        ( (LPUNKNOWN)*ppvObject )->AddRef( );
        hr = S_OK;
    }

    return hr;
}





STDMETHODIMP_(ULONG) CXTDataObject::AddRef( )
{
    return static_cast< ULONG >( InterlockedIncrement( &m_nRefCnt ) );
}





STDMETHODIMP_(ULONG) CXTDataObject::Release( )
{
    
    
    
    ULONG nRefCnt = static_cast< ULONG >( InterlockedDecrement( &m_nRefCnt ) );

    if ( 0 == nRefCnt )
    {
        delete this;
    }

    return nRefCnt;
}






STDMETHODIMP CXTDataObject::GetData(LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium )
{
    OSL_ASSERT( ( NULL != pFormatetc ) &&
                ( !IsBadReadPtr( (LPVOID)pFormatetc, sizeof( FORMATETC ) ) ) );
    OSL_ASSERT( ( NULL != pmedium ) &&
                ( !IsBadWritePtr( (LPVOID)pmedium, sizeof( STGMEDIUM ) ) ) );

    if ( ( NULL == pFormatetc ) || ( NULL == pmedium ) )
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;

    if ( CF_TEXT == pFormatetc->cfFormat )
    {
        char     buff[] = "Hello World, How are you!";
        LPSTREAM lpStream;

        hr = CreateStreamOnHGlobal( NULL, FALSE, &lpStream );
        if ( SUCCEEDED( hr ) )
        {
            hr = lpStream->Write( buff, sizeof( buff ) * sizeof( char ), NULL );
            if ( SUCCEEDED( hr ) )
            {
                HGLOBAL hGlob;

                GetHGlobalFromStream( lpStream, &hGlob );

                pmedium->tymed          = TYMED_HGLOBAL;
                pmedium->hGlobal        = hGlob;
                pmedium->pUnkForRelease = NULL;
            }
            lpStream->Release( );
            hr = S_OK;
        }
        else
        {
            pmedium->tymed = TYMED_NULL;
        }
    }
    else if ( CF_UNICODETEXT == pFormatetc->cfFormat )
    {
        WCHAR     buff[] = L"Hello World, How are you!";
        LPSTREAM lpStream;

        hr = CreateStreamOnHGlobal( NULL, FALSE, &lpStream );
        if ( SUCCEEDED( hr ) )
        {
            hr = lpStream->Write( buff, sizeof( buff ) * sizeof( WCHAR ), NULL );
            if ( SUCCEEDED( hr ) )
            {
                HGLOBAL hGlob;

                GetHGlobalFromStream( lpStream, &hGlob );

                pmedium->tymed          = TYMED_HGLOBAL;
                pmedium->hGlobal        = hGlob;
                pmedium->pUnkForRelease = NULL;
            }
            lpStream->Release( );
            hr = S_OK;
        }
        else
        {
            pmedium->tymed = TYMED_NULL;
        }
    }

    return hr;
}





STDMETHODIMP CXTDataObject::EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc )
{
    if ( ( NULL == ppenumFormatetc ) || ( DATADIR_SET == dwDirection ) )
        return E_INVALIDARG;

    *ppenumFormatetc = NULL;

    HRESULT hr = E_FAIL;

    if ( DATADIR_GET == dwDirection )
    {
        *ppenumFormatetc = new CEnumFormatEtc( this );
        static_cast< LPUNKNOWN >( *ppenumFormatetc )->AddRef( );
        hr = S_OK;
    }

    return hr;
}





STDMETHODIMP CXTDataObject::QueryGetData( LPFORMATETC pFormatetc )
{
    return E_NOTIMPL;
}





STDMETHODIMP CXTDataObject::GetDataHere( LPFORMATETC, LPSTGMEDIUM )
{
    return E_NOTIMPL;
}





STDMETHODIMP CXTDataObject::GetCanonicalFormatEtc( LPFORMATETC, LPFORMATETC )
{
    return E_NOTIMPL;
}





STDMETHODIMP CXTDataObject::SetData( LPFORMATETC, LPSTGMEDIUM, BOOL )
{
    return E_NOTIMPL;
}





STDMETHODIMP CXTDataObject::DAdvise( LPFORMATETC, DWORD, LPADVISESINK, DWORD * )
{
    return E_NOTIMPL;
}





STDMETHODIMP CXTDataObject::DUnadvise( DWORD )
{
    return E_NOTIMPL;
}





STDMETHODIMP CXTDataObject::EnumDAdvise( LPENUMSTATDATA * )
{
    return E_NOTIMPL;
}





CXTDataObject::operator IDataObject*( )
{
    return static_cast< IDataObject* >( this );
}











CEnumFormatEtc::CEnumFormatEtc( LPUNKNOWN pUnkDataObj ) :
    m_nRefCnt( 0 ),
    m_pUnkDataObj( pUnkDataObj ),
    m_nCurrentPos( 0 )
{
    m_cfFormats[0] = CF_UNICODETEXT;
    m_cfFormats[1] = CF_TEXT;
}





CEnumFormatEtc::~CEnumFormatEtc( )
{
}





STDMETHODIMP CEnumFormatEtc::QueryInterface( REFIID iid, LPVOID* ppvObject )
{
    if ( NULL == ppvObject )
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;

    *ppvObject = NULL;

    if ( ( __uuidof( IUnknown ) == iid ) || ( __uuidof( IEnumFORMATETC ) == iid ) )
    {
        *ppvObject = static_cast< IUnknown* >( this );
        static_cast< LPUNKNOWN >( *ppvObject )->AddRef( );
        hr = S_OK;
    }

    return hr;
}





STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef( )
{
    
    m_pUnkDataObj->AddRef( );
    return InterlockedIncrement( &m_nRefCnt );
}





STDMETHODIMP_(ULONG) CEnumFormatEtc::Release( )
{
    
    m_pUnkDataObj->Release( );

    
    
    
    ULONG nRefCnt = InterlockedDecrement( &m_nRefCnt );
    if ( 0 == nRefCnt )
        delete this;

    return nRefCnt;
}





STDMETHODIMP CEnumFormatEtc::Next( ULONG celt, LPFORMATETC rgelt, ULONG* pceltFetched )
{
    OSL_ASSERT( ( ( celt > 0 ) && ( NULL != rgelt ) ) ||
                ( ( 0 == celt ) && ( NULL == rgelt ) ) );

    if ( ( 0 != celt ) && ( NULL == rgelt ) )
        return E_INVALIDARG;

    ULONG   ulFetched = 0;
    ULONG   ulToFetch = celt;
    HRESULT hr        = S_FALSE;

    while( ( m_nCurrentPos < sizeof( m_cfFormats ) ) && ( ulToFetch > 0 ) )
    {
        OSL_ASSERT( !IsBadWritePtr( (LPVOID)rgelt, sizeof( FORMATETC ) ) );

        rgelt->cfFormat = m_cfFormats[m_nCurrentPos];
        rgelt->ptd      = NULL;
        rgelt->dwAspect = DVASPECT_CONTENT;
        rgelt->lindex   = -1;
        rgelt->tymed    = TYMED_HGLOBAL;

        ++m_nCurrentPos;
        ++rgelt;
        --ulToFetch;
        ++ulFetched;
    }

    if ( ulFetched == celt )
        hr = S_OK;

    if ( NULL != pceltFetched )
    {
        OSL_ASSERT( !IsBadWritePtr( (LPVOID)pceltFetched, sizeof( ULONG ) ) );
        *pceltFetched = ulFetched;
    }

    return hr;
}





STDMETHODIMP CEnumFormatEtc::Skip( ULONG celt )
{
    HRESULT hr = S_FALSE;

    if ( ( m_nCurrentPos + celt ) < sizeof( m_cfFormats ) )
    {
        m_nCurrentPos += celt;
        hr = S_OK;
    }

    return hr;
}





STDMETHODIMP CEnumFormatEtc::Reset( )
{
    m_nCurrentPos = 0;
    return S_OK;
}





STDMETHODIMP CEnumFormatEtc::Clone( IEnumFORMATETC** ppenum )
{
    OSL_ASSERT( NULL != ppenum );

    if ( NULL == ppenum )
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;

    *ppenum = NULL;

    CEnumFormatEtc* pCEnumFEtc = new CEnumFormatEtc( m_pUnkDataObj );
    if ( NULL != pCEnumFEtc )
    {
        pCEnumFEtc->m_nCurrentPos = m_nCurrentPos;
        *ppenum = static_cast< IEnumFORMATETC* >( pCEnumFEtc );
        static_cast< LPUNKNOWN >( *ppenum )->AddRef( );
        hr = NOERROR;
    }

    return hr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
