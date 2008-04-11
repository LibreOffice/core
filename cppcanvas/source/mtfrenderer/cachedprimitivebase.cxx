/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachedprimitivebase.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_cppcanvas.hxx"

#include <rtl/logfile.hxx>

#include <com/sun/star/rendering/RepaintResult.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <canvas/canvastools.hxx>
#include <cppcanvas/canvas.hxx>

#include "cachedprimitivebase.hxx"

using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        CachedPrimitiveBase::CachedPrimitiveBase( const CanvasSharedPtr& rCanvas,
                                                  bool                   bOnlyRedrawWithSameTransform ) :
            mpCanvas( rCanvas ),
            mxCachedPrimitive(),
            maLastTransformation(),
            mbOnlyRedrawWithSameTransform( bOnlyRedrawWithSameTransform )
        {
            // TODO(F2): also store last view transform, and refuse to
            // redraw if changed.
        }

        bool CachedPrimitiveBase::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
        {
            RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::CachedPrimitiveBase::render()" );
            RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::CachedPrimitiveBase: 0x%X", this );

            const rendering::ViewState& rViewState( mpCanvas->getViewState() );
            ::basegfx::B2DHomMatrix     aTotalTransform;

            ::canvas::tools::getViewStateTransform( aTotalTransform,
                                                    rViewState );
            aTotalTransform *= rTransformation;

            // can we use the cached primitive? For that, it must be
            // present in the first place, and, if
            // mbOnlyRedrawWithSameTransform is true, the overall
            // transformation must be the same.
            if( mxCachedPrimitive.is() &&
                (!mbOnlyRedrawWithSameTransform ||
                 maLastTransformation == aTotalTransform) )
            {
                if( mxCachedPrimitive->redraw( rViewState ) ==
                    rendering::RepaintResult::REDRAWN )
                {
                    // cached repaint succeeded, done.
                    return true;
                }
            }

            maLastTransformation = aTotalTransform;

            // delegate rendering to derived classes
            return render( mxCachedPrimitive,
                           rTransformation );
        }
    }
}
