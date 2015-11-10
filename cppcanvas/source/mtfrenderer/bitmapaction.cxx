/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
#include <boost/noncopyable.hpp>
#include "cachedprimitivebase.hxx"
#include "bitmapaction.hxx"
#include "outdevstate.hxx"
#include "mtftools.hxx"
#include <basegfx/matrix/b2dhommatrixtools.hxx>


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

                virtual bool renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                using Action::render;
                virtual bool renderPrimitive( uno::Reference< rendering::XCachedPrimitive >& rCachedPrimitive,
                                              const ::basegfx::B2DHomMatrix&                 rTransformation ) const override;

                uno::Reference< rendering::XBitmap >                    mxBitmap;
                CanvasSharedPtr                                         mpCanvas;
                rendering::RenderState                                  maState;
            };


            BitmapAction::BitmapAction( const ::BitmapEx&          rBmpEx,
                                        const ::basegfx::B2DPoint& rDstPoint,
                                        const CanvasSharedPtr&     rCanvas,
                                        const OutDevState&         rState ) :
                CachedPrimitiveBase( rCanvas, true ),
                mxBitmap( vcl::unotools::xBitmapFromBitmapEx( rCanvas->getUNOCanvas()->getDevice(),
                                                                rBmpEx ) ),
                mpCanvas( rCanvas ),
                maState()
            {
                tools::initRenderState(maState,rState);

                // Setup transformation such that the next render call is
                // moved rPoint away.
                const basegfx::B2DHomMatrix aLocalTransformation(basegfx::tools::createTranslateB2DHomMatrix(rDstPoint));
                ::canvas::tools::appendToRenderState( maState,
                                                      aLocalTransformation );

                // correct clip (which is relative to original transform)
                tools::modifyClip( maState,
                                   rState,
                                   rCanvas,
                                   rDstPoint,
                                   nullptr,
                                   nullptr );
            }

            BitmapAction::BitmapAction( const ::BitmapEx&           rBmpEx,
                                        const ::basegfx::B2DPoint&  rDstPoint,
                                        const ::basegfx::B2DVector& rDstSize,
                                        const CanvasSharedPtr&      rCanvas,
                                        const OutDevState&          rState      ) :
                CachedPrimitiveBase( rCanvas, true ),
                mxBitmap( vcl::unotools::xBitmapFromBitmapEx( rCanvas->getUNOCanvas()->getDevice(),
                                                                rBmpEx ) ),
                mpCanvas( rCanvas ),
                maState()
            {
                tools::initRenderState(maState,rState);

                // Setup transformation such that the next render call is
                // moved rPoint away, and scaled according to the ratio
                // given by src and dst size.
                const ::Size aBmpSize( rBmpEx.GetSizePixel() );

                const ::basegfx::B2DVector aScale( rDstSize.getX() / aBmpSize.Width(),
                                                   rDstSize.getY() / aBmpSize.Height() );
                const basegfx::B2DHomMatrix aLocalTransformation(basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aScale, rDstPoint));
                ::canvas::tools::appendToRenderState( maState, aLocalTransformation );

                // correct clip (which is relative to original transform)
                tools::modifyClip( maState,
                                   rState,
                                   rCanvas,
                                   rDstPoint,
                                   &aScale,
                                   nullptr );
            }

            bool BitmapAction::renderPrimitive( uno::Reference< rendering::XCachedPrimitive >& rCachedPrimitive,
                                                const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::BitmapAction::renderPrimitive()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::BitmapAction: 0x" << std::hex << this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                rCachedPrimitive = mpCanvas->getUNOCanvas()->drawBitmap( mxBitmap,
                                                                         mpCanvas->getViewState(),
                                                                         aLocalState );

                return true;
            }

            bool BitmapAction::renderSubset( const ::basegfx::B2DHomMatrix&   rTransformation,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
