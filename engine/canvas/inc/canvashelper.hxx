/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "cachedbitmap.hxx"
#include "outdevprovider.hxx"


namespace vclcanvas
{

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
        void init( vclcanvas::XGraphicDevice&                   rDevice,
                   const OutDevProviderSharedPtr&               rOutDev,
                   bool                                         bProtect,
                   bool                                         bHaveAlpha );

        /** Set primary output device

            This changes the primary output device, where rendering is
            sent to.
         */
        void setOutDev( const OutDevProviderSharedPtr&  rOutDev,
                        bool                            bProtect);


        // CanvasHelper functionality
        // ==========================

        // XCanvas (only providing, not implementing the
        // interface. Also note subtle method parameter differences)
        void clear();
        void drawLine( const vclcanvas::XCanvas*      rCanvas,
                       const css::geometry::RealPoint2D&   aStartPoint,
                       const css::geometry::RealPoint2D&   aEndPoint,
                       const ::vclcanvas::ViewState&    viewState,
                       const ::vclcanvas::RenderState&  renderState );
        void drawPolyPolygon( const vclcanvas::XCanvas*        rCanvas,
                             const css::uno::Reference<
                                 css::rendering::XPolyPolygon2D >&     xPolyPolygon,
                             const ::vclcanvas::ViewState&      viewState,
                             const ::vclcanvas::RenderState&    renderState );
        void strokePolyPolygon( const vclcanvas::XCanvas*          rCanvas,
                               const css::uno::Reference<
                                       css::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::vclcanvas::ViewState&        viewState,
                               const ::vclcanvas::RenderState&      renderState,
                               const css::rendering::StrokeAttributes& strokeAttributes );
        css::uno::Reference< vclcanvas::XCachedPrimitive >
            fillPolyPolygon( const vclcanvas::XCanvas*            rCanvas,
                             const css::uno::Reference<
                                     css::rendering::XPolyPolygon2D >&     xPolyPolygon,
                             const ::vclcanvas::ViewState&          viewState,
                             const ::vclcanvas::RenderState&        renderState );
        css::uno::Reference< vclcanvas::XCachedPrimitive >
            fillTexturedPolyPolygon( const vclcanvas::XCanvas*            rCanvas,
                                     const css::uno::Reference<
                                             css::rendering::XPolyPolygon2D >& xPolyPolygon,
                                     const ::vclcanvas::ViewState&          viewState,
                                     const ::vclcanvas::RenderState&        renderState,
                                     const cpo::uno::Sequence<
                                             css::rendering::Texture >&        textures );

        css::uno::Reference< css::rendering::XCanvasFont >
            createFont( const vclcanvas::XCanvas*         rCanvas,
                        const css::rendering::FontRequest&     fontRequest,
                        const cpo::uno::Sequence<
                            css::beans::PropertyValue >&       extraFontProperties,
                        const css::geometry::Matrix2D&         fontMatrix );

        void drawText( const vclcanvas::XCanvas*       rCanvas,
                      const css::rendering::StringContext& text,
                      const css::uno::Reference<
                              css::rendering::XCanvasFont >& xFont,
                      const ::vclcanvas::ViewState&     viewState,
                      const ::vclcanvas::RenderState&   renderState,
                      sal_Int8                                          textDirection );

        void drawTextLayout( const vclcanvas::XCanvas*         rCanvas,
                            const css::uno::Reference<
                                    css::rendering::XTextLayout >& laidOutText,
                            const ::vclcanvas::ViewState&       viewState,
                            const ::vclcanvas::RenderState&     renderState );

        css::uno::Reference< vclcanvas::XCachedPrimitive >
            drawBitmap( const vclcanvas::XCanvas*     rCanvas,
                        const css::uno::Reference<
                                css::rendering::XBitmap >& xBitmap,
                        const ::vclcanvas::ViewState&   viewState,
                        const ::vclcanvas::RenderState& renderState );
        css::uno::Reference< vclcanvas::XCachedPrimitive >
            drawBitmapModulated( const vclcanvas::XCanvas*        rCanvas,
                                 const css::uno::Reference<
                                         css::rendering::XBitmap >&        xBitmap,
                                 const ::vclcanvas::ViewState&      viewState,
                                 const ::vclcanvas::RenderState&    renderState );
        // cast away const, need to change refcount (as this is
        // ~invisible to client code, still logically const)
        css::uno::Reference< vclcanvas::XGraphicDevice >
            getDevice() { return css::uno::Reference< vclcanvas::XGraphicDevice >(mpDevice); }


        // BitmapCanvasHelper functionality
        // ================================

        css::geometry::IntegerSize2D getSize();

        /// Repaint a cached bitmap
        bool repaint( const GraphicObjectSharedPtr&                   rGrf,
                      const ::vclcanvas::ViewState&                viewState,
                      const ::vclcanvas::RenderState&              renderState,
                      const ::Point&                                  rPt,
                      const ::Size&                                   rSz,
                      const GraphicAttr&                              rAttr ) const;

        enum ColorType
        {
            LINE_COLOR, FILL_COLOR, TEXT_COLOR, IGNORE_COLOR
        };

        // returns alpha of color
        int setupOutDevState( const ::vclcanvas::ViewState&     viewState,
                              const ::vclcanvas::RenderState&   renderState,
                              ColorType                            eColorType ) const;

    protected:
        /** Phyical output device

            Deliberately not a refcounted reference, because of
            potential circular references for spritecanvas.
         */
        vclcanvas::XGraphicDevice*                   mpDevice;

        /// Rendering to this outdev preserves its state
        OutDevProviderSharedPtr                      mpProtectedOutDevProvider;

        /// Rendering to this outdev does not preserve its state
        OutDevProviderSharedPtr                      mpOutDevProvider;

        /// When true, content is able to represent alpha
        bool                                         mbHaveAlpha;

    private:
        css::uno::Reference< vclcanvas::XCachedPrimitive >
            implDrawBitmap( const vclcanvas::XCanvas*     rCanvas,
                            const css::uno::Reference<
                                    css::rendering::XBitmap >&     xBitmap,
                            const ::vclcanvas::ViewState&   viewState,
                            const ::vclcanvas::RenderState& renderState,
                            bool                                            bModulateColors );

        bool setupTextOutput( ::Point&                                                                              o_rOutPos,
                              const ::vclcanvas::ViewState&                                         viewState,
                              const ::vclcanvas::RenderState&                                       renderState,
                              const css::uno::Reference< css::rendering::XCanvasFont >&   xFont ) const;

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
