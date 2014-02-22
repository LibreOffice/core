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

#include "../DTransHelper.hxx"

#include "XTDo.hxx"

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#include <ole2.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <memory>
#include <tchar.h>





using namespace ::std;








/*
    in the constructor we enumerate all formats offered by the transferable
    and convert the formats into formatetc structures
    if the transferable supports text in different charsets we use either
    the charset equal to the charset of the current thread or an arbitrary
    charset supported by the transferable and the system
    if the transferable supports only unicodetext we offer in addition to
    this text in the charset of the current thread
    in order to allow the consumer of the clipboard to query for the charset
    of the text in the clipboard we offer a CF_LOCALE
*/
CXTDataObject::CXTDataObject( ) :
    m_nRefCnt( 0 )
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

/*------------------------------------------------------------------------

 IDataObject->GetData
 we deliver data only into global memory

 algo:
 1. convert the given formatect struct into a valid dataflavor
 2. if the transferable directly supports the requested format
 2.1. if text data requested add a trailing '\0' in order to prevent
        problems (windows needs '\0' terminated strings
 2.2. we expect unicode data as Sequence< sal_Unicode > and all other
        text and raw data as Sequence< sal_Int8 >

------------------------------------------------------------------------*/

STDMETHODIMP CXTDataObject::GetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium )
{
    if ( ( NULL == pFormatetc ) || ( NULL == pmedium ) )
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;
    char    pBuff[] = "Test OleClipboard";

    if ( CF_TEXT == pFormatetc->cfFormat )
    {
        CHGlobalHelper hGlobHlp( TRUE );

        hGlobHlp.Write( pBuff, sizeof( pBuff ), NULL );

        pmedium->tymed          = TYMED_HGLOBAL;
        pmedium->hGlobal        = hGlobHlp.GetHGlobal( );
        pmedium->pUnkForRelease = NULL;

        hr = S_OK;
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
    m_nCurrPos( 0 )
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
    if ( ( 0 != celt ) && ( NULL == rgelt ) )
        return E_INVALIDARG;

    ULONG   ulFetched = 0;
    ULONG   ulToFetch = celt;
    HRESULT hr        = S_FALSE;

    while( m_nCurrPos < 1 )
    {
        rgelt->cfFormat = CF_TEXT;
        rgelt->ptd      = NULL;
        rgelt->dwAspect = DVASPECT_CONTENT;
        rgelt->lindex   = -1;
        rgelt->tymed    = TYMED_HGLOBAL;

        ++m_nCurrPos;
        ++rgelt;
        --ulToFetch;
        ++ulFetched;
    }

    if ( ulFetched == celt )
        hr = S_OK;

    if ( NULL != pceltFetched )
    {
        *pceltFetched = ulFetched;
    }

    return hr;
}





STDMETHODIMP CEnumFormatEtc::Skip( ULONG celt )
{
    HRESULT hr = S_FALSE;

    /*
    if ( ( m_nCurrPos + celt ) < m_nClipFormats )
    {
        m_nCurrPos += celt;
        hr = S_OK;
    }
    */

    return hr;
}





STDMETHODIMP CEnumFormatEtc::Reset( )
{
    m_nCurrPos = 0;
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
        pCEnumFEtc->m_nCurrPos = m_nCurrPos;
        *ppenum = static_cast< IEnumFORMATETC* >( pCEnumFEtc );
        static_cast< LPUNKNOWN >( *ppenum )->AddRef( );
        hr = NOERROR;
    }

    return hr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
