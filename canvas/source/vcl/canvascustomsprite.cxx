/*************************************************************************
 *
 *  $RCSfile: canvascustomsprite.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2004-03-18 10:38:40 $
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

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#include <canvas/canvastools.hxx>

#include "canvascustomsprite.hxx"
#include "spritecanvas.hxx"
#include "impltools.hxx"

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
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

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

namespace vclcanvas
{

    CanvasCustomSprite::CanvasCustomSprite( const geometry::RealSize2D&     rSpriteSize,
                                            const SpriteCanvas::ImplRef&    rSpriteCanvas ) :
        maVDev( new VirtualDevice( rSpriteCanvas->getOutDev() ) ), // create from reference device
        maMaskVDev( new VirtualDevice( rSpriteCanvas->getOutDev(), 1) ), // create from reference device, bit depth: one
        maCanvasHelper(),
        mpSpriteCanvas( rSpriteCanvas ),
        maPosition(0.0, 0.0),
        maSize( ::vcl::unotools::sizeFromRealSize2D( rSpriteSize ) ),
        mfAlpha(0.0),
        mbActive( false )
    {
        // setup outdev sizes to total sprite size
        maVDev->SetOutputSizePixel( maSize );
        maMaskVDev->SetOutputSizePixel( maSize );

        // set mask vdev drawmode, such that everything is painted
        // black. That leaves us with a binary image, white for
        // background, black for painted content
        maMaskVDev->SetDrawMode( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_BLACKTEXT |
                                 DRAWMODE_BLACKGRADIENT | DRAWMODE_BLACKBITMAP );

        maCanvasHelper.setOutDev( *maVDev );
        maCanvasHelper.setBackgroundOutDev( *maMaskVDev );
    }

    CanvasCustomSprite::~CanvasCustomSprite()
    {
        hide();
    }

    OutDevProvider::ImplRef CanvasCustomSprite::getImplRef()
    {
        return OutDevProvider::ImplRef::createFromQuery( this );
    }

    void SAL_CALL CanvasCustomSprite::drawPoint( const geometry::RealPoint2D&   aPoint,
                                                 const rendering::ViewState&    viewState,
                                                 const rendering::RenderState&  renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        // TODO: You might consider copying the SpriteCanvas
        // background VDev behaviour here, notably the call to
        // SpriteSurface::updateSprite(). But on the other hand, there
        // are enough external cases that could mess that up (see
        // comment on XCustomSprite), that we're maybe wise to keep it
        // like this (i.e. invalidating the sprite content only once,
        // for the call to getContentCanvas). This way, the inevitable
        // errors will show up earlier, not only when multi-threading
        // the updateScreen.
        maCanvasHelper.drawPoint( aPoint, viewState, renderState, getImplRef() );
    }

    void SAL_CALL CanvasCustomSprite::drawLine( const geometry::RealPoint2D& aStartPoint, const geometry::RealPoint2D& aEndPoint, const rendering::ViewState& viewState, const rendering::RenderState& renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        maCanvasHelper.drawLine(aStartPoint, aEndPoint, viewState, renderState, getImplRef());
    }

    void SAL_CALL CanvasCustomSprite::drawBezier( const ::drafts::com::sun::star::geometry::RealBezierSegment2D&    aBezierSegment,
                                                  const ::drafts::com::sun::star::geometry::RealPoint2D&            aEndPoint,
                                                  const rendering::ViewState&                                       viewState,
                                                  const rendering::RenderState&                                     renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        maCanvasHelper.drawBezier(aBezierSegment, aEndPoint, viewState, renderState, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL CanvasCustomSprite::drawPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.drawPolyPolygon(xPolyPolygon, viewState, renderState, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL CanvasCustomSprite::strokePolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const rendering::StrokeAttributes& strokeAttributes ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.strokePolyPolygon(xPolyPolygon, viewState, renderState, strokeAttributes, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL CanvasCustomSprite::strokeTexturedPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const uno::Sequence< rendering::Texture >& textures, const rendering::StrokeAttributes& strokeAttributes ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.strokeTexturedPolyPolygon(xPolyPolygon, viewState, renderState, textures, strokeAttributes, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL CanvasCustomSprite::strokeTextureMappedPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const uno::Sequence< rendering::Texture >& textures, const uno::Reference< geometry::XMapping2D >& xMapping, const rendering::StrokeAttributes& strokeAttributes ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.strokeTextureMappedPolyPolygon(xPolyPolygon, viewState, renderState, textures, xMapping, strokeAttributes, getImplRef());
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL CanvasCustomSprite::queryStrokeShapes( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const rendering::StrokeAttributes& strokeAttributes ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.queryStrokeShapes(xPolyPolygon, viewState, renderState, strokeAttributes, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL CanvasCustomSprite::fillPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.fillPolyPolygon(xPolyPolygon, viewState, renderState, getImplRef() );
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL CanvasCustomSprite::fillTexturedPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const uno::Sequence< rendering::Texture >& textures ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.fillTexturedPolyPolygon(xPolyPolygon, viewState, renderState, textures, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL CanvasCustomSprite::fillTextureMappedPolyPolygon( const uno::Reference< rendering::XPolyPolygon2D >& xPolyPolygon, const rendering::ViewState& viewState, const rendering::RenderState& renderState, const uno::Sequence< rendering::Texture >& textures, const uno::Reference< geometry::XMapping2D >& xMapping ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.fillTextureMappedPolyPolygon(xPolyPolygon, viewState, renderState, textures, xMapping, getImplRef());
    }

    uno::Reference< rendering::XCanvasFont > SAL_CALL CanvasCustomSprite::queryFont( const rendering::FontRequest& fontRequest ) throw (uno::RuntimeException)
    {
        return maCanvasHelper.queryFont( fontRequest, getImplRef() );
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL CanvasCustomSprite::drawText( const rendering::StringContext&                    text,
                                                                                         const uno::Reference< rendering::XCanvasFont >&    xFont,
                                                                                         const rendering::ViewState&                        viewState,
                                                                                         const rendering::RenderState&                      renderState,
                                                                                         sal_Int8                                           textDirection ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.drawText(text, xFont, viewState, renderState, textDirection, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL CanvasCustomSprite::drawOffsettedText( const rendering::StringContext& text, const uno::Reference< rendering::XCanvasFont >& xFont, const uno::Sequence< double >& offsets, const rendering::ViewState& viewState, const rendering::RenderState& renderState, sal_Int8 textDirection ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.drawOffsettedText(text, xFont, offsets, viewState, renderState, textDirection, getImplRef());
    }

    uno::Reference< rendering::XCachedPrimitive > SAL_CALL CanvasCustomSprite::drawBitmap( const uno::Reference< rendering::XBitmap >& xBitmap, const rendering::ViewState& viewState, const rendering::RenderState& renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        return maCanvasHelper.drawBitmap(xBitmap, viewState, renderState, getImplRef());
    }

    uno::Reference< rendering::XGraphicDevice > SAL_CALL CanvasCustomSprite::getDevice() throw (uno::RuntimeException)
    {
        return maCanvasHelper.getDevice( getImplRef() );
    }

    void SAL_CALL CanvasCustomSprite::copyRect( const uno::Reference< rendering::XBitmapCanvas >&   sourceCanvas,
                                                const geometry::RealRectangle2D&                    sourceRect,
                                                const rendering::ViewState&                         sourceViewState,
                                                const rendering::RenderState&                       sourceRenderState,
                                                const geometry::RealRectangle2D&                    destRect,
                                                const rendering::ViewState&                         destViewState,
                                                const rendering::RenderState&                       destRenderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        tools::LocalGuard aGuard;
    }

    void SAL_CALL CanvasCustomSprite::setAlpha( double alpha ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
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

    void SAL_CALL CanvasCustomSprite::move( const ::drafts::com::sun::star::geometry::RealPoint2D&  aNewPos,
                                            const ::drafts::com::sun::star::rendering::ViewState&   viewState,
                                            const ::drafts::com::sun::star::rendering::RenderState& renderState ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
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
        // TODO
    }

    void SAL_CALL CanvasCustomSprite::clip( const uno::Reference< rendering::XPolyPolygon2D >&  aClip,
                                            const rendering::ViewState&                         viewState,
                                            const rendering::RenderState&                       renderState ) throw (lang::IllegalArgumentException,
                                                                                                                     uno::RuntimeException)
    {
        // TODO
    }

    void SAL_CALL CanvasCustomSprite::show(  ) throw (::com::sun::star::uno::RuntimeException)
    {
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

    void SAL_CALL CanvasCustomSprite::hide(  ) throw (::com::sun::star::uno::RuntimeException)
    {
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
        if( mbActive )
        {
            // the client is about to render into the sprite. Thus,
            // potentially the whole sprite area has changed.
            mpSpriteCanvas->updateSprite( Sprite::ImplRef( this ),
                                          ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                          Rectangle( ::vcl::unotools::pointFromB2DPoint( maPosition ),
                                                     maSize ) );
        }

        // TODO: Locking! Everywhere!
        tools::LocalGuard aGuard;

        // clear surface
        maVDev->EnableMapMode( FALSE );
        maVDev->SetFillColor( Color( COL_WHITE ) );
        maVDev->SetLineColor();
        maVDev->DrawRect( Rectangle(Point(), maSize) );

        maMaskVDev->SetDrawMode( DRAWMODE_DEFAULT );
        maMaskVDev->EnableMapMode( FALSE );
        maMaskVDev->SetFillColor( Color( COL_WHITE ) );
        maMaskVDev->SetLineColor();
        maMaskVDev->DrawRect( Rectangle(Point(), maSize) );
        maMaskVDev->SetDrawMode( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_BLACKTEXT |
                                 DRAWMODE_BLACKGRADIENT | DRAWMODE_BLACKBITMAP );

        return this;
    }

#define SERVICE_NAME "drafts.com.sun.star.rendering.Canvas"

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
        redraw( rTargetSurface,
                ::vcl::unotools::pointFromB2DPoint( maPosition ) );
    }

    void CanvasCustomSprite::redraw( OutputDevice& rTargetSurface, const Point& rOutputPos ) const
    {
        tools::OutDevStateKeeper aStateKeeper(rTargetSurface);

        const Point     aEmptyPoint;
        const Bitmap    aMaskBitmap( maMaskVDev->GetBitmap( aEmptyPoint, maSize ) );
        const Bitmap    aContentBitmap( maVDev->GetBitmap( aEmptyPoint, maSize ) );

        // TODO: Support for alpha-VDev
        rTargetSurface.EnableMapMode( FALSE );

        if( ::rtl::math::approxEqual(mfAlpha, 1.0) )
        {
            // copy to output
            rTargetSurface.DrawBitmapEx( rOutputPos,
                                         BitmapEx( aContentBitmap, aMaskBitmap ) );
        }
        else if( mfAlpha != 0.0 ) // TODO: be a little bit more tolerant here
        {
            // Only draw something if we're not completely transparent
            BYTE nColor( static_cast<UINT8>(255.0*(1.0 - mfAlpha) + .5) );
            AlphaMask aAlpha( maSize, &nColor );

            // mask out fully transparent areas
            aAlpha.Replace( aMaskBitmap, 255 );

            // alpha-blend to output
            rTargetSurface.DrawBitmapEx( rOutputPos,
                                         BitmapEx( aContentBitmap, aAlpha ) );
        }
    }

    ::basegfx::B2DSize CanvasCustomSprite::getSize() const
    {
        // TODO: Use AW's wrappers once resynced
        return ::basegfx::B2DSize( maSize.Width(),
                                   maSize.Height() );
    }

    // OutDevProvider
    ::OutputDevice& CanvasCustomSprite::getOutDev()
    {
        return *maVDev;
    }

    const ::OutputDevice& CanvasCustomSprite::getOutDev() const
    {
        return *maVDev;
    }

}
