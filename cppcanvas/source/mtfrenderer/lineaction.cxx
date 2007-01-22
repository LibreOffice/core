/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lineaction.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-22 11:49:54 $
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
