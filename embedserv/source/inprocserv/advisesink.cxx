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

#pragma warning(disable : 4668)

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

STDMETHODIMP_(void) OleWrapperAdviseSink::OnSave(void)
{
    if ( m_pListener )
    {
        m_pListener->OnSave();
    }
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnClose(void)
{
    if ( m_pListener )
    {
        m_pListener->OnClose();
    }

    if ( m_bHandleClosed )
        m_bClosed = TRUE;
}

} // namespace inprocserv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
