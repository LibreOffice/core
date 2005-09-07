/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvashelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:19:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _VCLCANVAS_CANVASHELPER_HXX_
#define _VCLCANVAS_CANVASHELPER_HXX_

#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP_
#include <com/sun/star/rendering/XCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XINTEGERBITMAP_HPP_
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#include <canvas/vclwrapper.hxx>

#include "outdevprovider.hxx"
#include "cachedbitmap.hxx"
#include "windowgraphicdevice.hxx"


namespace vclcanvas
{
    /** Helper class for basic canvas functionality. Also offers
        optional backbuffer painting, when providing it with a second
        OutputDevice to render into.
     */
    class CanvasHelper
    {
    public:
        CanvasHelper();

        /// Release all references
        void disposing();

        void setGraphicDevice( const WindowGraphicDevice::ImplRef& rDevice );

        /** Set primary output device

            @param bProtect
            When true, all output operations preserve outdev
            state. When false, outdev state might change at any time.
         */
        void setOutDev( const OutDevProviderSharedPtr& rOutDev, bool bProtect );

        /** Set secondary output device

            Used for sprites, to generate mask bitmap.
         */
        void setBackgroundOutDev( const OutDevProviderSharedPtr& rOutDev );

        // CanvasHelper functionality
        // ==========================

        // XCanvas (only providing, not implementing the
        // interface. Also note subtle method parameter differences)
        void drawPoint( const ::com::sun::star::rendering::XCanvas&         rCanvas,
                        const ::com::sun::star::geometry::RealPoint2D&      aPoint,
                        const ::com::sun::star::rendering::ViewState&       viewState,
                        const ::com::sun::star::rendering::RenderState&     renderState );
        void drawLine( const ::com::sun::star::rendering::XCanvas&      rCanvas,
                       const ::com::sun::star::geometry::RealPoint2D&   aStartPoint,
                       const ::com::sun::star::geometry::RealPoint2D&   aEndPoint,
                       const ::com::sun::star::rendering::ViewState&    viewState,
                       const ::com::sun::star::rendering::RenderState&  renderState );
        void drawBezier( const ::com::sun::star::rendering::XCanvas&            rCanvas,
                         const ::com::sun::star::geometry::RealBezierSegment2D& aBezierSegment,
                         const ::com::sun::star::geometry::RealPoint2D&         aEndPoint,
                         const ::com::sun::star::rendering::ViewState&          viewState,
                         const ::com::sun::star::rendering::RenderState&        renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawPolyPolygon( const ::com::sun::star::rendering::XCanvas&        rCanvas,
                             const ::com::sun::star::uno::Reference<
                                 ::com::sun::star::rendering::XPolyPolygon2D >&     xPolyPolygon,
                             const ::com::sun::star::rendering::ViewState&      viewState,
                             const ::com::sun::star::rendering::RenderState&    renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            strokePolyPolygon( const ::com::sun::star::rendering::XCanvas&          rCanvas,
                               const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::com::sun::star::rendering::ViewState&        viewState,
                               const ::com::sun::star::rendering::RenderState&      renderState,
                               const ::com::sun::star::rendering::StrokeAttributes& strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            strokeTexturedPolyPolygon( const ::com::sun::star::rendering::XCanvas&          rCanvas,
                                       const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                                       const ::com::sun::star::rendering::ViewState&        viewState,
                                       const ::com::sun::star::rendering::RenderState&      renderState,
                                       const ::com::sun::star::uno::Sequence<
                                               ::com::sun::star::rendering::Texture >&      textures,
                                       const ::com::sun::star::rendering::StrokeAttributes& strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            strokeTextureMappedPolyPolygon( const ::com::sun::star::rendering::XCanvas&             rCanvas,
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
            queryStrokeShapes( const ::com::sun::star::rendering::XCanvas&          rCanvas,
                               const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::com::sun::star::rendering::ViewState&        viewState,
                               const ::com::sun::star::rendering::RenderState&      renderState,
                               const ::com::sun::star::rendering::StrokeAttributes& strokeAttributes );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillPolyPolygon( const ::com::sun::star::rendering::XCanvas&            rCanvas,
                             const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::rendering::XPolyPolygon2D >&     xPolyPolygon,
                             const ::com::sun::star::rendering::ViewState&          viewState,
                             const ::com::sun::star::rendering::RenderState&        renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillTexturedPolyPolygon( const ::com::sun::star::rendering::XCanvas&            rCanvas,
                                     const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                                     const ::com::sun::star::rendering::ViewState&          viewState,
                                     const ::com::sun::star::rendering::RenderState&        renderState,
                                     const ::com::sun::star::uno::Sequence<
                                             ::com::sun::star::rendering::Texture >&        textures );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            fillTextureMappedPolyPolygon( const ::com::sun::star::rendering::XCanvas&           rCanvas,
                                          const ::com::sun::star::uno::Reference<
                                                  ::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
                                          const ::com::sun::star::rendering::ViewState&         viewState,
                                          const ::com::sun::star::rendering::RenderState&       renderState,
                                          const ::com::sun::star::uno::Sequence<
                                                  ::com::sun::star::rendering::Texture >&       textures,
                                          const ::com::sun::star::uno::Reference<
                                                  ::com::sun::star::geometry::XMapping2D >&         xMapping );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont >
            createFont( const ::com::sun::star::rendering::XCanvas&         rCanvas,
                        const ::com::sun::star::rendering::FontRequest&     fontRequest,
                        const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::PropertyValue >&       extraFontProperties,
                        const ::com::sun::star::geometry::Matrix2D&         fontMatrix );

        ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::FontInfo >
            queryAvailableFonts( const ::com::sun::star::rendering::XCanvas&        rCanvas,
                                 const ::com::sun::star::rendering::FontInfo&       aFilter,
                                 const ::com::sun::star::uno::Sequence<
                                         ::com::sun::star::beans::PropertyValue >&  aFontProperties );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawText( const ::com::sun::star::rendering::XCanvas&       rCanvas,
                      const ::com::sun::star::rendering::StringContext& text,
                      const ::com::sun::star::uno::Reference<
                              ::com::sun::star::rendering::XCanvasFont >& xFont,
                      const ::com::sun::star::rendering::ViewState&     viewState,
                      const ::com::sun::star::rendering::RenderState&   renderState,
                      sal_Int8                                          textDirection );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawTextLayout( const ::com::sun::star::rendering::XCanvas&         rCanvas,
                            const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::rendering::XTextLayout >& layoutetText,
                            const ::com::sun::star::rendering::ViewState&       viewState,
                            const ::com::sun::star::rendering::RenderState&     renderState );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawBitmap( const ::com::sun::star::rendering::XCanvas&     rCanvas,
                        const ::com::sun::star::uno::Reference<
                                ::com::sun::star::rendering::XBitmap >& xBitmap,
                        const ::com::sun::star::rendering::ViewState&   viewState,
                        const ::com::sun::star::rendering::RenderState& renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawBitmapModulated( const ::com::sun::star::rendering::XCanvas&        rCanvas,
                                 const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::rendering::XBitmap >&        xBitmap,
                                 const ::com::sun::star::rendering::ViewState&      viewState,
                                 const ::com::sun::star::rendering::RenderState&    renderState );
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >
            getDevice();

        // BitmapCanvasHelper functionality
        // ================================

        void copyRect( const ::com::sun::star::rendering::XCanvas&          rCanvas,
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
            getData( const ::com::sun::star::geometry::IntegerRectangle2D& rect );

        void setData( const ::com::sun::star::uno::Sequence< sal_Int8 >&                data,
                               const ::com::sun::star::geometry::IntegerRectangle2D&    rect );

        void setPixel( const ::com::sun::star::uno::Sequence< sal_Int8 >&           color,
                                const ::com::sun::star::geometry::IntegerPoint2D&   pos );

        ::com::sun::star::uno::Sequence< sal_Int8 >
            getPixel( const ::com::sun::star::geometry::IntegerPoint2D& pos );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapPalette > getPalette();

        ::com::sun::star::rendering::IntegerBitmapLayout getMemoryLayout();

        /// Repaint a cached bitmap
        bool repaint( const GraphicObjectSharedPtr& rGrf,
                      const ::Point&                rPt,
                      const ::Size&                 rSz,
                      const GraphicAttr&            rAttr ) const;

        // Flush drawing queue to screen (only works for Window outdev)
        void                    flush() const;

        enum ColorType
        {
            LINE_COLOR, FILL_COLOR, TEXT_COLOR, IGNORE_COLOR
        };

        // returns transparency of color
        int setupOutDevState( const ::com::sun::star::rendering::ViewState&     viewState,
                              const ::com::sun::star::rendering::RenderState&   renderState,
                              ColorType                                         eColorType );
    protected:
        /// Phyical output device
        WindowGraphicDevice::ImplRef    mxDevice;

        /// Render to this outdev preserves its state
        OutDevProviderSharedPtr         mpProtectedOutDev;

        OutDevProviderSharedPtr         mpOutDev;
        OutDevProviderSharedPtr         mp2ndOutDev;

    private:
        // default: disabled copy/assignment
        CanvasHelper(const CanvasHelper&);
        CanvasHelper& operator=( const CanvasHelper& );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            implDrawBitmap( const ::com::sun::star::rendering::XCanvas&     rCanvas,
                            const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::rendering::XBitmap >&     xBitmap,
                            const ::com::sun::star::rendering::ViewState&   viewState,
                            const ::com::sun::star::rendering::RenderState& renderState,
                            bool                                            bModulateColors );

        bool setupTextOutput( ::Point&                                                                                      o_rOutPos,
                              const ::com::sun::star::rendering::ViewState&                                         viewState,
                              const ::com::sun::star::rendering::RenderState&                                       renderState,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont >&   xFont );

    };
}

#endif /* _VCLCANVAS_CANVASHELPER_HXX_ */
