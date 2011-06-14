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
#include <canvas/canvastools.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapex.hxx>

#include <basegfx/tools/canvastools.hxx>

#include <algorithm>

#include "spritecanvas.hxx"
#include "windowoutdevholder.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Sequence< uno::Any >&                aArguments,
                                const uno::Reference< uno::XComponentContext >& rxContext ) :
        maArguments(aArguments),
        mxComponentContext( rxContext )
    {
    }

    void SpriteCanvas::initialize()
    {
        SolarMutexGuard aGuard;

        // #i64742# Only call initialize when not in probe mode
        if( maArguments.getLength() == 0 )
            return;

        OSL_TRACE( "SpriteCanvas created" );

        // add our own property to GraphicDevice
        maPropHelper.addProperties(
            ::canvas::PropertySetHelper::MakeMap
            ("UnsafeScrolling",
             boost::bind(&SpriteCanvasHelper::isUnsafeScrolling,
                         boost::ref(maCanvasHelper)),
             boost::bind(&SpriteCanvasHelper::enableUnsafeScrolling,
                         boost::ref(maCanvasHelper),
                         _1))
            ("SpriteBounds",
             boost::bind(&SpriteCanvasHelper::isSpriteBounds,
                         boost::ref(maCanvasHelper)),
             boost::bind(&SpriteCanvasHelper::enableSpriteBounds,
                         boost::ref(maCanvasHelper),
                         _1)));

        VERBOSE_TRACE( "VCLSpriteCanvas::initialize called" );

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
        OSL_TRACE( "SpriteCanvas destroyed" );
    }


    void SAL_CALL SpriteCanvas::disposing()
    {
        SolarMutexGuard aGuard;

        mxComponentContext.clear();

        // forward to parent
        SpriteCanvasBaseT::disposing();
    }

    ::sal_Bool SAL_CALL SpriteCanvas::showBuffer( ::sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        return updateScreen( bUpdateAll );
    }

    ::sal_Bool SAL_CALL SpriteCanvas::switchBuffer( ::sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        return updateScreen( bUpdateAll );
    }

    sal_Bool SAL_CALL SpriteCanvas::updateScreen( sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        SolarMutexGuard aGuard;

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return !mbIsVisible ? false : maCanvasHelper.updateScreen(bUpdateAll,
                                                                  mbSurfaceDirty);
    }

    ::rtl::OUString SAL_CALL SpriteCanvas::getServiceName(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SPRITECANVAS_SERVICE_NAME ) );
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
