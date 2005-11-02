/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvascustomsprite.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:59:14 $
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
#include <basegfx/numeric/ftools.hxx>

#include <canvas/canvastools.hxx>

#include "canvascustomsprite.hxx"


using namespace ::com::sun::star;


namespace vclcanvas
{

    CanvasCustomSprite::CanvasCustomSprite( const geometry::RealSize2D& rSpriteSize,
                                            const SpriteCanvasRef&      rSpriteCanvas,
                                            bool                        bShowSpriteBounds ) :
        mpSpriteCanvas( rSpriteCanvas )
    {
        ENSURE_AND_THROW( rSpriteCanvas.get() &&
                          rSpriteCanvas->getOutDev(),
                          "CanvasCustomSprite::CanvasCustomSprite(): Invalid sprite canvas" );

        // setup back buffer
        // -----------------

        const ::Size aSize(
            static_cast<sal_Int32>( ::std::max( 1.0,
                                                ceil( rSpriteSize.Width ))),  // round up to nearest int,
                                                                              // enforce sprite to have at
                                                                               // least (1,1) pixel size
            static_cast<sal_Int32>( ::std::max( 1.0,
                                                ceil( rSpriteSize.Height ))) );

        // create content backbuffer in screen depth
        BackBufferSharedPtr pBackBuffer( new BackBuffer( *rSpriteCanvas->getOutDev() ) );
        pBackBuffer->setSize( aSize );

        // create mask backbuffer, with one bit color depth
        BackBufferSharedPtr pBackBufferMask( new BackBuffer( *rSpriteCanvas->getOutDev(),
                                                             true ) );
        pBackBufferMask->setSize( aSize );

        // TODO(F1): Implement alpha vdev (could prolly enable
        // antialiasing again, then)

        // disable font antialiasing (causes ugly shadows otherwise)
        pBackBuffer->getVirDev().SetAntialiasing( ANTIALIASING_DISABLE_TEXT );
        pBackBufferMask->getVirDev().SetAntialiasing( ANTIALIASING_DISABLE_TEXT );

        // set mask vdev drawmode, such that everything is painted
        // black. That leaves us with a binary image, white for
        // background, black for painted content
        pBackBufferMask->getVirDev().SetDrawMode( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_BLACKTEXT |
                                                  DRAWMODE_BLACKGRADIENT | DRAWMODE_BLACKBITMAP );


        // setup canvas helper
        // -------------------

        // always render into back buffer, don't preserve state (it's
        // our private VDev, after all), have notion of alpha
        maCanvasHelper.init( *rSpriteCanvas.get(),
                             pBackBuffer,
                             false,
                             true );
        maCanvasHelper.setBackgroundOutDev( pBackBufferMask );


        // setup sprite helper
        // -------------------

        maSpriteHelper.init( rSpriteSize,
                             rSpriteCanvas,
                             pBackBuffer,
                             pBackBufferMask,
                             bShowSpriteBounds );
    }

    void SAL_CALL CanvasCustomSprite::disposing()
    {
        tools::LocalGuard aGuard;

        mpSpriteCanvas.clear();

        // forward to parent
        CanvasCustomSpriteBaseT::disposing();
    }

#define IMPLEMENTATION_NAME "VCLCanvas.CanvasCustomSprite"
#define SERVICE_NAME "com.sun.star.rendering.CanvasCustomSprite"

    ::rtl::OUString SAL_CALL CanvasCustomSprite::getImplementationName() throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
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
    void CanvasCustomSprite::redraw( OutputDevice& rOutDev,
                                     bool          bBufferedUpdate ) const
    {
        tools::LocalGuard aGuard;

        redraw( rOutDev, maSpriteHelper.getPosPixel(), bBufferedUpdate );
    }

    void CanvasCustomSprite::redraw( OutputDevice&              rOutDev,
                                     const ::basegfx::B2DPoint& rOrigOutputPos,
                                     bool                       bBufferedUpdate ) const
    {
        tools::LocalGuard aGuard;

        maSpriteHelper.redraw( rOutDev,
                               rOrigOutputPos,
                               mbSurfaceDirty,
                               bBufferedUpdate );

        mbSurfaceDirty = false;
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

}
