/*************************************************************************
 *
 *  $RCSfile: transfer.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-16 17:47:03 $
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

#ifdef WNT
#include <tools/prewin.h>
#include <shlobj.h>
#include <tools/postwin.h>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _FILELIST_HXX
#include <sot/filelist.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XFLUSHABLECLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#include "urlbmk.hxx"
#include "imap.hxx"
#include "transfer.hxx"
#include "inetimg.hxx"

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;

// --------------------------------
// - TransferableObjectDescriptor -
// --------------------------------

SvStream& operator>>( SvStream& rIStm, TransferableObjectDescriptor& rObjDesc )
{
    sal_uInt32 nSize;

    rIStm >> nSize;
    rIStm >> rObjDesc.maClassName;
    rIStm >> rObjDesc.mnViewAspect;
    rIStm >> rObjDesc.maSize.Width();
    rIStm >> rObjDesc.maSize.Height();
    rIStm >> rObjDesc.maDragStartPos.X();
    rIStm >> rObjDesc.maDragStartPos.Y();
    rIStm.ReadByteString( rObjDesc.maTypeName, gsl_getSystemTextEncoding() );
    rIStm.ReadByteString( rObjDesc.maDisplayName, gsl_getSystemTextEncoding() );

    return rIStm;
}

// -----------------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const TransferableObjectDescriptor& rObjDesc )
{
    const sal_uInt32 nFirstPos = rOStm.Tell();

    rOStm.SeekRel( 4 );
    rOStm << rObjDesc.maClassName;
    rOStm << rObjDesc.mnViewAspect;
    rOStm << rObjDesc.maSize.Width();
    rOStm << rObjDesc.maSize.Height();
    rOStm << rObjDesc.maDragStartPos.X();
    rOStm << rObjDesc.maDragStartPos.Y();
    rOStm.WriteByteString( rObjDesc.maTypeName, gsl_getSystemTextEncoding() );
    rOStm.WriteByteString( rObjDesc.maDisplayName, gsl_getSystemTextEncoding() );

    const sal_uInt32 nLastPos = rOStm.Tell();

    rOStm.Seek( nFirstPos );
    rOStm << ( nLastPos - nFirstPos );
    rOStm.Seek( nLastPos );

    return rOStm;
}

// -----------------------------------------
// - TransferableHelper::TerminateListener -
// -----------------------------------------

TransferableHelper::TerminateListener::TerminateListener( TransferableHelper& rTransferableHelper ) :
    mrParent( rTransferableHelper )
{
}

// -----------------------------------------------------------------------------

TransferableHelper::TerminateListener::~TerminateListener()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::TerminateListener::disposing( const EventObject& Source ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::TerminateListener::queryTermination( const EventObject& aEvent ) throw( TerminationVetoException, RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::TerminateListener::notifyTermination( const EventObject& aEvent ) throw( RuntimeException )
{
    mrParent.FlushToClipboard();
}

// ----------------------
// - TransferableHelper -
// ----------------------

TransferableHelper::TransferableHelper()
{
}

// -----------------------------------------------------------------------------

TransferableHelper::~TransferableHelper()
{
}

// -----------------------------------------------------------------------------

Any SAL_CALL TransferableHelper::getTransferData( const DataFlavor& rFlavor ) throw( UnsupportedFlavorException, IOException, RuntimeException )
{
    if( !maAny.hasValue() || !maFormats.size() || ( maLastFormat != rFlavor.MimeType ) )
    {
        maLastFormat = rFlavor.MimeType;
        maAny = Any();

        Application::GetSolarMutex().acquire();

        if( !maFormats.size() )
            AddSupportedFormats();

        GetData( rFlavor );

        // watch for special formats
        if( !maAny.hasValue() )
            ImplGetSubstitutionData( rFlavor );

        Application::GetSolarMutex().release();

        if( !maAny.hasValue() )
            throw UnsupportedFlavorException();
    }

    return maAny;
}

// -----------------------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL TransferableHelper::getTransferDataFlavors() throw( RuntimeException )
{
    if( !maFormats.size() )
    {
        Application::GetSolarMutex().acquire();
        AddSupportedFormats();
        Application::GetSolarMutex().release();
    }

    Sequence< DataFlavor >      aRet( maFormats.size() );
    DataFlavorExList::iterator  aIter( maFormats.begin() ), aEnd( maFormats.end() );
    sal_uInt32                  nCurPos = 0;

    while( aIter != aEnd )
        aRet[ nCurPos++ ] = (DataFlavor&)(*aIter++);

    return aRet;
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL TransferableHelper::isDataFlavorSupported( const DataFlavor& rFlavor ) throw( RuntimeException )
{
    sal_Bool bRet = sal_False;

    if( !maFormats.size() )
    {
        Application::GetSolarMutex().acquire();
        AddSupportedFormats();
        Application::GetSolarMutex().release();
    }

    DataFlavorExList::iterator  aIter( maFormats.begin() ), aEnd( maFormats.end() );

    while( aIter != aEnd )
    {
        if( TransferableDataHelper::IsEqual( rFlavor, *aIter++ ) )
        {
            bRet = sal_True;
            aIter = aEnd;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans ) throw( RuntimeException )
{
    if( mxTerminateListener.is() )
    {
        Reference< XMultiServiceFactory > xFact( ::comphelper::getProcessServiceFactory() );

        if( xFact.is() )
        {
            Reference< XDesktop > xDesktop( xFact->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );

            if( xDesktop.is() )
                xDesktop->removeTerminateListener( mxTerminateListener );
        }

        mxTerminateListener = Reference< XTerminateListener >();
    }

    Application::GetSolarMutex().acquire();
    ObjectReleased();
    Application::GetSolarMutex().release();
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::disposing( const EventObject& rSource ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::dragDropEnd( const DragSourceDropEvent& rDSDE ) throw( RuntimeException )
{
    Application::GetSolarMutex().acquire();
    DragFinished( rDSDE.DropSuccess ? rDSDE.DropAction : DNDConstants::ACTION_NONE );
    ObjectReleased();
    Application::GetSolarMutex().release();
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::dragEnter( const DragSourceDragEvent& rDSDE ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::dragExit( const DragSourceEvent& rDSE ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::dragOver( const DragSourceDragEvent& rDSDE ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::dropActionChanged( const DragSourceDragEvent& rDSDE ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void TransferableHelper::ImplGetSubstitutionData( const DataFlavor& rFlavor )
{
    DataFlavor aSubstFlavor;

    if( SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_WMF, aSubstFlavor ) && TransferableDataHelper::IsEqual( rFlavor, aSubstFlavor ) )
    {
        // WMF => GDIMETAFILE
        if( SotExchange::GetFormatDataFlavor( FORMAT_GDIMETAFILE, aSubstFlavor ) )
        {
            GetData( aSubstFlavor );

            if( maAny.hasValue() )
            {
                Sequence< sal_Int8 > aSeq;

                if( maAny >>= aSeq )
                {
                    SvMemoryStream* pSrcStm = new SvMemoryStream( (char*) aSeq.getConstArray(), aSeq.getLength(), STREAM_WRITE | STREAM_TRUNC );
                    GDIMetaFile     aMtf;

                    *pSrcStm >> aMtf;
                    delete pSrcStm;

                    Graphic         aGraphic( aMtf );
                    SvMemoryStream  aDstStm( 65535, 65535 );

                    if( GraphicConverter::Export( aDstStm, aGraphic, CVT_WMF ) == ERRCODE_NONE )
                        maAny <<= ( aSeq = Sequence< sal_Int8 >( (sal_Int8*) aDstStm.GetData(), aDstStm.Seek( STREAM_SEEK_TO_END ) ) );
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------

void TransferableHelper::AddFormat( SotFormatStringId nFormat )
{
    if( !HasFormat( nFormat ) )
    {
        DataFlavorEx aFlavorEx;

        if( SotExchange::GetFormatDataFlavor( nFormat, aFlavorEx ) )
        {
            aFlavorEx.mnSotId = nFormat;
            maFormats.push_back( aFlavorEx );
        }

        if( FORMAT_GDIMETAFILE == nFormat )
            AddFormat( SOT_FORMATSTR_ID_WMF );
    }
}

// -----------------------------------------------------------------------------

void TransferableHelper::AddFormat( const DataFlavor& rFlavor )
{
    if( !HasFormat( rFlavor ) )
    {
        DataFlavorEx aFlavorEx;

        aFlavorEx.MimeType = rFlavor.MimeType;
        aFlavorEx.HumanPresentableName = rFlavor.HumanPresentableName;
        aFlavorEx.DataType = rFlavor.DataType;
        aFlavorEx.mnSotId = SotExchange::RegisterFormat( rFlavor );

        maFormats.push_back( aFlavorEx );

        if( FORMAT_GDIMETAFILE == aFlavorEx.mnSotId )
            AddFormat( SOT_FORMATSTR_ID_WMF );
    }
}

// -----------------------------------------------------------------------------

void TransferableHelper::RemoveFormat( SotFormatStringId nFormat )
{
    DataFlavorExList::iterator  aIter( maFormats.begin() ), aEnd( maFormats.end() );

    while( aIter != aEnd )
    {
        if( nFormat == (*aIter).mnSotId )
            aIter = maFormats.erase( aIter );
        else
            aIter++;
    }
}

// -----------------------------------------------------------------------------

void TransferableHelper::RemoveFormat( const DataFlavor& rFlavor )
{
    DataFlavorExList::iterator  aIter( maFormats.begin() ), aEnd( maFormats.end() );

    while( aIter != aEnd )
    {
        if( TransferableDataHelper::IsEqual( rFlavor, *aIter++ ) )
            aIter = maFormats.erase( aIter );
        else
            aIter++;
    }
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::HasFormat( SotFormatStringId nFormat )
{
    DataFlavorExList::iterator  aIter( maFormats.begin() ), aEnd( maFormats.end() );
    sal_Bool                    bRet = sal_False;

    while( aIter != aEnd )
    {
        if( nFormat == (*aIter++).mnSotId )
        {
            bRet = sal_True;
            aIter = aEnd;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void TransferableHelper::ClearFormats()
{
    maFormats.clear();
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetAny( const Any& rAny, const DataFlavor& rFlavor )
{
    maAny = rAny;
    return( maAny.hasValue() );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetString( const ::rtl::OUString& rString, const DataFlavor& rFlavor )
{
    DataFlavor aFileFlavor;

    if( rString.getLength() &&
        SotExchange::GetFormatDataFlavor( FORMAT_FILE, aFileFlavor ) &&
        TransferableDataHelper::IsEqual( aFileFlavor, rFlavor ) )
    {
        const String            aString( rString );
        const ByteString        aByteStr( aString, gsl_getSystemTextEncoding() );
        Sequence< sal_Int8 >    aSeq( aByteStr.Len() + 1 );

        rtl_copyMemory( aSeq.getArray(), aByteStr.GetBuffer(), aByteStr.Len() );
        aSeq[ aByteStr.Len() ] = 0;
        maAny <<= aSeq;
    }
    else
        maAny <<= rString;

    return( maAny.hasValue() );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetBitmap( const Bitmap& rBitmap, const DataFlavor& rFlavor )
{
    // only Bitmap at the moment
    if( !rBitmap.IsEmpty() )
    {
        SvMemoryStream aMemStm( 65535, 65535 );

        aMemStm << rBitmap;
        maAny <<= Sequence< sal_Int8 >( (sal_Int8*) aMemStm.GetData(), aMemStm.Seek( STREAM_SEEK_TO_END ) );
    }

    return( maAny.hasValue() );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetGDIMetaFile( const GDIMetaFile& rMtf, const DataFlavor& rFlavor )
{
    // only GDIMetaFile at the moment
    if( rMtf.GetActionCount() )
    {
        SvMemoryStream aMemStm( 65535, 65535 );

        ( (GDIMetaFile&) rMtf ).Write( aMemStm );
        maAny <<= Sequence< sal_Int8 >( (sal_Int8*) aMemStm.GetData(), aMemStm.Seek( STREAM_SEEK_TO_END ) );
    }

    return( maAny.hasValue() != NULL );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetGraphic( const Graphic& rGraphic, const DataFlavor& rFlavor )
{
    if( rGraphic.GetType() != GRAPHIC_NONE )
    {
        SvMemoryStream aMemStm( 65535, 65535 );

        aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
        aMemStm << rGraphic;
        maAny <<= Sequence< sal_Int8 >( (sal_Int8*) aMemStm.GetData(), aMemStm.Seek( STREAM_SEEK_TO_END ) );
    }

    return( maAny.hasValue() != NULL );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetImageMap( const ImageMap& rIMap, const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    SvMemoryStream aMemStm( 65535, 65535 );

    aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
    aMemStm << rIMap;
    maAny <<= Sequence< sal_Int8 >( (sal_Int8*) aMemStm.GetData(), aMemStm.Seek( STREAM_SEEK_TO_END ) );

    return( maAny.hasValue() != NULL );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetTransferableObjectDescriptor( const TransferableObjectDescriptor& rDesc,
                                                              const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    SvMemoryStream aMemStm( 1024, 1024 );

    aMemStm << rDesc;
    maAny <<= Sequence< sal_Int8 >( (sal_Int8*) aMemStm.GetData(), aMemStm.Tell() );

    return( maAny.hasValue() );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetINetBookmark( const INetBookmark& rBmk,
                                              const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    rtl_TextEncoding eSysCSet = gsl_getSystemTextEncoding();

    switch( SotExchange::GetFormat( rFlavor ) )
    {
        case( SOT_FORMATSTR_ID_SOLK ):
        {
            ByteString sURL( rBmk.GetURL(), eSysCSet ),
                       sDesc( rBmk.GetDescription(), eSysCSet );
            ByteString sOut( ByteString::CreateFromInt32( sURL.Len() ));
            ( sOut += '@' ) += sURL;
            sOut += ByteString::CreateFromInt32( sDesc.Len() );
            ( sOut += '@' ) += sDesc;

            Sequence< sal_Int8 > aSeq( sOut.Len() );
            memcpy( aSeq.getArray(), sOut.GetBuffer(), sOut.Len() );
            maAny <<= aSeq;
        }
        break;

        case( FORMAT_STRING ):
            maAny <<= ::rtl::OUString( rBmk.GetURL() );
            break;

        case( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ):
        {
            ByteString sURL( rBmk.GetURL(), eSysCSet );
            Sequence< sal_Int8 > aSeq( sURL.Len() );
            memcpy( aSeq.getArray(), sURL.GetBuffer(), sURL.Len() );
            maAny <<= aSeq;
        }
        break;

        case( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ):
        {
            Sequence< sal_Int8 > aSeq( 2048 );

            memset( aSeq.getArray(), 0, 2048 );
            strcpy( (char*) aSeq.getArray(), ByteString( rBmk.GetURL(), eSysCSet).GetBuffer() );
            strcpy( (char*) aSeq.getArray() + 1024, ByteString( rBmk.GetDescription(), eSysCSet ).GetBuffer() );

            maAny <<= aSeq;
        }
        break;

#ifdef WNT
        case SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR:
        {
            Sequence< sal_Int8 >    aSeq( sizeof( FILEGROUPDESCRIPTOR ) );
            FILEGROUPDESCRIPTOR*    pFDesc = (FILEGROUPDESCRIPTOR*) aSeq.getArray();
            FILEDESCRIPTOR&         rFDesc1 = pFDesc->fgd[ 0 ];

            pFDesc->cItems = 1;
            memset( &rFDesc1, 0, sizeof( FILEDESCRIPTOR ) );
            rFDesc1.dwFlags = FD_LINKUI;

            ByteString aStr( rBmk.GetDescription(), eSysCSet );
            for( USHORT nChar = 0; nChar < aStr.Len(); ++nChar )
                if( strchr( "\\/:*?\"<>|", aStr.GetChar( nChar ) ) )
                    aStr.Erase( nChar--, 1 );

            aStr.Insert( "Shortcut to ", 0 );
            aStr += ".URL";
            strcpy( rFDesc1.cFileName, aStr.GetBuffer() );

            maAny <<= aSeq;
        }
        break;

        case SOT_FORMATSTR_ID_FILECONTENT:
        {
            String aStr( RTL_CONSTASCII_STRINGPARAM( "[InternetShortcut]\x0aURL=" ) );
            maAny <<= ::rtl::OUString( aStr += rBmk.GetURL() );
        }
        break;
#endif

        default:
        break;
    }

    return( maAny.hasValue() );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetINetImage( const INetImage& rINtImg,
                                           const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    SvMemoryStream aMemStm( 1024, 1024 );

    aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
    rINtImg.Write( aMemStm, SotExchange::GetFormat( rFlavor ) );

    maAny <<= Sequence< sal_Int8 >( (sal_Int8*) aMemStm.GetData(), aMemStm.Seek( STREAM_SEEK_TO_END ) );

    return( maAny.hasValue() != NULL );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetFileList( const FileList& rFileList,
                                          const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    SvMemoryStream aMemStm( 4096, 4096 );

    aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
    aMemStm << rFileList;

    maAny <<= Sequence< sal_Int8 >( (sal_Int8*) aMemStm.GetData(), aMemStm.Seek( STREAM_SEEK_TO_END ) );

    return( maAny.hasValue() != NULL );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetObject( void* pUserObject, sal_uInt32 nUserObjectId, const DataFlavor& rFlavor )
{
    SotStorageStreamRef xStm( new SotStorageStream( String() ) );

    xStm->SetVersion( SOFFICE_FILEFORMAT_50 );

    if( pUserObject && WriteObject( xStm, pUserObject, nUserObjectId, rFlavor ) )
    {
        const sal_uInt32        nLen = xStm->Seek( STREAM_SEEK_TO_END );
        Sequence< sal_Int8 >    aSeq( nLen );

        xStm->Seek( STREAM_SEEK_TO_BEGIN );
        xStm->Read( aSeq.getArray(),  nLen );

        if( nLen && ( SotExchange::GetFormat( rFlavor ) == SOT_FORMAT_STRING ) )
            maAny <<= ::rtl::OUString( (const sal_Char*) aSeq.getConstArray(), nLen - 1, gsl_getSystemTextEncoding() );
        else
            maAny <<= aSeq;
    }

    return( maAny.hasValue() != NULL );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetInterface( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rIf,
                                           const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    maAny <<= rIf;
    return( maAny.hasValue() != NULL );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const DataFlavor& rFlavor )
{
    DBG_ERROR( "TransferableHelper::WriteObject( ... ) not implemented" );
    return sal_False;
}

// -----------------------------------------------------------------------------

void TransferableHelper::DragFinished( sal_Int8 nDropAction )
{
}

// -----------------------------------------------------------------------------

void TransferableHelper::ObjectReleased()
{
}

// -----------------------------------------------------------------------------

void TransferableHelper::CopyToClipboard() const
{
    if( !mxClipboard.is() )
        ( (TransferableHelper*) this )->mxClipboard = GetSystemClipboard();

    if( mxClipboard.is() )
    {
        try
        {
            Reference< XMultiServiceFactory > xFact( ::comphelper::getProcessServiceFactory() );

            if( xFact.is() )
            {
                Reference< XDesktop > xDesktop( xFact->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );

// #81926: should be enabled
#if 0
                if( xDesktop.is() )
                {
                    TransferableHelper* pThis = (TransferableHelper*) this;
                    xDesktop->addTerminateListener( pThis->mxTerminateListener = new TerminateListener( *pThis ) );
                }
#endif
            }

            const sal_uInt32 nRef = Application::ReleaseSolarMutex();
            mxClipboard->setContents( (TransferableHelper*) this, (TransferableHelper*) this );
            Application::AcquireSolarMutex( nRef );
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            DBG_ERROR( "Could not copy to clipboard" );
        }
    }
}

// -----------------------------------------------------------------------------

void TransferableHelper::FlushToClipboard() const
{
    if( mxClipboard.is() )
    {
        Reference< XFlushableClipboard > xFlushableClipboard( mxClipboard, UNO_QUERY );

        try
        {
            if( xFlushableClipboard.is() )
            {
                const sal_uInt32 nRef = Application::ReleaseSolarMutex();
                xFlushableClipboard->flushClipboard();
                Application::AcquireSolarMutex( nRef );
            }
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            DBG_ERROR( "Could not flush clipboard" );
        }
    }
}

// -----------------------------------------------------------------------------

void TransferableHelper::StartDrag( Window* pWindow, sal_Int8 nDnDSourceActions,
                                    sal_Int32 nDnDPointer, sal_Int32 nDnDImage )
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );
    Reference< XDragSource > xDragSource( pWindow->GetDragSource() );

    if( xDragSource.is() )
    {
        DragGestureEvent    aEvt;
        const Point         aPt( pWindow->GetPointerPosPixel() );

        aEvt.DragAction = DNDConstants::ACTION_COPY;
        aEvt.DragOriginX = aPt.X();
        aEvt.DragOriginY = aPt.Y();
        aEvt.DragSource = xDragSource;

        try
        {
            const sal_uInt32 nRef = Application::ReleaseSolarMutex();
            xDragSource->startDrag( aEvt, nDnDSourceActions, nDnDPointer, nDnDImage, this, this );
            Application::AcquireSolarMutex( nRef );
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            DBG_ERROR( "Could not start drag" );
        }
    }
}

// -----------------------------------------------------------------------------

Reference< XClipboard> TransferableHelper::GetSystemClipboard()
{
    Reference< XClipboard > xClipboard;

    try
    {
        Reference< XMultiServiceFactory > xFact( ::comphelper::getProcessServiceFactory() );

        if( xFact.is() )
        {
            xClipboard = Reference< XClipboard >( xFact->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.SystemClipboard" ) ),
                                                                         UNO_QUERY );
        }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        DBG_ERROR( "Clipboard could not be initialized" );
    }

    return xClipboard;
}

// --------------------------
// - TransferableDataHelper -
// --------------------------

TransferableDataHelper::TransferableDataHelper() :
    mpFormatsVector( new ::std::vector< DataFlavorEx > )
{
}

// -----------------------------------------------------------------------------

TransferableDataHelper::TransferableDataHelper( const Reference< ::com::sun::star::datatransfer::XTransferable >& rxTransferable ) :
    mxTransfer( rxTransferable ),
    mpFormatsVector( new ::std::vector< DataFlavorEx > )
{
    InitFormats();
}

// -----------------------------------------------------------------------------

TransferableDataHelper::TransferableDataHelper( const TransferableDataHelper& rDataHelper ) :
    mxTransfer( rDataHelper.mxTransfer ),
    mpFormatsVector( new ::std::vector< DataFlavorEx >( *rDataHelper.mpFormatsVector ) )
{
}

// -----------------------------------------------------------------------------

TransferableDataHelper& TransferableDataHelper::operator=( const TransferableDataHelper& rDataHelper )
{
    mxTransfer = rDataHelper.mxTransfer;
    delete mpFormatsVector, mpFormatsVector = new ::std::vector< DataFlavorEx >( *rDataHelper.mpFormatsVector );

    return *this;
}

// -----------------------------------------------------------------------------

TransferableDataHelper::~TransferableDataHelper()
{
    delete mpFormatsVector;
}

// -----------------------------------------------------------------------------

void TransferableDataHelper::InitFormats()
{
    Application::GetSolarMutex().acquire();
    mpFormatsVector->clear();

    try
    {
        if( mxTransfer.is() )
        {
            const Sequence< DataFlavor > aFlavors( mxTransfer->getTransferDataFlavors() );

            for( sal_Int32 i = 0; i < aFlavors.getLength(); i++ )
            {
                DataFlavorEx        aFlavorEx;
                const DataFlavor&   rFlavor = aFlavors[ i ];

                aFlavorEx.MimeType = rFlavor.MimeType;
                aFlavorEx.HumanPresentableName = rFlavor.HumanPresentableName;
                aFlavorEx.DataType = rFlavor.DataType;
                aFlavorEx.mnSotId = SotExchange::RegisterFormat( rFlavor );

                mpFormatsVector->push_back( aFlavorEx );

                if( ( SOT_FORMATSTR_ID_WMF == aFlavorEx.mnSotId ) && !HasFormat( SOT_FORMAT_GDIMETAFILE ) )
                {
                    if( SotExchange::GetFormatDataFlavor( SOT_FORMAT_GDIMETAFILE, aFlavorEx ) )
                    {
                        aFlavorEx.mnSotId = SOT_FORMAT_GDIMETAFILE;
                        mpFormatsVector->push_back( aFlavorEx );
                    }
                }
            }
        }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }

    Application::GetSolarMutex().release();
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::HasFormat( SotFormatStringId nFormat ) const
{
    ::std::vector< DataFlavorEx >::iterator aIter( mpFormatsVector->begin() ), aEnd( mpFormatsVector->end() );
    sal_Bool                                bRet = sal_False;

    while( aIter != aEnd )
    {
        if( nFormat == (*aIter++).mnSotId )
        {
            bRet = sal_True;
            aIter = aEnd;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::HasFormat( const DataFlavor& rFlavor ) const
{
    ::std::vector< DataFlavorEx >::iterator aIter( mpFormatsVector->begin() ), aEnd( mpFormatsVector->end() );
    sal_Bool                                bRet = sal_False;

    while( aIter != aEnd )
    {
        if( TransferableDataHelper::IsEqual( rFlavor, *aIter++ ) )
        {
            bRet = sal_True;
            aIter = aEnd;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_uInt32 TransferableDataHelper::GetFormatCount() const
{
    return mpFormatsVector->size();
}

// -----------------------------------------------------------------------------

SotFormatStringId TransferableDataHelper::GetFormat( sal_uInt32 nFormat ) const
{
    DBG_ASSERT( nFormat < mpFormatsVector->size(), "TransferableDataHelper::GetFormat: invalid format index" );
        return( ( nFormat < mpFormatsVector->size() ) ? (*mpFormatsVector)[ nFormat ].mnSotId : 0 );
}

// -----------------------------------------------------------------------------

DataFlavor TransferableDataHelper::GetFormatDataFlavor( sal_uInt32 nFormat ) const
{
    DBG_ASSERT( nFormat < mpFormatsVector->size(), "TransferableDataHelper::GetFormat: invalid format index" );
    DataFlavor aRet;

    if( nFormat < mpFormatsVector->size() )
        aRet = (DataFlavor&) (*mpFormatsVector)[ nFormat ];

    return aRet;
}

// -----------------------------------------------------------------------------

Any TransferableDataHelper::GetAny( const DataFlavor& rFlavor ) const
{
    const sal_uInt32    nRef = Application::ReleaseSolarMutex();
    Any                 aRet;

    try
    {
        if( mxTransfer.is() )
            aRet = mxTransfer->getTransferData( rFlavor );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }

    Application::AcquireSolarMutex( nRef );

    return aRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetString( SotFormatStringId nFormat, String& rStr )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetString( aFlavor, rStr ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetString( const DataFlavor& rFlavor, String& rStr )
{
    const Any aAny( GetAny( rFlavor ) );
    sal_Bool bRet = sal_False;
    if( aAny.hasValue() )
    {
        ::rtl::OUString aOUString;
        Sequence< sal_Int8 > aSeq;
        bRet = sal_True;
        if( aAny >>= aOUString )
            rStr = aOUString;
        else if( aAny >>= aSeq )
        {
            // depends on the dataflavour which kind of charset we use
            // default to UTF8
            // in other cases its better to use system charset
            rtl_TextEncoding eCSet = gsl_getSystemTextEncoding();

            rStr = String( (sal_Char*)aSeq.getArray(), eCSet );
        }
        else
            bRet = sal_False;
    }
    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetBitmap( SotFormatStringId nFormat, Bitmap& rBmp )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetBitmap( aFlavor, rBmp ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetBitmap( const DataFlavor& rFlavor, Bitmap& rBmp )
{
    SotStorageStreamRef xStm;
    sal_Bool            bRet = GetSotStorageStream( rFlavor, xStm );

    if( bRet )
    {
        *xStm >> rBmp;
        bRet = ( xStm->GetError() == ERRCODE_NONE );
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetGDIMetaFile( SotFormatStringId nFormat, GDIMetaFile& rMtf )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetGDIMetaFile( aFlavor, rMtf ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetGDIMetaFile( const DataFlavor& rFlavor, GDIMetaFile& rMtf )
{
    SotStorageStreamRef xStm;
    sal_Bool            bRet = GetSotStorageStream( rFlavor, xStm );

    if( bRet )
    {
        *xStm >> rMtf;
        bRet = ( xStm->GetError() == ERRCODE_NONE );
    }

    if( !bRet )
    {
        DataFlavor aSubstFlavor;

        if( HasFormat( SOT_FORMATSTR_ID_WMF ) &&
            SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_WMF, aSubstFlavor ) &&
            GetSotStorageStream( aSubstFlavor, xStm ) )
        {
            Graphic aGraphic;

            if( GraphicConverter::Import( *xStm, aGraphic ) == ERRCODE_NONE )
            {
                rMtf = aGraphic.GetGDIMetaFile();
                bRet = TRUE;
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetGraphic( SotFormatStringId nFormat, Graphic& rGraphic )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetGraphic( aFlavor, rGraphic ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetGraphic( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, Graphic& rGraphic )
{
    DataFlavor  aFlavor;
    sal_Bool    bRet = sal_False;

    if( SotExchange::GetFormatDataFlavor( SOT_FORMAT_BITMAP, aFlavor ) &&
        TransferableDataHelper::IsEqual( aFlavor, rFlavor ) )
    {
        Bitmap aBmp;

        if( ( bRet = GetBitmap( aFlavor, aBmp ) ) == sal_True )
            rGraphic = aBmp;
    }
    else if( SotExchange::GetFormatDataFlavor( SOT_FORMAT_GDIMETAFILE, aFlavor ) &&
             TransferableDataHelper::IsEqual( aFlavor, rFlavor ) )
    {
        GDIMetaFile aMtf;

        if( ( bRet = GetGDIMetaFile( aFlavor, aMtf ) ) == sal_True )
            rGraphic = aMtf;
    }
    else
    {
        SotStorageStreamRef xStm;

        if( GetSotStorageStream( rFlavor, xStm ) )
        {
            *xStm >> rGraphic;
            bRet = ( xStm->GetError() == ERRCODE_NONE );
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetImageMap( SotFormatStringId nFormat, ImageMap& rIMap )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetImageMap( aFlavor, rIMap ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetImageMap( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, ImageMap& rIMap )
{
    SotStorageStreamRef xStm;
    sal_Bool            bRet = GetSotStorageStream( rFlavor, xStm );

    if( bRet )
    {
        *xStm >> rIMap;
        bRet = ( xStm->GetError() == ERRCODE_NONE );
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetTransferableObjectDescriptor( SotFormatStringId nFormat, TransferableObjectDescriptor& rDesc )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetTransferableObjectDescriptor( aFlavor, rDesc ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetTransferableObjectDescriptor( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, TransferableObjectDescriptor& rDesc )
{
    SotStorageStreamRef xStm;
    sal_Bool            bRet = GetSotStorageStream( rFlavor, xStm );

    if( bRet )
    {
        *xStm >> rDesc;
        bRet = ( xStm->GetError() == ERRCODE_NONE );
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetINetBookmark( SotFormatStringId nFormat, INetBookmark& rBmk )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetINetBookmark( aFlavor, rBmk ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetINetBookmark( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, INetBookmark& rBmk )
{
    const SotFormatStringId nFormat = SotExchange::GetFormat( rFlavor );
    sal_Bool                bRet = sal_False;

    switch( nFormat )
    {
        case( SOT_FORMATSTR_ID_SOLK ):
        case( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ):
        {
            String aString;
            if( GetString( rFlavor, aString ) )
            {
                if( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR == nFormat )
                {
                    rBmk = INetBookmark( aString, aString );
                    bRet = sal_True;
                }
                else
                {
                    String      aURL, aDesc;
                    sal_uInt16  nStart = aString.Search( '@' ), nLen = (sal_uInt16) aString.ToInt32();

                    if( !nLen && aString.GetChar( 0 ) != '0' )
                    {
                        DBG_WARNING( "SOLK: 1. len=0" )
                    }
                    if( nStart == STRING_NOTFOUND || nLen > aString.Len() - nStart - 3 )
                    {
                        DBG_WARNING( "SOLK: 1. illegal start or wrong len" )
                    }
                    aURL = aString.Copy( nStart + 1, nLen );

                    aString.Erase( 0, nStart + 1 + nLen );
                    nStart = aString.Search( '@' );
                    nLen = (sal_uInt16) aString.ToInt32();

                    if( !nLen && aString.GetChar( 0 ) != '0' )
                    {
                        DBG_WARNING( "SOLK: 2. len=0" )
                    }
                    if( nStart == STRING_NOTFOUND || nLen > aString.Len() - nStart - 1 )
                    {
                        DBG_WARNING( "SOLK: 2. illegal start or wrong len" )
                    }
                    aDesc = aString.Copy( nStart+1, nLen );

                    rBmk = INetBookmark( aURL, aDesc );
                    bRet = sal_True;
                }
            }
        }
        break;

        case( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ):
        {
            Sequence< sal_Int8 > aSeq;

            if( GetSequence( rFlavor, aSeq ) && ( 2048 == aSeq.getLength() ) )
            {
                rBmk = INetBookmark( String( (sal_Char*) aSeq.getConstArray(), gsl_getSystemTextEncoding() ),
                                     String( (sal_Char*) aSeq.getConstArray() + 1024, gsl_getSystemTextEncoding() ) );
                bRet = sal_True;
            }
        }
        break;

#ifdef WNT
        case SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR:
        {
            Sequence< sal_Int8 > aSeq;

            if( GetSequence( rFlavor, aSeq ) && aSeq.getLength() )
            {
                FILEGROUPDESCRIPTOR* pFDesc = (FILEGROUPDESCRIPTOR*) aSeq.getConstArray();

                if( pFDesc->cItems )
                {
                    ByteString          aDesc( pFDesc->fgd[ 0 ].cFileName );
                    rtl_TextEncoding    eTextEncoding = gsl_getSystemTextEncoding();

                    if( ( aDesc.Len() > 4 ) && aDesc.Copy( aDesc.Len() - 4 ).EqualsIgnoreCaseAscii( ".URL" ) )
                    {
                        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( INetURLObject( String( aDesc, eTextEncoding ) ).GetMainURL(), STREAM_STD_READ );

                        if( !pStream || pStream->GetError() )
                        {
                            DataFlavor aFileContentFlavor;

                            aSeq.realloc( 0 );
                            delete pStream;

                            if( SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_FILECONTENT, aFileContentFlavor ) &&
                                GetSequence( aFileContentFlavor, aSeq ) && aSeq.getLength() )
                            {
                                pStream = new SvMemoryStream( (sal_Char*) aSeq.getConstArray(), aSeq.getLength(), STREAM_STD_READ );
                            }
                            else
                                pStream = NULL;
                        }

                        if( pStream )
                        {
                            ByteString  aLine;
                            sal_Bool    bSttFnd = sal_False;

                            while( pStream->ReadLine( aLine ) )
                            {
                                if( aLine.EqualsIgnoreCaseAscii( "[InternetShortcut]" ) )
                                    bSttFnd = sal_True;
                                else if( bSttFnd && aLine.Copy( 0, 4 ).EqualsIgnoreCaseAscii( "URL=" ) )
                                {
                                    rBmk = INetBookmark( String( aLine.Erase( 0, 4 ), eTextEncoding ),
                                                         String( aDesc.Erase( aDesc.Len() - 4 ), eTextEncoding ) );
                                    bRet = sal_True;
                                    break;
                                }
                            }

                            delete pStream;
                        }
                    }
                }
            }
        }
        break;
#endif

        default:
        break;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetINetImage( SotFormatStringId nFormat,
                                                INetImage& rINtImg )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) &&
             GetINetImage( aFlavor, rINtImg ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetINetImage(
        const ::com::sun::star::datatransfer::DataFlavor& rFlavor,
        INetImage& rINtImg )
{
    SotStorageStreamRef xStm;
    sal_Bool bRet = GetSotStorageStream( rFlavor, xStm );

    if( bRet )
        bRet = rINtImg.Read( *xStm, SotExchange::GetFormat( rFlavor ) );
    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetFileList( SotFormatStringId nFormat,
                                                FileList& rFileList )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) &&
             GetFileList( aFlavor, rFileList ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetFileList(
            const ::com::sun::star::datatransfer::DataFlavor& rFlavor,
            FileList& rFileList )
{
    SotStorageStreamRef xStm;
    sal_Bool bRet = GetSotStorageStream( rFlavor, xStm );

    if( bRet )
        bRet = ERRCODE_NONE == ( *xStm >> rFileList ).GetError();
    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetSequence( SotFormatStringId nFormat, Sequence< sal_Int8 >& rSeq )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetSequence( aFlavor, rSeq ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetSequence( const DataFlavor& rFlavor, Sequence< sal_Int8 >& rSeq )
{
    const Any aAny( GetAny( rFlavor ) );
    return( aAny.hasValue() && ( aAny >>= rSeq ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetSotStorageStream( SotFormatStringId nFormat, SotStorageStreamRef& rxStream )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetSotStorageStream( aFlavor, rxStream ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetSotStorageStream( const DataFlavor& rFlavor, SotStorageStreamRef& rxStream )
{
    Sequence< sal_Int8 >    aSeq;
    sal_Bool                bRet = GetSequence( rFlavor, aSeq );

    if( bRet )
    {
        rxStream = new SotStorageStream( String() );
        rxStream->Write( aSeq.getConstArray(), aSeq.getLength() );
        rxStream->Seek( 0 );
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetInterface( SotFormatStringId nFormat, Reference< XInterface >& rIf )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetInterface( aFlavor, rIf ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetInterface( const DataFlavor& rFlavor, Reference< XInterface >& rIf )
{
    const Any aAny( GetAny( rFlavor ) );
    return( aAny.hasValue() && ( aAny >>= rIf ) );
}

// -----------------------------------------------------------------------------

TransferableDataHelper TransferableDataHelper::CreateFromSystemClipboard()
{
    Reference< XClipboard > xClipboard( TransferableHelper::GetSystemClipboard() );
    TransferableDataHelper  aRet;

    if( xClipboard.is() )
    {
        try
        {
            Reference< XTransferable > xTransferable( xClipboard->getContents() );

            if( xTransferable.is() )
            {
                aRet = TransferableDataHelper( xTransferable );
                aRet.mxClipboard = xClipboard;
            }
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            DBG_ERROR( "Could not get clipboard content" );
        }
    }

    return aRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::IsEqual( const ::com::sun::star::datatransfer::DataFlavor& rFlavor1,
                                          const ::com::sun::star::datatransfer::DataFlavor& rFlavor2,
                                          sal_Bool bCompareParameters )
{
    if( sal_True/*bCompareParameters*/ )
        return( rFlavor1.MimeType == rFlavor2.MimeType );
    else
        return( String( rFlavor1.MimeType ).GetToken( 0, ';' ) == String( rFlavor2.MimeType ).GetToken( 0, ';' ) );
}
