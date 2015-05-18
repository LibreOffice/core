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

#if defined _MSC_VER
#pragma warning(disable : 4668)
#endif

#include <advisesink.hxx>

namespace inprocserv
{

OleWrapperAdviseSink::OleWrapperAdviseSink()
: m_nRefCount( 0 )
, m_pFormatEtc( NULL )
, m_nAspect( DVASPECT_CONTENT )
, m_nRegID( 0 )
, m_bObjectAdvise( TRUE )
, m_nDataRegFlag( 0 )
, m_nViewRegFlag( 0 )
, m_bHandleClosed( TRUE )
, m_bClosed( FALSE )
{
}

OleWrapperAdviseSink::OleWrapperAdviseSink( const ComSmart< IAdviseSink >& pListener )
: m_nRefCount( 0 )
, m_pListener( pListener )
, m_pFormatEtc( NULL )
, m_nAspect( DVASPECT_CONTENT )
, m_nRegID( 0 )
, m_bObjectAdvise( TRUE )
, m_nDataRegFlag( 0 )
, m_nViewRegFlag( 0 )
, m_bHandleClosed( FALSE )
, m_bClosed( FALSE )
{
}

OleWrapperAdviseSink::OleWrapperAdviseSink( const ComSmart< IAdviseSink >& pListener, FORMATETC* pFormatEtc, DWORD nDataRegFlag )
: m_nRefCount( 0 )
, m_pListener( pListener )
, m_pFormatEtc( NULL )
, m_nAspect( DVASPECT_CONTENT )
, m_nRegID( 0 )
, m_bObjectAdvise( FALSE )
, m_nDataRegFlag( nDataRegFlag )
, m_nViewRegFlag( 0 )
, m_bHandleClosed( FALSE )
, m_bClosed( FALSE )
{
    if ( pFormatEtc )
    {
        m_pFormatEtc = new FORMATETC;
        m_pFormatEtc->cfFormat = pFormatEtc->cfFormat;
        m_pFormatEtc->ptd = NULL;
        m_pFormatEtc->dwAspect = pFormatEtc->dwAspect;
        m_pFormatEtc->lindex = pFormatEtc->lindex;
        m_pFormatEtc->tymed = pFormatEtc->tymed;
    }
}

OleWrapperAdviseSink::OleWrapperAdviseSink( const ComSmart< IAdviseSink >& pListener, DWORD nAspect, DWORD nViewRegFlag )
: m_nRefCount( 0 )
, m_pListener( pListener )
, m_pFormatEtc( NULL )
, m_nAspect( nAspect )
, m_nRegID( 0 )
, m_bObjectAdvise( TRUE )
, m_nDataRegFlag( 0 )
, m_nViewRegFlag( nViewRegFlag )
, m_bHandleClosed( FALSE )
, m_bClosed( FALSE )
{
}

OleWrapperAdviseSink::~OleWrapperAdviseSink()
{
    if ( m_pFormatEtc )
        delete m_pFormatEtc;
}

STDMETHODIMP OleWrapperAdviseSink::QueryInterface( REFIID riid , void** ppv )
{
    *ppv=NULL;

    if ( riid == IID_IUnknown )
        *ppv = (IUnknown*)this;

    if ( riid == IID_IAdviseSink )
        *ppv = (IAdviseSink*)this;

    if ( *ppv != NULL )
    {
        ((IUnknown*)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) OleWrapperAdviseSink::AddRef()
{
    return ++m_nRefCount;
}

STDMETHODIMP_(ULONG) OleWrapperAdviseSink::Release()
{
    ULONG nReturn = --m_nRefCount;
    if ( m_nRefCount == 0 )
        delete this;

    return nReturn;
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnDataChange( LPFORMATETC pFetc, LPSTGMEDIUM pMedium )
{
    if ( m_pListener )
    {
        m_pListener->OnDataChange( pFetc, pMedium );
    }
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnViewChange( DWORD dwAspect, LONG lindex )
{
    if ( m_pListener )
    {
        m_pListener->OnViewChange( dwAspect, lindex );
    }
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnRename( LPMONIKER pMoniker )
{
    if ( m_pListener )
    {
        m_pListener->OnRename( pMoniker );
    }
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnSave()
{
    if ( m_pListener )
    {
        m_pListener->OnSave();
    }
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnClose()
{
    if ( m_pListener )
    {
        m_pListener->OnClose();
    }

    if ( m_bHandleClosed )
        m_bClosed = TRUE;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
