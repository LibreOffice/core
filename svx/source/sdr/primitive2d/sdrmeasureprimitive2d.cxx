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

#include <sdr/primitive2d/sdrmeasureprimitive2d.hxx>
#include <sdr/primitive2d/sdrdecompositiontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <sdr/attribute/sdrtextattribute.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <rtl/ref.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <osl/diagnose.h>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        Primitive2DReference SdrMeasurePrimitive2D::impCreatePart(
            const attribute::SdrLineAttribute& rLineAttribute,
            const basegfx::B2DHomMatrix& rObjectMatrix,
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            bool bLeftActive,
            bool bRightActive) const
        {
            const attribute::SdrLineStartEndAttribute& rLineStartEnd = getSdrLSTAttribute().getLineStartEnd();
            basegfx::B2DPolygon aPolygon;

            aPolygon.append(rStart);
            aPolygon.append(rEnd);
            aPolygon.transform(rObjectMatrix);

            if(rLineStartEnd.isDefault() || (!bLeftActive && !bRightActive))
            {
                return createPolygonLinePrimitive(
                    aPolygon,
                    rLineAttribute,
                    attribute::SdrLineStartEndAttribute());
            }

            if(bLeftActive && bRightActive)
            {
                return createPolygonLinePrimitive(
                    aPolygon,
                    rLineAttribute,
                    rLineStartEnd);
            }

            const basegfx::B2DPolyPolygon aEmpty;
            const attribute::SdrLineStartEndAttribute aLineStartEnd(
                bLeftActive ? rLineStartEnd.getStartPolyPolygon() : aEmpty, bRightActive ? rLineStartEnd.getEndPolyPolygon() : aEmpty,
                bLeftActive ? rLineStartEnd.getStartWidth() : 0.0, bRightActive ? rLineStartEnd.getEndWidth() : 0.0,
                bLeftActive && rLineStartEnd.isStartActive(), bRightActive && rLineStartEnd.isEndActive(),
                bLeftActive && rLineStartEnd.isStartCentered(), bRightActive && rLineStartEnd.isEndCentered());

            return createPolygonLinePrimitive(
                aPolygon,
                rLineAttribute,
                aLineStartEnd);
        }

        void SdrMeasurePrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& aViewInformation) const
        {
            Primitive2DContainer aRetval;
            rtl::Reference<SdrBlockTextPrimitive2D> xBlockText;
            basegfx::B2DRange aTextRange;
            const basegfx::B2DVector aLine(getEnd() - getStart());
            const double fDistance(aLine.getLength());
            const double fAngle(atan2(aLine.getY(), aLine.getX()));
            bool bAutoUpsideDown(false);
            const attribute::SdrTextAttribute rTextAttribute = getSdrLSTAttribute().getText();
            const basegfx::B2DHomMatrix aObjectMatrix(
                basegfx::utils::createShearXRotateTranslateB2DHomMatrix(0.0, fAngle, getStart()));

            // prepare text, but do not add yet; it needs to be aligned to
            // the line geometry
            if(!rTextAttribute.isDefault())
            {
                basegfx::B2DHomMatrix aTextMatrix;
                double fTestAngle(fAngle);

                if(getTextRotation())
                {
                    aTextMatrix.rotate(-M_PI_2);
                    fTestAngle -= (M_PI_2);

                    if(getTextAutoAngle() && fTestAngle < -M_PI)
                    {
                        fTestAngle += 2 * M_PI;
                    }
                }

                if(getTextAutoAngle())
                {
                    if(fTestAngle > (M_PI / 4.0) || fTestAngle < (-M_PI * (3.0 / 4.0)))
                    {
                        bAutoUpsideDown = true;
                    }
                }

                // create primitive and get text range
                xBlockText = new SdrBlockTextPrimitive2D(
                    &rTextAttribute.getSdrText(),
                    rTextAttribute.getOutlinerParaObject(),
                    aTextMatrix,
                    SDRTEXTHORZADJUST_CENTER,
                    SDRTEXTVERTADJUST_CENTER,
                    rTextAttribute.isScroll(),
                    false,
                    false,
                    false);

                aTextRange = xBlockText->getB2DRange(aViewInformation);
            }

            // prepare line attribute and result
            double fTextX;
            double fTextY;
            {
                const attribute::SdrLineAttribute rLineAttribute(getSdrLSTAttribute().getLine());
                bool bArrowsOutside(false);
                bool bMainLineSplitted(false);
                const attribute::SdrLineStartEndAttribute& rLineStartEnd = getSdrLSTAttribute().getLineStartEnd();
                double fStartArrowW(0.0);
                double fStartArrowH(0.0);
                double fEndArrowW(0.0);
                double fEndArrowH(0.0);

                if(!rLineStartEnd.isDefault())
                {
                    if(rLineStartEnd.isStartActive())
                    {
                        const basegfx::B2DRange aArrowRange(basegfx::utils::getRange(rLineStartEnd.getStartPolyPolygon()));
                        fStartArrowW = rLineStartEnd.getStartWidth();
                        fStartArrowH = aArrowRange.getHeight() * fStartArrowW / aArrowRange.getWidth();

                        if(rLineStartEnd.isStartCentered())
                        {
                            fStartArrowH *= 0.5;
                        }
                    }

                    if(rLineStartEnd.isEndActive())
                    {
                        const basegfx::B2DRange aArrowRange(basegfx::utils::getRange(rLineStartEnd.getEndPolyPolygon()));
                        fEndArrowW = rLineStartEnd.getEndWidth();
                        fEndArrowH = aArrowRange.getHeight() * fEndArrowW / aArrowRange.getWidth();

                        if(rLineStartEnd.isEndCentered())
                        {
                            fEndArrowH *= 0.5;
                        }
                    }
                }

                const double fSpaceNeededByArrows(fStartArrowH + fEndArrowH + ((fStartArrowW + fEndArrowW) * 0.5));
                const double fArrowsOutsideLen((fStartArrowH + fEndArrowH + fStartArrowW + fEndArrowW) * 0.5);
                const double fHalfLineWidth(rLineAttribute.getWidth() * 0.5);

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

                    aRetval.push_back(impCreatePart(rLineAttribute, aObjectMatrix, aMainLeftLeft, aMainLeft, false, true));
                    aRetval.push_back(impCreatePart(rLineAttribute, aObjectMatrix, aMainRight, aMainRightRight, true, false));

                    if(!bMainLineSplitted || MEASURETEXTPOSITION_CENTERED != eHorizontal)
                    {
                        aRetval.push_back(impCreatePart(rLineAttribute, aObjectMatrix, aMainLeft, aMainRight, false, false));
                    }
                }
                else
                {
                    if(bMainLineSplitted)
                    {
                        const double fHalfLength((fDistance - (aTextRange.getWidth() + (fStartArrowH + fEndArrowH) * 0.25)) * 0.5);
                        const basegfx::B2DPoint aMainInnerLeft(aMainLeft.getX() + fHalfLength, aMainLeft.getY());
                        const basegfx::B2DPoint aMainInnerRight(aMainRight.getX() - fHalfLength, aMainRight.getY());

                        aRetval.push_back(impCreatePart(rLineAttribute, aObjectMatrix, aMainLeft, aMainInnerLeft, true, false));
                        aRetval.push_back(impCreatePart(rLineAttribute, aObjectMatrix, aMainInnerRight, aMainRight, false, true));
                    }
                    else
                    {
                        aRetval.push_back(impCreatePart(rLineAttribute, aObjectMatrix, aMainLeft, aMainRight, true, true));
                    }
                }

                // left/right help line value preparation
                const double fTopEdge(getBelow() ? getUpper() + getDistance() : -getUpper() - getDistance());
                const double fBottomLeft(getBelow() ? getLower() - getLeftDelta() : getLeftDelta() - getLower());
                const double fBottomRight(getBelow() ? getLower() - getRightDelta() : getRightDelta() - getLower());

                // left help line
                const basegfx::B2DPoint aLeftUp(0.0, fTopEdge);
                const basegfx::B2DPoint aLeftDown(0.0, fBottomLeft);

                aRetval.push_back(impCreatePart(rLineAttribute, aObjectMatrix, aLeftDown, aLeftUp, false, false));

                // right help line
                const basegfx::B2DPoint aRightUp(fDistance, fTopEdge);
                const basegfx::B2DPoint aRightDown(fDistance, fBottomRight);

                aRetval.push_back(impCreatePart(rLineAttribute, aObjectMatrix, aRightDown, aRightUp, false, false));

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

                    if(!rTextAttribute.isDefault())
                    {
                        fTextX -= rTextAttribute.getTextRightDistance();
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

                    if(!rTextAttribute.isDefault())
                    {
                        fTextX += rTextAttribute.getTextLeftDistance();
                    }
                }
                else // MEASURETEXTPOSITION_CENTERED
                {
                    // centered
                    fTextX = aMainLeft.getX() + ((fDistance - aTextRange.getWidth()) * 0.5);

                    if(!rTextAttribute.isDefault())
                    {
                        fTextX += (rTextAttribute.getTextLeftDistance() - rTextAttribute.getTextRightDistance()) / 2L;
                    }
                }

                // text vertical position
                if(MEASURETEXTPOSITION_NEGATIVE == eVertical)
                {
                    // top
                    const double fSmall(fArrowsOutsideLen * 0.10);
                    fTextY = aMainLeft.getY() - (aTextRange.getHeight() + fSmall + fHalfLineWidth);

                    if(!rTextAttribute.isDefault())
                    {
                        fTextY -= rTextAttribute.getTextLowerDistance();
                    }
                }
                else if(MEASURETEXTPOSITION_POSITIVE == eVertical)
                {
                    // bottom
                    const double fSmall(fArrowsOutsideLen * 0.10);
                    fTextY = aMainLeft.getY() + (fSmall + fHalfLineWidth);

                    if(!rTextAttribute.isDefault())
                    {
                        fTextY += rTextAttribute.getTextUpperDistance();
                    }
                }
                else // MEASURETEXTPOSITION_CENTERED
                {
                    // centered
                    fTextY = aMainLeft.getY() - (aTextRange.getHeight() * 0.5);

                    if(!rTextAttribute.isDefault())
                    {
                        fTextY += (rTextAttribute.getTextUpperDistance() - rTextAttribute.getTextLowerDistance()) / 2L;
                    }
                }
            }

            if(getSdrLSTAttribute().getLine().isDefault())
            {
                // embed line geometry to invisible (100% transparent) line group for HitTest
                Primitive2DReference xHiddenLines(new HiddenGeometryPrimitive2D(std::move(aRetval)));

                aRetval = Primitive2DContainer { xHiddenLines };
            }

            if(xBlockText.is())
            {
                // create transformation to text primitive end position
                basegfx::B2DHomMatrix aChange;

                // handle auto text rotation
                if(bAutoUpsideDown)
                {
                    aChange.rotate(M_PI);
                }

                // move from aTextRange.TopLeft to fTextX, fTextY
                aChange.translate(fTextX - aTextRange.getMinX(), fTextY - aTextRange.getMinY());

                // apply object matrix
                aChange *= aObjectMatrix;

                // apply to existing text primitive
                rtl::Reference<SdrTextPrimitive2D> pNewBlockText = xBlockText->createTransformedClone(aChange);
                OSL_ENSURE(pNewBlockText, "SdrMeasurePrimitive2D::create2DDecomposition: Could not create transformed clone of text primitive (!)");
                xBlockText.clear();

                // add to local primitives
                aRetval.push_back(pNewBlockText);
            }

            // add shadow
            if(!getSdrLSTAttribute().getShadow().isDefault())
            {
                aRetval = createEmbeddedShadowPrimitive(
                    std::move(aRetval),
                    getSdrLSTAttribute().getShadow());
            }

            rContainer.insert(rContainer.end(), aRetval.begin(), aRetval.end());
        }

        SdrMeasurePrimitive2D::SdrMeasurePrimitive2D(
            const attribute::SdrLineEffectsTextAttribute& rSdrLSTAttribute,
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
        :   maSdrLSTAttribute(rSdrLSTAttribute),
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
                const SdrMeasurePrimitive2D& rCompare = static_cast<const SdrMeasurePrimitive2D&>(rPrimitive);

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
        sal_uInt32 SdrMeasurePrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SDRMEASUREPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
