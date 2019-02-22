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


#include <tools/stream.hxx>
#include <tools/fract.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>
#include <svtools/DocumentToGraphicRenderer.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
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
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graph.hxx>
#include <rtl/ustrbuf.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include "exportdialog.hxx"

#define FORMAT_UNKNOWN  0
#define FORMAT_JPG      1
#define FORMAT_PNG      2
#define FORMAT_BMP      3
#define FORMAT_GIF      4
#define FORMAT_TIF      10
#define FORMAT_WMF      12
#define FORMAT_EMF      13
#define FORMAT_EPS      14
#define FORMAT_SVG      16

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
    else if ( rExt == "TIF" )
        nFormat = FORMAT_TIF;
    else if ( rExt == "WMF" )
        nFormat = FORMAT_WMF;
    else if ( rExt == "EMF" )
        nFormat = FORMAT_EMF;
    else if ( rExt == "EPS" )
        nFormat = FORMAT_EPS;
    else if ( rExt == "SVG" )
        nFormat = FORMAT_SVG;
    return nFormat;
}

static MapUnit GetMapUnit( sal_Int32 nUnit )
{
    MapUnit aMapUnit( MapUnit::MapPixel );
    switch( nUnit )
    {
        case UNIT_INCH  :   aMapUnit = MapUnit::MapInch; break;
        case UNIT_CM    :   aMapUnit = MapUnit::MapCM; break;
        case UNIT_MM    :   aMapUnit = MapUnit::MapMM; break;
        case UNIT_POINT :   aMapUnit = MapUnit::MapPoint; break;
        case UNIT_PIXEL :   aMapUnit = MapUnit::MapPixel; break;
    }
    return aMapUnit;
}

sal_Int32 ExportDialog::GetDefaultUnit()
{
    sal_Int32 nDefaultUnit = UNIT_CM;
    switch( mrFltCallPara.eFieldUnit )
    {
//      case FieldUnit::NONE :
//      case FieldUnit::PERCENT :
//      case FieldUnit::CUSTOM :
        default:                nDefaultUnit = UNIT_CM; break;

        case FieldUnit::MILE :       // PASSTHROUGH INTENDED
        case FieldUnit::FOOT :
        case FieldUnit::TWIP :
        case FieldUnit::PICA :       nDefaultUnit = UNIT_INCH; break;

        case FieldUnit::KM :         // PASSTHROUGH INTENDED
        case FieldUnit::M :
        case FieldUnit::MM_100TH :   nDefaultUnit = UNIT_CM; break;

        case FieldUnit::INCH :       nDefaultUnit = UNIT_INCH; break;
        case FieldUnit::CM :         nDefaultUnit = UNIT_CM; break;
        case FieldUnit::MM :         nDefaultUnit = UNIT_MM; break;
        case FieldUnit::POINT :      nDefaultUnit = UNIT_POINT; break;
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

uno::Sequence< beans::PropertyValue > ExportDialog::GetFilterData( bool bUpdateConfig )
{
    if ( bUpdateConfig )
    {
        sal_Int32 nUnit = mxLbSizeX->get_active();
        if ( nUnit < 0 )
            nUnit = UNIT_CM;

        if ( ( mnInitialResolutionUnit == UNIT_DEFAULT ) && ( nUnit == GetDefaultUnit() ) )
            nUnit = UNIT_DEFAULT;

        // updating ui configuration
        if ( mbIsPixelFormat )
        {
            if ( nUnit > UNIT_MAX_ID )
                nUnit = UNIT_PIXEL;

            sal_Int32 nResolution = mxNfResolution->get_value();
            if ( nResolution < 1 )
                nResolution = 96;

            mpOptionsItem->WriteInt32("PixelExportUnit", nUnit);
            mpOptionsItem->WriteInt32("PixelExportResolution", nResolution);
            mpOptionsItem->WriteInt32("PixelExportResolutionUnit", mxLbResolution->get_active());
        }
        else
        {
            if ( nUnit >= UNIT_PIXEL )
                nUnit = UNIT_CM;

            mpOptionsItem->WriteInt32("VectorExportUnit", nUnit);
        }
    }

    FilterConfigItem* pFilterOptions;
    if ( bUpdateConfig )
         pFilterOptions = mpFilterOptionsItem.get();
    else
    {
        uno::Sequence< beans::PropertyValue > aFilterData( mpFilterOptionsItem->GetFilterData() );
        pFilterOptions = new FilterConfigItem( &aFilterData );
    }

    const OUString sLogicalWidth("LogicalWidth");
    const OUString sLogicalHeight("LogicalHeight");
    if ( mbIsPixelFormat )
    {
        pFilterOptions->WriteInt32("PixelWidth", static_cast< sal_Int32 >( maSize.Width ) );
        pFilterOptions->WriteInt32("PixelHeight", static_cast< sal_Int32 >( maSize.Height ) );
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
            sal_Int32 nColor = mxLbColorDepth->get_active();
            if ( nColor == 1 )
                nColor = 0;
            else
                nColor = 1;
            pFilterOptions->WriteInt32("ColorMode", nColor);
            assert(mpSbCompression);
            pFilterOptions->WriteInt32("Quality", static_cast<sal_Int32>(mpSbCompression->get_value()));
        }
        break;

        case FORMAT_PNG :
        {
            assert(mpSbCompression);
            pFilterOptions->WriteInt32("Compression", static_cast<sal_Int32>(mpSbCompression->get_value()));
            sal_Int32 nInterlace = 0;
            if ( mxCbInterlaced->get_active() )
                nInterlace++;
            pFilterOptions->WriteInt32("Interlaced", nInterlace);
            sal_Int32 nValue = 0;
            if ( mxCbSaveTransparency->get_active() )
                nValue++;
            pFilterOptions->WriteInt32("Translucent", nValue);
        }
        break;

        case FORMAT_BMP :
        {
            pFilterOptions->WriteInt32("Color", mxLbColorDepth->get_active() + 1);
            pFilterOptions->WriteBool("RLE_Coding", mxCbRLEEncoding->get_active());
        }
        break;

        case FORMAT_GIF :
        {
            sal_Int32 nValue = 0;
            if ( mxCbInterlaced->get_active() )
                nValue++;
            pFilterOptions->WriteInt32("Interlaced", nValue);

            nValue = 0;
            if (mxCbSaveTransparency->get_active())
                nValue++;
            pFilterOptions->WriteInt32("Translucent", nValue);
        }
        break;

        case FORMAT_EPS :
        {
            sal_Int32 nCheck = 0;
            if ( mxCbEPSPreviewTIFF->get_active() )
                nCheck++;
            if ( mxCbEPSPreviewEPSI->get_active() )
                nCheck += 2;
            pFilterOptions->WriteInt32("Preview", nCheck);

            nCheck = 1;
            if ( mxRbEPSLevel2->get_active() )
                nCheck++;
            pFilterOptions->WriteInt32("Version", nCheck);

            nCheck = 1;
            if ( mxRbEPSColorFormat2->get_active() )
                nCheck++;
            pFilterOptions->WriteInt32("ColorFormat", nCheck);

            nCheck = 1;
            if ( mxRbEPSCompressionNone->get_active() )
                nCheck++;
            pFilterOptions->WriteInt32("CompressionMode", nCheck);
        }
        break;
    }

    uno::Sequence< beans::PropertyValue > aRet( pFilterOptions->GetFilterData() );
    if ( !bUpdateConfig )
        delete pFilterOptions;
    return aRet;
}


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
            css::uno::Any aAny;
            aAny = xPagePropSet->getPropertyValue("Width");
            aAny >>= nWidth;
            aAny = xPagePropSet->getPropertyValue("Height");
            aAny >>= nHeight;
            aShapesRange = basegfx::B2DRange( 0, 0, nWidth, nHeight );
        }
    }
    else if (mxShapes.is() || mxShape.is())
    {
        uno::Reference< graphic::XPrimitiveFactory2D > xPrimitiveFactory = graphic::PrimitiveFactory2D::create( mxContext );

        basegfx::B2DHomMatrix aViewTransformation( Application::GetDefaultDevice()->GetViewTransformation() );
        css::geometry::AffineMatrix2D aTransformation;
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
    else if (!mbGraphicsSource)
    {
        DocumentToGraphicRenderer aRenderer( mxSourceDocument, mbExportSelection);
        const sal_Int32 nCurrentPage = aRenderer.getCurrentPage();
        const Size aSize = aRenderer.getDocumentSizeIn100mm( nCurrentPage);
        return awt::Size( aSize.Width(), aSize.Height());
    }
    return awt::Size( static_cast<sal_Int32>(aShapesRange.getWidth()), static_cast<sal_Int32>(aShapesRange.getHeight()) );
}

void ExportDialog::GetGraphicSource()
{
    if (mxGraphic.is())
        return;

    if ( !mxSourceDocument.is() )
        return;

    uno::Reference< frame::XModel > xModel( mxSourceDocument, uno::UNO_QUERY );
    if ( !xModel.is() )
        return;

    uno::Reference< frame::XController > xController( xModel->getCurrentController() );
    if ( !xController.is() )
        return;

    if ( mbExportSelection )                // check if there is a selection
    {
        if (DocumentToGraphicRenderer::isShapeSelected( mxShapes, mxShape, xController))
            mbGraphicsSource = true;
    }
    if ( !mxShape.is() && !mxShapes.is() && mbGraphicsSource )
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
    // For !mbGraphicsSource the mxSourceDocument is used, from
    // which XRenderable can query XController and
    // XSelectionSupplier the same.
}

void ExportDialog::GetGraphicStream()
{
    if ( !IsTempExportAvailable() )
    {
        mpTempStream.reset(new SvMemoryStream());
        return;
    }

    bool bRecreateOutputStream = mpTempStream->Tell() == 0;

    static uno::Sequence< beans::PropertyValue > aOldFilterData;
    uno::Sequence< beans::PropertyValue > aNewFilterData( GetFilterData( false ) );
    if ( aOldFilterData != aNewFilterData )
    {
        aOldFilterData = aNewFilterData;
        bRecreateOutputStream = true;
    }
    try
    {
        if ( bRecreateOutputStream )
        {
            mpTempStream.reset(new SvMemoryStream());

            uno::Reference< graphic::XGraphic > xGraphic;
            if (!mbGraphicsSource && !mxGraphic.is())
            {
                // Create a Graphic to be used below.
                DocumentToGraphicRenderer aRenderer( mxSourceDocument, mbExportSelection);
                const sal_Int32 nCurrentPage = aRenderer.getCurrentPage();
                const Size aDocumentSizePixel = aRenderer.getDocumentSizeInPixels( nCurrentPage);

                const Size aTargetSizePixel( mbIsPixelFormat ?
                        Size( maSize.Width, maSize.Height) :
                        aDocumentSizePixel );

                Graphic aGraphic( aRenderer.renderToGraphic( nCurrentPage,
                            aDocumentSizePixel, aTargetSizePixel, COL_WHITE, /*bExtOutDevData=*/false));
                xGraphic = aGraphic.GetXGraphic();
            }

            if ( mxGraphic.is() || xGraphic.is() )
            {
                Graphic aGraphic( mxGraphic.is() ? mxGraphic : xGraphic );

                if ( aGraphic.GetType() == GraphicType::Bitmap )
                {
                    Size aSizePixel( aGraphic.GetSizePixel() );
                    if( maSize.Width && maSize.Height &&
                        ( ( maSize.Width != aSizePixel.Width() ) ||
                          ( maSize.Height != aSizePixel.Height() ) ) )
                    {
                        BitmapEx aBmpEx( aGraphic.GetBitmapEx() );
                        // export: use highest quality
                        aBmpEx.Scale( Size( maSize.Width, maSize.Height ), BmpScaleFlag::Lanczos );
                        aGraphic = aBmpEx;
                    }
                }

                GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
                const sal_uInt16 nFilter = rFilter.GetExportFormatNumberForShortName( maExt );
                if ( rFilter.IsExportPixelFormat( nFilter ) )
                {
                    mpTempStream->SetResizeOffset(1024);
                    mpTempStream->SetStreamSize(1024);
                    rFilter.ExportGraphic( aGraphic, "", *mpTempStream, nFilter, &aNewFilterData );
                }
            }
            else
            {
                uno::Reference< lang::XComponent > xSourceDoc;
                if ( mxPage.is() )
                    xSourceDoc.set( mxPage, uno::UNO_QUERY_THROW );
                else if ( mxShapes.is() )
                    xSourceDoc.set( mxShapes, uno::UNO_QUERY_THROW );
                else if ( mxShape.is() )
                    xSourceDoc.set( mxShape, uno::UNO_QUERY_THROW );
                if ( xSourceDoc.is() )
                {
                    uno::Reference < io::XStream > xStream( new utl::OStreamWrapper( *mpTempStream ) );
                    uno::Reference < io::XOutputStream > xOutputStream( xStream->getOutputStream() );

                    OUString sFormat( maExt );
                    uno::Sequence< beans::PropertyValue > aDescriptor( 3 );
                    aDescriptor[0].Name = "OutputStream";
                    aDescriptor[0].Value <<= xOutputStream;
                    aDescriptor[1].Name = "FilterName";
                    aDescriptor[1].Value <<= sFormat;
                    aDescriptor[2].Name = "FilterData";
                    aDescriptor[2].Value <<= aNewFilterData;

                    uno::Reference< drawing::XGraphicExportFilter > xGraphicExporter =
                        drawing::GraphicExportFilter::create( mxContext );

                    xGraphicExporter->setSourceDocument( xSourceDoc );
                    xGraphicExporter->filter( aDescriptor );
                }
            }
        }
    }
    catch( uno::Exception& )
    {

        // ups

    }
}

sal_uInt32 ExportDialog::GetRawFileSize() const
{
    sal_uInt64 nRawFileSize = 0;
    if ( mbIsPixelFormat )
    {
        sal_Int32 nBitsPerPixel = 24;
        OUString aEntry(mxLbColorDepth->get_active_text());
        if ( ms1BitThreshold == aEntry )
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
bool ExportDialog::IsTempExportAvailable() const
{
    return GetRawFileSize() < static_cast< sal_uInt32 >( mnMaxFilesizeForRealtimePreview );
}

ExportDialog::ExportDialog(FltCallDialogParameter& rPara,
    const css::uno::Reference< css::uno::XComponentContext >& rxContext,
    const css::uno::Reference< css::lang::XComponent >& rxSourceDocument,
    bool bExportSelection, bool bIsPixelFormat, bool bGraphicsSource,
    const css::uno::Reference< css::graphic::XGraphic >& rxGraphic)
    : GenericDialogController(rPara.pWindow, "svt/ui/graphicexport.ui", "GraphicExportDialog")
    , mrFltCallPara(rPara)
    , mxContext(rxContext)
    , mxSourceDocument(rxSourceDocument)
    , mxGraphic(rxGraphic)
    , msEstimatedSizePix1(SvtResId(STR_SVT_ESTIMATED_SIZE_PIX_1))
    , msEstimatedSizePix2(SvtResId(STR_SVT_ESTIMATED_SIZE_PIX_2))
    , msEstimatedSizeVec(SvtResId(STR_SVT_ESTIMATED_SIZE_VEC))
    , ms1BitThreshold(SvtResId(STR_SVT_1BIT_THRESHOLD))
    , ms1BitDithered(SvtResId(STR_SVT_1BIT_DITHERED))
    , ms4BitGrayscale(SvtResId(STR_SVT_4BIT_GRAYSCALE))
    , ms4BitColorPalette(SvtResId(STR_SVT_4BIT_COLOR_PALETTE))
    , ms8BitGrayscale(SvtResId(STR_SVT_8BIT_GRAYSCALE))
    , ms8BitColorPalette(SvtResId(STR_SVT_8BIT_COLOR_PALETTE))
    , ms24BitColor(SvtResId(STR_SVT_24BIT_TRUE_COLOR))
    , maExt(rPara.aFilterExt)
    , mnFormat(FORMAT_UNKNOWN)
    , mnMaxFilesizeForRealtimePreview(0)
    , mpTempStream(new SvMemoryStream())
    , maOriginalSize(awt::Size(0, 0))
    , mbIsPixelFormat(bIsPixelFormat)
    , mbExportSelection(bExportSelection)
    , mbGraphicsSource(bGraphicsSource)
    , mpSbCompression(nullptr)
    , mpNfCompression(nullptr)
    , mxMfSizeX(m_xBuilder->weld_spin_button("widthmf"))
    , mxLbSizeX(m_xBuilder->weld_combo_box("widthlb"))
    , mxMfSizeY(m_xBuilder->weld_spin_button( "heightmf"))
    , mxFtResolution(m_xBuilder->weld_label("resolutionft"))
    , mxNfResolution(m_xBuilder->weld_spin_button("resolutionmf"))
    , mxLbResolution(m_xBuilder->weld_combo_box("resolutionlb"))
    , mxColorDepth(m_xBuilder->weld_widget("colordepth"))
    , mxLbColorDepth(m_xBuilder->weld_combo_box("colordepthlb"))
    , mxJPGQuality(m_xBuilder->weld_widget("jpgquality"))
    , mxPNGCompression(m_xBuilder->weld_widget("pngcompression"))
    , mxSbPngCompression(m_xBuilder->weld_scale("compressionpngsb"))
    , mxNfPngCompression(m_xBuilder->weld_spin_button("compressionpngnf"))
    , mxSbJpgCompression(m_xBuilder->weld_scale("compressionjpgsb"))
    , mxNfJpgCompression(m_xBuilder->weld_spin_button("compressionjpgnf"))
    , mxMode(m_xBuilder->weld_widget("mode"))
    , mxCbInterlaced(m_xBuilder->weld_check_button("interlacedcb"))
    , mxBMPCompression(m_xBuilder->weld_widget("bmpcompression"))
    , mxCbRLEEncoding(m_xBuilder->weld_check_button("rlecb"))
    , mxDrawingObjects(m_xBuilder->weld_widget("drawingobjects"))
    , mxCbSaveTransparency(m_xBuilder->weld_check_button("savetransparencycb"))
    , mxEncoding(m_xBuilder->weld_widget("encoding"))
    , mxRbBinary(m_xBuilder->weld_radio_button("binarycb"))
    , mxRbText(m_xBuilder->weld_radio_button("textcb"))
    , mxEPSGrid(m_xBuilder->weld_widget("epsgrid"))
    , mxCbEPSPreviewTIFF(m_xBuilder->weld_check_button("tiffpreviewcb"))
    , mxCbEPSPreviewEPSI(m_xBuilder->weld_check_button("epsipreviewcb"))
    , mxRbEPSLevel1(m_xBuilder->weld_radio_button("level1rb"))
    , mxRbEPSLevel2(m_xBuilder->weld_radio_button("level2rb"))
    , mxRbEPSColorFormat1(m_xBuilder->weld_radio_button("color1rb"))
    , mxRbEPSColorFormat2(m_xBuilder->weld_radio_button("color2rb"))
    , mxRbEPSCompressionLZW(m_xBuilder->weld_radio_button("compresslzw"))
    , mxRbEPSCompressionNone(m_xBuilder->weld_radio_button("compressnone"))
    , mxInfo(m_xBuilder->weld_widget("information"))
    , mxFtEstimatedSize(m_xBuilder->weld_label("estsizeft"))
    , mxBtnOK(m_xBuilder->weld_button("ok"))
{
    GetGraphicSource();

    maExt = maExt.toAsciiUpperCase();

    OUString  aFilterConfigPath( "Office.Common/Filter/Graphic/Export/" );
    mpOptionsItem.reset(new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData ));
    aFilterConfigPath += maExt;
    mpFilterOptionsItem.reset(new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData ));

    mnInitialResolutionUnit = mbIsPixelFormat
        ? mpOptionsItem->ReadInt32("PixelExportUnit", UNIT_DEFAULT)
        : mpOptionsItem->ReadInt32("VectorExportUnit", UNIT_DEFAULT);

    mnMaxFilesizeForRealtimePreview = mpOptionsItem->ReadInt32("MaxFilesizeForRealtimePreview", 0);
    mxFtEstimatedSize->set_label(" \n ");

    m_xDialog->set_title(m_xDialog->get_title().replaceFirst("%1", maExt)); //Set dialog title

    mnFormat = GetFilterFormat( maExt );

    Size aResolution( Application::GetDefaultDevice()->LogicToPixel(Size(100, 100), MapMode(MapUnit::MapCM)) );
    maResolution.Width = aResolution.Width();
    maResolution.Height= aResolution.Height();

    if ( mxGraphic.is() )
    {
        Graphic aGraphic(mxGraphic);
        Size aSize = aGraphic.GetSizePixel();
        maSize = awt::Size(aSize.getWidth(), aSize.getHeight());
        double f100thmmPerPixel = 100000.0 / static_cast< double >( maResolution.Width );
        maOriginalSize = awt::Size(
                static_cast< sal_Int32 >( f100thmmPerPixel * maSize.Width ),
                static_cast< sal_Int32 >( f100thmmPerPixel * maSize.Height ) );
    }
    else
    {
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
    }
    setupControls();

    // Size
    mxLbSizeX->connect_changed( LINK( this, ExportDialog, SelectListBoxHdl ) );

    if (mpSbCompression)
        mpSbCompression->connect_value_changed(LINK(this, ExportDialog, SbCompressionUpdateHdl));
    if (mpNfCompression)
        mpNfCompression->connect_value_changed(LINK(this, ExportDialog, SelectHdl));

    mxMfSizeX->connect_value_changed( LINK( this, ExportDialog, UpdateHdlMtfSizeX ) );
    mxMfSizeY->connect_value_changed( LINK( this, ExportDialog, UpdateHdlMtfSizeY ) );

    mxNfResolution->connect_value_changed( LINK( this, ExportDialog, UpdateHdlNfResolution ) );
    mxLbResolution->connect_changed( LINK( this, ExportDialog, SelectListBoxHdl ) );

    mxLbColorDepth->connect_changed( LINK( this, ExportDialog, SelectListBoxHdl ) );

    mxCbInterlaced->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );

    mxCbSaveTransparency->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );

    mxCbEPSPreviewTIFF->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );
    mxCbEPSPreviewEPSI->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );

    mxRbEPSCompressionLZW->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );
    mxRbEPSCompressionNone->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );

    mxRbBinary->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );
    mxRbText->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );

    // BMP
    mxCbRLEEncoding->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );

    // EPS
    mxRbEPSLevel1->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );
    mxRbEPSLevel2->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );

    mxBtnOK->connect_clicked( LINK( this, ExportDialog, OK ) );

    updateControls();
}

void ExportDialog::setupSizeControls()
{
    sal_Int32 nUnit = mnInitialResolutionUnit;
    if (nUnit == UNIT_DEFAULT)
        nUnit = GetDefaultUnit();

    if (!mbIsPixelFormat)
    {
        mxFtResolution->hide();
        mxNfResolution->hide();
        mxLbResolution->hide();
        mxLbSizeX->remove( UNIT_PIXEL );        // removing pixel
        if ( nUnit >= UNIT_PIXEL )
            nUnit = UNIT_CM;
    }
    else if ( nUnit > UNIT_MAX_ID )
        nUnit = UNIT_PIXEL;
    if ( nUnit < 0 )
        nUnit = UNIT_CM;
    mxLbSizeX->set_active( static_cast< sal_uInt16 >( nUnit ) );

    if ( !mbIsPixelFormat )      // TODO: (metafileresolutionsupport) should be supported for vector formats also... this makes
return;

// sense eg for bitmap fillings in metafiles, to preserve high dpi output
                            // (atm without special vector support the bitmaps are rendered with 96dpi)
    sal_Int32 nResolution = mpOptionsItem->ReadInt32("PixelExportResolution", 96);
    if ( nResolution < 1 )
        nResolution = 96;
    mxNfResolution->set_value( nResolution );

    sal_Int32 nResolutionUnit = mpOptionsItem->ReadInt32("PixelExportResolutionUnit", 1);
    if ( ( nResolutionUnit < 0 ) || ( nResolutionUnit > 2 ) )
        nResolutionUnit = 1;
    mxLbResolution->set_active( static_cast< sal_uInt16 >( nResolutionUnit ) );
}

void ExportDialog::createFilterOptions()
{
    switch( mnFormat )
    {
        case FORMAT_JPG :
        {
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32("ColorMode", 0);
            if ( nColor == 1 )
                nColor = 0;
            else
                nColor = 1;
            mxLbColorDepth->append_text( ms8BitGrayscale );
            mxLbColorDepth->append_text( ms24BitColor );
            mxLbColorDepth->set_active( nColor );
            mxColorDepth->show();

            // Quality
            mxJPGQuality->show();
            sal_Int32 nQuality = mpFilterOptionsItem->ReadInt32("Quality", 75);
            if ((nQuality < 1 ) || (nQuality > 100))
                nQuality = 75;
            mpSbCompression = mxSbJpgCompression.get();
            mpNfCompression = mxNfJpgCompression.get();
            mpSbCompression->set_range(1, 100);
            mpNfCompression->set_range(1, 100);
            mpNfCompression->set_value(nQuality);
        }
        break;
        case FORMAT_PNG :
        {
            // Compression 1..9
            mxPNGCompression->show();
            sal_Int32 nCompression = mpFilterOptionsItem->ReadInt32("Compression", 6);
            if ( ( nCompression < 1 ) || ( nCompression > 9 ) )
                nCompression = 6;

            mpSbCompression = mxSbPngCompression.get();
            mpNfCompression = mxNfPngCompression.get();
            mpSbCompression->set_range(1, 9);
            mpNfCompression->set_range(1, 9);
            mpNfCompression->set_value(nCompression);

            // Interlaced
            mxMode->show();
            mxCbInterlaced->set_active(mpFilterOptionsItem->ReadInt32("Interlaced", 0) != 0);

            // Transparency
            mxDrawingObjects->show();
            mxCbSaveTransparency->set_active(mpFilterOptionsItem->ReadInt32("Translucent", 1) != 0);
        }
        break;
        case FORMAT_BMP :
        {
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32("Color", 0);
            if ( nColor == 0 )
                nColor = 6;
            else
                nColor--;
            mxLbColorDepth->append_text( ms1BitThreshold );
            mxLbColorDepth->append_text( ms1BitDithered );
            mxLbColorDepth->append_text( ms4BitGrayscale );
            mxLbColorDepth->append_text( ms4BitColorPalette );
            mxLbColorDepth->append_text( ms8BitGrayscale );
            mxLbColorDepth->append_text( ms8BitColorPalette );
            mxLbColorDepth->append_text( ms24BitColor );
            mxLbColorDepth->set_active( nColor );
            mxColorDepth->show();

            // RLE coding
            mxBMPCompression->show();
            mxCbRLEEncoding->set_active(mpFilterOptionsItem->ReadBool("RLE_Coding", true));
        }
        break;
        case FORMAT_GIF :
        {
            // Interlaced
            mxMode->show();
            mxCbInterlaced->set_active(mpFilterOptionsItem->ReadInt32("Interlaced", 1) != 0);

            // Transparency
            mxDrawingObjects->show();
            mxCbSaveTransparency->set_active(mpFilterOptionsItem->ReadInt32("Translucent", 1) != 0);
        }
        break;
        case FORMAT_EPS :
        {
            mxEPSGrid->show();

            sal_Int32 nPreview = mpFilterOptionsItem->ReadInt32("Preview", 0);
            sal_Int32 nVersion = mpFilterOptionsItem->ReadInt32("Version", 2);
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32("ColorFormat", 0);
            sal_Int32 nCompr = mpFilterOptionsItem->ReadInt32("CompressionMode", 2);

            mpFilterOptionsItem->ReadInt32("TextMode", 0);

            mxCbEPSPreviewTIFF->set_active( ( nPreview & 1 ) != 0 );
            mxCbEPSPreviewEPSI->set_active( ( nPreview & 2 ) != 0 );

            mxRbEPSLevel1->set_active( nVersion == 1 );
            mxRbEPSLevel2->set_active( nVersion == 2 );

            mxRbEPSColorFormat1->set_active( nColor == 1 );
            mxRbEPSColorFormat2->set_active( nColor != 1 );

            mxRbEPSCompressionLZW->set_active( nCompr == 1 );
            mxRbEPSCompressionNone->set_active( nCompr != 1 );
        }
        break;
    }
}

void ExportDialog::setupControls()
{
    setupSizeControls();
    createFilterOptions();

    if (mnMaxFilesizeForRealtimePreview || mbIsPixelFormat)
        mxInfo->show();
}

static OUString ImpValueOfInKB( sal_Int64 rVal )
{
    double fVal( static_cast<double>( rVal ) );
    fVal /= ( 1 << 10 );
    fVal += 0.05;
    OUStringBuffer aVal( OUString::number( fVal ) );
    sal_Int32 nX( OUString( aVal.getStr() ).indexOf( '.' ) );
    if ( nX > 0 )
        aVal.setLength( nX + 2 );
    return aVal.makeStringAndClear();
}

void ExportDialog::updateControls()
{
    // Size Controls
    if ( !mbIsPixelFormat )
    {
        awt::Size aSize100thmm( maSize );
        Size aSize( OutputDevice::LogicToLogic( Size(aSize100thmm.Width * 100, aSize100thmm.Height * 100),
            MapMode(MapUnit::Map100thMM),
            MapMode( GetMapUnit( mxLbSizeX->get_active() ) ) ) );
        mxMfSizeX->set_value( aSize.Width() );
        mxMfSizeY->set_value( aSize.Height() );
    }
    else
    {
        MapUnit aMapUnit( GetMapUnit( mxLbSizeX->get_active() ) );
        if ( aMapUnit == MapUnit::MapPixel )
        {   // calculating pixel count via resolution and original graphic size
            mxMfSizeX->set_digits( 0 );
            mxMfSizeY->set_digits( 0 );
            mxMfSizeX->set_value( maSize.Width );
            mxMfSizeY->set_value( maSize.Height );
        }
        else
        {
            mxMfSizeX->set_digits( 2 );
            mxMfSizeY->set_digits( 2 );
            double fRatio;
            switch( GetMapUnit( mxLbSizeX->get_active() ) )
            {
                case MapUnit::MapInch : fRatio = static_cast< double >( maResolution.Width ) * 0.0254; break;
                case MapUnit::MapMM :   fRatio = static_cast< double >( maResolution.Width ) * 0.001; break;
                case MapUnit::MapPoint :fRatio = ( static_cast< double >( maResolution.Width ) * 0.0254 ) / 72.0; break;
                default:
                case MapUnit::MapCM :   fRatio = static_cast< double >( maResolution.Width ) * 0.01; break;
            }
            mxMfSizeX->set_value( static_cast< sal_Int32 >( ( static_cast< double >( maSize.Width * 100 ) / fRatio ) + 0.5 ) );
            mxMfSizeY->set_value( static_cast< sal_Int32 >( ( static_cast< double >( maSize.Height * 100 ) / fRatio ) + 0.5 ) );
        }
    }
    sal_Int32 nResolution = 0;
    switch( mxLbResolution->get_active() )
    {
        case 0 : nResolution = maResolution.Width / 100; break;     // pixels / cm
        case 2 : nResolution = maResolution.Width; break;           // pixels / meter
        default:
        case 1 : nResolution = static_cast< sal_Int32 >(maResolution.Width * 0.0254); break;    // pixels / inch
    }
    mxNfResolution->set_value( nResolution );

    if (mpSbCompression && mpSbCompression->get_visible() && mpNfCompression)
        mpSbCompression->set_value(mpNfCompression->get_value());

    GetGraphicStream();

    // updating estimated size
    sal_Int64 nRealFileSize( mpTempStream->Tell() );
    if ( mbIsPixelFormat )
    {
        OUString aEst( nRealFileSize ? msEstimatedSizePix2 : msEstimatedSizePix1 );
        sal_Int64 nRawFileSize( GetRawFileSize() );
        sal_Int32 nInd = aEst.indexOf( "%" );
        if (nInd != -1)
            aEst = aEst.replaceAt( nInd, 2, ImpValueOfInKB( nRawFileSize ) );

        if ( nRealFileSize && nInd != -1 )
        {
            nInd = aEst.indexOf( "%", nInd );
            if (nInd != -1)
                aEst = aEst.replaceAt( nInd, 2, ImpValueOfInKB( nRealFileSize ) );
        }
        mxFtEstimatedSize->set_label( aEst );
    }
    else
    {
        if ( mnMaxFilesizeForRealtimePreview )
        {
            OUString aEst( msEstimatedSizeVec );
            sal_Int32 nInd = aEst.indexOf( "%" );
            if (nInd != -1)
                aEst = aEst.replaceAt( nInd, 2, ImpValueOfInKB( nRealFileSize ) );
            mxFtEstimatedSize->set_label( aEst );
        }
    }

    // EPS
    if ( mxRbEPSLevel1->get_visible() )
    {
        bool bEnabled = !mxRbEPSLevel1->get_active();
        mxRbEPSColorFormat1->set_sensitive( bEnabled );
        mxRbEPSColorFormat2->set_sensitive( bEnabled );
        mxRbEPSCompressionLZW->set_sensitive( bEnabled );
        mxRbEPSCompressionNone->set_sensitive( bEnabled );
    }
}

ExportDialog::~ExportDialog()
{
}

/*************************************************************************
|*
|* stores values set in the ini-file
|*
\************************************************************************/
IMPL_LINK_NOARG(ExportDialog, SelectHdl, weld::SpinButton&, void)
{
    updateControls();
}

IMPL_LINK_NOARG(ExportDialog, SelectListBoxHdl, weld::ComboBox&, void)
{
    updateControls();
}

IMPL_LINK_NOARG(ExportDialog, UpdateHdl, weld::ToggleButton&, void)
{
    updateControls();
}

IMPL_LINK_NOARG(ExportDialog, UpdateHdlMtfSizeX, weld::SpinButton&, void)
{
    double fRatio = static_cast< double >( maOriginalSize.Height ) / maOriginalSize.Width;

    if ( mbIsPixelFormat )
    {
        switch( GetMapUnit( mxLbSizeX->get_active() ) )
        {
            case MapUnit::MapInch :     maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.0254 * mxMfSizeX->get_value() / 100.0 + 0.5 ); break;
            case MapUnit::MapCM :       maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.01 * mxMfSizeX->get_value() / 100.0 + 0.5 ); break;
            case MapUnit::MapMM :       maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.001 * mxMfSizeX->get_value() / 100.0 + 0.5 ); break;
            case MapUnit::MapPoint :    maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.0254 * mxMfSizeX->get_value() / 100.0 * 72 + 0.5 ); break;
            default:
            case MapUnit::MapPixel :    maSize.Width = mxMfSizeX->get_value(); break;
        }
        maSize.Height = static_cast< sal_Int32 >( fRatio * maSize.Width + 0.5 );
    }
    else
    {
        Fraction aFract( 1, 100 );
        sal_Int32 nWidth = mxMfSizeX->get_value();
        sal_Int32 nHeight= static_cast< sal_Int32 >( nWidth * fRatio );
        const Size aSource( nWidth, nHeight );
        MapMode aSourceMapMode( GetMapUnit( mxLbSizeX->get_active() ),Point(), aFract, aFract );
        Size aDest(OutputDevice::LogicToLogic(aSource, aSourceMapMode, MapMode(MapUnit::Map100thMM)));

        maSize.Width = aDest.Width();
        maSize.Height = aDest.Height();
    }
    updateControls();
}

IMPL_LINK_NOARG(ExportDialog, UpdateHdlMtfSizeY, weld::SpinButton&, void)
{
    double fRatio = static_cast< double >( maOriginalSize.Width ) / maOriginalSize.Height;

    if ( mbIsPixelFormat )
    {
        switch( GetMapUnit( mxLbSizeX->get_active() ) )
        {
            case MapUnit::MapInch :     maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.0254 * mxMfSizeY->get_value() / 100.0 + 0.5 ); break;
            case MapUnit::MapCM :       maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.01 * mxMfSizeY->get_value() / 100.0 + 0.5 ); break;
            case MapUnit::MapMM :       maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.001 * mxMfSizeY->get_value() / 100.0 + 0.5 ); break;
            case MapUnit::MapPoint :    maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.0254 * mxMfSizeY->get_value() / 100.0 * 72 + 0.5 ); break;
            default:
            case MapUnit::MapPixel :    maSize.Height = mxMfSizeY->get_value(); break;
        }
        maSize.Width = static_cast< sal_Int32 >( fRatio * maSize.Height + 0.5 );
    }
    else
    {
        Fraction aFract( 1, 100 );
        sal_Int32 nHeight= mxMfSizeY->get_value();
        sal_Int32 nWidth = static_cast< sal_Int32 >( nHeight * fRatio );
        const Size aSource( nWidth, nHeight );
        MapMode aSourceMapMode( GetMapUnit( mxLbSizeX->get_active() ),Point(), aFract, aFract );
        Size aDest( OutputDevice::LogicToLogic(aSource, aSourceMapMode, MapMode(MapUnit::Map100thMM)) );

        maSize.Height = aDest.Height();
        maSize.Width = aDest.Width();
    }
    updateControls();
}

IMPL_LINK_NOARG(ExportDialog, UpdateHdlNfResolution, weld::SpinButton&, void)
{
    auto nResolution = mxNfResolution->get_value();
    if ( mxLbResolution->get_active() == 0 )      // pixels / cm
        nResolution *= 100;
    else if ( mxLbResolution->get_active() == 1 ) // pixels / inch
        nResolution = static_cast< sal_Int32 >( ( ( static_cast< double >( nResolution ) + 0.5 ) / 0.0254 ) );
    maResolution.Width = nResolution;
    maResolution.Height= nResolution;

    updateControls();
}

IMPL_LINK_NOARG(ExportDialog, SbCompressionUpdateHdl, weld::Scale&, void)
{
    mpNfCompression->set_value(mpSbCompression->get_value());
    updateControls();
}

IMPL_LINK_NOARG(ExportDialog, OK, weld::Button&, void)
{
    // writing config parameter

    mrFltCallPara.aFilterData = GetFilterData( true );
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
