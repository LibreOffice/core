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


#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>

#include <sal/types.h>
#include <vcl/canvastools.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <canvas/canvastools.hxx>

#include "cachedprimitivebase.hxx"
#include "polypolyaction.hxx"
#include "outdevstate.hxx"
#include "mtftools.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        namespace
        {
            class PolyPolyAction : public CachedPrimitiveBase
            {
            public:
                PolyPolyAction( const ::basegfx::B2DPolyPolygon&,
                                const CanvasSharedPtr&,
                                const OutDevState&,
                                bool bFill,
                                bool bStroke );
                PolyPolyAction( const ::basegfx::B2DPolyPolygon&,
                                const CanvasSharedPtr&,
                                const OutDevState&,
                                bool bFill,
                                bool bStroke,
                                int nTransparency );

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

                const uno::Reference< rendering::XPolyPolygon2D >   mxPolyPoly;
                const ::basegfx::B2DRange                           maBounds;
                const CanvasSharedPtr                               mpCanvas;

                // stroke color is now implicit: the maState.DeviceColor member
                rendering::RenderState                              maState;

                uno::Sequence< double >                             maFillColor;
            };

            PolyPolyAction::PolyPolyAction( const ::basegfx::B2DPolyPolygon&    rPolyPoly,
                                            const CanvasSharedPtr&              rCanvas,
                                            const OutDevState&                  rState,
                                            bool                                bFill,
                                            bool                                bStroke ) :
                CachedPrimitiveBase( rCanvas, false ),
                mxPolyPoly( ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon( rCanvas->getUNOCanvas()->getDevice(), rPolyPoly) ),
                maBounds( ::basegfx::utils::getRange(rPolyPoly) ),
                mpCanvas( rCanvas ),
                maState(),
                maFillColor()
            {
                tools::initRenderState(maState,rState);

                if( bFill )
                    maFillColor = rState.fillColor;

                if( bStroke )
                    maState.DeviceColor = rState.lineColor;
            }

            PolyPolyAction::PolyPolyAction( const ::basegfx::B2DPolyPolygon&    rPolyPoly,
                                            const CanvasSharedPtr&              rCanvas,
                                            const OutDevState&                  rState,
                                            bool                                bFill,
                                            bool                                bStroke,
                                            int                                 nTransparency ) :
                CachedPrimitiveBase( rCanvas, false ),
                mxPolyPoly( ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon( rCanvas->getUNOCanvas()->getDevice(), rPolyPoly) ),
                maBounds( ::basegfx::utils::getRange(rPolyPoly) ),
                mpCanvas( rCanvas ),
                maState(),
                maFillColor()
            {
                tools::initRenderState(maState,rState);

                if( bFill )
                {
                    maFillColor = rState.fillColor;

                    if( maFillColor.getLength() < 4 )
                        maFillColor.realloc( 4 );

                    // TODO(F1): Color management
                    // adapt fill color transparency
                    maFillColor[3] = 1.0 - nTransparency / 100.0;
                }

                if( bStroke )
                {
                    maState.DeviceColor = rState.lineColor;

                    if( maState.DeviceColor.getLength() < 4 )
                        maState.DeviceColor.realloc( 4 );

                    // TODO(F1): Color management
                    // adapt fill color transparency
                    maState.DeviceColor[3] = 1.0 - nTransparency / 100.0;
                }
            }

            bool PolyPolyAction::renderPrimitive( uno::Reference< rendering::XCachedPrimitive >& rCachedPrimitive,
                                                  const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::PolyPolyAction::renderPrimitive()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::PolyPolyAction: 0x" << std::hex << this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                if( maFillColor.getLength() )
                {
                    // TODO(E3): Use DBO's finalizer here,
                    // fillPolyPolygon() might throw
                    const uno::Sequence< double > aTmpColor( aLocalState.DeviceColor );
                    aLocalState.DeviceColor = maFillColor;

                    rCachedPrimitive = mpCanvas->getUNOCanvas()->fillPolyPolygon( mxPolyPoly,
                                                                                  mpCanvas->getViewState(),
                                                                                  aLocalState );

                    aLocalState.DeviceColor = aTmpColor;
                }

                if( aLocalState.DeviceColor.getLength() )
                {
                    rCachedPrimitive = mpCanvas->getUNOCanvas()->drawPolyPolygon( mxPolyPoly,
                                                                                  mpCanvas->getViewState(),
                                                                                  aLocalState );
                }

                return true;
            }

            bool PolyPolyAction::renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                               const Subset&                  rSubset ) const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.

                // polygon only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return CachedPrimitiveBase::render( rTransformation );
            }

            ::basegfx::B2DRange PolyPolyAction::getBounds( const ::basegfx::B2DHomMatrix&   rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return tools::calcDevicePixelBounds(
                    maBounds,
                    mpCanvas->getViewState(),
                    aLocalState );
            }

            ::basegfx::B2DRange PolyPolyAction::getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                           const Subset&                    rSubset ) const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.

                // polygon only contains a single action, empty bounds
                // if subset requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return ::basegfx::B2DRange();

                return getBounds( rTransformation );
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
                                        const CanvasSharedPtr&           rCanvas,
                                        const OutDevState&               rState,
                                        const rendering::Texture&        rTexture );

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

                const uno::Reference< rendering::XPolyPolygon2D >   mxPolyPoly;
                const ::basegfx::B2DRectangle                       maBounds;
                const CanvasSharedPtr                               mpCanvas;

                // stroke color is now implicit: the maState.DeviceColor member
                rendering::RenderState                              maState;
                const rendering::Texture                            maTexture;
            };

            TexturedPolyPolyAction::TexturedPolyPolyAction( const ::basegfx::B2DPolyPolygon& rPolyPoly,
                                                            const CanvasSharedPtr&           rCanvas,
                                                            const OutDevState&               rState,
                                                            const rendering::Texture&        rTexture ) :
                CachedPrimitiveBase( rCanvas, true ),
                mxPolyPoly( ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon( rCanvas->getUNOCanvas()->getDevice(), rPolyPoly) ),
                maBounds( ::basegfx::utils::getRange(rPolyPoly) ),
                mpCanvas( rCanvas ),
                maState(),
                maTexture( rTexture )
            {
                tools::initRenderState(maState,rState);
            }

            bool TexturedPolyPolyAction::renderPrimitive( uno::Reference< rendering::XCachedPrimitive >& rCachedPrimitive,
                                                          const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::PolyPolyAction::renderPrimitive()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::PolyPolyAction: 0x" << std::hex << this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                uno::Sequence< rendering::Texture > aSeq(1);
                aSeq[0] = maTexture;

                rCachedPrimitive = mpCanvas->getUNOCanvas()->fillTexturedPolyPolygon( mxPolyPoly,
                                                                                      mpCanvas->getViewState(),
                                                                                      aLocalState,
                                                                                      aSeq );
                return true;
            }

            bool TexturedPolyPolyAction::renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                                       const Subset&                  rSubset ) const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.

                // polygon only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return CachedPrimitiveBase::render( rTransformation );
            }

            ::basegfx::B2DRange TexturedPolyPolyAction::getBounds( const ::basegfx::B2DHomMatrix&   rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return tools::calcDevicePixelBounds(
                    maBounds,
                    mpCanvas->getViewState(),
                    aLocalState );
            }

            ::basegfx::B2DRange TexturedPolyPolyAction::getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                                   const Subset&                    rSubset ) const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.

                // polygon only contains a single action, empty bounds
                // if subset requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return ::basegfx::B2DRange();

                return getBounds( rTransformation );
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
                                       const CanvasSharedPtr&               rCanvas,
                                       const OutDevState&                   rState,
                                       const rendering::StrokeAttributes&   rStrokeAttributes );

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

                const uno::Reference< rendering::XPolyPolygon2D >   mxPolyPoly;
                const ::basegfx::B2DRectangle                       maBounds;
                const CanvasSharedPtr                               mpCanvas;
                rendering::RenderState                              maState;
                const rendering::StrokeAttributes                   maStrokeAttributes;
            };

            StrokedPolyPolyAction::StrokedPolyPolyAction( const ::basegfx::B2DPolyPolygon&      rPolyPoly,
                                                          const CanvasSharedPtr&                rCanvas,
                                                          const OutDevState&                    rState,
                                                          const rendering::StrokeAttributes&    rStrokeAttributes ) :
                CachedPrimitiveBase( rCanvas, false ),
                mxPolyPoly( ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon( rCanvas->getUNOCanvas()->getDevice(), rPolyPoly) ),
                maBounds( ::basegfx::utils::getRange(rPolyPoly) ),
                mpCanvas( rCanvas ),
                maState(),
                maStrokeAttributes( rStrokeAttributes )
            {
                tools::initRenderState(maState,rState);
                maState.DeviceColor = rState.lineColor;
            }

            bool StrokedPolyPolyAction::renderPrimitive( uno::Reference< rendering::XCachedPrimitive >& rCachedPrimitive,
                                                         const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::PolyPolyAction::renderPrimitive()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::PolyPolyAction: 0x" << std::hex << this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                rCachedPrimitive = mpCanvas->getUNOCanvas()->strokePolyPolygon( mxPolyPoly,
                                                                                mpCanvas->getViewState(),
                                                                                aLocalState,
                                                                                maStrokeAttributes );
                return true;
            }

            bool StrokedPolyPolyAction::renderSubset( const ::basegfx::B2DHomMatrix&  rTransformation,
                                                      const Subset&                   rSubset ) const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.

                // polygon only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return CachedPrimitiveBase::render( rTransformation );
            }

            ::basegfx::B2DRange StrokedPolyPolyAction::getBounds( const ::basegfx::B2DHomMatrix&    rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return tools::calcDevicePixelBounds(
                    maBounds,
                    mpCanvas->getViewState(),
                    aLocalState );
            }

            ::basegfx::B2DRange StrokedPolyPolyAction::getBounds( const ::basegfx::B2DHomMatrix&    rTransformation,
                                                                  const Subset&                 rSubset ) const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.

                // polygon only contains a single action, empty bounds
                // if subset requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return ::basegfx::B2DRange();

                return getBounds( rTransformation );
            }

            sal_Int32 StrokedPolyPolyAction::getActionCount() const
            {
                // TODO(F1): Split up poly-polygon into polygons, or even
                // line segments, when subsets are requested.
                return 1;
            }
        }

        std::shared_ptr<Action> PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon& rPoly,
                                                                     const CanvasSharedPtr&           rCanvas,
                                                                     const OutDevState&               rState    )
        {
            OSL_ENSURE( rState.isLineColorSet || rState.isFillColorSet,
                        "PolyPolyActionFactory::createPolyPolyAction() with empty line and fill color" );
            return std::shared_ptr<Action>( new PolyPolyAction( rPoly, rCanvas, rState,
                                                        rState.isFillColorSet,
                                                        rState.isLineColorSet ) );
        }

        std::shared_ptr<Action> PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon&   rPoly,
                                                                     const CanvasSharedPtr&             rCanvas,
                                                                     const OutDevState&                 rState,
                                                                     const rendering::Texture&          rTexture )
        {
            return std::shared_ptr<Action>( new TexturedPolyPolyAction( rPoly, rCanvas, rState, rTexture ) );
        }

        std::shared_ptr<Action> PolyPolyActionFactory::createLinePolyPolyAction( const ::basegfx::B2DPolyPolygon& rPoly,
                                                                         const CanvasSharedPtr&           rCanvas,
                                                                         const OutDevState&               rState )
        {
            OSL_ENSURE( rState.isLineColorSet,
                        "PolyPolyActionFactory::createLinePolyPolyAction() called with empty line color" );

            return std::shared_ptr<Action>( new PolyPolyAction( rPoly, rCanvas, rState,
                                                        false,
                                                        rState.isLineColorSet ) );
        }

        std::shared_ptr<Action> PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon&   rPoly,
                                                                     const CanvasSharedPtr&             rCanvas,
                                                                     const OutDevState&                 rState,
                                                                     const rendering::StrokeAttributes& rStrokeAttributes )
        {
            OSL_ENSURE( rState.isLineColorSet,
                        "PolyPolyActionFactory::createPolyPolyAction() for strokes called with empty line color" );
            return std::shared_ptr<Action>( new StrokedPolyPolyAction( rPoly, rCanvas, rState, rStrokeAttributes ) );
        }

        std::shared_ptr<Action> PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon& rPoly,
                                                                     const CanvasSharedPtr&           rCanvas,
                                                                     const OutDevState&               rState,
                                                                     int                              nTransparency     )
        {
            OSL_ENSURE( rState.isLineColorSet || rState.isFillColorSet,
                        "PolyPolyActionFactory::createPolyPolyAction() with empty line and fill color" );
            return std::shared_ptr<Action>( new PolyPolyAction( rPoly, rCanvas, rState,
                                                        rState.isFillColorSet,
                                                        rState.isLineColorSet,
                                                        nTransparency ) );
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
