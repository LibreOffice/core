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
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/utils/canvastools.hxx>
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
                                   aMetric.GetInternalLeading() + aMetric.GetAscent() );

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

            o_rMatrix = basegfx::utils::createScaleB2DHomMatrix(
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
            const bool bOffsetting( !rOffset.equalZero() );
            const bool bScaling( pScaling &&
                                 !rtl::math::approxEqual(pScaling->getX(), 1.0) &&
                                 !rtl::math::approxEqual(pScaling->getY(), 1.0) );
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

                const ::tools::Rectangle aLocalClipRect( rOutdevState.clipRect );

                if( bRotation )
                {
                    // rotation involved - convert to polygon first,
                    // then transform that
                    ::basegfx::B2DPolygon aLocalClip(
                        ::basegfx::utils::createPolygonFromRect(
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
                            ::basegfx::utils::createPolygonFromRect(
                                ::basegfx::B2DRectangle(
                                    (aLocalClipRect.Left() - rOffset.getX())/pScaling->getX(),
                                    (aLocalClipRect.Top() - rOffset.getY())/pScaling->getY(),
                                    (aLocalClipRect.Right() - rOffset.getX())/pScaling->getX(),
                                    (aLocalClipRect.Bottom() - rOffset.getY())/pScaling->getY() ) ) ) );
                }
                else
                {
                    // offset only - do it on the fly, have to convert
                    // to float anyway.
                    o_rRenderState.Clip = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rCanvas->getUNOCanvas()->getDevice(),
                        ::basegfx::B2DPolyPolygon(
                            ::basegfx::utils::createPolygonFromRect(
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

        // create overline/underline/strikeout line info struct
        TextLineInfo createTextLineInfo( const ::VirtualDevice&                     rVDev,
                                         const ::cppcanvas::internal::OutDevState&  rState )
        {
            const bool bOldMode( rVDev.IsMapModeEnabled() );

            // #i68512# Force metric regeneration with mapmode enabled
            // (prolly OutDev bug)
            rVDev.GetFontMetric();

            // will restore map mode below
            const_cast< ::VirtualDevice& >(rVDev).EnableMapMode( false );

            const ::FontMetric aMetric = rVDev.GetFontMetric();

            TextLineInfo aTextInfo(
                (aMetric.GetDescent() + 2) / 4.0,
                ((aMetric.GetInternalLeading() + 1.5) / 3.0),
                (aMetric.GetInternalLeading() / 2.0) - aMetric.GetAscent(),
                aMetric.GetDescent() / 2.0,
                (aMetric.GetInternalLeading() - aMetric.GetAscent()) / 3.0,
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
                    ::basegfx::utils::createPolygonFromRect(
                        ::basegfx::B2DRectangle( x + nX1, y + nY1, x + nX2, y + nY2 ) ) );
            }

            void appendRect( ::basegfx::B2DPolyPolygon& o_rPoly,
                             const double               nX1,
                             const double               nY1,
                             const double               nX2,
                             const double               nY2 )
            {
                o_rPoly.append(
                    ::basegfx::utils::createPolygonFromRect(
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
                        std::max( 1.0,
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
        ::basegfx::B2DPolyPolygon createTextLinesPolyPolygon( const ::basegfx::B2DPoint& rStartPos,
                                                              const double&              rLineWidth,
                                                              const TextLineInfo&        rTextLineInfo )
        {
            // fill the polypolygon with all text lines
            ::basegfx::B2DPolyPolygon aTextLinesPolyPoly;

            switch( rTextLineInfo.mnOverlineStyle )
            {
                case LINESTYLE_NONE:          // nothing to do
                    // FALLTHROUGH intended
                case LINESTYLE_DONTKNOW:
                    break;

                case LINESTYLE_SMALLWAVE:     // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_WAVE:          // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_SINGLE:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnOverlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnOverlineOffset + rTextLineInfo.mnOverlineHeight );
                    break;

                case LINESTYLE_BOLDDOTTED:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLDDASH:      // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLDLONGDASH:  // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLDDASHDOT:   // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLDDASHDOTDOT:// TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLDWAVE:      // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLD:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnOverlineOffset - rTextLineInfo.mnOverlineHeight,
                        rLineWidth,
                        rTextLineInfo.mnOverlineOffset + rTextLineInfo.mnOverlineHeight );
                    break;

                case LINESTYLE_DOUBLEWAVE:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_DOUBLE:
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

                case LINESTYLE_DASHDOTDOT:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_DOTTED:
                    appendDashes(
                        aTextLinesPolyPoly,
                        rStartPos.getX(),
                        rStartPos.getY() + rTextLineInfo.mnOverlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnOverlineHeight,
                        rTextLineInfo.mnOverlineHeight,
                        2*rTextLineInfo.mnOverlineHeight );
                    break;

                case LINESTYLE_DASHDOT:       // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_DASH:
                    appendDashes(
                        aTextLinesPolyPoly,
                        rStartPos.getX(),
                        rStartPos.getY() + rTextLineInfo.mnOverlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnOverlineHeight,
                        3*rTextLineInfo.mnOverlineHeight,
                        6*rTextLineInfo.mnOverlineHeight );
                    break;

                case LINESTYLE_LONGDASH:
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
                case LINESTYLE_NONE:          // nothing to do
                    // FALLTHROUGH intended
                case LINESTYLE_DONTKNOW:
                    break;

                case LINESTYLE_SMALLWAVE:     // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_WAVE:          // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_SINGLE:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnUnderlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnUnderlineOffset + rTextLineInfo.mnLineHeight );
                    break;

                case LINESTYLE_BOLDDOTTED:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLDDASH:      // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLDLONGDASH:  // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLDDASHDOT:   // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLDDASHDOTDOT:// TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLDWAVE:      // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_BOLD:
                    appendRect(
                        aTextLinesPolyPoly,
                        rStartPos,
                        0,
                        rTextLineInfo.mnUnderlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnUnderlineOffset + 2*rTextLineInfo.mnLineHeight );
                    break;

                case LINESTYLE_DOUBLEWAVE:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_DOUBLE:
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

                case LINESTYLE_DASHDOTDOT:    // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_DOTTED:
                    appendDashes(
                        aTextLinesPolyPoly,
                        rStartPos.getX(),
                        rStartPos.getY() + rTextLineInfo.mnUnderlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnLineHeight,
                        rTextLineInfo.mnLineHeight,
                        2*rTextLineInfo.mnLineHeight );
                    break;

                case LINESTYLE_DASHDOT:       // TODO(F3): NYI
                    // FALLTHROUGH intended
                case LINESTYLE_DASH:
                    appendDashes(
                        aTextLinesPolyPoly,
                        rStartPos.getX(),
                        rStartPos.getY() + rTextLineInfo.mnUnderlineOffset,
                        rLineWidth,
                        rTextLineInfo.mnLineHeight,
                        3*rTextLineInfo.mnLineHeight,
                        6*rTextLineInfo.mnLineHeight );
                    break;

                case LINESTYLE_LONGDASH:
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
