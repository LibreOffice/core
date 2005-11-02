/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pointaction.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:41:31 $
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
#include <com/sun/star/rendering/XCanvas.hpp>

#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <canvas/canvastools.hxx>

#include <boost/utility.hpp>

#include "pointaction.hxx"
#include "outdevstate.hxx"
#include "cppcanvas/canvas.hxx"
#include "mtftools.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        namespace
        {
            class PointAction : public Action, private ::boost::noncopyable
            {
            public:
                PointAction( const ::Point&,
                             const CanvasSharedPtr&,
                             const OutDevState& );
                PointAction( const ::Point&,
                             const CanvasSharedPtr&,
                             const OutDevState&,
                             const ::Color&     );

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const;

                virtual sal_Int32 getActionCount() const;

            private:
                // default: disabled copy/assignment
                PointAction(const PointAction&);
                PointAction& operator = ( const PointAction& );

                ::Point                                     maPoint;
                CanvasSharedPtr                             mpCanvas;
                ::com::sun::star::rendering::RenderState    maState;
            };

            PointAction::PointAction( const ::Point&            rPoint,
                                      const CanvasSharedPtr&    rCanvas,
                                      const OutDevState&        rState ) :
                maPoint( rPoint ),
                mpCanvas( rCanvas ),
                maState()
            {
                tools::initRenderState(maState,rState);
                maState.DeviceColor = rState.lineColor;
            }

            PointAction::PointAction( const ::Point&            rPoint,
                                      const CanvasSharedPtr&    rCanvas,
                                      const OutDevState&        rState,
                                      const ::Color&            rAltColor ) :
                maPoint( rPoint ),
                mpCanvas( rCanvas ),
                maState()
            {
                tools::initRenderState(maState,rState);
                maState.DeviceColor = ::vcl::unotools::colorToDoubleSequence( rCanvas->getUNOCanvas()->getDevice(),
                                                                              rAltColor );
            }

            bool PointAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::PointAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::PointAction: 0x%X", this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                mpCanvas->getUNOCanvas()->drawPoint( ::vcl::unotools::point2DFromPoint(maPoint),
                                                     mpCanvas->getViewState(),
                                                     aLocalState );

                return true;
            }

            bool PointAction::render( const ::basegfx::B2DHomMatrix&    rTransformation,
                                      const Subset&                     rSubset ) const
            {
                // point only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return render( rTransformation );
            }

            ::basegfx::B2DRange PointAction::getBounds( const ::basegfx::B2DHomMatrix&  rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return tools::calcDevicePixelBounds( ::basegfx::B2DRange( maPoint.X()-1,
                                                                          maPoint.Y()-1,
                                                                          maPoint.X()+1,
                                                                          maPoint.Y()+1 ),
                                                     mpCanvas->getViewState(),
                                                     aLocalState );
            }

            ::basegfx::B2DRange PointAction::getBounds( const ::basegfx::B2DHomMatrix&  rTransformation,
                                                        const Subset&                   rSubset ) const
            {
                // point only contains a single action, empty bounds
                // if subset requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return ::basegfx::B2DRange();

                return getBounds( rTransformation );
            }

            sal_Int32 PointAction::getActionCount() const
            {
                return 1;
            }
        }

        ActionSharedPtr PointActionFactory::createPointAction( const ::Point&           rPoint,
                                                               const CanvasSharedPtr&   rCanvas,
                                                               const OutDevState&       rState )
        {
            return ActionSharedPtr( new PointAction( rPoint, rCanvas, rState ) );
        }

        ActionSharedPtr PointActionFactory::createPointAction( const ::Point&           rPoint,
                                                               const CanvasSharedPtr&   rCanvas,
                                                               const OutDevState&       rState,
                                                               const ::Color&           rColor  )
        {
            return ActionSharedPtr( new PointAction( rPoint, rCanvas, rState, rColor ) );
        }
    }
}
