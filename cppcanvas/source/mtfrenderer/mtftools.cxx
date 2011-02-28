/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <canvas/canvastools.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <tools/poly.hxx>
#include "mtftools.hxx"
#include "outdevstate.hxx"
#include "polypolyaction.hxx"
#include <basegfx/matrix/b2dhommatrixtools.hxx>



using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace tools
    {
        void initRenderState( rendering::RenderState&                   renderState,
                              const ::cppcanvas::internal::OutDevState& outdevState )
        {
            ::canvas::tools::initRenderState( renderState );
            ::canvas::tools::setRenderStateTransform( renderState,
                                                      outdevState.transform );
            renderState.Clip = outdevState.xClipPoly;
        }

        ::Size getBaselineOffset( const ::cppcanvas::internal::OutDevState& outdevState,
                                  const VirtualDevice&                      rVDev )
        {
            const ::FontMetric& aMetric = rVDev.GetFontMetric();

            // calc offset for text output, the XCanvas always renders
            // baseline offset.
            switch( outdevState.textReferencePoint )
            {
                case ALIGN_TOP:
                    return ::Size( 0,
                                   aMetric.GetIntLeading() + aMetric.GetAscent() );

                default:
                    ENSURE_OR_THROW( false,
                                      "tools::getBaselineOffset(): Unexpected TextAlign value" );
                    // FALLTHROUGH intended (to calm compiler warning - case won't happen)
                case ALIGN_BASELINE:
                    return ::Size( 0, 0 );

                case ALIGN_BOTTOM:
                    return ::Size( 0,
                                   -aMetric.GetDescent() );

            }
        }

        ::basegfx::B2DHomMatrix& calcLogic2PixelLinearTransform( ::basegfx::B2DHomMatrix&   o_rMatrix,
                                                                 const VirtualDevice&       rVDev )
        {
            // select size value in the middle of the available range,
            // to have headroom both when map mode scales up, and when
            // it scales down.
            const ::Size aSizeLogic( 0x00010000L,
                                     0x00010000L );

            const ::Size aSizePixel( rVDev.LogicToPixel( aSizeLogic ) );

            o_rMatrix = basegfx::tools::createScaleB2DHomMatrix(
                aSizePixel.Width() / (double)aSizeLogic.Width(),
                aSizePixel.Height() / (double)aSizeLogic.Height() );

            return o_rMatrix;
        }

        ::basegfx::B2DHomMatrix& calcLogic2PixelAffineTransform( ::basegfx::B2DHomMatrix&   o_rMatrix,
                                                                 const VirtualDevice&       rVDev )
        {
            // retrieves scale
            calcLogic2PixelLinearTransform(o_rMatrix, rVDev);

            // translate according to curr map mode/pref map mode offset
            const ::Point  aEmptyPoint;
            const ::Point& rTranslatedPoint(
                rVDev.LogicToPixel( aEmptyPoint ));

            o_rMatrix.translate(rTranslatedPoint.X(),
                                rTranslatedPoint.Y());

            return o_rMatrix;
        }

        bool modifyClip( rendering::RenderState&                            o_rRenderState,
                         const struct ::cppcanvas::internal::OutDevState&   rOutdevState,
                         const CanvasSharedPtr&                             rCanvas,
                         const ::basegfx::B2DPoint&                         rOffset,
                         const ::basegfx::B2DVector*                        pScaling,
                         const double*                                      pRotation )
        {
            const ::Point aEmptyPoint;

            const bool bOffsetting( !rOffset.equalZero() );
            const bool bScaling( pScaling &&
                                 pScaling->getX() != 1.0 &&
                                 pScaling->getY() != 1.0 );
            const bool bRotation( pRotation &&
                                  *pRotation != 0.0 );

            if( !bOffsetting && !bScaling && !bRotation )
                return false; // nothing to do

            if( rOutdevState.clip.count() )
            {
                // general polygon case

                ::basegfx::B2DPolyPolygon aLocalClip( rOutdevState.clip );
                ::basegfx::B2DHomMatrix   aTransform;

                if( bOffsetting )
                    aTransform.translate( -rOffset.getX(),
                                          -rOffset.getY() );
                if( bScaling )
                    aTransform.scale( 1.0/pScaling->getX(), 1.0/pScaling->getY() );

                if( bRotation )
                    aTransform.rotate( - *pRotation );

                aLocalClip.transform( aTransform );

                o_rRenderState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    rCanvas->getUNOCanvas()->getDevice(),
                    aLocalClip );

                return true;
            }
            else if( !rOutdevState.clipRect.IsEmpty() )
            {
                // simple rect case

                const ::Rectangle aLocalClipRect( rOutdevState.clipRect );

                if( bRotation )
                {
                    // rotation involved - convert to polygon first,
                    // then transform that
                    ::basegfx::B2DPolygon aLocalClip(
                        ::basegfx::tools::createPolygonFromRect(
                                ::basegfx::B2DRectangle(
                                    (double)(aLocalClipRect.Left()),
                                    (double)(aLocalClipRect.Top()),
                                    (double)(aLocalClipRect.Right()),
                                    (double)(aLocalClipRect.Bottom()) ) ) );
                    ::basegfx::B2DHomMatrix aTransform;

                    if( bOffsetting )
                        aTransform.translate( -rOffset.getX(),
                                              -rOffset.getY() );
                    if( bScaling )
                        aTransform.scale( 1.0/pScaling->getX(), 1.0/pScaling->getY() );

                    aTransform.rotate( - *pRotation );

                    aLocalClip.transform( aTransform );

                    o_rRenderState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rCanvas->getUNOCanvas()->getDevice(),
                        ::basegfx::B2DPolyPolygon( aLocalClip ) );
                }
                else if( bScaling )
                {
                    // scale and offset - do it on the fly, have to
                    // convert to float anyway.
                    o_rRenderState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rCanvas->getUNOCanvas()->getDevice(),
                        ::basegfx::B2DPolyPolygon(
                            ::basegfx::tools::createPolygonFromRect(
                                ::basegfx::B2DRectangle(
                                    (double)(aLocalClipRect.Left() - rOffset.getX())/pScaling->getX(),
                                    (double)(aLocalClipRect.Top() - rOffset.getY())/pScaling->getY(),
                                    (double)(aLocalClipRect.Right() - rOffset.getX())/pScaling->getX(),
                                    (double)(aLocalClipRect.Bottom() - rOffset.getY())/pScaling->getY() ) ) ) );
                }
                else
                {
                    // offset only - do it on the fly, have to convert
                    // to float anyway.
                    o_rRenderState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rCanvas->getUNOCanvas()->getDevice(),
                        ::basegfx::B2DPolyPolygon(
                            ::basegfx::tools::createPolygonFromRect(
                                ::basegfx::B2DRectangle( aLocalClipRect.Left() - rOffset.getX(),
                                                         aLocalClipRect.Top() - rOffset.getY(),
                                                         aLocalClipRect.Right() - rOffset.getX(),
                                                         aLocalClipRect.Bottom() - rOffset.getY() ) ) ) );
                }

                return true;
            }

            // empty clip, nothing to do
            return false;
        }

        bool modifyClip( rendering::RenderState&                            o_rRenderState,
                         const struct ::cppcanvas::internal::OutDevState&   rOutdevState,
                         const CanvasSharedPtr&                             rCanvas,
                         const ::Point&                                     rOffset,
                         const ::basegfx::B2DVector*                        pScaling,
                         const double*                                      pRotation )
        {
            return modifyClip( o_rRenderState,
                               rOutdevState,
                               rCanvas,
                               ::basegfx::B2DPoint( rOffset.X(),
                                                    rOffset.Y() ),
                               pScaling,
                               pRotation );
        }

        bool modifyClip( rendering::RenderState&                            o_rRenderState,
                         const struct ::cppcanvas::internal::OutDevState&   rOutdevState,
                         const CanvasSharedPtr&                             rCanvas,
                         const ::basegfx::B2DHomMatrix&                     rTransform )
        {
            if( !rTransform.isIdentity() ||
                !rTransform.isInvertible() )
                return false; // nothing to do

            ::basegfx::B2DPolyPolygon aLocalClip;

            if( rOutdevState.clip.count() )
            {
                aLocalClip = rOutdevState.clip;
            }
            else if( !rOutdevState.clipRect.IsEmpty() )
            {
                const ::Rectangle aLocalClipRect( rOutdevState.clipRect );

                aLocalClip = ::basegfx::B2DPolyPolygon(
                    ::basegfx::tools::createPolygonFromRect(
                        ::basegfx::B2DRectangle(
                            aLocalClipRect.Left(),
                            aLocalClipRect.Top(),
                            aLocalClipRect.Right(),
                            aLocalClipRect.Bottom() ) ) );
            }
            else
            {
                // empty clip, nothing to do
                return false;
            }

            // invert transformation and modify
            ::basegfx::B2DHomMatrix aLocalTransform( rTransform );
            aLocalTransform.invert();

            aLocalClip.transform( aLocalTransform );

            o_rRenderState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                rCanvas->getUNOCanvas()->getDevice(),
                aLocalClip );

            return true;
        }

        // create overline/underline/strikeout line info struct
        TextLineInfo createTextLineInfo( const ::VirtualDevice&                     rVDev,
                                         const ::cppcanvas::internal::OutDevState&  rState )
        {
            const sal_Bool bOldMode( rVDev.IsMapModeEnabled() );

            // #i68512# Force metric regeneration with mapmode enabled
            // (prolly OutDev bug)
            rVDev.GetFontMetric();

            // will restore map mode below
            const_cast< ::VirtualDevice& >(rVDev).EnableMapMode( sal_False );

            const ::FontMetric aMetric = rVDev.GetFontMetric();

            TextLineInfo aTextInfo(
                (aMetric.GetDescent() + 2) / 4.0,
                ((aMetric.GetIntLeading() + 1.5) / 3.0),
                (aMetric.GetIntLeading() / 2.0) - aMetric.GetAscent(),
                aMetric.GetDescent() / 2.0,
                (aMetric.GetIntLeading() - aMetric.GetAscent()) / 3.0,
                rState.textOverlineStyle,
                rState.textUnderlineStyle,
                rState.textStrikeoutStyle );

            const_cast< ::VirtualDevice& >(rVDev).EnableMapMode( bOldMode );

            return aTextInfo;
        }

        namespace
        {
            void appendRect( ::basegfx::B2DPolyPolygon& o_rPoly,
                             const ::basegfx::B2DPoint& rStartPos,
                             const double               nX1,
                             const double               nY1,
                             const double               nX2,
                             const double               nY2 )
            {
                const double x( rStartPos.getX() );
                const double y( rStartPos.getY() );

                o_rPoly.append(
                    ::basegfx::tools::createPolygonFromRect(
                        ::basegfx::B2DRectangle( x + nX1, y + nY1, x + nX2, y + nY2 ) ) );
            }

            void appendRect( ::basegfx::B2DPolyPolygon& o_rPoly,
                             const double               nX1,
                             const double               nY1,
                             const double               nX2,
                             const double               nY2 )
            {
                o_rPoly.append(
                    ::basegfx::tools::createPolygonFromRect(
                        ::basegfx::B2DRectangle( nX1, nY1, nX2, nY2 ) ) );
            }

            void appendDashes( ::basegfx::B2DPolyPolygon&   o_rPoly,
                               const double                 nX,
                               const double                 nY,
                               const double                 nLineWidth,
                               const double                 nLineHeight,
                               const double                 nDashWidth,
                               const double                 nDashSkip )
            {
                const sal_Int32 nNumLoops(
                    static_cast< sal_Int32 >(
                        ::std::max( 1.0,
                                    nLineWidth / nDashSkip ) + .5) );

                double x = nX;
                for( sal_Int32 i=0; i<nNumLoops; ++i )
                {
                    appendRect( o_rPoly,
                                x,              nY,
                                x + nDashWidth, nY + nLineHeight );

                    x += nDashSkip;
                }
            }
        }

        // create line actions for text such as underline and
        // strikeout
        ::basegfx::B2DPolyPolygon createTextLinesPolyPolygon( const ::basegfx::B2DPoint rStartPos,
                                                              const double&             rLineWidth,
                                                              const TextLineInfo&       rTextLineInfo )
        {
            // fill the polypolygon with all text lines
            ::basegfx::B2DPolyPolygon aTextLinesPolyPoly;

            switch( rTextLineInfo.mnOverlineStyle )
            {
                case UNDERLINE_NONE:          // nothing to do
                    // FALLTHROUGH intended
                case UNDERLINE_DONTKNOW:
                    break;

                case UNDERLINE_SMALLWAVE:     // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_WAVE:          // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_SINGLE:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnOverlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnOverlineOffset + rTextLineInfo.mnOverlineHeight );
                    break;

                case UNDERLINE_BOLDDOTTED:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLDDASH:      // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLDLONGDASH:  // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLDDASHDOT:   // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLDDASHDOTDOT:// TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLDWAVE:      // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLD:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnOverlineOffset - rTextLineInfo.mnOverlineHeight,
                        rLineWidth,
                        rTextLineInfo.mnOverlineOffset + rTextLineInfo.mnOverlineHeight );
                    break;

                case UNDERLINE_DOUBLEWAVE:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_DOUBLE:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnOverlineOffset - rTextLineInfo.mnOverlineHeight * 2.0 ,
                        rLineWidth,
                        rTextLineInfo.mnOverlineOffset - rTextLineInfo.mnOverlineHeight );

                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnOverlineOffset + rTextLineInfo.mnOverlineHeight,
                        rLineWidth,
                        rTextLineInfo.mnOverlineOffset + rTextLineInfo.mnOverlineHeight * 2.0 );
                    break;

                case UNDERLINE_DASHDOTDOT:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_DOTTED:
                    appendDashes(
                        aTextLinesPolyPoly,
                        rStartPos.getX(),
                        rStartPos.getY() + rTextLineInfo.mnOverlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnOverlineHeight,
                        rTextLineInfo.mnOverlineHeight,
                        2*rTextLineInfo.mnOverlineHeight );
                    break;

                case UNDERLINE_DASHDOT:       // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_DASH:
                    appendDashes(
                        aTextLinesPolyPoly,
                        rStartPos.getX(),
                        rStartPos.getY() + rTextLineInfo.mnOverlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnOverlineHeight,
                        3*rTextLineInfo.mnOverlineHeight,
                        6*rTextLineInfo.mnOverlineHeight );
                    break;

                case UNDERLINE_LONGDASH:
                    appendDashes(
                        aTextLinesPolyPoly,
                        rStartPos.getX(),
                        rStartPos.getY() + rTextLineInfo.mnOverlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnOverlineHeight,
                        6*rTextLineInfo.mnOverlineHeight,
                        12*rTextLineInfo.mnOverlineHeight );
                    break;

                default:
                    ENSURE_OR_THROW( false,
                                      "::cppcanvas::internal::createTextLinesPolyPolygon(): Unexpected overline case" );
            }

            switch( rTextLineInfo.mnUnderlineStyle )
            {
                case UNDERLINE_NONE:          // nothing to do
                    // FALLTHROUGH intended
                case UNDERLINE_DONTKNOW:
                    break;

                case UNDERLINE_SMALLWAVE:     // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_WAVE:          // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_SINGLE:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnUnderlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnUnderlineOffset + rTextLineInfo.mnLineHeight );
                    break;

                case UNDERLINE_BOLDDOTTED:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLDDASH:      // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLDLONGDASH:  // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLDDASHDOT:   // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLDDASHDOTDOT:// TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLDWAVE:      // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_BOLD:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnUnderlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnUnderlineOffset + 2*rTextLineInfo.mnLineHeight );
                    break;

                case UNDERLINE_DOUBLEWAVE:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_DOUBLE:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnUnderlineOffset - rTextLineInfo.mnLineHeight,
                        rLineWidth,
                        rTextLineInfo.mnUnderlineOffset );

                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnUnderlineOffset + 2*rTextLineInfo.mnLineHeight,
                        rLineWidth,
                        rTextLineInfo.mnUnderlineOffset + 3*rTextLineInfo.mnLineHeight );
                    break;

                case UNDERLINE_DASHDOTDOT:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_DOTTED:
                    appendDashes(
                        aTextLinesPolyPoly,
                        rStartPos.getX(),
                        rStartPos.getY() + rTextLineInfo.mnUnderlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnLineHeight,
                        rTextLineInfo.mnLineHeight,
                        2*rTextLineInfo.mnLineHeight );
                    break;

                case UNDERLINE_DASHDOT:       // TODO(F3): NYI
                    // FALLTHROUGH intended
                case UNDERLINE_DASH:
                    appendDashes(
                        aTextLinesPolyPoly,
                        rStartPos.getX(),
                        rStartPos.getY() + rTextLineInfo.mnUnderlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnLineHeight,
                        3*rTextLineInfo.mnLineHeight,
                        6*rTextLineInfo.mnLineHeight );
                    break;

                case UNDERLINE_LONGDASH:
                    appendDashes(
                        aTextLinesPolyPoly,
                        rStartPos.getX(),
                        rStartPos.getY() + rTextLineInfo.mnUnderlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnLineHeight,
                        6*rTextLineInfo.mnLineHeight,
                        12*rTextLineInfo.mnLineHeight );
                    break;

                default:
                    ENSURE_OR_THROW( false,
                                      "::cppcanvas::internal::createTextLinesPolyPolygon(): Unexpected underline case" );
            }

            switch( rTextLineInfo.mnStrikeoutStyle )
            {
                case STRIKEOUT_NONE:    // nothing to do
                    // FALLTHROUGH intended
                case STRIKEOUT_DONTKNOW:
                    break;

                case STRIKEOUT_SLASH:   // TODO(Q1): we should handle this in the text layer
                    // FALLTHROUGH intended
                case STRIKEOUT_X:
                    break;

                case STRIKEOUT_SINGLE:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnStrikeoutOffset,
                        rLineWidth,
                        rTextLineInfo.mnStrikeoutOffset + rTextLineInfo.mnLineHeight );
                    break;

                case STRIKEOUT_BOLD:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnStrikeoutOffset,
                        rLineWidth,
                        rTextLineInfo.mnStrikeoutOffset + 2*rTextLineInfo.mnLineHeight );
                    break;

                case STRIKEOUT_DOUBLE:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnStrikeoutOffset - rTextLineInfo.mnLineHeight,
                        rLineWidth,
                        rTextLineInfo.mnStrikeoutOffset );

                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnStrikeoutOffset + 2*rTextLineInfo.mnLineHeight,
                        rLineWidth,
                        rTextLineInfo.mnStrikeoutOffset + 3*rTextLineInfo.mnLineHeight );
                    break;

                default:
                    ENSURE_OR_THROW( false,
                                      "::cppcanvas::internal::createTextLinesPolyPolygon(): Unexpected strikeout case" );
            }

            return aTextLinesPolyPoly;
        }

        ::basegfx::B2DRange calcDevicePixelBounds( const ::basegfx::B2DRange&       rBounds,
                                                   const rendering::ViewState&      viewState,
                                                   const rendering::RenderState&    renderState )
        {
            ::basegfx::B2DHomMatrix aTransform;
            ::canvas::tools::mergeViewAndRenderTransform( aTransform,
                                                          viewState,
                                                          renderState );

            ::basegfx::B2DRange aTransformedBounds;
            return ::canvas::tools::calcTransformedRectBounds( aTransformedBounds,
                                                               rBounds,
                                                               aTransform );
        }

        // create line actions for text such as underline and
        // strikeout
        ::basegfx::B2DPolyPolygon createTextLinesPolyPolygon( const double&         rStartOffset,
                                                              const double&         rLineWidth,
                                                              const TextLineInfo&   rTextLineInfo )
        {
            return createTextLinesPolyPolygon(
                ::basegfx::B2DPoint( rStartOffset,
                                     0.0 ),
                rLineWidth,
                rTextLineInfo );
        }
    }
}
