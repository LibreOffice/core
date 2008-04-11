/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_canvascustomsprite.cxx,v $
 * $Revision: 1.6 $
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
#include <canvas/verbosetrace.hxx>

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
        ENSURE_AND_THROW( rRefDevice.get(),
                          "CanvasCustomSprite::CanvasCustomSprite(): Invalid sprite canvas" );

        OSL_TRACE("sprite size: %d, %d",
                  ::canvas::tools::roundUp( rSpriteSize.Width ),
                  ::canvas::tools::roundUp( rSpriteSize.Height ));

        //mpBufferSurface = mpSpriteCanvas->getSurface( aSize, CAIRO_CONTENT_COLOR );
        mpBufferSurface = mpSpriteCanvas->getSurface( maSize );

        maCanvasHelper.init( maSize, *rRefDevice.get() );
        maCanvasHelper.setSurface( mpBufferSurface, true, this );

        maSpriteHelper.init( rSpriteSize,
                             rRefDevice );
        maSpriteHelper.setSurface( mpBufferSurface );

        // clear sprite to 100% transparent
        maCanvasHelper.clear();
    }

    ::cairo::Surface* CanvasCustomSprite::changeSurface( bool bHasAlpha, bool bCopyContent )
    {
        if( !bHasAlpha && !bCopyContent )
        {
            OSL_TRACE("replacing sprite background surface");

            if( mpBufferSurface )
                mpBufferSurface->Unref();
            mpBufferSurface = mpSpriteCanvas->getSurface( maSize, CAIRO_CONTENT_COLOR );

            maSpriteHelper.setSurface( mpBufferSurface );

            return mpBufferSurface;
        }

        return NULL;
    }

    void SAL_CALL CanvasCustomSprite::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpSpriteCanvas.clear();

        if( mpBufferSurface )
        {
            mpBufferSurface->Unref();
            mpBufferSurface = NULL;
        }

        // forward to parent
        CanvasCustomSpriteBaseT::disposing();
    }

    void CanvasCustomSprite::redraw( Cairo* pCairo,
                                     bool bBufferedUpdate ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        redraw( pCairo, maSpriteHelper.getPosPixel(), bBufferedUpdate );
    }

    void CanvasCustomSprite::redraw( Cairo* pCairo,
                                     const ::basegfx::B2DPoint& rOrigOutputPos,
                                     bool bBufferedUpdate ) const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        maSpriteHelper.redraw( pCairo,
                               rOrigOutputPos,
                               mbSurfaceDirty,
                               bBufferedUpdate );

        mbSurfaceDirty = false;
    }

    bool CanvasCustomSprite::repaint( Surface* pSurface,
                      const rendering::ViewState&   viewState,
                      const rendering::RenderState& renderState )
    {
    return maCanvasHelper.repaint( pSurface, viewState, renderState );
    }

#define IMPLEMENTATION_NAME "CairoCanvas.CanvasCustomSprite"
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
}
