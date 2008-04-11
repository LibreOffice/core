/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclunohelper.cxx,v $
 * $Revision: 1.18 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/window.hxx>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XPointer.hpp>
#include <com/sun/star/awt/SimpleFontMetric.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>

#include <com/sun/star/graphic/XGraphic.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/awt/vclxbitmap.hxx>
#include <toolkit/awt/vclxregion.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxgraphics.hxx>
#include <toolkit/awt/vclxpointer.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <toolkit/controls/unocontrolcontainermodel.hxx>

#include <vcl/graph.hxx>
#include <comphelper/processfactory.hxx>

//  ----------------------------------------------------
//  class VCLUnoHelper
//  ----------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit> VCLUnoHelper::CreateToolkit()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString::createFromAscii( szServiceName2_Toolkit ) );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit> xToolkit;
    if ( xI.is() )
        xToolkit = ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>( xI, ::com::sun::star::uno::UNO_QUERY );

    return xToolkit;
}

BitmapEx VCLUnoHelper::GetBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap>& rxBitmap )
{
    BitmapEx aBmp;

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > xGraphic( rxBitmap, ::com::sun::star::uno::UNO_QUERY );
    if( xGraphic.is() )
    {
        Graphic aGraphic( xGraphic );
        aBmp = aGraphic.GetBitmapEx();
    }
    else if ( rxBitmap.is() )
    {
        VCLXBitmap* pVCLBitmap = VCLXBitmap::GetImplementation( rxBitmap );
        if ( pVCLBitmap )
            aBmp = pVCLBitmap->GetBitmap();
        else
        {
            Bitmap aDIB, aMask;
            {
                ::com::sun::star::uno::Sequence<sal_Int8> aBytes = rxBitmap->getDIB();
                SvMemoryStream aMem( (char*) aBytes.getArray(), aBytes.getLength(), STREAM_READ );
                aMem >> aDIB;
            }
            {
                ::com::sun::star::uno::Sequence<sal_Int8> aBytes = rxBitmap->getMaskDIB();
                SvMemoryStream aMem( (char*) aBytes.getArray(), aBytes.getLength(), STREAM_READ );
                aMem >> aMask;
            }
            aBmp = BitmapEx( aDIB, aMask );
        }
    }
    return aBmp;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap> VCLUnoHelper::CreateBitmap( const BitmapEx& rBitmap )
{
    Graphic aGraphic( rBitmap );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap> xBmp( aGraphic.GetXGraphic(), ::com::sun::star::uno::UNO_QUERY );
    return xBmp;
}

Window* VCLUnoHelper::GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>& rxWindow )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( rxWindow );
    return pVCLXWindow ? pVCLXWindow->GetWindow() : NULL;
}

Window* VCLUnoHelper::GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow2>& rxWindow )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( rxWindow );
    return pVCLXWindow ? pVCLXWindow->GetWindow() : NULL;
}

Window* VCLUnoHelper::GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer>& rxWindow )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( rxWindow );
    return pVCLXWindow ? pVCLXWindow->GetWindow() : NULL;
}

Region VCLUnoHelper::GetRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion )
{
    Region aRegion;
    VCLXRegion* pVCLRegion = VCLXRegion::GetImplementation( rxRegion );
    if ( pVCLRegion )
        aRegion = pVCLRegion->GetRegion();
    else
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::awt::Rectangle > aRects = rxRegion->getRectangles();
        sal_Int32 nRects = aRects.getLength();
        for ( sal_Int32 n = 0; n < nRects; n++ )
            aRegion.Union( VCLRectangle( aRects.getArray()[n] ) );
    }
    return aRegion;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow> VCLUnoHelper::GetInterface( Window* pWindow )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xWin;
    if ( pWindow )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> xPeer = pWindow->GetComponentInterface();
        xWin = xWin.query( xPeer );
    }
    return xWin;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer> VCLUnoHelper::CreatePointer()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer> xPointer = new VCLXPointer;
    return xPointer;
}

OutputDevice* VCLUnoHelper::GetOutputDevice( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice>& rxDevice )
{
    OutputDevice* pOutDev = NULL;
    VCLXDevice* pDev = VCLXDevice::GetImplementation( rxDevice );
    if ( pDev )
        pOutDev = pDev->GetOutputDevice();
    return pOutDev;
}

OutputDevice* VCLUnoHelper::GetOutputDevice( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics>& rxGraphics )
{
    OutputDevice* pOutDev = NULL;
    VCLXGraphics* pGrf = VCLXGraphics::GetImplementation( rxGraphics );
    if ( pGrf )
        pOutDev = pGrf->GetOutputDevice();
    return pOutDev;
}

Polygon VCLUnoHelper::CreatePolygon( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY )
{
    sal_uInt32 nLen = DataX.getLength();
    const sal_Int32* pDataX = DataX.getConstArray();
    const sal_Int32* pDataY = DataY.getConstArray();
    Polygon aPoly( (sal_uInt16) nLen );
    for ( sal_uInt16 n = 0; n < nLen; n++ )
    {
        Point aPnt;
        aPnt.X() = pDataX[n];
        aPnt.Y() = pDataY[n];
        aPoly[n] = aPnt;
    }
    return aPoly;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer> VCLUnoHelper::CreateControlContainer( Window* pWindow )
{
    UnoControlContainer* pContainer = new UnoControlContainer( pWindow->GetComponentInterface( sal_True ) );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > x = pContainer;

    UnoControlModel* pContainerModel = new UnoControlContainerModel;
    pContainer->setModel( (::com::sun::star::awt::XControlModel*)pContainerModel );

    return x;
}

float VCLUnoHelper::ConvertFontWidth( FontWidth eWidth )
{
    if( eWidth == WIDTH_DONTKNOW )
        return ::com::sun::star::awt::FontWidth::DONTKNOW;
    else if( eWidth == WIDTH_ULTRA_CONDENSED )
        return ::com::sun::star::awt::FontWidth::ULTRACONDENSED;
    else if( eWidth == WIDTH_EXTRA_CONDENSED )
        return ::com::sun::star::awt::FontWidth::EXTRACONDENSED;
    else if( eWidth == WIDTH_CONDENSED )
        return ::com::sun::star::awt::FontWidth::CONDENSED;
    else if( eWidth == WIDTH_SEMI_CONDENSED )
        return ::com::sun::star::awt::FontWidth::SEMICONDENSED;
    else if( eWidth == WIDTH_NORMAL )
        return ::com::sun::star::awt::FontWidth::NORMAL;
    else if( eWidth == WIDTH_SEMI_EXPANDED )
        return ::com::sun::star::awt::FontWidth::SEMIEXPANDED;
    else if( eWidth == WIDTH_EXPANDED )
        return ::com::sun::star::awt::FontWidth::EXPANDED;
    else if( eWidth == WIDTH_EXTRA_EXPANDED )
        return ::com::sun::star::awt::FontWidth::EXTRAEXPANDED;
    else if( eWidth == WIDTH_ULTRA_EXPANDED )
        return ::com::sun::star::awt::FontWidth::ULTRAEXPANDED;

    DBG_ERROR( "Unknown FontWidth" );
    return ::com::sun::star::awt::FontWidth::DONTKNOW;
}

FontWidth VCLUnoHelper::ConvertFontWidth( float f )
{
    if( f <= ::com::sun::star::awt::FontWidth::DONTKNOW )
        return WIDTH_DONTKNOW;
    else if( f <= ::com::sun::star::awt::FontWidth::ULTRACONDENSED )
        return WIDTH_ULTRA_CONDENSED;
    else if( f <= ::com::sun::star::awt::FontWidth::EXTRACONDENSED )
        return WIDTH_EXTRA_CONDENSED;
    else if( f <= ::com::sun::star::awt::FontWidth::CONDENSED )
        return WIDTH_CONDENSED;
    else if( f <= ::com::sun::star::awt::FontWidth::SEMICONDENSED )
        return WIDTH_SEMI_CONDENSED;
    else if( f <= ::com::sun::star::awt::FontWidth::NORMAL )
        return WIDTH_NORMAL;
    else if( f <= ::com::sun::star::awt::FontWidth::SEMIEXPANDED )
        return WIDTH_SEMI_EXPANDED;
    else if( f <= ::com::sun::star::awt::FontWidth::EXPANDED )
        return WIDTH_EXPANDED;
    else if( f <= ::com::sun::star::awt::FontWidth::EXTRAEXPANDED )
        return WIDTH_EXTRA_EXPANDED;
    else if( f <= ::com::sun::star::awt::FontWidth::ULTRAEXPANDED )
        return WIDTH_ULTRA_EXPANDED;

    DBG_ERROR( "Unknown FontWidth" );
    return WIDTH_DONTKNOW;
}

float VCLUnoHelper::ConvertFontWeight( FontWeight eWeight )
{
    if( eWeight == WEIGHT_DONTKNOW )
        return ::com::sun::star::awt::FontWeight::DONTKNOW;
    else if( eWeight == WEIGHT_THIN )
        return ::com::sun::star::awt::FontWeight::THIN;
    else if( eWeight == WEIGHT_ULTRALIGHT )
        return ::com::sun::star::awt::FontWeight::ULTRALIGHT;
    else if( eWeight == WEIGHT_LIGHT )
        return ::com::sun::star::awt::FontWeight::LIGHT;
    else if( eWeight == WEIGHT_SEMILIGHT )
        return ::com::sun::star::awt::FontWeight::SEMILIGHT;
    else if( ( eWeight == WEIGHT_NORMAL ) || ( eWeight == WEIGHT_MEDIUM ) )
        return ::com::sun::star::awt::FontWeight::NORMAL;
    else if( eWeight == WEIGHT_SEMIBOLD )
        return ::com::sun::star::awt::FontWeight::SEMIBOLD;
    else if( eWeight == WEIGHT_BOLD )
        return ::com::sun::star::awt::FontWeight::BOLD;
    else if( eWeight == WEIGHT_ULTRABOLD )
        return ::com::sun::star::awt::FontWeight::ULTRABOLD;
    else if( eWeight == WEIGHT_BLACK )
        return ::com::sun::star::awt::FontWeight::BLACK;

    DBG_ERROR( "Unknown FontWeigth" );
    return ::com::sun::star::awt::FontWeight::DONTKNOW;
}

FontWeight VCLUnoHelper::ConvertFontWeight( float f )
{
    if( f <= ::com::sun::star::awt::FontWeight::DONTKNOW )
        return WEIGHT_DONTKNOW;
    else if( f <= ::com::sun::star::awt::FontWeight::THIN )
        return WEIGHT_THIN;
    else if( f <= ::com::sun::star::awt::FontWeight::ULTRALIGHT )
        return WEIGHT_ULTRALIGHT;
    else if( f <= ::com::sun::star::awt::FontWeight::LIGHT )
        return WEIGHT_LIGHT;
    else if( f <= ::com::sun::star::awt::FontWeight::SEMILIGHT )
        return WEIGHT_SEMILIGHT;
    else if( f <= ::com::sun::star::awt::FontWeight::NORMAL )
        return WEIGHT_NORMAL;
    else if( f <= ::com::sun::star::awt::FontWeight::SEMIBOLD )
        return WEIGHT_SEMIBOLD;
    else if( f <= ::com::sun::star::awt::FontWeight::BOLD )
        return WEIGHT_BOLD;
    else if( f <= ::com::sun::star::awt::FontWeight::ULTRABOLD )
        return WEIGHT_ULTRABOLD;
    else if( f <= ::com::sun::star::awt::FontWeight::BLACK )
        return WEIGHT_BLACK;

    DBG_ERROR( "Unknown FontWeigth" );
    return WEIGHT_DONTKNOW;
}


::com::sun::star::awt::FontDescriptor VCLUnoHelper::CreateFontDescriptor( const Font& rFont )
{
    ::com::sun::star::awt::FontDescriptor aFD;
    aFD.Name = rFont.GetName();
    aFD.StyleName = rFont.GetStyleName();
    aFD.Height = (sal_Int16)rFont.GetSize().Height();
    aFD.Width = (sal_Int16)rFont.GetSize().Width();
    aFD.Family = sal::static_int_cast< sal_Int16 >(rFont.GetFamily());
    aFD.CharSet = rFont.GetCharSet();
    aFD.Pitch = sal::static_int_cast< sal_Int16 >(rFont.GetPitch());
    aFD.CharacterWidth = VCLUnoHelper::ConvertFontWidth( rFont.GetWidthType() );
    aFD.Weight= VCLUnoHelper::ConvertFontWeight( rFont.GetWeight() );
    aFD.Slant = (::com::sun::star::awt::FontSlant)rFont.GetItalic();
    aFD.Underline = sal::static_int_cast< sal_Int16 >(rFont.GetUnderline());
    aFD.Strikeout = sal::static_int_cast< sal_Int16 >(rFont.GetStrikeout());
    aFD.Orientation = rFont.GetOrientation();
    aFD.Kerning = rFont.IsKerning();
    aFD.WordLineMode = rFont.IsWordLineMode();
    aFD.Type = 0;   // ??? => Nur an Metric...
    return aFD;
}

Font VCLUnoHelper::CreateFont( const ::com::sun::star::awt::FontDescriptor& rDescr, const Font& rInitFont )
{
    Font aFont( rInitFont );
    if ( rDescr.Name.getLength() )
        aFont.SetName( rDescr.Name );
    if ( rDescr.StyleName.getLength() )
        aFont.SetStyleName( rDescr.StyleName );
    if ( rDescr.Height )
        aFont.SetSize( Size( rDescr.Width, rDescr.Height ) );
    if ( (FontFamily)rDescr.Family != FAMILY_DONTKNOW )
        aFont.SetFamily( (FontFamily)rDescr.Family );
    if ( (CharSet)rDescr.CharSet != RTL_TEXTENCODING_DONTKNOW )
        aFont.SetCharSet( (CharSet)rDescr.CharSet );
    if ( (FontPitch)rDescr.Pitch != PITCH_DONTKNOW )
        aFont.SetPitch( (FontPitch)rDescr.Pitch );
    if ( rDescr.CharacterWidth )
        aFont.SetWidthType( VCLUnoHelper::ConvertFontWidth( rDescr.CharacterWidth ) );
    if ( rDescr.Weight )
        aFont.SetWeight( VCLUnoHelper::ConvertFontWeight( rDescr.Weight ) );
    if ( (FontItalic)rDescr.Slant != ITALIC_DONTKNOW )
        aFont.SetItalic( (FontItalic)rDescr.Slant );
    if ( (FontUnderline)rDescr.Underline != UNDERLINE_DONTKNOW )
        aFont.SetUnderline( (FontUnderline)rDescr.Underline );
    if ( (FontStrikeout)rDescr.Strikeout != STRIKEOUT_DONTKNOW )
        aFont.SetStrikeout( (FontStrikeout)rDescr.Strikeout );

    // Kein DONTKNOW
    aFont.SetOrientation( (short)rDescr.Orientation );
    aFont.SetKerning( rDescr.Kerning );
    aFont.SetWordLineMode( rDescr.WordLineMode );

    return aFont;
}

Font VCLUnoHelper::CreateFont( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >& rxFont )
{
    Font aFont;
    VCLXFont* pVCLXFont = VCLXFont::GetImplementation( rxFont );
    if ( pVCLXFont )
        aFont = pVCLXFont->GetFont();
    return aFont;
}


::com::sun::star::awt::SimpleFontMetric VCLUnoHelper::CreateFontMetric( const FontMetric& rFontMetric )
{
    ::com::sun::star::awt::SimpleFontMetric aFM;
    aFM.Ascent = (sal_Int16)rFontMetric.GetAscent();
    aFM.Descent = (sal_Int16)rFontMetric.GetDescent();
    aFM.Leading = (sal_Int16)rFontMetric.GetIntLeading();
    aFM.Slant = (sal_Int16)rFontMetric.GetSlant();
    aFM.FirstChar = 0x0020;
    aFM.LastChar = 0xFFFD;
    return aFM;
}

sal_Bool VCLUnoHelper::IsZero( ::com::sun::star::awt::Rectangle rRect )
{
    return ( !rRect.X && !rRect.Y && !rRect.Width && !rRect.Height );
}

MapUnit VCLUnoHelper::UnoEmbed2VCLMapUnit( sal_Int32 nUnoEmbedMapUnit )
{
    switch( nUnoEmbedMapUnit )
    {
        case ::com::sun::star::embed::EmbedMapUnits::ONE_100TH_MM:
            return MAP_100TH_MM;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_10TH_MM:
            return MAP_10TH_MM;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_MM:
            return MAP_MM;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_CM:
            return MAP_CM;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_1000TH_INCH:
            return MAP_1000TH_INCH;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_100TH_INCH:
            return MAP_100TH_INCH;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_10TH_INCH:
            return MAP_10TH_INCH;
        case ::com::sun::star::embed::EmbedMapUnits::ONE_INCH:
            return MAP_INCH;
        case ::com::sun::star::embed::EmbedMapUnits::POINT:
            return MAP_POINT;
        case ::com::sun::star::embed::EmbedMapUnits::TWIP:
            return MAP_TWIP;
        case ::com::sun::star::embed::EmbedMapUnits::PIXEL:
            return MAP_PIXEL;
    }

    OSL_ENSURE( sal_False, "Unexpected UNO map mode is provided!\n" );
    return MAP_LASTENUMDUMMY;
}

sal_Int32 VCLUnoHelper::VCL2UnoEmbedMapUnit( MapUnit nVCLMapUnit )
{
    switch( nVCLMapUnit )
    {
        case MAP_100TH_MM:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_100TH_MM;
        case MAP_10TH_MM:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_10TH_MM;
        case MAP_MM:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_MM;
        case MAP_CM:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_CM;
        case MAP_1000TH_INCH:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_1000TH_INCH;
        case MAP_100TH_INCH:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_100TH_INCH;
        case MAP_10TH_INCH:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_10TH_INCH;
        case MAP_INCH:
            return ::com::sun::star::embed::EmbedMapUnits::ONE_INCH;
        case MAP_POINT:
            return ::com::sun::star::embed::EmbedMapUnits::POINT;
        case MAP_TWIP:
            return ::com::sun::star::embed::EmbedMapUnits::TWIP;
        case MAP_PIXEL:
            return ::com::sun::star::embed::EmbedMapUnits::PIXEL;
        default: ; // avoid compiler warning
    }

    OSL_ENSURE( sal_False, "Unexpected VCL map mode is provided!\n" );
    return -1;
}

using namespace ::com::sun::star::util;

//====================================================================
//= file-local helpers
//====================================================================
namespace
{
    enum UnitConversionDirection
    {
        FieldUnitToMeasurementUnit,
        MeasurementUnitToFieldUnit
    };

    sal_Int16 convertMeasurementUnit( sal_Int16 _nUnit, UnitConversionDirection eDirection, sal_Int16& _rFieldToUNOValueFactor )
    {
        static struct _unit_table
        {
            FieldUnit eFieldUnit;
            sal_Int16 nMeasurementUnit;
            sal_Int16 nFieldToMeasureFactor;
        } aUnits[] = {
            { FUNIT_NONE,       -1 , -1},
            { FUNIT_MM,         MeasureUnit::MM,            1 },    // must precede MM_10TH
            { FUNIT_MM,         MeasureUnit::MM_10TH,       10 },
            { FUNIT_100TH_MM,   MeasureUnit::MM_100TH,      1 },
            { FUNIT_CM,         MeasureUnit::CM,            1 },
            { FUNIT_M,          MeasureUnit::M,             1 },
            { FUNIT_KM,         MeasureUnit::KM,            1 },
            { FUNIT_TWIP,       MeasureUnit::TWIP,          1 },
            { FUNIT_POINT,      MeasureUnit::POINT,         1 },
            { FUNIT_PICA,       MeasureUnit::PICA,          1 },
            { FUNIT_INCH,       MeasureUnit::INCH,          1 },    // must precede INCH_*TH
            { FUNIT_INCH,       MeasureUnit::INCH_10TH,     10 },
            { FUNIT_INCH,       MeasureUnit::INCH_100TH,    100 },
            { FUNIT_INCH,       MeasureUnit::INCH_1000TH,   1000 },
            { FUNIT_FOOT,       MeasureUnit::FOOT,          1 },
            { FUNIT_MILE,       MeasureUnit::MILE,          1 },
        };
        for ( size_t i = 0; i < sizeof( aUnits ) / sizeof( aUnits[0] ); ++i )
        {
            if ( eDirection == FieldUnitToMeasurementUnit )
            {
                if ( ( aUnits[ i ].eFieldUnit == (FieldUnit)_nUnit ) && ( aUnits[ i ].nFieldToMeasureFactor == _rFieldToUNOValueFactor ) )
                    return aUnits[ i ].nMeasurementUnit;
            }
            else
            {
                if ( aUnits[ i ].nMeasurementUnit == _nUnit )
                {
                    _rFieldToUNOValueFactor = aUnits[ i ].nFieldToMeasureFactor;
                    return (sal_Int16)aUnits[ i ].eFieldUnit;
                }
            }
        }
        if ( eDirection == FieldUnitToMeasurementUnit )
            return -1;

        _rFieldToUNOValueFactor = 1;
        return (sal_Int16)FUNIT_NONE;
    }
}
//========================================================================
//= MeasurementUnitConversion
//========================================================================
//------------------------------------------------------------------------
sal_Int16 VCLUnoHelper::ConvertToMeasurementUnit( FieldUnit _nFieldUnit, sal_Int16 _nUNOToFieldValueFactor )
{
    return convertMeasurementUnit( (sal_Int16)_nFieldUnit, FieldUnitToMeasurementUnit, _nUNOToFieldValueFactor );
}

//------------------------------------------------------------------------
FieldUnit VCLUnoHelper::ConvertToFieldUnit( sal_Int16 _nMeasurementUnit, sal_Int16& _rFieldToUNOValueFactor )
{
    return (FieldUnit)convertMeasurementUnit( _nMeasurementUnit, MeasurementUnitToFieldUnit, _rFieldToUNOValueFactor );
}
