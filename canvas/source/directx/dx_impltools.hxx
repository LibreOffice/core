/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    namespace tools
    {
        struct RawRGBABitmap;

        ::basegfx::B2DPolyPolygon
        polyPolygonFromXPolyPolygon2D( const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::rendering::XPolyPolygon2D >& );

        Gdiplus::Graphics* createGraphicsFromHDC(HDC);
        Gdiplus::Graphics* createGraphicsFromBitmap(const BitmapSharedPtr&);

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

        GraphicsPathSharedPtr graphicsPathFromB2DPolygon(
            const ::basegfx::B2DPolygon& rPoly,
            bool bNoLineJoin = false);

        GraphicsPathSharedPtr graphicsPathFromB2DPolyPolygon(
            const ::basegfx::B2DPolyPolygon& rPoly,
            bool bNoLineJoin = false);

        GraphicsPathSharedPtr graphicsPathFromXPolyPolygon2D(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >&,
            bool bNoLineJoin = false );

        bool drawGdiPlusBitmap( const GraphicsSharedPtr& rGraphics,
                                const BitmapSharedPtr&   rBitmap );
        bool drawDIBits( const ::boost::shared_ptr< Gdiplus::Graphics >& rGraphics,
                         const BITMAPINFO&                               rBI,
                         const void*                                     pBits );

        bool drawRGBABits( const ::boost::shared_ptr< Gdiplus::Graphics >& rGraphics,
                           const RawRGBABitmap&                            rRawRGBAData );

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
