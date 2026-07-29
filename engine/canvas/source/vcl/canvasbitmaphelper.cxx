/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>
#include <sal/log.hxx>

#include <canvastools.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/canvastools.hxx>

#include "canvasbitmap.hxx"
#include "canvasbitmaphelper.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    CanvasBitmapHelper::CanvasBitmapHelper()
    {
    }

    void CanvasBitmapHelper::init( const ::Bitmap&                rBitmap,
                                   vclcanvas::XGraphicDevice&     rDevice,
                                   const OutDevProviderSharedPtr& rOutDevReference )
    {
        mpOutDevReference = rOutDevReference;
        mpBackBuffer = std::make_shared<BitmapBackBuffer>( rBitmap, rOutDevReference->getOutDev() );

        // forward new settings to base class (ref device, output
        // surface, no protection (own backbuffer), alpha depends on
        // whether BmpEx is transparent or not)
        CanvasHelper::init( rDevice,
                            mpBackBuffer,
                            false,
                            rBitmap.HasAlpha() );
    }

    void CanvasBitmapHelper::disposing()
    {
        mpBackBuffer.reset();
        mpOutDevReference.reset();

        // forward to base class
        CanvasHelper::disposing();
    }

    geometry::IntegerSize2D CanvasBitmapHelper::getSize() const
    {
        if( !mpBackBuffer )
            return geometry::IntegerSize2D();

        return vcl::unotools::integerSize2DFromSize( mpBackBuffer->getBitmapSizePixel() );
    }

    void CanvasBitmapHelper::clear()
    {
        // are we disposed?
        if( mpBackBuffer )
            mpBackBuffer->clear(); // alpha vdev needs special treatment
    }

    ::Bitmap CanvasBitmapHelper::getBitmap() const
    {
        if( !mpBackBuffer )
            return ::Bitmap(); // we're disposed
        else
            return mpBackBuffer->getBitmapReference();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
