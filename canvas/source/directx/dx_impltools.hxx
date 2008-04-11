/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_impltools.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _DXCANVAS_IMPLTOOLS_HXX
#define _DXCANVAS_IMPLTOOLS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/TriState.hpp>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <boost/shared_ptr.hpp>
#include "dx_canvasfont.hxx"

namespace basegfx
{
    class B2DPoint;
    class B2DRange;
    class B2DHomMatrix;
    class B2IPoint;
    class B2IRange;
    class B2DPolyPolygon;
};

namespace com { namespace sun { namespace star { namespace geometry
{
    struct IntegerRectangle2D;
    struct RealPoint2D;
} } } }

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XCanvas;
    class  XGraphicDevice;
    class  XBitmap;
    class  XPolyPolygon2D;
    class  XCanvasFont;
} } } }


namespace dxcanvas
{
    class SurfaceGraphics;

    namespace tools
    {
        struct RawRGBABitmap;

        ::basegfx::B2DPolyPolygon
        polyPolygonFromXPolyPolygon2D( const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::rendering::XPolyPolygon2D >& );

        void setupGraphics( Gdiplus::Graphics& rGraphics );

        void gdiPlusMatrixFromB2DHomMatrix( Gdiplus::Matrix&        rGdiplusMatrix,
                                            const ::basegfx::B2DHomMatrix&  rMatrix );
        void gdiPlusMatrixFromAffineMatrix2D( Gdiplus::Matrix&                          rGdiplusMatrix,
                                              const ::com::sun::star::geometry::AffineMatrix2D& rMatrix );

        Gdiplus::PointF gdiPlusPointFFromRealPoint2D( const ::com::sun::star::geometry::RealPoint2D& );
        Gdiplus::RectF  gdiPlusRectFFromRectangle2D( const ::com::sun::star::geometry::RealRectangle2D& );
        Gdiplus::Rect   gdiPlusRectFromIntegerRectangle2D( const ::com::sun::star::geometry::IntegerRectangle2D& );
        RECT            gdiRectFromB2IRect( const ::basegfx::B2IRange& );

        ::com::sun::star::geometry::RealPoint2D     realPoint2DFromGdiPlusPointF( const Gdiplus::PointF& );
        ::com::sun::star::geometry::RealRectangle2D realRectangle2DFromGdiPlusRectF( const Gdiplus::RectF& );

        ::basegfx::B2DPoint b2dPointFromGdiPlusPointF( const Gdiplus::PointF& );
        ::basegfx::B2DRange b2dRangeFromGdiPlusRectF( const Gdiplus::RectF& );

        ::com::sun::star::uno::Sequence< double > argbToDoubleSequence( const Gdiplus::ARGB& rColor );
        ::com::sun::star::uno::Sequence< sal_Int8 > argbToIntSequence( const Gdiplus::ARGB& rColor );
        Gdiplus::ARGB sequenceToArgb( const ::com::sun::star::uno::Sequence< sal_Int8 >& rColor );
        Gdiplus::ARGB sequenceToArgb( const ::com::sun::star::uno::Sequence< double >&  rColor );

        GraphicsPathSharedPtr graphicsPathFromRealPoint2DSequence( const ::com::sun::star::uno::Sequence<
                                                                 ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > >& );

        GraphicsPathSharedPtr graphicsPathFromB2DPolygon( const ::basegfx::B2DPolygon& rPoly );
        GraphicsPathSharedPtr graphicsPathFromB2DPolyPolygon( const ::basegfx::B2DPolyPolygon& rPoly );

        GraphicsPathSharedPtr graphicsPathFromXPolyPolygon2D( const ::com::sun::star::uno::Reference<
                                                              ::com::sun::star::rendering::XPolyPolygon2D >& );
        bool drawDIBits( const ::boost::shared_ptr< SurfaceGraphics >&  rGraphics,
                         const BITMAPINFO&                      rBI,
                         const void*                                    pBits );

        bool drawRGBABits( const ::boost::shared_ptr< SurfaceGraphics >&    rGraphics,
                           const RawRGBABitmap&                             rRawRGBAData );

        bool drawXBitmap( const ::boost::shared_ptr< SurfaceGraphics >& rGraphics,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::rendering::XBitmap >&   xBitmap );

        BitmapSharedPtr bitmapFromXBitmap( const ::com::sun::star::uno::Reference<
                                                   ::com::sun::star::rendering::XBitmap >&  xBitmap );

        CanvasFont::ImplRef canvasFontFromXFont( const ::com::sun::star::uno::Reference<
                                                     ::com::sun::star::rendering::XCanvasFont >& xFont );

        void setModulateImageAttributes( Gdiplus::ImageAttributes& o_rAttr,
                                         double                           nRedModulation,
                                         double                           nGreenModulation,
                                         double                           nBlueModulation,
                                         double                           nAlphaModulation );
    }
}

#endif /* _DXCANVAS_IMPLTOOLS_HXX */
