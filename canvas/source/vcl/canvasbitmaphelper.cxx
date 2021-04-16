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

#include <sal/config.h>
#include <sal/log.hxx>

#include <canvas/canvastools.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/bitmapex.hxx>
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

    void CanvasBitmapHelper::init( const BitmapEx&                rBitmap,
                                   rendering::XGraphicDevice&     rDevice,
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
                            rBitmap.IsAlpha() );
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

    uno::Reference< rendering::XBitmap > CanvasBitmapHelper::getScaledBitmap( const geometry::RealSize2D&   newSize,
                                                                              bool                          beFast )
    {
        ENSURE_OR_THROW( mpDevice,
                          "disposed CanvasHelper" );

        SAL_INFO( "canvas.vcl", "::vclcanvas::CanvasBitmapHelper::getScaledBitmap()" );

        if( !mpBackBuffer || mpDevice )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        BitmapEx aRes( mpBackBuffer->getBitmapReference() );

        aRes.Scale( vcl::unotools::sizeFromRealSize2D(newSize),
                     beFast ? BmpScaleFlag::Default : BmpScaleFlag::BestQuality );

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( aRes, *mpDevice, mpOutDevReference ) );
    }

    uno::Sequence< sal_Int8 > CanvasBitmapHelper::getData( rendering::IntegerBitmapLayout&      rLayout,
                                                           const geometry::IntegerRectangle2D&  rect )
    {
        SAL_INFO( "canvas.vcl", "::vclcanvas::CanvasBitmapHelper::getData()" );

        if( !mpBackBuffer )
            return uno::Sequence< sal_Int8 >(); // we're disposed

        rLayout = getMemoryLayout();

        // TODO(F1): Support more formats.
        const Size aBmpSize( mpBackBuffer->getBitmapReference().GetSizePixel() );

        rLayout.ScanLines = aBmpSize.Height();
        rLayout.ScanLineBytes = aBmpSize.Width()*4;
        rLayout.ScanLineStride = rLayout.ScanLineBytes;

        uno::Sequence< sal_Int8 > aRes = vcl::bitmap::CanvasExtractBitmapData(mpBackBuffer->getBitmapReference(), rect);
        return aRes;
    }

    uno::Sequence< sal_Int8 > CanvasBitmapHelper::getPixel( rendering::IntegerBitmapLayout& rLayout,
                                                            const geometry::IntegerPoint2D& pos )
    {
        SAL_INFO( "canvas.vcl", "::vclcanvas::CanvasBitmapHelper::getPixel()" );

        if( !mpBackBuffer )
            return uno::Sequence< sal_Int8 >(); // we're disposed

        rLayout = getMemoryLayout();
        rLayout.ScanLines = 1;
        rLayout.ScanLineBytes = 4;
        rLayout.ScanLineStride = rLayout.ScanLineBytes;

        const Size aBmpSize( mpBackBuffer->getBitmapReference().GetSizePixel() );

        ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < aBmpSize.Width(),
                         "X coordinate out of bounds" );
        ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < aBmpSize.Height(),
                         "Y coordinate out of bounds" );

        ::Color aColor = mpBackBuffer->getBitmapReference().GetPixelColor(pos.X, pos.Y);

        uno::Sequence< sal_Int8 > aRes( 4 );
        sal_Int8* pRes = aRes.getArray();
        pRes[ 0 ] = aColor.GetRed();
        pRes[ 1 ] = aColor.GetGreen();
        pRes[ 2 ] = aColor.GetBlue();
        pRes[ 3 ] = aColor.GetAlpha();

        return aRes;
    }

    rendering::IntegerBitmapLayout CanvasBitmapHelper::getMemoryLayout() const
    {
        if( !mpOutDevProvider )
            return rendering::IntegerBitmapLayout(); // we're disposed

        rendering::IntegerBitmapLayout aBitmapLayout( ::canvas::tools::getStdMemoryLayout(getSize()) );
        if ( !hasAlpha() )
            aBitmapLayout.ColorSpace = canvas::tools::getStdColorSpaceWithoutAlpha();

        return aBitmapLayout;
    }

    BitmapEx CanvasBitmapHelper::getBitmap() const
    {
        if( !mpBackBuffer )
            return BitmapEx(); // we're disposed
        else
            return mpBackBuffer->getBitmapReference();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
