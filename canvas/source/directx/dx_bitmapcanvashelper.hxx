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

#include <basegfx/vector/b2isize.hxx>
#include <basegfx/vector/b2dsize.hxx>

#include "dx_graphicsprovider.hxx"
#include "dx_ibitmap.hxx"
#include "dx_gdiplususer.hxx"
#include "dx_impltools.hxx"
#include "dx_canvashelper.hxx"


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

        css::uno::Reference< css::rendering::XCachedPrimitive >
            drawTextLayout( const css::rendering::XCanvas*     pCanvas,
                            const css::uno::Reference<
                                css::rendering::XTextLayout >& laidOutText,
                            const css::rendering::ViewState&   viewState,
                            const css::rendering::RenderState& renderState );

        // BitmapCanvasHelper functionality
        // ================================

        void copyRect( const css::rendering::XCanvas*          pCanvas,
                       const css::uno::Reference<
                               css::rendering::XBitmapCanvas >&    sourceCanvas,
                       const css::geometry::RealRectangle2D&   sourceRect,
                       const css::rendering::ViewState&        sourceViewState,
                       const css::rendering::RenderState&      sourceRenderState,
                       const css::geometry::RealRectangle2D&   destRect,
                       const css::rendering::ViewState&        destViewState,
                       const css::rendering::RenderState&      destRenderState );

        css::geometry::IntegerSize2D getSize();

        css::uno::Reference< css::rendering::XBitmap >
            getScaledBitmap( const css::geometry::RealSize2D&  newSize,
                             bool                                           beFast );

        css::uno::Sequence< sal_Int8 >
            getData( css::rendering::IntegerBitmapLayout&      bitmapLayout,
                     const css::geometry::IntegerRectangle2D&  rect );

        void setData( const css::uno::Sequence< sal_Int8 >&         data,
                      const css::rendering::IntegerBitmapLayout&    bitmapLayout,
                      const css::geometry::IntegerRectangle2D&      rect );

        void setPixel( const css::uno::Sequence< sal_Int8 >&        color,
                       const css::rendering::IntegerBitmapLayout&   bitmapLayout,
                       const css::geometry::IntegerPoint2D&         pos );

        css::uno::Sequence< sal_Int8 >
            getPixel( css::rendering::IntegerBitmapLayout& bitmapLayout,
                      const css::geometry::IntegerPoint2D& pos );

        css::uno::Reference< css::rendering::XBitmapPalette > getPalette();

        css::rendering::IntegerBitmapLayout getMemoryLayout();

        bool hasAlpha() const;

    protected:
        /// Render target
        IBitmapSharedPtr mpTarget;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
