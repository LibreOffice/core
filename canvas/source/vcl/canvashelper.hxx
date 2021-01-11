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

#include "cachedbitmap.hxx"
#include "outdevprovider.hxx"


namespace vclcanvas
{
    class SpriteCanvas;

    /** Helper class for basic canvas functionality. Also offers
        optional backbuffer painting, when providing it with a second
        OutputDevice to render into.
     */
    class CanvasHelper
    {
    public:
        /** Create canvas helper
         */
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

            @param rOutDev
            Set primary output device for this canvas. That's where
            all content is output to.

            @param bProtect
            When true, all output operations preserve outdev
            state. When false, outdev state might change at any time.

            @param bHaveAlpha
            When true, hasAlpha() will always return true, otherwise, false.
         */
        void init( css::rendering::XGraphicDevice&              rDevice,
                   const OutDevProviderSharedPtr&               rOutDev,
                   bool                                         bProtect,
                   bool                                         bHaveAlpha );

        /** Set primary output device

            This changes the primary output device, where rendering is
            sent to.
         */
        void setOutDev( const OutDevProviderSharedPtr&  rOutDev,
                        bool                            bProtect);

        /** Set secondary output device

            Used for sprites, to generate mask bitmap.
         */
        void setBackgroundOutDev( const OutDevProviderSharedPtr& rOutDev );


        // CanvasHelper functionality
        // ==========================

        // XCanvas (only providing, not implementing the
        // interface. Also note subtle method parameter differences)
        void clear();
        void drawLine( const css::rendering::XCanvas*      rCanvas,
                       const css::geometry::RealPoint2D&   aStartPoint,
                       const css::geometry::RealPoint2D&   aEndPoint,
                       const css::rendering::ViewState&    viewState,
                       const css::rendering::RenderState&  renderState );
        void drawBezier( const css::rendering::XCanvas*            rCanvas,
                         const css::geometry::RealBezierSegment2D& aBezierSegment,
                         const css::geometry::RealPoint2D&         aEndPoint,
                         const css::rendering::ViewState&          viewState,
                         const css::rendering::RenderState&        renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawPolyPolygon( const css::rendering::XCanvas*        rCanvas,
                             const css::uno::Reference<
                                 css::rendering::XPolyPolygon2D >&     xPolyPolygon,
                             const css::rendering::ViewState&      viewState,
                             const css::rendering::RenderState&    renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            strokePolyPolygon( const css::rendering::XCanvas*          rCanvas,
                               const css::uno::Reference<
                                       css::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const css::rendering::ViewState&        viewState,
                               const css::rendering::RenderState&      renderState,
                               const css::rendering::StrokeAttributes& strokeAttributes );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            strokeTexturedPolyPolygon( const css::rendering::XCanvas*          rCanvas,
                                       const css::uno::Reference<
                                               css::rendering::XPolyPolygon2D >&   xPolyPolygon,
                                       const css::rendering::ViewState&        viewState,
                                       const css::rendering::RenderState&      renderState,
                                       const css::uno::Sequence<
                                               css::rendering::Texture >&      textures,
                                       const css::rendering::StrokeAttributes& strokeAttributes );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            strokeTextureMappedPolyPolygon( const css::rendering::XCanvas*             rCanvas,
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
            queryStrokeShapes( const css::rendering::XCanvas*          rCanvas,
                               const css::uno::Reference<
                                       css::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const css::rendering::ViewState&        viewState,
                               const css::rendering::RenderState&      renderState,
                               const css::rendering::StrokeAttributes& strokeAttributes );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillPolyPolygon( const css::rendering::XCanvas*            rCanvas,
                             const css::uno::Reference<
                                     css::rendering::XPolyPolygon2D >&     xPolyPolygon,
                             const css::rendering::ViewState&          viewState,
                             const css::rendering::RenderState&        renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillTexturedPolyPolygon( const css::rendering::XCanvas*            rCanvas,
                                     const css::uno::Reference<
                                             css::rendering::XPolyPolygon2D >& xPolyPolygon,
                                     const css::rendering::ViewState&          viewState,
                                     const css::rendering::RenderState&        renderState,
                                     const css::uno::Sequence<
                                             css::rendering::Texture >&        textures );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillTextureMappedPolyPolygon( const css::rendering::XCanvas*           rCanvas,
                                          const css::uno::Reference<
                                                  css::rendering::XPolyPolygon2D >&    xPolyPolygon,
                                          const css::rendering::ViewState&         viewState,
                                          const css::rendering::RenderState&       renderState,
                                          const css::uno::Sequence<
                                                  css::rendering::Texture >&       textures,
                                          const css::uno::Reference<
                                                  css::geometry::XMapping2D >&         xMapping );

        css::uno::Reference< css::rendering::XCanvasFont >
            createFont( const css::rendering::XCanvas*         rCanvas,
                        const css::rendering::FontRequest&     fontRequest,
                        const css::uno::Sequence<
                            css::beans::PropertyValue >&       extraFontProperties,
                        const css::geometry::Matrix2D&         fontMatrix );

        css::uno::Sequence< css::rendering::FontInfo >
            queryAvailableFonts( const css::rendering::XCanvas*        rCanvas,
                                 const css::rendering::FontInfo&       aFilter,
                                 const css::uno::Sequence<
                                         css::beans::PropertyValue >&  aFontProperties );

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawText( const css::rendering::XCanvas*       rCanvas,
                      const css::rendering::StringContext& text,
                      const css::uno::Reference<
                              css::rendering::XCanvasFont >& xFont,
                      const css::rendering::ViewState&     viewState,
                      const css::rendering::RenderState&   renderState,
                      sal_Int8                                          textDirection );

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawTextLayout( const css::rendering::XCanvas*         rCanvas,
                            const css::uno::Reference<
                                    css::rendering::XTextLayout >& laidOutText,
                            const css::rendering::ViewState&       viewState,
                            const css::rendering::RenderState&     renderState );

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawBitmap( const css::rendering::XCanvas*     rCanvas,
                        const css::uno::Reference<
                                css::rendering::XBitmap >& xBitmap,
                        const css::rendering::ViewState&   viewState,
                        const css::rendering::RenderState& renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawBitmapModulated( const css::rendering::XCanvas*        rCanvas,
                                 const css::uno::Reference<
                                         css::rendering::XBitmap >&        xBitmap,
                                 const css::rendering::ViewState&      viewState,
                                 const css::rendering::RenderState&    renderState );
        // cast away const, need to change refcount (as this is
        // ~invisible to client code, still logically const)
        css::uno::Reference< css::rendering::XGraphicDevice >
            getDevice() { return css::uno::Reference< css::rendering::XGraphicDevice >(mpDevice); }


        // BitmapCanvasHelper functionality
        // ================================

        css::geometry::IntegerSize2D getSize();

        css::uno::Reference< css::rendering::XBitmap >
            getScaledBitmap( const css::geometry::RealSize2D&  newSize,
                             bool                              beFast );

        css::uno::Sequence< sal_Int8 >
            getData( css::rendering::IntegerBitmapLayout&      bitmapLayout,
                     const css::geometry::IntegerRectangle2D&  rect );

        css::uno::Sequence< sal_Int8 >
            getPixel( css::rendering::IntegerBitmapLayout& bitmapLayout,
                      const css::geometry::IntegerPoint2D& pos );

        css::rendering::IntegerBitmapLayout getMemoryLayout();

        /// Repaint a cached bitmap
        bool repaint( const GraphicObjectSharedPtr&                   rGrf,
                      const css::rendering::ViewState&                viewState,
                      const css::rendering::RenderState&              renderState,
                      const ::Point&                                  rPt,
                      const ::Size&                                   rSz,
                      const GraphicAttr&                              rAttr ) const;

        /** Flush drawing queue.

            This only works for Window canvases, and ensures that all
            pending render operations are flushed to the
            driver/hardware.
         */
        void flush() const;

        enum ColorType
        {
            LINE_COLOR, FILL_COLOR, TEXT_COLOR, IGNORE_COLOR
        };

        // returns alpha of color
        int setupOutDevState( const css::rendering::ViewState&     viewState,
                              const css::rendering::RenderState&   renderState,
                              ColorType                            eColorType ) const;

        bool hasAlpha() const { return mbHaveAlpha; }

    protected:
        /** Phyical output device

            Deliberately not a refcounted reference, because of
            potential circular references for spritecanvas.
         */
        css::rendering::XGraphicDevice*              mpDevice;

        /// Rendering to this outdev preserves its state
        OutDevProviderSharedPtr                      mpProtectedOutDevProvider;

        /// Rendering to this outdev does not preserve its state
        OutDevProviderSharedPtr                      mpOutDevProvider;

        /// Rendering to this outdev does not preserve its state
        OutDevProviderSharedPtr                      mp2ndOutDevProvider;

        /// When true, content is able to represent alpha
        bool                                         mbHaveAlpha;

    private:
        css::uno::Reference< css::rendering::XCachedPrimitive >
            implDrawBitmap( const css::rendering::XCanvas*     rCanvas,
                            const css::uno::Reference<
                                    css::rendering::XBitmap >&     xBitmap,
                            const css::rendering::ViewState&   viewState,
                            const css::rendering::RenderState& renderState,
                            bool                                            bModulateColors );

        bool setupTextOutput( ::Point&                                                                              o_rOutPos,
                              const css::rendering::ViewState&                                         viewState,
                              const css::rendering::RenderState&                                       renderState,
                              const css::uno::Reference< css::rendering::XCanvasFont >&   xFont ) const;

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
