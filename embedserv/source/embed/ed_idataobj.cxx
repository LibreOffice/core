/*************************************************************************
 *
 *  $RCSfile: ed_idataobj.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mav $ $Date: 2003-03-12 15:37:57 $
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

// actually this workaround should be in presys.h!
//#define UINT64 USE_WIN_UINT64
//#define INT64 USE_WIN_INT64
//#define UINT32 USE_WIN_UINT32
//#define INT32 USE_WIN_INT32

//#include <tools/presys.h>
#include "embeddoc.hxx"
//#include <tools/postsys.h>

//#undef UINT64
//#undef INT64
//#undef UINT32
//#undef INT32


#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif


#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

using namespace ::com::sun::star;

//===============================================================================
// EmbedDocument_Impl
//===============================================================================

sal_uInt64 EmbedDocument_Impl::getMetaFileHandle_Impl( sal_Bool isEnhMeta )
{
    sal_uInt64 pResult = NULL;

    uno::Reference< datatransfer::XTransferable > xTransferable( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
    if ( xTransferable.is() )
    {
        uno::Sequence< sal_Int8 > aMetaBuffer;
        datatransfer::DataFlavor aFlavor;

        if ( isEnhMeta )
        {
            aFlavor.MimeType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                "application/x-openoffice;windows_formatname=\"Image EMF\"" ) );
            aFlavor.HumanPresentableName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Enhanced Windows MetaFile" ) );
        }
        else
        {
            aFlavor.MimeType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                "application/x-openoffice;windows_formatname=\"Image WMF\"" ) );
            aFlavor.HumanPresentableName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Windows GDIMetaFile" ) );
        }

        aFlavor.DataType = getCppuType( (const sal_uInt64*) 0 );

        uno::Any aAny = xTransferable->getTransferData( aFlavor );
        aAny >>= pResult;
    }

    return pResult;
}

//-------------------------------------------------------------------------------
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

        HENHMETAFILE hMeta = reinterpret_cast<HENHMETAFILE>( getMetaFileHandle_Impl( sal_True ) );

        if ( hMeta )
        {
            pMedium->tymed = TYMED_ENHMF;
            pMedium->hEnhMetaFile = hMeta;
            pMedium->pUnkForRelease = NULL;

            return S_OK;
        }

        return STG_E_MEDIUMFULL;
    }
    else if ( pFormatetc->cfFormat == CF_METAFILEPICT )
    {
          if ( !( pFormatetc->tymed & TYMED_MFPICT ) )
            return DV_E_TYMED;

        HGLOBAL hMeta = reinterpret_cast<HGLOBAL>( getMetaFileHandle_Impl( sal_False ) );

        if ( hMeta )
        {
            pMedium->tymed = TYMED_MFPICT;
            pMedium->hMetaFilePict = hMeta;
            pMedium->pUnkForRelease = NULL;
        }

        return STG_E_MEDIUMFULL;
    }
    else
    {
        CLIPFORMAT cf_embSource = RegisterClipboardFormatA( "Embed Source" );
        if ( pFormatetc->cfFormat == cf_embSource )
        {
            if ( !( pFormatetc->tymed & TYMED_ISTORAGE ) )
                return DV_E_TYMED;

            CComPtr< IStorage > pNewStg;
            HRESULT hr = StgCreateDocfile( NULL, STGM_CREATE | STGM_READWRITE | STGM_DELETEONRELEASE, 0, &pNewStg );
            if ( FAILED( hr ) || !pNewStg ) return STG_E_MEDIUMFULL;

            hr = SaveTo_Impl( pNewStg );
            if ( FAILED( hr ) ) return STG_E_MEDIUMFULL;

            pMedium->tymed = TYMED_ISTORAGE;
            pMedium->pstg = pNewStg;
            pMedium->pstg->AddRef();
            pMedium->pUnkForRelease = ( IUnknown* )pNewStg;

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

    CLIPFORMAT cf_embSource = RegisterClipboardFormatA( "Embed Source" );
    if ( pFormatetc->cfFormat == cf_embSource )
    {
        if ( !( pFormatetc->tymed & TYMED_ISTORAGE ) )
            return DV_E_TYMED;

        if ( !pMedium->pstg ) return STG_E_MEDIUMFULL;

        HRESULT hr = SaveTo_Impl( pMedium->pstg );
        if ( FAILED( hr ) ) return STG_E_MEDIUMFULL;

        pMedium->tymed = TYMED_ISTORAGE;
        pMedium->pUnkForRelease = NULL;

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
            CLIPFORMAT cf_embSource = RegisterClipboardFormatA( "Embed Source" );
            if ( pFormatetc->cfFormat == cf_embSource )
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

    pFormatetcOut->ptd = NULL;
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
        CLIPFORMAT cf_embSource = RegisterClipboardFormatA( "Embed Source" );
        if ( pFormatetcIn->cfFormat == cf_embSource )
        {
            pFormatetcOut->tymed = TYMED_ISTORAGE;
            return S_OK;
        }
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP EmbedDocument_Impl::SetData( FORMATETC * pFormatetc, STGMEDIUM * pMedium, BOOL fRelease )
{
    return E_NOTIMPL;
}

STDMETHODIMP EmbedDocument_Impl::EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC ** ppFormatetc )
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

    return m_pDAdviseHolder->Advise( (IDataObject*)this, pFormatetc, advf, pAdvSink, pdwConnection );
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

