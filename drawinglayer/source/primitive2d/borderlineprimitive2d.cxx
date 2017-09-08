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
            const drawinglayer::attribute::LineAttribute& rLineAttribute,
            double fStartLeft,
            double fStartRight,
            double fEndLeft,
            double fEndRight)
        :   maLineAttribute(rLineAttribute),
            mfStartLeft(fStartLeft),
            mfStartRight(fStartRight),
            mfEndLeft(fEndLeft),
            mfEndRight(fEndRight),
            mbIsGap(false)
        {
        }

        BorderLine::BorderLine(
            double fWidth)
        :   maLineAttribute(basegfx::BColor(), fWidth),
            mfStartLeft(0.0),
            mfStartRight(0.0),
            mfEndLeft(0.0),
            mfEndRight(0.0),
            mbIsGap(true)
        {
        }

        BorderLine::~BorderLine()
        {
        }

        bool BorderLine::operator==(const BorderLine& rBorderLine) const
        {
            return getLineAttribute() == rBorderLine.getLineAttribute()
                && getStartLeft() == rBorderLine.getStartLeft()
                && getStartRight() == rBorderLine.getStartRight()
                && getEndLeft() == rBorderLine.getEndLeft()
                && getEndRight() == rBorderLine.getEndRight()
                && isGap() == rBorderLine.isGap();
        }

        double BorderLine::getAdaptedWidth(double fMinWidth) const
        {
            if(isGap())
            {
                return std::max(getLineAttribute().getWidth(), fMinWidth);
            }
            else
            {
                return getLineAttribute().getWidth();
            }
        }

        // helper to add a centered, maybe stroked line primitive to rContainer
        void addPolygonStrokePrimitive2D(
            Primitive2DContainer& rContainer,
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            const attribute::LineAttribute& rLineAttribute,
            const attribute::StrokeAttribute& rStrokeAttribute)
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

        double BorderLinePrimitive2D::getFullWidth() const
        {
            double fRetval(0.0);

            for(const auto& candidate : maBorderLines)
            {
                fRetval += candidate.getAdaptedWidth(mfSmallestAllowedDiscreteGapDistance);
            }

            return fRetval;
        }

        void BorderLinePrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if (!getStart().equal(getEnd()) && !getBorderLines().empty())
            {
                // get data and vectors
                basegfx::B2DVector aVector(getEnd() - getStart());
                aVector.normalize();
                const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));
                const double fFullWidth(getFullWidth());
                double fOffset(fFullWidth * -0.5);

                for(const auto& candidate : maBorderLines)
                {
                    const double fWidth(candidate.getAdaptedWidth(mfSmallestAllowedDiscreteGapDistance) * 0.5);

                    if(!candidate.isGap())
                    {
                        const basegfx::B2DVector aDeltaY(aPerpendicular * (fOffset + (fWidth * 0.5)));
                        const basegfx::B2DPoint aStart(getStart() - (aVector * candidate.getStartAverage()) + aDeltaY);
                        const basegfx::B2DPoint aEnd(getEnd() + (aVector * candidate.getEndAverage()) + aDeltaY);

                        addPolygonStrokePrimitive2D(
                            rContainer,
                            aStart,
                            aEnd,
                            candidate.getLineAttribute(),
                            getStrokeAttribute());
                    }

                    fOffset += fWidth;
                }
            }
        }




        //         static double fPatScFact(10.0); // 10.0 multiply, see old code
        //         const std::vector<double> aDashing(svtools::GetLineDashing(getStyle(), getPatternScale() * fPatScFact));
        //         const attribute::StrokeAttribute aStrokeAttribute(aDashing);

        //         if (3 == getBorderLines().size())
        //         {
        //             // double line with gap. Use mfSmallestAllowedDiscreteGapDistance (see get2DDecomposition) as distance.
        //             // That value is prepared to be at least one pixel (discrete unit) so that the
        //             // decomposition is view-dependent in this cases
        //             const BorderLine& rLeft(getBorderLines()[0]);
        //             const BorderLine& rGap(getBorderLines()[1]);
        //             const BorderLine& rRight(getBorderLines()[2]);
        //             const double fFullWidth(rLeft.getWidth() + mfSmallestAllowedDiscreteGapDistance + rRight.getWidth());

        //             {
        //                 // inside line (left of vector). Create stroke primitive centered on left line width
        //                 const double fDeltaY((rLeft.getWidth() - fFullWidth) * 0.5);
        //                 const basegfx::B2DVector aDeltaY(aPerpendicular * fDeltaY);
        //                 const basegfx::B2DPoint aStart(getStart() - (aVector * rLeft.getBorderLineExtend().getStartAverage()) + aDeltaY);
        //                 const basegfx::B2DPoint aEnd(getEnd() + (aVector * rLeft.getBorderLineExtend().getEndAverage()) + aDeltaY);
        //                 const attribute::LineAttribute aLineAttribute(rLeft.getRGBColor(), rLeft.getWidth());

        //                 addPolygonStrokePrimitive2D(
        //                     rContainer,
        //                     aStart,
        //                     aEnd,
        //                     aLineAttribute,
        //                     aStrokeAttribute);
        //             }

        //             if (hasGapColor())
        //             {
        //                 // gap (if visible, found practical usage in Writer MultiColorBorderLines).
        //                 // Create stroke primitive on vector with given color centered on gap position
        //                 const double fDeltaY(((fFullWidth - mfSmallestAllowedDiscreteGapDistance) * 0.5) - rRight.getWidth());
        //                 const basegfx::B2DVector aDeltaY(aPerpendicular * fDeltaY);
        //                 const basegfx::B2DPoint aStart(getStart() - (aVector * rGap.getBorderLineExtend().getStartAverage()) + aDeltaY);
        //                 const basegfx::B2DPoint aEnd(getEnd() + (aVector * rGap.getBorderLineExtend().getEndAverage()) + aDeltaY);
        //                 const attribute::LineAttribute aLineAttribute(rGap.getRGBColor(), mfSmallestAllowedDiscreteGapDistance);

        //                 addPolygonStrokePrimitive2D(
        //                     rContainer,
        //                     aStart,
        //                     aEnd,
        //                     aLineAttribute,
        //                     aStrokeAttribute);
        //             }

        //             {
        //                 // outside line (right of vector). Create stroke primitive centered on right line width
        //                 const double fDeltaY((fFullWidth - rRight.getWidth()) * 0.5);
        //                 const basegfx::B2DVector aDeltaY(aPerpendicular * fDeltaY);
        //                 const basegfx::B2DPoint aStart(getStart() - (aVector * rRight.getBorderLineExtend().getStartAverage()) + aDeltaY);
        //                 const basegfx::B2DPoint aEnd(getEnd() + (aVector * rRight.getBorderLineExtend().getEndAverage()) + aDeltaY);
        //                 const attribute::LineAttribute aLineAttribute(rRight.getRGBColor(), rRight.getWidth());

        //                 addPolygonStrokePrimitive2D(
        //                     rContainer,
        //                     aStart,
        //                     aEnd,
        //                     aLineAttribute,
        //                     aStrokeAttribute);
        //             }
        //         }
        //         else
        //         {
        //             // single line, only inside values used, no vertical offsets
        //             const BorderLine& rBorderLine(getBorderLines()[0]);
        //             const attribute::LineAttribute aLineAttribute(rBorderLine.getRGBColor(), rBorderLine.getWidth());

        //             addPolygonStrokePrimitive2D(
        //                 rContainer,
        //                 getStart() - (aVector * rBorderLine.getBorderLineExtend().getStartAverage()),
        //                 getEnd() + (aVector * rBorderLine.getBorderLineExtend().getEndAverage()),
        //                 aLineAttribute,
        //                 aStrokeAttribute);
        //         }
        //     }
        // }

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
            const std::vector< BorderLine >& rBorderLines,
            const drawinglayer::attribute::StrokeAttribute& rStrokeAttribute)
        :   BufferedDecompositionPrimitive2D(),
            maStart(rStart),
            maEnd(rEnd),
            maBorderLines(rBorderLines),
            maStrokeAttribute(rStrokeAttribute),
            mfSmallestAllowedDiscreteGapDistance(0.0)
        {
        }

        bool BorderLinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const BorderLinePrimitive2D& rCompare = static_cast<const BorderLinePrimitive2D&>(rPrimitive);

                if (getStart() == rCompare.getStart()
                    && getEnd() == rCompare.getEnd()
                    && getStrokeAttribute() == rCompare.getStrokeAttribute())
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

        bool BorderLinePrimitive2D::getSmallestGap(double& rfSmallestGap) const
        {
            bool bGapFound(false);

            for(const auto& candidate : maBorderLines)
            {
                if(candidate.isGap())
                {
                    if(bGapFound)
                    {
                        rfSmallestGap = std::min(rfSmallestGap, candidate.getLineAttribute().getWidth());
                    }
                    else
                    {
                        bGapFound = true;
                        rfSmallestGap = candidate.getLineAttribute().getWidth();
                    }
                }
            }

            return bGapFound;
        }

        void BorderLinePrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard(m_aMutex);

            if (!getStart().equal(getEnd()) && getBorderLines().size() > 1)
            {
                // Line with potential gap. In this case, we want to be view-dependent.
                // get the smallest gap
                double fSmallestGap(0.0);

                if(getSmallestGap(fSmallestGap))
                {
                    // Get the current DiscreteUnit, look at X and Y and use the maximum
                    const basegfx::B2DVector aDiscreteVector(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 1.0));
                    const double fDiscreteUnit(std::min(fabs(aDiscreteVector.getX()), fabs(aDiscreteVector.getY())));

                    // When discrete unit is bigger than distance (distance is less than one pixel),
                    // force distance to one pixel. Or expressed different, do not let the distance
                    // get smaller than one pixel. This is done for screen rendering and compatibility.
                    // This can also be done using DiscreteMetricDependentPrimitive2D as base class
                    // for this class, but specialization is better here for later buffering (only
                    // do this when 'double line with gap')
                    const double fNewDiscreteDistance(std::max(fDiscreteUnit, fSmallestGap));

                    if (!rtl::math::approxEqual(fNewDiscreteDistance, mfSmallestAllowedDiscreteGapDistance))
                    {
                        if (!getBuffered2DDecomposition().empty())
                        {
                            // conditions of last local decomposition have changed, delete
                            const_cast< BorderLinePrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DContainer());
                        }

                        // remember value for usage in create2DDecomposition
                        const_cast< BorderLinePrimitive2D* >(this)->mfSmallestAllowedDiscreteGapDistance = fNewDiscreteDistance;
                    }
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
