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
#include <sal/log.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>

#include <canvas/canvastools.hxx>

#include "cairo_spritecanvas.hxx"

using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Sequence< uno::Any >&                aArguments,
                                const uno::Reference< uno::XComponentContext >& /*rxContext*/ ) :
        maArguments(aArguments)
    {
    }

    void SpriteCanvas::initialize()
    {
        SAL_INFO("canvas.cairo", "CairoSpriteCanvas created " << this);

        // #i64742# Only call initialize when not in probe mode
        if( maArguments.getLength() == 0 )
            return;

        /* maArguments:
           0: ptr to creating instance (Window or VirtualDevice)
           1: SystemEnvData as a streamed Any (or empty for VirtualDevice)
           2: current bounds of creating instance
           3: bool, denoting always on top state for Window (always false for VirtualDevice)
           4: XWindow for creating Window (or empty for VirtualDevice)
           5: SystemGraphicsData as a streamed Any
         */
        ENSURE_ARG_OR_THROW( maArguments.getLength() >= 4 &&
                             maArguments[0].getValueTypeClass() == uno::TypeClass_HYPER &&
                             maArguments[4].getValueTypeClass() == uno::TypeClass_INTERFACE,
                             "CairoSpriteCanvas::initialize: wrong number of arguments, or wrong types" );

        awt::Rectangle aRect;
        maArguments[2] >>= aRect;

        bool bIsFullscreen( false );
        maArguments[3] >>= bIsFullscreen;

        uno::Reference< awt::XWindow > xParentWindow;
        maArguments[4] >>= xParentWindow;

        VclPtr<vcl::Window> pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
        if( !pParentWindow )
            throw lang::NoSupportException(
                "Parent window not VCL window, or canvas out-of-process!", nullptr);

        bool bHasCairo = pParentWindow->SupportsCairo();
        ENSURE_ARG_OR_THROW(bHasCairo,
                            "CairoSpriteCanvas::SpriteCanvas: No Cairo capability");

        Size aPixelSize( pParentWindow->GetOutputSizePixel() );
        const ::basegfx::B2ISize aSize( aPixelSize.Width(),
                                        aPixelSize.Height() );

        // setup helper
        maDeviceHelper.init( *pParentWindow,
                             *this,
                             aSize,
                             bIsFullscreen );

        setWindow(uno::Reference<awt::XWindow2>(xParentWindow, uno::UNO_QUERY_THROW));

        maCanvasHelper.init( maRedrawManager,
                             *this,
                             aSize );

        maArguments.realloc(0);
    }

    void SpriteCanvas::disposeThis()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // forward to parent
        SpriteCanvasBaseT::disposeThis();
    }

    sal_Bool SAL_CALL SpriteCanvas::showBuffer( sal_Bool bUpdateAll )
    {
        return updateScreen( bUpdateAll );
    }

    sal_Bool SAL_CALL SpriteCanvas::switchBuffer( sal_Bool bUpdateAll )
    {
        return updateScreen( bUpdateAll );
    }

    sal_Bool SAL_CALL SpriteCanvas::updateScreen( sal_Bool bUpdateAll )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return mbIsVisible && maCanvasHelper.updateScreen(
            ::basegfx::unotools::b2IRectangleFromAwtRectangle(maBounds),
            bUpdateAll,
            mbSurfaceDirty);
    }

    OUString SAL_CALL SpriteCanvas::getServiceName(  )
    {
        return OUString( SPRITECANVAS_SERVICE_NAME );
    }

    SurfaceSharedPtr SpriteCanvas::getSurface()
    {
        return maDeviceHelper.getBufferSurface();
    }

    SurfaceSharedPtr SpriteCanvas::createSurface( const ::basegfx::B2ISize& rSize, int aContent )
    {
        return maDeviceHelper.createSurface( rSize, aContent );
    }

    SurfaceSharedPtr SpriteCanvas::createSurface( ::Bitmap& rBitmap )
    {
        BitmapSystemData aData;
        if( rBitmap.GetSystemData( aData ) ) {
            const Size& rSize = rBitmap.GetSizePixel();

            return maDeviceHelper.createSurface( aData, rSize );
        }

        return SurfaceSharedPtr();
    }

    SurfaceSharedPtr SpriteCanvas::changeSurface()
    {
        // non-modifiable surface here
        return SurfaceSharedPtr();
    }

    OutputDevice* SpriteCanvas::getOutputDevice()
    {
        return maDeviceHelper.getOutputDevice();
    }

    SurfaceSharedPtr const & SpriteCanvas::getBufferSurface()
    {
        return maDeviceHelper.getBufferSurface();
    }

    SurfaceSharedPtr const & SpriteCanvas::getWindowSurface()
    {
        return maDeviceHelper.getWindowSurface();
    }

    const ::basegfx::B2ISize& SpriteCanvas::getSizePixel()
    {
        return maDeviceHelper.getSizePixel();
    }

    void SpriteCanvas::setSizePixel( const ::basegfx::B2ISize& rSize )
    {
        maCanvasHelper.setSize( rSize );
        // re-set background surface, in case it needed recreation
        maCanvasHelper.setSurface( maDeviceHelper.getBufferSurface(),
                                   false );
    }

    void SpriteCanvas::flush()
    {
        maDeviceHelper.flush();
    }

    bool SpriteCanvas::repaint( const SurfaceSharedPtr&       pSurface,
                                const rendering::ViewState&   viewState,
                                const rendering::RenderState& renderState )
    {
        return maCanvasHelper.repaint( pSurface, viewState, renderState );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
