/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifndef GCC
#   pragma hdrstop
#endif

#include <tools/stream.hxx>
#include <svtools/filter.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XPrimitiveFactory2D.hpp>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <unotools/streamwrap.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graph.hxx>
#include <rtl/ustrbuf.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include "exportdialog.hxx"
#include "exportdialog.hrc"

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

static sal_Int16 GetFilterFormat(const rtl::OUString& rExt)
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
        sal_Int32 nUnit = maLbSizeX.GetSelectEntryPos();
        if ( nUnit < 0 )
            nUnit = UNIT_CM;

        if ( ( mnInitialResolutionUnit == UNIT_DEFAULT ) && ( nUnit == GetDefaultUnit() ) )
            nUnit = UNIT_DEFAULT;

        // updating ui configuration
        if ( mbIsPixelFormat )
        {
            if ( nUnit > UNIT_MAX_ID )
                nUnit = UNIT_PIXEL;

            sal_Int32 nResolution = maNfResolution.GetValue();
            if ( nResolution < 1 )
                nResolution = 96;

            mpOptionsItem->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "PixelExportUnit" ) ), nUnit );
            mpOptionsItem->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "PixelExportResolution" ) ), nResolution );
            mpOptionsItem->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "PixelExportResolutionUnit" ) ), maLbResolution.GetSelectEntryPos() );
        }
        else
        {
            if ( nUnit >= UNIT_PIXEL )
                nUnit = UNIT_CM;

            mpOptionsItem->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "VectorExportUnit" ) ), nUnit );
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

    const String sLogicalWidth( String( RTL_CONSTASCII_USTRINGPARAM( "LogicalWidth" ) ) );
    const String sLogicalHeight( String( RTL_CONSTASCII_USTRINGPARAM( "LogicalHeight" ) ) );
    if ( mbIsPixelFormat )
    {
        pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "PixelWidth" ) ), static_cast< sal_Int32 >( maSize.Width ) );
        pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "PixelHeight" ) ), static_cast< sal_Int32 >( maSize.Height ) );
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
            sal_Int32 nColor = maLbColorDepth.GetSelectEntryPos();
            if ( nColor == 1 )
                nColor = 0;
            else
                nColor = 1;
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "ColorMode" ) ), nColor );
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) ), static_cast< sal_Int32 >( maSbCompression.GetThumbPos() ) );
        }
        break;

        case FORMAT_PNG :
        {
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Compression" ) ), static_cast< sal_Int32 >( maSbCompression.GetThumbPos() ) );
            sal_Int32 nInterlace = 0;
            if ( maCbInterlaced.IsChecked() )
                nInterlace++;
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Interlaced" ) ), nInterlace );
        }
        break;

        case FORMAT_BMP :
        {
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Color" ) ), maLbColorDepth.GetSelectEntryPos() + 1 );
            pFilterOptions->WriteBool( String( RTL_CONSTASCII_USTRINGPARAM( "RLE_Coding" ) ), maCbRLEEncoding.IsChecked() );
        }
        break;

        case FORMAT_GIF :
        {
            sal_Int32 nValue = 0;
            if ( maCbInterlaced.IsChecked() )
                nValue++;
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Interlaced" ) ), nValue );

            nValue = 0;
            if ( maCbSaveTransparency.IsChecked() )
                nValue++;
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Translucent" ) ), nValue );
        }
        break;

        case FORMAT_PBM :
        case FORMAT_PGM :
        case FORMAT_PPM :
        {
            sal_Int32 nFormat = 0;
            if ( maRbText.IsChecked() )
                nFormat++;
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "FileFormat" ) ), nFormat );
        }
        break;

        case FORMAT_EPS :
        {
            sal_Int32 nCheck = 0;
            if ( maCbEPSPreviewTIFF.IsChecked() )
                nCheck++;
            if ( maCbEPSPreviewEPSI.IsChecked() )
                nCheck += 2;
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Preview" ) ), nCheck );

            nCheck = 1;
            if ( maRbEPSLevel2.IsChecked() )
                nCheck++;
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Version" ) ), nCheck );

            nCheck = 1;
            if ( maRbEPSColorFormat2.IsChecked() )
                nCheck++;
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "ColorFormat" ) ), nCheck );

            nCheck = 1;
            if ( maRbEPSCompressionNone.IsChecked() )
                nCheck++;
            pFilterOptions->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( "CompressionMode" ) ), nCheck );
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
            aAny = xPagePropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) );
            aAny >>= nWidth;
            aAny = xPagePropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) );
            aAny >>= nHeight;
            aShapesRange = basegfx::B2DRange( 0, 0, nWidth, nHeight );
        }
    }
    else
    {
        uno::Reference< graphic::XPrimitiveFactory2D > xPrimitiveFactory(
            mxMgr->createInstance( String( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.graphic.PrimitiveFactory2D" ) ) ), uno::UNO_QUERY );
        if ( xPrimitiveFactory.is() )
        {
            basegfx::B2DHomMatrix aViewTransformation( Application::GetDefaultDevice()->GetViewTransformation() );
            com::sun::star::geometry::AffineMatrix2D aTransformation;
            aTransformation.m00 = aViewTransformation.get(0,0);
            aTransformation.m01 = aViewTransformation.get(0,1);
            aTransformation.m02 = aViewTransformation.get(0,2);
            aTransformation.m10 = aViewTransformation.get(1,0);
            aTransformation.m11 = aViewTransformation.get(1,1);
            aTransformation.m12 = aViewTransformation.get(1,2);

            const rtl::OUString sViewTransformation( RTL_CONSTASCII_USTRINGPARAM( "ViewTransformation" ) );
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

            uno::Reference< document::XExporter > xGraphicExporter(
                mxMgr->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GraphicExportFilter") ) ), uno::UNO_QUERY_THROW );
            uno::Reference< document::XFilter > xFilter( xGraphicExporter, uno::UNO_QUERY_THROW );

            sal_Int32 nProperties = 2;
            uno::Sequence< beans::PropertyValue > aFilterData( nProperties );


            rtl::OUString sFormat( maExt );
            uno::Sequence< beans::PropertyValue > aDescriptor( 3 );
            aDescriptor[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("OutputStream") );
            aDescriptor[0].Value <<= xOutputStream;
            aDescriptor[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FilterName") );
            aDescriptor[1].Value <<= sFormat;
            aDescriptor[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FilterData") );
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
                xFilter->filter( aDescriptor );
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
        String aEntry( maLbColorDepth.GetSelectEntry() );
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

ExportDialog::ExportDialog( FltCallDialogParameter& rPara,
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > rxMgr,
        const com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rxSourceDocument,
            sal_Bool bExportSelection, sal_Bool bIsPixelFormat ) :
                ModalDialog             ( rPara.pWindow, ResId( DLG_EXPORT, *rPara.pResMgr ) ),
                mrFltCallPara           ( rPara ),
                mpMgr                   ( rPara.pResMgr ),
                mxMgr                   ( rxMgr ),
                mxSourceDocument        ( rxSourceDocument ),
                maFlExportSize          ( this, ResId( FL_EXPORT_SIZE, *rPara.pResMgr ) ),
                maFtSizeX               ( this, ResId( FT_SIZEX, *rPara.pResMgr ) ),
                maMfSizeX               ( this, ResId( MF_SIZEX, *rPara.pResMgr ) ),
                maLbSizeX               ( this, ResId( LB_SIZEX, *rPara.pResMgr ) ),
                maFtSizeY               ( this, ResId( FT_SIZEY, *rPara.pResMgr ) ),
                maMfSizeY               ( this, ResId( MF_SIZEY, *rPara.pResMgr ) ),
                maLbSizeY               ( this, ResId( LB_SIZEY, *rPara.pResMgr ) ),
                maFtResolution          ( this, ResId( FT_RESOLUTION, *rPara.pResMgr ) ),
                maNfResolution          ( this, ResId( NF_RESOLUTION, *rPara.pResMgr ) ),
                maLbResolution          ( this, ResId( LB_RESOLUTION, *rPara.pResMgr ) ),
                maFlColorDepth          ( this, ResId( FL_COLOR_DEPTH, *rPara.pResMgr ) ),
                maLbColorDepth          ( this, ResId( LB_COLOR_DEPTH, *rPara.pResMgr ) ),
                maFlJPGQuality          ( this, ResId( FL_JPG_QUALITY, *rPara.pResMgr ) ),
                maFlMode                ( this, ResId( FL_MODE, *rPara.pResMgr ) ),
                maSbCompression         ( this, ResId( SB_COMPRESSION, *rPara.pResMgr ) ),
                maNfCompression         ( this, ResId( NF_COMPRESSION, *rPara.pResMgr ) ),
                maFtJPGMin              ( this, ResId( FT_JPG_MIN, *rPara.pResMgr ) ),
                maFtJPGMax              ( this, ResId( FT_JPG_MAX, *rPara.pResMgr ) ),
                maFtPNGMin              ( this, ResId( FT_PNG_MIN, *rPara.pResMgr ) ),
                maFtPNGMax              ( this, ResId( FT_PNG_MAX, *rPara.pResMgr ) ),
                maCbInterlaced          ( this, ResId( CB_INTERLACED, *rPara.pResMgr ) ),
                maCbRLEEncoding         ( this, ResId( CB_RLE_ENCODING, *rPara.pResMgr ) ),
                maFlGIFDrawingObjects   ( this, ResId( FL_GIF_DRAWING_OBJECTS, *rPara.pResMgr ) ),
                maCbSaveTransparency    ( this, ResId( CB_SAVE_TRANSPARENCY, *rPara.pResMgr ) ),
                maRbBinary              ( this, ResId( RB_BINARY, *rPara.pResMgr ) ),
                maRbText                ( this, ResId( RB_TEXT, *rPara.pResMgr ) ),
                maFlEPSPreview          ( this, ResId( FL_EPS_PREVIEW, *rPara.pResMgr ) ),
                maCbEPSPreviewTIFF      ( this, ResId( CB_EPS_PREVIEW_TIFF, *rPara.pResMgr ) ),
                maCbEPSPreviewEPSI      ( this, ResId( CB_EPS_PREVIEW_EPSI, *rPara.pResMgr ) ),
                maFlEPSVersion          ( this, ResId( FL_EPS_VERSION, *rPara.pResMgr ) ),
                maRbEPSLevel1           ( this, ResId( RB_EPS_LEVEL1, *rPara.pResMgr ) ),
                maRbEPSLevel2           ( this, ResId( RB_EPS_LEVEL2, *rPara.pResMgr ) ),
                maFlEPSColorFormat      ( this, ResId( FL_EPS_COLOR_FORMAT, *rPara.pResMgr ) ),
                maRbEPSColorFormat1     ( this, ResId( RB_EPS_COLOR_FORMAT1, *rPara.pResMgr ) ),
                maRbEPSColorFormat2     ( this, ResId( RB_EPS_COLOR_FORMAT2, *rPara.pResMgr ) ),
                maFlCompression         ( this, ResId( FL_COMPRESSION, *rPara.pResMgr ) ),
                maRbEPSCompressionLZW   ( this, ResId( RB_EPS_COMPRESSION_LZW, *rPara.pResMgr ) ),
                maRbEPSCompressionNone  ( this, ResId( RB_EPS_COMPRESSION_NONE, *rPara.pResMgr ) ),
                maFlEstimatedSize       ( this, ResId( FL_ESTIMATED_SIZE, *rPara.pResMgr ) ),
                maFtEstimatedSize       ( this, ResId( FT_ESTIMATED_SIZE, *rPara.pResMgr ) ),
                msEstimatedSizePix1     ( ResId( STR_ESTIMATED_SIZE_PIX_1, *rPara.pResMgr ).toString() ),
                msEstimatedSizePix2     ( ResId( STR_ESTIMATED_SIZE_PIX_2, *rPara.pResMgr ).toString() ),
                msEstimatedSizeVec      ( ResId( STR_ESTIMATED_SIZE_VEC, *rPara.pResMgr ).toString() ),
                maFlButtons             ( this, ResId( FL_BUTTONS, *rPara.pResMgr ) ),
                maBtnOK                 ( this, ResId( BTN_OK, *rPara.pResMgr ) ),
                maBtnCancel             ( this, ResId( BTN_CANCEL, *rPara.pResMgr ) ),
                maBtnHelp               ( this, ResId( BTN_HELP, *rPara.pResMgr ) ),
                ms1BitTreshold          ( ResId( STR_1BIT_THRESHOLD, *rPara.pResMgr ).toString() ),
                ms1BitDithered          ( ResId( STR_1BIT_DITHERED, *rPara.pResMgr ).toString() ),
                ms4BitGrayscale         ( ResId( STR_4BIT_GRAYSCALE, *rPara.pResMgr ).toString() ),
                ms4BitColorPalette      ( ResId( STR_4BIT_COLOR_PALETTE, *rPara.pResMgr ).toString() ),
                ms8BitGrayscale         ( ResId( STR_8BIT_GRAYSCALE, *rPara.pResMgr ).toString() ),
                ms8BitColorPalette      ( ResId( STR_8BIT_COLOR_PALETTE, *rPara.pResMgr ).toString() ),
                ms24BitColor            ( ResId( STR_24BIT_TRUE_COLOR, *rPara.pResMgr ).toString() ),
                maExt                   ( rPara.aFilterExt ),
                mnFormat                ( FORMAT_UNKNOWN ),
                mnMaxFilesizeForRealtimePreview( 0 ),
                mpTempStream            ( new SvMemoryStream() ),
                maOriginalSize          ( awt::Size( 0, 0 ) ),
                mbIsPixelFormat         ( bIsPixelFormat ),
                mbExportSelection       ( bExportSelection ),
                mbPreserveAspectRatio   ( sal_True )
{
    GetGraphicSource();

    maExt.ToUpperAscii();

    String  aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/" ) );
    mpOptionsItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );
    aFilterConfigPath.Append( maExt );
    mpFilterOptionsItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );

    mnInitialResolutionUnit = mbIsPixelFormat
        ? mpOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "PixelExportUnit" ) ), UNIT_DEFAULT )
        : mpOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "VectorExportUnit" ) ), UNIT_DEFAULT );

    mnMaxFilesizeForRealtimePreview = mpOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "MaxFilesizeForRealtimePreview" ) ), 0 );
    maFtEstimatedSize.SetText( String( RTL_CONSTASCII_USTRINGPARAM( " \n " ) ) );

    String  aTitle( maExt );
    aTitle += ResId(DLG_EXPORT_TITLE, *mpMgr).toString();
    SetText( aTitle );

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

    // Size
    maLbSizeX.SetSelectHdl( LINK( this, ExportDialog, UpdateHdl ) );

    maSbCompression.SetScrollHdl( LINK( this, ExportDialog, SbCompressionUpdateHdl ) );
    maNfCompression.SetModifyHdl( LINK( this, ExportDialog, UpdateHdl ) );

    maMfSizeX.SetModifyHdl( LINK( this, ExportDialog, UpdateHdlMtfSizeX ) );
    maMfSizeY.SetModifyHdl( LINK( this, ExportDialog, UpdateHdlMtfSizeY ) );

    maNfResolution.SetModifyHdl( LINK( this, ExportDialog, UpdateHdlNfResolution ) );
    maLbResolution.SetSelectHdl( LINK( this, ExportDialog, UpdateHdl ) );

    maLbColorDepth.SetSelectHdl( LINK( this, ExportDialog, UpdateHdl ) );

    maCbInterlaced.SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    maCbSaveTransparency.SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    maCbEPSPreviewTIFF.SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );
    maCbEPSPreviewEPSI.SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    maRbEPSCompressionLZW.SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );
    maRbEPSCompressionNone.SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    maRbBinary.SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );
    maRbText.SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    // BMP
    maCbRLEEncoding.SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    // EPS
    maRbEPSLevel1.SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );
    maRbEPSLevel2.SetClickHdl( LINK( this, ExportDialog, UpdateHdl ) );

    maBtnOK.SetClickHdl( LINK( this, ExportDialog, OK ) );

    setupLayout();
    updateControls();

    FreeResource();
}

void ExportDialog::createSizeControls( vcl::RowOrColumn& rLayout )
{
    size_t nIndex;
    Size aBorder( LogicToPixel( Size( 5, 5 ), MapMode( MAP_APPFONT ) ) );
    long nIndent = aBorder.Width();

    // Size controls
    rLayout.addWindow( &maFlExportSize );

    Size aLbMax( maLbSizeX.GetSizePixel() );
    aLbMax.Width() = Max( aLbMax.Width(), maLbResolution.GetSizePixel().Width() );

    boost::shared_ptr< vcl::LabelColumn > xSizeColumns( new vcl::LabelColumn( &rLayout ) );
    rLayout.addChild( xSizeColumns );

    // row 1
    boost::shared_ptr< vcl::RowOrColumn > xColumn( new vcl::RowOrColumn( xSizeColumns.get(), false ) );
    xSizeColumns->addRow( &maFtSizeX, xColumn, nIndent );
    Size aMinSize( maMfSizeX.GetSizePixel() );
    nIndex = xColumn->addWindow( &maMfSizeX );
    xColumn->setMinimumSize( nIndex, aMinSize );
    nIndex = xColumn->addWindow( &maLbSizeX );
    xColumn->setMinimumSize( nIndex, aLbMax );

    // row 2
    xColumn = boost::shared_ptr< vcl::RowOrColumn >( new vcl::RowOrColumn( xSizeColumns.get(), false ) );
    xSizeColumns->addRow( &maFtSizeY, xColumn, nIndent );
    nIndex = xColumn->addWindow( &maMfSizeY );
    xColumn->setMinimumSize( nIndex, aMinSize );
    nIndex = xColumn->addWindow( &maLbSizeY );
    xColumn->setMinimumSize( nIndex, aLbMax );

    // row 3
    if ( mbIsPixelFormat )      // TODO: (metafileresolutionsupport)
    {
        xColumn = boost::shared_ptr< vcl::RowOrColumn >( new vcl::RowOrColumn( xSizeColumns.get(), false ) );
        xSizeColumns->addRow( &maFtResolution, xColumn, nIndent );
        nIndex = xColumn->addWindow( &maNfResolution );
        xColumn->setMinimumSize( nIndex, aMinSize );
        nIndex = xColumn->addWindow( &maLbResolution );
        xColumn->setMinimumSize( nIndex, aLbMax );
    }

    sal_Int32 nUnit = mnInitialResolutionUnit;
    if ( nUnit == UNIT_DEFAULT )
        nUnit = GetDefaultUnit();

    if ( !mbIsPixelFormat )
    {
        maLbSizeX.RemoveEntry( UNIT_PIXEL );        // removing pixel
        if ( nUnit >= UNIT_PIXEL )
            nUnit = UNIT_CM;
    }
    else if ( nUnit > UNIT_MAX_ID )
        nUnit = UNIT_PIXEL;
    if ( nUnit < 0 )
        nUnit = UNIT_CM;
    maLbSizeX.SelectEntryPos( static_cast< sal_uInt16 >( nUnit ) );

    if ( mbIsPixelFormat )      // TODO: (metafileresolutionsupport) should be supported for vector formats also... this makes
    {                           // sense eg for bitmap fillings in metafiles, to preserve high dpi output
                                // (atm without special vector support the bitmaps are rendered with 96dpi)
        sal_Int32 nResolution = mpOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "PixelExportResolution" ) ), 96 );
        if ( nResolution < 1 )
            nResolution = 96;
        maNfResolution.SetValue( nResolution );

        sal_Int32 nResolutionUnit = mpOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "PixelExportResolutionUnit" ) ), 1 );
        if ( ( nResolutionUnit < 0 ) || ( nResolutionUnit > 2 ) )
            nResolutionUnit = 1;
        maLbResolution.SelectEntryPos( static_cast< sal_uInt16 >( nResolutionUnit ) );
    }

    boost::shared_ptr< vcl::Spacer > xSpacer( new vcl::Spacer( &rLayout, 2 ) );
    rLayout.addChild( xSpacer );
}

void ExportDialog::createColorDepthControls( vcl::RowOrColumn& rLayout )
{
    // Color Depth
    Size aBorder( LogicToPixel( Size( 5, 5 ), MapMode( MAP_APPFONT ) ) );
    long nIndent = aBorder.Width();

    boost::shared_ptr< vcl::RowOrColumn > xRow( new vcl::RowOrColumn( &rLayout, false ) );
    rLayout.addChild( xRow );
    xRow->addWindow( &maFlColorDepth );

    xRow = boost::shared_ptr< vcl::RowOrColumn >( new vcl::RowOrColumn( &rLayout, false ) );
    rLayout.addChild( xRow );
    boost::shared_ptr< vcl::Indenter > xIndenter( new vcl::Indenter( &rLayout, nIndent ) );
    xRow->addChild( xIndenter );
    boost::shared_ptr< vcl::RowOrColumn > xRows( new vcl::RowOrColumn( &rLayout, true ) );
    xIndenter->setChild( xRows );
    xRows->addWindow( &maLbColorDepth );

    boost::shared_ptr< vcl::Spacer > xSpacer( new vcl::Spacer( &rLayout, 2 ) );
    rLayout.addChild( xSpacer );
}

void ExportDialog::createScrollBar( vcl::RowOrColumn& rLayout )
{
    boost::shared_ptr< vcl::RowOrColumn > xRow( new vcl::RowOrColumn( &rLayout, false ) );
    rLayout.addChild( xRow );

    Size aMinSize( maSbCompression.GetSizePixel() );
    size_t nIndex = xRow->addWindow( &maSbCompression );
    xRow->setMinimumSize( nIndex, aMinSize );
    aMinSize = maNfCompression.GetSizePixel();
    nIndex = xRow->addWindow( &maNfCompression );
    xRow->setMinimumSize( nIndex, aMinSize );
}

void ExportDialog::createFilterOptions( vcl::RowOrColumn& rLayout )
{
    Size aBorder( LogicToPixel( Size( 5, 5 ), MapMode( MAP_APPFONT ) ) );
    long nIndent = aBorder.Width();

    switch( mnFormat )
    {
        case FORMAT_JPG :
        {
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "ColorMode" ) ), 0 );
            if ( nColor == 1 )
                nColor = 0;
            else
                nColor = 1;
            maLbColorDepth.InsertEntry( ms8BitGrayscale );
            maLbColorDepth.InsertEntry( ms24BitColor );
            maLbColorDepth.SelectEntryPos( nColor );
            createColorDepthControls( maLayout );

            rLayout.addWindow( &maFlJPGQuality );

            // Quality
            boost::shared_ptr< vcl::Indenter > xIndenter( new vcl::Indenter( &rLayout, nIndent ) );
            rLayout.addChild( xIndenter );
            boost::shared_ptr< vcl::RowOrColumn > xRows( new vcl::RowOrColumn( &rLayout, true ) );
            xIndenter->setChild( xRows );
            createScrollBar( *xRows.get() );
            xRows->addWindow( &maFtJPGMin );
            xRows->addWindow( &maFtJPGMax );

            boost::shared_ptr< vcl::Spacer > xSpacer( new vcl::Spacer( &rLayout, 2 ) );
            rLayout.addChild( xSpacer );

            sal_Int32 nQuality = mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) ), 75 );
            if ( ( nQuality < 1 ) || ( nQuality > 100 ) )
                nQuality = 75;

            maSbCompression.SetRangeMin( 1 );
            maSbCompression.SetRangeMax( 100 );
            maNfCompression.SetMin( 1 );
            maNfCompression.SetMax( 100 );
            maNfCompression.SetValue( nQuality );
            maNfCompression.SetStrictFormat( sal_True );
        }
        break;
        case FORMAT_PNG :
        {
            rLayout.addWindow( &maFlCompression );

            // Compression 1..9
            boost::shared_ptr< vcl::Indenter > xIndenter( new vcl::Indenter( &rLayout, nIndent ) );
            rLayout.addChild( xIndenter );
            boost::shared_ptr< vcl::RowOrColumn > xRows( new vcl::RowOrColumn( &rLayout, true ) );
            xIndenter->setChild( xRows );
            createScrollBar( *xRows.get() );
            xRows->addWindow( &maFtPNGMin );
            xRows->addWindow( &maFtPNGMax );
            boost::shared_ptr< vcl::Spacer > xSpacer( new vcl::Spacer( &rLayout, 2 ) );
            rLayout.addChild( xSpacer );

            // Interlaced
            rLayout.addWindow( &maFlMode );
            xIndenter.reset( new vcl::Indenter( &rLayout, nIndent ) );
            rLayout.addChild( xIndenter );
            xRows.reset( new vcl::RowOrColumn( &rLayout, true ) );
            xIndenter->setChild( xRows );
            xRows->addWindow( &maCbInterlaced );

            xSpacer.reset( new vcl::Spacer( &rLayout, 2 ) );
            rLayout.addChild( xSpacer );

            sal_Int32 nCompression = mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Compression" ) ), 6 );
            if ( ( nCompression < 1 ) || ( nCompression > 9 ) )
                nCompression = 6;
            maSbCompression.SetRangeMin( 1 );
            maSbCompression.SetRangeMax( 9 );
            maNfCompression.SetMin( 1 );
            maNfCompression.SetMax( 9 );
            maNfCompression.SetValue( 9 );
            maNfCompression.SetStrictFormat( sal_True );

            maCbInterlaced.Check( mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Interlaced" ) ), 0 ) != 0 );
        }
        break;
        case FORMAT_BMP :
        {
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Color" ) ), 0 );
            if ( nColor == 0 )
                nColor = 6;
            else
                nColor--;
            maLbColorDepth.InsertEntry( ms1BitTreshold );
            maLbColorDepth.InsertEntry( ms1BitDithered );
            maLbColorDepth.InsertEntry( ms4BitGrayscale );
            maLbColorDepth.InsertEntry( ms4BitColorPalette );
            maLbColorDepth.InsertEntry( ms8BitGrayscale );
            maLbColorDepth.InsertEntry( ms8BitColorPalette );
            maLbColorDepth.InsertEntry( ms24BitColor );
            maLbColorDepth.SelectEntryPos( nColor );
            createColorDepthControls( maLayout );

            rLayout.addWindow( &maFlCompression );
            // RLE coding
            boost::shared_ptr< vcl::Indenter > xIndenter( new vcl::Indenter( &rLayout, nIndent ) );
            rLayout.addChild( xIndenter );
            boost::shared_ptr< vcl::RowOrColumn > xRows( new vcl::RowOrColumn( &rLayout, true ) );
            xIndenter->setChild( xRows );
            xRows->addWindow( &maCbRLEEncoding );
            boost::shared_ptr< vcl::Spacer > xSpacer( new vcl::Spacer( &rLayout, 2 ) );
            rLayout.addChild( xSpacer );

            maCbRLEEncoding.Check( mpFilterOptionsItem->ReadBool( String( RTL_CONSTASCII_USTRINGPARAM( "RLE_Coding" ) ), sal_True ) );
        }
        break;
        case FORMAT_GIF :
        {
            rLayout.addWindow( &maFlMode );
            boost::shared_ptr< vcl::Indenter > xIndenter( new vcl::Indenter( &rLayout, nIndent ) );
            rLayout.addChild( xIndenter );
            boost::shared_ptr< vcl::RowOrColumn > xRows( new vcl::RowOrColumn( &rLayout, true ) );
            xIndenter->setChild( xRows );
            xRows->addWindow( &maCbInterlaced );
            boost::shared_ptr< vcl::Spacer > xSpacer( new vcl::Spacer( &rLayout, 2 ) );
            rLayout.addChild( xSpacer );

            rLayout.addWindow( &maFlGIFDrawingObjects );
            xIndenter = boost::shared_ptr< vcl::Indenter >( new vcl::Indenter( &rLayout, nIndent ) );
            rLayout.addChild( xIndenter );
            xRows = boost::shared_ptr< vcl::RowOrColumn >( new vcl::RowOrColumn( &rLayout, true ) );
            xIndenter->setChild( xRows );
            xRows->addWindow( &maCbSaveTransparency );
            xSpacer.reset( new vcl::Spacer( &rLayout, 2 ) );
            rLayout.addChild( xSpacer );

            maCbInterlaced.Check( mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Interlaced" ) ), 1 ) != 0 );
            maCbSaveTransparency.Check( mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Translucent" ) ), 1 ) != 0 );
        }
        break;
        case FORMAT_PBM :
        case FORMAT_PGM :
        case FORMAT_PPM :
        {
            rLayout.addWindow( &maFlJPGQuality );

            // RB Binary / Text
            boost::shared_ptr< vcl::Indenter > xIndenter( new vcl::Indenter( &rLayout, nIndent ) );
            rLayout.addChild( xIndenter );
            boost::shared_ptr< vcl::RowOrColumn > xRows( new vcl::RowOrColumn( &rLayout, true ) );
            xIndenter->setChild( xRows );
            xRows->addWindow( &maRbBinary );
            xRows->addWindow( &maRbText );
            boost::shared_ptr< vcl::Spacer > xSpacer( new vcl::Spacer( &rLayout, 2 ) );
            rLayout.addChild( xSpacer );

            sal_Int32 nFormat = mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "FileFormat" ) ), 1 );
            maRbBinary.Check( nFormat == 0 );
            maRbText.Check( nFormat != 0 );
        }
        break;
        case FORMAT_EPS :
        {
            boost::shared_ptr< vcl::RowOrColumn > xColumns( new vcl::RowOrColumn( &rLayout, false ) );
            rLayout.addChild( xColumns );
            boost::shared_ptr< vcl::RowOrColumn > xLeft( new vcl::RowOrColumn( &rLayout, true ) );
            xColumns->addChild( xLeft );

            xLeft->addWindow( &maFlEPSPreview );
            boost::shared_ptr< vcl::Indenter > xIndenter( new vcl::Indenter( xLeft.get(), nIndent ) );
            xLeft->addChild( xIndenter );
            boost::shared_ptr< vcl::RowOrColumn > xRows( new vcl::RowOrColumn( xLeft.get(), true ) );
            xIndenter->setChild( xRows );
            xRows->addWindow( &maCbEPSPreviewTIFF );
            xRows->addWindow( &maCbEPSPreviewEPSI );
            boost::shared_ptr< vcl::Spacer > xSpacer( new vcl::Spacer( xLeft.get(), 2 ) );
            xLeft->addChild( xSpacer );

            xLeft->addWindow( &maFlEPSVersion );
            xIndenter = boost::shared_ptr< vcl::Indenter >( new vcl::Indenter( xLeft.get(), nIndent ) );
            xLeft->addChild( xIndenter );
            xRows = boost::shared_ptr< vcl::RowOrColumn >( new vcl::RowOrColumn( xLeft.get(), true ) );
            xIndenter->setChild( xRows );
            xRows->addWindow( &maRbEPSLevel1 );
            xRows->addWindow( &maRbEPSLevel2 );
            xSpacer.reset( new vcl::Spacer( xLeft.get(), 2 ) );
            xLeft->addChild( xSpacer );

            boost::shared_ptr< vcl::RowOrColumn > xRight( new vcl::RowOrColumn( &rLayout, true ) );
            xColumns->addChild( xRight );

            xRight->addWindow( &maFlEPSColorFormat );
            xIndenter = boost::shared_ptr< vcl::Indenter >( new vcl::Indenter( xRight.get(), nIndent ) );
            xRight->addChild( xIndenter );
            xRows = boost::shared_ptr< vcl::RowOrColumn >( new vcl::RowOrColumn( xRight.get(), true ) );
            xIndenter->setChild( xRows );
            xRows->addWindow( &maRbEPSColorFormat1 );
            xRows->addWindow( &maRbEPSColorFormat2 );
            xSpacer.reset( new vcl::Spacer( xRight.get(), 2 ) );
            xRight->addChild( xSpacer );

            xRight->addWindow( &maFlCompression );
            xIndenter = boost::shared_ptr< vcl::Indenter >( new vcl::Indenter( xRight.get(), nIndent ) );
            xRight->addChild( xIndenter );
            xRows = boost::shared_ptr< vcl::RowOrColumn >( new vcl::RowOrColumn( xRight.get(), true ) );
            xIndenter->setChild( xRows );
            xRows->addWindow( &maRbEPSCompressionLZW );
            xRows->addWindow( &maRbEPSCompressionNone );

            xSpacer.reset( new vcl::Spacer( &rLayout, 2 ) );
            rLayout.addChild( xSpacer );

            sal_Int32 nPreview = mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Preview" ) ), 0 );
            sal_Int32 nVersion = mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Version" ) ), 2 );
            sal_Int32 nColor = mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "ColorFormat" ) ), 0 );
            sal_Int32 nCompr = mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "CompressionMode" ) ), 2 );

            mpFilterOptionsItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "TextMode" ) ), 0 );

            maCbEPSPreviewTIFF.Check( ( nPreview & 1 ) != 0 );
            maCbEPSPreviewEPSI.Check( ( nPreview & 2 ) != 0 );

            maRbEPSLevel1.Check( nVersion == 1 );
            maRbEPSLevel2.Check( nVersion == 2 );

            maRbEPSColorFormat1.Check( nColor == 1 );
            maRbEPSColorFormat2.Check( nColor != 1 );

            maRbEPSCompressionLZW.Check( nCompr == 1 );
            maRbEPSCompressionNone.Check( nCompr != 1 );
        }
        break;
    }
}

void ExportDialog::createButtons( vcl::RowOrColumn& rLayout )
{
    rLayout.addWindow( &maFlButtons );
    boost::shared_ptr< vcl::Spacer > xSpacer( new vcl::Spacer( &rLayout, 2 ) );
    rLayout.addChild( xSpacer );

    Size aBorder( LogicToPixel( Size( 5, 5 ), MapMode( MAP_APPFONT ) ) );

    boost::shared_ptr< vcl::RowOrColumn > xButtons( new vcl::RowOrColumn( &rLayout, false ) );
    size_t nIndex = rLayout.addChild( xButtons );
    rLayout.setBorders( nIndex, aBorder.Width(), 0, aBorder.Width(), aBorder.Width() );

    Size aMinSize( maBtnCancel.GetSizePixel() );
    // insert help button
    xButtons->setMinimumSize( xButtons->addWindow( &maBtnHelp ), aMinSize );

    // insert a spacer, cancel and OK buttons are right aligned

    xSpacer.reset( new vcl::Spacer( xButtons.get(), 2 ) );
    xButtons->addChild( xSpacer );
    xButtons->setMinimumSize( xButtons->addWindow( &maBtnOK ), aMinSize );
    xButtons->setMinimumSize( xButtons->addWindow( &maBtnCancel ), aMinSize );
}

void ExportDialog::setupLayout()
{
    Size aBorder( LogicToPixel( Size( 5, 5 ), MapMode( MAP_APPFONT ) ) );
    maLayout.setParentWindow( this );
    maLayout.setOuterBorder( aBorder.Width() );

    createSizeControls( maLayout );
    createFilterOptions( maLayout );

    if ( mnMaxFilesizeForRealtimePreview || mbIsPixelFormat )
    {
        maLayout.addWindow( &maFlEstimatedSize );
        maLayout.addWindow( &maFtEstimatedSize );
    }
    createButtons( maLayout );

    maLayout.show();
    maDialogSize = maLayout.getOptimalSize( WINDOWSIZE_PREFERRED );
    maLayout.setManagedArea( Rectangle( Point(), maDialogSize ) );
    SetOutputSizePixel( maDialogSize );

    maRectFlButtons = Rectangle( maFlButtons.GetPosPixel(), maFlButtons.GetSizePixel() );
    maRectBtnHelp   = Rectangle( maBtnHelp.GetPosPixel(), maBtnHelp.GetSizePixel() );
    maRectBtnOK     = Rectangle( maBtnOK.GetPosPixel(), maBtnOK.GetSizePixel() );
    maRectBtnCancel = Rectangle( maBtnCancel.GetPosPixel(), maBtnOK.GetSizePixel() );

    maLbSizeY.Hide();
}

static rtl::OUString ImpValueOfInKB( const sal_Int64& rVal )
{
    double fVal( static_cast<double>( rVal ) );
    fVal /= ( 1 << 10 );
    fVal += 0.05;
    rtl::OUStringBuffer aVal( rtl::OUString::valueOf( fVal ) );
    sal_Int32 nX( rtl::OUString( aVal.getStr() ).indexOf( '.', 0 ) );
    if ( nX > 0 )
        aVal.setLength( nX + 2 );
    return aVal.makeStringAndClear();
}

void ExportDialog::updatePreview()
{
    SetOutputSizePixel( maDialogSize );

    maFlButtons.SetSizePixel( Size( maRectFlButtons.GetWidth(), maRectFlButtons.GetHeight() ) );
    maBtnHelp.SetPosPixel( Point( maRectBtnHelp.Left(), maRectBtnHelp.Top() ) );
    maBtnOK.SetPosPixel( Point( maRectBtnOK.Left(), maRectBtnOK.Top() ) );
    maBtnCancel.SetPosPixel( Point( maRectBtnCancel.Left(), maRectBtnCancel.Top() ) );
}

void ExportDialog::updateControls()
{
    GetGraphicStream();

    // Size Controls
    if ( !mbIsPixelFormat )
    {
        awt::Size aSize100thmm( maSize );
        Size aSize( LogicToLogic( Size( aSize100thmm.Width * 100, aSize100thmm.Height * 100 ), MAP_100TH_MM,
            MapMode( GetMapUnit( maLbSizeX.GetSelectEntryPos() ) ) ) );
        maMfSizeX.SetValue( aSize.Width() );
        maMfSizeY.SetValue( aSize.Height() );
    }
    else
    {
        MapUnit aMapUnit( GetMapUnit( maLbSizeX.GetSelectEntryPos() ) );
        if ( aMapUnit == MAP_PIXEL )
        {   // calculating pixel count via resolution and original graphic size
            maMfSizeX.SetDecimalDigits( 0 );
            maMfSizeY.SetDecimalDigits( 0 );
            maMfSizeX.SetValue( maSize.Width );
            maMfSizeY.SetValue( maSize.Height );
        }
        else
        {
            maMfSizeX.SetDecimalDigits( 2 );
            maMfSizeY.SetDecimalDigits( 2 );
            double fRatio;
            switch( GetMapUnit( maLbSizeX.GetSelectEntryPos() ) )
            {
                case MAP_INCH : fRatio = static_cast< double >( maResolution.Width ) * 0.0254; break;
                case MAP_MM :   fRatio = static_cast< double >( maResolution.Width ) * 0.001; break;
                case MAP_POINT :fRatio = ( static_cast< double >( maResolution.Width ) * 0.0254 ) / 72.0; break;
                default:
                case MAP_CM :   fRatio = static_cast< double >( maResolution.Width ) * 0.01; break;
            }
            maMfSizeX.SetValue( static_cast< sal_Int32 >( ( static_cast< double >( maSize.Width * 100 ) / fRatio ) + 0.5 ) );
            maMfSizeY.SetValue( static_cast< sal_Int32 >( ( static_cast< double >( maSize.Height * 100 ) / fRatio ) + 0.5 ) );
        }
    }
    sal_Int32 nResolution = 0;
    switch( maLbResolution.GetSelectEntryPos() )
    {
        case 0 : nResolution = maResolution.Width / 100; break;     // pixels / cm
        case 2 : nResolution = maResolution.Width; break;           // pixels / meter
        default:
        case 1 : nResolution = static_cast< sal_Int32 >(maResolution.Width * 0.0254); break;    // pixels / inch
    }
    maNfResolution.SetValue( nResolution );

    if ( maSbCompression.IsVisible() )
        maSbCompression.SetThumbPos( maNfCompression.GetValue() );

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
        maFtEstimatedSize.SetText( aEst );
    }
    else
    {
        if ( mnMaxFilesizeForRealtimePreview )
        {
            String aEst( msEstimatedSizeVec );
            xub_StrLen nInd = aEst.Search( '%', 0 );
            aEst.Replace( nInd, 2, ImpValueOfInKB( nRealFileSize ) );
            maFtEstimatedSize.SetText( aEst );
        }
    }
    updatePreview();

    // EPS
    if ( maRbEPSLevel1.IsVisible() )
    {
        sal_Bool bEnabled = maRbEPSLevel1.IsChecked() == sal_False;
        maRbEPSColorFormat1.Enable( bEnabled );
        maRbEPSColorFormat2.Enable( bEnabled );
        maRbEPSCompressionLZW.Enable( bEnabled );
        maRbEPSCompressionNone.Enable( bEnabled );
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
        switch( GetMapUnit( maLbSizeX.GetSelectEntryPos() ) )
        {
            case MAP_INCH :     maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.0254 * maMfSizeX.GetValue() / 100.0 + 0.5 ); break;
            case MAP_CM :       maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.01 * maMfSizeX.GetValue() / 100.0 + 0.5 ); break;
            case MAP_MM :       maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.001 * maMfSizeX.GetValue() / 100.0 + 0.5 ); break;
            case MAP_POINT :    maSize.Width = static_cast< sal_Int32 >( static_cast< double >( maResolution.Width ) * 0.0254 * maMfSizeX.GetValue() / 100.0 * 72 + 0.5 ); break;
            default:
            case MAP_PIXEL :    maSize.Width = maMfSizeX.GetValue(); break;
        }
        maSize.Height = static_cast< sal_Int32 >( fRatio * maSize.Width + 0.5 );
    }
    else
    {
        Fraction aFract( 1, 100 );
        sal_Int32 nWidth = maMfSizeX.GetValue();
        sal_Int32 nHeight= static_cast< sal_Int32 >( nWidth * fRatio );
        const Size aSource( static_cast< sal_Int32 >( nWidth ), static_cast< sal_Int32 >( nHeight ) );
        MapMode aSourceMapMode( GetMapUnit( maLbSizeX.GetSelectEntryPos() ),Point(), aFract, aFract );
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
        switch( GetMapUnit( maLbSizeX.GetSelectEntryPos() ) )
        {
            case MAP_INCH :     maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.0254 * maMfSizeY.GetValue() / 100.0 + 0.5 ); break;
            case MAP_CM :       maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.01 * maMfSizeY.GetValue() / 100.0 + 0.5 ); break;
            case MAP_MM :       maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.001 * maMfSizeY.GetValue() / 100.0 + 0.5 ); break;
            case MAP_POINT :    maSize.Height = static_cast< sal_Int32 >( static_cast< double >( maResolution.Height ) * 0.0254 * maMfSizeY.GetValue() / 100.0 * 72 + 0.5 ); break;
            default:
            case MAP_PIXEL :    maSize.Height = maMfSizeY.GetValue(); break;
        }
        maSize.Width = static_cast< sal_Int32 >( fRatio * maSize.Height + 0.5 );
    }
    else
    {
        Fraction aFract( 1, 100 );
        sal_Int32 nHeight= maMfSizeY.GetValue();
        sal_Int32 nWidth = static_cast< sal_Int32 >( nHeight * fRatio );
        const Size aSource( static_cast< sal_Int32 >( nWidth ), static_cast< sal_Int32 >( nHeight ) );
        MapMode aSourceMapMode( GetMapUnit( maLbSizeX.GetSelectEntryPos() ),Point(), aFract, aFract );
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
    sal_Int32 nResolution = maNfResolution.GetValue();
    if ( maLbResolution.GetSelectEntryPos() == 0 )      // pixels / cm
        nResolution *= 100;
    else if ( maLbResolution.GetSelectEntryPos() == 1 ) // pixels / inch
        nResolution = static_cast< sal_Int32 >( ( ( static_cast< double >( nResolution ) + 0.5 ) / 0.0254 ) );
    maResolution.Width = nResolution;
    maResolution.Height= nResolution;

    updateControls();
    return 0;
}

IMPL_LINK_NOARG(ExportDialog, SbCompressionUpdateHdl)
{
    maNfCompression.SetValue( maSbCompression.GetThumbPos() );
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
