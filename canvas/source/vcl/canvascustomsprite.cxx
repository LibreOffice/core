/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvascustomsprite.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:18:26 $
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

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _SV_ALPHA_HXX
#include <vcl/alpha.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#include <canvas/canvastools.hxx>

#include "canvascustomsprite.hxx"


using namespace ::com::sun::star;


namespace vclcanvas
{

    CanvasCustomSprite::CanvasCustomSprite( const geometry::RealSize2D&         rSpriteSize,
                                            const WindowGraphicDevice::ImplRef& rDevice,
                                            const SpriteCanvas::ImplRef&        rSpriteCanvas ) :
        mpBackBuffer(),
        mpBackBufferMask(),
        mpSpriteCanvas( rSpriteCanvas ),
        maContent(),
        maCurrClipBounds(),
        maPosition(0.0, 0.0),
        maSize( static_cast< sal_Int32 >(
                    ::std::max( 1.0,
                                ceil( rSpriteSize.Width ) ) ), // round up to nearest int,
                                                               // enforce sprite to have at
                                                               // least (1,1) pixel size
                static_cast< sal_Int32 >(
                    ::std::max( 1.0,
                                ceil( rSpriteSize.Height ) ) ) ),
        maTransform(),
        mxClipPoly(),
        mfAlpha(0.0),
        mbActive(false),
        mbIsCurrClipRectangle(true),
        mbIsContentFullyOpaque( false ),
        mbTransformDirty( true )
    {
        ENSURE_AND_THROW( rDevice.get() && rSpriteCanvas.get(),
                          "CanvasBitmap::CanvasBitmap(): Invalid device or sprite canvas" );

        // setup graphic device
        maCanvasHelper.setGraphicDevice( rDevice );


        // setup back buffer
        // -----------------

        // create content backbuffer in screen depth
        mpBackBuffer.reset( new BackBuffer( *rDevice->getOutDev() ) );
        mpBackBuffer->getVirDev().SetOutputSizePixel( maSize );

        // create mask backbuffer, with one bit color depth
        mpBackBufferMask.reset( new BackBuffer( *rDevice->getOutDev(), true ) );
        mpBackBufferMask->getVirDev().SetOutputSizePixel( maSize );

        // TODO(F1): Implement alpha vdev (could prolly enable
        // antialiasing again, then)

        // disable font antialiasing (causes ugly shadows otherwise)
        mpBackBuffer->getVirDev().SetAntialiasing( ANTIALIASING_DISABLE_TEXT );
        mpBackBufferMask->getVirDev().SetAntialiasing( ANTIALIASING_DISABLE_TEXT );

        // set mask vdev drawmode, such that everything is painted
        // black. That leaves us with a binary image, white for
        // background, black for painted content
        mpBackBufferMask->getVirDev().SetDrawMode( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_BLACKTEXT |
                                                   DRAWMODE_BLACKGRADIENT | DRAWMODE_BLACKBITMAP );


        // setup canvas helper
        // -------------------

        // always render into back buffer, don't preserve state
        // (it's our private VDev, after all)
        maCanvasHelper.setOutDev( mpBackBuffer, false );
        maCanvasHelper.setBackgroundOutDev( mpBackBufferMask );
    }

    CanvasCustomSprite::~CanvasCustomSprite()
    {
    }

    void SAL_CALL CanvasCustomSprite::disposing()
    {
        tools::LocalGuard aGuard;

        mpSpriteCanvas.clear();

        // forward to parent
        CanvasCustomSprite_Base::disposing();
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL CanvasCustomSprite::drawBitmap( const uno::Reference< rendering::XBitmap >&  xBitmap,
                                                                                           const rendering::ViewState&                  viewState,
                                                                                           const rendering::RenderState&                renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        const ::BitmapEx& rBmpEx( tools::bitmapExFromXBitmap(xBitmap) );

        // check whether bitmap is non-alpha, and whether its
        // transformed size covers the whole sprite.
        if( !rBmpEx.IsTransparent() )
        {
            // TODO(Q2): Factor out to canvastools or similar

            ::basegfx::B2DHomMatrix aTransform;
            ::canvas::tools::mergeViewAndRenderTransform(aTransform,
                                                         viewState,
                                                         renderState);

            const geometry::IntegerSize2D& rSize( xBitmap->getSize() );

            ::basegfx::B2DPolygon aPoly(
                ::basegfx::tools::createPolygonFromRect(
                    ::basegfx::B2DRectangle( 0.0,0.0,
                                             rSize.Width+1,
                                             rSize.Height+1 ) ) );
            aPoly.transform( aTransform );

            if( ::basegfx::tools::isInside(
                    aPoly,
                    ::basegfx::tools::createPolygonFromRect(
                        ::basegfx::B2DRectangle( 0.0,0.0,
                                                 maSize.Width(),
                                                 maSize.Height() ) ),
                    true ) )
            {
                // bitmap will fully cover the sprite, set flag
                // appropriately
                mbIsContentFullyOpaque = true;
            }
        }

        // delegate to base
        return CanvasCustomSprite_Base::drawBitmap( xBitmap,
                                                    viewState,
                                                    renderState );
    }

    void SAL_CALL CanvasCustomSprite::setAlpha( double alpha ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        if( !mpSpriteCanvas.get() )
            return; // we're disposed

        ::canvas::tools::checkRange(alpha, 0.0, 1.0);

        if( alpha != mfAlpha )
        {
            mfAlpha = alpha;

            if( mbActive )
                mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                              ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                              getSpriteRect() );
        }
    }

    void SAL_CALL CanvasCustomSprite::move( const geometry::RealPoint2D&  aNewPos,
                                            const rendering::ViewState&   viewState,
                                            const rendering::RenderState& renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        if( !mpSpriteCanvas.get() )
            return; // we're disposed

        ::basegfx::B2DHomMatrix aTransform;
        ::canvas::tools::mergeViewAndRenderTransform(aTransform,
                                                     viewState,
                                                     renderState);

        ::basegfx::B2DPoint aPoint( ::basegfx::unotools::b2DPointFromRealPoint2D(aNewPos) );
        aPoint *= aTransform;

        if( aPoint != maPosition )
        {
            const ::Rectangle&          rBounds( getFullSpriteRect() );
            const ::basegfx::B2DPoint&  rOutPos(
                ::vcl::unotools::b2DPointFromPoint( rBounds.TopLeft() ) );

            if( mbActive )
                mpSpriteCanvas->moveSprite( Sprite::ImplRef( this ),
                                            rBounds.TopLeft(),
                                            ::vcl::unotools::pointFromB2DPoint(
                                                rOutPos - maPosition + aPoint ),
                                            rBounds.GetSize() );

            maPosition = aPoint;
        }
    }

    void SAL_CALL CanvasCustomSprite::transform( const geometry::AffineMatrix2D& aTransformation ) throw (lang::IllegalArgumentException,
                                                                                                          uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        ::basegfx::B2DHomMatrix aMatrix;
        ::basegfx::unotools::homMatrixFromAffineMatrix(aMatrix,
                                                       aTransformation);

        if( maTransform != aMatrix )
        {
            // retrieve bounds before and after transformation change,
            // and calc union of them, as the resulting update area.
            Rectangle aPrevBounds(
                getSpriteRect() );

            maTransform = aMatrix;

            aPrevBounds.Union( getSpriteRect() );

            if( mbActive )
                mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                              ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                              aPrevBounds );

            mbTransformDirty = true;
        }
    }

    namespace
    {
        /** This method computes the up to four rectangles that make
            up the set difference between two rectangles.

            @param a
            First rect

            @param b
            Second rect

            @param out
            Pointer to an array of at least four elements, herein, the
            resulting parts of the set difference are returned.

            @return
            The number of rectangles written to the out array.
         */
        unsigned int b2dRectComputeSetDifference( const ::basegfx::B2DRectangle&    a,
                                                  const ::basegfx::B2DRectangle&    b,
                                                  ::basegfx::B2DRectangle*          out )
        {
            OSL_ENSURE( out != NULL,
                        "b2dRectComputeSetDifference(): Invalid output array" );

            // special-casing the empty rect case (this will fail most
            // of the times below, because of the DBL_MIN/MAX special
            // values denoting emptyness in the rectangle.
            if( a.isEmpty() )
            {
                *out = b;
                return 1;
            }
            if( b.isEmpty() )
            {
                *out = a;
                return 1;
            }

            unsigned int num_rectangles = 0;

            double ax(a.getMinX());
            double ay(a.getMinY());
            double aw(a.getWidth());
            double ah(a.getHeight());
            double bx(b.getMinX());
            double by(b.getMinY());
            double bw(b.getWidth());
            double bh(b.getHeight());

            double h0 = (by > ay) ? by - ay : 0.0;
            double h3 = (by + bh < ay + ah) ? ay + ah - by - bh : 0.0;
            double w1 = (bx > ax) ? bx - ax : 0.0;
            double w2 = (ax + aw > bx + bw) ? ax + aw - bx - bw : 0.0;
            double h12 = (h0 + h3 < ah) ? ah - h0 - h3 : 0.0;

            if (h0 > 0)
                out[num_rectangles++] = ::basegfx::B2DRectangle(ax,ay,ax+aw,ay+h0);

            if (w1 > 0 && h12 > 0)
                out[num_rectangles++] = ::basegfx::B2DRectangle(ax,ay+h0,ax+w1,ay+h0+h12);

            if (w2 > 0 && h12 > 0)
                out[num_rectangles++] = ::basegfx::B2DRectangle(bx+bw,ay+h0,bx+bw+w2,ay+h0+h12);

            if (h3 > 0)
                out[num_rectangles++] = ::basegfx::B2DRectangle(ax,ay+h0+h12,ax+aw,ay+h0+h12+h3);

            return num_rectangles;
        }
    }

    void SAL_CALL CanvasCustomSprite::clip( const uno::Reference< rendering::XPolyPolygon2D >& xClip ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        mxClipPoly = xClip;

        if( mbActive )
        {
            const sal_Int32 nNumClipPolygons( mxClipPoly->getNumberOfPolygons() );

            if( !mxClipPoly.is() ||
                nNumClipPolygons == 0 )
            {
                // empty clip polygon -> everything is visible now
                maCurrClipBounds.reset();
                mbIsCurrClipRectangle = true;
            }
            else
            {
                // new clip is not empty - determine actual update
                // area
                const ::basegfx::B2DPolyPolygon& rClipPath(
                    tools::polyPolygonFromXPolyPolygon2D( mxClipPoly ) );

                // clip which is about to be set, expressed as a
                // b2drectangle
                const ::basegfx::B2DRectangle& rClipBounds(
                    ::basegfx::tools::getRange( rClipPath ) );

                const ::basegfx::B2DRectangle aBounds( 0.0, 0.0,
                                                       maSize.Width(),
                                                       maSize.Height() );

                // rectangular area which is actually covered by the sprite.
                // coordinates are relative to the sprite origin.
                ::basegfx::B2DRectangle aSpriteRectPixel;
                ::canvas::tools::calcTransformedRectBounds( aSpriteRectPixel,
                                                            aBounds,
                                                            maTransform );

                // aClipBoundsA = new clip [set intersection] sprite
                ::basegfx::B2DRectangle aClipBoundsA(rClipBounds);
                aClipBoundsA.intersect( aSpriteRectPixel );

                if( nNumClipPolygons != 1 )
                {
                    // new clip cannot be a single rectangle -> cannot
                    // optimize update
                    mbIsCurrClipRectangle = false;
                    maCurrClipBounds = aClipBoundsA;
                }
                else
                {
                    // new clip could be a single rectangle
                    const bool bNewClipIsRect(
                        ::basegfx::tools::isRectangle( rClipPath.getB2DPolygon(0) ) );

                    // both new and old clip are truly rectangles
                    // - can now take the optimized path
                    const bool bUseOptimizedUpdate( bNewClipIsRect &&
                                                    mbIsCurrClipRectangle );

                    const ::basegfx::B2DRectangle aOldBounds( maCurrClipBounds );

                    // store new current clip type
                    maCurrClipBounds = aClipBoundsA;
                    mbIsCurrClipRectangle = bNewClipIsRect;

                    if( bUseOptimizedUpdate  )
                    {
                        // aClipBoundsB = maCurrClipBounds, i.e. last clip [set intersection] sprite
                        ::basegfx::B2DRectangle aClipDifference[4];
                        const unsigned int num_rectangles(
                            b2dRectComputeSetDifference(aClipBoundsA,
                                                        aOldBounds,
                                                        aClipDifference) );

                        for(unsigned int i=0; i<num_rectangles; ++i)
                        {
                            // aClipDifference[i] now contains the final
                            // update area, coordinates are still relative
                            // to the sprite origin. before submitting
                            // this area to 'updateSprite()' we need to
                            // translate this area to the final position,
                            // coordinates need to be relative to the
                            // spritecanvas.
                            mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                                          ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                                          Rectangle( static_cast< sal_Int32 >( maPosition.getX() + aClipDifference[i].getMinX() ),
                                                                     static_cast< sal_Int32 >( maPosition.getY() + aClipDifference[i].getMinY() ),
                                                                     static_cast< sal_Int32 >( ceil( maPosition.getX() + aClipDifference[i].getMaxX() ) ),
                                                                     static_cast< sal_Int32 >( ceil( maPosition.getY() + aClipDifference[i].getMaxY() ) ) ) );
                        }

                        // early exit, needed to process the four
                        // difference rects independently.
                        return;
                    }
                }
            }

            mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                          ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                          getSpriteRect() );
        }
    }

    void SAL_CALL CanvasCustomSprite::setPriority( double nPriority ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F2): Implement sprite priority
    }

    void SAL_CALL CanvasCustomSprite::show() throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        if( !mpSpriteCanvas.get() )
            return; // we're disposed

        if( !mbActive )
        {
            mpSpriteCanvas->showSprite( Sprite::ImplRef( this ) );
            mbActive = true;

            if( mfAlpha != 0.0 )
            {
                mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                              ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                              getSpriteRect() );
            }
        }
    }

    void SAL_CALL CanvasCustomSprite::hide() throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        if( !mpSpriteCanvas.get() )
            return; // we're disposed

        if( mbActive )
        {
            mpSpriteCanvas->hideSprite( Sprite::ImplRef( this ) );
            mbActive = false;

            if( mfAlpha != 0.0 )
            {
                mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                              ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                              getSpriteRect() );
            }
        }
    }

    uno::Reference< rendering::XCanvas > SAL_CALL CanvasCustomSprite::getContentCanvas() throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        if( !mpSpriteCanvas.get() || !mpBackBuffer.get() )
            uno::Reference< rendering::XCanvas >(); // we're disposed

        if( mbActive )
        {
            // the client is about to render into the sprite. Thus,
            // potentially the whole sprite area has changed.
            mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                          ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                          getSpriteRect() );
        }

        // clear surface
        OutputDevice& rOutDev( mpBackBuffer->getOutDev() );
        rOutDev.EnableMapMode( FALSE );
        rOutDev.SetFillColor( Color( COL_WHITE ) );
        rOutDev.SetLineColor();
        rOutDev.DrawRect( Rectangle(Point(), maSize) );

        OutputDevice& rMaskOutDev( mpBackBufferMask->getOutDev() );
        rMaskOutDev.SetDrawMode( DRAWMODE_DEFAULT );
        rMaskOutDev.EnableMapMode( FALSE );
        rMaskOutDev.SetFillColor( Color( COL_WHITE ) );
        rMaskOutDev.SetLineColor();
        rMaskOutDev.DrawRect( Rectangle(Point(), maSize) );
        rMaskOutDev.SetDrawMode( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_BLACKTEXT |
                                 DRAWMODE_BLACKGRADIENT | DRAWMODE_BLACKBITMAP );

        // surface content has changed (we cleared it, at least)
        mbSurfaceDirty = true;

        // just cleared content to fully transparent
        mbIsContentFullyOpaque = false;

        return this;
    }

#define SERVICE_NAME "com.sun.star.rendering.CanvasCustomSprite"

    ::rtl::OUString SAL_CALL CanvasCustomSprite::getImplementationName() throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CANVASCUSTOMSPRITE_IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasCustomSprite::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasCustomSprite::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

    // Sprite
    void CanvasCustomSprite::redraw( OutputDevice& rTargetSurface ) const
    {
        tools::LocalGuard aGuard;

        redraw( rTargetSurface,
                ::vcl::unotools::pointFromB2DPoint( maPosition ) );
    }

    void CanvasCustomSprite::redraw( OutputDevice&  rTargetSurface,
                                     const Point&   rOutputPos ) const
    {
        tools::LocalGuard aGuard;

        if( !mpSpriteCanvas.get() || !mpBackBuffer.get() )
            return; // we're disposed

        // log output pos in device pixel
        VERBOSE_TRACE( "CanvasCustomSprite::redraw(): output pos is (%f, %f)",
                       maPosition.getX(),
                       maPosition.getY() );

        if( mbActive &&
            !::basegfx::fTools::equalZero( mfAlpha ) )
        {
            const Point aEmptyPoint;

            // TODO(F3): Support for alpha-VDev

            // Do we have to update our bitmaps (necessary if virdev
            // was painted to, or transformation changed)?
            const bool bNeedBitmapUpdate( mbSurfaceDirty ||
                                          mbTransformDirty ||
                                          maContent->IsEmpty() );
            if( bNeedBitmapUpdate )
            {
                mbSurfaceDirty = false;
                mbTransformDirty = false;

                Bitmap aBmp( mpBackBuffer->getOutDev().GetBitmap( aEmptyPoint, maSize ) );

                if( mbIsContentFullyOpaque )
                {
                    // optimized case: content canvas is fully opaque
                    // maContent = BitmapEx( aBmp.CreateDisplayBitmap( &rTargetSurface ) );
                    maContent = BitmapEx( aBmp );
                }
                else
                {
                    Bitmap aMask( mpBackBufferMask->getOutDev().GetBitmap( aEmptyPoint, maSize ) );

                    if( aMask.GetBitCount() != 1 )
                    {
                        OSL_ENSURE(false,
                                   "CanvasCustomSprite::redraw(): Mask bitmap is not monochrome (performance!)");
                        aMask.MakeMono(255);
                    }

                    // maContent = BitmapEx( aBmp.CreateDisplayBitmap( &rTargetSurface ),
                    //                       aMask.CreateDisplayBitmap( &rTargetSurface ) );
                    maContent = BitmapEx( aBmp, aMask );
                }
            }

            // might get changed below (e.g. adapted for
            // transformations)
            ::Size  aOutputSize( maSize );
            ::Point aOutPos( rOutputPos );

            // check whether matrix is "easy" to handle - pure
            // translations or scales are handled by OutputDevice
            // alone
            if( !maTransform.isIdentity() )
            {
                if( !::basegfx::fTools::equalZero( maTransform.get(0,1) ) ||
                    !::basegfx::fTools::equalZero( maTransform.get(1,0) ) )
                {
                    // "complex" transformation, employ affine
                    // transformator

                    ::basegfx::B2DHomMatrix aTransform( maTransform );
                    aTransform.translate( aOutPos.X(),
                                          aOutPos.Y() );

                    // modify output position, to account for the fact
                    // that transformBitmap() always normalizes its output
                    // bitmap into the smallest enclosing box.
                    ::basegfx::B2DRectangle aDestRect;
                    ::canvas::tools::calcTransformedRectBounds( aDestRect,
                                                                ::basegfx::B2DRectangle(0,
                                                                                        0,
                                                                                        maSize.Width(),
                                                                                        maSize.Height()),
                                                                aTransform );

                    aOutPos.X() = ::basegfx::fround( aDestRect.getMinX() );
                    aOutPos.Y() = ::basegfx::fround( aDestRect.getMinY() );

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
                        ::basegfx::fround( maSize.getWidth()  * maTransform.get(0,0) ) );
                    aOutputSize.setHeight(
                        ::basegfx::fround( maSize.getHeight() * maTransform.get(1,1) ) );

                    aOutPos.X() = ::basegfx::fround( aOutPos.X() + maTransform.get(0,2) );
                    aOutPos.Y() = ::basegfx::fround( aOutPos.Y() + maTransform.get(1,2) );
                }
            }

            // transformBitmap() might return empty bitmaps, for tiny
            // scales.
            if( !!(*maContent) )
            {
                rTargetSurface.Push( PUSH_CLIPREGION );

                // apply clip (if any)
                if( mxClipPoly.is() )
                {
                    const ::basegfx::B2DPolyPolygon& rClipPoly( tools::polyPolygonFromXPolyPolygon2D( mxClipPoly ) );

                    if( rClipPoly.count() )
                    {
                        PolyPolygon aPolyPoly( rClipPoly );

                        aPolyPoly.Translate( rOutputPos );

                        const Region aClipRegion( Region::GetRegionFromPolyPolygon( aPolyPoly ) );

                        rTargetSurface.IntersectClipRegion( aClipRegion );
                    }
                }

                if( ::rtl::math::approxEqual(mfAlpha, 1.0) )
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
                    BYTE nColor( static_cast<UINT8>( ::basegfx::fround( 255.0*(1.0 - mfAlpha) + .5) ) );
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

#if defined(VERBOSE) && defined(DBG_UTIL)
                // Paint little red sprite area markers
                rTargetSurface.SetLineColor( Color( 255,0,0 ) );
                rTargetSurface.SetFillColor();
                rTargetSurface.DrawLine( Point( aOutPos.X(),
                                                aOutPos.Y() ),
                                         Point( aOutPos.X()+4,
                                                aOutPos.Y() ) );
                rTargetSurface.DrawLine( Point( aOutPos.X(),
                                                aOutPos.Y() ),
                                         Point( aOutPos.X(),
                                                aOutPos.Y()+4 ) );
                rTargetSurface.DrawLine( Point( aOutPos.X()+aOutputSize.Width()-5,
                                                aOutPos.Y() ),
                                         Point( aOutPos.X()+aOutputSize.Width()-1,
                                                aOutPos.Y() ) );
                rTargetSurface.DrawLine( Point( aOutPos.X()+aOutputSize.Width()-1,
                                                aOutPos.Y() ),
                                         Point( aOutPos.X()+aOutputSize.Width()-1,
                                                aOutPos.Y()+4 ) );
                rTargetSurface.DrawLine( Point( aOutPos.X(),
                                                aOutPos.Y()+aOutputSize.Height()-1 ),
                                         Point( aOutPos.X()+4,
                                                aOutPos.Y()+aOutputSize.Height()-1 ) );
                rTargetSurface.DrawLine( Point( aOutPos.X(),
                                                aOutPos.Y()+aOutputSize.Height()-5 ),
                                         Point( aOutPos.X(),
                                                aOutPos.Y()+aOutputSize.Height()-1 ) );
                rTargetSurface.DrawLine( Point( aOutPos.X()+aOutputSize.Width()-5,
                                                aOutPos.Y()+aOutputSize.Height()-1 ),
                                         Point( aOutPos.X()+aOutputSize.Width()-1,
                                                aOutPos.Y()+aOutputSize.Height()-1 ) );
                rTargetSurface.DrawLine( Point( aOutPos.X()+aOutputSize.Width()-1,
                                                aOutPos.Y()+aOutputSize.Height()-5 ),
                                         Point( aOutPos.X()+aOutputSize.Width()-1,
                                                aOutPos.Y()+aOutputSize.Height()-1 ) );
#endif
            }
        }
    }

    bool CanvasCustomSprite::isAreaUpdateOpaque( const Rectangle& rUpdateArea ) const
    {
        if( !mbIsCurrClipRectangle ||
            !mbIsContentFullyOpaque ||
            !::rtl::math::approxEqual(mfAlpha, 1.0) )
        {
            // sprite either transparent, or clip rect does not
            // represent exact bounds -> update might not be fully
            // opaque
            return false;
        }
        else
        {
            const Rectangle& rSpriteRect( getSpriteRect() );

            // make sure sprite rect covers update area fully -
            // although the update area originates from the sprite,
            // it's by no means guaranteed that it's limited to this
            // sprite's update area - after all, other sprites might
            // have been merged, or this sprite is moving.

            // Note: as Rectangle::IsInside() checks for _strict_
            // insidedness (i.e. all rect edges must be strictly
            // inside, not equal to one of the spriteRect's edges),
            // need the check for equality here.
            return rSpriteRect == rUpdateArea || rSpriteRect.IsInside( rUpdateArea );
        }
    }

    ::basegfx::B2DPoint CanvasCustomSprite::getSpritePos() const
    {
        tools::LocalGuard aGuard;

        return maPosition;
    }

    ::basegfx::B2DSize CanvasCustomSprite::getSpriteSize() const
    {
        tools::LocalGuard aGuard;

        // TODO(Q1): Use AW's wrappers once resynced
        return ::basegfx::B2DSize( maSize.Width(),
                                   maSize.Height() );
    }

    bool CanvasCustomSprite::repaint( const GraphicObjectSharedPtr& rGrf,
                                      const ::Point&                rPt,
                                      const ::Size&                 rSz,
                                      const GraphicAttr&            rAttr ) const
    {
        tools::LocalGuard aGuard;

        mbSurfaceDirty = true;

        return maCanvasHelper.repaint( rGrf, rPt, rSz, rAttr );
    }

    Rectangle CanvasCustomSprite::getSpriteRect( const ::basegfx::B2DRectangle& rBounds ) const
    {
        // Internal! Only call with locked object mutex!
        ::basegfx::B2DHomMatrix aTransform( maTransform );
        aTransform.translate( maPosition.getX(),
                              maPosition.getY() );

        // transform bounds at origin, as the sprite transformation is
        // formulated that way
        ::basegfx::B2DRectangle aTransformedBounds;
        ::canvas::tools::calcTransformedRectBounds( aTransformedBounds,
                                                    rBounds,
                                                    aTransform );

        // return integer rect, rounded away from the center
        return Rectangle( static_cast< sal_Int32 >( aTransformedBounds.getMinX() ),
                          static_cast< sal_Int32 >( aTransformedBounds.getMinY() ),
                          static_cast< sal_Int32 >( aTransformedBounds.getMaxX() )+1,
                          static_cast< sal_Int32 >( aTransformedBounds.getMaxY() )+1 );
    }

    Rectangle CanvasCustomSprite::getSpriteRect() const
    {
        // Internal! Only call with locked object mutex!

        // return effective sprite rect, i.e. take active clip into
        // account
        if( maCurrClipBounds.isEmpty() )
            return getSpriteRect( ::basegfx::B2DRectangle( 0.0, 0.0,
                                                           maSize.Width(),
                                                           maSize.Height() ) );
        else
            // return integer rect, rounded away from the center
            return Rectangle( static_cast< sal_Int32 >( maPosition.getX() + maCurrClipBounds.getMinX() ),
                              static_cast< sal_Int32 >( maPosition.getY() + maCurrClipBounds.getMinY() ),
                              static_cast< sal_Int32 >( ceil( maPosition.getX() + maCurrClipBounds.getMaxX() ) ),
                              static_cast< sal_Int32 >( ceil( maPosition.getY() + maCurrClipBounds.getMaxY() ) ) );
    }

    Rectangle CanvasCustomSprite::getFullSpriteRect() const
    {
        // Internal! Only call with locked object mutex!
        return getSpriteRect( ::basegfx::B2DRectangle( 0.0, 0.0,
                                                       maSize.Width(),
                                                       maSize.Height() ) );
    }
}
