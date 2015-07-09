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

#ifndef INCLUDED_CANVAS_SOURCE_VCL_CANVASHELPER_HXX
#define INCLUDED_CANVAS_SOURCE_VCL_CANVASHELPER_HXX

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <vcl/outdev.hxx>

#include <canvas/vclwrapper.hxx>

#include "cachedbitmap.hxx"
#include "outdevprovider.hxx"

#include <boost/noncopyable.hpp>


namespace vclcanvas
{
    class SpriteCanvas;

    /** Helper class for basic canvas functionality. Also offers
        optional backbuffer painting, when providing it with a second
        OutputDevice to render into.
     */
    class CanvasHelper : private ::boost::noncopyable
    {
    public:
        /** Create canvas helper
         */
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

            @param rOutDev
            Set primary output device for this canvas. That's where
            all content is output to.

            @param bProtect
            When true, all output operations preserve outdev
            state. When false, outdev state might change at any time.

            @param bHaveAlpha
            When true, hasAlpha() will always return true, otherwise, false.
         */
        void init( ::com::sun::star::rendering::XGraphicDevice& rDevice,
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
        void drawPoint( const ::com::sun::star::rendering::XCanvas*         rCanvas,
                        const ::com::sun::star::geometry::RealPoint2D&      aPoint,
                        const ::com::sun::star::rendering::ViewState&       viewState,
                        const ::com::sun::star::rendering::RenderState&     renderState );
        void drawLine( const ::com::sun::star::rendering::XCanvas*      rCanvas,
                       const ::com::sun::star::geometry::RealPoint2D&   aStartPoint,
                       const ::com::sun::star::geometry::RealPoint2D&   aEndPoint,
                       const ::com::sun::star::rendering::ViewState&    viewState,
                       const ::com::sun::star::rendering::RenderState&  renderState );
        void drawBezier( const ::com::sun::star::rendering::XCanvas*            rCanvas,
                         const ::com::sun::star::geometry::RealBezierSegment2D& aBezierSegment,
                         const ::com::sun::star::geometry::RealPoint2D&         aEndPoint,
                         const ::com::sun::star::rendering::ViewState&          viewState,
                         const ::com::sun::star::rendering::RenderState&        renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawPolyPolygon( const ::com::sun::star::rendering::XCanvas*        rCanvas,
                             const ::com::sun::star::uno::Reference<
                                 ::com::sun::star::rendering::XPolyPolygon2D >&     xPolyPolygon,
                             const ::com::sun::star::rendering::ViewState&      viewState,
                             const ::com::sun::star::rendering::RenderState&    renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            strokePolyPolygon( const ::com::sun::star::rendering::XCanvas*          rCanvas,
                               const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::com::sun::star::rendering::ViewState&        viewState,
                               const ::com::sun::star::rendering::RenderState&      renderState,
                               const ::com::sun::star::rendering::StrokeAttributes& strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            strokeTexturedPolyPolygon( const ::com::sun::star::rendering::XCanvas*          rCanvas,
                                       const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                                       const ::com::sun::star::rendering::ViewState&        viewState,
                                       const ::com::sun::star::rendering::RenderState&      renderState,
                                       const ::com::sun::star::uno::Sequence<
                                               ::com::sun::star::rendering::Texture >&      textures,
                                       const ::com::sun::star::rendering::StrokeAttributes& strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            strokeTextureMappedPolyPolygon( const ::com::sun::star::rendering::XCanvas*             rCanvas,
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
            queryStrokeShapes( const ::com::sun::star::rendering::XCanvas*          rCanvas,
                               const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::com::sun::star::rendering::ViewState&        viewState,
                               const ::com::sun::star::rendering::RenderState&      renderState,
                               const ::com::sun::star::rendering::StrokeAttributes& strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillPolyPolygon( const ::com::sun::star::rendering::XCanvas*            rCanvas,
                             const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::rendering::XPolyPolygon2D >&     xPolyPolygon,
                             const ::com::sun::star::rendering::ViewState&          viewState,
                             const ::com::sun::star::rendering::RenderState&        renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillTexturedPolyPolygon( const ::com::sun::star::rendering::XCanvas*            rCanvas,
                                     const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                                     const ::com::sun::star::rendering::ViewState&          viewState,
                                     const ::com::sun::star::rendering::RenderState&        renderState,
                                     const ::com::sun::star::uno::Sequence<
                                             ::com::sun::star::rendering::Texture >&        textures );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillTextureMappedPolyPolygon( const ::com::sun::star::rendering::XCanvas*           rCanvas,
                                          const ::com::sun::star::uno::Reference<
                                                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
                                          const ::com::sun::star::rendering::ViewState&         viewState,
                                          const ::com::sun::star::rendering::RenderState&       renderState,
                                          const ::com::sun::star::uno::Sequence<
                                                  ::com::sun::star::rendering::Texture >&       textures,
                                          const ::com::sun::star::uno::Reference<
                                                  ::com::sun::star::geometry::XMapping2D >&         xMapping );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont >
            createFont( const ::com::sun::star::rendering::XCanvas*         rCanvas,
                        const ::com::sun::star::rendering::FontRequest&     fontRequest,
                        const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::PropertyValue >&       extraFontProperties,
                        const ::com::sun::star::geometry::Matrix2D&         fontMatrix );

        ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::FontInfo >
            queryAvailableFonts( const ::com::sun::star::rendering::XCanvas*        rCanvas,
                                 const ::com::sun::star::rendering::FontInfo&       aFilter,
                                 const ::com::sun::star::uno::Sequence<
                                         ::com::sun::star::beans::PropertyValue >&  aFontProperties );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawText( const ::com::sun::star::rendering::XCanvas*       rCanvas,
                      const ::com::sun::star::rendering::StringContext& text,
                      const ::com::sun::star::uno::Reference<
                              ::com::sun::star::rendering::XCanvasFont >& xFont,
                      const ::com::sun::star::rendering::ViewState&     viewState,
                      const ::com::sun::star::rendering::RenderState&   renderState,
                      sal_Int8                                          textDirection );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawTextLayout( const ::com::sun::star::rendering::XCanvas*         rCanvas,
                            const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::rendering::XTextLayout >& laidOutText,
                            const ::com::sun::star::rendering::ViewState&       viewState,
                            const ::com::sun::star::rendering::RenderState&     renderState );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawBitmap( const ::com::sun::star::rendering::XCanvas*     rCanvas,
                        const ::com::sun::star::uno::Reference<
                                ::com::sun::star::rendering::XBitmap >& xBitmap,
                        const ::com::sun::star::rendering::ViewState&   viewState,
                        const ::com::sun::star::rendering::RenderState& renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawBitmapModulated( const ::com::sun::star::rendering::XCanvas*        rCanvas,
                                 const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::rendering::XBitmap >&        xBitmap,
                                 const ::com::sun::star::rendering::ViewState&      viewState,
                                 const ::com::sun::star::rendering::RenderState&    renderState );
        // cast away const, need to change refcount (as this is
        // ~invisible to client code, still logically const)
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >
            getDevice() { return css::uno::Reference< css::rendering::XGraphicDevice >(mpDevice); }


        // BitmapCanvasHelper functionality
        // ================================

        ::com::sun::star::geometry::IntegerSize2D getSize();

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > queryBitmapCanvas();

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >
            getScaledBitmap( const ::com::sun::star::geometry::RealSize2D&  newSize,
                             bool                                           beFast );

        ::com::sun::star::uno::Sequence< sal_Int8 >
            getData( ::com::sun::star::rendering::IntegerBitmapLayout&      bitmapLayout,
                     const ::com::sun::star::geometry::IntegerRectangle2D&  rect );

        void setData( const ::com::sun::star::uno::Sequence< sal_Int8 >&        data,
                      const ::com::sun::star::rendering::IntegerBitmapLayout&   bitmapLayout,
                      const ::com::sun::star::geometry::IntegerRectangle2D&     rect );

        void setPixel( const ::com::sun::star::uno::Sequence< sal_Int8 >&       color,
                       const ::com::sun::star::rendering::IntegerBitmapLayout&  bitmapLayout,
                       const ::com::sun::star::geometry::IntegerPoint2D&        pos );

        ::com::sun::star::uno::Sequence< sal_Int8 >
            getPixel( ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
                      const ::com::sun::star::geometry::IntegerPoint2D& pos );

        ::com::sun::star::rendering::IntegerBitmapLayout getMemoryLayout();

        /// Repaint a cached bitmap
        bool repaint( const GraphicObjectSharedPtr&                   rGrf,
                      const ::com::sun::star::rendering::ViewState&   viewState,
                      const ::com::sun::star::rendering::RenderState& renderState,
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

        // returns transparency of color
        int setupOutDevState( const ::com::sun::star::rendering::ViewState&     viewState,
                              const ::com::sun::star::rendering::RenderState&   renderState,
                              ColorType                                         eColorType ) const;

        bool hasAlpha() const { return mbHaveAlpha; }

    protected:
        /** Phyical output device

            Deliberately not a refcounted reference, because of
            potential circular references for spritecanvas.
         */
        ::com::sun::star::rendering::XGraphicDevice* mpDevice;

        /// Rendering to this outdev preserves its state
        OutDevProviderSharedPtr                      mpProtectedOutDev;

        /// Rendering to this outdev does not preserve its state
        OutDevProviderSharedPtr                      mpOutDev;

        /// Rendering to this outdev does not preserve its state
        OutDevProviderSharedPtr                      mp2ndOutDev;

        /// When true, content is able to represent alpha
        bool                                         mbHaveAlpha;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            implDrawBitmap( const ::com::sun::star::rendering::XCanvas*     rCanvas,
                            const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::rendering::XBitmap >&     xBitmap,
                            const ::com::sun::star::rendering::ViewState&   viewState,
                            const ::com::sun::star::rendering::RenderState& renderState,
                            bool                                            bModulateColors );

        bool setupTextOutput( ::Point&                                                                              o_rOutPos,
                              const ::com::sun::star::rendering::ViewState&                                         viewState,
                              const ::com::sun::star::rendering::RenderState&                                       renderState,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont >&   xFont ) const;

    };
}

#endif // INCLUDED_CANVAS_SOURCE_VCL_CANVASHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
