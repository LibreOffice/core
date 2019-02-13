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

#include <algorithm>

#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <tools/diagnose_ex.h>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>

#include <canvas/canvastools.hxx>

#include "spritecanvas.hxx"
#include "windowoutdevholder.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Sequence< uno::Any >&                aArguments,
                                const uno::Reference< uno::XComponentContext >& /*rxContext*/ ) :
        maArguments(aArguments)
    {
    }

    void SpriteCanvas::initialize()
    {
        SolarMutexGuard aGuard;

        // #i64742# Only call initialize when not in probe mode
        if( maArguments.getLength() == 0 )
            return;

        SAL_INFO("canvas.vcl", "SpriteCanvas created" );

        // add our own property to GraphicDevice
        maPropHelper.addProperties(
            ::canvas::PropertySetHelper::MakeMap
            ("UnsafeScrolling",
             [this]() { return this->maCanvasHelper.isUnsafeScrolling(); },
             [this](css::uno::Any const& aAny) mutable { this->maCanvasHelper.enableUnsafeScrolling(aAny); } )
            ("SpriteBounds",
             [this]() { return this->maCanvasHelper.isSpriteBounds(); },
             [this](css::uno::Any const& aAny) mutable { this->maCanvasHelper.enableSpriteBounds(aAny); } ));

        SAL_INFO("canvas.vcl", "VCLSpriteCanvas::initialize called" );

        ENSURE_ARG_OR_THROW( maArguments.getLength() >= 1,
                             "VCLSpriteCanvas::initialize: wrong number of arguments" );

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
                             "VCLSpriteCanvas::initialize: wrong number of arguments, or wrong types" );

        uno::Reference< awt::XWindow > xParentWindow;
        maArguments[4] >>= xParentWindow;

        OutDevProviderSharedPtr pOutDev( new WindowOutDevHolder(xParentWindow) );

        // setup helper
        maDeviceHelper.init( pOutDev );
        setWindow(uno::Reference<awt::XWindow2>(xParentWindow, uno::UNO_QUERY_THROW));
        maCanvasHelper.init( maDeviceHelper.getBackBuffer(),
                             *this,
                             maRedrawManager,
                             false,   // no OutDev state preservation
                             false ); // no alpha on surface

        maArguments.realloc(0);
    }

    SpriteCanvas::~SpriteCanvas()
    {
        SAL_INFO("canvas.vcl", "SpriteCanvas destroyed" );
    }


    void SpriteCanvas::disposeThis()
    {
        SolarMutexGuard aGuard;

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
        SolarMutexGuard aGuard;

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return mbIsVisible && maCanvasHelper.updateScreen(bUpdateAll,
                                                                  mbSurfaceDirty);
    }

    OUString SAL_CALL SpriteCanvas::getServiceName(  )
    {
        return OUString( SPRITECANVAS_SERVICE_NAME );
    }

    bool SpriteCanvas::repaint( const GraphicObjectSharedPtr&   rGrf,
                                const rendering::ViewState&     viewState,
                                const rendering::RenderState&   renderState,
                                const ::Point&                  rPt,
                                const ::Size&                   rSz,
                                const GraphicAttr&              rAttr ) const
    {
        SolarMutexGuard aGuard;

        return maCanvasHelper.repaint( rGrf, viewState, renderState, rPt, rSz, rAttr );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
