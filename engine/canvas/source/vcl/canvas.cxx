/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <canvas.hxx>

#include <com/sun/star/lang/NoSupportException.hpp>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/outdev.hxx>

#include "outdevholder.hxx"
#include <canvasbitmap.hxx>

using namespace ::com::sun::star;

namespace vclcanvas
{
    Canvas::Canvas( OutputDevice* pOutDev )
    {
        SolarMutexGuard aGuard;

        SAL_INFO("canvas.vcl", "vclcanvas::Canvas() called" );

        if( !pOutDev )
            throw lang::NoSupportException(u"Passed OutDev invalid!"_ustr, nullptr);

        OutDevProviderSharedPtr pOutdevProvider = std::make_shared<OutDevHolder>(*pOutDev);

        // setup helper
        maDeviceHelper.init( pOutdevProvider );
        maCanvasHelper.init( *this,
                             pOutdevProvider,
                             true,   // OutDev state preservation
                             false ); // no alpha on surface
    }

    Canvas::~Canvas()
    {
        SAL_INFO("canvas.vcl", "VCLCanvas destroyed" );
    }

    void Canvas::disposeThis()
    {
        SolarMutexGuard aGuard;

        // forward to parent
        CanvasBaseT::disposeThis();
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
