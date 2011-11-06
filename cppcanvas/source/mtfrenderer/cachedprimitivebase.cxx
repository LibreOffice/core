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
