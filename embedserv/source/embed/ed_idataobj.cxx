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
#ifdef _MSC_VER
#pragma warning(disable : 4917 4555)
#endif

#include "embeddoc.hxx"

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>


#include <osl/thread.h>

using namespace ::com::sun::star;


// EmbedDocument_Impl


sal_uInt64 EmbedDocument_Impl::getMetaFileHandle_Impl( bool isEnhMeta )
{
    sal_uInt64 pResult = NULL;

    uno::Reference< datatransfer::XTransferable > xTransferable( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
    if ( xTransferable.is() )
    {
        datatransfer::DataFlavor aFlavor;

        if ( isEnhMeta )
        {
            aFlavor.MimeType = "application/x-openoffice-emf;windows_formatname=\"Image EMF\"";
            aFlavor.HumanPresentableName = "Enhanced Windows MetaFile";
        }
        else
        {
            aFlavor.MimeType = "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"";
            aFlavor.HumanPresentableName = "Windows GDIMetaFile";
        }

        aFlavor.DataType = cppu::UnoType<sal_uInt64>::get();

        uno::Any aAny = xTransferable->getTransferData( aFlavor );
        aAny >>= pResult;
    }

    return pResult;
}


// IDataObject

STDMETHODIMP EmbedDocument_Impl::GetData( FORMATETC * pFormatetc, STGMEDIUM * pMedium )
{
    if ( !pFormatetc )
        return DV_E_FORMATETC;

    if ( !pMedium )
        return STG_E_MEDIUMFULL;

    if ( pFormatetc->dwAspect == DVASPECT_THUMBNAIL
      || pFormatetc->dwAspect == DVASPECT_ICON
      || pFormatetc->dwAspect == DVASPECT_DOCPRINT )
        return DV_E_DVASPECT;

    if ( pFormatetc->cfFormat == CF_ENHMETAFILE )
    {
        if ( !( pFormatetc->tymed & TYMED_ENHMF ) )
            return DV_E_TYMED;

        HENHMETAFILE hMeta = reinterpret_cast<HENHMETAFILE>( getMetaFileHandle_Impl( true ) );

        if ( hMeta )
        {
            pMedium->tymed = TYMED_ENHMF;
            pMedium->hEnhMetaFile = hMeta;
            pMedium->pUnkForRelease = nullptr;

            return S_OK;
        }

        return STG_E_MEDIUMFULL;
    }
    else if ( pFormatetc->cfFormat == CF_METAFILEPICT )
    {
          if ( !( pFormatetc->tymed & TYMED_MFPICT ) )
            return DV_E_TYMED;

        HGLOBAL hMeta = reinterpret_cast<HGLOBAL>( getMetaFileHandle_Impl( false ) );

        if ( hMeta )
        {
            pMedium->tymed = TYMED_MFPICT;
            pMedium->hMetaFilePict = hMeta;
            pMedium->pUnkForRelease = nullptr;

            return S_OK;
        }

        return STG_E_MEDIUMFULL;
    }
    else
    {
        CLIPFORMAT cf_embSource = (CLIPFORMAT)RegisterClipboardFormatW( L"Embed Source" );
        CLIPFORMAT cf_embObj = (CLIPFORMAT)RegisterClipboardFormatW( L"Embedded Object" );
        if ( pFormatetc->cfFormat == cf_embSource || pFormatetc->cfFormat == cf_embObj )
        {
            if ( !( pFormatetc->tymed & TYMED_ISTORAGE ) )
                return DV_E_TYMED;

            CComPtr< IStorage > pNewStg;
            HRESULT hr = StgCreateDocfile( nullptr, STGM_CREATE | STGM_READWRITE | STGM_DELETEONRELEASE, 0, &pNewStg );
            if ( FAILED( hr ) || !pNewStg ) return STG_E_MEDIUMFULL;

            hr = SaveTo_Impl( pNewStg );
            if ( FAILED( hr ) ) return STG_E_MEDIUMFULL;

            pMedium->tymed = TYMED_ISTORAGE;
            pMedium->pstg = pNewStg;
            pMedium->pstg->AddRef();
            pMedium->pUnkForRelease = static_cast<IUnknown*>(pNewStg);

            return S_OK;
        }
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP EmbedDocument_Impl::GetDataHere( FORMATETC * pFormatetc, STGMEDIUM * pMedium )
{
    if ( !pFormatetc )
        return DV_E_FORMATETC;

    if ( !pMedium )
        return STG_E_MEDIUMFULL;

    if ( pFormatetc->dwAspect == DVASPECT_THUMBNAIL
      || pFormatetc->dwAspect == DVASPECT_ICON
      || pFormatetc->dwAspect == DVASPECT_DOCPRINT )
        return DV_E_DVASPECT;

    CLIPFORMAT cf_embSource = (CLIPFORMAT)RegisterClipboardFormatW( L"Embed Source" );
    CLIPFORMAT cf_embObj = (CLIPFORMAT)RegisterClipboardFormatW( L"Embedded Object" );

    if ( pFormatetc->cfFormat == cf_embSource || pFormatetc->cfFormat == cf_embObj )
    {
        if ( !( pFormatetc->tymed & TYMED_ISTORAGE ) )
            return DV_E_TYMED;

        if ( !pMedium->pstg ) return STG_E_MEDIUMFULL;

        HRESULT hr = SaveTo_Impl( pMedium->pstg );
        if ( FAILED( hr ) ) return STG_E_MEDIUMFULL;

        pMedium->tymed = TYMED_ISTORAGE;
        pMedium->pUnkForRelease = nullptr;

        return S_OK;
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP EmbedDocument_Impl::QueryGetData( FORMATETC * pFormatetc )
{
    if ( pFormatetc )
    {
        if ( pFormatetc->dwAspect == DVASPECT_THUMBNAIL
          || pFormatetc->dwAspect == DVASPECT_ICON
          || pFormatetc->dwAspect == DVASPECT_DOCPRINT )
            return DV_E_DVASPECT;

        if ( pFormatetc->cfFormat == CF_ENHMETAFILE )
        {
            if ( !( pFormatetc->tymed & TYMED_ENHMF ) )
                return DV_E_TYMED;

            return S_OK;
        }
        else if ( pFormatetc->cfFormat == CF_METAFILEPICT )
        {
              if ( !( pFormatetc->tymed & TYMED_MFPICT ) )
                return DV_E_TYMED;

            return S_OK;
        }
        else
        {
            CLIPFORMAT cf_embSource = (CLIPFORMAT)RegisterClipboardFormatW( L"Embed Source" );
            CLIPFORMAT cf_embObj = (CLIPFORMAT)RegisterClipboardFormatW( L"Embedded Object" );
            if ( pFormatetc->cfFormat == cf_embSource || pFormatetc->cfFormat == cf_embObj )
            {
                if ( !( pFormatetc->tymed & TYMED_ISTORAGE ) )
                    return DV_E_TYMED;

                return S_OK;
            }
        }
    }

    return DV_E_FORMATETC;

}

STDMETHODIMP EmbedDocument_Impl::GetCanonicalFormatEtc( FORMATETC * pFormatetcIn, FORMATETC * pFormatetcOut )
{
    if ( !pFormatetcIn || !pFormatetcOut )
        return DV_E_FORMATETC;

    pFormatetcOut->ptd = nullptr;
    pFormatetcOut->cfFormat = pFormatetcIn->cfFormat;
    pFormatetcOut->dwAspect = DVASPECT_CONTENT;

    if ( pFormatetcIn->cfFormat == CF_ENHMETAFILE )
    {
        pFormatetcOut->tymed = TYMED_ENHMF;
        return S_OK;
    }
    else if ( pFormatetcIn->cfFormat == CF_METAFILEPICT )
    {
          pFormatetcOut->tymed = TYMED_MFPICT;
        return S_OK;
    }
    else
    {
        CLIPFORMAT cf_embSource = (CLIPFORMAT)RegisterClipboardFormatW( L"Embed Source" );
        CLIPFORMAT cf_embObj = (CLIPFORMAT)RegisterClipboardFormatW( L"Embedded Object" );
        if ( pFormatetcIn->cfFormat == cf_embSource || pFormatetcIn->cfFormat == cf_embObj )
        {
            pFormatetcOut->tymed = TYMED_ISTORAGE;
            return S_OK;
        }
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP EmbedDocument_Impl::SetData( FORMATETC * /*pFormatetc*/, STGMEDIUM * /*pMedium*/, BOOL /*fRelease*/ )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC ** /*ppFormatetc*/ )
{
    if ( dwDirection == DATADIR_GET )
        return OLE_S_USEREG;

    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::DAdvise( FORMATETC * pFormatetc, DWORD advf, IAdviseSink * pAdvSink, DWORD * pdwConnection )
{
    if ( !m_pDAdviseHolder )
        if ( !SUCCEEDED( CreateDataAdviseHolder( &m_pDAdviseHolder ) ) || !m_pDAdviseHolder )
            return E_OUTOFMEMORY;

    return m_pDAdviseHolder->Advise( static_cast<IDataObject*>(this), pFormatetc, advf, pAdvSink, pdwConnection );
}

STDMETHODIMP EmbedDocument_Impl::DUnadvise( DWORD dwConnection )
{
    if ( !m_pDAdviseHolder )
        if ( !SUCCEEDED( CreateDataAdviseHolder( &m_pDAdviseHolder ) ) || !m_pDAdviseHolder )
            return E_OUTOFMEMORY;

    return m_pDAdviseHolder->Unadvise( dwConnection );
}

STDMETHODIMP EmbedDocument_Impl::EnumDAdvise( IEnumSTATDATA ** ppenumAdvise )
{
    if ( !m_pDAdviseHolder )
        if ( !SUCCEEDED( CreateDataAdviseHolder( &m_pDAdviseHolder ) ) || !m_pDAdviseHolder )
            return E_OUTOFMEMORY;

    return m_pDAdviseHolder->EnumAdvise( ppenumAdvise );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
