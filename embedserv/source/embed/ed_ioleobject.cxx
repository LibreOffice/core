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

#include "embeddoc.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/beans/PropertyValue.hpp>


using namespace ::com::sun::star;

// IOleObject


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
    // the code should be ignored for links
    if ( !m_aFileName.getLength() )
    {
        m_pDocHolder->setTitle(SAL_U(szContainerObj));
        m_pDocHolder->setContainerName(SAL_U(szContainerApp));
    }

    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::Close( DWORD dwSaveOption )
{
    HRESULT hr = S_OK;

    if ( m_pDocHolder->HasFrame() )
    {
        if ( dwSaveOption == 2 && m_aFileName.getLength() )
        {
            // ask the user about saving
            if ( m_pDocHolder->ExecuteSuspendCloseFrame() )
            {
                m_pDocHolder->CloseDocument();
                return S_OK;
            }
            else
                return OLE_E_PROMPTSAVECANCELLED;
        }

        if ( dwSaveOption != 1 )
            hr = SaveObject(); // ADVF_DATAONSTOP);

        m_pDocHolder->CloseFrame();
        OLENotifyDeactivation();
    }

    m_pDocHolder->FreeOffice();
    m_pDocHolder->CloseDocument();

    OLENotifyClosing();

    return hr;
}


HRESULT EmbedDocument_Impl::OLENotifyClosing()
{
    AdviseSinkHashMap aAHM(m_aAdviseHashMap);

    for ( AdviseSinkHashMapIterator iAdvise = aAHM.begin();
          iAdvise != aAHM.end(); iAdvise++ )
    {
        if ( iAdvise->second )
            iAdvise->second->OnClose();
    }

    return S_OK;

}

STDMETHODIMP EmbedDocument_Impl::SetMoniker( DWORD /*dwWhichMoniker*/, IMoniker * /*pmk*/ )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::GetMoniker( DWORD /*dwAssign*/, DWORD /*dwWhichMoniker*/, IMoniker ** /*ppmk*/ )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::InitFromData( IDataObject * /*pDataObject*/, BOOL /*fCreation*/, DWORD /*dwReserved*/ )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::GetClipboardData( DWORD /*dwReserved*/, IDataObject ** /*ppDataObject*/ )
{
    return E_NOTIMPL;
}

/**
 *  Well, this is a not so very inefficient way to deliver
 *
 */

STDMETHODIMP EmbedDocument_Impl::DoVerb(
    LONG iVerb,
    LPMSG,
    IOleClientSite *pActiveSite,
    LONG,
    HWND,
    LPCRECT )
{
    // no locking is used since the OLE must use the same thread always
    if ( m_bIsInVerbHandling )
        return OLEOBJ_S_CANNOT_DOVERB_NOW;

    // an object can not handle any Verbs in Hands off mode
    if ( m_pMasterStorage == nullptr || m_pOwnStream == nullptr )
        return OLE_E_CANT_BINDTOSOURCE;


    BooleanGuard_Impl aGuard( m_bIsInVerbHandling );

    if ( iVerb == OLEIVERB_PRIMARY )
    {
        if ( m_aFileName.getLength() )
        {
            // that should be a link
            iVerb = OLEIVERB_OPEN;
        }
        else
            iVerb = OLEIVERB_SHOW;
    }

    try
    {
        switch(iVerb) {
            case OLEIVERB_DISCARDUNDOSTATE:
                // free any undostate?
                break;
            case OLEIVERB_INPLACEACTIVATE:
                OSL_ENSURE(m_pDocHolder,"no document for inplace activation");

                return m_pDocHolder->InPlaceActivate(pActiveSite,FALSE);
            case OLEIVERB_UIACTIVATE:
                OSL_ENSURE(m_pDocHolder,"no document for     inplace activation");

                return m_pDocHolder->InPlaceActivate(pActiveSite,TRUE);
            case OLEIVERB_PRIMARY:
            case OLEIVERB_SHOW:
                OSL_ENSURE(m_pDocHolder,"no document for inplace activation");

                if(m_pDocHolder->isActive())
                    return NOERROR; //Already active

                if(SUCCEEDED(
                    m_pDocHolder->InPlaceActivate(
                        pActiveSite,TRUE)))
                    return NOERROR;

                SAL_FALLTHROUGH;
            case OLEIVERB_OPEN:
                OSL_ENSURE(m_pDocHolder,"no document to open");

                // the commented code could be useful in case
                // outer window would be resized depending from inner one
                // RECTL aEmbArea;
                // m_pDocHolder->GetVisArea( &aEmbArea );
                // m_pDocHolder->show();
                // m_pDocHolder->SetVisArea( &aEmbArea );

                if(m_pDocHolder->isActive())
                {
                    m_pDocHolder->InPlaceDeactivate();
                    m_pDocHolder->DisableInplaceActivation(true);
                }

                SIZEL aEmbSize;
                m_pDocHolder->GetExtent( &aEmbSize );
                m_pDocHolder->show();
                m_pDocHolder->resizeWin( aEmbSize );

                if ( m_pClientSite )
                    m_pClientSite->OnShowWindow( TRUE );

                notify();
                break;
            case OLEIVERB_HIDE:
                OSL_ENSURE(m_pDocHolder,"no document to hide");

                if(m_pDocHolder->isActive())
                    m_pDocHolder->InPlaceDeactivate();
                else {
                    m_pDocHolder->hide();

                    if( m_pClientSite )
                        m_pClientSite->OnShowWindow(FALSE);
                }
                break;
            default:
                break;
        }
    }
    catch( const uno::Exception& )
    {
        return OLEOBJ_S_CANNOT_DOVERB_NOW;
    }

    return NOERROR;
}


STDMETHODIMP EmbedDocument_Impl::EnumVerbs( IEnumOLEVERB ** /*ppEnumOleVerb*/ )
{
    return OLE_S_USEREG;
}

STDMETHODIMP EmbedDocument_Impl::Update()
{
    return S_OK;
//    HRESULT hr = CACHE_E_NOCACHE_UPDATED;
//    return hr;
}

STDMETHODIMP EmbedDocument_Impl::IsUpToDate()
{
    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::GetUserClassID( CLSID *pClsid )
{
    return GetClassID( pClsid );
}

STDMETHODIMP EmbedDocument_Impl::GetUserType( DWORD /*dwFormOfTypeUe*/, LPOLESTR * /*pszUserType*/ )
{
    return OLE_S_USEREG;
}

STDMETHODIMP EmbedDocument_Impl::SetExtent( DWORD /*dwDrawAspect*/, SIZEL *psizel )
{
    if ( !psizel )
        return E_FAIL;

    m_pDocHolder->SetExtent( psizel );

    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::GetExtent( DWORD /*dwDrawAspect*/, SIZEL * psizel )
{
    if ( !psizel )
        return E_INVALIDARG;

    if ( FAILED( m_pDocHolder->GetExtent( psizel ) ) )
    {
        // return default values
        psizel->cx = 500;
        psizel->cy = 500;
    }

    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::Advise( IAdviseSink *pAdvSink, DWORD *pdwConnection )
{
    if ( m_nAdviseNum == 0xFFFFFFFF )
        return E_OUTOFMEMORY;

    pAdvSink->AddRef();
    m_aAdviseHashMap.insert( std::pair< DWORD, IAdviseSink* >( m_nAdviseNum, pAdvSink ) );
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

STDMETHODIMP EmbedDocument_Impl::EnumAdvise( IEnumSTATDATA ** /*ppenumAdvise*/ )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::GetMiscStatus( DWORD /*dwAspect*/, DWORD * /*pdwStatus*/ )
{
    return OLE_S_USEREG;
}

STDMETHODIMP EmbedDocument_Impl::SetColorScheme( LOGPALETTE * /*pLogpal*/ )
{
    return E_NOTIMPL;
}


// IDispatch

STDMETHODIMP EmbedDocument_Impl::GetTypeInfoCount( unsigned int FAR*  pctinfo )
{
    if ( m_pDocHolder->GetIDispatch() )
        return m_pDocHolder->GetIDispatch()->GetTypeInfoCount( pctinfo );

    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::GetTypeInfo( unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo )
{
    if ( m_pDocHolder->GetIDispatch() )
        return m_pDocHolder->GetIDispatch()->GetTypeInfo( iTInfo, lcid, ppTInfo );

    return DISP_E_BADINDEX; // the only error that can be returned
}

STDMETHODIMP EmbedDocument_Impl::GetIDsOfNames( REFIID riid,
                                                OLECHAR FAR* FAR* rgszNames,
                                                unsigned int cNames,
                                                LCID lcid,
                                                DISPID FAR* rgDispId )
{
    if ( m_pDocHolder->GetIDispatch() )
        return m_pDocHolder->GetIDispatch()->GetIDsOfNames( riid, rgszNames, cNames, lcid, rgDispId );

    for ( unsigned int ind = 0; ind < cNames; ind++ )
        rgDispId[ind] = DISPID_UNKNOWN;

    return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP EmbedDocument_Impl::Invoke( DISPID dispIdMember,
                                         REFIID riid,
                                         LCID lcid,
                                         WORD wFlags,
                                         DISPPARAMS FAR* pDispParams,
                                         VARIANT FAR* pVarResult,
                                         EXCEPINFO FAR* pExcepInfo,
                                         unsigned int FAR* puArgErr )
{
    if ( m_pDocHolder->GetIDispatch() )
        return m_pDocHolder->GetIDispatch()->Invoke( dispIdMember,
                                                     riid,
                                                     lcid,
                                                     wFlags,
                                                     pDispParams,
                                                     pVarResult,
                                                     pExcepInfo,
                                                     puArgErr );

    return DISP_E_MEMBERNOTFOUND;
}


// IExternalConnection

DWORD STDMETHODCALLTYPE EmbedDocument_Impl::AddConnection( DWORD , DWORD )
{
    return AddRef();
}

DWORD STDMETHODCALLTYPE EmbedDocument_Impl::ReleaseConnection( DWORD , DWORD , BOOL )
{
    return Release();
}

// C++ - methods

HRESULT EmbedDocument_Impl::SaveObject()
{
    HRESULT hr = S_OK;

    if(m_pClientSite) {
        hr = m_pClientSite->SaveObject();

        for ( AdviseSinkHashMapIterator iAdvise =
                  m_aAdviseHashMap.begin();
              iAdvise != m_aAdviseHashMap.end();
              iAdvise++ )
            if ( iAdvise->second )
                iAdvise->second->OnSave( );
    }
    else if ( m_aFileName.getLength() && IsDirty() == S_OK )
    {
        OUString aPreservFileName = m_aFileName;

        // in case of links the containers does not provide client site sometimes
        hr = Save( static_cast<LPCOLESTR>(nullptr), FALSE ); // triggers saving to the link location
        SaveCompleted(SAL_W(aPreservFileName.getStr()));
    }

    notify( false );

    return hr;
}


HRESULT EmbedDocument_Impl::ShowObject()
{
    HRESULT hr = S_OK;

    if(m_pClientSite)
        hr = m_pClientSite->ShowObject();

    return hr;
}


void EmbedDocument_Impl::notify( bool bDataChanged )
{
    for ( AdviseSinkHashMapIterator iAdvise =
              m_aAdviseHashMap.begin();
          iAdvise != m_aAdviseHashMap.end();
          iAdvise++ )
        if ( iAdvise->second )
            iAdvise->second->OnViewChange( DVASPECT_CONTENT, -1 );

    if ( m_pDAdviseHolder && bDataChanged )
        m_pDAdviseHolder->SendOnDataChange( static_cast<IDataObject*>(this), 0, 0 );
}

void EmbedDocument_Impl::Deactivate()
{
    HRESULT hr = S_OK;

    if ( m_pDocHolder->HasFrame() )
    {
        hr = SaveObject();
        m_pDocHolder->CloseFrame();
        OLENotifyDeactivation();
    }
}

HRESULT EmbedDocument_Impl::OLENotifyDeactivation()
{
    HRESULT hr = S_OK;

    if ( m_pClientSite )
        hr = m_pClientSite->OnShowWindow( FALSE );

    return hr;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
