/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bitmapaction.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:39:39 $
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
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/XCachedPrimitive.hpp>

#include <vcl/bitmapex.hxx>
#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>

#include <canvas/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <boost/utility.hpp>

#include "cachedprimitivebase.hxx"
#include "bitmapaction.hxx"
#include "outdevstate.hxx"
#include "mtftools.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        namespace
        {

            class BitmapAction : public CachedPrimitiveBase
            {
            public:
                BitmapAction( const ::BitmapEx&,
                              const ::Point&    rDstPoint,
                              const CanvasSharedPtr&,
                              const OutDevState& );
                BitmapAction( const ::BitmapEx&,
                              const ::Point&    rDstPoint,
                              const ::Size&     rDstSize,
                              const CanvasSharedPtr&,
                              const OutDevState& );

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const;

                virtual sal_Int32 getActionCount() const;

            private:
                virtual bool render( uno::Reference< rendering::XCachedPrimitive >& rCachedPrimitive,
                                     const ::basegfx::B2DHomMatrix&                 rTransformation ) const;

                uno::Reference< rendering::XBitmap >                    mxBitmap;
                CanvasSharedPtr                                         mpCanvas;
                rendering::RenderState                                  maState;
            };


            BitmapAction::BitmapAction( const ::BitmapEx&       rBmpEx,
                                        const ::Point&          rDstPoint,
                                        const CanvasSharedPtr&  rCanvas,
                                        const OutDevState&      rState ) :
                CachedPrimitiveBase( rCanvas, true ),
                mxBitmap( ::vcl::unotools::xBitmapFromBitmapEx( rCanvas->getUNOCanvas()->getDevice(),
                                                                rBmpEx ) ),
                mpCanvas( rCanvas ),
                maState()
            {
                tools::initRenderState(maState,rState);

                // Setup transformation such that the next render call is
                // moved rPoint away.
                ::basegfx::B2DHomMatrix aLocalTransformation;
                aLocalTransformation.translate( rDstPoint.X(),
                                                rDstPoint.Y() );
                ::canvas::tools::appendToRenderState( maState,
                                                      aLocalTransformation );

                // correct clip (which is relative to original transform)
                tools::modifyClip( maState,
                                   rState,
                                   rCanvas,
                                   rDstPoint,
                                   NULL,
                                   NULL );
            }

            BitmapAction::BitmapAction( const ::BitmapEx&       rBmpEx,
                                        const ::Point&          rDstPoint,
                                        const ::Size&           rDstSize,
                                        const CanvasSharedPtr&  rCanvas,
                                        const OutDevState&      rState      ) :
                CachedPrimitiveBase( rCanvas, true ),
                mxBitmap( ::vcl::unotools::xBitmapFromBitmapEx( rCanvas->getUNOCanvas()->getDevice(),
                                                                rBmpEx ) ),
                mpCanvas( rCanvas ),
                maState()
            {
                tools::initRenderState(maState,rState);

                // Setup transformation such that the next render call is
                // moved rPoint away, and scaled according to the ratio
                // given by src and dst size.
                const ::Size aBmpSize( rBmpEx.GetSizePixel() );
                ::basegfx::B2DHomMatrix aLocalTransformation;

                const ::basegfx::B2DSize aScale( static_cast<double>(rDstSize.Width()) / aBmpSize.Width(),
                                                 static_cast<double>(rDstSize.Height()) / aBmpSize.Height() );
                aLocalTransformation.scale( aScale.getX(), aScale.getY() );
                aLocalTransformation.translate( rDstPoint.X(),
                                                rDstPoint.Y() );
                ::canvas::tools::appendToRenderState( maState,
                                                      aLocalTransformation );

                // correct clip (which is relative to original transform)
                tools::modifyClip( maState,
                                   rState,
                                   rCanvas,
                                   rDstPoint,
                                   &aScale,
                                   NULL );
            }

            bool BitmapAction::render( uno::Reference< rendering::XCachedPrimitive >& rCachedPrimitive,
                                       const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::BitmapAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::BitmapAction: 0x%X", this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                rCachedPrimitive = mpCanvas->getUNOCanvas()->drawBitmap( mxBitmap,
                                                                         mpCanvas->getViewState(),
                                                                         aLocalState );

                return true;
            }

            bool BitmapAction::render( const ::basegfx::B2DHomMatrix&   rTransformation,
                                       const Subset&                    rSubset ) const
            {
                // bitmap only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return CachedPrimitiveBase::render( rTransformation );
            }

            ::basegfx::B2DRange BitmapAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                const geometry::IntegerSize2D aSize( mxBitmap->getSize() );

                return tools::calcDevicePixelBounds( ::basegfx::B2DRange( 0,0,
                                                                          aSize.Width,
                                                                          aSize.Height ),
                                                     mpCanvas->getViewState(),
                                                     aLocalState );
            }

            ::basegfx::B2DRange BitmapAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation,
                                                         const Subset&                  rSubset ) const
            {
                // bitmap only contains a single action, empty bounds
                // if subset requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return ::basegfx::B2DRange();

                return getBounds( rTransformation );
            }

            sal_Int32 BitmapAction::getActionCount() const
            {
                return 1;
            }
        }

        ActionSharedPtr BitmapActionFactory::createBitmapAction( const ::BitmapEx&      rBmpEx,
                                                                 const ::Point&         rDstPoint,
                                                                 const CanvasSharedPtr& rCanvas,
                                                                 const OutDevState&     rState )
        {
            return ActionSharedPtr( new BitmapAction(rBmpEx,
                                                     rDstPoint,
                                                     rCanvas,
                                                     rState ) );
        }

        ActionSharedPtr BitmapActionFactory::createBitmapAction( const ::BitmapEx&      rBmpEx,
                                                                 const ::Point&         rDstPoint,
                                                                 const ::Size&          rDstSize,
                                                                 const CanvasSharedPtr& rCanvas,
                                                                 const OutDevState&     rState )
        {
            return ActionSharedPtr( new BitmapAction(rBmpEx,
                                                     rDstPoint,
                                                     rDstSize,
                                                     rCanvas,
                                                     rState ) );
        }
    }
}
