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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXGRAPHICS_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXGRAPHICS_HXX


#include <com/sun/star/awt/XGraphics2.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weak.hxx>

#include <vcl/font.hxx>
#include <tools/color.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/vclptr.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <memory>

class OutputDevice;
namespace vcl { class Region; }
namespace com { namespace sun { namespace star { namespace graphic { class XGraphic; } } } }


enum class InitOutDevFlags
{
    NONE         = 0x0000,
    FONT         = 0x0001,
    COLORS       = 0x0002,
};
namespace o3tl
{
    template<> struct typed_flags<InitOutDevFlags> : is_typed_flags<InitOutDevFlags, 0x03> {};
}


//  class VCLXGraphics


class VCLXGraphics :    public css::awt::XGraphics2,
                        public css::lang::XTypeProvider,
                        public css::lang::XUnoTunnel,
                        public ::cppu::OWeakObject
{
private:
    // used to return same reference on each call to getDevice()
    css::uno::Reference< css::awt::XDevice> mxDevice;

    VclPtr<OutputDevice> mpOutputDevice;
    vcl::Font       maFont;
    Color           maTextColor;
    Color           maTextFillColor;
    Color           maLineColor;
    Color           maFillColor;
    RasterOp        meRasterOp;
    std::unique_ptr<vcl::Region> mpClipRegion;

    void initAttrs();

public:
                    VCLXGraphics();
                    virtual ~VCLXGraphics() override;

    void            Init( OutputDevice* pOutDev );
    void            InitOutputDevice( InitOutDevFlags nFlags );

    void            SetOutputDevice( OutputDevice* pOutDev );
    OutputDevice*   GetOutputDevice() const { return mpOutputDevice; }

    // css::uno::XInterface
    css::uno::Any                  SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // css::lang::XUnoTunnel
    static const css::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXGraphics*                                        GetImplementation( const css::uno::Reference< css::uno::XInterface >& rxIFace );
    sal_Int64                                                   SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rIdentifier ) override;

    // css::lang::XTypeProvider
    css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
    css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

    // css::awt::XGraphics Attributes
    virtual css::uno::Reference< css::awt::XDevice > SAL_CALL getDevice() override;
    virtual void SAL_CALL setTextColor( ::sal_Int32 _textcolor ) override;
    virtual void SAL_CALL setTextFillColor( ::sal_Int32 _textfillcolor ) override;
    virtual void SAL_CALL setLineColor( ::sal_Int32 _linecolor ) override;
    virtual void SAL_CALL setFillColor( ::sal_Int32 _fillcolor ) override;
    virtual void SAL_CALL setRasterOp( css::awt::RasterOperation _rasterop ) override;
    virtual void SAL_CALL setFont( const css::uno::Reference< css::awt::XFont >& _font ) override;
    virtual css::awt::SimpleFontMetric SAL_CALL getFontMetric() override;

    // css::awt::XGraphics Methods
    virtual void SAL_CALL selectFont( const css::awt::FontDescriptor& aDescription ) override;
    virtual void SAL_CALL setClipRegion( const css::uno::Reference< css::awt::XRegion >& Clipping ) override;
    virtual void SAL_CALL intersectClipRegion( const css::uno::Reference< css::awt::XRegion >& xClipping ) override;
    virtual void SAL_CALL push(  ) override;
    virtual void SAL_CALL pop(  ) override;
    virtual void SAL_CALL clear( const css::awt::Rectangle& aRect ) override;
    virtual void SAL_CALL copy( const css::uno::Reference< css::awt::XDevice >& xSource, ::sal_Int32 nSourceX, ::sal_Int32 nSourceY, ::sal_Int32 nSourceWidth, ::sal_Int32 nSourceHeight, ::sal_Int32 nDestX, ::sal_Int32 nDestY, ::sal_Int32 nDestWidth, ::sal_Int32 nDestHeight ) override;
    virtual void SAL_CALL draw( const css::uno::Reference< css::awt::XDisplayBitmap >& xBitmapHandle, ::sal_Int32 SourceX, ::sal_Int32 SourceY, ::sal_Int32 SourceWidth, ::sal_Int32 SourceHeight, ::sal_Int32 DestX, ::sal_Int32 DestY, ::sal_Int32 DestWidth, ::sal_Int32 DestHeight ) override;
    virtual void SAL_CALL drawPixel( ::sal_Int32 X, ::sal_Int32 Y ) override;
    virtual void SAL_CALL drawLine( ::sal_Int32 X1, ::sal_Int32 Y1, ::sal_Int32 X2, ::sal_Int32 Y2 ) override;
    virtual void SAL_CALL drawRect( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height ) override;
    virtual void SAL_CALL drawRoundedRect( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height, ::sal_Int32 nHorzRound, ::sal_Int32 nVertRound ) override;
    virtual void SAL_CALL drawPolyLine( const css::uno::Sequence< ::sal_Int32 >& DataX, const css::uno::Sequence< ::sal_Int32 >& DataY ) override;
    virtual void SAL_CALL drawPolygon( const css::uno::Sequence< ::sal_Int32 >& DataX, const css::uno::Sequence< ::sal_Int32 >& DataY ) override;
    virtual void SAL_CALL drawPolyPolygon( const css::uno::Sequence< css::uno::Sequence< ::sal_Int32 > >& DataX, const css::uno::Sequence< css::uno::Sequence< ::sal_Int32 > >& DataY ) override;
    virtual void SAL_CALL drawEllipse( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height ) override;
    virtual void SAL_CALL drawArc( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height, ::sal_Int32 X1, ::sal_Int32 Y1, ::sal_Int32 X2, ::sal_Int32 Y2 ) override;
    virtual void SAL_CALL drawPie( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height, ::sal_Int32 X1, ::sal_Int32 Y1, ::sal_Int32 X2, ::sal_Int32 Y2 ) override;
    virtual void SAL_CALL drawChord( ::sal_Int32 nX, ::sal_Int32 nY, ::sal_Int32 nWidth, ::sal_Int32 nHeight, ::sal_Int32 nX1, ::sal_Int32 nY1, ::sal_Int32 nX2, ::sal_Int32 nY2 ) override;
    virtual void SAL_CALL drawGradient( ::sal_Int32 nX, ::sal_Int32 nY, ::sal_Int32 nWidth, ::sal_Int32 Height, const css::awt::Gradient& aGradient ) override;
    virtual void SAL_CALL drawText( ::sal_Int32 X, ::sal_Int32 Y, const OUString& Text ) override;
    virtual void SAL_CALL drawTextArray( ::sal_Int32 X, ::sal_Int32 Y, const OUString& Text, const css::uno::Sequence< ::sal_Int32 >& Longs ) override;
    virtual void SAL_CALL drawImage( ::sal_Int32 nX, ::sal_Int32 nY, ::sal_Int32 nWidth, ::sal_Int32 nHeight, ::sal_Int16 nStyle, const css::uno::Reference< css::graphic::XGraphic >& aGraphic ) override;
};

#endif // INCLUDED_TOOLKIT_AWT_VCLXGRAPHICS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
