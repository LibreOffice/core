/*************************************************************************
 *
 *  $RCSfile: canvascustomsprite.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2005-04-18 09:10:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            const ::Rectangle&          rBounds( getSpriteRect() );
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

    void SAL_CALL CanvasCustomSprite::clip( const uno::Reference< rendering::XPolyPolygon2D >& xClip ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        mxClipPoly = xClip;

        if( mbActive )
            mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                          ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                          getSpriteRect() );
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

    void CanvasCustomSprite::redraw( OutputDevice& rTargetSurface, const Point& rOutputPos ) const
    {
        tools::LocalGuard aGuard;

        if( !mpSpriteCanvas.get() || !mpBackBuffer.get() )
            return; // we're disposed

        // log output pos in device pixel
        VERBOSE_TRACE( "CanvasCustomSprite::redraw(): output pos is (%f, %f)",
                       maPosition.getX(),
                       maPosition.getY() );

        // Prepare the outdev
        rTargetSurface.EnableMapMode( FALSE );
        rTargetSurface.SetClipRegion();

        if( mbActive &&
            !::basegfx::fTools::equalZero( mfAlpha ) )
        {
            const Point aEmptyPoint;

            // TODO(F3): Support for alpha-VDev

            // apply clip (if any)
            if( mxClipPoly.is() )
            {
                const ::basegfx::B2DPolyPolygon& rClipPoly( tools::polyPolygonFromXPolyPolygon2D( mxClipPoly ) );

                if( rClipPoly.count() )
                {
                    PolyPolygon aPolyPoly( rClipPoly );

                    aPolyPoly.Translate( rOutputPos );

                    const Region aClipRegion( Region::GetRegionFromPolyPolygon( aPolyPoly ) );

                    rTargetSurface.SetClipRegion( aClipRegion );
                }
            }

            // might get changed below (e.g. adapted for
            // transformations)
            ::Size  aOutputSize( maSize );
            ::Point aOutPos( rOutputPos );

            // Do we have to update our bitmaps (necessary if virdev
            // was painted to, or transformation changed)?
            if( mbSurfaceDirty ||
                mbTransformDirty ||
                maContent->IsEmpty() )
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
            }

            // transformBitmap() might return empty bitmaps, for tiny
            // scales.
            if( !!(*maContent) )
            {
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

            rTargetSurface.SetClipRegion();
        }
    }

    ::basegfx::B2DPoint CanvasCustomSprite::getPos() const
    {
        tools::LocalGuard aGuard;

        return maPosition;
    }

    ::basegfx::B2DSize CanvasCustomSprite::getSize() const
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

    Rectangle CanvasCustomSprite::getSpriteRect() const
    {
        // Internal! Only call with locked object mutex!
        ::basegfx::B2DRectangle aBounds( 0.0, 0.0,
                                         maSize.Width(),
                                         maSize.Height() );

        ::basegfx::B2DHomMatrix aTransform( maTransform );
        aTransform.translate( maPosition.getX(),
                              maPosition.getY() );

        // transform bounds at origin, as the sprite transformation is
        // formulated that way
        ::basegfx::B2DRectangle aTransformedBounds;
        ::canvas::tools::calcTransformedRectBounds( aTransformedBounds,
                                                    aBounds,
                                                    aTransform );

        // return integer rect, rounded away from the center
        return Rectangle( static_cast< sal_Int32 >( aTransformedBounds.getMinX() ),
                          static_cast< sal_Int32 >( aTransformedBounds.getMinY() ),
                          static_cast< sal_Int32 >( aTransformedBounds.getMaxX() )+1,
                          static_cast< sal_Int32 >( aTransformedBounds.getMaxY() )+1 );
    }
}
