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
#include <tools/debug.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <cppcanvas/canvas.hxx>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/rendering/PanoseProportion.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <canvas/canvastools.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/BitmapPalette.hxx>
#include <tools/poly.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <implrenderer.hxx>
#include <tools.hxx>
#include <outdevstate.hxx>
#include <action.hxx>
#include <sal/log.hxx>
#include "bitmapaction.hxx"
#include "lineaction.hxx"
#include "pointaction.hxx"
#include "polypolyaction.hxx"
#include "textaction.hxx"
#include "transparencygroupaction.hxx"
#include <vector>
#include <algorithm>
#include <memory>
#include <string_view>
#include "mtftools.hxx"

using namespace ::com::sun::star;


// free support functions
// ======================
namespace
{
    template < class MetaActionType > void setStateColor( MetaActionType*                   pAct,
                                                          bool&                             rIsColorSet,
                                                          uno::Sequence< double >&          rColorSequence,
                                                          const cppcanvas::CanvasSharedPtr& rCanvas )
    {
        rIsColorSet = pAct->IsSetting();
        if (!rIsColorSet)
            return;

        ::Color aColor( pAct->GetColor() );

        // force alpha part of color to
        // opaque. transparent painting is done
        // explicitly via MetaActionType::Transparent
        aColor.SetAlpha(255);
        //aColor.SetTransparency(128);

        rColorSequence = vcl::unotools::colorToDoubleSequence(
            aColor,
            rCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() );
    }

    void setupStrokeAttributes( rendering::StrokeAttributes&                          o_rStrokeAttributes,
                                const ::cppcanvas::internal::ActionFactoryParameters& rParms,
                                const LineInfo&                                       rLineInfo                 )
    {
        const ::basegfx::B2DSize aWidth( rLineInfo.GetWidth(), 0 );
        o_rStrokeAttributes.StrokeWidth =
            (rParms.mrStates.getState().mapModeTransform * aWidth).getLength();

        // setup reasonable defaults
        o_rStrokeAttributes.MiterLimit   = 15.0; // 1.0 was no good default; GDI+'s limit is 10.0, our's is 15.0
        o_rStrokeAttributes.StartCapType = rendering::PathCapType::BUTT;
        o_rStrokeAttributes.EndCapType   = rendering::PathCapType::BUTT;

        switch (rLineInfo.GetLineJoin())
        {
            case basegfx::B2DLineJoin::NONE:
                o_rStrokeAttributes.JoinType = rendering::PathJoinType::NONE;
                break;
            case basegfx::B2DLineJoin::Bevel:
                o_rStrokeAttributes.JoinType = rendering::PathJoinType::BEVEL;
                break;
            case basegfx::B2DLineJoin::Miter:
                o_rStrokeAttributes.JoinType = rendering::PathJoinType::MITER;
                break;
            case basegfx::B2DLineJoin::Round:
                o_rStrokeAttributes.JoinType = rendering::PathJoinType::ROUND;
                break;
        }

        switch(rLineInfo.GetLineCap())
        {
            default: /* css::drawing::LineCap_BUTT */
            {
                o_rStrokeAttributes.StartCapType = rendering::PathCapType::BUTT;
                o_rStrokeAttributes.EndCapType   = rendering::PathCapType::BUTT;
                break;
            }
            case css::drawing::LineCap_ROUND:
            {
                o_rStrokeAttributes.StartCapType = rendering::PathCapType::ROUND;
                o_rStrokeAttributes.EndCapType   = rendering::PathCapType::ROUND;
                break;
            }
            case css::drawing::LineCap_SQUARE:
            {
                o_rStrokeAttributes.StartCapType = rendering::PathCapType::SQUARE;
                o_rStrokeAttributes.EndCapType   = rendering::PathCapType::SQUARE;
                break;
            }
        }

        if( LineStyle::Dash != rLineInfo.GetStyle() )
            return;

        const ::cppcanvas::internal::OutDevState& rState( rParms.mrStates.getState() );

        // TODO(F1): Interpret OutDev::GetRefPoint() for the start of the dashing.

        // interpret dash info only if explicitly enabled as
        // style
        const ::basegfx::B2DSize aDistance( rLineInfo.GetDistance(), 0 );
        const double nDistance( (rState.mapModeTransform * aDistance).getLength() );

        const ::basegfx::B2DSize aDashLen( rLineInfo.GetDashLen(), 0 );
        const double nDashLen( (rState.mapModeTransform * aDashLen).getLength() );

        const ::basegfx::B2DSize aDotLen( rLineInfo.GetDotLen(), 0 );
        const double nDotLen( (rState.mapModeTransform * aDotLen).getLength() );

        const sal_Int32 nNumArryEntries( 2*rLineInfo.GetDashCount() +
                                         2*rLineInfo.GetDotCount() );

        o_rStrokeAttributes.DashArray.realloc( nNumArryEntries );
        double* pDashArray = o_rStrokeAttributes.DashArray.getArray();


        // iteratively fill dash array, first with dashes, then
        // with dots.


        sal_Int32 nCurrEntry=0;

        for( sal_Int32 i=0; i<rLineInfo.GetDashCount(); ++i )
        {
            pDashArray[nCurrEntry++] = nDashLen;
            pDashArray[nCurrEntry++] = nDistance;
        }
        for( sal_Int32 i=0; i<rLineInfo.GetDotCount(); ++i )
        {
            pDashArray[nCurrEntry++] = nDotLen;
            pDashArray[nCurrEntry++] = nDistance;
        }
    }


    /** Create masked BitmapEx, where the white areas of rBitmap are
        transparent, and the other appear in rMaskColor.
     */
    BitmapEx createMaskBmpEx( const Bitmap&  rBitmap,
                              const ::Color& rMaskColor )
    {
        const ::Color aWhite( COL_WHITE );
        BitmapPalette aBiLevelPalette{
            aWhite, rMaskColor
        };

        AlphaMask aMask( rBitmap.CreateAlphaMask( aWhite ));
        Bitmap aSolid( rBitmap.GetSizePixel(),
                       vcl::PixelFormat::N8_BPP,
                       &aBiLevelPalette );
        aSolid.Erase( rMaskColor );

        return BitmapEx( aSolid, aMask );
    }

    OUString convertToLocalizedNumerals(std::u16string_view rStr,
        LanguageType eTextLanguage)
    {
        OUStringBuffer aBuf(rStr);
        for (sal_Int32 i = 0; i < aBuf.getLength(); ++i)
        {
            sal_Unicode nChar = aBuf[i];
            if (nChar >= '0' && nChar <= '9')
                aBuf[i] = GetLocalizedChar(nChar, eTextLanguage);
        }
        return aBuf.makeStringAndClear();
    }
}

namespace cppcanvas::internal
{
        // state stack manipulators

        void VectorOfOutDevStates::clearStateStack()
        {
            m_aStates.clear();
            m_aStates.push_back(OutDevState());
        }

        OutDevState& VectorOfOutDevStates::getState()
        {
            return m_aStates.back();
        }

        const OutDevState& VectorOfOutDevStates::getState() const
        {
            return m_aStates.back();
        }

        void VectorOfOutDevStates::pushState(vcl::PushFlags nFlags)
        {
            m_aStates.push_back( getState() );
            getState().pushFlags = nFlags;
        }

        void VectorOfOutDevStates::popState()
        {
            if( getState().pushFlags != vcl::PushFlags::ALL )
            {
                // a state is pushed which is incomplete, i.e. does not
                // restore everything to the previous stack level when
                // popped.
                // That means, we take the old state, and restore every
                // OutDevState member whose flag is set, from the new to the
                // old state. Then the new state gets overwritten by the
                // calculated state

                // preset to-be-calculated new state with old state
                OutDevState aCalculatedNewState( getState() );

                // selectively copy to-be-restored content over saved old
                // state
                m_aStates.pop_back();

                const OutDevState& rNewState( getState() );

                if( aCalculatedNewState.pushFlags & vcl::PushFlags::LINECOLOR )
                {
                    aCalculatedNewState.lineColor      = rNewState.lineColor;
                    aCalculatedNewState.isLineColorSet = rNewState.isLineColorSet;
                }

                if( aCalculatedNewState.pushFlags & vcl::PushFlags::FILLCOLOR )
                {
                    aCalculatedNewState.fillColor      = rNewState.fillColor;
                    aCalculatedNewState.isFillColorSet = rNewState.isFillColorSet;
                }

                if( aCalculatedNewState.pushFlags & vcl::PushFlags::FONT )
                {
                    aCalculatedNewState.xFont                   = rNewState.xFont;
                    aCalculatedNewState.fontRotation            = rNewState.fontRotation;
                    aCalculatedNewState.textReliefStyle         = rNewState.textReliefStyle;
                    aCalculatedNewState.textOverlineStyle       = rNewState.textOverlineStyle;
                    aCalculatedNewState.textUnderlineStyle      = rNewState.textUnderlineStyle;
                    aCalculatedNewState.textStrikeoutStyle      = rNewState.textStrikeoutStyle;
                    aCalculatedNewState.textEmphasisMark        = rNewState.textEmphasisMark;
                    aCalculatedNewState.isTextEffectShadowSet   = rNewState.isTextEffectShadowSet;
                    aCalculatedNewState.isTextWordUnderlineSet  = rNewState.isTextWordUnderlineSet;
                    aCalculatedNewState.isTextOutlineModeSet    = rNewState.isTextOutlineModeSet;
                }

                if( aCalculatedNewState.pushFlags & vcl::PushFlags::TEXTCOLOR )
                {
                    aCalculatedNewState.textColor = rNewState.textColor;
                }

                if( aCalculatedNewState.pushFlags & vcl::PushFlags::MAPMODE )
                {
                    aCalculatedNewState.mapModeTransform = rNewState.mapModeTransform;
                }

                if( aCalculatedNewState.pushFlags & vcl::PushFlags::CLIPREGION )
                {
                    aCalculatedNewState.clip        = rNewState.clip;
                    aCalculatedNewState.clipRect    = rNewState.clipRect;
                    aCalculatedNewState.xClipPoly   = rNewState.xClipPoly;
                }

                // TODO(F2): Raster ops NYI
                // if( (aCalculatedNewState.pushFlags & vcl::PushFlags::RASTEROP) )
                // {
                // }

                if( aCalculatedNewState.pushFlags & vcl::PushFlags::TEXTFILLCOLOR )
                {
                    aCalculatedNewState.textFillColor      = rNewState.textFillColor;
                    aCalculatedNewState.isTextFillColorSet = rNewState.isTextFillColorSet;
                }

                if( aCalculatedNewState.pushFlags & vcl::PushFlags::TEXTALIGN )
                {
                    aCalculatedNewState.textReferencePoint = rNewState.textReferencePoint;
                }

                // TODO(F1): Refpoint handling NYI
                // if( (aCalculatedNewState.pushFlags & vcl::PushFlags::REFPOINT) )
                // {
                // }

                if( aCalculatedNewState.pushFlags & vcl::PushFlags::TEXTLINECOLOR )
                {
                    aCalculatedNewState.textLineColor      = rNewState.textLineColor;
                    aCalculatedNewState.isTextLineColorSet = rNewState.isTextLineColorSet;
                }

                if( aCalculatedNewState.pushFlags & vcl::PushFlags::OVERLINECOLOR )
                {
                    aCalculatedNewState.textOverlineColor = rNewState.textOverlineColor;
                    aCalculatedNewState.isTextOverlineColorSet = rNewState.isTextOverlineColorSet;
                }

                if( aCalculatedNewState.pushFlags & vcl::PushFlags::TEXTLAYOUTMODE )
                {
                    aCalculatedNewState.textAlignment = rNewState.textAlignment;
                    aCalculatedNewState.textDirection = rNewState.textDirection;
                }

                // TODO(F2): Text language handling NYI
                // if( (aCalculatedNewState.pushFlags & vcl::PushFlags::TEXTLANGUAGE) )
                // {
                // }

                // always copy push mode
                aCalculatedNewState.pushFlags = rNewState.pushFlags;

                // flush to stack
                getState() = std::move(aCalculatedNewState);
            }
            else
            {
                m_aStates.pop_back();
            }
        }

        bool ImplRenderer::createFillAndStroke( const ::basegfx::B2DPolyPolygon& rPolyPoly,
                                                const ActionFactoryParameters&   rParms )
        {
            const OutDevState& rState( rParms.mrStates.getState() );
            if( (!rState.isLineColorSet &&
                 !rState.isFillColorSet) ||
                (!rState.lineColor.hasElements() &&
                 !rState.fillColor.hasElements()) )
            {
                return false;
            }

            std::shared_ptr<Action> pPolyAction(
                internal::PolyPolyActionFactory::createPolyPolyAction(
                    rPolyPoly, rParms.mrCanvas, rState ) );

            if( pPolyAction )
            {
                maActions.emplace_back(
                        pPolyAction,
                        rParms.mrCurrActionIndex );

                rParms.mrCurrActionIndex += pPolyAction->getActionCount()-1;
            }

            return true;
        }

        bool ImplRenderer::createFillAndStroke( const ::basegfx::B2DPolygon&   rPoly,
                                                const ActionFactoryParameters& rParms )
        {
            return createFillAndStroke( ::basegfx::B2DPolyPolygon( rPoly ),
                                        rParms );
        }

        void ImplRenderer::skipContent( GDIMetaFile& rMtf,
                                        const char*  pCommentString,
                                        sal_Int32&   io_rCurrActionIndex )
        {
            ENSURE_OR_THROW( pCommentString,
                              "ImplRenderer::skipContent(): NULL string given" );

            MetaAction* pCurrAct;
            while( (pCurrAct=rMtf.NextAction()) != nullptr )
            {
                // increment action index, we've skipped an action.
                ++io_rCurrActionIndex;

                if( pCurrAct->GetType() == MetaActionType::COMMENT &&
                    static_cast<MetaCommentAction*>(pCurrAct)->GetComment().equalsIgnoreAsciiCase(
                        pCommentString) )
                {
                    // requested comment found, done
                    return;
                }
            }

            // EOF
        }

        bool ImplRenderer::isActionContained( GDIMetaFile&   rMtf,
                                              const char*    pCommentString,
                                              MetaActionType nType )
        {
            ENSURE_OR_THROW( pCommentString,
                              "ImplRenderer::isActionContained(): NULL string given" );

            bool bRet( false );

            // at least _one_ call to GDIMetaFile::NextAction() is
            // executed
            size_t nPos( 1 );

            MetaAction* pCurrAct;
            while( (pCurrAct=rMtf.NextAction()) != nullptr )
            {
                if( pCurrAct->GetType() == nType )
                {
                    bRet = true; // action type found
                    break;
                }

                if( pCurrAct->GetType() == MetaActionType::COMMENT &&
                    static_cast<MetaCommentAction*>(pCurrAct)->GetComment().equalsIgnoreAsciiCase(
                        pCommentString) )
                {
                    // delimiting end comment found, done
                    bRet = false; // not yet found
                    break;
                }

                ++nPos;
            }

            // rewind metafile to previous position (this method must
            // not change the current metaaction)
            while (nPos)
            {
                --nPos;
                rMtf.WindPrev();
            }

            if( !pCurrAct )
            {
                // EOF, and not yet found
                bRet = false;
            }

            return bRet;
        }

        void ImplRenderer::createGradientAction( const ::tools::PolyPolygon&    rPoly,
                                                 const ::Gradient&              rGradient,
                                                 const ActionFactoryParameters& rParms,
                                                 bool                           bIsPolygonRectangle,
                                                 bool                           bSubsettableActions )
        {
            DBG_TESTSOLARMUTEX();

            ::basegfx::B2DPolyPolygon aDevicePoly( rPoly.getB2DPolyPolygon() );
            aDevicePoly.transform( rParms.mrStates.getState().mapModeTransform );

            // decide, whether this gradient can be rendered natively
            // by the canvas, or must be emulated via VCL gradient
            // action extraction.
            const sal_uInt16 nSteps( rGradient.GetSteps() );

            if( // step count is infinite, can use native canvas
                // gradients here
                nSteps == 0 ||
                // step count is sufficiently high, such that no
                // discernible difference should be visible.
                nSteps > 64 )
            {
                uno::Reference< lang::XMultiServiceFactory> xFactory(
                    rParms.mrCanvas->getUNOCanvas()->getDevice()->getParametricPolyPolygonFactory() );

                if( xFactory.is() )
                {
                    rendering::Texture aTexture;

                    aTexture.RepeatModeX = rendering::TexturingMode::CLAMP;
                    aTexture.RepeatModeY = rendering::TexturingMode::CLAMP;
                    aTexture.Alpha = 1.0;


                    // setup start/end color values


                    // scale color coefficients with gradient intensities
                    const sal_uInt16 nStartIntensity( rGradient.GetStartIntensity() );
                    ::Color aVCLStartColor( rGradient.GetStartColor() );
                    aVCLStartColor.SetRed( static_cast<sal_uInt8>(aVCLStartColor.GetRed() * nStartIntensity / 100) );
                    aVCLStartColor.SetGreen( static_cast<sal_uInt8>(aVCLStartColor.GetGreen() * nStartIntensity / 100) );
                    aVCLStartColor.SetBlue( static_cast<sal_uInt8>(aVCLStartColor.GetBlue() * nStartIntensity / 100) );

                    const sal_uInt16 nEndIntensity( rGradient.GetEndIntensity() );
                    ::Color aVCLEndColor( rGradient.GetEndColor() );
                    aVCLEndColor.SetRed( static_cast<sal_uInt8>(aVCLEndColor.GetRed() * nEndIntensity / 100) );
                    aVCLEndColor.SetGreen( static_cast<sal_uInt8>(aVCLEndColor.GetGreen() * nEndIntensity / 100) );
                    aVCLEndColor.SetBlue( static_cast<sal_uInt8>(aVCLEndColor.GetBlue() * nEndIntensity / 100) );

                    uno::Reference<rendering::XColorSpace> xColorSpace(
                        rParms.mrCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace());
                    const uno::Sequence< double > aStartColor(
                        vcl::unotools::colorToDoubleSequence( aVCLStartColor,
                                                                xColorSpace ));
                    const uno::Sequence< double > aEndColor(
                        vcl::unotools::colorToDoubleSequence( aVCLEndColor,
                                                                xColorSpace ));

                    uno::Sequence< uno::Sequence < double > > aColors;
                    uno::Sequence< double > aStops;

                    if( rGradient.GetStyle() == css::awt::GradientStyle_AXIAL )
                    {
                        aStops = { 0.0, 0.5, 1.0 };
                        aColors = { aEndColor, aStartColor, aEndColor };
                    }
                    else
                    {
                        aStops = { 0.0, 1.0 };
                        aColors = { aStartColor, aEndColor };
                    }

                    const ::basegfx::B2DRectangle aBounds(
                        ::basegfx::utils::getRange(aDevicePoly) );
                    const ::basegfx::B2DVector aOffset(
                        rGradient.GetOfsX() / 100.0,
                        rGradient.GetOfsY() / 100.0);
                    double fRotation = toRadians( rGradient.GetAngle() );
                    const double fBorder( rGradient.GetBorder() / 100.0 );

                    basegfx::B2DHomMatrix aRot90;
                    aRot90.rotate(M_PI_2);

                    basegfx::ODFGradientInfo aGradInfo;
                    OUString aGradientService;
                    switch( rGradient.GetStyle() )
                    {
                        case css::awt::GradientStyle_LINEAR:
                            aGradInfo = basegfx::utils::createLinearODFGradientInfo(
                                                                        aBounds,
                                                                        nSteps,
                                                                        fBorder,
                                                                        fRotation);
                            // map ODF to svg gradient orientation - x
                            // instead of y direction
                            aGradInfo.setTextureTransform(aGradInfo.getTextureTransform() * aRot90);
                            aGradientService = "LinearGradient";
                            break;

                        case css::awt::GradientStyle_AXIAL:
                        {
                            // Adapt the border so that it is suitable
                            // for the axial gradient.  An axial
                            // gradient consists of two linear
                            // gradients.  Each of those covers half
                            // of the total size.  In order to
                            // compensate for the condensed display of
                            // the linear gradients, we have to
                            // enlarge the area taken up by the actual
                            // gradient (1-fBorder).  After that we
                            // have to turn the result back into a
                            // border value, hence the second (left
                            // most 1-...
                            const double fAxialBorder (1-2*(1-fBorder));
                            aGradInfo = basegfx::utils::createAxialODFGradientInfo(
                                                                        aBounds,
                                                                        nSteps,
                                                                        fAxialBorder,
                                                                        fRotation);
                            // map ODF to svg gradient orientation - x
                            // instead of y direction
                            aGradInfo.setTextureTransform(aGradInfo.getTextureTransform() * aRot90);

                            // map ODF axial gradient to 3-stop linear
                            // gradient - shift left by 0.5
                            basegfx::B2DHomMatrix aShift;

                            aShift.translate(-0.5,0);
                            aGradInfo.setTextureTransform(aGradInfo.getTextureTransform() * aShift);
                            aGradientService = "LinearGradient";
                            break;
                        }

                        case css::awt::GradientStyle_RADIAL:
                            aGradInfo = basegfx::utils::createRadialODFGradientInfo(
                                                                        aBounds,
                                                                        aOffset,
                                                                        nSteps,
                                                                        fBorder);
                            aGradientService = "EllipticalGradient";
                            break;

                        case css::awt::GradientStyle_ELLIPTICAL:
                            aGradInfo = basegfx::utils::createEllipticalODFGradientInfo(
                                                                            aBounds,
                                                                            aOffset,
                                                                            nSteps,
                                                                            fBorder,
                                                                            fRotation);
                            aGradientService = "EllipticalGradient";
                            break;

                        case css::awt::GradientStyle_SQUARE:
                            aGradInfo = basegfx::utils::createSquareODFGradientInfo(
                                                                        aBounds,
                                                                        aOffset,
                                                                        nSteps,
                                                                        fBorder,
                                                                        fRotation);
                            aGradientService = "RectangularGradient";
                            break;

                        case css::awt::GradientStyle_RECT:
                            aGradInfo = basegfx::utils::createRectangularODFGradientInfo(
                                                                             aBounds,
                                                                             aOffset,
                                                                             nSteps,
                                                                             fBorder,
                                                                             fRotation);
                            aGradientService = "RectangularGradient";
                            break;

                        default:
                            ENSURE_OR_THROW( false,
                                             "ImplRenderer::createGradientAction(): Unexpected gradient type" );
                            break;
                    }

                    ::basegfx::unotools::affineMatrixFromHomMatrix( aTexture.AffineTransform,
                                                                    aGradInfo.getTextureTransform() );

                    uno::Sequence<uno::Any> args(comphelper::InitAnyPropertySequence(
                    {
                        {"Colors", uno::Any(aColors)},
                        {"Stops", uno::Any(aStops)},
                        {"AspectRatio", uno::Any(aGradInfo.getAspectRatio())},
                    }));
                    aTexture.Gradient.set(
                        xFactory->createInstanceWithArguments(aGradientService,
                                                              args),
                        uno::UNO_QUERY);
                    if( aTexture.Gradient.is() )
                    {
                        std::shared_ptr<Action> pPolyAction(
                            internal::PolyPolyActionFactory::createPolyPolyAction(
                                aDevicePoly,
                                rParms.mrCanvas,
                                rParms.mrStates.getState(),
                                aTexture ) );

                        if( pPolyAction )
                        {
                            maActions.emplace_back(
                                    pPolyAction,
                                    rParms.mrCurrActionIndex );

                            rParms.mrCurrActionIndex += pPolyAction->getActionCount()-1;
                        }

                        // done, using native gradients
                        return;
                    }
                }
            }

            // cannot currently use native canvas gradients, as a
            // finite step size is given (this funny feature is not
            // supported by the XCanvas API)
            rParms.mrStates.pushState(vcl::PushFlags::ALL);

            if( !bIsPolygonRectangle )
            {
                // only clip, if given polygon is not a rectangle in
                // the first place (the gradient is always limited to
                // the given bound rect)
                updateClipping(
                    aDevicePoly,
                    rParms,
                    true );
            }

            GDIMetaFile aTmpMtf;
            Gradient aGradient(rGradient);
            aGradient.AddGradientActions( rPoly.GetBoundRect(), aTmpMtf );

            createActions( aTmpMtf, rParms, bSubsettableActions );

            rParms.mrStates.popState();
        }

        uno::Reference< rendering::XCanvasFont > ImplRenderer::createFont( double&                        o_rFontRotation,
                                                                           const vcl::Font&               rFont,
                                                                           const ActionFactoryParameters& rParms )
        {
            rendering::FontRequest aFontRequest;

            if( rParms.mrParms.maFontName )
                aFontRequest.FontDescription.FamilyName = *rParms.mrParms.maFontName;
            else
                aFontRequest.FontDescription.FamilyName = rFont.GetFamilyName();

            aFontRequest.FontDescription.StyleName = rFont.GetStyleName();

            aFontRequest.FontDescription.IsSymbolFont = (rFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL) ? util::TriState_YES : util::TriState_NO;
            aFontRequest.FontDescription.IsVertical = rFont.IsVertical() ? util::TriState_YES : util::TriState_NO;

            // TODO(F2): improve vclenum->panose conversion
            aFontRequest.FontDescription.FontDescription.Weight =
                rParms.mrParms.maFontWeight ?
                *rParms.mrParms.maFontWeight :
                ::canvas::tools::numeric_cast<sal_Int8>( ::basegfx::fround( rFont.GetWeight() ) );
            aFontRequest.FontDescription.FontDescription.Letterform =
                rParms.mrParms.maFontLetterForm ?
                *rParms.mrParms.maFontLetterForm :
                (rFont.GetItalic() == ITALIC_NONE) ? 0 : 9;
            aFontRequest.FontDescription.FontDescription.Proportion =
                (rFont.GetPitch() == PITCH_FIXED)
                    ? rendering::PanoseProportion::MONO_SPACED
                    : rendering::PanoseProportion::ANYTHING;

            LanguageType aLang = rFont.GetLanguage();
            aFontRequest.Locale = LanguageTag::convertToLocale( aLang, false);

            // setup state-local text transformation,
            // if the font be rotated
            const auto nFontAngle( rFont.GetOrientation() );
            if( nFontAngle )
            {
                // set to unity transform rotated by font angle
                const double nAngle( toRadians(nFontAngle) );
                o_rFontRotation = -nAngle;
            }
            else
            {
                o_rFontRotation = 0.0;
            }

            geometry::Matrix2D aFontMatrix;
            ::canvas::tools::setIdentityMatrix2D( aFontMatrix );

            // TODO(F2): use correct scale direction, font
            // height might be width or anything else

            // TODO(Q3): This code smells of programming by
            // coincidence (the next two if statements)

            ::Size rFontSizeLog( rFont.GetFontSize() );

            if (rFontSizeLog.Height() == 0)
            {
                // guess 16 pixel (as in VCL)
                rFontSizeLog = ::Size(0, 16);

                // convert to target MapUnit if not pixels
                rFontSizeLog = OutputDevice::LogicToLogic(rFontSizeLog, MapMode(MapUnit::MapPixel), rParms.mrVDev.GetMapMode());
            }

            const sal_Int32 nFontWidthLog = rFontSizeLog.Width();
            if( nFontWidthLog != 0 )
            {
                vcl::Font aTestFont = rFont;
                aTestFont.SetAverageFontWidth( 0 );
                sal_Int32 nNormalWidth = rParms.mrVDev.GetFontMetric( aTestFont ).GetAverageFontWidth();
                if( nNormalWidth != nFontWidthLog )
                    if( nNormalWidth )
                        aFontMatrix.m00 = static_cast<double>(nFontWidthLog) / nNormalWidth;
            }

            // #i52608# apply map mode scale also to font matrix - an
            // anisotrophic mapmode must be reflected in an
            // anisotrophic font matrix scale.
            const OutDevState& rState( rParms.mrStates.getState() );
            if( !::basegfx::fTools::equal(
                    rState.mapModeTransform.get(0,0),
                    rState.mapModeTransform.get(1,1)) )
            {
                const double nScaleX( rState.mapModeTransform.get(0,0) );
                const double nScaleY( rState.mapModeTransform.get(1,1) );

                // note: no reason to check for division by zero, we
                // always have the value closer (or equal) to zero as
                // the nominator.
                if( fabs(nScaleX) < fabs(nScaleY) )
                    aFontMatrix.m00 *= nScaleX / nScaleY;
                else
                    aFontMatrix.m11 *= nScaleY / nScaleX;
            }
            aFontRequest.CellSize = (rState.mapModeTransform * vcl::unotools::b2DSizeFromSize(rFontSizeLog)).getHeight();

            if (rFont.GetEmphasisMark() != FontEmphasisMark::NONE)
            {
                uno::Sequence< beans::PropertyValue > aProperties{ comphelper::makePropertyValue(
                    u"EmphasisMark"_ustr, sal_uInt32(rFont.GetEmphasisMark())) };
                return rParms.mrCanvas->getUNOCanvas()->createFont(aFontRequest,
                                                                aProperties,
                                                                aFontMatrix);
            }

            return rParms.mrCanvas->getUNOCanvas()->createFont( aFontRequest,
                                                                uno::Sequence< beans::PropertyValue >(),
                                                                aFontMatrix );
        }

        // create text effects such as shadow/relief/embossed
        void ImplRenderer::createTextAction( const ::Point&                 rStartPoint,
                                             const OUString&                rString,
                                             int                            nIndex,
                                             int                            nLength,
                                             KernArraySpan                pCharWidths,
                                             std::span<const sal_Bool>     pKashidaArray,
                                             const ActionFactoryParameters& rParms,
                                             bool                           bSubsettableActions )
        {
            ENSURE_OR_THROW( nIndex >= 0 && nLength <= rString.getLength() + nIndex,
                              "ImplRenderer::createTextWithEffectsAction(): Invalid text index" );

            if( !nLength )
                return; // zero-length text, no visible output

            const OutDevState& rState( rParms.mrStates.getState() );

            // TODO(F2): implement all text effects
            // if( rState.textAlignment );             // TODO(F2): NYI

            ::Color aTextFillColor( COL_AUTO );
            ::Color aShadowColor( COL_AUTO );
            ::Color aReliefColor( COL_AUTO );
            ::Size  aShadowOffset;
            ::Size  aReliefOffset;

            uno::Reference<rendering::XColorSpace> xColorSpace(
                rParms.mrCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() );

            if( rState.isTextEffectShadowSet )
            {
                // calculate shadow offset (similar to outdev3.cxx)
                // TODO(F3): better match with outdev3.cxx
                sal_Int32 nShadowOffset = static_cast<sal_Int32>(1.5 + ((rParms.mrVDev.GetFont().GetFontHeight()-24.0)/24.0));
                if( nShadowOffset < 1 )
                    nShadowOffset = 1;

                aShadowOffset.setWidth( nShadowOffset );
                aShadowOffset.setHeight( nShadowOffset );

                // determine shadow color (from outdev3.cxx)
                ::Color aTextColor = vcl::unotools::doubleSequenceToColor(
                    rState.textColor, xColorSpace );
                bool bIsDark = (aTextColor == COL_BLACK)
                    || (aTextColor.GetLuminance() < 8);

                aShadowColor = bIsDark ? COL_LIGHTGRAY : COL_BLACK;
                aShadowColor.SetAlpha( aTextColor.GetAlpha() );
            }

            if( rState.textReliefStyle != FontRelief::NONE )
            {
                // calculate relief offset (similar to outdev3.cxx)
                sal_Int32 nReliefOffset = rParms.mrVDev.PixelToLogic( Size( 1, 1 ) ).Height();
                nReliefOffset += nReliefOffset/2;
                if( nReliefOffset < 1 )
                    nReliefOffset = 1;

                if( rState.textReliefStyle == FontRelief::Engraved )
                    nReliefOffset = -nReliefOffset;

                aReliefOffset.setWidth( nReliefOffset );
                aReliefOffset.setHeight( nReliefOffset );

                // determine relief color (from outdev3.cxx)
                ::Color aTextColor = vcl::unotools::doubleSequenceToColor(
                    rState.textColor, xColorSpace );

                aReliefColor = COL_LIGHTGRAY;

                // we don't have an automatic color, so black is always
                // drawn on white (literally copied from
                // vcl/source/gdi/outdev3.cxx)
                if( aTextColor == COL_BLACK )
                {
                    aTextColor = COL_WHITE;
                    rParms.mrStates.getState().textColor =
                        vcl::unotools::colorToDoubleSequence(
                            aTextColor, xColorSpace );
                }

                if( aTextColor == COL_WHITE )
                    aReliefColor = COL_BLACK;
                aReliefColor.SetAlpha( aTextColor.GetAlpha() );
            }

            if (rState.isTextFillColorSet)
                aTextFillColor = vcl::unotools::doubleSequenceToColor(rState.textFillColor, xColorSpace);

            // create the actual text action
            std::shared_ptr<Action> pTextAction(
                TextActionFactory::createTextAction(
                    rStartPoint,
                    aReliefOffset,
                    aReliefColor,
                    aShadowOffset,
                    aShadowColor,
                    aTextFillColor,
                    rString,
                    nIndex,
                    nLength,
                    pCharWidths,
                    pKashidaArray,
                    rParms.mrVDev,
                    rParms.mrCanvas,
                    rState,
                    rParms.mrParms,
                    bSubsettableActions ) );

            std::shared_ptr<Action> pStrikeoutTextAction;

            if ( rState.textStrikeoutStyle == STRIKEOUT_X || rState.textStrikeoutStyle == STRIKEOUT_SLASH )
            {
                ::tools::Long nWidth = rParms.mrVDev.GetTextWidth( rString,nIndex,nLength );

                sal_Unicode pChars[4];
                if ( rState.textStrikeoutStyle == STRIKEOUT_X )
                    pChars[0] = 'X';
                else
                    pChars[0] = '/';
                pChars[3]=pChars[2]=pChars[1]=pChars[0];

                ::tools::Long nStrikeoutWidth = (rParms.mrVDev.GetTextWidth(
                    OUString(pChars, std::size(pChars))) + 2) / 4;

                if( nStrikeoutWidth <= 0 )
                    nStrikeoutWidth = 1;

                ::tools::Long nMaxWidth = nStrikeoutWidth/2;
                if ( nMaxWidth < 2 )
                    nMaxWidth = 2;
                nMaxWidth += nWidth + 1;

                ::tools::Long nFullStrikeoutWidth = 0;
                OUStringBuffer aStrikeoutText;
                while( (nFullStrikeoutWidth+=nStrikeoutWidth ) < nMaxWidth+1 )
                    aStrikeoutText.append(pChars[0]);

                sal_Int32 nLen = aStrikeoutText.getLength();

                if( nLen )
                {
                    ::tools::Long nInterval = ( nWidth - nStrikeoutWidth * nLen ) / nLen;
                    nStrikeoutWidth += nInterval;
                    KernArray aStrikeoutCharWidths;

                    for ( int i = 0;i< nLen; i++ )
                        aStrikeoutCharWidths.push_back(nStrikeoutWidth * (i + 1));

                    pStrikeoutTextAction =
                        TextActionFactory::createTextAction(
                            rStartPoint,
                            aReliefOffset,
                            aReliefColor,
                            aShadowOffset,
                            aShadowColor,
                            aTextFillColor,
                            aStrikeoutText.makeStringAndClear(),
                            0/*nStartPos*/,
                            nLen,
                            aStrikeoutCharWidths,
                            pKashidaArray,
                            rParms.mrVDev,
                            rParms.mrCanvas,
                            rState,
                            rParms.mrParms,
                            bSubsettableActions ) ;
                }
            }

            if( !pTextAction )
                return;

            maActions.emplace_back(
                    pTextAction,
                    rParms.mrCurrActionIndex );

            if ( pStrikeoutTextAction )
            {
                maActions.emplace_back(
                    pStrikeoutTextAction,
                    rParms.mrCurrActionIndex );
            }

            rParms.mrCurrActionIndex += pTextAction->getActionCount()-1;
        }

        void ImplRenderer::updateClipping( const ::basegfx::B2DPolyPolygon& rClipPoly,
                                           const ActionFactoryParameters&   rParms,
                                           bool                             bIntersect )
        {
            ::cppcanvas::internal::OutDevState& rState( rParms.mrStates.getState() );

            const bool bEmptyClipRect( rState.clipRect.IsEmpty() );
            const bool bEmptyClipPoly( rState.clip.count() == 0 );

            ENSURE_OR_THROW( bEmptyClipPoly || bEmptyClipRect,
                              "ImplRenderer::updateClipping(): Clip rect and polygon are both set!" );

            if( !bIntersect ||
                (bEmptyClipRect && bEmptyClipPoly) )
            {
                rState.clip = rClipPoly;
            }
            else
            {
                if( !bEmptyClipRect )
                {
                    // TODO(P3): Use Liang-Barsky polygon clip here,
                    // after all, one object is just a rectangle!

                    // convert rect to polygon beforehand, must revert
                    // to general polygon clipping here.
                    ::tools::Rectangle aRect = rState.clipRect;
                    // #121100# VCL rectangular clips always
                    // include one more pixel to the right
                    // and the bottom
                    aRect.AdjustRight(1);
                    aRect.AdjustBottom(1);
                    rState.clip = ::basegfx::B2DPolyPolygon(
                        ::basegfx::utils::createPolygonFromRect(
                            vcl::unotools::b2DRectangleFromRectangle(aRect) ) );
                }

                // AW: Simplified
                rState.clip = basegfx::utils::clipPolyPolygonOnPolyPolygon(
                    rClipPoly, rState.clip, true, false);
            }

            // by now, our clip resides in the OutDevState::clip
            // poly-polygon.
            rState.clipRect.SetEmpty();

            if( rState.clip.count() == 0 )
            {
                if( rState.clipRect.IsEmpty() )
                {
                    rState.xClipPoly.clear();
                }
                else
                {
                    ::tools::Rectangle aRect = rState.clipRect;
                    // #121100# VCL rectangular clips
                    // always include one more pixel to
                    // the right and the bottom
                    aRect.AdjustRight(1);
                    aRect.AdjustBottom(1);
                    rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rParms.mrCanvas->getUNOCanvas()->getDevice(),
                        ::basegfx::B2DPolyPolygon(
                            ::basegfx::utils::createPolygonFromRect(
                                vcl::unotools::b2DRectangleFromRectangle(aRect) ) ) );
                }
            }
            else
            {
                rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    rParms.mrCanvas->getUNOCanvas()->getDevice(),
                    rState.clip );
            }
        }

        void ImplRenderer::updateClipping( const ::tools::Rectangle&             rClipRect,
                                           const ActionFactoryParameters& rParms,
                                           bool                           bIntersect )
        {
            ::cppcanvas::internal::OutDevState& rState( rParms.mrStates.getState() );

            const bool bEmptyClipRect( rState.clipRect.IsEmpty() );
            const bool bEmptyClipPoly( rState.clip.count() == 0 );

            ENSURE_OR_THROW( bEmptyClipPoly || bEmptyClipRect,
                              "ImplRenderer::updateClipping(): Clip rect and polygon are both set!" );

            if( !bIntersect ||
                (bEmptyClipRect && bEmptyClipPoly) )
            {
                rState.clipRect = rClipRect;
                rState.clip.clear();
            }
            else if( bEmptyClipPoly )
            {
                rState.clipRect.Intersection( rClipRect );
                rState.clip.clear();
            }
            else
            {
                // TODO(P3): Handle a fourth case here, when all clip
                // polygons are rectangular, once B2DMultiRange's
                // sweep line implementation is done.

                // general case: convert to polygon and clip


                // convert rect to polygon beforehand, must revert
                // to general polygon clipping here.
                ::basegfx::B2DPolyPolygon aClipPoly(
                    ::basegfx::utils::createPolygonFromRect(
                        vcl::unotools::b2DRectangleFromRectangle(rClipRect) ) );

                rState.clipRect.SetEmpty();

                // AW: Simplified
                rState.clip = basegfx::utils::clipPolyPolygonOnPolyPolygon(
                    aClipPoly, rState.clip, true, false);
            }

            if( rState.clip.count() == 0 )
            {
                if( rState.clipRect.IsEmpty() )
                {
                    rState.xClipPoly.clear();
                }
                else
                {
                    // #121100# VCL rectangular clips
                    // always include one more pixel to
                    // the right and the bottom
                    ::tools::Rectangle aRect = rState.clipRect;
                    aRect.AdjustRight(1);
                    aRect.AdjustBottom(1);
                    rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rParms.mrCanvas->getUNOCanvas()->getDevice(),
                        ::basegfx::B2DPolyPolygon(
                            ::basegfx::utils::createPolygonFromRect(
                                vcl::unotools::b2DRectangleFromRectangle(aRect) ) ) );
                }
            }
            else
            {
                rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    rParms.mrCanvas->getUNOCanvas()->getDevice(),
                    rState.clip );
            }
        }

        void ImplRenderer::createActions( GDIMetaFile&                   rMtf,
                                          const ActionFactoryParameters& rFactoryParms,
                                          bool                           bSubsettableActions )
        {
            /* TODO(P2): interpret mtf-comments
               ================================

               - gradient fillings (do that via comments)

               - think about mapping. _If_ we do everything in logical
                    coordinates (which would solve the probs for stroke
                 widths and text offsets), then we would have to
                 recalc scaling for every drawing operation. This is
                 because the outdev map mode might change at any time.
                 Also keep in mind, that, although we've double precision
                 float arithmetic now, different offsets might still
                 generate different roundings (aka
                 'OutputDevice::SetPixelOffset())

             */

            // alias common parameters
            VectorOfOutDevStates&  rStates(rFactoryParms.mrStates);
            const CanvasSharedPtr& rCanvas(rFactoryParms.mrCanvas);
            ::VirtualDevice&       rVDev(rFactoryParms.mrVDev);
            const Parameters&      rParms(rFactoryParms.mrParms);
            sal_Int32&             io_rCurrActionIndex(rFactoryParms.mrCurrActionIndex);


            // Loop over every metaaction
            // ==========================
            MetaAction* pCurrAct;

            // TODO(P1): think about caching
            for( pCurrAct=rMtf.FirstAction();
                 pCurrAct;
                 pCurrAct = rMtf.NextAction() )
            {
                // execute every action, to keep VDev state up-to-date
                // currently used only for
                // - the map mode
                // - the line/fill color when processing a MetaActionType::Transparent
                // - SetFont to process font metric specific actions
                pCurrAct->Execute( &rVDev );

                SAL_INFO("cppcanvas.emf", "MTF\trecord type: 0x" << static_cast<sal_uInt16>(pCurrAct->GetType()) << " (" << static_cast<sal_uInt16>(pCurrAct->GetType()) << ")");

                switch( pCurrAct->GetType() )
                {


                    // In the first part of this monster-switch, we
                    // handle all state-changing meta actions. These
                    // are all handled locally.


                    case MetaActionType::PUSH:
                    {
                        MetaPushAction* pPushAction = static_cast<MetaPushAction*>(pCurrAct);
                        rStates.pushState(pPushAction->GetFlags());
                    }
                    break;

                    case MetaActionType::POP:
                        rStates.popState();
                        break;

                    case MetaActionType::TEXTLANGUAGE:
                    case MetaActionType::REFPOINT:
                        // handled via pCurrAct->Execute( &rVDev )
                        break;

                    case MetaActionType::MAPMODE:
                        // modify current mapModeTransformation
                        // transformation, such that subsequent
                        // coordinates map correctly
                        tools::calcLogic2PixelAffineTransform( rStates.getState().mapModeTransform,
                                                               rVDev );
                        break;

                    // monitor clip regions, to assemble clip polygon on our own
                    case MetaActionType::CLIPREGION:
                    {
                        MetaClipRegionAction* pClipAction = static_cast<MetaClipRegionAction*>(pCurrAct);

                        if( !pClipAction->IsClipping() )
                        {
                            // clear clipping
                            rStates.getState().clip.clear();
                        }
                        else
                        {
                            if( !pClipAction->GetRegion().HasPolyPolygonOrB2DPolyPolygon() )
                            {
                                SAL_INFO( "cppcanvas.emf", "ImplRenderer::createActions(): non-polygonal clip "
                                               "region encountered, falling back to bounding box!" );

                                // #121806# explicitly kept integer
                                ::tools::Rectangle aClipRect(
                                    rVDev.LogicToPixel(
                                        pClipAction->GetRegion().GetBoundRect() ) );

                                // intersect current clip with given rect
                                updateClipping(
                                    aClipRect,
                                    rFactoryParms,
                                    false );
                            }
                            else
                            {
                                // set new clip polygon (don't intersect
                                // with old one, just set it)

                                // #121806# explicitly kept integer
                                basegfx::B2DPolyPolygon aPolyPolygon(pClipAction->GetRegion().GetAsB2DPolyPolygon());

                                aPolyPolygon.transform(rVDev.GetViewTransformation());
                                updateClipping(
                                    aPolyPolygon,
                                    rFactoryParms,
                                    false );
                            }
                        }

                        break;
                    }

                    case MetaActionType::ISECTRECTCLIPREGION:
                    {
                        MetaISectRectClipRegionAction* pClipAction = static_cast<MetaISectRectClipRegionAction*>(pCurrAct);

                        // #121806# explicitly kept integer
                        ::tools::Rectangle aClipRect(
                            rVDev.LogicToPixel( pClipAction->GetRect() ) );

                        // intersect current clip with given rect
                        updateClipping(
                            aClipRect,
                            rFactoryParms,
                            true );

                        break;
                    }

                    case MetaActionType::ISECTREGIONCLIPREGION:
                    {
                        MetaISectRegionClipRegionAction* pClipAction = static_cast<MetaISectRegionClipRegionAction*>(pCurrAct);

                        if( !pClipAction->GetRegion().HasPolyPolygonOrB2DPolyPolygon() )
                        {
                            SAL_INFO( "cppcanvas.emf", "ImplRenderer::createActions(): non-polygonal clip "
                                           "region encountered, falling back to bounding box!" );

                            // #121806# explicitly kept integer
                            ::tools::Rectangle aClipRect(
                                rVDev.LogicToPixel( pClipAction->GetRegion().GetBoundRect() ) );

                            // intersect current clip with given rect
                            updateClipping(
                                aClipRect,
                                rFactoryParms,
                                true );
                        }
                        else
                        {
                            // intersect current clip with given clip polygon

                            // #121806# explicitly kept integer
                            basegfx::B2DPolyPolygon aPolyPolygon(pClipAction->GetRegion().GetAsB2DPolyPolygon());

                            aPolyPolygon.transform(rVDev.GetViewTransformation());
                            updateClipping(
                                aPolyPolygon,
                                rFactoryParms,
                                true );
                        }

                        break;
                    }

                    case MetaActionType::MOVECLIPREGION:
                        // TODO(F2): NYI
                        break;

                    case MetaActionType::LINECOLOR:
                        if( !rParms.maLineColor )
                        {
                            setStateColor( static_cast<MetaLineColorAction*>(pCurrAct),
                                           rStates.getState().isLineColorSet,
                                           rStates.getState().lineColor,
                                           rCanvas );
                        }
                        else
                        {
                            // #120994# Do switch on/off LineColor, even when an overriding one is set
                            bool bSetting(static_cast<MetaLineColorAction*>(pCurrAct)->IsSetting());

                            rStates.getState().isLineColorSet = bSetting;
                        }
                        break;

                    case MetaActionType::FILLCOLOR:
                        if( !rParms.maFillColor )
                        {
                            setStateColor( static_cast<MetaFillColorAction*>(pCurrAct),
                                           rStates.getState().isFillColorSet,
                                           rStates.getState().fillColor,
                                           rCanvas );
                        }
                        else
                        {
                            // #120994# Do switch on/off FillColor, even when an overriding one is set
                            bool bSetting(static_cast<MetaFillColorAction*>(pCurrAct)->IsSetting());

                            rStates.getState().isFillColorSet = bSetting;
                        }
                        break;

                    case MetaActionType::TEXTCOLOR:
                    {
                        if( !rParms.maTextColor )
                        {
                            // Text color is set unconditionally, thus, no
                            // use of setStateColor here
                            ::Color aColor( static_cast<MetaTextColorAction*>(pCurrAct)->GetColor() );

                            // force alpha part of color to
                            // opaque. transparent painting is done
                            // explicitly via MetaActionType::Transparent
                            aColor.SetAlpha(255);

                            rStates.getState().textColor =
                                vcl::unotools::colorToDoubleSequence(
                                    aColor,
                                    rCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() );
                        }
                    }
                    break;

                    case MetaActionType::TEXTFILLCOLOR:
                        if( !rParms.maTextColor )
                        {
                            setStateColor( static_cast<MetaTextFillColorAction*>(pCurrAct),
                                           rStates.getState().isTextFillColorSet,
                                           rStates.getState().textFillColor,
                                           rCanvas );
                        }
                        else
                        {
                            // #120994# Do switch on/off TextFillColor, even when an overriding one is set
                            bool bSetting(static_cast<MetaTextFillColorAction*>(pCurrAct)->IsSetting());

                            rStates.getState().isTextFillColorSet = bSetting;
                        }
                        break;

                    case MetaActionType::TEXTLINECOLOR:
                        if( !rParms.maTextColor )
                        {
                            setStateColor( static_cast<MetaTextLineColorAction*>(pCurrAct),
                                           rStates.getState().isTextLineColorSet,
                                           rStates.getState().textLineColor,
                                           rCanvas );
                        }
                        else
                        {
                            // #120994# Do switch on/off TextLineColor, even when an overriding one is set
                            bool bSetting(static_cast<MetaTextLineColorAction*>(pCurrAct)->IsSetting());

                            rStates.getState().isTextLineColorSet = bSetting;
                        }
                        break;

                    case MetaActionType::OVERLINECOLOR:
                        if( !rParms.maTextColor )
                        {
                            setStateColor( static_cast<MetaOverlineColorAction*>(pCurrAct),
                                           rStates.getState().isTextOverlineColorSet,
                                           rStates.getState().textOverlineColor,
                                           rCanvas );
                        }
                        else
                        {
                            bool bSetting(static_cast<MetaOverlineColorAction*>(pCurrAct)->IsSetting());

                            rStates.getState().isTextOverlineColorSet = bSetting;
                        }
                        break;

                    case MetaActionType::TEXTALIGN:
                    {
                        ::cppcanvas::internal::OutDevState& rState = rStates.getState();
                        const TextAlign eTextAlign( static_cast<MetaTextAlignAction*>(pCurrAct)->GetTextAlign() );

                        rState.textReferencePoint = eTextAlign;
                    }
                    break;

                    case MetaActionType::FONT:
                    {
                        ::cppcanvas::internal::OutDevState& rState = rStates.getState();
                        const vcl::Font& rFont( static_cast<MetaFontAction*>(pCurrAct)->GetFont() );

                        rState.xFont = createFont( rState.fontRotation,
                                                   rFont,
                                                   rFactoryParms );

                        // TODO(Q2): define and use appropriate enumeration types
                        rState.textReliefStyle          = rFont.GetRelief();
                        rState.textOverlineStyle        = static_cast<sal_Int8>(rFont.GetOverline());
                        rState.textUnderlineStyle       = rParms.maFontUnderline.has_value() ?
                            (*rParms.maFontUnderline ? sal_Int8(LINESTYLE_SINGLE) : sal_Int8(LINESTYLE_NONE)) :
                            static_cast<sal_Int8>(rFont.GetUnderline());
                        rState.textStrikeoutStyle       = static_cast<sal_Int8>(rFont.GetStrikeout());
                        rState.textEmphasisMark         = rFont.GetEmphasisMark();
                        rState.isTextEffectShadowSet    = rFont.IsShadow();
                        rState.isTextWordUnderlineSet   = rFont.IsWordLineMode();
                        rState.isTextOutlineModeSet     = rFont.IsOutline();
                    }
                    break;

                    case MetaActionType::RASTEROP:
                        // TODO(F2): NYI
                        break;

                    case MetaActionType::LAYOUTMODE:
                    {
                        // TODO(F2): A lot is missing here
                        vcl::text::ComplexTextLayoutFlags nLayoutMode = static_cast<MetaLayoutModeAction*>(pCurrAct)->GetLayoutMode();
                        ::cppcanvas::internal::OutDevState& rState = rStates.getState();

                        vcl::text::ComplexTextLayoutFlags nBidiLayoutMode = nLayoutMode & (vcl::text::ComplexTextLayoutFlags::BiDiRtl|vcl::text::ComplexTextLayoutFlags::BiDiStrong);
                        if( nBidiLayoutMode == vcl::text::ComplexTextLayoutFlags::Default)
                                rState.textDirection = rendering::TextDirection::WEAK_LEFT_TO_RIGHT;
                        else if( nBidiLayoutMode == vcl::text::ComplexTextLayoutFlags::BiDiStrong)
                                rState.textDirection = rendering::TextDirection::STRONG_LEFT_TO_RIGHT;
                        else if( nBidiLayoutMode == vcl::text::ComplexTextLayoutFlags::BiDiRtl)
                                rState.textDirection = rendering::TextDirection::WEAK_RIGHT_TO_LEFT;
                        else if( nBidiLayoutMode == (vcl::text::ComplexTextLayoutFlags::BiDiRtl | vcl::text::ComplexTextLayoutFlags::BiDiStrong))
                                rState.textDirection = rendering::TextDirection::STRONG_RIGHT_TO_LEFT;

                        rState.textAlignment = 0; // TODO(F2): rendering::TextAlignment::LEFT_ALIGNED;
                        if( (nLayoutMode & (vcl::text::ComplexTextLayoutFlags::BiDiRtl | vcl::text::ComplexTextLayoutFlags::TextOriginRight) )
                            && !(nLayoutMode & vcl::text::ComplexTextLayoutFlags::TextOriginLeft ) )
                        {
                            rState.textAlignment = 1; // TODO(F2): rendering::TextAlignment::RIGHT_ALIGNED;
                        }
                    }
                    break;


                    // In the second part of this monster-switch, we
                    // handle all recursing meta actions. These are the
                    // ones generating a metafile by themselves, which is
                    // then processed by recursively calling this method.


                    case MetaActionType::GRADIENT:
                    {
                        MetaGradientAction* pGradAct = static_cast<MetaGradientAction*>(pCurrAct);
                        createGradientAction( ::tools::PolyPolygon( pGradAct->GetRect() ),
                                              pGradAct->GetGradient(),
                                              rFactoryParms,
                                              true,
                                              bSubsettableActions );
                    }
                    break;

                    case MetaActionType::HATCH:
                    {
                        // TODO(F2): use native Canvas hatches here
                        GDIMetaFile aTmpMtf;

                        rVDev.AddHatchActions( static_cast<MetaHatchAction*>(pCurrAct)->GetPolyPolygon(),
                                               static_cast<MetaHatchAction*>(pCurrAct)->GetHatch(),
                                               aTmpMtf );
                        createActions( aTmpMtf, rFactoryParms,
                                       bSubsettableActions );
                    }
                    break;

                    case MetaActionType::EPS:
                    {
                        MetaEPSAction*      pAct = static_cast<MetaEPSAction*>(pCurrAct);
                        const GDIMetaFile&  rSubstitute = pAct->GetSubstitute();

                        // #121806# explicitly kept integer
                        const Size aMtfSize( rSubstitute.GetPrefSize() );
                        const Size aMtfSizePixPre( rVDev.LogicToPixel( aMtfSize,
                                                                       rSubstitute.GetPrefMapMode() ) );

                        // #i44110# correct null-sized output - there
                        // are metafiles which have zero size in at
                        // least one dimension

                        // Remark the 1L cannot be replaced, that would cause max to compare long/int
                        const Size aMtfSizePix( std::max( aMtfSizePixPre.Width(), ::tools::Long(1) ),
                                                std::max( aMtfSizePixPre.Height(), ::tools::Long(1) ) );

                        // Setup local transform, such that the
                        // metafile renders itself into the given
                        // output rectangle
                        rStates.pushState(vcl::PushFlags::ALL);

                        rVDev.Push();
                        rVDev.SetMapMode( rSubstitute.GetPrefMapMode() );

                        const ::Point aPos( rVDev.LogicToPixel( pAct->GetPoint() ) );
                        const ::Size  aSize( rVDev.LogicToPixel( pAct->GetSize() ) );

                        rStates.getState().transform.translate( aPos.X(),
                                                                aPos.Y() );
                        rStates.getState().transform.scale( static_cast<double>(aSize.Width()) / aMtfSizePix.Width(),
                                                             static_cast<double>(aSize.Height()) / aMtfSizePix.Height() );

                        createActions( const_cast<GDIMetaFile&>(pAct->GetSubstitute()),
                                       rFactoryParms,
                                       bSubsettableActions );

                        rVDev.Pop();
                        rStates.popState();
                    }
                    break;

                    // handle metafile comments, to retrieve
                    // meta-information for gradients, fills and
                    // strokes. May skip actions, and may recurse.
                    case MetaActionType::COMMENT:
                    {
                        MetaCommentAction* pAct = static_cast<MetaCommentAction*>(pCurrAct);

                        // Handle gradients
                        if (pAct->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_BEGIN"))
                        {
                            MetaGradientExAction* pGradAction = nullptr;
                            bool bDone( false );
                            while( !bDone )
                            {
                                pCurrAct=rMtf.NextAction();
                                if (!pCurrAct)
                                    break;
                                switch( pCurrAct->GetType() )
                                {
                                    // extract gradient info
                                    case MetaActionType::GRADIENTEX:
                                        pGradAction = static_cast<MetaGradientExAction*>(pCurrAct);
                                        break;

                                    // skip broken-down rendering, output gradient when sequence is ended
                                    case MetaActionType::COMMENT:
                                        if( static_cast<MetaCommentAction*>(pCurrAct)->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_END") )
                                        {
                                            bDone = true;

                                            if( pGradAction )
                                            {
                                                createGradientAction( pGradAction->GetPolyPolygon(),
                                                                      pGradAction->GetGradient(),
                                                                      rFactoryParms,
                                                                      false,
                                                                      bSubsettableActions );
                                            }
                                        }
                                        break;
                                    default: break;
                                }
                            }
                        }
                        // TODO(P2): Handle drawing layer strokes, via
                        // XPATHSTROKE_SEQ_BEGIN comment

                        // Handle drawing layer fills
                        else if( pAct->GetComment() == "XPATHFILL_SEQ_BEGIN" )
                        {
                            const sal_uInt8* pData = pAct->GetData();
                            if ( pData )
                            {
                                SvMemoryStream  aMemStm( const_cast<sal_uInt8 *>(pData), pAct->GetDataSize(), StreamMode::READ );

                                SvtGraphicFill aFill;
                                ReadSvtGraphicFill( aMemStm, aFill );

                                // TODO(P2): Also handle gradients and
                                // hatches like this

                                // only evaluate comment for pure
                                // bitmap fills. If a transparency
                                // gradient is involved (denoted by
                                // the FloatTransparent action), take
                                // the normal meta actions.
                                if( aFill.getFillType() == SvtGraphicFill::fillTexture &&
                                    !isActionContained( rMtf,
                                                       "XPATHFILL_SEQ_END",
                                                        MetaActionType::FLOATTRANSPARENT ) )
                                {
                                    rendering::Texture aTexture;

                                    // TODO(F1): the SvtGraphicFill
                                    // can also transport metafiles
                                    // here, handle that case, too
                                    Graphic aGraphic;
                                    aFill.getGraphic( aGraphic );

                                    BitmapEx     aBmpEx( aGraphic.GetBitmapEx() );
                                    const ::Size aBmpSize( aBmpEx.GetSizePixel() );

                                    ::SvtGraphicFill::Transform aTransform;
                                    aFill.getTransform( aTransform );

                                    ::basegfx::B2DHomMatrix aMatrix;

                                    // convert to basegfx matrix
                                    aMatrix.set(0,0, aTransform.matrix[ 0 ] );
                                    aMatrix.set(0,1, aTransform.matrix[ 1 ] );
                                    aMatrix.set(0,2, aTransform.matrix[ 2 ] );
                                    aMatrix.set(1,0, aTransform.matrix[ 3 ] );
                                    aMatrix.set(1,1, aTransform.matrix[ 4 ] );
                                    aMatrix.set(1,2, aTransform.matrix[ 5 ] );

                                    ::basegfx::B2DHomMatrix aScale;
                                    aScale.scale( aBmpSize.Width(),
                                                  aBmpSize.Height() );

                                    // post-multiply with the bitmap
                                    // size (XCanvas' texture assumes
                                    // the given bitmap to be
                                    // normalized to [0,1]x[0,1]
                                    // rectangle)
                                    aMatrix = aMatrix * aScale;

                                    // pre-multiply with the
                                    // logic-to-pixel scale factor
                                    // (the metafile comment works in
                                    // logical coordinates).
                                    ::basegfx::B2DHomMatrix aLogic2PixelTransform;
                                    aMatrix *= tools::calcLogic2PixelLinearTransform( aLogic2PixelTransform,
                                                                                      rVDev );

                                    ::basegfx::unotools::affineMatrixFromHomMatrix(
                                        aTexture.AffineTransform,
                                        aMatrix );

                                    aTexture.Alpha = 1.0 - aFill.getTransparency();
                                    aTexture.Bitmap = vcl::unotools::xBitmapFromBitmapEx( aBmpEx );
                                    if( aFill.isTiling() )
                                    {
                                        aTexture.RepeatModeX = rendering::TexturingMode::REPEAT;
                                        aTexture.RepeatModeY = rendering::TexturingMode::REPEAT;
                                    }
                                    else
                                    {
                                        aTexture.RepeatModeX = rendering::TexturingMode::NONE;
                                        aTexture.RepeatModeY = rendering::TexturingMode::NONE;
                                    }

                                    ::tools::PolyPolygon aPath;
                                    aFill.getPath( aPath );

                                    ::basegfx::B2DPolyPolygon aPoly( aPath.getB2DPolyPolygon() );
                                    aPoly.transform( rStates.getState().mapModeTransform );
                                    std::shared_ptr<Action> pPolyAction(
                                        internal::PolyPolyActionFactory::createPolyPolyAction(
                                            aPoly,
                                            rCanvas,
                                            rStates.getState(),
                                            aTexture ) );

                                    if( pPolyAction )
                                    {
                                        maActions.emplace_back(
                                                pPolyAction,
                                                io_rCurrActionIndex );

                                        io_rCurrActionIndex += pPolyAction->getActionCount()-1;
                                    }

                                    // skip broken-down render output
                                    skipContent( rMtf,
                                                 "XPATHFILL_SEQ_END",
                                                 io_rCurrActionIndex );
                                }
                            }
                        }
                        // Handle drawing layer fills
                        else if( pAct->GetComment() == "EMF_PLUS" ) {
                            SAL_WARN ("cppcanvas.emf", "EMF+ code was refactored and removed");
                        } else if( pAct->GetComment() == "EMF_PLUS_HEADER_INFO" ) {
                            SAL_INFO ("cppcanvas.emf", "EMF+ passed to canvas mtf renderer - header info, size: " << pAct->GetDataSize ());

                            SvMemoryStream rMF (const_cast<sal_uInt8 *>(pAct->GetData ()), pAct->GetDataSize (), StreamMode::READ);

                            rMF.ReadInt32( nFrameLeft ).ReadInt32( nFrameTop ).ReadInt32( nFrameRight ).ReadInt32( nFrameBottom );
                            SAL_INFO ("cppcanvas.emf", "EMF+ picture frame: " << nFrameLeft << "," << nFrameTop << " - " << nFrameRight << "," << nFrameBottom);
                            rMF.ReadInt32( nPixX ).ReadInt32( nPixY ).ReadInt32( nMmX ).ReadInt32( nMmY );
                            SAL_INFO ("cppcanvas.emf", "EMF+ ref device pixel size: " << nPixX << "x" << nPixY << " mm size: " << nMmX << "x" << nMmY);

                            ReadXForm( rMF, aBaseTransform );
                            //aWorldTransform.Set (aBaseTransform);
                        }
                    }
                    break;


                    // In the third part of this monster-switch, we
                    // handle all 'acting' meta actions. These are all
                    // processed by constructing function objects for
                    // them, which will later ease caching.


                    case MetaActionType::POINT:
                    {
                        const OutDevState& rState( rStates.getState() );
                        if( rState.lineColor.hasElements() )
                        {
                            std::shared_ptr<Action> pPointAction(
                                internal::PointActionFactory::createPointAction(
                                    rState.mapModeTransform * vcl::unotools::b2DPointFromPoint(
                                        static_cast<MetaPointAction*>(pCurrAct)->GetPoint() ),
                                    rCanvas,
                                    rState ) );

                            if( pPointAction )
                            {
                                maActions.emplace_back(
                                        pPointAction,
                                        io_rCurrActionIndex );

                                io_rCurrActionIndex += pPointAction->getActionCount()-1;
                            }
                        }
                    }
                    break;

                    case MetaActionType::PIXEL:
                    {
                        const OutDevState& rState( rStates.getState() );
                        if( rState.lineColor.hasElements() )
                        {
                            std::shared_ptr<Action> pPointAction(
                                internal::PointActionFactory::createPointAction(
                                    rState.mapModeTransform * vcl::unotools::b2DPointFromPoint(
                                        static_cast<MetaPixelAction*>(pCurrAct)->GetPoint() ),
                                    rCanvas,
                                    rState,
                                    static_cast<MetaPixelAction*>(pCurrAct)->GetColor() ) );

                            if( pPointAction )
                            {
                                maActions.emplace_back(
                                        pPointAction,
                                        io_rCurrActionIndex );

                                io_rCurrActionIndex += pPointAction->getActionCount()-1;
                            }
                        }
                    }
                    break;

                    case MetaActionType::LINE:
                    {
                        const OutDevState& rState( rStates.getState() );
                        if( rState.lineColor.hasElements() )
                        {
                            MetaLineAction* pLineAct = static_cast<MetaLineAction*>(pCurrAct);

                            const LineInfo& rLineInfo( pLineAct->GetLineInfo() );

                            const ::basegfx::B2DPoint aStartPoint(
                                rState.mapModeTransform * vcl::unotools::b2DPointFromPoint( pLineAct->GetStartPoint() ));
                            const ::basegfx::B2DPoint aEndPoint(
                                rState.mapModeTransform * vcl::unotools::b2DPointFromPoint( pLineAct->GetEndPoint() ));

                            std::shared_ptr<Action> pLineAction;

                            if( rLineInfo.IsDefault() )
                            {
                                // plain hair line
                                pLineAction =
                                    internal::LineActionFactory::createLineAction(
                                        aStartPoint,
                                        aEndPoint,
                                        rCanvas,
                                        rState );

                                if( pLineAction )
                                {
                                    maActions.emplace_back(
                                            pLineAction,
                                            io_rCurrActionIndex );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            else if( LineStyle::NONE != rLineInfo.GetStyle() )
                            {
                                // 'thick' line
                                rendering::StrokeAttributes aStrokeAttributes;

                                setupStrokeAttributes( aStrokeAttributes,
                                                       rFactoryParms,
                                                       rLineInfo );

                                // XCanvas can only stroke polygons,
                                // not simple lines - thus, handle
                                // this case via the polypolygon
                                // action
                                ::basegfx::B2DPolygon aPoly;
                                aPoly.append( aStartPoint );
                                aPoly.append( aEndPoint );
                                pLineAction =
                                    internal::PolyPolyActionFactory::createPolyPolyAction(
                                        ::basegfx::B2DPolyPolygon( aPoly ),
                                        rCanvas, rState, aStrokeAttributes );

                                if( pLineAction )
                                {
                                    maActions.emplace_back(
                                            pLineAction,
                                            io_rCurrActionIndex );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            // else: line style is default
                            // (i.e. invisible), don't generate action
                        }
                    }
                    break;

                    case MetaActionType::RECT:
                    {
                        const ::tools::Rectangle& rRect(
                            static_cast<MetaRectAction*>(pCurrAct)->GetRect() );

                        if( rRect.IsEmpty() )
                            break;

                        const OutDevState& rState( rStates.getState() );
                        const ::basegfx::B2DPoint aTopLeftPixel(
                            rState.mapModeTransform * vcl::unotools::b2DPointFromPoint( rRect.TopLeft() ) );
                        const ::basegfx::B2DPoint aBottomRightPixel(
                            rState.mapModeTransform * vcl::unotools::b2DPointFromPoint( rRect.BottomRight() ) +
                            // #121100# OutputDevice::DrawRect() fills
                            // rectangles Apple-like, i.e. with one
                            // additional pixel to the right and bottom.
                            ::basegfx::B2DPoint(1,1) );

                        createFillAndStroke( ::basegfx::utils::createPolygonFromRect(
                                                 ::basegfx::B2DRange( aTopLeftPixel,
                                                                      aBottomRightPixel )),
                                             rFactoryParms );
                        break;
                    }

                    case MetaActionType::ROUNDRECT:
                    {
                        const ::tools::Rectangle& rRect(
                            static_cast<MetaRoundRectAction*>(pCurrAct)->GetRect());

                        if( rRect.IsEmpty() )
                            break;

                        ::basegfx::B2DPolygon aPoly(
                            ::basegfx::utils::createPolygonFromRect(
                                ::basegfx::B2DRange(
                                    vcl::unotools::b2DPointFromPoint( rRect.TopLeft() ),
                                    vcl::unotools::b2DPointFromPoint( rRect.BottomRight() ) +
                                    ::basegfx::B2DPoint(1,1) ),
                                static_cast<double>(static_cast<MetaRoundRectAction*>(pCurrAct)->GetHorzRound()) / rRect.GetWidth(),
                                static_cast<double>(static_cast<MetaRoundRectAction*>(pCurrAct)->GetVertRound()) / rRect.GetHeight() ) );
                        aPoly.transform( rStates.getState().mapModeTransform );

                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case MetaActionType::ELLIPSE:
                    {
                        const ::tools::Rectangle& rRect(
                            static_cast<MetaEllipseAction*>(pCurrAct)->GetRect() );

                        if( rRect.IsEmpty() )
                            break;

                        const ::basegfx::B2DRange aRange(
                            vcl::unotools::b2DPointFromPoint( rRect.TopLeft() ),
                            vcl::unotools::b2DPointFromPoint( rRect.BottomRight() ) +
                            ::basegfx::B2DPoint(1,1) );

                        ::basegfx::B2DPolygon aPoly(
                            ::basegfx::utils::createPolygonFromEllipse(
                                aRange.getCenter(),
                                aRange.getWidth() / 2,       // divide by 2 since createPolygonFromEllipse
                                aRange.getHeight() / 2 ));   // expects the radius and NOT the diameter!
                        aPoly.transform( rStates.getState().mapModeTransform );

                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case MetaActionType::ARC:
                    {
                        // TODO(F1): Missing basegfx functionality. Mind empty rects!
                        const ::tools::Polygon aToolsPoly( static_cast<MetaArcAction*>(pCurrAct)->GetRect(),
                                                         static_cast<MetaArcAction*>(pCurrAct)->GetStartPoint(),
                                                         static_cast<MetaArcAction*>(pCurrAct)->GetEndPoint(), PolyStyle::Arc );
                        ::basegfx::B2DPolygon aPoly( aToolsPoly.getB2DPolygon() );
                        aPoly.transform( rStates.getState().mapModeTransform );

                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case MetaActionType::PIE:
                    {
                        // TODO(F1): Missing basegfx functionality. Mind empty rects!
                        const ::tools::Polygon aToolsPoly( static_cast<MetaPieAction*>(pCurrAct)->GetRect(),
                                                         static_cast<MetaPieAction*>(pCurrAct)->GetStartPoint(),
                                                         static_cast<MetaPieAction*>(pCurrAct)->GetEndPoint(), PolyStyle::Pie );
                        ::basegfx::B2DPolygon aPoly( aToolsPoly.getB2DPolygon() );
                        aPoly.transform( rStates.getState().mapModeTransform );

                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case MetaActionType::CHORD:
                    {
                        // TODO(F1): Missing basegfx functionality. Mind empty rects!
                        const ::tools::Polygon aToolsPoly( static_cast<MetaChordAction*>(pCurrAct)->GetRect(),
                                                         static_cast<MetaChordAction*>(pCurrAct)->GetStartPoint(),
                                                         static_cast<MetaChordAction*>(pCurrAct)->GetEndPoint(), PolyStyle::Chord );
                        ::basegfx::B2DPolygon aPoly( aToolsPoly.getB2DPolygon() );
                        aPoly.transform( rStates.getState().mapModeTransform );

                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case MetaActionType::POLYLINE:
                    {
                        const OutDevState& rState( rStates.getState() );
                        if( rState.lineColor.hasElements() ||
                            rState.fillColor.hasElements() )
                        {
                            MetaPolyLineAction* pPolyLineAct = static_cast<MetaPolyLineAction*>(pCurrAct);

                            const LineInfo& rLineInfo( pPolyLineAct->GetLineInfo() );
                            ::basegfx::B2DPolygon aPoly( pPolyLineAct->GetPolygon().getB2DPolygon() );
                            aPoly.transform( rState.mapModeTransform );

                            std::shared_ptr<Action> pLineAction;

                            if( rLineInfo.IsDefault() )
                            {
                                // plain hair line polygon
                                pLineAction =
                                    internal::PolyPolyActionFactory::createLinePolyPolyAction(
                                        ::basegfx::B2DPolyPolygon(aPoly),
                                        rCanvas,
                                        rState );

                                if( pLineAction )
                                {
                                    maActions.emplace_back(
                                            pLineAction,
                                            io_rCurrActionIndex );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            else if( LineStyle::NONE != rLineInfo.GetStyle() )
                            {
                                // 'thick' line polygon
                                rendering::StrokeAttributes aStrokeAttributes;

                                setupStrokeAttributes( aStrokeAttributes,
                                                       rFactoryParms,
                                                       rLineInfo );

                                pLineAction =
                                    internal::PolyPolyActionFactory::createPolyPolyAction(
                                        ::basegfx::B2DPolyPolygon(aPoly),
                                        rCanvas,
                                        rState,
                                        aStrokeAttributes ) ;

                                if( pLineAction )
                                {
                                    maActions.emplace_back(
                                            pLineAction,
                                            io_rCurrActionIndex );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            // else: line style is default
                            // (i.e. invisible), don't generate action
                        }
                    }
                    break;

                    case MetaActionType::POLYGON:
                    {
                        ::basegfx::B2DPolygon aPoly( static_cast<MetaPolygonAction*>(pCurrAct)->GetPolygon().getB2DPolygon() );
                        aPoly.transform( rStates.getState().mapModeTransform );
                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case MetaActionType::POLYPOLYGON:
                    {
                        ::basegfx::B2DPolyPolygon aPoly( static_cast<MetaPolyPolygonAction*>(pCurrAct)->GetPolyPolygon().getB2DPolyPolygon() );
                        aPoly.transform( rStates.getState().mapModeTransform );
                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case MetaActionType::BMP:
                    {
                        MetaBmpAction* pAct = static_cast<MetaBmpAction*>(pCurrAct);

                        std::shared_ptr<Action> pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    BitmapEx(pAct->GetBitmap()),
                                    rStates.getState().mapModeTransform *
                                    vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                    rCanvas,
                                    rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.emplace_back(
                                    pBmpAction,
                                    io_rCurrActionIndex );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case MetaActionType::BMPSCALE:
                    {
                        MetaBmpScaleAction* pAct = static_cast<MetaBmpScaleAction*>(pCurrAct);

                        std::shared_ptr<Action> pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    BitmapEx(pAct->GetBitmap()),
                                    rStates.getState().mapModeTransform * vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                    rStates.getState().mapModeTransform * vcl::unotools::b2DVectorFromSize( pAct->GetSize() ),
                                    rCanvas,
                                    rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.emplace_back(
                                    pBmpAction,
                                    io_rCurrActionIndex );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case MetaActionType::BMPSCALEPART:
                    {
                        MetaBmpScalePartAction* pAct = static_cast<MetaBmpScalePartAction*>(pCurrAct);

                        // crop bitmap to given source rectangle (no
                        // need to copy and convert the whole bitmap)
                        ::Bitmap aBmp( pAct->GetBitmap() );
                        const ::tools::Rectangle aCropRect( pAct->GetSrcPoint(),
                                                    pAct->GetSrcSize() );
                        aBmp.Crop( aCropRect );

                        std::shared_ptr<Action> pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    BitmapEx(aBmp),
                                    rStates.getState().mapModeTransform *
                                    vcl::unotools::b2DPointFromPoint( pAct->GetDestPoint() ),
                                    rStates.getState().mapModeTransform *
                                    vcl::unotools::b2DVectorFromSize( pAct->GetDestSize() ),
                                    rCanvas,
                                    rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.emplace_back(
                                    pBmpAction,
                                    io_rCurrActionIndex );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case MetaActionType::BMPEX:
                    {
                        MetaBmpExAction* pAct = static_cast<MetaBmpExAction*>(pCurrAct);

                        std::shared_ptr<Action> pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    pAct->GetBitmapEx(),
                                    rStates.getState().mapModeTransform *
                                    vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                    rCanvas,
                                    rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.emplace_back(
                                    pBmpAction,
                                    io_rCurrActionIndex );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case MetaActionType::BMPEXSCALE:
                    {
                        MetaBmpExScaleAction* pAct = static_cast<MetaBmpExScaleAction*>(pCurrAct);

                        std::shared_ptr<Action> pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    pAct->GetBitmapEx(),
                                    rStates.getState().mapModeTransform *
                                    vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                    rStates.getState().mapModeTransform *
                                    vcl::unotools::b2DVectorFromSize( pAct->GetSize() ),
                                    rCanvas,
                                    rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.emplace_back(
                                    pBmpAction,
                                    io_rCurrActionIndex );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case MetaActionType::BMPEXSCALEPART:
                    {
                        MetaBmpExScalePartAction* pAct = static_cast<MetaBmpExScalePartAction*>(pCurrAct);

                        // crop bitmap to given source rectangle (no
                        // need to copy and convert the whole bitmap)
                        BitmapEx aBmp( pAct->GetBitmapEx() );
                        const ::tools::Rectangle aCropRect( pAct->GetSrcPoint(),
                                                   pAct->GetSrcSize() );
                        aBmp.Crop( aCropRect );

                        std::shared_ptr<Action> pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rStates.getState().mapModeTransform *
                                vcl::unotools::b2DPointFromPoint( pAct->GetDestPoint() ),
                                rStates.getState().mapModeTransform *
                                vcl::unotools::b2DVectorFromSize( pAct->GetDestSize() ),
                                rCanvas,
                                rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.emplace_back(
                                    pBmpAction,
                                    io_rCurrActionIndex );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case MetaActionType::MASK:
                    {
                        MetaMaskAction* pAct = static_cast<MetaMaskAction*>(pCurrAct);

                        // create masked BitmapEx right here, as the
                        // canvas does not provide equivalent
                        // functionality
                        BitmapEx aBmp( createMaskBmpEx( pAct->GetBitmap(),
                                                        pAct->GetColor() ));

                        std::shared_ptr<Action> pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rStates.getState().mapModeTransform *
                                vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                rCanvas,
                                rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.emplace_back(
                                    pBmpAction,
                                    io_rCurrActionIndex );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case MetaActionType::MASKSCALE:
                    {
                        MetaMaskScaleAction* pAct = static_cast<MetaMaskScaleAction*>(pCurrAct);

                        // create masked BitmapEx right here, as the
                        // canvas does not provide equivalent
                        // functionality
                        BitmapEx aBmp( createMaskBmpEx( pAct->GetBitmap(),
                                                        pAct->GetColor() ));

                        std::shared_ptr<Action> pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rStates.getState().mapModeTransform *
                                vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                rStates.getState().mapModeTransform *
                                vcl::unotools::b2DVectorFromSize( pAct->GetSize() ),
                                rCanvas,
                                rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.emplace_back(
                                    pBmpAction,
                                    io_rCurrActionIndex );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case MetaActionType::MASKSCALEPART:
                    {
                        MetaMaskScalePartAction* pAct = static_cast<MetaMaskScalePartAction*>(pCurrAct);

                        // create masked BitmapEx right here, as the
                        // canvas does not provide equivalent
                        // functionality
                        BitmapEx aBmp( createMaskBmpEx( pAct->GetBitmap(),
                                                        pAct->GetColor() ));

                        // crop bitmap to given source rectangle (no
                        // need to copy and convert the whole bitmap)
                        const ::tools::Rectangle aCropRect( pAct->GetSrcPoint(),
                                                   pAct->GetSrcSize() );
                        aBmp.Crop( aCropRect );

                        std::shared_ptr<Action> pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rStates.getState().mapModeTransform *
                                vcl::unotools::b2DPointFromPoint( pAct->GetDestPoint() ),
                                rStates.getState().mapModeTransform *
                                vcl::unotools::b2DVectorFromSize( pAct->GetDestSize() ),
                                rCanvas,
                                rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.emplace_back(
                                    pBmpAction,
                                    io_rCurrActionIndex );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case MetaActionType::GRADIENTEX:
                        // TODO(F1): use native Canvas gradients here
                        // action is ignored here, because redundant to MetaActionType::GRADIENT
                        break;

                    case MetaActionType::WALLPAPER:
                        // TODO(F2): NYI
                        break;

                    case MetaActionType::Transparent:
                    {
                        const OutDevState& rState( rStates.getState() );
                        if( rState.lineColor.hasElements() ||
                            rState.fillColor.hasElements() )
                        {
                            MetaTransparentAction* pAct = static_cast<MetaTransparentAction*>(pCurrAct);
                            ::basegfx::B2DPolyPolygon aPoly( pAct->GetPolyPolygon().getB2DPolyPolygon() );
                            aPoly.transform( rState.mapModeTransform );

                            std::shared_ptr<Action> pPolyAction(
                                internal::PolyPolyActionFactory::createPolyPolyAction(
                                    aPoly,
                                    rCanvas,
                                    rState,
                                    pAct->GetTransparence() ) );

                            if( pPolyAction )
                            {
                                maActions.emplace_back(
                                        pPolyAction,
                                        io_rCurrActionIndex );

                                io_rCurrActionIndex += pPolyAction->getActionCount()-1;
                            }
                        }
                    }
                    break;

                    case MetaActionType::FLOATTRANSPARENT:
                    {
                        MetaFloatTransparentAction* pAct = static_cast<MetaFloatTransparentAction*>(pCurrAct);

                        std::unique_ptr< GDIMetaFile > pMtf(
                            new ::GDIMetaFile( pAct->GetGDIMetaFile() ) );

                        // TODO(P2): Use native canvas gradients here (saves a lot of UNO calls)
                        std::optional< Gradient > pGradient( pAct->GetGradient() );

                        DBG_TESTSOLARMUTEX();

                        std::shared_ptr<Action> pFloatTransAction(
                            internal::TransparencyGroupActionFactory::createTransparencyGroupAction(
                                std::move(pMtf),
                                std::move(pGradient),
                                rStates.getState().mapModeTransform *
                                vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                rStates.getState().mapModeTransform *
                                vcl::unotools::b2DVectorFromSize( pAct->GetSize() ),
                                rCanvas,
                                rStates.getState() ) );

                        if( pFloatTransAction )
                        {
                            maActions.emplace_back(
                                    pFloatTransAction,
                                    io_rCurrActionIndex );

                            io_rCurrActionIndex += pFloatTransAction->getActionCount()-1;
                        }
                    }
                    break;

                    case MetaActionType::TEXT:
                    {
                        MetaTextAction* pAct = static_cast<MetaTextAction*>(pCurrAct);
                        OUString sText = pAct->GetText();

                        if (rVDev.GetDigitLanguage())
                            sText = convertToLocalizedNumerals(sText, rVDev.GetDigitLanguage());

                        const sal_Int32 nLen = std::min(pAct->GetLen(), pAct->GetText().getLength() - pAct->GetIndex());

                        createTextAction(
                            pAct->GetPoint(),
                            sText,
                            pAct->GetIndex(),
                            nLen,
                            {},
                            {},
                            rFactoryParms,
                            bSubsettableActions );
                    }
                    break;

                    case MetaActionType::TEXTARRAY:
                    {
                        MetaTextArrayAction* pAct = static_cast<MetaTextArrayAction*>(pCurrAct);
                        OUString sText = pAct->GetText();

                        if (rVDev.GetDigitLanguage())
                            sText = convertToLocalizedNumerals(sText, rVDev.GetDigitLanguage());

                        const sal_Int32 nLen = std::min(pAct->GetLen(), pAct->GetText().getLength() - pAct->GetIndex());

                        createTextAction(
                            pAct->GetPoint(),
                            sText,
                            pAct->GetIndex(),
                            nLen,
                            pAct->GetDXArray(),
                            pAct->GetKashidaArray(),
                            rFactoryParms,
                            bSubsettableActions );
                    }
                    break;

                    case MetaActionType::TEXTLINE:
                    {
                        MetaTextLineAction*      pAct = static_cast<MetaTextLineAction*>(pCurrAct);

                        const OutDevState&       rState( rStates.getState() );
                        const ::Size             aBaselineOffset( tools::getBaselineOffset( rState,
                                                                                            rVDev ) );
                        const ::basegfx::B2DSize aSize( rState.mapModeTransform *
                                                        ::basegfx::B2DSize(pAct->GetWidth(),
                                                                           0 ));

                        std::shared_ptr<Action> pPolyAction(
                            PolyPolyActionFactory::createPolyPolyAction(
                                tools::createTextLinesPolyPolygon(
                                    rState.mapModeTransform *
                                    ::basegfx::B2DPoint(
                                        vcl::unotools::b2DPointFromPoint(pAct->GetStartPoint()) +
                                        vcl::unotools::b2DVectorFromSize(aBaselineOffset)),
                                    aSize.getWidth(),
                                    tools::createTextLineInfo( rVDev,
                                                               rState )),
                                rCanvas,
                                rState ) );

                        if( pPolyAction )
                        {
                            maActions.emplace_back(
                                    pPolyAction,
                                    io_rCurrActionIndex );

                            io_rCurrActionIndex += pPolyAction->getActionCount()-1;
                        }
                    }
                    break;

                    case MetaActionType::TEXTRECT:
                    {
                        MetaTextRectAction* pAct = static_cast<MetaTextRectAction*>(pCurrAct);

                        rStates.pushState(vcl::PushFlags::ALL);

                        // use the VDev to break up the text rect
                        // action into readily formatted lines
                        GDIMetaFile aTmpMtf;
                        rVDev.AddTextRectActions( pAct->GetRect(),
                                                  pAct->GetText(),
                                                  pAct->GetStyle(),
                                                  aTmpMtf );

                        createActions( aTmpMtf,
                                       rFactoryParms,
                                       bSubsettableActions );

                        rStates.popState();

                        break;
                    }

                    case MetaActionType::STRETCHTEXT:
                    {
                        MetaStretchTextAction* pAct = static_cast<MetaStretchTextAction*>(pCurrAct);
                        OUString sText = pAct->GetText();

                        if (rVDev.GetDigitLanguage())
                            sText = convertToLocalizedNumerals(sText, rVDev.GetDigitLanguage());

                        const sal_Int32 nLen = std::min(pAct->GetLen(), pAct->GetText().getLength() - pAct->GetIndex());

                        // #i70897# Nothing to do, actually...
                        if( nLen == 0 )
                            break;

                        // have to fit the text into the given
                        // width. This is achieved by internally
                        // generating a DX array, and uniformly
                        // distributing the excess/insufficient width
                        // to every logical character.
                        KernArray aDXArray;

                        rVDev.GetTextArray( pAct->GetText(), &aDXArray,
                                            pAct->GetIndex(), pAct->GetLen() );

                        const double nWidthDifferencePerDx = ( pAct->GetWidth() - aDXArray[ nLen-1 ] ) / nLen;

                        // Last entry of pDXArray contains total width of the text
                        for (sal_Int32 i = 1; i <= nLen; ++i)
                        {
                            // calc ratio for every array entry, to
                            // distribute rounding errors 'evenly'
                            // across the characters. Note that each
                            // entry represents the 'end' position of
                            // the corresponding character, thus, we
                            // let i run from 1 to nLen.
                            aDXArray[i - 1] += i * nWidthDifferencePerDx;
                        }

                        createTextAction(
                            pAct->GetPoint(),
                            sText,
                            pAct->GetIndex(),
                            nLen,
                            aDXArray,
                            {},
                            rFactoryParms,
                            bSubsettableActions );
                    }
                    break;

                    default:
                        SAL_WARN( "cppcanvas.emf", "Unknown meta action type encountered" );
                        break;
                }

                // increment action index (each mtf action counts _at
                // least_ one. Some count for more, therefore,
                // io_rCurrActionIndex is sometimes incremented by
                // pAct->getActionCount()-1 above, the -1 being the
                // correction for the unconditional increment here).
                ++io_rCurrActionIndex;
            }
        }


        namespace
        {
            class ActionRenderer
            {
            public:
                explicit ActionRenderer( ::basegfx::B2DHomMatrix aTransformation ) :
                    maTransformation(std::move( aTransformation )),
                    mbRet( true )
                {
                }

                bool result() const
                {
                    return mbRet;
                }

                void operator()( const ::cppcanvas::internal::ImplRenderer::MtfAction& rAction )
                {
                    // ANDing the result. We want to fail if at least
                    // one action failed.
                    mbRet &= rAction.mpAction->render( maTransformation );
                }

                void operator()( const ::cppcanvas::internal::ImplRenderer::MtfAction&  rAction,
                                 const Action::Subset&                                  rSubset )
                {
                    // ANDing the result. We want to fail if at least
                    // one action failed.
                    mbRet &= rAction.mpAction->renderSubset( maTransformation,
                                                             rSubset );
                }

            private:
                ::basegfx::B2DHomMatrix maTransformation;
                bool                    mbRet;
            };

            class AreaQuery
            {
            public:
                explicit AreaQuery( ::basegfx::B2DHomMatrix aTransformation ) :
                    maTransformation(std::move( aTransformation ))
                {
                }

                static bool result()
                {
                    return true; // nothing can fail here
                }

                void operator()( const ::cppcanvas::internal::ImplRenderer::MtfAction& rAction )
                {
                    maBounds.expand( rAction.mpAction->getBounds( maTransformation ) );
                }

                void operator()( const ::cppcanvas::internal::ImplRenderer::MtfAction&  rAction,
                                 const Action::Subset&                                  rSubset )
                {
                    maBounds.expand( rAction.mpAction->getBounds( maTransformation,
                                                                  rSubset ) );
                }

                const ::basegfx::B2DRange& getBounds() const
                {
                    return maBounds;
                }

            private:
                ::basegfx::B2DHomMatrix maTransformation;
                ::basegfx::B2DRange     maBounds;
            };

            // Doing that via inline class. Compilers tend to not inline free
            // functions.
            struct UpperBoundActionIndexComparator
            {
                bool operator()( const ::cppcanvas::internal::ImplRenderer::MtfAction& rLHS,
                                 const ::cppcanvas::internal::ImplRenderer::MtfAction& rRHS )
                {
                    const sal_Int32 nLHSCount( rLHS.mpAction ?
                                               rLHS.mpAction->getActionCount() : 0 );
                    const sal_Int32 nRHSCount( rRHS.mpAction ?
                                               rRHS.mpAction->getActionCount() : 0 );

                    // compare end of action range, to have an action selected
                    // by lower_bound even if the requested index points in
                    // the middle of the action's range
                    return rLHS.mnOrigIndex + nLHSCount < rRHS.mnOrigIndex + nRHSCount;
                }
            };

            /** Algorithm to apply given functor to a subset range

                @tpl Functor

                Functor to call for each element of the subset
                range. Must provide the following method signatures:
                bool result() (returning false if operation failed)

             */
            template< typename Functor > bool
                forSubsetRange( Functor&                                            rFunctor,
                                ImplRenderer::ActionVector::const_iterator          aRangeBegin,
                                const ImplRenderer::ActionVector::const_iterator&   aRangeEnd,
                                sal_Int32                                           nStartIndex,
                                sal_Int32                                           nEndIndex,
                                const ImplRenderer::ActionVector::const_iterator&   rEnd )
            {
                if( aRangeBegin == aRangeEnd )
                {
                    // only a single action. Setup subset, and call functor
                    Action::Subset aSubset;
                    aSubset.mnSubsetBegin = std::max( sal_Int32( 0 ),
                                                        nStartIndex - aRangeBegin->mnOrigIndex );
                    aSubset.mnSubsetEnd   = std::min( aRangeBegin->mpAction->getActionCount(),
                                                        nEndIndex - aRangeBegin->mnOrigIndex );

                    ENSURE_OR_RETURN_FALSE( aSubset.mnSubsetBegin >= 0 && aSubset.mnSubsetEnd >= 0,
                                      "ImplRenderer::forSubsetRange(): Invalid indices" );

                    rFunctor( *aRangeBegin, aSubset );
                }
                else
                {
                    // more than one action.

                    // render partial first, full intermediate, and
                    // partial last action
                    Action::Subset aSubset;
                    aSubset.mnSubsetBegin = std::max( sal_Int32( 0 ),
                                                        nStartIndex - aRangeBegin->mnOrigIndex );
                    aSubset.mnSubsetEnd   = aRangeBegin->mpAction->getActionCount();

                    ENSURE_OR_RETURN_FALSE( aSubset.mnSubsetBegin >= 0 && aSubset.mnSubsetEnd >= 0,
                                      "ImplRenderer::forSubsetRange(): Invalid indices" );

                    rFunctor( *aRangeBegin, aSubset );

                    // first action rendered, skip to next
                    ++aRangeBegin;

                    // render full middle actions
                    while( aRangeBegin != aRangeEnd )
                        rFunctor( *aRangeBegin++ );

                    if( aRangeEnd == rEnd ||
                        aRangeEnd->mnOrigIndex > nEndIndex )
                    {
                        // aRangeEnd denotes end of action vector,

                        // or

                        // nEndIndex references something _after_
                        // aRangeBegin, but _before_ aRangeEnd

                        // either way: no partial action left
                        return rFunctor.result();
                    }

                    aSubset.mnSubsetBegin = 0;
                    aSubset.mnSubsetEnd   = nEndIndex - aRangeEnd->mnOrigIndex;

                    ENSURE_OR_RETURN_FALSE(aSubset.mnSubsetEnd >= 0,
                                      "ImplRenderer::forSubsetRange(): Invalid indices" );

                    rFunctor( *aRangeEnd, aSubset );
                }

                return rFunctor.result();
            }
        }

        bool ImplRenderer::getSubsetIndices( sal_Int32&                     io_rStartIndex,
                                             sal_Int32&                     io_rEndIndex,
                                             ActionVector::const_iterator&  o_rRangeBegin,
                                             ActionVector::const_iterator&  o_rRangeEnd ) const
        {
            ENSURE_OR_RETURN_FALSE( io_rStartIndex<=io_rEndIndex,
                              "ImplRenderer::getSubsetIndices(): invalid action range" );

            ENSURE_OR_RETURN_FALSE( !maActions.empty(),
                              "ImplRenderer::getSubsetIndices(): no actions to render" );

            const sal_Int32 nMinActionIndex( maActions.front().mnOrigIndex );
            const sal_Int32 nMaxActionIndex( maActions.back().mnOrigIndex +
                                             maActions.back().mpAction->getActionCount() );

            // clip given range to permissible values (there might be
            // ranges before and behind the valid indices)
            io_rStartIndex = std::max( nMinActionIndex,
                                         io_rStartIndex );
            io_rEndIndex = std::min( nMaxActionIndex,
                                       io_rEndIndex );

            if( io_rStartIndex == io_rEndIndex ||
                io_rStartIndex > io_rEndIndex )
            {
                // empty range, don't render anything. The second
                // condition e.g. happens if the requested range lies
                // fully before or behind the valid action indices.
                return false;
            }


            const ActionVector::const_iterator aBegin( maActions.begin() );
            const ActionVector::const_iterator aEnd( maActions.end() );


            // find start and end action
            // =========================
            o_rRangeBegin = std::lower_bound( aBegin, aEnd,
                                                MtfAction( std::shared_ptr<Action>(), io_rStartIndex ),
                                                UpperBoundActionIndexComparator() );
            o_rRangeEnd   = std::lower_bound( aBegin, aEnd,
                                                MtfAction( std::shared_ptr<Action>(), io_rEndIndex ),
                                                UpperBoundActionIndexComparator() );
            return true;
        }


        // Public methods


        ImplRenderer::ImplRenderer( const CanvasSharedPtr&  rCanvas,
                                    const GDIMetaFile&      rMtf,
                                    const Parameters&       rParams )
            : CanvasGraphicHelper(rCanvas)
            , nFrameLeft(0)
            , nFrameTop(0)
            , nFrameRight(0)
            , nFrameBottom(0)
            , nPixX(0)
            , nPixY(0)
            , nMmX(0)
            , nMmY(0)
        {
            SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::ImplRenderer::ImplRenderer(mtf)" );

            OSL_ENSURE( rCanvas && rCanvas->getUNOCanvas().is(),
                        "ImplRenderer::ImplRenderer(): Invalid canvas" );
            OSL_ENSURE( rCanvas->getUNOCanvas()->getDevice().is(),
                        "ImplRenderer::ImplRenderer(): Invalid graphic device" );

            // make sure canvas and graphic device are valid; action
            // creation don't check that every time
            if( !rCanvas ||
                !rCanvas->getUNOCanvas().is() ||
                !rCanvas->getUNOCanvas()->getDevice().is() )
            {
                // leave actions empty
                return;
            }

            VectorOfOutDevStates    aStateStack;

            ScopedVclPtrInstance< VirtualDevice > aVDev;
            aVDev->EnableOutput( false );

            // Setup VDev for state tracking and mapping
            // =========================================

            aVDev->SetMapMode( rMtf.GetPrefMapMode() );

            const Size aMtfSize( rMtf.GetPrefSize() );
            const Size aMtfSizePixPre( aVDev->LogicToPixel( aMtfSize,
                                                           rMtf.GetPrefMapMode() ) );

            // #i44110# correct null-sized output - there are shapes
            // which have zero size in at least one dimension
            // Remark the 1L cannot be replaced, that would cause max to compare long/int
            const Size aMtfSizePix( std::max( aMtfSizePixPre.Width(), ::tools::Long(1) ),
                                    std::max( aMtfSizePixPre.Height(), ::tools::Long(1) ) );

            sal_Int32 nCurrActions(0);
            ActionFactoryParameters aParms(aStateStack,
                                           rCanvas,
                                           *aVDev,
                                           rParams,
                                           nCurrActions );

            // init state stack
            aStateStack.clearStateStack();

            // Setup local state, such that the metafile renders
            // itself into a one-by-one square at the origin for
            // identity view and render transformations
            aStateStack.getState().transform.scale( 1.0 / aMtfSizePix.Width(),
                                                     1.0 / aMtfSizePix.Height() );

            tools::calcLogic2PixelAffineTransform( aStateStack.getState().mapModeTransform,
                                                   *aVDev );

            {
                ::cppcanvas::internal::OutDevState& rState = aStateStack.getState();
                // setup default text color to black
                rState.textColor =
                    rState.textFillColor =
                    rState.textOverlineColor =
                    rState.textLineColor = tools::intSRGBAToDoubleSequence( 0x000000FF );
            }

            // apply overrides from the Parameters struct
            if( rParams.maFillColor )
            {
                ::cppcanvas::internal::OutDevState& rState = aStateStack.getState();
                rState.isFillColorSet = true;
                rState.fillColor = tools::intSRGBAToDoubleSequence( *rParams.maFillColor );
            }
            if( rParams.maLineColor )
            {
                ::cppcanvas::internal::OutDevState& rState = aStateStack.getState();
                rState.isLineColorSet = true;
                rState.lineColor = tools::intSRGBAToDoubleSequence( *rParams.maLineColor );
            }
            if( rParams.maTextColor )
            {
                ::cppcanvas::internal::OutDevState& rState = aStateStack.getState();
                rState.isTextFillColorSet = true;
                rState.isTextOverlineColorSet = true;
                rState.isTextLineColorSet = true;
                rState.textColor =
                    rState.textFillColor =
                    rState.textOverlineColor =
                    rState.textLineColor = tools::intSRGBAToDoubleSequence( *rParams.maTextColor );
            }
            if( rParams.maFontName ||
                rParams.maFontWeight ||
                rParams.maFontLetterForm ||
                rParams.maFontUnderline.has_value() )
            {
                ::cppcanvas::internal::OutDevState& rState = aStateStack.getState();

                rState.xFont = createFont( rState.fontRotation,
                                           vcl::Font(), // default font
                                           aParms );
            }

            /* EMF+ */
            createActions( const_cast<GDIMetaFile&>(rMtf), // HACK(Q2):
                                                           // we're
                                                           // changing
                                                              // the
                                                              // current
                                                              // action
                                                              // in
                                                              // createActions!
                           aParms,
                           true // TODO(P1): make subsettability configurable
                            );
        }

        ImplRenderer::~ImplRenderer()
        {
        }

        bool ImplRenderer::drawSubset( sal_Int32    nStartIndex,
                                       sal_Int32    nEndIndex ) const
        {
            SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::ImplRenderer::drawSubset()" );

            ActionVector::const_iterator aRangeBegin;
            ActionVector::const_iterator aRangeEnd;

            try
            {
                if( !getSubsetIndices( nStartIndex, nEndIndex,
                                       aRangeBegin, aRangeEnd ) )
                    return true; // nothing to render (but _that_ was successful)

                // now, aRangeBegin references the action in which the
                // subset rendering must start, and aRangeEnd references
                // the action in which the subset rendering must end (it
                // might also end right at the start of the referenced
                // action, such that zero of that action needs to be
                // rendered).


                // render subset of actions
                // ========================

                ::basegfx::B2DHomMatrix aMatrix = ::canvas::tools::getRenderStateTransform( getRenderState() );

                ActionRenderer aRenderer( aMatrix );

                return forSubsetRange( aRenderer,
                                       aRangeBegin,
                                       aRangeEnd,
                                       nStartIndex,
                                       nEndIndex,
                                       maActions.end() );
            }
            catch( uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("cppcanvas.emf");
                // convert error to return value
                return false;
            }
        }

        ::basegfx::B2DRange ImplRenderer::getSubsetArea( sal_Int32  nStartIndex,
                                                         sal_Int32  nEndIndex ) const
        {
            SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::ImplRenderer::getSubsetArea()" );

            ActionVector::const_iterator aRangeBegin;
            ActionVector::const_iterator aRangeEnd;

            if( !getSubsetIndices( nStartIndex, nEndIndex,
                                   aRangeBegin, aRangeEnd ) )
                return ::basegfx::B2DRange(); // nothing to render -> empty range

            // now, aRangeBegin references the action in which the
            // subset querying must start, and aRangeEnd references
            // the action in which the subset querying must end (it
            // might also end right at the start of the referenced
            // action, such that zero of that action needs to be
            // queried).


            // query bounds for subset of actions
            // ==================================

            ::basegfx::B2DHomMatrix aMatrix = ::canvas::tools::getRenderStateTransform(
                                                      getRenderState() );

            AreaQuery aQuery( aMatrix );
            forSubsetRange( aQuery,
                            aRangeBegin,
                            aRangeEnd,
                            nStartIndex,
                            nEndIndex,
                            maActions.end() );

            return aQuery.getBounds();
        }

        bool ImplRenderer::draw() const
        {
            SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::ImplRenderer::draw()" );

            ::basegfx::B2DHomMatrix aMatrix = ::canvas::tools::getRenderStateTransform(
                                                      getRenderState() );

            try
            {
                return std::for_each( maActions.begin(), maActions.end(), ActionRenderer( aMatrix ) ).result();
            }
            catch( uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION( "cppcanvas.emf");
                return false;
            }
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
