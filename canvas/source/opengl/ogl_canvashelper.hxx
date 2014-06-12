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
        void init( ::com::sun::star::rendering::XGraphicDevice& rDevice,
                   SpriteDeviceHelper& rDeviceHelper );

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
                                    ::com::sun::star::rendering::XPolyPolygon2D >&  xPolyPolygon,
                             const ::com::sun::star::rendering::ViewState&          viewState,
                             const ::com::sun::star::rendering::RenderState&        renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            strokePolyPolygon( const ::com::sun::star::rendering::XCanvas*          pCanvas,
                               const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::rendering::XPolyPolygon2D >&  xPolyPolygon,
                               const ::com::sun::star::rendering::ViewState&        viewState,
                               const ::com::sun::star::rendering::RenderState&      renderState,
                               const ::com::sun::star::rendering::StrokeAttributes& strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            strokeTexturedPolyPolygon( const ::com::sun::star::rendering::XCanvas*          pCanvas,
                                       const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                       const ::com::sun::star::rendering::ViewState&        viewState,
                                       const ::com::sun::star::rendering::RenderState&      renderState,
                                       const ::com::sun::star::uno::Sequence<
                                            ::com::sun::star::rendering::Texture >&         textures,
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
                                    ::com::sun::star::rendering::XPolyPolygon2D >&  xPolyPolygon,
                               const ::com::sun::star::rendering::ViewState&        viewState,
                               const ::com::sun::star::rendering::RenderState&      renderState,
                               const ::com::sun::star::rendering::StrokeAttributes& strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillPolyPolygon( const ::com::sun::star::rendering::XCanvas*            pCanvas,
                             const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::rendering::XPolyPolygon2D >&  xPolyPolygon,
                             const ::com::sun::star::rendering::ViewState&          viewState,
                             const ::com::sun::star::rendering::RenderState&        renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillTexturedPolyPolygon( const ::com::sun::star::rendering::XCanvas*            pCanvas,
                                     const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                     const ::com::sun::star::rendering::ViewState&          viewState,
                                     const ::com::sun::star::rendering::RenderState&        renderState,
                                     const ::com::sun::star::uno::Sequence<
                                            ::com::sun::star::rendering::Texture >&         textures );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillTextureMappedPolyPolygon( const ::com::sun::star::rendering::XCanvas*           pCanvas,
                                          const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                          const ::com::sun::star::rendering::ViewState&         viewState,
                                          const ::com::sun::star::rendering::RenderState&       renderState,
                                          const ::com::sun::star::uno::Sequence<
                                                ::com::sun::star::rendering::Texture >&         textures,
                                          const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::geometry::XMapping2D >&       xMapping );

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
                                        ::com::sun::star::beans::PropertyValue >&   aFontProperties );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawText( const ::com::sun::star::rendering::XCanvas*           pCanvas,
                      const ::com::sun::star::rendering::StringContext&     text,
                      const ::com::sun::star::uno::Reference<
                            ::com::sun::star::rendering::XCanvasFont >&     xFont,
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
                                        ::com::sun::star::rendering::XBitmap >&     xBitmap,
                                 const ::com::sun::star::rendering::ViewState&      viewState,
                                 const ::com::sun::star::rendering::RenderState&    renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >
            getDevice() { return css::uno::Reference< css::rendering::XGraphicDevice >(mpDevice); }

        // Flush drawing queue to screen
        void                    flush() const;

        /** Called from XCanvas base classes, to notify that content
            is _about_ to change
        */
        void modifying() {}

        /** Write out recorded actions
         */
        bool renderRecordedActions() const;

        /** Retrieve number of recorded actions
         */
        size_t getRecordedActionCount() const;

        SpriteDeviceHelper* getDeviceHelper() const { return mpDeviceHelper; }
        ::com::sun::star::rendering::XGraphicDevice* getDevice() const { return mpDevice; }

        struct Action;
        typedef o3tl::cow_wrapper< std::vector<Action>,
                                   o3tl::ThreadSafeRefCountingPolicy > RecordVectorT;

    private:
        // declared, but not defined
        CanvasHelper( const CanvasHelper& );

        void setupGraphicsState( Action&                                         o_action,
                                 const ::com::sun::star::rendering::ViewState&   viewState,
                                 const ::com::sun::star::rendering::RenderState& renderState );

        /** Phyical output device

            Deliberately not a refcounted reference, because of
            potential circular references for spritecanvas.
         */
        ::com::sun::star::rendering::XGraphicDevice* mpDevice;

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
