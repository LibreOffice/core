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

#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>

#include "spritecanvas.hxx"
#include "outdevholder.hxx"
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
        if( !maArguments.hasElements() )
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

        ENSURE_ARG_OR_THROW( maArguments.hasElements(),
                             "VCLSpriteCanvas::initialize: wrong number of arguments" );

        /* maArguments:
           0: ptr to creating instance (Window or VirtualDevice)
           1: current bounds of creating instance
           2: bool, denoting always on top state for Window (always false for VirtualDevice)
           3: XWindow for creating Window (or empty for VirtualDevice)
           4: SystemGraphicsData as a streamed Any
         */
        ENSURE_ARG_OR_THROW( maArguments.getLength() >= 4 &&
                             maArguments[0].getValueTypeClass() == uno::TypeClass_HYPER &&
                             maArguments[3].getValueTypeClass() == uno::TypeClass_INTERFACE,
                             "VCLSpriteCanvas::initialize: wrong number of arguments, or wrong types" );

        sal_Int64 nPtr = 0;
        maArguments[0] >>= nPtr;

        OutputDevice* pOutDev = reinterpret_cast<OutputDevice*>(nPtr);
        if( !pOutDev )
            throw lang::NoSupportException(u"Passed OutDev invalid!"_ustr, nullptr);

        uno::Reference< awt::XWindow > xParentWindow;
        maArguments[3] >>= xParentWindow;

        OutDevProviderSharedPtr pOutDevProvider;
        if( xParentWindow.is())
            pOutDevProvider = std::make_shared<WindowOutDevHolder>(xParentWindow);
        else
            pOutDevProvider = std::make_shared<OutDevHolder>(*pOutDev);

        // setup helper
        maDeviceHelper.init( pOutDevProvider );
        setWindow( xParentWindow.is()
            ? uno::Reference<awt::XWindow2>(xParentWindow, uno::UNO_QUERY_THROW)
            : uno::Reference<awt::XWindow2>());
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
        return u"com.sun.star.rendering.SpriteCanvas.VCL"_ustr;
    }

    // XServiceInfo
    css::uno::Sequence<OUString> SpriteCanvas::getSupportedServiceNames()
    {
        return { SpriteCanvas::getServiceName() };
    }
    OUString SpriteCanvas::getImplementationName()
    {
        return u"com.sun.star.comp.rendering.SpriteCanvas.VCL"_ustr;
    }
    sal_Bool SpriteCanvas::supportsService(const OUString& sServiceName)
    {
        return cppu::supportsService(this, sServiceName);
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_rendering_SpriteCanvas_VCL_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
{
    rtl::Reference<vclcanvas::SpriteCanvas> p = new vclcanvas::SpriteCanvas(args, context);
    p->initialize();
    return cppu::acquire(p.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
