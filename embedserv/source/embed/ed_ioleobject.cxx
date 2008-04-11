/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ed_ioleobject.cxx,v $
 * $Revision: 1.20 $
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

#include "embeddoc.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>


using namespace ::com::sun::star;


extern ::rtl::OUString  getFilterNameFromGUID_Impl( GUID* );

//-------------------------------------------------------------------------------
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
        m_pDocHolder->setTitle(
            rtl::OUString(
                (sal_Unicode*)szContainerObj));
        m_pDocHolder->setContainerName(
            rtl::OUString(
                (sal_Unicode*)szContainerApp));
    }

    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::Close( DWORD dwSaveOption )
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

    HRESULT hr = S_OK;

    if ( dwSaveOption != 1 )
        hr = SaveObject(); // ADVF_DATAONSTOP);

    m_pDocHolder->FreeOffice();
    m_pDocHolder->CloseDocument();
    m_pDocHolder->CloseFrame();

    OLENotifyClosing();

    return hr;
}


HRESULT EmbedDocument_Impl::OLENotifyClosing()
{
    HRESULT hr = S_OK;

    if ( m_pClientSite )
        m_pClientSite->OnShowWindow( FALSE );

    AdviseSinkHashMap aAHM(m_aAdviseHashMap);

    for ( AdviseSinkHashMapIterator iAdvise = aAHM.begin();
          iAdvise != aAHM.end(); iAdvise++ )
    {
        if ( iAdvise->second )
            iAdvise->second->OnClose();
    }

    return hr;

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
                break;
            case OLEIVERB_UIACTIVATE:
                OSL_ENSURE(m_pDocHolder,"no document for     inplace activation");

                return m_pDocHolder->InPlaceActivate(pActiveSite,TRUE);
                break;
            case OLEIVERB_PRIMARY:
            case OLEIVERB_SHOW:
                OSL_ENSURE(m_pDocHolder,"no document for inplace activation");

                if(m_pDocHolder->isActive())
                    return NOERROR; //Already active

                if(SUCCEEDED(
                    m_pDocHolder->InPlaceActivate(
                        pActiveSite,TRUE)))
                    return NOERROR;

                // intended fall trough
            case OLEIVERB_OPEN:
                OSL_ENSURE(m_pDocHolder,"no document to open");

                // the commented code could be usefull in case
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
    catch( uno::Exception& )
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

//-------------------------------------------------------------------------------
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
        ::rtl::OUString aPreservFileName = m_aFileName;

        // in case of links the containers does not provide client site sometimes
        hr = Save( (LPCOLESTR)NULL, FALSE ); // triggers saving to the link location
        SaveCompleted( (LPCOLESTR)aPreservFileName.getStr() );
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
        m_pDAdviseHolder->SendOnDataChange( (IDataObject*)this, 0, 0 );
}

// Fix strange warnings about some
// ATL::CAxHostWindow::QueryInterface|AddRef|Releae functions.
// warning C4505: 'xxx' : unreferenced local function has been removed
#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif
