/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachedbitmap.cxx,v $
 * $Revision: 1.8 $
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

#include "cachedbitmap.hxx"
#include "repainttarget.hxx"

#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>


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
        CachedPrimitiveBase( rUsedViewState, rTarget, true ),
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
        ENSURE_AND_THROW( bSameViewTransform,
                          "CachedBitmap::doRedraw(): base called with changed view transform "
                          "(told otherwise during construction)" );

        // TODO(P1): Could adapt to modified clips as well
        if( rNewState.Clip != rOldState.Clip )
            return rendering::RepaintResult::FAILED;

        RepaintTarget* pTarget = dynamic_cast< RepaintTarget* >(rTargetCanvas.get());

        ENSURE_AND_THROW( pTarget,
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
