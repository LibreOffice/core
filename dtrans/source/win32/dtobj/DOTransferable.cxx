/*************************************************************************
 *
 *  $RCSfile: DOTransferable.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-05 13:10:39 $
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

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _DOWRAPPERTRANSFERABLE_HXX_
#include "DOTransferable.hxx"
#endif

#ifndef _IMPLHELPER_HXX_
#include "..\misc\ImplHelper.hxx"
#endif

#ifndef _WINCLIP_HXX_
#include "..\misc\WinClip.hxx"
#endif

#ifndef _DTRANSHELPER_HXX_
#include "DTransHelper.hxx"
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _MIMEATTRIB_HXX_
#include "MimeAttrib.hxx"
#endif

#ifndef _FMTFILTER_HXX_
#include "FmtFilter.hxx"
#endif

#ifndef _FETC_HXX_
#include "Fetc.hxx"
#endif

#include <olestd.h>

#define STR2(x) #x
#define STR(x) STR2(x)
#define PRAGMA_MSG( msg ) message( __FILE__ "(" STR(__LINE__) "): " #msg )

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace rtl;
using namespace std;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using CStgTransferHelper::CStgTransferException;

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
    m_DataFormatTranslator( m_SrvMgr )
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

    CFormatEtc fetc = dataFlavorToFormatEtc( aFlavor );
    OSL_ASSERT( CF_INVALID != fetc.getClipformat() );

    //------------------------------------------------
    //  get the data from clipboard in a byte stream
    //------------------------------------------------

    ByteSequence_t clipDataStream = getClipboardData( fetc );

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
            addSupportedFlavor( aFlavor );

            // see MSDN IEnumFORMATETC
            CoTaskMemFree( fetc.ptd );
        }
    }
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

//inline
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

inline
CFormatEtc SAL_CALL CDOTransferable::dataFlavorToFormatEtc( const DataFlavor& aFlavor ) const
{
    return m_DataFormatTranslator.getFormatEtcFromDataFlavor( aFlavor );
}

//------------------------------------------------------------------------
// helper function
//------------------------------------------------------------------------

//inline
DataFlavor SAL_CALL CDOTransferable::formatEtcToDataFlavor( const FORMATETC& aFormatEtc )
{
    return m_DataFormatTranslator.getDataFlavorFromFormatEtc( this, aFormatEtc );
}

//------------------------------------------------------------------------
// i think it's not necessary to call ReleaseStgMedium
// in case of failures because nothing should have been
// allocated etc.
//------------------------------------------------------------------------

CDOTransferable::ByteSequence_t CDOTransferable::getClipboardData( CFormatEtc& aFormatEtc )
{
    STGMEDIUM stgmedium;
    HRESULT hr = m_rDataObject->GetData( aFormatEtc, &stgmedium );

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
        clipDataToByteStream( stgmedium, byteStream );

        // format conversion if necessary
        if ( CF_DIB == aFormatEtc.getClipformat() )
            byteStream = WinDIBToOOBMP( byteStream );
        else if ( CF_METAFILEPICT == aFormatEtc.getClipformat() )
            byteStream = WinMFPictToOOMFPict( byteStream );

        ReleaseStgMedium( &stgmedium );
    }
    catch( CStgTransferException& )
    {
        ReleaseStgMedium( &stgmedium );
        throw IOException( );
    }

    return byteStream;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CDOTransferable::clipDataToByteStream( STGMEDIUM stgmedium, ByteSequence_t& aByteSequence )
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
        #pragma PRAGMA_MSG( Has to be implemented )
        break;

    default:
        throw UnsupportedFlavorException( );
        break;
    }

    int nMemSize = memTransferHelper.memSize( );
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
            OUString::createFromAscii( "com.sun.star.datatransfer.MimeContentTypeFactory" ) ), UNO_QUERY );
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
        OSL_ENSURE( sal_False, "Invalid content type detected" );
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
    return xLhs->getFullMediaType().equalsIgnoreCase( xRhs->getFullMediaType( ) );
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

                if ( !pLhs.equalsIgnoreCase( pRhs ) )
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