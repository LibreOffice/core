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


#include <comphelper/diagnose_ex.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <canvas/canvastools.hxx>
#include <rtl/math.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include "mtftools.hxx"
#include <outdevstate.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>


using namespace ::com::sun::star;

namespace cppcanvas::tools
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

                case ALIGN_BASELINE:
                    return ::Size( 0, 0 );

                case ALIGN_BOTTOM:
                    return ::Size( 0,
                                   -aMetric.GetDescent() );

                default:
                    throw css::uno::RuntimeException(
                                      u"tools::getBaselineOffset(): Unexpected TextAlign value"_ustr );
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
                aSizePixel.Width() / static_cast<double>(aSizeLogic.Width()),
                aSizePixel.Height() / static_cast<double>(aSizeLogic.Height()) );

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
                                    vcl::unotools::b2DRectangleFromRectangle(aLocalClipRect) ) );
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
            void appendWaveline( ::basegfx::B2DPolyPolygon& o_rPoly,
                             const ::basegfx::B2DPoint& rStartPos,
                             const double               nStartOffset,
                             const double               nWidth,
                             const double               nHeight,
                             sal_Int8                   nLineStyle)
            {
                const double x(rStartPos.getX());
                const double y(rStartPos.getY() + nStartOffset + nHeight);
                double nWaveWidth = nHeight * 10.6 * 0.25;
                // Offset for the double line.
                double nOffset = 0.0;

                if (nLineStyle == LINESTYLE_DOUBLEWAVE)
                    nOffset = -nHeight * 0.5;
                else
                    nWaveWidth *= 2.0;

                basegfx::B2DPolygon aLine;
                aLine.append(basegfx::B2DPoint(x, y + nOffset));
                aLine.append(basegfx::B2DPoint(x + nWidth, y + nOffset));

                o_rPoly.append(::basegfx::utils::createWaveline(aLine, nWaveWidth, nWaveWidth * 0.5));

                if (nLineStyle == LINESTYLE_DOUBLEWAVE)
                {
                    nOffset = nHeight * 1.2;

                    basegfx::B2DPolygon aLine2;
                    aLine2.append(basegfx::B2DPoint(x, y + nOffset));
                    aLine2.append(basegfx::B2DPoint(x + nWidth, y + nOffset));
                    o_rPoly.append(::basegfx::utils::createWaveline(aLine2, nWaveWidth, nWaveWidth * 0.5));
                }
            }

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

            bool appendDashes( ::basegfx::B2DPolyPolygon&   o_rPoly,
                               const double                 nX,
                               double                       nY,
                               const double                 nLineWidth,
                               double                       nLineHeight,
                               sal_Int8                     nLineStyle,
                               bool                         bIsOverline)
            {
                static const int aDottedArray[]     = { 1, 1, 0};               // DOTTED LINE
                static const int aDotDashArray[]    = { 1, 1, 4, 1, 0};         // DASHDOT
                static const int aDashDotDotArray[] = { 1, 1, 1, 1, 4, 1, 0};   // DASHDOTDOT
                static const int aDashedArray[]     = { 5, 2, 0};               // DASHED LINE
                static const int aLongDashArray[]   = { 7, 2, 0};               // LONGDASH
                const int *pArray = nullptr;
                bool bIsBold = false;

                switch(nLineStyle)
                {
                    case LINESTYLE_BOLDDOTTED:
                        bIsBold = true;
                        [[fallthrough]];
                    case LINESTYLE_DOTTED:
                        pArray = aDottedArray;
                    break;

                    case LINESTYLE_BOLDDASH:
                        bIsBold = true;
                        [[fallthrough]];
                    case LINESTYLE_DASH:
                        pArray = aDashedArray;
                    break;

                    case LINESTYLE_BOLDLONGDASH:
                        bIsBold = true;
                        [[fallthrough]];
                    case LINESTYLE_LONGDASH:
                        pArray = aLongDashArray;
                    break;

                    case LINESTYLE_BOLDDASHDOT:
                        bIsBold = true;
                        [[fallthrough]];
                    case LINESTYLE_DASHDOT:
                        pArray = aDotDashArray;
                    break;
                    case LINESTYLE_BOLDDASHDOTDOT:
                        bIsBold = true;
                        [[fallthrough]];
                    case LINESTYLE_DASHDOTDOT:
                        pArray = aDashDotDotArray;
                    break;
                }

                if (!pArray)
                    return false;

                if (bIsBold)
                {
                    if (bIsOverline)
                        nY -= nLineHeight;

                    nLineHeight *= 2;
                }

                const double nEnd = nX + nLineWidth;
                sal_Int32 nIndex = 0;
                bool bAppend = true;
                double nX1 = nX;

                while(nX1 < nEnd)
                {
                    if (pArray[nIndex] == 0)
                        nIndex = 0;

                    const double nX2 = std::min(nEnd, nX1 + pArray[nIndex] * nLineHeight);

                    if (bAppend)
                        appendRect(o_rPoly, nX1, nY, nX2, nY + nLineHeight);

                    nX1 = nX2;

                    ++nIndex;

                    bAppend = !bAppend;
                }
                return true;
            }

            // create line actions for text such as underline and
            // strikeout
            void createOverlinePolyPolygon(::basegfx::B2DPolyPolygon& rTextLinesPolyPoly,
                                            const ::basegfx::B2DPoint& rStartPos,
                                            const double&              rLineWidth,
                                            const TextLineInfo&        rTextLineInfo)
            {
                switch( rTextLineInfo.mnOverlineStyle )
                {
                    case LINESTYLE_NONE:          // nothing to do
                    case LINESTYLE_DONTKNOW:
                        break;

                    case LINESTYLE_DOUBLEWAVE:
                    case LINESTYLE_SMALLWAVE:
                    case LINESTYLE_BOLDWAVE:
                    case LINESTYLE_WAVE:
                        appendWaveline(
                            rTextLinesPolyPoly,
                            rStartPos,
                            rTextLineInfo.mnOverlineOffset,
                            rLineWidth,
                            rTextLineInfo.mnOverlineHeight,
                            rTextLineInfo.mnOverlineStyle);

                        break;
                    case LINESTYLE_SINGLE:
                        appendRect(
                            rTextLinesPolyPoly,
                            rStartPos,
                            0,
                            rTextLineInfo.mnOverlineOffset,
                            rLineWidth,
                            rTextLineInfo.mnOverlineOffset + rTextLineInfo.mnOverlineHeight );
                        break;
                    case LINESTYLE_BOLD:
                        appendRect(
                            rTextLinesPolyPoly,
                            rStartPos,
                            0,
                            rTextLineInfo.mnOverlineOffset - rTextLineInfo.mnOverlineHeight,
                            rLineWidth,
                            rTextLineInfo.mnOverlineOffset + rTextLineInfo.mnOverlineHeight );
                        break;

                    case LINESTYLE_DOUBLE:
                        appendRect(
                            rTextLinesPolyPoly,
                            rStartPos,
                            0,
                            rTextLineInfo.mnOverlineOffset - rTextLineInfo.mnOverlineHeight * 2.0 ,
                            rLineWidth,
                            rTextLineInfo.mnOverlineOffset - rTextLineInfo.mnOverlineHeight );

                        appendRect(
                            rTextLinesPolyPoly,
                            rStartPos,
                            0,
                            rTextLineInfo.mnOverlineOffset + rTextLineInfo.mnOverlineHeight,
                            rLineWidth,
                            rTextLineInfo.mnOverlineOffset + rTextLineInfo.mnOverlineHeight * 2.0 );
                        break;

                    default:
                        if (!appendDashes(
                            rTextLinesPolyPoly,
                            rStartPos.getX(),
                            rStartPos.getY() + rTextLineInfo.mnOverlineOffset,
                            rLineWidth,
                            rTextLineInfo.mnOverlineHeight,
                            rTextLineInfo.mnOverlineStyle,
                            true))
                        {
                            ENSURE_OR_THROW( false,
                                          "::cppcanvas::internal::createTextLinesPolyPolygon(): Unexpected overline case" );
                        }
                }
            }

            void createUnderlinePolyPolygon(::basegfx::B2DPolyPolygon& rTextLinesPolyPoly,
                                            const ::basegfx::B2DPoint& rStartPos,
                                            const double&              rLineWidth,
                                            const TextLineInfo&        rTextLineInfo )
            {

                switch( rTextLineInfo.mnUnderlineStyle )
                {
                    case LINESTYLE_NONE:          // nothing to do
                    case LINESTYLE_DONTKNOW:
                        break;

                    case LINESTYLE_DOUBLEWAVE:
                    case LINESTYLE_SMALLWAVE:
                    case LINESTYLE_BOLDWAVE:
                    case LINESTYLE_WAVE:
                        appendWaveline(
                            rTextLinesPolyPoly,
                            rStartPos,
                            rTextLineInfo.mnUnderlineOffset,
                            rLineWidth,
                            rTextLineInfo.mnLineHeight,
                            rTextLineInfo.mnUnderlineStyle);
                        break;
                    case LINESTYLE_SINGLE:
                        appendRect(
                            rTextLinesPolyPoly,
                            rStartPos,
                            0,
                            rTextLineInfo.mnUnderlineOffset,
                            rLineWidth,
                            rTextLineInfo.mnUnderlineOffset + rTextLineInfo.mnLineHeight );
                        break;

                    case LINESTYLE_BOLD:
                        appendRect(
                            rTextLinesPolyPoly,
                            rStartPos,
                            0,
                            rTextLineInfo.mnUnderlineOffset,
                            rLineWidth,
                            rTextLineInfo.mnUnderlineOffset + 2*rTextLineInfo.mnLineHeight );
                        break;

                    case LINESTYLE_DOUBLE:
                        appendRect(
                            rTextLinesPolyPoly,
                            rStartPos,
                            0,
                            rTextLineInfo.mnUnderlineOffset - rTextLineInfo.mnLineHeight,
                            rLineWidth,
                            rTextLineInfo.mnUnderlineOffset );

                        appendRect(
                            rTextLinesPolyPoly,
                            rStartPos,
                            0,
                            rTextLineInfo.mnUnderlineOffset + 2*rTextLineInfo.mnLineHeight,
                            rLineWidth,
                            rTextLineInfo.mnUnderlineOffset + 3*rTextLineInfo.mnLineHeight );
                        break;

                    default:
                        if (!appendDashes(
                            rTextLinesPolyPoly,
                            rStartPos.getX(),
                            rStartPos.getY() + rTextLineInfo.mnUnderlineOffset,
                            rLineWidth,
                            rTextLineInfo.mnLineHeight,
                            rTextLineInfo.mnUnderlineStyle,
                            false))
                        {
                            ENSURE_OR_THROW( false,
                                          "::cppcanvas::internal::createTextLinesPolyPolygon(): Unexpected underline case" );
                        }
                }
            }

            void createStrikeoutPolyPolygon(::basegfx::B2DPolyPolygon& rTextLinesPolyPoly,
                                            const ::basegfx::B2DPoint& rStartPos,
                                            const double&              rLineWidth,
                                            const TextLineInfo&        rTextLineInfo)
            {
                switch( rTextLineInfo.mnStrikeoutStyle )
                {
                    case STRIKEOUT_NONE:    // nothing to do
                    case STRIKEOUT_DONTKNOW:
                        break;

                    case STRIKEOUT_SLASH:   // TODO(Q1): we should handle this in the text layer
                    case STRIKEOUT_X:
                        break;

                    case STRIKEOUT_SINGLE:
                        appendRect(
                            rTextLinesPolyPoly,
                            rStartPos,
                            0,
                            rTextLineInfo.mnStrikeoutOffset,
                            rLineWidth,
                            rTextLineInfo.mnStrikeoutOffset + rTextLineInfo.mnLineHeight );
                        break;

                    case STRIKEOUT_BOLD:
                        appendRect(
                            rTextLinesPolyPoly,
                            rStartPos,
                            0,
                            rTextLineInfo.mnStrikeoutOffset,
                            rLineWidth,
                            rTextLineInfo.mnStrikeoutOffset + 2*rTextLineInfo.mnLineHeight );
                        break;

                    case STRIKEOUT_DOUBLE:
                        appendRect(
                            rTextLinesPolyPoly,
                            rStartPos,
                            0,
                            rTextLineInfo.mnStrikeoutOffset - rTextLineInfo.mnLineHeight,
                            rLineWidth,
                            rTextLineInfo.mnStrikeoutOffset );

                        appendRect(
                            rTextLinesPolyPoly,
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
            }
        }

        ::basegfx::B2DPolyPolygon createTextLinesPolyPolygon( const ::basegfx::B2DPoint& rStartPos,
                                                              const double&              rLineWidth,
                                                              const TextLineInfo&        rTextLineInfo )
        {
            // fill the polypolygon with all text lines
            ::basegfx::B2DPolyPolygon aTextLinesPolyPoly;

            createOverlinePolyPolygon(aTextLinesPolyPoly, rStartPos, rLineWidth, rTextLineInfo);
            createUnderlinePolyPolygon(aTextLinesPolyPoly, rStartPos, rLineWidth, rTextLineInfo);
            createStrikeoutPolyPolygon(aTextLinesPolyPoly, rStartPos, rLineWidth, rTextLineInfo);
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

        void createTextLinesPolyPolygon( const double&              rStartOffset,
                                         const double&              rLineWidth,
                                         const TextLineInfo&        rTextLineInfo,
                                         ::basegfx::B2DPolyPolygon& rOverlinePolyPoly,
                                         ::basegfx::B2DPolyPolygon& rUnderlinePolyPoly,
                                         ::basegfx::B2DPolyPolygon& rStrikeoutPolyPoly )
        {
            ::basegfx::B2DPoint aStartPos(rStartOffset, 0.0);

            createOverlinePolyPolygon(rOverlinePolyPoly, aStartPos, rLineWidth, rTextLineInfo);
            createUnderlinePolyPolygon(rUnderlinePolyPoly, aStartPos, rLineWidth, rTextLineInfo);
            createStrikeoutPolyPolygon(rStrikeoutPolyPoly, aStartPos, rLineWidth, rTextLineInfo);
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
