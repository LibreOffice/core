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
#include <canvas/canvastools.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapex.hxx>

#include <basegfx/tools/canvastools.hxx>

#include <algorithm>

#include "canvas.hxx"
#include "windowoutdevholder.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    namespace
    {
        class OutDevHolder : public OutDevProvider,
            private ::boost::noncopyable
        {
        public:
            explicit OutDevHolder( OutputDevice& rOutDev ) :
                mrOutDev(rOutDev)
            {}

        private:
            virtual OutputDevice&       getOutDev() { return mrOutDev; }
            virtual const OutputDevice& getOutDev() const { return mrOutDev; }

            // TODO(Q2): Lifetime issue. This _only_ works reliably,
            // if disposing the Canvas correctly disposes all
            // entities which hold this pointer.
            OutputDevice& mrOutDev;
        };
    }

    Canvas::Canvas( const uno::Sequence< uno::Any >&                aArguments,
                    const uno::Reference< uno::XComponentContext >& rxContext ) :
        maArguments(aArguments),
        mxComponentContext( rxContext )
    {
    }

    void Canvas::initialize()
    {
        // #i64742# Only perform initialization when not in probe mode
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
        SolarMutexGuard aGuard;

        VERBOSE_TRACE( "VCLCanvas::initialize called" );

        ENSURE_ARG_OR_THROW( maArguments.getLength() >= 6 &&
                             maArguments[0].getValueTypeClass() == uno::TypeClass_HYPER,
                             "Canvas::initialize: wrong number of arguments, or wrong types" );

        sal_Int64 nPtr = 0;
        maArguments[0] >>= nPtr;

        OutputDevice* pOutDev = reinterpret_cast<OutputDevice*>(nPtr);
        if( !pOutDev )
            throw lang::NoSupportException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                     "Passed OutDev invalid!")),
                NULL);

        OutDevProviderSharedPtr pOutdevProvider( new OutDevHolder(*pOutDev) );

        // setup helper
        maDeviceHelper.init( pOutdevProvider );
        maCanvasHelper.init( *this,
                             pOutdevProvider,
                             true,   // OutDev state preservation
                             false ); // no alpha on surface

        maArguments.realloc(0);
    }

    Canvas::~Canvas()
    {
        OSL_TRACE( "Canvas destroyed" );
    }

    void Canvas::disposeThis()
    {
        SolarMutexGuard aGuard;

        mxComponentContext.clear();

        // forward to parent
        CanvasBaseT::disposeThis();
    }

    ::rtl::OUString SAL_CALL Canvas::getServiceName(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CANVAS_SERVICE_NAME ) );
    }

    bool Canvas::repaint( const GraphicObjectSharedPtr& rGrf,
                          const rendering::ViewState&   viewState,
                          const rendering::RenderState& renderState,
                          const ::Point&                rPt,
                          const ::Size&                 rSz,
                          const GraphicAttr&            rAttr ) const
    {
        SolarMutexGuard aGuard;

        return maCanvasHelper.repaint( rGrf, viewState, renderState, rPt, rSz, rAttr );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
