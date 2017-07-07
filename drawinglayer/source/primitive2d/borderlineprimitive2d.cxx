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

#include <algorithm>
#include <cmath>
#include <numeric>

#if defined(ANDROID)
namespace std
{
template<typename T>
T round(T x)
{
    return ::round(x);
}
}
#endif

namespace drawinglayer
{
    namespace primitive2d
    {
        // helper to add a centered, maybe stroked line primitive to rContainer
        void addPolygonStrokePrimitive2D(
            Primitive2DContainer& rContainer,
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            const basegfx::BColor& rColor,
            double fWidth,
            SvxBorderLineStyle aStyle,
            double fPatternScale)
        {
            basegfx::B2DPolygon aPolygon;

            aPolygon.append(rStart);
            aPolygon.append(rEnd);

            const attribute::LineAttribute aLineAttribute(rColor, fWidth);
            static double fPatScFact(10.0); // 10.0 multiply, see old code
            const std::vector<double> aDashing(svtools::GetLineDashing(aStyle, fPatternScale * fPatScFact));

            if (aDashing.empty())
            {
                rContainer.push_back(
                    new PolygonStrokePrimitive2D(
                        aPolygon,
                        aLineAttribute));
            }
            else
            {
                const attribute::StrokeAttribute aStrokeAttribute(aDashing);

                rContainer.push_back(
                    new PolygonStrokePrimitive2D(
                        aPolygon,
                        aLineAttribute,
                        aStrokeAttribute));
            }
        }

        void BorderLinePrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if (!getStart().equal(getEnd()) && (isInsideUsed() || isOutsideUsed()))
            {
                // get data and vectors
                basegfx::B2DVector aVector(getEnd() - getStart());
                aVector.normalize();
                const basegfx::B2DVector aPerpendicular(basegfx::getPerpendicular(aVector));

                if (isOutsideUsed() && isInsideUsed())
                {
                    // double line with gap. Use mfDiscreteDistance (see get2DDecomposition) as distance.
                    // That value is prepared to be at least one pixel (discrete unit) so that the
                    // decomposition is view-dependent in this cases
                    if (isInsideUsed())
                    {
                        // inside line (left). Create stroke primitive centered on line width
                        const double fDeltaY((mfDiscreteDistance + getLeftWidth()) * 0.5);
                        const basegfx::B2DVector aDeltaY(aPerpendicular * fDeltaY);
                        const basegfx::B2DPoint aStart(getStart() - (aVector * getExtendLeftStart()) - aDeltaY);
                        const basegfx::B2DPoint aEnd(getEnd() + (aVector * getExtendLeftEnd()) - aDeltaY);

                        addPolygonStrokePrimitive2D(
                            rContainer,
                            aStart,
                            aEnd,
                            getRGBColorLeft(),
                            getLeftWidth(),
                            getStyle(),
                            getPatternScale());
                    }

                    if (hasGapColor() && isDistanceUsed())
                    {
                        // gap (if visible, found no practicval usage).
                        // Create stroke primitive on vector with given color
                        addPolygonStrokePrimitive2D(
                            rContainer,
                            getStart(),
                            getEnd(),
                            getRGBColorGap(),
                            mfDiscreteDistance,
                            getStyle(),
                            getPatternScale());
                    }

                    if (isOutsideUsed())
                    {
                        // outside line (right). Create stroke primitive centered on line width
                        const double fDeltaY((mfDiscreteDistance + getRightWidth()) * 0.5);
                        const basegfx::B2DVector aDeltaY(aPerpendicular * fDeltaY);
                        const basegfx::B2DPoint aStart(getStart() - (aVector * getExtendRightStart()) + aDeltaY);
                        const basegfx::B2DPoint aEnd(getEnd() + (aVector * getExtendRightEnd()) + aDeltaY);

                        addPolygonStrokePrimitive2D(
                            rContainer,
                            aStart,
                            aEnd,
                            getRGBColorRight(),
                            getRightWidth(),
                            getStyle(),
                            getPatternScale());
                    }
                }
                else if(isInsideUsed())
                {
                    // single line, only inside values used, no vertical offsets
                    addPolygonStrokePrimitive2D(
                        rContainer,
                        getStart(),
                        getEnd(),
                        getRGBColorLeft(),
                        getLeftWidth(),
                        getStyle(),
                        getPatternScale());
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
            double fLeftWidth,
            double fDistance,
            double fRightWidth,
            double fExtendLeftStart,
            double fExtendLeftEnd,
            double fExtendRightStart,
            double fExtendRightEnd,
            const basegfx::BColor& rRGBColorRight,
            const basegfx::BColor& rRGBColorLeft,
            const basegfx::BColor& rRGBColorGap,
            bool bHasGapColor,
            SvxBorderLineStyle nStyle,
            double fPatternScale)
        :   BufferedDecompositionPrimitive2D(),
            maStart(rStart),
            maEnd(rEnd),
            mfLeftWidth(fLeftWidth),
            mfDistance(fDistance),
            mfRightWidth(fRightWidth),
            mfExtendLeftStart(fExtendLeftStart),
            mfExtendLeftEnd(fExtendLeftEnd),
            mfExtendRightStart(fExtendRightStart),
            mfExtendRightEnd(fExtendRightEnd),
            maRGBColorRight(rRGBColorRight),
            maRGBColorLeft(rRGBColorLeft),
            maRGBColorGap(rRGBColorGap),
            mbHasGapColor(bHasGapColor),
            mnStyle(nStyle),
            mfPatternScale(fPatternScale),
            mfDiscreteDistance(0.0)
        {
        }

        bool BorderLinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const BorderLinePrimitive2D& rCompare = static_cast<const BorderLinePrimitive2D&>(rPrimitive);

                return (getStart() == rCompare.getStart()
                    && getEnd() == rCompare.getEnd()
                    && getLeftWidth() == rCompare.getLeftWidth()
                    && getDistance() == rCompare.getDistance()
                    && getRightWidth() == rCompare.getRightWidth()
                    && getExtendLeftStart() == rCompare.getExtendLeftStart()
                    && getExtendLeftEnd() == rCompare.getExtendLeftEnd()
                    && getExtendRightStart() == rCompare.getExtendRightStart()
                    && getExtendRightEnd() == rCompare.getExtendRightEnd()
                    && getRGBColorRight() == rCompare.getRGBColorRight()
                    && getRGBColorLeft() == rCompare.getRGBColorLeft()
                    && getRGBColorGap() == rCompare.getRGBColorGap()
                    && hasGapColor() == rCompare.hasGapColor()
                    && getStyle() == rCompare.getStyle()
                    && getPatternScale() == rCompare.getPatternScale());
            }

            return false;
        }

        void BorderLinePrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard(m_aMutex);

            if (!getStart().equal(getEnd()) && isOutsideUsed() && isInsideUsed())
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
                const double fNewDiscreteDistance(std::max(fDiscreteUnit, getDistance()));

                if (fNewDiscreteDistance != mfDiscreteDistance)
                {
                    if (!getBuffered2DDecomposition().empty())
                    {
                        // conditions of last local decomposition have changed, delete
                        const_cast< BorderLinePrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DContainer());
                    }

                    // remember value for usage in create2DDecomposition
                    const_cast< BorderLinePrimitive2D* >(this)->mfDiscreteDistance = fNewDiscreteDistance;
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
