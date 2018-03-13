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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <canvas/canvastools.hxx>
#include <com/sun/star/util/Endianness.hpp>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>
#include <tools/poly.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/window.hxx>

#include "canvasbitmap.hxx"
#include "canvasbitmaphelper.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    CanvasBitmapHelper::CanvasBitmapHelper() :
        mpBackBuffer(),
        mpOutDevReference()
    {
    }

    void CanvasBitmapHelper::init( const BitmapEx&                rBitmap,
                                   rendering::XGraphicDevice&     rDevice,
                                   const OutDevProviderSharedPtr& rOutDevReference )
    {
        mpOutDevReference = rOutDevReference;
        mpBackBuffer.reset( new BitmapBackBuffer( rBitmap, rOutDevReference->getOutDev() ));

        // forward new settings to base class (ref device, output
        // surface, no protection (own backbuffer), alpha depends on
        // whether BmpEx is transparent or not)
        CanvasHelper::init( rDevice,
                            mpBackBuffer,
                            false,
                            rBitmap.IsTransparent() );
    }

    void CanvasBitmapHelper::disposing()
    {
        mpBackBuffer.reset();
        mpOutDevReference.reset();

        // forward to base class
        CanvasHelper::disposing();
    }

    geometry::IntegerSize2D CanvasBitmapHelper::getSize()
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
        Bitmap aBitmap( mpBackBuffer->getBitmapReference().GetBitmap() );
        Bitmap aAlpha( mpBackBuffer->getBitmapReference().GetAlpha().GetBitmap() );

        Bitmap::ScopedReadAccess pReadAccess( aBitmap );
        Bitmap::ScopedReadAccess pAlphaReadAccess( aAlpha.IsEmpty() ?
                                                 nullptr : aAlpha.AcquireReadAccess(),
                                                 aAlpha );

        ENSURE_OR_THROW( pReadAccess.get() != nullptr,
                         "Could not acquire read access to bitmap" );

        // TODO(F1): Support more formats.
        const Size aBmpSize( aBitmap.GetSizePixel() );

        rLayout.ScanLines = aBmpSize.Height();
        rLayout.ScanLineBytes = aBmpSize.Width()*4;
        rLayout.ScanLineStride = rLayout.ScanLineBytes;

        // for the time being, always return as BGRA
        uno::Sequence< sal_Int8 > aRes( 4*aBmpSize.Width()*aBmpSize.Height() );
        sal_Int8* pRes = aRes.getArray();

        int nCurrPos(0);
        for( long y=rect.Y1;
             y<aBmpSize.Height() && y<rect.Y2;
             ++y )
        {
            Scanline pScanlineReadAlpha = pAlphaReadAccess->GetScanline( y );
            if( pAlphaReadAccess.get() != nullptr )
            {
                for( long x=rect.X1;
                     x<aBmpSize.Width() && x<rect.X2;
                     ++x )
                {
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetRed();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetGreen();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetBlue();
                    pRes[ nCurrPos++ ] = pAlphaReadAccess->GetIndexFromData( pScanlineReadAlpha, x );
                }
            }
            else
            {
                for( long x=rect.X1;
                     x<aBmpSize.Width() && x<rect.X2;
                     ++x )
                {
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetRed();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetGreen();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetBlue();
                    pRes[ nCurrPos++ ] = sal_uInt8(255);
                }
            }
        }

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

        Bitmap aBitmap( mpBackBuffer->getBitmapReference().GetBitmap() );
        Bitmap aAlpha( mpBackBuffer->getBitmapReference().GetAlpha().GetBitmap() );

        Bitmap::ScopedReadAccess pReadAccess( aBitmap );
        Bitmap::ScopedReadAccess pAlphaReadAccess( aAlpha.IsEmpty() ?
                                                 nullptr : aAlpha.AcquireReadAccess(),
                                                 aAlpha );
        ENSURE_OR_THROW( pReadAccess.get() != nullptr,
                         "Could not acquire read access to bitmap" );

        uno::Sequence< sal_Int8 > aRes( 4 );
        sal_Int8* pRes = aRes.getArray();

        const BitmapColor aColor( pReadAccess->GetColor( pos.Y, pos.X ) );
        pRes[ 0 ] = aColor.GetRed();
        pRes[ 1 ] = aColor.GetGreen();
        pRes[ 2 ] = aColor.GetBlue();

        if( pAlphaReadAccess.get() != nullptr )
            pRes[ 3 ] = pAlphaReadAccess->GetPixel( pos.Y, pos.X ).GetIndex();
        else
            pRes[ 3 ] = sal_uInt8(255);

        return aRes;
    }

    rendering::IntegerBitmapLayout CanvasBitmapHelper::getMemoryLayout()
    {
        if( !mpOutDev.get() )
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
