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

#pragma once

#include <com/sun/star/geometry/IntegerPoint2D.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>

#include <vcl/vclptr.hxx>
#include <vcl/virdev.hxx>

#include <vcl/cairo.hxx>
#include "cairo_surfaceprovider.hxx"

class VirtualDevice;

namespace basegfx {
    class B2DPolyPolygon;
}

namespace cairocanvas
{
    class SpriteCanvas;

    enum Operation {
        Stroke,
        Fill,
        Clip
    };

    class CanvasHelper
    {
    public:
        /// make noncopyable
        CanvasHelper(const CanvasHelper&) = delete;
        const CanvasHelper& operator=(const CanvasHelper&) = delete;

        CanvasHelper();

        /// Release all references
        void disposing();

        /** Initialize canvas helper

            This method late-initializes the canvas helper, providing
            it with the necessary device and size. Note that the
            CanvasHelper does <em>not</em> take ownership of the
            passed rDevice reference, nor does it perform any
            reference counting. Thus, to prevent the reference counted
            SpriteCanvas object from deletion, the user of this class
            is responsible for holding ref-counted references itself!

            @param rSizePixel
            Size of the output surface in pixel.

            @param rDevice
            Reference device this canvas is associated with

         */
        void init( const ::basegfx::B2ISize& rSizePixel,
                   SurfaceProvider&          rSurfaceProvider,
                   css::rendering::XGraphicDevice* pDevice );

        void setSize( const ::basegfx::B2ISize& rSize );
        void setSurface( const ::cairo::SurfaceSharedPtr& pSurface, bool bHasAlpha );

        // CanvasHelper functionality
        // ==========================

        // XCanvas (only providing, not implementing the
        // interface. Also note subtle method parameter differences)
        void clear();
        void drawLine( const css::rendering::XCanvas*      pCanvas,
                       const css::geometry::RealPoint2D&   aStartPoint,
                       const css::geometry::RealPoint2D&   aEndPoint,
                       const css::rendering::ViewState&    viewState,
                       const css::rendering::RenderState&  renderState );
        void drawBezier( const css::rendering::XCanvas*            pCanvas,
                         const css::geometry::RealBezierSegment2D& aBezierSegment,
                         const css::geometry::RealPoint2D&         aEndPoint,
                         const css::rendering::ViewState&          viewState,
                         const css::rendering::RenderState&        renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawPolyPolygon( const css::rendering::XCanvas*            pCanvas,
                             const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
                             const css::rendering::ViewState&          viewState,
                             const css::rendering::RenderState&        renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            strokePolyPolygon( const css::rendering::XCanvas*          pCanvas,
                               const css::uno::Reference<
                                       css::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const css::rendering::ViewState&        viewState,
                               const css::rendering::RenderState&      renderState,
                               const css::rendering::StrokeAttributes& strokeAttributes );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            strokeTexturedPolyPolygon( const css::rendering::XCanvas*          pCanvas,
                                       const css::uno::Reference< css::rendering::XPolyPolygon2D >&   xPolyPolygon,
                                       const css::rendering::ViewState&        viewState,
                                       const css::rendering::RenderState&      renderState,
                                       const css::uno::Sequence< css::rendering::Texture >&      textures,
                                       const css::rendering::StrokeAttributes& strokeAttributes );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            strokeTextureMappedPolyPolygon( const css::rendering::XCanvas*             pCanvas,
                                            const css::uno::Reference< css::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                            const css::rendering::ViewState&           viewState,
                                            const css::rendering::RenderState&         renderState,
                                            const css::uno::Sequence< css::rendering::Texture >&         textures,
                                            const css::uno::Reference< css::geometry::XMapping2D >&       xMapping,
                                            const css::rendering::StrokeAttributes&    strokeAttributes );
        css::uno::Reference< css::rendering::XPolyPolygon2D >
            queryStrokeShapes( const css::rendering::XCanvas*          pCanvas,
                               const css::uno::Reference< css::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const css::rendering::ViewState&        viewState,
                               const css::rendering::RenderState&      renderState,
                               const css::rendering::StrokeAttributes& strokeAttributes );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillPolyPolygon( const css::rendering::XCanvas*            pCanvas,
                             const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
                             const css::rendering::ViewState&          viewState,
                             const css::rendering::RenderState&        renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillTexturedPolyPolygon( const css::rendering::XCanvas*            pCanvas,
                                     const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
                                     const css::rendering::ViewState&          viewState,
                                     const css::rendering::RenderState&        renderState,
                                     const css::uno::Sequence< css::rendering::Texture >&        textures );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillTextureMappedPolyPolygon( const css::rendering::XCanvas*           pCanvas,
                                          const css::uno::Reference< css::rendering::XPolyPolygon2D >&    xPolyPolygon,
                                          const css::rendering::ViewState&         viewState,
                                          const css::rendering::RenderState&       renderState,
                                          const css::uno::Sequence< css::rendering::Texture >&       textures,
                                          const css::uno::Reference< css::geometry::XMapping2D >&     xMapping );

        css::uno::Reference< css::rendering::XCanvasFont >
            createFont( const css::rendering::XCanvas*             pCanvas,
                        const css::rendering::FontRequest&         fontRequest,
                        const css::uno::Sequence< css::beans::PropertyValue >&       extraFontProperties,
                        const css::geometry::Matrix2D&             fontMatrix );

        css::uno::Sequence< css::rendering::FontInfo >
            queryAvailableFonts( const css::rendering::XCanvas*        pCanvas,
                                 const css::rendering::FontInfo&       aFilter,
                                 const css::uno::Sequence< css::beans::PropertyValue >&  aFontProperties );

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawText( const css::rendering::XCanvas*           pCanvas,
                      const css::rendering::StringContext&     text,
                      const css::uno::Reference< css::rendering::XCanvasFont >&   xFont,
                      const css::rendering::ViewState&         viewState,
                      const css::rendering::RenderState&       renderState,
                      sal_Int8                                 textDirection );

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawTextLayout( const css::rendering::XCanvas*     pCanvas,
                            const css::uno::Reference< css::rendering::XTextLayout >& laidOutText,
                            const css::rendering::ViewState&   viewState,
                            const css::rendering::RenderState& renderState );

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawBitmap( const css::rendering::XCanvas*     pCanvas,
                        const css::uno::Reference< css::rendering::XBitmap >& xBitmap,
                        const css::rendering::ViewState&   viewState,
                        const css::rendering::RenderState& renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawBitmapModulated( const css::rendering::XCanvas*        pCanvas,
                                 const css::uno::Reference< css::rendering::XBitmap >&    xBitmap,
                                 const css::rendering::ViewState&      viewState,
                                 const css::rendering::RenderState&    renderState );
        css::uno::Reference< css::rendering::XGraphicDevice >
            getDevice() { return css::uno::Reference< css::rendering::XGraphicDevice >(mpDevice); }

        // BitmapCanvasHelper functionality
        // ================================

        css::geometry::IntegerSize2D getSize() const;

        css::uno::Reference< css::rendering::XBitmap >
            getScaledBitmap( const css::geometry::RealSize2D&  newSize,
                             bool                                           beFast );

        css::uno::Sequence< sal_Int8 >
            getData( css::rendering::IntegerBitmapLayout&      bitmapLayout,
                     const css::geometry::IntegerRectangle2D&  rect );

        css::uno::Sequence< sal_Int8 >
            getPixel( css::rendering::IntegerBitmapLayout& bitmapLayout,
                      const css::geometry::IntegerPoint2D& pos );

        css::rendering::IntegerBitmapLayout getMemoryLayout();

        void doPolyPolygonPath( const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
                                Operation aOperation,
                                bool bNoLineJoin = false,
                                const css::uno::Sequence< css::rendering::Texture >* pTextures=nullptr ) const;

        css::uno::Reference< css::rendering::XCachedPrimitive > implDrawBitmapSurface(
                   const css::rendering::XCanvas*      pCanvas,
                   const ::cairo::SurfaceSharedPtr&                 pSurface,
                   const css::rendering::ViewState&    viewState,
                   const css::rendering::RenderState&  renderState,
                   const css::geometry::IntegerSize2D& rSize,
                   bool bModulateColors,
                   bool bHasAlpha );

        bool repaint( const ::cairo::SurfaceSharedPtr& pSurface,
              const css::rendering::ViewState& viewState,
              const css::rendering::RenderState& renderState );

    protected:
        /** Surface provider

            Deliberately not a refcounted reference, because of
            potential circular references for canvas. Provides us with
            our output surface and associated functionality.
         */
        SurfaceProvider* mpSurfaceProvider;

        /** Phyical output device

            Deliberately not a refcounted reference, because of
            potential circular references for spritecanvas.
         */
        css::rendering::XGraphicDevice* mpDevice;

    private:

        VclPtr<VirtualDevice> mpVirtualDevice;

        void useStates( const css::rendering::ViewState& viewState,
                        const css::rendering::RenderState& renderState,
                        bool setColor );

        css::rendering::IntegerBitmapLayout impl_getMemoryLayout( sal_Int32 nWidth, sal_Int32 nHeight );

        /// When true, content is able to represent alpha
        bool mbHaveAlpha;

        ::cairo::CairoSharedPtr     mpCairo;
        ::cairo::SurfaceSharedPtr   mpSurface;
        ::basegfx::B2ISize maSize;
    };

    /// also needed from SpriteHelper
    void doPolyPolygonImplementation( const ::basegfx::B2DPolyPolygon& aPolyPolygon,
                                      Operation aOperation,
                                      cairo_t* pCairo,
                                      const css::uno::Sequence< css::rendering::Texture >* pTextures,
                                      const SurfaceProviderRef& pDevice,
                                      PolyFillMode eFillrule );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
