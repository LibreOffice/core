/*************************************************************************
 *
 *  $RCSfile: canvashelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 17:12:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _VCLCANVAS_CANVASHELPER_HXX_
#define _VCLCANVAS_CANVASHELPER_HXX_

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XCANVAS_HPP_
#include <drafts/com/sun/star/rendering/XCanvas.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XINTEGERBITMAP_HPP_
#include <drafts/com/sun/star/rendering/XIntegerBitmap.hpp>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#include <canvas/vclwrapper.hxx>

#include "outdevprovider.hxx"
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
        void SAL_CALL drawPoint( const ::drafts::com::sun::star::geometry::RealPoint2D&     aPoint,
                                 const ::drafts::com::sun::star::rendering::ViewState&      viewState,
                                 const ::drafts::com::sun::star::rendering::RenderState&    renderState );
        void SAL_CALL drawLine( const ::drafts::com::sun::star::geometry::RealPoint2D&  aStartPoint,
                                const ::drafts::com::sun::star::geometry::RealPoint2D&  aEndPoint,
                                const ::drafts::com::sun::star::rendering::ViewState&   viewState,
                                const ::drafts::com::sun::star::rendering::RenderState& renderState );
        void SAL_CALL drawBezier( const ::drafts::com::sun::star::geometry::RealBezierSegment2D&    aBezierSegment,
                                  const ::drafts::com::sun::star::geometry::RealPoint2D&            aEndPoint,
                                  const ::drafts::com::sun::star::rendering::ViewState&             viewState,
                                  const ::drafts::com::sun::star::rendering::RenderState&           renderState );
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                             const ::drafts::com::sun::star::rendering::ViewState&                                          viewState,
                             const ::drafts::com::sun::star::rendering::RenderState&                                        renderState );
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            strokePolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::drafts::com::sun::star::rendering::ViewState&                                            viewState,
                               const ::drafts::com::sun::star::rendering::RenderState&                                          renderState,
                               const ::drafts::com::sun::star::rendering::StrokeAttributes&                                     strokeAttributes );
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            strokeTexturedPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                                       const ::drafts::com::sun::star::rendering::ViewState&                                            viewState,
                                       const ::drafts::com::sun::star::rendering::RenderState&                                          renderState,
                                       const ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::rendering::Texture >&           textures,
                                       const ::drafts::com::sun::star::rendering::StrokeAttributes&                                     strokeAttributes );
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            strokeTextureMappedPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >&  xPolyPolygon,
                                            const ::drafts::com::sun::star::rendering::ViewState&                                           viewState,
                                            const ::drafts::com::sun::star::rendering::RenderState&                                         renderState,
                                            const ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::rendering::Texture >&          textures,
                                            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::geometry::XMapping2D >&       xMapping,
                                            const ::drafts::com::sun::star::rendering::StrokeAttributes&                                    strokeAttributes );
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >   SAL_CALL
            queryStrokeShapes( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >&   xPolyPolygon,
                               const ::drafts::com::sun::star::rendering::ViewState&                                            viewState,
                               const ::drafts::com::sun::star::rendering::RenderState&                                          renderState,
                               const ::drafts::com::sun::star::rendering::StrokeAttributes&                                     strokeAttributes );
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            fillPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                             const ::drafts::com::sun::star::rendering::ViewState&                                          viewState,
                             const ::drafts::com::sun::star::rendering::RenderState&                                        renderState );
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            fillTexturedPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >& xPolyPolygon,
                                     const ::drafts::com::sun::star::rendering::ViewState&                                          viewState,
                                     const ::drafts::com::sun::star::rendering::RenderState&                                        renderState,
                                     const ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::rendering::Texture >&         textures );
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            fillTextureMappedPolyPolygon( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XPolyPolygon2D >&    xPolyPolygon,
                                          const ::drafts::com::sun::star::rendering::ViewState&                                             viewState,
                                          const ::drafts::com::sun::star::rendering::RenderState&                                           renderState,
                                          const ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::rendering::Texture >&            textures,
                                          const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::geometry::XMapping2D >&         xMapping );

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCanvasFont > SAL_CALL
            createFont( const ::drafts::com::sun::star::rendering::FontRequest&                                 fontRequest,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&        extraFontProperties,
                        const ::drafts::com::sun::star::geometry::Matrix2D&                                     fontMatrix );

        ::com::sun::star::uno::Sequence< ::drafts::com::sun::star::rendering::FontInfo > SAL_CALL
            queryAvailableFonts( const ::drafts::com::sun::star::rendering::FontInfo&                               aFilter,
                                 const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&   aFontProperties );

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawText( const ::drafts::com::sun::star::rendering::StringContext&                                     text,
                      const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCanvasFont >&   xFont,
                      const ::drafts::com::sun::star::rendering::ViewState&                                         viewState,
                      const ::drafts::com::sun::star::rendering::RenderState&                                       renderState,
                      sal_Int8                                                                                      textDirection );

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawTextLayout( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XTextLayout >& layoutetText,
                            const ::drafts::com::sun::star::rendering::ViewState&                                       viewState,
                            const ::drafts::com::sun::star::rendering::RenderState&                                     renderState );

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawBitmap( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmap >& xBitmap,
                        const ::drafts::com::sun::star::rendering::ViewState&                                   viewState,
                        const ::drafts::com::sun::star::rendering::RenderState&                                 renderState );
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCachedPrimitive > SAL_CALL
            drawBitmapModulated( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmap >&    xBitmap,
                                 const ::drafts::com::sun::star::rendering::ViewState&                                      viewState,
                                 const ::drafts::com::sun::star::rendering::RenderState&                                    renderState );
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XGraphicDevice > SAL_CALL
            getDevice();

        // BitmapCanvasHelper functionality
        // ================================

        void SAL_CALL copyRect( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmapCanvas >&   sourceCanvas,
                                const ::drafts::com::sun::star::geometry::RealRectangle2D&                                      sourceRect,
                                const ::drafts::com::sun::star::rendering::ViewState&                                           sourceViewState,
                                const ::drafts::com::sun::star::rendering::RenderState&                                         sourceRenderState,
                                const ::drafts::com::sun::star::geometry::RealRectangle2D&                                      destRect,
                                const ::drafts::com::sun::star::rendering::ViewState&                                           destViewState,
                                const ::drafts::com::sun::star::rendering::RenderState&                                         destRenderState );

        ::drafts::com::sun::star::geometry::IntegerSize2D SAL_CALL getSize();

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmapCanvas > SAL_CALL queryBitmapCanvas();

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmap > SAL_CALL
            getScaledBitmap( const ::drafts::com::sun::star::geometry::RealSize2D&  newSize,
                             sal_Bool                                               beFast );

        ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
            getData( const ::drafts::com::sun::star::geometry::IntegerRectangle2D& rect );

        void SAL_CALL setData( const ::com::sun::star::uno::Sequence< sal_Int8 >&               data,
                               const ::drafts::com::sun::star::geometry::IntegerRectangle2D&    rect );

        void SAL_CALL setPixel( const ::com::sun::star::uno::Sequence< sal_Int8 >&          color,
                                const ::drafts::com::sun::star::geometry::IntegerPoint2D&   pos );

        ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
            getPixel( const ::drafts::com::sun::star::geometry::IntegerPoint2D& pos );

        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XBitmapPalette > SAL_CALL getPalette();

        ::drafts::com::sun::star::rendering::IntegerBitmapLayout SAL_CALL getMemoryLayout();


        // Flush drawing queue to screen (only works for Window outdev)
        void                    flush() const;

        enum ColorType
        {
            LINE_COLOR, FILL_COLOR, TEXT_COLOR, IGNORE_COLOR
        };

        // returns transparency of color
        int setupOutDevState( const ::drafts::com::sun::star::rendering::ViewState&     viewState,
                              const ::drafts::com::sun::star::rendering::RenderState&   renderState,
                              ColorType                                                 eColorType );
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

        bool setupTextOutput( ::Point&                                                                                      o_rOutPos,
                              const ::drafts::com::sun::star::rendering::ViewState&                                         viewState,
                              const ::drafts::com::sun::star::rendering::RenderState&                                       renderState,
                              const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XCanvasFont >&   xFont );

    };
}

#endif /* _VCLCANVAS_CANVASHELPER_HXX_ */
