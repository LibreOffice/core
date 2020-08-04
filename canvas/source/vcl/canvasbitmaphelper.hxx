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

#include "canvashelper.hxx"

#include <vcl/bitmapex.hxx>

#include "bitmapbackbuffer.hxx"


namespace vclcanvas
{
    /** Helper class for basic canvasbitmap functionality. Extends
        CanvasHelper with some CanvasBitmap specialities, such as alpha
        support.

        Note that a plain CanvasHelper, although it does support the
        XBitmap interface, has no provision for alpha channel on VCL
        (at least no efficient one. If the alpha VDev one day gets
        part of SAL, we might change that).
     */
    class CanvasBitmapHelper : public CanvasHelper
    {
    public:
        CanvasBitmapHelper();

        /** Set a new bitmap on this helper.

            This method late-initializes the bitmap canvas helper,
            providing it with the necessary device and output
            objects. The internally stored bitmap representation is
            updated from the given bitmap, including any size
            changes. Note that the CanvasHelper does <em>not</em> take
            ownership of the SpriteCanvas object, nor does it perform
            any reference counting. Thus, to prevent reference counted
            objects from deletion, the user of this class is
            responsible for holding ref-counted references to those
            objects!

            @param rBitmap
            Content of this bitmap is used as our new content (our
            internal size is adapted to the size of the bitmap given)

            @param rDevice
            Reference device for this canvas bitmap

            @param rOutDevProvider
            Reference output device. Used to create matching bitmap.
         */
        void init( const BitmapEx&                              rBitmap,
                   css::rendering::XGraphicDevice&              rDevice,
                   const OutDevProviderSharedPtr&               rOutDevProvider );


        // Overridden CanvasHelper functionality
        // =====================================

        void disposing();

        void clear();

        css::geometry::IntegerSize2D getSize() const;

        css::uno::Reference< css::rendering::XBitmap >
            getScaledBitmap( const css::geometry::RealSize2D&  newSize,
                             bool                                           beFast );

        css::uno::Sequence< sal_Int8 >
            getData( css::rendering::IntegerBitmapLayout&      bitmapLayout,
                     const css::geometry::IntegerRectangle2D&  rect );

        css::uno::Sequence< sal_Int8 >
            getPixel( css::rendering::IntegerBitmapLayout& bitmapLayout,
                      const css::geometry::IntegerPoint2D& pos );

        css::rendering::IntegerBitmapLayout getMemoryLayout() const;

        /// @internal
        BitmapEx getBitmap() const;

    private:

        BitmapBackBufferSharedPtr   mpBackBuffer;
        OutDevProviderSharedPtr     mpOutDevReference;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
