/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textaction.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:42:02 $
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

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <rtl/logfile.hxx>

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

#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/virdev.hxx>

#include <basegfx/tools/canvastools.hxx>
#include <canvas/canvastools.hxx>

#include <boost/scoped_array.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>

#include "textaction.hxx"
#include "outdevstate.hxx"
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
                                   NULL,
                                   &rState.fontRotation );

                ::basegfx::B2DHomMatrix aLocalTransformation;

                aLocalTransformation.rotate( rState.fontRotation );
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

            ::basegfx::B2DPolyPolygon textLinesFromLogicalOffsets( const uno::Sequence< double >&   rOffsets,
                                                                   const tools::TextLineInfo&       rTextLineInfo )
            {
                return tools::createTextLinesPolyPolygon(
                    0.0,
                    // extract character cell furthest to the right
                    *(::std::max_element(
                          rOffsets.getConstArray(),
                          rOffsets.getConstArray() + rOffsets.getLength() )),
                    rTextLineInfo );
            }

            uno::Sequence< double > setupDXArray( const sal_Int32*  pCharWidths,
                                                  sal_Int32         nLen,
                                                  VirtualDevice&    rVDev )
            {
                // convert character widths from logical units
                uno::Sequence< double > aCharWidthSeq( nLen );
                double*                 pOutputWidths( aCharWidthSeq.getArray() );
                for( int i = 0; i < nLen; ++i )
                {
                    // TODO(F2): use correct scale direction
                    const Size aSize( ::basegfx::fround( *pCharWidths++ + .5 ), 0 );
                    *pOutputWidths++ = rVDev.LogicToPixel( aSize ).Width();
                }

                return aCharWidthSeq;
            }

            uno::Sequence< double > setupDXArray( const ::String&   rText,
                                                  sal_Int32         nStartPos,
                                                  sal_Int32         nLen,
                                                  VirtualDevice&    rVDev )
            {
                // no external DX array given, create one from given
                // string
                ::boost::scoped_array< sal_Int32 > pCharWidths( new sal_Int32[nLen] );

                rVDev.GetTextArray( rText, pCharWidths.get(),
                                    static_cast<USHORT>(nStartPos),
                                    static_cast<USHORT>(nLen) );

                return setupDXArray( pCharWidths.get(), nLen, rVDev );
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
                                  const ::rtl::OUString&                    rText,
                                  sal_Int32                                 nStartPos,
                                  sal_Int32                                 nLen,
                                  const uno::Sequence< double >&            rOffsets,
                                  const CanvasSharedPtr&                    rCanvas,
                                  const OutDevState&                        rState,
                                  const ::basegfx::B2DHomMatrix*            pTextTransform )
            {
                ENSURE_AND_THROW( rOffsets.getLength(),
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

                ENSURE_AND_THROW( o_rTextLayout.is(),
                                  "::cppcanvas::internal::initArrayAction(): Invalid font" );

                o_rTextLayout->applyLogicalAdvancements( rOffsets );
            }

            sal_Int32 getLineWidth( ::VirtualDevice& rVDev,
                                    const rendering::StringContext& rStringContext )
            {
                // TODO(F2): use correct scale direction
                const ::Size aSize( rVDev.GetTextWidth( rStringContext.Text,
                                                        static_cast<USHORT>(rStringContext.StartPosition),
                                                        static_cast<USHORT>(rStringContext.Length) ),
                                    0 );

                return rVDev.LogicToPixel( aSize ).Width();
            }

            uno::Sequence< double >
                calcSubsetOffsets( rendering::RenderState&                          io_rRenderState,
                                   double&                                          o_rMinPos,
                                   double&                                          o_rMaxPos,
                                   const uno::Reference< rendering::XTextLayout >&  rOrigTextLayout,
                                   const ::cppcanvas::internal::Action::Subset&     rSubset )
            {
                ENSURE_AND_THROW( rSubset.mnSubsetEnd > rSubset.mnSubsetBegin,
                                  "::cppcanvas::internal::calcSubsetOffsets(): invalid subset range range" );

                uno::Sequence< double > aOrigOffsets( rOrigTextLayout->queryLogicalAdvancements() );
                const double*           pOffsets( aOrigOffsets.getConstArray() );

                ENSURE_AND_THROW( aOrigOffsets.getLength() >= rSubset.mnSubsetEnd,
                                  "::cppcanvas::internal::calcSubsetOffsets(): invalid subset range range" );

                // TODO(F3): It currently seems that for RTL text, the
                // DX offsets are nevertheless increasing in logical
                // text order (I'd expect they are decreasing,
                // mimicking the fact that the text is output
                // right-to-left). This breaks text effects for ALL
                // RTL languages.

                // determine leftmost position in given subset range -
                // as the DX array contains the output positions
                // starting with the second character (the first is
                // assumed to have output position 0), correct begin
                // iterator.
                const double nMinPos( rSubset.mnSubsetBegin <= 0 ? 0 :
                                      *(::std::min_element( pOffsets+rSubset.mnSubsetBegin-1,
                                                            pOffsets+rSubset.mnSubsetEnd )) );

                // determine rightmost position in given subset range
                // - as the DX array contains the output positions
                // starting with the second character (the first is
                // assumed to have output position 0), correct begin
                // iterator.
                const double nMaxPos(
                    *(::std::max_element( pOffsets + (rSubset.mnSubsetBegin <= 0 ?
                                                      0 : rSubset.mnSubsetBegin-1),
                                          pOffsets + rSubset.mnSubsetEnd )) );


                // adapt render state, to move text output to given offset
                // -------------------------------------------------------

                // TODO(F1): Strictly speaking, we also have to adapt
                // the clip here, which normally should _not_ move
                // with the output offset. Neglected for now, as it
                // does not matter for drawing layer output

                if( rSubset.mnSubsetBegin > 0 )
                {
                    ::basegfx::B2DHomMatrix aTranslation;
                    if( rOrigTextLayout->getFont()->getFontRequest().FontDescription.IsVertical )
                    {
                        // vertical text -> offset in y direction
                        aTranslation.translate( 0.0, nMinPos );
                    }
                    else
                    {
                        // horizontal text -> offset in x direction
                        aTranslation.translate( nMinPos, 0.0 );
                    }

                    ::canvas::tools::appendToRenderState( io_rRenderState,
                                                          aTranslation );
                }


                // reduce DX array to given substring
                // ----------------------------------

                const sal_Int32         nNewElements( rSubset.mnSubsetEnd - rSubset.mnSubsetBegin );
                uno::Sequence< double > aAdaptedOffsets( nNewElements );
                double*                 pAdaptedOffsets( aAdaptedOffsets.getArray() );

                // move to new output position (subtract nMinPos,
                // which is the new '0' position), copy only the range
                // as given by rSubset.
                ::std::transform( pOffsets + rSubset.mnSubsetBegin,
                                  pOffsets + rSubset.mnSubsetEnd,
                                  pAdaptedOffsets,
                                  ::boost::bind( ::std::minus<double>(),
                                                 _1,
                                                 nMinPos ) );

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
                // ---------------------------------------------------

                const sal_Int32 nNewStartPos( rOrigContext.StartPosition + ::std::min(
                                                  rSubset.mnSubsetBegin, rOrigContext.Length-1 ) );
                const sal_Int32 nNewLength( ::std::max(
                                                ::std::min(
                                                    rSubset.mnSubsetEnd - rSubset.mnSubsetBegin,
                                                    rOrigContext.Length ),
                                                0L ) );

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

                ENSURE_AND_THROW( io_rTextLayout.is(),
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
                virtual bool operator()( const rendering::RenderState& rRenderState ) const = 0;
            };

            /** Render effect text.

                @param rRenderer
                Functor object, will be called to render the actual
                part of the text effect (the text itself and the means
                to render it are unknown to this method)
             */
            bool renderEffectText( const TextRenderer&                          rRenderer,
                                   const rendering::RenderState&                rRenderState,
                                   const rendering::ViewState&                  rViewState,
                                   const uno::Reference< rendering::XCanvas >&  xCanvas,
                                   const ::Color&                               rShadowColor,
                                   const ::Size&                                rShadowOffset,
                                   const ::Color&                               rReliefColor,
                                   const ::Size&                                rReliefOffset )
            {
                ::Color aEmptyColor( COL_AUTO );

                // draw shadow text, if enabled
                if( rShadowColor != aEmptyColor )
                {
                    rendering::RenderState aShadowState( rRenderState );
                    ::basegfx::B2DHomMatrix aTranslate;

                    aTranslate.translate( rShadowOffset.Width(),
                                          rShadowOffset.Height() );

                    ::canvas::tools::appendToRenderState(aShadowState, aTranslate);

                    aShadowState.DeviceColor =
                        ::vcl::unotools::colorToDoubleSequence( xCanvas->getDevice(),
                                                                rShadowColor );

                    rRenderer( aShadowState );
                }

                // draw relief text, if enabled
                if( rReliefColor != aEmptyColor )
                {
                    rendering::RenderState aReliefState( rRenderState );
                    ::basegfx::B2DHomMatrix aTranslate;

                    aTranslate.translate( rReliefOffset.Width(),
                                          rReliefOffset.Height() );

                    ::canvas::tools::appendToRenderState(aReliefState, aTranslate);

                    aReliefState.DeviceColor =
                        ::vcl::unotools::colorToDoubleSequence( xCanvas->getDevice(),
                                                                rReliefColor );

                    rRenderer( aReliefState );
                }

                // draw normal text
                rRenderer( rRenderState );

                return true;
            }


            ::basegfx::B2DRange calcEffectTextBounds( const ::basegfx::B2DRange&    rTextBounds,
                                                      const ::basegfx::B2DRange&    rLineBounds,
                                                      const ::Size&                 rReliefOffset,
                                                      const ::Size&                 rShadowOffset,
                                                      const rendering::RenderState& rRenderState,
                                                      const rendering::ViewState&   rViewState )
            {
                ::basegfx::B2DRange aBounds( rTextBounds );

                // add extends of text lines
                aBounds.expand( rLineBounds );

                // TODO(Q3): Provide this functionality at the B2DRange
                ::basegfx::B2DRange aTotalBounds( aBounds );
                aTotalBounds.expand(
                    ::basegfx::B2DRange( aBounds.getMinX() + rReliefOffset.Width(),
                                         aBounds.getMinY() + rReliefOffset.Height(),
                                         aBounds.getMaxX() + rReliefOffset.Width(),
                                         aBounds.getMaxY() + rReliefOffset.Height() ) );
                aTotalBounds.expand(
                    ::basegfx::B2DRange( aBounds.getMinX() + rShadowOffset.Width(),
                                         aBounds.getMinY() + rShadowOffset.Height(),
                                         aBounds.getMaxX() + rShadowOffset.Width(),
                                         aBounds.getMaxY() + rShadowOffset.Height() ) );

                return tools::calcDevicePixelBounds( aTotalBounds,
                                                     rViewState,
                                                     rRenderState );
            }

            void initEffectLinePolyPolygon( ::basegfx::B2DSize&                             o_rOverallSize,
                                            uno::Reference< rendering::XPolyPolygon2D >&    o_rTextLines,
                                            const CanvasSharedPtr&                          rCanvas,
                                            const uno::Sequence< double >&                  rOffsets,
                                            const tools::TextLineInfo                       rLineInfo   )
            {
                const ::basegfx::B2DPolyPolygon aPoly(
                    textLinesFromLogicalOffsets(
                        rOffsets,
                        rLineInfo ) );

                o_rOverallSize = ::basegfx::tools::getRange( aPoly ).getRange();

                o_rTextLines = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    rCanvas->getUNOCanvas()->getDevice(),
                    aPoly );
            }

            void initEffectLinePolyPolygon( ::basegfx::B2DSize&                             o_rOverallSize,
                                            uno::Reference< rendering::XPolyPolygon2D >&    o_rTextLines,
                                            const CanvasSharedPtr&                          rCanvas,
                                            sal_Int32                                       nLineWidth,
                                            const tools::TextLineInfo                       rLineInfo   )
            {
                const ::basegfx::B2DPolyPolygon aPoly(
                    tools::createTextLinesPolyPolygon( 0.0, nLineWidth,
                                                       rLineInfo ) );

                o_rOverallSize = ::basegfx::tools::getRange( aPoly ).getRange();

                o_rTextLines = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    rCanvas->getUNOCanvas()->getDevice(),
                    aPoly );
            }


            // -------------------------------------------------------------------------

            class TextAction : public Action, private ::boost::noncopyable
            {
            public:
                TextAction( const ::Point&              rStartPoint,
                            const ::rtl::OUString&      rString,
                            sal_Int32                   nStartPos,
                            sal_Int32                   nLen,
                            VirtualDevice&              rVDev,
                            const CanvasSharedPtr&      rCanvas,
                            const OutDevState&          rState );

                TextAction( const ::Point&                  rStartPoint,
                            const ::rtl::OUString&          rString,
                            sal_Int32                       nStartPos,
                            sal_Int32                       nLen,
                            VirtualDevice&                  rVDev,
                            const CanvasSharedPtr&          rCanvas,
                            const OutDevState&              rState,
                            const ::basegfx::B2DHomMatrix&  rTextTransform );

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const;

                virtual sal_Int32 getActionCount() const;

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

            TextAction::TextAction( const ::Point&              rStartPoint,
                                    const ::rtl::OUString&      rString,
                                    sal_Int32                   nStartPos,
                                    sal_Int32                   nLen,
                                    VirtualDevice&              rVDev,
                                    const CanvasSharedPtr&      rCanvas,
                                    const OutDevState&          rState  ) :
                mxFont( rState.xFont ),
                maStringContext( rString, nStartPos, nLen ),
                mpCanvas( rCanvas ),
                maState(),
                maTextDirection( rState.textDirection )
            {
                init( maState, mxFont,
                      ::vcl::unotools::b2DPointFromPoint( rStartPoint ),
                      rState, rCanvas );

                ENSURE_AND_THROW( mxFont.is(),
                                  "::cppcanvas::internal::TextAction(): Invalid font" );
            }

            TextAction::TextAction( const ::Point&                  rStartPoint,
                                    const ::rtl::OUString&          rString,
                                    sal_Int32                       nStartPos,
                                    sal_Int32                       nLen,
                                    VirtualDevice&                  rVDev,
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
                      ::vcl::unotools::b2DPointFromPoint( rStartPoint ),
                      rState, rCanvas, rTextTransform );

                ENSURE_AND_THROW( mxFont.is(),
                                  "::cppcanvas::internal::TextAction(): Invalid font" );
            }

            bool TextAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::TextAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::TextAction: 0x%X", this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                mpCanvas->getUNOCanvas()->drawText( maStringContext, mxFont,
                                                    mpCanvas->getViewState(), aLocalState, maTextDirection );

                return true;
            }

            bool TextAction::render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const
            {
                OSL_ENSURE( false,
                            "TextAction::render(): Subset not supported by this object" );

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
                                                       const Subset&                    rSubset ) const
            {
                OSL_ENSURE( false,
                            "TextAction::getBounds(): Subset not supported by this object" );

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


            // -------------------------------------------------------------------------

            class EffectTextAction :
                public Action,
                public TextRenderer,
                private ::boost::noncopyable
            {
            public:
                EffectTextAction( const ::Point&            rStartPoint,
                                  const ::Size&             rReliefOffset,
                                  const ::Color&            rReliefColor,
                                  const ::Size&             rShadowOffset,
                                  const ::Color&            rShadowColor,
                                  const ::rtl::OUString&    rText,
                                  sal_Int32                 nStartPos,
                                  sal_Int32                 nLen,
                                  VirtualDevice&            rVDev,
                                  const CanvasSharedPtr&    rCanvas,
                                  const OutDevState&        rState );

                EffectTextAction( const ::Point&                    rStartPoint,
                                  const ::Size&                     rReliefOffset,
                                  const ::Color&                    rReliefColor,
                                  const ::Size&                     rShadowOffset,
                                  const ::Color&                    rShadowColor,
                                  const ::rtl::OUString&            rText,
                                  sal_Int32                         nStartPos,
                                  sal_Int32                         nLen,
                                  VirtualDevice&                    rVDev,
                                  const CanvasSharedPtr&            rCanvas,
                                  const OutDevState&                rState,
                                  const ::basegfx::B2DHomMatrix&    rTextTransform );

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const;

                virtual sal_Int32 getActionCount() const;

            private:
                /// Interface TextRenderer
                virtual bool operator()( const rendering::RenderState& rRenderState ) const;

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
                const sal_Int32                             mnLineWidth;
                uno::Reference< rendering::XPolyPolygon2D > mxTextLines;
                const ::Size                                maReliefOffset;
                const ::Color                               maReliefColor;
                const ::Size                                maShadowOffset;
                const ::Color                               maShadowColor;
                const sal_Int8                              maTextDirection;
            };

            EffectTextAction::EffectTextAction( const ::Point&          rStartPoint,
                                                const ::Size&           rReliefOffset,
                                                const ::Color&          rReliefColor,
                                                const ::Size&           rShadowOffset,
                                                const ::Color&          rShadowColor,
                                                const ::rtl::OUString&  rText,
                                                sal_Int32               nStartPos,
                                                sal_Int32               nLen,
                                                VirtualDevice&          rVDev,
                                                const CanvasSharedPtr&  rCanvas,
                                                const OutDevState&      rState ) :
                mxFont( rState.xFont ),
                maStringContext( rText, nStartPos, nLen ),
                mpCanvas( rCanvas ),
                maState(),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maLinesOverallSize(),
                mnLineWidth( getLineWidth( rVDev, maStringContext ) ),
                mxTextLines(),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor ),
                maTextDirection( rState.textDirection )
            {
                initEffectLinePolyPolygon( maLinesOverallSize,
                                           mxTextLines,
                                           rCanvas,
                                           mnLineWidth,
                                           maTextLineInfo );

                init( maState, mxFont,
                      ::vcl::unotools::b2DPointFromPoint( rStartPoint ),
                      rState, rCanvas );

                ENSURE_AND_THROW( mxFont.is() && mxTextLines.is(),
                                  "::cppcanvas::internal::EffectTextAction(): Invalid font or lines" );
            }

            EffectTextAction::EffectTextAction( const ::Point&                  rStartPoint,
                                                const ::Size&                   rReliefOffset,
                                                const ::Color&                  rReliefColor,
                                                const ::Size&                   rShadowOffset,
                                                const ::Color&                  rShadowColor,
                                                const ::rtl::OUString&          rText,
                                                sal_Int32                       nStartPos,
                                                sal_Int32                       nLen,
                                                VirtualDevice&                  rVDev,
                                                const CanvasSharedPtr&          rCanvas,
                                                const OutDevState&              rState,
                                                const ::basegfx::B2DHomMatrix&  rTextTransform ) :
                mxFont( rState.xFont ),
                maStringContext( rText, nStartPos, nLen ),
                mpCanvas( rCanvas ),
                maState(),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maLinesOverallSize(),
                mnLineWidth( getLineWidth( rVDev, maStringContext ) ),
                mxTextLines(),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor ),
                maTextDirection( rState.textDirection )
            {
                initEffectLinePolyPolygon( maLinesOverallSize,
                                           mxTextLines,
                                           rCanvas,
                                           mnLineWidth,
                                           maTextLineInfo );

                init( maState, mxFont,
                      ::vcl::unotools::b2DPointFromPoint( rStartPoint ),
                      rState, rCanvas, rTextTransform );

                ENSURE_AND_THROW( mxFont.is() && mxTextLines.is(),
                                  "::cppcanvas::internal::EffectTextAction(): Invalid font or lines" );
            }

            bool EffectTextAction::operator()( const rendering::RenderState& rRenderState ) const
            {
                const rendering::ViewState& rViewState( mpCanvas->getViewState() );
                const uno::Reference< rendering::XCanvas >& rCanvas( mpCanvas->getUNOCanvas() );

                rCanvas->fillPolyPolygon( mxTextLines,
                                          rViewState,
                                          rRenderState );

                rCanvas->drawText( maStringContext, mxFont,
                                   rViewState,
                                   rRenderState,
                                   maTextDirection );

                return true;
            }

            bool EffectTextAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::EffectTextAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::EffectTextAction: 0x%X", this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return renderEffectText( *this,
                                         aLocalState,
                                         mpCanvas->getViewState(),
                                         mpCanvas->getUNOCanvas(),
                                         maShadowColor,
                                         maShadowOffset,
                                         maReliefColor,
                                         maReliefOffset );
            }

            bool EffectTextAction::render( const ::basegfx::B2DHomMatrix&   rTransformation,
                                           const Subset&                    rSubset ) const
            {
                OSL_ENSURE( false,
                            "EffectTextAction::render(): Subset not supported by this object" );

                // TODO(P1): Retrieve necessary font metric info for
                // TextAction from XCanvas. Currently, the
                // TextActionFactory does not generate this object for
                // subsettable text
                return render( rTransformation );
            }

            ::basegfx::B2DRange EffectTextAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const
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

                return calcEffectTextBounds( ::basegfx::unotools::b2DRectangleFromRealRectangle2D(
                                                 xTextLayout->queryTextBounds() ),
                                             ::basegfx::B2DRange( 0,0,
                                                                  maLinesOverallSize.getX(),
                                                                  maLinesOverallSize.getY() ),
                                             maReliefOffset,
                                             maShadowOffset,
                                             aLocalState,
                                             mpCanvas->getViewState() );
            }

            ::basegfx::B2DRange EffectTextAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation,
                                                             const Subset&                  rSubset ) const
            {
                OSL_ENSURE( false,
                            "EffectTextAction::getBounds(): Subset not supported by this object" );

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


            // -------------------------------------------------------------------------

            class TextArrayAction : public Action, private ::boost::noncopyable
            {
            public:
                TextArrayAction( const ::Point&                 rStartPoint,
                                 const ::rtl::OUString&         rString,
                                 sal_Int32                      nStartPos,
                                 sal_Int32                      nLen,
                                 const uno::Sequence< double >& rOffsets,
                                 VirtualDevice&                 rVDev,
                                 const CanvasSharedPtr&         rCanvas,
                                 const OutDevState&             rState );

                TextArrayAction( const ::Point&                 rStartPoint,
                                 const ::rtl::OUString&         rString,
                                 sal_Int32                      nStartPos,
                                 sal_Int32                      nLen,
                                 const uno::Sequence< double >& rOffsets,
                                 VirtualDevice&                 rVDev,
                                 const CanvasSharedPtr&         rCanvas,
                                 const OutDevState&             rState,
                                 const ::basegfx::B2DHomMatrix& rTextTransform );

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const;

                virtual sal_Int32 getActionCount() const;

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
            };

            TextArrayAction::TextArrayAction( const ::Point&                    rStartPoint,
                                              const ::rtl::OUString&            rString,
                                              sal_Int32                         nStartPos,
                                              sal_Int32                         nLen,
                                              const uno::Sequence< double >&    rOffsets,
                                              VirtualDevice&                    rVDev,
                                              const CanvasSharedPtr&            rCanvas,
                                              const OutDevState&                rState ) :
                mxTextLayout(),
                mpCanvas( rCanvas ),
                maState()
            {
                initArrayAction( maState,
                                 mxTextLayout,
                                 ::vcl::unotools::b2DPointFromPoint( rStartPoint ),
                                 rString,
                                 nStartPos,
                                 nLen,
                                 rOffsets,
                                 rCanvas,
                                 rState, NULL );
            }

            TextArrayAction::TextArrayAction( const ::Point&                rStartPoint,
                                              const ::rtl::OUString&            rString,
                                              sal_Int32                         nStartPos,
                                              sal_Int32                         nLen,
                                              const uno::Sequence< double >&    rOffsets,
                                              VirtualDevice&                    rVDev,
                                              const CanvasSharedPtr&            rCanvas,
                                              const OutDevState&                rState,
                                              const ::basegfx::B2DHomMatrix&    rTextTransform ) :
                mxTextLayout(),
                mpCanvas( rCanvas ),
                maState()
            {
                initArrayAction( maState,
                                 mxTextLayout,
                                 ::vcl::unotools::b2DPointFromPoint( rStartPoint ),
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
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::TextArrayAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::TextArrayAction: 0x%X", this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

#ifdef SPECIAL_DEBUG
                aLocalState.Clip.clear();
                aLocalState.DeviceColor =
                    ::vcl::unotools::colorToDoubleSequence( mpCanvas->getUNOCanvas()->getDevice(),
                                                            ::Color( 0x80FF0000 ) );

                if( maState.Clip.is() )
                    mpCanvas->getUNOCanvas()->drawPolyPolygon( maState.Clip,
                                                               mpCanvas->getViewState(),
                                                               aLocalState );

                aLocalState.DeviceColor = maState.DeviceColor;
#endif

                mpCanvas->getUNOCanvas()->drawTextLayout( mxTextLayout,
                                                          mpCanvas->getViewState(),
                                                          aLocalState );

                return true;
            }

            bool TextArrayAction::render( const ::basegfx::B2DHomMatrix&    rTransformation,
                                          const Subset&                     rSubset ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::TextArrayAction::render( subset )" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::TextArrayAction: 0x%X", this );

                rendering::RenderState                      aLocalState( maState );
                uno::Reference< rendering::XTextLayout >    xTextLayout( mxTextLayout );

                double nDummy0, nDummy1;
                createSubsetLayout( xTextLayout,
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
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::TextArrayAction::getBounds( subset )" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::TextArrayAction: 0x%X", this );

                rendering::RenderState                      aLocalState( maState );
                uno::Reference< rendering::XTextLayout >    xTextLayout( mxTextLayout );

                double nDummy0, nDummy1;
                createSubsetLayout( xTextLayout,
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


            // -------------------------------------------------------------------------

            class EffectTextArrayAction :
                public Action,
                public TextRenderer,
                private ::boost::noncopyable
            {
            public:
                EffectTextArrayAction( const ::Point&                   rStartPoint,
                                       const ::Size&                    rReliefOffset,
                                       const ::Color&                   rReliefColor,
                                       const ::Size&                    rShadowOffset,
                                       const ::Color&                   rShadowColor,
                                       const ::rtl::OUString&           rText,
                                       sal_Int32                        nStartPos,
                                       sal_Int32                        nLen,
                                       const uno::Sequence< double >&   rOffsets,
                                       VirtualDevice&                   rVDev,
                                       const CanvasSharedPtr&           rCanvas,
                                       const OutDevState&               rState  );
                EffectTextArrayAction( const ::Point&                   rStartPoint,
                                       const ::Size&                    rReliefOffset,
                                       const ::Color&                   rReliefColor,
                                       const ::Size&                    rShadowOffset,
                                       const ::Color&                   rShadowColor,
                                       const ::rtl::OUString&           rText,
                                       sal_Int32                        nStartPos,
                                       sal_Int32                        nLen,
                                       const uno::Sequence< double >&   rOffsets,
                                       VirtualDevice&                   rVDev,
                                       const CanvasSharedPtr&           rCanvas,
                                       const OutDevState&               rState,
                                       const ::basegfx::B2DHomMatrix&   rTextTransform );

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const;

                virtual sal_Int32 getActionCount() const;

            private:
                // TextRenderer interface
                virtual bool operator()( const rendering::RenderState& rRenderState ) const;

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
                ::basegfx::B2DSize                              maLinesOverallSize;
                uno::Reference< rendering::XPolyPolygon2D >     mxTextLines;
                const ::Size                                    maReliefOffset;
                const ::Color                                   maReliefColor;
                const ::Size                                    maShadowOffset;
                const ::Color                                   maShadowColor;
            };

            EffectTextArrayAction::EffectTextArrayAction( const ::Point&                    rStartPoint,
                                                          const ::Size&                     rReliefOffset,
                                                          const ::Color&                    rReliefColor,
                                                          const ::Size&                     rShadowOffset,
                                                          const ::Color&                    rShadowColor,
                                                          const ::rtl::OUString&            rText,
                                                          sal_Int32                         nStartPos,
                                                          sal_Int32                         nLen,
                                                          const uno::Sequence< double >&    rOffsets,
                                                          VirtualDevice&                    rVDev,
                                                          const CanvasSharedPtr&            rCanvas,
                                                          const OutDevState&                rState  ) :
                mxTextLayout(),
                mpCanvas( rCanvas ),
                maState(),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maLinesOverallSize(),
                mxTextLines(),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor )
            {
                initEffectLinePolyPolygon( maLinesOverallSize,
                                           mxTextLines,
                                           rCanvas,
                                           rOffsets,
                                           maTextLineInfo );

                initArrayAction( maState,
                                 mxTextLayout,
                                 ::vcl::unotools::b2DPointFromPoint( rStartPoint ),
                                 rText,
                                 nStartPos,
                                 nLen,
                                 rOffsets,
                                 rCanvas,
                                 rState, NULL );
            }

            EffectTextArrayAction::EffectTextArrayAction( const ::Point&                    rStartPoint,
                                                          const ::Size&                     rReliefOffset,
                                                          const ::Color&                    rReliefColor,
                                                          const ::Size&                     rShadowOffset,
                                                          const ::Color&                    rShadowColor,
                                                          const ::rtl::OUString&            rText,
                                                          sal_Int32                         nStartPos,
                                                          sal_Int32                         nLen,
                                                          const uno::Sequence< double >&    rOffsets,
                                                          VirtualDevice&                    rVDev,
                                                          const CanvasSharedPtr&            rCanvas,
                                                          const OutDevState&                rState,
                                                          const ::basegfx::B2DHomMatrix&    rTextTransform ) :
                mxTextLayout(),
                mpCanvas( rCanvas ),
                maState(),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maLinesOverallSize(),
                mxTextLines(),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor )
            {
                initEffectLinePolyPolygon( maLinesOverallSize,
                                           mxTextLines,
                                           rCanvas,
                                           rOffsets,
                                           maTextLineInfo );

                initArrayAction( maState,
                                 mxTextLayout,
                                 ::vcl::unotools::b2DPointFromPoint( rStartPoint ),
                                 rText,
                                 nStartPos,
                                 nLen,
                                 rOffsets,
                                 rCanvas,
                                 rState,
                                 &rTextTransform );
            }

            bool EffectTextArrayAction::operator()( const rendering::RenderState& rRenderState ) const
            {
                const rendering::ViewState& rViewState( mpCanvas->getViewState() );
                const uno::Reference< rendering::XCanvas >& rCanvas( mpCanvas->getUNOCanvas() );

                rCanvas->fillPolyPolygon( mxTextLines,
                                          rViewState,
                                          rRenderState );

                rCanvas->drawTextLayout( mxTextLayout,
                                         rViewState,
                                         rRenderState );

                return true;
            }

            bool EffectTextArrayAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::EffectTextArrayAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::EffectTextArrayAction: 0x%X", this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return renderEffectText( *this,
                                         aLocalState,
                                         mpCanvas->getViewState(),
                                         mpCanvas->getUNOCanvas(),
                                         maShadowColor,
                                         maShadowOffset,
                                         maReliefColor,
                                         maReliefOffset );
            }

            class EffectTextArrayRenderHelper : public TextRenderer
            {
            public:
                EffectTextArrayRenderHelper( const uno::Reference< rendering::XCanvas >&        rCanvas,
                                             const uno::Reference< rendering::XTextLayout >&    rTextLayout,
                                             const uno::Reference< rendering::XPolyPolygon2D >& rLinePolygon,
                                             const rendering::ViewState&                        rViewState ) :
                    mrCanvas( rCanvas ),
                    mrTextLayout( rTextLayout ),
                    mrLinePolygon( rLinePolygon ),
                    mrViewState( rViewState )
                {
                }

                // TextRenderer interface
                virtual bool operator()( const rendering::RenderState& rRenderState ) const
                {
                    mrCanvas->fillPolyPolygon( mrLinePolygon,
                                               mrViewState,
                                               rRenderState );

                    mrCanvas->drawTextLayout( mrTextLayout,
                                              mrViewState,
                                              rRenderState );

                    return true;
                }

            private:
                const uno::Reference< rendering::XCanvas >&         mrCanvas;
                const uno::Reference< rendering::XTextLayout >&     mrTextLayout;
                const uno::Reference< rendering::XPolyPolygon2D >&  mrLinePolygon;
                const rendering::ViewState&                         mrViewState;
            };

            bool EffectTextArrayAction::render( const ::basegfx::B2DHomMatrix&  rTransformation,
                                                const Subset&                   rSubset ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::EffectTextArrayAction::render( subset )" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::EffectTextArrayAction: 0x%X", this );

                rendering::RenderState                      aLocalState( maState );
                uno::Reference< rendering::XTextLayout >    xTextLayout( mxTextLayout );

                double nMinPos(0.0);
                double nMaxPos(0.0);

                createSubsetLayout( xTextLayout,
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

                uno::Reference< rendering::XPolyPolygon2D > xTextLines(
                    ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        xCanvas->getDevice(),
                        tools::createTextLinesPolyPolygon(
                            0.0, nMaxPos - nMinPos,
                            maTextLineInfo ) ) );


                // render everything
                // =================

                return renderEffectText(
                    EffectTextArrayRenderHelper( xCanvas,
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
            }

            ::basegfx::B2DRange EffectTextArrayAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return calcEffectTextBounds( ::basegfx::unotools::b2DRectangleFromRealRectangle2D(
                                                 mxTextLayout->queryTextBounds() ),
                                             ::basegfx::B2DRange( 0,0,
                                                                  maLinesOverallSize.getX(),
                                                                  maLinesOverallSize.getY() ),
                                             maReliefOffset,
                                             maShadowOffset,
                                             aLocalState,
                                             mpCanvas->getViewState() );
            }

            ::basegfx::B2DRange EffectTextArrayAction::getBounds( const ::basegfx::B2DHomMatrix&    rTransformation,
                                                                  const Subset&                     rSubset ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::EffectTextArrayAction::getBounds( subset )" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::EffectTextArrayAction: 0x%X", this );

                rendering::RenderState                      aLocalState( maState );
                uno::Reference< rendering::XTextLayout >    xTextLayout( mxTextLayout );

                double nMinPos(0.0);
                double nMaxPos(0.0);

                createSubsetLayout( xTextLayout,
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

                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return calcEffectTextBounds( ::basegfx::unotools::b2DRectangleFromRealRectangle2D(
                                                 xTextLayout->queryTextBounds() ),
                                             ::basegfx::tools::getRange( aPoly ),
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


            // -------------------------------------------------------------------------

            class OutlineAction :
                public Action,
                public TextRenderer,
                private ::boost::noncopyable
            {
            public:
                OutlineAction( const ::Point&                                       rStartPoint,
                               const ::Size&                                        rReliefOffset,
                               const ::Color&                                       rReliefColor,
                               const ::Size&                                        rShadowOffset,
                               const ::Color&                                       rShadowColor,
                               const ::Rectangle&                                   rOutlineBounds,
                               const uno::Reference< rendering::XPolyPolygon2D >&   rTextPoly,
                               const ::std::vector< sal_Int32 >&                    rPolygonGlyphMap,
                               const uno::Sequence< double >&                       rOffsets,
                               VirtualDevice&                                       rVDev,
                               const CanvasSharedPtr&                               rCanvas,
                               const OutDevState&                                   rState  );
                OutlineAction( const ::Point&                                       rStartPoint,
                               const ::Size&                                        rReliefOffset,
                               const ::Color&                                       rReliefColor,
                               const ::Size&                                        rShadowOffset,
                               const ::Color&                                       rShadowColor,
                               const ::Rectangle&                                   rOutlineBounds,
                               const uno::Reference< rendering::XPolyPolygon2D >&   rTextPoly,
                               const ::std::vector< sal_Int32 >&                    rPolygonGlyphMap,
                               const uno::Sequence< double >&                       rOffsets,
                               VirtualDevice&                                       rVDev,
                               const CanvasSharedPtr&                               rCanvas,
                               const OutDevState&                                   rState,
                               const ::basegfx::B2DHomMatrix&                       rTextTransform );

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation,
                                     const Subset&                  rSubset ) const;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const;

                virtual sal_Int32 getActionCount() const;

            private:
                // TextRenderer interface
                virtual bool operator()( const rendering::RenderState& rRenderState ) const;

                // TODO(P2): This is potentially a real mass object
                // (every character might be a separate TextAction),
                // thus, make it as lightweight as possible. For
                // example, share common RenderState among several
                // TextActions, maybe using maOffsets for the
                // translation.

                uno::Reference< rendering::XPolyPolygon2D >         mxTextPoly;

                /** This vector denotes the index of the start polygon
                    for the respective glyph sequence.

                    To get a polygon index range for a given character
                    index i, take [ maPolygonGlyphMap[i],
                    maPolygonGlyphMap[i+1] ). Note that this is wrong
                    for BiDi
                 */
                const ::std::vector< sal_Int32 >                    maPolygonGlyphMap;
                const uno::Sequence< double >                       maOffsets;
                const CanvasSharedPtr                               mpCanvas;
                rendering::RenderState                              maState;
                double                                              mnOutlineWidth;
                const uno::Sequence< double >                       maFillColor;
                const tools::TextLineInfo                           maTextLineInfo;
                ::basegfx::B2DSize                                  maLinesOverallSize;
                const ::Rectangle                                   maOutlineBounds;
                uno::Reference< rendering::XPolyPolygon2D >         mxTextLines;
                const ::Size                                        maReliefOffset;
                const ::Color                                       maReliefColor;
                const ::Size                                        maShadowOffset;
                const ::Color                                       maShadowColor;
            };

            double calcOutlineWidth( VirtualDevice& rVDev )
            {
                const ::Size aFontSize( 0,
                                        rVDev.GetFont().GetHeight() / 32 );

                const double nOutlineWidth(
                    rVDev.LogicToPixel( aFontSize ).Height() );

                return nOutlineWidth < 1.0 ? 1.0 : nOutlineWidth;
            }

            OutlineAction::OutlineAction( const ::Point&                                        rStartPoint,
                                          const ::Size&                                         rReliefOffset,
                                          const ::Color&                                        rReliefColor,
                                          const ::Size&                                         rShadowOffset,
                                          const ::Color&                                        rShadowColor,
                                          const ::Rectangle&                                    rOutlineBounds,
                                          const uno::Reference< rendering::XPolyPolygon2D >&    rTextPoly,
                                          const ::std::vector< sal_Int32 >&                     rPolygonGlyphMap,
                                          const uno::Sequence< double >&                        rOffsets,
                                          VirtualDevice&                                        rVDev,
                                          const CanvasSharedPtr&                                rCanvas,
                                          const OutDevState&                                    rState  ) :
                mxTextPoly( rTextPoly ),
                maPolygonGlyphMap( rPolygonGlyphMap ),
                maOffsets( rOffsets ),
                mpCanvas( rCanvas ),
                maState(),
                mnOutlineWidth( calcOutlineWidth(rVDev) ),
                maFillColor(
                    ::vcl::unotools::colorToDoubleSequence( rCanvas->getUNOCanvas()->getDevice(),
                                                            ::Color( COL_WHITE ) ) ),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maLinesOverallSize(),
                maOutlineBounds( rOutlineBounds ),
                mxTextLines(),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor )
            {
                initEffectLinePolyPolygon( maLinesOverallSize,
                                           mxTextLines,
                                           rCanvas,
                                           rOffsets,
                                           maTextLineInfo );

                init( maState,
                      ::vcl::unotools::b2DPointFromPoint( rStartPoint ),
                      rState,
                      rCanvas );
            }

            OutlineAction::OutlineAction( const ::Point&                                        rStartPoint,
                                          const ::Size&                                         rReliefOffset,
                                          const ::Color&                                        rReliefColor,
                                          const ::Size&                                         rShadowOffset,
                                          const ::Color&                                        rShadowColor,
                                          const ::Rectangle&                                    rOutlineBounds,
                                          const uno::Reference< rendering::XPolyPolygon2D >&    rTextPoly,
                                          const ::std::vector< sal_Int32 >&                     rPolygonGlyphMap,
                                          const uno::Sequence< double >&                        rOffsets,
                                          VirtualDevice&                                        rVDev,
                                          const CanvasSharedPtr&                                rCanvas,
                                          const OutDevState&                                    rState,
                                          const ::basegfx::B2DHomMatrix&                        rTextTransform ) :
                mxTextPoly( rTextPoly ),
                maPolygonGlyphMap( rPolygonGlyphMap ),
                maOffsets( rOffsets ),
                mpCanvas( rCanvas ),
                maState(),
                mnOutlineWidth( calcOutlineWidth(rVDev) ),
                maFillColor(
                    ::vcl::unotools::colorToDoubleSequence( rCanvas->getUNOCanvas()->getDevice(),
                                                            ::Color( COL_WHITE ) ) ),
                maTextLineInfo( tools::createTextLineInfo( rVDev, rState ) ),
                maLinesOverallSize(),
                maOutlineBounds( rOutlineBounds ),
                mxTextLines(),
                maReliefOffset( rReliefOffset ),
                maReliefColor( rReliefColor ),
                maShadowOffset( rShadowOffset ),
                maShadowColor( rShadowColor )
            {
                initEffectLinePolyPolygon( maLinesOverallSize,
                                           mxTextLines,
                                           rCanvas,
                                           rOffsets,
                                           maTextLineInfo );

                init( maState,
                      ::vcl::unotools::b2DPointFromPoint( rStartPoint ),
                      rState,
                      rCanvas,
                      rTextTransform );
            }

            bool OutlineAction::operator()( const rendering::RenderState& rRenderState ) const
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

                // underlines/strikethrough
                rCanvas->fillPolyPolygon( mxTextLines,
                                          rViewState,
                                          rRenderState );

                // background of text
                mpCanvas->getUNOCanvas()->fillPolyPolygon( mxTextPoly,
                                                           rViewState,
                                                           aLocalState );

                // border line of text
                mpCanvas->getUNOCanvas()->strokePolyPolygon( mxTextPoly,
                                                             rViewState,
                                                             rRenderState,
                                                             aStrokeAttributes );

                return true;
            }

            bool OutlineAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::EffectTextArrayAction::render()" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::EffectTextArrayAction: 0x%X", this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return renderEffectText( *this,
                                         aLocalState,
                                         mpCanvas->getViewState(),
                                         mpCanvas->getUNOCanvas(),
                                         maShadowColor,
                                         maShadowOffset,
                                         maReliefColor,
                                         maReliefOffset );
            }

            class OutlineTextArrayRenderHelper : public TextRenderer
            {
            public:
                OutlineTextArrayRenderHelper( const uno::Reference< rendering::XCanvas >&        rCanvas,
                                              const uno::Reference< rendering::XPolyPolygon2D >& rTextPolygon,
                                              const uno::Reference< rendering::XPolyPolygon2D >& rLinePolygon,
                                              const rendering::ViewState&                        rViewState,
                                              double                                             nOutlineWidth ) :
                    maFillColor(
                        ::vcl::unotools::colorToDoubleSequence(
                            rCanvas->getDevice(),
                            ::Color( COL_WHITE ) ) ),
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

                    // underlines/strikethrough
                    mrCanvas->drawPolyPolygon( mrLinePolygon,
                                               mrViewState,
                                               rRenderState );

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

            bool OutlineAction::render( const ::basegfx::B2DHomMatrix&  rTransformation,
                                        const Subset&                   rSubset ) const
            {
                RTL_LOGFILE_CONTEXT( aLog, "::cppcanvas::internal::OutlineAction::render( subset )" );
                RTL_LOGFILE_CONTEXT_TRACE1( aLog, "::cppcanvas::internal::OutlineAction: 0x%X", this );

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

                return calcEffectTextBounds( ::vcl::unotools::b2DRectangleFromRectangle(
                                                 maOutlineBounds ),
                                             ::basegfx::B2DRange( 0,0,
                                                                  maLinesOverallSize.getX(),
                                                                  maLinesOverallSize.getY() ),
                                             maReliefOffset,
                                             maShadowOffset,
                                             aLocalState,
                                             mpCanvas->getViewState() );
            }

            ::basegfx::B2DRange OutlineAction::getBounds( const ::basegfx::B2DHomMatrix&    rTransformation,
                                                                  const Subset&                     rSubset ) const
            {
                OSL_ENSURE( false,
                            "OutlineAction::getBounds(): Subset not yet supported by this object" );

                return getBounds( rTransformation );
            }

            sal_Int32 OutlineAction::getActionCount() const
            {
                // TODO(F3): Subsetting NYI for outline text!
                return maOffsets.getLength();
            }


            // ======================================================================
            //
            // Action factory methods
            //
            // ======================================================================

            /** Create an outline action

                This method extracts the polygonal outline from the
                text, and creates a properly setup OutlineAction from
                it.
             */
            ActionSharedPtr createOutline( const ::Point&                   rStartPoint,
                                           const ::Size&                    rReliefOffset,
                                           const ::Color&                   rReliefColor,
                                           const ::Size&                    rShadowOffset,
                                           const ::Color&                   rShadowColor,
                                           const String&                    rText,
                                           sal_Int32                        nStartPos,
                                           sal_Int32                        nLen,
                                           const sal_Int32*                 pDXArray,
                                           VirtualDevice&                   rVDev,
                                           const CanvasSharedPtr&           rCanvas,
                                           const OutDevState&               rState,
                                           const Renderer::Parameters&      rParms  )
            {
                // operate on raw DX array here (in logical coordinate
                // system), to have a higher resolution
                // PolyPolygon. That polygon is then converted to
                // device coordinate system.

                // TODO(F3): Don't understand parameter semantics of
                // GetTextOutlines()
                ::PolyPolygon   aResultingVCLPolyPolygon;
                PolyPolyVector  aVCLPolyPolyVector;
                if( rVDev.GetTextOutlines( aVCLPolyPolyVector, rText,
                                           static_cast<USHORT>(nStartPos),
                                           static_cast<USHORT>(nStartPos),
                                           static_cast<USHORT>(nLen),
                                           TRUE, 0, pDXArray ) )
                {
                    ::std::vector< sal_Int32 > aPolygonGlyphMap;

                    // first glyph starts at polygon index 0
                    aPolygonGlyphMap.push_back( 0 );

                    // temporarily remove offsetting from mapmode
                    // (outline polygons must stay at origin, only
                    // need to be scaled)
                    const ::MapMode aOldMapMode( rVDev.GetMapMode() );

                    ::MapMode aScaleOnlyMapMode( aOldMapMode );
                    aScaleOnlyMapMode.SetOrigin( ::Point() );
                    rVDev.SetMapMode( aScaleOnlyMapMode );

                    PolyPolyVector::const_iterator       aIter( aVCLPolyPolyVector.begin() );
                    const PolyPolyVector::const_iterator aEnd( aVCLPolyPolyVector.end() );
                    for( ; aIter!= aEnd; ++aIter )
                    {
                        ::PolyPolygon aVCLPolyPolygon;

                        aVCLPolyPolygon = rVDev.LogicToPixel( *aIter );

                        // append result to collecting polypoly
                        for( USHORT i=0; i<aVCLPolyPolygon.Count(); ++i )
                        {
                            // #i47795# Ensure closed polygons (since
                            // FreeType returns the glyph outlines
                            // open)
                            const ::Polygon& rPoly( aVCLPolyPolygon.GetObject( i ) );
                            const USHORT nCount( rPoly.GetSize() );
                            if( nCount<3 ||
                                rPoly[0] == rPoly[nCount-1] )
                            {
                                // polygon either degenerate, or
                                // already closed.
                                aResultingVCLPolyPolygon.Insert( rPoly );
                            }
                            else
                            {
                                ::Polygon aPoly(nCount+1);

                                // close polygon explicitely
                                if( rPoly.HasFlags() )
                                {
                                    for( USHORT j=0; j<nCount; ++j )
                                    {
                                        aPoly[j] = rPoly[j];
                                        aPoly.SetFlags(j, rPoly.GetFlags(j));
                                    }

                                    // duplicate first point
                                    aPoly[nCount] = rPoly[0];
                                    aPoly.SetFlags(nCount, POLY_NORMAL);
                                }
                                else
                                {
                                    for( USHORT j=0; j<nCount; ++j )
                                    {
                                        aPoly[j] = rPoly[j];
                                    }

                                    // duplicate first point
                                    aPoly[nCount] = rPoly[0];
                                }

                                aResultingVCLPolyPolygon.Insert( aPoly );
                            }
                        }

                        // TODO(F3): Depending on the semantics of
                        // GetTextOutlines(), this here is wrong!

                        // calc next glyph index
                        aPolygonGlyphMap.push_back( aResultingVCLPolyPolygon.Count() );
                    }

                    rVDev.SetMapMode( aOldMapMode );

                    const uno::Sequence< double > aCharWidthSeq(
                        pDXArray ?
                        setupDXArray( pDXArray, nLen, rVDev ) :
                        setupDXArray( rText,
                                      nStartPos,
                                      nLen,
                                      rVDev ) );
                    const uno::Reference< rendering::XPolyPolygon2D > xTextPoly(
                        ::vcl::unotools::xPolyPolygonFromPolyPolygon(
                            rCanvas->getUNOCanvas()->getDevice(),
                            aResultingVCLPolyPolygon ) );

                    ::Point aEmptyPoint;
                    if( rParms.maTextTransformation.isValid() )
                    {
                        return ActionSharedPtr(
                            new OutlineAction(
                                rStartPoint,
                                rReliefOffset,
                                rReliefColor,
                                rShadowOffset,
                                rShadowColor,
                                aResultingVCLPolyPolygon.GetBoundRect(),
                                xTextPoly,
                                aPolygonGlyphMap,
                                aCharWidthSeq,
                                rVDev,
                                rCanvas,
                                rState,
                                rParms.maTextTransformation.getValue() ) );
                    }
                    else
                    {
                        return ActionSharedPtr(
                            new OutlineAction(
                                rStartPoint,
                                rReliefOffset,
                                rReliefColor,
                                rShadowOffset,
                                rShadowColor,
                                aResultingVCLPolyPolygon.GetBoundRect(),
                                xTextPoly,
                                aPolygonGlyphMap,
                                aCharWidthSeq,
                                rVDev,
                                rCanvas,
                                rState  ) );
                    }
                }

                return ActionSharedPtr();
            }

        }


        // ---------------------------------------------------------------------------------

        ActionSharedPtr TextActionFactory::createTextAction( const ::Point&                 rStartPoint,
                                                             const ::Size&                  rReliefOffset,
                                                             const ::Color&                 rReliefColor,
                                                             const ::Size&                  rShadowOffset,
                                                             const ::Color&                 rShadowColor,
                                                             const String&                  rText,
                                                             sal_Int32                      nStartPos,
                                                             sal_Int32                      nLen,
                                                             const sal_Int32*               pDXArray,
                                                             VirtualDevice&                 rVDev,
                                                             const CanvasSharedPtr&         rCanvas,
                                                             const OutDevState&             rState,
                                                             const Renderer::Parameters&    rParms,
                                                             bool                           bSubsettable    )
        {
            const ::Size  aBaselineOffset( tools::getBaselineOffset( rState,
                                                                     rVDev ) );
            const ::Point aStartPoint( rVDev.LogicToPixel(
                                           ::Point(
                                               rStartPoint.X() + aBaselineOffset.Width(),
                                               rStartPoint.Y() + aBaselineOffset.Height() ) ) );
            const ::Size  aReliefOffset( rVDev.LogicToPixel( rReliefOffset ) );
            const ::Size  aShadowOffset( rVDev.LogicToPixel( rShadowOffset ) );

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
                setupDXArray( pDXArray, nLen, rVDev ) :
                setupDXArray( rText,
                              nStartPos,
                              nLen,
                              rVDev ) );

            // determine type of text action to create
            // =======================================

            const ::Color aEmptyColor( COL_AUTO );

            // no DX array, and no need to subset - no need to store
            // DX array, then.
            if( !pDXArray && !bSubsettable )
            {
                // effects, or not?
                if( !rState.textUnderlineStyle &&
                    !rState.textStrikeoutStyle &&
                    rReliefColor == aEmptyColor &&
                    rShadowColor == aEmptyColor )
                {
                    // nope
                    if( rParms.maTextTransformation.isValid() )
                    {
                        return ActionSharedPtr( new TextAction(
                                                    aStartPoint,
                                                    rText,
                                                    nStartPos,
                                                    nLen,
                                                    rVDev,
                                                    rCanvas,
                                                    rState,
                                                    rParms.maTextTransformation.getValue() ) );
                    }
                    else
                    {
                        return ActionSharedPtr( new TextAction(
                                                    aStartPoint,
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
                    // at least one of the effects requested
                    if( rParms.maTextTransformation.isValid() )
                        return ActionSharedPtr( new EffectTextAction(
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
                                                    rParms.maTextTransformation.getValue() ) );
                    else
                        return ActionSharedPtr( new EffectTextAction(
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
                if( !rState.textUnderlineStyle &&
                    !rState.textStrikeoutStyle &&
                    rReliefColor == aEmptyColor &&
                    rShadowColor == aEmptyColor )
                {
                    // nope
                    if( rParms.maTextTransformation.isValid() )
                        return ActionSharedPtr( new TextArrayAction(
                                                    aStartPoint,
                                                    rText,
                                                    nStartPos,
                                                    nLen,
                                                    aCharWidths,
                                                    rVDev,
                                                    rCanvas,
                                                    rState,
                                                    rParms.maTextTransformation.getValue() ) );
                    else
                        return ActionSharedPtr( new TextArrayAction(
                                                    aStartPoint,
                                                    rText,
                                                    nStartPos,
                                                    nLen,
                                                    aCharWidths,
                                                    rVDev,
                                                    rCanvas,
                                                    rState ) );
                }
                else
                {
                    // at least one of the effects requested
                    if( rParms.maTextTransformation.isValid() )
                        return ActionSharedPtr( new EffectTextArrayAction(
                                                    aStartPoint,
                                                    aReliefOffset,
                                                    rReliefColor,
                                                    aShadowOffset,
                                                    rShadowColor,
                                                    rText,
                                                    nStartPos,
                                                    nLen,
                                                    aCharWidths,
                                                    rVDev,
                                                    rCanvas,
                                                    rState,
                                                    rParms.maTextTransformation.getValue() ) );
                    else
                        return ActionSharedPtr( new EffectTextArrayAction(
                                                    aStartPoint,
                                                    aReliefOffset,
                                                    rReliefColor,
                                                    aShadowOffset,
                                                    rShadowColor,
                                                    rText,
                                                    nStartPos,
                                                    nLen,
                                                    aCharWidths,
                                                    rVDev,
                                                    rCanvas,
                                                    rState ) );
                }
            }
        }
    }
}
