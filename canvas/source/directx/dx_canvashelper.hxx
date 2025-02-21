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

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <basegfx/vector/b2isize.hxx>
#include <basegfx/vector/b2dsize.hxx>

#include "dx_graphicsprovider.hxx"
#include "dx_gdiplususer.hxx"
#include "dx_impltools.hxx"


namespace dxcanvas
{
    /** Helper class for basic canvas functionality. Also offers
        optional backbuffer painting, when providing it with a second
        HDC to render into.
     */
    class CanvasHelper
    {
    public:
        CanvasHelper();

        /// make noncopyable
        CanvasHelper(const CanvasHelper&) = delete;
        const CanvasHelper& operator=(const CanvasHelper&) = delete;

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
        void setDevice( css::rendering::XGraphicDevice& rDevice );

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
        void drawPoint( const css::rendering::XCanvas*     pCanvas,
                        const css::geometry::RealPoint2D&  aPoint,
                        const css::rendering::ViewState&   viewState,
                        const css::rendering::RenderState& renderState );
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
                             const css::uno::Reference<
                                     css::rendering::XPolyPolygon2D >& xPolyPolygon,
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
                                       const css::uno::Reference<
                                               css::rendering::XPolyPolygon2D >&   xPolyPolygon,
                                       const css::rendering::ViewState&        viewState,
                                       const css::rendering::RenderState&      renderState,
                                       const css::uno::Sequence<
                                               css::rendering::Texture >&      textures,
                                       const css::rendering::StrokeAttributes& strokeAttributes );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            strokeTextureMappedPolyPolygon( const css::rendering::XCanvas*             pCanvas,
                                            const css::uno::Reference<
                                                    css::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                            const css::rendering::ViewState&           viewState,
                                            const css::rendering::RenderState&         renderState,
                                            const css::uno::Sequence<
                                                    css::rendering::Texture >&         textures,
                                            const css::uno::Reference<
                                                    css::geometry::XMapping2D >&       xMapping,
                                            const css::rendering::StrokeAttributes&    strokeAttributes );
        css::uno::Reference< css::rendering::XPolyPolygon2D >
            queryStrokeShapes( const css::rendering::XCanvas*          pCanvas,
                               const css::uno::Reference<
                                       css::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const css::rendering::ViewState&        viewState,
                               const css::rendering::RenderState&      renderState,
                               const css::rendering::StrokeAttributes& strokeAttributes );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillPolyPolygon( const css::rendering::XCanvas*            pCanvas,
                             const css::uno::Reference<
                                     css::rendering::XPolyPolygon2D >&     xPolyPolygon,
                             const css::rendering::ViewState&          viewState,
                             const css::rendering::RenderState&        renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillTexturedPolyPolygon( const css::rendering::XCanvas*            pCanvas,
                                     const css::uno::Reference<
                                             css::rendering::XPolyPolygon2D >&     xPolyPolygon,
                                     const css::rendering::ViewState&          viewState,
                                     const css::rendering::RenderState&        renderState,
                                     const css::uno::Sequence<
                                             css::rendering::Texture >&        textures );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillTextureMappedPolyPolygon( const css::rendering::XCanvas*           pCanvas,
                                          const css::uno::Reference<
                                                  css::rendering::XPolyPolygon2D >&    xPolyPolygon,
                                          const css::rendering::ViewState&         viewState,
                                          const css::rendering::RenderState&       renderState,
                                          const css::uno::Sequence<
                                                  css::rendering::Texture >&       textures,
                                          const css::uno::Reference<
                                                  css::geometry::XMapping2D >&         xMapping );

        css::uno::Reference< css::rendering::XCanvasFont >
            createFont( const css::rendering::XCanvas*             pCanvas,
                        const css::rendering::FontRequest&         fontRequest,
                        const css::uno::Sequence<
                                css::beans::PropertyValue >&       extraFontProperties,
                        const css::geometry::Matrix2D&             fontMatrix );

        css::uno::Sequence< css::rendering::FontInfo >
            queryAvailableFonts( const css::rendering::XCanvas*        pCanvas,
                                 const css::rendering::FontInfo&       aFilter,
                                 const css::uno::Sequence<
                                         css::beans::PropertyValue >&  aFontProperties );

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawText( const css::rendering::XCanvas*           pCanvas,
                      const css::rendering::StringContext&     text,
                      const css::uno::Reference<
                              css::rendering::XCanvasFont >&   xFont,
                      const css::rendering::ViewState&         viewState,
                      const css::rendering::RenderState&       renderState,
                      sal_Int8                                              textDirection );

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawTextLayout( const css::rendering::XCanvas*     pCanvas,
                            const css::uno::Reference<
                                css::rendering::XTextLayout >& laidOutText,
                            const css::rendering::ViewState&   viewState,
                            const css::rendering::RenderState& renderState );

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawBitmap( const css::rendering::XCanvas*     pCanvas,
                        const css::uno::Reference<
                                css::rendering::XBitmap >& xBitmap,
                        const css::rendering::ViewState&   viewState,
                        const css::rendering::RenderState& renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawBitmapModulated( const css::rendering::XCanvas*        pCanvas,
                                 const css::uno::Reference<
                                         css::rendering::XBitmap >&        xBitmap,
                                 const css::rendering::ViewState&      viewState,
                                 const css::rendering::RenderState&    renderState );
        css::uno::Reference< css::rendering::XGraphicDevice >
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
        css::rendering::XGraphicDevice* mpDevice;

        /// Provides the Gdiplus::Graphics to render into
        GraphicsProviderSharedPtr                  mpGraphicsProvider;

        bool needOutput() const { return bool(mpGraphicsProvider); };

        // returns transparency of color
        void setupGraphicsState( GraphicsSharedPtr const & rGraphics,
                                 const css::rendering::ViewState&   viewState,
                                 const css::rendering::RenderState& renderState );

        Gdiplus::CompositingMode    calcCompositingMode( sal_Int8 nMode );

        /// Current (transformation-independent) output buffer offset
        ::basegfx::B2ISize              maOutputOffset;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
