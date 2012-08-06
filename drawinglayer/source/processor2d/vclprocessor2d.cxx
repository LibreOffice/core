/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <drawinglayer/processor2d/vclprocessor2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <tools/debug.hxx>
#include <vcl/outdev.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/rendergraphicprimitive2d.hxx>
#include <vclhelperbitmaptransform.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <vclhelperbitmaprender.hxx>
#include <drawinglayer/attribute/sdrfillbitmapattribute.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <vclhelpergradient.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <vclhelperbufferdevice.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <drawinglayer/primitive2d/pagepreviewprimitive2d.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/metric.hxx>
#include <drawinglayer/primitive2d/textenumsprimitive2d.hxx>
#include <drawinglayer/primitive2d/epsprimitive2d.hxx>
#include <vcl/rendergraphicrasterizer.hxx>

#include "getdigitlanguage.hxx"

//////////////////////////////////////////////////////////////////////////////
// control support

#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>

//////////////////////////////////////////////////////////////////////////////
// for test, can be removed again

#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dtrapezoid.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        //////////////////////////////////////////////////////////////////////////////
        // UNO class usages
        using ::com::sun::star::uno::Reference;
        using ::com::sun::star::uno::UNO_QUERY;
        using ::com::sun::star::uno::UNO_QUERY_THROW;
        using ::com::sun::star::uno::Exception;
        using ::com::sun::star::awt::XView;
        using ::com::sun::star::awt::XGraphics;
        using ::com::sun::star::awt::XWindow;
        using ::com::sun::star::awt::PosSize::POSSIZE;

        //////////////////////////////////////////////////////////////////////////////
        // rendering support

        // directdraw of text simple portion or decorated portion primitive. When decorated, all the extra
        // information is translated to VCL parameters and set at the font.
        // Acceptance is restricted to no shearing and positive scaling in X and Y (no font mirroring
        // for VCL)
        void VclProcessor2D::RenderTextSimpleOrDecoratedPortionPrimitive2D(const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate)
        {
            // decompose matrix to have position and size of text
            basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rTextCandidate.getTextTransform());
            basegfx::B2DVector aFontScaling, aTranslate;
            double fRotate, fShearX;
            aLocalTransform.decompose(aFontScaling, aTranslate, fRotate, fShearX);
            bool bPrimitiveAccepted(false);

            if(basegfx::fTools::equalZero(fShearX))
            {
                if(basegfx::fTools::less(aFontScaling.getX(), 0.0) && basegfx::fTools::less(aFontScaling.getY(), 0.0))
                {
                    // handle special case: If scale is negative in (x,y) (3rd quadrant), it can
                    // be expressed as rotation by PI. Use this since the Font rendering will not
                    // apply the negative scales in any form
                    aFontScaling = basegfx::absolute(aFontScaling);
                    fRotate += F_PI;
                }

                if(basegfx::fTools::more(aFontScaling.getX(), 0.0) && basegfx::fTools::more(aFontScaling.getY(), 0.0))
                {
                    // Get the VCL font (use FontHeight as FontWidth)
                    Font aFont(primitive2d::getVclFontFromFontAttribute(
                        rTextCandidate.getFontAttribute(),
                        aFontScaling.getX(),
                        aFontScaling.getY(),
                        fRotate,
                        rTextCandidate.getLocale()));

                    // handle additional font attributes
                    const primitive2d::TextDecoratedPortionPrimitive2D* pTCPP =
                        dynamic_cast<const primitive2d::TextDecoratedPortionPrimitive2D*>( &rTextCandidate );

                    if( pTCPP != NULL )
                    {

                        // set the color of text decorations
                        const basegfx::BColor aTextlineColor = maBColorModifierStack.getModifiedColor(pTCPP->getTextlineColor());
                        mpOutputDevice->SetTextLineColor( Color(aTextlineColor) );

                        // set Overline attribute
                        const FontUnderline eFontOverline(primitive2d::mapTextLineToFontUnderline( pTCPP->getFontOverline() ));
                        if( eFontOverline != UNDERLINE_NONE )
                        {
                            aFont.SetOverline( eFontOverline );
                            const basegfx::BColor aOverlineColor = maBColorModifierStack.getModifiedColor(pTCPP->getOverlineColor());
                            mpOutputDevice->SetOverlineColor( Color(aOverlineColor) );
                            if( pTCPP->getWordLineMode() )
                                aFont.SetWordLineMode( true );
                        }

                        // set Underline attribute
                        const FontUnderline eFontUnderline(primitive2d::mapTextLineToFontUnderline( pTCPP->getFontUnderline() ));
                        if( eFontUnderline != UNDERLINE_NONE )
                        {
                            aFont.SetUnderline( eFontUnderline );
                            if( pTCPP->getWordLineMode() )
                                aFont.SetWordLineMode( true );
//TODO: ???                 if( pTCPP->getUnderlineAbove() )
//                              aFont.SetUnderlineAbove( true );
                        }

                        // set Strikeout attribute
                        const FontStrikeout eFontStrikeout(primitive2d::mapTextStrikeoutToFontStrikeout(pTCPP->getTextStrikeout()));

                        if( eFontStrikeout != STRIKEOUT_NONE )
                            aFont.SetStrikeout( eFontStrikeout );

                        // set EmphasisMark attribute
                        FontEmphasisMark eFontEmphasisMark = EMPHASISMARK_NONE;
                        switch( pTCPP->getTextEmphasisMark() )
                        {
                            default:
                                DBG_WARNING1( "DrawingLayer: Unknown EmphasisMark style (%d)!", pTCPP->getTextEmphasisMark() );
                                // fall through
                            case primitive2d::TEXT_EMPHASISMARK_NONE:   eFontEmphasisMark = EMPHASISMARK_NONE; break;
                            case primitive2d::TEXT_EMPHASISMARK_DOT:    eFontEmphasisMark = EMPHASISMARK_DOT; break;
                            case primitive2d::TEXT_EMPHASISMARK_CIRCLE: eFontEmphasisMark = EMPHASISMARK_CIRCLE; break;
                            case primitive2d::TEXT_EMPHASISMARK_DISC:   eFontEmphasisMark = EMPHASISMARK_DISC; break;
                            case primitive2d::TEXT_EMPHASISMARK_ACCENT: eFontEmphasisMark = EMPHASISMARK_ACCENT; break;
                        }

                        if( eFontEmphasisMark != EMPHASISMARK_NONE )
                        {
                            DBG_ASSERT( (pTCPP->getEmphasisMarkAbove() != pTCPP->getEmphasisMarkBelow()),
                                "DrawingLayer: Bad EmphasisMark position!" );
                            if( pTCPP->getEmphasisMarkAbove() )
                                eFontEmphasisMark |= EMPHASISMARK_POS_ABOVE;
                            else
                                eFontEmphasisMark |= EMPHASISMARK_POS_BELOW;
                            aFont.SetEmphasisMark( eFontEmphasisMark );
                        }

                        // set Relief attribute
                        FontRelief eFontRelief = RELIEF_NONE;
                        switch( pTCPP->getTextRelief() )
                        {
                            default:
                                DBG_WARNING1( "DrawingLayer: Unknown Relief style (%d)!", pTCPP->getTextRelief() );
                                // fall through
                            case primitive2d::TEXT_RELIEF_NONE:     eFontRelief = RELIEF_NONE; break;
                            case primitive2d::TEXT_RELIEF_EMBOSSED: eFontRelief = RELIEF_EMBOSSED; break;
                            case primitive2d::TEXT_RELIEF_ENGRAVED: eFontRelief = RELIEF_ENGRAVED; break;
                        }

                        if( eFontRelief != RELIEF_NONE )
                            aFont.SetRelief( eFontRelief );

                        // set Shadow attribute
                        if( pTCPP->getShadow() )
                            aFont.SetShadow( true );
                    }

                    // create transformed integer DXArray in view coordinate system
                    ::std::vector< sal_Int32 > aTransformedDXArray;

                    if(rTextCandidate.getDXArray().size())
                    {
                        aTransformedDXArray.reserve(rTextCandidate.getDXArray().size());
                        const basegfx::B2DVector aPixelVector(maCurrentTransformation * basegfx::B2DVector(1.0, 0.0));
                        const double fPixelVectorFactor(aPixelVector.getLength());

                        for(::std::vector< double >::const_iterator aStart(rTextCandidate.getDXArray().begin());
                            aStart != rTextCandidate.getDXArray().end(); ++aStart)
                        {
                            aTransformedDXArray.push_back(basegfx::fround((*aStart) * fPixelVectorFactor));
                        }
                    }

                    // set parameters and paint text snippet
                    const basegfx::BColor aRGBFontColor(maBColorModifierStack.getModifiedColor(rTextCandidate.getFontColor()));
                    const basegfx::B2DPoint aPoint(aLocalTransform * basegfx::B2DPoint(0.0, 0.0));
                    const Point aStartPoint(basegfx::fround(aPoint.getX()), basegfx::fround(aPoint.getY()));
                    const sal_uInt32 nOldLayoutMode(mpOutputDevice->GetLayoutMode());

                    if(rTextCandidate.getFontAttribute().getRTL())
                    {
                        sal_uInt32 nRTLLayoutMode(nOldLayoutMode & ~(TEXT_LAYOUT_COMPLEX_DISABLED|TEXT_LAYOUT_BIDI_STRONG));
                        nRTLLayoutMode |= TEXT_LAYOUT_BIDI_RTL|TEXT_LAYOUT_TEXTORIGIN_LEFT;
                        mpOutputDevice->SetLayoutMode(nRTLLayoutMode);
                    }

                    mpOutputDevice->SetFont(aFont);
                    mpOutputDevice->SetTextColor(Color(aRGBFontColor));

                    String aText( rTextCandidate.getText() );
                    xub_StrLen nPos = rTextCandidate.getTextPosition();
                    xub_StrLen nLen = rTextCandidate.getTextLength();

                    sal_Int32* pDXArray = aTransformedDXArray.size() ? &(aTransformedDXArray[0]) : NULL ;

                    if ( rTextCandidate.isFilled() )
                    {
                        basegfx::B2DVector aOldFontScaling, aOldTranslate;
                        double fOldRotate, fOldShearX;
                        rTextCandidate.getTextTransform().decompose(aOldFontScaling, aOldTranslate, fOldRotate, fOldShearX);

                        long nWidthToFill = static_cast<long>(rTextCandidate.getWidthToFill( ) * aFontScaling.getX() / aOldFontScaling.getX());

                        long nWidth = mpOutputDevice->GetTextArray(
                            rTextCandidate.getText(), pDXArray, 0, 1 );
                        long nChars = 2;
                        if ( nWidth )
                            nChars = nWidthToFill / nWidth;

                        String aFilled;
                        aFilled.Fill( (sal_uInt16)nChars, aText.GetChar( 0 ) );
                        aText = aFilled;
                        nPos = 0;
                        nLen = nChars;
                    }

                    if(!aTransformedDXArray.empty())
                    {
                        mpOutputDevice->DrawTextArray(
                            aStartPoint,
                            aText,
                            pDXArray,
                            nPos,
                            nLen);
                    }
                    else
                    {
                        mpOutputDevice->DrawText(
                            aStartPoint,
                            aText,
                            nPos,
                            nLen);
                    }

                    if(rTextCandidate.getFontAttribute().getRTL())
                    {
                        mpOutputDevice->SetLayoutMode(nOldLayoutMode);
                    }

                    bPrimitiveAccepted = true;
                }
            }

            if(!bPrimitiveAccepted)
            {
                // let break down
                process(rTextCandidate.get2DDecomposition(getViewInformation2D()));
            }
        }

        // direct draw of hairline
        void VclProcessor2D::RenderPolygonHairlinePrimitive2D(const primitive2d::PolygonHairlinePrimitive2D& rPolygonCandidate, bool bPixelBased)
        {
            const basegfx::BColor aHairlineColor(maBColorModifierStack.getModifiedColor(rPolygonCandidate.getBColor()));
            mpOutputDevice->SetLineColor(Color(aHairlineColor));
            mpOutputDevice->SetFillColor();

            basegfx::B2DPolygon aLocalPolygon(rPolygonCandidate.getB2DPolygon());
            aLocalPolygon.transform(maCurrentTransformation);

            static bool bCheckTrapezoidDecomposition(false);
            static bool bShowOutlinesThere(false);
            if(bCheckTrapezoidDecomposition)
            {
                // clip against discrete ViewPort
                const basegfx::B2DRange& rDiscreteViewport = getViewInformation2D().getDiscreteViewport();
                basegfx::B2DPolyPolygon aLocalPolyPolygon(basegfx::tools::clipPolygonOnRange(
                    aLocalPolygon, rDiscreteViewport, true, false));

                if(aLocalPolyPolygon.count())
                {
                    // subdivide
                    aLocalPolyPolygon = basegfx::tools::adaptiveSubdivideByDistance(
                        aLocalPolyPolygon, 0.5);

                    // trapezoidize
                    static double fLineWidth(2.0);
                    basegfx::B2DTrapezoidVector aB2DTrapezoidVector;
                    basegfx::tools::createLineTrapezoidFromB2DPolyPolygon(aB2DTrapezoidVector, aLocalPolyPolygon, fLineWidth);

                    const sal_uInt32 nCount(aB2DTrapezoidVector.size());

                    if(nCount)
                    {
                        basegfx::BColor aInvPolygonColor(aHairlineColor);
                        aInvPolygonColor.invert();

                        for(sal_uInt32 a(0); a < nCount; a++)
                        {
                            const basegfx::B2DPolygon aTempPolygon(aB2DTrapezoidVector[a].getB2DPolygon());

                            if(bShowOutlinesThere)
                            {
                                mpOutputDevice->SetFillColor(Color(aHairlineColor));
                                mpOutputDevice->SetLineColor();
                            }

                            mpOutputDevice->DrawPolygon(aTempPolygon);

                            if(bShowOutlinesThere)
                            {
                                mpOutputDevice->SetFillColor();
                                mpOutputDevice->SetLineColor(Color(aInvPolygonColor));
                                mpOutputDevice->DrawPolyLine(aTempPolygon, 0.0);
                            }
                        }
                    }
                }
            }
            else
            {
                if(bPixelBased && getOptionsDrawinglayer().IsAntiAliasing() && getOptionsDrawinglayer().IsSnapHorVerLinesToDiscrete())
                {
                    // #i98289#
                    // when a Hairline is painted and AntiAliasing is on the option SnapHorVerLinesToDiscrete
                    // allows to suppress AntiAliasing for pure horizontal or vertical lines. This is done since
                    // not-AntiAliased such lines look more pleasing to the eye (e.g. 2D chart content). This
                    // NEEDS to be done in discrete coordinates, so only useful for pixel based rendering.
                    aLocalPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aLocalPolygon);
                }

                mpOutputDevice->DrawPolyLine(aLocalPolygon, 0.0);
            }
        }

        // direct draw of transformed BitmapEx primitive
        void VclProcessor2D::RenderBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate)
        {
            // create local transform
            basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rBitmapCandidate.getTransform());
            BitmapEx aBitmapEx(rBitmapCandidate.getBitmapEx());
            bool bPainted(false);

            if(maBColorModifierStack.count())
            {
                aBitmapEx = impModifyBitmapEx(maBColorModifierStack, aBitmapEx);

                if(aBitmapEx.IsEmpty())
                {
                    // color gets completely replaced, get it
                    const basegfx::BColor aModifiedColor(maBColorModifierStack.getModifiedColor(basegfx::BColor()));
                    basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());
                    aPolygon.transform(aLocalTransform);

                    mpOutputDevice->SetFillColor(Color(aModifiedColor));
                    mpOutputDevice->SetLineColor();
                    mpOutputDevice->DrawPolygon(aPolygon);

                    bPainted = true;
                }
            }

            if(!bPainted)
            {
                static bool bForceUseOfOwnTransformer(false);
                static bool bUseGraphicManager(true);

                // decompose matrix to check for shear, rotate and mirroring
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                aLocalTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                if(!bForceUseOfOwnTransformer && basegfx::fTools::equalZero(fShearX))
                {
                    if(!bUseGraphicManager && basegfx::fTools::equalZero(fRotate))
                    {
                        RenderBitmapPrimitive2D_BitmapEx(*mpOutputDevice, aBitmapEx, aLocalTransform);
                    }
                    else
                    {
                        RenderBitmapPrimitive2D_GraphicManager(*mpOutputDevice, aBitmapEx, aLocalTransform);
                    }
                }
                else
                {
                    if(!aBitmapEx.IsTransparent() && (!basegfx::fTools::equalZero(fShearX) || !basegfx::fTools::equalZero(fRotate)))
                    {
                        // parts will be uncovered, extend aBitmapEx with a mask bitmap
                        const Bitmap aContent(aBitmapEx.GetBitmap());
                        aBitmapEx = BitmapEx(aContent, Bitmap(aContent.GetSizePixel(), 1));
                    }

                    RenderBitmapPrimitive2D_self(*mpOutputDevice, aBitmapEx, aLocalTransform);
                }
            }
        }

        void VclProcessor2D::RenderRenderGraphicPrimitive2D(const primitive2d::RenderGraphicPrimitive2D& rRenderGraphicCandidate)
        {
            // create local transform
            basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rRenderGraphicCandidate.getTransform());
            vcl::RenderGraphic aRenderGraphic(rRenderGraphicCandidate.getRenderGraphic());
            bool bPainted(false);

            if(maBColorModifierStack.count())
            {
                // !!! TODO
                // aRenderGraphic = impModifyRenderGraphic(maBColorModifierStack, aRenderGraphic);

                if(aRenderGraphic.IsEmpty())
                {
                    // color gets completely replaced, get it
                    const basegfx::BColor aModifiedColor(maBColorModifierStack.getModifiedColor(basegfx::BColor()));
                    basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());
                    aPolygon.transform(aLocalTransform);

                    mpOutputDevice->SetFillColor(Color(aModifiedColor));
                    mpOutputDevice->SetLineColor();
                    mpOutputDevice->DrawPolygon(aPolygon);

                    bPainted = true;
                }
            }

            if(!bPainted)
            {
                // decompose matrix to check for shear, rotate and mirroring
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                aLocalTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                basegfx::B2DRange aOutlineRange(0.0, 0.0, 1.0, 1.0);

                if( basegfx::fTools::equalZero( fRotate ) )
                {
                    aOutlineRange.transform( aLocalTransform );
                }
                else
                {
                    // !!! TODO
                    // if rotated, create the unrotated output rectangle for the GraphicManager paint
                    /*
                    const basegfx::B2DHomMatrix aSimpleObjectMatrix(basegfx::tools::createScaleTranslateB2DHomMatrix(
                        fabs(aScale.getX()), fabs(aScale.getY()),
                        aTranslate.getX(), aTranslate.getY()));

                    aOutlineRange.transform(aSimpleObjectMatrix);
                    */
                }

                // prepare dest coordinates
                const Point                         aPoint( basegfx::fround(aOutlineRange.getMinX() ),
                                                            basegfx::fround(aOutlineRange.getMinY() ) );
                const Size                          aSize( basegfx::fround(aOutlineRange.getWidth() ),
                                                           basegfx::fround(aOutlineRange.getHeight() ) );
                const Size                          aSizePixel( mpOutputDevice->LogicToPixel( aSize ) );
                const vcl::RenderGraphicRasterizer  aRasterizer( aRenderGraphic );
                const BitmapEx                      aBitmapEx( aRasterizer.Rasterize( aSizePixel, fRotate, fShearX ) );

                if( !aBitmapEx.IsEmpty() )
                {
                    mpOutputDevice->DrawBitmapEx( aPoint, aSize, aBitmapEx );
                }
            }
        }

        void VclProcessor2D::RenderFillBitmapPrimitive2D(const primitive2d::FillBitmapPrimitive2D& rFillBitmapCandidate)
        {
            const attribute::FillBitmapAttribute& rFillBitmapAttribute(rFillBitmapCandidate.getFillBitmap());
            bool bPrimitiveAccepted(false);

            if(rFillBitmapAttribute.getTiling())
            {
                // decompose matrix to check for shear, rotate and mirroring
                basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rFillBitmapCandidate.getTransformation());
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                aLocalTransform.decompose(aScale, aTranslate, fRotate, fShearX);

                if(basegfx::fTools::equalZero(fRotate) && basegfx::fTools::equalZero(fShearX))
                {
                    // no shear or rotate, draw direct in pixel coordinates
                    bPrimitiveAccepted = true;
                    BitmapEx aBitmapEx(rFillBitmapAttribute.getBitmapEx());
                    bool bPainted(false);

                    if(maBColorModifierStack.count())
                    {
                        aBitmapEx = impModifyBitmapEx(maBColorModifierStack, aBitmapEx);

                        if(aBitmapEx.IsEmpty())
                        {
                            // color gets completely replaced, get it
                            const basegfx::BColor aModifiedColor(maBColorModifierStack.getModifiedColor(basegfx::BColor()));
                            basegfx::B2DPolygon aPolygon(basegfx::tools::createUnitPolygon());
                            aPolygon.transform(aLocalTransform);

                            mpOutputDevice->SetFillColor(Color(aModifiedColor));
                            mpOutputDevice->SetLineColor();
                            mpOutputDevice->DrawPolygon(aPolygon);

                            bPainted = true;
                        }
                    }

                    if(!bPainted)
                    {
                        const basegfx::B2DPoint aObjTopLeft(aTranslate.getX(), aTranslate.getY());
                        const basegfx::B2DPoint aObjBottomRight(aTranslate.getX() + aScale.getX(), aTranslate.getY() + aScale.getY());
                        const Point aObjTL(mpOutputDevice->LogicToPixel(Point((sal_Int32)aObjTopLeft.getX(), (sal_Int32)aObjTopLeft.getY())));
                        const Point aObjBR(mpOutputDevice->LogicToPixel(Point((sal_Int32)aObjBottomRight.getX(), (sal_Int32)aObjBottomRight.getY())));

                        const basegfx::B2DPoint aBmpTopLeft(aLocalTransform * rFillBitmapAttribute.getTopLeft());
                        const basegfx::B2DPoint aBmpBottomRight(aLocalTransform * basegfx::B2DPoint(rFillBitmapAttribute.getTopLeft() + rFillBitmapAttribute.getSize()));
                        const Point aBmpTL(mpOutputDevice->LogicToPixel(Point((sal_Int32)aBmpTopLeft.getX(), (sal_Int32)aBmpTopLeft.getY())));
                        const Point aBmpBR(mpOutputDevice->LogicToPixel(Point((sal_Int32)aBmpBottomRight.getX(), (sal_Int32)aBmpBottomRight.getY())));

                        sal_Int32 nOWidth(aObjBR.X() - aObjTL.X());
                        sal_Int32 nOHeight(aObjBR.Y() - aObjTL.Y());

                        // only do something when object has a size in discrete units
                        if(nOWidth > 0 && nOHeight > 0)
                        {
                            sal_Int32 nBWidth(aBmpBR.X() - aBmpTL.X());
                            sal_Int32 nBHeight(aBmpBR.Y() - aBmpTL.Y());

                            // only do something when bitmap fill has a size in discrete units
                            if(nBWidth > 0 && nBHeight > 0)
                            {
                                sal_Int32 nBLeft(aBmpTL.X());
                                sal_Int32 nBTop(aBmpTL.Y());

                                if(nBLeft > aObjTL.X())
                                {
                                    nBLeft -= ((nBLeft / nBWidth) + 1L) * nBWidth;
                                }

                                if(nBLeft + nBWidth <= aObjTL.X())
                                {
                                    nBLeft -= (nBLeft / nBWidth) * nBWidth;
                                }

                                if(nBTop > aObjTL.Y())
                                {
                                    nBTop -= ((nBTop / nBHeight) + 1L) * nBHeight;
                                }

                                if(nBTop + nBHeight <= aObjTL.Y())
                                {
                                    nBTop -= (nBTop / nBHeight) * nBHeight;
                                }

                                // nBWidth, nBHeight is the pixel size of the neede bitmap. To not need to scale it
                                // in vcl many times, create a size-optimized version
                                const Size aNeededBitmapSizePixel(nBWidth, nBHeight);

                                if(aNeededBitmapSizePixel != aBitmapEx.GetSizePixel())
                                {
                                    aBitmapEx.Scale(aNeededBitmapSizePixel);
                                }

                                // prepare OutDev
                                const Point aEmptyPoint(0, 0);
                                const Rectangle aVisiblePixel(aEmptyPoint, mpOutputDevice->GetOutputSizePixel());
                                const bool bWasEnabled(mpOutputDevice->IsMapModeEnabled());
                                mpOutputDevice->EnableMapMode(false);

                                for(sal_Int32 nXPos(nBLeft); nXPos < aObjTL.X() + nOWidth; nXPos += nBWidth)
                                {
                                    for(sal_Int32 nYPos(nBTop); nYPos < aObjTL.Y() + nOHeight; nYPos += nBHeight)
                                    {
                                        const Rectangle aOutRectPixel(Point(nXPos, nYPos), aNeededBitmapSizePixel);

                                        if(aOutRectPixel.IsOver(aVisiblePixel))
                                        {
                                            mpOutputDevice->DrawBitmapEx(aOutRectPixel.TopLeft(), aBitmapEx);
                                        }
                                    }
                                }

                                // restore OutDev
                                mpOutputDevice->EnableMapMode(bWasEnabled);
                            }
                        }
                    }
                }
            }

            if(!bPrimitiveAccepted)
            {
                // do not accept, use decomposition
                process(rFillBitmapCandidate.get2DDecomposition(getViewInformation2D()));
            }
        }

        // direct draw of gradient
        void VclProcessor2D::RenderPolyPolygonGradientPrimitive2D(const primitive2d::PolyPolygonGradientPrimitive2D& rPolygonCandidate)
        {
            const attribute::FillGradientAttribute& rGradient(rPolygonCandidate.getFillGradient());
            basegfx::BColor aStartColor(maBColorModifierStack.getModifiedColor(rGradient.getStartColor()));
            basegfx::BColor aEndColor(maBColorModifierStack.getModifiedColor(rGradient.getEndColor()));
            basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());

            if(aLocalPolyPolygon.count())
            {
                aLocalPolyPolygon.transform(maCurrentTransformation);

                if(aStartColor == aEndColor)
                {
                    // no gradient at all, draw as polygon in AA and non-AA case
                    mpOutputDevice->SetLineColor();
                    mpOutputDevice->SetFillColor(Color(aStartColor));
                    mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);
                }
                else if(getOptionsDrawinglayer().IsAntiAliasing())
                {
                    // For AA, direct render has to be avoided since it uses XOR maskings which will not
                    // work with AA. Instead, the decompose which uses MaskPrimitive2D with fillings is
                    // used
                    process(rPolygonCandidate.get2DDecomposition(getViewInformation2D()));
                }
                else
                {
                    impDrawGradientToOutDev(
                        *mpOutputDevice, aLocalPolyPolygon, rGradient.getStyle(), rGradient.getSteps(),
                        aStartColor, aEndColor, rGradient.getBorder(),
                        rGradient.getAngle(), rGradient.getOffsetX(), rGradient.getOffsetY(), false);
                }
            }
        }

        // direct draw of bitmap
        void VclProcessor2D::RenderPolyPolygonBitmapPrimitive2D(const primitive2d::PolyPolygonBitmapPrimitive2D& rPolygonCandidate)
        {
            bool bDone(false);
            const basegfx::B2DPolyPolygon& rPolyPolygon = rPolygonCandidate.getB2DPolyPolygon();

            if(rPolyPolygon.count())
            {
                const attribute::FillBitmapAttribute& rFillBitmapAttribute = rPolygonCandidate.getFillBitmap();
                const BitmapEx& rBitmapEx = rFillBitmapAttribute.getBitmapEx();

                if(rBitmapEx.IsEmpty())
                {
                    // empty bitmap, done
                    bDone = true;
                }
                else
                {
                    // try to catch cases where the bitmap will be color-modified to a single
                    // color (e.g. shadow). This would NOT be optimizable with an transparence channel
                    // at the Bitmap which we do not have here. When this should change, this
                    // optimization has to be reworked accordingly.
                    const sal_uInt32 nBColorModifierStackCount(maBColorModifierStack.count());

                    if(nBColorModifierStackCount)
                    {
                        const basegfx::BColorModifier& rTopmostModifier = maBColorModifierStack.getBColorModifier(nBColorModifierStackCount - 1);

                        if(basegfx::BCOLORMODIFYMODE_REPLACE == rTopmostModifier.getMode())
                        {
                            // the bitmap fill is in unified color, so we can replace it with
                            // a single polygon fill. The form of the fill depends on tiling
                            if(rFillBitmapAttribute.getTiling())
                            {
                                // with tiling, fill the whole PolyPolygon with the modifier color
                                basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolyPolygon);

                                aLocalPolyPolygon.transform(maCurrentTransformation);
                                mpOutputDevice->SetLineColor();
                                mpOutputDevice->SetFillColor(Color(rTopmostModifier.getBColor()));
                                mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);
                            }
                            else
                            {
                                // without tiling, only the area common to the bitmap tile and the
                                // PolyPolygon is filled. Create the bitmap tile area in object
                                // coordinates. For this, the object transformation needs to be created
                                // from the already scaled PolyPolygon. The tile area in object
                                // coordinates wil always be non-rotated, so it's not necessary to
                                // work with a polygon here
                                basegfx::B2DRange aTileRange(rFillBitmapAttribute.getTopLeft(),
                                    rFillBitmapAttribute.getTopLeft() + rFillBitmapAttribute.getSize());
                                const basegfx::B2DRange aPolyPolygonRange(rPolyPolygon.getB2DRange());
                                basegfx::B2DHomMatrix aNewObjectTransform;

                                aNewObjectTransform.set(0, 0, aPolyPolygonRange.getWidth());
                                aNewObjectTransform.set(1, 1, aPolyPolygonRange.getHeight());
                                aNewObjectTransform.set(0, 2, aPolyPolygonRange.getMinX());
                                aNewObjectTransform.set(1, 2, aPolyPolygonRange.getMinY());
                                aTileRange.transform(aNewObjectTransform);

                                // now clip the object polyPolygon against the tile range
                                // to get the common area (OR)
                                basegfx::B2DPolyPolygon aTarget = basegfx::tools::clipPolyPolygonOnRange(rPolyPolygon, aTileRange, true, false);

                                if(aTarget.count())
                                {
                                    aTarget.transform(maCurrentTransformation);
                                    mpOutputDevice->SetLineColor();
                                    mpOutputDevice->SetFillColor(Color(rTopmostModifier.getBColor()));
                                    mpOutputDevice->DrawPolyPolygon(aTarget);
                                }
                            }

                            bDone = true;
                        }
                    }
                }
            }
            else
            {
                // empty polyPolygon, done
                bDone = true;
            }

            if(!bDone)
            {
                // use default decomposition
                process(rPolygonCandidate.get2DDecomposition(getViewInformation2D()));
            }
        }

        // direct draw of PolyPolygon with color
        void VclProcessor2D::RenderPolyPolygonColorPrimitive2D(const primitive2d::PolyPolygonColorPrimitive2D& rPolygonCandidate)
        {
            const basegfx::BColor aPolygonColor(maBColorModifierStack.getModifiedColor(rPolygonCandidate.getBColor()));
            mpOutputDevice->SetFillColor(Color(aPolygonColor));
            mpOutputDevice->SetLineColor();

            basegfx::B2DPolyPolygon aLocalPolyPolygon(rPolygonCandidate.getB2DPolyPolygon());
            aLocalPolyPolygon.transform(maCurrentTransformation);

            static bool bCheckTrapezoidDecomposition(false);
            static bool bShowOutlinesThere(false);
            if(bCheckTrapezoidDecomposition)
            {
                // clip against discrete ViewPort
                const basegfx::B2DRange& rDiscreteViewport = getViewInformation2D().getDiscreteViewport();
                aLocalPolyPolygon = basegfx::tools::clipPolyPolygonOnRange(
                    aLocalPolyPolygon, rDiscreteViewport, true, false);

                if(aLocalPolyPolygon.count())
                {
                    // subdivide
                    aLocalPolyPolygon = basegfx::tools::adaptiveSubdivideByDistance(
                        aLocalPolyPolygon, 0.5);

                    // trapezoidize
                    basegfx::B2DTrapezoidVector aB2DTrapezoidVector;
                    basegfx::tools::trapezoidSubdivide(aB2DTrapezoidVector, aLocalPolyPolygon);

                    const sal_uInt32 nCount(aB2DTrapezoidVector.size());

                    if(nCount)
                    {
                        basegfx::BColor aInvPolygonColor(aPolygonColor);
                        aInvPolygonColor.invert();

                        for(sal_uInt32 a(0); a < nCount; a++)
                        {
                            const basegfx::B2DPolygon aTempPolygon(aB2DTrapezoidVector[a].getB2DPolygon());

                            if(bShowOutlinesThere)
                            {
                                mpOutputDevice->SetFillColor(Color(aPolygonColor));
                                mpOutputDevice->SetLineColor();
                            }

                            mpOutputDevice->DrawPolygon(aTempPolygon);

                            if(bShowOutlinesThere)
                            {
                                mpOutputDevice->SetFillColor();
                                mpOutputDevice->SetLineColor(Color(aInvPolygonColor));
                                mpOutputDevice->DrawPolyLine(aTempPolygon, 0.0);
                            }
                        }
                    }
                }
            }
            else
            {
                // remember that we enter a PolygonStrokePrimitive2D decomposition,
                // used for AA thick line drawing
                mnPolygonStrokePrimitive2D++;

                mpOutputDevice->DrawPolyPolygon(aLocalPolyPolygon);

                if(mnPolygonStrokePrimitive2D
                    && getOptionsDrawinglayer().IsAntiAliasing()
                    && (mpOutputDevice->GetAntialiasing() & ANTIALIASING_ENABLE_B2DDRAW))
                {
                    // when AA is on and this filled polygons are the result of stroked line geometry,
                    // draw the geometry once extra as lines to avoid AA 'gaps' between partial polygons
                    mpOutputDevice->SetFillColor();
                    mpOutputDevice->SetLineColor(Color(aPolygonColor));
                    const sal_uInt32 nCount(aLocalPolyPolygon.count());

                    for(sal_uInt32 a(0); a < nCount; a++)
                    {
                        mpOutputDevice->DrawPolyLine(aLocalPolyPolygon.getB2DPolygon(a), 0.0);
                    }
                }

                // leave PolygonStrokePrimitive2D
                mnPolygonStrokePrimitive2D--;
            }
        }

        // direct draw of MetaFile
        void VclProcessor2D::RenderMetafilePrimitive2D(const primitive2d::MetafilePrimitive2D& rMetaCandidate)
        {
            // decompose matrix to check for shear, rotate and mirroring
            basegfx::B2DHomMatrix aLocalTransform(maCurrentTransformation * rMetaCandidate.getTransform());
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;
            aLocalTransform.decompose(aScale, aTranslate, fRotate, fShearX);

            if(basegfx::fTools::less(aScale.getX(), 0.0) && basegfx::fTools::less(aScale.getY(), 0.0))
            {
                // #i102175# handle special case: If scale is negative in (x,y) (3rd quadrant), it can
                // be expressed as rotation by PI. This needs to be done for Metafiles since
                // these can be rotated, but not really mirrored
                aScale = basegfx::absolute(aScale);
                fRotate += F_PI;
            }

            // get BoundRect
            basegfx::B2DRange aOutlineRange(rMetaCandidate.getB2DRange(getViewInformation2D()));
            aOutlineRange.transform(maCurrentTransformation);

            // Due to the integer MapModes used from VCL aind inside MetaFiles errors of up to three
            // pixels in size may happen. As long as there is no better way (e.g. convert the MetaFile
            // to primitives) it is necessary to reduce maximum pixel size by 1 in X and Y and to use
            // the inner pixel bounds accordingly (ceil resp. floor). This will also be done for logic
            // units e.g. when creating a new MetaFile, but since much huger value ranges are used
            // there typically will be okay for this compromize.
            Rectangle aDestRectView(
                // !!CAUTION!! Here, ceil and floor are exchanged BY PURPOSE, do NOT copy when
                // looking for a standard conversion to rectangle (!)
                (sal_Int32)ceil(aOutlineRange.getMinX()), (sal_Int32)ceil(aOutlineRange.getMinY()),
                (sal_Int32)floor(aOutlineRange.getMaxX()), (sal_Int32)floor(aOutlineRange.getMaxY()));

            // get metafile (copy it)
            GDIMetaFile aMetaFile;

            if(maBColorModifierStack.count())
            {
                const basegfx::BColor aRGBBaseColor(0, 0, 0);
                const basegfx::BColor aRGBColor(maBColorModifierStack.getModifiedColor(aRGBBaseColor));
                aMetaFile = rMetaCandidate.getMetaFile().GetMonochromeMtf(Color(aRGBColor));
            }
            else
            {
                aMetaFile = rMetaCandidate.getMetaFile();
            }

            // rotation
            if(!basegfx::fTools::equalZero(fRotate))
            {
                // #i103530#
                // MetaFile::Rotate has no input parameter check, so the parameter needs to be
                // well-aligned to the old range [0..3600] 10th degrees with inverse orientation
                sal_Int16 nRotation((sal_Int16)((fRotate / F_PI180) * -10.0));

                while(nRotation < 0)
                    nRotation += 3600;

                while(nRotation >= 3600)
                    nRotation -= 3600;

                aMetaFile.Rotate(nRotation);
            }

            // Prepare target output size
            Size aDestSize(aDestRectView.GetSize());

            if(aDestSize.getWidth() && aDestSize.getHeight())
            {
                // Get preferred Metafile output size. When it's very equal to the output size, it's probably
                // a rounding error somewhere, so correct it to get a 1:1 output without single pixel scalings
                // of the Metafile (esp. for contaned Bitmaps, e.g 3D charts)
                const Size aPrefSize(mpOutputDevice->LogicToPixel(aMetaFile.GetPrefSize(), aMetaFile.GetPrefMapMode()));

                if(aPrefSize.getWidth() && (aPrefSize.getWidth() - 1 == aDestSize.getWidth() || aPrefSize.getWidth() + 1 == aDestSize.getWidth()))
                {
                    aDestSize.setWidth(aPrefSize.getWidth());
                }

                if(aPrefSize.getHeight() && (aPrefSize.getHeight() - 1 == aDestSize.getHeight() || aPrefSize.getHeight() + 1 == aDestSize.getHeight()))
                {
                    aDestSize.setHeight(aPrefSize.getHeight());
                }

                // paint it
                aMetaFile.WindStart();
                aMetaFile.Play(mpOutputDevice, aDestRectView.TopLeft(), aDestSize);
            }
        }

        // mask group. Force output to VDev and create mask from given mask
        void VclProcessor2D::RenderMaskPrimitive2DPixel(const primitive2d::MaskPrimitive2D& rMaskCandidate)
        {
            if(rMaskCandidate.getChildren().hasElements())
            {
                basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());

                if(aMask.count())
                {
                    aMask.transform(maCurrentTransformation);
                    const basegfx::B2DRange aRange(basegfx::tools::getRange(aMask));
                    impBufferDevice aBufferDevice(*mpOutputDevice, aRange, true);

                    if(aBufferDevice.isVisible())
                    {
                        // remember last OutDev and set to content
                        OutputDevice* pLastOutputDevice = mpOutputDevice;
                        mpOutputDevice = &aBufferDevice.getContent();

                        // paint to it
                        process(rMaskCandidate.getChildren());

                        // back to old OutDev
                        mpOutputDevice = pLastOutputDevice;

                        // draw mask
                        if(getOptionsDrawinglayer().IsAntiAliasing())
                        {
                            // with AA, use 8bit AlphaMask to get nice borders
                            VirtualDevice& rTransparence = aBufferDevice.getTransparence();
                            rTransparence.SetLineColor();
                            rTransparence.SetFillColor(COL_BLACK);
                            rTransparence.DrawPolyPolygon(aMask);

                            // dump buffer to outdev
                            aBufferDevice.paint();
                        }
                        else
                        {
                            // No AA, use 1bit mask
                            VirtualDevice& rMask = aBufferDevice.getMask();
                            rMask.SetLineColor();
                            rMask.SetFillColor(COL_BLACK);
                            rMask.DrawPolyPolygon(aMask);

                            // dump buffer to outdev
                            aBufferDevice.paint();
                        }
                    }
                }
            }
        }

        // modified color group. Force output to unified color.
        void VclProcessor2D::RenderModifiedColorPrimitive2D(const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate)
        {
            if(rModifiedCandidate.getChildren().hasElements())
            {
                maBColorModifierStack.push(rModifiedCandidate.getColorModifier());
                process(rModifiedCandidate.getChildren());
                maBColorModifierStack.pop();
            }
        }

        // unified sub-transparence. Draw to VDev first.
        void VclProcessor2D::RenderUnifiedTransparencePrimitive2D(const primitive2d::UnifiedTransparencePrimitive2D& rTransCandidate)
        {
            static bool bForceToDecomposition(false);

            if(rTransCandidate.getChildren().hasElements())
            {
                if(bForceToDecomposition)
                {
                    // use decomposition
                    process(rTransCandidate.get2DDecomposition(getViewInformation2D()));
                }
                else
                {
                    if(0.0 == rTransCandidate.getTransparence())
                    {
                        // no transparence used, so just use the content
                        process(rTransCandidate.getChildren());
                    }
                    else if(rTransCandidate.getTransparence() > 0.0 && rTransCandidate.getTransparence() < 1.0)
                    {
                        // transparence is in visible range
                        basegfx::B2DRange aRange(primitive2d::getB2DRangeFromPrimitive2DSequence(rTransCandidate.getChildren(), getViewInformation2D()));
                        aRange.transform(maCurrentTransformation);
                        impBufferDevice aBufferDevice(*mpOutputDevice, aRange, true);

                        if(aBufferDevice.isVisible())
                        {
                            // remember last OutDev and set to content
                            OutputDevice* pLastOutputDevice = mpOutputDevice;
                            mpOutputDevice = &aBufferDevice.getContent();

                            // paint content to it
                            process(rTransCandidate.getChildren());

                            // back to old OutDev
                            mpOutputDevice = pLastOutputDevice;

                            // dump buffer to outdev using given transparence
                            aBufferDevice.paint(rTransCandidate.getTransparence());
                        }
                    }
                }
            }
        }

        // sub-transparence group. Draw to VDev first.
        void VclProcessor2D::RenderTransparencePrimitive2D(const primitive2d::TransparencePrimitive2D& rTransCandidate)
        {
            if(rTransCandidate.getChildren().hasElements())
            {
                basegfx::B2DRange aRange(primitive2d::getB2DRangeFromPrimitive2DSequence(rTransCandidate.getChildren(), getViewInformation2D()));
                aRange.transform(maCurrentTransformation);
                impBufferDevice aBufferDevice(*mpOutputDevice, aRange, true);

                if(aBufferDevice.isVisible())
                {
                    // remember last OutDev and set to content
                    OutputDevice* pLastOutputDevice = mpOutputDevice;
                    mpOutputDevice = &aBufferDevice.getContent();

                    // paint content to it
                    process(rTransCandidate.getChildren());

                    // set to mask
                    mpOutputDevice = &aBufferDevice.getTransparence();

                    // when painting transparence masks, reset the color stack
                    basegfx::BColorModifierStack aLastBColorModifierStack(maBColorModifierStack);
                    maBColorModifierStack = basegfx::BColorModifierStack();

                    // paint mask to it (always with transparence intensities, evtl. with AA)
                    process(rTransCandidate.getTransparence());

                    // back to old color stack
                    maBColorModifierStack = aLastBColorModifierStack;

                    // back to old OutDev
                    mpOutputDevice = pLastOutputDevice;

                    // dump buffer to outdev
                    aBufferDevice.paint();
                }
            }
        }

        // transform group.
        void VclProcessor2D::RenderTransformPrimitive2D(const primitive2d::TransformPrimitive2D& rTransformCandidate)
        {
            // remember current transformation and ViewInformation
            const basegfx::B2DHomMatrix aLastCurrentTransformation(maCurrentTransformation);
            const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

            // create new transformations for CurrentTransformation
            // and for local ViewInformation2D
            maCurrentTransformation = maCurrentTransformation * rTransformCandidate.getTransformation();
            const geometry::ViewInformation2D aViewInformation2D(
                getViewInformation2D().getObjectTransformation() * rTransformCandidate.getTransformation(),
                getViewInformation2D().getViewTransformation(),
                getViewInformation2D().getViewport(),
                getViewInformation2D().getVisualizedPage(),
                getViewInformation2D().getViewTime(),
                getViewInformation2D().getExtendedInformationSequence());
            updateViewInformation(aViewInformation2D);

            // proccess content
            process(rTransformCandidate.getChildren());

            // restore transformations
            maCurrentTransformation = aLastCurrentTransformation;
            updateViewInformation(aLastViewInformation2D);
        }

        // new XDrawPage for ViewInformation2D
        void VclProcessor2D::RenderPagePreviewPrimitive2D(const primitive2d::PagePreviewPrimitive2D& rPagePreviewCandidate)
        {
            // remember current transformation and ViewInformation
            const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

            // create new local ViewInformation2D
            const geometry::ViewInformation2D aViewInformation2D(
                getViewInformation2D().getObjectTransformation(),
                getViewInformation2D().getViewTransformation(),
                getViewInformation2D().getViewport(),
                rPagePreviewCandidate.getXDrawPage(),
                getViewInformation2D().getViewTime(),
                getViewInformation2D().getExtendedInformationSequence());
            updateViewInformation(aViewInformation2D);

            // proccess decomposed content
            process(rPagePreviewCandidate.get2DDecomposition(getViewInformation2D()));

            // restore transformations
            updateViewInformation(aLastViewInformation2D);
        }

        // marker
        void VclProcessor2D::RenderMarkerArrayPrimitive2D(const primitive2d::MarkerArrayPrimitive2D& rMarkArrayCandidate)
        {
            static bool bCheckCompleteMarkerDecompose(false);
            if(bCheckCompleteMarkerDecompose)
            {
                process(rMarkArrayCandidate.get2DDecomposition(getViewInformation2D()));
                return;
            }

            // get data
            const std::vector< basegfx::B2DPoint >& rPositions = rMarkArrayCandidate.getPositions();
            const sal_uInt32 nCount(rPositions.size());

            if(nCount && !rMarkArrayCandidate.getMarker().IsEmpty())
            {
                // get pixel size
                const BitmapEx& rMarker(rMarkArrayCandidate.getMarker());
                const Size aBitmapSize(rMarker.GetSizePixel());

                if(aBitmapSize.Width() && aBitmapSize.Height())
                {
                    // get discrete half size
                    const basegfx::B2DVector aDiscreteHalfSize(
                        (aBitmapSize.getWidth() - 1.0) * 0.5,
                        (aBitmapSize.getHeight() - 1.0) * 0.5);
                    const bool bWasEnabled(mpOutputDevice->IsMapModeEnabled());

                    // do not forget evtl. moved origin in target device MapMode when
                    // switching it off; it would be missing and lead to wrong positions.
                    // All his could be done using logic sizes and coordinates, too, but
                    // we want a 1:1 bitmap rendering here, so it's more safe and faster
                    // to work with switching off MapMode usage completely.
                    const Point aOrigin(mpOutputDevice->GetMapMode().GetOrigin());

                    mpOutputDevice->EnableMapMode(false);

                    for(std::vector< basegfx::B2DPoint >::const_iterator aIter(rPositions.begin()); aIter != rPositions.end(); ++aIter)
                    {
                        const basegfx::B2DPoint aDiscreteTopLeft((maCurrentTransformation * (*aIter)) - aDiscreteHalfSize);
                        const Point aDiscretePoint(basegfx::fround(aDiscreteTopLeft.getX()), basegfx::fround(aDiscreteTopLeft.getY()));

                        mpOutputDevice->DrawBitmapEx(aDiscretePoint + aOrigin, rMarker);
                    }

                    mpOutputDevice->EnableMapMode(bWasEnabled);
                }
            }
        }

        // point
        void VclProcessor2D::RenderPointArrayPrimitive2D(const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate)
        {
            const std::vector< basegfx::B2DPoint >& rPositions = rPointArrayCandidate.getPositions();
            const basegfx::BColor aRGBColor(maBColorModifierStack.getModifiedColor(rPointArrayCandidate.getRGBColor()));
            const Color aVCLColor(aRGBColor);

            for(std::vector< basegfx::B2DPoint >::const_iterator aIter(rPositions.begin()); aIter != rPositions.end(); ++aIter)
            {
                const basegfx::B2DPoint aViewPosition(maCurrentTransformation * (*aIter));
                const Point aPos(basegfx::fround(aViewPosition.getX()), basegfx::fround(aViewPosition.getY()));

                mpOutputDevice->DrawPixel(aPos, aVCLColor);
            }
        }

        void VclProcessor2D::RenderPolygonStrokePrimitive2D(const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokeCandidate)
        {
            // #i101491# method restructured to clearly use the DrawPolyLine
            // calls starting from a deined line width
            const attribute::LineAttribute& rLineAttribute = rPolygonStrokeCandidate.getLineAttribute();
            const double fLineWidth(rLineAttribute.getWidth());
            bool bDone(false);

            if(basegfx::fTools::more(fLineWidth, 0.0))
            {
                const basegfx::B2DVector aDiscreteUnit(maCurrentTransformation * basegfx::B2DVector(fLineWidth, 0.0));
                const double fDiscreteLineWidth(aDiscreteUnit.getLength());
                const attribute::StrokeAttribute& rStrokeAttribute = rPolygonStrokeCandidate.getStrokeAttribute();
                const basegfx::BColor aHairlineColor(maBColorModifierStack.getModifiedColor(rLineAttribute.getColor()));
                basegfx::B2DPolyPolygon aHairlinePolyPolygon;

                mpOutputDevice->SetLineColor(Color(aHairlineColor));
                mpOutputDevice->SetFillColor();

                if(0.0 == rStrokeAttribute.getFullDotDashLen())
                {
                    // no line dashing, just copy
                    aHairlinePolyPolygon.append(rPolygonStrokeCandidate.getB2DPolygon());
                }
                else
                {
                    // else apply LineStyle
                    basegfx::tools::applyLineDashing(rPolygonStrokeCandidate.getB2DPolygon(),
                        rStrokeAttribute.getDotDashArray(),
                        &aHairlinePolyPolygon, 0, rStrokeAttribute.getFullDotDashLen());
                }

                const sal_uInt32 nCount(aHairlinePolyPolygon.count());

                if(nCount)
                {
                    const bool bAntiAliased(getOptionsDrawinglayer().IsAntiAliasing());
                    aHairlinePolyPolygon.transform(maCurrentTransformation);

                    for(sal_uInt32 a(0); a < nCount; a++)
                    {
                        basegfx::B2DPolygon aCandidate(aHairlinePolyPolygon.getB2DPolygon(a));

                        if(bAntiAliased)
                        {
                            if(basegfx::fTools::lessOrEqual(fDiscreteLineWidth, 1.0))
                            {
                                // line in range ]0.0 .. 1.0[
                                // paint as simple hairline
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);
                                bDone = true;
                            }
                            else if(basegfx::fTools::lessOrEqual(fDiscreteLineWidth, 2.0))
                            {
                                // line in range [1.0 .. 2.0[
                                // paint as 2x2 with dynamic line distance
                                basegfx::B2DHomMatrix aMat;
                                const double fDistance(fDiscreteLineWidth - 1.0);
                                const double fHalfDistance(fDistance * 0.5);

                                aMat.set(0, 2, -fHalfDistance);
                                aMat.set(1, 2, -fHalfDistance);
                                aCandidate.transform(aMat);
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, fDistance);
                                aMat.set(1, 2, 0.0);
                                aCandidate.transform(aMat);
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, 0.0);
                                aMat.set(1, 2, fDistance);
                                aCandidate.transform(aMat);
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, -fDistance);
                                aMat.set(1, 2, 0.0);
                                aCandidate.transform(aMat);
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);
                                bDone = true;
                            }
                            else if(basegfx::fTools::lessOrEqual(fDiscreteLineWidth, 3.0))
                            {
                                // line in range [2.0 .. 3.0]
                                // paint as cross in a 3x3  with dynamic line distance
                                basegfx::B2DHomMatrix aMat;
                                const double fDistance((fDiscreteLineWidth - 1.0) * 0.5);

                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, -fDistance);
                                aMat.set(1, 2, 0.0);
                                aCandidate.transform(aMat);
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, fDistance);
                                aMat.set(1, 2, -fDistance);
                                aCandidate.transform(aMat);
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, fDistance);
                                aMat.set(1, 2, fDistance);
                                aCandidate.transform(aMat);
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, -fDistance);
                                aMat.set(1, 2, fDistance);
                                aCandidate.transform(aMat);
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);
                                bDone = true;
                            }
                            else
                            {
                                // #i101491# line width above 3.0
                            }
                        }
                        else
                        {
                            if(basegfx::fTools::lessOrEqual(fDiscreteLineWidth, 1.5))
                            {
                                // line width below 1.5, draw the basic hairline polygon
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);
                                bDone = true;
                            }
                            else if(basegfx::fTools::lessOrEqual(fDiscreteLineWidth, 2.5))
                            {
                                // line width is in range ]1.5 .. 2.5], use four hairlines
                                // drawn in a square
                                basegfx::B2DHomMatrix aMat;
                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, 1.0);
                                aMat.set(1, 2, 0.0);
                                aCandidate.transform(aMat);

                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, 0.0);
                                aMat.set(1, 2, 1.0);
                                aCandidate.transform(aMat);

                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);

                                aMat.set(0, 2, -1.0);
                                aMat.set(1, 2, 0.0);
                                aCandidate.transform(aMat);

                                mpOutputDevice->DrawPolyLine(aCandidate, 0.0);
                                bDone = true;
                            }
                            else
                            {
                                // #i101491# line width is above 2.5
                            }
                        }

                        if(!bDone && rPolygonStrokeCandidate.getB2DPolygon().count() > 1000)
                        {
                            // #i101491# If the polygon complexity uses more than a given amount, do
                            // use OuputDevice::DrawPolyLine directly; this will avoid buffering all
                            // decompositions in primtives (memory) and fallback to old line painting
                            // for very complex polygons, too
                            mpOutputDevice->DrawPolyLine(aCandidate, fDiscreteLineWidth, rLineAttribute.getLineJoin());
                            bDone = true;
                        }
                    }
                }
            }

            if(!bDone)
            {
                // remeber that we enter a PolygonStrokePrimitive2D decomposition,
                // used for AA thick line drawing
                mnPolygonStrokePrimitive2D++;

                // line width is big enough for standard filled polygon visualisation or zero
                process(rPolygonStrokeCandidate.get2DDecomposition(getViewInformation2D()));

                // leave PolygonStrokePrimitive2D
                mnPolygonStrokePrimitive2D--;
            }
        }

        void VclProcessor2D::RenderEpsPrimitive2D(const primitive2d::EpsPrimitive2D& rEpsPrimitive2D)
        {
            // The new decomposition of Metafiles made it necessary to add an Eps
            // primitive to handle embedded Eps data. On some devices, this can be
            // painted directly (mac, printer).
            // To be able to handle the replacement correctly, i need to handle it myself
            // since DrawEPS will not be able e.g. to rotate the replacement. To be able
            // to do that, i added a boolean return to OutputDevice::DrawEPS(..)
            // to know when EPS was handled directly already.
            basegfx::B2DRange aRange(0.0, 0.0, 1.0, 1.0);
            aRange.transform(maCurrentTransformation * rEpsPrimitive2D.getEpsTransform());

            if(!aRange.isEmpty())
            {
                const Rectangle aRectangle(
                    (sal_Int32)floor(aRange.getMinX()), (sal_Int32)floor(aRange.getMinY()),
                    (sal_Int32)ceil(aRange.getMaxX()), (sal_Int32)ceil(aRange.getMaxY()));

                if(!aRectangle.IsEmpty())
                {
                    // try to paint EPS directly without fallback visualisation
                    const bool bEPSPaintedDirectly(mpOutputDevice->DrawEPS(
                        aRectangle.TopLeft(),
                        aRectangle.GetSize(),
                        rEpsPrimitive2D.getGfxLink(),
                        0));

                    if(!bEPSPaintedDirectly)
                    {
                        // use the decomposition which will correctly handle the
                        // fallback visualisation using full transformation (e.g. rotation)
                        process(rEpsPrimitive2D.get2DDecomposition(getViewInformation2D()));
                    }
                }
            }
        }

        void VclProcessor2D::adaptLineToFillDrawMode() const
        {
            const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());

            if(nOriginalDrawMode & (DRAWMODE_BLACKLINE|DRAWMODE_GRAYLINE|DRAWMODE_GHOSTEDLINE|DRAWMODE_WHITELINE|DRAWMODE_SETTINGSLINE))
            {
                sal_uInt32 nAdaptedDrawMode(nOriginalDrawMode);

                if(nOriginalDrawMode & DRAWMODE_BLACKLINE)
                {
                    nAdaptedDrawMode |= DRAWMODE_BLACKFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_BLACKFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_GRAYLINE)
                {
                    nAdaptedDrawMode |= DRAWMODE_GRAYFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_GRAYFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_GHOSTEDLINE)
                {
                    nAdaptedDrawMode |= DRAWMODE_GHOSTEDFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_GHOSTEDFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_WHITELINE)
                {
                    nAdaptedDrawMode |= DRAWMODE_WHITEFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_WHITEFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_SETTINGSLINE)
                {
                    nAdaptedDrawMode |= DRAWMODE_SETTINGSFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_SETTINGSFILL;
                }

                mpOutputDevice->SetDrawMode(nAdaptedDrawMode);
            }
        }

        void VclProcessor2D::adaptTextToFillDrawMode() const
        {
            const sal_uInt32 nOriginalDrawMode(mpOutputDevice->GetDrawMode());
            if(nOriginalDrawMode & (DRAWMODE_BLACKTEXT|DRAWMODE_GRAYTEXT|DRAWMODE_GHOSTEDTEXT|DRAWMODE_WHITETEXT|DRAWMODE_SETTINGSTEXT))
            {
                sal_uInt32 nAdaptedDrawMode(nOriginalDrawMode);

                if(nOriginalDrawMode & DRAWMODE_BLACKTEXT)
                {
                    nAdaptedDrawMode |= DRAWMODE_BLACKFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_BLACKFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_GRAYTEXT)
                {
                    nAdaptedDrawMode |= DRAWMODE_GRAYFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_GRAYFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_GHOSTEDTEXT)
                {
                    nAdaptedDrawMode |= DRAWMODE_GHOSTEDFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_GHOSTEDFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_WHITETEXT)
                {
                    nAdaptedDrawMode |= DRAWMODE_WHITEFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_WHITEFILL;
                }

                if(nOriginalDrawMode & DRAWMODE_SETTINGSTEXT)
                {
                    nAdaptedDrawMode |= DRAWMODE_SETTINGSFILL;
                }
                else
                {
                    nAdaptedDrawMode &= ~DRAWMODE_SETTINGSFILL;
                }

                mpOutputDevice->SetDrawMode(nAdaptedDrawMode);
            }
        }

        //////////////////////////////////////////////////////////////////////////////
        // process support

        VclProcessor2D::VclProcessor2D(
            const geometry::ViewInformation2D& rViewInformation,
            OutputDevice& rOutDev)
        :   BaseProcessor2D(rViewInformation),
            mpOutputDevice(&rOutDev),
            maBColorModifierStack(),
            maCurrentTransformation(),
            maDrawinglayerOpt(),
            mnPolygonStrokePrimitive2D(0)
        {
            // set digit language, derived from SvtCTLOptions to have the correct
            // number display for arabic/hindi numerals
            rOutDev.SetDigitLanguage(drawinglayer::detail::getDigitLanguage());
        }

        VclProcessor2D::~VclProcessor2D()
        {
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
