/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spritehelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-17 16:08:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <rtl/math.hxx>

#include <vcl/outdev.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/alpha.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygoncutandtouch.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <canvas/canvastools.hxx>

#include "spritehelper.hxx"

using namespace ::com::sun::star;


namespace vclcanvas
{
    SpriteHelper::SpriteHelper() :
        mpBackBuffer(),
        mpBackBufferMask(),
        mpSpriteCanvas(),
        maContent(),
        mbShowSpriteBounds(false)
    {
    }

    void SpriteHelper::init( const geometry::RealSize2D&    rSpriteSize,
                             const SpriteCanvasRef&         rSpriteCanvas,
                             const BackBufferSharedPtr&     rBackBuffer,
                             const BackBufferSharedPtr&     rBackBufferMask,
                             bool                           bShowSpriteBounds )
    {
        ENSURE_AND_THROW( rSpriteCanvas.get() && rBackBuffer && rBackBufferMask,
                          "SpriteHelper::init(): Invalid sprite canvas or back buffer" );

        mpBackBuffer        = rBackBuffer;
        mpBackBufferMask    = rBackBufferMask;
        mpSpriteCanvas      = rSpriteCanvas;
        mbShowSpriteBounds  = bShowSpriteBounds;

        CanvasCustomSpriteHelper::init( rSpriteSize, rSpriteCanvas.get() );
    }

    void SpriteHelper::disposing()
    {
        mpBackBuffer.reset();
        mpBackBufferMask.reset();
        mpSpriteCanvas.clear();

        // forward to parent
        CanvasCustomSpriteHelper::disposing();
    }

    void SpriteHelper::redraw( OutputDevice&                rTargetSurface,
                               const ::basegfx::B2DPoint&   rPos,
                               bool&                        io_bSurfacesDirty,
                               bool                         bBufferedUpdate ) const
    {
        if( !mpSpriteCanvas.get() ||
            !mpBackBuffer ||
            !mpBackBufferMask )
        {
            return; // we're disposed
        }

        // log output pos in device pixel
        VERBOSE_TRACE( "SpriteHelper::redraw(): output pos is (%f, %f)",
                       rPos.getX(),
                       rPos.getY() );

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
            ::Size  aOutputSize( ::vcl::unotools::sizeFromB2DSize( rOrigOutputSize ) );
            ::Point aOutPos( ::vcl::unotools::pointFromB2DPoint( rPos ) );


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
                Bitmap aBmp( mpBackBuffer->getOutDev().GetBitmap( aEmptyPoint,
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
                    Bitmap aMask( mpBackBufferMask->getOutDev().GetBitmap( aEmptyPoint,
                                                                           aOutputSize ) );

                    if( aMask.GetBitCount() != 1 )
                    {
                        OSL_ENSURE(false,
                                   "CanvasCustomSprite::redraw(): Mask bitmap is not "
                                   "monochrome (performance!)");
                        aMask.MakeMono(255);
                    }

                    // Note: since we retrieved aBmp and aMask
                    // directly from an OutDev, it's already a
                    // 'display bitmap' on windows.
                    maContent = BitmapEx( aBmp, aMask );
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
                if( !::basegfx::fTools::equalZero( aTransform.get(0,1) ) ||
                    !::basegfx::fTools::equalZero( aTransform.get(1,0) ) )
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

                    aOutPos.X() = ::basegfx::fround( aDestRect.getMinX() );
                    aOutPos.Y() = ::basegfx::fround( aDestRect.getMinY() );

                    // TODO(P3): Use optimized bitmap transformation here.

                    // actually re-create the bitmap ONLY if necessary
                    if( bNeedBitmapUpdate )
                        maContent = tools::transformBitmap( *maContent,
                                                            aTransform,
                                                            uno::Sequence<double>(),
                                                            tools::MODULATE_NONE );

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

                    aOutPos.X() = ::basegfx::fround( aTransform.get(0,2) );
                    aOutPos.Y() = ::basegfx::fround( aTransform.get(1,2) );
                }
            }

            // transformBitmap() might return empty bitmaps, for tiny
            // scales.
            if( !!(*maContent) )
            {
                // when true, fast path for slide transition has
                // already redrawn the sprite.
                bool bSpriteRedrawn( false );

                rTargetSurface.Push( PUSH_CLIPREGION );

                // apply clip (if any)
                if( getClip().is() )
                {
                    ::basegfx::B2DPolyPolygon aClipPoly(
                        ::canvas::tools::polyPolygonFromXPolyPolygon2D(
                            getClip() ));

                    if( aClipPoly.count() )
                    {
                        // aTransform already contains the
                        // translational component, moving the clip to
                        // the final sprite output position.
                        aClipPoly.transform( aTransform );

#ifndef WNT
                        // non-Windows only - bAtLeastOnePolygon is
                        // only used in non-WNT code below

                        // check whether maybe the clip consists
                        // solely out of rectangular polygons. If this
                        // is the case, enforce using the triangle
                        // clip region setup - non-optimized X11
                        // drivers tend to perform abyssmally on
                        // XPolygonRegion, which is used internally,
                        // when filling complex polypolygons.
                        bool bAtLeastOnePolygon( false );
                        const sal_Int32 nPolygons( aClipPoly.count() );

                        for( sal_Int32 i=0; i<nPolygons; ++i )
                        {
                            if( !::basegfx::tools::isRectangle(
                                    aClipPoly.getB2DPolygon(i)) )
                            {
                                bAtLeastOnePolygon = true;
                                break;
                            }
                        }
#endif

                        if( mbShowSpriteBounds )
                        {
                            // Paint green sprite clip area
                            rTargetSurface.SetLineColor( Color( 0,255,0 ) );
                            rTargetSurface.SetFillColor();

                            rTargetSurface.DrawPolyPolygon( aClipPoly );
                        }

#ifndef WNT
                        // as a matter of fact, this fast path only
                        // performs well for X11 - under Windows, the
                        // clip via SetTriangleClipRegion is faster.
                        if( bAtLeastOnePolygon &&
                            bBufferedUpdate &&
                            ::rtl::math::approxEqual(fAlpha, 1.0) &&
                            !maContent->IsTransparent() )
                        {
                            // fast path for slide transitions
                            // (buffered, no alpha, no mask (because
                            // full slide is contained in the sprite))

                            // XOR bitmap onto backbuffer, clear area
                            // that should be _visible_ with black,
                            // XOR bitmap again on top of that -
                            // result: XOR cancels out where no black
                            // has been rendered, and yields the
                            // original bitmap, where black is
                            // underneath.
                            rTargetSurface.Push( PUSH_RASTEROP );
                            rTargetSurface.SetRasterOp( ROP_XOR );
                            rTargetSurface.DrawBitmap( aOutPos,
                                                       aOutputSize,
                                                       maContent->GetBitmap() );

                            rTargetSurface.SetLineColor();
                            rTargetSurface.SetFillColor( COL_BLACK );
                            rTargetSurface.SetRasterOp( ROP_0 );
                            rTargetSurface.DrawPolyPolygon( aClipPoly );

                            rTargetSurface.SetRasterOp( ROP_XOR );
                            rTargetSurface.DrawBitmap( aOutPos,
                                                       aOutputSize,
                                                       maContent->GetBitmap() );

                            rTargetSurface.Pop();

                            bSpriteRedrawn = true;
                        }
                        else
#endif
                        {
                            // redraw is direcly on the front buffer,
                            // or using alpha blending - cannot use
                            // XOR, thus, employing the still somewhat
                            // speedier triangle clip method
                            const ::basegfx::B2DPolyPolygon aPreparedClip(::basegfx::tools::addPointsAtCutsAndTouches(aClipPoly));
                            ::basegfx::B2DPolygon aTriangulatedClip(::basegfx::triangulator::triangulate(aPreparedClip));

                            // restrict the clipping area to the visible portion of the output device.
                            Size aSize(rTargetSurface.GetOutputSizePixel());
                            ::basegfx::B2DRange aOutputRect(::basegfx::B2DPoint(0,0),::basegfx::B2DPoint(aSize.Width(),aSize.Height()));
                            ::basegfx::B2DPolygon aClippedClip(::basegfx::tools::clipTriangleListOnRange(aTriangulatedClip,aOutputRect));

                            PolyPolygon aPolyPoly( aClippedClip );
                            rTargetSurface.SetTriangleClipRegion(aPolyPoly);
                        }
                    }
                }

                if( !bSpriteRedrawn )
                {
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
                        BYTE nColor( static_cast<UINT8>( ::basegfx::fround( 255.0*(1.0 - fAlpha) + .5) ) );
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
                }

                rTargetSurface.Pop();

                if( mbShowSpriteBounds )
                {
                    ::PolyPolygon aMarkerPoly(
                        ::canvas::tools::getBoundMarkPolyPolygon(
                            ::basegfx::B2DRectangle(aOutPos.X(),
                                                    aOutPos.Y(),
                                                    aOutPos.X() + aOutputSize.Width()-1,
                                                    aOutPos.Y() + aOutputSize.Height()-1) ) );

                    // Paint little red sprite area markers
                    rTargetSurface.SetLineColor( Color( 255,0,0 ) );
                    rTargetSurface.SetFillColor();

                    for( int i=0; i<aMarkerPoly.Count(); ++i )
                    {
                        rTargetSurface.DrawPolyLine( aMarkerPoly.GetObject(i) );
                    }
                }
            }
        }
    }

    void SpriteHelper::clearSurface()
    {
        ENSURE_AND_THROW( mpBackBuffer && mpBackBufferMask,
                          "SpriteHelper::clearSurface(): disposed" );

        const ::Point       aEmptyPoint;
        const ::Rectangle   aSpriteRect( aEmptyPoint,
                                 ::vcl::unotools::sizeFromB2DSize( getSizePixel() ) );
        const ::Color       aWhiteColor( COL_WHITE );

        OutputDevice& rOutDev( mpBackBuffer->getOutDev() );
        rOutDev.EnableMapMode( FALSE );
        rOutDev.SetFillColor( aWhiteColor );
        rOutDev.SetLineColor();
        rOutDev.DrawRect( aSpriteRect );

        OutputDevice& rMaskOutDev( mpBackBufferMask->getOutDev() );
        rMaskOutDev.SetDrawMode( DRAWMODE_DEFAULT );
        rMaskOutDev.EnableMapMode( FALSE );
        rMaskOutDev.SetFillColor( aWhiteColor );
        rMaskOutDev.SetLineColor();
        rMaskOutDev.DrawRect( aSpriteRect );
        rMaskOutDev.SetDrawMode( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_BLACKTEXT |
                                 DRAWMODE_BLACKGRADIENT | DRAWMODE_BLACKBITMAP );
    }

    ::basegfx::B2DPolyPolygon SpriteHelper::polyPolygonFromXPolyPolygon2D( uno::Reference< rendering::XPolyPolygon2D >& xPoly ) const
    {
        return ::canvas::tools::polyPolygonFromXPolyPolygon2D( xPoly );
    }

}
