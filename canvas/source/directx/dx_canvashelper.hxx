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

#ifndef _DXCANVAS_CANVASHELPER_HXX_
#define _DXCANVAS_CANVASHELPER_HXX_

#include <com/sun/star/rendering/XCanvas.hpp>

#include <basegfx/vector/b2isize.hxx>
#include <basegfx/vector/b2dsize.hxx>

#include "dx_graphicsprovider.hxx"
#include "dx_gdiplususer.hxx"
#include "dx_impltools.hxx"

#include <boost/utility.hpp>


namespace dxcanvas
{
    /** Helper class for basic canvas functionality. Also offers
        optional backbuffer painting, when providing it with a second
        HDC to render into.
     */
    class CanvasHelper : private ::boost::noncopyable
    {
    public:
        CanvasHelper();

        /// Release all references
        void disposing();

        /** Initialize canvas helper

            This method late-initializes the canvas helper, providing
            it with the necessary device and output objects. Note that
            the CanvasHelper does <em>not</em> take ownership of the
            passed rDevice reference, nor does it perform any
            reference counting. Thus, to prevent the reference counted
            SpriteCanvas object from deletion, the user of this class
            is responsible for holding ref-counted references itself!

            @param rDevice
            Reference device this canvas is associated with

         */
        void setDevice( com::sun::star::rendering::XGraphicDevice& rDevice );

        /** Set the target for rendering operations

            @param rTarget
            Render target
         */
        void setTarget( const GraphicsProviderSharedPtr& rTarget );

        /** Set the target for rendering operations

            @param rTarget
            Render target

            @param rOutputOffset
            Output offset in pixel
         */
        void setTarget( const GraphicsProviderSharedPtr& rTarget,
                        const ::basegfx::B2ISize&        rOutputOffset );


        // CanvasHelper functionality
        // ==========================

        // XCanvas (only providing, not implementing the
        // interface. Also note subtle method parameter differences)
        void clear();
        void drawPoint( const ::com::sun::star::rendering::XCanvas*     pCanvas,
                        const ::com::sun::star::geometry::RealPoint2D&  aPoint,
                        const ::com::sun::star::rendering::ViewState&   viewState,
                        const ::com::sun::star::rendering::RenderState& renderState );
        void drawLine( const ::com::sun::star::rendering::XCanvas*      pCanvas,
                       const ::com::sun::star::geometry::RealPoint2D&   aStartPoint,
                       const ::com::sun::star::geometry::RealPoint2D&   aEndPoint,
                       const ::com::sun::star::rendering::ViewState&    viewState,
                       const ::com::sun::star::rendering::RenderState&  renderState );
        void drawBezier( const ::com::sun::star::rendering::XCanvas*            pCanvas,
                         const ::com::sun::star::geometry::RealBezierSegment2D& aBezierSegment,
                         const ::com::sun::star::geometry::RealPoint2D&         aEndPoint,
                         const ::com::sun::star::rendering::ViewState&          viewState,
                         const ::com::sun::star::rendering::RenderState&        renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawPolyPolygon( const ::com::sun::star::rendering::XCanvas*            pCanvas,
                             const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                             const ::com::sun::star::rendering::ViewState&          viewState,
                             const ::com::sun::star::rendering::RenderState&        renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            strokePolyPolygon( const ::com::sun::star::rendering::XCanvas*          pCanvas,
                               const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::com::sun::star::rendering::ViewState&        viewState,
                               const ::com::sun::star::rendering::RenderState&      renderState,
                               const ::com::sun::star::rendering::StrokeAttributes& strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            strokeTexturedPolyPolygon( const ::com::sun::star::rendering::XCanvas*          pCanvas,
                                       const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                                       const ::com::sun::star::rendering::ViewState&        viewState,
                                       const ::com::sun::star::rendering::RenderState&      renderState,
                                       const ::com::sun::star::uno::Sequence<
                                               ::com::sun::star::rendering::Texture >&      textures,
                                       const ::com::sun::star::rendering::StrokeAttributes& strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            strokeTextureMappedPolyPolygon( const ::com::sun::star::rendering::XCanvas*             pCanvas,
                                            const ::com::sun::star::uno::Reference<
                                                    ::com::sun::star::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                            const ::com::sun::star::rendering::ViewState&           viewState,
                                            const ::com::sun::star::rendering::RenderState&         renderState,
                                            const ::com::sun::star::uno::Sequence<
                                                    ::com::sun::star::rendering::Texture >&         textures,
                                            const ::com::sun::star::uno::Reference<
                                                    ::com::sun::star::geometry::XMapping2D >&       xMapping,
                                            const ::com::sun::star::rendering::StrokeAttributes&    strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >
            queryStrokeShapes( const ::com::sun::star::rendering::XCanvas*          pCanvas,
                               const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::com::sun::star::rendering::ViewState&        viewState,
                               const ::com::sun::star::rendering::RenderState&      renderState,
                               const ::com::sun::star::rendering::StrokeAttributes& strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillPolyPolygon( const ::com::sun::star::rendering::XCanvas*            pCanvas,
                             const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::rendering::XPolyPolygon2D >&     xPolyPolygon,
                             const ::com::sun::star::rendering::ViewState&          viewState,
                             const ::com::sun::star::rendering::RenderState&        renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillTexturedPolyPolygon( const ::com::sun::star::rendering::XCanvas*            pCanvas,
                                     const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::rendering::XPolyPolygon2D >&     xPolyPolygon,
                                     const ::com::sun::star::rendering::ViewState&          viewState,
                                     const ::com::sun::star::rendering::RenderState&        renderState,
                                     const ::com::sun::star::uno::Sequence<
                                             ::com::sun::star::rendering::Texture >&        textures );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillTextureMappedPolyPolygon( const ::com::sun::star::rendering::XCanvas*           pCanvas,
                                          const ::com::sun::star::uno::Reference<
                                                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
                                          const ::com::sun::star::rendering::ViewState&         viewState,
                                          const ::com::sun::star::rendering::RenderState&       renderState,
                                          const ::com::sun::star::uno::Sequence<
                                                  ::com::sun::star::rendering::Texture >&       textures,
                                          const ::com::sun::star::uno::Reference<
                                                  ::com::sun::star::geometry::XMapping2D >&         xMapping );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont > SAL_CALL
            createFont( const ::com::sun::star::rendering::XCanvas*             pCanvas,
                        const ::com::sun::star::rendering::FontRequest&         fontRequest,
                        const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >&       extraFontProperties,
                        const ::com::sun::star::geometry::Matrix2D&             fontMatrix );

        ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::FontInfo >
            queryAvailableFonts( const ::com::sun::star::rendering::XCanvas*        pCanvas,
                                 const ::com::sun::star::rendering::FontInfo&       aFilter,
                                 const ::com::sun::star::uno::Sequence<
                                         ::com::sun::star::beans::PropertyValue >&  aFontProperties );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawText( const ::com::sun::star::rendering::XCanvas*           pCanvas,
                      const ::com::sun::star::rendering::StringContext&     text,
                      const ::com::sun::star::uno::Reference<
                              ::com::sun::star::rendering::XCanvasFont >&   xFont,
                      const ::com::sun::star::rendering::ViewState&         viewState,
                      const ::com::sun::star::rendering::RenderState&       renderState,
                      sal_Int8                                              textDirection );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawTextLayout( const ::com::sun::star::rendering::XCanvas*     pCanvas,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::rendering::XTextLayout >& layoutetText,
                            const ::com::sun::star::rendering::ViewState&   viewState,
                            const ::com::sun::star::rendering::RenderState& renderState );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawBitmap( const ::com::sun::star::rendering::XCanvas*     pCanvas,
                        const ::com::sun::star::uno::Reference<
                                ::com::sun::star::rendering::XBitmap >& xBitmap,
                        const ::com::sun::star::rendering::ViewState&   viewState,
                        const ::com::sun::star::rendering::RenderState& renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawBitmapModulated( const ::com::sun::star::rendering::XCanvas*        pCanvas,
                                 const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::rendering::XBitmap >&        xBitmap,
                                 const ::com::sun::star::rendering::ViewState&      viewState,
                                 const ::com::sun::star::rendering::RenderState&    renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >
            getDevice();

        // Flush drawing queue to screen
        void                    flush() const;

        /** Called from XCanvas base classes, to notify that content
            is _about_ to change
        */
        void modifying() {}

    protected:
        /// Refcounted global GDI+ state container
        GDIPlusUserSharedPtr            mpGdiPlusUser;

        /** Phyical output device

            Deliberately not a refcounted reference, because of
            potential circular references for spritecanvas.
         */
        com::sun::star::rendering::XGraphicDevice* mpDevice;

        /// Provides the Gdiplus::Graphics to render into
        GraphicsProviderSharedPtr                  mpGraphicsProvider;

        bool needOutput() const { return mpGraphicsProvider.get() != NULL; };

        // returns transparency of color
        void setupGraphicsState( GraphicsSharedPtr&                              rGraphics,
                                 const ::com::sun::star::rendering::ViewState&   viewState,
                                 const ::com::sun::star::rendering::RenderState& renderState );

        Gdiplus::CompositingMode    calcCompositingMode( sal_Int8 nMode );

        /// Current (transformation-independent) output buffer offset
        ::basegfx::B2ISize              maOutputOffset;
    };
}

#endif /* _DXCANVAS_CANVASHELPER_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
