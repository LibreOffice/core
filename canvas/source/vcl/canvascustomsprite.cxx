/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
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

    CanvasCustomSprite::CanvasCustomSprite( const geometry::RealSize2D&               rSpriteSize,
                                            rendering::XGraphicDevice&                rDevice,
                                            const ::canvas::SpriteSurface::Reference& rOwningSpriteCanvas,
                                            const OutDevProviderSharedPtr&            rOutDevProvider,
                                            bool                                      bShowSpriteBounds )
    {
        ENSURE_OR_THROW( rOwningSpriteCanvas.get() &&
                         rOutDevProvider,
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
        BackBufferSharedPtr pBackBuffer( new BackBuffer( rOutDevProvider->getOutDev() ) );
        pBackBuffer->setSize( aSize );

        // create mask backbuffer, with one bit color depth
        BackBufferSharedPtr pBackBufferMask( new BackBuffer( rOutDevProvider->getOutDev(),
                                                             true ) );
        pBackBufferMask->setSize( aSize );

        // TODO(F1): Implement alpha vdev (could prolly enable
        // antialiasing again, then)

        // disable font antialiasing (causes ugly shadows otherwise)
        pBackBuffer->getOutDev().SetAntialiasing( ANTIALIASING_DISABLE_TEXT );
        pBackBufferMask->getOutDev().SetAntialiasing( ANTIALIASING_DISABLE_TEXT );

        // set mask vdev drawmode, such that everything is painted
        // black. That leaves us with a binary image, white for
        // background, black for painted content
        pBackBufferMask->getOutDev().SetDrawMode( DRAWMODE_BLACKLINE | DRAWMODE_BLACKFILL | DRAWMODE_BLACKTEXT |
                                                  DRAWMODE_BLACKGRADIENT | DRAWMODE_BLACKBITMAP );


        // setup canvas helper
        // -------------------

        // always render into back buffer, don't preserve state (it's
        // our private VDev, after all), have notion of alpha
        maCanvasHelper.init( rDevice,
                             pBackBuffer,
                             false,
                             true );
        maCanvasHelper.setBackgroundOutDev( pBackBufferMask );


        // setup sprite helper
        // -------------------

        maSpriteHelper.init( rSpriteSize,
                             rOwningSpriteCanvas,
                             pBackBuffer,
                             pBackBufferMask,
                             bShowSpriteBounds );

        // clear sprite to 100% transparent
        maCanvasHelper.clear();
    }

    void SAL_CALL CanvasCustomSprite::disposing()
    {
        SolarMutexGuard aGuard;

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
        SolarMutexGuard aGuard;

        redraw( rOutDev, maSpriteHelper.getPosPixel(), bBufferedUpdate );
    }

    void CanvasCustomSprite::redraw( OutputDevice&              rOutDev,
                                     const ::basegfx::B2DPoint& rOrigOutputPos,
                                     bool                       bBufferedUpdate ) const
    {
        SolarMutexGuard aGuard;

        maSpriteHelper.redraw( rOutDev,
                               rOrigOutputPos,
                               mbSurfaceDirty,
                               bBufferedUpdate );

        mbSurfaceDirty = false;
    }

    bool CanvasCustomSprite::repaint( const GraphicObjectSharedPtr& rGrf,
                                      const rendering::ViewState&   viewState,
                                      const rendering::RenderState& renderState,
                                      const ::Point&                rPt,
                                      const ::Size&                 rSz,
                                      const GraphicAttr&            rAttr ) const
    {
        SolarMutexGuard aGuard;

        mbSurfaceDirty = true;

        return maCanvasHelper.repaint( rGrf, viewState, renderState, rPt, rSz, rAttr );
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
