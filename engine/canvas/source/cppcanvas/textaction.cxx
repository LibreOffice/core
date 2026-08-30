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


#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/StrokeAttributes.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <tools/gen.hxx>
#include <utility>
#include <vcl/canvastools.hxx>
#include <vcl/virdev.hxx>

#include <basegfx/utils/canvastools.hxx>
#include <canvastools.hxx>
#include <memory>
#include <sal/log.hxx>

#include "textaction.hxx"
#include <textlayout.hxx>
#include "textlineshelper.hxx"
#include "outdevstate.hxx"
#include "mtftools.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
        namespace
        {
            void init( vclcanvas::RenderState&                  o_rRenderState,
                       const ::basegfx::B2DPoint&               rStartPoint,
                       const OutDevState&                       rState)
            {
                cppcanvastools::initRenderState(o_rRenderState,rState);

                // #i36950# Offset clip back to origin (as it's also moved
                // by rStartPoint)
                // #i53964# Also take VCL font rotation into account,
                // since this, opposed to the FontMatrix rotation
                // elsewhere, _does_ get incorporated into the render
                // state transform.
                cppcanvastools::modifyClip( o_rRenderState,
                                   rState,
                                   rStartPoint,
                                   nullptr,
                                   &rState.fontRotation );

                basegfx::B2DHomMatrix aLocalTransformation(basegfx::utils::createRotateB2DHomMatrix(rState.fontRotation));
                aLocalTransformation.translate( rStartPoint.getX(),
                                                rStartPoint.getY() );
                o_rRenderState.AffineTransform *= aLocalTransformation;

                o_rRenderState.DeviceColor = rState.textColor;
            }

            void init( vclcanvas::RenderState&                      o_rRenderState,
                       rtl::Reference< vclcanvas::CanvasFont >&     o_rFont,
                       const ::basegfx::B2DPoint&                   rStartPoint,
                       const OutDevState&                           rState,
                       vclcanvas::Canvas&  rUnoCanvas      )
            {
                // ensure that o_rFont is valid. It is possible that
                // text actions are generated without previously
                // setting a font. Then, just take a default font
                if( !o_rFont.is() )
                {
                    // Use completely default FontRequest
                    const rendering::FontRequest aFontRequest;

                    geometry::Matrix2D aFontMatrix;
                    ::canvastools::setIdentityMatrix2D( aFontMatrix );

                    o_rFont = rUnoCanvas.createFont(
                        aFontRequest,
                        FontEmphasisMark::NONE,
                        aFontMatrix );
                }

                init( o_rRenderState,
                      rStartPoint,
                      rState );
            }

            void initLayoutWidth(double& rLayoutWidth, const cpo::uno::Sequence<double>& rOffsets)
            {
                ENSURE_OR_THROW(rOffsets.hasElements(),
                                  "::cppcanvas::initLayoutWidth(): zero-length array" );
                rLayoutWidth = *(std::max_element(rOffsets.begin(), rOffsets.end()));
            }

            cpo::uno::Sequence< double > setupDXArray( KernArraySpan    rCharWidths,
                                                  sal_Int32          nLen,
                                                  const OutDevState& rState )
            {
                // convert character widths from logical units
                cpo::uno::Sequence< double > aCharWidthSeq( nLen );
                double*                 pOutputWidths( aCharWidthSeq.getArray() );

                // #143885# maintain (nearly) full precision of DX
                // array, by circumventing integer-based
                // OutDev-mapping
                const double nScale( rState.mapModeTransform.get(0,0) );
                for( int i = 0; i < nLen; ++i )
                {
                    // TODO(F2): use correct scale direction
                    *pOutputWidths++ = rCharWidths[i] * nScale;
                }

                return aCharWidthSeq;
            }

            cpo::uno::Sequence< double > setupDXArray( const OUString&    rText,
                                                  sal_Int32          nStartPos,
                                                  sal_Int32          nLen,
                                                  VirtualDevice const & rVDev,
                                                  const OutDevState& rState )
            {
                // no external DX array given, create one from given
                // string
                KernArray aCharWidths;

                rVDev.GetTextArray( rText, &aCharWidths, nStartPos, nLen );

                return setupDXArray( aCharWidths, nLen, rState );
            }

            ::basegfx::B2DPoint adaptStartPoint( const ::basegfx::B2DPoint&     rStartPoint,
                                                 const OutDevState&             rState,
                                                 const cpo::uno::Sequence< double >& rOffsets )
            {
                ::basegfx::B2DPoint aLocalPoint( rStartPoint );

                if( rState.textAlignment )
                {
                    // text origin is right, not left. Modify start point
                    // accordingly, because XCanvas::drawTextLayout()
                    // always aligns left!

                    const double nOffset( rOffsets[ rOffsets.getLength()-1 ] );

                    // correct start point for rotated text: rotate around
                    // former start point
                    aLocalPoint.setX( aLocalPoint.getX() + cos( rState.fontRotation )*nOffset );
                    aLocalPoint.setY( aLocalPoint.getY() + sin( rState.fontRotation )*nOffset );
                }

                return aLocalPoint;
            }

            /** Perform common setup for array text actions

                This method creates the vclcanvas::TextLayout object and
                initializes it, e.g. with the logical advancements.
             */
            void initArrayAction( vclcanvas::RenderState&                   o_rRenderState,
                                  rtl::Reference< vclcanvas::TextLayout >& o_rTextLayout,
                                  const ::basegfx::B2DPoint&                rStartPoint,
                                  const OUString&                    rText,
                                  sal_Int32                                 nStartPos,
                                  sal_Int32                                 nLen,
                                  const cpo::uno::Sequence< double >&            rOffsets,
                                  const cpo::uno::Sequence< bool >&          rKashidas,
                                  vclcanvas::Canvas& rCanvas,
                                  const OutDevState&                        rState )
            {
                ENSURE_OR_THROW( rOffsets.hasElements(),
                                  "::cppcanvas::initArrayAction(): zero-length DX array" );

                const ::basegfx::B2DPoint aLocalStartPoint(
                    adaptStartPoint( rStartPoint, rState, rOffsets ) );

                rtl::Reference< vclcanvas::CanvasFont > xFont( rState.xFont );

                init( o_rRenderState, xFont, aLocalStartPoint, rState, rCanvas );

                o_rTextLayout = xFont->createTextLayout(
                    rendering::StringContext( rText, nStartPos, nLen ),
                    rState.textDirection,
                    0 );

                ENSURE_OR_THROW( o_rTextLayout.is(),
                                  "::cppcanvas::initArrayAction(): Invalid font" );

                o_rTextLayout->applyLogicalAdvancements( rOffsets );
                o_rTextLayout->applyKashidaPositions( rKashidas );

            }

            /** Interface for renderEffectText functor below.

                This is interface is used from the renderEffectText()
                method below, to call the client implementation.
             */
            class TextRenderer
            {
            public:
                virtual ~TextRenderer() {}

                /// Render text with given RenderState
                virtual bool operator()( vclcanvas::Canvas& rCanvas,
                                         const vclcanvas::ViewState& rViewState,
                                         const vclcanvas::RenderState& rRenderState,
                                         const ::Color& rTextFillColor, bool bNormalText ) const = 0;
            };

            /** Render effect text.

                @param rRenderer
                Functor object, will be called to render the actual
                part of the text effect (the text itself and the means
                to render it are unknown to this method)
             */
            bool renderEffectText( vclcanvas::Canvas& rCanvas,
                                   const vclcanvas::ViewState&                  rViewState,
                                   const TextRenderer&                          rRenderer,
                                   const vclcanvas::RenderState&                rRenderState,
                                   const ::Color&                               rShadowColor,
                                   const ::basegfx::B2DSize&                    rShadowOffset,
                                   const ::Color&                               rReliefColor,
                                   const ::basegfx::B2DSize&                    rReliefOffset,
                                   const ::Color&                               rTextFillColor )
            {
                ::Color aEmptyColor( COL_AUTO );

                // draw shadow text, if enabled
                if( rShadowColor != aEmptyColor )
                {
                    vclcanvas::RenderState aShadowState( rRenderState );
                    ::basegfx::B2DHomMatrix aTranslate;

                    aTranslate.translate(rShadowOffset.getWidth(),
                                         rShadowOffset.getHeight());

                    aShadowState.AffineTransform *= aTranslate;

                    aShadowState.DeviceColor = rShadowColor;

                    rRenderer( rCanvas, rViewState, aShadowState, rTextFillColor, false );
                }

                // draw relief text, if enabled
                if( rReliefColor != aEmptyColor )
                {
                    vclcanvas::RenderState aReliefState( rRenderState );
                    ::basegfx::B2DHomMatrix aTranslate;

                    aTranslate.translate(rReliefOffset.getWidth(),
                                         rReliefOffset.getHeight());

                    aReliefState.AffineTransform *= aTranslate;

                    aReliefState.DeviceColor = rReliefColor;

                    rRenderer( rCanvas, rViewState, aReliefState, rTextFillColor, false );
                }

                // draw normal text
                rRenderer( rCanvas, rViewState, rRenderState, rTextFillColor, true );

                return true;
            }


            void initEffectLinePolyPolygon( ::basegfx::B2DSize&                             o_rOverallSize,
                                            basegfx::B2DPolyPolygon&                        o_rTextLines,
                                            double                                          nLineWidth,
                                            const cppcanvastools::TextLineInfo&                      rLineInfo   )
            {
                const ::basegfx::B2DPolyPolygon aPoly(
                    cppcanvastools::createTextLinesPolyPolygon( 0.0, nLineWidth,
                                                       rLineInfo ) );
                auto aRange = aPoly.getB2DRange().getRange();
                o_rOverallSize = basegfx::B2DSize(aRange.getX(), aRange.getY());

                o_rTextLines = aPoly;
            }


            class TextArrayAction : public Action
            {
            public:
                TextArrayAction( const ::basegfx::B2DPoint&     rStartPoint,
                                 const OUString&         rString,
                                 sal_Int32                      nStartPos,
                                 sal_Int32                      nLen,
                                 const cpo::uno::Sequence< double >& rOffsets,
                                 const cpo::uno::Sequence< bool >& rKashidas,
                                 const OutDevState&             rState,
                                 vclcanvas::Canvas& );

                TextArrayAction(const TextArrayAction&) = delete;
                const TextArrayAction& operator=(const TextArrayAction&) = delete;

                virtual bool render( vclcanvas::Canvas& rCanvas,
                                     const vclcanvas::ViewState& rViewState,
                                     const ::basegfx::B2DHomMatrix& rTransformation ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                // TODO(P2): This is potentially a real mass object
                // (every character might be a separate TextAction),
                // thus, make it as lightweight as possible. For
                // example, share common RenderState among several
                // TextActions, maybe using maOffsets for the
                // translation.

                rtl::Reference<vclcanvas::TextLayout>       mxTextLayout;
                vclcanvas::RenderState                      maState;
                double                                      mnLayoutWidth;
            };

            TextArrayAction::TextArrayAction( const ::basegfx::B2DPoint&        rStartPoint,
                                              const OUString&            rString,
                                              sal_Int32                         nStartPos,
                                              sal_Int32                         nLen,
                                              const cpo::uno::Sequence< double >&    rOffsets,
                                              const cpo::uno::Sequence< bool >&  rKashidas,
                                              const OutDevState&                rState,
                                              vclcanvas::Canvas& rCanvas )
            {
                initLayoutWidth(mnLayoutWidth, rOffsets);

                initArrayAction( maState,
                                 mxTextLayout,
                                 rStartPoint,
                                 rString,
                                 nStartPos,
                                 nLen,
                                 rOffsets,
                                 rKashidas,
                                 rCanvas,
                                 rState );
            }

            bool TextArrayAction::render( vclcanvas::Canvas& rCanvas,
                                          const vclcanvas::ViewState& rViewState,
                                          const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::TextArrayAction::render()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::TextArrayAction: 0x" << std::hex << this );

                vclcanvas::RenderState aLocalState( maState );
                aLocalState.AffineTransform = rTransformation * aLocalState.AffineTransform;

                rCanvas.drawTextLayout( mxTextLayout,
                                      rViewState,
                                      aLocalState );

                return true;
            }

            sal_Int32 TextArrayAction::getActionCount() const
            {
                const rendering::StringContext aOrigContext( mxTextLayout->getText() );

                return aOrigContext.Length;
            }


            class EffectTextArrayAction :
                public Action,
                public TextRenderer
            {
            public:
                EffectTextArrayAction( const ::basegfx::B2DPoint&       rStartPoint,
                                       const ::basegfx::B2DSize&        rReliefOffset,
                                       const ::Color&                   rReliefColor,
                                       const ::basegfx::B2DSize&        rShadowOffset,
                                       const ::Color&                   rShadowColor,
                                       const ::Color&                   rTextFillColor,
                                       const OUString&           rText,
                                       sal_Int32                        nStartPos,
                                       sal_Int32                        nLen,
                                       const cpo::uno::Sequence< double >&   rOffsets,
                                       const cpo::uno::Sequence< bool >& rKashidas,
                                       VirtualDevice const &            rVDev,
                                       const OutDevState&               rState,
                                       vclcanvas::Canvas& );

                EffectTextArrayAction(const EffectTextArrayAction&) = delete;
                const EffectTextArrayAction& operator=(const EffectTextArrayAction&) = delete;

                virtual bool render( vclcanvas::Canvas& rCanvas,
                                     const vclcanvas::ViewState& rViewState,
                                     const ::basegfx::B2DHomMatrix& rTransformation ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                // TextRenderer interface
                virtual bool operator()( vclcanvas::Canvas& rCanvas,
                                         const vclcanvas::ViewState& rViewState,
                                         const vclcanvas::RenderState& rRenderState,
                                         const ::Color& rTextFillColor, bool bNormalText ) const override;

                basegfx::B2DPolyPolygon queryTextBoundsPoly() const;

                // TODO(P2): This is potentially a real mass object
                // (every character might be a separate TextAction),
                // thus, make it as lightweight as possible. For
                // example, share common RenderState among several
                // TextActions, maybe using maOffsets for the
                // translation.

                rtl::Reference<vclcanvas::TextLayout>           mxTextLayout;
                vclcanvas::RenderState                          maState;
                const cppcanvastools::TextLineInfo                       maTextLineInfo;
                TextLinesHelper                                 maTextLinesHelper;
                const ::basegfx::B2DSize                        maReliefOffset;
                const ::Color                                   maReliefColor;
                const ::basegfx::B2DSize                        maShadowOffset;
                const ::Color                                   maShadowColor;
                const ::Color                                   maTextFillColor;
                double                                          mnLayoutWidth;
            };

            EffectTextArrayAction::EffectTextArrayAction( const ::basegfx::B2DPoint&        rStartPoint,
                                                          const ::basegfx::B2DSize&         rReliefOffset,
                                                          const ::Color&                    rReliefColor,
                                                          const ::basegfx::B2DSize&         rShadowOffset,
                                                          const ::Color&                    rShadowColor,
                                                          const ::Color&                    rTextFillColor,
                                                          const OUString&            rText,
                                                          sal_Int32                         nStartPos,
                                                          sal_Int32                         nLen,
                                                          const cpo::uno::Sequence< double >&    rOffsets,
                                                          const cpo::uno::Sequence< bool >&  rKashidas,
                                                          VirtualDevice const &             rVDev,
                                                          const OutDevState&                rState,
                                                          vclcanvas::Canvas& rCanvas ) :
                maTextLineInfo( cppcanvastools::createTextLineInfo( rVDev, rState ) ),
                maTextLinesHelper(rState),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor ),
                maTextFillColor( rTextFillColor )
            {
                initLayoutWidth(mnLayoutWidth, rOffsets);

                maTextLinesHelper.init(mnLayoutWidth, maTextLineInfo);

                initArrayAction( maState,
                                 mxTextLayout,
                                 rStartPoint,
                                 rText,
                                 nStartPos,
                                 nLen,
                                 rOffsets,
                                 rKashidas,
                                 rCanvas,
                                 rState );
            }

            basegfx::B2DPolyPolygon EffectTextArrayAction::queryTextBoundsPoly() const
            {
                const geometry::RealRectangle2D aTextBounds(mxTextLayout->queryTextBounds());
                auto aB2DBounds = ::basegfx::unotools::b2DRectangleFromRealRectangle2D(aTextBounds);
                auto aTextBoundsPoly = ::basegfx::utils::createPolygonFromRect(aB2DBounds);
                return basegfx::B2DPolyPolygon(aTextBoundsPoly);
            }

            bool EffectTextArrayAction::operator()( vclcanvas::Canvas& rCanvas,
                                                    const vclcanvas::ViewState& rViewState,
                                                    const vclcanvas::RenderState& rRenderState,
                                                    const ::Color& rTextFillColor, bool bNormalText) const
            {
                //rhbz#1589029 non-transparent text fill background support
                if (rTextFillColor != COL_AUTO)
                {
                    vclcanvas::RenderState aLocalState(rRenderState);
                    aLocalState.DeviceColor = rTextFillColor;
                    auto xTextBounds = queryTextBoundsPoly();
                    // background of text
                    rCanvas.fillPolyPolygon(xTextBounds, rViewState, aLocalState);
                }

                // under/over lines
                maTextLinesHelper.render(rCanvas, rViewState, rRenderState, bNormalText);

                rCanvas.drawTextLayout( mxTextLayout,
                                         rViewState,
                                         rRenderState );

                return true;
            }

            bool EffectTextArrayAction::render( vclcanvas::Canvas& rCanvas,
                                                const vclcanvas::ViewState& rViewState,
                                                const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::EffectTextArrayAction::render()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::EffectTextArrayAction: 0x" << std::hex << this );

                vclcanvas::RenderState aLocalState( maState );
                aLocalState.AffineTransform = rTransformation * aLocalState.AffineTransform;

                return renderEffectText( rCanvas, rViewState, *this,
                                         aLocalState,
                                         maShadowColor,
                                         maShadowOffset,
                                         maReliefColor,
                                         maReliefOffset,
                                         maTextFillColor);
            }

            sal_Int32 EffectTextArrayAction::getActionCount() const
            {
                const rendering::StringContext aOrigContext( mxTextLayout->getText() );

                return aOrigContext.Length;
            }


            class OutlineAction :
                public Action,
                public TextRenderer
            {
            public:
                OutlineAction( const ::basegfx::B2DPoint&                           rStartPoint,
                               const ::basegfx::B2DSize&                            rReliefOffset,
                               const ::Color&                                       rReliefColor,
                               const ::basegfx::B2DSize&                            rShadowOffset,
                               const ::Color&                                       rShadowColor,
                               const ::Color&                                       rFillColor,
                               basegfx::B2DPolyPolygon                              xFillPoly,
                               basegfx::B2DPolyPolygon                              xTextPoly,
                               const cpo::uno::Sequence< double >&                       rOffsets,
                               VirtualDevice const &                                rVDev,
                               const OutDevState&                                   rState  );

                OutlineAction(const OutlineAction&) = delete;
                const OutlineAction& operator=(const OutlineAction&) = delete;

                virtual bool render( vclcanvas::Canvas& rCanvas,
                                     const vclcanvas::ViewState& rViewState,
                                     const ::basegfx::B2DHomMatrix& rTransformation ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                // TextRenderer interface
                virtual bool operator()( vclcanvas::Canvas& rCanvas,
                                         const vclcanvas::ViewState& rViewState,
                                         const vclcanvas::RenderState& rRenderState,
                                         const ::Color& rTextFillColor, bool bNormalText ) const override;

                // TODO(P2): This is potentially a real mass object
                // (every character might be a separate TextAction),
                // thus, make it as lightweight as possible. For
                // example, share common RenderState among several
                // TextActions, maybe using maOffsets for the
                // translation.

                basegfx::B2DPolyPolygon                             mxTextPoly;

                const cpo::uno::Sequence< double >                       maOffsets;
                vclcanvas::RenderState                              maState;
                double                                              mnOutlineWidth;
                ::Color                                             maFillColor;
                basegfx::B2DPolyPolygon                             mxBackgroundFillPoly;
                const cppcanvastools::TextLineInfo                           maTextLineInfo;
                ::basegfx::B2DSize                                  maLinesOverallSize;
                basegfx::B2DPolyPolygon                             mxTextLines;
                const ::basegfx::B2DSize                            maReliefOffset;
                const ::Color                                       maReliefColor;
                const ::basegfx::B2DSize                            maShadowOffset;
                const ::Color                                       maShadowColor;
                const ::Color                                       maTextFillColor;
                const ::Color                                       maBackgroundFillColor;
            };

            double calcOutlineWidth( const OutDevState& rState,
                                     VirtualDevice const & rVDev )
            {
                const ::basegfx::B2DSize aFontSize( 0,
                                                    rVDev.GetFont().GetFontHeight() / 64.0 );

                const double nOutlineWidth(
                    (rState.mapModeTransform * aFontSize).getHeight() );

                return nOutlineWidth < 1.0 ? 1.0 : nOutlineWidth;
            }

            OutlineAction::OutlineAction( const ::basegfx::B2DPoint&                            rStartPoint,
                                          const ::basegfx::B2DSize&                             rReliefOffset,
                                          const ::Color&                                        rReliefColor,
                                          const ::basegfx::B2DSize&                             rShadowOffset,
                                          const ::Color&                                        rShadowColor,
                                          const ::Color&                                        rFillColor,
                                          basegfx::B2DPolyPolygon                               xFillPoly,
                                          basegfx::B2DPolyPolygon                               xTextPoly,
                                          const cpo::uno::Sequence< double >&                        rOffsets,
                                          VirtualDevice const &                                 rVDev,
                                          const OutDevState&                                    rState  ) :
                mxTextPoly(std::move( xTextPoly )),
                maOffsets( rOffsets ),
                mnOutlineWidth( calcOutlineWidth(rState,rVDev) ),
                maFillColor( COL_WHITE ),
                mxBackgroundFillPoly(std::move( xFillPoly )),
                maTextLineInfo( cppcanvastools::createTextLineInfo( rVDev, rState ) ),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor ),
                maBackgroundFillColor( rFillColor )
            {
                double nLayoutWidth = 0.0;

                initLayoutWidth(nLayoutWidth, rOffsets);

                initEffectLinePolyPolygon( maLinesOverallSize,
                                           mxTextLines,
                                           nLayoutWidth,
                                           maTextLineInfo );

                init( maState,
                      rStartPoint,
                      rState );
            }

            bool OutlineAction::operator()( vclcanvas::Canvas& rCanvas,
                                            const vclcanvas::ViewState& rViewState,
                                            const vclcanvas::RenderState& rRenderState,
                                            const ::Color& /*rTextFillColor*/, bool /*bNormalText*/ ) const
            {
                if (mxBackgroundFillPoly.count())
                {
                    vclcanvas::RenderState aLocalState( rRenderState );
                    aLocalState.DeviceColor = maBackgroundFillColor;
                    rCanvas.fillPolyPolygon(mxBackgroundFillPoly, rViewState, aLocalState);
                }

                rendering::StrokeAttributes aStrokeAttributes;

                aStrokeAttributes.StrokeWidth  = mnOutlineWidth;
                aStrokeAttributes.MiterLimit   = 1.0;
                aStrokeAttributes.StartCapType = rendering::PathCapType::BUTT;
                aStrokeAttributes.EndCapType   = rendering::PathCapType::BUTT;
                aStrokeAttributes.JoinType     = rendering::PathJoinType::MITER;

                vclcanvas::RenderState aLocalState( rRenderState );
                aLocalState.DeviceColor = maFillColor;

                // TODO(P1): implement caching

                // background of text
                rCanvas.fillPolyPolygon( mxTextPoly,
                                          rViewState,
                                          aLocalState );

                // border line of text
                rCanvas.strokePolyPolygon( mxTextPoly,
                                            rViewState,
                                            rRenderState,
                                            aStrokeAttributes );

                // underlines/strikethrough - background
                rCanvas.fillPolyPolygon( mxTextLines,
                                          rViewState,
                                          aLocalState );
                // underlines/strikethrough - border
                rCanvas.strokePolyPolygon( mxTextLines,
                                            rViewState,
                                            rRenderState,
                                            aStrokeAttributes );

                return true;
            }

            bool OutlineAction::render( vclcanvas::Canvas& rCanvas,
                                        const vclcanvas::ViewState& rViewState,
                                        const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::EffectTextArrayAction::render()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::EffectTextArrayAction: 0x" << std::hex << this );

                vclcanvas::RenderState aLocalState( maState );
                aLocalState.AffineTransform = rTransformation * aLocalState.AffineTransform;

                return renderEffectText( rCanvas, rViewState, *this,
                                         aLocalState,
                                         maShadowColor,
                                         maShadowOffset,
                                         maReliefColor,
                                         maReliefOffset,
                                         maTextFillColor);
            }

#if 0 // see #if'ed out use in OutlineAction::renderSubset below:
            class OutlineTextArrayRenderHelper : public TextRenderer
            {
            public:
                OutlineTextArrayRenderHelper( const uno::Reference< rendering::XCanvas >&        rCanvas,
                                              const basegfx::B2DPolyPolygon& rTextPolygon,
                                              const basegfx::B2DPolyPolygon& rLinePolygon,
                                              const vclcanvas::ViewState&                        rViewState,
                                              double                                             nOutlineWidth ) :
                    maFillColor(::COL_WHITE),
                    mnOutlineWidth( nOutlineWidth ),
                    mrCanvas( rCanvas ),
                    mrTextPolygon( rTextPolygon ),
                    mrLinePolygon( rLinePolygon ),
                    mrViewState( rViewState )
                {
                }

                // TextRenderer interface
                virtual bool operator()( const vclcanvas::RenderState& rRenderState ) const
                {
                    rendering::StrokeAttributes aStrokeAttributes;

                    aStrokeAttributes.StrokeWidth  = mnOutlineWidth;
                    aStrokeAttributes.MiterLimit   = 1.0;
                    aStrokeAttributes.StartCapType = rendering::PathCapType::BUTT;
                    aStrokeAttributes.EndCapType   = rendering::PathCapType::BUTT;
                    aStrokeAttributes.JoinType     = rendering::PathJoinType::MITER;

                    vclcanvas::RenderState aLocalState( rRenderState );
                    aLocalState.DeviceColor = maFillColor;

                    // TODO(P1): implement caching

                    // background of text
                    mrCanvas->fillPolyPolygon( mrTextPolygon,
                                               mrViewState,
                                               aLocalState );

                    // border line of text
                    mrCanvas->strokePolyPolygon( mrTextPolygon,
                                                 mrViewState,
                                                 rRenderState,
                                                 aStrokeAttributes );

                    // underlines/strikethrough - background
                    mrCanvas->fillPolyPolygon( mrLinePolygon,
                                               mrViewState,
                                               aLocalState );
                    // underlines/strikethrough - border
                    mrCanvas->strokePolyPolygon( mrLinePolygon,
                                                 mrViewState,
                                                 rRenderState,
                                                 aStrokeAttributes );

                    return true;
                }

            private:
                const cpo::uno::Sequence< double >                       maFillColor;
                double                                              mnOutlineWidth;
                const uno::Reference< rendering::XCanvas >&         mrCanvas;
                basegfx::B2DPolyPolygon&                            mrTextPolygon;
                basegfx::B2DPolyPolygon&                            mrLinePolygon;
                const vclcanvas::ViewState&                         mrViewState;
            };
#endif

            sal_Int32 OutlineAction::getActionCount() const
            {
                // TODO(F3): Subsetting NYI for outline text!
                return maOffsets.getLength();
            }


            // Action factory methods


            /** Create an outline action

                This method extracts the polygonal outline from the
                text, and creates a properly setup OutlineAction from
                it.
             */
            std::shared_ptr<Action> createOutline( const ::basegfx::B2DPoint&       rStartPoint,
                                                   const ::basegfx::B2DSize&        rReliefOffset,
                                                   const ::Color&                   rReliefColor,
                                                   const ::basegfx::B2DSize&        rShadowOffset,
                                                   const ::Color&                   rShadowColor,
                                                   const ::Color&                   rTextFillColor,
                                                   const OUString&                  rText,
                                                   sal_Int32                        nStartPos,
                                                   sal_Int32                        nLen,
                                                   KernArraySpan                    pDXArray,
                                                   std::span<const bool>            pKashidaArray,
                                                   VirtualDevice&                   rVDev,
                                                   const OutDevState&               rState  )
            {
                // operate on raw DX array here (in logical coordinate
                // system), to have a higher resolution
                // PolyPolygon. That polygon is then converted to
                // device coordinate system.

                // #i68512# Temporarily switch off font rotation
                // (which is already contained in the render state
                // transformation matrix - otherwise, glyph polygons
                // will be rotated twice)
                const vcl::Font aOrigFont( rVDev.GetFont() );
                vcl::Font       aUnrotatedFont( aOrigFont );
                aUnrotatedFont.SetOrientation(0_deg10);
                rVDev.SetFont( aUnrotatedFont );

                // TODO(F3): Don't understand parameter semantics of
                // GetTextOutlines()
                ::basegfx::B2DPolyPolygon aResultingPolyPolygon;
                PolyPolyVector aVCLPolyPolyVector;
                const bool bHaveOutlines( rVDev.GetTextOutlines( aVCLPolyPolyVector, rText,
                                                                 static_cast<sal_uInt16>(nStartPos),
                                                                 static_cast<sal_uInt16>(nStartPos),
                                                                 static_cast<sal_uInt16>(nLen),
                                                                 0, pDXArray, pKashidaArray ) );
                rVDev.SetFont(aOrigFont);

                if( !bHaveOutlines )
                    return std::shared_ptr<Action>();

                // remove offsetting from mapmode transformation
                // (outline polygons must stay at origin, only need to
                // be scaled)
                ::basegfx::B2DHomMatrix aMapModeTransform(
                    rState.mapModeTransform );
                aMapModeTransform.set(0,2, 0.0);
                aMapModeTransform.set(1,2, 0.0);

                for( const auto& rVCLPolyPolygon : aVCLPolyPolyVector )
                {
                    ::basegfx::B2DPolyPolygon aPolyPolygon = rVCLPolyPolygon.getB2DPolyPolygon();
                    aPolyPolygon.transform( aMapModeTransform );

                    // append result to collecting polypoly
                    for( sal_uInt32 i=0; i<aPolyPolygon.count(); ++i )
                    {
                        // #i47795# Ensure closed polygons (since
                        // FreeType returns the glyph outlines
                        // open)
                        const ::basegfx::B2DPolygon& rPoly( aPolyPolygon.getB2DPolygon( i ) );
                        const sal_uInt32 nCount( rPoly.count() );
                        if( nCount<3 ||
                            rPoly.isClosed() )
                        {
                            // polygon either degenerate, or
                            // already closed.
                            aResultingPolyPolygon.append( rPoly );
                        }
                        else
                        {
                            ::basegfx::B2DPolygon aPoly(rPoly);
                            aPoly.setClosed(true);

                            aResultingPolyPolygon.append( aPoly );
                        }
                    }
                }

                const cpo::uno::Sequence< double > aCharWidthSeq(
                    !pDXArray.empty() ?
                    setupDXArray( pDXArray, nLen, rState ) :
                    setupDXArray( rText,
                                  nStartPos,
                                  nLen,
                                  rVDev,
                                  rState ));

                // create background color fill polygon?
                basegfx::B2DPolyPolygon xTextBoundsPoly;
                if (rTextFillColor != COL_AUTO)
                {
                    rendering::StringContext aStringContext( rText, nStartPos, nLen );
                    rtl::Reference<vclcanvas::TextLayout> xTextLayout(
                        rState.xFont->createTextLayout(
                            aStringContext,
                            rState.textDirection,
                            0 ) );

                    auto aTextBounds = xTextLayout->queryTextBounds();
                    auto aB2DBounds = ::basegfx::unotools::b2DRectangleFromRealRectangle2D(aTextBounds);
                    auto aTextBoundsPoly = ::basegfx::utils::createPolygonFromRect(aB2DBounds);
                    xTextBoundsPoly = basegfx::B2DPolyPolygon(aTextBoundsPoly);
                }

                return std::make_shared<OutlineAction>(
                        rStartPoint,
                        rReliefOffset,
                        rReliefColor,
                        rShadowOffset,
                        rShadowColor,
                        rTextFillColor,
                        xTextBoundsPoly,
                        aResultingPolyPolygon,
                        aCharWidthSeq,
                        rVDev,
                        rState  );
            }

        } // namespace


        std::shared_ptr<Action> TextActionFactory::createTextAction( const ::Point&                 rStartPoint,
                                                             const ::Size&                  rReliefOffset,
                                                             const ::Color&                 rReliefColor,
                                                             const ::Size&                  rShadowOffset,
                                                             const ::Color&                 rShadowColor,
                                                             const ::Color&                 rTextFillColor,
                                                             const OUString&                rText,
                                                             sal_Int32                      nStartPos,
                                                             sal_Int32                      nLen,
                                                             KernArraySpan                  pDXArray,
                                                             std::span<const bool>          pKashidaArray,
                                                             VirtualDevice&                 rVDev,
                                                             const OutDevState&             rState,
                                                             vclcanvas::Canvas& rCanvas    )
        {
            const ::Size  aBaselineOffset( cppcanvastools::getBaselineOffset( rState,
                                                                     rVDev ) );
            // #143885# maintain (nearly) full precision positioning,
            // by circumventing integer-based OutDev-mapping
            const ::basegfx::B2DPoint aStartPoint(
                rState.mapModeTransform *
                ::basegfx::B2DPoint(rStartPoint.X() + aBaselineOffset.Width(),
                                    rStartPoint.Y() + aBaselineOffset.Height()) );

            const ::basegfx::B2DSize aReliefOffset(
                rState.mapModeTransform * vcl::unotools::b2DSizeFromSize( rReliefOffset ) );
            const ::basegfx::B2DSize aShadowOffset(
                rState.mapModeTransform * vcl::unotools::b2DSizeFromSize( rShadowOffset ) );

            if( rState.isTextOutlineModeSet )
            {
                return createOutline(
                            aStartPoint,
                            aReliefOffset,
                            rReliefColor,
                            aShadowOffset,
                            rShadowColor,
                            rTextFillColor,
                            rText,
                            nStartPos,
                            nLen,
                            pDXArray,
                            pKashidaArray,
                            rVDev,
                            rState );
            }

            // convert DX array to device coordinate system (and
            // create it in the first place, if pDXArray is NULL)
            const cpo::uno::Sequence< double > aCharWidths(
                !pDXArray.empty() ?
                setupDXArray( pDXArray, nLen, rState ) :
                setupDXArray( rText,
                              nStartPos,
                              nLen,
                              rVDev,
                              rState ));

            const cpo::uno::Sequence< bool > aKashidas(pKashidaArray.data(), pKashidaArray.size());

            // determine type of text action to create
            // =======================================

            const ::Color aEmptyColor( COL_AUTO );

            std::shared_ptr<Action> ret;

            // DX array necessary - any effects?
            if( !rState.textOverlineStyle &&
                !rState.textUnderlineStyle &&
                !rState.textStrikeoutStyle &&
                rReliefColor == aEmptyColor &&
                rShadowColor == aEmptyColor &&
                rTextFillColor == aEmptyColor )
            {
                // nope
                ret = std::make_shared<TextArrayAction>(
                                            aStartPoint,
                                            rText,
                                            nStartPos,
                                            nLen,
                                            aCharWidths,
                                            aKashidas,
                                            rState,
                                            rCanvas );
            }
            else
            {
                // at least one of the effects requested
                ret = std::make_shared<EffectTextArrayAction>(
                                            aStartPoint,
                                            aReliefOffset,
                                            rReliefColor,
                                            aShadowOffset,
                                            rShadowColor,
                                            rTextFillColor,
                                            rText,
                                            nStartPos,
                                            nLen,
                                            aCharWidths,
                                            aKashidas,
                                            rVDev,
                                            rState,
                                            rCanvas );
            }
            return ret;
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
