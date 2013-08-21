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


#ifndef __GNUC__
#   pragma hdrstop
#endif

#include <tools/stream.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <svtools/svtools.hrc>
#include <svtools/svtresid.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/graphic/PrimitiveFactory2D.hpp>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <unotools/streamwrap.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graph.hxx>
#include <rtl/ustrbuf.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <comphelper/processfactory.hxx>
#include "exportdialog.hxx"

#define FORMAT_UNKNOWN  0
#define FORMAT_JPG      1
#define FORMAT_PNG      2
#define FORMAT_BMP      3
#define FORMAT_GIF      4
#define FORMAT_PBM      5
#define FORMAT_PGM      6
#define FORMAT_PPM      7
#define FORMAT_PCT      8
#define FORMAT_RAS      9
#define FORMAT_TIF      10
#define FORMAT_XPM      11
#define FORMAT_WMF      12
#define FORMAT_EMF      13
#define FORMAT_EPS      14
#define FORMAT_MET      15
#define FORMAT_SVG      16
#define FORMAT_SVM      17

#define UNIT_DEFAULT    -1
#define UNIT_INCH       0
#define UNIT_CM         1
#define UNIT_MM         2
#define UNIT_POINT      3
#define UNIT_PIXEL      4
#define UNIT_MAX_ID     UNIT_PIXEL

using namespace ::com::sun::star;

static sal_Int16 GetFilterFormat(const OUString& rExt)
{
    sal_Int16 nFormat = FORMAT_UNKNOWN;
    if ( rExt == "JPG" )
        nFormat = FORMAT_JPG;
    else if ( rExt == "PNG" )
        nFormat = FORMAT_PNG;
    else if ( rExt == "BMP" )
        nFormat = FORMAT_BMP;
    else if ( rExt == "GIF" )
        nFormat = FORMAT_GIF;
    else if ( rExt == "PBM" )
        nFormat = FORMAT_PBM;
    else if ( rExt == "PGM" )
        nFormat = FORMAT_PGM;
    else if ( rExt == "PPM" )
        nFormat = FORMAT_PPM;
    else if ( rExt == "PCT" )
        nFormat = FORMAT_PCT;
    else if ( rExt == "RAS" )
        nFormat = FORMAT_RAS;
    else if ( rExt == "TIF" )
        nFormat = FORMAT_TIF;
    else if ( rExt == "XPM" )
        nFormat = FORMAT_XPM;
    else if ( rExt == "WMF" )
        nFormat = FORMAT_WMF;
    else if ( rExt == "EMF" )
        nFormat = FORMAT_EMF;
    else if ( rExt == "EPS" )
        nFormat = FORMAT_EPS;
    else if ( rExt == "MET" )
        nFormat = FORMAT_MET;
    else if ( rExt == "SVG" )
        nFormat = FORMAT_SVG;
    else if ( rExt == "SVM" )
        nFormat = FORMAT_SVM;
    return nFormat;
}

static MapUnit GetMapUnit( sal_Int32 nUnit )
{
    MapUnit aMapUnit( MAP_PIXEL );
    switch( nUnit )
    {
        case UNIT_INCH  :   aMapUnit = MAP_INCH; break;
        case UNIT_CM    :   aMapUnit = MAP_CM; break;
        case UNIT_MM    :   aMapUnit = MAP_MM; break;
        case UNIT_POINT :   aMapUnit = MAP_POINT; break;
        case UNIT_PIXEL :   aMapUnit = MAP_PIXEL; break;
    }
    return aMapUnit;
}

sal_Int32 ExportDialog::GetDefaultUnit()
{
    sal_Int32 nDefaultUnit = UNIT_CM;
    switch( mrFltCallPara.eFieldUnit )
    {
//      case FUNIT_NONE :
//      case FUNIT_PERCENT :
//      case FUNIT_CUSTOM :
        default:                nDefaultUnit = UNIT_CM; break;

        case FUNIT_MILE :       // PASSTHROUGH INTENDED
        case FUNIT_FOOT :
        case FUNIT_TWIP :
        case FUNIT_PICA :       nDefaultUnit = UNIT_INCH; break;

        case FUNIT_KM :         // PASSTHROUGH INTENDED
        case FUNIT_M :
        case FUNIT_100TH_MM :   nDefaultUnit = UNIT_CM; break;

        case FUNIT_INCH :       nDefaultUnit = UNIT_INCH; break;
        case FUNIT_CM :         nDefaultUnit = UNIT_CM; break;
        case FUNIT_MM :         nDefaultUnit = UNIT_MM; break;
        case FUNIT_POINT :      nDefaultUnit = UNIT_POINT; break;
    }
    return nDefaultUnit;
}

static basegfx::B2DRange GetShapeRangeForXShape( const uno::Reference< drawing::XShape >& rxShape,
    const uno::Reference< graphic::XPrimitiveFactory2D >& rxPrimitiveFactory2D, const uno::Sequence< beans::PropertyValue >& rViewInformation )
{
    basegfx::B2DRange aShapeRange;

    const uno::Sequence< beans::PropertyValue > aParams;
    const uno::Sequence< uno::Reference< graphic::XPrimitive2D > > aPrimitiveSequence( rxPrimitiveFactory2D->createPrimitivesFromXShape( rxShape, aParams ) );

    const sal_Int32 nCount = aPrimitiveSequence.getLength();
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const geometry::RealRectangle2D aRect( aPrimitiveSequence[ nIndex ]->getRange( rViewInformation ) );
        aShapeRange.expand( basegfx::B2DTuple( aRect.X1, aRect.Y1 ) );
        aShapeRange.expand( basegfx::B2DTuple( aRect.X2, aRect.Y2 ) );
    }
    return aShapeRange;
}

uno::Sequence< beans::PropertyValue > ExportDialog::GetFilterData( sal_Bool bUpdateConfig )
{
    if ( bUpdateConfig )
    {
        sal_Int32 nUnit = mpLbSizeX->GetSelectEntryPos();
        if ( nUnit < 0 )
            nUnit = UNIT_CM;

        if ( ( mnInitialResolutionUnit == UNIT_DEFAULT ) && ( nUnit == GetDefaultUnit() ) )
            nUnit = UNIT_DEFAULT;

        // updating ui configuration
        if ( mbIsPixelFormat )
        {
            if ( nUnit > UNIT_MAX_ID )
                nUnit = UNIT_PIXEL;

            sal_Int32 nResolution = mpNfResolution->GetValue();
            if ( nResolution < 1 )
                nResolution = 96;

            mpOptionsItem->WriteInt32(OUString("PixelExportUnit"), nUnit);
            mpOptionsItem->WriteInt32(OUString("PixelExportResolution"), nResolution);
            mpOptionsItem->WriteInt32(OUString("PixelExportResolutionUnit"), mpLbResolution->GetSelectEntryPos());
        }
        else
        {
            if ( nUnit >= UNIT_PIXEL )
                nUnit = UNIT_CM;

            mpOptionsItem->WriteInt32(OUString("VectorExportUnit"), nUnit);
        }
    }

    FilterConfigItem* pFilterOptions;
    if ( bUpdateConfig )
         pFilterOptions = mpFilterOptionsItem;
    else
    {
        uno::Sequence< beans::PropertyValue > aFilterData( mpFilterOptionsItem->GetFilterData() );
        pFilterOptions = new FilterConfigItem( &aFilterData );
    }

    const OUString sLogicalWidth(OUString("LogicalWidth"));
    const OUString sLogicalHeight(OUString("LogicalHeight"));
    if ( mbIsPixelFormat )
    {
        pFilterOptions->WriteInt32(OUString("PixelWidth"), static_cast< sal_Int32 >( maSize.Width ) );
        pFilterOptions->WriteInt32(OUString("PixelHeight"), static_cast< sal_Int32 >( maSize.Height ) );
        if ( maResolution.Width && maResolution.Height )
        {
            const double f100thmmPerPixelX = 100000.0 / maResolution.Width;
            const double f100thmmPerPixelY = 100000.0 / maResolution.Height;
            sal_Int32 nLogicalWidth = static_cast< sal_Int32 >( f100thmmPerPixelX * maSize.Width );
            sal_Int32 nLogicalHeight= static_cast< sal_Int32 >( f100thmmPerPixelY * maSize.Height );
            if ( nLogicalWidth && nLogicalHeight )
            {
                pFilterOptions->WriteInt32( sLogicalWidth, nLogicalWidth );
                pFilterOptions->WriteInt32( sLogicalHeight, nLogicalHeight );
            }
        }
    }
    else
    {
        pFilterOptions->WriteInt32( sLogicalWidth, static_cast< sal_Int32 >( maSize.Width ) );
        pFilterOptions->WriteInt32( sLogicalHeight, static_cast< sal_Int32 >( maSize.Height ) );
    }
    switch ( mnFormat )
    {
        case FORMAT_JPG :
        {
            sal_Int32 nColor = mpLbColorDepth->GetSelectEntryPos();
            if ( nColor == 1 )
                nColor = 0;
            else
                nColor = 1;
            pFilterOptions->WriteInt32(OUString("ColorMode"), nColor);
            pFilterOptions->WriteInt32(OUString("Quality"), static_cast<sal_Int32>(mpSbCompression->GetThumbPos()));
        }
        break;

        case FORMAT_PNG :
        {
            pFilterOptions->WriteInt32(OUString("Compression"), static_cast<sal_Int32>(mpSbCompression->GetThumbPos()));
            sal_Int32 nInterlace = 0;
            if ( mpCbInterlaced->IsChecked() )
                nInterlace++;
            pFilterOptions->WriteInt32(OUString("Interlaced"), nInterlace);
            sal_Int32 nValue = 0;
            if ( mpCbSaveTransparency->IsChecked() )
                nValue++;
            pFilterOptions->WriteInt32(OUString("Translucent"), nValue);
        }
        break;

        case FORMAT_BMP :
        {
            pFilterOptions->WriteInt32(OUString("Color"), mpLbColorDepth->GetSelectEntryPos() + 1);
            pFilterOptions->WriteBool(OUString("RLE_Coding"), mpCbRLEEncoding->IsChecked());
        }
        break;

        case FORMAT_GIF :
        {
            sal_Int32 nValue = 0;
            if ( mpCbInterlaced->IsChecked() )
                nValue++;
            pFilterOptions->WriteInt32(OUString("Interlaced"), nValue);

            nValue = 0;
            if (mpCbSaveTransparency->IsChecked())
                nValue++;
            pFilterOptions->WriteInt32(OUString("Translucent"), nValue);
        }
        break;

        case FORMAT_PBM :
        case FORMAT_PGM :
        case FORMAT_PPM :
        {
            sal_Int32 nFormat = 0;
            if ( mpRbText->IsChecked() )
                nFormat++;
            pFilterOptions->WriteInt32(OUString("FileFormat"), nFormat);
        }
        break;

        case FORMAT_EPS :
        {
            sal_Int32 nCheck = 0;
            if ( mpCbEPSPreviewTIFF->IsChecked() )
                nCheck++;
            if ( mpCbEPSPreviewEPSI->IsChecked() )
                nCheck += 2;
            pFilterOptions->WriteInt32(OUString("Preview"), nCheck);

            nCheck = 1;
            if ( mpRbEPSLevel2->IsChecked() )
                nCheck++;
            pFilterOptions->WriteInt32(OUString("Version"), nCheck);

            nCheck = 1;
            if ( mpRbEPSColorFormat2->IsChecked() )
                nCheck++;
            pFilterOptions->WriteInt32(OUString("ColorFormat"), nCheck);

            nCheck = 1;
            if ( mpRbEPSCompressionNone->IsChecked() )
                nCheck++;
            pFilterOptions->WriteInt32(OUString("CompressionMode"), nCheck);
        }
        break;
    }

    uno::Sequence< beans::PropertyValue > aRet( pFilterOptions->GetFilterData() );
    if ( bUpdateConfig == sal_False )
        delete pFilterOptions;
    return aRet;
}

//
awt::Size ExportDialog::GetOriginalSize()
{
    basegfx::B2DRange aShapesRange;

    if ( mxPage.is () )
    {
        uno::Reference< beans::XPropertySet > xPagePropSet( mxPage, uno::UNO_QUERY );
        if ( xPagePropSet.is() )
        {
            sal_Int32 nWidth = 0;
            sal_Int32 nHeight= 0;
            com::sun::star::uno::Any aAny;
            aAny = xPagePropSet->getPropertyValue("Width");
            aAny >>= nWidth;
            aAny = xPagePropSet->getPropertyValue("Height");
            aAny >>= nHeight;
            aShapesRange = basegfx::B2DRange( 0, 0, nWidth, nHeight );
        }
    }
    else
    {
        uno::Reference< graphic::XPrimitiveFactory2D > xPrimitiveFactory = graphic::PrimitiveFactory2D::create( mxContext );

        basegfx::B2DHomMatrix aViewTransformation( Application::GetDefaultDevice()->GetViewTransformation() );
        com::sun::star::geometry::AffineMatrix2D aTransformation;
        aTransformation.m00 = aViewTransformation.get(0,0);
        aTransformation.m01 = aViewTransformation.get(0,1);
        aTransformation.m02 = aViewTransformation.get(0,2);
        aTransformation.m10 = aViewTransformation.get(1,0);
        aTransformation.m11 = aViewTransformation.get(1,1);
        aTransformation.m12 = aViewTransformation.get(1,2);

        const OUString sViewTransformation( "ViewTransformation" );
        uno::Sequence< beans::PropertyValue > aViewInformation( 1 );
        aViewInformation[ 0 ].Value <<= aTransformation;
        aViewInformation[ 0 ].Name  = sViewTransformation;

        if ( mxShape.is() )
            aShapesRange = GetShapeRangeForXShape( mxShape, xPrimitiveFactory, aViewInformation );
        else if ( mxShapes.is() )
        {
            const sal_Int32 nCount = mxShapes->getCount();
            for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                uno::Reference< drawing::XShape > xShape;
                mxShapes->getByIndex( nIndex ) >>= xShape;
                aShapesRange.expand( GetShapeRangeForXShape( xShape, xPrimitiveFactory, aViewInformation ) );
            }
        }
    }
    return awt::Size( static_cast<sal_Int32>(aShapesRange.getWidth()), static_cast<sal_Int32>(aShapesRange.getHeight()) );
}

void ExportDialog::GetGraphicSource()
{
    if ( mxSourceDocument.is() )
    {
        uno::Reference< frame::XModel > xModel( mxSourceDocument, uno::UNO_QUERY );
        if ( xModel.is() )
        {
            uno::Reference< frame::XController > xController( xModel->getCurrentController() );
            if ( xController.is() )
            {
                if ( mbExportSelection )                // check if there is a selection
                {
                    uno::Reference< view::XSelectionSupplier > xSelectionSupplier( xController, uno::UNO_QUERY );
                    if ( xSelectionSupplier.is() )
                    {
                        uno::Any aAny( xSelectionSupplier->getSelection() );
                        if ( ! ( aAny >>= mxShapes ) )
                            aAny >>= mxShape;
                    }
                }
                if ( !mxShape.is() && !mxShapes.is() )
                {
                    uno::Reference< drawing::XDrawView > xDrawView( xController, uno::UNO_QUERY );
                    if ( xDrawView.is() )
                    {
                        uno::Reference< drawing::XDrawPage > xCurrentPage( xDrawView->getCurrentPage() );
                        if ( xCurrentPage.is() )
                        {
                            mxPage = xCurrentPage;      // exporting whole page
                        }
                    }
                }
            }
        }
    }
}

sal_Bool ExportDialog::GetGraphicStream()
{
    sal_Bool bRet = sal_False;

    if ( !IsTempExportAvailable() )
    {
        delete mpTempStream, mpTempStream = new SvMemoryStream();
        maBitmap = Bitmap();
        return bRet;
    }

    sal_Bool bRecreateOutputStream = mpTempStream->Tell() == 0;

    static uno::Sequence< beans::PropertyValue > aOldFilterData;
    uno::Sequence< beans::PropertyValue > aNewFilterData( GetFilterData( sal_False ) );
    if ( aOldFilterData != aNewFilterData )
    {
        aOldFilterData = aNewFilterData;
        bRecreateOutputStream = sal_True;
    }
    try
    {
        if ( bRecreateOutputStream )
        {
            delete mpTempStream, mpTempStream = new SvMemoryStream();
            maBitmap = Bitmap();

            uno::Reference < io::XStream > xStream( new utl::OStreamWrapper( *mpTempStream ) );
            uno::Reference < io::XOutputStream > xOutputStream( xStream->getOutputStream() );

            uno::Reference< drawing::XGraphicExportFilter > xGraphicExporter =
                drawing::GraphicExportFilter::create( mxContext );

            sal_Int32 nProperties = 2;
            uno::Sequence< beans::PropertyValue > aFilterData( nProperties );


            OUString sFormat( maExt );
            uno::Sequence< beans::PropertyValue > aDescriptor( 3 );
            aDescriptor[0].Name = OUString("OutputStream");
            aDescriptor[0].Value <<= xOutputStream;
            aDescriptor[1].Name = OUString("FilterName");
            aDescriptor[1].Value <<= sFormat;
            aDescriptor[2].Name = OUString("FilterData");
            aDescriptor[2].Value <<= aNewFilterData;

            uno::Reference< lang::XComponent > xSourceDoc;
            if ( mxPage.is() )
                xSourceDoc = uno::Reference< lang::XComponent >( mxPage, uno::UNO_QUERY_THROW );
            else if ( mxShapes.is() )
                xSourceDoc = uno::Reference< lang::XComponent >( mxShapes, uno::UNO_QUERY_THROW );
            else if ( mxShape.is() )
                xSourceDoc = uno::Reference< lang::XComponent >( mxShape, uno::UNO_QUERY_THROW );
            if ( xSourceDoc.is() )
            {
                xGraphicExporter->setSourceDocument( xSourceDoc );
                xGraphicExporter->filter( aDescriptor );
                bRet = sal_True;

                if ( mnFormat == FORMAT_JPG )
                {
                    mpTempStream->Seek( STREAM_SEEK_TO_BEGIN );
                    maBitmap = GetGraphicBitmap( *mpTempStream );
                    mpTempStream->Seek( STREAM_SEEK_TO_END );
                }
            }
        }
        else
            bRet = sal_True;
    }
    catch( uno::Exception& )
    {

        // ups

    }
    return bRet;
}

Bitmap ExportDialog::GetGraphicBitmap( SvStream& rInputStream )
{
    Bitmap aRet;
    Graphic aGraphic;
    GraphicFilter aFilter( sal_False );
    if ( aFilter.ImportGraphic( aGraphic, String(), rInputStream, GRFILTER_FORMAT_NOTFOUND, NULL, 0, static_cast<com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >*>(NULL), NULL ) == GRFILTER_OK )
    {
        aRet = aGraphic.GetBitmap();
    }
    return aRet;
}

sal_uInt32 ExportDialog::GetRawFileSize() const
{
    sal_uInt64 nRawFileSize = 0;
    if ( mbIsPixelFormat )
    {
        sal_Int32 nBitsPerPixel = 24;
        String aEntry( mpLbColorDepth->GetSelectEntry() );
        if ( ms1BitTreshold == aEntry )
            nBitsPerPixel = 1;
        else if ( ms1BitDithered == aEntry )
            nBitsPerPixel = 1;
        else if ( ms4BitGrayscale == aEntry )
            nBitsPerPixel = 4;
        else if ( ms4BitColorPalette == aEntry )
            nBitsPerPixel = 8;
        else if ( ms8BitGrayscale == aEntry )
            nBitsPerPixel = 8;
        else if ( ms8BitColorPalette == aEntry )
            nBitsPerPixel = 8;
        else if ( ms24BitColor == aEntry )
            nBitsPerPixel = 24;

        if ( mbIsPixelFormat )
        {
            nRawFileSize = ( maSize.Width * nBitsPerPixel + 7 ) &~ 7;   // rounding up to 8 bits
            nRawFileSize /= 8;                                          // in bytes
            nRawFileSize *= maSize.Height;
        }
        if ( nRawFileSize > SAL_MAX_UINT32 )
            nRawFileSize = 0;
    }
    return static_cast< sal_uInt32 >( nRawFileSize );
}

// checks if the source dimension/resolution is not too big
// to determine the exact graphic output size and preview for jpg
sal_Bool ExportDialog::IsTempExportAvailable() const
{
    return GetRawFileSize() < static_cast< sal_uInt32 >( mnMaxFilesizeForRealtimePreview );
}

ExportDialog::ExportDialog(FltCallDialogParameter& rPara,
    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
    const com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rxSourceDocument,
    sal_Bool bExportSelection, sal_Bool bIsPixelFormat)
    : ModalDialog(rPara.pWindow, "GraphicExportDialog", "svt/ui/graphicexport.ui")
    , mrFltCallPara(rPara)
    , mxContext(rxContext)
    , mxSourceDocument(rxSourceDocument)
    , mpSbCompression(NULL)
    , mpNfCompression(NULL)
    , msEstimatedSizePix1(SVT_RESSTR(STR_SVT_ESTIMATED_SIZE_PIX_1))
    , msEstimatedSizePix2(SVT_RESSTR(STR_SVT_ESTIMATED_SIZE_PIX_2))
    , msEstimatedSizeVec(SVT_RESSTR(STR_SVT_ESTIMATED_SIZE_VEC))
    , ms1BitTreshold(SVT_RESSTR(STR_SVT_1BIT_THRESHOLD))
    , ms1BitDithered(SVT_RESSTR(STR_SVT_1BIT_DITHERED))
    , ms4BitGrayscale(SVT_RESSTR(STR_SVT_4BIT_GRAYSCALE))
    , ms4BitColorPalette(SVT_RESSTR(STR_SVT_4BIT_COLOR_PALETTE))
    , ms8BitGrayscale(SVT_RESSTR(STR_SVT_8BIT_GRAYSCALE))
    , ms8BitColorPalette(SVT_RESSTR(STR_SVT_8BIT_COLOR_PALETTE))
    , ms24BitColor(SVT_RESSTR(STR_SVT_24BIT_TRUE_COLOR))
    , maExt(rPara.aFilterExt)
    , mnFormat(FORMAT_UNKNOWN)
    , mnMaxFilesizeForRealtimePreview(0)
    , mpTempStream(new SvMemoryStream())
    , maOriginalSize(awt::Size(0, 0))
    , mbIsPixelFormat(bIsPixelFormat)
    , mbExportSelection(bExportSelection)
    , mbPreserveAspectRatio(sal_True)
{
    get(mpMfSizeX, "widthmf-nospin");
    get(mpMfSizeY, "heightmf-nospin");
    get(mpLbSizeX, "widthlb");
    get(mpFtResolution, "resolutionft");
    get(mpNfResolution, "resolutionmf-nospin");
    get(mpLbResolution, "resolutionlb");

    get(mpColorDepth, "colordepth");
    get(mpLbColorDepth, "colordepthlb");

    get(mpJPGQuality, "jpgquality");
    get(mpPNGCompression, "pngcompression");

    get(mpBMPCompression, "bmpcompression");
    get(mpCbRLEEncoding, "rlecb");

    get(mpMode, "mode");
    get(mpCbInterlaced, "interlacedcb");

    get(mpDrawingObjects, "drawingobjects");
    get(mpCbSaveTransparency, "savetransparencycb");

    get(mpEncoding, "encoding");
    get(mpRbBinary, "binarycb");
    get(mpRbText, "textcb");

    get(mpEPSGrid, "epsgrid");
    get(mpCbEPSPreviewTIFF, "tiffpreviewcb");
    get(mpCbEPSPreviewEPSI, "epsipreviewcb");
    get(mpRbEPSLevel1, "level1rb");
    get(mpRbEPSLevel2, "level2rb");
    get(mpRbEPSColorFormat1, "color1rb");
    get(mpRbEPSColorFormat2, "color2rb");
    get(mpRbEPSCompressionLZW, "compresslzw");
    get(mpRbEPSCompressionNone, "compressnone");

    GetGraphicSource();

    get(mpInfo, "information");
    get(mpFtEstimatedSize, "estsizeft");;

    get(mpBtnOK, "ok");

    maExt.ToUpperAscii();

    String  aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/" ) );
    mpOptionsItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );
    aFilterConfigPath.Append( maExt );
    mpFilterOptionsItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );

    mnInitialResolutionUnit = mbIsPixelFormat
        ? mpOptionsItem->ReadInt32(OUString("PixelExportUnit"), UNIT_DEFAULT)
        : mpOptionsItem->ReadInt32(OUString( "VectorExportUnit"), UNIT_DEFAULT);

    mnMaxFilesizeForRealtimePreview = mpOptionsItem->ReadInt32(OUString("MaxFilesizeForRealtimePreview"), 0);
    mpFtEstimatedSize->SetText(OUString(" \n "));

    String aTitle(maExt);
    aTitle += GetText();
    SetText(aTitle);

    mnFormat = GetFilterFormat( maExt );

    Size aResolution( Application::GetDefaultDevice()->LogicToPixel( Size( 100, 100 ), MAP_CM ) );
    maResolution.Width = aResolution.Width();
    maResolution.Height= aResolution.Height();
    maOriginalSize = GetOriginalSize();
    if ( bIsPixelFormat )
    {
        double fPixelsPer100thmm = static_cast< double >( maResolution.Width ) / 100000.0;
        maSize = awt::Size( static_cast< sal_Int32 >( ( fPixelsPer100thmm * maOriginalSize.Width ) + 0.5 ),
            static_cast< sal_Int32 >( ( fPixelsPer100thmm * maOriginalSize.Height ) + 0.5 ) );
    }
    else
    {
        maSize = maOriginalSize;
    }

    setupControls();

    // Size
    mpLbSizeX->SetSelectHdl( LINK( this, ExportDialog, UpdateHdl ) );

    if (mpSbCompression)
        mpSbCompression->SetScrollHdl(LINK(this, ExportDialog, SbCompressionUpdateHdl));
    if (mpNfCompression)
        mpNfCompression->SetModifyHdl(LINK(this, ExportDialog, UpdateHdl));

    mpMfSizeX->SetModifyHdl( LINK( this, ExportDialog, UpdateHdlMtfSizeX ) );
    mpMfSizeY->SetModifyHdl( LINK( this, ExportDialog, UpdateHdlMtfSizeY ) );

    mpNfResolution->SetModifyHdl( LINK( this, ExportDialog, UpdateHdlNfResolution ) );
    mpLbResolution->SetSelectHdl( LINK( this, ExportDialog, UpdateHdl ) );

    mpLbColorDepth->SetSelectHdl( LINK( this, ExportDialog, UpdateHdl ) );

    mpCbInterlaced->SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    mpCbSaveTransparency->SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    mpCbEPSPreviewTIFF->SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );
    mpCbEPSPreviewEPSI->SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    mpRbEPSCompressionLZW->SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );
    mpRbEPSCompressionNone->SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    mpRbBinary->SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );
    mpRbText->SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    // BMP
    mpCbRLEEncoding->SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    // EPS
    mpRbEPSLevel1->SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );
    mpRbEPSLevel2->SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    mpBtnOK->SetClickHdl( LINK( this, ExportDialog, OK ) );

    updateControls();
}

void ExportDialog::setupSizeControls()
{
    sal_Int32 nUnit = mnInitialResolutionUnit;
    if (nUnit == UNIT_DEFAULT)
        nUnit = GetDefaultUnit();

    if (!mbIsPixelFormat)
    {
        mpFtResolution->Hide();
        mpNfResolution->Hide();
        mpLbResolution->Hide();
        mpLbSizeX->RemoveEntry( UNIT_PIXEL );        // removing pixel
        if ( nUnit >= UNIT_PIXEL )
            nUnit = UNIT_CM;
    }
    else if ( nUnit > UNIT_MAX_ID )
        nUnit = UNIT_PIXEL;
    if ( nUnit < 0 )
        nUnit = UNIT_CM;
    mpLbSizeX->SelectEntryPos( static_cast< sal_uInt16 >( nUnit ) );

    if ( mbIsPixelFormat )      // TODO: (metafileresolutionsupport) should be supported for vector formats also... this makes
    {                           // sense eg for bitmap fillings in metafiles, to preserve high dpi output
                                // (atm without special vector support the bitmaps are rendered with 96dpi)
        sal_Int32 nResolution = mpOptionsItem->ReadInt32(OUString("PixelExportResolution"), 96);
        if ( nResolution < 1 )
            nResolution = 96;
        mpNfResolution->SetValue( nResolution );

        sal_Int32 nResolutionUnit = mpOptionsItem->ReadInt32(OUString("PixelExportResolutionUnit"), 1);
        if ( ( nResolutionUnit < 0 ) || ( nResolutionUnit > 2 ) )
            nResolutionUnit = 1;
        mpLbResolution->SelectEntryPos( static_cast< sal_uInt16 >( nResolutionUnit ) );
    }
}

void ExportDialog::createFilterOptions()
{
    switch( mnFormat )
    {
        case FORMAT_JPG :
        {
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32(OUString("ColorMode"), 0);
            if ( nColor == 1 )
                nColor = 0;
            else
                nColor = 1;
            mpLbColorDepth->InsertEntry( ms8BitGrayscale );
            mpLbColorDepth->InsertEntry( ms24BitColor );
            mpLbColorDepth->SelectEntryPos( nColor );
            mpColorDepth->Show();

            // Quality
            mpJPGQuality->Show();
            sal_Int32 nQuality = mpFilterOptionsItem->ReadInt32(OUString("Quality"), 75);
            if ((nQuality < 1 ) || (nQuality > 100))
                nQuality = 75;
            get(mpSbCompression, "compressionjpgsb");
            get(mpNfCompression, "compressionjpgnf-nospin");
            mpSbCompression->SetRangeMin( 1 );
            mpSbCompression->SetRangeMax( 100 );
            mpNfCompression->SetMin( 1 );
            mpNfCompression->SetMax( 100 );
            mpNfCompression->SetValue( nQuality );
            mpNfCompression->SetStrictFormat( sal_True );
        }
        break;
        case FORMAT_PNG :
        {
            // Compression 1..9
            mpPNGCompression->Show();
            sal_Int32 nCompression = mpFilterOptionsItem->ReadInt32(OUString("Compression"), 6);
            if ( ( nCompression < 1 ) || ( nCompression > 9 ) )
                nCompression = 6;

            get(mpSbCompression, "compressionpngsb");
            get(mpNfCompression, "compressionpngnf-nospin");
            mpSbCompression->SetRangeMin( 1 );
            mpSbCompression->SetRangeMax( 9 );
            mpNfCompression->SetMin( 1 );
            mpNfCompression->SetMax( 9 );
            mpNfCompression->SetValue( 9 );
            mpNfCompression->SetStrictFormat( sal_True );

            // Interlaced
            mpMode->Show();
            mpCbInterlaced->Check(mpFilterOptionsItem->ReadInt32(OUString("Interlaced"), 0) != 0);

            // Transparency
            mpDrawingObjects->Show();
            mpCbSaveTransparency->Check(mpFilterOptionsItem->ReadInt32(OUString("Translucent"), 1) != 0);
        }
        break;
        case FORMAT_BMP :
        {
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32(OUString("Color"), 0);
            if ( nColor == 0 )
                nColor = 6;
            else
                nColor--;
            mpLbColorDepth->InsertEntry( ms1BitTreshold );
            mpLbColorDepth->InsertEntry( ms1BitDithered );
            mpLbColorDepth->InsertEntry( ms4BitGrayscale );
            mpLbColorDepth->InsertEntry( ms4BitColorPalette );
            mpLbColorDepth->InsertEntry( ms8BitGrayscale );
            mpLbColorDepth->InsertEntry( ms8BitColorPalette );
            mpLbColorDepth->InsertEntry( ms24BitColor );
            mpLbColorDepth->SelectEntryPos( nColor );
            mpColorDepth->Show();

            // RLE coding
            mpBMPCompression->Show();
            mpCbRLEEncoding->Check(mpFilterOptionsItem->ReadBool(OUString("RLE_Coding"), sal_True));
        }
        break;
        case FORMAT_GIF :
        {
            // Interlaced
            mpMode->Show();
            mpCbInterlaced->Check(mpFilterOptionsItem->ReadInt32(OUString("Interlaced"), 1) != 0);

            // Transparency
            mpDrawingObjects->Show();
            mpCbSaveTransparency->Check(mpFilterOptionsItem->ReadInt32(OUString("Translucent"), 1) != 0);
        }
        break;
        case FORMAT_PBM :
        case FORMAT_PGM :
        case FORMAT_PPM :
        {
            // RB Binary / Text
            mpEncoding->Show();
            sal_Int32 nFormat = mpFilterOptionsItem->ReadInt32(OUString("FileFormat"), 1);
            mpRbBinary->Check( nFormat == 0 );
            mpRbText->Check( nFormat != 0 );
        }
        break;
        case FORMAT_EPS :
        {
            mpEPSGrid->Show();

            sal_Int32 nPreview = mpFilterOptionsItem->ReadInt32(OUString("Preview"), 0);
            sal_Int32 nVersion = mpFilterOptionsItem->ReadInt32(OUString("Version"), 2);
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32(OUString("ColorFormat"), 0);
            sal_Int32 nCompr = mpFilterOptionsItem->ReadInt32(OUString("CompressionMode"), 2);

            mpFilterOptionsItem->ReadInt32(OUString("TextMode"), 0);

            mpCbEPSPreviewTIFF->Check( ( nPreview & 1 ) != 0 );
            mpCbEPSPreviewEPSI->Check( ( nPreview & 2 ) != 0 );

            mpRbEPSLevel1->Check( nVersion == 1 );
            mpRbEPSLevel2->Check( nVersion == 2 );

            mpRbEPSColorFormat1->Check( nColor == 1 );
            mpRbEPSColorFormat2->Check( nColor != 1 );

            mpRbEPSCompressionLZW->Check( nCompr == 1 );
            mpRbEPSCompressionNone->Check( nCompr != 1 );
        }
        break;
    }
}

void ExportDialog::setupControls()
{
    setupSizeControls();
    createFilterOptions();

    if (mnMaxFilesizeForRealtimePreview || mbIsPixelFormat)
        mpInfo->Show();
}

static OUString ImpValueOfInKB( const sal_Int64& rVal )
{
    double fVal( static_cast<double>( rVal ) );
    fVal /= ( 1 << 10 );
    fVal += 0.05;
    OUStringBuffer aVal( OUString::number( fVal ) );
    sal_Int32 nX( OUString( aVal.getStr() ).indexOf( '.', 0 ) );
    if ( nX > 0 )
        aVal.setLength( nX + 2 );
    return aVal.makeStringAndClear();
}

void ExportDialog::updateControls()
{
    GetGraphicStream();

    // Size Controls
    if ( !mbIsPixelFormat )
    {
        awt::Size aSize100thmm( maSize );
        Size aSize( LogicToLogic( Size( aSize100thmm.Width * 100, aSize100thmm.Height * 100 ), MAP_100TH_MM,
            MapMode( GetMapUnit( mpLbSizeX->GetSelectEntryPos() ) ) ) );
        mpMfSizeX->SetValue( aSize.Width() );
        mpMfSizeY->SetValue( aSize.Height() );
    }
    else
    {
        MapUnit aMapUnit( GetMapUnit( mpLbSizeX->GetSelectEntryPos() ) );
        if ( aMapUnit == MAP_PIXEL )
        {   // calculating pixel count via resolution and original graphic size
            mpMfSizeX->SetDecimalDigits( 0 );
            mpMfSizeY->SetDecimalDigits( 0 );
            mpMfSizeX->SetValue( maSize.Width );
            mpMfSizeY->SetValue( maSize.Height );
        }
        else
        {
            mpMfSizeX->SetDecimalDigits( 2 );
            mpMfSizeY->SetDecimalDigits( 2 );
            double fRatio;
            switch( GetMapUnit( mpLbSizeX->GetSelectEntryPos() ) )
            {
                case MAP_INCH : fRatio = static_cast< double >( maResolution.Width ) * 0.0254; break;
                case MAP_MM :   fRatio = static_cast< double >( maResolution.Width ) * 0.001; break;
                case MAP_POINT :fRatio = ( static_cast< double >( maResolution.Width ) * 0.0254 ) / 72.0; break;
                default:
                case MAP_CM :   fRatio = static_cast< double >( maResolution.Width ) * 0.01; break;
            }
            mpMfSizeX->SetValue( static_cast< sal_Int32 >( ( static_cast< double >( maSize.Width * 100 ) / fRatio ) + 0.5 ) );
            mpMfSizeY->SetValue( static_cast< sal_Int32 >( ( static_cast< double >( maSize.Height * 100 ) / fRatio ) + 0.5 ) );
        }
    }
    sal_Int32 nResolution = 0;
    switch( mpLbResolution->GetSelectEntryPos() )
    {
        case 0 : nResolution = maResolution.Width / 100; break;     // pixels / cm
        case 2 : nResolution = maResolution.Width; break;           // pixels / meter
        default:
        case 1 : nResolution = static_cast< sal_Int32 >(maResolution.Width * 0.0254); break;    // pixels / inch
    }
    mpNfResolution->SetValue( nResolution );

    if (mpSbCompression && mpSbCompression->IsVisible() && mpNfCompression)
        mpSbCompression->SetThumbPos(mpNfCompression->GetValue());

    // updating estimated size
    sal_Int64 nRealFileSize( mpTempStream->Tell() );
    if ( mbIsPixelFormat )
    {
        String aEst( nRealFileSize ? msEstimatedSizePix2 : msEstimatedSizePix1 );
        sal_Int64 nRawFileSize( GetRawFileSize() );
        xub_StrLen nInd = aEst.Search( '%' );
        aEst.Replace( nInd, 2, ImpValueOfInKB( nRawFileSize ) );

        if ( nRealFileSize )
        {
            nInd = aEst.Search( '%', nInd );
            aEst.Replace( nInd, 2, ImpValueOfInKB( nRealFileSize ) );
        }
        mpFtEstimatedSize->SetText( aEst );
    }
    else
    {
        if ( mnMaxFilesizeForRealtimePreview )
        {
            String aEst( msEstimatedSizeVec );
            xub_StrLen nInd = aEst.Search( '%', 0 );
            aEst.Replace( nInd, 2, ImpValueOfInKB( nRealFileSize ) );
            mpFtEstimatedSize->SetText( aEst );
        }
    }

    // EPS
    if ( mpRbEPSLevel1->IsVisible() )
    {
        sal_Bool bEnabled = mpRbEPSLevel1->IsChecked() == sal_False;
        mpRbEPSColorFormat1->Enable( bEnabled );
        mpRbEPSColorFormat2->Enable( bEnabled );
        mpRbEPSCompressionLZW->Enable( bEnabled );
        mpRbEPSCompressionNone->Enable( bEnabled );
    }
}

ExportDialog::~ExportDialog()
{
    delete mpFilterOptionsItem;
    delete mpOptionsItem;
}


/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/
IMPL_LINK_NOARG(ExportDialog, UpdateHdl)
{
    updateControls();
    return 0;
}

IMPL_LINK_NOARG(ExportDialog, UpdateHdlMtfSizeX)
{
    double fRatio = static_cast< double >( maOriginalSize.Height ) / maOriginalSize.Width;

    if ( mbIsPixelFormat )
    {
        switch( GetMapUnit( mpLbSizeX->GetSelectEntryPos() ) )
        {
            case MAP_INCH :     maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.0254 * mpMfSizeX->GetValue() / 100.0 + 0.5 ); break;
            case MAP_CM :       maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.01 * mpMfSizeX->GetValue() / 100.0 + 0.5 ); break;
            case MAP_MM :       maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.001 * mpMfSizeX->GetValue() / 100.0 + 0.5 ); break;
            case MAP_POINT :    maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.0254 * mpMfSizeX->GetValue() / 100.0 * 72 + 0.5 ); break;
            default:
            case MAP_PIXEL :    maSize.Width = mpMfSizeX->GetValue(); break;
        }
        maSize.Height = static_cast< sal_Int32 >( fRatio * maSize.Width + 0.5 );
    }
    else
    {
        Fraction aFract( 1, 100 );
        sal_Int32 nWidth = mpMfSizeX->GetValue();
        sal_Int32 nHeight= static_cast< sal_Int32 >( nWidth * fRatio );
        const Size aSource( static_cast< sal_Int32 >( nWidth ), static_cast< sal_Int32 >( nHeight ) );
        MapMode aSourceMapMode( GetMapUnit( mpLbSizeX->GetSelectEntryPos() ),Point(), aFract, aFract );
        Size aDest( LogicToLogic( aSource, aSourceMapMode, MAP_100TH_MM ) );

        maSize.Width = aDest.Width();
        if ( mbPreserveAspectRatio )
            maSize.Height = aDest.Height();
    }
    updateControls();
    return 0;
}

IMPL_LINK_NOARG(ExportDialog, UpdateHdlMtfSizeY)
{
    double fRatio = static_cast< double >( maOriginalSize.Width ) / maOriginalSize.Height;

    if ( mbIsPixelFormat )
    {
        switch( GetMapUnit( mpLbSizeX->GetSelectEntryPos() ) )
        {
            case MAP_INCH :     maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.0254 * mpMfSizeY->GetValue() / 100.0 + 0.5 ); break;
            case MAP_CM :       maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.01 * mpMfSizeY->GetValue() / 100.0 + 0.5 ); break;
            case MAP_MM :       maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.001 * mpMfSizeY->GetValue() / 100.0 + 0.5 ); break;
            case MAP_POINT :    maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.0254 * mpMfSizeY->GetValue() / 100.0 * 72 + 0.5 ); break;
            default:
            case MAP_PIXEL :    maSize.Height = mpMfSizeY->GetValue(); break;
        }
        maSize.Width = static_cast< sal_Int32 >( fRatio * maSize.Height + 0.5 );
    }
    else
    {
        Fraction aFract( 1, 100 );
        sal_Int32 nHeight= mpMfSizeY->GetValue();
        sal_Int32 nWidth = static_cast< sal_Int32 >( nHeight * fRatio );
        const Size aSource( static_cast< sal_Int32 >( nWidth ), static_cast< sal_Int32 >( nHeight ) );
        MapMode aSourceMapMode( GetMapUnit( mpLbSizeX->GetSelectEntryPos() ),Point(), aFract, aFract );
        Size aDest( LogicToLogic( aSource, aSourceMapMode, MAP_100TH_MM ) );

        maSize.Height = aDest.Height();
        if ( mbPreserveAspectRatio )
            maSize.Width = aDest.Width();
    }
    updateControls();
    return 0;
}

IMPL_LINK_NOARG(ExportDialog, UpdateHdlNfResolution)
{
    sal_Int32 nResolution = mpNfResolution->GetValue();
    if ( mpLbResolution->GetSelectEntryPos() == 0 )      // pixels / cm
        nResolution *= 100;
    else if ( mpLbResolution->GetSelectEntryPos() == 1 ) // pixels / inch
        nResolution = static_cast< sal_Int32 >( ( ( static_cast< double >( nResolution ) + 0.5 ) / 0.0254 ) );
    maResolution.Width = nResolution;
    maResolution.Height= nResolution;

    updateControls();
    return 0;
}

IMPL_LINK_NOARG(ExportDialog, SbCompressionUpdateHdl)
{
    mpNfCompression->SetValue( mpSbCompression->GetThumbPos() );
    updateControls();
    return 0;
}

IMPL_LINK_NOARG(ExportDialog, OK)
{
    // writing config parameter


    mrFltCallPara.aFilterData = GetFilterData( sal_True );
    EndDialog( RET_OK );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
