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

#include <sal/config.h>

#include <algorithm>

#include <comphelper/propertyvalue.hxx>
#include <o3tl/safeint.hxx>
#include <tools/stream.hxx>
#include <tools/fract.hxx>
#include <utility>
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
#define FORMAT_WEBP     17

#define UNIT_DEFAULT    -1
#define UNIT_INCH       0
#define UNIT_CM         1
#define UNIT_MM         2
#define UNIT_POINT      3
#define UNIT_PIXEL      4
#define UNIT_MAX_ID     UNIT_PIXEL

using namespace ::com::sun::star;

static sal_Int16 GetFilterFormat(std::u16string_view rExt)
{
    sal_Int16 nFormat = FORMAT_UNKNOWN;
    if ( rExt == u"JPG" )
        nFormat = FORMAT_JPG;
    else if ( rExt == u"PNG" )
        nFormat = FORMAT_PNG;
    else if ( rExt == u"BMP" )
        nFormat = FORMAT_BMP;
    else if ( rExt == u"GIF" )
        nFormat = FORMAT_GIF;
    else if ( rExt == u"TIF" )
        nFormat = FORMAT_TIF;
    else if ( rExt == u"WMF" )
        nFormat = FORMAT_WMF;
    else if ( rExt == u"EMF" )
        nFormat = FORMAT_EMF;
    else if ( rExt == u"EPS" )
        nFormat = FORMAT_EPS;
    else if ( rExt == u"SVG" )
        nFormat = FORMAT_SVG;
    else if ( rExt == u"WEBP" )
        nFormat = FORMAT_WEBP;
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

sal_Int32 ExportDialog::GetDefaultUnit() const
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

    for( const auto& rPrimitive : aPrimitiveSequence )
    {
        const geometry::RealRectangle2D aRect( rPrimitive->getRange( rViewInformation ) );
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

            mpOptionsItem->WriteInt32(u"PixelExportUnit"_ustr, nUnit);
            mpOptionsItem->WriteInt32(u"PixelExportResolution"_ustr, nResolution);
            mpOptionsItem->WriteInt32(u"PixelExportResolutionUnit"_ustr, mxLbResolution->get_active());
        }
        else
        {
            if ( nUnit >= UNIT_PIXEL )
                nUnit = UNIT_CM;

            mpOptionsItem->WriteInt32(u"VectorExportUnit"_ustr, nUnit);
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

    static constexpr OUString sLogicalWidth(u"LogicalWidth"_ustr);
    static constexpr OUString sLogicalHeight(u"LogicalHeight"_ustr);
    if ( mbIsPixelFormat )
    {
        pFilterOptions->WriteInt32(u"PixelWidth"_ustr, maSize.Width );
        pFilterOptions->WriteInt32(u"PixelHeight"_ustr, maSize.Height );
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
        pFilterOptions->WriteInt32( sLogicalWidth, maSize.Width );
        pFilterOptions->WriteInt32( sLogicalHeight, maSize.Height );
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
            pFilterOptions->WriteInt32(u"ColorMode"_ustr, nColor);
            assert(mpSbCompression);
            pFilterOptions->WriteInt32(u"Quality"_ustr, static_cast<sal_Int32>(mpSbCompression->get_value()));
        }
        break;

        case FORMAT_PNG :
        {
            assert(mpSbCompression);
            pFilterOptions->WriteInt32(u"Compression"_ustr, static_cast<sal_Int32>(mpSbCompression->get_value()));
            sal_Int32 nInterlace = 0;
            if ( mxCbInterlaced->get_active() )
                nInterlace++;
            pFilterOptions->WriteInt32(u"Interlaced"_ustr, nInterlace);
            sal_Int32 nValue = 0;
            if ( mxCbSaveTransparency->get_active() )
                nValue++;
            pFilterOptions->WriteInt32(u"Translucent"_ustr, nValue);
        }
        break;

        case FORMAT_BMP :
        {
            pFilterOptions->WriteInt32(u"Color"_ustr, mxLbColorDepth->get_active() + 1);
            pFilterOptions->WriteBool(u"RLE_Coding"_ustr, mxCbRLEEncoding->get_active());
        }
        break;

        case FORMAT_GIF :
        {
            sal_Int32 nValue = 0;
            if ( mxCbInterlaced->get_active() )
                nValue++;
            pFilterOptions->WriteInt32(u"Interlaced"_ustr, nValue);

            nValue = 0;
            if (mxCbSaveTransparency->get_active())
                nValue++;
            pFilterOptions->WriteInt32(u"Translucent"_ustr, nValue);
        }
        break;

        case FORMAT_EPS :
        {
            sal_Int32 nCheck = 0;
            if ( mxCbEPSPreviewTIFF->get_active() )
                nCheck++;
            if ( mxCbEPSPreviewEPSI->get_active() )
                nCheck += 2;
            pFilterOptions->WriteInt32(u"Preview"_ustr, nCheck);

            nCheck = 1;
            if ( mxRbEPSLevel2->get_active() )
                nCheck++;
            pFilterOptions->WriteInt32(u"Version"_ustr, nCheck);

            nCheck = 1;
            if ( mxRbEPSColorFormat2->get_active() )
                nCheck++;
            pFilterOptions->WriteInt32(u"ColorFormat"_ustr, nCheck);

            nCheck = 1;
            if ( mxRbEPSCompressionNone->get_active() )
                nCheck++;
            pFilterOptions->WriteInt32(u"CompressionMode"_ustr, nCheck);
        }
        break;

        case FORMAT_WEBP :
        {
            assert(mpSbCompression);
            pFilterOptions->WriteInt32(u"Quality"_ustr, static_cast<sal_Int32>(mpSbCompression->get_value()));
            pFilterOptions->WriteBool(u"Lossless"_ustr, mxCbLossless->get_active());
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
            aAny = xPagePropSet->getPropertyValue(u"Width"_ustr);
            aAny >>= nWidth;
            aAny = xPagePropSet->getPropertyValue(u"Height"_ustr);
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

        uno::Sequence< beans::PropertyValue > aViewInformation{ comphelper::makePropertyValue(
            u"ViewTransformation"_ustr, aTransformation) };

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
                    rFilter.ExportGraphic( aGraphic, u"", *mpTempStream, nFilter, &aNewFilterData );
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
                    uno::Sequence< beans::PropertyValue > aDescriptor{
                        comphelper::makePropertyValue(u"OutputStream"_ustr, xOutputStream),
                        comphelper::makePropertyValue(u"FilterName"_ustr, sFormat),
                        comphelper::makePropertyValue(u"FilterData"_ustr, aNewFilterData)
                    };

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
    return GetRawFileSize() < o3tl::make_unsigned( mnMaxFilesizeForRealtimePreview );
}

ExportDialog::ExportDialog(FltCallDialogParameter& rPara,
    css::uno::Reference< css::uno::XComponentContext > xContext,
    const css::uno::Reference< css::lang::XComponent >& rxSourceDocument,
    bool bExportSelection, bool bIsPixelFormat, bool bGraphicsSource,
    const css::uno::Reference< css::graphic::XGraphic >& rxGraphic)
    : GenericDialogController(rPara.pWindow, u"svt/ui/graphicexport.ui"_ustr, u"GraphicExportDialog"_ustr)
    , mrFltCallPara(rPara)
    , mxContext(std::move(xContext))
    , mxSourceDocument(rxSourceDocument)
    , mxGraphic(rxGraphic)
    , msEstimatedSizePix1(SvtResId(STR_SVT_ESTIMATED_SIZE_PIX_1))
    , msEstimatedSizePix2(SvtResId(STR_SVT_ESTIMATED_SIZE_PIX_2))
    , msEstimatedSizeVec(SvtResId(STR_SVT_ESTIMATED_SIZE_VEC))
    , ms1BitThreshold(SvtResId(STR_SVT_1BIT_THRESHOLD))
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
    , mxMfSizeX(m_xBuilder->weld_spin_button(u"widthmf"_ustr))
    , mxLbSizeX(m_xBuilder->weld_combo_box(u"widthlb"_ustr))
    , mxMfSizeY(m_xBuilder->weld_spin_button( u"heightmf"_ustr))
    , mxFtResolution(m_xBuilder->weld_label(u"resolutionft"_ustr))
    , mxNfResolution(m_xBuilder->weld_spin_button(u"resolutionmf"_ustr))
    , mxLbResolution(m_xBuilder->weld_combo_box(u"resolutionlb"_ustr))
    , mxColorDepth(m_xBuilder->weld_widget(u"colordepth"_ustr))
    , mxLbColorDepth(m_xBuilder->weld_combo_box(u"colordepthlb"_ustr))
    , mxJPGWEBPQuality(m_xBuilder->weld_widget(u"jpgwebpquality"_ustr))
    , mxPNGCompression(m_xBuilder->weld_widget(u"pngcompression"_ustr))
    , mxSbPngCompression(m_xBuilder->weld_scale(u"compressionpngsb"_ustr))
    , mxNfPngCompression(m_xBuilder->weld_spin_button(u"compressionpngnf"_ustr))
    , mxSbJpgWebpCompression(m_xBuilder->weld_scale(u"compressionjpgwebpsb"_ustr))
    , mxNfJpgWebpCompression(m_xBuilder->weld_spin_button(u"compressionjpgwebpnf"_ustr))
    , mxCbLossless(m_xBuilder->weld_check_button(u"losslesscb"_ustr))
    , mxMode(m_xBuilder->weld_widget(u"mode"_ustr))
    , mxCbInterlaced(m_xBuilder->weld_check_button(u"interlacedcb"_ustr))
    , mxBMPCompression(m_xBuilder->weld_widget(u"bmpcompression"_ustr))
    , mxCbRLEEncoding(m_xBuilder->weld_check_button(u"rlecb"_ustr))
    , mxDrawingObjects(m_xBuilder->weld_widget(u"drawingobjects"_ustr))
    , mxCbSaveTransparency(m_xBuilder->weld_check_button(u"savetransparencycb"_ustr))
    , mxEncoding(m_xBuilder->weld_widget(u"encoding"_ustr))
    , mxRbBinary(m_xBuilder->weld_radio_button(u"binarycb"_ustr))
    , mxRbText(m_xBuilder->weld_radio_button(u"textcb"_ustr))
    , mxEPSGrid(m_xBuilder->weld_widget(u"epsgrid"_ustr))
    , mxModifyDimension(m_xBuilder->weld_radio_button(u"modifydimensionscb"_ustr))
    , mxModifyResolution(m_xBuilder->weld_radio_button(u"modifyresolutioncb"_ustr))
    , mxCbEPSPreviewTIFF(m_xBuilder->weld_check_button(u"tiffpreviewcb"_ustr))
    , mxCbEPSPreviewEPSI(m_xBuilder->weld_check_button(u"epsipreviewcb"_ustr))
    , mxRbEPSLevel1(m_xBuilder->weld_radio_button(u"level1rb"_ustr))
    , mxRbEPSLevel2(m_xBuilder->weld_radio_button(u"level2rb"_ustr))
    , mxRbEPSColorFormat1(m_xBuilder->weld_radio_button(u"color1rb"_ustr))
    , mxRbEPSColorFormat2(m_xBuilder->weld_radio_button(u"color2rb"_ustr))
    , mxRbEPSCompressionLZW(m_xBuilder->weld_radio_button(u"compresslzw"_ustr))
    , mxRbEPSCompressionNone(m_xBuilder->weld_radio_button(u"compressnone"_ustr))
    , mxInfo(m_xBuilder->weld_widget(u"information"_ustr))
    , mxFtEstimatedSize(m_xBuilder->weld_label(u"estsizeft"_ustr))
    , mxBtnOK(m_xBuilder->weld_button(u"ok"_ustr))
{
    GetGraphicSource();

    maExt = maExt.toAsciiUpperCase();

    OUString  aFilterConfigPath( u"Office.Common/Filter/Graphic/Export/"_ustr );
    mpOptionsItem.reset(new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData ));
    aFilterConfigPath += maExt;
    mpFilterOptionsItem.reset(new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData ));

    mnInitialResolutionUnit = mbIsPixelFormat
        ? mpOptionsItem->ReadInt32(u"PixelExportUnit"_ustr, UNIT_DEFAULT)
        : mpOptionsItem->ReadInt32(u"VectorExportUnit"_ustr, UNIT_DEFAULT);

    mnMaxFilesizeForRealtimePreview = std::max(
        mpOptionsItem->ReadInt32(u"MaxFilesizeForRealtimePreview"_ustr, 0), sal_Int32(0));
    mxFtEstimatedSize->set_label(u" \n "_ustr);

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

    mxCbLossless->connect_toggled( LINK( this, ExportDialog, UpdateHdlLossless ) );

    mxCbSaveTransparency->connect_toggled( LINK( this, ExportDialog, UpdateHdl ) );

    mxModifyDimension->connect_toggled( LINK( this, ExportDialog, UpdateLock ) );
    mxModifyResolution->connect_toggled( LINK( this, ExportDialog, UpdateLock ) );

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
    sal_Int32 nResolution = mpOptionsItem->ReadInt32(u"PixelExportResolution"_ustr, 96);
    if ( nResolution < 1 )
        nResolution = 96;
    mxNfResolution->set_value( nResolution );

    sal_Int32 nResolutionUnit = mpOptionsItem->ReadInt32(u"PixelExportResolutionUnit"_ustr, 1);
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
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32(u"ColorMode"_ustr, 0);
            if ( nColor == 1 )
                nColor = 0;
            else
                nColor = 1;
            mxLbColorDepth->append_text( ms8BitGrayscale );
            mxLbColorDepth->append_text( ms24BitColor );
            mxLbColorDepth->set_active( nColor );
            mxColorDepth->show();

            // Quality
            mxJPGWEBPQuality->show();
            sal_Int32 nQuality = mpFilterOptionsItem->ReadInt32(u"Quality"_ustr, 75);
            if ((nQuality < 1 ) || (nQuality > 100))
                nQuality = 75;
            mpSbCompression = mxSbJpgWebpCompression.get();
            mpNfCompression = mxNfJpgWebpCompression.get();
            mpSbCompression->set_range(1, 100);
            mpNfCompression->set_range(1, 100);
            mpNfCompression->set_value(nQuality);
            mxCbLossless->hide(); // only for WebP
        }
        break;
        case FORMAT_PNG :
        {
            // Compression 1..9
            mxPNGCompression->show();
            sal_Int32 nCompression = mpFilterOptionsItem->ReadInt32(u"Compression"_ustr, 6);
            if ( ( nCompression < 1 ) || ( nCompression > 9 ) )
                nCompression = 6;

            mpSbCompression = mxSbPngCompression.get();
            mpNfCompression = mxNfPngCompression.get();
            mpSbCompression->set_range(1, 9);
            mpNfCompression->set_range(1, 9);
            mpNfCompression->set_value(nCompression);

            // Interlaced
            mxMode->show();
            mxCbInterlaced->set_active(mpFilterOptionsItem->ReadInt32(u"Interlaced"_ustr, 0) != 0);

            // Transparency
            mxDrawingObjects->show();
            mxCbSaveTransparency->set_active(mpFilterOptionsItem->ReadInt32(u"Translucent"_ustr, 1) != 0);
        }
        break;
        case FORMAT_BMP :
        {
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32(u"Color"_ustr, 0);
            if ( nColor == 0 )
                nColor = 6;
            else
                nColor--;
            mxLbColorDepth->append_text( ms1BitThreshold );
            mxLbColorDepth->append_text( ms8BitGrayscale );
            mxLbColorDepth->append_text( ms8BitColorPalette );
            mxLbColorDepth->append_text( ms24BitColor );
            mxLbColorDepth->set_active( nColor );
            mxColorDepth->show();

            // RLE coding
            mxBMPCompression->show();
            mxCbRLEEncoding->set_active(mpFilterOptionsItem->ReadBool(u"RLE_Coding"_ustr, true));
        }
        break;
        case FORMAT_GIF :
        {
            // Interlaced
            mxMode->show();
            mxCbInterlaced->set_active(mpFilterOptionsItem->ReadInt32(u"Interlaced"_ustr, 1) != 0);

            // Transparency
            mxDrawingObjects->show();
            mxCbSaveTransparency->set_active(mpFilterOptionsItem->ReadInt32(u"Translucent"_ustr, 1) != 0);
        }
        break;
        case FORMAT_EPS :
        {
            mxEPSGrid->show();

            sal_Int32 nPreview = mpFilterOptionsItem->ReadInt32(u"Preview"_ustr, 0);
            sal_Int32 nVersion = mpFilterOptionsItem->ReadInt32(u"Version"_ustr, 2);
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32(u"ColorFormat"_ustr, 0);
            sal_Int32 nCompr = mpFilterOptionsItem->ReadInt32(u"CompressionMode"_ustr, 2);

            mpFilterOptionsItem->ReadInt32(u"TextMode"_ustr, 0);

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
        case FORMAT_WEBP :
        {
            // Quality
            mxJPGWEBPQuality->show();
            sal_Int32 nQuality = mpFilterOptionsItem->ReadInt32(u"Quality"_ustr, 75);
            if ((nQuality < 1 ) || (nQuality > 100))
                nQuality = 75;
            mpSbCompression = mxSbJpgWebpCompression.get();
            mpNfCompression = mxNfJpgWebpCompression.get();
            mpSbCompression->set_range(1, 100);
            mpNfCompression->set_range(1, 100);
            mpNfCompression->set_value(nQuality);

            // Lossless
            mxCbLossless->set_active(mpFilterOptionsItem->ReadBool(u"Lossless"_ustr, true));
            UpdateHdlLossless(*mxCbLossless);
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
    sal_Int32 nX( aVal.indexOf( '.' ) );
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

IMPL_LINK_NOARG(ExportDialog, UpdateHdl, weld::Toggleable&, void)
{
    updateControls();
}

IMPL_LINK_NOARG(ExportDialog, UpdateHdlLossless, weld::Toggleable&, void)
{
    mpSbCompression->set_sensitive(!mxCbLossless->get_active());
    mpNfCompression->set_sensitive(!mxCbLossless->get_active());
    updateControls();
}

IMPL_LINK_NOARG(ExportDialog, UpdateLock, weld::Toggleable&, void)
{
    if (mxModifyResolution->get_active())
    {
        mxMfSizeY->set_sensitive(false);
        mxMfSizeX->set_sensitive(false);
        mxNfResolution->set_sensitive(true);
    }
    else
    {
        mxMfSizeY->set_sensitive(true);
        mxMfSizeX->set_sensitive(true);
        mxNfResolution->set_sensitive(false);
    }
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
