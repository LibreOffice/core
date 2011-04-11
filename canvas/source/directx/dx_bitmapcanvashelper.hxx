/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _DXCANVAS_BITMAPCANVASHELPER_HXX_
#define _DXCANVAS_BITMAPCANVASHELPER_HXX_

#include <com/sun/star/rendering/XCanvas.hpp>

#include <basegfx/vector/b2isize.hxx>
#include <basegfx/vector/b2dsize.hxx>

#include "dx_graphicsprovider.hxx"
#include "dx_ibitmap.hxx"
#include "dx_gdiplususer.hxx"
#include "dx_impltools.hxx"
#include "dx_canvashelper.hxx"

#include <boost/utility.hpp>


namespace dxcanvas
{
    /** Helper class for basic canvas functionality. Also offers
        optional backbuffer painting, when providing it with a second
        HDC to render into.
     */
    class BitmapCanvasHelper : public CanvasHelper
    {
    public:
        BitmapCanvasHelper();

        /// Release all references
        void disposing();

        /** Set the target for rendering operations

            @param rTarget
            Render target
         */
        void setTarget( const IBitmapSharedPtr& rTarget );

        /** Set the target for rendering operations

            @param rTarget
            Render target

            @param rOutputOffset
            Output offset in pixel
         */
        void setTarget( const IBitmapSharedPtr&   rTarget,
                        const ::basegfx::B2ISize& rOutputOffset );


        // CanvasHelper functionality is implementation-inherited. yuck.
        // =============================================================
        void clear();

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCachedPrimitive >
            drawTextLayout( const ::com::sun::star::rendering::XCanvas*     pCanvas,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::rendering::XTextLayout >& layoutetText,
                            const ::com::sun::star::rendering::ViewState&   viewState,
                            const ::com::sun::star::rendering::RenderState& renderState );

        // BitmapCanvasHelper functionality
        // ================================

        void copyRect( const ::com::sun::star::rendering::XCanvas*          pCanvas,
                       const ::com::sun::star::uno::Reference<
                               ::com::sun::star::rendering::XBitmapCanvas >&    sourceCanvas,
                       const ::com::sun::star::geometry::RealRectangle2D&   sourceRect,
                       const ::com::sun::star::rendering::ViewState&        sourceViewState,
                       const ::com::sun::star::rendering::RenderState&      sourceRenderState,
                       const ::com::sun::star::geometry::RealRectangle2D&   destRect,
                       const ::com::sun::star::rendering::ViewState&        destViewState,
                       const ::com::sun::star::rendering::RenderState&      destRenderState );

        ::com::sun::star::geometry::IntegerSize2D getSize();

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >
            getScaledBitmap( const ::com::sun::star::geometry::RealSize2D&  newSize,
                             sal_Bool                                       beFast );

        ::com::sun::star::uno::Sequence< sal_Int8 >
            getData( ::com::sun::star::rendering::IntegerBitmapLayout&      bitmapLayout,
                     const ::com::sun::star::geometry::IntegerRectangle2D&  rect );

        void setData( const ::com::sun::star::uno::Sequence< sal_Int8 >&         data,
                      const ::com::sun::star::rendering::IntegerBitmapLayout&    bitmapLayout,
                      const ::com::sun::star::geometry::IntegerRectangle2D&      rect );

        void setPixel( const ::com::sun::star::uno::Sequence< sal_Int8 >&        color,
                       const ::com::sun::star::rendering::IntegerBitmapLayout&   bitmapLayout,
                       const ::com::sun::star::geometry::IntegerPoint2D&         pos );

        ::com::sun::star::uno::Sequence< sal_Int8 >
            getPixel( ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
                      const ::com::sun::star::geometry::IntegerPoint2D& pos );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapPalette > getPalette();

        ::com::sun::star::rendering::IntegerBitmapLayout getMemoryLayout();

        bool hasAlpha() const;

    protected:
        /// Render target
        IBitmapSharedPtr mpTarget;
    };
}

#endif /* _DXCANVAS_BITMAPCANVASHELPER_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
