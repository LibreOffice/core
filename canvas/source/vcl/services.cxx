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

#include <algorithm>

#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/servicedecl.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>

#include <canvas/canvastools.hxx>

#include "canvas.hxx"
#include "spritecanvas.hxx"


using namespace ::com::sun::star;

namespace sdecl = comphelper::service_decl;

namespace vclcanvas
{
    static uno::Reference<uno::XInterface> initCanvas( Canvas* pCanvas )
    {
        uno::Reference<uno::XInterface> xRet(static_cast<cppu::OWeakObject*>(pCanvas));
        pCanvas->initialize();
        return xRet;
    }

    sdecl::class_<Canvas, sdecl::with_args<true> > const serviceImpl1(&initCanvas);
    const sdecl::ServiceDecl vclCanvasDecl(
        serviceImpl1,
        CANVAS_IMPLEMENTATION_NAME,
        CANVAS_SERVICE_NAME );

    static uno::Reference<uno::XInterface> initSpriteCanvas( SpriteCanvas* pCanvas )
    {
        uno::Reference<uno::XInterface> xRet(static_cast<cppu::OWeakObject*>(pCanvas));
        pCanvas->initialize();
        return xRet;
    }

    sdecl::class_<SpriteCanvas, sdecl::with_args<true> > const serviceImpl2(&initSpriteCanvas);
    const sdecl::ServiceDecl vclSpriteCanvasDecl(
        serviceImpl2,
        SPRITECANVAS_IMPLEMENTATION_NAME,
        SPRITECANVAS_SERVICE_NAME );
}

// The C shared lib entry points
extern "C"
SAL_DLLPUBLIC_EXPORT void* SAL_CALL vclcanvas_component_getFactory( sal_Char const* pImplName,
                                         void*, void* )
{
    return sdecl::component_getFactoryHelper( pImplName, {&vclcanvas::vclCanvasDecl, &vclcanvas::vclSpriteCanvasDecl} );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
