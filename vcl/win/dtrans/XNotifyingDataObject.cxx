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

#include <osl/diagnose.h>
#include "XNotifyingDataObject.hxx"
#include "WinClipboard.hxx"

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Reference;

CXNotifyingDataObject::CXNotifyingDataObject(
    const IDataObjectPtr& aIDataObject,
    const Reference< XTransferable >& aXTransferable,
    const Reference< XClipboardOwner >& aXClipOwner,
    CWinClipboard* const theWinClipoard) :
    m_nRefCnt( 0 ),
    m_aIDataObject( aIDataObject ),
    m_XTransferable( aXTransferable ),
    m_XClipboardOwner( aXClipOwner ),
    m_pWinClipImpl( theWinClipoard )
{
}

STDMETHODIMP CXNotifyingDataObject::QueryInterface( REFIID iid, void** ppvObject )
{
    if ( nullptr == ppvObject )
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;

    *ppvObject = nullptr;
    if ( ( __uuidof( IUnknown ) == iid ) ||
         ( __uuidof( IDataObject ) == iid ) )
    {
        *ppvObject = static_cast< IUnknown* >( this );
        static_cast<LPUNKNOWN>(*ppvObject)->AddRef( );
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP_(ULONG) CXNotifyingDataObject::AddRef( )
{
    return static_cast< ULONG >( InterlockedIncrement( &m_nRefCnt ) );
}

STDMETHODIMP_(ULONG) CXNotifyingDataObject::Release( )
{
    ULONG nRefCnt =
        static_cast< ULONG >( InterlockedDecrement( &m_nRefCnt ) );

    if ( 0 == nRefCnt )
    {
        if ( m_pWinClipImpl )
            m_pWinClipImpl->onReleaseDataObject( this );

        delete this;
    }

    return nRefCnt;
}

STDMETHODIMP CXNotifyingDataObject::GetData( FORMATETC * pFormatetc, STGMEDIUM * pmedium )
{
    return m_aIDataObject->GetData(pFormatetc, pmedium);
}

STDMETHODIMP CXNotifyingDataObject::EnumFormatEtc(
    DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc )
{
    return m_aIDataObject->EnumFormatEtc(dwDirection, ppenumFormatetc);
}

STDMETHODIMP CXNotifyingDataObject::QueryGetData( FORMATETC * pFormatetc )
{
    return m_aIDataObject->QueryGetData(pFormatetc);
}

STDMETHODIMP CXNotifyingDataObject::GetDataHere( FORMATETC * lpFetc, STGMEDIUM * lpStgMedium )
{
    return m_aIDataObject->GetDataHere(lpFetc, lpStgMedium);
}

STDMETHODIMP CXNotifyingDataObject::GetCanonicalFormatEtc( FORMATETC * lpFetc, FORMATETC * lpCanonicalFetc )
{
    return m_aIDataObject->GetCanonicalFormatEtc(lpFetc, lpCanonicalFetc);
}

STDMETHODIMP CXNotifyingDataObject::SetData( FORMATETC * lpFetc, STGMEDIUM * lpStgMedium, BOOL bRelease )
{
    return m_aIDataObject->SetData( lpFetc, lpStgMedium, bRelease );
}

STDMETHODIMP CXNotifyingDataObject::DAdvise(
    FORMATETC * lpFetc, DWORD advf, IAdviseSink * lpAdvSink, DWORD* pdwConnection )
{
    return m_aIDataObject->DAdvise( lpFetc, advf, lpAdvSink, pdwConnection );
}

STDMETHODIMP CXNotifyingDataObject::DUnadvise( DWORD dwConnection )
{
    return m_aIDataObject->DUnadvise( dwConnection );
}

STDMETHODIMP CXNotifyingDataObject::EnumDAdvise( IEnumSTATDATA ** ppenumAdvise )
{
    return m_aIDataObject->EnumDAdvise( ppenumAdvise );
}

CXNotifyingDataObject::operator IDataObject*( )
{
    return static_cast< IDataObject* >( this );
}

void CXNotifyingDataObject::lostOwnership( )
{
    try
    {
        if (m_XClipboardOwner.is())
            m_XClipboardOwner->lostOwnership(
                static_cast<XClipboardEx*>(m_pWinClipImpl), m_XTransferable);
    }
    catch(RuntimeException&)
    {
        OSL_FAIL( "RuntimeException caught" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
