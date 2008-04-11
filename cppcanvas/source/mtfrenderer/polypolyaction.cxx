/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: polypolyaction.cxx,v $
 * $Revision: 1.13 $
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

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>

#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <canvas/canvastools.hxx>

#include <boost/utility.hpp>

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
                mxPolyPoly( ::vcl::unotools::xPolyPolygonFromPolyPolygon( rCanvas->getUNOCanvas()->getDevice(), /*#i76339#*/ ::PolyPolygon(rPolyPoly) ) ),
                maBounds( ::basegfx::tools::getRange(rPolyPoly) ),
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
                mxPolyPoly( ::vcl::unotools::xPolyPolygonFromPolyPolygon( rCanvas->getUNOCanvas()->getDevice(), /*#i76339#*/ ::PolyPolygon(rPolyPoly) ) ),
                maBounds( ::basegfx::tools::getRange(rPolyPoly) ),
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

            bool PolyPolyAction::render( uno::Reference< rendering::XCachedPrimitive >& rCachedPrimitive,
                                         const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::PolyPolyAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::PolyPolyAction: 0x%X", this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

#ifdef SPECIAL_DEBUG
                aLocalState.Clip.clear();
                aLocalState.DeviceColor =
                    ::vcl::unotools::colorToDoubleSequence( mpCanvas->getUNOCanvas()->getDevice(),
                                                            ::Color( 0x80FF0000 ) );

                if( maState.Clip.is() )
                    mpCanvas->getUNOCanvas()->fillPolyPolygon( maState.Clip,
                                                               mpCanvas->getViewState(),
                                                               aLocalState );

                aLocalState.DeviceColor = maState.DeviceColor;
#endif

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

            bool PolyPolyAction::render( const ::basegfx::B2DHomMatrix& rTransformation,
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


            // -------------------------------------------------------------------------------

            class TexturedPolyPolyAction : public CachedPrimitiveBase
            {
            public:
                TexturedPolyPolyAction( const ::PolyPolygon&        rPoly,
                                        const CanvasSharedPtr&      rCanvas,
                                        const OutDevState&          rState,
                                        const rendering::Texture&   rTexture );

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

                const uno::Reference< rendering::XPolyPolygon2D >   mxPolyPoly;
                const ::Rectangle                                   maBounds;
                const CanvasSharedPtr                               mpCanvas;

                // stroke color is now implicit: the maState.DeviceColor member
                rendering::RenderState                              maState;
                const rendering::Texture                            maTexture;
            };

            TexturedPolyPolyAction::TexturedPolyPolyAction( const ::PolyPolygon&        rPolyPoly,
                                                            const CanvasSharedPtr&      rCanvas,
                                                            const OutDevState&          rState,
                                                            const rendering::Texture&   rTexture ) :
                CachedPrimitiveBase( rCanvas, true ),
                mxPolyPoly( ::vcl::unotools::xPolyPolygonFromPolyPolygon( rCanvas->getUNOCanvas()->getDevice(),
                                                                          rPolyPoly ) ),
                maBounds( rPolyPoly.GetBoundRect() ),
                mpCanvas( rCanvas ),
                maState(),
                maTexture( rTexture )
            {
                tools::initRenderState(maState,rState);
            }

            bool TexturedPolyPolyAction::render( uno::Reference< rendering::XCachedPrimitive >& rCachedPrimitive,
                                                 const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::PolyPolyAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::PolyPolyAction: 0x%X", this );

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

            bool TexturedPolyPolyAction::render( const ::basegfx::B2DHomMatrix& rTransformation,
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
                    ::vcl::unotools::b2DRectangleFromRectangle( maBounds ),
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

            // -------------------------------------------------------------------------------

            class StrokedPolyPolyAction : public CachedPrimitiveBase
            {
            public:
                StrokedPolyPolyAction( const ::PolyPolygon&                 rPoly,
                                       const CanvasSharedPtr&               rCanvas,
                                       const OutDevState&                   rState,
                                       const rendering::StrokeAttributes&   rStrokeAttributes );

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

                const uno::Reference< rendering::XPolyPolygon2D >   mxPolyPoly;
                const ::Rectangle                                   maBounds;
                const CanvasSharedPtr                               mpCanvas;
                rendering::RenderState                              maState;
                const rendering::StrokeAttributes                   maStrokeAttributes;
            };

            StrokedPolyPolyAction::StrokedPolyPolyAction( const ::PolyPolygon&                  rPolyPoly,
                                                          const CanvasSharedPtr&                rCanvas,
                                                          const OutDevState&                    rState,
                                                          const rendering::StrokeAttributes&    rStrokeAttributes ) :
                CachedPrimitiveBase( rCanvas, false ),
                mxPolyPoly( ::vcl::unotools::xPolyPolygonFromPolyPolygon( rCanvas->getUNOCanvas()->getDevice(),
                                                                          rPolyPoly ) ),
                maBounds( rPolyPoly.GetBoundRect() ),
                mpCanvas( rCanvas ),
                maState(),
                maStrokeAttributes( rStrokeAttributes )
            {
                tools::initRenderState(maState,rState);
                maState.DeviceColor = rState.lineColor;
            }

            bool StrokedPolyPolyAction::render( uno::Reference< rendering::XCachedPrimitive >& rCachedPrimitive,
                                                const ::basegfx::B2DHomMatrix&                 rTransformation ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::PolyPolyAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::PolyPolyAction: 0x%X", this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                rCachedPrimitive = mpCanvas->getUNOCanvas()->strokePolyPolygon( mxPolyPoly,
                                                                                mpCanvas->getViewState(),
                                                                                aLocalState,
                                                                                maStrokeAttributes );
                return true;
            }

            bool StrokedPolyPolyAction::render( const ::basegfx::B2DHomMatrix&  rTransformation,
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
                    ::vcl::unotools::b2DRectangleFromRectangle( maBounds ),
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

        ActionSharedPtr PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon& rPoly,
                                                                     const CanvasSharedPtr&           rCanvas,
                                                                     const OutDevState&               rState    )
        {
            OSL_ENSURE( rState.isLineColorSet || rState.isFillColorSet,
                        "PolyPolyActionFactory::createPolyPolyAction() with empty line and fill color" );
            return ActionSharedPtr( new PolyPolyAction( rPoly, rCanvas, rState,
                                                        rState.isFillColorSet,
                                                        rState.isLineColorSet ) );
        }

        ActionSharedPtr PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon& rPoly,
                                                                     const CanvasSharedPtr&             rCanvas,
                                                                     const OutDevState&                 rState,
                                                                     const rendering::Texture&          rTexture )
        {
            return ActionSharedPtr( new TexturedPolyPolyAction( /*#i76339#*/ ::PolyPolygon(rPoly), rCanvas, rState, rTexture ) );
        }

        ActionSharedPtr PolyPolyActionFactory::createLinePolyPolyAction( const ::basegfx::B2DPolyPolygon& rPoly,
                                                                         const CanvasSharedPtr&           rCanvas,
                                                                         const OutDevState&               rState )
        {
            OSL_ENSURE( rState.isLineColorSet,
                        "PolyPolyActionFactory::createLinePolyPolyAction() called with empty line color" );

            return ActionSharedPtr( new PolyPolyAction( rPoly, rCanvas, rState,
                                                        false,
                                                        rState.isLineColorSet ) );
        }

        ActionSharedPtr PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon&   rPoly,
                                                                     const CanvasSharedPtr&             rCanvas,
                                                                     const OutDevState&                 rState,
                                                                     const rendering::StrokeAttributes& rStrokeAttributes )
        {
            OSL_ENSURE( rState.isLineColorSet,
                        "PolyPolyActionFactory::createPolyPolyAction() for strokes called with empty line color" );
            return ActionSharedPtr( new StrokedPolyPolyAction( /*#i76339#*/ ::PolyPolygon(rPoly), rCanvas, rState, rStrokeAttributes ) );
        }

        ActionSharedPtr PolyPolyActionFactory::createPolyPolyAction( const ::basegfx::B2DPolyPolygon& rPoly,
                                                                     const CanvasSharedPtr&           rCanvas,
                                                                     const OutDevState&               rState,
                                                                     int                              nTransparency     )
        {
            OSL_ENSURE( rState.isLineColorSet || rState.isFillColorSet,
                        "PolyPolyActionFactory::createPolyPolyAction() with empty line and fill color" );
            return ActionSharedPtr( new PolyPolyAction( rPoly, rCanvas, rState,
                                                        rState.isFillColorSet,
                                                        rState.isLineColorSet,
                                                        nTransparency ) );
        }

    }
}
