/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include <RenderState.hxx>
#include <vcl/bitmap.hxx>
#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>
#include <canvastools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <sal/log.hxx>
#include "cachedprimitivebase.hxx"
#include "bitmapaction.hxx"
#include "outdevstate.hxx"
#include "mtftools.hxx"
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <canvas.hxx>

using namespace ::com::sun::star;

namespace cppcanvas
{
        namespace
        {

            class BitmapAction : public CachedPrimitiveBase
            {
            public:
                BitmapAction( const ::Bitmap&,
                              const ::basegfx::B2DPoint& rDstPoint,
                              const OutDevState& );
                BitmapAction( const ::Bitmap&,
                              const ::basegfx::B2DPoint&  rDstPoint,
                              const ::basegfx::B2DVector& rDstSize,
                              const OutDevState& );

                virtual sal_Int32 getActionCount() const override;

            private:
                using Action::render;
                virtual bool renderPrimitive( vclcanvas::Canvas& rCanvas,
                                              const vclcanvas::ViewState& rViewState,
                                              rtl::Reference< vclcanvas::CachedBitmap >& rCachedPrimitive,
                                              const ::basegfx::B2DHomMatrix&                 rTransformation ) const override;

                Bitmap                                                  maBitmap;
                vclcanvas::RenderState                                  maState;
            };


            BitmapAction::BitmapAction( const ::Bitmap&          rBmp,
                                        const ::basegfx::B2DPoint& rDstPoint,
                                        const OutDevState&         rState ) :
                CachedPrimitiveBase( true ),
                maBitmap( rBmp )
            {
                cppcanvastools::initRenderState(maState,rState);

                // Setup transformation such that the next render call is
                // moved rPoint away.
                const basegfx::B2DHomMatrix aLocalTransformation(basegfx::utils::createTranslateB2DHomMatrix(rDstPoint));
                maState.AffineTransform *= aLocalTransformation;

                // correct clip (which is relative to original transform)
                cppcanvastools::modifyClip( maState,
                                   rState,
                                   rDstPoint,
                                   nullptr,
                                   nullptr );
            }

            BitmapAction::BitmapAction( const ::Bitmap&           rBmp,
                                        const ::basegfx::B2DPoint&  rDstPoint,
                                        const ::basegfx::B2DVector& rDstSize,
                                        const OutDevState&          rState      ) :
                CachedPrimitiveBase( true ),
                maBitmap( rBmp )
            {
                cppcanvastools::initRenderState(maState,rState);

                // Setup transformation such that the next render call is
                // moved rPoint away, and scaled according to the ratio
                // given by src and dst size.
                const ::Size aBmpSize( rBmp.GetSizePixel() );

                const ::basegfx::B2DVector aScale( rDstSize.getX() / aBmpSize.Width(),
                                                   rDstSize.getY() / aBmpSize.Height() );
                const basegfx::B2DHomMatrix aLocalTransformation(basegfx::utils::createScaleTranslateB2DHomMatrix(
                    aScale, rDstPoint));
                maState.AffineTransform *= aLocalTransformation;

                // correct clip (which is relative to original transform)
                cppcanvastools::modifyClip( maState,
                                   rState,
                                   rDstPoint,
                                   &aScale,
                                   nullptr );
            }

            bool BitmapAction::renderPrimitive( vclcanvas::Canvas& rCanvas,
                                                const vclcanvas::ViewState& rViewState,
                                                rtl::Reference< vclcanvas::CachedBitmap >& rCachedPrimitive,
                                                const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::BitmapAction::renderPrimitive()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::BitmapAction: 0x" << std::hex << this );

                vclcanvas::RenderState aLocalState( maState );
                aLocalState.AffineTransform = rTransformation * aLocalState.AffineTransform;

                rCachedPrimitive = rCanvas.drawBitmap( maBitmap,
                                                     rViewState,
                                                     aLocalState );

                return true;
            }

            sal_Int32 BitmapAction::getActionCount() const
            {
                return 1;
            }
        }

        std::shared_ptr<Action> BitmapActionFactory::createBitmapAction( const ::Bitmap&          rBmp,
                                                                 const ::basegfx::B2DPoint& rDstPoint,
                                                                 const OutDevState&         rState )
        {
            return std::make_shared<BitmapAction>(rBmp, rDstPoint, rState );
        }

        std::shared_ptr<Action> BitmapActionFactory::createBitmapAction( const ::Bitmap&           rBmp,
                                                                 const ::basegfx::B2DPoint&  rDstPoint,
                                                                 const ::basegfx::B2DVector& rDstSize,
                                                                 const OutDevState&          rState )
        {
            return std::make_shared<BitmapAction>(rBmp,
                                                  rDstPoint,
                                                  rDstSize,
                                                  rState );
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
