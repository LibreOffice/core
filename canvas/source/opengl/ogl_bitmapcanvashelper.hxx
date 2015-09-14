/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CANVAS_SOURCE_OPENGL_OGL_BITMAPCANVASHELPER_HXX
#define INCLUDED_CANVAS_SOURCE_OPENGL_OGL_BITMAPCANVASHELPER_HXX

#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <basegfx/vector/b2isize.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include "ogl_canvashelper.hxx"

namespace oglcanvas
{
    /** Helper class for basic canvas functionality. */
    class BitmapCanvasHelper : public CanvasHelper
    {
    public:
        BitmapCanvasHelper();

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
        void init( css::rendering::XGraphicDevice&     rDevice,
                   SpriteDeviceHelper&                              rDeviceHelper,
                   const css::geometry::IntegerSize2D& rSize );

        // BitmapCanvasHelper functionality
        // ================================

        css::geometry::IntegerSize2D getSize() { return maSize; }

        css::uno::Reference< css::rendering::XBitmapCanvas > queryBitmapCanvas();

        css::uno::Reference< css::rendering::XBitmap >
            getScaledBitmap( const css::geometry::RealSize2D&  newSize,
                             bool                                               beFast );

        css::uno::Sequence< sal_Int8 >
            getData( css::rendering::IntegerBitmapLayout&      bitmapLayout,
                     const css::geometry::IntegerRectangle2D&  rect );

        css::uno::Sequence< sal_Int8 >
            getPixel( css::rendering::IntegerBitmapLayout& bitmapLayout,
                      const css::geometry::IntegerPoint2D& pos );

        css::rendering::IntegerBitmapLayout getMemoryLayout();

    private:
        css::geometry::IntegerSize2D maSize;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
