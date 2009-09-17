/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrmeasureprimitive2d.cxx,v $
 *
 * $Revision: 1.2.18.1 $
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

#include "precompiled_svx.hxx"
#include <svx/sdr/primitive2d/sdrmeasureprimitive2d.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <svx/sdr/attribute/sdrtextattribute.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/hittestprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DReference SdrMeasurePrimitive2D::impCreatePart(
            const basegfx::B2DHomMatrix& rObjectMatrix,
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            bool bLeftActive,
            bool bRightActive) const
        {
            basegfx::B2DPolygon aPolygon;
            aPolygon.append(rStart);
            aPolygon.append(rEnd);

            if(!getSdrLSTAttribute().getLineStartEnd() || (!bLeftActive && !bRightActive))
            {
                return createPolygonLinePrimitive(aPolygon, rObjectMatrix, *getSdrLSTAttribute().getLine(), 0L);
            }

            if(bLeftActive && bRightActive)
            {
                return createPolygonLinePrimitive(aPolygon, rObjectMatrix, *getSdrLSTAttribute().getLine(), getSdrLSTAttribute().getLineStartEnd());
            }

            const attribute::SdrLineStartEndAttribute* pLineStartEnd = getSdrLSTAttribute().getLineStartEnd();
            const basegfx::B2DPolyPolygon aEmpty;
            const attribute::SdrLineStartEndAttribute aLineStartEnd(
                bLeftActive ? pLineStartEnd->getStartPolyPolygon() : aEmpty, bRightActive ? pLineStartEnd->getEndPolyPolygon() : aEmpty,
                bLeftActive ? pLineStartEnd->getStartWidth() : 0.0, bRightActive ? pLineStartEnd->getEndWidth() : 0.0,
                bLeftActive ? pLineStartEnd->isStartActive() : false, bRightActive ? pLineStartEnd->isEndActive() : false,
                bLeftActive ? pLineStartEnd->isStartCentered() : false, bRightActive? pLineStartEnd->isEndCentered() : false);

            return createPolygonLinePrimitive(aPolygon, rObjectMatrix, *getSdrLSTAttribute().getLine(), &aLineStartEnd);
        }

        Primitive2DSequence SdrMeasurePrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DSequence aRetval;
            SdrBlockTextPrimitive2D* pBlockText = 0L;
            basegfx::B2DRange aTextRange;
            double fTextX((getStart().getX() + getEnd().getX()) * 0.5);
            double fTextY((getStart().getX() + getEnd().getX()) * 0.5);
            const basegfx::B2DVector aLine(getEnd() - getStart());
            const double fDistance(aLine.getLength());
            const double fAngle(atan2(aLine.getY(), aLine.getX()));
            bool bAutoUpsideDown(false);
            const attribute::SdrTextAttribute* pTextAttribute = getSdrLSTAttribute().getText();

            basegfx::B2DHomMatrix aObjectMatrix;
            aObjectMatrix.rotate(fAngle);
            aObjectMatrix.translate(getStart().getX(), getStart().getY());

            if(pTextAttribute)
            {
                basegfx::B2DHomMatrix aTextMatrix;
                double fTestAngle(fAngle);

                if(getTextRotation())
                {
                    aTextMatrix.rotate(-90.0 * F_PI180);
                    fTestAngle -= (90.0 * F_PI180);

                    if(getTextAutoAngle() && fTestAngle < -F_PI)
                    {
                        fTestAngle += F_2PI;
                    }
                }

                if(getTextAutoAngle())
                {
                    if(fTestAngle > (F_PI / 4.0) || fTestAngle < (-F_PI * (3.0 / 4.0)))
                    {
                        bAutoUpsideDown = true;
                    }
                }

                // create primitive and get text range
                pBlockText = new SdrBlockTextPrimitive2D(
                    &pTextAttribute->getSdrText(),
                    pTextAttribute->getOutlinerParaObject(),
                    aTextMatrix,
                    pTextAttribute->isScroll(),
                    false,
                    false);
                aTextRange = pBlockText->getB2DRange(aViewInformation);
            }

            // prepare line attribute and result
            const attribute::SdrLineAttribute* pLineAttribute(getSdrLSTAttribute().getLine());

            if(!pLineAttribute)
            {
                // if initially no line is defined, create one for HitTest and BoundRect
                pLineAttribute = new attribute::SdrLineAttribute(basegfx::BColor(0.0, 0.0, 0.0));
            }

            {
                bool bArrowsOutside(false);
                bool bMainLineSplitted(false);
                const attribute::SdrLineStartEndAttribute* pLineStartEnd = getSdrLSTAttribute().getLineStartEnd();
                double fStartArrowW(0.0);
                double fStartArrowH(0.0);
                double fEndArrowW(0.0);
                double fEndArrowH(0.0);

                if(pLineStartEnd)
                {
                    if(pLineStartEnd->isStartActive())
                    {
                        const basegfx::B2DRange aArrowRange(basegfx::tools::getRange(pLineStartEnd->getStartPolyPolygon()));
                        fStartArrowW = pLineStartEnd->getStartWidth();
                        fStartArrowH = aArrowRange.getHeight() * fStartArrowW / aArrowRange.getWidth();

                        if(pLineStartEnd->isStartCentered())
                        {
                            fStartArrowH *= 0.5;
                        }
                    }

                    if(pLineStartEnd->isEndActive())
                    {
                        const basegfx::B2DRange aArrowRange(basegfx::tools::getRange(pLineStartEnd->getEndPolyPolygon()));
                        fEndArrowW = pLineStartEnd->getEndWidth();
                        fEndArrowH = aArrowRange.getHeight() * fEndArrowW / aArrowRange.getWidth();

                        if(pLineStartEnd->isEndCentered())
                        {
                            fEndArrowH *= 0.5;
                        }
                    }
                }

                const double fSpaceNeededByArrows(fStartArrowH + fEndArrowH + ((fStartArrowW + fEndArrowW) * 0.5));
                const double fArrowsOutsideLen((fStartArrowH + fEndArrowH + fStartArrowW + fEndArrowW) * 0.5);
                const double fHalfLineWidth(pLineAttribute->getWidth() * 0.5);

                if(fSpaceNeededByArrows > fDistance)
                {
                    bArrowsOutside = true;
                }

                MeasureTextPosition eHorizontal(getHorizontal());
                MeasureTextPosition eVertical(getVertical());

                if(MEASURETEXTPOSITION_AUTOMATIC == eVertical)
                {
                    eVertical = MEASURETEXTPOSITION_NEGATIVE;
                }

                if(MEASURETEXTPOSITION_CENTERED == eVertical)
                {
                    bMainLineSplitted = true;
                }

                if(MEASURETEXTPOSITION_AUTOMATIC == eHorizontal)
                {
                    if(aTextRange.getWidth() > fDistance)
                    {
                        eHorizontal = MEASURETEXTPOSITION_NEGATIVE;
                    }
                    else
                    {
                        eHorizontal = MEASURETEXTPOSITION_CENTERED;
                    }

                    if(bMainLineSplitted)
                    {
                        if(aTextRange.getWidth() + fSpaceNeededByArrows > fDistance)
                        {
                            bArrowsOutside = true;
                        }
                    }
                    else
                    {
                        const double fSmallArrowNeed(fStartArrowH + fEndArrowH + ((fStartArrowW + fEndArrowW) * 0.125));

                        if(aTextRange.getWidth() + fSmallArrowNeed > fDistance)
                        {
                            bArrowsOutside = true;
                        }
                    }
                }

                if(MEASURETEXTPOSITION_CENTERED != eHorizontal)
                {
                    bArrowsOutside = true;
                }

                // switch text above/below?
                if(getBelow() || (bAutoUpsideDown && !getTextRotation()))
                {
                    if(MEASURETEXTPOSITION_NEGATIVE == eVertical)
                    {
                        eVertical = MEASURETEXTPOSITION_POSITIVE;
                    }
                    else if(MEASURETEXTPOSITION_POSITIVE == eVertical)
                    {
                        eVertical = MEASURETEXTPOSITION_NEGATIVE;
                    }
                }

                const double fMainLineOffset(getBelow() ? getDistance() : -getDistance());
                const basegfx::B2DPoint aMainLeft(0.0, fMainLineOffset);
                const basegfx::B2DPoint aMainRight(fDistance, fMainLineOffset);

                // main line
                if(bArrowsOutside)
                {
                    double fLenLeft(fArrowsOutsideLen);
                    double fLenRight(fArrowsOutsideLen);

                    if(!bMainLineSplitted)
                    {
                        if(MEASURETEXTPOSITION_NEGATIVE == eHorizontal)
                        {
                            fLenLeft = fStartArrowH + aTextRange.getWidth();
                        }
                        else if(MEASURETEXTPOSITION_POSITIVE == eHorizontal)
                        {
                            fLenRight = fEndArrowH + aTextRange.getWidth();
                        }
                    }

                    const basegfx::B2DPoint aMainLeftLeft(aMainLeft.getX() - fLenLeft, aMainLeft.getY());
                    const basegfx::B2DPoint aMainRightRight(aMainRight.getX() + fLenRight, aMainRight.getY());

                    appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, impCreatePart(aObjectMatrix, aMainLeftLeft, aMainLeft, false, true));
                    appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, impCreatePart(aObjectMatrix, aMainRight, aMainRightRight, true, false));

                    if(!bMainLineSplitted || MEASURETEXTPOSITION_CENTERED != eHorizontal)
                    {
                        appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, impCreatePart(aObjectMatrix, aMainLeft, aMainRight, false, false));
                    }
                }
                else
                {
                    if(bMainLineSplitted)
                    {
                        const double fHalfLength((fDistance - (aTextRange.getWidth() + (fStartArrowH + fEndArrowH) * 0.25)) * 0.5);
                        const basegfx::B2DPoint aMainInnerLeft(aMainLeft.getX() + fHalfLength, aMainLeft.getY());
                        const basegfx::B2DPoint aMainInnerRight(aMainRight.getX() - fHalfLength, aMainRight.getY());

                        appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, impCreatePart(aObjectMatrix, aMainLeft, aMainInnerLeft, true, false));
                        appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, impCreatePart(aObjectMatrix, aMainInnerRight, aMainRight, false, true));
                    }
                    else
                    {
                        appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, impCreatePart(aObjectMatrix, aMainLeft, aMainRight, true, true));
                    }
                }

                // left/right help line value preparation
                const double fTopEdge(getBelow() ? getUpper() + getDistance() : -getUpper() - getDistance());
                const double fBottomLeft(getBelow() ? getLower() - getLeftDelta() : getLeftDelta() - getLower());
                const double fBottomRight(getBelow() ? getLower() - getRightDelta() : getRightDelta() - getLower());

                // left help line
                const basegfx::B2DPoint aLeftUp(0.0, fTopEdge);
                const basegfx::B2DPoint aLeftDown(0.0, fBottomLeft);

                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, impCreatePart(aObjectMatrix, aLeftDown, aLeftUp, false, false));

                // right help line
                const basegfx::B2DPoint aRightUp(fDistance, fTopEdge);
                const basegfx::B2DPoint aRightDown(fDistance, fBottomRight);

                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, impCreatePart(aObjectMatrix, aRightDown, aRightUp, false, false));

                // text horizontal position
                if(MEASURETEXTPOSITION_NEGATIVE == eHorizontal)
                {
                    // left
                    const double fSmall(fArrowsOutsideLen * 0.18);
                    fTextX = aMainLeft.getX() - (fStartArrowH + aTextRange.getWidth() + fSmall + fHalfLineWidth);

                    if(bMainLineSplitted)
                    {
                        fTextX -= (fArrowsOutsideLen - fStartArrowH);
                    }

                    if(pTextAttribute)
                    {
                        fTextX -= pTextAttribute->getTextRightDistance();
                    }
                }
                else if(MEASURETEXTPOSITION_POSITIVE == eHorizontal)
                {
                    // right
                    const double fSmall(fArrowsOutsideLen * 0.18);
                    fTextX = aMainRight.getX() + (fEndArrowH + fSmall + fHalfLineWidth);

                    if(bMainLineSplitted)
                    {
                        fTextX += (fArrowsOutsideLen - fEndArrowH);
                    }

                    if(pTextAttribute)
                    {
                        fTextX += pTextAttribute->getTextLeftDistance();
                    }
                }
                else // MEASURETEXTPOSITION_CENTERED
                {
                    // centered
                    fTextX = aMainLeft.getX() + ((fDistance - aTextRange.getWidth()) * 0.5);

                    if(pTextAttribute)
                    {
                        fTextX += (pTextAttribute->getTextLeftDistance() - pTextAttribute->getTextRightDistance()) / 2L;
                    }
                }

                // text vertical position
                if(MEASURETEXTPOSITION_NEGATIVE == eVertical)
                {
                    // top
                    const double fSmall(fArrowsOutsideLen * 0.10);
                    fTextY = aMainLeft.getY() - (aTextRange.getHeight() + fSmall + fHalfLineWidth);

                    if(pTextAttribute)
                    {
                        fTextY -= pTextAttribute->getTextLowerDistance();
                    }
                }
                else if(MEASURETEXTPOSITION_POSITIVE == eVertical)
                {
                    // bottom
                    const double fSmall(fArrowsOutsideLen * 0.10);
                    fTextY = aMainLeft.getY() + (fSmall + fHalfLineWidth);

                    if(pTextAttribute)
                    {
                        fTextY += pTextAttribute->getTextUpperDistance();
                    }
                }
                else // MEASURETEXTPOSITION_CENTERED
                {
                    // centered
                    fTextY = aMainLeft.getY() - (aTextRange.getHeight() * 0.5);

                    if(pTextAttribute)
                    {
                        fTextY += (pTextAttribute->getTextUpperDistance() - pTextAttribute->getTextLowerDistance()) / 2L;
                    }
                }
            }

            if(!getSdrLSTAttribute().getLine())
            {
                // embed line geometry to invisible line group
                const Primitive2DReference xHiddenLines(new HitTestPrimitive2D(aRetval));
                aRetval = Primitive2DSequence(&xHiddenLines, 1);

                // delete temporary LineAttribute again
                delete pLineAttribute;
            }

            if(pBlockText)
            {
                // create transformation to text primitive end position
                basegfx::B2DHomMatrix aChange;

                // handle auto text rotation
                if(bAutoUpsideDown)
                {
                    aChange.rotate(F_PI);
                }

                // move from aTextRange.TopLeft to fTextX, fTextY
                aChange.translate(fTextX - aTextRange.getMinX(), fTextY - aTextRange.getMinY());

                // apply object matrix
                aChange *= aObjectMatrix;

                // apply to existing text primitive
                SdrTextPrimitive2D* pNewBlockText = pBlockText->createTransformedClone(aChange);
                OSL_ENSURE(pNewBlockText, "SdrMeasurePrimitive2D::create2DDecomposition: Could not create transformed clone of text primitive (!)");
                delete pBlockText;

                // add to local primitives
                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, Primitive2DReference(pNewBlockText));
            }

            // add shadow
            if(getSdrLSTAttribute().getShadow())
            {
                aRetval = createEmbeddedShadowPrimitive(aRetval, *getSdrLSTAttribute().getShadow());
            }

            return aRetval;
        }

        SdrMeasurePrimitive2D::SdrMeasurePrimitive2D(
            const attribute::SdrLineShadowTextAttribute& rSdrLSTAttribute,
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            MeasureTextPosition eHorizontal,
            MeasureTextPosition eVertical,
            double fDistance,
            double fUpper,
            double fLower,
            double fLeftDelta,
            double fRightDelta,
            bool bBelow,
            bool bTextRotation,
            bool bTextAutoAngle)
        :   BufferedDecompositionPrimitive2D(),
            maSdrLSTAttribute(rSdrLSTAttribute),
            maStart(rStart),
            maEnd(rEnd),
            meHorizontal(eHorizontal),
            meVertical(eVertical),
            mfDistance(fDistance),
            mfUpper(fUpper),
            mfLower(fLower),
            mfLeftDelta(fLeftDelta),
            mfRightDelta(fRightDelta),
            mbBelow(bBelow),
            mbTextRotation(bTextRotation),
            mbTextAutoAngle(bTextAutoAngle)
        {
        }

        bool SdrMeasurePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrMeasurePrimitive2D& rCompare = (SdrMeasurePrimitive2D&)rPrimitive;

                return (getStart() == rCompare.getStart()
                    && getEnd() == rCompare.getEnd()
                    && getHorizontal() == rCompare.getHorizontal()
                    && getVertical() == rCompare.getVertical()
                    && getDistance() == rCompare.getDistance()
                    && getUpper() == rCompare.getUpper()
                    && getLower() == rCompare.getLower()
                    && getLeftDelta() == rCompare.getLeftDelta()
                    && getRightDelta() == rCompare.getRightDelta()
                    && getBelow() == rCompare.getBelow()
                    && getTextRotation() == rCompare.getTextRotation()
                    && getTextAutoAngle() == rCompare.getTextAutoAngle()
                    && getSdrLSTAttribute() == rCompare.getSdrLSTAttribute());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrMeasurePrimitive2D, PRIMITIVE2D_ID_SDRMEASUREPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
