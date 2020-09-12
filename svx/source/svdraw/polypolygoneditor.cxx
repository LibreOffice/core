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


#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <svx/polypolygoneditor.hxx>

namespace sdr {

PolyPolygonEditor::PolyPolygonEditor( const basegfx::B2DPolyPolygon& rPolyPolygon)
: maPolyPolygon( rPolyPolygon )
{
}

bool PolyPolygonEditor::DeletePoints( const o3tl::sorted_vector< sal_uInt16 >& rAbsPoints )
{
    bool bPolyPolyChanged = false;

    auto aIter( rAbsPoints.rbegin() );
    for( ; aIter != rAbsPoints.rend(); ++aIter )
    {
        sal_uInt32 nPoly, nPnt;
        if( GetRelativePolyPoint(maPolyPolygon,(*aIter), nPoly, nPnt) )
        {
            // remove point
            basegfx::B2DPolygon aCandidate(maPolyPolygon.getB2DPolygon(nPoly));

            aCandidate.remove(nPnt);


            if( aCandidate.count() < 2 )
            {
                maPolyPolygon.remove(nPoly);
            }
            else
            {
                maPolyPolygon.setB2DPolygon(nPoly, aCandidate);
            }

            bPolyPolyChanged = true;
        }
    }

    return bPolyPolyChanged;
}

bool PolyPolygonEditor::SetSegmentsKind(SdrPathSegmentKind eKind, const o3tl::sorted_vector< sal_uInt16 >& rAbsPoints )
{
    bool bPolyPolyChanged = false;

    auto aIter( rAbsPoints.rbegin() );
    for( ; aIter != rAbsPoints.rend(); ++aIter )
    {
        sal_uInt32 nPolyNum, nPntNum;

        if(PolyPolygonEditor::GetRelativePolyPoint(maPolyPolygon, (*aIter), nPolyNum, nPntNum))
        {
            // do change at aNewPolyPolygon. Take a look at edge.
            basegfx::B2DPolygon aCandidate(maPolyPolygon.getB2DPolygon(nPolyNum));
            const sal_uInt32 nCount(aCandidate.count());

            if(nCount && (nPntNum + 1 < nCount || aCandidate.isClosed()))
            {
                bool bCandidateChanged(false);

                // it's a valid edge, check control point usage
                const sal_uInt32 nNextIndex((nPntNum + 1) % nCount);
                const bool bContolUsed(aCandidate.areControlPointsUsed()
                    && (aCandidate.isNextControlPointUsed(nPntNum) || aCandidate.isPrevControlPointUsed(nNextIndex)));

                if(bContolUsed)
                {
                    if(SdrPathSegmentKind::Toggle == eKind || SdrPathSegmentKind::Line == eKind)
                    {
                        // remove control
                        aCandidate.resetNextControlPoint(nPntNum);
                        aCandidate.resetPrevControlPoint(nNextIndex);
                        bCandidateChanged = true;
                    }
                }
                else
                {
                    if(SdrPathSegmentKind::Toggle == eKind || SdrPathSegmentKind::Curve == eKind)
                    {
                        // add control
                        const basegfx::B2DPoint aStart(aCandidate.getB2DPoint(nPntNum));
                        const basegfx::B2DPoint aEnd(aCandidate.getB2DPoint(nNextIndex));

                        aCandidate.setNextControlPoint(nPntNum, interpolate(aStart, aEnd, (1.0 / 3.0)));
                        aCandidate.setPrevControlPoint(nNextIndex, interpolate(aStart, aEnd, (2.0 / 3.0)));
                        bCandidateChanged = true;
                    }
                }

                if(bCandidateChanged)
                {
                    maPolyPolygon.setB2DPolygon(nPolyNum, aCandidate);
                    bPolyPolyChanged = true;
                }
            }
        }
    }

    return bPolyPolyChanged;
}

bool PolyPolygonEditor::SetPointsSmooth( basegfx::B2VectorContinuity eFlags, const o3tl::sorted_vector< sal_uInt16 >& rAbsPoints)
{
    bool bPolyPolygonChanged(false);

    auto aIter( rAbsPoints.rbegin() );
    for( ; aIter != rAbsPoints.rend(); ++aIter )
    {
        sal_uInt32 nPolyNum, nPntNum;

        if(PolyPolygonEditor::GetRelativePolyPoint(maPolyPolygon, (*aIter), nPolyNum, nPntNum))
        {
            // do change at aNewPolyPolygon...
            basegfx::B2DPolygon aCandidate(maPolyPolygon.getB2DPolygon(nPolyNum));

            // set continuity in point, make sure there is a curve
            bool bPolygonChanged = basegfx::utils::expandToCurveInPoint(aCandidate, nPntNum);
            bPolygonChanged |= basegfx::utils::setContinuityInPoint(aCandidate, nPntNum, eFlags);

            if(bPolygonChanged)
            {
                maPolyPolygon.setB2DPolygon(nPolyNum, aCandidate);
                bPolyPolygonChanged = true;
            }
        }
    }

    return bPolyPolygonChanged;
}

bool PolyPolygonEditor::GetRelativePolyPoint( const basegfx::B2DPolyPolygon& rPoly, sal_uInt32 nAbsPnt, sal_uInt32& rPolyNum, sal_uInt32& rPointNum )
{
    const sal_uInt32 nPolyCount(rPoly.count());
    sal_uInt32 nPolyNum(0);

    while(nPolyNum < nPolyCount)
    {
        const sal_uInt32 nPointCount(rPoly.getB2DPolygon(nPolyNum).count());

        if(nAbsPnt < nPointCount)
        {
            rPolyNum = nPolyNum;
            rPointNum = nAbsPnt;

            return true;
        }
        else
        {
            nPolyNum++;
            nAbsPnt -= nPointCount;
        }
    }

    return false;
}

} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
