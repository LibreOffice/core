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


#include <sal/types.h>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <RenderState.hxx>
#include <canvastools.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include "cachedprimitivebase.hxx"
#include "polypolyaction.hxx"
#include "outdevstate.hxx"
#include <utility>
#include "mtftools.hxx"
#include <XCanvas.hxx>

using namespace ::com::sun::star;

namespace cppcanvas
{
        namespace
        {
            class PolyPolyAction : public CachedPrimitiveBase
            {
            public:
                PolyPolyAction( const ::basegfx::B2DPolyPolygon&,
                                const OutDevState&,
                                bool bFill,
                                bool bStroke );
                PolyPolyAction( const ::basegfx::B2DPolyPolygon&,
                                const OutDevState&,
                                bool bFill,
                                bool bStroke,
                                int nTransparency );

                virtual bool renderSubset( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                           const vclcanvas::ViewState& rViewState,
                                           const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                using Action::render;
                virtual bool renderPrimitive( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                              const vclcanvas::ViewState& rViewState,
                                              uno::Reference< vclcanvas::XCachedPrimitive >& rCachedPrimitive,
                                              const ::basegfx::B2DHomMatrix&                 rTransformation ) const override;

                const uno::Reference< rendering::XPolyPolygon2D >   mxPolyPoly;

                // stroke color is now implicit: the maState.DeviceColor member
                vclcanvas::RenderState                              maState;

                cpo::uno::Sequence< double >                             maFillColor;
            };

            PolyPolyAction::PolyPolyAction( const ::basegfx::B2DPolyPolygon&    rPolyPoly,
                                            const OutDevState&                  rState,
                                            bool                                bFill,
                                            bool                                bStroke ) :
                CachedPrimitiveBase( false ),
                mxPolyPoly( ::canvastools::xPolyPolygonFromB2DPolyPolygon( rPolyPoly) )
            {
                cppcanvastools::initRenderState(maState,rState);

                if( bFill )
                    maFillColor = rState.fillColor;

                if( bStroke )
                    maState.DeviceColor = rState.lineColor;
            }

            PolyPolyAction::PolyPolyAction( const ::basegfx::B2DPolyPolygon&    rPolyPoly,
                                            const OutDevState&                  rState,
                                            bool                                bFill,
                                            bool                                bStroke,
                                            int                                 nTransparency ) :
                CachedPrimitiveBase( false ),
                mxPolyPoly( ::canvastools::xPolyPolygonFromB2DPolyPolygon( rPolyPoly) )
            {
                cppcanvastools::initRenderState(maState,rState);

                if( bFill )
                {
                    maFillColor = rState.fillColor;

                    if( maFillColor.getLength() < 4 )
                        maFillColor.realloc( 4 );

                    // TODO(F1): Color management
                    // adapt fill color transparency
                    maFillColor.getArray()[3] = 1.0 - nTransparency / 100.0;
                }

                if( bStroke )
                {
                    maState.DeviceColor = rState.lineColor;

                    if( maState.DeviceColor.getLength() < 4 )
                        maState.DeviceColor.realloc( 4 );

                    // TODO(F1): Color management
                    // adapt fill color transparency
                    maState.DeviceColor.getArray()[3] = 1.0 - nTransparency / 100.0;
                }
            }

            bool PolyPolyAction::renderPrimitive( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                                  const vclcanvas::ViewState& rViewState,
                                                  uno::Reference< vclcanvas::XCachedPrimitive >& rCachedPrimitive,
                                                  const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::PolyPolyAction::renderPrimitive()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::PolyPolyAction: 0x" << std::hex << this );

                vclcanvas::RenderState aLocalState( maState );
                ::canvastools::prependToRenderState(aLocalState, rTransformation);

                if( maFillColor.hasElements() )
                {
                    // TODO(E3): Use DBO's finalizer here,
                    // fillPolyPolygon() might throw
                    cpo::uno::Sequence<double> aTmpColor( aLocalState.DeviceColor );
                    aLocalState.DeviceColor = maFillColor;

                    rCachedPrimitive = rCanvas->fillPolyPolygon( mxPolyPoly,
                                                                  rViewState,
                                                                  aLocalState );

                    aLocalState.DeviceColor = std::move(aTmpColor);
                }

                if( aLocalState.DeviceColor.hasElements() )
                {
                    rCanvas->drawPolyPolygon( mxPolyPoly,
                                              rViewState,
                                              aLocalState );
                }

                return true;
            }

            bool PolyPolyAction::renderSubset( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                               const vclcanvas::ViewState& rViewState,
                                               const ::basegfx::B2DHomMatrix& rTransformation,
                                               const Subset&                  rSubset ) const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.

                // polygon only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return CachedPrimitiveBase::render( rCanvas, rViewState, rTransformation );
            }

            sal_Int32 PolyPolyAction::getActionCount() const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.
                return 1;
            }


            class TexturedPolyPolyAction : public CachedPrimitiveBase
            {
            public:
                TexturedPolyPolyAction( const ::basegfx::B2DPolyPolygon& rPoly,
                                        const OutDevState&               rState,
                                        const rendering::Texture&        rTexture );

                virtual bool renderSubset( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                           const vclcanvas::ViewState& rViewState,
                                           const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                using Action::render;
                virtual bool renderPrimitive( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                              const vclcanvas::ViewState& rViewState,
                                              uno::Reference< vclcanvas::XCachedPrimitive >& rCachedPrimitive,
                                              const ::basegfx::B2DHomMatrix&                 rTransformation ) const override;

                const uno::Reference< rendering::XPolyPolygon2D >   mxPolyPoly;

                // stroke color is now implicit: the maState.DeviceColor member
                vclcanvas::RenderState                              maState;
                const rendering::Texture                            maTexture;
            };

            TexturedPolyPolyAction::TexturedPolyPolyAction( const ::basegfx::B2DPolyPolygon& rPolyPoly,
                                                            const OutDevState&               rState,
                                                            const rendering::Texture&        rTexture ) :
                CachedPrimitiveBase( true ),
                mxPolyPoly( ::canvastools::xPolyPolygonFromB2DPolyPolygon( rPolyPoly) ),
                maTexture( rTexture )
            {
                cppcanvastools::initRenderState(maState,rState);
            }

            bool TexturedPolyPolyAction::renderPrimitive( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                                          const vclcanvas::ViewState& rViewState,
                                                          uno::Reference< vclcanvas::XCachedPrimitive >& rCachedPrimitive,
                                                          const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::PolyPolyAction::renderPrimitive()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::PolyPolyAction: 0x" << std::hex << this );

                vclcanvas::RenderState aLocalState( maState );
                ::canvastools::prependToRenderState(aLocalState, rTransformation);

                cpo::uno::Sequence< rendering::Texture > aSeq { maTexture };

                rCachedPrimitive = rCanvas->fillTexturedPolyPolygon( mxPolyPoly,
                                                                  rViewState,
                                                                  aLocalState,
                                                                  aSeq );
                return true;
            }

            bool TexturedPolyPolyAction::renderSubset( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                                       const vclcanvas::ViewState& rViewState,
                                                       const ::basegfx::B2DHomMatrix& rTransformation,
                                                       const Subset&                  rSubset ) const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.

                // polygon only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return CachedPrimitiveBase::render( rCanvas, rViewState, rTransformation );
            }

            sal_Int32 TexturedPolyPolyAction::getActionCount() const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.
                return 1;
            }


            class StrokedPolyPolyAction : public CachedPrimitiveBase
            {
            public:
                StrokedPolyPolyAction( const ::basegfx::B2DPolyPolygon&     rPoly,
                                       const OutDevState&                   rState,
                                       rendering::StrokeAttributes          aStrokeAttributes );

                virtual bool renderSubset( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                           const vclcanvas::ViewState& rViewState,
                                           const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                using Action::render;
                virtual bool renderPrimitive( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                              const vclcanvas::ViewState& rViewState,
                                              uno::Reference< vclcanvas::XCachedPrimitive >& rCachedPrimitive,
                                              const ::basegfx::B2DHomMatrix&                 rTransformation ) const override;

                const uno::Reference< rendering::XPolyPolygon2D >   mxPolyPoly;
                vclcanvas::RenderState                              maState;
                const rendering::StrokeAttributes                   maStrokeAttributes;
            };

            StrokedPolyPolyAction::StrokedPolyPolyAction( const ::basegfx::B2DPolyPolygon&      rPolyPoly,
                                                          const OutDevState&                    rState,
                                                          rendering::StrokeAttributes           aStrokeAttributes ) :
                CachedPrimitiveBase( false ),
                mxPolyPoly( ::canvastools::xPolyPolygonFromB2DPolyPolygon( rPolyPoly) ),
                maStrokeAttributes(std::move( aStrokeAttributes ))
            {
                cppcanvastools::initRenderState(maState,rState);
                maState.DeviceColor = rState.lineColor;
            }

            bool StrokedPolyPolyAction::renderPrimitive( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                                         const vclcanvas::ViewState& rViewState,
                                                         uno::Reference< vclcanvas::XCachedPrimitive >& /*rCachedPrimitive*/,
                                                         const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::PolyPolyAction::renderPrimitive()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::PolyPolyAction: 0x" << std::hex << this );

                vclcanvas::RenderState aLocalState( maState );
                ::canvastools::prependToRenderState(aLocalState, rTransformation);

                rCanvas->strokePolyPolygon( mxPolyPoly,
                                            rViewState,
                                            aLocalState,
                                            maStrokeAttributes );
                return true;
            }

            bool StrokedPolyPolyAction::renderSubset( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                                      const vclcanvas::ViewState& rViewState,
                                                      const ::basegfx::B2DHomMatrix&  rTransformation,
                                                      const Subset&                   rSubset ) const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.

                // polygon only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return CachedPrimitiveBase::render( rCanvas, rViewState, rTransformation );
            }

            sal_Int32 StrokedPolyPolyAction::getActionCount() const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.
                return 1;
            }
        }

        std::shared_ptr<Action> PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon& rPoly,
                                                                     const OutDevState&               rState    )
        {
            OSL_ENSURE( rState.isLineColorSet || rState.isFillColorSet,
                        "PolyPolyActionFactory::createPolyPolyAction() with empty line and fill color" );
            return std::make_shared<PolyPolyAction>( rPoly, rState,
                                                     rState.isFillColorSet,
                                                     rState.isLineColorSet );
        }

        std::shared_ptr<Action> PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon&   rPoly,
                                                                     const OutDevState&                 rState,
                                                                     const rendering::Texture&          rTexture )
        {
            return std::make_shared<TexturedPolyPolyAction>( rPoly, rState, rTexture );
        }

        std::shared_ptr<Action> PolyPolyActionFactory::createLinePolyPolyAction( const ::basegfx::B2DPolyPolygon& rPoly,
                                                                         const OutDevState&               rState )
        {
            OSL_ENSURE( rState.isLineColorSet,
                        "PolyPolyActionFactory::createLinePolyPolyAction() called with empty line color" );

            return std::make_shared<PolyPolyAction>( rPoly, rState,
                                                     false,
                                                     rState.isLineColorSet );
        }

        std::shared_ptr<Action> PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon&   rPoly,
                                                                     const OutDevState&                 rState,
                                                                     const rendering::StrokeAttributes& rStrokeAttributes )
        {
            OSL_ENSURE( rState.isLineColorSet,
                        "PolyPolyActionFactory::createPolyPolyAction() for strokes called with empty line color" );
            return std::make_shared<StrokedPolyPolyAction>( rPoly, rState, rStrokeAttributes );
        }

        std::shared_ptr<Action> PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon& rPoly,
                                                                     const OutDevState&               rState,
                                                                     int                              nTransparency     )
        {
            OSL_ENSURE( rState.isLineColorSet || rState.isFillColorSet,
                        "PolyPolyActionFactory::createPolyPolyAction() with empty line and fill color" );
            return std::make_shared<PolyPolyAction>( rPoly, rState,
                                                     rState.isFillColorSet,
                                                     rState.isLineColorSet,
                                                     nTransparency );
        }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
