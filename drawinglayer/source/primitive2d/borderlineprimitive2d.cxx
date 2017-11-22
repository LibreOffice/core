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
                    const double fWidth(candidate.getAdaptedWidth(mfSmallestAllowedDiscreteGapDistance));

                    if(!candidate.isGap())
                    {
                        const basegfx::B2DVector aDeltaY(aPerpendicular * (fOffset + (fWidth * 0.5)));
                        const basegfx::B2DPoint aStart(getStart() + aDeltaY);
                        const basegfx::B2DPoint aEnd(getEnd() + aDeltaY);
                        const bool bStartPerpendicular(rtl::math::approxEqual(candidate.getStartLeft(), candidate.getStartRight()));
                        const bool bEndPerpendicular(rtl::math::approxEqual(candidate.getEndLeft(), candidate.getEndRight()));

                        if(bStartPerpendicular && bEndPerpendicular)
                        {
                            // start and end extends lead to an edge perpendicular to the line, so we can just use
                            // a PolygonStrokePrimitive2D for representation
                            addPolygonStrokePrimitive2D(
                                rContainer,
                                aStart - (aVector * candidate.getStartLeft()),
                                aEnd + (aVector * candidate.getEndLeft()),
                                candidate.getLineAttribute(),
                                getStrokeAttribute());
                        }
                        else
                        {
                            // start and/or end extensions lead to a lineStart/End that is *not*
                            // perpendicular to the line itself
                            if(getStrokeAttribute().isDefault() || 0.0 == getStrokeAttribute().getFullDotDashLen())
                            {
                                // without stroke, we can simply represent that using a filled polygon
                                const basegfx::B2DVector aHalfLineOffset(aPerpendicular * (candidate.getLineAttribute().getWidth() * 0.5));
                                basegfx::B2DPolygon aPolygon;

                                aPolygon.append(aStart - aHalfLineOffset - (aVector * candidate.getStartLeft()));
                                aPolygon.append(aEnd - aHalfLineOffset + (aVector * candidate.getEndLeft()));
                                aPolygon.append(aEnd + aHalfLineOffset + (aVector * candidate.getEndRight()));
                                aPolygon.append(aStart + aHalfLineOffset - (aVector * candidate.getStartRight()));

                                rContainer.push_back(
                                    new PolyPolygonColorPrimitive2D(
                                        basegfx::B2DPolyPolygon(aPolygon),
                                        candidate.getLineAttribute().getColor()));
                            }
                            else
                            {
                                // with stroke, we have a problem - a filled polygon would lose the
                                // stroke. Let's represent the start and/or end as triangles, the main
                                // line still as PolygonStrokePrimitive2D.
                                // Fill default line Start/End for stroke, so we need no adaptions in else paths
                                basegfx::B2DPoint aStrokeStart(aStart - (aVector * candidate.getStartLeft()));
                                basegfx::B2DPoint aStrokeEnd(aEnd + (aVector * candidate.getEndLeft()));
                                const basegfx::B2DVector aHalfLineOffset(aPerpendicular * (candidate.getLineAttribute().getWidth() * 0.5));

                                if(!bStartPerpendicular)
                                {
                                    const double fMin(std::min(candidate.getStartLeft(), candidate.getStartRight()));
                                    const double fMax(std::max(candidate.getStartLeft(), candidate.getStartRight()));
                                    basegfx::B2DPolygon aPolygon;

                                    // create a triangle with min/max values for LineStart and add
                                    if(rtl::math::approxEqual(candidate.getStartLeft(), fMax))
                                    {
                                        aPolygon.append(aStart - aHalfLineOffset - (aVector * candidate.getStartLeft()));
                                    }

                                    aPolygon.append(aStart - aHalfLineOffset - (aVector * fMin));
                                    aPolygon.append(aStart + aHalfLineOffset - (aVector * fMin));

                                    if(rtl::math::approxEqual(candidate.getStartRight(), fMax))
                                    {
                                        aPolygon.append(aStart + aHalfLineOffset - (aVector * candidate.getStartRight()));
                                    }

                                    rContainer.push_back(
                                        new PolyPolygonColorPrimitive2D(
                                            basegfx::B2DPolyPolygon(aPolygon),
                                            candidate.getLineAttribute().getColor()));

                                    // Adapt StrokeStart accordingly
                                    aStrokeStart = aStart - (aVector * fMin);
                                }

                                if(!bEndPerpendicular)
                                {
                                    const double fMin(std::min(candidate.getEndLeft(), candidate.getEndRight()));
                                    const double fMax(std::max(candidate.getEndLeft(), candidate.getEndRight()));
                                    basegfx::B2DPolygon aPolygon;

                                    // create a triangle with min/max values for LineEnd and add
                                    if(rtl::math::approxEqual(candidate.getEndLeft(), fMax))
                                    {
                                        aPolygon.append(aEnd - aHalfLineOffset + (aVector * candidate.getEndLeft()));
                                    }

                                    if(rtl::math::approxEqual(candidate.getEndRight(), fMax))
                                    {
                                        aPolygon.append(aEnd + aHalfLineOffset + (aVector * candidate.getEndRight()));
                                    }

                                    aPolygon.append(aEnd + aHalfLineOffset + (aVector * fMin));
                                    aPolygon.append(aEnd - aHalfLineOffset + (aVector * fMin));

                                    rContainer.push_back(
                                        new PolyPolygonColorPrimitive2D(
                                            basegfx::B2DPolyPolygon(aPolygon),
                                            candidate.getLineAttribute().getColor()));

                                    // Adapt StrokeEnd accordingly
                                    aStrokeEnd = aEnd + (aVector * fMin);
                                }

                                addPolygonStrokePrimitive2D(
                                    rContainer,
                                    aStrokeStart,
                                    aStrokeEnd,
                                    candidate.getLineAttribute(),
                                    getStrokeAttribute());
                            }
                        }
                    }

                    fOffset += fWidth;
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

        Primitive2DReference tryMergeBorderLinePrimitive2D(
            const Primitive2DReference& rCandidateA,
            const Primitive2DReference& rCandidateB)
        {
            // try to cast to BorderLinePrimitive2D
            const primitive2d::BorderLinePrimitive2D* pCandidateA = dynamic_cast< const primitive2d::BorderLinePrimitive2D* >(rCandidateA.get());
            const primitive2d::BorderLinePrimitive2D* pCandidateB = dynamic_cast< const primitive2d::BorderLinePrimitive2D* >(rCandidateB.get());

            // we need a comparable BorderLinePrimitive2D
            if(nullptr == pCandidateA || nullptr == pCandidateB)
            {
                return Primitive2DReference();
            }

            // start of candidate has to match end of this
            if(!pCandidateA->getEnd().equal(pCandidateB->getStart()))
            {
                return Primitive2DReference();
            }

            // candidate A needs a length
            if(pCandidateA->getStart().equal(pCandidateA->getEnd()))
            {
                return Primitive2DReference();
            }

            // candidate B needs a length
            if(pCandidateB->getStart().equal(pCandidateB->getEnd()))
            {
                return Primitive2DReference();
            }

            // StrokeAttribute has to be equal
            if(!(pCandidateA->getStrokeAttribute() == pCandidateB->getStrokeAttribute()))
            {
                return Primitive2DReference();
            }

            // direction has to be equal -> cross product == 0.0
            const basegfx::B2DVector aVT(pCandidateA->getEnd() - pCandidateA->getStart());
            const basegfx::B2DVector aVC(pCandidateB->getEnd() - pCandidateB->getStart());
            if(!rtl::math::approxEqual(0.0, aVC.cross(aVT)))
            {
                return Primitive2DReference();
            }

            // number BorderLines has to be equal
            const size_t count(pCandidateA->getBorderLines().size());
            if(count != pCandidateB->getBorderLines().size())
            {
                return Primitive2DReference();
            }

            for(size_t a(0); a < count; a++)
            {
                const BorderLine& rBT(pCandidateA->getBorderLines()[a]);
                const BorderLine& rBC(pCandidateB->getBorderLines()[a]);

                // LineAttribute has to be the same
                if(!(rBC.getLineAttribute() == rBT.getLineAttribute()))
                {
                    return Primitive2DReference();
                }

                // isGap has to be the same
                if(rBC.isGap() != rBT.isGap())
                {
                    return Primitive2DReference();
                }

                if(rBT.isGap())
                {
                    // when gap, width has to be equal
                    if(!rtl::math::approxEqual(rBT.getLineAttribute().getWidth(), rBC.getLineAttribute().getWidth()))
                    {
                        return Primitive2DReference();
                    }
                }
                else
                {
                    // when not gap, the line extends have at least reach to the center ( > 0.0),
                    // else there is a extend usage. When > 0.0 they just overlap, no problem
                    if(rBT.getEndLeft() >= 0.0
                        && rBT.getEndRight() >= 0.0
                        && rBC.getStartLeft() >= 0.0
                        && rBC.getStartRight() >= 0.0)
                    {
                        // okay
                    }
                    else
                    {
                        return Primitive2DReference();
                    }
                }
            }

            // all conditions met, create merged primitive
            std::vector< BorderLine > aMergedBorderLines;

            for(size_t a(0); a < count; a++)
            {
                const BorderLine& rBT(pCandidateA->getBorderLines()[a]);
                const BorderLine& rBC(pCandidateB->getBorderLines()[a]);

                if(rBT.isGap())
                {
                    aMergedBorderLines.push_back(rBT);
                }
                else
                {
                    aMergedBorderLines.push_back(
                        BorderLine(
                            rBT.getLineAttribute(),
                            rBT.getStartLeft(), rBT.getStartRight(),
                            rBC.getEndLeft(), rBC.getEndRight()));
                }
            }

            return Primitive2DReference(
                new BorderLinePrimitive2D(
                    pCandidateA->getStart(),
                    pCandidateB->getEnd(),
                    aMergedBorderLines,
                    pCandidateA->getStrokeAttribute()));
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
