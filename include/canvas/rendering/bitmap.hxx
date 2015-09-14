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

#ifndef INCLUDED_CANVAS_RENDERING_BITMAP_HXX
#define INCLUDED_CANVAS_RENDERING_BITMAP_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <com/sun/star/geometry/IntegerPoint2D.hpp>
#include <canvas/rendering/icolorbuffer.hxx>
#include <canvas/rendering/icachedprimitive.hxx>
#include <canvas/rendering/isurfaceproxymanager.hxx>

#include <memory>
#include <vector>

namespace basegfx
{
    class B2IVector;
    class B2DPoint;
    class B2DHomMatrix;
    class B2DPolyPolygon;
}

namespace canvas
{
    class ImplBitmap;

    /** Bitmap class, with XCanvas-like render interface.

        This class provides a bitmap, that can be rendered into,
        with an interface compatible to XCanvas. Furthermore, the
        bitmaps held here can optionally be backed by (possibly
        hw-accelerated) textures.
    */
    class Bitmap
    {
    public:
        /** Create bitmap with given size

            @param rSize
            Size of the bitmap, in pixel

            @param rMgr
            SurfaceProxyManager, to use for HW acceleration

            @param bWithAlpha
            When true, the created bitmap will have an alpha channel,
            false otherwise
         */
        Bitmap( const ::basegfx::B2IVector&          rSize,
                const ISurfaceProxyManagerSharedPtr& rMgr,
                bool                                 bWithAlpha );
        ~Bitmap();

        /// Query whether this bitmap contains alpha channel information
        bool hasAlpha() const;

        /// Query size of the bitmap in pixel
        ::basegfx::B2IVector getSize() const;

        ::com::sun::star::uno::Sequence< sal_Int8 > getData(
            ::com::sun::star::rendering::IntegerBitmapLayout&       bitmapLayout,
            const ::com::sun::star::geometry::IntegerRectangle2D&   rect );

        void setData(
            const ::com::sun::star::uno::Sequence< sal_Int8 >&      data,
            const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
            const ::com::sun::star::geometry::IntegerRectangle2D&   rect );

        void setPixel(
            const ::com::sun::star::uno::Sequence< sal_Int8 >&      color,
            const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
            const ::com::sun::star::geometry::IntegerPoint2D&       pos );

        ::com::sun::star::uno::Sequence< sal_Int8 > getPixel(
            ::com::sun::star::rendering::IntegerBitmapLayout&       bitmapLayout,
            const ::com::sun::star::geometry::IntegerPoint2D&       pos );

        /** Render the bitmap content to associated
            SurfaceProxyManager's screen.

            @param fAlpha
            Overall alpha for content

            @param rPos
            Output position

            @param rTransform
            Output transformation (does not affect output position)
         */
        bool draw( double                         fAlpha,
                   const ::basegfx::B2DPoint&     rPos,
                   const ::basegfx::B2DHomMatrix& rTransform );

        /** Render the bitmap content to associated
            SurfaceProxyManager's screen.

            @param fAlpha
            Overall alpha for content

            @param rPos
            Output position

            @param rArea
            Subset of the surface to render. Coordinate system are
            surface area pixel, given area will be clipped to the
            surface bounds.

            @param rTransform
            Output transformation (does not affect output position)
         */
        bool draw( double                         fAlpha,
                   const ::basegfx::B2DPoint&     rPos,
                   const ::basegfx::B2DRange&     rArea,
                   const ::basegfx::B2DHomMatrix& rTransform );

        /** Render the bitmap content to associated
            SurfaceProxyManager's screen.

            @param fAlpha
            Overall alpha for content

            @param rPos
            Output position

            @param rClipPoly
            Clip polygon for the surface. The clip polygon is also
            subject to the output transformation.

            @param rTransform
            Output transformation (does not affect output position)
         */
        bool draw( double                           fAlpha,
                   const ::basegfx::B2DPoint&       rPos,
                   const ::basegfx::B2DPolyPolygon& rClipPoly,
                   const ::basegfx::B2DHomMatrix&   rTransform );

        /** Clear whole bitmap with given color.

            This method sets every single pixel of the bitmap to the
            specified color value.
         */
        void clear( const ::com::sun::star::uno::Sequence< double >& color );

        void fillB2DPolyPolygon(
            const ::basegfx::B2DPolyPolygon&                    rPolyPolygon,
            const ::com::sun::star::rendering::ViewState&       viewState,
            const ::com::sun::star::rendering::RenderState&     renderState );


        // High-level drawing operations (from the XCanvas interface)


        /// See XCanvas interface
        void drawPoint( const ::com::sun::star::geometry::RealPoint2D&      aPoint,
                        const ::com::sun::star::rendering::ViewState&       viewState,
                        const ::com::sun::star::rendering::RenderState&     renderState );
        /// See XCanvas interface
        void drawLine( const ::com::sun::star::geometry::RealPoint2D&   aStartPoint,
                       const ::com::sun::star::geometry::RealPoint2D&   aEndPoint,
                       const ::com::sun::star::rendering::ViewState&    viewState,
                       const ::com::sun::star::rendering::RenderState&  renderState );
        /// See XCanvas interface
        void drawBezier( const ::com::sun::star::geometry::RealBezierSegment2D& aBezierSegment,
                         const ::com::sun::star::geometry::RealPoint2D&         aEndPoint,
                         const ::com::sun::star::rendering::ViewState&          viewState,
                         const ::com::sun::star::rendering::RenderState&        renderState );
        /// See XCanvas interface
        ICachedPrimitiveSharedPtr drawPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState );

        /// See XCanvas interface
        ICachedPrimitiveSharedPtr strokePolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState,
            const ::com::sun::star::rendering::StrokeAttributes&    strokeAttributes );
        /** See XCanvas interface

            @param textureAnnotations
            Vector of shared pointers to bitmap textures,
            <em>corresponding</em> in indices to the textures
            sequence. This is to decouple this interface from the
            client's XBitmap-implementation class.
         */
        ICachedPrimitiveSharedPtr strokeTexturedPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState,
            const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::rendering::Texture >&           textures,
            const ::std::vector< std::shared_ptr<Bitmap> >&     textureAnnotations,
            const ::com::sun::star::rendering::StrokeAttributes&    strokeAttributes );
        /** See XCanvas interface

            @param textureAnnotations
            Vector of shared pointers to bitmap textures,
            <em>corresponding</em> in indices to the textures
            sequence. This is to decouple this interface from the
            client's XBitmap-implementation class.
         */
        ICachedPrimitiveSharedPtr strokeTextureMappedPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState,
            const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::rendering::Texture >&           textures,
            const ::std::vector< std::shared_ptr<Bitmap> >&     textureAnnotations,
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::geometry::XMapping2D >&         xMapping,
            const ::com::sun::star::rendering::StrokeAttributes&    strokeAttributes );
        /// See XCanvas interface
        ICachedPrimitiveSharedPtr fillPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState );
        /** See XCanvas interface

            @param textureAnnotations
            Vector of shared pointers to bitmap textures,
            <em>corresponding</em> in indices to the textures
            sequence. This is to decouple this interface from the
            client's XBitmap-implementation class.
         */
        ICachedPrimitiveSharedPtr fillTexturedPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState,
            const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::rendering::Texture >&           textures,
            const ::std::vector< std::shared_ptr<Bitmap> >&     textureAnnotations );
        /** See XCanvas interface

            @param textureAnnotations
            Vector of shared pointers to bitmap textures,
            <em>corresponding</em> in indices to the textures
            sequence. This is to decouple this interface from the
            client's XBitmap-implementation class.
         */
        ICachedPrimitiveSharedPtr fillTextureMappedPolyPolygon(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
            const ::com::sun::star::rendering::ViewState&           viewState,
            const ::com::sun::star::rendering::RenderState&         renderState,
            const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::rendering::Texture >&           textures,
            const ::std::vector< std::shared_ptr<Bitmap> >&     textureAnnotations,
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::geometry::XMapping2D >&         xMapping );

        /// See XCanvas interface
        ICachedPrimitiveSharedPtr drawBitmap(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XBitmap >&   xBitmap,
            const ::com::sun::star::rendering::ViewState&   viewState,
            const ::com::sun::star::rendering::RenderState& renderState );
        /// See XCanvas interface
        ICachedPrimitiveSharedPtr drawBitmap(
            const std::shared_ptr<Bitmap>&              rImage,
            const ::com::sun::star::rendering::ViewState&   viewState,
            const ::com::sun::star::rendering::RenderState& renderState );

        /// See XCanvas interface
        ICachedPrimitiveSharedPtr drawBitmapModulated(
            const ::com::sun::star::uno::Reference<
                  ::com::sun::star::rendering::XBitmap >&       xBitmap,
            const ::com::sun::star::rendering::ViewState&       viewState,
            const ::com::sun::star::rendering::RenderState&     renderState );
        /// See XCanvas interface
        ICachedPrimitiveSharedPtr drawBitmapModulated(
            const std::shared_ptr<Bitmap>&                  rImage,
            const ::com::sun::star::rendering::ViewState&       viewState,
            const ::com::sun::star::rendering::RenderState&     renderState );

    private:
        friend class ImplBitmap;

        const std::unique_ptr< ImplBitmap > mxImpl;
    };

    typedef std::shared_ptr< Bitmap > BitmapSharedPtr;
}

#endif // INCLUDED_CANVAS_RENDERING_BITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
