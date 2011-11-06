/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include "cairo_cachedbitmap.hxx"
#include "cairo_repainttarget.hxx"

#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>


using namespace ::cairo;
using namespace ::com::sun::star;

namespace cairocanvas
{
    CachedBitmap::CachedBitmap( const SurfaceSharedPtr&                     pSurface,
                                const rendering::ViewState&                 rUsedViewState,
                                const rendering::RenderState&               rUsedRenderState,
                                const uno::Reference< rendering::XCanvas >& rTarget ) :
        CachedPrimitiveBase( rUsedViewState, rTarget, true ),
        mpSurface( pSurface ),
        maRenderState( rUsedRenderState )
    {}

    void SAL_CALL CachedBitmap::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpSurface.reset();
        CachedPrimitiveBase::disposing();
    }

    ::sal_Int8 CachedBitmap::doRedraw( const rendering::ViewState&                  rNewState,
                                       const rendering::ViewState&                  /*rOldState*/,
                                       const uno::Reference< rendering::XCanvas >&  rTargetCanvas,
                                       bool                                         bSameViewTransform )
    {
        ENSURE_OR_THROW( bSameViewTransform,
                          "CachedBitmap::doRedraw(): base called with changed view transform "
                          "(told otherwise during construction)" );

        RepaintTarget* pTarget = dynamic_cast< RepaintTarget* >(rTargetCanvas.get());

        ENSURE_OR_THROW( pTarget,
                          "CachedBitmap::redraw(): cannot cast target to RepaintTarget" );

        if( !pTarget->repaint( mpSurface,
                               rNewState,
                               maRenderState ) )
        {
            // target failed to repaint
            return rendering::RepaintResult::FAILED;
        }

        return rendering::RepaintResult::REDRAWN;
    }
}
