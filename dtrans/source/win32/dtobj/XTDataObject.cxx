/*************************************************************************
 *
 *  $RCSfile: XTDataObject.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-02 17:40:28 $
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
// a helper class that will be thrown by the function validateFormatEtc
//------------------------------------------------------------------------

class CInvalidFormatEtcException
{
public:
    HRESULT m_hr;
    CInvalidFormatEtcException( HRESULT hr ) : m_hr( hr ) {};
};

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

STDMETHODIMP CXTDataObject::GetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium )
{
    if ( !(pFormatetc && pmedium) )
        return E_INVALIDARG;

    try
    {
        // prepare data transfer
        invalidateStgMedium( *pmedium );
        validateFormatEtc( pFormatetc );

        // handle locale request, because locale is a artificial format for us
        if ( CF_LOCALE == pFormatetc->cfFormat )
            transferLocaleToClipbAndSetupStgMedium( *pFormatetc, *pmedium );
        else if ( CF_UNICODETEXT == pFormatetc->cfFormat )
            transferUnicodeToClipbAndSetupStgMedium( *pFormatetc, *pmedium );
        else
            transferAnyDataToClipbAndSetupStgMedium( *pFormatetc, *pmedium );
    }
    catch(UnsupportedFlavorException&)
    {
        if ( isSynthesizeableFormat( pFormatetc ) )
        {
            // synthesize format
            return S_OK;
        }

        return DV_E_FORMATETC;
    }
    catch( CInvalidFormatEtcException& ex )
    {
        return ex.m_hr;
    }
    catch( CStgTransferException& ex )
    {
        return translateStgExceptionCode( ex.m_hr );
    }
    catch(...)
    {
        return E_UNEXPECTED;
    }

    return S_OK;
}

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

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

// inline
void SAL_CALL CXTDataObject::transferDataToStorageAndSetupStgMedium(
    const sal_Int8* lpStorage, const FORMATETC& fetc, sal_uInt32 nInitStgSize,
    sal_uInt32 nBytesToTransfer, STGMEDIUM& stgmedium )
{
    OSL_PRECOND( !nInitStgSize || nInitStgSize && (nInitStgSize >= nBytesToTransfer),
                 "Memory size less than number of bytes to transfer" );

    // if the client wants the data only via IStream we setup the storage transfer
    // helper so that the stream will not be released on destruction
    sal_Bool bShouldReleaseStream =
        ((fetc.tymed & TYMED_HGLOBAL) || !(fetc.tymed & TYMED_ISTREAM));

    CStgTransferHelper stgTransfHelper(
        AUTO_INIT, NULL, sal_False, bShouldReleaseStream );

    // setup storage size
    if ( nInitStgSize > 0 )
        stgTransfHelper.init(
            nInitStgSize, GMEM_MOVEABLE | GMEM_ZEROINIT, sal_False, bShouldReleaseStream );

#ifndef _DEBUG
    stgTransfHelper.write( lpStorage, nBytesToTransfer );
#else
    sal_uInt32 nBytesWritten = 0;
    stgTransfHelper.write( lpStorage, nBytesToTransfer, &nBytesWritten );
    OSL_ASSERT( nBytesWritten == nBytesToTransfer );
#endif

    setupStgMedium( fetc, stgTransfHelper, stgmedium );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

//inline
void SAL_CALL CXTDataObject::transferLocaleToClipbAndSetupStgMedium(
    FORMATETC& fetc, STGMEDIUM& stgmedium )
{
    if ( m_FormatRegistrar.hasSynthesizedLocale( ) )
    {
        LCID lcid = m_FormatRegistrar.getSynthesizedLocale( );
        transferDataToStorageAndSetupStgMedium(
            reinterpret_cast< sal_Int8* >( &lcid ),
            fetc,
            0,
            sizeof( LCID ),
            stgmedium );
    }
    else
        throw CInvalidFormatEtcException( DV_E_FORMATETC );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

//inline
void SAL_CALL CXTDataObject::transferUnicodeToClipbAndSetupStgMedium(
    FORMATETC& fetc, STGMEDIUM& stgmedium )
{
    DataFlavor aFlavor = formatEtcToDataFlavor( fetc );

    OUString aText;
    m_XTransferable->getTransferData( aFlavor ) >>= aText;

    sal_uInt32 nBytesToTransfer = aText.getLength( ) * sizeof( sal_Unicode );

    // to be sure there is an ending 0
    sal_uInt32 nRequiredMemSize = nBytesToTransfer + sizeof( sal_Unicode );

    transferDataToStorageAndSetupStgMedium(
        reinterpret_cast< const sal_Int8* >( aText.getStr( ) ),
        fetc,
        nRequiredMemSize,
        nBytesToTransfer,
        stgmedium );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

//inline
void SAL_CALL CXTDataObject::transferAnyDataToClipbAndSetupStgMedium(
    FORMATETC& fetc, STGMEDIUM& stgmedium )
{
    DataFlavor aFlavor = formatEtcToDataFlavor( fetc );

    Sequence< sal_Int8 > clipDataStream;
    m_XTransferable->getTransferData( aFlavor ) >>= clipDataStream;

    sal_uInt32 nRequiredMemSize = 0;
    if ( isOemOrAnsiTextFormat( fetc.cfFormat ) )
        nRequiredMemSize = sizeof( sal_Int8 ) * clipDataStream.getLength( ) + 1;

    // prepare data for transmision
    if ( CF_DIB == fetc.cfFormat )
        clipDataStream = OOBmpToWinDIB( clipDataStream );

    // transfer data
    if ( CF_METAFILEPICT == fetc.cfFormat )
    {
        HMETAFILE hMfPict = OOMFPictToWinMFPict( clipDataStream );
        transferDataToStorageAndSetupStgMedium(
            reinterpret_cast< sal_Int8* >( &hMfPict ),
            fetc,
            0,
            sizeof( HMETAFILE ),
            stgmedium );
    }
    else
        transferDataToStorageAndSetupStgMedium(
            clipDataStream.getArray( ),
            fetc,
            nRequiredMemSize,
            clipDataStream.getLength( ),
            stgmedium );
}

//------------------------------------------------------------------------
// IDataObject->EnumFormatEtc
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::EnumFormatEtc(
    DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc )
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
    HRESULT hr = E_INVALIDARG;

    if ( NULL != pFormatetc )
        hr = m_FormatEtcContainer.hasFormatEtc( *pFormatetc ) ? S_OK : S_FALSE;

    return hr;
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
DataFlavor SAL_CALL CXTDataObject::formatEtcToDataFlavor( const FORMATETC& aFormatEtc ) const
{
    DataFlavor aFlavor =
        m_DataFormatTranslator.getDataFlavorFromFormatEtc( m_XTransferable, aFormatEtc );

    if ( !aFlavor.MimeType.getLength( ) )
        throw UnsupportedFlavorException( );

    return aFlavor;
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

inline
void CXTDataObject::validateFormatEtc( LPFORMATETC lpFormatEtc ) const
{
    OSL_ASSERT( lpFormatEtc );

    if ( lpFormatEtc->lindex != -1 )
        throw CInvalidFormatEtcException( DV_E_LINDEX );

    if ( !(lpFormatEtc->dwAspect & DVASPECT_CONTENT) /*&&
         !(lpFormatEtc->dwAspect & DVASPECT_COPY) &&
         !(lpFormatEtc->dwAspect & DVASPECT_LINK)*/ &&
         !(lpFormatEtc->dwAspect & DVASPECT_SHORTNAME) )
        throw CInvalidFormatEtcException( DV_E_DVASPECT );

    if ( !(lpFormatEtc->tymed & TYMED_HGLOBAL) &&
         !(lpFormatEtc->tymed & TYMED_ISTREAM) &&
         !(lpFormatEtc->tymed & TYMED_MFPICT) &&
         !(lpFormatEtc->tymed & TYMED_ENHMF) )
        throw CInvalidFormatEtcException( DV_E_TYMED );

    if ( lpFormatEtc->cfFormat == CF_METAFILEPICT &&
         !(lpFormatEtc->tymed & TYMED_MFPICT) )
        throw CInvalidFormatEtcException( DV_E_TYMED );

    if ( lpFormatEtc->cfFormat == CF_ENHMETAFILE &&
         !(lpFormatEtc->tymed & TYMED_ENHMF) )
        throw CInvalidFormatEtcException( DV_E_TYMED );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

//inline
void SAL_CALL CXTDataObject::setupStgMedium( const FORMATETC& fetc,
                                             CStgTransferHelper& stgTransHlp,
                                             STGMEDIUM& stgmedium )
{
    stgmedium.pUnkForRelease = NULL;

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

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
void SAL_CALL CXTDataObject::invalidateStgMedium( STGMEDIUM& stgmedium ) const
{
    stgmedium.tymed = TYMED_NULL;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
sal_Bool SAL_CALL CXTDataObject::isOemOrAnsiTextFormat( const CLIPFORMAT& aClipformat ) const
{
    return ( (aClipformat == CF_TEXT) || (aClipformat == CF_OEMTEXT) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
HRESULT SAL_CALL CXTDataObject::translateStgExceptionCode( HRESULT hr ) const
{
    HRESULT hrTransl;

    switch( hr )
    {
    case STG_E_MEDIUMFULL:
        hrTransl = hr;
        break;

    default:
        hrTransl = E_UNEXPECTED;
        break;
    }

    return hrTransl;
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
