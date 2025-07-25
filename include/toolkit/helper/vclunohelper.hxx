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

#ifndef INCLUDED_TOOLKIT_HELPER_VCLUNOHELPER_HXX
#define INCLUDED_TOOLKIT_HELPER_VCLUNOHELPER_HXX

#include <toolkit/dllapi.h>
#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/Rectangle.hpp>

#include <vcl/bitmapex.hxx>
#include <vcl/font.hxx>
#include <vcl/region.hxx>
#include <tools/mapunit.hxx>
#include <tools/fldunit.hxx>
#include <tools/poly.hxx>
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star::uno { template <typename > class Sequence; }

namespace com::sun::star::uno {
    class XInterface;
}

namespace com::sun::star::awt {
    class XBitmap;
    class XWindow;
    class XWindow2;
    class XWindowPeer;
    class XGraphics;
    class XRegion;
    class XDevice;
    class XToolkit;
    class XFont;
    class XControlContainer;
    struct SimpleFontMetric;
    struct FontDescriptor;
    struct Rectangle;
    struct KeyEvent;
}


enum class PointerStyle;

class FontMetric;
class OutputDevice;
class MouseEvent;
class KeyEvent;

class TOOLKIT_DLLPUBLIC VCLUnoHelper
{
public:
    // Toolkit
    static css::uno::Reference< css::awt::XToolkit>   CreateToolkit();

    // Bitmap
    static BitmapEx                                                         GetBitmap( const css::uno::Reference< css::awt::XBitmap>& rxBitmap );
    static css::uno::Reference< css::awt::XBitmap>    CreateBitmap( const BitmapEx& rBitmap );

    // Window
    static vcl::Window*                               GetWindow( const css::uno::Reference< css::awt::XWindow>& rxWindow );
    static vcl::Window*                               GetWindow( const css::uno::Reference< css::awt::XWindowPeer>& rxWindowPeer );
    static css::uno::Reference< css::awt::XWindow>    GetInterface( vcl::Window* pWindow );

    // OutputDevice
    static OutputDevice*                            GetOutputDevice( const css::uno::Reference< css::awt::XDevice>& rxDevice );
    static OutputDevice*                            GetOutputDevice( const css::uno::Reference< css::awt::XGraphics>& rxGraphics );

    // Region
    static vcl::Region                                   GetRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion );

    // Polygon
    static tools::Polygon CreatePolygon( const css::uno::Sequence< sal_Int32 >& DataX, const css::uno::Sequence< sal_Int32 >& DataY );

    /** convert Font to css::awt::FontDescriptor
        @param  rFont  Font to be converted
        @return the new FontDescriptor
    */
    static css::awt::FontDescriptor    CreateFontDescriptor( const vcl::Font& rFont );
    static vcl::Font                                CreateFont( const css::awt::FontDescriptor& rDescr, const vcl::Font& rInitFont );
    static vcl::Font                                CreateFont( const css::uno::Reference< css::awt::XFont >& rxFont );
    static css::awt::SimpleFontMetric  CreateFontMetric( const FontMetric& rFontMetric );

    // Rectangle
    static bool                                     IsZero(const css::awt::Rectangle& rRect);

    static rtl::Reference< UnoControlContainer>  CreateControlContainer( vcl::Window* pWindow );

    // MapUnits
    static MapUnit                                  UnoEmbed2VCLMapUnit( sal_Int32 nUnoEmbedMapUnit );
    static sal_Int32                                VCL2UnoEmbedMapUnit( MapUnit nVCLMapUnit );


    //= MeasurementUnitConversion

    /** small helper to convert between MeasurementUnit and
        FieldUnit
    */
    static sal_Int16                                ConvertToMeasurementUnit( FieldUnit _nFieldUnit, sal_Int16 _rFieldToUNOValueFactor );
    static FieldUnit                                ConvertToFieldUnit( sal_Int16 _nMeasurementUnit, sal_Int16& _rFieldToUNOValueFactor );

    /// @throws css::lang::IllegalArgumentException
    static MapUnit /* MapModeUnit */ ConvertToMapModeUnit(sal_Int16 /* com.sun.star.util.MeasureUnit.* */ _nMeasureUnit);

    static css::awt::MouseEvent
        createMouseEvent(
            const ::MouseEvent& _rVclEvent,
            const css::uno::Reference< css::uno::XInterface >& _rxContext
        );

    static ::MouseEvent createVCLMouseEvent( const css::awt::MouseEvent& _rAwtEvent );

    static css::awt::KeyEvent
        createKeyEvent(
            const ::KeyEvent& _rVclEvent,
            const css::uno::Reference< css::uno::XInterface >& _rxContext
        );

    static ::KeyEvent createVCLKeyEvent( const css::awt::KeyEvent& _rAwtEvent );

    static ::PointerStyle getMousePointer(const css::uno::Reference<css::awt::XWindowPeer>& rWindowPeer);
    static void setMousePointer(const css::uno::Reference<css::awt::XWindowPeer>& rWindowPeer, ::PointerStyle mousepointer);
};

#endif // INCLUDED_TOOLKIT_HELPER_VCLUNOHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
