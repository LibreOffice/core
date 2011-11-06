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
        WRITEDEBUGINFO( "OleWrapperAdviseSink::OnDataChange():" );
        m_pListener->OnDataChange( pFetc, pMedium );
    }
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnViewChange( DWORD dwAspect, LONG lindex )
{
    if ( m_pListener )
    {
        WRITEDEBUGINFO( "OleWrapperAdviseSink::OnViewChange():" );
        m_pListener->OnViewChange( dwAspect, lindex );
    }
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnRename( LPMONIKER pMoniker )
{
    if ( m_pListener )
    {
        WRITEDEBUGINFO( "OleWrapperAdviseSink::OnRename():" );
        m_pListener->OnRename( pMoniker );
    }
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnSave(void)
{
    if ( m_pListener )
    {
        WRITEDEBUGINFO( "OleWrapperAdviseSink::OnSave():" );
        m_pListener->OnSave();
    }
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnClose(void)
{
    if ( m_pListener )
    {
        WRITEDEBUGINFO( "OleWrapperAdviseSink::OnClose():" );
        m_pListener->OnClose();
    }

    if ( m_bHandleClosed )
        m_bClosed = TRUE;
}

} // namespace inprocserv

