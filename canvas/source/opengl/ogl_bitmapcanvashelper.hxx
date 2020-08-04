/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/geometry/IntegerPoint2D.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>

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

        const css::geometry::IntegerSize2D& getSize() const { return maSize; }

        css::uno::Reference< css::rendering::XBitmap >
            getScaledBitmap( const css::geometry::RealSize2D&  newSize,
                             bool                                               beFast );

        css::uno::Sequence< sal_Int8 >
            getData( css::rendering::IntegerBitmapLayout&      bitmapLayout,
                     const css::geometry::IntegerRectangle2D&  rect );

        css::uno::Sequence< sal_Int8 >
            getPixel( css::rendering::IntegerBitmapLayout& bitmapLayout,
                      const css::geometry::IntegerPoint2D& pos );

        css::rendering::IntegerBitmapLayout getMemoryLayout() const;

    private:
        css::geometry::IntegerSize2D maSize;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
