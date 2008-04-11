/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bitmapaction.cxx,v $
 * $Revision: 1.12 $
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
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/RepaintResult.hpp>
#include <com/sun/star/rendering/XCachedPrimitive.hpp>

#include <vcl/bitmapex.hxx>
#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>

#include <canvas/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/point/b2dpoint.hxx>
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
                              const ::basegfx::B2DPoint& rDstPoint,
                              const CanvasSharedPtr&,
                              const OutDevState& );
                BitmapAction( const ::BitmapEx&,
                              const ::basegfx::B2DPoint&  rDstPoint,
                              const ::basegfx::B2DVector& rDstSize,
                              const CanvasSharedPtr&,
                              const OutDevState& );

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const;

                virtual sal_Int32 getActionCount() const;

            private:
                using Action::render;
                virtual bool render( uno::Reference< rendering::XCachedPrimitive >& rCachedPrimitive,
                                     const ::basegfx::B2DHomMatrix&                 rTransformation ) const;

                uno::Reference< rendering::XBitmap >                    mxBitmap;
                CanvasSharedPtr                                         mpCanvas;
                rendering::RenderState                                  maState;
            };


            BitmapAction::BitmapAction( const ::BitmapEx&          rBmpEx,
                                        const ::basegfx::B2DPoint& rDstPoint,
                                        const CanvasSharedPtr&     rCanvas,
                                        const OutDevState&         rState ) :
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
                aLocalTransformation.translate( rDstPoint.getX(),
                                                rDstPoint.getY() );
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

            BitmapAction::BitmapAction( const ::BitmapEx&           rBmpEx,
                                        const ::basegfx::B2DPoint&  rDstPoint,
                                        const ::basegfx::B2DVector& rDstSize,
                                        const CanvasSharedPtr&      rCanvas,
                                        const OutDevState&          rState      ) :
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

                const ::basegfx::B2DVector aScale( rDstSize.getX() / aBmpSize.Width(),
                                                   rDstSize.getY() / aBmpSize.Height() );
                aLocalTransformation.scale( aScale.getX(), aScale.getY() );
                aLocalTransformation.translate( rDstPoint.getX(),
                                                rDstPoint.getY() );
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

        ActionSharedPtr BitmapActionFactory::createBitmapAction( const ::BitmapEx&          rBmpEx,
                                                                 const ::basegfx::B2DPoint& rDstPoint,
                                                                 const CanvasSharedPtr&     rCanvas,
                                                                 const OutDevState&         rState )
        {
            return ActionSharedPtr( new BitmapAction(rBmpEx,
                                                     rDstPoint,
                                                     rCanvas,
                                                     rState ) );
        }

        ActionSharedPtr BitmapActionFactory::createBitmapAction( const ::BitmapEx&           rBmpEx,
                                                                 const ::basegfx::B2DPoint&  rDstPoint,
                                                                 const ::basegfx::B2DVector& rDstSize,
                                                                 const CanvasSharedPtr&      rCanvas,
                                                                 const OutDevState&          rState )
        {
            return ActionSharedPtr( new BitmapAction(rBmpEx,
                                                     rDstPoint,
                                                     rDstSize,
                                                     rCanvas,
                                                     rState ) );
        }
    }
}
