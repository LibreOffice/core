/*************************************************************************
 *
 *  $RCSfile: canvascustomsprite.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:26:18 $
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
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#include <canvas/canvastools.hxx>

#include "canvascustomsprite.hxx"


using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;


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
        maSize( ::vcl::unotools::sizeFromRealSize2D( rSpriteSize ) ),
        mxClipPoly(),
        mfAlpha(0.0),
        mbActive(false)
    {
        ENSURE_AND_THROW( rDevice.get() && rSpriteCanvas.get(),
                          "CanvasBitmap::CanvasBitmap(): Invalid device or sprite canvas" );

        // to prevent truncations due to round-offs
        maSize.Width() += 1; maSize.Height() += 1;

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

    void SAL_CALL CanvasCustomSprite::setAlpha( double alpha ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        if( !mpSpriteCanvas.get() )
            return; // we're disposed

        ::canvas::tools::checkRange(alpha, 0.0, 1.0);

        if( alpha != mfAlpha )
        {
            if( mbActive )
                mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                              ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                              Rectangle( ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                                         maSize ) );

            mfAlpha = alpha;
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
            if( mbActive )
                mpSpriteCanvas->moveSprite( Sprite::ImplRef( this ),
                                            ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                            ::vcl::unotools::pointFromB2DPoint( aPoint ) );

            maPosition = aPoint;
        }
    }

    void SAL_CALL CanvasCustomSprite::transform( const geometry::AffineMatrix2D& aTransformation ) throw (lang::IllegalArgumentException,
                                                                                                          uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(P3): Implement transformed sprites
        if( mbActive )
            mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                          ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                          Rectangle( ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                                     maSize ) );
    }

    void SAL_CALL CanvasCustomSprite::clip( const uno::Reference< rendering::XPolyPolygon2D >& xClip ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        mxClipPoly = xClip;

        if( mbActive )
            mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                          ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                          Rectangle( ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                                     maSize ) );
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
                                              Rectangle( ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                                         maSize ) );
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
                                              Rectangle( ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                                         maSize ) );
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
                                          Rectangle( ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                                     maSize ) );
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

        return this;
    }

#define SERVICE_NAME "drafts.com.sun.star.rendering.CanvasCustomSprite"

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

                    const Region aClipRegion( aPolyPoly );

                    rTargetSurface.SetClipRegion( aClipRegion );
                }
            }

            // To we have to update our bitmaps (necessary when virdev was
            // painted to)?
            if( mbSurfaceDirty ||
                maContent->IsEmpty() )
            {
                mbSurfaceDirty = false;

                Bitmap aBmp( mpBackBuffer->getOutDev().GetBitmap( aEmptyPoint, maSize ) );
                Bitmap aMask( mpBackBufferMask->getOutDev().GetBitmap( aEmptyPoint, maSize ) );

                if( aMask.GetBitCount() != 1 )
                {
                    OSL_ENSURE(false,
                               "CanvasCustomSprite::redraw(): Mask bitmap is not monochrome (performance!)");
                    aMask.MakeMono(255);
                }

                maContent = BitmapEx( aBmp.CreateDisplayBitmap( &rTargetSurface ),
                                      aMask.CreateDisplayBitmap( &rTargetSurface ) );
            }

            if( ::rtl::math::approxEqual(mfAlpha, 1.0) )
            {
                // fully opaque -> copy to output
                rTargetSurface.DrawBitmapEx( rOutputPos, *maContent );
            }
            else
            {
                // TODO(P3): Switch to OutputDevice::DrawTransparent()
                // here

                // draw semi-transparent
                BYTE nColor( static_cast<UINT8>( ::basegfx::fround( 255.0*(1.0 - mfAlpha) + .5) ) );
                AlphaMask aAlpha( maSize, &nColor );

                // mask out fully transparent areas
                aAlpha.Replace( maContent->GetMask(), 255 );

                // alpha-blend to output
                rTargetSurface.DrawBitmapEx( rOutputPos,
                                             BitmapEx( maContent->GetBitmap(),
                                                       aAlpha ) );
            }

            rTargetSurface.SetClipRegion();
        }


#if defined(VERBOSE) && defined(DBG_UTIL)
        // Paint little red sprite area markers
        rTargetSurface.SetLineColor( Color( 255,0,0 ) );
        rTargetSurface.SetFillColor();
        rTargetSurface.DrawLine( Point( rOutputPos.X(),
                                        rOutputPos.Y() ),
                                 Point( rOutputPos.X()+4,
                                        rOutputPos.Y() ) );
        rTargetSurface.DrawLine( Point( rOutputPos.X(),
                                        rOutputPos.Y() ),
                                 Point( rOutputPos.X(),
                                        rOutputPos.Y()+4 ) );
        rTargetSurface.DrawLine( Point( rOutputPos.X()+maSize.Width()-5,
                                        rOutputPos.Y() ),
                                 Point( rOutputPos.X()+maSize.Width()-1,
                                        rOutputPos.Y() ) );
        rTargetSurface.DrawLine( Point( rOutputPos.X()+maSize.Width()-1,
                                        rOutputPos.Y() ),
                                 Point( rOutputPos.X()+maSize.Width()-1,
                                        rOutputPos.Y()+4 ) );
        rTargetSurface.DrawLine( Point( rOutputPos.X(),
                                        rOutputPos.Y()+maSize.Height()-1 ),
                                 Point( rOutputPos.X()+4,
                                        rOutputPos.Y()+maSize.Height()-1 ) );
        rTargetSurface.DrawLine( Point( rOutputPos.X(),
                                        rOutputPos.Y()+maSize.Height()-5 ),
                                 Point( rOutputPos.X(),
                                        rOutputPos.Y()+maSize.Height()-1 ) );
        rTargetSurface.DrawLine( Point( rOutputPos.X()+maSize.Width()-5,
                                        rOutputPos.Y()+maSize.Height()-1 ),
                                 Point( rOutputPos.X()+maSize.Width()-1,
                                        rOutputPos.Y()+maSize.Height()-1 ) );
        rTargetSurface.DrawLine( Point( rOutputPos.X()+maSize.Width()-1,
                                        rOutputPos.Y()+maSize.Height()-5 ),
                                 Point( rOutputPos.X()+maSize.Width()-1,
                                        rOutputPos.Y()+maSize.Height()-1 ) );
#endif
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
}
