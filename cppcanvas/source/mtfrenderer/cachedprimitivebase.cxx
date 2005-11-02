/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedprimitivebase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:39:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
