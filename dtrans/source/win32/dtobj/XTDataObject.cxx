/*************************************************************************
 *
 *  $RCSfile: XTDataObject.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-02-27 07:53:22 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _TWRAPPERDATAOBJECT_HXX_
#include "XTDataObject.hxx"
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/dataflavor.hpp>
#endif

#ifndef _IMPLHELPER_HXX_
#include "..\misc\ImplHelper.hxx"
#endif

#ifndef _DTRANSHELPER_HXX_
#include "DTransHelper.hxx"
#endif

#ifndef _TXTCNVTHLP_HXX_
#include "TxtCnvtHlp.hxx"
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDEX_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#endif

#ifndef _FMTFILTER_HXX_
#include "FmtFilter.hxx"
#endif

#include <windows.h>
#include <shlobj.h>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------

CXTDataObject::CXTDataObject( const Reference< XMultiServiceFactory >& aServiceManager,
                              const Reference< XTransferable >& aXTransferable ) :
    m_nRefCnt( 0 ),
    m_SrvMgr( aServiceManager ),
    m_XTransferable( aXTransferable ),
    m_DataFormatTranslator( aServiceManager ),
    m_FormatRegistrar( m_SrvMgr, m_DataFormatTranslator )
{
    m_FormatRegistrar.RegisterFormats( m_XTransferable->getTransferDataFlavors( ),
                                       m_FormatEtcContainer );
}

//------------------------------------------------------------------------
// IUnknown->QueryInterface
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::QueryInterface( REFIID iid, LPVOID* ppvObject )
{
    if ( NULL == ppvObject )
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;

    *ppvObject = NULL;
    if ( ( __uuidof( IUnknown ) == iid ) ||
         ( __uuidof( IDataObject ) == iid ) )
    {
        *ppvObject = static_cast< IUnknown* >( this );
        ( (LPUNKNOWN)*ppvObject )->AddRef( );
        hr = S_OK;
    }

    return hr;
}

//------------------------------------------------------------------------
// IUnknown->AddRef
//------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CXTDataObject::AddRef( )
{
    return static_cast< ULONG >( InterlockedIncrement( &m_nRefCnt ) );
}

//------------------------------------------------------------------------
// IUnknown->Release
//------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CXTDataObject::Release( )
{
    ULONG nRefCnt =
        static_cast< ULONG >( InterlockedDecrement( &m_nRefCnt ) );

    if ( 0 == nRefCnt )
        delete this;

    return nRefCnt;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool CXTDataObject::isRequestForLocale( LPFORMATETC lpFormatEtc ) const
{
    return ( CF_LOCALE == lpFormatEtc->cfFormat );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
DataFlavor SAL_CALL CXTDataObject::formatEtcToDataFlavor( const FORMATETC& aFormatEtc ) const
{
    return m_DataFormatTranslator.getDataFlavorFromFormatEtc( m_XTransferable, aFormatEtc );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CXTDataObject::isSynthesizeableFormat( LPFORMATETC lpFormatEtc ) const
{
    return ( (lpFormatEtc->cfFormat == CF_TEXT) ||
             (lpFormatEtc->cfFormat == CF_OEMTEXT) ||
             (lpFormatEtc->cfFormat == CF_UNICODETEXT) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::GetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium )
{
    if ( !(pFormatetc && pmedium) )
        return E_INVALIDARG;

    HRESULT hr = validateFormatEtc( pFormatetc );
    if ( FAILED( hr ) )
        return hr;

    DataFlavor aFlavor;

    try
    {
        if ( isRequestForLocale( pFormatetc ) )
        {
            if ( m_FormatRegistrar.hasSynthesizedLocale( ) )
            {
                LCID lcid = m_FormatRegistrar.getSynthesizedLocale( );
                TransferDataToStorageAndSetupStgMedium( reinterpret_cast< sal_Int8* >( &lcid ),
                                       *pFormatetc,
                                       0,
                                       sizeof( LCID ),
                                       *pmedium );
                return S_OK;
            }
            else
                return DV_E_FORMATETC;
        }

        aFlavor = formatEtcToDataFlavor( *pFormatetc );

        if ( CF_UNICODETEXT == pFormatetc->cfFormat )
        {
            OUString aText;
            m_XTransferable->getTransferData( aFlavor ) >>= aText;

            sal_uInt32 nRequiredMemSize = aText.getLength( ) * sizeof( sal_Unicode ) + 1;
            sal_uInt32 nBytesToTransfer = aText.getLength( ) * sizeof( sal_Unicode );
            hr = TransferDataToStorageAndSetupStgMedium( reinterpret_cast< const sal_Int8* >( aText.getStr( ) ),
                                   *pFormatetc,
                                   nRequiredMemSize,
                                   nBytesToTransfer,
                                   *pmedium );
        }
        else
        {
            Sequence< sal_Int8 > clipDataStream;
            m_XTransferable->getTransferData( aFlavor ) >>= clipDataStream;

            sal_uInt32 nRequiredMemSize = 0;
            if ( (CF_TEXT == pFormatetc->cfFormat) || (CF_OEMTEXT == pFormatetc->cfFormat) )
                nRequiredMemSize = sizeof( sal_Int8 ) * clipDataStream.getLength( ) + 1;

            // prepare data for transmision
            if ( CF_DIB == pFormatetc->cfFormat )
            {
                clipDataStream = OOBmpToWinDIB( clipDataStream );
            }

            if ( CF_METAFILEPICT == pFormatetc->cfFormat )
            {
                HMETAFILE hMfPict = OOMFPictToWinMFPict( clipDataStream );
                TransferDataToStorageAndSetupStgMedium( reinterpret_cast< sal_Int8* >( &hMfPict ),
                                       *pFormatetc,
                                       0,
                                       sizeof( HMETAFILE ),
                                       *pmedium );
            }
            else
                TransferDataToStorageAndSetupStgMedium( clipDataStream.getArray( ),
                                       *pFormatetc,
                                       nRequiredMemSize,
                                       clipDataStream.getLength( ),
                                       *pmedium );
        }
    }
    catch(UnsupportedFlavorException&)
    {
        if ( isSynthesizeableFormat( pFormatetc ) )
        {
            /*
            FORMATETC fetc;

            synthesize format
            if ( requested format is unicodetext ) // implies the transferable has really only text
            {
                fetc = m_DataFormatTranslator.getFormatEtcForClipformat( CF_TEXT );
                aFlavor = formatEtcToDataFlavor( fetc );

                get transfer data
                convert transfer data to unicode text
                transfer result to storage
            }
            else // implies the transferable has really only unicodetext
            {
                fetc = m_DataFormatTranslator.getFormatEtcForClipformat( CF_UNICODETEXT );
                aFlavor = formatEtcToDataFlavor( fetc );

                get transfer data
                if ( request is for ansi text )
                    convert transfer data to text using the CP_ACP
                else
                    convert transfer data to oemtext using CP_OEMCP

                transfer result to storage
            }

            transfer data to storage
            */

            return S_OK;
        }

        return DV_E_FORMATETC;
    }
    catch(COutOfMemoryException&)
    {
        return STG_E_MEDIUMFULL;
    }
    catch(...)
    {
        return E_UNEXPECTED;
    }

    return S_OK;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

HRESULT SAL_CALL CXTDataObject::TransferDataToStorageAndSetupStgMedium( const sal_Int8* lpStorage,
                                                                         const FORMATETC& fetc,
                                                                        sal_uInt32 nInitStgSize,
                                                                        sal_uInt32 nBytesToTransfer,
                                                                        STGMEDIUM& stgmedium )
{
    HRESULT hr = S_OK;

    if ( isValidTymedForClipformat( fetc ) )
    {
        // if the client wants the data only via IStream we setup the storage transfer
        // helper so that the stream will not be released on destruction
        sal_Bool bShouldReleaseStream =
            ( (fetc.tymed & TYMED_ISTREAM) && !(fetc.tymed & TYMED_HGLOBAL) );
        CStgTransferHelper stgTransfHelper( AUTO_INIT,
                                            NULL,
                                            sal_False,
                                            bShouldReleaseStream );

        // setup storage size
        if ( nInitStgSize > 0 )
            stgTransfHelper.init( nInitStgSize,
                                  GMEM_MOVEABLE | GMEM_ZEROINIT,
                                  sal_False,
                                  bShouldReleaseStream );

        sal_Bool bResult = stgTransfHelper.write( lpStorage, nBytesToTransfer );

        setupStgMedium( bResult, fetc, stgTransfHelper, stgmedium );
        if ( !bResult )
            hr = E_UNEXPECTED;
    }
    else
        hr = DV_E_FORMATETC;

    return hr;
}

//------------------------------------------------------------------------
// IDataObject->EnumFormatEtc
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::EnumFormatEtc( DWORD dwDirection,
                                           IEnumFORMATETC** ppenumFormatetc )
{
    if ( ( NULL == ppenumFormatetc ) || ( DATADIR_SET == dwDirection ) )
        return E_INVALIDARG;

    *ppenumFormatetc = NULL;

    if ( DATADIR_GET == dwDirection )
    {
        *ppenumFormatetc = new CEnumFormatEtc( this, m_FormatEtcContainer );
        if ( NULL != *ppenumFormatetc )
            static_cast< LPUNKNOWN >( *ppenumFormatetc )->AddRef( );
    }

    return ( NULL != *ppenumFormatetc ) ? S_OK : E_OUTOFMEMORY;
}

//------------------------------------------------------------------------
// IDataObject->QueryGetData
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::QueryGetData( LPFORMATETC pFormatetc )
{
    if ( NULL == pFormatetc )
        return E_INVALIDARG;

    return m_FormatEtcContainer.hasFormatEtc( *pFormatetc ) ? S_OK : S_FALSE;
}

//------------------------------------------------------------------------
// IDataObject->GetDataHere
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::GetDataHere( LPFORMATETC, LPSTGMEDIUM )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// IDataObject->GetCanonicalFormatEtc
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::GetCanonicalFormatEtc( LPFORMATETC, LPFORMATETC )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// IDataObject->SetData
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::SetData( LPFORMATETC, LPSTGMEDIUM, BOOL )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// IDataObject->DAdvise
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::DAdvise( LPFORMATETC, DWORD, LPADVISESINK, DWORD * )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// IDataObject->DUnadvise
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::DUnadvise( DWORD )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// IDataObject->EnumDAdvise
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::EnumDAdvise( LPENUMSTATDATA * )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// for our convenience
//------------------------------------------------------------------------

CXTDataObject::operator IDataObject*( )
{
    return static_cast< IDataObject* >( this );
}

//------------------------------------------------------------------------
// aks for the total size of all available and renderable clipboard
// formats
//------------------------------------------------------------------------

sal_Int64 SAL_CALL CXTDataObject::QueryDataSize( )
{
    return 0;
}

//------------------------------------------------------------------------
// in preparation of clipboard flushing transfer all data from remote
// to here
//------------------------------------------------------------------------

void SAL_CALL CXTDataObject::GetAllDataFromSource( )
{
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
STDMETHODIMP CXTDataObject::validateFormatEtc( LPFORMATETC lpFormatEtc ) const
{
    OSL_ASSERT( lpFormatEtc );

    if ( lpFormatEtc->lindex != -1 )
        return DV_E_LINDEX;

    if ( (lpFormatEtc->dwAspect != DVASPECT_CONTENT) ||
         (lpFormatEtc->dwAspect != DVASPECT_COPY) ||
         (lpFormatEtc->dwAspect != DVASPECT_LINK) ||
         (lpFormatEtc->dwAspect != DVASPECT_SHORTNAME) )
        return DV_E_DVASPECT;

    if ( (lpFormatEtc->tymed != TYMED_HGLOBAL) ||
         (lpFormatEtc->tymed != TYMED_ISTREAM) ||
         (lpFormatEtc->tymed != TYMED_MFPICT) ||
         (lpFormatEtc->tymed != TYMED_ENHMF) )
        return DV_E_TYMED;

    return S_OK;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
void SAL_CALL CXTDataObject::setupStgMedium( sal_Bool bTransferSuccessful,
                                             const FORMATETC& fetc,
                                             CStgTransferHelper& stgTransHlp,
                                             STGMEDIUM& stgmedium )
{
    stgmedium.pUnkForRelease = NULL;

    if ( bTransferSuccessful )
    {
        if ( fetc.tymed & TYMED_MFPICT )
        {
            stgmedium.tymed         = TYMED_MFPICT;
            stgmedium.hMetaFilePict = static_cast< HMETAFILEPICT >( stgTransHlp.getHGlobal( ) );
        }
        else if ( fetc.tymed & TYMED_ENHMF )
        {
            stgmedium.tymed        = TYMED_ENHMF;
            stgmedium.hEnhMetaFile = static_cast< HENHMETAFILE >( stgTransHlp.getHGlobal( ) );
        }
        else if ( fetc.tymed & TYMED_HGLOBAL )
        {
            stgmedium.tymed   = TYMED_HGLOBAL;
            stgmedium.hGlobal = stgTransHlp.getHGlobal( );
        }
        else if ( fetc.tymed & TYMED_ISTREAM )
        {
            stgmedium.tymed = TYMED_ISTREAM;
            stgTransHlp.getIStream( &stgmedium.pstm );
        }
        else
            OSL_ASSERT( sal_False );
    }
    else
        stgmedium.tymed = TYMED_NULL;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool SAL_CALL CXTDataObject::isValidTymedForClipformat( const FORMATETC& fetc ) const
{
    if ( fetc.cfFormat == CF_METAFILEPICT && !(fetc.tymed & TYMED_MFPICT) )
        return sal_False;

    if ( fetc.cfFormat == CF_ENHMETAFILE && !(fetc.tymed & TYMED_ENHMF) )
        return sal_False;

    if ( !(fetc.tymed & TYMED_HGLOBAL) ||
         !(fetc.tymed & TYMED_ISTREAM) )
         return sal_False;

    return sal_True;
}

//============================================================================
// CEnumFormatEtc
//============================================================================

//----------------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------------

CEnumFormatEtc::CEnumFormatEtc( LPUNKNOWN lpUnkOuter, const CFormatEtcContainer& aFormatEtcContainer ) :
    m_nRefCnt( 0 ),
    m_lpUnkOuter( lpUnkOuter ),
    m_FormatEtcContainer( aFormatEtcContainer )
{
}

//----------------------------------------------------------------------------
// IUnknown->QueryInterface
//----------------------------------------------------------------------------

STDMETHODIMP CEnumFormatEtc::QueryInterface( REFIID iid, LPVOID* ppvObject )
{
    if ( NULL == ppvObject )
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;

    *ppvObject = NULL;

    if ( ( __uuidof( IUnknown ) == iid ) ||
         ( __uuidof( IEnumFORMATETC ) == iid ) )
    {
        *ppvObject = static_cast< IUnknown* >( this );
        static_cast< LPUNKNOWN >( *ppvObject )->AddRef( );
        hr = S_OK;
    }

    return hr;
}

//----------------------------------------------------------------------------
// IUnknown->AddRef
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef( )
{
    // keep the dataobject alive
    m_lpUnkOuter->AddRef( );
    return InterlockedIncrement( &m_nRefCnt );
}

//----------------------------------------------------------------------------
// IUnknown->Release
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release( )
{
    // release the outer dataobject
    m_lpUnkOuter->Release( );

    ULONG nRefCnt = InterlockedDecrement( &m_nRefCnt );
    if ( 0 == nRefCnt )
        delete this;

    return nRefCnt;
}

//----------------------------------------------------------------------------
// IEnumFORMATETC->Next
//----------------------------------------------------------------------------

STDMETHODIMP CEnumFormatEtc::Next( ULONG nRequested, LPFORMATETC lpDest, ULONG* lpFetched )
{
    if ( (( 0 != nRequested ) && ( NULL == lpDest )) ||
         IsBadWritePtr( lpDest, sizeof( FORMATETC ) * nRequested ) )
        return E_INVALIDARG;

    sal_uInt32 nFetched = m_FormatEtcContainer.nextFormatEtc( lpDest, nRequested );

    if ( lpFetched )
        *lpFetched = nFetched;

    return nFetched == nRequested ? S_OK : S_FALSE;
}

//----------------------------------------------------------------------------
// IEnumFORMATETC->Skip
//----------------------------------------------------------------------------

STDMETHODIMP CEnumFormatEtc::Skip( ULONG celt )
{
    return m_FormatEtcContainer.skipFormatEtc( celt ) ? S_OK : S_FALSE;
}

//----------------------------------------------------------------------------
// IEnumFORMATETC->Reset
//----------------------------------------------------------------------------

STDMETHODIMP CEnumFormatEtc::Reset( )
{
    m_FormatEtcContainer.beginEnumFormatEtc( );
    return S_OK;
}

//----------------------------------------------------------------------------
// IEnumFORMATETC->Clone
//----------------------------------------------------------------------------

STDMETHODIMP CEnumFormatEtc::Clone( IEnumFORMATETC** ppenum )
{
    if ( NULL == ppenum )
        return E_INVALIDARG;

    *ppenum = NULL;

    CEnumFormatEtc* pCEnumFEtc =
        new CEnumFormatEtc( m_lpUnkOuter, m_FormatEtcContainer );
    if ( NULL != pCEnumFEtc )
    {
        *ppenum = static_cast< IEnumFORMATETC* >( pCEnumFEtc );
        static_cast< LPUNKNOWN >( *ppenum )->AddRef( );
    }

    return ( NULL != *ppenum ) ? S_OK : E_OUTOFMEMORY;
}
