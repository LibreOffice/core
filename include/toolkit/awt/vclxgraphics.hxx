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
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <vcl/font.hxx>
#include <tools/color.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/vclptr.hxx>
#include <o3tl/typed_flags_set.hxx>

class OutputDevice;
namespace vcl { class Region; }


enum class InitOutDevFlags
{
    FONT         = 0x0001,
    COLORS       = 0x0002,
    RASTEROP     = 0x0004,
    CLIPREGION   = 0x0008,
};
namespace o3tl
{
    template<> struct typed_flags<InitOutDevFlags> : is_typed_flags<InitOutDevFlags, 0x0f> {};
}


//  class VCLXGraphics


class VCLXGraphics :    public ::com::sun::star::awt::XGraphics2,
                        public ::com::sun::star::lang::XTypeProvider,
                        public ::com::sun::star::lang::XUnoTunnel,
                        public ::cppu::OWeakObject
{
private:
    // used to return same reference on each call to getDevice()
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice> mxDevice;

    VclPtr<OutputDevice> mpOutputDevice;
    vcl::Font       maFont;
    Color           maTextColor;
    Color           maTextFillColor;
    Color           maLineColor;
    Color           maFillColor;
    RasterOp        meRasterOp;
    vcl::Region*         mpClipRegion;

    void initAttrs();

public:
                    VCLXGraphics();
                    virtual ~VCLXGraphics();

    void            Init( OutputDevice* pOutDev );
    void            InitOutputDevice( InitOutDevFlags nFlags );

    void            SetOutputDevice( OutputDevice* pOutDev );
    OutputDevice*   GetOutputDevice() const { return mpOutputDevice; }

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXGraphics*                                        GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace );
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XGraphics Attributes
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > SAL_CALL getDevice() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTextColor( ::sal_Int32 _textcolor ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTextFillColor( ::sal_Int32 _textfillcolor ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLineColor( ::sal_Int32 _linecolor ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFillColor( ::sal_Int32 _fillcolor ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRasterOp( ::com::sun::star::awt::RasterOperation _rasterop ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFont( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >& _font ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::SimpleFontMetric SAL_CALL getFontMetric() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XGraphics Methods
    virtual void SAL_CALL selectFont( const ::com::sun::star::awt::FontDescriptor& aDescription ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setClipRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& Clipping ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL intersectClipRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& xClipping ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL push(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL pop(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clear( const ::com::sun::star::awt::Rectangle& aRect ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL copy( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >& xSource, ::sal_Int32 nSourceX, ::sal_Int32 nSourceY, ::sal_Int32 nSourceWidth, ::sal_Int32 nSourceHeight, ::sal_Int32 nDestX, ::sal_Int32 nDestY, ::sal_Int32 nDestWidth, ::sal_Int32 nDestHeight ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL draw( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap >& xBitmapHandle, ::sal_Int32 SourceX, ::sal_Int32 SourceY, ::sal_Int32 SourceWidth, ::sal_Int32 SourceHeight, ::sal_Int32 DestX, ::sal_Int32 DestY, ::sal_Int32 DestWidth, ::sal_Int32 DestHeight ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawPixel( ::sal_Int32 X, ::sal_Int32 Y ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawLine( ::sal_Int32 X1, ::sal_Int32 Y1, ::sal_Int32 X2, ::sal_Int32 Y2 ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawRect( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawRoundedRect( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height, ::sal_Int32 nHorzRound, ::sal_Int32 nVertRound ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawPolyLine( const ::com::sun::star::uno::Sequence< ::sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< ::sal_Int32 >& DataY ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawPolygon( const ::com::sun::star::uno::Sequence< ::sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< ::sal_Int32 >& DataY ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawPolyPolygon( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::sal_Int32 > >& DataX, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::sal_Int32 > >& DataY ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawEllipse( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawArc( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height, ::sal_Int32 X1, ::sal_Int32 Y1, ::sal_Int32 X2, ::sal_Int32 Y2 ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawPie( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height, ::sal_Int32 X1, ::sal_Int32 Y1, ::sal_Int32 X2, ::sal_Int32 Y2 ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawChord( ::sal_Int32 nX, ::sal_Int32 nY, ::sal_Int32 nWidth, ::sal_Int32 nHeight, ::sal_Int32 nX1, ::sal_Int32 nY1, ::sal_Int32 nX2, ::sal_Int32 nY2 ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawGradient( ::sal_Int32 nX, ::sal_Int32 nY, ::sal_Int32 nWidth, ::sal_Int32 Height, const ::com::sun::star::awt::Gradient& aGradient ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawText( ::sal_Int32 X, ::sal_Int32 Y, const OUString& Text ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawTextArray( ::sal_Int32 X, ::sal_Int32 Y, const OUString& Text, const ::com::sun::star::uno::Sequence< ::sal_Int32 >& Longs ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL drawImage( ::sal_Int32 nX, ::sal_Int32 nY, ::sal_Int32 nWidth, ::sal_Int32 nHeight, ::sal_Int16 nStyle, const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& aGraphic ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_TOOLKIT_AWT_VCLXGRAPHICS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
