/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvashelper.hxx,v $
 * $Revision: 1.10 $
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

#ifndef _VCLCANVAS_CANVASHELPER_HXX_
#define _VCLCANVAS_CANVASHELPER_HXX_

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <vcl/outdev.hxx>

#include <canvas/vclwrapper.hxx>

#include "cachedbitmap.hxx"
#include "outdevprovider.hxx"

#include <boost/utility.hpp>


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
        void init( SpriteCanvas&                    rDevice,
                   const OutDevProviderSharedPtr&   rOutDev,
                   bool                             bProtect,
                   bool                             bHaveAlpha );

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
                                    ::com::sun::star::rendering::XTextLayout >& layoutetText,
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
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >
            getDevice();

        // BitmapCanvasHelper functionality
        // ================================

        void copyRect( const ::com::sun::star::rendering::XCanvas*          rCanvas,
                       const ::com::sun::star::uno::Reference<
                               ::com::sun::star::rendering::XBitmapCanvas >&    sourceCanvas,
                       const ::com::sun::star::geometry::RealRectangle2D&   sourceRect,
                       const ::com::sun::star::rendering::ViewState&        sourceViewState,
                       const ::com::sun::star::rendering::RenderState&      sourceRenderState,
                       const ::com::sun::star::geometry::RealRectangle2D&   destRect,
                       const ::com::sun::star::rendering::ViewState&        destViewState,
                       const ::com::sun::star::rendering::RenderState&      destRenderState );

        ::com::sun::star::geometry::IntegerSize2D getSize();

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > queryBitmapCanvas();

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >
            getScaledBitmap( const ::com::sun::star::geometry::RealSize2D&  newSize,
                             sal_Bool                                               beFast );

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

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapPalette > getPalette();

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

        /** Called from XCanvas base classes, to notify that content
            is _about_ to change
        */
        void modifying() {}

        bool hasAlpha() const { return mbHaveAlpha; }

    protected:
        /** Phyical output device

            Deliberately not a refcounted reference, because of
            potential circular references for spritecanvas.
         */
        SpriteCanvas*               mpDevice;

        /// Rendering to this outdev preserves its state
        OutDevProviderSharedPtr     mpProtectedOutDev;

        /// Rendering to this outdev does not preserve its state
        OutDevProviderSharedPtr     mpOutDev;

        /// Rendering to this outdev does not preserve its state
        OutDevProviderSharedPtr     mp2ndOutDev;

        /// When true, content is able to represent alpha
        bool                        mbHaveAlpha;

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

#endif /* _VCLCANVAS_CANVASHELPER_HXX_ */
