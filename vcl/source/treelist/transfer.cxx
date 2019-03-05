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

#ifdef _WIN32
#include <prewin.h>
#include <postwin.h>
#include <shlobj.h>
#endif
#include <osl/mutex.hxx>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
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
#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/sequence.hxx>
#include <sot/filelist.hxx>
#include <cppuhelper/implbase.hxx>

#include <comphelper/seqstream.hxx>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/datatransfer/MimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/XTransferable2.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <svl/urlbmk.hxx>
#include <vcl/inetimg.hxx>
#include <vcl/wmf.hxx>
#include <vcl/imap.hxx>
#include <vcl/transfer.hxx>
#include <rtl/strbuf.hxx>
#include <cstdio>
#include <vcl/dibtools.hxx>
#include <vcl/pngread.hxx>
#include <vcl/pngwrite.hxx>
#include <vcl/graphicfilter.hxx>
#include <memory>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;


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
        for (sal_Bool & rb : pToAccept)
            rb = false;

        const char aQuotedParamChars[] =
            "()<>@,;:/[]?=!#$&'*+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ^_`abcdefghijklmnopqrstuvwxyz{|}~. ";

        for ( sal_Int32 nInd = 0; nInd < RTL_CONSTASCII_LENGTH(aQuotedParamChars); ++nInd )
        {
            sal_Unicode nChar = aQuotedParamChars[nInd];
            if ( nChar < 128 )
                pToAccept[nChar] = true;
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
                rObjDesc.maSize.setWidth( xMimeType->getParameterValue( aWidthString ).toInt32() );
            }

            if( xMimeType->hasParameter( aHeightString ) )
            {
                rObjDesc.maSize.setHeight( xMimeType->getParameterValue( aHeightString ).toInt32() );
            }

            if( xMimeType->hasParameter( aPosXString ) )
            {
                rObjDesc.maDragStartPos.setX( xMimeType->getParameterValue( aPosXString ).toInt32() );
            }

            if( xMimeType->hasParameter( aPosYString ) )
            {
                rObjDesc.maDragStartPos.setY( xMimeType->getParameterValue( aPosYString ).toInt32() );
            }
        }
    }
    catch( const css::uno::Exception& )
    {
    }
}


TransferableHelper::TerminateListener::TerminateListener( TransferableHelper& rTransferableHelper ) :
    mrParent( rTransferableHelper )
{
}


TransferableHelper::TerminateListener::~TerminateListener()
{
}


void SAL_CALL TransferableHelper::TerminateListener::disposing( const EventObject& )
{
}


void SAL_CALL TransferableHelper::TerminateListener::queryTermination( const EventObject& )
{
}


void SAL_CALL TransferableHelper::TerminateListener::notifyTermination( const EventObject& )
{
    mrParent.ImplFlush();
}

OUString SAL_CALL TransferableHelper::TerminateListener::getImplementationName()
{
    return OUString("com.sun.star.comp.svt.TransferableHelperTerminateListener");
}

sal_Bool SAL_CALL TransferableHelper::TerminateListener::supportsService(const OUString& /*rServiceName*/)
{
    return false;
}

css::uno::Sequence<OUString> TransferableHelper::TerminateListener::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>();
}


Any SAL_CALL TransferableHelper::getTransferData( const DataFlavor& rFlavor )
{
    return getTransferData2(rFlavor, OUString());
}

Any SAL_CALL TransferableHelper::getTransferData2( const DataFlavor& rFlavor, const OUString& rDestDoc )
{
    if( !maAny.hasValue() || maFormats.empty() || ( maLastFormat != rFlavor.MimeType ) )
    {
        const SolarMutexGuard aGuard;

        maLastFormat = rFlavor.MimeType;
        maAny = Any();

        try
        {
            DataFlavor  aSubstFlavor;
            bool        bDone = false;

            // add formats if not already done
            if (maFormats.empty())
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
                            maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aDstStm.GetData() ),
                                                            aDstStm.TellEnd() );
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
                            maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aDstStm.GetData() ),
                                                            aDstStm.TellEnd() );
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
        }
        catch( const css::uno::Exception& )
        {
        }

        if( !maAny.hasValue() )
            throw UnsupportedFlavorException();
    }

    return maAny;
}


Sequence< DataFlavor > SAL_CALL TransferableHelper::getTransferDataFlavors()
{
    const SolarMutexGuard aGuard;

    try
    {
        if(maFormats.empty())
            AddSupportedFormats();
    }
    catch( const css::uno::Exception& )
    {
    }

    return comphelper::containerToSequence<DataFlavor>(maFormats);
}


sal_Bool SAL_CALL TransferableHelper::isDataFlavorSupported( const DataFlavor& rFlavor )
{
    const SolarMutexGuard aGuard;

    try
    {
        if (maFormats.empty())
            AddSupportedFormats();
    }
    catch( const css::uno::Exception& )
    {
    }

    for (auto const& format : maFormats)
    {
        if( TransferableDataHelper::IsEqual( format, rFlavor ) )
        {
            return true;
        }
    }

    return false;
}


void SAL_CALL TransferableHelper::lostOwnership( const Reference< XClipboard >&, const Reference< XTransferable >& )
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


void SAL_CALL TransferableHelper::disposing( const EventObject& )
{
}


void SAL_CALL TransferableHelper::dragDropEnd( const DragSourceDropEvent& rDSDE )
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


void SAL_CALL TransferableHelper::dragEnter( const DragSourceDragEvent& )
{
}


void SAL_CALL TransferableHelper::dragExit( const DragSourceEvent& )
{
}


void SAL_CALL TransferableHelper::dragOver( const DragSourceDragEvent& )
{
}


void SAL_CALL TransferableHelper::dropActionChanged( const DragSourceDragEvent& )
{
}


sal_Int64 SAL_CALL TransferableHelper::getSomething( const Sequence< sal_Int8 >& rId )
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
    if( !mxClipboard.is() )
        return;

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


void TransferableHelper::AddFormat( SotClipboardFormatId nFormat )
{
    DataFlavor aFlavor;

    if( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) )
        AddFormat( aFlavor );
}


void TransferableHelper::AddFormat( const DataFlavor& rFlavor )
{
    bool bAdd = true;

    for (auto & format : maFormats)
    {
        if( TransferableDataHelper::IsEqual( format, rFlavor ) )
        {
            // update MimeType for SotClipboardFormatId::OBJECTDESCRIPTOR in every case
            if ((SotClipboardFormatId::OBJECTDESCRIPTOR == format.mnSotId) && mxObjDesc)
            {
                DataFlavor aObjDescFlavor;

                SotExchange::GetFormatDataFlavor( SotClipboardFormatId::OBJECTDESCRIPTOR, aObjDescFlavor );
                format.MimeType = aObjDescFlavor.MimeType;
                format.MimeType += ::ImplGetParameterString(*mxObjDesc);
            }

            bAdd = false;
            break;
        }
    }

    if( !bAdd )
        return;

    DataFlavorEx   aFlavorEx;

    aFlavorEx.MimeType = rFlavor.MimeType;
    aFlavorEx.HumanPresentableName = rFlavor.HumanPresentableName;
    aFlavorEx.DataType = rFlavor.DataType;
    aFlavorEx.mnSotId = SotExchange::RegisterFormat( rFlavor );

    if ((SotClipboardFormatId::OBJECTDESCRIPTOR == aFlavorEx.mnSotId) && mxObjDesc)
        aFlavorEx.MimeType += ::ImplGetParameterString(*mxObjDesc);

    maFormats.push_back(aFlavorEx);

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


void TransferableHelper::RemoveFormat( SotClipboardFormatId nFormat )
{
    DataFlavor aFlavor;

    if( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) )
        RemoveFormat( aFlavor );
}


void TransferableHelper::RemoveFormat( const DataFlavor& rFlavor )
{
    DataFlavorExVector::iterator aIter(maFormats.begin());

    while (aIter != maFormats.end())
    {
        if( TransferableDataHelper::IsEqual( *aIter, rFlavor ) )
            aIter = maFormats.erase(aIter);
        else
            ++aIter;
    }
}


bool TransferableHelper::HasFormat( SotClipboardFormatId nFormat )
{
    return std::any_of(maFormats.begin(), maFormats.end(),
              [&](const DataFlavorEx& data) { return data.mnSotId == nFormat; });
}


void TransferableHelper::ClearFormats()
{
    maFormats.clear();
    maAny.clear();
}


bool TransferableHelper::SetAny( const Any& rAny )
{
    maAny = rAny;
    return maAny.hasValue();
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

    return maAny.hasValue();
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

            // explicitly use Bitmap::Write with bCompressed = sal_False and bFileHeader = sal_True
            WriteDIB(aBitmap, aMemStm, false, true);
        }

        maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.TellEnd() );
    }

    return maAny.hasValue();
}


bool TransferableHelper::SetGDIMetaFile( const GDIMetaFile& rMtf )
{
    if( rMtf.GetActionSize() )
    {
        SvMemoryStream aMemStm( 65535, 65535 );

        const_cast<GDIMetaFile&>(rMtf).Write( aMemStm );
        maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.TellEnd() );
    }

    return maAny.hasValue();
}


bool TransferableHelper::SetGraphic( const Graphic& rGraphic )
{
    if( rGraphic.GetType() != GraphicType::NONE )
    {
        SvMemoryStream aMemStm( 65535, 65535 );

        aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
        aMemStm.SetCompressMode( SvStreamCompressFlags::NATIVE );
        WriteGraphic( aMemStm, rGraphic );
        maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );
    }

    return maAny.hasValue();
}


bool TransferableHelper::SetImageMap( const ImageMap& rIMap )
{
    SvMemoryStream aMemStm( 8192, 8192 );

    aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
    rIMap.Write( aMemStm );
    maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );

    return maAny.hasValue();
}


bool TransferableHelper::SetTransferableObjectDescriptor( const TransferableObjectDescriptor& rDesc )
{
    PrepareOLE( rDesc );

    SvMemoryStream aMemStm( 1024, 1024 );

    WriteTransferableObjectDescriptor( aMemStm, rDesc );
    maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Tell() );

    return maAny.hasValue();
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
            maAny <<= rBmk.GetURL();
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

#ifdef _WIN32
        case SotClipboardFormatId::FILEGRPDESCRIPTOR:
        {
            Sequence< sal_Int8 >    aSeq( sizeof( FILEGROUPDESCRIPTOR ) );
            FILEGROUPDESCRIPTOR*    pFDesc = reinterpret_cast<FILEGROUPDESCRIPTOR*>(aSeq.getArray());
            FILEDESCRIPTOR&         rFDesc1 = pFDesc->fgd[ 0 ];

            pFDesc->cItems = 1;
            memset( &rFDesc1, 0, sizeof( FILEDESCRIPTOR ) );
            rFDesc1.dwFlags = FD_LINKUI;

            OStringBuffer aStr(OUStringToOString(
                rBmk.GetDescription(), eSysCSet));
            for( sal_Int32 nChar = 0; nChar < aStr.getLength(); ++nChar )
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

    return maAny.hasValue();
}


bool TransferableHelper::SetINetImage( const INetImage& rINtImg,
                                       const css::datatransfer::DataFlavor& rFlavor )
{
    SvMemoryStream aMemStm( 1024, 1024 );

    aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
    rINtImg.Write( aMemStm, SotExchange::GetFormat( rFlavor ) );

    maAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) );

    return maAny.hasValue();
}


bool TransferableHelper::SetObject( void* pUserObject, sal_uInt32 nUserObjectId, const DataFlavor& rFlavor )
{
    tools::SvRef<SotStorageStream> xStm( new SotStorageStream( OUString() ) );

    xStm->SetVersion( SOFFICE_FILEFORMAT_50 );

    if( pUserObject && WriteObject( xStm, pUserObject, nUserObjectId, rFlavor ) )
    {
        const sal_uInt32        nLen = xStm->TellEnd();
        Sequence< sal_Int8 >    aSeq( nLen );

        xStm->Seek( STREAM_SEEK_TO_BEGIN );
        xStm->ReadBytes(aSeq.getArray(), nLen);

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

    return maAny.hasValue();
}


bool TransferableHelper::WriteObject( tools::SvRef<SotStorageStream>&, void*, sal_uInt32, const DataFlavor& )
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
    mxObjDesc.reset(new TransferableObjectDescriptor(rObjDesc));

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

    if( !(mxClipboard.is() && !mxTerminateListener.is()) )
        return;

    try
    {
        TransferableHelper* pThis = const_cast< TransferableHelper* >( this );
        pThis->mxTerminateListener = new TerminateListener( *pThis );
        Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
        xDesktop->addTerminateListener( pThis->mxTerminateListener );

        mxClipboard->setContents( pThis, pThis );
    }
    catch( const css::uno::Exception& )
    {
    }
}


void TransferableHelper::CopyToSelection( vcl::Window *pWindow ) const
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );
    Reference< XClipboard > xSelection;

    if( pWindow )
        xSelection = pWindow->GetPrimarySelection();

    if( !(xSelection.is() && !mxTerminateListener.is()) )
        return;

    try
    {
        TransferableHelper* pThis = const_cast< TransferableHelper* >( this );
        pThis->mxTerminateListener = new TerminateListener( *pThis );
        Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
        xDesktop->addTerminateListener( pThis->mxTerminateListener );

        xSelection->setContents( pThis, pThis );
    }
    catch( const css::uno::Exception& )
    {
    }
}


void TransferableHelper::StartDrag( vcl::Window* pWindow, sal_Int8 nDnDSourceActions )

{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );
    Reference< XDragSource > xDragSource( pWindow->GetDragSource() );

    if( !xDragSource.is() )
        return;

    /*
     *    #96792# release mouse before actually starting DnD.
     *    This is necessary for the X11 DnD implementation to work.
     */
    if( pWindow->IsMouseCaptured() )
        pWindow->ReleaseMouse();

    const Point aPt( pWindow->GetPointerPosPixel() );

    // On macOS we are forced to execute 'startDrag' synchronously
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

        xDragSource->startDrag( aEvt, nDnDSourceActions, DND_POINTER_NONE, DND_IMAGE_NONE, this, this );
    }
    catch( const css::uno::Exception& )
    {
    }
}

void TransferableHelper::ClearSelection( vcl::Window *pWindow )
{
    DBG_ASSERT( pWindow, "Window pointer is NULL" );
    Reference< XClipboard > xSelection( pWindow->GetPrimarySelection() );

    if( xSelection.is() )
        xSelection->setContents( nullptr, nullptr );
}

namespace
{
    class theTransferableHelperUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theTransferableHelperUnoTunnelId > {};
}

const Sequence< sal_Int8 >& TransferableHelper::getUnoTunnelId()
{
    return theTransferableHelperUnoTunnelId::get().getSeq();
}

class TransferableClipboardNotifier : public ::cppu::WeakImplHelper< XClipboardListener >
{
private:
    ::osl::Mutex&                   mrMutex;
    Reference< XClipboardNotifier > mxNotifier;
    TransferableDataHelper*         mpListener;

protected:
    // XClipboardListener
    virtual void SAL_CALL changedContents( const clipboard::ClipboardEvent& event ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& Source ) override;

public:
    TransferableClipboardNotifier( const Reference< XClipboard >& _rxClipboard, TransferableDataHelper& _rListener, ::osl::Mutex& _rMutex );

    /// determines whether we're currently listening
    bool isListening() const { return mpListener != nullptr; }

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


void SAL_CALL TransferableClipboardNotifier::changedContents( const clipboard::ClipboardEvent& event )
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


void SAL_CALL TransferableClipboardNotifier::disposing( const EventObject& )
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

struct TransferableDataHelper_Impl
{
    ::osl::Mutex                    maMutex;
    rtl::Reference<TransferableClipboardNotifier>  mxClipboardListener;

    TransferableDataHelper_Impl()
    {
    }
};

TransferableDataHelper::TransferableDataHelper()
    : mxObjDesc(new TransferableObjectDescriptor)
    , mxImpl(new TransferableDataHelper_Impl)
{
}

TransferableDataHelper::TransferableDataHelper(const Reference< css::datatransfer::XTransferable >& rxTransferable)
    : mxTransfer(rxTransferable)
    , mxObjDesc(new TransferableObjectDescriptor)
    , mxImpl(new TransferableDataHelper_Impl)
{
    InitFormats();
}

TransferableDataHelper::TransferableDataHelper(const TransferableDataHelper& rDataHelper)
    : mxTransfer(rDataHelper.mxTransfer)
    , mxClipboard(rDataHelper.mxClipboard)
    , maFormats(rDataHelper.maFormats)
    , mxObjDesc(new TransferableObjectDescriptor(*rDataHelper.mxObjDesc))
    , mxImpl(new TransferableDataHelper_Impl)
{
}

TransferableDataHelper::TransferableDataHelper(TransferableDataHelper&& rDataHelper)
    : mxTransfer(std::move(rDataHelper.mxTransfer))
    , mxClipboard(std::move(rDataHelper.mxClipboard))
    , maFormats(std::move(rDataHelper.maFormats))
    , mxObjDesc(std::move(rDataHelper.mxObjDesc))
    , mxImpl(new TransferableDataHelper_Impl)
{
}

TransferableDataHelper& TransferableDataHelper::operator=( const TransferableDataHelper& rDataHelper )
{
    if ( this != &rDataHelper )
    {
        ::osl::MutexGuard aGuard(mxImpl->maMutex);

        const bool bWasClipboardListening = mxImpl->mxClipboardListener.is();

        if (bWasClipboardListening)
            StopClipboardListening();

        mxTransfer = rDataHelper.mxTransfer;
        maFormats = rDataHelper.maFormats;
        mxObjDesc.reset(new TransferableObjectDescriptor(*rDataHelper.mxObjDesc));
        mxClipboard = rDataHelper.mxClipboard;

        if (bWasClipboardListening)
            StartClipboardListening();
    }

    return *this;
}

TransferableDataHelper& TransferableDataHelper::operator=(TransferableDataHelper&& rDataHelper)
{
    ::osl::MutexGuard aGuard(mxImpl->maMutex);

    const bool bWasClipboardListening = mxImpl->mxClipboardListener.is();

    if (bWasClipboardListening)
        StopClipboardListening();

    mxTransfer = std::move(rDataHelper.mxTransfer);
    maFormats = std::move(rDataHelper.maFormats);
    mxObjDesc = std::move(rDataHelper.mxObjDesc);
    mxClipboard = std::move(rDataHelper.mxClipboard);

    if (bWasClipboardListening)
        StartClipboardListening();

    return *this;
}

TransferableDataHelper::~TransferableDataHelper()
{
    StopClipboardListening( );
    {
        ::osl::MutexGuard aGuard(mxImpl->maMutex);
        maFormats.clear();
        mxObjDesc.reset();
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


        for (auto const& rFlavor : rDataFlavorSeq)
        {
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
            else if( xMimeType.is() && xMimeType->getFullMediaType().equalsIgnoreAsciiCase( "text/richtext" ) )
            {
                rDataFlavorExVector[ rDataFlavorExVector.size() - 1 ].mnSotId = SotClipboardFormatId::RICHTEXT;
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
    ::osl::MutexGuard aGuard(mxImpl->maMutex);

    maFormats.clear();
    mxObjDesc.reset(new TransferableObjectDescriptor);

    if( !mxTransfer.is() )
        return;

    TransferableDataHelper::FillDataFlavorExVector(mxTransfer->getTransferDataFlavors(), maFormats);

    for (auto const& format : maFormats)
    {
        if( SotClipboardFormatId::OBJECTDESCRIPTOR == format.mnSotId )
        {
            ImplSetParameterString(*mxObjDesc, format);
            break;
        }
    }
}


bool TransferableDataHelper::HasFormat( SotClipboardFormatId nFormat ) const
{
    ::osl::MutexGuard aGuard(mxImpl->maMutex);
    return std::any_of(maFormats.begin(), maFormats.end(),
              [&](const DataFlavorEx& data) { return data.mnSotId == nFormat; });
}

bool TransferableDataHelper::HasFormat( const DataFlavor& rFlavor ) const
{
    ::osl::MutexGuard aGuard(mxImpl->maMutex);
    for (auto const& format : maFormats)
    {
        if( TransferableDataHelper::IsEqual( rFlavor, format ) )
            return true;
    }

    return false;
}

sal_uInt32 TransferableDataHelper::GetFormatCount() const
{
    ::osl::MutexGuard aGuard(mxImpl->maMutex);
    return maFormats.size();
}

SotClipboardFormatId TransferableDataHelper::GetFormat( sal_uInt32 nFormat ) const
{
    ::osl::MutexGuard aGuard(mxImpl->maMutex);
    DBG_ASSERT(nFormat < maFormats.size(), "TransferableDataHelper::GetFormat: invalid format index");
    return( ( nFormat < maFormats.size() ) ? maFormats[ nFormat ].mnSotId : SotClipboardFormatId::NONE );
}

DataFlavor TransferableDataHelper::GetFormatDataFlavor( sal_uInt32 nFormat ) const
{
    ::osl::MutexGuard aGuard(mxImpl->maMutex);
    DBG_ASSERT(nFormat < maFormats.size(), "TransferableDataHelper::GetFormat: invalid format index");

    DataFlavor aRet;

    if (nFormat < maFormats.size())
        aRet = maFormats[nFormat];

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
    ::osl::MutexGuard aGuard(mxImpl->maMutex);
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
                for (auto const& format : maFormats)
                {
                    if( ( nRequestFormat == format.mnSotId ) && !rFlavor.MimeType.equalsIgnoreAsciiCase( format.MimeType ) )
                    {
                        if (xTransfer2.is())
                            aRet = xTransfer2->getTransferData2(format, rDestDoc);
                        else
                            aRet = mxTransfer->getTransferData(format);
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
    bool bSuppressJPEG(false);

    if(!bRet && HasFormat(SotClipboardFormatId::PNG) && SotExchange::GetFormatDataFlavor(SotClipboardFormatId::PNG, aSubstFlavor))
    {
        // when no direct success, try if PNG is available
        bRet = GetSotStorageStream(aSubstFlavor, xStm);
        bSuppressJPEG = bRet;
    }

    if(!bRet && HasFormat(SotClipboardFormatId::JPEG) && SotExchange::GetFormatDataFlavor(SotClipboardFormatId::JPEG, aSubstFlavor))
    {
        bRet = GetSotStorageStream(aSubstFlavor, xStm);
        bSuppressPNG = bRet;
    }

    if(!bRet && HasFormat(SotClipboardFormatId::BMP) && SotExchange::GetFormatDataFlavor(SotClipboardFormatId::BMP, aSubstFlavor))
    {
        // when no direct success, try if BMP is available
        bRet = GetSotStorageStream(aSubstFlavor, xStm);
        bSuppressPNG = bRet;
        bSuppressJPEG = bRet;
    }

    if(bRet)
    {
        if(!bSuppressPNG && rFlavor.MimeType.equalsIgnoreAsciiCase("image/png"))
        {
            // it's a PNG, import to BitmapEx
            vcl::PNGReader aPNGReader(*xStm);

            rBmpEx = aPNGReader.Read();
        }
        else if(!bSuppressJPEG && rFlavor.MimeType.equalsIgnoreAsciiCase("image/jpeg"))
        {
            // it's a JPEG, import to BitmapEx
            GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
            Graphic aGraphic;
            if (rFilter.ImportGraphic(aGraphic, "", *xStm) == ERRCODE_NONE)
                rBmpEx = aGraphic.GetBitmapEx();
        }

        if(rBmpEx.IsEmpty())
        {
            Bitmap aBitmap;
            AlphaMask aMask;

            // explicitly use Bitmap::Read with bFileHeader = sal_True
            // #i124085# keep DIBV5 for read from clipboard, but should not happen
            ReadDIBV5(aBitmap, aMask, *xStm);

            if(aMask.GetBitmap().IsEmpty())
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

            if(MapUnit::MapPixel != aMapMode.GetMapUnit())
            {
                const Size aSize(OutputDevice::LogicToLogic(rBmpEx.GetPrefSize(), aMapMode, MapMode(MapUnit::Map100thMM)));

                // #i122388# This wrongly corrects in the given case; changing from 5000 100th mm to
                // the described 50 cm (which is 50000 100th mm)
                if((aSize.Width() > 50000) || (aSize.Height() > 50000))
                {
                    rBmpEx.SetPrefMapMode(MapMode(MapUnit::MapPixel));

                    // #i122388# also adapt size by applying the mew MapMode
                    const Size aNewSize(OutputDevice::LogicToLogic(aSize, MapMode(MapUnit::Map100thMM), MapMode(MapUnit::MapPixel)));
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

        bRet = GetBitmapEx( aFlavor, aBmpEx );
        if( bRet )
            rGraphic = aBmpEx;
    }
    else if (SotExchange::GetFormatDataFlavor(SotClipboardFormatId::JPEG, aFlavor) && TransferableDataHelper::IsEqual(aFlavor, rFlavor))
    {
        BitmapEx aBitmapEx;

        bRet = GetBitmapEx(aFlavor, aBitmapEx);
        if (bRet)
            rGraphic = aBitmapEx;
    }
    else if(SotExchange::GetFormatDataFlavor( SotClipboardFormatId::BITMAP, aFlavor ) &&
        TransferableDataHelper::IsEqual( aFlavor, rFlavor ) )
    {
        BitmapEx aBmpEx;

        bRet = GetBitmapEx( aFlavor, aBmpEx );
        if( bRet )
            rGraphic = aBmpEx;
    }
    else if( SotExchange::GetFormatDataFlavor( SotClipboardFormatId::GDIMETAFILE, aFlavor ) &&
             TransferableDataHelper::IsEqual( aFlavor, rFlavor ) )
    {
        GDIMetaFile aMtf;

        bRet = GetGDIMetaFile( aFlavor, aMtf );
        if( bRet )
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
        rIMap.Read( *xStm );
        bRet = ( xStm->GetError() == ERRCODE_NONE );
    }

    return bRet;
}


bool TransferableDataHelper::GetTransferableObjectDescriptor( SotClipboardFormatId nFormat, TransferableObjectDescriptor& rDesc )
{
    DataFlavor aFlavor;
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetTransferableObjectDescriptor( rDesc ) );
}


bool TransferableDataHelper::GetTransferableObjectDescriptor( TransferableObjectDescriptor& rDesc )
{
    rDesc = *mxObjDesc;
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
        case SotClipboardFormatId::SOLK:
        case SotClipboardFormatId::UNIFORMRESOURCELOCATOR:
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

        case SotClipboardFormatId::NETSCAPE_BOOKMARK:
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

#ifdef _WIN32
        case SotClipboardFormatId::FILEGRPDESCRIPTOR:
        {
            Sequence<sal_Int8> aSeq = GetSequence(rFlavor, OUString());

            if (aSeq.getLength())
            {
                FILEGROUPDESCRIPTOR const * pFDesc = reinterpret_cast<FILEGROUPDESCRIPTOR const *>(aSeq.getConstArray());

                if( pFDesc->cItems )
                {
                    OString aDesc( pFDesc->fgd[ 0 ].cFileName );
                    rtl_TextEncoding    eTextEncoding = osl_getThreadTextEncoding();

                    if( ( aDesc.getLength() > 4 ) && aDesc.copy(aDesc.getLength() - 4).equalsIgnoreAsciiCase(".URL") )
                    {
                        std::unique_ptr<SvStream> pStream(::utl::UcbStreamHelper::CreateStream( INetURLObject( OStringToOUString(aDesc, eTextEncoding) ).GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                                                                  StreamMode::STD_READ ));

                        if( !pStream || pStream->GetError() )
                        {
                            DataFlavor aFileContentFlavor;

                            aSeq.realloc( 0 );
                            pStream.reset();

                            if (SotExchange::GetFormatDataFlavor(SotClipboardFormatId::FILECONTENT, aFileContentFlavor))
                            {
                                aSeq = GetSequence(aFileContentFlavor, OUString());
                                if (aSeq.getLength())
                                    pStream.reset(new SvMemoryStream( const_cast<sal_Int8 *>(aSeq.getConstArray()), aSeq.getLength(), StreamMode::STD_READ ));
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
    return( SotExchange::GetFormatDataFlavor( nFormat, aFlavor ) && GetFileList( rFileList ) );
}


bool TransferableDataHelper::GetFileList( FileList& rFileList )
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
                    bRet = ( ReadFileList( *xStm, rFileList ).GetError() == ERRCODE_NONE );
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
        rxStream->WriteBytes( aSeq.getConstArray(), aSeq.getLength() );
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
    ::osl::MutexGuard aGuard(mxImpl->maMutex);

    StopClipboardListening( );

    mxImpl->mxClipboardListener = new TransferableClipboardNotifier(mxClipboard, *this, mxImpl->maMutex);

    return mxImpl->mxClipboardListener->isListening();
}

void TransferableDataHelper::StopClipboardListening( )
{
    ::osl::MutexGuard aGuard(mxImpl->maMutex);

    if (mxImpl->mxClipboardListener.is())
    {
        mxImpl->mxClipboardListener->dispose();
        mxImpl->mxClipboardListener.clear();
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
                                      const css::datatransfer::DataFlavor& rRequestFlavor )
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
