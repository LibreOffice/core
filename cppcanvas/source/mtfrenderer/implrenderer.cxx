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

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppcanvas/canvas.hxx>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/rendering/PanoseProportion.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/gradienttools.hxx>
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
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <canvas/canvastools.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/metric.hxx>
#include <vcl/graphictools.hxx>
#include <tools/poly.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <implrenderer.hxx>
#include <tools.hxx>
#include <outdevstate.hxx>
#include <action.hxx>
#include <bitmapaction.hxx>
#include <lineaction.hxx>
#include <pointaction.hxx>
#include <polypolyaction.hxx>
#include <textaction.hxx>
#include <transparencygroupaction.hxx>
#include <vector>
#include <algorithm>
#include <iterator>
#include <boost/scoped_array.hpp>
#include "mtftools.hxx"
#include "outdevstate.hxx"
#include <basegfx/matrix/b2dhommatrixtools.hxx>

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
        // set rIsColorSet and check for true at the same time
        if( (rIsColorSet=pAct->IsSetting()) != false )
        {
            ::Color aColor( pAct->GetColor() );

            // force alpha part of color to
            // opaque. transparent painting is done
            // explicitly via META_TRANSPARENT_ACTION
            aColor.SetTransparency(0);
            //aColor.SetTransparency(128);

            rColorSequence = ::vcl::unotools::colorToDoubleSequence(
                aColor,
                rCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() );
        }
    }

    void setupStrokeAttributes( rendering::StrokeAttributes&                          o_rStrokeAttributes,
                                const ::cppcanvas::internal::ActionFactoryParameters& rParms,
                                const LineInfo&                                       rLineInfo                 )
    {
        const ::basegfx::B2DSize aWidth( rLineInfo.GetWidth(), 0 );
        o_rStrokeAttributes.StrokeWidth =
            (rParms.mrStates.getState().mapModeTransform * aWidth).getX();

        // setup reasonable defaults
        o_rStrokeAttributes.MiterLimit   = 15.0; // 1.0 was no good default; GDI+'s limit is 10.0, our's is 15.0
        o_rStrokeAttributes.StartCapType = rendering::PathCapType::BUTT;
        o_rStrokeAttributes.EndCapType   = rendering::PathCapType::BUTT;

        switch(rLineInfo.GetLineJoin())
        {
            default: // B2DLINEJOIN_NONE, B2DLINEJOIN_MIDDLE
                o_rStrokeAttributes.JoinType = rendering::PathJoinType::NONE;
                break;
            case basegfx::B2DLINEJOIN_BEVEL:
                o_rStrokeAttributes.JoinType = rendering::PathJoinType::BEVEL;
                break;
            case basegfx::B2DLINEJOIN_MITER:
                o_rStrokeAttributes.JoinType = rendering::PathJoinType::MITER;
                break;
            case basegfx::B2DLINEJOIN_ROUND:
                o_rStrokeAttributes.JoinType = rendering::PathJoinType::ROUND;
                break;
        }

        switch(rLineInfo.GetLineCap())
        {
            default: /* com::sun::star::drawing::LineCap_BUTT */
            {
                o_rStrokeAttributes.StartCapType = rendering::PathCapType::BUTT;
                o_rStrokeAttributes.EndCapType   = rendering::PathCapType::BUTT;
                break;
            }
            case com::sun::star::drawing::LineCap_ROUND:
            {
                o_rStrokeAttributes.StartCapType = rendering::PathCapType::ROUND;
                o_rStrokeAttributes.EndCapType   = rendering::PathCapType::ROUND;
                break;
            }
            case com::sun::star::drawing::LineCap_SQUARE:
            {
                o_rStrokeAttributes.StartCapType = rendering::PathCapType::SQUARE;
                o_rStrokeAttributes.EndCapType   = rendering::PathCapType::SQUARE;
                break;
            }
        }

        if( LINE_DASH == rLineInfo.GetStyle() )
        {
            const ::cppcanvas::internal::OutDevState& rState( rParms.mrStates.getState() );

            // TODO(F1): Interpret OutDev::GetRefPoint() for the start of the dashing.

            // interpret dash info only if explicitly enabled as
            // style
            const ::basegfx::B2DSize aDistance( rLineInfo.GetDistance(), 0 );
            const double nDistance( (rState.mapModeTransform * aDistance).getX() );

            const ::basegfx::B2DSize aDashLen( rLineInfo.GetDashLen(), 0 );
            const double nDashLen( (rState.mapModeTransform * aDashLen).getX() );

            const ::basegfx::B2DSize aDotLen( rLineInfo.GetDotLen(), 0 );
            const double nDotLen( (rState.mapModeTransform * aDotLen).getX() );

            const sal_Int32 nNumArryEntries( 2*rLineInfo.GetDashCount() +
                                             2*rLineInfo.GetDotCount() );

            o_rStrokeAttributes.DashArray.realloc( nNumArryEntries );
            double* pDashArray = o_rStrokeAttributes.DashArray.getArray();


            // iteratively fill dash array, first with dashs, then
            // with dots.
            // ===================================================

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
    }


    /** Create masked BitmapEx, where the white areas of rBitmap are
        transparent, and the other appear in rMaskColor.
     */
    BitmapEx createMaskBmpEx( const Bitmap&  rBitmap,
                              const ::Color& rMaskColor )
    {
        const ::Color aWhite( COL_WHITE );
        BitmapPalette aBiLevelPalette(2);
        aBiLevelPalette[0] = aWhite;
        aBiLevelPalette[1] = rMaskColor;

        Bitmap aMask( rBitmap.CreateMask( aWhite ));
        Bitmap aSolid( rBitmap.GetSizePixel(),
                       1,
                       &aBiLevelPalette );
        aSolid.Erase( rMaskColor );

        return BitmapEx( aSolid, aMask );
    }

    OUString convertToLocalizedNumerals(const OUString& rStr,
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

namespace cppcanvas
{
    namespace internal
    {
        // state stack manipulators
        // ------------------------
        void VectorOfOutDevStates::clearStateStack()
        {
            m_aStates.clear();
            const OutDevState aDefaultState;
            m_aStates.push_back(aDefaultState);
        }

        OutDevState& VectorOfOutDevStates::getState()
        {
            return m_aStates.back();
        }

        const OutDevState& VectorOfOutDevStates::getState() const
        {
            return m_aStates.back();
        }

        void VectorOfOutDevStates::pushState(sal_uInt16 nFlags)
        {
            m_aStates.push_back( getState() );
            getState().pushFlags = nFlags;
        }

        void VectorOfOutDevStates::popState()
        {
            if( getState().pushFlags != PUSH_ALL )
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

                if( (aCalculatedNewState.pushFlags & PUSH_LINECOLOR) )
                {
                    aCalculatedNewState.lineColor      = rNewState.lineColor;
                    aCalculatedNewState.isLineColorSet = rNewState.isLineColorSet;
                }

                if( (aCalculatedNewState.pushFlags & PUSH_FILLCOLOR) )
                {
                    aCalculatedNewState.fillColor      = rNewState.fillColor;
                    aCalculatedNewState.isFillColorSet = rNewState.isFillColorSet;
                }

                if( (aCalculatedNewState.pushFlags & PUSH_FONT) )
                {
                    aCalculatedNewState.xFont                   = rNewState.xFont;
                    aCalculatedNewState.fontRotation            = rNewState.fontRotation;
                    aCalculatedNewState.textReliefStyle         = rNewState.textReliefStyle;
                    aCalculatedNewState.textOverlineStyle       = rNewState.textOverlineStyle;
                    aCalculatedNewState.textUnderlineStyle      = rNewState.textUnderlineStyle;
                    aCalculatedNewState.textStrikeoutStyle      = rNewState.textStrikeoutStyle;
                    aCalculatedNewState.textEmphasisMarkStyle   = rNewState.textEmphasisMarkStyle;
                    aCalculatedNewState.isTextEffectShadowSet   = rNewState.isTextEffectShadowSet;
                    aCalculatedNewState.isTextWordUnderlineSet  = rNewState.isTextWordUnderlineSet;
                    aCalculatedNewState.isTextOutlineModeSet    = rNewState.isTextOutlineModeSet;
                }

                if( (aCalculatedNewState.pushFlags & PUSH_TEXTCOLOR) )
                {
                    aCalculatedNewState.textColor = rNewState.textColor;
                }

                if( (aCalculatedNewState.pushFlags & PUSH_MAPMODE) )
                {
                    aCalculatedNewState.mapModeTransform = rNewState.mapModeTransform;
                }

                if( (aCalculatedNewState.pushFlags & PUSH_CLIPREGION) )
                {
                    aCalculatedNewState.clip        = rNewState.clip;
                    aCalculatedNewState.clipRect    = rNewState.clipRect;
                    aCalculatedNewState.xClipPoly   = rNewState.xClipPoly;
                }

                // TODO(F2): Raster ops NYI
                // if( (aCalculatedNewState.pushFlags & PUSH_RASTEROP) )
                // {
                // }

                if( (aCalculatedNewState.pushFlags & PUSH_TEXTFILLCOLOR) )
                {
                    aCalculatedNewState.textFillColor      = rNewState.textFillColor;
                    aCalculatedNewState.isTextFillColorSet = rNewState.isTextFillColorSet;
                }

                if( (aCalculatedNewState.pushFlags & PUSH_TEXTALIGN) )
                {
                    aCalculatedNewState.textReferencePoint = rNewState.textReferencePoint;
                }

                // TODO(F1): Refpoint handling NYI
                // if( (aCalculatedNewState.pushFlags & PUSH_REFPOINT) )
                // {
                // }

                if( (aCalculatedNewState.pushFlags & PUSH_TEXTLINECOLOR) )
                {
                    aCalculatedNewState.textLineColor      = rNewState.textLineColor;
                    aCalculatedNewState.isTextLineColorSet = rNewState.isTextLineColorSet;
                }

                if( (aCalculatedNewState.pushFlags & PUSH_TEXTLAYOUTMODE) )
                {
                    aCalculatedNewState.textAlignment = rNewState.textAlignment;
                    aCalculatedNewState.textDirection = rNewState.textDirection;
                }

                // TODO(F2): Text language handling NYI
                // if( (aCalculatedNewState.pushFlags & PUSH_TEXTLANGUAGE) )
                // {
                // }

                // always copy push mode
                aCalculatedNewState.pushFlags = rNewState.pushFlags;

                // flush to stack
                getState() = aCalculatedNewState;
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
                (rState.lineColor.getLength() == 0 &&
                 rState.fillColor.getLength() == 0) )
            {
                return false;
            }

            ActionSharedPtr pPolyAction(
                internal::PolyPolyActionFactory::createPolyPolyAction(
                    rPolyPoly, rParms.mrCanvas, rState ) );

            if( pPolyAction )
            {
                maActions.push_back(
                    MtfAction(
                        pPolyAction,
                        rParms.mrCurrActionIndex ) );

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
                                        sal_Int32&   io_rCurrActionIndex ) const
        {
            ENSURE_OR_THROW( pCommentString,
                              "ImplRenderer::skipContent(): NULL string given" );

            MetaAction* pCurrAct;
            while( (pCurrAct=rMtf.NextAction()) != NULL )
            {
                // increment action index, we've skipped an action.
                ++io_rCurrActionIndex;

                if( pCurrAct->GetType() == META_COMMENT_ACTION &&
                    static_cast<MetaCommentAction*>(pCurrAct)->GetComment().equalsIgnoreAsciiCase(
                        pCommentString) )
                {
                    // requested comment found, done
                    return;
                }
            }

            // EOF
            return;
        }

        bool ImplRenderer::isActionContained( GDIMetaFile& rMtf,
                                              const char*  pCommentString,
                                              sal_uInt16       nType ) const
        {
            ENSURE_OR_THROW( pCommentString,
                              "ImplRenderer::isActionContained(): NULL string given" );

            bool bRet( false );

            // at least _one_ call to GDIMetaFile::NextAction() is
            // executed
            sal_uIntPtr nPos( 1 );

            MetaAction* pCurrAct;
            while( (pCurrAct=rMtf.NextAction()) != NULL )
            {
                if( pCurrAct->GetType() == nType )
                {
                    bRet = true; // action type found
                    break;
                }

                if( pCurrAct->GetType() == META_COMMENT_ACTION &&
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
            while( nPos-- )
                rMtf.WindPrev();

            if( !pCurrAct )
            {
                // EOF, and not yet found
                bRet = false;
            }

            return bRet;
        }

        void ImplRenderer::createGradientAction( const ::PolyPolygon&           rPoly,
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
                    // ----------------------------

                    // scale color coefficients with gradient intensities
                    const sal_uInt16 nStartIntensity( rGradient.GetStartIntensity() );
                    ::Color aVCLStartColor( rGradient.GetStartColor() );
                    aVCLStartColor.SetRed( (sal_uInt8)(aVCLStartColor.GetRed() * nStartIntensity / 100) );
                    aVCLStartColor.SetGreen( (sal_uInt8)(aVCLStartColor.GetGreen() * nStartIntensity / 100) );
                    aVCLStartColor.SetBlue( (sal_uInt8)(aVCLStartColor.GetBlue() * nStartIntensity / 100) );

                    const sal_uInt16 nEndIntensity( rGradient.GetEndIntensity() );
                    ::Color aVCLEndColor( rGradient.GetEndColor() );
                    aVCLEndColor.SetRed( (sal_uInt8)(aVCLEndColor.GetRed() * nEndIntensity / 100) );
                    aVCLEndColor.SetGreen( (sal_uInt8)(aVCLEndColor.GetGreen() * nEndIntensity / 100) );
                    aVCLEndColor.SetBlue( (sal_uInt8)(aVCLEndColor.GetBlue() * nEndIntensity / 100) );

                    uno::Reference<rendering::XColorSpace> xColorSpace(
                        rParms.mrCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace());
                    const uno::Sequence< double > aStartColor(
                        ::vcl::unotools::colorToDoubleSequence( aVCLStartColor,
                                                                xColorSpace ));
                    const uno::Sequence< double > aEndColor(
                        ::vcl::unotools::colorToDoubleSequence( aVCLEndColor,
                                                                xColorSpace ));

                    uno::Sequence< uno::Sequence < double > > aColors(2);
                    uno::Sequence< double > aStops(2);

                    if( rGradient.GetStyle() == GradientStyle_AXIAL )
                    {
                        aStops.realloc(3);
                        aColors.realloc(3);

                        aStops[0] = 0.0;
                        aStops[1] = 0.5;
                        aStops[2] = 1.0;

                        aColors[0] = aEndColor;
                        aColors[1] = aStartColor;
                        aColors[2] = aEndColor;
                    }
                    else
                    {
                        aStops[0] = 0.0;
                        aStops[1] = 1.0;

                        aColors[0] = aStartColor;
                        aColors[1] = aEndColor;
                    }

                    const ::basegfx::B2DRectangle aBounds(
                        ::basegfx::tools::getRange(aDevicePoly) );
                    const ::basegfx::B2DVector aOffset(
                        rGradient.GetOfsX() / 100.0,
                        rGradient.GetOfsY() / 100.0);
                    double fRotation( rGradient.GetAngle() * M_PI / 1800.0 );
                    const double fBorder( rGradient.GetBorder() / 100.0 );

                    basegfx::B2DHomMatrix aRot90;
                    aRot90.rotate(M_PI_2);

                    basegfx::ODFGradientInfo aGradInfo;
                    OUString aGradientService;
                    switch( rGradient.GetStyle() )
                    {
                        case GradientStyle_LINEAR:
                            aGradInfo = basegfx::tools::createLinearODFGradientInfo(
                                                                        aBounds,
                                                                        nSteps,
                                                                        fBorder,
                                                                        fRotation);
                            // map ODF to svg gradient orientation - x
                            // instead of y direction
                            aGradInfo.setTextureTransform(aGradInfo.getTextureTransform() * aRot90);
                            aGradientService = "LinearGradient";
                            break;

                        case GradientStyle_AXIAL:
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
                            aGradInfo = basegfx::tools::createAxialODFGradientInfo(
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

                        case GradientStyle_RADIAL:
                            aGradInfo = basegfx::tools::createRadialODFGradientInfo(
                                                                        aBounds,
                                                                        aOffset,
                                                                        nSteps,
                                                                        fBorder);
                            aGradientService = "EllipticalGradient";
                            break;

                        case GradientStyle_ELLIPTICAL:
                            aGradInfo = basegfx::tools::createEllipticalODFGradientInfo(
                                                                            aBounds,
                                                                            aOffset,
                                                                            nSteps,
                                                                            fBorder,
                                                                            fRotation);
                            aGradientService = "EllipticalGradient";
                            break;

                        case GradientStyle_SQUARE:
                            aGradInfo = basegfx::tools::createSquareODFGradientInfo(
                                                                        aBounds,
                                                                        aOffset,
                                                                        nSteps,
                                                                        fBorder,
                                                                        fRotation);
                            aGradientService = "RectangularGradient";
                            break;

                        case GradientStyle_RECT:
                            aGradInfo = basegfx::tools::createRectangularODFGradientInfo(
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

                    uno::Sequence<uno::Any> args(3);
                    beans::PropertyValue aProp;
                    aProp.Name = "Colors";
                    aProp.Value <<= aColors;
                    args[0] <<= aProp;
                    aProp.Name = "Stops";
                    aProp.Value <<= aStops;
                    args[1] <<= aProp;
                    aProp.Name = "AspectRatio";
                    aProp.Value <<= aGradInfo.getAspectRatio();
                    args[2] <<= aProp;

                    aTexture.Gradient.set(
                        xFactory->createInstanceWithArguments(aGradientService,
                                                              args),
                        uno::UNO_QUERY);
                    if( aTexture.Gradient.is() )
                    {
                        ActionSharedPtr pPolyAction(
                            internal::PolyPolyActionFactory::createPolyPolyAction(
                                aDevicePoly,
                                rParms.mrCanvas,
                                rParms.mrStates.getState(),
                                aTexture ) );

                        if( pPolyAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pPolyAction,
                                    rParms.mrCurrActionIndex ) );

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
            rParms.mrStates.pushState(PUSH_ALL);

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
            rParms.mrVDev.AddGradientActions( rPoly.GetBoundRect(),
                                              rGradient,
                                               aTmpMtf );

            createActions( aTmpMtf, rParms, bSubsettableActions );

            rParms.mrStates.popState();
        }

        uno::Reference< rendering::XCanvasFont > ImplRenderer::createFont( double&                        o_rFontRotation,
                                                                           const ::Font&                  rFont,
                                                                           const ActionFactoryParameters& rParms ) const
        {
            rendering::FontRequest aFontRequest;

            if( rParms.mrParms.maFontName.is_initialized() )
                aFontRequest.FontDescription.FamilyName = *rParms.mrParms.maFontName;
            else
                aFontRequest.FontDescription.FamilyName = rFont.GetName();

            aFontRequest.FontDescription.StyleName = rFont.GetStyleName();

            aFontRequest.FontDescription.IsSymbolFont = (rFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL) ? util::TriState_YES : util::TriState_NO;
            aFontRequest.FontDescription.IsVertical = rFont.IsVertical() ? util::TriState_YES : util::TriState_NO;

            // TODO(F2): improve vclenum->panose conversion
            aFontRequest.FontDescription.FontDescription.Weight =
                rParms.mrParms.maFontWeight.is_initialized() ?
                *rParms.mrParms.maFontWeight :
                ::canvas::tools::numeric_cast<sal_Int8>( ::basegfx::fround( rFont.GetWeight() ) );
            aFontRequest.FontDescription.FontDescription.Letterform =
                rParms.mrParms.maFontLetterForm.is_initialized() ?
                *rParms.mrParms.maFontLetterForm :
                (rFont.GetItalic() == ITALIC_NONE) ? 0 : 9;
            aFontRequest.FontDescription.FontDescription.Proportion =
                rParms.mrParms.maFontProportion.is_initialized() ?
                *rParms.mrParms.maFontProportion :
                (rFont.GetPitch() == PITCH_FIXED)
                    ? rendering::PanoseProportion::MONO_SPACED
                    : rendering::PanoseProportion::ANYTHING;

            LanguageType aLang = rFont.GetLanguage();
            aFontRequest.Locale = LanguageTag::convertToLocale( aLang, false);

            // setup state-local text transformation,
            // if the font be rotated
            const short nFontAngle( rFont.GetOrientation() );
            if( nFontAngle != 0 )
            {
                // set to unity transform rotated by font angle
                const double nAngle( nFontAngle * (F_PI / 1800.0) );
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
            const ::Size rFontSizeLog( rFont.GetSize() );
            const sal_Int32 nFontWidthLog = rFontSizeLog.Width();
            if( nFontWidthLog != 0 )
            {
                ::Font aTestFont = rFont;
                aTestFont.SetWidth( 0 );
                sal_Int32 nNormalWidth = rParms.mrVDev.GetFontMetric( aTestFont ).GetWidth();
                if( nNormalWidth != nFontWidthLog )
                    if( nNormalWidth )
                        aFontMatrix.m00 = (double)nFontWidthLog / nNormalWidth;
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
            aFontRequest.CellSize = (rState.mapModeTransform * ::vcl::unotools::b2DSizeFromSize(rFontSizeLog)).getY();

            return rParms.mrCanvas->getUNOCanvas()->createFont( aFontRequest,
                                                                uno::Sequence< beans::PropertyValue >(),
                                                                aFontMatrix );
        }

        // create text effects such as shadow/relief/embossed
        void ImplRenderer::createTextAction( const ::Point&                 rStartPoint,
                                             const OUString                 rString,
                                             int                            nIndex,
                                             int                            nLength,
                                             const sal_Int32*               pCharWidths,
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
                sal_Int32 nShadowOffset = static_cast<sal_Int32>(1.5 + ((rParms.mrVDev.GetFont().GetHeight()-24.0)/24.0));
                if( nShadowOffset < 1 )
                    nShadowOffset = 1;

                aShadowOffset.setWidth( nShadowOffset );
                aShadowOffset.setHeight( nShadowOffset );

                // determine shadow color (from outdev3.cxx)
                ::Color aTextColor = ::vcl::unotools::doubleSequenceToColor(
                    rState.textColor, xColorSpace );
                bool bIsDark = (aTextColor.GetColor() == COL_BLACK)
                    || (aTextColor.GetLuminance() < 8);

                aShadowColor = bIsDark ? COL_LIGHTGRAY : COL_BLACK;
                aShadowColor.SetTransparency( aTextColor.GetTransparency() );
            }

            if( rState.textReliefStyle )
            {
                // calculate relief offset (similar to outdev3.cxx)
                sal_Int32 nReliefOffset = rParms.mrVDev.PixelToLogic( Size( 1, 1 ) ).Height();
                nReliefOffset += nReliefOffset/2;
                if( nReliefOffset < 1 )
                    nReliefOffset = 1;

                if( rState.textReliefStyle == RELIEF_ENGRAVED )
                    nReliefOffset = -nReliefOffset;

                aReliefOffset.setWidth( nReliefOffset );
                aReliefOffset.setHeight( nReliefOffset );

                // determine relief color (from outdev3.cxx)
                ::Color aTextColor = ::vcl::unotools::doubleSequenceToColor(
                    rState.textColor, xColorSpace );

                aReliefColor = ::Color( COL_LIGHTGRAY );

                // we don't have a automatic color, so black is always
                // drawn on white (literally copied from
                // vcl/source/gdi/outdev3.cxx)
                if( aTextColor.GetColor() == COL_BLACK )
                {
                    aTextColor = ::Color( COL_WHITE );
                    rParms.mrStates.getState().textColor =
                        ::vcl::unotools::colorToDoubleSequence(
                            aTextColor, xColorSpace );
                }

                if( aTextColor.GetColor() == COL_WHITE )
                    aReliefColor = ::Color( COL_BLACK );
                aReliefColor.SetTransparency( aTextColor.GetTransparency() );
            }

            // create the actual text action
            ActionSharedPtr pTextAction(
                TextActionFactory::createTextAction(
                    rStartPoint,
                    aReliefOffset,
                    aReliefColor,
                    aShadowOffset,
                    aShadowColor,
                    rString,
                    nIndex,
                    nLength,
                    pCharWidths,
                    rParms.mrVDev,
                    rParms.mrCanvas,
                    rState,
                    rParms.mrParms,
                    bSubsettableActions ) );

            ActionSharedPtr pStrikeoutTextAction;

            if ( rState.textStrikeoutStyle == STRIKEOUT_X || rState.textStrikeoutStyle == STRIKEOUT_SLASH )
            {
                long nWidth = rParms.mrVDev.GetTextWidth( rString,nIndex,nLength );

                sal_Unicode pChars[4];
                if ( rState.textStrikeoutStyle == STRIKEOUT_X )
                    pChars[0] = 'X';
                else
                    pChars[0] = '/';
                pChars[3]=pChars[2]=pChars[1]=pChars[0];

                long nStrikeoutWidth = (rParms.mrVDev.GetTextWidth(
                    OUString(pChars, SAL_N_ELEMENTS(pChars))) + 2) / 4;

                if( nStrikeoutWidth <= 0 )
                    nStrikeoutWidth = 1;

                long nMaxWidth = nStrikeoutWidth/2;
                if ( nMaxWidth < 2 )
                    nMaxWidth = 2;
                nMaxWidth += nWidth + 1;

                long nFullStrikeoutWidth = 0;
                OUString aStrikeoutText;
                while( (nFullStrikeoutWidth+=nStrikeoutWidth ) < nMaxWidth+1 )
                    aStrikeoutText += OUString(pChars[0]);

                sal_Int32 nLen = aStrikeoutText.getLength();

                if( nLen )
                {
                    long nInterval = ( nWidth - nStrikeoutWidth * nLen ) / nLen;
                    nStrikeoutWidth += nInterval;
                    sal_Int32* pStrikeoutCharWidths = new sal_Int32[nLen];

                    for ( int i = 0;i<nLen; i++)
                    {
                        pStrikeoutCharWidths[i] = nStrikeoutWidth;
                    }

                    for ( int i = 1;i< nLen; i++ )
                    {
                        pStrikeoutCharWidths[ i ] += pStrikeoutCharWidths[ i-1 ];
                    }

                    sal_Int32 nStartPos = 0;

                    pStrikeoutTextAction =
                        TextActionFactory::createTextAction(
                            rStartPoint,
                            aReliefOffset,
                            aReliefColor,
                            aShadowOffset,
                            aShadowColor,
                            aStrikeoutText,
                            nStartPos,
                            aStrikeoutText.getLength(),
                            pStrikeoutCharWidths,
                            rParms.mrVDev,
                            rParms.mrCanvas,
                            rState,
                            rParms.mrParms,
                            bSubsettableActions ) ;
                }
            }

            if( pTextAction )
            {
                maActions.push_back(
                    MtfAction(
                        pTextAction,
                        rParms.mrCurrActionIndex ) );

                if ( pStrikeoutTextAction )
                {
                    maActions.push_back(
                        MtfAction(
                        pStrikeoutTextAction,
                        rParms.mrCurrActionIndex ) );
                }

                rParms.mrCurrActionIndex += pTextAction->getActionCount()-1;
            }
        }

        void ImplRenderer::updateClipping( const ::basegfx::B2DPolyPolygon& rClipPoly,
                                           const ActionFactoryParameters&   rParms,
                                           bool                             bIntersect )
        {
            ::cppcanvas::internal::OutDevState& rState( rParms.mrStates.getState() );
            ::basegfx::B2DPolyPolygon aClipPoly( rClipPoly );

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
                    rState.clip = ::basegfx::B2DPolyPolygon(
                        ::basegfx::tools::createPolygonFromRect(
                            // #121100# VCL rectangular clips always
                            // include one more pixel to the right
                            // and the bottom
                            ::basegfx::B2DRectangle( rState.clipRect.Left(),
                                                     rState.clipRect.Top(),
                                                     rState.clipRect.Right()+1,
                                                     rState.clipRect.Bottom()+1 ) ) );
                }

                // AW: Simplified
                rState.clip = basegfx::tools::clipPolyPolygonOnPolyPolygon(
                    aClipPoly, rState.clip, true, false);
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
                    rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rParms.mrCanvas->getUNOCanvas()->getDevice(),
                        ::basegfx::B2DPolyPolygon(
                            ::basegfx::tools::createPolygonFromRect(
                                // #121100# VCL rectangular clips
                                // always include one more pixel to
                                // the right and the bottom
                                ::basegfx::B2DRectangle( rState.clipRect.Left(),
                                                         rState.clipRect.Top(),
                                                         rState.clipRect.Right()+1,
                                                         rState.clipRect.Bottom()+1 ) ) ) );
                }
            }
            else
            {
                rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    rParms.mrCanvas->getUNOCanvas()->getDevice(),
                    rState.clip );
            }
        }

        void ImplRenderer::updateClipping( const ::Rectangle&             rClipRect,
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
                // -----------------------------------------

                // convert rect to polygon beforehand, must revert
                // to general polygon clipping here.
                ::basegfx::B2DPolyPolygon aClipPoly(
                    ::basegfx::tools::createPolygonFromRect(
                        ::basegfx::B2DRectangle( rClipRect.Left(),
                                                 rClipRect.Top(),
                                                 rClipRect.Right(),
                                                 rClipRect.Bottom() ) ) );

                rState.clipRect.SetEmpty();

                // AW: Simplified
                rState.clip = basegfx::tools::clipPolyPolygonOnPolyPolygon(
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
                    rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                        rParms.mrCanvas->getUNOCanvas()->getDevice(),
                        ::basegfx::B2DPolyPolygon(
                            ::basegfx::tools::createPolygonFromRect(
                                // #121100# VCL rectangular clips
                                // always include one more pixel to
                                // the right and the bottom
                                ::basegfx::B2DRectangle( rState.clipRect.Left(),
                                                         rState.clipRect.Top(),
                                                         rState.clipRect.Right()+1,
                                                         rState.clipRect.Bottom()+1 ) ) ) );
                }
            }
            else
            {
                rState.xClipPoly = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(
                    rParms.mrCanvas->getUNOCanvas()->getDevice(),
                    rState.clip );
            }
        }

        bool ImplRenderer::createActions( GDIMetaFile&                   rMtf,
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
                // - the line/fill color when processing a META_TRANSPARENT_ACTION
                // - SetFont to process font metric specific actions
                pCurrAct->Execute( &rVDev );

                SAL_INFO("cppcanvas.emf", "MTF\trecord type: 0x" << pCurrAct->GetType() << " (" << pCurrAct->GetType() << ")");

                switch( pCurrAct->GetType() )
                {
                    // ------------------------------------------------------------

                    // In the first part of this monster-switch, we
                    // handle all state-changing meta actions. These
                    // are all handled locally.

                    // ------------------------------------------------------------

                    case META_PUSH_ACTION:
                    {
                        MetaPushAction* pPushAction = static_cast<MetaPushAction*>(pCurrAct);
                        rStates.pushState(pPushAction->GetFlags());
                    }
                    break;

                    case META_POP_ACTION:
                        rStates.popState();
                        break;

                    case META_TEXTLANGUAGE_ACTION:
                        // FALLTHROUGH intended
                    case META_REFPOINT_ACTION:
                        // handled via pCurrAct->Execute( &rVDev )
                        break;

                    case META_MAPMODE_ACTION:
                        // modify current mapModeTransformation
                        // transformation, such that subsequent
                        // coordinates map correctly
                        tools::calcLogic2PixelAffineTransform( rStates.getState().mapModeTransform,
                                                               rVDev );
                        break;

                    // monitor clip regions, to assemble clip polygon on our own
                    case META_CLIPREGION_ACTION:
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
                                VERBOSE_TRACE( "ImplRenderer::createActions(): non-polygonal clip "
                                               "region encountered, falling back to bounding box!" );

                                // #121806# explicitly kept integer
                                Rectangle aClipRect(
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

                    case META_ISECTRECTCLIPREGION_ACTION:
                    {
                        MetaISectRectClipRegionAction* pClipAction = static_cast<MetaISectRectClipRegionAction*>(pCurrAct);

                        // #121806# explicitly kept integer
                        Rectangle aClipRect(
                            rVDev.LogicToPixel( pClipAction->GetRect() ) );

                        // intersect current clip with given rect
                        updateClipping(
                            aClipRect,
                            rFactoryParms,
                            true );

                        break;
                    }

                    case META_ISECTREGIONCLIPREGION_ACTION:
                    {
                        MetaISectRegionClipRegionAction* pClipAction = static_cast<MetaISectRegionClipRegionAction*>(pCurrAct);

                        if( !pClipAction->GetRegion().HasPolyPolygonOrB2DPolyPolygon() )
                        {
                            VERBOSE_TRACE( "ImplRenderer::createActions(): non-polygonal clip "
                                           "region encountered, falling back to bounding box!" );

                            // #121806# explicitly kept integer
                            Rectangle aClipRect(
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

                    case META_MOVECLIPREGION_ACTION:
                        // TODO(F2): NYI
                        break;

                    case META_LINECOLOR_ACTION:
                        if( !rParms.maLineColor.is_initialized() )
                        {
                            setStateColor( static_cast<MetaLineColorAction*>(pCurrAct),
                                           rStates.getState().isLineColorSet,
                                           rStates.getState().lineColor,
                                           rCanvas );
                        }
                        else
                        {
                            // #120994# Do switch on/off LineColor, even when a overriding one is set
                            bool bSetting(static_cast<MetaLineColorAction*>(pCurrAct)->IsSetting());

                            rStates.getState().isLineColorSet = bSetting;
                        }
                        break;

                    case META_FILLCOLOR_ACTION:
                        if( !rParms.maFillColor.is_initialized() )
                        {
                            setStateColor( static_cast<MetaFillColorAction*>(pCurrAct),
                                           rStates.getState().isFillColorSet,
                                           rStates.getState().fillColor,
                                           rCanvas );
                        }
                        else
                        {
                            // #120994# Do switch on/off FillColor, even when a overriding one is set
                            bool bSetting(static_cast<MetaFillColorAction*>(pCurrAct)->IsSetting());

                            rStates.getState().isFillColorSet = bSetting;
                        }
                        break;

                    case META_TEXTCOLOR_ACTION:
                    {
                        if( !rParms.maTextColor.is_initialized() )
                        {
                            // Text color is set unconditionally, thus, no
                            // use of setStateColor here
                            ::Color aColor( static_cast<MetaTextColorAction*>(pCurrAct)->GetColor() );

                            // force alpha part of color to
                            // opaque. transparent painting is done
                            // explicitly via META_TRANSPARENT_ACTION
                            aColor.SetTransparency(0);

                            rStates.getState().textColor =
                                ::vcl::unotools::colorToDoubleSequence(
                                    aColor,
                                    rCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() );
                        }
                    }
                    break;

                    case META_TEXTFILLCOLOR_ACTION:
                        if( !rParms.maTextColor.is_initialized() )
                        {
                            setStateColor( static_cast<MetaTextFillColorAction*>(pCurrAct),
                                           rStates.getState().isTextFillColorSet,
                                           rStates.getState().textFillColor,
                                           rCanvas );
                        }
                        else
                        {
                            // #120994# Do switch on/off TextFillColor, even when a overriding one is set
                            bool bSetting(static_cast<MetaTextFillColorAction*>(pCurrAct)->IsSetting());

                            rStates.getState().isTextFillColorSet = bSetting;
                        }
                        break;

                    case META_TEXTLINECOLOR_ACTION:
                        if( !rParms.maTextColor.is_initialized() )
                        {
                            setStateColor( static_cast<MetaTextLineColorAction*>(pCurrAct),
                                           rStates.getState().isTextLineColorSet,
                                           rStates.getState().textLineColor,
                                           rCanvas );
                        }
                        else
                        {
                            // #120994# Do switch on/off TextLineColor, even when a overriding one is set
                            bool bSetting(static_cast<MetaTextLineColorAction*>(pCurrAct)->IsSetting());

                            rStates.getState().isTextLineColorSet = bSetting;
                        }
                        break;

                    case META_TEXTALIGN_ACTION:
                    {
                        ::cppcanvas::internal::OutDevState& rState = rStates.getState();
                        const TextAlign eTextAlign( static_cast<MetaTextAlignAction*>(pCurrAct)->GetTextAlign() );

                        rState.textReferencePoint = eTextAlign;
                    }
                    break;

                    case META_FONT_ACTION:
                    {
                        ::cppcanvas::internal::OutDevState& rState = rStates.getState();
                        const ::Font& rFont( static_cast<MetaFontAction*>(pCurrAct)->GetFont() );

                        rState.xFont = createFont( rState.fontRotation,
                                                   rFont,
                                                   rFactoryParms );

                        // TODO(Q2): define and use appropriate enumeration types
                        rState.textReliefStyle          = (sal_Int8)rFont.GetRelief();
                        rState.textOverlineStyle        = (sal_Int8)rFont.GetOverline();
                        rState.textUnderlineStyle       = rParms.maFontUnderline.is_initialized() ?
                            (*rParms.maFontUnderline ? (sal_Int8)UNDERLINE_SINGLE : (sal_Int8)UNDERLINE_NONE) :
                            (sal_Int8)rFont.GetUnderline();
                        rState.textStrikeoutStyle       = (sal_Int8)rFont.GetStrikeout();
                        rState.textEmphasisMarkStyle    = (sal_Int8)rFont.GetEmphasisMark();
                        rState.isTextEffectShadowSet    = (rFont.IsShadow() != sal_False);
                        rState.isTextWordUnderlineSet   = (rFont.IsWordLineMode() != sal_False);
                        rState.isTextOutlineModeSet     = (rFont.IsOutline() != sal_False);
                    }
                    break;

                    case META_RASTEROP_ACTION:
                        // TODO(F2): NYI
                        break;

                    case META_LAYOUTMODE_ACTION:
                    {
                        // TODO(F2): A lot is missing here
                        int nLayoutMode = static_cast<MetaLayoutModeAction*>(pCurrAct)->GetLayoutMode();
                        ::cppcanvas::internal::OutDevState& rState = rStates.getState();
                        switch( nLayoutMode & (TEXT_LAYOUT_BIDI_RTL|TEXT_LAYOUT_BIDI_STRONG) )
                        {
                            case TEXT_LAYOUT_BIDI_LTR:
                                rState.textDirection = rendering::TextDirection::WEAK_LEFT_TO_RIGHT;
                                break;

                            case (TEXT_LAYOUT_BIDI_LTR | TEXT_LAYOUT_BIDI_STRONG):
                                rState.textDirection = rendering::TextDirection::STRONG_LEFT_TO_RIGHT;
                                break;

                            case TEXT_LAYOUT_BIDI_RTL:
                                rState.textDirection = rendering::TextDirection::WEAK_RIGHT_TO_LEFT;
                                break;

                            case (TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_BIDI_STRONG):
                                rState.textDirection = rendering::TextDirection::STRONG_RIGHT_TO_LEFT;
                                break;
                        }

                        rState.textAlignment = 0; // TODO(F2): rendering::TextAlignment::LEFT_ALIGNED;
                        if( (nLayoutMode & (TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_TEXTORIGIN_RIGHT) )
                            && !(nLayoutMode & TEXT_LAYOUT_TEXTORIGIN_LEFT ) )
                        {
                            rState.textAlignment = 1; // TODO(F2): rendering::TextAlignment::RIGHT_ALIGNED;
                        }
                    }
                    break;

                    // ------------------------------------------------------------

                    // In the second part of this monster-switch, we
                    // handle all recursing meta actions. These are the
                    // ones generating a metafile by themselves, which is
                    // then processed by recursively calling this method.

                    // ------------------------------------------------------------

                    case META_GRADIENT_ACTION:
                    {
                        MetaGradientAction* pGradAct = static_cast<MetaGradientAction*>(pCurrAct);
                        createGradientAction( ::Polygon( pGradAct->GetRect() ),
                                              pGradAct->GetGradient(),
                                              rFactoryParms,
                                              true,
                                              bSubsettableActions );
                    }
                    break;

                    case META_HATCH_ACTION:
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

                    case META_EPS_ACTION:
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
                        const Size aMtfSizePix( ::std::max( aMtfSizePixPre.Width(), 1L ),
                                                ::std::max( aMtfSizePixPre.Height(), 1L ) );

                        // Setup local transform, such that the
                        // metafile renders itself into the given
                        // output rectangle
                        rStates.pushState(PUSH_ALL);

                        rVDev.Push();
                        rVDev.SetMapMode( rSubstitute.GetPrefMapMode() );

                        const ::Point& rPos( rVDev.LogicToPixel( pAct->GetPoint() ) );
                        const ::Size&  rSize( rVDev.LogicToPixel( pAct->GetSize() ) );

                        rStates.getState().transform.translate( rPos.X(),
                                                                 rPos.Y() );
                        rStates.getState().transform.scale( (double)rSize.Width() / aMtfSizePix.Width(),
                                                             (double)rSize.Height() / aMtfSizePix.Height() );

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
                    case META_COMMENT_ACTION:
                    {
                        MetaCommentAction* pAct = static_cast<MetaCommentAction*>(pCurrAct);

                        // Handle gradients
                        if (pAct->GetComment().equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XGRAD_SEQ_BEGIN")))
                        {
                            MetaGradientExAction* pGradAction = NULL;
                            bool bDone( false );
                            while( !bDone &&
                                   (pCurrAct=rMtf.NextAction()) != NULL )
                            {
                                switch( pCurrAct->GetType() )
                                {
                                    // extract gradient info
                                    case META_GRADIENTEX_ACTION:
                                        pGradAction = static_cast<MetaGradientExAction*>(pCurrAct);
                                        break;

                                    // skip broken-down rendering, output gradient when sequence is ended
                                    case META_COMMENT_ACTION:
                                        if( static_cast<MetaCommentAction*>(pCurrAct)->GetComment().equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("XGRAD_SEQ_END")) )
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
                                }
                            }
                        }
                        // TODO(P2): Handle drawing layer strokes, via
                        // XPATHSTROKE_SEQ_BEGIN comment

                        // Handle drawing layer fills
                        else if( pAct->GetComment().equalsL(RTL_CONSTASCII_STRINGPARAM("XPATHFILL_SEQ_BEGIN")) )
                        {
                            const sal_uInt8* pData = pAct->GetData();
                            if ( pData )
                            {
                                SvMemoryStream  aMemStm( (void*)pData, pAct->GetDataSize(), STREAM_READ );

                                SvtGraphicFill aFill;
                                aMemStm >> aFill;

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
                                                        META_FLOATTRANSPARENT_ACTION ) )
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
                                    aTexture.Bitmap =
                                        ::vcl::unotools::xBitmapFromBitmapEx(
                                            rCanvas->getUNOCanvas()->getDevice(),
                                            aBmpEx );
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

                                    ::PolyPolygon aPath;
                                    aFill.getPath( aPath );

                                    ::basegfx::B2DPolyPolygon aPoly( aPath.getB2DPolyPolygon() );
                                    aPoly.transform( rStates.getState().mapModeTransform );
                                    ActionSharedPtr pPolyAction(
                                        internal::PolyPolyActionFactory::createPolyPolyAction(
                                            aPoly,
                                            rCanvas,
                                            rStates.getState(),
                                            aTexture ) );

                                    if( pPolyAction )
                                    {
                                        maActions.push_back(
                                            MtfAction(
                                                pPolyAction,
                                                io_rCurrActionIndex ) );

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
                        else if( pAct->GetComment().equalsL(RTL_CONSTASCII_STRINGPARAM("EMF_PLUS")) ) {
                            static int count = -1, limit = 0x7fffffff;
                            if (count == -1) {
                                count = 0;
                                if (char *env = getenv ("EMF_PLUS_LIMIT")) {
                                    limit = atoi (env);
                                    SAL_INFO ("cppcanvas.emf", "EMF+ records limit: " << limit);
                                }
                            }
                            SAL_INFO ("cppcanvas.emf", "EMF+ passed to canvas mtf renderer, size: " << pAct->GetDataSize ());
                            if (count < limit)
                                processEMFPlus( pAct, rFactoryParms, rStates.getState(), rCanvas );
                            count ++;
                        } else if( pAct->GetComment().equalsL(RTL_CONSTASCII_STRINGPARAM("EMF_PLUS_HEADER_INFO")) ) {
                            SAL_INFO ("cppcanvas.emf", "EMF+ passed to canvas mtf renderer - header info, size: " << pAct->GetDataSize ());

                            SvMemoryStream rMF ((void*) pAct->GetData (), pAct->GetDataSize (), STREAM_READ);

                            rMF >> nFrameLeft >> nFrameTop >> nFrameRight >> nFrameBottom;
                            SAL_INFO ("cppcanvas.emf", "EMF+ picture frame: " << nFrameLeft << "," << nFrameTop << " - " << nFrameRight << "," << nFrameBottom);
                            rMF >> nPixX >> nPixY >> nMmX >> nMmY;
                            SAL_INFO ("cppcanvas.emf", "EMF+ ref device pixel size: " << nPixX << "x" << nPixY << " mm size: " << nMmX << "x" << nMmY);

                            rMF >> aBaseTransform;
                            //aWorldTransform.Set (aBaseTransform);
                        }
                    }
                    break;

                    // ------------------------------------------------------------

                    // In the third part of this monster-switch, we
                    // handle all 'acting' meta actions. These are all
                    // processed by constructing function objects for
                    // them, which will later ease caching.

                    // ------------------------------------------------------------

                    case META_POINT_ACTION:
                    {
                        const OutDevState& rState( rStates.getState() );
                        if( rState.lineColor.getLength() )
                        {
                            ActionSharedPtr pPointAction(
                                internal::PointActionFactory::createPointAction(
                                    rState.mapModeTransform * ::vcl::unotools::b2DPointFromPoint(
                                        static_cast<MetaPointAction*>(pCurrAct)->GetPoint() ),
                                    rCanvas,
                                    rState ) );

                            if( pPointAction )
                            {
                                maActions.push_back(
                                    MtfAction(
                                        pPointAction,
                                        io_rCurrActionIndex ) );

                                io_rCurrActionIndex += pPointAction->getActionCount()-1;
                            }
                        }
                    }
                    break;

                    case META_PIXEL_ACTION:
                    {
                        const OutDevState& rState( rStates.getState() );
                        if( rState.lineColor.getLength() )
                        {
                            ActionSharedPtr pPointAction(
                                internal::PointActionFactory::createPointAction(
                                    rState.mapModeTransform * ::vcl::unotools::b2DPointFromPoint(
                                        static_cast<MetaPixelAction*>(pCurrAct)->GetPoint() ),
                                    rCanvas,
                                    rState,
                                    static_cast<MetaPixelAction*>(pCurrAct)->GetColor() ) );

                            if( pPointAction )
                            {
                                maActions.push_back(
                                    MtfAction(
                                        pPointAction,
                                        io_rCurrActionIndex ) );

                                io_rCurrActionIndex += pPointAction->getActionCount()-1;
                            }
                        }
                    }
                    break;

                    case META_LINE_ACTION:
                    {
                        const OutDevState& rState( rStates.getState() );
                        if( rState.lineColor.getLength() )
                        {
                            MetaLineAction* pLineAct = static_cast<MetaLineAction*>(pCurrAct);

                            const LineInfo& rLineInfo( pLineAct->GetLineInfo() );

                            const ::basegfx::B2DPoint aStartPoint(
                                rState.mapModeTransform * ::vcl::unotools::b2DPointFromPoint( pLineAct->GetStartPoint() ));
                            const ::basegfx::B2DPoint aEndPoint(
                                rState.mapModeTransform * ::vcl::unotools::b2DPointFromPoint( pLineAct->GetEndPoint() ));

                            ActionSharedPtr pLineAction;

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
                                    maActions.push_back(
                                        MtfAction(
                                            pLineAction,
                                            io_rCurrActionIndex ) );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            else if( LINE_NONE != rLineInfo.GetStyle() )
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
                                    maActions.push_back(
                                        MtfAction(
                                            pLineAction,
                                            io_rCurrActionIndex ) );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            // else: line style is default
                            // (i.e. invisible), don't generate action
                        }
                    }
                    break;

                    case META_RECT_ACTION:
                    {
                        const Rectangle& rRect(
                            static_cast<MetaRectAction*>(pCurrAct)->GetRect() );

                        if( rRect.IsEmpty() )
                            break;

                        const OutDevState& rState( rStates.getState() );
                        const ::basegfx::B2DPoint aTopLeftPixel(
                            rState.mapModeTransform * ::vcl::unotools::b2DPointFromPoint( rRect.TopLeft() ) );
                        const ::basegfx::B2DPoint aBottomRightPixel(
                            rState.mapModeTransform * ::vcl::unotools::b2DPointFromPoint( rRect.BottomRight() ) +
                            // #121100# OutputDevice::DrawRect() fills
                            // rectangles Apple-like, i.e. with one
                            // additional pixel to the right and bottom.
                            ::basegfx::B2DPoint(1,1) );

                        createFillAndStroke( ::basegfx::tools::createPolygonFromRect(
                                                 ::basegfx::B2DRange( aTopLeftPixel,
                                                                      aBottomRightPixel )),
                                             rFactoryParms );
                        break;
                    }

                    case META_ROUNDRECT_ACTION:
                    {
                        const Rectangle& rRect(
                            static_cast<MetaRoundRectAction*>(pCurrAct)->GetRect());

                        if( rRect.IsEmpty() )
                            break;

                        ::basegfx::B2DPolygon aPoly(
                            ::basegfx::tools::createPolygonFromRect(
                                ::basegfx::B2DRange(
                                    ::vcl::unotools::b2DPointFromPoint( rRect.TopLeft() ),
                                    ::vcl::unotools::b2DPointFromPoint( rRect.BottomRight() ) +
                                    ::basegfx::B2DPoint(1,1) ),
                                ( (double) static_cast<MetaRoundRectAction*>(pCurrAct)->GetHorzRound() ) / rRect.GetWidth(),
                                ( (double) static_cast<MetaRoundRectAction*>(pCurrAct)->GetVertRound() ) / rRect.GetHeight() ) );
                        aPoly.transform( rStates.getState().mapModeTransform );

                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case META_ELLIPSE_ACTION:
                    {
                        const Rectangle& rRect(
                            static_cast<MetaEllipseAction*>(pCurrAct)->GetRect() );

                        if( rRect.IsEmpty() )
                            break;

                        const ::basegfx::B2DRange aRange(
                            ::vcl::unotools::b2DPointFromPoint( rRect.TopLeft() ),
                            ::vcl::unotools::b2DPointFromPoint( rRect.BottomRight() ) +
                            ::basegfx::B2DPoint(1,1) );

                        ::basegfx::B2DPolygon aPoly(
                            ::basegfx::tools::createPolygonFromEllipse(
                                aRange.getCenter(),
                                aRange.getWidth(),
                                aRange.getHeight() ));
                        aPoly.transform( rStates.getState().mapModeTransform );

                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case META_ARC_ACTION:
                    {
                        // TODO(F1): Missing basegfx functionality. Mind empty rects!
                        const Polygon aToolsPoly( static_cast<MetaArcAction*>(pCurrAct)->GetRect(),
                                                  static_cast<MetaArcAction*>(pCurrAct)->GetStartPoint(),
                                                  static_cast<MetaArcAction*>(pCurrAct)->GetEndPoint(), POLY_ARC );
                        ::basegfx::B2DPolygon aPoly( aToolsPoly.getB2DPolygon() );
                        aPoly.transform( rStates.getState().mapModeTransform );

                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case META_PIE_ACTION:
                    {
                        // TODO(F1): Missing basegfx functionality. Mind empty rects!
                        const Polygon aToolsPoly( static_cast<MetaPieAction*>(pCurrAct)->GetRect(),
                                                  static_cast<MetaPieAction*>(pCurrAct)->GetStartPoint(),
                                                  static_cast<MetaPieAction*>(pCurrAct)->GetEndPoint(), POLY_PIE );
                        ::basegfx::B2DPolygon aPoly( aToolsPoly.getB2DPolygon() );
                        aPoly.transform( rStates.getState().mapModeTransform );

                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case META_CHORD_ACTION:
                    {
                        // TODO(F1): Missing basegfx functionality. Mind empty rects!
                        const Polygon aToolsPoly( static_cast<MetaChordAction*>(pCurrAct)->GetRect(),
                                                  static_cast<MetaChordAction*>(pCurrAct)->GetStartPoint(),
                                                  static_cast<MetaChordAction*>(pCurrAct)->GetEndPoint(), POLY_CHORD );
                        ::basegfx::B2DPolygon aPoly( aToolsPoly.getB2DPolygon() );
                        aPoly.transform( rStates.getState().mapModeTransform );

                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case META_POLYLINE_ACTION:
                    {
                        const OutDevState& rState( rStates.getState() );
                        if( rState.lineColor.getLength() ||
                            rState.fillColor.getLength() )
                        {
                            MetaPolyLineAction* pPolyLineAct = static_cast<MetaPolyLineAction*>(pCurrAct);

                            const LineInfo& rLineInfo( pPolyLineAct->GetLineInfo() );
                            ::basegfx::B2DPolygon aPoly( pPolyLineAct->GetPolygon().getB2DPolygon() );
                            aPoly.transform( rState.mapModeTransform );

                            ActionSharedPtr pLineAction;

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
                                    maActions.push_back(
                                        MtfAction(
                                            pLineAction,
                                            io_rCurrActionIndex ) );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            else if( LINE_NONE != rLineInfo.GetStyle() )
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
                                    maActions.push_back(
                                        MtfAction(
                                            pLineAction,
                                            io_rCurrActionIndex ) );

                                    io_rCurrActionIndex += pLineAction->getActionCount()-1;
                                }
                            }
                            // else: line style is default
                            // (i.e. invisible), don't generate action
                        }
                    }
                    break;

                    case META_POLYGON_ACTION:
                    {
                        ::basegfx::B2DPolygon aPoly( static_cast<MetaPolygonAction*>(pCurrAct)->GetPolygon().getB2DPolygon() );
                        aPoly.transform( rStates.getState().mapModeTransform );
                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case META_POLYPOLYGON_ACTION:
                    {
                        ::basegfx::B2DPolyPolygon aPoly( static_cast<MetaPolyPolygonAction*>(pCurrAct)->GetPolyPolygon().getB2DPolyPolygon() );
                        aPoly.transform( rStates.getState().mapModeTransform );
                        createFillAndStroke( aPoly,
                                             rFactoryParms );
                    }
                    break;

                    case META_BMP_ACTION:
                    {
                        MetaBmpAction* pAct = static_cast<MetaBmpAction*>(pCurrAct);

                        ActionSharedPtr pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    pAct->GetBitmap(),
                                    rStates.getState().mapModeTransform *
                                    ::vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                    rCanvas,
                                    rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_BMPSCALE_ACTION:
                    {
                        MetaBmpScaleAction* pAct = static_cast<MetaBmpScaleAction*>(pCurrAct);

                        ActionSharedPtr pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    pAct->GetBitmap(),
                                    rStates.getState().mapModeTransform *
                                    ::vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                    rStates.getState().mapModeTransform *
                                    ::vcl::unotools::b2DSizeFromSize( pAct->GetSize() ),
                                    rCanvas,
                                    rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_BMPSCALEPART_ACTION:
                    {
                        MetaBmpScalePartAction* pAct = static_cast<MetaBmpScalePartAction*>(pCurrAct);

                        // crop bitmap to given source rectangle (no
                        // need to copy and convert the whole bitmap)
                        Bitmap aBmp( pAct->GetBitmap() );
                        const Rectangle aCropRect( pAct->GetSrcPoint(),
                                                    pAct->GetSrcSize() );
                        aBmp.Crop( aCropRect );

                        ActionSharedPtr pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    aBmp,
                                    rStates.getState().mapModeTransform *
                                    ::vcl::unotools::b2DPointFromPoint( pAct->GetDestPoint() ),
                                    rStates.getState().mapModeTransform *
                                    ::vcl::unotools::b2DSizeFromSize( pAct->GetDestSize() ),
                                    rCanvas,
                                    rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_BMPEX_ACTION:
                    {
                        MetaBmpExAction* pAct = static_cast<MetaBmpExAction*>(pCurrAct);

                        ActionSharedPtr pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    pAct->GetBitmapEx(),
                                    rStates.getState().mapModeTransform *
                                    ::vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                    rCanvas,
                                    rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_BMPEXSCALE_ACTION:
                    {
                        MetaBmpExScaleAction* pAct = static_cast<MetaBmpExScaleAction*>(pCurrAct);

                        ActionSharedPtr pBmpAction(
                                internal::BitmapActionFactory::createBitmapAction(
                                    pAct->GetBitmapEx(),
                                    rStates.getState().mapModeTransform *
                                    ::vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                    rStates.getState().mapModeTransform *
                                    ::vcl::unotools::b2DSizeFromSize( pAct->GetSize() ),
                                    rCanvas,
                                    rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_BMPEXSCALEPART_ACTION:
                    {
                        MetaBmpExScalePartAction* pAct = static_cast<MetaBmpExScalePartAction*>(pCurrAct);

                        // crop bitmap to given source rectangle (no
                        // need to copy and convert the whole bitmap)
                        BitmapEx aBmp( pAct->GetBitmapEx() );
                        const Rectangle aCropRect( pAct->GetSrcPoint(),
                                                   pAct->GetSrcSize() );
                        aBmp.Crop( aCropRect );

                        ActionSharedPtr pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rStates.getState().mapModeTransform *
                                ::vcl::unotools::b2DPointFromPoint( pAct->GetDestPoint() ),
                                rStates.getState().mapModeTransform *
                                ::vcl::unotools::b2DSizeFromSize( pAct->GetDestSize() ),
                                rCanvas,
                                rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_MASK_ACTION:
                    {
                        MetaMaskAction* pAct = static_cast<MetaMaskAction*>(pCurrAct);

                        // create masked BitmapEx right here, as the
                        // canvas does not provide equivalent
                        // functionality
                        BitmapEx aBmp( createMaskBmpEx( pAct->GetBitmap(),
                                                        pAct->GetColor() ));

                        ActionSharedPtr pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rStates.getState().mapModeTransform *
                                ::vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                rCanvas,
                                rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_MASKSCALE_ACTION:
                    {
                        MetaMaskScaleAction* pAct = static_cast<MetaMaskScaleAction*>(pCurrAct);

                        // create masked BitmapEx right here, as the
                        // canvas does not provide equivalent
                        // functionality
                        BitmapEx aBmp( createMaskBmpEx( pAct->GetBitmap(),
                                                        pAct->GetColor() ));

                        ActionSharedPtr pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rStates.getState().mapModeTransform *
                                ::vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                rStates.getState().mapModeTransform *
                                ::vcl::unotools::b2DSizeFromSize( pAct->GetSize() ),
                                rCanvas,
                                rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_MASKSCALEPART_ACTION:
                    {
                        MetaMaskScalePartAction* pAct = static_cast<MetaMaskScalePartAction*>(pCurrAct);

                        // create masked BitmapEx right here, as the
                        // canvas does not provide equivalent
                        // functionality
                        BitmapEx aBmp( createMaskBmpEx( pAct->GetBitmap(),
                                                        pAct->GetColor() ));

                        // crop bitmap to given source rectangle (no
                        // need to copy and convert the whole bitmap)
                        const Rectangle aCropRect( pAct->GetSrcPoint(),
                                                   pAct->GetSrcSize() );
                        aBmp.Crop( aCropRect );

                        ActionSharedPtr pBmpAction(
                            internal::BitmapActionFactory::createBitmapAction(
                                aBmp,
                                rStates.getState().mapModeTransform *
                                ::vcl::unotools::b2DPointFromPoint( pAct->GetDestPoint() ),
                                rStates.getState().mapModeTransform *
                                ::vcl::unotools::b2DSizeFromSize( pAct->GetDestSize() ),
                                rCanvas,
                                rStates.getState() ) );

                        if( pBmpAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pBmpAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pBmpAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_GRADIENTEX_ACTION:
                        // TODO(F1): use native Canvas gradients here
                        // action is ignored here, because redundant to META_GRADIENT_ACTION
                        break;

                    case META_WALLPAPER_ACTION:
                        // TODO(F2): NYI
                        break;

                    case META_TRANSPARENT_ACTION:
                    {
                        const OutDevState& rState( rStates.getState() );
                        if( rState.lineColor.getLength() ||
                            rState.fillColor.getLength() )
                        {
                            MetaTransparentAction* pAct = static_cast<MetaTransparentAction*>(pCurrAct);
                            ::basegfx::B2DPolyPolygon aPoly( pAct->GetPolyPolygon().getB2DPolyPolygon() );
                            aPoly.transform( rState.mapModeTransform );

                            ActionSharedPtr pPolyAction(
                                internal::PolyPolyActionFactory::createPolyPolyAction(
                                    aPoly,
                                    rCanvas,
                                    rState,
                                    pAct->GetTransparence() ) );

                            if( pPolyAction )
                            {
                                maActions.push_back(
                                    MtfAction(
                                        pPolyAction,
                                        io_rCurrActionIndex ) );

                                io_rCurrActionIndex += pPolyAction->getActionCount()-1;
                            }
                        }
                    }
                    break;

                    case META_FLOATTRANSPARENT_ACTION:
                    {
                        MetaFloatTransparentAction* pAct = static_cast<MetaFloatTransparentAction*>(pCurrAct);

                        internal::MtfAutoPtr pMtf(
                            new ::GDIMetaFile( pAct->GetGDIMetaFile() ) );

                        // TODO(P2): Use native canvas gradients here (saves a lot of UNO calls)
                        internal::GradientAutoPtr pGradient(
                            new Gradient( pAct->GetGradient() ) );

                        DBG_TESTSOLARMUTEX();

                        ActionSharedPtr pFloatTransAction(
                            internal::TransparencyGroupActionFactory::createTransparencyGroupAction(
                                pMtf,
                                pGradient,
                                rParms,
                                rStates.getState().mapModeTransform *
                                ::vcl::unotools::b2DPointFromPoint( pAct->GetPoint() ),
                                rStates.getState().mapModeTransform *
                                ::vcl::unotools::b2DSizeFromSize( pAct->GetSize() ),
                                rCanvas,
                                rStates.getState() ) );

                        if( pFloatTransAction )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pFloatTransAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pFloatTransAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_TEXT_ACTION:
                    {
                        MetaTextAction* pAct = static_cast<MetaTextAction*>(pCurrAct);
                        OUString sText = pAct->GetText();

                        if (rVDev.GetDigitLanguage())
                            sText = convertToLocalizedNumerals(sText, rVDev.GetDigitLanguage());

                        createTextAction(
                            pAct->GetPoint(),
                            sText,
                            pAct->GetIndex(),
                            pAct->GetLen() == (sal_uInt16)STRING_LEN ? pAct->GetText().getLength() - pAct->GetIndex() : pAct->GetLen(),
                            NULL,
                            rFactoryParms,
                            bSubsettableActions );
                    }
                    break;

                    case META_TEXTARRAY_ACTION:
                    {
                        MetaTextArrayAction* pAct = static_cast<MetaTextArrayAction*>(pCurrAct);
                        OUString sText = pAct->GetText();

                        if (rVDev.GetDigitLanguage())
                            sText = convertToLocalizedNumerals(sText, rVDev.GetDigitLanguage());

                        createTextAction(
                            pAct->GetPoint(),
                            sText,
                            pAct->GetIndex(),
                            pAct->GetLen() == (sal_uInt16)STRING_LEN ? pAct->GetText().getLength() - pAct->GetIndex() : pAct->GetLen(),
                            pAct->GetDXArray(),
                            rFactoryParms,
                            bSubsettableActions );
                    }
                    break;

                    case META_TEXTLINE_ACTION:
                    {
                        MetaTextLineAction*      pAct = static_cast<MetaTextLineAction*>(pCurrAct);

                        const OutDevState&       rState( rStates.getState() );
                        const ::Size             aBaselineOffset( tools::getBaselineOffset( rState,
                                                                                            rVDev ) );
                        const ::basegfx::B2DSize aSize( rState.mapModeTransform *
                                                        ::basegfx::B2DSize(pAct->GetWidth(),
                                                                           0 ));

                        ActionSharedPtr pPolyAction(
                            PolyPolyActionFactory::createPolyPolyAction(
                                tools::createTextLinesPolyPolygon(
                                    rState.mapModeTransform *
                                    ::basegfx::B2DPoint(
                                        ::vcl::unotools::b2DPointFromPoint(pAct->GetStartPoint()) +
                                        ::vcl::unotools::b2DSizeFromSize(aBaselineOffset)),
                                    aSize.getX(),
                                    tools::createTextLineInfo( rVDev,
                                                               rState )),
                                rCanvas,
                                rState ) );

                        if( pPolyAction.get() )
                        {
                            maActions.push_back(
                                MtfAction(
                                    pPolyAction,
                                    io_rCurrActionIndex ) );

                            io_rCurrActionIndex += pPolyAction->getActionCount()-1;
                        }
                    }
                    break;

                    case META_TEXTRECT_ACTION:
                    {
                        MetaTextRectAction* pAct = static_cast<MetaTextRectAction*>(pCurrAct);

                        rStates.pushState(PUSH_ALL);

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

                    case META_STRETCHTEXT_ACTION:
                    {
                        MetaStretchTextAction* pAct = static_cast<MetaStretchTextAction*>(pCurrAct);
                        OUString sText = pAct->GetText();

                        if (rVDev.GetDigitLanguage())
                            sText = convertToLocalizedNumerals(sText, rVDev.GetDigitLanguage());

                        const sal_uInt16 nLen( pAct->GetLen() == (sal_uInt16)STRING_LEN ?
                                           pAct->GetText().getLength() - pAct->GetIndex() : pAct->GetLen() );

                        // #i70897# Nothing to do, actually...
                        if( nLen == 0 )
                            break;

                        // have to fit the text into the given
                        // width. This is achieved by internally
                        // generating a DX array, and uniformly
                        // distributing the excess/insufficient width
                        // to every logical character.
                        ::boost::scoped_array< sal_Int32 > pDXArray( new sal_Int32[nLen] );

                        rVDev.GetTextArray( pAct->GetText(), pDXArray.get(),
                                            pAct->GetIndex(), pAct->GetLen() );

                        const sal_Int32 nWidthDifference( pAct->GetWidth() - pDXArray[ nLen-1 ] );

                        // Last entry of pDXArray contains total width of the text
                        sal_Int32* p=pDXArray.get();
                        for( sal_uInt16 i=1; i<=nLen; ++i )
                        {
                            // calc ratio for every array entry, to
                            // distribute rounding errors 'evenly'
                            // across the characters. Note that each
                            // entry represents the 'end' position of
                            // the corresponding character, thus, we
                            // let i run from 1 to nLen.
                            *p++ += (sal_Int32)i*nWidthDifference/nLen;
                        }

                        createTextAction(
                            pAct->GetPoint(),
                            sText,
                            pAct->GetIndex(),
                            pAct->GetLen() == (sal_uInt16)STRING_LEN ? pAct->GetText().getLength() - pAct->GetIndex() : pAct->GetLen(),
                            pDXArray.get(),
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

            return true;
        }


        namespace
        {
            class ActionRenderer
            {
            public:
                ActionRenderer( const ::basegfx::B2DHomMatrix& rTransformation ) :
                    maTransformation( rTransformation ),
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
                AreaQuery( const ::basegfx::B2DHomMatrix& rTransformation ) :
                    maTransformation( rTransformation ),
                    maBounds()
                {
                }

                bool result() const
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

                ::basegfx::B2DRange getBounds() const
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
                                ImplRenderer::ActionVector::const_iterator          aRangeEnd,
                                sal_Int32                                           nStartIndex,
                                sal_Int32                                           nEndIndex,
                                const ImplRenderer::ActionVector::const_iterator&   rEnd )
            {
                if( aRangeBegin == aRangeEnd )
                {
                    // only a single action. Setup subset, and call functor
                    Action::Subset aSubset;
                    aSubset.mnSubsetBegin = ::std::max( sal_Int32( 0 ),
                                                        nStartIndex - aRangeBegin->mnOrigIndex );
                    aSubset.mnSubsetEnd   = ::std::min( aRangeBegin->mpAction->getActionCount(),
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
                    aSubset.mnSubsetBegin = ::std::max( sal_Int32( 0 ),
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
                        //
                        // or
                        //
                        // nEndIndex references something _after_
                        // aRangeBegin, but _before_ aRangeEnd
                        //
                        // either way: no partial action left
                        return rFunctor.result();
                    }

                    aSubset.mnSubsetBegin = 0;
                    aSubset.mnSubsetEnd   = nEndIndex - aRangeEnd->mnOrigIndex;

                    ENSURE_OR_RETURN_FALSE( aSubset.mnSubsetBegin >= 0 && aSubset.mnSubsetEnd >= 0,
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
            io_rStartIndex = ::std::max( nMinActionIndex,
                                         io_rStartIndex );
            io_rEndIndex = ::std::min( nMaxActionIndex,
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
            o_rRangeBegin = ::std::lower_bound( aBegin, aEnd,
                                                MtfAction( ActionSharedPtr(), io_rStartIndex ),
                                                UpperBoundActionIndexComparator() );
            o_rRangeEnd   = ::std::lower_bound( aBegin, aEnd,
                                                MtfAction( ActionSharedPtr(), io_rEndIndex ),
                                                UpperBoundActionIndexComparator() );
            return true;
        }


        // Public methods
        // ====================================================================

        ImplRenderer::ImplRenderer( const CanvasSharedPtr&  rCanvas,
                                    const GDIMetaFile&      rMtf,
                                    const Parameters&       rParams ) :
            CanvasGraphicHelper( rCanvas ),
            maActions()
        {
            SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::ImplRenderer::ImplRenderer(mtf)" );

            OSL_ENSURE( rCanvas.get() != NULL && rCanvas->getUNOCanvas().is(),
                        "ImplRenderer::ImplRenderer(): Invalid canvas" );
            OSL_ENSURE( rCanvas->getUNOCanvas()->getDevice().is(),
                        "ImplRenderer::ImplRenderer(): Invalid graphic device" );

            // make sure canvas and graphic device are valid; action
            // creation don't check that every time
            if( rCanvas.get() == NULL ||
                !rCanvas->getUNOCanvas().is() ||
                !rCanvas->getUNOCanvas()->getDevice().is() )
            {
                // leave actions empty
                return;
            }

            VectorOfOutDevStates    aStateStack;

            VirtualDevice aVDev;
            aVDev.EnableOutput( sal_False );

            // Setup VDev for state tracking and mapping
            // =========================================

            aVDev.SetMapMode( rMtf.GetPrefMapMode() );

            const Size aMtfSize( rMtf.GetPrefSize() );
            const Size aMtfSizePixPre( aVDev.LogicToPixel( aMtfSize,
                                                           rMtf.GetPrefMapMode() ) );

            // #i44110# correct null-sized output - there are shapes
            // which have zero size in at least one dimension
            const Size aMtfSizePix( ::std::max( aMtfSizePixPre.Width(), 1L ),
                                    ::std::max( aMtfSizePixPre.Height(), 1L ) );

            sal_Int32 nCurrActions(0);
            ActionFactoryParameters aParms(aStateStack,
                                           rCanvas,
                                           aVDev,
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
                                                   aVDev );

            ColorSharedPtr pColor( getCanvas()->createColor() );

            {
                ::cppcanvas::internal::OutDevState& rState = aStateStack.getState();
                // setup default text color to black
                rState.textColor =
                    rState.textFillColor =
                    rState.textLineColor = pColor->getDeviceColor( 0x000000FF );
            }

            // apply overrides from the Parameters struct
            if( rParams.maFillColor.is_initialized() )
            {
                ::cppcanvas::internal::OutDevState& rState = aStateStack.getState();
                rState.isFillColorSet = true;
                rState.fillColor = pColor->getDeviceColor( *rParams.maFillColor );
            }
            if( rParams.maLineColor.is_initialized() )
            {
                ::cppcanvas::internal::OutDevState& rState = aStateStack.getState();
                rState.isLineColorSet = true;
                rState.lineColor = pColor->getDeviceColor( *rParams.maLineColor );
            }
            if( rParams.maTextColor.is_initialized() )
            {
                ::cppcanvas::internal::OutDevState& rState = aStateStack.getState();
                rState.isTextFillColorSet = true;
                rState.isTextLineColorSet = true;
                rState.textColor =
                    rState.textFillColor =
                    rState.textLineColor = pColor->getDeviceColor( *rParams.maTextColor );
            }
            if( rParams.maFontName.is_initialized() ||
                rParams.maFontWeight.is_initialized() ||
                rParams.maFontLetterForm.is_initialized() ||
                rParams.maFontUnderline.is_initialized() ||
                rParams.maFontProportion.is_initialized() )
            {
                ::cppcanvas::internal::OutDevState& rState = aStateStack.getState();

                rState.xFont = createFont( rState.fontRotation,
                                           ::Font(), // default font
                                           aParms );
            }

            /* EMF+ */
            memset (aObjects, 0, sizeof (aObjects));
            mbMultipart = false;

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
            // don't leak EMFPObjects
            for(unsigned int i=0; i<SAL_N_ELEMENTS(aObjects); ++i)
                delete aObjects[i];
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

                ::basegfx::B2DHomMatrix aMatrix;
                ::canvas::tools::getRenderStateTransform( aMatrix,
                                                          getRenderState() );

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
                SAL_WARN("cppcanvas.emf", "" << OUStringToOString(
                                comphelper::anyToString( cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );

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

            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::getRenderStateTransform( aMatrix,
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

            ::basegfx::B2DHomMatrix aMatrix;
            ::canvas::tools::getRenderStateTransform( aMatrix,
                                                      getRenderState() );

            try
            {
                return ::std::for_each( maActions.begin(), maActions.end(), ActionRenderer( aMatrix ) ).result();
            }
            catch( uno::Exception& )
            {
                SAL_WARN( "cppcanvas.emf", "" << OUStringToOString(
                                comphelper::anyToString( cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );

                return false;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
