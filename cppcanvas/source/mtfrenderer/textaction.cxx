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


#include <tools/diagnose_ex.h>

#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/virdev.hxx>

#include <basegfx/utils/canvastools.hxx>
#include <canvas/canvastools.hxx>
#include <memory>
#include <sal/log.hxx>

#include "textaction.hxx"
#include "textlineshelper.hxx"
#include <outdevstate.hxx>
#include "mtftools.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        namespace
        {
            void init( rendering::RenderState&                  o_rRenderState,
                       const ::basegfx::B2DPoint&               rStartPoint,
                       const OutDevState&                       rState,
                       const CanvasSharedPtr&                   rCanvas      )
            {
                tools::initRenderState(o_rRenderState,rState);

                // #i36950# Offset clip back to origin (as it's also moved
                // by rStartPoint)
                // #i53964# Also take VCL font rotation into account,
                // since this, opposed to the FontMatrix rotation
                // elsewhere, _does_ get incorporated into the render
                // state transform.
                tools::modifyClip( o_rRenderState,
                                   rState,
                                   rCanvas,
                                   rStartPoint,
                                   nullptr,
                                   &rState.fontRotation );

                basegfx::B2DHomMatrix aLocalTransformation(basegfx::utils::createRotateB2DHomMatrix(rState.fontRotation));
                aLocalTransformation.translate( rStartPoint.getX(),
                                                rStartPoint.getY() );
                ::canvas::tools::appendToRenderState( o_rRenderState,
                                                      aLocalTransformation );

                o_rRenderState.DeviceColor = rState.textColor;
            }

            void init( rendering::RenderState&                  o_rRenderState,
                       const ::basegfx::B2DPoint&               rStartPoint,
                       const OutDevState&                       rState,
                       const CanvasSharedPtr&                   rCanvas,
                       const ::basegfx::B2DHomMatrix&           rTextTransform  )
            {
                init( o_rRenderState, rStartPoint, rState, rCanvas );

                // TODO(F2): Also inversely-transform clip with
                // rTextTransform (which is actually rather hard, as the
                // text transform is _prepended_ to the render state)!

                // prepend extra font transform to render state
                // (prepend it, because it's interpreted in the unit
                // rect coordinate space)
                ::canvas::tools::prependToRenderState( o_rRenderState,
                                                       rTextTransform );
            }

            void init( rendering::RenderState&                      o_rRenderState,
                       uno::Reference< rendering::XCanvasFont >&    o_rFont,
                       const ::basegfx::B2DPoint&                   rStartPoint,
                       const OutDevState&                           rState,
                       const CanvasSharedPtr&                       rCanvas      )
            {
                // ensure that o_rFont is valid. It is possible that
                // text actions are generated without previously
                // setting a font. Then, just take a default font
                if( !o_rFont.is() )
                {
                    // Use completely default FontRequest
                    const rendering::FontRequest aFontRequest;

                    geometry::Matrix2D aFontMatrix;
                    ::canvas::tools::setIdentityMatrix2D( aFontMatrix );

                    o_rFont = rCanvas->getUNOCanvas()->createFont(
                        aFontRequest,
                        uno::Sequence< beans::PropertyValue >(),
                        aFontMatrix );
                }

                init( o_rRenderState,
                      rStartPoint,
                      rState,
                      rCanvas );
            }

            void init( rendering::RenderState&                      o_rRenderState,
                       uno::Reference< rendering::XCanvasFont >&    o_rFont,
                       const ::basegfx::B2DPoint&                   rStartPoint,
                       const OutDevState&                           rState,
                       const CanvasSharedPtr&                       rCanvas,
                       const ::basegfx::B2DHomMatrix&               rTextTransform  )
            {
                init( o_rRenderState, o_rFont, rStartPoint, rState, rCanvas );

                // TODO(F2): Also inversely-transform clip with
                // rTextTransform (which is actually rather hard, as the
                // text transform is _prepended_ to the render state)!

                // prepend extra font transform to render state
                // (prepend it, because it's interpreted in the unit
                // rect coordinate space)
                ::canvas::tools::prependToRenderState( o_rRenderState,
                                                       rTextTransform );
            }

            void initLayoutWidth(double& rLayoutWidth, const uno::Sequence<double>& rOffsets)
            {
                ENSURE_OR_THROW(rOffsets.getLength(),
                                  "::cppcanvas::internal::initLayoutWidth(): zero-length array" );
                rLayoutWidth = *(std::max_element(rOffsets.begin(), rOffsets.end()));
            }

            uno::Sequence< double > setupDXArray( const long*   pCharWidths,
                                                  sal_Int32          nLen,
                                                  const OutDevState& rState )
            {
                // convert character widths from logical units
                uno::Sequence< double > aCharWidthSeq( nLen );
                double*                 pOutputWidths( aCharWidthSeq.getArray() );

                // #143885# maintain (nearly) full precision of DX
                // array, by circumventing integer-based
                // OutDev-mapping
                const double nScale( rState.mapModeTransform.get(0,0) );
                for( int i = 0; i < nLen; ++i )
                {
                    // TODO(F2): use correct scale direction
                    *pOutputWidths++ = *pCharWidths++ * nScale;
                }

                return aCharWidthSeq;
            }

            uno::Sequence< double > setupDXArray( const OUString&    rText,
                                                  sal_Int32          nStartPos,
                                                  sal_Int32          nLen,
                                                  VirtualDevice const & rVDev,
                                                  const OutDevState& rState )
            {
                // no external DX array given, create one from given
                // string
                std::unique_ptr< long []> pCharWidths( new long[nLen] );

                rVDev.GetTextArray( rText, pCharWidths.get(),
                                    nStartPos, nLen );

                return setupDXArray( pCharWidths.get(), nLen, rState );
            }

            ::basegfx::B2DPoint adaptStartPoint( const ::basegfx::B2DPoint&     rStartPoint,
                                                 const OutDevState&             rState,
                                                 const uno::Sequence< double >& rOffsets )
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

                This method creates the XTextLayout object and
                initializes it, e.g. with the logical advancements.
             */
            void initArrayAction( rendering::RenderState&                   o_rRenderState,
                                  uno::Reference< rendering::XTextLayout >& o_rTextLayout,
                                  const ::basegfx::B2DPoint&                rStartPoint,
                                  const OUString&                    rText,
                                  sal_Int32                                 nStartPos,
                                  sal_Int32                                 nLen,
                                  const uno::Sequence< double >&            rOffsets,
                                  const CanvasSharedPtr&                    rCanvas,
                                  const OutDevState&                        rState,
                                  const ::basegfx::B2DHomMatrix*            pTextTransform )
            {
                ENSURE_OR_THROW( rOffsets.getLength(),
                                  "::cppcanvas::internal::initArrayAction(): zero-length DX array" );

                const ::basegfx::B2DPoint aLocalStartPoint(
                    adaptStartPoint( rStartPoint, rState, rOffsets ) );

                uno::Reference< rendering::XCanvasFont > xFont( rState.xFont );

                if( pTextTransform )
                    init( o_rRenderState, xFont, aLocalStartPoint, rState, rCanvas, *pTextTransform );
                else
                    init( o_rRenderState, xFont, aLocalStartPoint, rState, rCanvas );

                o_rTextLayout = xFont->createTextLayout(
                    rendering::StringContext( rText, nStartPos, nLen ),
                    rState.textDirection,
                    0 );

                ENSURE_OR_THROW( o_rTextLayout.is(),
                                  "::cppcanvas::internal::initArrayAction(): Invalid font" );

                o_rTextLayout->applyLogicalAdvancements( rOffsets );

            }

            double getLineWidth( ::VirtualDevice const &         rVDev,
                                 const OutDevState&              rState,
                                 const rendering::StringContext& rStringContext )
            {
                // TODO(F2): use correct scale direction
                const ::basegfx::B2DSize aSize( rVDev.GetTextWidth( rStringContext.Text,
                                                                    static_cast<sal_uInt16>(rStringContext.StartPosition),
                                                                    static_cast<sal_uInt16>(rStringContext.Length) ),
                                    0 );

                return (rState.mapModeTransform * aSize).getX();
            }

            uno::Sequence< double >
                calcSubsetOffsets( rendering::RenderState&                          io_rRenderState,
                                   double&                                          o_rMinPos,
                                   double&                                          o_rMaxPos,
                                   const uno::Reference< rendering::XTextLayout >&  rOrigTextLayout,
                                   double                                           nLayoutWidth,
                                   const ::cppcanvas::internal::Action::Subset&     rSubset )
            {
                ENSURE_OR_THROW( rSubset.mnSubsetEnd > rSubset.mnSubsetBegin,
                                  "::cppcanvas::internal::calcSubsetOffsets(): invalid subset range range" );

                uno::Sequence< double > aOrigOffsets( rOrigTextLayout->queryLogicalAdvancements() );
                const double*           pOffsets( aOrigOffsets.getConstArray() );

                ENSURE_OR_THROW( aOrigOffsets.getLength() >= rSubset.mnSubsetEnd,
                                  "::cppcanvas::internal::calcSubsetOffsets(): invalid subset range range" );


                // determine leftmost position in given subset range -
                // as the DX array contains the output positions
                // starting with the second character (the first is
                // assumed to have output position 0), correct begin
                // iterator.
                const double nMinPos( rSubset.mnSubsetBegin <= 0 ? 0 :
                                      *(std::min_element( pOffsets+rSubset.mnSubsetBegin-1,
                                                            pOffsets+rSubset.mnSubsetEnd )) );

                // determine rightmost position in given subset range
                // - as the DX array contains the output positions
                // starting with the second character (the first is
                // assumed to have output position 0), correct begin
                // iterator.
                const double nMaxPos(
                    *(std::max_element( pOffsets + (rSubset.mnSubsetBegin <= 0 ?
                                                      0 : rSubset.mnSubsetBegin-1),
                                          pOffsets + rSubset.mnSubsetEnd )) );

                // Logical advancements always increase in logical text order.
                // For RTL text, nMaxPos is the distance from the right edge to
                // the leftmost position in the subset, so we have to convert
                // it to the offset from the origin (i.e. left edge ).
                // LTR: |---- min --->|---- max --->|            |
                // RTL: |             |<--- max ----|<--- min ---|
                //      |<- nOffset ->|                          |
                const double nOffset = rOrigTextLayout->getMainTextDirection()
                    ? nLayoutWidth - nMaxPos : nMinPos;


                // adapt render state, to move text output to given offset


                // TODO(F1): Strictly speaking, we also have to adapt
                // the clip here, which normally should _not_ move
                // with the output offset. Neglected for now, as it
                // does not matter for drawing layer output

                if (nOffset > 0.0)
                {
                    ::basegfx::B2DHomMatrix aTranslation;
                    if( rOrigTextLayout->getFont()->getFontRequest().FontDescription.IsVertical == css::util::TriState_YES )
                    {
                        // vertical text -> offset in y direction
                        aTranslation.translate(0.0, nOffset);
                    }
                    else
                    {
                        // horizontal text -> offset in x direction
                        aTranslation.translate(nOffset, 0.0);
                    }

                    ::canvas::tools::appendToRenderState( io_rRenderState,
                                                          aTranslation );
                }


                // reduce DX array to given substring


                const sal_Int32         nNewElements( rSubset.mnSubsetEnd - rSubset.mnSubsetBegin );
                uno::Sequence< double > aAdaptedOffsets( nNewElements );
                double*                 pAdaptedOffsets( aAdaptedOffsets.getArray() );

                // move to new output position (subtract nMinPos,
                // which is the new '0' position), copy only the range
                // as given by rSubset.
                std::transform( pOffsets + rSubset.mnSubsetBegin,
                                  pOffsets + rSubset.mnSubsetEnd,
                                  pAdaptedOffsets,
                                  [nMinPos](double aPos) { return aPos - nMinPos; } );

                o_rMinPos = nMinPos;
                o_rMaxPos = nMaxPos;

                return aAdaptedOffsets;
            }

            uno::Reference< rendering::XTextLayout >
                createSubsetLayout( const rendering::StringContext&                 rOrigContext,
                                    const ::cppcanvas::internal::Action::Subset&    rSubset,
                                    const uno::Reference< rendering::XTextLayout >& rOrigTextLayout )
            {
                // create temporary new text layout with subset string


                const sal_Int32 nNewStartPos( rOrigContext.StartPosition + std::min(
                                                  rSubset.mnSubsetBegin, rOrigContext.Length-1 ) );
                const sal_Int32 nNewLength( std::max(
                                                std::min(
                                                    rSubset.mnSubsetEnd - rSubset.mnSubsetBegin,
                                                    rOrigContext.Length ),
                                                sal_Int32( 0 ) ) );

                const rendering::StringContext aContext( rOrigContext.Text,
                                                         nNewStartPos,
                                                         nNewLength );

                uno::Reference< rendering::XTextLayout > xTextLayout(
                    rOrigTextLayout->getFont()->createTextLayout( aContext,
                                                                  rOrigTextLayout->getMainTextDirection(),
                                                                  0 ),
                    uno::UNO_QUERY_THROW );

                return xTextLayout;
            }

            /** Setup subset text layout

                @param io_rTextLayout
                Must contain original (full set) text layout on input,
                will contain subsetted text layout (or empty
                reference, for empty subsets) on output.

                @param io_rRenderState
                Must contain original render state on input, will
                contain shifted render state concatenated with
                rTransformation on output.

                @param rTransformation
                Additional transformation, to be prepended to render
                state

                @param rSubset
                Subset to prepare
             */
            void createSubsetLayout( uno::Reference< rendering::XTextLayout >&  io_rTextLayout,
                                     double                                     nLayoutWidth,
                                     rendering::RenderState&                    io_rRenderState,
                                     double&                                    o_rMinPos,
                                     double&                                    o_rMaxPos,
                                     const ::basegfx::B2DHomMatrix&             rTransformation,
                                     const Action::Subset&                      rSubset )
            {
                ::canvas::tools::prependToRenderState(io_rRenderState, rTransformation);

                if( rSubset.mnSubsetBegin == rSubset.mnSubsetEnd )
                {
                     // empty range, empty layout
                    io_rTextLayout.clear();

                    return;
                }

                ENSURE_OR_THROW( io_rTextLayout.is(),
                                  "createSubsetLayout(): Invalid input layout" );

                const rendering::StringContext& rOrigContext( io_rTextLayout->getText() );

                if( rSubset.mnSubsetBegin == 0 &&
                    rSubset.mnSubsetEnd == rOrigContext.Length )
                {
                    // full range, no need for subsetting
                    return;
                }

                uno::Reference< rendering::XTextLayout > xTextLayout(
                    createSubsetLayout( rOrigContext, rSubset, io_rTextLayout ) );

                if( xTextLayout.is() )
                {
                    xTextLayout->applyLogicalAdvancements(
                        calcSubsetOffsets( io_rRenderState,
                                           o_rMinPos,
                                           o_rMaxPos,
                                           io_rTextLayout,
                                           nLayoutWidth,
                                           rSubset ) );
                }

                io_rTextLayout = xTextLayout;
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
                virtual bool operator()( const rendering::RenderState& rRenderState, const ::Color& rTextFillColor, bool bNormalText ) const = 0;
            };

            /** Render effect text.

                @param rRenderer
                Functor object, will be called to render the actual
                part of the text effect (the text itself and the means
                to render it are unknown to this method)
             */
            bool renderEffectText( const TextRenderer&                          rRenderer,
                                   const rendering::RenderState&                rRenderState,
                                   const uno::Reference< rendering::XCanvas >&  xCanvas,
                                   const ::Color&                               rShadowColor,
                                   const ::basegfx::B2DSize&                    rShadowOffset,
                                   const ::Color&                               rReliefColor,
                                   const ::basegfx::B2DSize&                    rReliefOffset,
                                   const ::Color&                               rTextFillColor )
            {
                ::Color aEmptyColor( COL_AUTO );
                uno::Reference<rendering::XColorSpace> xColorSpace(
                    xCanvas->getDevice()->getDeviceColorSpace() );

                // draw shadow text, if enabled
                if( rShadowColor != aEmptyColor )
                {
                    rendering::RenderState aShadowState( rRenderState );
                    ::basegfx::B2DHomMatrix aTranslate;

                    aTranslate.translate( rShadowOffset.getX(),
                                          rShadowOffset.getY() );

                    ::canvas::tools::appendToRenderState(aShadowState, aTranslate);

                    aShadowState.DeviceColor =
                        vcl::unotools::colorToDoubleSequence( rShadowColor,
                                                                xColorSpace );

                    rRenderer( aShadowState, rTextFillColor, false );
                }

                // draw relief text, if enabled
                if( rReliefColor != aEmptyColor )
                {
                    rendering::RenderState aReliefState( rRenderState );
                    ::basegfx::B2DHomMatrix aTranslate;

                    aTranslate.translate( rReliefOffset.getX(),
                                          rReliefOffset.getY() );

                    ::canvas::tools::appendToRenderState(aReliefState, aTranslate);

                    aReliefState.DeviceColor =
                        vcl::unotools::colorToDoubleSequence( rReliefColor,
                                                                xColorSpace );

                    rRenderer( aReliefState, rTextFillColor, false );
                }

                // draw normal text
                rRenderer( rRenderState, rTextFillColor, true );

                return true;
            }


            ::basegfx::B2DRange calcEffectTextBounds( const ::basegfx::B2DRange&    rTextBounds,
                                                      const ::basegfx::B2DRange&    rLineBounds,
                                                      const ::basegfx::B2DSize&     rReliefOffset,
                                                      const ::basegfx::B2DSize&     rShadowOffset,
                                                      const rendering::RenderState& rRenderState,
                                                      const rendering::ViewState&   rViewState )
            {
                ::basegfx::B2DRange aBounds( rTextBounds );

                // add extends of text lines
                aBounds.expand( rLineBounds );

                // TODO(Q3): Provide this functionality at the B2DRange
                ::basegfx::B2DRange aTotalBounds( aBounds );
                aTotalBounds.expand(
                    ::basegfx::B2DRange( aBounds.getMinX() + rReliefOffset.getX(),
                                         aBounds.getMinY() + rReliefOffset.getY(),
                                         aBounds.getMaxX() + rReliefOffset.getX(),
                                         aBounds.getMaxY() + rReliefOffset.getY() ) );
                aTotalBounds.expand(
                    ::basegfx::B2DRange( aBounds.getMinX() + rShadowOffset.getX(),
                                         aBounds.getMinY() + rShadowOffset.getY(),
                                         aBounds.getMaxX() + rShadowOffset.getX(),
                                         aBounds.getMaxY() + rShadowOffset.getY() ) );

                return tools::calcDevicePixelBounds( aTotalBounds,
                                                     rViewState,
                                                     rRenderState );
            }

            void initEffectLinePolyPolygon( ::basegfx::B2DSize&                             o_rOverallSize,
                                            uno::Reference< rendering::XPolyPolygon2D >&    o_rTextLines,
                                            const CanvasSharedPtr&                          rCanvas,
                                            double                                          nLineWidth,
                                            const tools::TextLineInfo&                      rLineInfo   )
            {
                const ::basegfx::B2DPolyPolygon aPoly(
                    tools::createTextLinesPolyPolygon( 0.0, nLineWidth,
                                                       rLineInfo ) );

                o_rOverallSize = ::basegfx::utils::getRange( aPoly ).getRange();

                o_rTextLines = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    rCanvas->getUNOCanvas()->getDevice(),
                    aPoly );
            }


            class TextAction : public Action
            {
            public:
                TextAction( const ::basegfx::B2DPoint&  rStartPoint,
                            const OUString&      rString,
                            sal_Int32                   nStartPos,
                            sal_Int32                   nLen,
                            const CanvasSharedPtr&      rCanvas,
                            const OutDevState&          rState );

                TextAction( const ::basegfx::B2DPoint&      rStartPoint,
                            const OUString&          rString,
                            sal_Int32                       nStartPos,
                            sal_Int32                       nLen,
                            const CanvasSharedPtr&          rCanvas,
                            const OutDevState&              rState,
                            const ::basegfx::B2DHomMatrix&  rTextTransform );

                TextAction(const TextAction&) = delete;
                const TextAction& operator=(const TextAction&) = delete;

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual bool renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                // TODO(P2): This is potentially a real mass object
                // (every character might be a separate TextAction),
                // thus, make it as lightweight as possible. For
                // example, share common RenderState among several
                // TextActions, maybe using maOffsets for the
                // translation.

                uno::Reference< rendering::XCanvasFont >    mxFont;
                const rendering::StringContext              maStringContext;
                const CanvasSharedPtr                       mpCanvas;
                rendering::RenderState                      maState;
                const sal_Int8                              maTextDirection;
            };

            TextAction::TextAction( const ::basegfx::B2DPoint&  rStartPoint,
                                    const OUString&      rString,
                                    sal_Int32                   nStartPos,
                                    sal_Int32                   nLen,
                                    const CanvasSharedPtr&      rCanvas,
                                    const OutDevState&          rState  ) :
                mxFont( rState.xFont ),
                maStringContext( rString, nStartPos, nLen ),
                mpCanvas( rCanvas ),
                maState(),
                maTextDirection( rState.textDirection )
            {
                init( maState, mxFont,
                      rStartPoint,
                      rState, rCanvas );

                ENSURE_OR_THROW( mxFont.is(),
                                  "::cppcanvas::internal::TextAction(): Invalid font" );
            }

            TextAction::TextAction( const ::basegfx::B2DPoint&      rStartPoint,
                                    const OUString&          rString,
                                    sal_Int32                       nStartPos,
                                    sal_Int32                       nLen,
                                    const CanvasSharedPtr&          rCanvas,
                                    const OutDevState&              rState,
                                    const ::basegfx::B2DHomMatrix&  rTextTransform ) :
                mxFont( rState.xFont ),
                maStringContext( rString, nStartPos, nLen ),
                mpCanvas( rCanvas ),
                maState(),
                maTextDirection( rState.textDirection )
            {
                init( maState, mxFont,
                      rStartPoint,
                      rState, rCanvas, rTextTransform );

                ENSURE_OR_THROW( mxFont.is(),
                                  "::cppcanvas::internal::TextAction(): Invalid font" );
            }

            bool TextAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::TextAction::render()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::TextAction: 0x" << std::hex << this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                mpCanvas->getUNOCanvas()->drawText( maStringContext, mxFont,
                                                    mpCanvas->getViewState(), aLocalState, maTextDirection );

                return true;
            }

            bool TextAction::renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  /*rSubset*/ ) const
            {
                SAL_WARN( "cppcanvas.emf", "TextAction::renderSubset(): Subset not supported by this object" );

                // TODO(P1): Retrieve necessary font metric info for
                // TextAction from XCanvas. Currently, the
                // TextActionFactory does not generate this object for
                // _subsettable_ text
                return render( rTransformation );
            }

            ::basegfx::B2DRange TextAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                // create XTextLayout, to have the
                // XTextLayout::queryTextBounds() method available
                uno::Reference< rendering::XTextLayout > xTextLayout(
                    mxFont->createTextLayout(
                        maStringContext,
                        maTextDirection,
                        0 ) );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return tools::calcDevicePixelBounds( ::basegfx::unotools::b2DRectangleFromRealRectangle2D(
                                                         xTextLayout->queryTextBounds() ),
                                                     mpCanvas->getViewState(),
                                                     aLocalState );
            }

            ::basegfx::B2DRange TextAction::getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    /*rSubset*/ ) const
            {
                SAL_WARN( "cppcanvas.emf", "TextAction::getBounds(): Subset not supported by this object" );

                // TODO(P1): Retrieve necessary font metric info for
                // TextAction from XCanvas. Currently, the
                // TextActionFactory does not generate this object for
                // _subsettable_ text
                return getBounds( rTransformation );
            }

            sal_Int32 TextAction::getActionCount() const
            {
                // TODO(P1): Retrieve necessary font metric info for
                // TextAction from XCanvas. Currently, the
                // TextActionFactory does not generate this object for
                // _subsettable_ text
                return 1;
            }


            class EffectTextAction :
                public Action,
                public TextRenderer
            {
            public:
                EffectTextAction( const ::basegfx::B2DPoint& rStartPoint,
                                  const ::basegfx::B2DSize&  rReliefOffset,
                                  const ::Color&             rReliefColor,
                                  const ::basegfx::B2DSize&  rShadowOffset,
                                  const ::Color&             rShadowColor,
                                  const OUString&     rText,
                                  sal_Int32                  nStartPos,
                                  sal_Int32                  nLen,
                                  VirtualDevice const &      rVDev,
                                  const CanvasSharedPtr&     rCanvas,
                                  const OutDevState&         rState );

                EffectTextAction( const ::basegfx::B2DPoint&        rStartPoint,
                                  const ::basegfx::B2DSize&         rReliefOffset,
                                  const ::Color&                    rReliefColor,
                                  const ::basegfx::B2DSize&         rShadowOffset,
                                  const ::Color&                    rShadowColor,
                                  const OUString&            rText,
                                  sal_Int32                         nStartPos,
                                  sal_Int32                         nLen,
                                  VirtualDevice const &             rVDev,
                                  const CanvasSharedPtr&            rCanvas,
                                  const OutDevState&                rState,
                                  const ::basegfx::B2DHomMatrix&    rTextTransform );

                EffectTextAction(const EffectTextAction&) = delete;
                const EffectTextAction& operator=(const EffectTextAction&) = delete;

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual bool renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                /// Interface TextRenderer
                virtual bool operator()( const rendering::RenderState& rRenderState, const ::Color& rTextFillColor, bool bNormalText ) const override;

                geometry::RealRectangle2D queryTextBounds() const;
                css::uno::Reference<css::rendering::XPolyPolygon2D> queryTextBounds(const uno::Reference<rendering::XCanvas>& rCanvas) const;

                // TODO(P2): This is potentially a real mass object
                // (every character might be a separate TextAction),
                // thus, make it as lightweight as possible. For
                // example, share common RenderState among several
                // TextActions, maybe using maOffsets for the
                // translation.

                uno::Reference< rendering::XCanvasFont >    mxFont;
                const rendering::StringContext              maStringContext;
                const CanvasSharedPtr                       mpCanvas;
                rendering::RenderState                      maState;
                const tools::TextLineInfo                   maTextLineInfo;
                ::basegfx::B2DSize                          maLinesOverallSize;
                uno::Reference< rendering::XPolyPolygon2D > mxTextLines;
                const ::basegfx::B2DSize                    maReliefOffset;
                const ::Color                               maReliefColor;
                const ::basegfx::B2DSize                    maShadowOffset;
                const ::Color                               maShadowColor;
                const ::Color                               maTextFillColor;
                const sal_Int8                              maTextDirection;
            };

            EffectTextAction::EffectTextAction( const ::basegfx::B2DPoint& rStartPoint,
                                                const ::basegfx::B2DSize&  rReliefOffset,
                                                const ::Color&             rReliefColor,
                                                const ::basegfx::B2DSize&  rShadowOffset,
                                                const ::Color&             rShadowColor,
                                                const OUString&     rText,
                                                sal_Int32                  nStartPos,
                                                sal_Int32                  nLen,
                                                VirtualDevice const &      rVDev,
                                                const CanvasSharedPtr&     rCanvas,
                                                const OutDevState&         rState ) :
                mxFont( rState.xFont ),
                maStringContext( rText, nStartPos, nLen ),
                mpCanvas( rCanvas ),
                maState(),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maLinesOverallSize(),
                mxTextLines(),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor ),
                maTextDirection( rState.textDirection )
            {
                const double nLineWidth(getLineWidth( rVDev, rState, maStringContext ));
                initEffectLinePolyPolygon( maLinesOverallSize,
                                           mxTextLines,
                                           rCanvas,
                                           nLineWidth,
                                           maTextLineInfo );

                init( maState, mxFont,
                      rStartPoint,
                      rState, rCanvas );

                ENSURE_OR_THROW( mxFont.is() && mxTextLines.is(),
                                  "::cppcanvas::internal::EffectTextAction(): Invalid font or lines" );
            }

            EffectTextAction::EffectTextAction( const ::basegfx::B2DPoint&      rStartPoint,
                                                const ::basegfx::B2DSize&       rReliefOffset,
                                                const ::Color&                  rReliefColor,
                                                const ::basegfx::B2DSize&       rShadowOffset,
                                                const ::Color&                  rShadowColor,
                                                const OUString&          rText,
                                                sal_Int32                       nStartPos,
                                                sal_Int32                       nLen,
                                                VirtualDevice const &           rVDev,
                                                const CanvasSharedPtr&          rCanvas,
                                                const OutDevState&              rState,
                                                const ::basegfx::B2DHomMatrix&  rTextTransform ) :
                mxFont( rState.xFont ),
                maStringContext( rText, nStartPos, nLen ),
                mpCanvas( rCanvas ),
                maState(),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maLinesOverallSize(),
                mxTextLines(),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor ),
                maTextDirection( rState.textDirection )
            {
                const double nLineWidth( getLineWidth( rVDev, rState, maStringContext ) );
                initEffectLinePolyPolygon( maLinesOverallSize,
                                           mxTextLines,
                                           rCanvas,
                                           nLineWidth,
                                           maTextLineInfo );

                init( maState, mxFont,
                      rStartPoint,
                      rState, rCanvas, rTextTransform );

                ENSURE_OR_THROW( mxFont.is() && mxTextLines.is(),
                                  "::cppcanvas::internal::EffectTextAction(): Invalid font or lines" );
            }

            bool EffectTextAction::operator()( const rendering::RenderState& rRenderState, const ::Color& rTextFillColor, bool /*bNormalText*/ ) const
            {
                const rendering::ViewState& rViewState( mpCanvas->getViewState() );
                const uno::Reference< rendering::XCanvas >& rCanvas( mpCanvas->getUNOCanvas() );

                rCanvas->fillPolyPolygon( mxTextLines,
                                          rViewState,
                                          rRenderState );

                //rhbz#1589029 non-transparent text fill background support
                if (rTextFillColor != COL_AUTO)
                {
                    rendering::RenderState aLocalState( rRenderState );
                    aLocalState.DeviceColor = vcl::unotools::colorToDoubleSequence(
                        rTextFillColor, rCanvas->getDevice()->getDeviceColorSpace());
                    auto xTextBounds = queryTextBounds(rCanvas);
                    // background of text
                    rCanvas->fillPolyPolygon(xTextBounds, rViewState, aLocalState);
                }

                rCanvas->drawText( maStringContext, mxFont,
                                   rViewState,
                                   rRenderState,
                                   maTextDirection );

                return true;
            }

            bool EffectTextAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::EffectTextAction::render()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::EffectTextAction: 0x" << std::hex << this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return renderEffectText( *this,
                                         aLocalState,
                                         mpCanvas->getUNOCanvas(),
                                         maShadowColor,
                                         maShadowOffset,
                                         maReliefColor,
                                         maReliefOffset,
                                         maTextFillColor);
            }

            bool EffectTextAction::renderSubset( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                 const Subset&                    /*rSubset*/ ) const
            {
                SAL_WARN( "cppcanvas.emf", "EffectTextAction::renderSubset(): Subset not supported by this object" );

                // TODO(P1): Retrieve necessary font metric info for
                // TextAction from XCanvas. Currently, the
                // TextActionFactory does not generate this object for
                // subsettable text
                return render( rTransformation );
            }

            geometry::RealRectangle2D EffectTextAction::queryTextBounds() const
            {
                // create XTextLayout, to have the
                // XTextLayout::queryTextBounds() method available
                uno::Reference< rendering::XTextLayout > xTextLayout(
                    mxFont->createTextLayout(
                        maStringContext,
                        maTextDirection,
                        0 ) );

                return xTextLayout->queryTextBounds();
            }

            css::uno::Reference<css::rendering::XPolyPolygon2D> EffectTextAction::queryTextBounds(const uno::Reference<rendering::XCanvas>& rCanvas) const
            {
                auto aTextBounds = queryTextBounds();
                auto aB2DBounds = ::basegfx::unotools::b2DRectangleFromRealRectangle2D(aTextBounds);
                auto aTextBoundsPoly = ::basegfx::utils::createPolygonFromRect(aB2DBounds);
                return ::basegfx::unotools::xPolyPolygonFromB2DPolygon(rCanvas->getDevice(), aTextBoundsPoly);
            }

            ::basegfx::B2DRange EffectTextAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return calcEffectTextBounds( ::basegfx::unotools::b2DRectangleFromRealRectangle2D(
                                                 queryTextBounds() ),
                                             ::basegfx::B2DRange( 0,0,
                                                                  maLinesOverallSize.getX(),
                                                                  maLinesOverallSize.getY() ),
                                             maReliefOffset,
                                             maShadowOffset,
                                             aLocalState,
                                             mpCanvas->getViewState() );
            }

            ::basegfx::B2DRange EffectTextAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation,
                                                             const Subset&                  /*rSubset*/ ) const
            {
                SAL_WARN( "cppcanvas.emf", "EffectTextAction::getBounds(): Subset not supported by this object" );

                // TODO(P1): Retrieve necessary font metric info for
                // TextAction from XCanvas. Currently, the
                // TextActionFactory does not generate this object for
                // _subsettable_ text
                return getBounds( rTransformation );
            }

            sal_Int32 EffectTextAction::getActionCount() const
            {
                // TODO(P1): Retrieve necessary font metric info for
                // TextAction from XCanvas. Currently, the
                // TextActionFactory does not generate this object for
                // subsettable text
                return 1;
            }


            class TextArrayAction : public Action
            {
            public:
                TextArrayAction( const ::basegfx::B2DPoint&     rStartPoint,
                                 const OUString&         rString,
                                 sal_Int32                      nStartPos,
                                 sal_Int32                      nLen,
                                 const uno::Sequence< double >& rOffsets,
                                 const CanvasSharedPtr&         rCanvas,
                                 const OutDevState&             rState );

                TextArrayAction( const ::basegfx::B2DPoint&     rStartPoint,
                                 const OUString&         rString,
                                 sal_Int32                      nStartPos,
                                 sal_Int32                      nLen,
                                 const uno::Sequence< double >& rOffsets,
                                 const CanvasSharedPtr&         rCanvas,
                                 const OutDevState&             rState,
                                 const ::basegfx::B2DHomMatrix& rTextTransform );

                TextArrayAction(const TextArrayAction&) = delete;
                const TextArrayAction& operator=(const TextArrayAction&) = delete;

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual bool renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                // TODO(P2): This is potentially a real mass object
                // (every character might be a separate TextAction),
                // thus, make it as lightweight as possible. For
                // example, share common RenderState among several
                // TextActions, maybe using maOffsets for the
                // translation.

                uno::Reference< rendering::XTextLayout >    mxTextLayout;
                const CanvasSharedPtr                       mpCanvas;
                rendering::RenderState                      maState;
                double                                      mnLayoutWidth;
            };

            TextArrayAction::TextArrayAction( const ::basegfx::B2DPoint&        rStartPoint,
                                              const OUString&            rString,
                                              sal_Int32                         nStartPos,
                                              sal_Int32                         nLen,
                                              const uno::Sequence< double >&    rOffsets,
                                              const CanvasSharedPtr&            rCanvas,
                                              const OutDevState&                rState ) :
                mxTextLayout(),
                mpCanvas( rCanvas ),
                maState()
            {
                initLayoutWidth(mnLayoutWidth, rOffsets);

                initArrayAction( maState,
                                 mxTextLayout,
                                 rStartPoint,
                                 rString,
                                 nStartPos,
                                 nLen,
                                 rOffsets,
                                 rCanvas,
                                 rState, nullptr );
            }

            TextArrayAction::TextArrayAction( const ::basegfx::B2DPoint&        rStartPoint,
                                              const OUString&            rString,
                                              sal_Int32                         nStartPos,
                                              sal_Int32                         nLen,
                                              const uno::Sequence< double >&    rOffsets,
                                              const CanvasSharedPtr&            rCanvas,
                                              const OutDevState&                rState,
                                              const ::basegfx::B2DHomMatrix&    rTextTransform ) :
                mxTextLayout(),
                mpCanvas( rCanvas ),
                maState()
            {
                initLayoutWidth(mnLayoutWidth, rOffsets);

                initArrayAction( maState,
                                 mxTextLayout,
                                 rStartPoint,
                                 rString,
                                 nStartPos,
                                 nLen,
                                 rOffsets,
                                 rCanvas,
                                 rState,
                                 &rTextTransform );
            }

            bool TextArrayAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::TextArrayAction::render()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::TextArrayAction: 0x" << std::hex << this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                mpCanvas->getUNOCanvas()->drawTextLayout( mxTextLayout,
                                                          mpCanvas->getViewState(),
                                                          aLocalState );

                return true;
            }

            bool TextArrayAction::renderSubset( const ::basegfx::B2DHomMatrix&    rTransformation,
                                                const Subset&                     rSubset ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::TextArrayAction::renderSubset()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::TextArrayAction: 0x" << std::hex << this );

                rendering::RenderState                      aLocalState( maState );
                uno::Reference< rendering::XTextLayout >    xTextLayout( mxTextLayout );

                double nDummy0, nDummy1;
                createSubsetLayout( xTextLayout,
                                    mnLayoutWidth,
                                    aLocalState,
                                    nDummy0,
                                    nDummy1,
                                    rTransformation,
                                    rSubset );

                if( !xTextLayout.is() )
                    return true; // empty layout, render nothing

                mpCanvas->getUNOCanvas()->drawTextLayout( xTextLayout,
                                                          mpCanvas->getViewState(),
                                                          aLocalState );

                return true;
            }

            ::basegfx::B2DRange TextArrayAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return tools::calcDevicePixelBounds( ::basegfx::unotools::b2DRectangleFromRealRectangle2D(
                                                         mxTextLayout->queryTextBounds() ),
                                                     mpCanvas->getViewState(),
                                                     aLocalState );
            }

            ::basegfx::B2DRange TextArrayAction::getBounds( const ::basegfx::B2DHomMatrix&  rTransformation,
                                                            const Subset&                   rSubset ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::TextArrayAction::getBounds( subset )" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::TextArrayAction: 0x" << std::hex << this );

                rendering::RenderState                      aLocalState( maState );
                uno::Reference< rendering::XTextLayout >    xTextLayout( mxTextLayout );

                double nDummy0, nDummy1;
                createSubsetLayout( xTextLayout,
                                    mnLayoutWidth,
                                    aLocalState,
                                    nDummy0,
                                    nDummy1,
                                    rTransformation,
                                    rSubset );

                if( !xTextLayout.is() )
                    return ::basegfx::B2DRange(); // empty layout, empty bounds

                return tools::calcDevicePixelBounds( ::basegfx::unotools::b2DRectangleFromRealRectangle2D(
                                                         xTextLayout->queryTextBounds() ),
                                                     mpCanvas->getViewState(),
                                                     aLocalState );
            }

            sal_Int32 TextArrayAction::getActionCount() const
            {
                const rendering::StringContext& rOrigContext( mxTextLayout->getText() );

                return rOrigContext.Length;
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
                                       const uno::Sequence< double >&   rOffsets,
                                       VirtualDevice const &            rVDev,
                                       const CanvasSharedPtr&           rCanvas,
                                       const OutDevState&               rState  );
                EffectTextArrayAction( const ::basegfx::B2DPoint&       rStartPoint,
                                       const ::basegfx::B2DSize&        rReliefOffset,
                                       const ::Color&                   rReliefColor,
                                       const ::basegfx::B2DSize&        rShadowOffset,
                                       const ::Color&                   rShadowColor,
                                       const ::Color&                   rTextFillColor,
                                       const OUString&           rText,
                                       sal_Int32                        nStartPos,
                                       sal_Int32                        nLen,
                                       const uno::Sequence< double >&   rOffsets,
                                       VirtualDevice const &            rVDev,
                                       const CanvasSharedPtr&           rCanvas,
                                       const OutDevState&               rState,
                                       const ::basegfx::B2DHomMatrix&   rTextTransform );

                EffectTextArrayAction(const EffectTextArrayAction&) = delete;
                const EffectTextArrayAction& operator=(const EffectTextArrayAction&) = delete;

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual bool renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                // TextRenderer interface
                virtual bool operator()( const rendering::RenderState& rRenderState, const ::Color& rTextFillColor, bool bNormalText ) const override;

                css::uno::Reference<css::rendering::XPolyPolygon2D> queryTextBounds(const uno::Reference<rendering::XCanvas>& rCanvas) const;

                // TODO(P2): This is potentially a real mass object
                // (every character might be a separate TextAction),
                // thus, make it as lightweight as possible. For
                // example, share common RenderState among several
                // TextActions, maybe using maOffsets for the
                // translation.

                uno::Reference< rendering::XTextLayout >        mxTextLayout;
                const CanvasSharedPtr                           mpCanvas;
                rendering::RenderState                          maState;
                const tools::TextLineInfo                       maTextLineInfo;
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
                                                          const uno::Sequence< double >&    rOffsets,
                                                          VirtualDevice const &             rVDev,
                                                          const CanvasSharedPtr&            rCanvas,
                                                          const OutDevState&                rState  ) :
                mxTextLayout(),
                mpCanvas( rCanvas ),
                maState(),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maTextLinesHelper(mpCanvas, rState),
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
                                 rCanvas,
                                 rState, nullptr );
            }

            EffectTextArrayAction::EffectTextArrayAction( const ::basegfx::B2DPoint&        rStartPoint,
                                                          const ::basegfx::B2DSize&         rReliefOffset,
                                                          const ::Color&                    rReliefColor,
                                                          const ::basegfx::B2DSize&         rShadowOffset,
                                                          const ::Color&                    rShadowColor,
                                                          const ::Color&                    rTextFillColor,
                                                          const OUString&            rText,
                                                          sal_Int32                         nStartPos,
                                                          sal_Int32                         nLen,
                                                          const uno::Sequence< double >&    rOffsets,
                                                          VirtualDevice const &             rVDev,
                                                          const CanvasSharedPtr&            rCanvas,
                                                          const OutDevState&                rState,
                                                          const ::basegfx::B2DHomMatrix&    rTextTransform ) :
                mxTextLayout(),
                mpCanvas( rCanvas ),
                maState(),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maTextLinesHelper(mpCanvas, rState),
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
                                 rCanvas,
                                 rState,
                                 &rTextTransform );
            }

            css::uno::Reference<css::rendering::XPolyPolygon2D> EffectTextArrayAction::queryTextBounds(const uno::Reference<rendering::XCanvas>& rCanvas) const
            {
                const geometry::RealRectangle2D aTextBounds(mxTextLayout->queryTextBounds());
                auto aB2DBounds = ::basegfx::unotools::b2DRectangleFromRealRectangle2D(aTextBounds);
                auto aTextBoundsPoly = ::basegfx::utils::createPolygonFromRect(aB2DBounds);
                return ::basegfx::unotools::xPolyPolygonFromB2DPolygon(rCanvas->getDevice(), aTextBoundsPoly);
            }

            bool EffectTextArrayAction::operator()( const rendering::RenderState& rRenderState, const ::Color& rTextFillColor, bool bNormalText) const
            {
                const rendering::ViewState& rViewState( mpCanvas->getViewState() );
                const uno::Reference< rendering::XCanvas >& rCanvas( mpCanvas->getUNOCanvas() );

                maTextLinesHelper.render(rRenderState, bNormalText);

                //rhbz#1589029 non-transparent text fill background support
                if (rTextFillColor != COL_AUTO)
                {
                    rendering::RenderState aLocalState(rRenderState);
                    aLocalState.DeviceColor = vcl::unotools::colorToDoubleSequence(
                        rTextFillColor, rCanvas->getDevice()->getDeviceColorSpace());
                    auto xTextBounds = queryTextBounds(rCanvas);
                    // background of text
                    rCanvas->fillPolyPolygon(xTextBounds, rViewState, aLocalState);
                }

                rCanvas->drawTextLayout( mxTextLayout,
                                         rViewState,
                                         rRenderState );

                return true;
            }

            bool EffectTextArrayAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::EffectTextArrayAction::render()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::EffectTextArrayAction: 0x" << std::hex << this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return renderEffectText( *this,
                                         aLocalState,
                                         mpCanvas->getUNOCanvas(),
                                         maShadowColor,
                                         maShadowOffset,
                                         maReliefColor,
                                         maReliefOffset,
                                         maTextFillColor);
            }

            class EffectTextArrayRenderHelper : public TextRenderer
            {
            public:
                EffectTextArrayRenderHelper( const uno::Reference< rendering::XCanvas >&        rCanvas,
                                             const uno::Reference< rendering::XTextLayout >&    rTextLayout,
                                             const TextLinesHelper&                             rTextLinesHelper,
                                             const rendering::ViewState&                        rViewState ) :
                    mrCanvas( rCanvas ),
                    mrTextLayout( rTextLayout ),
                    mrTextLinesHelper( rTextLinesHelper ),
                    mrViewState( rViewState )
                {
                }

                // TextRenderer interface
                virtual bool operator()( const rendering::RenderState& rRenderState, const ::Color& rTextFillColor,bool bNormalText) const override
                {
                    mrTextLinesHelper.render(rRenderState, bNormalText);

                    //rhbz#1589029 non-transparent text fill background support
                    if (rTextFillColor != COL_AUTO)
                    {
                        rendering::RenderState aLocalState(rRenderState);
                        aLocalState.DeviceColor = vcl::unotools::colorToDoubleSequence(
                            rTextFillColor, mrCanvas->getDevice()->getDeviceColorSpace());
                        auto xTextBounds = queryTextBounds();
                        // background of text
                        mrCanvas->fillPolyPolygon(xTextBounds, mrViewState, aLocalState);
                    }

                    mrCanvas->drawTextLayout( mrTextLayout,
                                              mrViewState,
                                              rRenderState );

                    return true;
                }

            private:

                css::uno::Reference<css::rendering::XPolyPolygon2D> queryTextBounds() const
                {
                    const geometry::RealRectangle2D aTextBounds(mrTextLayout->queryTextBounds());
                    auto aB2DBounds = ::basegfx::unotools::b2DRectangleFromRealRectangle2D(aTextBounds);
                    auto aTextBoundsPoly = ::basegfx::utils::createPolygonFromRect(aB2DBounds);
                    return ::basegfx::unotools::xPolyPolygonFromB2DPolygon(mrCanvas->getDevice(), aTextBoundsPoly);
                }

                const uno::Reference< rendering::XCanvas >&         mrCanvas;
                const uno::Reference< rendering::XTextLayout >&     mrTextLayout;
                const TextLinesHelper&                              mrTextLinesHelper;
                const rendering::ViewState&                         mrViewState;
            };

            bool EffectTextArrayAction::renderSubset( const ::basegfx::B2DHomMatrix&  rTransformation,
                                                      const Subset&                   rSubset ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::EffectTextArrayAction::renderSubset()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::EffectTextArrayAction: 0x" << std::hex << this );

                rendering::RenderState                   aLocalState( maState );
                uno::Reference< rendering::XTextLayout > xTextLayout( mxTextLayout );
                const geometry::RealRectangle2D          aTextBounds( mxTextLayout->queryTextBounds() );

                double nMinPos(0.0);
                double nMaxPos(aTextBounds.X2 - aTextBounds.X1);

                createSubsetLayout( xTextLayout,
                                    mnLayoutWidth,
                                    aLocalState,
                                    nMinPos,
                                    nMaxPos,
                                    rTransformation,
                                    rSubset );

                if( !xTextLayout.is() )
                    return true; // empty layout, render nothing


                // create and setup local line polygon
                // ===================================

                uno::Reference< rendering::XCanvas > xCanvas( mpCanvas->getUNOCanvas() );
                const rendering::ViewState&          rViewState( mpCanvas->getViewState() );

                TextLinesHelper aHelper = maTextLinesHelper;
                aHelper.init(nMaxPos - nMinPos, maTextLineInfo);


                // render everything
                // =================

                return renderEffectText(
                    EffectTextArrayRenderHelper( xCanvas,
                                                 xTextLayout,
                                                 aHelper,
                                                 rViewState ),
                    aLocalState,
                    xCanvas,
                    maShadowColor,
                    maShadowOffset,
                    maReliefColor,
                    maReliefOffset,
                    maTextFillColor);
            }

            ::basegfx::B2DRange EffectTextArrayAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                ::basegfx::B2DSize aSize = maTextLinesHelper.getOverallSize();

                return calcEffectTextBounds( ::basegfx::unotools::b2DRectangleFromRealRectangle2D(
                                                 mxTextLayout->queryTextBounds() ),
                                             ::basegfx::B2DRange( 0,0,
                                                                  aSize.getX(),
                                                                  aSize.getY() ),
                                             maReliefOffset,
                                             maShadowOffset,
                                             aLocalState,
                                             mpCanvas->getViewState() );
            }

            ::basegfx::B2DRange EffectTextArrayAction::getBounds( const ::basegfx::B2DHomMatrix&    rTransformation,
                                                                  const Subset&                     rSubset ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::EffectTextArrayAction::getBounds( subset )" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::EffectTextArrayAction: 0x" << std::hex << this );

                rendering::RenderState                   aLocalState( maState );
                uno::Reference< rendering::XTextLayout > xTextLayout( mxTextLayout );
                const geometry::RealRectangle2D          aTextBounds( mxTextLayout->queryTextBounds() );

                double nMinPos(0.0);
                double nMaxPos(aTextBounds.X2 - aTextBounds.X1);

                createSubsetLayout( xTextLayout,
                                    mnLayoutWidth,
                                    aLocalState,
                                    nMinPos,
                                    nMaxPos,
                                    rTransformation,
                                    rSubset );

                if( !xTextLayout.is() )
                    return ::basegfx::B2DRange(); // empty layout, empty bounds


                // create and setup local line polygon
                // ===================================

                const ::basegfx::B2DPolyPolygon aPoly(
                    tools::createTextLinesPolyPolygon(
                        0.0, nMaxPos - nMinPos,
                        maTextLineInfo ) );

                return calcEffectTextBounds( ::basegfx::unotools::b2DRectangleFromRealRectangle2D(
                                                 xTextLayout->queryTextBounds() ),
                                             ::basegfx::utils::getRange( aPoly ),
                                             maReliefOffset,
                                             maShadowOffset,
                                             aLocalState,
                                             mpCanvas->getViewState() );
            }

            sal_Int32 EffectTextArrayAction::getActionCount() const
            {
                const rendering::StringContext& rOrigContext( mxTextLayout->getText() );

                return rOrigContext.Length;
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
                               const ::basegfx::B2DRectangle&                       rOutlineBounds,
                               const uno::Reference< rendering::XPolyPolygon2D >&   rTextPoly,
                               const uno::Sequence< double >&                       rOffsets,
                               VirtualDevice const &                                rVDev,
                               const CanvasSharedPtr&                               rCanvas,
                               const OutDevState&                                   rState  );
                OutlineAction( const ::basegfx::B2DPoint&                           rStartPoint,
                               const ::basegfx::B2DSize&                            rReliefOffset,
                               const ::Color&                                       rReliefColor,
                               const ::basegfx::B2DSize&                            rShadowOffset,
                               const ::Color&                                       rShadowColor,
                               const ::basegfx::B2DRectangle&                       rOutlineBounds,
                               const uno::Reference< rendering::XPolyPolygon2D >&   rTextPoly,
                               const uno::Sequence< double >&                       rOffsets,
                               VirtualDevice const &                                rVDev,
                               const CanvasSharedPtr&                               rCanvas,
                               const OutDevState&                                   rState,
                               const ::basegfx::B2DHomMatrix&                       rTextTransform );

                OutlineAction(const OutlineAction&) = delete;
                const OutlineAction& operator=(const OutlineAction&) = delete;

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual bool renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                // TextRenderer interface
                virtual bool operator()( const rendering::RenderState& rRenderState, const ::Color& rTextFillColor, bool bNormalText ) const override;

                // TODO(P2): This is potentially a real mass object
                // (every character might be a separate TextAction),
                // thus, make it as lightweight as possible. For
                // example, share common RenderState among several
                // TextActions, maybe using maOffsets for the
                // translation.

                uno::Reference< rendering::XPolyPolygon2D >         mxTextPoly;

                const uno::Sequence< double >                       maOffsets;
                const CanvasSharedPtr                               mpCanvas;
                rendering::RenderState                              maState;
                double                                              mnOutlineWidth;
                const uno::Sequence< double >                       maFillColor;
                const tools::TextLineInfo                           maTextLineInfo;
                ::basegfx::B2DSize                                  maLinesOverallSize;
                const ::basegfx::B2DRectangle                       maOutlineBounds;
                uno::Reference< rendering::XPolyPolygon2D >         mxTextLines;
                const ::basegfx::B2DSize                            maReliefOffset;
                const ::Color                                       maReliefColor;
                const ::basegfx::B2DSize                            maShadowOffset;
                const ::Color                                       maShadowColor;
                const ::Color                                       maTextFillColor;
            };

            double calcOutlineWidth( const OutDevState& rState,
                                     VirtualDevice const & rVDev )
            {
                const ::basegfx::B2DSize aFontSize( 0,
                                                    rVDev.GetFont().GetFontHeight() / 64.0 );

                const double nOutlineWidth(
                    (rState.mapModeTransform * aFontSize).getY() );

                return nOutlineWidth < 1.0 ? 1.0 : nOutlineWidth;
            }

            OutlineAction::OutlineAction( const ::basegfx::B2DPoint&                            rStartPoint,
                                          const ::basegfx::B2DSize&                             rReliefOffset,
                                          const ::Color&                                        rReliefColor,
                                          const ::basegfx::B2DSize&                             rShadowOffset,
                                          const ::Color&                                        rShadowColor,
                                          const ::basegfx::B2DRectangle&                        rOutlineBounds,
                                          const uno::Reference< rendering::XPolyPolygon2D >&    rTextPoly,
                                          const uno::Sequence< double >&                        rOffsets,
                                          VirtualDevice const &                                 rVDev,
                                          const CanvasSharedPtr&                                rCanvas,
                                          const OutDevState&                                    rState  ) :
                mxTextPoly( rTextPoly ),
                maOffsets( rOffsets ),
                mpCanvas( rCanvas ),
                maState(),
                mnOutlineWidth( calcOutlineWidth(rState,rVDev) ),
                maFillColor(
                    vcl::unotools::colorToDoubleSequence(
                        COL_WHITE,
                        rCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() )),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maLinesOverallSize(),
                maOutlineBounds( rOutlineBounds ),
                mxTextLines(),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor )
            {
                double nLayoutWidth = 0.0;

                initLayoutWidth(nLayoutWidth, rOffsets);

                initEffectLinePolyPolygon( maLinesOverallSize,
                                           mxTextLines,
                                           rCanvas,
                                           nLayoutWidth,
                                           maTextLineInfo );

                init( maState,
                      rStartPoint,
                      rState,
                      rCanvas );
            }

            OutlineAction::OutlineAction( const ::basegfx::B2DPoint&                            rStartPoint,
                                          const ::basegfx::B2DSize&                             rReliefOffset,
                                          const ::Color&                                        rReliefColor,
                                          const ::basegfx::B2DSize&                             rShadowOffset,
                                          const ::Color&                                        rShadowColor,
                                          const ::basegfx::B2DRectangle&                        rOutlineBounds,
                                          const uno::Reference< rendering::XPolyPolygon2D >&    rTextPoly,
                                          const uno::Sequence< double >&                        rOffsets,
                                          VirtualDevice const &                                 rVDev,
                                          const CanvasSharedPtr&                                rCanvas,
                                          const OutDevState&                                    rState,
                                          const ::basegfx::B2DHomMatrix&                        rTextTransform ) :
                mxTextPoly( rTextPoly ),
                maOffsets( rOffsets ),
                mpCanvas( rCanvas ),
                maState(),
                mnOutlineWidth( calcOutlineWidth(rState,rVDev) ),
                maFillColor(
                    vcl::unotools::colorToDoubleSequence(
                        COL_WHITE,
                        rCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() )),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maLinesOverallSize(),
                maOutlineBounds( rOutlineBounds ),
                mxTextLines(),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor )
            {
                double nLayoutWidth = 0.0;
                initLayoutWidth(nLayoutWidth, rOffsets);

                initEffectLinePolyPolygon( maLinesOverallSize,
                                           mxTextLines,
                                           rCanvas,
                                           nLayoutWidth,
                                           maTextLineInfo );

                init( maState,
                      rStartPoint,
                      rState,
                      rCanvas,
                      rTextTransform );
            }

            bool OutlineAction::operator()( const rendering::RenderState& rRenderState, const ::Color& /*rTextFillColor*/, bool /*bNormalText*/ ) const
            {
                const rendering::ViewState&                 rViewState( mpCanvas->getViewState() );
                const uno::Reference< rendering::XCanvas >& rCanvas( mpCanvas->getUNOCanvas() );

                rendering::StrokeAttributes aStrokeAttributes;

                aStrokeAttributes.StrokeWidth  = mnOutlineWidth;
                aStrokeAttributes.MiterLimit   = 1.0;
                aStrokeAttributes.StartCapType = rendering::PathCapType::BUTT;
                aStrokeAttributes.EndCapType   = rendering::PathCapType::BUTT;
                aStrokeAttributes.JoinType     = rendering::PathJoinType::MITER;

                rendering::RenderState aLocalState( rRenderState );
                aLocalState.DeviceColor = maFillColor;

                // TODO(P1): implement caching

                // background of text
                rCanvas->fillPolyPolygon( mxTextPoly,
                                          rViewState,
                                          aLocalState );

                // border line of text
                rCanvas->strokePolyPolygon( mxTextPoly,
                                            rViewState,
                                            rRenderState,
                                            aStrokeAttributes );

                // underlines/strikethrough - background
                rCanvas->fillPolyPolygon( mxTextLines,
                                          rViewState,
                                          aLocalState );
                // underlines/strikethrough - border
                rCanvas->strokePolyPolygon( mxTextLines,
                                            rViewState,
                                            rRenderState,
                                            aStrokeAttributes );

                return true;
            }

            bool OutlineAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::EffectTextArrayAction::render()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::EffectTextArrayAction: 0x" << std::hex << this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return renderEffectText( *this,
                                         aLocalState,
                                         mpCanvas->getUNOCanvas(),
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
                                              const uno::Reference< rendering::XPolyPolygon2D >& rTextPolygon,
                                              const uno::Reference< rendering::XPolyPolygon2D >& rLinePolygon,
                                              const rendering::ViewState&                        rViewState,
                                              double                                             nOutlineWidth ) :
                    maFillColor(
                        vcl::unotools::colorToDoubleSequence(
                            ::COL_WHITE,
                            rCanvas->getDevice()->getDeviceColorSpace() )),
                    mnOutlineWidth( nOutlineWidth ),
                    mrCanvas( rCanvas ),
                    mrTextPolygon( rTextPolygon ),
                    mrLinePolygon( rLinePolygon ),
                    mrViewState( rViewState )
                {
                }

                // TextRenderer interface
                virtual bool operator()( const rendering::RenderState& rRenderState ) const
                {
                    rendering::StrokeAttributes aStrokeAttributes;

                    aStrokeAttributes.StrokeWidth  = mnOutlineWidth;
                    aStrokeAttributes.MiterLimit   = 1.0;
                    aStrokeAttributes.StartCapType = rendering::PathCapType::BUTT;
                    aStrokeAttributes.EndCapType   = rendering::PathCapType::BUTT;
                    aStrokeAttributes.JoinType     = rendering::PathJoinType::MITER;

                    rendering::RenderState aLocalState( rRenderState );
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
                const uno::Sequence< double >                       maFillColor;
                double                                              mnOutlineWidth;
                const uno::Reference< rendering::XCanvas >&         mrCanvas;
                const uno::Reference< rendering::XPolyPolygon2D >&  mrTextPolygon;
                const uno::Reference< rendering::XPolyPolygon2D >&  mrLinePolygon;
                const rendering::ViewState&                         mrViewState;
            };
#endif

            bool OutlineAction::renderSubset( const ::basegfx::B2DHomMatrix&  rTransformation,
                                              const Subset&                   rSubset ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::OutlineAction::renderSubset()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::OutlineAction: 0x" << std::hex << this );

                if( rSubset.mnSubsetBegin == rSubset.mnSubsetEnd )
                    return true; // empty range, render nothing

#if 1
                // TODO(F3): Subsetting NYI for outline text!
                return render( rTransformation );
#else
                const rendering::StringContext rOrigContext( mxTextLayout->getText() );

                if( rSubset.mnSubsetBegin == 0 &&
                    rSubset.mnSubsetEnd == rOrigContext.Length )
                {
                    // full range, no need for subsetting
                    return render( rTransformation );
                }

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);


                // create and setup local Text polygon
                // ===================================

                uno::Reference< rendering::XPolyPolygon2D > xTextPolygon();

                // TODO(P3): Provide an API method for that!

                if( !xTextLayout.is() )
                    return false;

                // render everything
                // =================

                return renderEffectText(
                    OutlineTextArrayRenderHelper(
                        xCanvas,
                        mnOutlineWidth,
                        xTextLayout,
                        xTextLines,
                        rViewState ),
                    aLocalState,
                    rViewState,
                    xCanvas,
                    maShadowColor,
                    maShadowOffset,
                    maReliefColor,
                    maReliefOffset );
#endif
            }

            ::basegfx::B2DRange OutlineAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return calcEffectTextBounds( maOutlineBounds,
                                             ::basegfx::B2DRange( 0,0,
                                                                  maLinesOverallSize.getX(),
                                                                  maLinesOverallSize.getY() ),
                                             maReliefOffset,
                                             maShadowOffset,
                                             aLocalState,
                                             mpCanvas->getViewState() );
            }

            ::basegfx::B2DRange OutlineAction::getBounds( const ::basegfx::B2DHomMatrix&    rTransformation,
                                                          const Subset&                     /*rSubset*/ ) const
            {
                SAL_WARN( "cppcanvas.emf", "OutlineAction::getBounds(): Subset not yet supported by this object" );

                return getBounds( rTransformation );
            }

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
                                           const OUString&                  rText,
                                           sal_Int32                        nStartPos,
                                           sal_Int32                        nLen,
                                           const long*                      pDXArray,
                                           VirtualDevice&                   rVDev,
                                           const CanvasSharedPtr&           rCanvas,
                                           const OutDevState&               rState,
                                           const Renderer::Parameters&      rParms  )
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
                aUnrotatedFont.SetOrientation(0);
                rVDev.SetFont( aUnrotatedFont );

                // TODO(F3): Don't understand parameter semantics of
                // GetTextOutlines()
                ::basegfx::B2DPolyPolygon aResultingPolyPolygon;
                PolyPolyVector aVCLPolyPolyVector;
                const bool bHaveOutlines( rVDev.GetTextOutlines( aVCLPolyPolyVector, rText,
                                                                 static_cast<sal_uInt16>(nStartPos),
                                                                 static_cast<sal_uInt16>(nStartPos),
                                                                 static_cast<sal_uInt16>(nLen),
                                                                 0, pDXArray ) );
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
                    ::basegfx::B2DPolyPolygon aPolyPolygon;

                    aPolyPolygon = rVCLPolyPolygon.getB2DPolyPolygon();
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

                const uno::Sequence< double > aCharWidthSeq(
                    pDXArray ?
                    setupDXArray( pDXArray, nLen, rState ) :
                    setupDXArray( rText,
                                  nStartPos,
                                  nLen,
                                  rVDev,
                                  rState ));
                const uno::Reference< rendering::XPolyPolygon2D > xTextPoly(
                    ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rCanvas->getUNOCanvas()->getDevice(),
                        aResultingPolyPolygon ) );

                if( rParms.maTextTransformation.is_initialized() )
                {
                    return std::shared_ptr<Action>(
                        new OutlineAction(
                            rStartPoint,
                            rReliefOffset,
                            rReliefColor,
                            rShadowOffset,
                            rShadowColor,
                            ::basegfx::utils::getRange(aResultingPolyPolygon),
                            xTextPoly,
                            aCharWidthSeq,
                            rVDev,
                            rCanvas,
                            rState,
                            *rParms.maTextTransformation ) );
                }
                else
                {
                    return std::shared_ptr<Action>(
                        new OutlineAction(
                            rStartPoint,
                            rReliefOffset,
                            rReliefColor,
                            rShadowOffset,
                            rShadowColor,
                            ::basegfx::utils::getRange(aResultingPolyPolygon),
                            xTextPoly,
                            aCharWidthSeq,
                            rVDev,
                            rCanvas,
                            rState  ) );
                }
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
                                                             const long*                    pDXArray,
                                                             VirtualDevice&                 rVDev,
                                                             const CanvasSharedPtr&         rCanvas,
                                                             const OutDevState&             rState,
                                                             const Renderer::Parameters&    rParms,
                                                             bool                           bSubsettable    )
        {
            const ::Size  aBaselineOffset( tools::getBaselineOffset( rState,
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
                            rText,
                            nStartPos,
                            nLen,
                            pDXArray,
                            rVDev,
                            rCanvas,
                            rState,
                            rParms );
            }

            // convert DX array to device coordinate system (and
            // create it in the first place, if pDXArray is NULL)
            const uno::Sequence< double > aCharWidths(
                pDXArray ?
                setupDXArray( pDXArray, nLen, rState ) :
                setupDXArray( rText,
                              nStartPos,
                              nLen,
                              rVDev,
                              rState ));

            // determine type of text action to create
            // =======================================

            const ::Color aEmptyColor( COL_AUTO );

            std::shared_ptr<Action> ret;

            // no DX array, and no need to subset - no need to store
            // DX array, then.
            if( !pDXArray && !bSubsettable )
            {
                // effects, or not?
                if( !rState.textOverlineStyle &&
                    !rState.textUnderlineStyle &&
                    !rState.textStrikeoutStyle &&
                    rReliefColor == aEmptyColor &&
                    rShadowColor == aEmptyColor )
                {
                    // nope
                    if( rParms.maTextTransformation.is_initialized() )
                    {
                        ret = std::shared_ptr<Action>( new TextAction(
                                                    aStartPoint,
                                                    rText,
                                                    nStartPos,
                                                    nLen,
                                                    rCanvas,
                                                    rState,
                                                    *rParms.maTextTransformation ) );
                    }
                    else
                    {
                        ret = std::shared_ptr<Action>( new TextAction(
                                                    aStartPoint,
                                                    rText,
                                                    nStartPos,
                                                    nLen,
                                                    rCanvas,
                                                    rState ) );
                    }
                }
                else
                {
                    // at least one of the effects requested
                    if( rParms.maTextTransformation.is_initialized() )
                        ret = std::shared_ptr<Action>( new EffectTextAction(
                                                    aStartPoint,
                                                    aReliefOffset,
                                                    rReliefColor,
                                                    aShadowOffset,
                                                    rShadowColor,
                                                    rText,
                                                    nStartPos,
                                                    nLen,
                                                    rVDev,
                                                    rCanvas,
                                                    rState,
                                                    *rParms.maTextTransformation ) );
                    else
                        ret = std::shared_ptr<Action>( new EffectTextAction(
                                                    aStartPoint,
                                                    aReliefOffset,
                                                    rReliefColor,
                                                    aShadowOffset,
                                                    rShadowColor,
                                                    rText,
                                                    nStartPos,
                                                    nLen,
                                                    rVDev,
                                                    rCanvas,
                                                    rState ) );
                }
            }
            else
            {
                // DX array necessary - any effects?
                if( !rState.textOverlineStyle &&
                    !rState.textUnderlineStyle &&
                    !rState.textStrikeoutStyle &&
                    rReliefColor == aEmptyColor &&
                    rShadowColor == aEmptyColor &&
                    rTextFillColor == aEmptyColor )
                {
                    // nope
                    if( rParms.maTextTransformation.is_initialized() )
                        ret = std::shared_ptr<Action>( new TextArrayAction(
                                                    aStartPoint,
                                                    rText,
                                                    nStartPos,
                                                    nLen,
                                                    aCharWidths,
                                                    rCanvas,
                                                    rState,
                                                    *rParms.maTextTransformation ) );
                    else
                        ret = std::shared_ptr<Action>( new TextArrayAction(
                                                    aStartPoint,
                                                    rText,
                                                    nStartPos,
                                                    nLen,
                                                    aCharWidths,
                                                    rCanvas,
                                                    rState ) );
                }
                else
                {
                    // at least one of the effects requested
                    if( rParms.maTextTransformation.is_initialized() )
                        ret = std::shared_ptr<Action>( new EffectTextArrayAction(
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
                                                    rVDev,
                                                    rCanvas,
                                                    rState,
                                                    *rParms.maTextTransformation ) );
                    else
                        ret = std::shared_ptr<Action>( new EffectTextArrayAction(
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
                                                    rVDev,
                                                    rCanvas,
                                                    rState ) );
                }
            }
            return ret;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
