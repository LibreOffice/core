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

#include <svx/sdr/primitive2d/sdrframeborderprimitive2d.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svtools/borderhelper.hxx>

namespace
{
    double snapToDiscreteUnit(
        double fValue,
        double fMinimalDiscreteUnit)
    {
        if(0.0 != fValue)
        {
            fValue = std::max(fValue, fMinimalDiscreteUnit);
        }

        return fValue;
    }

    class StyleVectorCombination
    {
    private:
        struct OffsetAndHalfWidthAndColor
        {
            double          mfOffset;
            double          mfHalfWidth;
            Color           maColor;

            OffsetAndHalfWidthAndColor(double offset, double halfWidth, Color color) :
                mfOffset(offset),
                mfHalfWidth(halfWidth),
                maColor(color)
            {}
        };

        double                                      mfRefModeOffset;
        basegfx::B2DVector                          maB2DVector;
        double                                      mfAngle;
        std::vector< OffsetAndHalfWidthAndColor >   maOffsets;

    public:
        StyleVectorCombination(
            const svx::frame::Style& rStyle,
            const basegfx::B2DVector& rB2DVector,
            double fAngle,
            bool bMirrored,
            const Color* pForceColor,
            double fMinimalDiscreteUnit)
        :   mfRefModeOffset(0.0),
            maB2DVector(rB2DVector),
            mfAngle(fAngle),
            maOffsets()
        {
            if (!rStyle.IsUsed())
                return;

            svx::frame::RefMode aRefMode(rStyle.GetRefMode());
            Color aPrim(rStyle.GetColorPrim());
            Color aSecn(rStyle.GetColorSecn());
            const bool bSecnUsed(0.0 != rStyle.Secn());

            // Get the single segment line widths. This is the point where the
            // minimal discrete unit will be used if given (fMinimalDiscreteUnit). If
            // not given it's 0.0 and thus will have no influence.
            double fPrim(snapToDiscreteUnit(rStyle.Prim(), fMinimalDiscreteUnit));
            const double fDist(snapToDiscreteUnit(rStyle.Dist(), fMinimalDiscreteUnit));
            double fSecn(snapToDiscreteUnit(rStyle.Secn(), fMinimalDiscreteUnit));

            // Of course also do not use svx::frame::Style::GetWidth() for obvious
            // reasons.
            const double fStyleWidth(fPrim + fDist + fSecn);

            if(bMirrored)
            {
                switch(aRefMode)
                {
                    case svx::frame::RefMode::Begin: aRefMode = svx::frame::RefMode::End; break;
                    case svx::frame::RefMode::End: aRefMode = svx::frame::RefMode::Begin; break;
                    default: break;
                }

                if(bSecnUsed)
                {
                    std::swap(aPrim, aSecn);
                    std::swap(fPrim, fSecn);
                }
            }

            if (svx::frame::RefMode::Centered != aRefMode)
            {
                const double fHalfWidth(fStyleWidth * 0.5);

                if (svx::frame::RefMode::Begin == aRefMode)
                {
                    // move aligned below vector
                    mfRefModeOffset = fHalfWidth;
                }
                else if (svx::frame::RefMode::End == aRefMode)
                {
                    // move aligned above vector
                    mfRefModeOffset = -fHalfWidth;
                }
            }

            if (bSecnUsed)
            {
                // both or all three lines used
                const bool bPrimTransparent(rStyle.GetColorPrim().IsFullyTransparent());
                const bool bDistTransparent(!rStyle.UseGapColor() || rStyle.GetColorGap().IsFullyTransparent());
                const bool bSecnTransparent(aSecn.IsFullyTransparent());

                if(!bPrimTransparent || !bDistTransparent || !bSecnTransparent)
                {
                    const double a(mfRefModeOffset - (fStyleWidth * 0.5));
                    const double b(a + fPrim);
                    const double c(b + fDist);
                    const double d(c + fSecn);

                    maOffsets.push_back(
                        OffsetAndHalfWidthAndColor(
                            (a + b) * 0.5,
                            fPrim * 0.5,
                            nullptr != pForceColor ? *pForceColor : aPrim));

                    maOffsets.push_back(
                        OffsetAndHalfWidthAndColor(
                            (b + c) * 0.5,
                            fDist * 0.5,
                            rStyle.UseGapColor()
                                ? (nullptr != pForceColor ? *pForceColor : rStyle.GetColorGap())
                                : COL_TRANSPARENT));

                    maOffsets.push_back(
                        OffsetAndHalfWidthAndColor(
                            (c + d) * 0.5,
                            fSecn * 0.5,
                            nullptr != pForceColor ? *pForceColor : aSecn));
                }
            }
            else
            {
                // one line used, push two values, from outer to inner
                if(!rStyle.GetColorPrim().IsFullyTransparent())
                {
                    maOffsets.push_back(
                        OffsetAndHalfWidthAndColor(
                            mfRefModeOffset,
                            fPrim * 0.5,
                            nullptr != pForceColor ? *pForceColor : aPrim));
                }
            }
        }

        double getRefModeOffset() const { return mfRefModeOffset; }
        const basegfx::B2DVector& getB2DVector() const { return maB2DVector; }
        double getAngle() const { return mfAngle; }
        bool empty() const { return maOffsets.empty(); }
        size_t size() const { return maOffsets.size(); }

        void getColorAndOffsetAndHalfWidth(size_t nIndex, Color& rColor, double& rfOffset, double& rfHalfWidth) const
        {
            if(nIndex >= maOffsets.size())
                return;
            const OffsetAndHalfWidthAndColor& rCandidate(maOffsets[nIndex]);
            rfOffset = rCandidate.mfOffset;
            rfHalfWidth = rCandidate.mfHalfWidth;
            rColor = rCandidate.maColor;
        }
    };

    class StyleVectorTable
    {
    private:
        std::vector< StyleVectorCombination >       maEntries;

    public:
        StyleVectorTable()
        :   maEntries()
        {
        }

        void add(
            const svx::frame::Style& rStyle,
            const basegfx::B2DVector& rMyVector,
            const basegfx::B2DVector& rOtherVector,
            bool bMirrored,
            double fMinimalDiscreteUnit)
        {
            if(!rStyle.IsUsed() || basegfx::areParallel(rMyVector, rOtherVector))
                return;

            // create angle between both. angle() needs vectors pointing away from the same point,
            // so take the mirrored one. Add F_PI to get from -pi..+pi to [0..F_PI2] for sorting
            const double fAngle(basegfx::B2DVector(-rMyVector.getX(), -rMyVector.getY()).angle(rOtherVector) + F_PI);
            maEntries.emplace_back(
                rStyle,
                rOtherVector,
                fAngle,
                bMirrored,
                nullptr,
                fMinimalDiscreteUnit);
        }

        void sort()
        {
            // sort inverse from highest to lowest
            std::sort(
                maEntries.begin(),
                maEntries.end(),
                [](const StyleVectorCombination& a, const StyleVectorCombination& b)
                    { return a.getAngle() > b.getAngle(); });
        }

        bool empty() const { return maEntries.empty(); }
        const std::vector< StyleVectorCombination >& getEntries() const{ return maEntries; }
    };

    struct CutSet
    {
        double          mfOLML;
        double          mfORML;
        double          mfOLMR;
        double          mfORMR;

        CutSet() : mfOLML(0.0), mfORML(0.0), mfOLMR(0.0), mfORMR(0.0)
        {
        }

        bool operator<( const CutSet& rOther) const
        {
            const double fA(mfOLML + mfORML + mfOLMR + mfORMR);
            const double fB(rOther.mfOLML + rOther.mfORML + rOther.mfOLMR + rOther.mfORMR);

            return fA < fB;
        }

        double getSum() const { return mfOLML + mfORML + mfOLMR + mfORMR; }
    };

    void getCutSet(
        CutSet& rCutSet,
        const basegfx::B2DPoint& rLeft,
        const basegfx::B2DPoint& rRight,
        const basegfx::B2DVector& rX,
        const basegfx::B2DPoint& rOtherLeft,
        const basegfx::B2DPoint& rOtherRight,
        const basegfx::B2DVector& rOtherX)
    {
        basegfx::utils::findCut(
            rLeft,
            rX,
            rOtherLeft,
            rOtherX,
            CutFlagValue::LINE,
            &rCutSet.mfOLML);

        basegfx::utils::findCut(
            rRight,
            rX,
            rOtherLeft,
            rOtherX,
            CutFlagValue::LINE,
            &rCutSet.mfOLMR);

        basegfx::utils::findCut(
            rLeft,
            rX,
            rOtherRight,
            rOtherX,
            CutFlagValue::LINE,
            &rCutSet.mfORML);

        basegfx::utils::findCut(
            rRight,
            rX,
            rOtherRight,
            rOtherX,
            CutFlagValue::LINE,
            &rCutSet.mfORMR);
    }

    struct ExtendSet
    {
        double          mfExtLeft;
        double          mfExtRight;

        ExtendSet() : mfExtLeft(0.0), mfExtRight(0.0) {}
    };

    void getExtends(
        std::vector<ExtendSet>& rExtendSet,                         // target Left/Right values to fill
        const basegfx::B2DPoint& rOrigin,                           // own vector start
        const StyleVectorCombination& rCombination,                 // own vector and offsets for lines
        const basegfx::B2DVector& rPerpendX,                        // normalized perpendicular to own vector
        const std::vector< StyleVectorCombination >& rStyleVector)  // other vectors emerging in this point
    {
        if(!(!rCombination.empty() && !rStyleVector.empty() && rCombination.size() == rExtendSet.size()))
            return;

        const size_t nOffsetA(rCombination.size());

        if(1 == nOffsetA)
        {
            Color aMyColor; double fMyOffset(0.0); double fMyHalfWidth(0.0);
            rCombination.getColorAndOffsetAndHalfWidth(0, aMyColor, fMyOffset, fMyHalfWidth);

            if(!aMyColor.IsFullyTransparent())
            {
                const basegfx::B2DPoint aLeft(rOrigin + (rPerpendX * (fMyOffset - fMyHalfWidth)));
                const basegfx::B2DPoint aRight(rOrigin + (rPerpendX * (fMyOffset + fMyHalfWidth)));
                std::vector< CutSet > aCutSets;

                for(const auto& rStyleCandidate : rStyleVector)
                {
                    const basegfx::B2DVector aOtherPerpend(basegfx::getNormalizedPerpendicular(rStyleCandidate.getB2DVector()));
                    const size_t nOffsetB(rStyleCandidate.size());

                    for(size_t other(0); other < nOffsetB; other++)
                    {
                        Color aOtherColor; double fOtherOffset(0.0); double fOtherHalfWidth(0.0);
                        rStyleCandidate.getColorAndOffsetAndHalfWidth(other, aOtherColor, fOtherOffset, fOtherHalfWidth);

                        if(!aOtherColor.IsFullyTransparent())
                        {
                            const basegfx::B2DPoint aOtherLeft(rOrigin + (aOtherPerpend * (fOtherOffset - fOtherHalfWidth)));
                            const basegfx::B2DPoint aOtherRight(rOrigin + (aOtherPerpend * (fOtherOffset + fOtherHalfWidth)));

                            CutSet aNewCutSet;
                            getCutSet(aNewCutSet, aLeft, aRight, rCombination.getB2DVector(), aOtherLeft, aOtherRight, rStyleCandidate.getB2DVector());
                            aCutSets.push_back(aNewCutSet);
                        }
                    }
                }

                if(!aCutSets.empty())
                {
                    CutSet aCutSet(aCutSets[0]);
                    const size_t nNumCutSets(aCutSets.size());

                    if(1 != nNumCutSets)
                    {
                        double fCutSet(aCutSet.getSum());

                        for(size_t a(1); a < nNumCutSets; a++)
                        {
                            const CutSet& rCandidate(aCutSets[a]);
                            const double fCandidate(rCandidate.getSum());

                            if(basegfx::fTools::equalZero(fCandidate - fCutSet))
                            {
                                // both have equal center point, use medium cut
                                const double fNewOLML(std::max(std::min(rCandidate.mfOLML, rCandidate.mfORML), std::min(aCutSet.mfOLML, aCutSet.mfORML)));
                                const double fNewORML(std::min(std::max(rCandidate.mfOLML, rCandidate.mfORML), std::max(aCutSet.mfOLML, aCutSet.mfORML)));
                                const double fNewOLMR(std::max(std::min(rCandidate.mfOLMR, rCandidate.mfORMR), std::min(aCutSet.mfOLMR, aCutSet.mfORMR)));
                                const double fNewORMR(std::min(std::max(rCandidate.mfOLMR, rCandidate.mfORMR), std::max(aCutSet.mfOLMR, aCutSet.mfORMR)));
                                aCutSet.mfOLML = fNewOLML;
                                aCutSet.mfORML = fNewORML;
                                aCutSet.mfOLMR = fNewOLMR;
                                aCutSet.mfORMR = fNewORMR;
                                fCutSet = aCutSet.getSum();
                            }
                            else if(fCandidate < fCutSet)
                            {
                                // get minimum
                                fCutSet = fCandidate;
                                aCutSet = rCandidate;
                            }
                        }
                    }

                    ExtendSet& rExt(rExtendSet[0]);

                    rExt.mfExtLeft = std::min(aCutSet.mfOLML, aCutSet.mfORML);
                    rExt.mfExtRight = std::min(aCutSet.mfOLMR, aCutSet.mfORMR);
                }
            }
        }
        else
        {
            size_t nVisEdgeUp(0);
            size_t nVisEdgeDn(0);

            for(size_t my(0); my < nOffsetA; my++)
            {
                Color aMyColor; double fMyOffset(0.0); double fMyHalfWidth(0.0);
                rCombination.getColorAndOffsetAndHalfWidth(my, aMyColor, fMyOffset, fMyHalfWidth);

                if(!aMyColor.IsFullyTransparent())
                {
                    const basegfx::B2DPoint aLeft(rOrigin + (rPerpendX * (fMyOffset - fMyHalfWidth)));
                    const basegfx::B2DPoint aRight(rOrigin + (rPerpendX * (fMyOffset + fMyHalfWidth)));
                    const bool bUpper(my <= (nOffsetA >> 1));
                    const StyleVectorCombination& rStyleCandidate(bUpper ? rStyleVector.front() : rStyleVector.back());
                    const basegfx::B2DVector aOtherPerpend(basegfx::getNormalizedPerpendicular(rStyleCandidate.getB2DVector()));
                    const size_t nOffsetB(rStyleCandidate.size());
                    std::vector< CutSet > aCutSets;

                    for(size_t other(0); other < nOffsetB; other++)
                    {
                        Color aOtherColor; double fOtherOffset(0.0); double fOtherHalfWidth(0.0);
                        rStyleCandidate.getColorAndOffsetAndHalfWidth(other, aOtherColor, fOtherOffset, fOtherHalfWidth);

                        if(!aOtherColor.IsFullyTransparent())
                        {
                            const basegfx::B2DPoint aOtherLeft(rOrigin + (aOtherPerpend * (fOtherOffset - fOtherHalfWidth)));
                            const basegfx::B2DPoint aOtherRight(rOrigin + (aOtherPerpend * (fOtherOffset + fOtherHalfWidth)));
                            CutSet aCutSet;
                            getCutSet(aCutSet, aLeft, aRight, rCombination.getB2DVector(), aOtherLeft, aOtherRight, rStyleCandidate.getB2DVector());
                            aCutSets.push_back(aCutSet);
                        }
                    }

                    if(!aCutSets.empty())
                    {
                        // sort: min to start, max to end
                        std::sort(aCutSets.begin(), aCutSets.end());
                        const bool bOtherUpper(rStyleCandidate.getAngle() > F_PI);

                        // check if we need min or max
                        //  bUpper      bOtherUpper        MinMax
                        //    t             t               max
                        //    t             f               min
                        //    f             f               max
                        //    f             t               min
                        const bool bMax(bUpper == bOtherUpper);
                        size_t nBaseIndex(0);
                        const size_t nNumCutSets(aCutSets.size());

                        if(bMax)
                        {
                            // access at end
                            nBaseIndex = nNumCutSets - 1 - (bUpper ? nVisEdgeUp : nVisEdgeDn);
                        }
                        else
                        {
                            // access at start
                            nBaseIndex = bUpper ? nVisEdgeUp : nVisEdgeDn;
                        }

                        const size_t nSecuredIndex(std::clamp(nBaseIndex, size_t(0), size_t(nNumCutSets - 1)));
                        const CutSet& rCutSet(aCutSets[nSecuredIndex]);
                        ExtendSet& rExt(rExtendSet[my]);

                        rExt.mfExtLeft = std::min(rCutSet.mfOLML, rCutSet.mfORML);
                        rExt.mfExtRight = std::min(rCutSet.mfOLMR, rCutSet.mfORMR);
                    }

                    if(bUpper)
                    {
                        nVisEdgeUp++;
                    }
                    else
                    {
                        nVisEdgeDn++;
                    }
                }
            }
        }
    }

    /**
     *  Helper method to create the correct drawinglayer::primitive2d::BorderLinePrimitive2D
     *  for the given data, especially the correct drawinglayer::primitive2d::BorderLine entries
     *  including the correctly solved/created LineStartEnd extends
     *
     *  rTarget : Here the evtl. created BorderLinePrimitive2D will be appended
     *  rOrigin : StartPoint of the Borderline
     *  rX      : Vector of the Borderline
     *  rBorder : svx::frame::Style of the of the Borderline
     *  rStartStyleVectorTable : All other Borderlines which have to be taken into account because
     *      they have the same StartPoint as the current Borderline. These will be used to calculate
     *      the correct LineStartEnd extends tor the BorderLinePrimitive2D. The definition should be
     *      built up using svx::frame::StyleVectorTable and StyleVectorTable::add and includes:
     *          rStyle      : the svx::frame::Style of one other BorderLine
     *          rMyVector   : the Vector of the *new* to-be-defined BorderLine, identical to rX
     *          rOtherVector: the Vector of one other BorderLine (may be, but does not need to be normalized),
     *                        always *pointing away* from the common StartPoint rOrigin
     *          bMirrored   : define if rStyle of one other BorderLine shall be mirrored (e.g. bottom-right edges)
     *      With multiple BorderLines the definitions have to be CounterClockWise. This will be
     *      ensured by StyleVectorTable sorting the entries, but knowing this may allow more efficient
     *      data creation.
     *  rEndStyleVectorTable: All other BorderLines that have the same EndPoint. There are differences to
     *      the Start definitions:
     *          - do not forget to consequently use -rX for rMyVector
     *          - definitions have to be ClockWise for the EndBorderLines, will be ensured by sorting
     *
     *  If you take all this into account, you will get correctly extended BorderLinePrimitive2D
     *  representations for the new to be defined BorderLine. That extensions will overlap nicely
     *  with the corresponding BorderLines and take all multiple line definitions in the ::Style into
     *  account.
     *  The internal solver is *not limited* to ::Style(s) with three parts (Left/Gap/Right), this is
     *  just due to svx::frame::Style's definitions. A new solver based on this one can be created
     *  anytime using more mulötiple borders based on the more flexible
     *  std::vector< drawinglayer::primitive2d::BorderLine > if needed.
     */
    void CreateBorderPrimitives(
        drawinglayer::primitive2d::Primitive2DContainer& rTarget,   /// target for created primitives
        const basegfx::B2DPoint& rOrigin,                           /// start point of borderline
        const basegfx::B2DVector& rX,                               /// X-Axis of borderline with length
        const svx::frame::Style& rBorder,                           /// Style of borderline
        const StyleVectorTable& rStartStyleVectorTable,             /// Styles and vectors (pointing away) at borderline start, ccw
        const StyleVectorTable& rEndStyleVectorTable,               /// Styles and vectors (pointing away) at borderline end, cw
        const Color* pForceColor,                                   /// If specified, overrides frame border color.
        double fMinimalDiscreteUnit)                                /// minimal discrete unit to use for svx::frame::Style width values
    {
        // get offset color pairs for  style, one per visible line
        const StyleVectorCombination aCombination(
            rBorder,
            rX,
            0.0,
            false,
            pForceColor,
            fMinimalDiscreteUnit);

        if(aCombination.empty())
            return;

        const basegfx::B2DVector aPerpendX(basegfx::getNormalizedPerpendicular(rX));
        const bool bHasStartStyles(!rStartStyleVectorTable.empty());
        const bool bHasEndStyles(!rEndStyleVectorTable.empty());
        const size_t nOffsets(aCombination.size());
        std::vector<ExtendSet> aExtendSetStart(nOffsets);
        std::vector<ExtendSet> aExtendSetEnd(nOffsets);

        if(bHasStartStyles)
        {
            // create extends for line starts, use given point/vector and offsets
            getExtends(aExtendSetStart, rOrigin, aCombination, aPerpendX, rStartStyleVectorTable.getEntries());
        }

        if(bHasEndStyles)
        {
            // Create extends for line ends, create inverse point/vector and inverse offsets.
            const StyleVectorCombination aMirroredCombination(
                rBorder,
                -rX,
                0.0,
                true,
                pForceColor,
                fMinimalDiscreteUnit);

            getExtends(aExtendSetEnd, rOrigin + rX, aMirroredCombination, -aPerpendX, rEndStyleVectorTable.getEntries());

            // also need to inverse the result to apply to the correct lines
            std::reverse(aExtendSetEnd.begin(), aExtendSetEnd.end());
        }

        std::vector< drawinglayer::primitive2d::BorderLine > aBorderlines;
        const double fNegLength(-rX.getLength());

        for(size_t a(0); a < nOffsets; a++)
        {
            Color aMyColor;
            double fMyOffset(0.0);
            double fMyHalfWidth(0.0);
            aCombination.getColorAndOffsetAndHalfWidth(a, aMyColor, fMyOffset, fMyHalfWidth);
            const ExtendSet& rExtStart(aExtendSetStart[a]);
            const ExtendSet& rExtEnd(aExtendSetEnd[a]);

            if(aMyColor.IsFullyTransparent())
            {
                aBorderlines.push_back(
                    drawinglayer::primitive2d::BorderLine(
                        fMyHalfWidth * 2.0));
            }
            else
            {
                aBorderlines.push_back(
                    drawinglayer::primitive2d::BorderLine(
                        drawinglayer::attribute::LineAttribute(
                            aMyColor.getBColor(),
                            fMyHalfWidth * 2.0),
                        fNegLength * rExtStart.mfExtLeft,
                        fNegLength * rExtStart.mfExtRight,
                        fNegLength * rExtEnd.mfExtRight,
                        fNegLength * rExtEnd.mfExtLeft));
            }
        }

        static const double fPatScFact(10.0); // 10.0 multiply, see old code
        const std::vector<double> aDashing(svtools::GetLineDashing(rBorder.Type(), rBorder.PatternScale() * fPatScFact));
        const drawinglayer::attribute::StrokeAttribute aStrokeAttribute(aDashing);
        const basegfx::B2DPoint aStart(rOrigin + (aPerpendX * aCombination.getRefModeOffset()));

        rTarget.append(
            drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::BorderLinePrimitive2D(
                    aStart,
                    aStart + rX,
                    aBorderlines,
                    aStrokeAttribute)));
    }

    double getMinimalNonZeroValue(double fCurrent, double fNew)
    {
        if(0.0 != fNew)
        {
            if(0.0 != fCurrent)
            {
                fCurrent = std::min(fNew, fCurrent);
            }
            else
            {
                fCurrent = fNew;
            }
        }

        return fCurrent;
    }

    double getMinimalNonZeroBorderWidthFromStyle(double fCurrent, const svx::frame::Style& rStyle)
    {
        if(rStyle.IsUsed())
        {
            fCurrent = getMinimalNonZeroValue(fCurrent, rStyle.Prim());
            fCurrent = getMinimalNonZeroValue(fCurrent, rStyle.Dist());
            fCurrent = getMinimalNonZeroValue(fCurrent, rStyle.Secn());
        }

        return fCurrent;
    }
}

namespace drawinglayer::primitive2d
{
        SdrFrameBorderData::SdrConnectStyleData::SdrConnectStyleData(
            const svx::frame::Style& rStyle,
            const basegfx::B2DVector& rNormalizedPerpendicular,
            bool bStyleMirrored)
        :   maStyle(rStyle),
            maNormalizedPerpendicular(rNormalizedPerpendicular),
            mbStyleMirrored(bStyleMirrored)
        {
        }

        bool SdrFrameBorderData::SdrConnectStyleData::operator==(const SdrFrameBorderData::SdrConnectStyleData& rCompare) const
        {
            return mbStyleMirrored == rCompare.mbStyleMirrored
                && maStyle == rCompare.maStyle
                && maNormalizedPerpendicular == rCompare.maNormalizedPerpendicular;
        }

        SdrFrameBorderData::SdrFrameBorderData(
            const basegfx::B2DPoint& rOrigin,
            const basegfx::B2DVector& rX,
            const svx::frame::Style& rStyle,
            const Color* pForceColor)
        :   maOrigin(rOrigin),
            maX(rX),
            maStyle(rStyle),
            maColor(nullptr != pForceColor ? *pForceColor : Color()),
            mbForceColor(nullptr != pForceColor),
            maStart(),
            maEnd()
        {
        }

        void SdrFrameBorderData::addSdrConnectStyleData(
            bool bStart,
            const svx::frame::Style& rStyle,
            const basegfx::B2DVector& rNormalizedPerpendicular,
            bool bStyleMirrored)
        {
            if(rStyle.IsUsed())
            {
                if(bStart)
                {
                    maStart.emplace_back(rStyle, rNormalizedPerpendicular, bStyleMirrored);
                }
                else
                {
                    maEnd.emplace_back(rStyle, rNormalizedPerpendicular, bStyleMirrored);
                }
            }
        }

        void SdrFrameBorderData::create2DDecomposition(
            Primitive2DContainer& rContainer,
            double fMinimalDiscreteUnit) const
        {
            StyleVectorTable aStartVector;
            StyleVectorTable aEndVector;
            const basegfx::B2DVector aAxis(-maX);

            for(const auto& rStart : maStart)
            {
                aStartVector.add(
                    rStart.getStyle(),
                    maX,
                    rStart.getNormalizedPerpendicular(),
                    rStart.getStyleMirrored(),
                    fMinimalDiscreteUnit);
            }

            for(const auto& rEnd : maEnd)
            {
                aEndVector.add(
                    rEnd.getStyle(),
                    aAxis,
                    rEnd.getNormalizedPerpendicular(),
                    rEnd.getStyleMirrored(),
                    fMinimalDiscreteUnit);
            }

            aStartVector.sort();
            aEndVector.sort();

            CreateBorderPrimitives(
                rContainer,
                maOrigin,
                maX,
                maStyle,
                aStartVector,
                aEndVector,
                mbForceColor ? &maColor : nullptr,
                fMinimalDiscreteUnit);
        }

        double SdrFrameBorderData::getMinimalNonZeroBorderWidth() const
        {
            double fRetval(getMinimalNonZeroBorderWidthFromStyle(0.0, maStyle));

            for(const auto& rStart : maStart)
            {
                fRetval = getMinimalNonZeroBorderWidthFromStyle(fRetval, rStart.getStyle());
            }

            for(const auto& rEnd : maEnd)
            {
                fRetval = getMinimalNonZeroBorderWidthFromStyle(fRetval, rEnd.getStyle());
            }

            return fRetval;
        }


        bool SdrFrameBorderData::operator==(const SdrFrameBorderData& rCompare) const
        {
            return maOrigin == rCompare.maOrigin
                && maX == rCompare.maX
                && maStyle == rCompare.maStyle
                && maColor == rCompare.maColor
                && mbForceColor == rCompare.mbForceColor
                && maStart == rCompare.maStart
                && maEnd == rCompare.maEnd;
        }


        void SdrFrameBorderPrimitive2D::create2DDecomposition(
            Primitive2DContainer& rContainer,
            const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            if(!getFrameBorders())
            {
                return;
            }

            Primitive2DContainer aRetval;

            // Check and use the minimal non-zero BorderWidth for decompose
            // if that is set and wanted
            const double fMinimalDiscreteUnit(doForceToSingleDiscreteUnit()
                ? mfMinimalNonZeroBorderWidthUsedForDecompose
                : 0.0);

            {
                // decompose all buffered SdrFrameBorderData entries and try to merge them
                // to reduce existing number of BorderLinePrimitive2D(s)
                for(const auto& rCandidate : *getFrameBorders())
                {
                    // get decomposition on one SdrFrameBorderData entry
                    Primitive2DContainer aPartial;
                    rCandidate.create2DDecomposition(
                        aPartial,
                        fMinimalDiscreteUnit);

                    for(const auto& aCandidatePartial : aPartial)
                    {
                        bool bDidMerge(false);

                        // This algorithm is O(N^2) and repeated dynamic_cast inside would be quite costly.
                        // So check first and skip if the primitives aren't BorderLinePrimitive2D.
                        const drawinglayer::primitive2d::BorderLinePrimitive2D* candidatePartialAsBorder
                            = dynamic_cast<const drawinglayer::primitive2d::BorderLinePrimitive2D*>(aCandidatePartial.get());
                        if(candidatePartialAsBorder)
                        {
                            for(auto& aCandidateRetval : aRetval)
                            {
                                const drawinglayer::primitive2d::BorderLinePrimitive2D* candidateRetvalAsBorder
                                    = dynamic_cast<const drawinglayer::primitive2d::BorderLinePrimitive2D*>(aCandidateRetval.get());
                                if(candidateRetvalAsBorder)
                                {
                                    // try to merge by appending new data to existing data
                                    const drawinglayer::primitive2d::Primitive2DReference aMergeRetvalPartial(
                                        drawinglayer::primitive2d::tryMergeBorderLinePrimitive2D(
                                            candidateRetvalAsBorder,
                                            candidatePartialAsBorder));

                                    if(aMergeRetvalPartial.is())
                                    {
                                        // could append, replace existing data with merged data, done
                                        aCandidateRetval = aMergeRetvalPartial;
                                        bDidMerge = true;
                                        break;
                                    }

                                    // try to merge by appending existing data to new data
                                    const drawinglayer::primitive2d::Primitive2DReference aMergePartialRetval(
                                        drawinglayer::primitive2d::tryMergeBorderLinePrimitive2D(
                                            candidatePartialAsBorder,
                                            candidateRetvalAsBorder));

                                    if(aMergePartialRetval.is())
                                    {
                                        // could append, replace existing data with merged data, done
                                        aCandidateRetval = aMergePartialRetval;
                                        bDidMerge = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if(!bDidMerge)
                        {
                            // no merge after checking all existing data, append as new segment
                            aRetval.append(aCandidatePartial);
                        }
                    }
                }
            }

            rContainer.insert(rContainer.end(), aRetval.begin(), aRetval.end());
        }

        SdrFrameBorderPrimitive2D::SdrFrameBorderPrimitive2D(
            std::shared_ptr<SdrFrameBorderDataVector>& rFrameBorders,
            bool bForceToSingleDiscreteUnit)
        :   BufferedDecompositionPrimitive2D(),
            maFrameBorders(std::move(rFrameBorders)),
            mfMinimalNonZeroBorderWidth(0.0),
            mfMinimalNonZeroBorderWidthUsedForDecompose(0.0),
            mbForceToSingleDiscreteUnit(bForceToSingleDiscreteUnit)
        {
            if(getFrameBorders() && doForceToSingleDiscreteUnit())
            {
                // detect used minimal non-zero partial border width
                for(const auto& rCandidate : *getFrameBorders())
                {
                    mfMinimalNonZeroBorderWidth = getMinimalNonZeroValue(
                        mfMinimalNonZeroBorderWidth,
                        rCandidate.getMinimalNonZeroBorderWidth());
                }
            }
        }

        bool SdrFrameBorderPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrFrameBorderPrimitive2D& rCompare = static_cast<const SdrFrameBorderPrimitive2D&>(rPrimitive);

                return (getFrameBorders() == rCompare.getFrameBorders()
                    || (getFrameBorders() && rCompare.getFrameBorders()
                        && *getFrameBorders() == *rCompare.getFrameBorders()))
                    && doForceToSingleDiscreteUnit() == rCompare.doForceToSingleDiscreteUnit();
            }

            return false;
        }

        void SdrFrameBorderPrimitive2D::get2DDecomposition(
            Primitive2DDecompositionVisitor& rVisitor,
            const geometry::ViewInformation2D& rViewInformation) const
        {
            if(doForceToSingleDiscreteUnit())
            {
                // Get the current DiscreteUnit, look at X and Y and use the maximum
                const basegfx::B2DVector aDiscreteVector(rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 1.0));
                double fDiscreteUnit(std::min(fabs(aDiscreteVector.getX()), fabs(aDiscreteVector.getY())));

                if(fDiscreteUnit <= mfMinimalNonZeroBorderWidth)
                {
                    // no need to use it, reset
                    fDiscreteUnit = 0.0;
                }

                if(fDiscreteUnit != mfMinimalNonZeroBorderWidthUsedForDecompose)
                {
                    // conditions of last local decomposition have changed, delete
                    // possible content
                    if(!getBuffered2DDecomposition().empty())
                    {
                        const_cast< SdrFrameBorderPrimitive2D* >(this)->setBuffered2DDecomposition(Primitive2DContainer());
                    }

                    // remember new conditions
                    const_cast< SdrFrameBorderPrimitive2D* >(this)->mfMinimalNonZeroBorderWidthUsedForDecompose = fDiscreteUnit;
                }
            }

            // call parent. This will call back ::create2DDecomposition above
            // where mfMinimalNonZeroBorderWidthUsedForDecompose will be used
            // when doForceToSingleDiscreteUnit() is true
            BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SdrFrameBorderPrimitive2D, PRIMITIVE2D_ID_SDRFRAMEBORDERTPRIMITIVE2D)

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
