/*************************************************************************
 *
 *  $RCSfile: ed_ioleobject.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: abi $ $Date: 2003-03-26 13:51:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "embeddoc.hxx"

#ifndef _COM_SUN_STAR_FRAME_XController_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif


using namespace ::com::sun::star;


extern ::rtl::OUString  getFilterNameFromGUID_Impl( GUID* );


STDMETHODIMP EmbedDocument_Impl::SetClientSite( IOleClientSite* pSite )
{
    m_pClientSite = pSite;
    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::GetClientSite( IOleClientSite** pSite )
{
    *pSite = m_pClientSite;
    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::SetHostNames( LPCOLESTR szContainerApp, LPCOLESTR szContainerObj )
{
    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::Close( DWORD dwSaveOption )
{
    HRESULT hr = S_OK;

    if ( dwSaveOption && m_pClientSite )
        hr = m_pClientSite->SaveObject();

    if ( m_pDAdviseHolder )
        m_pDAdviseHolder->SendOnDataChange( (IDataObject*)this, 0, ADVF_DATAONSTOP );

    m_pDocHolder->CloseFrame();

    if ( m_pClientSite )
        m_pClientSite->OnShowWindow( FALSE );

    for ( AdviseSinkHashMapIterator iAdvise = m_aAdviseHashMap.begin(); iAdvise != m_aAdviseHashMap.end(); iAdvise++ )
    {
        if ( iAdvise->second )
            iAdvise->second->OnClose();
    }

    return hr;
}

STDMETHODIMP EmbedDocument_Impl::SetMoniker( DWORD dwWhichMoniker, IMoniker *pmk )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::GetMoniker( DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::InitFromData( IDataObject *pDataObject, BOOL fCreation, DWORD dwReserved )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::GetClipboardData( DWORD dwReserved, IDataObject **ppDataObject )
{
    return E_NOTIMPL;
}


void EmbedDocument_Impl::notify()
{
    for ( AdviseSinkHashMapIterator iAdvise =
              m_aAdviseHashMap.begin();
          iAdvise != m_aAdviseHashMap.end();
          iAdvise++ )
        if ( iAdvise->second )
            iAdvise->second->OnViewChange( DVASPECT_CONTENT, -1 );

    if ( m_pDAdviseHolder )
        m_pDAdviseHolder->SendOnDataChange( (IDataObject*)this, 0, 0 );
}


STDMETHODIMP EmbedDocument_Impl::DoVerb( LONG iVerb, LPMSG lpmsg, IOleClientSite *pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect )
{
    if ( iVerb == OLEIVERB_PRIMARY || iVerb == OLEIVERB_SHOW || iVerb == OLEIVERB_OPEN )
    {
        if( m_pDocHolder )
            m_pDocHolder->show();

        if ( m_pClientSite )
            m_pClientSite->OnShowWindow( TRUE );

        notify();

        return S_OK;
    }
    else if( iVerb == OLEIVERB_HIDE )
    {
        if(m_pDocHolder)
            m_pDocHolder->hide();

        if( m_pClientSite )
            m_pClientSite->OnShowWindow( FALSE );

        return S_OK;
    }
    else
        return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::EnumVerbs( IEnumOLEVERB **ppEnumOleVerb )
{
    return OLE_S_USEREG;
}

STDMETHODIMP EmbedDocument_Impl::Update()
{
    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::IsUpToDate()
{
    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::GetUserClassID( CLSID *pClsid )
{
    return GetClassID( pClsid );
}

STDMETHODIMP EmbedDocument_Impl::GetUserType( DWORD dwFormOfType, LPOLESTR *pszUserType )
{
    return OLE_S_USEREG;
}

STDMETHODIMP EmbedDocument_Impl::SetExtent( DWORD dwDrawAspect, SIZEL *psizel )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::GetExtent( DWORD dwDrawAspect, SIZEL *psizel )
{
    if ( psizel )
    {
        psizel->cx = 10000;
        psizel->cy = 10000;
    }

    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::Advise( IAdviseSink *pAdvSink, DWORD *pdwConnection )
{
    if ( m_nAdviseNum == 0xFFFFFFFF )
        return E_OUTOFMEMORY;

    pAdvSink->AddRef();
    m_aAdviseHashMap.insert( ::std::pair< DWORD, IAdviseSink* >( m_nAdviseNum, pAdvSink ) );
    *pdwConnection = m_nAdviseNum++;

    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::Unadvise( DWORD dwConnection )
{
    AdviseSinkHashMapIterator iAdvise = m_aAdviseHashMap.find( dwConnection );
    if ( iAdvise != m_aAdviseHashMap.end() )
    {
        iAdvise->second->Release();
        m_aAdviseHashMap.erase( iAdvise );
    }
    else
        return OLE_E_NOCONNECTION;

    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::EnumAdvise( IEnumSTATDATA **ppenumAdvise )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::GetMiscStatus( DWORD dwAspect, DWORD *pdwStatus )
{
    return OLE_S_USEREG;
}

STDMETHODIMP EmbedDocument_Impl::SetColorScheme( LOGPALETTE *pLogpal )
{
    return E_NOTIMPL;
}

