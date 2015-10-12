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

#ifndef INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLBITMAP_HXX
#define INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLBITMAP_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>

#include <cppcanvas/bitmap.hxx>
#include <canvasgraphichelper.hxx>


/*Definition of ImplBitmap */

namespace cppcanvas
{

    namespace internal
    {
        // share partial CanvasGraphic implementation from CanvasGraphicHelper
        class ImplBitmap : public virtual ::cppcanvas::Bitmap, protected CanvasGraphicHelper
        {
        public:

            ImplBitmap( const CanvasSharedPtr& rParentCanvas,
                        const css::uno::Reference< css::rendering::XBitmap >& rBitmap );

            virtual ~ImplBitmap();

            // CanvasGraphic implementation (that was not already implemented by CanvasGraphicHelper)
            virtual bool draw() const SAL_OVERRIDE;
            virtual bool drawAlphaModulated( double nAlphaModulation ) const SAL_OVERRIDE;

            virtual BitmapCanvasSharedPtr                       getBitmapCanvas() const SAL_OVERRIDE;

            // Bitmap implementation
            virtual css::uno::Reference< css::rendering::XBitmap >  getUNOBitmap() const SAL_OVERRIDE;

        private:
            ImplBitmap(const ImplBitmap&) = delete;
            ImplBitmap& operator=( const ImplBitmap& ) = delete;

            const css::uno::Reference< css::rendering::XBitmap > mxBitmap;
            BitmapCanvasSharedPtr                                mpBitmapCanvas;
         };
    }
}

#endif // INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLBITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
