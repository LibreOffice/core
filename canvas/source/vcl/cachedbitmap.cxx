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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <tools/diagnose_ex.h>

#include "cachedbitmap.hxx"
#include "repainttarget.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    CachedBitmap::CachedBitmap( const GraphicObjectSharedPtr&               rGraphicObject,
                                const ::Point&                              rPoint,
                                const ::Size&                               rSize,
                                const GraphicAttr&                          rAttr,
                                const rendering::ViewState&                 rUsedViewState,
                                const rendering::RenderState&               rUsedRenderState,
                                const uno::Reference< rendering::XCanvas >& rTarget ) :
        CachedPrimitiveBase( rUsedViewState, rTarget ),
        mpGraphicObject( rGraphicObject ),
        maRenderState(rUsedRenderState),
        maPoint( rPoint ),
        maSize( rSize ),
        maAttributes( rAttr )
    {
    }

    void SAL_CALL CachedBitmap::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpGraphicObject.reset();

        CachedPrimitiveBase::disposing();
    }

    ::sal_Int8 CachedBitmap::doRedraw( const rendering::ViewState&                  rNewState,
                                       const rendering::ViewState&                  rOldState,
                                       const uno::Reference< rendering::XCanvas >&  rTargetCanvas,
                                       bool                                         bSameViewTransform )
    {
        ENSURE_OR_THROW( bSameViewTransform,
                         "CachedBitmap::doRedraw(): base called with changed view transform "
                         "(told otherwise during construction)" );

        // TODO(P1): Could adapt to modified clips as well
        if( rNewState.Clip != rOldState.Clip )
            return rendering::RepaintResult::FAILED;

        RepaintTarget* pTarget = dynamic_cast< RepaintTarget* >(rTargetCanvas.get());

        ENSURE_OR_THROW( pTarget,
                          "CachedBitmap::redraw(): cannot cast target to RepaintTarget" );

        if( !pTarget->repaint( mpGraphicObject,
                               rNewState,
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
