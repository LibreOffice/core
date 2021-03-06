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

#include <sal/types.h>
#include <rtl/process.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include "DOTransferable.hxx"
#include "ImplHelper.hxx"
#include "WinClip.hxx"
#include "WinClipboard.hxx"
#include "DTransHelper.hxx"
#include "TxtCnvtHlp.hxx"
#include "MimeAttrib.hxx"
#include "FmtFilter.hxx"
#include "Fetc.hxx"
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/datatransfer/MimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace std;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

namespace
{
    const Type CPPUTYPE_SEQINT8  = cppu::UnoType<Sequence< sal_Int8 >>::get();
    const Type CPPUTYPE_OUSTRING = cppu::UnoType<OUString>::get();

    bool isValidFlavor( const DataFlavor& aFlavor )
    {
        return ( aFlavor.MimeType.getLength( ) &&
                 ( ( aFlavor.DataType ==  CPPUTYPE_SEQINT8 ) ||
                 ( aFlavor.DataType == CPPUTYPE_OUSTRING ) ) );
    }

void clipDataToByteStream( CLIPFORMAT cf, STGMEDIUM stgmedium, CDOTransferable::ByteSequence_t& aByteSequence )
{
    CStgTransferHelper memTransferHelper;
    LPSTREAM pStream = nullptr;

    switch( stgmedium.tymed )
    {
    case TYMED_HGLOBAL:
        memTransferHelper.init( stgmedium.hGlobal );
        break;

    case TYMED_MFPICT:
        memTransferHelper.init( stgmedium.hMetaFilePict );
        break;

    case TYMED_ENHMF:
        memTransferHelper.init( stgmedium.hEnhMetaFile );
        break;

    case TYMED_ISTREAM:
        pStream = stgmedium.pstm;
        break;

    default:
        throw UnsupportedFlavorException( );
        break;
    }

    if (pStream)
    {
        // We have a stream, read from it.
        STATSTG aStat;
        HRESULT hr = pStream->Stat(&aStat, STATFLAG_NONAME);
        if (FAILED(hr))
        {
            SAL_WARN("vcl.win.dtrans", "clipDataToByteStream: Stat() failed");
            return;
        }

        size_t nMemSize = aStat.cbSize.QuadPart;
        aByteSequence.realloc(nMemSize);
        LARGE_INTEGER li;
        li.QuadPart = 0;
        hr = pStream->Seek(li, STREAM_SEEK_SET, nullptr);
        if (FAILED(hr))
        {
            SAL_WARN("vcl.win.dtrans", "clipDataToByteStream: Seek() failed");
        }

        ULONG nRead = 0;
        hr = pStream->Read(aByteSequence.getArray(), nMemSize, &nRead);
        if (FAILED(hr))
        {
            SAL_WARN("vcl.win.dtrans", "clipDataToByteStream: Read() failed");
        }
        if (nRead < nMemSize)
        {
            SAL_WARN("vcl.win.dtrans", "clipDataToByteStream: Read() was partial");
        }

        return;
    }

    int nMemSize = memTransferHelper.memSize( cf );
    aByteSequence.realloc( nMemSize );
    memTransferHelper.read( aByteSequence.getArray( ), nMemSize );
}

OUString byteStreamToOUString( CDOTransferable::ByteSequence_t& aByteStream )
{
    sal_Int32 nWChars;
    sal_Int32 nMemSize = aByteStream.getLength( );

    // if there is a trailing L"\0" subtract 1 from length
    // for unknown reason, the sequence may sometimes arrive empty
    if ( aByteStream.getLength( ) > 1 &&
         0 == aByteStream[ aByteStream.getLength( ) - 2 ] &&
         0 == aByteStream[ aByteStream.getLength( ) - 1 ] )
        nWChars = static_cast< sal_Int32 >( nMemSize / sizeof( sal_Unicode ) ) - 1;
    else
        nWChars = static_cast< sal_Int32 >( nMemSize / sizeof( sal_Unicode ) );

    return OUString( reinterpret_cast< sal_Unicode* >( aByteStream.getArray( ) ), nWChars );
}

Any byteStreamToAny( CDOTransferable::ByteSequence_t& aByteStream, const Type& aRequestedDataType )
{
    Any aAny;

    if ( aRequestedDataType == CPPUTYPE_OUSTRING )
    {
        OUString str = byteStreamToOUString( aByteStream );
        if (str.isEmpty())
            throw RuntimeException();
        aAny <<= str;
    }
    else
        aAny <<= aByteStream;

    return aAny;
}

bool cmpFullMediaType(
    const Reference< XMimeContentType >& xLhs, const Reference< XMimeContentType >& xRhs )
{
    return xLhs->getFullMediaType().equalsIgnoreAsciiCase( xRhs->getFullMediaType( ) );
}

bool cmpAllContentTypeParameter(
    const Reference< XMimeContentType >& xLhs, const Reference< XMimeContentType >& xRhs )
{
    Sequence< OUString > xLhsFlavors = xLhs->getParameters( );
    Sequence< OUString > xRhsFlavors = xRhs->getParameters( );
    bool bRet = true;

    try
    {
        if ( xLhsFlavors.getLength( ) == xRhsFlavors.getLength( ) )
        {
            OUString pLhs;
            OUString pRhs;

            for ( sal_Int32 i = 0; i < xLhsFlavors.getLength( ); i++ )
            {
                pLhs = xLhs->getParameterValue( xLhsFlavors[i] );
                pRhs = xRhs->getParameterValue( xLhsFlavors[i] );

                if ( !pLhs.equalsIgnoreAsciiCase( pRhs ) )
                {
                    bRet = false;
                    break;
                }
            }
        }
        else
            bRet = false;
    }
    catch( NoSuchElementException& )
    {
        bRet = false;
    }
    catch( IllegalArgumentException& )
    {
        bRet = false;
    }

    return bRet;
}

} // end namespace

CDOTransferable::CDOTransferable(
    const Reference< XComponentContext >& rxContext, IDataObjectPtr rDataObject ) :
    m_rDataObject( rDataObject ),
    m_xContext( rxContext ),
    m_DataFormatTranslator( rxContext ),
    m_bUnicodeRegistered( false ),
    m_TxtFormatOnClipboard( CF_INVALID )
{
    initFlavorList();
}

CDOTransferable::CDOTransferable(
    const Reference<XComponentContext>& rxContext,
    const css::uno::Reference<css::datatransfer::clipboard::XClipboard>& xClipboard,
    const std::vector<sal_uInt32>& rFormats)
    : m_xClipboard(xClipboard)
    , m_xContext(rxContext)
    , m_DataFormatTranslator(rxContext)
    , m_bUnicodeRegistered(false)
    , m_TxtFormatOnClipboard(CF_INVALID)
{
    initFlavorListFromFormatList(rFormats);
}

Any SAL_CALL CDOTransferable::getTransferData( const DataFlavor& aFlavor )
{
    OSL_ASSERT( isValidFlavor( aFlavor ) );

    MutexGuard aGuard( m_aMutex );

    // convert dataflavor to formatetc

    CFormatEtc fetc = m_DataFormatTranslator.getFormatEtcFromDataFlavor( aFlavor );
    OSL_ASSERT( CF_INVALID != fetc.getClipformat() );

    //  get the data from clipboard in a byte stream

    ByteSequence_t clipDataStream;

    try
    {
        clipDataStream = getClipboardData( fetc );
    }
    catch( UnsupportedFlavorException& )
    {
        if ( CDataFormatTranslator::isUnicodeTextFormat( fetc.getClipformat( ) ) &&
             m_bUnicodeRegistered )
        {
             OUString aUnicodeText = synthesizeUnicodeText( );
             Any aAny = makeAny( aUnicodeText );
             return aAny;
        }
        // #i124085# CF_DIBV5 should not be possible, but keep for reading from the
        // clipboard for being on the safe side
        else if(CF_DIBV5 == fetc.getClipformat())
        {
            // #i123407# CF_DIBV5 has priority; if the try to fetch this failed,
            // check CF_DIB availability as an alternative
            fetc.setClipformat(CF_DIB);

            clipDataStream = getClipboardData( fetc );
                // pass UnsupportedFlavorException out, tried all possibilities
        }
        else
            throw; // pass through exception
    }

    // return the data as any

    return byteStreamToAny( clipDataStream, aFlavor.DataType );
}

// getTransferDataFlavors

Sequence< DataFlavor > SAL_CALL CDOTransferable::getTransferDataFlavors(  )
{
    return m_FlavorList;
}

// isDataFlavorSupported
// returns true if we find a DataFlavor with the same MimeType and
// DataType

sal_Bool SAL_CALL CDOTransferable::isDataFlavorSupported( const DataFlavor& aFlavor )
{
    OSL_ASSERT( isValidFlavor( aFlavor ) );

    for ( DataFlavor const & df : std::as_const(m_FlavorList) )
        if ( compareDataFlavors( aFlavor, df ) )
            return true;

    return false;
}

// the list of dataflavors currently on the clipboard will be initialized
// only once; if the client of this Transferable will hold a reference
// to it and the underlying clipboard content changes, the client does
// possible operate on an invalid list
// if there is only text on the clipboard we will also offer unicode text
// an synthesize this format on the fly if requested, to accomplish this
// we save the first offered text format which we will later use for the
// conversion

void CDOTransferable::initFlavorList( )
{
    std::vector<sal_uInt32> aFormats;
    sal::systools::COMReference<IEnumFORMATETC> pEnumFormatEtc;
    HRESULT hr = m_rDataObject->EnumFormatEtc( DATADIR_GET, &pEnumFormatEtc );
    if ( SUCCEEDED( hr ) )
    {
        pEnumFormatEtc->Reset( );

        FORMATETC fetc;
        while ( S_OK == pEnumFormatEtc->Next( 1, &fetc, nullptr ) )
        {
            aFormats.push_back(fetc.cfFormat);
            // see MSDN IEnumFORMATETC
            CoTaskMemFree( fetc.ptd );
        }
        initFlavorListFromFormatList(aFormats);
    }
}

void CDOTransferable::initFlavorListFromFormatList(const std::vector<sal_uInt32>& rFormats)
{
    for (sal_uInt32 cfFormat : rFormats)
    {
        // we use locales only to determine the
        // charset if there is text on the cliboard
        // we don't offer this format
        if (CF_LOCALE == cfFormat)
            continue;

        // if text or oemtext is offered we pretend to have unicode text
        if (CDataFormatTranslator::isTextFormat(cfFormat))
        {
            if (!m_bUnicodeRegistered)
            {
                m_TxtFormatOnClipboard = cfFormat;
                m_bUnicodeRegistered   = true;

                // register unicode text as format
                addSupportedFlavor(formatEtcToDataFlavor(CF_UNICODETEXT));
            }
        }
        else
            addSupportedFlavor(formatEtcToDataFlavor(cfFormat));
    }
}

inline
void CDOTransferable::addSupportedFlavor( const DataFlavor& aFlavor )
{
    // we ignore all formats that couldn't be translated
    if ( aFlavor.MimeType.getLength( ) )
    {
        OSL_ASSERT( isValidFlavor( aFlavor ) );

        m_FlavorList.realloc( m_FlavorList.getLength( ) + 1 );
        m_FlavorList[m_FlavorList.getLength( ) - 1] = aFlavor;
    }
}

DataFlavor CDOTransferable::formatEtcToDataFlavor(sal_uInt32 cfFormat)
{
    return m_DataFormatTranslator.getDataFlavorFromFormatEtc(cfFormat);
}

// returns the current locale on clipboard; if there is no locale on
// clipboard the function returns the current thread locale

LCID CDOTransferable::getLocaleFromClipboard( )
{
    LCID lcid = GetThreadLocale( );

    try
    {
        CFormatEtc fetc = CDataFormatTranslator::getFormatEtcForClipformat( CF_LOCALE );
        ByteSequence_t aLCIDSeq = getClipboardData( fetc );
        lcid = *reinterpret_cast<LCID*>( aLCIDSeq.getArray( ) );

        // because of a Win95/98 Bug; there the high word
        // of a locale has the same value as the
        // low word e.g. 0x07040704 that's not right
        // correct is 0x00000704
        lcid &= 0x0000FFFF;
    }
    catch(...)
    {
        // we take the default locale
    }

    return lcid;
}

void CDOTransferable::tryToGetIDataObjectIfAbsent()
{
    if (!m_rDataObject.is())
    {
        auto xClipboard = m_xClipboard.get(); // holding the reference while we get the object
        if (CWinClipboard* pWinClipboard = dynamic_cast<CWinClipboard*>(xClipboard.get()))
        {
            m_rDataObject = pWinClipboard->getIDataObject();
        }
    }
}

// I think it's not necessary to call ReleaseStgMedium
// in case of failures because nothing should have been
// allocated etc.

CDOTransferable::ByteSequence_t CDOTransferable::getClipboardData( CFormatEtc& aFormatEtc )
{
    STGMEDIUM stgmedium;
    tryToGetIDataObjectIfAbsent();
    if (!m_rDataObject.is()) // Maybe we are shutting down, and clipboard is already destroyed?
        throw RuntimeException();
    HRESULT hr = m_rDataObject->GetData( aFormatEtc, &stgmedium );

    // in case of failure to get a WMF metafile handle, try to get a memory block
    if( FAILED( hr ) &&
        ( CF_METAFILEPICT == aFormatEtc.getClipformat() ) &&
        ( TYMED_MFPICT == aFormatEtc.getTymed() ) )
    {
        CFormatEtc aTempFormat( aFormatEtc );
        aTempFormat.setTymed( TYMED_HGLOBAL );
        hr = m_rDataObject->GetData( aTempFormat, &stgmedium );
    }

    if (FAILED(hr) && aFormatEtc.getTymed() == TYMED_HGLOBAL)
    {
        // Handle type is not memory, try stream.
        CFormatEtc aTempFormat(aFormatEtc);
        aTempFormat.setTymed(TYMED_ISTREAM);
        hr = m_rDataObject->GetData(aTempFormat, &stgmedium);
    }

    if ( FAILED( hr ) )
    {
        OSL_ASSERT( (hr != E_INVALIDARG) &&
                    (hr != DV_E_DVASPECT) &&
                    (hr != DV_E_LINDEX) &&
                    (hr != DV_E_TYMED) );

        if ( DV_E_FORMATETC == hr )
            throw UnsupportedFlavorException( );
        else if ( STG_E_MEDIUMFULL == hr )
            throw IOException( );
        else
            throw RuntimeException( );
    }

    ByteSequence_t byteStream;

    try
    {
        if ( CF_ENHMETAFILE == aFormatEtc.getClipformat() )
            byteStream = WinENHMFPictToOOMFPict( stgmedium.hEnhMetaFile );
        else if (CF_HDROP == aFormatEtc.getClipformat())
            byteStream = CF_HDROPToFileList(stgmedium.hGlobal);
        else if ( CF_BITMAP == aFormatEtc.getClipformat() )
        {
            byteStream = WinBITMAPToOOBMP(stgmedium.hBitmap);
            if( aFormatEtc.getTymed() == TYMED_GDI &&
                ! stgmedium.pUnkForRelease )
            {
                DeleteObject(stgmedium.hBitmap);
            }
        }
        else
        {
            clipDataToByteStream( aFormatEtc.getClipformat( ), stgmedium, byteStream );

            // format conversion if necessary
            // #i124085# DIBV5 should not happen currently, but keep as a hint here
            if(CF_DIBV5 == aFormatEtc.getClipformat() || CF_DIB == aFormatEtc.getClipformat())
            {
                byteStream = WinDIBToOOBMP(byteStream);
            }
            else if(CF_METAFILEPICT == aFormatEtc.getClipformat())
            {
                byteStream = WinMFPictToOOMFPict(byteStream);
            }
        }

        ReleaseStgMedium( &stgmedium );
    }
    catch( CStgTransferHelper::CStgTransferException& )
    {
        ReleaseStgMedium( &stgmedium );
        throw IOException( );
    }

    return byteStream;
}

OUString CDOTransferable::synthesizeUnicodeText( )
{
    ByteSequence_t aTextSequence;
    CFormatEtc     fetc;
    LCID           lcid = getLocaleFromClipboard( );
    sal_uInt32     cpForTxtCnvt = 0;

    if ( CF_TEXT == m_TxtFormatOnClipboard )
    {
        fetc = CDataFormatTranslator::getFormatEtcForClipformat( CF_TEXT );
        aTextSequence = getClipboardData( fetc );

        // determine the codepage used for text conversion
        cpForTxtCnvt = getWinCPFromLocaleId( lcid, LOCALE_IDEFAULTANSICODEPAGE ).toInt32( );
    }
    else if ( CF_OEMTEXT == m_TxtFormatOnClipboard )
    {
        fetc = CDataFormatTranslator::getFormatEtcForClipformat( CF_OEMTEXT );
        aTextSequence = getClipboardData( fetc );

        // determine the codepage used for text conversion
        cpForTxtCnvt = getWinCPFromLocaleId( lcid, LOCALE_IDEFAULTCODEPAGE ).toInt32( );
    }
    else
        OSL_ASSERT( false );

    CStgTransferHelper stgTransferHelper;

    // convert the text
    MultiByteToWideCharEx( cpForTxtCnvt,
                           reinterpret_cast<char*>( aTextSequence.getArray( ) ),
                           -1,
                           stgTransferHelper,
                           false);

    CRawHGlobalPtr  ptrHGlob(stgTransferHelper);
    sal_Unicode*    pWChar = static_cast<sal_Unicode*>(ptrHGlob.GetMemPtr());

    return OUString(pWChar);
}

bool CDOTransferable::compareDataFlavors(
    const DataFlavor& lhs, const DataFlavor& rhs )
{
    if ( !m_rXMimeCntFactory.is( ) )
    {
        m_rXMimeCntFactory = MimeContentTypeFactory::create( m_xContext );
    }

    bool bRet = false;

    try
    {
        Reference< XMimeContentType > xLhs( m_rXMimeCntFactory->createMimeContentType( lhs.MimeType ) );
        Reference< XMimeContentType > xRhs( m_rXMimeCntFactory->createMimeContentType( rhs.MimeType ) );

        if ( cmpFullMediaType( xLhs, xRhs ) )
        {
            bRet = cmpAllContentTypeParameter( xLhs, xRhs );
        }
    }
    catch( IllegalArgumentException& )
    {
        OSL_FAIL( "Invalid content type detected" );
        bRet = false;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
