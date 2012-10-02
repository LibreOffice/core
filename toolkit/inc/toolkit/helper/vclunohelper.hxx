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

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#define _TOOLKIT_HELPER_VCLUNOHELPER_HXX_

#include <toolkit/dllapi.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>


namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
}}}}

namespace com { namespace sun { namespace star { namespace awt {
    class XBitmap;
    class XWindow;
    class XWindow2;
    class XWindowPeer;
    class XGraphics;
    class XRegion;
    class XDevice;
    class XPointer;
    class XToolkit;
    class XFont;
    class XControlContainer;
    struct Size;
    struct Point;
    struct SimpleFontMetric;
    struct FontDescriptor;
    struct Rectangle;
    struct KeyEvent;
}}}}


#include <vcl/bitmapex.hxx>
#include <vcl/region.hxx>
#include <vcl/metric.hxx>
#include <tools/mapunit.hxx>
#include <tools/fldunit.hxx>
#include <tools/poly.hxx>

class Window;
class OutputDevice;
class MouseEvent;
class KeyEvent;

//  ----------------------------------------------------
//  class VclUnoHelper
//  ----------------------------------------------------
class TOOLKIT_DLLPUBLIC VCLUnoHelper
{
public:
    // Toolkit
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>   CreateToolkit();

    // Bitmap
    static BitmapEx                                                         GetBitmap( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap>& rxBitmap );
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XBitmap>    CreateBitmap( const BitmapEx& rBitmap );

    // Window
    static Window*                                                          GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>& rxWindow );
    static Window*                                                          GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow2>& rxWindow2 );
    static Window*                                                          GetWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer>& rxWindowPeer );
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>    GetInterface( Window* pWindow );

    // OutputDevice
    static OutputDevice*                            GetOutputDevice( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice>& rxDevice );
    static OutputDevice*                            GetOutputDevice( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics>& rxGraphics );

    // Region
    static Region                                   GetRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion );

    // Polygon
    static Polygon                                  CreatePolygon( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY );

    // Font
    static ::com::sun::star::awt::FontDescriptor    CreateFontDescriptor( const Font& rFont );
    static Font                                     CreateFont( const ::com::sun::star::awt::FontDescriptor& rDescr, const Font& rInitFont );
    static Font                                     CreateFont( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >& rxFont );
    static ::com::sun::star::awt::SimpleFontMetric  CreateFontMetric( const FontMetric& rFontMetric );
    static float                                    ConvertFontWidth( FontWidth eWidth );
    static FontWidth                                ConvertFontWidth( float f );
    static float                                    ConvertFontWeight( FontWeight eWeight );
    static FontWeight                               ConvertFontWeight( float f );

    // Rectangle
    static sal_Bool                                 IsZero( ::com::sun::star::awt::Rectangle rRect );

    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer>  CreateControlContainer( Window* pWindow );

    // MapUnits
    static MapUnit                                  UnoEmbed2VCLMapUnit( sal_Int32 nUnoEmbedMapUnit );
    static sal_Int32                                VCL2UnoEmbedMapUnit( MapUnit nVCLMapUnit );

    //========================================================================
    //= MeasurementUnitConversion
    //========================================================================
    /** small helper to convert between <type>MeasurementUnit</type> and
        <type>FieldUnit</type>
    */
    static sal_Int16                                ConvertToMeasurementUnit( FieldUnit _nFieldUnit, sal_Int16 _rFieldToUNOValueFactor );
    static FieldUnit                                ConvertToFieldUnit( sal_Int16 _nMeasurementUnit, sal_Int16& _rFieldToUNOValueFactor );

    static MapUnit /* MapModeUnit */ ConvertToMapModeUnit(sal_Int16 /* com.sun.star.util.MeasureUnit.* */ _nMeasureUnit) throw (::com::sun::star::lang::IllegalArgumentException);

    static ::Size /* VCLSize */ ConvertToVCLSize(::com::sun::star::awt::Size const& _aSize);
    static ::com::sun::star::awt::Size ConvertToAWTSize(::Size /* VCLSize */ const& _aSize);

    static ::Point /* VCLPoint */ ConvertToVCLPoint(::com::sun::star::awt::Point const& _aPoint);
    static ::com::sun::star::awt::Point ConvertToAWTPoint(::Point /* VCLPoint */ const& _aPoint);

    static ::Rectangle ConvertToVCLRect( ::com::sun::star::awt::Rectangle const & _rRect );
    static ::com::sun::star::awt::Rectangle ConvertToAWTRect( ::Rectangle const & _rRect );

    static ::com::sun::star::awt::MouseEvent
        createMouseEvent(
            const ::MouseEvent& _rVclEvent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext
        );

    static ::com::sun::star::awt::KeyEvent
        createKeyEvent(
            const ::KeyEvent& _rVclEvent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext
        );
};


#endif  // _TOOLKIT_HELPER_VCLUNOHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
