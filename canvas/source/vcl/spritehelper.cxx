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
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/alpha.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/BitmapMonochromeFilter.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include <canvas/canvastools.hxx>

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
        ENSURE_OR_THROW( rOwningSpriteCanvas.get() && rBackBuffer && rBackBufferMask,
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

        if( isActive() &&
            !::basegfx::fTools::equalZero( fAlpha ) )
        {
            const Point                 aEmptyPoint;
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
                BitmapEx aBmp( mpBackBuffer->getOutDev().GetBitmapEx( aEmptyPoint,
                                                                  aOutputSize ) );

                if( isContentFullyOpaque() )
                {
                    // optimized case: content canvas is fully
                    // opaque. Note: since we retrieved aBmp directly
                    // from an OutDev, it's already a 'display bitmap'
                    // on windows.
                    maContent = BitmapEx( aBmp );
                }
                else
                {
                    // sprite content might contain alpha, create
                    // BmpEx, then.
                    BitmapEx aMask( mpBackBufferMask->getOutDev().GetBitmapEx( aEmptyPoint,
                                                                           aOutputSize ) );

                    // bitmasks are much faster than alphamasks on some platforms
                    // so convert to bitmask if useful
#ifndef MACOSX
                    if( aMask.GetBitCount() != 1 )
                    {
                        OSL_FAIL("CanvasCustomSprite::redraw(): Mask bitmap is not "
                                   "monochrome (performance!)");
                        BitmapEx aMaskEx(aMask);
                        BitmapFilter::Filter(aMaskEx, BitmapMonochromeFilter(255));
                        aMask = aMaskEx.GetBitmap();
                    }
#endif

                    // Note: since we retrieved aBmp and aMask
                    // directly from an OutDev, it's already a
                    // 'display bitmap' on windows.
                    maContent = BitmapEx( aBmp.GetBitmap(), aMask.GetBitmap() );
                }
            }

            ::basegfx::B2DHomMatrix aTransform( getTransformation() );

            // check whether matrix is "easy" to handle - pure
            // translations or scales are handled by OutputDevice
            // alone
            const bool bIdentityTransform( aTransform.isIdentity() );

            // make transformation absolute (put sprite to final
            // output position). Need to happen here, as we also have
            // to translate the clip polygon
            aTransform.translate( aOutPos.X(),
                                  aOutPos.Y() );

            if( !bIdentityTransform )
            {
                // Avoid the trick with the negative width in the OpenGL case,
                // OutputDevice::DrawDeviceAlphaBitmap() doesn't like it.
                if( !::basegfx::fTools::equalZero( aTransform.get(0,1) ) ||
                    !::basegfx::fTools::equalZero( aTransform.get(1,0) ) ||
                    OpenGLHelper::isVCLOpenGLEnabled())
                {
                    // "complex" transformation, employ affine
                    // transformator

                    // modify output position, to account for the fact
                    // that transformBitmap() always normalizes its output
                    // bitmap into the smallest enclosing box.
                    ::basegfx::B2DRectangle aDestRect;
                    ::canvas::tools::calcTransformedRectBounds( aDestRect,
                                                                ::basegfx::B2DRectangle(0,
                                                                                        0,
                                                                                        rOrigOutputSize.getX(),
                                                                                        rOrigOutputSize.getY()),
                                                                aTransform );

                    aOutPos.setX( ::basegfx::fround( aDestRect.getMinX() ) );
                    aOutPos.setY( ::basegfx::fround( aDestRect.getMinY() ) );

                    // TODO(P3): Use optimized bitmap transformation here.

                    // actually re-create the bitmap ONLY if necessary
                    if( bNeedBitmapUpdate )
                        maContent = tools::transformBitmap( *maContent,
                                                            aTransform );

                    aOutputSize = maContent->GetSizePixel();
                }
                else
                {
                    // relatively 'simplistic' transformation -
                    // retrieve scale and translational offset
                    aOutputSize.setWidth (
                        ::basegfx::fround( rOrigOutputSize.getX() * aTransform.get(0,0) ) );
                    aOutputSize.setHeight(
                        ::basegfx::fround( rOrigOutputSize.getY() * aTransform.get(1,1) ) );

                    aOutPos.setX( ::basegfx::fround( aTransform.get(0,2) ) );
                    aOutPos.setY( ::basegfx::fround( aTransform.get(1,2) ) );
                }
            }

            // transformBitmap() might return empty bitmaps, for tiny
            // scales.
            if( !!(*maContent) )
            {
                rTargetSurface.Push( PushFlags::CLIPREGION );

                // apply clip (if any)
                if( getClip().is() )
                {
                    ::basegfx::B2DPolyPolygon aClipPoly(
                        ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(
                            getClip() ));

                    if( aClipPoly.count() )
                    {
                        // aTransform already contains the
                        // translational component, moving the clip to
                        // the final sprite output position.
                        aClipPoly.transform( aTransform );

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

                if( ::rtl::math::approxEqual(fAlpha, 1.0) )
                {
                    // no alpha modulation -> just copy to output
                    if( maContent->IsTransparent() )
                        rTargetSurface.DrawBitmapEx( aOutPos, aOutputSize, *maContent );
                    else
                        rTargetSurface.DrawBitmap( aOutPos, aOutputSize, maContent->GetBitmap() );
                }
                else
                {
                    // TODO(P3): Switch to OutputDevice::DrawTransparent()
                    // here

                    // draw semi-transparent
                    sal_uInt8 nColor( static_cast<sal_uInt8>( ::basegfx::fround( 255.0*(1.0 - fAlpha) + .5) ) );
                    AlphaMask aAlpha( maContent->GetSizePixel(),
                                      &nColor );

                    // mask out fully transparent areas
                    if( maContent->IsTransparent() )
                        aAlpha.Replace( maContent->GetMask(), 255 );

                    // alpha-blend to output
                    rTargetSurface.DrawBitmapEx( aOutPos, aOutputSize,
                                                 BitmapEx( maContent->GetBitmap(),
                                                           aAlpha ) );
                }

                rTargetSurface.Pop();

                if( mbShowSpriteBounds )
                {
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
                    aVCLFont.SetFontHeight( std::min(long(20),aOutputSize.Height()) );
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
            }
        }
    }

    ::basegfx::B2DPolyPolygon SpriteHelper::polyPolygonFromXPolyPolygon2D( uno::Reference< rendering::XPolyPolygon2D >& xPoly ) const
    {
        return ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D( xPoly );
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
