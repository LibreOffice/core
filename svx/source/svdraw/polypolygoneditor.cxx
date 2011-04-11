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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include "svx/polypolygoneditor.hxx"

namespace sdr {

PolyPolygonEditor::PolyPolygonEditor( const basegfx::B2DPolyPolygon& rPolyPolygon, bool bClosed )
: maPolyPolygon( rPolyPolygon )
, mbIsClosed( bClosed )
{
}

bool PolyPolygonEditor::DeletePoints( const std::set< sal_uInt16 >& rAbsPoints )
{
    bool bPolyPolyChanged = false;

    std::set< sal_uInt16 >::const_reverse_iterator aIter;( rAbsPoints.rbegin() );
    for( aIter = rAbsPoints.rbegin(); aIter != rAbsPoints.rend(); ++aIter )
    {
        sal_uInt32 nPoly, nPnt;
        if( GetRelativePolyPoint(maPolyPolygon,(*aIter), nPoly, nPnt) )
        {
            // remove point
            basegfx::B2DPolygon aCandidate(maPolyPolygon.getB2DPolygon(nPoly));

            aCandidate.remove(nPnt);

            if( ( mbIsClosed && aCandidate.count() < 3L) || (aCandidate.count() < 2L) )
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

bool PolyPolygonEditor::SetSegmentsKind(SdrPathSegmentKind eKind, const std::set< sal_uInt16 >& rAbsPoints )
{
    bool bPolyPolyChanged = false;

    std::set< sal_uInt16 >::const_reverse_iterator aIter;( rAbsPoints.rbegin() );
    for( aIter = rAbsPoints.rbegin(); aIter != rAbsPoints.rend(); ++aIter )
    {
        sal_uInt32 nPolyNum, nPntNum;

        if(PolyPolygonEditor::GetRelativePolyPoint(maPolyPolygon, (*aIter), nPolyNum, nPntNum))
        {
            // do change at aNewPolyPolygon. Take a look at edge.
            basegfx::B2DPolygon aCandidate(maPolyPolygon.getB2DPolygon(nPolyNum));
            bool bCandidateChanged(false);
            const sal_uInt32 nCount(aCandidate.count());

            if(nCount && (nPntNum + 1 < nCount || aCandidate.isClosed()))
            {
                // it's a valid edge, check control point usage
                const sal_uInt32 nNextIndex((nPntNum + 1) % nCount);
                const bool bContolUsed(aCandidate.areControlPointsUsed()
                    && (aCandidate.isNextControlPointUsed(nPntNum) || aCandidate.isPrevControlPointUsed(nNextIndex)));

                if(bContolUsed)
                {
                    if(SDRPATHSEGMENT_TOGGLE == eKind || SDRPATHSEGMENT_LINE == eKind)
                    {
                        // remove control
                        aCandidate.resetNextControlPoint(nPntNum);
                        aCandidate.resetPrevControlPoint(nNextIndex);
                        bCandidateChanged = true;
                    }
                }
                else
                {
                    if(SDRPATHSEGMENT_TOGGLE == eKind || SDRPATHSEGMENT_CURVE == eKind)
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

bool PolyPolygonEditor::SetPointsSmooth( basegfx::B2VectorContinuity eFlags, const std::set< sal_uInt16 >& rAbsPoints)
{
    bool bPolyPolygonChanged(false);

    std::set< sal_uInt16 >::const_reverse_iterator aIter;( rAbsPoints.rbegin() );
    for( aIter = rAbsPoints.rbegin(); aIter != rAbsPoints.rend(); ++aIter )
    {
        sal_uInt32 nPolyNum, nPntNum;

        if(PolyPolygonEditor::GetRelativePolyPoint(maPolyPolygon, (*aIter), nPolyNum, nPntNum))
        {
            // do change at aNewPolyPolygon...
            basegfx::B2DPolygon aCandidate(maPolyPolygon.getB2DPolygon(nPolyNum));

            // set continuity in point, make sure there is a curve
            bool bPolygonChanged(false);
            bPolygonChanged = basegfx::tools::expandToCurveInPoint(aCandidate, nPntNum);
            bPolygonChanged |= basegfx::tools::setContinuityInPoint(aCandidate, nPntNum, eFlags);

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
    sal_uInt32 nPolyNum(0L);

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
