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

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <svtools/borderhelper.hxx>
#include <editeng/borderline.hxx>
#include <android/compatibility.hxx>

#include <algorithm>
#include <cmath>
#include <numeric>


namespace drawinglayer
{
    namespace primitive2d
    {
        BorderLine::BorderLine(
            double fWidth,
            const basegfx::BColor& rRGBColor,
            double fExtendStart,
            double fExtendEnd)
        :   mfWidth(fWidth),
            maRGBColor(rRGBColor),
            mfExtendStart(fExtendStart),
            mfExtendEnd(fExtendEnd)
        {
        }

        BorderLine::~BorderLine()
        {
        }

        bool BorderLine::operator==(const BorderLine& rBorderLine) const
        {
            return getWidth() == rBorderLine.getWidth()
                && getRGBColor() == rBorderLine.getRGBColor()
                && getExtendStart() == rBorderLine.getExtendStart()
                && getExtendEnd() == rBorderLine.getExtendEnd();
        }

        // helper to add a centered, maybe stroked line primitive to rContainer
        void addPolygonStrokePrimitive2D(
            Primitive2DContainer& rContainer,
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            const attribute::LineAttribute& rLineAttribute,
            const attribute::StrokeAttribute & rStrokeAttribute)
        {
            basegfx::B2DPolygon aPolygon;

            aPolygon.append(rStart);
            aPolygon.append(rEnd);

            if (rStrokeAttribute.isDefault())
            {
                rContainer.push_back(
                    new PolygonStrokePrimitive2D(
                        aPolygon,
                        rLineAttribute));
            }
            else
            {
                rContainer.push_back(
                    new PolygonStrokePrimitive2D(
                        aPolygon,
                        rLineAttribute,
                        rStrokeAttribute));
            }
        }

        void BorderLinePrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if (!getStart().equal(getEnd()))
            {
                // get data and vectors
                basegfx::B2DVector aVector(getEnd() - getStart());
                aVector.normalize();
                const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));
                static double fPatScFact(10.0); // 10.0 multiply, see old code
                const std::vector<double> aDashing(svtools::GetLineDashing(getStyle(), getPatternScale() * fPatScFact));
                const attribute::StrokeAttribute aStrokeAttribute(aDashing);

                if (3 == getBorderLines().size())
                {
                    // double line with gap. Use mfDiscreteGapDistance (see get2DDecomposition) as distance.
                    // That value is prepared to be at least one pixel (discrete unit) so that the
                    // decomposition is view-dependent in this cases
                    const BorderLine& rLeft(getBorderLines()[0]);
                    const BorderLine& rGap(getBorderLines()[1]);
                    const BorderLine& rRight(getBorderLines()[2]);
                    const double fFullWidth(rLeft.getWidth() + mfDiscreteGapDistance + rRight.getWidth());

                    {
                        // inside line (left of vector). Create stroke primitive centered on left line width
                        const double fDeltaY((rLeft.getWidth() - fFullWidth) * 0.5);
                        const basegfx::B2DVector aDeltaY(aPerpendicular * fDeltaY);
                        const basegfx::B2DPoint aStart(getStart() - (aVector * rLeft.getExtendStart()) + aDeltaY);
                        const basegfx::B2DPoint aEnd(getEnd() + (aVector * rLeft.getExtendEnd()) + aDeltaY);
                        const attribute::LineAttribute aLineAttribute(rLeft.getRGBColor(), rLeft.getWidth());

                        addPolygonStrokePrimitive2D(
                            rContainer,
                            aStart,
                            aEnd,
                            aLineAttribute,
                            aStrokeAttribute);
                    }

                    if (hasGapColor())
                    {
                        // gap (if visible, found practical usage in Writer MultiColorBorderLines).
                        // Create stroke primitive on vector with given color centered on gap position
                        const double fDeltaY(((fFullWidth - mfDiscreteGapDistance) * 0.5) - rRight.getWidth());
                        const basegfx::B2DVector aDeltaY(aPerpendicular * fDeltaY);
                        const basegfx::B2DPoint aStart(getStart() - (aVector * rGap.getExtendStart()) + aDeltaY);
                        const basegfx::B2DPoint aEnd(getEnd() + (aVector * rGap.getExtendEnd()) + aDeltaY);
                        const attribute::LineAttribute aLineAttribute(rGap.getRGBColor(), mfDiscreteGapDistance);

                        addPolygonStrokePrimitive2D(
                            rContainer,
                            aStart,
                            aEnd,
                            aLineAttribute,
                            aStrokeAttribute);
                    }

                    {
                        // outside line (right of vector). Create stroke primitive centered on right line width
                        const double fDeltaY((fFullWidth - rRight.getWidth()) * 0.5);
                        const basegfx::B2DVector aDeltaY(aPerpendicular * fDeltaY);
                        const basegfx::B2DPoint aStart(getStart() - (aVector * rRight.getExtendStart()) + aDeltaY);
                        const basegfx::B2DPoint aEnd(getEnd() + (aVector * rRight.getExtendEnd()) + aDeltaY);
                        const attribute::LineAttribute aLineAttribute(rRight.getRGBColor(), rRight.getWidth());

                        addPolygonStrokePrimitive2D(
                            rContainer,
                            aStart,
                            aEnd,
                            aLineAttribute,
                            aStrokeAttribute);
                    }
                }
                else
                {
                    // single line, only inside values used, no vertical offsets
                    const BorderLine& rBorderLine(getBorderLines()[0]);
                    const attribute::LineAttribute aLineAttribute(rBorderLine.getRGBColor(), rBorderLine.getWidth());

                    addPolygonStrokePrimitive2D(
                        rContainer,
                        getStart() - (aVector * rBorderLine.getExtendStart()),
                        getEnd() + (aVector * rBorderLine.getExtendEnd()),
                        aLineAttribute,
                        aStrokeAttribute);
                }
            }
        }

        bool BorderLinePrimitive2D::isHorizontalOrVertical(const geometry::ViewInformation2D& rViewInformation) const
        {
            if (!getStart().equal(getEnd()))
            {
                const basegfx::B2DHomMatrix& rOTVT = rViewInformation.getObjectToViewTransformation();
                const basegfx::B2DVector aVector(rOTVT * getEnd() - rOTVT * getStart());

                return basegfx::fTools::equalZero(aVector.getX()) || basegfx::fTools::equalZero(aVector.getY());
            }

            return false;
        }

        BorderLinePrimitive2D::BorderLinePrimitive2D(
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            const BorderLine& rBorderLine,
            SvxBorderLineStyle nStyle,
            double fPatternScale)
        :   BufferedDecompositionPrimitive2D(),
            maStart(rStart),
            maEnd(rEnd),
            maBorderLines(),
            mbHasGapColor(false),
            mnStyle(nStyle),
            mfPatternScale(fPatternScale),
            mfDiscreteGapDistance(0.0)
        {
            maBorderLines.push_back(rBorderLine);
        }

        BorderLinePrimitive2D::BorderLinePrimitive2D(
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            const BorderLine& rLeft,
            const BorderLine& rGap,
            const BorderLine& rRight,
            bool bHasGapColor,
            SvxBorderLineStyle nStyle,
            double fPatternScale)
        :   BufferedDecompositionPrimitive2D(),
            maStart(rStart),
            maEnd(rEnd),
            maBorderLines(),
            mbHasGapColor(bHasGapColor),
            mnStyle(nStyle),
            mfPatternScale(fPatternScale),
            mfDiscreteGapDistance(0.0)
        {
            maBorderLines.push_back(rLeft);
            maBorderLines.push_back(rGap);
            maBorderLines.push_back(rRight);
        }

        bool BorderLinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const BorderLinePrimitive2D& rCompare = static_cast<const BorderLinePrimitive2D&>(rPrimitive);

                if (getStart() == rCompare.getStart()
                    && getEnd() == rCompare.getEnd()
                    && hasGapColor() == rCompare.hasGapColor()
                    && getStyle() == rCompare.getStyle()
                    && getPatternScale() == rCompare.getPatternScale())
                {
                    if (getBorderLines().size() == rCompare.getBorderLines().size())
                    {
                        for (size_t a(0); a < getBorderLines().size(); a++)
                        {
                            if (!(getBorderLines()[a] == rCompare.getBorderLines()[a]))
                            {
                                return false;
                            }
                        }
                    }
                }
            }

            return false;
        }

        void BorderLinePrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard(m_aMutex);

            if (!getStart().equal(getEnd()) && 3 == getBorderLines().size())
            {
                // Double line with gap. In this case, we want to be view-dependent.
                // Get the current DiscreteUnit, look at X and Y and use the maximum
                const basegfx::B2DVector aDiscreteVector(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 1.0));
                const double fDiscreteUnit(std::min(fabs(aDiscreteVector.getX()), fabs(aDiscreteVector.getY())));

                // When discrete unit is bigger than distance (distance is less than one pixel),
                // force distance to one pixel. Or expressed different, do not let the distance
                // get smaller than one pixel. This is done for screen rendering and compatibility.
                // This can also be done using DiscreteMetricDependentPrimitive2D as base class
                // for this class, but specialization is better here for later buffering (only
                // do this when 'double line with gap')
                const double fDistance(getBorderLines()[1].getWidth());
                const double fNewDiscreteDistance(std::max(fDiscreteUnit, fDistance));

                if (!rtl::math::approxEqual(fNewDiscreteDistance, mfDiscreteGapDistance))
                {
                    if (!getBuffered2DDecomposition().empty())
                    {
                        // conditions of last local decomposition have changed, delete
                        const_cast< BorderLinePrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DContainer());
                    }

                    // remember value for usage in create2DDecomposition
                    const_cast< BorderLinePrimitive2D* >(this)->mfDiscreteGapDistance = fNewDiscreteDistance;
                }
            }

            // call base implementation
            BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(BorderLinePrimitive2D, PRIMITIVE2D_ID_BORDERLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
