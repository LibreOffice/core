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
#include <vcl/event.hxx>
#include <vcl/window.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/metric.hxx>
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
#include <vcl/graph.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <vcl/dibtools.hxx>

using namespace ::com::sun::star;


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
    VclPtr<OutputDevice> pOutDev;
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
    tools::Polygon aPoly( static_cast<sal_uInt16>(nLen) );
    for ( sal_Int32 n = 0; n < nLen; n++ )
    {
        Point aPnt;
        aPnt.setX( pDataX[n] );
        aPnt.setY( pDataY[n] );
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

css::awt::FontDescriptor VCLUnoHelper::CreateFontDescriptor( const vcl::Font& rFont )
{
    css::awt::FontDescriptor aFD;
    aFD.Name = rFont.GetFamilyName();
    aFD.StyleName = rFont.GetStyleName();
    aFD.Height = static_cast<sal_Int16>(rFont.GetFontSize().Height());
    aFD.Width = static_cast<sal_Int16>(rFont.GetFontSize().Width());
    aFD.Family = sal::static_int_cast< sal_Int16 >(rFont.GetFamilyType());
    aFD.CharSet = rFont.GetCharSet();
    aFD.Pitch = sal::static_int_cast< sal_Int16 >(rFont.GetPitch());
    aFD.CharacterWidth = vcl::unohelper::ConvertFontWidth(rFont.GetWidthType());
    aFD.Weight = vcl::unohelper::ConvertFontWeight(rFont.GetWeight());
    aFD.Slant = vcl::unohelper::ConvertFontSlant(rFont.GetItalic());
    aFD.Underline = sal::static_int_cast< sal_Int16 >(rFont.GetUnderline());
    aFD.Strikeout = sal::static_int_cast< sal_Int16 >(rFont.GetStrikeout());
    aFD.Orientation = rFont.GetOrientation();
    aFD.Kerning = rFont.IsKerning();
    aFD.WordLineMode = rFont.IsWordLineMode();
    aFD.Type = 0;   // ??? => Only in Metric...
    return aFD;
}

vcl::Font VCLUnoHelper::CreateFont( const css::awt::FontDescriptor& rDescr, const vcl::Font& rInitFont )
{
    vcl::Font aFont( rInitFont );
    if ( !rDescr.Name.isEmpty() )
        aFont.SetFamilyName( rDescr.Name );
    if ( !rDescr.StyleName.isEmpty() )
        aFont.SetStyleName( rDescr.StyleName );
    if ( rDescr.Height )
        aFont.SetFontSize( Size( rDescr.Width, rDescr.Height ) );
    if ( static_cast<FontFamily>(rDescr.Family) != FAMILY_DONTKNOW )
        aFont.SetFamily( static_cast<FontFamily>(rDescr.Family) );
    if ( static_cast<rtl_TextEncoding>(rDescr.CharSet) != RTL_TEXTENCODING_DONTKNOW )
        aFont.SetCharSet( static_cast<rtl_TextEncoding>(rDescr.CharSet) );
    if ( static_cast<FontPitch>(rDescr.Pitch) != PITCH_DONTKNOW )
        aFont.SetPitch( static_cast<FontPitch>(rDescr.Pitch) );
    if ( rDescr.CharacterWidth )
        aFont.SetWidthType(vcl::unohelper::ConvertFontWidth(rDescr.CharacterWidth));
    if ( rDescr.Weight )
        aFont.SetWeight(vcl::unohelper::ConvertFontWeight(rDescr.Weight));
    if ( rDescr.Slant != css::awt::FontSlant_DONTKNOW )
        aFont.SetItalic(vcl::unohelper::ConvertFontSlant(rDescr.Slant));
    if ( static_cast<FontLineStyle>(rDescr.Underline) != LINESTYLE_DONTKNOW )
        aFont.SetUnderline( static_cast<FontLineStyle>(rDescr.Underline) );
    if ( static_cast<FontStrikeout>(rDescr.Strikeout) != STRIKEOUT_DONTKNOW )
        aFont.SetStrikeout( static_cast<FontStrikeout>(rDescr.Strikeout) );

    // Not DONTKNOW
    aFont.SetOrientation( static_cast<short>(rDescr.Orientation) );
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
    aFM.Ascent = static_cast<sal_Int16>(rFontMetric.GetAscent());
    aFM.Descent = static_cast<sal_Int16>(rFontMetric.GetDescent());
    aFM.Leading = static_cast<sal_Int16>(rFontMetric.GetInternalLeading());
    aFM.Slant = static_cast<sal_Int16>(rFontMetric.GetSlant());
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
            return MapUnit::Map100thMM;
        case css::embed::EmbedMapUnits::ONE_10TH_MM:
            return MapUnit::Map10thMM;
        case css::embed::EmbedMapUnits::ONE_MM:
            return MapUnit::MapMM;
        case css::embed::EmbedMapUnits::ONE_CM:
            return MapUnit::MapCM;
        case css::embed::EmbedMapUnits::ONE_1000TH_INCH:
            return MapUnit::Map1000thInch;
        case css::embed::EmbedMapUnits::ONE_100TH_INCH:
            return MapUnit::Map100thInch;
        case css::embed::EmbedMapUnits::ONE_10TH_INCH:
            return MapUnit::Map10thInch;
        case css::embed::EmbedMapUnits::ONE_INCH:
            return MapUnit::MapInch;
        case css::embed::EmbedMapUnits::POINT:
            return MapUnit::MapPoint;
        case css::embed::EmbedMapUnits::TWIP:
            return MapUnit::MapTwip;
        case css::embed::EmbedMapUnits::PIXEL:
            return MapUnit::MapPixel;
    }

    OSL_FAIL( "Unexpected UNO map mode is provided!" );
    return MapUnit::LASTENUMDUMMY;
}

sal_Int32 VCLUnoHelper::VCL2UnoEmbedMapUnit( MapUnit nVCLMapUnit )
{
    switch( nVCLMapUnit )
    {
        case MapUnit::Map100thMM:
            return css::embed::EmbedMapUnits::ONE_100TH_MM;
        case MapUnit::Map10thMM:
            return css::embed::EmbedMapUnits::ONE_10TH_MM;
        case MapUnit::MapMM:
            return css::embed::EmbedMapUnits::ONE_MM;
        case MapUnit::MapCM:
            return css::embed::EmbedMapUnits::ONE_CM;
        case MapUnit::Map1000thInch:
            return css::embed::EmbedMapUnits::ONE_1000TH_INCH;
        case MapUnit::Map100thInch:
            return css::embed::EmbedMapUnits::ONE_100TH_INCH;
        case MapUnit::Map10thInch:
            return css::embed::EmbedMapUnits::ONE_10TH_INCH;
        case MapUnit::MapInch:
            return css::embed::EmbedMapUnits::ONE_INCH;
        case MapUnit::MapPoint:
            return css::embed::EmbedMapUnits::POINT;
        case MapUnit::MapTwip:
            return css::embed::EmbedMapUnits::TWIP;
        case MapUnit::MapPixel:
            return css::embed::EmbedMapUnits::PIXEL;
        default: ; // avoid compiler warning
    }

    OSL_FAIL( "Unexpected VCL map mode is provided!" );
    return -1;
}

using namespace ::com::sun::star::util;


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
        } const aUnits[] = {
            { FieldUnit::NONE,       -1 , -1},
            { FieldUnit::MM,         MeasureUnit::MM,            1 },    // must precede MM_10TH
            { FieldUnit::MM,         MeasureUnit::MM_10TH,       10 },
            { FieldUnit::MM_100TH,   MeasureUnit::MM_100TH,      1 },
            { FieldUnit::CM,         MeasureUnit::CM,            1 },
            { FieldUnit::M,          MeasureUnit::M,             1 },
            { FieldUnit::KM,         MeasureUnit::KM,            1 },
            { FieldUnit::TWIP,       MeasureUnit::TWIP,          1 },
            { FieldUnit::POINT,      MeasureUnit::POINT,         1 },
            { FieldUnit::PICA,       MeasureUnit::PICA,          1 },
            { FieldUnit::INCH,       MeasureUnit::INCH,          1 },    // must precede INCH_*TH
            { FieldUnit::INCH,       MeasureUnit::INCH_10TH,     10 },
            { FieldUnit::INCH,       MeasureUnit::INCH_100TH,    100 },
            { FieldUnit::INCH,       MeasureUnit::INCH_1000TH,   1000 },
            { FieldUnit::FOOT,       MeasureUnit::FOOT,          1 },
            { FieldUnit::MILE,       MeasureUnit::MILE,          1 },
        };
        for (auto & aUnit : aUnits)
        {
            if ( eDirection == FieldUnitToMeasurementUnit )
            {
                if ( ( aUnit.eFieldUnit == static_cast<FieldUnit>(_nUnit) ) && ( aUnit.nFieldToMeasureFactor == _rFieldToUNOValueFactor ) )
                    return aUnit.nMeasurementUnit;
            }
            else
            {
                if ( aUnit.nMeasurementUnit == _nUnit )
                {
                    _rFieldToUNOValueFactor = aUnit.nFieldToMeasureFactor;
                    return static_cast<sal_Int16>(aUnit.eFieldUnit);
                }
            }
        }
        if ( eDirection == FieldUnitToMeasurementUnit )
            return -1;

        _rFieldToUNOValueFactor = 1;
        return sal_Int16(FieldUnit::NONE);
    }
}

//= MeasurementUnitConversion


sal_Int16 VCLUnoHelper::ConvertToMeasurementUnit( FieldUnit _nFieldUnit, sal_Int16 _nUNOToFieldValueFactor )
{
    return convertMeasurementUnit( static_cast<sal_Int16>(_nFieldUnit), FieldUnitToMeasurementUnit, _nUNOToFieldValueFactor );
}


FieldUnit VCLUnoHelper::ConvertToFieldUnit( sal_Int16 _nMeasurementUnit, sal_Int16& _rFieldToUNOValueFactor )
{
    return static_cast<FieldUnit>(convertMeasurementUnit( _nMeasurementUnit, MeasurementUnitToFieldUnit, _rFieldToUNOValueFactor ));
}


MapUnit /* MapModeUnit */ VCLUnoHelper::ConvertToMapModeUnit(sal_Int16 /* com.sun.star.util.MeasureUnit.* */ _nMeasureUnit)
{
    MapUnit eMode;
    switch(_nMeasureUnit)
    {
    case css::util::MeasureUnit::MM_100TH:
        eMode = MapUnit::Map100thMM;
        break;

    case css::util::MeasureUnit::MM_10TH:
        eMode = MapUnit::Map10thMM;
        break;

    case css::util::MeasureUnit::MM:
        eMode = MapUnit::MapMM;
        break;

    case css::util::MeasureUnit::CM:
        eMode = MapUnit::MapCM;
        break;

    case css::util::MeasureUnit::INCH_1000TH:
        eMode = MapUnit::Map1000thInch;
        break;

    case css::util::MeasureUnit::INCH_100TH:
        eMode = MapUnit::Map100thInch;
        break;

    case css::util::MeasureUnit::INCH_10TH:
        eMode = MapUnit::Map10thInch;
        break;

    case css::util::MeasureUnit::INCH:
        eMode = MapUnit::MapInch;
        break;

    case css::util::MeasureUnit::POINT:
        eMode = MapUnit::MapPoint;
        break;

    case css::util::MeasureUnit::TWIP:
        eMode = MapUnit::MapTwip;
        break;

    case css::util::MeasureUnit::PIXEL:
        eMode = MapUnit::MapPixel;
        break;

    case css::util::MeasureUnit::APPFONT:
        eMode = MapUnit::MapAppFont;
        break;

    case css::util::MeasureUnit::SYSFONT:
        eMode = MapUnit::MapSysFont;
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

::tools::Rectangle VCLUnoHelper::ConvertToVCLRect( css::awt::Rectangle const & _rRect )
{
    return ::tools::Rectangle( _rRect.X, _rRect.Y, _rRect.X + _rRect.Width - 1, _rRect.Y + _rRect.Height - 1 );
}

css::awt::Rectangle VCLUnoHelper::ConvertToAWTRect( ::tools::Rectangle const & _rRect )
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
    aMouseEvent.PopupTrigger = false;

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
