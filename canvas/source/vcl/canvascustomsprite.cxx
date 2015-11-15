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
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/alpha.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>

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
        pBackBuffer->getOutDev().SetAntialiasing( AntialiasingFlags::DisableText );
        pBackBufferMask->getOutDev().SetAntialiasing( AntialiasingFlags::DisableText );

        // set mask vdev drawmode, such that everything is painted
        // black. That leaves us with a binary image, white for
        // background, black for painted content
        pBackBufferMask->getOutDev().SetDrawMode( DrawModeFlags::BlackLine | DrawModeFlags::BlackFill | DrawModeFlags::BlackText |
                                                  DrawModeFlags::BlackGradient | DrawModeFlags::BlackBitmap );


        // setup canvas helper


        // always render into back buffer, don't preserve state (it's
        // our private VDev, after all), have notion of alpha
        maCanvasHelper.init( rDevice,
                             pBackBuffer,
                             false,
                             true );
        maCanvasHelper.setBackgroundOutDev( pBackBufferMask );


        // setup sprite helper


        maSpriteHelper.init( rSpriteSize,
                             rOwningSpriteCanvas,
                             pBackBuffer,
                             pBackBufferMask,
                             bShowSpriteBounds );

        // clear sprite to 100% transparent
        maCanvasHelper.clear();
    }

    OUString SAL_CALL CanvasCustomSprite::getImplementationName() throw( uno::RuntimeException, std::exception )
    {
        return OUString( "VCLCanvas.CanvasCustomSprite" );
    }

    sal_Bool SAL_CALL CanvasCustomSprite::supportsService( const OUString& ServiceName ) throw( uno::RuntimeException, std::exception )
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL CanvasCustomSprite::getSupportedServiceNames()  throw( uno::RuntimeException, std::exception )
    {
        uno::Sequence< OUString > aRet { "com.sun.star.rendering.CanvasCustomSprite" };

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
