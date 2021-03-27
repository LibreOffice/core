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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/alpha.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/BitmapMonochromeFilter.hxx>
#include <vcl/skia/SkiaHelper.hxx>

#include <canvas/canvastools.hxx>
#include <config_features.h>

#include "impltools.hxx"
#include "spritehelper.hxx"

using namespace ::com::sun::star;


namespace vclcanvas
{
    SpriteHelper::SpriteHelper() :
        mpBackBuffer(),
        mpBackBufferMask(),
        maContent(),
        mbShowSpriteBounds(false)
    {
    }

    void SpriteHelper::init( const geometry::RealSize2D&               rSpriteSize,
                             const ::canvas::SpriteSurface::Reference& rOwningSpriteCanvas,
                             const BackBufferSharedPtr&                rBackBuffer,
                             const BackBufferSharedPtr&                rBackBufferMask,
                             bool                                      bShowSpriteBounds )
    {
        ENSURE_OR_THROW( rOwningSpriteCanvas && rBackBuffer && rBackBufferMask,
                         "SpriteHelper::init(): Invalid sprite canvas or back buffer" );

        mpBackBuffer        = rBackBuffer;
        mpBackBufferMask    = rBackBufferMask;
        mbShowSpriteBounds  = bShowSpriteBounds;

        init( rSpriteSize, rOwningSpriteCanvas );
    }

    void SpriteHelper::disposing()
    {
        mpBackBuffer.reset();
        mpBackBufferMask.reset();

        // forward to parent
        CanvasCustomSpriteHelper::disposing();
    }

    void SpriteHelper::redraw( OutputDevice&                rTargetSurface,
                               const ::basegfx::B2DPoint&   rPos,
                               bool&                        io_bSurfacesDirty,
                               bool                         bBufferedUpdate ) const
    {
        (void)bBufferedUpdate; // not used on every platform

        if( !mpBackBuffer ||
            !mpBackBufferMask )
        {
            return; // we're disposed
        }

        // log output pos in device pixel
        SAL_INFO("canvas.vcl", "SpriteHelper::redraw(): output pos is (" <<
                 rPos.getX() << "," << rPos.getY() << ")");

        const double fAlpha( getAlpha() );

        if( !isActive() || ::basegfx::fTools::equalZero( fAlpha ) )
            return;

        const ::basegfx::B2DVector& rOrigOutputSize( getSizePixel() );

        // might get changed below (e.g. adapted for
        // transformations). IMPORTANT: both position and size are
        // rounded to integer values. From now on, only those
        // rounded values are used, to keep clip and content in
        // sync.
        ::Size  aOutputSize( vcl::unotools::sizeFromB2DSize( rOrigOutputSize ) );
        ::Point aOutPos( vcl::unotools::pointFromB2DPoint( rPos ) );


        // TODO(F3): Support for alpha-VDev

        // Do we have to update our bitmaps (necessary if virdev
        // was painted to, or transformation changed)?
        const bool bNeedBitmapUpdate( io_bSurfacesDirty ||
                                      hasTransformChanged() ||
                                      maContent->IsEmpty() );

        // updating content of sprite cache - surface is no
        // longer dirty in relation to our cache
        io_bSurfacesDirty = false;
        transformUpdated();

        if( bNeedBitmapUpdate )
        {
            const Point aEmptyPoint;
            BitmapEx aBmp( mpBackBuffer->getOutDev().GetBitmapEx( aEmptyPoint,
                                                              aOutputSize ) );

            if( isContentFullyOpaque() )
            {
                // optimized case: content canvas is fully
                // opaque. Note: since we retrieved aBmp directly
                // from an OutDev, it's already a 'display bitmap'
                // on windows.
                maContent = aBmp;
            }
            else
            {
                // sprite content might contain alpha, create
                // BmpEx, then.
                BitmapEx aMask( mpBackBufferMask->getOutDev().GetBitmapEx( aEmptyPoint,
                                                                       aOutputSize ) );

                // bitmasks are much faster than alphamasks on some platforms
                // so convert to bitmask if useful
                bool convertTo1Bpp = aMask.getPixelFormat() != vcl::PixelFormat::N1_BPP;
#ifdef MACOSX
                convertTo1Bpp = false;
#endif
                if( SkiaHelper::isVCLSkiaEnabled())
                    convertTo1Bpp = false;

                if( convertTo1Bpp )
                {
                    OSL_FAIL("CanvasCustomSprite::redraw(): Mask bitmap is not "
                               "monochrome (performance!)");
                    BitmapEx aMaskEx(aMask);
                    BitmapFilter::Filter(aMaskEx, BitmapMonochromeFilter(255));
                    aMask = aMaskEx.GetBitmap();
                }

                // Note: since we retrieved aBmp and aMask
                // directly from an OutDev, it's already a
                // 'display bitmap' on windows.
                if (aMask.getPixelFormat() == vcl::PixelFormat::N1_BPP)
                    maContent = BitmapEx( aBmp.GetBitmap(), aMask.GetBitmap() );
                else
                    maContent = BitmapEx( aBmp.GetBitmap(), AlphaMask( aMask.GetBitmap()) );
            }
        }

        ::basegfx::B2DHomMatrix aTransform( getTransformation() );

        rTargetSurface.Push( PushFlags::CLIPREGION );

        // apply clip (if any)
        if( getClip().is() )
        {
            ::basegfx::B2DPolyPolygon aClipPoly(
                ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(
                    getClip() ));

            if( aClipPoly.count() )
            {
                // Move the clip to the final sprite output position.
                ::basegfx::B2DHomMatrix aClipTransform( aTransform );
                aClipTransform.translate( aOutPos.X(), aOutPos.Y() );
                aClipPoly.transform( aClipTransform );

                if( mbShowSpriteBounds )
                {
                    // Paint green sprite clip area
                    rTargetSurface.SetLineColor( Color( 0,255,0 ) );
                    rTargetSurface.SetFillColor();

                    rTargetSurface.DrawPolyPolygon(::tools::PolyPolygon(aClipPoly)); // #i76339#
                }

                vcl::Region aClipRegion( aClipPoly );
                rTargetSurface.SetClipRegion( aClipRegion );
            }
        }

        ::basegfx::B2DHomMatrix aSizeTransform, aMoveTransform;
        aSizeTransform.scale( aOutputSize.Width(), aOutputSize.Height() );
        aMoveTransform.translate( aOutPos.X(), aOutPos.Y() );
        aTransform = aMoveTransform * aTransform * aSizeTransform;

        rTargetSurface.DrawTransformedBitmapEx( aTransform, *maContent, fAlpha );

        rTargetSurface.Pop();

        if( !mbShowSpriteBounds )
            return;

        ::tools::PolyPolygon aMarkerPoly(
            ::canvas::tools::getBoundMarksPolyPolygon(
                ::basegfx::B2DRectangle(aOutPos.X(),
                                        aOutPos.Y(),
                                        aOutPos.X() + aOutputSize.Width()-1,
                                        aOutPos.Y() + aOutputSize.Height()-1) ) );

        // Paint little red sprite area markers
        rTargetSurface.SetLineColor( COL_RED );
        rTargetSurface.SetFillColor();

        for( int i=0; i<aMarkerPoly.Count(); ++i )
        {
            rTargetSurface.DrawPolyLine( aMarkerPoly.GetObject(static_cast<sal_uInt16>(i)) );
        }

        // paint sprite prio
        vcl::Font aVCLFont;
        aVCLFont.SetFontHeight( std::min(::tools::Long(20),aOutputSize.Height()) );
        aVCLFont.SetColor( COL_RED );

        rTargetSurface.SetTextAlign(ALIGN_TOP);
        rTargetSurface.SetTextColor( COL_RED );
        rTargetSurface.SetFont( aVCLFont );

        OUString text( ::rtl::math::doubleToUString( getPriority(),
                                                            rtl_math_StringFormat_F,
                                                            2,'.',nullptr,' ') );

        rTargetSurface.DrawText( aOutPos+Point(2,2), text );
        SAL_INFO( "canvas.vcl",
                  "sprite " << this << " has prio " << getPriority());
    }

    ::basegfx::B2DPolyPolygon SpriteHelper::polyPolygonFromXPolyPolygon2D( uno::Reference< rendering::XPolyPolygon2D >& xPoly ) const
    {
        return ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D( xPoly );
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
