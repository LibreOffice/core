/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polypolygoneditor.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 11:51:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    for( aIter = rAbsPoints.rbegin(); aIter != rAbsPoints.rend(); aIter++ )
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
    for( aIter = rAbsPoints.rbegin(); aIter != rAbsPoints.rend(); aIter++ )
    {
        sal_uInt32 nPolyNum, nPntNum;

        if(PolyPolygonEditor::GetRelativePolyPoint(maPolyPolygon, (*aIter), nPolyNum, nPntNum))
        {
            // do change at aNewPolyPolygon. Take a look at edge.
            basegfx::B2DPolygon aCandidate(maPolyPolygon.getB2DPolygon(nPolyNum));
            bool bCandidateChanged(false);
            const sal_uInt32 nCount(aCandidate.count());

            if(nCount && (nPntNum < nCount || aCandidate.isClosed()))
            {
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
    for( aIter = rAbsPoints.rbegin(); aIter != rAbsPoints.rend(); aIter++ )
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
