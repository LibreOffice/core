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

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <tools/diagnose_ex.h>

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <canvas/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>

#include "cairo_canvascustomsprite.hxx"
#include "cairo_spritecanvas.hxx"


using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    CanvasCustomSprite::CanvasCustomSprite( const ::com::sun::star::geometry::RealSize2D&   rSpriteSize,
                                            const SpriteCanvasRef&                          rRefDevice ) :
        mpSpriteCanvas( rRefDevice ),
        maSize( ::canvas::tools::roundUp( rSpriteSize.Width ),
                ::canvas::tools::roundUp( rSpriteSize.Height ) )
    {
        ENSURE_OR_THROW( rRefDevice.get(),
                          "CanvasCustomSprite::CanvasCustomSprite(): Invalid sprite canvas" );

        OSL_TRACE("sprite size: %d, %d",
                  ::canvas::tools::roundUp( rSpriteSize.Width ),
                  ::canvas::tools::roundUp( rSpriteSize.Height ));

        mpBufferSurface = mpSpriteCanvas->createSurface( maSize );

        maCanvasHelper.init( maSize,
                             *rRefDevice,
                             rRefDevice.get() );
        maCanvasHelper.setSurface( mpBufferSurface, true );

        maSpriteHelper.init( rSpriteSize,
                             rRefDevice );
        maSpriteHelper.setSurface( mpBufferSurface );

        // clear sprite to 100% transparent
        maCanvasHelper.clear();
    }

    void CanvasCustomSprite::disposeThis()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpSpriteCanvas.clear();
        mpBufferSurface.reset();

        // forward to parent
        CanvasCustomSpriteBaseT::disposeThis();
    }

    void CanvasCustomSprite::redraw( const CairoSharedPtr& pCairo,
                                     bool                  bBufferedUpdate ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        redraw( pCairo, maSpriteHelper.getPosPixel(), bBufferedUpdate );
    }

    void CanvasCustomSprite::redraw( const CairoSharedPtr&      pCairo,
                                     const ::basegfx::B2DPoint& rOrigOutputPos,
                                     bool                       bBufferedUpdate ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        maSpriteHelper.redraw( pCairo,
                               rOrigOutputPos,
                               mbSurfaceDirty,
                               bBufferedUpdate );

        mbSurfaceDirty = false;
    }

    bool CanvasCustomSprite::repaint( const SurfaceSharedPtr&       pSurface,
                                      const rendering::ViewState&   viewState,
                                      const rendering::RenderState& renderState )
    {
        return maCanvasHelper.repaint( pSurface, viewState, renderState );
    }

    SurfaceSharedPtr CanvasCustomSprite::getSurface()
    {
        return mpBufferSurface;
    }

    SurfaceSharedPtr CanvasCustomSprite::createSurface( const ::basegfx::B2ISize& rSize, Content aContent )
    {
        return mpSpriteCanvas->createSurface(rSize,aContent);
    }

    SurfaceSharedPtr CanvasCustomSprite::createSurface( ::Bitmap& rBitmap )
    {
        return mpSpriteCanvas->createSurface(rBitmap);
    }

    SurfaceSharedPtr CanvasCustomSprite::changeSurface( bool bHasAlpha, bool bCopyContent )
    {
        if( !bHasAlpha && !bCopyContent )
        {
            OSL_TRACE("replacing sprite background surface");

            mpBufferSurface = mpSpriteCanvas->createSurface( maSize, CAIRO_CONTENT_COLOR );
            maSpriteHelper.setSurface( mpBufferSurface );

            return mpBufferSurface;
        }

        return SurfaceSharedPtr();
    }

    OutputDevice* CanvasCustomSprite::getOutputDevice()
    {
        return mpSpriteCanvas->getOutputDevice();
    }

#define IMPLEMENTATION_NAME "CairoCanvas.CanvasCustomSprite"
#define SERVICE_NAME "com.sun.star.rendering.CanvasCustomSprite"

    ::rtl::OUString SAL_CALL CanvasCustomSprite::getImplementationName() throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasCustomSprite::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName == SERVICE_NAME;
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasCustomSprite::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
