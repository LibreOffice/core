/*************************************************************************
 *
 *  $RCSfile: transfer.cxx,v $
 *
 *  $Revision: 1.63 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:39:19 $
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
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
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
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDNOTIFIER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XFLUSHABLECLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XMIMECONTENTTYPEFACTORY_HPP_
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XMIMECONTENTTYPE_HPP_
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#include "urlbmk.hxx"
#include "inetimg.hxx"
#include "imap.hxx"
#include "transfer.hxx"

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
    mrParent.ImplFlush();
}

// ----------------------
// - TransferableHelper -
// ----------------------

TransferableHelper::TransferableHelper() :
    mpFormats( new DataFlavorExVector )
{
}

// -----------------------------------------------------------------------------

TransferableHelper::~TransferableHelper()
{
    delete mpFormats;
}

// -----------------------------------------------------------------------------

Any SAL_CALL TransferableHelper::getTransferData( const DataFlavor& rFlavor ) throw( UnsupportedFlavorException, IOException, RuntimeException )
{
    if( !maAny.hasValue() || !mpFormats->size() || ( maLastFormat != rFlavor.MimeType ) )
    {
        const ::vos::OGuard aGuard( Application::GetSolarMutex() );

        maLastFormat = rFlavor.MimeType;
        maAny = Any();

        try
        {
            DataFlavor  aSubstFlavor;
            sal_Bool    bDone = sal_False;

            // add formats if not already done
            if( !mpFormats->size() )
                AddSupportedFormats();

            // check alien formats first and try to get a substitution format
            if( SotExchange::GetFormatDataFlavor( FORMAT_STRING, aSubstFlavor ) &&
                TransferableDataHelper::IsEqual( aSubstFlavor, rFlavor ) )
            {
                GetData( aSubstFlavor );
                bDone = maAny.hasValue();
            }
            else if( SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_BMP, aSubstFlavor ) &&
                     TransferableDataHelper::IsEqual( aSubstFlavor, rFlavor ) &&
                     SotExchange::GetFormatDataFlavor( FORMAT_BITMAP, aSubstFlavor ) )
            {
                GetData( aSubstFlavor );
                bDone = sal_True;
            }
            else if( SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_EMF, aSubstFlavor ) &&
                     TransferableDataHelper::IsEqual( aSubstFlavor, rFlavor ) &&
                     SotExchange::GetFormatDataFlavor( FORMAT_GDIMETAFILE, aSubstFlavor ) )
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

                        if( GraphicConverter::Export( aDstStm, aGraphic, CVT_EMF ) == ERRCODE_NONE )
                        {
                            maAny <<= ( aSeq = Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aDstStm.GetData() ),
                                                                     aDstStm.Seek( STREAM_SEEK_TO_END ) ) );
                            bDone = sal_True;
                        }
                    }
                }
            }
            else if( SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_WMF, aSubstFlavor ) &&
                     TransferableDataHelper::IsEqual( aSubstFlavor, rFlavor ) &&
                     SotExchange::GetFormatDataFlavor( FORMAT_GDIMETAFILE, aSubstFlavor ) )
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
                        {
                            maAny <<= ( aSeq = Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aDstStm.GetData() ),
                                                                     aDstStm.Seek( STREAM_SEEK_TO_END ) ) );
                            bDone = sal_True;
                        }
                    }
                }
            }

            // reset Any if substitute doesn't work
            if( !bDone && maAny.hasValue() )
                maAny = Any();

            // if any is not yet filled, use standard format
            if( !maAny.hasValue() )
                GetData( rFlavor );
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        if( !maAny.hasValue() )
            throw UnsupportedFlavorException();
    }

    return maAny;
}

// -----------------------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL TransferableHelper::getTransferDataFlavors() throw( RuntimeException )
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );




    try
    {
        if( !mpFormats->size() )
            AddSupportedFormats();
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }

    Sequence< DataFlavor >          aRet( mpFormats->size() );
    DataFlavorExVector::iterator    aIter( mpFormats->begin() ), aEnd( mpFormats->end() );
    sal_uInt32                      nCurPos = 0;

    while( aIter != aEnd )
        aRet[ nCurPos++ ] = *aIter++;

    return aRet;
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL TransferableHelper::isDataFlavorSupported( const DataFlavor& rFlavor ) throw( RuntimeException )
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );
    sal_Bool            bRet = sal_False;

    try
    {
        if( !mpFormats->size() )
            AddSupportedFormats();
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }

    DataFlavorExVector::iterator aIter( mpFormats->begin() ), aEnd( mpFormats->end() );

    while( aIter != aEnd )
    {
        if( TransferableDataHelper::IsEqual( *aIter, rFlavor ) )
        {
            aIter = aEnd;
            bRet = sal_True;
        }
        else
            aIter++;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans ) throw( RuntimeException )
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    try
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

        ObjectReleased();
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::disposing( const EventObject& rSource ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableHelper::dragDropEnd( const DragSourceDropEvent& rDSDE ) throw( RuntimeException )
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    try
    {
        DragFinished( rDSDE.DropSuccess ? ( rDSDE.DropAction & ~DNDConstants::ACTION_DEFAULT ) : DNDConstants::ACTION_NONE );
        ObjectReleased();
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
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

sal_Int64 SAL_CALL TransferableHelper::getSomething( const Sequence< sal_Int8 >& rId ) throw( RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) &&
        ( 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = (sal_Int64) this;
    }
    else
        nRet = 0;

    return nRet;
}

// -----------------------------------------------------------------------------

void TransferableHelper::ImplFlush()
{
    if( mxClipboard.is() )
    {
        Reference< XFlushableClipboard >    xFlushableClipboard( mxClipboard, UNO_QUERY );
        const sal_uInt32                    nRef = Application::ReleaseSolarMutex();

        try
        {
            if( xFlushableClipboard.is() )
                 xFlushableClipboard->flushClipboard();
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            DBG_ERROR( "Could not flush clipboard" );
        }

        Application::AcquireSolarMutex( nRef );
    }
}

// -----------------------------------------------------------------------------

void TransferableHelper::AddFormat( SotFormatStringId nFormat )
{
    DataFlavor aFlavor;

    if( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) )
        AddFormat( aFlavor );
}

// -----------------------------------------------------------------------------

void TransferableHelper::AddFormat( const DataFlavor& rFlavor )
{
    DataFlavorExVector::iterator    aIter( mpFormats->begin() ), aEnd( mpFormats->end() );
    sal_Bool                        bAdd = sal_True;

    while( aIter != aEnd )

    {

        if( TransferableDataHelper::IsEqual( *aIter, rFlavor ) )
        {
            aIter = aEnd;
            bAdd = sal_False;
        }
        else
            aIter++;
    }

    if( bAdd )
    {
        DataFlavorEx aFlavorEx;

        aFlavorEx.MimeType = rFlavor.MimeType;
        aFlavorEx.HumanPresentableName = rFlavor.HumanPresentableName;
        aFlavorEx.DataType = rFlavor.DataType;
        aFlavorEx.mnSotId = SotExchange::RegisterFormat( rFlavor );

        mpFormats->push_back( aFlavorEx );

        if( FORMAT_BITMAP == aFlavorEx.mnSotId )
        {
            AddFormat( SOT_FORMATSTR_ID_BMP );
        }
        else if( FORMAT_GDIMETAFILE == aFlavorEx.mnSotId )
        {
            AddFormat( SOT_FORMATSTR_ID_EMF );
            AddFormat( SOT_FORMATSTR_ID_WMF );
        }
    }
}

// -----------------------------------------------------------------------------

void TransferableHelper::RemoveFormat( SotFormatStringId nFormat )
{
    DataFlavor aFlavor;

    if( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) )
        RemoveFormat( aFlavor );
}

// -----------------------------------------------------------------------------

void TransferableHelper::RemoveFormat( const DataFlavor& rFlavor )
{
    DataFlavorExVector::iterator aIter( mpFormats->begin() ), aEnd( mpFormats->end() );

    while( aIter != aEnd )
    {
        if( TransferableDataHelper::IsEqual( *aIter, rFlavor ) )
        {
            aIter = mpFormats->erase( aIter );
            aEnd = mpFormats->end();
        }
        else
            ++aIter;
    }
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::HasFormat( SotFormatStringId nFormat )
{
    DataFlavorExVector::iterator    aIter( mpFormats->begin() ), aEnd( mpFormats->end() );
    sal_Bool                        bRet = sal_False;

    while( aIter != aEnd )
    {
        if( nFormat == (*aIter).mnSotId )
        {
            aIter = aEnd;
            bRet = sal_True;
        }
        else
            ++aIter;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void TransferableHelper::ClearFormats()
{
    mpFormats->clear();
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
    if( !rBitmap.IsEmpty() )
    {
        SvMemoryStream aMemStm( 65535, 65535 );

        aMemStm << rBitmap;
        maAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );
    }

    return( maAny.hasValue() );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetGDIMetaFile( const GDIMetaFile& rMtf, const DataFlavor& rFlavor )
{
    if( rMtf.GetActionCount() )
    {
        SvMemoryStream aMemStm( 65535, 65535 );

        ( (GDIMetaFile&) rMtf ).Write( aMemStm );
        maAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );
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
        aMemStm.SetCompressMode( COMPRESSMODE_NATIVE );
        aMemStm << rGraphic;
        maAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );
    }

    return( maAny.hasValue() != NULL );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetImageMap( const ImageMap& rIMap, const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    SvMemoryStream aMemStm( 8192, 8192 );

    aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
    aMemStm << rIMap;
    maAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );

    return( maAny.hasValue() != NULL );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetTransferableObjectDescriptor( const TransferableObjectDescriptor& rDesc,
                                                              const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    SvMemoryStream aMemStm( 1024, 1024 );

    aMemStm << rDesc;
    maAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Tell() );

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
            strcpy( reinterpret_cast< char* >( aSeq.getArray() ), ByteString( rBmk.GetURL(), eSysCSet).GetBuffer() );
            strcpy( reinterpret_cast< char* >( aSeq.getArray() ) + 1024, ByteString( rBmk.GetDescription(), eSysCSet ).GetBuffer() );

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

    maAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );

    return( maAny.hasValue() != NULL );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableHelper::SetFileList( const FileList& rFileList,
                                          const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    SvMemoryStream aMemStm( 4096, 4096 );

    aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
    aMemStm << rFileList;

    maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );

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
        {
            //JP 24.7.2001: as I know was this only for the writer application and this
            //              writes now UTF16 format into the stream
            //JP 6.8.2001:  and now it writes UTF8 because then exist no problem with
            //              little / big endians! - Bug 88121
            maAny <<= ::rtl::OUString( reinterpret_cast< const sal_Char* >( aSeq.getConstArray() ), nLen - 1, RTL_TEXTENCODING_UTF8 );
        }
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

void TransferableHelper::CopyToClipboard( Window *pWindow ) const
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );
    Reference< XClipboard > xClipboard;

    if( pWindow )
        xClipboard = pWindow->GetClipboard();

    if( xClipboard.is() )
        mxClipboard = xClipboard;

    if( mxClipboard.is() && !mxTerminateListener.is() )
    {
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

        try
        {
            TransferableHelper*                 pThis = const_cast< TransferableHelper* >( this );
            Reference< XMultiServiceFactory >   xFact( ::comphelper::getProcessServiceFactory() );

            if( xFact.is() )
            {
                Reference< XDesktop > xDesktop( xFact->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );

                if( xDesktop.is() )
                    xDesktop->addTerminateListener( pThis->mxTerminateListener = new TerminateListener( *pThis ) );
            }

            mxClipboard->setContents( pThis, pThis );
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        Application::AcquireSolarMutex( nRef );
    }
}

// -----------------------------------------------------------------------------

void TransferableHelper::CopyToSelection( Window *pWindow ) const
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );
    Reference< XClipboard > xSelection;

    if( pWindow )
        xSelection = pWindow->GetSelection();

    if( xSelection.is() && !mxTerminateListener.is() )
    {
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

        try
        {
            TransferableHelper*                 pThis = const_cast< TransferableHelper* >( this );
            Reference< XMultiServiceFactory >   xFact( ::comphelper::getProcessServiceFactory() );

            if( xFact.is() )
            {
                Reference< XDesktop > xDesktop( xFact->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );

                if( xDesktop.is() )
                    xDesktop->addTerminateListener( pThis->mxTerminateListener = new TerminateListener( *pThis ) );
            }

            xSelection->setContents( pThis, pThis );
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        Application::AcquireSolarMutex( nRef );
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
        /*
         *    #96792# release mouse before actually starting DnD.
         *    This is necessary for the X11 DnD implementation to work.
         */
        if( pWindow->IsMouseCaptured() )
            pWindow->ReleaseMouse();

        const Point aPt( pWindow->GetPointerPosPixel() );

        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

        try
        {
            DragGestureEvent    aEvt;
            aEvt.DragAction = DNDConstants::ACTION_COPY;
            aEvt.DragOriginX = aPt.X();
            aEvt.DragOriginY = aPt.Y();
            aEvt.DragSource = xDragSource;

            xDragSource->startDrag( aEvt, nDnDSourceActions, nDnDPointer, nDnDImage, this, this );
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        Application::AcquireSolarMutex( nRef );
    }
}

// -----------------------------------------------------------------------------

void TransferableHelper::ClearSelection( Window *pWindow )
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );
    Reference< XClipboard > xSelection( pWindow->GetSelection() );

    if( xSelection.is() )
        xSelection->setContents( NULL, NULL );
}

// -----------------------------------------------------------------------------

Reference< XClipboard> TransferableHelper::GetSystemClipboard()
{
    Window *pFocusWindow = Application::GetFocusWindow();

    if( pFocusWindow )
        return pFocusWindow->GetClipboard();

    return  Reference< XClipboard > ();
}

// -----------------------------------------------------------------------------

const Sequence< sal_Int8 >& TransferableHelper::getUnoTunnelId()
{
    static Sequence< sal_Int8 > aSeq;

    if( !aSeq.getLength() )
    {
        static osl::Mutex           aCreateMutex;
        osl::Guard< osl::Mutex >    aGuard( aCreateMutex );

        aSeq.realloc( 16 );
        rtl_createUuid( reinterpret_cast< sal_uInt8* >( aSeq.getArray() ), 0, sal_True );
    }


    return aSeq;
}

// ---------------------------------
// - TransferableClipboardNotifier -
// ---------------------------------

class TransferableClipboardNotifier : public ::cppu::WeakImplHelper1< XClipboardListener >
{
private:

    TransferableDataHelper*     mpListener;

protected:

    // XClipboardListener
    virtual void SAL_CALL changedContents( const clipboard::ClipboardEvent& event ) throw (RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);

public:

                                TransferableClipboardNotifier( TransferableDataHelper* _pListener );
};

// -----------------------------------------------------------------------------

TransferableClipboardNotifier::TransferableClipboardNotifier( TransferableDataHelper* _pListener ) :
    mpListener( _pListener )
{
    DBG_ASSERT( mpListener, "TransferableClipboardNotifier::TransferableClipboardNotifier: invalid master listener!" );
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableClipboardNotifier::changedContents( const clipboard::ClipboardEvent& event ) throw (RuntimeException)
{
    if( mpListener )
        mpListener->ClipboardContentChanged( event.Contents );
}

// -----------------------------------------------------------------------------

void SAL_CALL TransferableClipboardNotifier::disposing( const EventObject& Source ) throw (RuntimeException)
{
    mpListener = NULL;

}

// --------------------------
// - TransferableDataHelper -
// --------------------------

TransferableDataHelper::TransferableDataHelper() :
    mpFormats( new DataFlavorExVector ),
    mpClipboardListener( NULL )
{
}

// -----------------------------------------------------------------------------

TransferableDataHelper::TransferableDataHelper( const Reference< ::com::sun::star::datatransfer::XTransferable >& rxTransferable ) :
    mxTransfer( rxTransferable ),
    mpFormats( new DataFlavorExVector ),

    mpClipboardListener( NULL )
{
    InitFormats();
}

// -----------------------------------------------------------------------------

TransferableDataHelper::TransferableDataHelper( const TransferableDataHelper& rDataHelper ) :
    mxTransfer( rDataHelper.mxTransfer ),
    mpFormats( new DataFlavorExVector( *rDataHelper.mpFormats ) ),
    mxClipboard( rDataHelper.mxClipboard ),
    mpClipboardListener( NULL )
{
}

// -----------------------------------------------------------------------------

TransferableDataHelper& TransferableDataHelper::operator=( const TransferableDataHelper& rDataHelper )
{
    if ( this != &rDataHelper )
    {
        mxTransfer = rDataHelper.mxTransfer;
        delete mpFormats, mpFormats = new DataFlavorExVector( *rDataHelper.mpFormats );

        mxClipboard = rDataHelper.mxClipboard;
        if( mpClipboardListener )
            StopClipboardListening();
    }

    return *this;
}

// -----------------------------------------------------------------------------

TransferableDataHelper::~TransferableDataHelper()
{
    delete mpFormats;
    StopClipboardListening( );
}

// -----------------------------------------------------------------------------

void TransferableDataHelper::FillDataFlavorExVector( const Sequence< DataFlavor >& rDataFlavorSeq,
                                                     DataFlavorExVector& rDataFlavorExVector )
{
    try
    {
        Reference< XMultiServiceFactory >       xFact( ::comphelper::getProcessServiceFactory() );
        Reference< XMimeContentTypeFactory >    xMimeFact;
        DataFlavorEx                            aFlavorEx;
        const ::rtl::OUString                   aCharsetStr( ::rtl::OUString::createFromAscii( "charset" ) );

        if( xFact.is() )
            xMimeFact = Reference< XMimeContentTypeFactory >( xFact->createInstance( ::rtl::OUString::createFromAscii(
                                                              "com.sun.star.datatransfer.MimeContentTypeFactory" ) ),
                                                              UNO_QUERY );

        for( sal_Int32 i = 0; i < rDataFlavorSeq.getLength(); i++ )
        {
            const DataFlavor&               rFlavor = rDataFlavorSeq[ i ];
            Reference< XMimeContentType >   xMimeType;

            try
            {
                if( xMimeFact.is() && rFlavor.MimeType.getLength() )
                    xMimeType = xMimeFact->createMimeContentType( rFlavor.MimeType );
            }
            catch( const ::com::sun::star::uno::Exception& )
            {

            }

            aFlavorEx.MimeType = rFlavor.MimeType;
            aFlavorEx.HumanPresentableName = rFlavor.HumanPresentableName;
            aFlavorEx.DataType = rFlavor.DataType;
            aFlavorEx.mnSotId = SotExchange::RegisterFormat( rFlavor );

            rDataFlavorExVector.push_back( aFlavorEx );

            // add additional formats for special mime types
            if( SOT_FORMATSTR_ID_BMP == aFlavorEx.mnSotId )
            {
                if( SotExchange::GetFormatDataFlavor( SOT_FORMAT_BITMAP, aFlavorEx ) )
                {
                    aFlavorEx.mnSotId = SOT_FORMAT_BITMAP;
                    rDataFlavorExVector.push_back( aFlavorEx );
                }
            }
            else if( SOT_FORMATSTR_ID_WMF == aFlavorEx.mnSotId || SOT_FORMATSTR_ID_EMF == aFlavorEx.mnSotId )
            {
                if( SotExchange::GetFormatDataFlavor( SOT_FORMAT_GDIMETAFILE, aFlavorEx ) )
                {
                    aFlavorEx.mnSotId = SOT_FORMAT_GDIMETAFILE;
                    rDataFlavorExVector.push_back( aFlavorEx );
                }
            }
            else if ( SOT_FORMATSTR_ID_HTML_SIMPLE == aFlavorEx.mnSotId  )
            {
                // #104735# HTML_SIMPLE may also be inserted without comments
                aFlavorEx.mnSotId = SOT_FORMATSTR_ID_HTML_NO_COMMENT;
                rDataFlavorExVector.push_back( aFlavorEx );
            }
            else if( xMimeType.is() && xMimeType->getFullMediaType().equalsIgnoreAsciiCase( ::rtl::OUString::createFromAscii( "text/plain" ) ) )
            {
                // add, if it is a UTF-8 byte buffer
                if( xMimeType->hasParameter( aCharsetStr ) )
                {
                    const ::rtl::OUString aCharset( xMimeType->getParameterValue( aCharsetStr ) );

                    if( xMimeType->getParameterValue( aCharsetStr ).equalsIgnoreAsciiCase( ::rtl::OUString::createFromAscii( "unicode" ) ) ||
                        xMimeType->getParameterValue( aCharsetStr ).equalsIgnoreAsciiCase( ::rtl::OUString::createFromAscii( "utf-16" ) ) )
                    {
                        rDataFlavorExVector[ rDataFlavorExVector.size() - 1 ].mnSotId = FORMAT_STRING;

                    }
                }
            }
            else if( xMimeType.is() && xMimeType->getFullMediaType().equalsIgnoreAsciiCase( ::rtl::OUString::createFromAscii( "text/rtf" ) ) )
            {
                rDataFlavorExVector[ rDataFlavorExVector.size() - 1 ].mnSotId = FORMAT_RTF;
            }
            else if( xMimeType.is() && xMimeType->getFullMediaType().equalsIgnoreAsciiCase( ::rtl::OUString::createFromAscii( "text/html" ) ) )

            {
                rDataFlavorExVector[ rDataFlavorExVector.size() - 1 ].mnSotId = SOT_FORMATSTR_ID_HTML;
            }
            else if( xMimeType.is() && xMimeType->getFullMediaType().equalsIgnoreAsciiCase( ::rtl::OUString::createFromAscii( "text/uri-list" ) ) )
            {
                rDataFlavorExVector[ rDataFlavorExVector.size() - 1 ].mnSotId = SOT_FORMAT_FILE_LIST;
            }
        }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

void TransferableDataHelper::InitFormats()
{
    mpFormats->clear();

    if( mxTransfer.is() )
        TransferableDataHelper::FillDataFlavorExVector( mxTransfer->getTransferDataFlavors(), *mpFormats );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::HasFormat( SotFormatStringId nFormat ) const
{
    DataFlavorExVector::iterator    aIter( mpFormats->begin() ), aEnd( mpFormats->end() );
    sal_Bool                        bRet = sal_False;

    while( aIter != aEnd )
    {
        if( nFormat == (*aIter++).mnSotId )
        {
            aIter = aEnd;
            bRet = sal_True;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::HasFormat( const DataFlavor& rFlavor ) const
{
    DataFlavorExVector::iterator    aIter( mpFormats->begin() ), aEnd( mpFormats->end() );
    sal_Bool                        bRet = sal_False;

    while( aIter != aEnd )
    {
        if( TransferableDataHelper::IsEqual( rFlavor, *aIter++ ) )
        {
            aIter = aEnd;
            bRet = sal_True;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_uInt32 TransferableDataHelper::GetFormatCount() const
{
    return mpFormats->size();
}

// -----------------------------------------------------------------------------


SotFormatStringId TransferableDataHelper::GetFormat( sal_uInt32 nFormat ) const
{
    DBG_ASSERT( nFormat < mpFormats->size(), "TransferableDataHelper::GetFormat: invalid format index" );
    return( ( nFormat < mpFormats->size() ) ? (*mpFormats)[ nFormat ].mnSotId : 0 );
}

// -----------------------------------------------------------------------------

DataFlavor TransferableDataHelper::GetFormatDataFlavor( sal_uInt32 nFormat ) const
{
    DBG_ASSERT( nFormat < mpFormats->size(), "TransferableDataHelper::GetFormat: invalid format index" );

    DataFlavor aRet;

    if( nFormat < mpFormats->size() )
        aRet = (*mpFormats)[ nFormat ];

    return aRet;
}

// -----------------------------------------------------------------------------

Reference< XTransferable > TransferableDataHelper::GetXTransferable() const
{
    Reference< XTransferable > xRet;

    if( mxTransfer.is() )
    {
        try
        {
            xRet = mxTransfer;

            // do a dummy call to check, if this interface is valid (nasty)
            Sequence< DataFlavor > aTestSeq( xRet->getTransferDataFlavors() );

        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            xRet = Reference< XTransferable >();
        }
    }

    return xRet;
}

// -----------------------------------------------------------------------------

Any TransferableDataHelper::GetAny( SotFormatStringId nFormat ) const
{
    Any aReturn;

    DataFlavor aFlavor;
    if ( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) )
        aReturn = GetAny( aFlavor );

    return aReturn;
}


// -----------------------------------------------------------------------------

Any TransferableDataHelper::GetAny( const DataFlavor& rFlavor ) const
{
    Any aRet;

    try
    {
        if( mxTransfer.is() )
        {
            DataFlavorExVector::iterator    aIter( mpFormats->begin() ), aEnd( mpFormats->end() );
            const SotFormatStringId         nRequestFormat = SotExchange::GetFormat( rFlavor );

            if( nRequestFormat )
            {
                // try to get alien format first
                while( aIter != aEnd )
                {
                    if( ( nRequestFormat == (*aIter).mnSotId ) && !rFlavor.MimeType.equalsIgnoreAsciiCase( (*aIter).MimeType ) )
                        aRet = mxTransfer->getTransferData( *aIter );

                    if( aRet.hasValue() )
                        aIter = aEnd;
                    else
                        aIter++;
                }
            }

            if( !aRet.hasValue() )
                aRet = mxTransfer->getTransferData( rFlavor );
        }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }

    return aRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetString( SotFormatStringId nFormat, String& rStr )
{
    ::rtl::OUString aOUString;
    sal_Bool        bRet = GetString( nFormat, aOUString );

    rStr = aOUString;

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetString( const DataFlavor& rFlavor, String& rStr )
{
    ::rtl::OUString aOUString;
    sal_Bool        bRet = GetString( rFlavor, aOUString );

    rStr = aOUString;

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetString( SotFormatStringId nFormat, ::rtl::OUString& rStr )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetString( aFlavor, rStr ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetString( const DataFlavor& rFlavor, ::rtl::OUString& rStr )
{
    Any         aAny( GetAny( rFlavor ) );
    sal_Bool    bRet = sal_False;

    if( aAny.hasValue() )
    {
        ::rtl::OUString         aOUString;
        Sequence< sal_Int8 >    aSeq;

        if( aAny >>= aOUString )
        {
            rStr = aOUString;
            bRet = sal_True;
        }
        else if( aAny >>= aSeq )
        {

            const sal_Char* pChars = reinterpret_cast< const sal_Char* >( aSeq.getConstArray() );
            sal_Int32       nLen = aSeq.getLength();

            //JP 10.10.2001: 92930 - don't copy the last zero characterinto the string.
            //DVO 2002-05-27: strip _all_ trailing zeros
            while( nLen && ( 0 == *( pChars + nLen - 1 ) ) )
                --nLen;

            rStr = ::rtl::OUString( pChars, nLen, gsl_getSystemTextEncoding() );
            bRet = sal_True;
        }
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
    DataFlavor          aSubstFlavor;
    sal_Bool            bRet = GetSotStorageStream( rFlavor, xStm );

    if( bRet )
    {
        *xStm >> rBmp;
        bRet = ( xStm->GetError() == ERRCODE_NONE );
    }

    if( !bRet &&
        HasFormat( SOT_FORMATSTR_ID_BMP ) &&
        SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_BMP, aSubstFlavor ) &&
        GetSotStorageStream( aSubstFlavor, xStm ) )
    {
        xStm->ResetError();
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
    DataFlavor          aSubstFlavor;
    sal_Bool            bRet = sal_False;

    if( GetSotStorageStream( rFlavor, xStm ) )
    {
        *xStm >> rMtf;
        bRet = ( xStm->GetError() == ERRCODE_NONE );
    }

    if( !bRet &&
        HasFormat( SOT_FORMATSTR_ID_EMF ) &&
        SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_EMF, aSubstFlavor ) &&
        GetSotStorageStream( aSubstFlavor, xStm ) )
    {
        Graphic aGraphic;

        if( GraphicConverter::Import( *xStm, aGraphic ) == ERRCODE_NONE )
        {
            rMtf = aGraphic.GetGDIMetaFile();
            bRet = TRUE;
        }
    }

    if( !bRet &&
        HasFormat( SOT_FORMATSTR_ID_WMF ) &&
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
    sal_Bool bRet = sal_False;
    if( HasFormat( rFlavor ))
    {
    const SotFormatStringId nFormat = SotExchange::GetFormat( rFlavor );
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
                rBmk = INetBookmark( String( reinterpret_cast< const sal_Char* >( aSeq.getConstArray() ), gsl_getSystemTextEncoding() ),
                                     String( reinterpret_cast< const sal_Char* >( aSeq.getConstArray() ) + 1024, gsl_getSystemTextEncoding() ) );
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
                        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( INetURLObject( String( aDesc, eTextEncoding ) ).GetMainURL( INetURLObject::NO_DECODE ),
                                                                                  STREAM_STD_READ );

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

    }
    }
    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetINetImage( SotFormatStringId nFormat,
                                                INetImage& rINtImg )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetINetImage( aFlavor, rINtImg ) );
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
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetFileList( aFlavor, rFileList ) );
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::GetFileList(
            const ::com::sun::star::datatransfer::DataFlavor& rFlavor,
            FileList& rFileList )
{
    SotStorageStreamRef xStm;
    sal_Bool            bRet = sal_False;

    for( sal_uInt32 i = 0, nFormatCount = GetFormatCount(); ( i < nFormatCount ) && !bRet; ++i )
    {
        if( SOT_FORMAT_FILE_LIST == GetFormat( i ) )
        {
            const DataFlavor aFlavor( GetFormatDataFlavor( i ) );

            if( GetSotStorageStream( aFlavor, xStm ) )
            {
                if( aFlavor.MimeType.indexOf( ::rtl::OUString::createFromAscii( "text/uri-list" ) ) > -1 )
                {
                    ByteString aByteString;

                    while( xStm->ReadLine( aByteString ) )
                        if( aByteString.Len() && aByteString.GetChar( 0 ) != '#' )
                            rFileList.AppendFile( String( aByteString, RTL_TEXTENCODING_UTF8 ) );

                    bRet = sal_True;
                 }
                 else
                    bRet = ( ( *xStm >> rFileList ).GetError() == ERRCODE_NONE );
            }
        }
    }

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
void TransferableDataHelper::ClipboardContentChanged( const Reference< XTransferable >& _rxNewContent )
{
    mxTransfer = _rxNewContent;
    InitFormats();
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::StartClipboardListening( )
{
    if( mpClipboardListener )
        StopClipboardListening( );

    Reference< XClipboardNotifier > xNotifier( mxClipboard, UNO_QUERY );

    if( xNotifier.is() )
    {
        mpClipboardListener = new TransferableClipboardNotifier( this );
        xNotifier->addClipboardListener( mpClipboardListener );

        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------------

void TransferableDataHelper::StopClipboardListening( )
{
    Reference< XClipboardNotifier > xNotifier( mxClipboard, UNO_QUERY );

    if( mpClipboardListener && xNotifier.is() )
          xNotifier->removeClipboardListener( mpClipboardListener );

    mpClipboardListener = NULL;
}

// -----------------------------------------------------------------------------

TransferableDataHelper TransferableDataHelper::CreateFromSystemClipboard( Window * pWindow )
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );

    Reference< XClipboard > xClipboard;
       TransferableDataHelper   aRet;

    if( pWindow )
        xClipboard = pWindow->GetClipboard();

    if( xClipboard.is() )
       {
           try

        {
            Reference< XTransferable > xTransferable( xClipboard->getContents() );

            if( xTransferable.is() )
            {
                aRet = TransferableDataHelper( xTransferable );
                   aRet.mxClipboard = xClipboard;
                    // also copy the clipboard - 99030 - 23.05.2002 - fs@openoffice.org
            }
           }
        catch( const ::com::sun::star::uno::Exception& )
        {
           }
    }

    return aRet;
}


// -----------------------------------------------------------------------------

TransferableDataHelper TransferableDataHelper::CreateFromSelection( Window* pWindow )
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );

    Reference< XClipboard > xSelection( pWindow->GetSelection() );
       TransferableDataHelper   aRet;

    if( pWindow )
        xSelection = pWindow->GetSelection();

    if( xSelection.is() )
       {
        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

          try
        {
            Reference< XTransferable > xTransferable( xSelection->getContents() );

            if( xTransferable.is() )
               {
                aRet = TransferableDataHelper( xTransferable );
                   aRet.mxClipboard = xSelection;
            }
           }
        catch( const ::com::sun::star::uno::Exception& )
        {
           }

        Application::AcquireSolarMutex( nRef );
    }

    return aRet;
}

// -----------------------------------------------------------------------------

sal_Bool TransferableDataHelper::IsEqual( const ::com::sun::star::datatransfer::DataFlavor& rInternalFlavor,
                                          const ::com::sun::star::datatransfer::DataFlavor& rRequestFlavor,
                                          sal_Bool bCompareParameters )
{
    Reference< XMultiServiceFactory >       xFact( ::comphelper::getProcessServiceFactory() );
    Reference< XMimeContentTypeFactory >    xMimeFact;
    sal_Bool                                bRet = sal_False;

    try
    {
        if( xFact.is() )
            xMimeFact = Reference< XMimeContentTypeFactory >( xFact->createInstance( ::rtl::OUString::createFromAscii(
                                                              "com.sun.star.datatransfer.MimeContentTypeFactory" ) ),
                                                              UNO_QUERY );

        if( xMimeFact.is() )
        {
            Reference< XMimeContentType > xRequestType1( xMimeFact->createMimeContentType( rInternalFlavor.MimeType ) );
            Reference< XMimeContentType > xRequestType2( xMimeFact->createMimeContentType( rRequestFlavor.MimeType ) );

            if( xRequestType1.is() && xRequestType2.is() )
            {
                if( xRequestType1->getFullMediaType().equalsIgnoreAsciiCase( xRequestType2->getFullMediaType() ) )
                {
                    if( xRequestType1->getFullMediaType().equalsIgnoreAsciiCase( ::rtl::OUString::createFromAscii( "text/plain" ) ) )
                    {
                        // special handling for text/plain media types
                        const ::rtl::OUString aCharsetString( ::rtl::OUString::createFromAscii( "charset" ) );

                        if( !xRequestType2->hasParameter( aCharsetString ) ||
                            xRequestType2->getParameterValue( aCharsetString ).equalsIgnoreAsciiCase( ::rtl::OUString::createFromAscii( "utf-16" ) ) ||
                            xRequestType2->getParameterValue( aCharsetString ).equalsIgnoreAsciiCase( ::rtl::OUString::createFromAscii( "unicode" ) ) )
                        {
                            bRet = sal_True;
                        }
                    }
                    else if( xRequestType1->getFullMediaType().equalsIgnoreAsciiCase( ::rtl::OUString::createFromAscii( "application/x-openoffice" ) ) )
                    {
                        // special handling for application/x-openoffice media types
                        const ::rtl::OUString aFormatString( ::rtl::OUString::createFromAscii( "windows_formatname" ) );

                        if( xRequestType1->hasParameter( aFormatString ) &&
                            xRequestType2->hasParameter( aFormatString ) &&
                            xRequestType1->getParameterValue( aFormatString ).equalsIgnoreAsciiCase( xRequestType2->getParameterValue( aFormatString ) ) )
                        {
                            bRet = sal_True;
                        }
                    }
                    else
                        bRet = sal_True;
                }
            }
        }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        bRet = rInternalFlavor.MimeType.equalsIgnoreAsciiCase( rRequestFlavor.MimeType );
    }

    return bRet;
}
