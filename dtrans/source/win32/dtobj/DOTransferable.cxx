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

#include "DOTransferable.hxx"
#include "../misc/ImplHelper.hxx"
#include "../misc/WinClip.hxx"
#include "DTransHelper.hxx"
#include "../misc/ImplHelper.hxx"
#include "TxtCnvtHlp.hxx"
#include "MimeAttrib.hxx"
#include "FmtFilter.hxx"
#include "Fetc.hxx"


#if(_MSC_VER < 1300) && !defined(__MINGW32__)
#include <olestd.h>
#endif

#define STR2(x) #x
#define STR(x) STR2(x)
#define PRAGMA_MSG( msg ) message( __FILE__ "(" STR(__LINE__) "): " #msg )

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace std;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

using ::rtl::OUString;

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

namespace
{
    const Type CPPUTYPE_SEQINT8  = getCppuType( ( Sequence< sal_Int8 >* )0 );
    const Type CPPUTYPE_OUSTRING = getCppuType( (OUString*)0 );

    inline
    sal_Bool isValidFlavor( const DataFlavor& aFlavor )
    {
        return ( aFlavor.MimeType.getLength( ) &&
                 ( ( aFlavor.DataType ==  CPPUTYPE_SEQINT8 ) ||
                 ( aFlavor.DataType == CPPUTYPE_OUSTRING ) ) );
    }

} // end namespace


//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------

CDOTransferable::CDOTransferable(
    const Reference< XMultiServiceFactory >& ServiceManager, IDataObjectPtr rDataObject ) :
    m_rDataObject( rDataObject ),
    m_SrvMgr( ServiceManager ),
    m_DataFormatTranslator( m_SrvMgr ),
    m_bUnicodeRegistered( sal_False ),
    m_TxtFormatOnClipboard( CF_INVALID )
{
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

Any SAL_CALL CDOTransferable::getTransferData( const DataFlavor& aFlavor )
        throw( UnsupportedFlavorException, IOException, RuntimeException )
{
    OSL_ASSERT( isValidFlavor( aFlavor ) );

    MutexGuard aGuard( m_aMutex );

    //------------------------------------------------
    // convert dataflavor to formatetc
    //------------------------------------------------

    CFormatEtc fetc = m_DataFormatTranslator.getFormatEtcFromDataFlavor( aFlavor );
    OSL_ASSERT( CF_INVALID != fetc.getClipformat() );

    //------------------------------------------------
    //  get the data from clipboard in a byte stream
    //------------------------------------------------

    ByteSequence_t clipDataStream;

    try
    {
        clipDataStream = getClipboardData( fetc );
    }
    catch( UnsupportedFlavorException& )
    {
        if ( m_DataFormatTranslator.isUnicodeTextFormat( fetc.getClipformat( ) ) &&
             m_bUnicodeRegistered )
        {
             OUString aUnicodeText = synthesizeUnicodeText( );
             Any aAny = makeAny( aUnicodeText );
             return aAny;
        }
        else
            throw; // pass through exception
    }

    //------------------------------------------------
    // return the data as any
    //------------------------------------------------

    return byteStreamToAny( clipDataStream, aFlavor.DataType );
}

//------------------------------------------------------------------------
// getTransferDataFlavors
//------------------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL CDOTransferable::getTransferDataFlavors(  )
    throw( RuntimeException )
{
    return m_FlavorList;
}

//------------------------------------------------------------------------
// isDataFlavorSupported
// returns true if we find a DataFlavor with the same MimeType and
// DataType
//------------------------------------------------------------------------

sal_Bool SAL_CALL CDOTransferable::isDataFlavorSupported( const DataFlavor& aFlavor )
    throw( RuntimeException )
{
    OSL_ASSERT( isValidFlavor( aFlavor ) );

    for ( sal_Int32 i = 0; i < m_FlavorList.getLength( ); i++ )
        if ( compareDataFlavors( aFlavor, m_FlavorList[i] ) )
            return sal_True;

    return sal_False;
}

//------------------------------------------------------------------------
// helper function
// the list of datafalvors currently on the clipboard will be initialized
// only once; if the client of this Transferable will hold a reference
// to it und the underlying clipboard content changes, the client does
// possible operate on a invalid list
// if there is only text on the clipboard we will also offer unicode text
// an synthesize this format on the fly if requested, to accomplish this
// we save the first offered text format which we will later use for the
// conversion
//------------------------------------------------------------------------

void SAL_CALL CDOTransferable::initFlavorList( )
{
    IEnumFORMATETCPtr pEnumFormatEtc;
    HRESULT hr = m_rDataObject->EnumFormatEtc( DATADIR_GET, &pEnumFormatEtc );
    if ( SUCCEEDED( hr ) )
    {
        pEnumFormatEtc->Reset( );

        FORMATETC fetc;
        while ( S_FALSE != pEnumFormatEtc->Next( 1, &fetc, NULL ) )
        {
            // we use locales only to determine the
            // charset if there is text on the cliboard
            // we don't offer this format
            if ( CF_LOCALE == fetc.cfFormat )
                continue;

            DataFlavor aFlavor = formatEtcToDataFlavor( fetc );

            // if text or oemtext is offered we also pretend to have unicode text
            if ( m_DataFormatTranslator.isOemOrAnsiTextFormat( fetc.cfFormat ) &&
                 !m_bUnicodeRegistered )
            {
                addSupportedFlavor( aFlavor );

                m_TxtFormatOnClipboard = fetc.cfFormat;
                m_bUnicodeRegistered   = sal_True;

                // register unicode text as accompany format
                aFlavor = formatEtcToDataFlavor(
                    m_DataFormatTranslator.getFormatEtcForClipformat( CF_UNICODETEXT ) );
                addSupportedFlavor( aFlavor );
            }
            else if ( (CF_UNICODETEXT == fetc.cfFormat) && !m_bUnicodeRegistered )
            {
                addSupportedFlavor( aFlavor );
                m_bUnicodeRegistered = sal_True;
            }
            else
                addSupportedFlavor( aFlavor );

            // see MSDN IEnumFORMATETC
            CoTaskMemFree( fetc.ptd );
        }
    }
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
void SAL_CALL CDOTransferable::addSupportedFlavor( const DataFlavor& aFlavor )
{
    // we ignore all formats that couldn't be translated
    if ( aFlavor.MimeType.getLength( ) )
    {
        OSL_ASSERT( isValidFlavor( aFlavor ) );

        m_FlavorList.realloc( m_FlavorList.getLength( ) + 1 );
        m_FlavorList[m_FlavorList.getLength( ) - 1] = aFlavor;
    }
}

//------------------------------------------------------------------------
// helper function
//------------------------------------------------------------------------

//inline
DataFlavor SAL_CALL CDOTransferable::formatEtcToDataFlavor( const FORMATETC& aFormatEtc )
{
    DataFlavor aFlavor;
    LCID lcid = 0;

    // for non-unicode text format we must provid a locale to get
    // the character-set of the text, if there is no locale on the
    // clipboard we assume the text is in a charset appropriate for
    // the current thread locale
    if ( (CF_TEXT == aFormatEtc.cfFormat) || (CF_OEMTEXT == aFormatEtc.cfFormat) )
        lcid = getLocaleFromClipboard( );

    return m_DataFormatTranslator.getDataFlavorFromFormatEtc( aFormatEtc, lcid );
}

//------------------------------------------------------------------------
// returns the current locale on clipboard; if there is no locale on
// clipboard the function returns the current thread locale
//------------------------------------------------------------------------

LCID SAL_CALL CDOTransferable::getLocaleFromClipboard( )
{
    LCID lcid = GetThreadLocale( );

    try
    {
        CFormatEtc fetc = m_DataFormatTranslator.getFormatEtcForClipformat( CF_LOCALE );
        ByteSequence_t aLCIDSeq = getClipboardData( fetc );
        lcid = *(reinterpret_cast<LCID*>( aLCIDSeq.getArray( ) ) );

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

//------------------------------------------------------------------------
// i think it's not necessary to call ReleaseStgMedium
// in case of failures because nothing should have been
// allocated etc.
//------------------------------------------------------------------------

CDOTransferable::ByteSequence_t SAL_CALL CDOTransferable::getClipboardData( CFormatEtc& aFormatEtc )
{
    STGMEDIUM stgmedium;
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
            if ( CF_DIB == aFormatEtc.getClipformat() )
                byteStream = WinDIBToOOBMP( byteStream );
            else if ( CF_METAFILEPICT == aFormatEtc.getClipformat() )
                byteStream = WinMFPictToOOMFPict( byteStream );
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

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CDOTransferable::synthesizeUnicodeText( )
{
    ByteSequence_t aTextSequence;
    CFormatEtc     fetc;
    LCID           lcid = getLocaleFromClipboard( );
    sal_uInt32     cpForTxtCnvt = 0;

    if ( CF_TEXT == m_TxtFormatOnClipboard )
    {
        fetc = m_DataFormatTranslator.getFormatEtcForClipformat( CF_TEXT );
        aTextSequence = getClipboardData( fetc );

        // determine the codepage used for text conversion
        cpForTxtCnvt = getWinCPFromLocaleId( lcid, LOCALE_IDEFAULTANSICODEPAGE ).toInt32( );
    }
    else if ( CF_OEMTEXT == m_TxtFormatOnClipboard )
    {
        fetc = m_DataFormatTranslator.getFormatEtcForClipformat( CF_OEMTEXT );
        aTextSequence = getClipboardData( fetc );

        // determine the codepage used for text conversion
        cpForTxtCnvt = getWinCPFromLocaleId( lcid, LOCALE_IDEFAULTCODEPAGE ).toInt32( );
    }
    else
        OSL_ASSERT( sal_False );

    CStgTransferHelper stgTransferHelper;

    // convert the text
    MultiByteToWideCharEx( cpForTxtCnvt,
                           reinterpret_cast<char*>( aTextSequence.getArray( ) ),
                           sal::static_int_cast<sal_uInt32>(-1), // Huh ?
                           stgTransferHelper,
                           sal_False);

    CRawHGlobalPtr  ptrHGlob(stgTransferHelper);
    sal_Unicode*    pWChar = reinterpret_cast<sal_Unicode*>(ptrHGlob.GetMemPtr());

    return OUString(pWChar);
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CDOTransferable::clipDataToByteStream( CLIPFORMAT cf, STGMEDIUM stgmedium, ByteSequence_t& aByteSequence )
{
    CStgTransferHelper memTransferHelper;

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
        #ifdef _MSC_VER
        #pragma PRAGMA_MSG( Has to be implemented )
        #endif
        break;

    default:
        throw UnsupportedFlavorException( );
        break;
    }

    int nMemSize = memTransferHelper.memSize( cf );
    aByteSequence.realloc( nMemSize );
    memTransferHelper.read( aByteSequence.getArray( ), nMemSize );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
Any CDOTransferable::byteStreamToAny( ByteSequence_t& aByteStream, const Type& aRequestedDataType )
{
    Any aAny;

    if ( aRequestedDataType == CPPUTYPE_OUSTRING )
    {
        OUString str = byteStreamToOUString( aByteStream );
        aAny = makeAny( str );
    }
    else
        aAny = makeAny( aByteStream );

    return aAny;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

inline
OUString CDOTransferable::byteStreamToOUString( ByteSequence_t& aByteStream )
{
    sal_Int32 nWChars;
    sal_Int32 nMemSize = aByteStream.getLength( );

    // if there is a trailing L"\0" substract 1 from length
    if ( 0 == aByteStream[ aByteStream.getLength( ) - 2 ] &&
         0 == aByteStream[ aByteStream.getLength( ) - 1 ] )
        nWChars = static_cast< sal_Int32 >( nMemSize / sizeof( sal_Unicode ) ) - 1;
    else
        nWChars = static_cast< sal_Int32 >( nMemSize / sizeof( sal_Unicode ) );

    return OUString( reinterpret_cast< sal_Unicode* >( aByteStream.getArray( ) ), nWChars );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CDOTransferable::compareDataFlavors(
    const DataFlavor& lhs, const DataFlavor& rhs )
{
    if ( !m_rXMimeCntFactory.is( ) )
    {
        m_rXMimeCntFactory = Reference< XMimeContentTypeFactory >( m_SrvMgr->createInstance(
            OUString("com.sun.star.datatransfer.MimeContentTypeFactory") ), UNO_QUERY );
    }
    OSL_ASSERT( m_rXMimeCntFactory.is( ) );

    sal_Bool bRet = sal_False;

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
        bRet = sal_False;
    }

    return bRet;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CDOTransferable::cmpFullMediaType(
    const Reference< XMimeContentType >& xLhs, const Reference< XMimeContentType >& xRhs ) const
{
    return xLhs->getFullMediaType().equalsIgnoreAsciiCase( xRhs->getFullMediaType( ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CDOTransferable::cmpAllContentTypeParameter(
    const Reference< XMimeContentType >& xLhs, const Reference< XMimeContentType >& xRhs ) const
{
    Sequence< OUString > xLhsFlavors = xLhs->getParameters( );
    Sequence< OUString > xRhsFlavors = xRhs->getParameters( );
    sal_Bool bRet = sal_True;

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
                    bRet = sal_False;
                    break;
                }
            }
        }
        else
            bRet = sal_False;
    }
    catch( NoSuchElementException& )
    {
        bRet = sal_False;
    }
    catch( IllegalArgumentException& )
    {
        bRet = sal_False;
    }

    return bRet;
}

::com::sun::star::uno::Any SAL_CALL CDOTransferable::getData( const Sequence< sal_Int8>& aProcessId  )
        throw (::com::sun::star::uno::RuntimeException)
{
    Any retVal;

    sal_uInt8 * arProcCaller= (sal_uInt8*)(sal_Int8*) aProcessId.getConstArray();
    sal_uInt8 arId[16];
    rtl_getGlobalProcessId(arId);
    if( ! memcmp( arId, arProcCaller,16))
    {
        if (m_rDataObject.is())
        {
            IDataObject* pObj= m_rDataObject.get();
            pObj->AddRef();
            retVal.setValue( &pObj, getCppuType((sal_uInt32*)0));
        }
    }
    return retVal;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
