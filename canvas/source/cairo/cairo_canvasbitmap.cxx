/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cairo_canvasbitmap.cxx,v $
 * $Revision: 1.4 $
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
#include <canvas/canvastools.hxx>

#include "cairo_canvasbitmap.hxx"


using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    CanvasBitmap::CanvasBitmap( const ::basegfx::B2ISize& rSize,
                                const DeviceRef&          rDevice,
                                bool                      bHasAlpha ) :
        mpDevice( rDevice )
    {
        ENSURE_AND_THROW( mpDevice.is(),
                          "CanvasBitmap::CanvasBitmap(): Invalid surface or device" );

        OSL_TRACE( "bitmap size: %dx%d", rSize.getX(), rSize.getY() );

        mpBufferSurface = mpDevice->getSurface( rSize, bHasAlpha ? CAIRO_CONTENT_COLOR_ALPHA : CAIRO_CONTENT_COLOR );
        mpBufferCairo = mpBufferSurface->getCairo();

        maCanvasHelper.init( rSize, *mpDevice.get() );
        maCanvasHelper.setSurface( mpBufferSurface, bHasAlpha );

        mbHasAlpha = bHasAlpha;
    }

    void SAL_CALL CanvasBitmap::disposing()
    {
        mpDevice.clear();

        if( mpBufferCairo ) {
            cairo_destroy( mpBufferCairo );
            mpBufferCairo = NULL;
        }

        if( mpBufferSurface ) {
            mpBufferSurface->Unref();
            mpBufferSurface = NULL;
        }

        // forward to parent
        CanvasBitmap_Base::disposing();
    }

    Surface* CanvasBitmap::getSurface()
    {
        return mpBufferSurface;
    }

    bool CanvasBitmap::repaint( Surface* pSurface,
                                const rendering::ViewState& viewState,
                                const rendering::RenderState&   renderState )
    {
        return maCanvasHelper.repaint( pSurface, viewState, renderState );
    }

#define IMPLEMENTATION_NAME "CairoCanvas.CanvasBitmap"
#define SERVICE_NAME "com.sun.star.rendering.CanvasBitmap"

    ::rtl::OUString SAL_CALL CanvasBitmap::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

}
