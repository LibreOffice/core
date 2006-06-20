/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedbitmap.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:19:35 $
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
                                const uno::Reference< rendering::XCanvas >& rTarget ) :
        CachedPrimitiveBase( rUsedViewState, rTarget, true ),
        mpGraphicObject( rGraphicObject ),
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
        (void)rNewState;
        (void)rOldState;

        ENSURE_AND_THROW( bSameViewTransform,
                          "CachedBitmap::doRedraw(): base called with changed view transform "
                          "(told otherwise during construction)" );

        RepaintTarget* pTarget = dynamic_cast< RepaintTarget* >(rTargetCanvas.get());

        ENSURE_AND_THROW( pTarget,
                          "CachedBitmap::redraw(): cannot cast target to RepaintTarget" );

        if( !pTarget->repaint( mpGraphicObject,
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
