/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CANVAS_SOURCE_OPENGL_OGL_CANVASHELPER_HXX
#define INCLUDED_CANVAS_SOURCE_OPENGL_OGL_CANVASHELPER_HXX

#include <com/sun/star/rendering/XCanvas.hpp>

#include <basegfx/vector/b2isize.hxx>
#include <basegfx/vector/b2dsize.hxx>

#include <o3tl/cow_wrapper.hxx>
#include <vector>

namespace oglcanvas
{
    class SpriteDeviceHelper;

    /** Helper class for basic canvas functionality. */
    class CanvasHelper
    {
    public:
        CanvasHelper();

        // outline because of incomplete type Action
        ~CanvasHelper();
        CanvasHelper& operator=( const CanvasHelper& );

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
        void init( css::rendering::XGraphicDevice& rDevice,
                   SpriteDeviceHelper& rDeviceHelper );

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
                                    css::rendering::XPolyPolygon2D >&  xPolyPolygon,
                             const css::rendering::ViewState&          viewState,
                             const css::rendering::RenderState&        renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            strokePolyPolygon( const css::rendering::XCanvas*          pCanvas,
                               const css::uno::Reference<
                                    css::rendering::XPolyPolygon2D >&  xPolyPolygon,
                               const css::rendering::ViewState&        viewState,
                               const css::rendering::RenderState&      renderState,
                               const css::rendering::StrokeAttributes& strokeAttributes );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            strokeTexturedPolyPolygon( const css::rendering::XCanvas*          pCanvas,
                                       const css::uno::Reference<
                                            css::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                       const css::rendering::ViewState&        viewState,
                                       const css::rendering::RenderState&      renderState,
                                       const css::uno::Sequence<
                                            css::rendering::Texture >&         textures,
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
                                    css::rendering::XPolyPolygon2D >&  xPolyPolygon,
                               const css::rendering::ViewState&        viewState,
                               const css::rendering::RenderState&      renderState,
                               const css::rendering::StrokeAttributes& strokeAttributes );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillPolyPolygon( const css::rendering::XCanvas*            pCanvas,
                             const css::uno::Reference<
                                    css::rendering::XPolyPolygon2D >&  xPolyPolygon,
                             const css::rendering::ViewState&          viewState,
                             const css::rendering::RenderState&        renderState );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillTexturedPolyPolygon( const css::rendering::XCanvas*            pCanvas,
                                     const css::uno::Reference<
                                            css::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                     const css::rendering::ViewState&          viewState,
                                     const css::rendering::RenderState&        renderState,
                                     const css::uno::Sequence<
                                            css::rendering::Texture >&         textures );
        css::uno::Reference< css::rendering::XCachedPrimitive >
            fillTextureMappedPolyPolygon( const css::rendering::XCanvas*           pCanvas,
                                          const css::uno::Reference<
                                                css::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                          const css::rendering::ViewState&         viewState,
                                          const css::rendering::RenderState&       renderState,
                                          const css::uno::Sequence<
                                                css::rendering::Texture >&         textures,
                                          const css::uno::Reference<
                                                css::geometry::XMapping2D >&       xMapping );

        css::uno::Reference< css::rendering::XCanvasFont > SAL_CALL
            createFont( const css::rendering::XCanvas*             pCanvas,
                        const css::rendering::FontRequest&         fontRequest,
                        const css::uno::Sequence<
                                css::beans::PropertyValue >&       extraFontProperties,
                        const css::geometry::Matrix2D&             fontMatrix );

        css::uno::Sequence< css::rendering::FontInfo >
            queryAvailableFonts( const css::rendering::XCanvas*        pCanvas,
                                 const css::rendering::FontInfo&       aFilter,
                                 const css::uno::Sequence<
                                        css::beans::PropertyValue >&   aFontProperties );

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawText( const css::rendering::XCanvas*           pCanvas,
                      const css::rendering::StringContext&     text,
                      const css::uno::Reference<
                            css::rendering::XCanvasFont >&     xFont,
                      const css::rendering::ViewState&         viewState,
                      const css::rendering::RenderState&       renderState,
                      sal_Int8                                              textDirection );

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawTextLayout( const css::rendering::XCanvas*     pCanvas,
                            const css::uno::Reference<
                                css::rendering::XTextLayout >& layoutetText,
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
                                        css::rendering::XBitmap >&     xBitmap,
                                 const css::rendering::ViewState&      viewState,
                                 const css::rendering::RenderState&    renderState );
        css::uno::Reference< css::rendering::XGraphicDevice >
            getDevice() { return css::uno::Reference< css::rendering::XGraphicDevice >(mpDevice); }

        /** Write out recorded actions
         */
        bool renderRecordedActions() const;

        /** Retrieve number of recorded actions
         */
        size_t getRecordedActionCount() const;

        SpriteDeviceHelper* getDeviceHelper() const { return mpDeviceHelper; }
        css::rendering::XGraphicDevice* getDevice() const { return mpDevice; }

        struct Action;
        typedef o3tl::cow_wrapper< std::vector<Action>,
                                   o3tl::ThreadSafeRefCountingPolicy > RecordVectorT;

    private:
        CanvasHelper( const CanvasHelper& ) = delete;

        void setupGraphicsState( Action&                                         o_action,
                                 const css::rendering::ViewState&   viewState,
                                 const css::rendering::RenderState& renderState );

        /** Phyical output device

            Deliberately not a refcounted reference, because of
            potential circular references for spritecanvas.
         */
        css::rendering::XGraphicDevice*              mpDevice;

        /** Internal helper - used for a few global GL objects,
            e.g. shader programs; and caches
         */
        SpriteDeviceHelper*                          mpDeviceHelper;

        /** Ptr to array of recorded render calls

            Gets shared copy-on-write, when this CanvasHelper is
            copied (used e.g. for CanvasBitmap)
         */
        RecordVectorT                                mpRecordedActions;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
