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

#ifdef WNT
#include <prewin.h>
#include <postwin.h>
#include <shlobj.h>
#endif
#include <osl/mutex.hxx>
#include <rtl/uri.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <sot/exchange.hxx>
#include <sot/storage.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/sequence.hxx>
#include <sot/filelist.hxx>
#include <cppuhelper/implbase.hxx>

#include <comphelper/seqstream.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/datatransfer/MimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/XTransferable2.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <svl/urlbmk.hxx>
#include <svtools/inetimg.hxx>
#include <vcl/wmf.hxx>
#include <svtools/imap.hxx>
#include <svtools/transfer.hxx>
#include <rtl/strbuf.hxx>
#include <cstdio>
#include <vcl/dibtools.hxx>
#include <vcl/pngread.hxx>
#include <vcl/pngwrite.hxx>
#include <memory>

// - Namespaces -


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;


// - TransferableObjectDescriptor -


#define TOD_SIG1 0x01234567
#define TOD_SIG2 0x89abcdef

SvStream& WriteTransferableObjectDescriptor( SvStream& rOStm, const TransferableObjectDescriptor& rObjDesc )
{
    const sal_uInt32    nFirstPos = rOStm.Tell(), nViewAspect = rObjDesc.mnViewAspect;
    const sal_uInt32    nSig1 = TOD_SIG1, nSig2 = TOD_SIG2;

    rOStm.SeekRel( 4 );
    WriteSvGlobalName( rOStm, rObjDesc.maClassName );
    rOStm.WriteUInt32( nViewAspect );
    rOStm.WriteInt32( rObjDesc.maSize.Width() );
    rOStm.WriteInt32( rObjDesc.maSize.Height() );
    rOStm.WriteInt32( rObjDesc.maDragStartPos.X() );
    rOStm.WriteInt32( rObjDesc.maDragStartPos.Y() );
    rOStm.WriteUniOrByteString( rObjDesc.maTypeName, osl_getThreadTextEncoding() );
    rOStm.WriteUniOrByteString( rObjDesc.maDisplayName, osl_getThreadTextEncoding() );
    rOStm.WriteUInt32( nSig1 ).WriteUInt32( nSig2 );

    const sal_uInt32 nLastPos = rOStm.Tell();

    rOStm.Seek( nFirstPos );
    rOStm.WriteUInt32( nLastPos - nFirstPos  );
    rOStm.Seek( nLastPos );

    return rOStm;
}


// the reading of the parameter is done using the special service css::datatransfer::MimeContentType,
// a similar approach should be implemented for creation of the mimetype string;
// for now the set of acceptable characters has to be hardcoded, in future it should be part of the service that creates the mimetype

static OUString ImplGetParameterString( const TransferableObjectDescriptor& rObjDesc )
{
    const OUString   aClassName( rObjDesc.maClassName.GetHexName() );
    OUString         aParams;

    if( !aClassName.isEmpty() )
    {
        aParams += ";classname=\"" + aClassName + "\"";
    }

    if( !rObjDesc.maTypeName.isEmpty() )
    {
        aParams += ";typename=\""  + rObjDesc.maTypeName + "\"";
    }

    if( !rObjDesc.maDisplayName.isEmpty() )
    {
        // the display name might contain unacceptable characters, encode all of them
        // this seems to be the only parameter currently that might contain such characters
        sal_Bool pToAccept[128];
        for ( sal_Int32 nBInd = 0; nBInd < 128; nBInd++ )
            pToAccept[nBInd] = sal_False;

        const char aQuotedParamChars[] =
            "()<>@,;:/[]?=!#$&'*+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ^_`abcdefghijklmnopqrstuvwxyz{|}~. ";

        for ( sal_Int32 nInd = 0; nInd < RTL_CONSTASCII_LENGTH(aQuotedParamChars); ++nInd )
        {
            sal_Unicode nChar = aQuotedParamChars[nInd];
            if ( nChar < 128 )
                pToAccept[nChar] = sal_True;
        }

        aParams += ";displayname=\""
            + rtl::Uri::encode(
                rObjDesc.maDisplayName, pToAccept, rtl_UriEncodeIgnoreEscapes,
                RTL_TEXTENCODING_UTF8)
            + "\"";
    }

    aParams += ";viewaspect=\"" + OUString::number(rObjDesc.mnViewAspect)
        + "\";width=\"" + OUString::number(rObjDesc.maSize.Width())
        + "\";height=\"" + OUString::number(rObjDesc.maSize.Height())
        + "\";posx=\"" + OUString::number(rObjDesc.maDragStartPos.X())
        + "\";posy=\"" + OUString::number(rObjDesc.maDragStartPos.X()) + "\"";

    return aParams;
}



static void ImplSetParameterString( TransferableObjectDescriptor& rObjDesc, const DataFlavorEx& rFlavorEx )
{
    Reference< XComponentContext >       xContext( ::comphelper::getProcessComponentContext() );

    try
    {
        Reference< XMimeContentTypeFactory >  xMimeFact = MimeContentTypeFactory::create( xContext );

        Reference< XMimeContentType > xMimeType( xMimeFact->createMimeContentType( rFlavorEx.MimeType ) );

        if( xMimeType.is() )
        {
            const OUString aClassNameString( "classname" );
            const OUString aTypeNameString( "typename" );
            const OUString aDisplayNameString( "displayname" );
            const OUString aViewAspectString( "viewaspect" );
            const OUString aWidthString( "width" );
            const OUString aHeightString( "height" );
            const OUString aPosXString( "posx" );
            const OUString aPosYString( "posy" );

            if( xMimeType->hasParameter( aClassNameString ) )
            {
                rObjDesc.maClassName.MakeId( xMimeType->getParameterValue( aClassNameString ) );
            }

            if( xMimeType->hasParameter( aTypeNameString ) )
            {
                rObjDesc.maTypeName = xMimeType->getParameterValue( aTypeNameString );
            }

            if( xMimeType->hasParameter( aDisplayNameString ) )
            {
                // the display name might contain unacceptable characters, in this case they should be encoded
                // this seems to be the only parameter currently that might contain such characters
                rObjDesc.maDisplayName = ::rtl::Uri::decode( xMimeType->getParameterValue( aDisplayNameString ), rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
            }

            if( xMimeType->hasParameter( aViewAspectString ) )
            {
                rObjDesc.mnViewAspect = static_cast< sal_uInt16 >( xMimeType->getParameterValue( aViewAspectString ).toInt32() );
            }

            if( xMimeType->hasParameter( aWidthString ) )
            {
                rObjDesc.maSize.Width() = xMimeType->getParameterValue( aWidthString ).toInt32();
            }

            if( xMimeType->hasParameter( aHeightString ) )
            {
                rObjDesc.maSize.Height() = xMimeType->getParameterValue( aHeightString ).toInt32();
            }

            if( xMimeType->hasParameter( aPosXString ) )
            {
                rObjDesc.maDragStartPos.X() = xMimeType->getParameterValue( aPosXString ).toInt32();
            }

            if( xMimeType->hasParameter( aPosYString ) )
            {
                rObjDesc.maDragStartPos.Y() = xMimeType->getParameterValue( aPosYString ).toInt32();
            }
        }
    }
    catch( const css::uno::Exception& )
    {
    }
}


// - TransferableHelper::TerminateListener -


TransferableHelper::TerminateListener::TerminateListener( TransferableHelper& rTransferableHelper ) :
    mrParent( rTransferableHelper )
{
}



TransferableHelper::TerminateListener::~TerminateListener()
{
}



void SAL_CALL TransferableHelper::TerminateListener::disposing( const EventObject& ) throw( RuntimeException, std::exception )
{
}



void SAL_CALL TransferableHelper::TerminateListener::queryTermination( const EventObject& ) throw( TerminationVetoException, RuntimeException, std::exception )
{
}



void SAL_CALL TransferableHelper::TerminateListener::notifyTermination( const EventObject& ) throw( RuntimeException, std::exception )
{
    mrParent.ImplFlush();
}


// - TransferableHelper -


TransferableHelper::TransferableHelper() :
    mpFormats( new DataFlavorExVector ),
    mpObjDesc( nullptr )
{
}



TransferableHelper::~TransferableHelper()
{
    delete mpObjDesc;
    delete mpFormats;
}


Any SAL_CALL TransferableHelper::getTransferData( const DataFlavor& rFlavor )
    throw (UnsupportedFlavorException, IOException, RuntimeException, std::exception)
{
    return getTransferData2(rFlavor, OUString());
}

Any SAL_CALL TransferableHelper::getTransferData2( const DataFlavor& rFlavor, const OUString& rDestDoc )
    throw (UnsupportedFlavorException, IOException, RuntimeException, std::exception)
{
    if( !maAny.hasValue() || mpFormats->empty() || ( maLastFormat != rFlavor.MimeType ) )
    {
        const SolarMutexGuard aGuard;

        maLastFormat = rFlavor.MimeType;
        maAny = Any();

        try
        {
            DataFlavor  aSubstFlavor;
            bool        bDone = false;

            // add formats if not already done
            if( mpFormats->empty() )
                AddSupportedFormats();

            // check alien formats first and try to get a substitution format
            if( SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aSubstFlavor ) &&
                TransferableDataHelper::IsEqual( aSubstFlavor, rFlavor ) )
            {
                GetData(aSubstFlavor, rDestDoc);
                bDone = maAny.hasValue();
            }
            else if(SotExchange::GetFormatDataFlavor(SotClipboardFormatId::BMP, aSubstFlavor )
                && TransferableDataHelper::IsEqual( aSubstFlavor, rFlavor )
                && SotExchange::GetFormatDataFlavor(SotClipboardFormatId::BITMAP, aSubstFlavor))
            {
                GetData(aSubstFlavor, rDestDoc);
                bDone = true;
            }
            else if( SotExchange::GetFormatDataFlavor( SotClipboardFormatId::EMF, aSubstFlavor ) &&
                     TransferableDataHelper::IsEqual( aSubstFlavor, rFlavor ) &&
                     SotExchange::GetFormatDataFlavor( SotClipboardFormatId::GDIMETAFILE, aSubstFlavor ) )
            {
                GetData(aSubstFlavor, rDestDoc);

                if( maAny.hasValue() )
                {
                    Sequence< sal_Int8 > aSeq;

                    if( maAny >>= aSeq )
                    {
                        std::unique_ptr<SvMemoryStream> pSrcStm(new SvMemoryStream( aSeq.getArray(), aSeq.getLength(), StreamMode::WRITE | StreamMode::TRUNC ));
                        GDIMetaFile     aMtf;

                        ReadGDIMetaFile( *pSrcStm, aMtf );
                        pSrcStm.reset();

                        Graphic         aGraphic( aMtf );
                        SvMemoryStream  aDstStm( 65535, 65535 );

                        if( GraphicConverter::Export( aDstStm, aGraphic, ConvertDataFormat::EMF ) == ERRCODE_NONE )
                        {
                            maAny <<= ( aSeq = Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aDstStm.GetData() ),
                                                                     aDstStm.Seek( STREAM_SEEK_TO_END ) ) );
                            bDone = true;
                        }
                    }
                }
            }
            else if( SotExchange::GetFormatDataFlavor( SotClipboardFormatId::WMF, aSubstFlavor ) &&
                     TransferableDataHelper::IsEqual( aSubstFlavor, rFlavor ) &&
                     SotExchange::GetFormatDataFlavor( SotClipboardFormatId::GDIMETAFILE, aSubstFlavor ) )
            {
                GetData(aSubstFlavor, rDestDoc);

                if( maAny.hasValue() )
                {
                    Sequence< sal_Int8 > aSeq;

                    if( maAny >>= aSeq )
                    {
                        std::unique_ptr<SvMemoryStream> pSrcStm(new SvMemoryStream( aSeq.getArray(), aSeq.getLength(), StreamMode::WRITE | StreamMode::TRUNC ));
                        GDIMetaFile     aMtf;

                        ReadGDIMetaFile( *pSrcStm, aMtf );
                        pSrcStm.reset();

                        SvMemoryStream  aDstStm( 65535, 65535 );

                        // taking wmf without file header
                        if ( ConvertGDIMetaFileToWMF( aMtf, aDstStm, nullptr, false ) )
                        {
                            maAny <<= ( aSeq = Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aDstStm.GetData() ),
                                                                     aDstStm.Seek( STREAM_SEEK_TO_END ) ) );
                            bDone = true;
                        }
                    }
                }
            }

            // reset Any if substitute doesn't work
            if( !bDone && maAny.hasValue() )
                maAny = Any();

            // if any is not yet filled, use standard format
            if( !maAny.hasValue() )
                GetData(rFlavor, rDestDoc);

#ifdef DEBUG
            if( maAny.hasValue() && css::uno::TypeClass_STRING != maAny.getValueType().getTypeClass() )
                fprintf( stderr, "TransferableHelper delivers sequence of data [ %s ]\n", OUStringToOString(rFlavor.MimeType, RTL_TEXTENCODING_ASCII_US).getStr() );
#endif
        }
        catch( const css::uno::Exception& )
        {
        }

        if( !maAny.hasValue() )
            throw UnsupportedFlavorException();
    }

    return maAny;
}



Sequence< DataFlavor > SAL_CALL TransferableHelper::getTransferDataFlavors() throw( RuntimeException, std::exception )
{
    const SolarMutexGuard aGuard;

    try
    {
        if( mpFormats->empty() )
            AddSupportedFormats();
    }
    catch( const css::uno::Exception& )
    {
    }

    return comphelper::containerToSequence<DataFlavor>(*mpFormats);
}



sal_Bool SAL_CALL TransferableHelper::isDataFlavorSupported( const DataFlavor& rFlavor ) throw( RuntimeException, std::exception )
{
    const SolarMutexGuard aGuard;
    bool bRet = false;

    try
    {
        if( mpFormats->empty() )
            AddSupportedFormats();
    }
    catch( const css::uno::Exception& )
    {
    }

    for (DataFlavorExVector::const_iterator aIter( mpFormats->begin() ), aEnd( mpFormats->end() ); aIter != aEnd ; ++aIter)
    {
        if( TransferableDataHelper::IsEqual( *aIter, rFlavor ) )
        {
            bRet = true;
            break;
        }
    }

    return bRet;
}



void SAL_CALL TransferableHelper::lostOwnership( const Reference< XClipboard >&, const Reference< XTransferable >& ) throw( RuntimeException, std::exception )
{
    const SolarMutexGuard aGuard;

    try
    {
        if( mxTerminateListener.is() )
        {
            Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
            xDesktop->removeTerminateListener( mxTerminateListener );

            mxTerminateListener.clear();
        }

        ObjectReleased();
    }
    catch( const css::uno::Exception& )
    {
    }
}



void SAL_CALL TransferableHelper::disposing( const EventObject& ) throw( RuntimeException, std::exception )
{
}



void SAL_CALL TransferableHelper::dragDropEnd( const DragSourceDropEvent& rDSDE ) throw( RuntimeException, std::exception )
{
    const SolarMutexGuard aGuard;

    try
    {
        DragFinished( rDSDE.DropSuccess ? ( rDSDE.DropAction & ~DNDConstants::ACTION_DEFAULT ) : DNDConstants::ACTION_NONE );
        ObjectReleased();
    }
    catch( const css::uno::Exception& )
    {
    }
}



void SAL_CALL TransferableHelper::dragEnter( const DragSourceDragEvent& ) throw( RuntimeException, std::exception )
{
}



void SAL_CALL TransferableHelper::dragExit( const DragSourceEvent& ) throw( RuntimeException, std::exception )
{
}



void SAL_CALL TransferableHelper::dragOver( const DragSourceDragEvent& ) throw( RuntimeException, std::exception )
{
}



void SAL_CALL TransferableHelper::dropActionChanged( const DragSourceDragEvent& ) throw( RuntimeException, std::exception )
{
}



sal_Int64 SAL_CALL TransferableHelper::getSomething( const Sequence< sal_Int8 >& rId ) throw( RuntimeException, std::exception )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) &&
        ( 0 == memcmp( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    else
        nRet = 0;

    return nRet;
}



void TransferableHelper::ImplFlush()
{
    if( mxClipboard.is() )
    {
        Reference< XFlushableClipboard >    xFlushableClipboard( mxClipboard, UNO_QUERY );
        SolarMutexReleaser aReleaser;

        try
        {
            if( xFlushableClipboard.is() )
                 xFlushableClipboard->flushClipboard();
        }
        catch( const css::uno::Exception& )
        {
            OSL_FAIL( "Could not flush clipboard" );
        }
    }
}



void TransferableHelper::AddFormat( SotClipboardFormatId nFormat )
{
    DataFlavor aFlavor;

    if( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) )
        AddFormat( aFlavor );
}



void TransferableHelper::AddFormat( const DataFlavor& rFlavor )
{
    bool bAdd = true;

    for (DataFlavorExVector::iterator aIter( mpFormats->begin() ), aEnd( mpFormats->end() ); aIter != aEnd ; ++aIter)
    {
        if( TransferableDataHelper::IsEqual( *aIter, rFlavor ) )
        {
            // update MimeType for SotClipboardFormatId::OBJECTDESCRIPTOR in every case
            if( ( SotClipboardFormatId::OBJECTDESCRIPTOR == aIter->mnSotId ) && mpObjDesc )
            {
                DataFlavor aObjDescFlavor;

                SotExchange::GetFormatDataFlavor( SotClipboardFormatId::OBJECTDESCRIPTOR, aObjDescFlavor );
                aIter->MimeType = aObjDescFlavor.MimeType;
                aIter->MimeType += ::ImplGetParameterString( *mpObjDesc );

#ifdef DEBUG
                fprintf( stderr, "TransferableHelper exchanged objectdescriptor [ %s ]\n",
                         OUStringToOString(aIter->MimeType, RTL_TEXTENCODING_ASCII_US).getStr() );
#endif
            }

            bAdd = false;
            break;
        }
    }

    if( bAdd )
    {
        DataFlavorEx   aFlavorEx;

        aFlavorEx.MimeType = rFlavor.MimeType;
        aFlavorEx.HumanPresentableName = rFlavor.HumanPresentableName;
        aFlavorEx.DataType = rFlavor.DataType;
        aFlavorEx.mnSotId = SotExchange::RegisterFormat( rFlavor );

        if( ( SotClipboardFormatId::OBJECTDESCRIPTOR == aFlavorEx.mnSotId ) && mpObjDesc )
            aFlavorEx.MimeType += ::ImplGetParameterString( *mpObjDesc );

        mpFormats->push_back( aFlavorEx );

        if( SotClipboardFormatId::BITMAP == aFlavorEx.mnSotId )
        {
            AddFormat( SotClipboardFormatId::PNG );
            AddFormat( SotClipboardFormatId::BMP );
        }
        else if( SotClipboardFormatId::GDIMETAFILE == aFlavorEx.mnSotId )
        {
            AddFormat( SotClipboardFormatId::EMF );
            AddFormat( SotClipboardFormatId::WMF );
        }
    }
}



void TransferableHelper::RemoveFormat( SotClipboardFormatId nFormat )
{
    DataFlavor aFlavor;

    if( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) )
        RemoveFormat( aFlavor );
}



void TransferableHelper::RemoveFormat( const DataFlavor& rFlavor )
{
    DataFlavorExVector::iterator aIter( mpFormats->begin() );

    while (aIter != mpFormats->end())
    {
        if( TransferableDataHelper::IsEqual( *aIter, rFlavor ) )
            aIter = mpFormats->erase( aIter );
        else
            ++aIter;
    }
}



bool TransferableHelper::HasFormat( SotClipboardFormatId nFormat )
{
    bool bRet = false;

    for (DataFlavorExVector::const_iterator aIter( mpFormats->begin() ), aEnd( mpFormats->end() ); aIter != aEnd ; ++aIter)
    {
        if( nFormat == (*aIter).mnSotId )
        {
            bRet = true;
            break;
        }
    }

    return bRet;
}



void TransferableHelper::ClearFormats()
{
    mpFormats->clear();
    maAny.clear();
}



bool TransferableHelper::SetAny( const Any& rAny, const DataFlavor& )
{
    maAny = rAny;
    return( maAny.hasValue() );
}



bool TransferableHelper::SetString( const OUString& rString, const DataFlavor& rFlavor )
{
    DataFlavor aFileFlavor;

    if( !rString.isEmpty() &&
        SotExchange::GetFormatDataFlavor( SotClipboardFormatId::SIMPLE_FILE, aFileFlavor ) &&
        TransferableDataHelper::IsEqual( aFileFlavor, rFlavor ) )
    {
        const OString aByteStr(OUStringToOString(rString, osl_getThreadTextEncoding()));
        Sequence< sal_Int8 >    aSeq( aByteStr.getLength() + 1 );

        memcpy( aSeq.getArray(), aByteStr.getStr(), aByteStr.getLength() );
        aSeq[ aByteStr.getLength() ] = 0;
        maAny <<= aSeq;
    }
    else
        maAny <<= rString;

    return( maAny.hasValue() );
}



bool TransferableHelper::SetBitmapEx( const BitmapEx& rBitmapEx, const DataFlavor& rFlavor )
{
    if( !rBitmapEx.IsEmpty() )
    {
        SvMemoryStream aMemStm( 65535, 65535 );

        if(rFlavor.MimeType.equalsIgnoreAsciiCase("image/png"))
        {
            // write a PNG
            vcl::PNGWriter aPNGWriter(rBitmapEx);

            aPNGWriter.Write(aMemStm);
        }
        else
        {
            const Bitmap aBitmap(rBitmapEx.GetBitmap());

            // explicitely use Bitmap::Write with bCompressed = sal_False and bFileHeader = sal_True
            WriteDIB(aBitmap, aMemStm, false, true);
        }

        maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );
    }

    return( maAny.hasValue() );
}



bool TransferableHelper::SetGDIMetaFile( const GDIMetaFile& rMtf, const DataFlavor& )
{
    if( rMtf.GetActionSize() )
    {
        SvMemoryStream aMemStm( 65535, 65535 );

        ( (GDIMetaFile&) rMtf ).Write( aMemStm );
        maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );
    }

    return( maAny.hasValue() );
}



bool TransferableHelper::SetGraphic( const Graphic& rGraphic, const DataFlavor& )
{
    if( rGraphic.GetType() != GRAPHIC_NONE )
    {
        SvMemoryStream aMemStm( 65535, 65535 );

        aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
        aMemStm.SetCompressMode( SvStreamCompressFlags::NATIVE );
        WriteGraphic( aMemStm, rGraphic );
        maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );
    }

    return( maAny.hasValue() );
}



bool TransferableHelper::SetImageMap( const ImageMap& rIMap, const css::datatransfer::DataFlavor& )
{
    SvMemoryStream aMemStm( 8192, 8192 );

    aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
    rIMap.Write( aMemStm, OUString() );
    maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );

    return( maAny.hasValue() );
}



bool TransferableHelper::SetTransferableObjectDescriptor( const TransferableObjectDescriptor& rDesc,
                                                          const css::datatransfer::DataFlavor& )
{
    PrepareOLE( rDesc );

    SvMemoryStream aMemStm( 1024, 1024 );

    WriteTransferableObjectDescriptor( aMemStm, rDesc );
    maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Tell() );

    return( maAny.hasValue() );
 }



bool TransferableHelper::SetINetBookmark( const INetBookmark& rBmk,
                                          const css::datatransfer::DataFlavor& rFlavor )
{
    rtl_TextEncoding eSysCSet = osl_getThreadTextEncoding();

    switch( SotExchange::GetFormat( rFlavor ) )
    {
        case SotClipboardFormatId::SOLK:
        {
            OString sURL(OUStringToOString(rBmk.GetURL(), eSysCSet));
            OString sDesc(OUStringToOString(rBmk.GetDescription(), eSysCSet));
            OStringBuffer sOut;
            sOut.append(sURL.getLength());
            sOut.append('@').append(sURL);
            sOut.append(sDesc.getLength());
            sOut.append('@').append(sDesc);

            Sequence< sal_Int8 > aSeq(sOut.getLength());
            memcpy(aSeq.getArray(), sOut.getStr(), sOut.getLength());
            maAny <<= aSeq;
        }
        break;

        case SotClipboardFormatId::STRING:
            maAny <<= OUString( rBmk.GetURL() );
            break;

        case SotClipboardFormatId::UNIFORMRESOURCELOCATOR:
        {
            OString sURL(OUStringToOString(rBmk.GetURL(), eSysCSet));
            Sequence< sal_Int8 > aSeq( sURL.getLength() );
            memcpy( aSeq.getArray(), sURL.getStr(), sURL.getLength() );
            maAny <<= aSeq;
        }
        break;

        case SotClipboardFormatId::NETSCAPE_BOOKMARK:
        {
            Sequence< sal_Int8 > aSeq( 2048 );

            memset( aSeq.getArray(), 0, 2048 );
            strcpy( reinterpret_cast< char* >( aSeq.getArray() ), OUStringToOString(rBmk.GetURL(), eSysCSet).getStr() );
            strcpy( reinterpret_cast< char* >( aSeq.getArray() ) + 1024, OUStringToOString(rBmk.GetDescription(), eSysCSet).getStr() );

            maAny <<= aSeq;
        }
        break;

#ifdef WNT
        case SotClipboardFormatId::FILEGRPDESCRIPTOR:
        {
            Sequence< sal_Int8 >    aSeq( sizeof( FILEGROUPDESCRIPTOR ) );
            FILEGROUPDESCRIPTOR*    pFDesc = (FILEGROUPDESCRIPTOR*) aSeq.getArray();
            FILEDESCRIPTOR&         rFDesc1 = pFDesc->fgd[ 0 ];

            pFDesc->cItems = 1;
            memset( &rFDesc1, 0, sizeof( FILEDESCRIPTOR ) );
            rFDesc1.dwFlags = FD_LINKUI;

            OStringBuffer aStr(OUStringToOString(
                rBmk.GetDescription(), eSysCSet));
            for( sal_uInt16 nChar = 0; nChar < aStr.getLength(); ++nChar )
                if( strchr( "\\/:*?\"<>|", aStr[nChar] ) )
                    aStr.remove(nChar--, 1);

            aStr.insert(0, "Shortcut to ");
            aStr.append(".URL");
            strcpy( rFDesc1.cFileName, aStr.getStr() );

            maAny <<= aSeq;
        }
        break;

        case SotClipboardFormatId::FILECONTENT:
        {
            OUString aStr( "[InternetShortcut]\x0aURL=" );
            maAny <<= ( aStr += rBmk.GetURL() );
        }
        break;
#endif

        default:
        break;
    }

    return( maAny.hasValue() );
}



bool TransferableHelper::SetINetImage( const INetImage& rINtImg,
                                       const css::datatransfer::DataFlavor& rFlavor )
{
    SvMemoryStream aMemStm( 1024, 1024 );

    aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
    rINtImg.Write( aMemStm, SotExchange::GetFormat( rFlavor ) );

    maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );

    return( maAny.hasValue() );
}



bool TransferableHelper::SetObject( void* pUserObject, SotClipboardFormatId nUserObjectId, const DataFlavor& rFlavor )
{
    tools::SvRef<SotStorageStream> xStm( new SotStorageStream( OUString() ) );

    xStm->SetVersion( SOFFICE_FILEFORMAT_50 );

    if( pUserObject && WriteObject( xStm, pUserObject, nUserObjectId, rFlavor ) )
    {
        const sal_uInt32        nLen = xStm->Seek( STREAM_SEEK_TO_END );
        Sequence< sal_Int8 >    aSeq( nLen );

        xStm->Seek( STREAM_SEEK_TO_BEGIN );
        xStm->Read( aSeq.getArray(),  nLen );

        if( nLen && ( SotExchange::GetFormat( rFlavor ) == SotClipboardFormatId::STRING ) )
        {
            //JP 24.7.2001: as I know was this only for the writer application and this
            //              writes now UTF16 format into the stream
            //JP 6.8.2001:  and now it writes UTF8 because then exist no problem with
            //              little / big endians! - Bug 88121
            maAny <<= OUString( reinterpret_cast< const sal_Char* >( aSeq.getConstArray() ), nLen - 1, RTL_TEXTENCODING_UTF8 );
        }
        else
            maAny <<= aSeq;
    }

    return( maAny.hasValue() );
}



bool TransferableHelper::WriteObject( tools::SvRef<SotStorageStream>&, void*, SotClipboardFormatId, const DataFlavor& )
{
    OSL_FAIL( "TransferableHelper::WriteObject( ... ) not implemented" );
    return false;
}



void TransferableHelper::DragFinished( sal_Int8 )
{
}



void TransferableHelper::ObjectReleased()
{
}



void TransferableHelper::PrepareOLE( const TransferableObjectDescriptor& rObjDesc )
{
    delete mpObjDesc;
    mpObjDesc = new TransferableObjectDescriptor( rObjDesc );

    if( HasFormat( SotClipboardFormatId::OBJECTDESCRIPTOR ) )
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
}



void TransferableHelper::CopyToClipboard( vcl::Window *pWindow ) const
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );
    Reference< XClipboard > xClipboard;

    if( pWindow )
        xClipboard = pWindow->GetClipboard();

    if( xClipboard.is() )
        mxClipboard = xClipboard;

    if( mxClipboard.is() && !mxTerminateListener.is() )
    {
        SolarMutexReleaser aReleaser;

        try
        {
            TransferableHelper*                 pThis = const_cast< TransferableHelper* >( this );
            Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
            xDesktop->addTerminateListener( pThis->mxTerminateListener = new TerminateListener( *pThis ) );

            mxClipboard->setContents( pThis, pThis );
        }
        catch( const css::uno::Exception& )
        {
        }
    }
}



void TransferableHelper::CopyToSelection( vcl::Window *pWindow ) const
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );
    Reference< XClipboard > xSelection;

    if( pWindow )
        xSelection = pWindow->GetPrimarySelection();

    if( xSelection.is() && !mxTerminateListener.is() )
    {
        SolarMutexReleaser aReleaser;

        try
        {
            TransferableHelper*                 pThis = const_cast< TransferableHelper* >( this );
            Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
            xDesktop->addTerminateListener( pThis->mxTerminateListener = new TerminateListener( *pThis ) );

            xSelection->setContents( pThis, pThis );
        }
        catch( const css::uno::Exception& )
        {
        }
    }
}



void TransferableHelper::StartDrag( vcl::Window* pWindow, sal_Int8 nDnDSourceActions,
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

        // On Mac OS X we are forced to execute 'startDrag' synchronously
        // contrary to the XDragSource interface specification because
        // we can receive drag events from the system only in the main
        // thread
#if !defined(MACOSX)
        SolarMutexReleaser aReleaser;
#endif

        try
        {
            DragGestureEvent    aEvt;
            aEvt.DragAction = DNDConstants::ACTION_COPY;
            aEvt.DragOriginX = aPt.X();
            aEvt.DragOriginY = aPt.Y();
            aEvt.DragSource = xDragSource;

            xDragSource->startDrag( aEvt, nDnDSourceActions, nDnDPointer, nDnDImage, this, this );
        }
        catch( const css::uno::Exception& )
        {
        }
    }
}



void TransferableHelper::ClearSelection( vcl::Window *pWindow )
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );
    Reference< XClipboard > xSelection( pWindow->GetPrimarySelection() );

    if( xSelection.is() )
        xSelection->setContents( nullptr, nullptr );
}



Reference< XClipboard> TransferableHelper::GetSystemClipboard()
{
    vcl::Window *pFocusWindow = Application::GetFocusWindow();

    if( pFocusWindow )
        return pFocusWindow->GetClipboard();

    return  Reference< XClipboard > ();
}

namespace
{
    class theTransferableHelperUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theTransferableHelperUnoTunnelId > {};
}

const Sequence< sal_Int8 >& TransferableHelper::getUnoTunnelId()
{
    return theTransferableHelperUnoTunnelId::get().getSeq();
}


// - TransferableClipboardNotifier -


class TransferableClipboardNotifier : public ::cppu::WeakImplHelper< XClipboardListener >
{
private:
    ::osl::Mutex&                   mrMutex;
    Reference< XClipboardNotifier > mxNotifier;
    TransferableDataHelper*         mpListener;

protected:
    // XClipboardListener
    virtual void SAL_CALL changedContents( const clipboard::ClipboardEvent& event ) throw (RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException, std::exception) override;

public:
    TransferableClipboardNotifier( const Reference< XClipboard >& _rxClipboard, TransferableDataHelper& _rListener, ::osl::Mutex& _rMutex );

    /// determines whether we're currently listening
    inline bool isListening() const { return !isDisposed(); }

    /// determines whether the instance is disposed
    inline bool isDisposed() const { return mpListener == nullptr; }

    /// makes the instance non-functional
    void    dispose();
};



TransferableClipboardNotifier::TransferableClipboardNotifier( const Reference< XClipboard >& _rxClipboard, TransferableDataHelper& _rListener, ::osl::Mutex& _rMutex )
    :mrMutex( _rMutex )
    ,mxNotifier( _rxClipboard, UNO_QUERY )
    ,mpListener( &_rListener )
{
    osl_atomic_increment( &m_refCount );
    {
        if ( mxNotifier.is() )
            mxNotifier->addClipboardListener( this );
        else
            // born dead
            mpListener = nullptr;
    }
    osl_atomic_decrement( &m_refCount );
}



void SAL_CALL TransferableClipboardNotifier::changedContents( const clipboard::ClipboardEvent& event ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
        // the SolarMutex here is necessary, since
        // - we cannot call mpListener without our own mutex locked
        // - Rebind respectively InitFormats (called by Rebind) will
        // try to lock the SolarMutex, too
    ::osl::MutexGuard aGuard( mrMutex );
    if( mpListener )
        mpListener->Rebind( event.Contents );
}



void SAL_CALL TransferableClipboardNotifier::disposing( const EventObject& ) throw (RuntimeException, std::exception)
{
    // clipboard is being disposed. Hmm. Okay, become disfunctional myself.
    dispose();
}



void TransferableClipboardNotifier::dispose()
{
    ::osl::MutexGuard aGuard( mrMutex );

    Reference< XClipboardListener > xKeepMeAlive( this );

    if ( mxNotifier.is() )
        mxNotifier->removeClipboardListener( this );
    mxNotifier.clear();

    mpListener = nullptr;
}


// - TransferableDataHelper_Impl -


struct TransferableDataHelper_Impl
{
    ::osl::Mutex                    maMutex;
    TransferableClipboardNotifier*  mpClipboardListener;

    TransferableDataHelper_Impl()
        :mpClipboardListener( nullptr )
    {
    }
};


// - TransferableDataHelper -


TransferableDataHelper::TransferableDataHelper() :
    mpFormats( new DataFlavorExVector ),
    mpObjDesc( new TransferableObjectDescriptor ),
    mpImpl( new TransferableDataHelper_Impl )
{
}



TransferableDataHelper::TransferableDataHelper( const Reference< css::datatransfer::XTransferable >& rxTransferable ) :
    mxTransfer( rxTransferable ),
    mpFormats( new DataFlavorExVector ),
    mpObjDesc( new TransferableObjectDescriptor ),
    mpImpl( new TransferableDataHelper_Impl )
{
    InitFormats();
}



TransferableDataHelper::TransferableDataHelper( const TransferableDataHelper& rDataHelper ) :
    mxTransfer( rDataHelper.mxTransfer ),
    mxClipboard( rDataHelper.mxClipboard ),
    mpFormats( new DataFlavorExVector( *rDataHelper.mpFormats ) ),
    mpObjDesc( new TransferableObjectDescriptor( *rDataHelper.mpObjDesc ) ),
    mpImpl( new TransferableDataHelper_Impl )
{
}



TransferableDataHelper& TransferableDataHelper::operator=( const TransferableDataHelper& rDataHelper )
{
    if ( this != &rDataHelper )
    {
        ::osl::MutexGuard aGuard( mpImpl->maMutex );

        bool bWasClipboardListening = ( nullptr != mpImpl->mpClipboardListener );

        if ( bWasClipboardListening )
            StopClipboardListening();

        mxTransfer = rDataHelper.mxTransfer;
        delete mpFormats, mpFormats = new DataFlavorExVector( *rDataHelper.mpFormats );
        delete mpObjDesc, mpObjDesc = new TransferableObjectDescriptor( *rDataHelper.mpObjDesc );
        mxClipboard = rDataHelper.mxClipboard;

        if ( bWasClipboardListening )
            StartClipboardListening();
    }

    return *this;
}



TransferableDataHelper::~TransferableDataHelper()
{
    StopClipboardListening( );
    {
        ::osl::MutexGuard aGuard( mpImpl->maMutex );
        delete mpFormats, mpFormats = nullptr;
        delete mpObjDesc, mpObjDesc = nullptr;
    }
}



void TransferableDataHelper::FillDataFlavorExVector( const Sequence< DataFlavor >& rDataFlavorSeq,
                                                     DataFlavorExVector& rDataFlavorExVector )
{
    try
    {
        Reference< XComponentContext >          xContext( ::comphelper::getProcessComponentContext() );
        Reference< XMimeContentTypeFactory >    xMimeFact = MimeContentTypeFactory::create( xContext );
        DataFlavorEx                            aFlavorEx;
        const OUString                   aCharsetStr( "charset" );


        for( sal_Int32 i = 0; i < rDataFlavorSeq.getLength(); i++ )
        {
            const DataFlavor&               rFlavor = rDataFlavorSeq[ i ];
            Reference< XMimeContentType >   xMimeType;

            try
            {
                if( !rFlavor.MimeType.isEmpty() )
                    xMimeType = xMimeFact->createMimeContentType( rFlavor.MimeType );
            }
            catch( const css::uno::Exception& )
            {
            }

            aFlavorEx.MimeType = rFlavor.MimeType;
            aFlavorEx.HumanPresentableName = rFlavor.HumanPresentableName;
            aFlavorEx.DataType = rFlavor.DataType;
            aFlavorEx.mnSotId = SotExchange::RegisterFormat( rFlavor );

            rDataFlavorExVector.push_back( aFlavorEx );

            // add additional formats for special mime types
            if(SotClipboardFormatId::BMP == aFlavorEx.mnSotId || SotClipboardFormatId::PNG == aFlavorEx.mnSotId)
            {
                if( SotExchange::GetFormatDataFlavor( SotClipboardFormatId::BITMAP, aFlavorEx ) )
                {
                    aFlavorEx.mnSotId = SotClipboardFormatId::BITMAP;
                    rDataFlavorExVector.push_back( aFlavorEx );
                }
            }
            else if( SotClipboardFormatId::WMF == aFlavorEx.mnSotId || SotClipboardFormatId::EMF == aFlavorEx.mnSotId )
            {
                if( SotExchange::GetFormatDataFlavor( SotClipboardFormatId::GDIMETAFILE, aFlavorEx ) )
                {
                    aFlavorEx.mnSotId = SotClipboardFormatId::GDIMETAFILE;
                    rDataFlavorExVector.push_back( aFlavorEx );
                }
            }
            else if ( SotClipboardFormatId::HTML_SIMPLE == aFlavorEx.mnSotId  )
            {
                // #104735# HTML_SIMPLE may also be inserted without comments
                aFlavorEx.mnSotId = SotClipboardFormatId::HTML_NO_COMMENT;
                rDataFlavorExVector.push_back( aFlavorEx );
            }
            else if( xMimeType.is() && xMimeType->getFullMediaType().equalsIgnoreAsciiCase( "text/plain" ) )
            {
                // add, if it is a UTF-8 byte buffer
                if( xMimeType->hasParameter( aCharsetStr ) )
                {
                    if( xMimeType->getParameterValue( aCharsetStr ).equalsIgnoreAsciiCase( "unicode" ) ||
                        xMimeType->getParameterValue( aCharsetStr ).equalsIgnoreAsciiCase( "utf-16" ) )
                    {
                        rDataFlavorExVector[ rDataFlavorExVector.size() - 1 ].mnSotId = SotClipboardFormatId::STRING;

                    }
                }
            }
            else if( xMimeType.is() && xMimeType->getFullMediaType().equalsIgnoreAsciiCase( "text/rtf" ) )
            {
                rDataFlavorExVector[ rDataFlavorExVector.size() - 1 ].mnSotId = SotClipboardFormatId::RTF;
            }
            else if( xMimeType.is() && xMimeType->getFullMediaType().equalsIgnoreAsciiCase( "text/html" ) )

            {
                rDataFlavorExVector[ rDataFlavorExVector.size() - 1 ].mnSotId = SotClipboardFormatId::HTML;
            }
            else if( xMimeType.is() && xMimeType->getFullMediaType().equalsIgnoreAsciiCase( "text/uri-list" ) )
            {
                rDataFlavorExVector[ rDataFlavorExVector.size() - 1 ].mnSotId = SotClipboardFormatId::FILE_LIST;
            }
            else if( xMimeType.is() && xMimeType->getFullMediaType().equalsIgnoreAsciiCase( "application/x-openoffice-objectdescriptor-xml" ) )
            {
                rDataFlavorExVector[ rDataFlavorExVector.size() - 1 ].mnSotId = SotClipboardFormatId::OBJECTDESCRIPTOR;
            }
        }
    }
    catch( const css::uno::Exception& )
    {
    }
}



void TransferableDataHelper::InitFormats()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( mpImpl->maMutex );

    mpFormats->clear();
    delete mpObjDesc, mpObjDesc = new TransferableObjectDescriptor;

    if( mxTransfer.is() )
    {
        TransferableDataHelper::FillDataFlavorExVector( mxTransfer->getTransferDataFlavors(), *mpFormats );

        for (DataFlavorExVector::const_iterator aIter( mpFormats->begin() ), aEnd( mpFormats->end() ); aIter != aEnd ; ++aIter)
        {
            if( SotClipboardFormatId::OBJECTDESCRIPTOR == aIter->mnSotId )
            {
                ImplSetParameterString( *mpObjDesc, *aIter );
                break;
            }
        }
    }
}



bool TransferableDataHelper::HasFormat( SotClipboardFormatId nFormat ) const
{
    ::osl::MutexGuard aGuard( mpImpl->maMutex );

    DataFlavorExVector::iterator    aIter( mpFormats->begin() ), aEnd( mpFormats->end() );
    bool                            bRet = false;

    while( aIter != aEnd )
    {
        if( nFormat == (*aIter++).mnSotId )
        {
            aIter = aEnd;
            bRet = true;
        }
    }

    return bRet;
}



bool TransferableDataHelper::HasFormat( const DataFlavor& rFlavor ) const
{
    ::osl::MutexGuard aGuard( mpImpl->maMutex );

    DataFlavorExVector::iterator    aIter( mpFormats->begin() ), aEnd( mpFormats->end() );
    bool                            bRet = false;

    while( aIter != aEnd )
    {
        if( TransferableDataHelper::IsEqual( rFlavor, *aIter++ ) )
        {
            aIter = aEnd;
            bRet = true;
        }
    }

    return bRet;
}



sal_uInt32 TransferableDataHelper::GetFormatCount() const
{
    ::osl::MutexGuard aGuard( mpImpl->maMutex );
    return mpFormats->size();
}




SotClipboardFormatId TransferableDataHelper::GetFormat( sal_uInt32 nFormat ) const
{
    ::osl::MutexGuard aGuard( mpImpl->maMutex );
    DBG_ASSERT( nFormat < mpFormats->size(), "TransferableDataHelper::GetFormat: invalid format index" );
    return( ( nFormat < mpFormats->size() ) ? (*mpFormats)[ nFormat ].mnSotId : SotClipboardFormatId::NONE );
}



DataFlavor TransferableDataHelper::GetFormatDataFlavor( sal_uInt32 nFormat ) const
{
    ::osl::MutexGuard aGuard( mpImpl->maMutex );
    DBG_ASSERT( nFormat < mpFormats->size(), "TransferableDataHelper::GetFormat: invalid format index" );

    DataFlavor aRet;

    if( nFormat < mpFormats->size() )
        aRet = (*mpFormats)[ nFormat ];

    return aRet;
}



Reference< XTransferable > TransferableDataHelper::GetXTransferable() const
{
    Reference< XTransferable > xRet;

    if( mxTransfer.is() )
    {
        try
        {
            xRet = mxTransfer;

            // do a dummy call to check, if this interface is valid (nasty)
            xRet->getTransferDataFlavors();

        }
        catch( const css::uno::Exception& )
        {
            xRet.clear();
        }
    }

    return xRet;
}



Any TransferableDataHelper::GetAny( SotClipboardFormatId nFormat, const OUString& rDestDoc ) const
{
    Any aReturn;

    DataFlavor aFlavor;
    if ( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) )
        aReturn = GetAny(aFlavor, rDestDoc);

    return aReturn;
}




Any TransferableDataHelper::GetAny( const DataFlavor& rFlavor, const OUString& rDestDoc ) const
{
    ::osl::MutexGuard aGuard( mpImpl->maMutex );
    Any aRet;

    try
    {
        if( mxTransfer.is() )
        {
            const SotClipboardFormatId         nRequestFormat = SotExchange::GetFormat( rFlavor );

            Reference<css::datatransfer::XTransferable2> xTransfer2(mxTransfer, UNO_QUERY);

            if( nRequestFormat != SotClipboardFormatId::NONE )
            {
                // try to get alien format first
                for (DataFlavorExVector::const_iterator aIter( mpFormats->begin() ), aEnd( mpFormats->end() ); aIter != aEnd ; ++aIter)
                {
                    if( ( nRequestFormat == (*aIter).mnSotId ) && !rFlavor.MimeType.equalsIgnoreAsciiCase( (*aIter).MimeType ) )
                    {
                        if (xTransfer2.is())
                            aRet = xTransfer2->getTransferData2(*aIter, rDestDoc);
                        else
                            aRet = mxTransfer->getTransferData(*aIter);
                    }

                    if( aRet.hasValue() )
                        break;
                }
            }

            if( !aRet.hasValue() )
            {
                if (xTransfer2.is())
                    aRet = xTransfer2->getTransferData2(rFlavor, rDestDoc);
                else
                    aRet = mxTransfer->getTransferData(rFlavor);
            }
        }
    }
    catch( const css::uno::Exception& )
    {
    }

    return aRet;
}



bool TransferableDataHelper::GetString( SotClipboardFormatId nFormat, OUString& rStr )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetString( aFlavor, rStr ) );
}



bool TransferableDataHelper::GetString( const DataFlavor& rFlavor, OUString& rStr )
{
    Any aAny = GetAny(rFlavor, OUString());
    bool        bRet = false;

    if( aAny.hasValue() )
    {
        OUString         aOUString;
        Sequence< sal_Int8 >    aSeq;

        if( aAny >>= aOUString )
        {
            rStr = aOUString;
            bRet = true;
        }
        else if( aAny >>= aSeq )
        {

            const sal_Char* pChars = reinterpret_cast< const sal_Char* >( aSeq.getConstArray() );
            sal_Int32       nLen = aSeq.getLength();

            //JP 10.10.2001: 92930 - don't copy the last zero character into the string.
            //DVO 2002-05-27: strip _all_ trailing zeros
            while( nLen && ( 0 == *( pChars + nLen - 1 ) ) )
                --nLen;

            rStr = OUString( pChars, nLen, osl_getThreadTextEncoding() );
            bRet = true;
        }
    }

    return bRet;
}



bool TransferableDataHelper::GetBitmapEx( SotClipboardFormatId nFormat, BitmapEx& rBmpEx )
{
    if(SotClipboardFormatId::BITMAP == nFormat)
    {
        // try to get PNG first
        DataFlavor aFlavor;

        if(SotExchange::GetFormatDataFlavor(SotClipboardFormatId::PNG, aFlavor))
        {
            if(GetBitmapEx(aFlavor, rBmpEx))
            {
                return true;
            }
        }
    }

    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetBitmapEx( aFlavor, rBmpEx ) );
}



bool TransferableDataHelper::GetBitmapEx( const DataFlavor& rFlavor, BitmapEx& rBmpEx )
{
    tools::SvRef<SotStorageStream> xStm;
    DataFlavor aSubstFlavor;
    bool bRet(GetSotStorageStream(rFlavor, xStm));
    bool bSuppressPNG(false); // #122982# If PNG stream not accessed, but BMP one, suppress trying to load PNG

    if(!bRet && HasFormat(SotClipboardFormatId::PNG) && SotExchange::GetFormatDataFlavor(SotClipboardFormatId::PNG, aSubstFlavor))
    {
        // when no direct success, try if PNG is available
        bRet = GetSotStorageStream(aSubstFlavor, xStm);
    }

    if(!bRet && HasFormat(SotClipboardFormatId::BMP) && SotExchange::GetFormatDataFlavor(SotClipboardFormatId::BMP, aSubstFlavor))
    {
        // when no direct success, try if BMP is available
        bRet = GetSotStorageStream(aSubstFlavor, xStm);
        bSuppressPNG = bRet;
    }

    if(bRet)
    {
        if(!bSuppressPNG && rFlavor.MimeType.equalsIgnoreAsciiCase("image/png"))
        {
            // it's a PNG, import to BitmapEx
            vcl::PNGReader aPNGReader(*xStm);

            rBmpEx = aPNGReader.Read();
        }

        if(rBmpEx.IsEmpty())
        {
            Bitmap aBitmap;
            Bitmap aMask;

            // explicitely use Bitmap::Read with bFileHeader = sal_True
            // #i124085# keep DIBV5 for read from clipboard, but should not happen
            ReadDIBV5(aBitmap, aMask, *xStm);

            if(aMask.IsEmpty())
            {
                rBmpEx = aBitmap;
            }
            else
            {
                rBmpEx = BitmapEx(aBitmap, aMask);
            }
        }

        bRet = (ERRCODE_NONE == xStm->GetError() && !rBmpEx.IsEmpty());

        /* SJ: #110748# At the moment we are having problems with DDB inserted as DIB. The
           problem is, that some graphics are inserted much too big because the nXPelsPerMeter
           and nYPelsPerMeter of the bitmap fileheader isn't including the correct value.
           Due to this reason the following code assumes that bitmaps with a logical size
           greater than 50 cm aren't having the correct mapmode set.

           The following code should be removed if DDBs and DIBs are supported via clipboard
           properly.
        */
        if(bRet)
        {
            const MapMode aMapMode(rBmpEx.GetPrefMapMode());

            if(MAP_PIXEL != aMapMode.GetMapUnit())
            {
                const Size aSize(OutputDevice::LogicToLogic(rBmpEx.GetPrefSize(), aMapMode, MAP_100TH_MM));

                // #i122388# This wrongly corrects in the given case; changing from 5000 100th mm to
                // the described 50 cm (which is 50000 100th mm)
                if((aSize.Width() > 50000) || (aSize.Height() > 50000))
                {
                    rBmpEx.SetPrefMapMode(MAP_PIXEL);

                    // #i122388# also adapt size by applying the mew MapMode
                    const Size aNewSize(OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, MAP_PIXEL));
                    rBmpEx.SetPrefSize(aNewSize);
                }
            }
        }
    }

    return bRet;
}



bool TransferableDataHelper::GetGDIMetaFile(SotClipboardFormatId nFormat, GDIMetaFile& rMtf, size_t nMaxActions)
{
    DataFlavor aFlavor;
    return SotExchange::GetFormatDataFlavor(nFormat, aFlavor) &&
        GetGDIMetaFile(aFlavor, rMtf) &&
        (nMaxActions == 0 || rMtf.GetActionSize() < nMaxActions);
}



bool TransferableDataHelper::GetGDIMetaFile( const DataFlavor& rFlavor, GDIMetaFile& rMtf )
{
    tools::SvRef<SotStorageStream> xStm;
    DataFlavor          aSubstFlavor;
    bool                bRet = false;

    if( GetSotStorageStream( rFlavor, xStm ) )
    {
        ReadGDIMetaFile( *xStm, rMtf );
        bRet = ( xStm->GetError() == ERRCODE_NONE );
    }

    if( !bRet &&
        HasFormat( SotClipboardFormatId::EMF ) &&
        SotExchange::GetFormatDataFlavor( SotClipboardFormatId::EMF, aSubstFlavor ) &&
        GetSotStorageStream( aSubstFlavor, xStm ) )
    {
        Graphic aGraphic;

        if( GraphicConverter::Import( *xStm, aGraphic ) == ERRCODE_NONE )
        {
            rMtf = aGraphic.GetGDIMetaFile();
            bRet = true;
        }
    }

    if( !bRet &&
        HasFormat( SotClipboardFormatId::WMF ) &&
        SotExchange::GetFormatDataFlavor( SotClipboardFormatId::WMF, aSubstFlavor ) &&
        GetSotStorageStream( aSubstFlavor, xStm ) )
    {
        Graphic aGraphic;

        if( GraphicConverter::Import( *xStm, aGraphic ) == ERRCODE_NONE )
        {
            rMtf = aGraphic.GetGDIMetaFile();
            bRet = true;
        }
    }

    return bRet;
}



bool TransferableDataHelper::GetGraphic( SotClipboardFormatId nFormat, Graphic& rGraphic )
{
    if(SotClipboardFormatId::BITMAP == nFormat)
    {
        // try to get PNG first
        DataFlavor aFlavor;

        if(SotExchange::GetFormatDataFlavor(SotClipboardFormatId::PNG, aFlavor))
        {
            if(GetGraphic(aFlavor, rGraphic))
            {
                return true;
            }
        }
    }

    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetGraphic( aFlavor, rGraphic ) );
}



bool TransferableDataHelper::GetGraphic( const css::datatransfer::DataFlavor& rFlavor, Graphic& rGraphic )
{
    DataFlavor  aFlavor;
    bool        bRet = false;

    if(SotExchange::GetFormatDataFlavor(SotClipboardFormatId::PNG, aFlavor) &&
        TransferableDataHelper::IsEqual(aFlavor, rFlavor))
    {
        // try to get PNG first
        BitmapEx aBmpEx;

        if( ( bRet = GetBitmapEx( aFlavor, aBmpEx ) ) )
            rGraphic = aBmpEx;
    }
    else if(SotExchange::GetFormatDataFlavor( SotClipboardFormatId::BITMAP, aFlavor ) &&
        TransferableDataHelper::IsEqual( aFlavor, rFlavor ) )
    {
        BitmapEx aBmpEx;

        if( ( bRet = GetBitmapEx( aFlavor, aBmpEx ) ) )
            rGraphic = aBmpEx;
    }
    else if( SotExchange::GetFormatDataFlavor( SotClipboardFormatId::GDIMETAFILE, aFlavor ) &&
             TransferableDataHelper::IsEqual( aFlavor, rFlavor ) )
    {
        GDIMetaFile aMtf;

        if( ( bRet = GetGDIMetaFile( aFlavor, aMtf ) ) )
            rGraphic = aMtf;
    }
    else
    {
        tools::SvRef<SotStorageStream> xStm;

        if( GetSotStorageStream( rFlavor, xStm ) )
        {
            ReadGraphic( *xStm, rGraphic );
            bRet = ( xStm->GetError() == ERRCODE_NONE );
        }
    }

    return bRet;
}



bool TransferableDataHelper::GetImageMap( SotClipboardFormatId nFormat, ImageMap& rIMap )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetImageMap( aFlavor, rIMap ) );
}



bool TransferableDataHelper::GetImageMap( const css::datatransfer::DataFlavor& rFlavor, ImageMap& rIMap )
{
    tools::SvRef<SotStorageStream> xStm;
    bool                bRet = GetSotStorageStream( rFlavor, xStm );

    if( bRet )
    {
        rIMap.Read( *xStm, OUString() );
        bRet = ( xStm->GetError() == ERRCODE_NONE );
    }

    return bRet;
}



bool TransferableDataHelper::GetTransferableObjectDescriptor( SotClipboardFormatId nFormat, TransferableObjectDescriptor& rDesc )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetTransferableObjectDescriptor( aFlavor, rDesc ) );
}



bool TransferableDataHelper::GetTransferableObjectDescriptor( const css::datatransfer::DataFlavor&, TransferableObjectDescriptor& rDesc )
{
    rDesc = *mpObjDesc;
    return true;
}



bool TransferableDataHelper::GetINetBookmark( SotClipboardFormatId nFormat, INetBookmark& rBmk )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetINetBookmark( aFlavor, rBmk ) );
}



bool TransferableDataHelper::GetINetBookmark( const css::datatransfer::DataFlavor& rFlavor, INetBookmark& rBmk )
{
    bool bRet = false;
    if( HasFormat( rFlavor ))
    {
    const SotClipboardFormatId nFormat = SotExchange::GetFormat( rFlavor );
    switch( nFormat )
    {
        case( SotClipboardFormatId::SOLK ):
        case( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ):
        {
            OUString aString;
            if( GetString( rFlavor, aString ) )
            {
                if( SotClipboardFormatId::UNIFORMRESOURCELOCATOR == nFormat )
                {
                    rBmk = INetBookmark( aString, aString );
                    bRet = true;
                }
                else
                {
                    OUString    aURL, aDesc;
                    sal_Int32   nStart = aString.indexOf( '@' ), nLen = aString.toInt32();

                    if( !nLen && aString[ 0 ] != '0' )
                    {
                        SAL_INFO( "svtools", "SOLK: 1. len=0" );
                    }
                    if( nStart == -1 || nLen > aString.getLength() - nStart - 3 )
                    {
                        SAL_INFO( "svtools", "SOLK: 1. illegal start or wrong len" );
                    }
                    aURL = aString.copy( nStart + 1, nLen );

                    aString = aString.replaceAt( 0, nStart + 1 + nLen, "" );
                    nStart = aString.indexOf( '@' );
                    nLen = aString.toInt32();

                    if( !nLen && aString[ 0 ] != '0' )
                    {
                        SAL_INFO( "svtools", "SOLK: 2. len=0" );
                    }
                    if( nStart == -1 || nLen > aString.getLength() - nStart - 1 )
                    {
                        SAL_INFO( "svtools", "SOLK: 2. illegal start or wrong len" );
                    }
                    aDesc = aString.copy( nStart+1, nLen );

                    rBmk = INetBookmark( aURL, aDesc );
                    bRet = true;
                }
            }
        }
        break;

        case( SotClipboardFormatId::NETSCAPE_BOOKMARK ):
        {
            Sequence<sal_Int8> aSeq = GetSequence(rFlavor, OUString());

            if (2048 == aSeq.getLength())
            {
                const sal_Char* p1 = reinterpret_cast< const sal_Char* >( aSeq.getConstArray() );
                const sal_Char* p2 =  reinterpret_cast< const sal_Char* >( aSeq.getConstArray() ) + 1024;
                rBmk = INetBookmark( OUString( p1, strlen(p1), osl_getThreadTextEncoding() ),
                                     OUString( p2, strlen(p2), osl_getThreadTextEncoding() ) );
                bRet = true;
            }
        }
        break;

#ifdef WNT
        case SotClipboardFormatId::FILEGRPDESCRIPTOR:
        {
            Sequence<sal_Int8> aSeq = GetSequence(rFlavor, OUString());

            if (aSeq.getLength())
            {
                FILEGROUPDESCRIPTOR* pFDesc = (FILEGROUPDESCRIPTOR*) aSeq.getConstArray();

                if( pFDesc->cItems )
                {
                    OString aDesc( pFDesc->fgd[ 0 ].cFileName );
                    rtl_TextEncoding    eTextEncoding = osl_getThreadTextEncoding();

                    if( ( aDesc.getLength() > 4 ) && aDesc.copy(aDesc.getLength() - 4).equalsIgnoreAsciiCase(".URL") )
                    {
                        std::unique_ptr<SvStream> pStream(::utl::UcbStreamHelper::CreateStream( INetURLObject( OStringToOUString(aDesc, eTextEncoding) ).GetMainURL( INetURLObject::NO_DECODE ),
                                                                                  STREAM_STD_READ ));

                        if( !pStream || pStream->GetError() )
                        {
                            DataFlavor aFileContentFlavor;

                            aSeq.realloc( 0 );
                            pStream.reset();

                            if (SotExchange::GetFormatDataFlavor(SotClipboardFormatId::FILECONTENT, aFileContentFlavor))
                            {
                                aSeq = GetSequence(aFileContentFlavor, OUString());
                                if (aSeq.getLength())
                                    pStream.reset(new SvMemoryStream( (sal_Char*) aSeq.getConstArray(), aSeq.getLength(), STREAM_STD_READ ));
                            }
                        }

                        if( pStream )
                        {
                            OString aLine;
                            bool    bSttFnd = false;

                            while( pStream->ReadLine( aLine ) )
                            {
                                if (aLine.equalsIgnoreAsciiCase("[InternetShortcut]"))
                                    bSttFnd = true;
                                else if (bSttFnd && aLine.copy(0, 4).equalsIgnoreAsciiCase("URL="))
                                {
                                    rBmk = INetBookmark( OStringToOUString(aLine.copy(4), eTextEncoding),
                                                         OStringToOUString(aDesc.copy(0, aDesc.getLength() - 4), eTextEncoding) );
                                    bRet = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        break;
#endif
        default: break;
    }
    }
    return bRet;
}



bool TransferableDataHelper::GetINetImage( SotClipboardFormatId nFormat,
                                                INetImage& rINtImg )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetINetImage( aFlavor, rINtImg ) );
}



bool TransferableDataHelper::GetINetImage(
        const css::datatransfer::DataFlavor& rFlavor,
        INetImage& rINtImg )
{
    tools::SvRef<SotStorageStream> xStm;
    bool bRet = GetSotStorageStream( rFlavor, xStm );

    if( bRet )
        bRet = rINtImg.Read( *xStm, SotExchange::GetFormat( rFlavor ) );
    return bRet;
}



bool TransferableDataHelper::GetFileList( SotClipboardFormatId nFormat,
                                                FileList& rFileList )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetFileList( aFlavor, rFileList ) );
}



bool TransferableDataHelper::GetFileList(
            const css::datatransfer::DataFlavor&,
            FileList& rFileList )
{
    tools::SvRef<SotStorageStream> xStm;
    bool                bRet = false;

    for( sal_uInt32 i = 0, nFormatCount = GetFormatCount(); ( i < nFormatCount ) && !bRet; ++i )
    {
        if( SotClipboardFormatId::FILE_LIST == GetFormat( i ) )
        {
            const DataFlavor aFlavor( GetFormatDataFlavor( i ) );

            if( GetSotStorageStream( aFlavor, xStm ) )
            {
                if( aFlavor.MimeType.indexOf( "text/uri-list" ) > -1 )
                {
                    OString aDiskString;

                    while( xStm->ReadLine( aDiskString ) )
                        if( !aDiskString.isEmpty() && aDiskString[0] != '#' )
                            rFileList.AppendFile( OStringToOUString(aDiskString, RTL_TEXTENCODING_UTF8) );

                    bRet = true;
                 }
                 else
                    bRet = ( ( ReadFileList( *xStm, rFileList ) ).GetError() == ERRCODE_NONE );
            }
        }
    }

    return bRet;
}



Sequence<sal_Int8> TransferableDataHelper::GetSequence( SotClipboardFormatId nFormat, const OUString& rDestDoc )
{
    DataFlavor aFlavor;
    if (!SotExchange::GetFormatDataFlavor(nFormat, aFlavor))
        return Sequence<sal_Int8>();

    return GetSequence(aFlavor, rDestDoc);
}

Sequence<sal_Int8> TransferableDataHelper::GetSequence( const DataFlavor& rFlavor, const OUString& rDestDoc )
{
#ifdef DEBUG
    fprintf( stderr, "TransferableDataHelper requests sequence of data\n" );
#endif

    const Any aAny = GetAny(rFlavor, rDestDoc);
    Sequence<sal_Int8> aSeq;
    if (aAny.hasValue())
        aAny >>= aSeq;

    return aSeq;
}



bool TransferableDataHelper::GetSotStorageStream( SotClipboardFormatId nFormat, tools::SvRef<SotStorageStream>& rxStream )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetSotStorageStream( aFlavor, rxStream ) );
}



bool TransferableDataHelper::GetSotStorageStream( const DataFlavor& rFlavor, tools::SvRef<SotStorageStream>& rxStream )
{
    Sequence<sal_Int8> aSeq = GetSequence(rFlavor, OUString());

    if (aSeq.getLength())
    {
        rxStream = new SotStorageStream( "" );
        rxStream->Write( aSeq.getConstArray(), aSeq.getLength() );
        rxStream->Seek( 0 );
    }

    return aSeq.getLength();
}

Reference<XInputStream> TransferableDataHelper::GetInputStream( SotClipboardFormatId nFormat, const OUString& rDestDoc )
{
    DataFlavor aFlavor;
    if (!SotExchange::GetFormatDataFlavor(nFormat, aFlavor))
        return Reference<XInputStream>();

    return GetInputStream(aFlavor, rDestDoc);
}

Reference<XInputStream> TransferableDataHelper::GetInputStream( const DataFlavor& rFlavor, const OUString& rDestDoc )
{
    Sequence<sal_Int8> aSeq = GetSequence(rFlavor, rDestDoc);

    if (!aSeq.getLength())
        return Reference<XInputStream>();

    Reference<XInputStream> xStream(new comphelper::SequenceInputStream(aSeq));
    return xStream;
}

void TransferableDataHelper::Rebind( const Reference< XTransferable >& _rxNewContent )
{
    mxTransfer = _rxNewContent;
    InitFormats();
}



bool TransferableDataHelper::StartClipboardListening( )
{
    ::osl::MutexGuard aGuard( mpImpl->maMutex );

    StopClipboardListening( );

    mpImpl->mpClipboardListener = new TransferableClipboardNotifier( mxClipboard, *this, mpImpl->maMutex );
    mpImpl->mpClipboardListener->acquire();

    return mpImpl->mpClipboardListener->isListening();
}



void TransferableDataHelper::StopClipboardListening( )
{
    ::osl::MutexGuard aGuard( mpImpl->maMutex );

    if ( mpImpl->mpClipboardListener )
    {
        mpImpl->mpClipboardListener->dispose();
        mpImpl->mpClipboardListener->release();
        mpImpl->mpClipboardListener = nullptr;
    }
}



TransferableDataHelper TransferableDataHelper::CreateFromSystemClipboard( vcl::Window * pWindow )
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );

    Reference< XClipboard > xClipboard;
    TransferableDataHelper  aRet;

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
                // also copy the clipboard
                aRet.mxClipboard = xClipboard;
            }
        }
        catch( const css::uno::Exception& )
        {
        }
    }

    return aRet;
}




TransferableDataHelper TransferableDataHelper::CreateFromSelection( vcl::Window* pWindow )
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );

    Reference< XClipboard > xSelection;
       TransferableDataHelper   aRet;

    if( pWindow )
        xSelection = pWindow->GetPrimarySelection();

    if( xSelection.is() )
       {
           SolarMutexReleaser aReleaser;

           try
               {
                   Reference< XTransferable > xTransferable( xSelection->getContents() );

                   if( xTransferable.is() )
                       {
                           aRet = TransferableDataHelper( xTransferable );
                           aRet.mxClipboard = xSelection;
                       }
               }
           catch( const css::uno::Exception& )
               {
               }
       }

    return aRet;
}


bool TransferableDataHelper::IsEqual( const css::datatransfer::DataFlavor& rInternalFlavor,
                                      const css::datatransfer::DataFlavor& rRequestFlavor,
                                      bool )
{
    Reference< XComponentContext >          xContext( ::comphelper::getProcessComponentContext() );
    bool                                    bRet = false;

    try
    {
        Reference< XMimeContentTypeFactory >    xMimeFact = MimeContentTypeFactory::create( xContext );

        Reference< XMimeContentType > xRequestType1( xMimeFact->createMimeContentType( rInternalFlavor.MimeType ) );
        Reference< XMimeContentType > xRequestType2( xMimeFact->createMimeContentType( rRequestFlavor.MimeType ) );

        if( xRequestType1.is() && xRequestType2.is() )
        {
            if( xRequestType1->getFullMediaType().equalsIgnoreAsciiCase( xRequestType2->getFullMediaType() ) )
            {
                if( xRequestType1->getFullMediaType().equalsIgnoreAsciiCase( "text/plain" ) )
                {
                    // special handling for text/plain media types
                    const OUString aCharsetString( "charset" );

                    if( !xRequestType2->hasParameter( aCharsetString ) ||
                        xRequestType2->getParameterValue( aCharsetString ).equalsIgnoreAsciiCase( "utf-16" ) ||
                        xRequestType2->getParameterValue( aCharsetString ).equalsIgnoreAsciiCase( "unicode" ) )
                    {
                        bRet = true;
                    }
                }
                else if( xRequestType1->getFullMediaType().equalsIgnoreAsciiCase( "application/x-openoffice" ) )
                {
                    // special handling for application/x-openoffice media types
                    const OUString aFormatString( "windows_formatname" );

                    if( xRequestType1->hasParameter( aFormatString ) &&
                        xRequestType2->hasParameter( aFormatString ) &&
                        xRequestType1->getParameterValue( aFormatString ).equalsIgnoreAsciiCase( xRequestType2->getParameterValue( aFormatString ) ) )
                    {
                        bRet = true;
                    }
                }
                else
                    bRet = true;
            }
        }
    }
    catch( const css::uno::Exception& )
    {
        bRet = rInternalFlavor.MimeType.equalsIgnoreAsciiCase( rRequestFlavor.MimeType );
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
