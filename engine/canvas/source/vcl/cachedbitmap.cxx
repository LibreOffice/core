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

#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

#include <cachedbitmap.hxx>
#include <canvas.hxx>


using namespace ::com::sun::star;

namespace vclcanvas
{
    CachedBitmap::CachedBitmap( GraphicObjectSharedPtr                      xGraphicObject,
                                const ::Point&                              rPoint,
                                const ::Size&                               rSize,
                                const GraphicAttr&                          rAttr,
                                const vclcanvas::ViewState&                 rUsedViewState,
                                vclcanvas::RenderState                      aUsedRenderState,
                                vclcanvas::Canvas& rTarget ) :
        maUsedViewState( rUsedViewState ),
        mrTarget( rTarget ),
        mpGraphicObject(std::move( xGraphicObject )),
        maRenderState(std::move(aUsedRenderState)),
        maPoint( rPoint ),
        maSize( rSize ),
        maAttributes( rAttr )
    {
    }

    sal_Int8 CachedBitmap::redraw( const vclcanvas::ViewState& aState )
    {
        const bool bSameViewTransform( maUsedViewState.AffineTransform == aState.AffineTransform );

        if( !bSameViewTransform )
        {
            // differing transformations, don't try to draft the
            // output, just plain fail here.
            return rendering::RepaintResult::FAILED;
        }

        if( !mrTarget.repaint( mpGraphicObject,
                               aState,
                               maRenderState,
                               maPoint,
                               maSize,
                               maAttributes ) )
        {
            // target failed to repaint
            return rendering::RepaintResult::FAILED;
        }

        return rendering::RepaintResult::REDRAWN;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
