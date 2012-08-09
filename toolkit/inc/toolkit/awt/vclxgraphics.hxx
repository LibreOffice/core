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

#ifndef _TOOLKIT_AWT_VCLXGRAPHICS_HXX_
#define _TOOLKIT_AWT_VCLXGRAPHICS_HXX_


#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <vcl/font.hxx>
#include <tools/color.hxx>
#include <vcl/vclenum.hxx>

class OutputDevice;
class Region;


#define INITOUTDEV_FONT         0x0001
#define INITOUTDEV_COLORS       0x0002
#define INITOUTDEV_RASTEROP     0x0004
#define INITOUTDEV_CLIPREGION   0x0008
#define INITOUTDEV_ALL          0xFFFF


//  ----------------------------------------------------
//  class VCLXGraphics
//  ----------------------------------------------------

class VCLXGraphics :    public ::com::sun::star::awt::XGraphics,
                        public ::com::sun::star::lang::XTypeProvider,
                        public ::com::sun::star::lang::XUnoTunnel,
                        public ::cppu::OWeakObject
{
private:
    // used to return same reference on each call to getDevice()
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice> mxDevice;

    OutputDevice*   mpOutputDevice;
    Font            maFont;
    Color           maTextColor;
    Color           maTextFillColor;
    Color           maLineColor;
    Color           maFillColor;
    RasterOp        meRasterOp;
    Region*         mpClipRegion;

public:
                    VCLXGraphics();
                    ~VCLXGraphics();

    void            Init( OutputDevice* pOutDev );
    void            InitOutputDevice( sal_uInt16 nFlags );

    void            SetOutputDevice( OutputDevice* pOutDev );
    OutputDevice*   GetOutputDevice() const { return mpOutputDevice; }

    const Font&     GetFont() const { return maFont; }

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw()  { OWeakObject::release(); }

    // ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXGraphics*                                        GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace ) throw();
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XGraphics
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >  SAL_CALL getDevice(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::SimpleFontMetric                             SAL_CALL getFontMetric() throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setFont( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont >& xNewFont ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL selectFont( const ::com::sun::star::awt::FontDescriptor& aDescription ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setTextColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setTextFillColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setLineColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setFillColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setRasterOp( ::com::sun::star::awt::RasterOperation ROP ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL setClipRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& Clipping ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL intersectClipRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& xClipping ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL push(  ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL pop(  ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL copy( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >& xSource, sal_Int32 nSourceX, sal_Int32 nSourceY, sal_Int32 nSourceWidth, sal_Int32 nSourceHeight, sal_Int32 nDestX, sal_Int32 nDestY, sal_Int32 nDestWidth, sal_Int32 nDestHeight ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL draw( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayBitmap >& xBitmapHandle, sal_Int32 SourceX, sal_Int32 SourceY, sal_Int32 SourceWidth, sal_Int32 SourceHeight, sal_Int32 DestX, sal_Int32 DestY, sal_Int32 DestWidth, sal_Int32 DestHeight ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawPixel( sal_Int32 X, sal_Int32 Y ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawLine( sal_Int32 X1, sal_Int32 Y1, sal_Int32 X2, sal_Int32 Y2 ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawRect( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawRoundedRect( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int32 nHorzRound, sal_Int32 nVertRound ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawPolyLine( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawPolygon( const ::com::sun::star::uno::Sequence< sal_Int32 >& DataX, const ::com::sun::star::uno::Sequence< sal_Int32 >& DataY ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawPolyPolygon( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > >& DataX, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > >& DataY ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawEllipse( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawArc( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int32 X1, sal_Int32 Y1, sal_Int32 X2, sal_Int32 Y2 ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawPie( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int32 X1, sal_Int32 Y1, sal_Int32 X2, sal_Int32 Y2 ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawChord( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int32 nX1, sal_Int32 nY1, sal_Int32 nX2, sal_Int32 nY2 ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawGradient( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 Height, const ::com::sun::star::awt::Gradient& aGradient ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawText( sal_Int32 X, sal_Int32 Y, const ::rtl::OUString& Text ) throw(::com::sun::star::uno::RuntimeException);
    void        SAL_CALL drawTextArray( sal_Int32 X, sal_Int32 Y, const ::rtl::OUString& Text, const ::com::sun::star::uno::Sequence< sal_Int32 >& Longs ) throw(::com::sun::star::uno::RuntimeException);
};




#endif // _TOOLKIT_AWT_VCLXGRAPHICS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
