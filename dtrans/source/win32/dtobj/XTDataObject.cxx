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

#include <osl/diagnose.h>

#include "XTDataObject.hxx"
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include "../misc/ImplHelper.hxx"
#include "DTransHelper.hxx"
#include "TxtCnvtHlp.hxx"
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include "com/sun/star/awt/AsyncCallback.hpp"
#include "com/sun/star/awt/XCallback.hpp"
#include "FmtFilter.hxx"
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>

#if defined _MSC_VER
#pragma warning(push,1)
#pragma warning(disable:4917)
#endif
#include <windows.h>
#include <shlobj.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#ifdef __MINGW32__
#if defined __uuidof
#undef __uuidof
#endif
#define __uuidof(I) IID_##I
#endif

// namespace directives

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace
{

/**
   We need to destroy XTransferable in the main thread to avoid dead lock
   when locking in the clipboard thread. So we transfer the ownership of the
   XTransferable reference to this object and release it when the callback
   is executed in main thread.
*/
class AsyncDereference : public cppu::WeakImplHelper<css::awt::XCallback>
{
    Reference<XTransferable> maTransferable;

public:
    AsyncDereference(css::uno::Reference<css::datatransfer::XTransferable> const & rTransferable)
        : maTransferable(rTransferable)
    {}

    virtual void SAL_CALL notify(css::uno::Any const &)
        throw (css::uno::RuntimeException, std::exception) override
    {
        maTransferable.set(nullptr);
    }
};

}

// a helper class that will be thrown by the function validateFormatEtc

class CInvalidFormatEtcException
{
public:
    HRESULT m_hr;
    explicit CInvalidFormatEtcException( HRESULT hr ) : m_hr( hr ) {};
};

// ctor

CXTDataObject::CXTDataObject( const Reference< XComponentContext >& rxContext,
                              const Reference< XTransferable >& aXTransferable )
    : m_nRefCnt( 0 )
    , m_XTransferable( aXTransferable )
    , m_XComponentContext( rxContext )
    , m_bFormatEtcContainerInitialized( sal_False )
    , m_DataFormatTranslator( rxContext )
    , m_FormatRegistrar( rxContext, m_DataFormatTranslator )
{
}

CXTDataObject::~CXTDataObject()
{
    css::awt::AsyncCallback::create(m_XComponentContext)->addCallback(
        new AsyncDereference(m_XTransferable),
        css::uno::Any());
}

// IUnknown->QueryInterface

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

// IUnknown->AddRef

STDMETHODIMP_(ULONG) CXTDataObject::AddRef( )
{
    return static_cast< ULONG >( InterlockedIncrement( &m_nRefCnt ) );
}

// IUnknown->Release

STDMETHODIMP_(ULONG) CXTDataObject::Release( )
{
    ULONG nRefCnt =
        static_cast< ULONG >( InterlockedDecrement( &m_nRefCnt ) );

    if ( 0 == nRefCnt )
        delete this;

    return nRefCnt;
}

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
            renderLocaleAndSetupStgMedium( *pFormatetc, *pmedium );
        else if ( CF_UNICODETEXT == pFormatetc->cfFormat )
            renderUnicodeAndSetupStgMedium( *pFormatetc, *pmedium );
        else
            renderAnyDataAndSetupStgMedium( *pFormatetc, *pmedium );
    }
    catch(UnsupportedFlavorException&)
    {
        HRESULT hr = DV_E_FORMATETC;

        CFormatEtc aFormatetc(*pFormatetc);
        if (m_FormatRegistrar.isSynthesizeableFormat(aFormatetc))
            hr = renderSynthesizedFormatAndSetupStgMedium( *pFormatetc, *pmedium );

        return hr;
    }
    catch( CInvalidFormatEtcException& ex )
    {
        return ex.m_hr;
    }
    catch( CStgTransferHelper::CStgTransferException& ex )
    {
        return translateStgExceptionCode( ex.m_hr );
    }
    catch(...)
    {
        return E_UNEXPECTED;
    }

    return S_OK;
}

// inline
void SAL_CALL CXTDataObject::renderDataAndSetupStgMedium(
    const sal_Int8* lpStorage, const FORMATETC& fetc, sal_uInt32 nInitStgSize,
    sal_uInt32 nBytesToTransfer, STGMEDIUM& stgmedium )
{
    OSL_PRECOND( !nInitStgSize || nInitStgSize && (nInitStgSize >= nBytesToTransfer),
                 "Memory size less than number of bytes to transfer" );

    CStgTransferHelper stgTransfHelper( AUTO_INIT );

    // setup storage size
    if ( nInitStgSize > 0 )
        stgTransfHelper.init( nInitStgSize, GHND );

#if OSL_DEBUG_LEVEL > 0
    sal_uInt32 nBytesWritten = 0;
    stgTransfHelper.write( lpStorage, nBytesToTransfer, &nBytesWritten );
    OSL_ASSERT( nBytesWritten == nBytesToTransfer );
#else
    stgTransfHelper.write( lpStorage, nBytesToTransfer );
#endif

    setupStgMedium( fetc, stgTransfHelper, stgmedium );
}

//inline
void SAL_CALL CXTDataObject::renderLocaleAndSetupStgMedium(
    FORMATETC& fetc, STGMEDIUM& stgmedium )
{
    if ( m_FormatRegistrar.hasSynthesizedLocale( ) )
    {
        LCID lcid = m_FormatRegistrar.getSynthesizedLocale( );
        renderDataAndSetupStgMedium(
            reinterpret_cast< sal_Int8* >( &lcid ),
            fetc,
            0,
            sizeof( LCID ),
            stgmedium );
    }
    else
        throw CInvalidFormatEtcException( DV_E_FORMATETC );
}

//inline
void SAL_CALL CXTDataObject::renderUnicodeAndSetupStgMedium(
    FORMATETC& fetc, STGMEDIUM& stgmedium )
{
    DataFlavor aFlavor = formatEtcToDataFlavor( fetc );

    Any aAny = m_XTransferable->getTransferData( aFlavor );

    // unfortunately not all transferables fulfill the
    // spec. an do throw an UnsupportedFlavorException
    // so we must check the any
    if ( !aAny.hasValue( ) )
    {
        OSL_FAIL( "XTransferable should throw an exception if ask for an unsupported flavor" );
        throw UnsupportedFlavorException( );
    }

    OUString aText;
    aAny >>= aText;

    sal_uInt32 nBytesToTransfer = aText.getLength( ) * sizeof( sal_Unicode );

    // to be sure there is an ending 0
    sal_uInt32 nRequiredMemSize = nBytesToTransfer + sizeof( sal_Unicode );

    renderDataAndSetupStgMedium(
        reinterpret_cast< const sal_Int8* >( aText.getStr( ) ),
        fetc,
        nRequiredMemSize,
        nBytesToTransfer,
        stgmedium );
}

//inline
void SAL_CALL CXTDataObject::renderAnyDataAndSetupStgMedium(
    FORMATETC& fetc, STGMEDIUM& stgmedium )
{
    DataFlavor aFlavor = formatEtcToDataFlavor( fetc );

    Any aAny = m_XTransferable->getTransferData( aFlavor );

    // unfortunately not all transferables fulfill the
    // spec. an do throw an UnsupportedFlavorException
    // so we must check the any
    if ( !aAny.hasValue( ) )
    {
        OSL_FAIL( "XTransferable should throw an exception if ask for an unsupported flavor" );
        throw UnsupportedFlavorException( );
    }

    // unfortunately not all transferables fulfill the
    // spec. an do throw an UnsupportedFlavorException
    // so we must check the any
    if ( !aAny.hasValue( ) )
        throw UnsupportedFlavorException( );

    Sequence< sal_Int8 > clipDataStream;
    aAny >>= clipDataStream;

    sal_uInt32 nRequiredMemSize = 0;
    if ( m_DataFormatTranslator.isOemOrAnsiTextFormat( fetc.cfFormat ) )
        nRequiredMemSize = sizeof( sal_Int8 ) * clipDataStream.getLength( ) + 1;

    // prepare data for transmision
    // #i124085# DIBV5 should not happen for now, but keep as hint here
    if ( CF_DIBV5 == fetc.cfFormat || CF_DIB == fetc.cfFormat )
    {
#ifdef DBG_UTIL
        if(CF_DIBV5 == fetc.cfFormat)
        {
            OSL_ENSURE(sal_uInt32(clipDataStream.getLength()) > (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPV5HEADER)), "Wrong size on CF_DIBV5 data (!)");
        }
        else // CF_DIB == fetc.cfFormat
        {
            OSL_ENSURE(sal_uInt32(clipDataStream.getLength()) > (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)), "Wrong size on CF_DIB data (!)");
        }
#endif

        // remove BITMAPFILEHEADER
        clipDataStream = OOBmpToWinDIB( clipDataStream );
    }

    if ( CF_METAFILEPICT == fetc.cfFormat )
    {
        stgmedium.tymed          = TYMED_MFPICT;
        stgmedium.hMetaFilePict  = OOMFPictToWinMFPict( clipDataStream );
        stgmedium.pUnkForRelease = NULL;
    }
    else if( CF_ENHMETAFILE == fetc.cfFormat )
    {
        stgmedium.tymed          = TYMED_ENHMF;
        stgmedium.hMetaFilePict  = OOMFPictToWinENHMFPict( clipDataStream );
        stgmedium.pUnkForRelease = NULL;
    }
    else
        renderDataAndSetupStgMedium(
            clipDataStream.getArray( ),
            fetc,
            nRequiredMemSize,
            clipDataStream.getLength( ),
            stgmedium );
}

HRESULT SAL_CALL CXTDataObject::renderSynthesizedFormatAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium )
{
    HRESULT hr = S_OK;

    try
    {
        if ( CF_UNICODETEXT == fetc.cfFormat )
            // the transferable seems to have only text
            renderSynthesizedUnicodeAndSetupStgMedium( fetc, stgmedium );
        else if ( m_DataFormatTranslator.isOemOrAnsiTextFormat( fetc.cfFormat ) )
            // the transferable seems to have only unicode text
            renderSynthesizedTextAndSetupStgMedium( fetc, stgmedium );
        else
            // the transferable seems to have only text/html
            renderSynthesizedHtmlAndSetupStgMedium( fetc, stgmedium );
    }
    catch(UnsupportedFlavorException&)
    {
        hr = DV_E_FORMATETC;
    }
    catch( CInvalidFormatEtcException& )
    {
        OSL_FAIL( "Unexpected exception" );
    }
    catch( CStgTransferHelper::CStgTransferException& ex )
    {
        return translateStgExceptionCode( ex.m_hr );
    }
    catch(...)
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

// the transferable must have only text, so we will synthesize unicode text

void SAL_CALL CXTDataObject::renderSynthesizedUnicodeAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium )
{
    OSL_ASSERT( CF_UNICODETEXT == fetc.cfFormat );

    Any aAny = m_XTransferable->getTransferData( m_FormatRegistrar.getRegisteredTextFlavor( ) );

    // unfortunately not all transferables fulfill the
    // spec. an do throw an UnsupportedFlavorException
    // so we must check the any
    if ( !aAny.hasValue( ) )
    {
        OSL_FAIL( "XTransferable should throw an exception if ask for an unsupported flavor" );
        throw UnsupportedFlavorException( );
    }

    Sequence< sal_Int8 > aText;
    aAny >>= aText;

    CStgTransferHelper stgTransfHelper;

    MultiByteToWideCharEx(
        m_FormatRegistrar.getRegisteredTextCodePage( ),
        reinterpret_cast< char* >( aText.getArray( ) ),
        aText.getLength( ),
        stgTransfHelper );

    setupStgMedium( fetc, stgTransfHelper, stgmedium );
}

// the transferable must have only unicode text so we will sythesize text

void SAL_CALL CXTDataObject::renderSynthesizedTextAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium )
{
    OSL_ASSERT( m_DataFormatTranslator.isOemOrAnsiTextFormat( fetc.cfFormat ) );

    DataFlavor aFlavor = formatEtcToDataFlavor(
        m_DataFormatTranslator.getFormatEtcForClipformat( CF_UNICODETEXT ) );

    Any aAny = m_XTransferable->getTransferData( aFlavor );

    // unfortunately not all transferables fulfill the
    // spec. an do throw an UnsupportedFlavorException
    // so we must check the any
    if ( !aAny.hasValue( ) )
    {
        OSL_FAIL( "XTransferable should throw an exception if ask for an unsupported flavor" );
        throw UnsupportedFlavorException( );
    }

    OUString aUnicodeText;
    aAny >>= aUnicodeText;

    CStgTransferHelper stgTransfHelper;

    WideCharToMultiByteEx(
        GetACP( ),
        reinterpret_cast<LPCWSTR>( aUnicodeText.getStr( ) ),
        aUnicodeText.getLength( ),
        stgTransfHelper );

    setupStgMedium( fetc, stgTransfHelper, stgmedium );
}

void SAL_CALL CXTDataObject::renderSynthesizedHtmlAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium )
{
    OSL_ASSERT( m_DataFormatTranslator.isHTMLFormat( fetc.cfFormat ) );

    DataFlavor aFlavor;

    // creating a DataFlavor on the fly
    aFlavor.MimeType = "text/html";
    aFlavor.DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();

    Any aAny = m_XTransferable->getTransferData( aFlavor );

    // unfortunately not all transferables fulfill the
    // spec. an do throw an UnsupportedFlavorException
    // so we must check the any
    if ( !aAny.hasValue( ) )
    {
        OSL_FAIL( "XTransferable should throw an exception if ask for an unsupported flavor" );
        throw UnsupportedFlavorException( );
    }

    Sequence< sal_Int8 > aTextHtmlSequence;
    aAny >>= aTextHtmlSequence;

    Sequence< sal_Int8 > aHTMLFormatSequence = TextHtmlToHTMLFormat( aTextHtmlSequence );

    sal_uInt32 nBytesToTransfer = aHTMLFormatSequence.getLength( );

    renderDataAndSetupStgMedium(
        reinterpret_cast< const sal_Int8* >( aHTMLFormatSequence.getArray( ) ),
        fetc,
        0,
        nBytesToTransfer,
        stgmedium );
}

// IDataObject->EnumFormatEtc

STDMETHODIMP CXTDataObject::EnumFormatEtc(
    DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc )
{
    if ( NULL == ppenumFormatetc )
        return E_INVALIDARG;

    if ( DATADIR_SET == dwDirection )
        return E_NOTIMPL;

    *ppenumFormatetc = NULL;

    InitializeFormatEtcContainer( );

    HRESULT hr;
    if ( DATADIR_GET == dwDirection )
    {
        *ppenumFormatetc = new CEnumFormatEtc( this, m_FormatEtcContainer );
        static_cast< LPUNKNOWN >( *ppenumFormatetc )->AddRef( );

        hr = S_OK;
    }
    else
        hr = E_INVALIDARG;

    return hr;
}

// IDataObject->QueryGetData

STDMETHODIMP CXTDataObject::QueryGetData( LPFORMATETC pFormatetc )
{
    if ( (NULL == pFormatetc) || IsBadReadPtr( pFormatetc, sizeof( FORMATETC ) ) )
        return E_INVALIDARG;

    InitializeFormatEtcContainer( );

    CFormatEtc aFormatetc(*pFormatetc);
    return m_FormatEtcContainer.hasFormatEtc(aFormatetc) ? S_OK : S_FALSE;
}

// IDataObject->GetDataHere

STDMETHODIMP CXTDataObject::GetDataHere( LPFORMATETC, LPSTGMEDIUM )
{
    return E_NOTIMPL;
}

// IDataObject->GetCanonicalFormatEtc

STDMETHODIMP CXTDataObject::GetCanonicalFormatEtc( LPFORMATETC, LPFORMATETC )
{
    return E_NOTIMPL;
}

// IDataObject->SetData

STDMETHODIMP CXTDataObject::SetData( LPFORMATETC, LPSTGMEDIUM, BOOL )
{
    return E_NOTIMPL;
}

// IDataObject->DAdvise

STDMETHODIMP CXTDataObject::DAdvise( LPFORMATETC, DWORD, LPADVISESINK, DWORD * )
{
    return E_NOTIMPL;
}

// IDataObject->DUnadvise

STDMETHODIMP CXTDataObject::DUnadvise( DWORD )
{
    return E_NOTIMPL;
}

// IDataObject->EnumDAdvise

STDMETHODIMP CXTDataObject::EnumDAdvise( LPENUMSTATDATA * )
{
    return E_NOTIMPL;
}

// for our convenience

CXTDataObject::operator IDataObject*( )
{
    return static_cast< IDataObject* >( this );
}

inline
DataFlavor SAL_CALL CXTDataObject::formatEtcToDataFlavor( const FORMATETC& aFormatEtc ) const
{
    DataFlavor aFlavor;

    if ( m_FormatRegistrar.hasSynthesizedLocale( ) )
        aFlavor =
            m_DataFormatTranslator.getDataFlavorFromFormatEtc( aFormatEtc, m_FormatRegistrar.getSynthesizedLocale( ) );
    else
        aFlavor = m_DataFormatTranslator.getDataFlavorFromFormatEtc( aFormatEtc );

    if ( !aFlavor.MimeType.getLength( ) )
        throw UnsupportedFlavorException( );

    return aFlavor;
}

inline
void CXTDataObject::validateFormatEtc( LPFORMATETC lpFormatEtc ) const
{
    OSL_ASSERT( lpFormatEtc );

    if ( lpFormatEtc->lindex != -1 )
        throw CInvalidFormatEtcException( DV_E_LINDEX );

    if ( !(lpFormatEtc->dwAspect & DVASPECT_CONTENT) &&
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

//inline
void SAL_CALL CXTDataObject::setupStgMedium( const FORMATETC& fetc,
                                             CStgTransferHelper& stgTransHlp,
                                             STGMEDIUM& stgmedium )
{
    stgmedium.pUnkForRelease = NULL;

    if ( fetc.cfFormat == CF_METAFILEPICT )
    {
        stgmedium.tymed         = TYMED_MFPICT;
        stgmedium.hMetaFilePict = static_cast< HMETAFILEPICT >( stgTransHlp.getHGlobal( ) );
    }
    else if ( fetc.cfFormat == CF_ENHMETAFILE )
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

inline
void SAL_CALL CXTDataObject::invalidateStgMedium( STGMEDIUM& stgmedium ) const
{
    stgmedium.tymed = TYMED_NULL;
}

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

inline void SAL_CALL CXTDataObject::InitializeFormatEtcContainer( )
{
    if ( !m_bFormatEtcContainerInitialized )
    {
        m_FormatRegistrar.RegisterFormats( m_XTransferable, m_FormatEtcContainer );
        m_bFormatEtcContainerInitialized = sal_True;
    }
}

// CEnumFormatEtc

// ctor

CEnumFormatEtc::CEnumFormatEtc( LPUNKNOWN lpUnkOuter, const CFormatEtcContainer& aFormatEtcContainer ) :
    m_nRefCnt( 0 ),
    m_lpUnkOuter( lpUnkOuter ),
    m_FormatEtcContainer( aFormatEtcContainer )
{
    Reset( );
}

// IUnknown->QueryInterface

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

// IUnknown->AddRef

STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef( )
{
    // keep the dataobject alive
    m_lpUnkOuter->AddRef( );
    return InterlockedIncrement( &m_nRefCnt );
}

// IUnknown->Release

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release( )
{
    // release the outer dataobject
    m_lpUnkOuter->Release( );

    ULONG nRefCnt = InterlockedDecrement( &m_nRefCnt );
    if ( 0 == nRefCnt )
        delete this;

    return nRefCnt;
}

// IEnumFORMATETC->Next

STDMETHODIMP CEnumFormatEtc::Next( ULONG nRequested, LPFORMATETC lpDest, ULONG* lpFetched )
{
    if ( ( nRequested < 1 ) ||
         (( nRequested > 1 ) && ( NULL == lpFetched )) ||
         IsBadWritePtr( lpDest, sizeof( FORMATETC ) * nRequested ) )
        return E_INVALIDARG;

    sal_uInt32 nFetched = m_FormatEtcContainer.nextFormatEtc( lpDest, nRequested );

    if ( NULL != lpFetched )
        *lpFetched = nFetched;

    return (nFetched == nRequested) ? S_OK : S_FALSE;
}

// IEnumFORMATETC->Skip

STDMETHODIMP CEnumFormatEtc::Skip( ULONG celt )
{
    return m_FormatEtcContainer.skipFormatEtc( celt ) ? S_OK : S_FALSE;
}

// IEnumFORMATETC->Reset

STDMETHODIMP CEnumFormatEtc::Reset( )
{
    m_FormatEtcContainer.beginEnumFormatEtc( );
    return S_OK;
}

// IEnumFORMATETC->Clone

STDMETHODIMP CEnumFormatEtc::Clone( IEnumFORMATETC** ppenum )
{
    if ( NULL == ppenum )
        return E_INVALIDARG;

    *ppenum = new CEnumFormatEtc( m_lpUnkOuter, m_FormatEtcContainer );
    static_cast< LPUNKNOWN >( *ppenum )->AddRef( );

    return S_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
