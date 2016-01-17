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
#include <vcl/bitmap.hxx>
#include <vcl/window.hxx>
#include <sal/macros.h>
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
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
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
#include <vcl/graphic.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <vcl/dibtools.hxx>

using namespace ::com::sun::star;


//  class VCLUnoHelper


uno::Reference< css::awt::XToolkit> VCLUnoHelper::CreateToolkit()
{
    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    uno::Reference< awt::XToolkit> xToolkit( awt::Toolkit::create(xContext), uno::UNO_QUERY_THROW );
    return xToolkit;
}

BitmapEx VCLUnoHelper::GetBitmap( const css::uno::Reference< css::awt::XBitmap>& rxBitmap )
{
    BitmapEx aBmp;

    css::uno::Reference< css::graphic::XGraphic > xGraphic( rxBitmap, css::uno::UNO_QUERY );
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
                css::uno::Sequence<sal_Int8> aBytes = rxBitmap->getDIB();
                SvMemoryStream aMem( aBytes.getArray(), aBytes.getLength(), StreamMode::READ );
                ReadDIB(aDIB, aMem, true);
            }
            {
                css::uno::Sequence<sal_Int8> aBytes = rxBitmap->getMaskDIB();
                SvMemoryStream aMem( aBytes.getArray(), aBytes.getLength(), StreamMode::READ );
                ReadDIB(aMask, aMem, true);
            }
            aBmp = BitmapEx( aDIB, aMask );
        }
    }
    return aBmp;
}

css::uno::Reference< css::awt::XBitmap> VCLUnoHelper::CreateBitmap( const BitmapEx& rBitmap )
{
    Graphic aGraphic( rBitmap );
    css::uno::Reference< css::awt::XBitmap> xBmp( aGraphic.GetXGraphic(), css::uno::UNO_QUERY );
    return xBmp;
}

VclPtr< vcl::Window > VCLUnoHelper::GetWindow( const css::uno::Reference< css::awt::XWindow>& rxWindow )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( rxWindow );
    return pVCLXWindow ? pVCLXWindow->GetWindow() : VclPtr< vcl::Window >();
}

VclPtr< vcl::Window > VCLUnoHelper::GetWindow( const css::uno::Reference< css::awt::XWindow2>& rxWindow )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( rxWindow );
    return pVCLXWindow ? pVCLXWindow->GetWindow() : VclPtr< vcl::Window >();
}

VclPtr< vcl::Window > VCLUnoHelper::GetWindow( const css::uno::Reference< css::awt::XWindowPeer>& rxWindow )
{
    VCLXWindow* pVCLXWindow = VCLXWindow::GetImplementation( rxWindow );
    return pVCLXWindow ? pVCLXWindow->GetWindow() : VclPtr< vcl::Window >();
}

vcl::Region VCLUnoHelper::GetRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion )
{
    vcl::Region aRegion;
    VCLXRegion* pVCLRegion = VCLXRegion::GetImplementation( rxRegion );
    if ( pVCLRegion )
        aRegion = pVCLRegion->GetRegion();
    else
    {
        css::uno::Sequence< css::awt::Rectangle > aRects = rxRegion->getRectangles();
        sal_Int32 nRects = aRects.getLength();
        for ( sal_Int32 n = 0; n < nRects; n++ )
            aRegion.Union( VCLRectangle( aRects.getArray()[n] ) );
    }
    return aRegion;
}

css::uno::Reference< css::awt::XWindow> VCLUnoHelper::GetInterface( vcl::Window* pWindow )
{
    css::uno::Reference< css::awt::XWindow > xWin;
    if ( pWindow )
    {
        css::uno::Reference< css::awt::XWindowPeer> xPeer = pWindow->GetComponentInterface();
        xWin.set(xPeer, css::uno::UNO_QUERY);
    }
    return xWin;
}

OutputDevice* VCLUnoHelper::GetOutputDevice( const css::uno::Reference< css::awt::XDevice>& rxDevice )
{
    OutputDevice* pOutDev = nullptr;
    VCLXDevice* pDev = VCLXDevice::GetImplementation( rxDevice );
    if ( pDev )
        pOutDev = pDev->GetOutputDevice();
    return pOutDev;
}

OutputDevice* VCLUnoHelper::GetOutputDevice( const css::uno::Reference< css::awt::XGraphics>& rxGraphics )
{
    OutputDevice* pOutDev = nullptr;
    VCLXGraphics* pGrf = VCLXGraphics::GetImplementation( rxGraphics );
    if ( pGrf )
        pOutDev = pGrf->GetOutputDevice();
    return pOutDev;
}

tools::Polygon VCLUnoHelper::CreatePolygon( const css::uno::Sequence< sal_Int32 >& DataX,
                                            const css::uno::Sequence< sal_Int32 >& DataY )
{
    sal_Int32 nLen = DataX.getLength();
    const sal_Int32* pDataX = DataX.getConstArray();
    const sal_Int32* pDataY = DataY.getConstArray();
    tools::Polygon aPoly( (sal_uInt16) nLen );
    for ( sal_Int32 n = 0; n < nLen; n++ )
    {
        Point aPnt;
        aPnt.X() = pDataX[n];
        aPnt.Y() = pDataY[n];
        aPoly[n] = aPnt;
    }
    return aPoly;
}

css::uno::Reference< css::awt::XControlContainer> VCLUnoHelper::CreateControlContainer( vcl::Window* pWindow )
{
    UnoControlContainer* pContainer = new UnoControlContainer( pWindow->GetComponentInterface() );
    css::uno::Reference< css::awt::XControlContainer > x = pContainer;

    UnoControlModel* pContainerModel = new UnoControlContainerModel( ::comphelper::getProcessComponentContext() );
    pContainer->setModel( static_cast<css::awt::XControlModel*>(pContainerModel) );

    return x;
}

float VCLUnoHelper::ConvertFontWidth( FontWidth eWidth )
{
    if( eWidth == WIDTH_DONTKNOW )
        return css::awt::FontWidth::DONTKNOW;
    else if( eWidth == WIDTH_ULTRA_CONDENSED )
        return css::awt::FontWidth::ULTRACONDENSED;
    else if( eWidth == WIDTH_EXTRA_CONDENSED )
        return css::awt::FontWidth::EXTRACONDENSED;
    else if( eWidth == WIDTH_CONDENSED )
        return css::awt::FontWidth::CONDENSED;
    else if( eWidth == WIDTH_SEMI_CONDENSED )
        return css::awt::FontWidth::SEMICONDENSED;
    else if( eWidth == WIDTH_NORMAL )
        return css::awt::FontWidth::NORMAL;
    else if( eWidth == WIDTH_SEMI_EXPANDED )
        return css::awt::FontWidth::SEMIEXPANDED;
    else if( eWidth == WIDTH_EXPANDED )
        return css::awt::FontWidth::EXPANDED;
    else if( eWidth == WIDTH_EXTRA_EXPANDED )
        return css::awt::FontWidth::EXTRAEXPANDED;
    else if( eWidth == WIDTH_ULTRA_EXPANDED )
        return css::awt::FontWidth::ULTRAEXPANDED;

    OSL_FAIL( "Unknown FontWidth" );
    return css::awt::FontWidth::DONTKNOW;
}

FontWidth VCLUnoHelper::ConvertFontWidth( float f )
{
    if( f <= css::awt::FontWidth::DONTKNOW )
        return WIDTH_DONTKNOW;
    else if( f <= css::awt::FontWidth::ULTRACONDENSED )
        return WIDTH_ULTRA_CONDENSED;
    else if( f <= css::awt::FontWidth::EXTRACONDENSED )
        return WIDTH_EXTRA_CONDENSED;
    else if( f <= css::awt::FontWidth::CONDENSED )
        return WIDTH_CONDENSED;
    else if( f <= css::awt::FontWidth::SEMICONDENSED )
        return WIDTH_SEMI_CONDENSED;
    else if( f <= css::awt::FontWidth::NORMAL )
        return WIDTH_NORMAL;
    else if( f <= css::awt::FontWidth::SEMIEXPANDED )
        return WIDTH_SEMI_EXPANDED;
    else if( f <= css::awt::FontWidth::EXPANDED )
        return WIDTH_EXPANDED;
    else if( f <= css::awt::FontWidth::EXTRAEXPANDED )
        return WIDTH_EXTRA_EXPANDED;
    else if( f <= css::awt::FontWidth::ULTRAEXPANDED )
        return WIDTH_ULTRA_EXPANDED;

    OSL_FAIL( "Unknown FontWidth" );
    return WIDTH_DONTKNOW;
}

float VCLUnoHelper::ConvertFontWeight( FontWeight eWeight )
{
    if( eWeight == WEIGHT_DONTKNOW )
        return css::awt::FontWeight::DONTKNOW;
    else if( eWeight == WEIGHT_THIN )
        return css::awt::FontWeight::THIN;
    else if( eWeight == WEIGHT_ULTRALIGHT )
        return css::awt::FontWeight::ULTRALIGHT;
    else if( eWeight == WEIGHT_LIGHT )
        return css::awt::FontWeight::LIGHT;
    else if( eWeight == WEIGHT_SEMILIGHT )
        return css::awt::FontWeight::SEMILIGHT;
    else if( ( eWeight == WEIGHT_NORMAL ) || ( eWeight == WEIGHT_MEDIUM ) )
        return css::awt::FontWeight::NORMAL;
    else if( eWeight == WEIGHT_SEMIBOLD )
        return css::awt::FontWeight::SEMIBOLD;
    else if( eWeight == WEIGHT_BOLD )
        return css::awt::FontWeight::BOLD;
    else if( eWeight == WEIGHT_ULTRABOLD )
        return css::awt::FontWeight::ULTRABOLD;
    else if( eWeight == WEIGHT_BLACK )
        return css::awt::FontWeight::BLACK;

    OSL_FAIL( "Unknown FontWeight" );
    return css::awt::FontWeight::DONTKNOW;
}

FontWeight VCLUnoHelper::ConvertFontWeight( float f )
{
    if( f <= css::awt::FontWeight::DONTKNOW )
        return WEIGHT_DONTKNOW;
    else if( f <= css::awt::FontWeight::THIN )
        return WEIGHT_THIN;
    else if( f <= css::awt::FontWeight::ULTRALIGHT )
        return WEIGHT_ULTRALIGHT;
    else if( f <= css::awt::FontWeight::LIGHT )
        return WEIGHT_LIGHT;
    else if( f <= css::awt::FontWeight::SEMILIGHT )
        return WEIGHT_SEMILIGHT;
    else if( f <= css::awt::FontWeight::NORMAL )
        return WEIGHT_NORMAL;
    else if( f <= css::awt::FontWeight::SEMIBOLD )
        return WEIGHT_SEMIBOLD;
    else if( f <= css::awt::FontWeight::BOLD )
        return WEIGHT_BOLD;
    else if( f <= css::awt::FontWeight::ULTRABOLD )
        return WEIGHT_ULTRABOLD;
    else if( f <= css::awt::FontWeight::BLACK )
        return WEIGHT_BLACK;

    OSL_FAIL( "Unknown FontWeight" );
    return WEIGHT_DONTKNOW;
}

css::awt::FontSlant VCLUnoHelper::ConvertFontSlant(FontItalic eItalic)
{
    css::awt::FontSlant eRet(css::awt::FontSlant_DONTKNOW);
    switch (eItalic)
    {
        case ITALIC_NONE:
            eRet = css::awt::FontSlant_NONE;
            break;
        case ITALIC_OBLIQUE:
            eRet = css::awt::FontSlant_OBLIQUE;
            break;
        case ITALIC_NORMAL:
            eRet = css::awt::FontSlant_ITALIC;
            break;
        case ITALIC_DONTKNOW:
            eRet = css::awt::FontSlant_DONTKNOW;
            break;
        case FontItalic_FORCE_EQUAL_SIZE:
            eRet = css::awt::FontSlant_MAKE_FIXED_SIZE;
            break;
    }
    return eRet;
}

FontItalic VCLUnoHelper::ConvertFontSlant(css::awt::FontSlant eSlant)
{
    FontItalic eRet = ITALIC_DONTKNOW;
    switch (eSlant)
    {
        case css::awt::FontSlant_NONE:
            eRet = ITALIC_NONE;
            break;
        case css::awt::FontSlant_OBLIQUE:
            eRet = ITALIC_OBLIQUE;
            break;
        case css::awt::FontSlant_ITALIC:
            eRet = ITALIC_NORMAL;
            break;
        case css::awt::FontSlant_DONTKNOW:
            eRet = ITALIC_DONTKNOW;
            break;
        case css::awt::FontSlant_REVERSE_OBLIQUE:
            //there is no vcl reverse oblique
            eRet = ITALIC_OBLIQUE;
            break;
        case css::awt::FontSlant_REVERSE_ITALIC:
            //there is no vcl reverse normal
            eRet = ITALIC_NORMAL;
            break;
        case css::awt::FontSlant_MAKE_FIXED_SIZE:
            eRet = FontItalic_FORCE_EQUAL_SIZE;
            break;
    }
    return eRet;
}

css::awt::FontDescriptor VCLUnoHelper::CreateFontDescriptor( const vcl::Font& rFont )
{
    css::awt::FontDescriptor aFD;
    aFD.Name = rFont.GetFamilyName();
    aFD.StyleName = rFont.GetStyleName();
    aFD.Height = (sal_Int16)rFont.GetSize().Height();
    aFD.Width = (sal_Int16)rFont.GetSize().Width();
    aFD.Family = sal::static_int_cast< sal_Int16 >(rFont.GetFamily());
    aFD.CharSet = rFont.GetCharSet();
    aFD.Pitch = sal::static_int_cast< sal_Int16 >(rFont.GetPitch());
    aFD.CharacterWidth = VCLUnoHelper::ConvertFontWidth( rFont.GetWidthType() );
    aFD.Weight= VCLUnoHelper::ConvertFontWeight( rFont.GetWeight() );
    aFD.Slant = VCLUnoHelper::ConvertFontSlant( rFont.GetItalic() );
    aFD.Underline = sal::static_int_cast< sal_Int16 >(rFont.GetUnderline());
    aFD.Strikeout = sal::static_int_cast< sal_Int16 >(rFont.GetStrikeout());
    aFD.Orientation = rFont.GetOrientation();
    aFD.Kerning = rFont.IsKerning();
    aFD.WordLineMode = rFont.IsWordLineMode();
    aFD.Type = 0;   // ??? => Nur an Metric...
    return aFD;
}

vcl::Font VCLUnoHelper::CreateFont( const css::awt::FontDescriptor& rDescr, const vcl::Font& rInitFont )
{
    vcl::Font aFont( rInitFont );
    if ( !rDescr.Name.isEmpty() )
        aFont.SetName( rDescr.Name );
    if ( !rDescr.StyleName.isEmpty() )
        aFont.SetStyleName( rDescr.StyleName );
    if ( rDescr.Height )
        aFont.SetSize( Size( rDescr.Width, rDescr.Height ) );
    if ( (FontFamily)rDescr.Family != FAMILY_DONTKNOW )
        aFont.SetFamily( (FontFamily)rDescr.Family );
    if ( (rtl_TextEncoding)rDescr.CharSet != RTL_TEXTENCODING_DONTKNOW )
        aFont.SetCharSet( (rtl_TextEncoding)rDescr.CharSet );
    if ( (FontPitch)rDescr.Pitch != PITCH_DONTKNOW )
        aFont.SetPitch( (FontPitch)rDescr.Pitch );
    if ( rDescr.CharacterWidth )
        aFont.SetWidthType( VCLUnoHelper::ConvertFontWidth( rDescr.CharacterWidth ) );
    if ( rDescr.Weight )
        aFont.SetWeight( VCLUnoHelper::ConvertFontWeight( rDescr.Weight ) );
    if ( rDescr.Slant != css::awt::FontSlant_DONTKNOW )
        aFont.SetItalic( VCLUnoHelper::ConvertFontSlant( rDescr.Slant ) );
    if ( (FontUnderline)rDescr.Underline != UNDERLINE_DONTKNOW )
        aFont.SetUnderline( (FontUnderline)rDescr.Underline );
    if ( (FontStrikeout)rDescr.Strikeout != STRIKEOUT_DONTKNOW )
        aFont.SetStrikeout( (FontStrikeout)rDescr.Strikeout );

    // Kein DONTKNOW
    aFont.SetOrientation( (short)rDescr.Orientation );
    aFont.SetKerning( static_cast<FontKerning>(rDescr.Kerning) );
    aFont.SetWordLineMode( rDescr.WordLineMode );

    return aFont;
}

vcl::Font VCLUnoHelper::CreateFont( const css::uno::Reference< css::awt::XFont >& rxFont )
{
    vcl::Font aFont;
    VCLXFont* pVCLXFont = VCLXFont::GetImplementation( rxFont );
    if ( pVCLXFont )
        aFont = pVCLXFont->GetFont();
    return aFont;
}


css::awt::SimpleFontMetric VCLUnoHelper::CreateFontMetric( const FontMetric& rFontMetric )
{
    css::awt::SimpleFontMetric aFM;
    aFM.Ascent = (sal_Int16)rFontMetric.GetAscent();
    aFM.Descent = (sal_Int16)rFontMetric.GetDescent();
    aFM.Leading = (sal_Int16)rFontMetric.GetIntLeading();
    aFM.Slant = (sal_Int16)rFontMetric.GetSlant();
    aFM.FirstChar = 0x0020;
    aFM.LastChar = 0xFFFD;
    return aFM;
}

bool VCLUnoHelper::IsZero(const css::awt::Rectangle& rRect)
{
    return ( !rRect.X && !rRect.Y && !rRect.Width && !rRect.Height );
}

MapUnit VCLUnoHelper::UnoEmbed2VCLMapUnit( sal_Int32 nUnoEmbedMapUnit )
{
    switch( nUnoEmbedMapUnit )
    {
        case css::embed::EmbedMapUnits::ONE_100TH_MM:
            return MAP_100TH_MM;
        case css::embed::EmbedMapUnits::ONE_10TH_MM:
            return MAP_10TH_MM;
        case css::embed::EmbedMapUnits::ONE_MM:
            return MAP_MM;
        case css::embed::EmbedMapUnits::ONE_CM:
            return MAP_CM;
        case css::embed::EmbedMapUnits::ONE_1000TH_INCH:
            return MAP_1000TH_INCH;
        case css::embed::EmbedMapUnits::ONE_100TH_INCH:
            return MAP_100TH_INCH;
        case css::embed::EmbedMapUnits::ONE_10TH_INCH:
            return MAP_10TH_INCH;
        case css::embed::EmbedMapUnits::ONE_INCH:
            return MAP_INCH;
        case css::embed::EmbedMapUnits::POINT:
            return MAP_POINT;
        case css::embed::EmbedMapUnits::TWIP:
            return MAP_TWIP;
        case css::embed::EmbedMapUnits::PIXEL:
            return MAP_PIXEL;
    }

    OSL_FAIL( "Unexpected UNO map mode is provided!\n" );
    return MAP_LASTENUMDUMMY;
}

sal_Int32 VCLUnoHelper::VCL2UnoEmbedMapUnit( MapUnit nVCLMapUnit )
{
    switch( nVCLMapUnit )
    {
        case MAP_100TH_MM:
            return css::embed::EmbedMapUnits::ONE_100TH_MM;
        case MAP_10TH_MM:
            return css::embed::EmbedMapUnits::ONE_10TH_MM;
        case MAP_MM:
            return css::embed::EmbedMapUnits::ONE_MM;
        case MAP_CM:
            return css::embed::EmbedMapUnits::ONE_CM;
        case MAP_1000TH_INCH:
            return css::embed::EmbedMapUnits::ONE_1000TH_INCH;
        case MAP_100TH_INCH:
            return css::embed::EmbedMapUnits::ONE_100TH_INCH;
        case MAP_10TH_INCH:
            return css::embed::EmbedMapUnits::ONE_10TH_INCH;
        case MAP_INCH:
            return css::embed::EmbedMapUnits::ONE_INCH;
        case MAP_POINT:
            return css::embed::EmbedMapUnits::POINT;
        case MAP_TWIP:
            return css::embed::EmbedMapUnits::TWIP;
        case MAP_PIXEL:
            return css::embed::EmbedMapUnits::PIXEL;
        default: ; // avoid compiler warning
    }

    OSL_FAIL( "Unexpected VCL map mode is provided!\n" );
    return -1;
}

using namespace ::com::sun::star::util;


//= file-local helpers

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
        for ( size_t i = 0; i < SAL_N_ELEMENTS( aUnits ); ++i )
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

//= MeasurementUnitConversion


sal_Int16 VCLUnoHelper::ConvertToMeasurementUnit( FieldUnit _nFieldUnit, sal_Int16 _nUNOToFieldValueFactor )
{
    return convertMeasurementUnit( (sal_Int16)_nFieldUnit, FieldUnitToMeasurementUnit, _nUNOToFieldValueFactor );
}


FieldUnit VCLUnoHelper::ConvertToFieldUnit( sal_Int16 _nMeasurementUnit, sal_Int16& _rFieldToUNOValueFactor )
{
    return (FieldUnit)convertMeasurementUnit( _nMeasurementUnit, MeasurementUnitToFieldUnit, _rFieldToUNOValueFactor );
}


MapUnit /* MapModeUnit */ VCLUnoHelper::ConvertToMapModeUnit(sal_Int16 /* com.sun.star.util.MeasureUnit.* */ _nMeasureUnit) throw (css::lang::IllegalArgumentException)
{
    MapUnit eMode;
    switch(_nMeasureUnit)
    {
    case css::util::MeasureUnit::MM_100TH:
        eMode = MAP_100TH_MM;
        break;

    case css::util::MeasureUnit::MM_10TH:
        eMode = MAP_10TH_MM;
        break;

    case css::util::MeasureUnit::MM:
        eMode = MAP_MM;
        break;

    case css::util::MeasureUnit::CM:
        eMode = MAP_CM;
        break;

    case css::util::MeasureUnit::INCH_1000TH:
        eMode = MAP_1000TH_INCH;
        break;

    case css::util::MeasureUnit::INCH_100TH:
        eMode = MAP_100TH_INCH;
        break;

    case css::util::MeasureUnit::INCH_10TH:
        eMode = MAP_10TH_INCH;
        break;

    case css::util::MeasureUnit::INCH:
        eMode = MAP_INCH;
        break;

    case css::util::MeasureUnit::POINT:
        eMode = MAP_POINT;
        break;

    case css::util::MeasureUnit::TWIP:
        eMode = MAP_TWIP;
        break;

    case css::util::MeasureUnit::PIXEL:
        eMode = MAP_PIXEL;
        break;

    case css::util::MeasureUnit::APPFONT:
        eMode = MAP_APPFONT;
        break;

    case css::util::MeasureUnit::SYSFONT:
        eMode = MAP_SYSFONT;
        break;

    default:
        throw css::lang::IllegalArgumentException("Unsupported measure unit.", nullptr, 1 );
    }
    return eMode;
}

::Size VCLUnoHelper::ConvertToVCLSize(css::awt::Size const& _aSize)
{
    ::Size aVCLSize(_aSize.Width, _aSize.Height);
    return aVCLSize;
}

css::awt::Size VCLUnoHelper::ConvertToAWTSize(::Size /* VCLSize */ const& _aSize)
{
    css::awt::Size aAWTSize(_aSize.Width(), _aSize.Height());
    return aAWTSize;
}


::Point VCLUnoHelper::ConvertToVCLPoint(css::awt::Point const& _aPoint)
{
    ::Point aVCLPoint(_aPoint.X, _aPoint.Y);
    return aVCLPoint;
}

css::awt::Point VCLUnoHelper::ConvertToAWTPoint(::Point /* VCLPoint */ const& _aPoint)
{
    css::awt::Point aAWTPoint(_aPoint.X(), _aPoint.Y());
    return aAWTPoint;
}

::Rectangle VCLUnoHelper::ConvertToVCLRect( css::awt::Rectangle const & _rRect )
{
    return ::Rectangle( _rRect.X, _rRect.Y, _rRect.X + _rRect.Width - 1, _rRect.Y + _rRect.Height - 1 );
}

css::awt::Rectangle VCLUnoHelper::ConvertToAWTRect( ::Rectangle const & _rRect )
{
    return css::awt::Rectangle( _rRect.Left(), _rRect.Top(), _rRect.GetWidth(), _rRect.GetHeight() );
}

awt::MouseEvent VCLUnoHelper::createMouseEvent( const ::MouseEvent& _rVclEvent, const uno::Reference< uno::XInterface >& _rxContext )
{
    awt::MouseEvent aMouseEvent;
    aMouseEvent.Source = _rxContext;

    aMouseEvent.Modifiers = 0;
    if ( _rVclEvent.IsShift() )
        aMouseEvent.Modifiers |= css::awt::KeyModifier::SHIFT;
    if ( _rVclEvent.IsMod1() )
        aMouseEvent.Modifiers |= css::awt::KeyModifier::MOD1;
    if ( _rVclEvent.IsMod2() )
        aMouseEvent.Modifiers |= css::awt::KeyModifier::MOD2;

    aMouseEvent.Buttons = 0;
    if ( _rVclEvent.IsLeft() )
        aMouseEvent.Buttons |= css::awt::MouseButton::LEFT;
    if ( _rVclEvent.IsRight() )
        aMouseEvent.Buttons |= css::awt::MouseButton::RIGHT;
    if ( _rVclEvent.IsMiddle() )
        aMouseEvent.Buttons |= css::awt::MouseButton::MIDDLE;

    aMouseEvent.X = _rVclEvent.GetPosPixel().X();
    aMouseEvent.Y = _rVclEvent.GetPosPixel().Y();
    aMouseEvent.ClickCount = _rVclEvent.GetClicks();
    aMouseEvent.PopupTrigger = sal_False;

    return aMouseEvent;
}

::MouseEvent VCLUnoHelper::createVCLMouseEvent( const awt::MouseEvent& _rAwtEvent )
{
    ::MouseEvent aMouseEvent( Point( _rAwtEvent.X, _rAwtEvent.Y ), _rAwtEvent.ClickCount,
                              ::MouseEventModifiers::NONE, _rAwtEvent.Buttons, _rAwtEvent.Modifiers );

    return aMouseEvent;
}

awt::KeyEvent VCLUnoHelper::createKeyEvent( const ::KeyEvent& _rVclEvent, const uno::Reference< uno::XInterface >& _rxContext )
{
    awt::KeyEvent aKeyEvent;
    aKeyEvent.Source = _rxContext;

    aKeyEvent.Modifiers = 0;
    if ( _rVclEvent.GetKeyCode().IsShift() )
        aKeyEvent.Modifiers |= awt::KeyModifier::SHIFT;
    if ( _rVclEvent.GetKeyCode().IsMod1() )
        aKeyEvent.Modifiers |= awt::KeyModifier::MOD1;
    if ( _rVclEvent.GetKeyCode().IsMod2() )
        aKeyEvent.Modifiers |= awt::KeyModifier::MOD2;
    if ( _rVclEvent.GetKeyCode().IsMod3() )
            aKeyEvent.Modifiers |= awt::KeyModifier::MOD3;

    aKeyEvent.KeyCode = _rVclEvent.GetKeyCode().GetCode();
    aKeyEvent.KeyChar = _rVclEvent.GetCharCode();
    aKeyEvent.KeyFunc = ::sal::static_int_cast< sal_Int16 >( _rVclEvent.GetKeyCode().GetFunction());

    return aKeyEvent;
}

::KeyEvent VCLUnoHelper::createVCLKeyEvent( const awt::KeyEvent& _rAwtEvent )
{
    sal_Unicode nChar = _rAwtEvent.KeyChar;
    vcl::KeyCode aKeyCode( _rAwtEvent.KeyCode, _rAwtEvent.Modifiers & awt::KeyModifier::SHIFT,
                           _rAwtEvent.Modifiers & awt::KeyModifier::MOD1,
                           _rAwtEvent.Modifiers & awt::KeyModifier::MOD2,
                           _rAwtEvent.Modifiers & awt::KeyModifier::MOD3 );

    return ::KeyEvent (nChar, aKeyCode);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
