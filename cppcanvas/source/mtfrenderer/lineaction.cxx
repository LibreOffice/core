/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lineaction.cxx,v $
 * $Revision: 1.11 $
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

#include <lineaction.hxx>
#include <outdevstate.hxx>

#include <rtl/logfile.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>

#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <canvas/canvastools.hxx>

#include <boost/utility.hpp>

#include <cppcanvas/canvas.hxx>

#include <mtftools.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        namespace
        {
            class LineAction : public Action, private ::boost::noncopyable
            {
            public:
                LineAction( const ::basegfx::B2DPoint&,
                            const ::basegfx::B2DPoint&,
                            const CanvasSharedPtr&,
                            const OutDevState& );

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const;

                virtual sal_Int32 getActionCount() const;

            private:
                ::basegfx::B2DPoint     maStartPoint;
                ::basegfx::B2DPoint     maEndPoint;
                CanvasSharedPtr         mpCanvas;
                rendering::RenderState  maState;
            };

            LineAction::LineAction( const ::basegfx::B2DPoint& rStartPoint,
                                    const ::basegfx::B2DPoint& rEndPoint,
                                    const CanvasSharedPtr&     rCanvas,
                                    const OutDevState&         rState ) :
                maStartPoint( rStartPoint ),
                maEndPoint( rEndPoint ),
                mpCanvas( rCanvas ),
                maState()
            {
                tools::initRenderState(maState,rState);
                maState.DeviceColor = rState.lineColor;
            }

            bool LineAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::LineAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::LineAction: 0x%X", this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                mpCanvas->getUNOCanvas()->drawLine( ::basegfx::unotools::point2DFromB2DPoint(maStartPoint),
                                                    ::basegfx::unotools::point2DFromB2DPoint(maEndPoint),
                                                    mpCanvas->getViewState(),
                                                    aLocalState );

                return true;
            }

            bool LineAction::render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const
            {
                // line only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return render( rTransformation );
            }

            ::basegfx::B2DRange LineAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return tools::calcDevicePixelBounds( ::basegfx::B2DRange( maStartPoint,
                                                                          maEndPoint ),
                                                     mpCanvas->getViewState(),
                                                     aLocalState );
            }

            ::basegfx::B2DRange LineAction::getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const
            {
                // line only contains a single action, empty bounds
                // if subset requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return ::basegfx::B2DRange();

                return getBounds( rTransformation );
            }

            sal_Int32 LineAction::getActionCount() const
            {
                return 1;
            }
        }

        ActionSharedPtr LineActionFactory::createLineAction( const ::basegfx::B2DPoint& rStartPoint,
                                                             const ::basegfx::B2DPoint& rEndPoint,
                                                             const CanvasSharedPtr&     rCanvas,
                                                             const OutDevState&         rState  )
        {
            return ActionSharedPtr( new LineAction( rStartPoint,
                                                    rEndPoint,
                                                    rCanvas,
                                                    rState) );
        }

    }
}
