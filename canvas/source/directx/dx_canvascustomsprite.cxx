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

#include <cctype>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/math.hxx>
#include <tools/diagnose_ex.h>

#include <canvas/canvastools.hxx>

#include "dx_canvascustomsprite.hxx"
#include "dx_impltools.hxx"
#include "dx_spritecanvas.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    CanvasCustomSprite::CanvasCustomSprite( const css::geometry::RealSize2D&   rSpriteSize,
                                            const SpriteCanvasRef&                          rRefDevice,
                                            const IDXRenderModuleSharedPtr&                 rRenderModule,
                                            const ::canvas::ISurfaceProxyManagerSharedPtr&  rSurfaceProxy,
                                            bool                                            bShowSpriteBounds ) :
        mpSpriteCanvas( rRefDevice ),
        mpSurface()
    {
        ENSURE_OR_THROW( rRefDevice.get(),
                         "CanvasCustomSprite::CanvasCustomSprite(): Invalid sprite canvas" );

        mpSurface.reset(
            new DXSurfaceBitmap(
                ::basegfx::B2IVector(
                    ::canvas::tools::roundUp( rSpriteSize.Width ),
                    ::canvas::tools::roundUp( rSpriteSize.Height )),
                rSurfaceProxy,
                rRenderModule,
                true));

        maCanvasHelper.setDevice( *rRefDevice.get() );
        maCanvasHelper.setTarget( mpSurface );

        maSpriteHelper.init( rSpriteSize,
                             rRefDevice,
                             rRenderModule,
                             mpSurface,
                             bShowSpriteBounds );

        // clear sprite to 100% transparent
        maCanvasHelper.clear();
    }

    void CanvasCustomSprite::disposeThis()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpSurface.reset();
        mpSpriteCanvas.clear();

        // forward to parent
        CanvasCustomSpriteBaseT::disposeThis();
    }

    OUString SAL_CALL CanvasCustomSprite::getImplementationName() throw( uno::RuntimeException )
    {
        return OUString( "DXCanvas.CanvasCustomSprite" );
    }

    sal_Bool SAL_CALL CanvasCustomSprite::supportsService( const OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL CanvasCustomSprite::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< OUString > aRet { "com.sun.star.rendering.CanvasCustomSprite" };

        return aRet;
    }

    void CanvasCustomSprite::redraw() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        maSpriteHelper.redraw( mbSurfaceDirty );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
