/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include "svx/polypolygoneditor.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace PolyPolygonEditor
    {
        bool DeletePoints(basegfx::B2DPolyPolygon& o_rCandidate, const sdr::selection::Indices& rAbsPoints)
        {
            bool bPolyPolyChanged(false);

            for(sdr::selection::Indices::const_reverse_iterator aIter(rAbsPoints.rbegin()); aIter != rAbsPoints.rend(); aIter++)
            {
                sal_uInt32 nPoly, nPnt;

                if(GetRelativePolyPoint(o_rCandidate, (*aIter), nPoly, nPnt))
                {
                    // remove point
                    basegfx::B2DPolygon aCandidate(o_rCandidate.getB2DPolygon(nPoly));

                    aCandidate.remove(nPnt);

                    if((aCandidate.isClosed() && aCandidate.count() < 3) || (aCandidate.count() < 2))
                    {
                        o_rCandidate.remove(nPoly);
                    }
                    else
                    {
                        o_rCandidate.setB2DPolygon(nPoly, aCandidate);
                    }

                    bPolyPolyChanged = true;
                }
            }

            return bPolyPolyChanged;
        }

        bool SetSegmentsKind(basegfx::B2DPolyPolygon& o_rCandidate, SdrPathSegmentKind eKind, const sdr::selection::Indices& rAbsPoints)
        {
            bool bPolyPolyChanged(false);

            for(sdr::selection::Indices::const_reverse_iterator aIter(rAbsPoints.rbegin()); aIter != rAbsPoints.rend(); aIter++)
            {
                sal_uInt32 nPolyNum, nPntNum;

                if(GetRelativePolyPoint(o_rCandidate, (*aIter), nPolyNum, nPntNum))
                {
                    // do change at aNewPolyPolygon. Take a look at edge.
                    basegfx::B2DPolygon aCandidate(o_rCandidate.getB2DPolygon(nPolyNum));
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
                            o_rCandidate.setB2DPolygon(nPolyNum, aCandidate);
                            bPolyPolyChanged = true;
                        }
                    }
                }
            }

            return bPolyPolyChanged;
        }

        bool SetPointsSmooth(basegfx::B2DPolyPolygon& o_rCandidate, basegfx::B2VectorContinuity eFlags, const sdr::selection::Indices& rAbsPoints)
        {
            bool bPolyPolygonChanged(false);
            sdr::selection::Indices::const_reverse_iterator aIter;

            for( aIter = rAbsPoints.rbegin(); aIter != rAbsPoints.rend(); aIter++ )
            {
                sal_uInt32 nPolyNum, nPntNum;

                if(GetRelativePolyPoint(o_rCandidate, (*aIter), nPolyNum, nPntNum))
                {
                    // do change at aNewPolyPolygon...
                    basegfx::B2DPolygon aCandidate(o_rCandidate.getB2DPolygon(nPolyNum));

                    // set continuity in point, make sure there is a curve
                    bool bPolygonChanged(false);
                    bPolygonChanged = basegfx::tools::expandToCurveInPoint(aCandidate, nPntNum);
                    bPolygonChanged |= basegfx::tools::setContinuityInPoint(aCandidate, nPntNum, eFlags);

                    if(bPolygonChanged)
                    {
                                o_rCandidate.setB2DPolygon(nPolyNum, aCandidate);
                        bPolyPolygonChanged = true;
                    }
                }
            }

            return bPolyPolygonChanged;
        }

        bool GetRelativePolyPoint(const basegfx::B2DPolyPolygon& rPoly, sal_uInt32 nAbsPnt, sal_uInt32& rPolyNum, sal_uInt32& rPointNum)
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
    } // end of namespace PolyPolygonEditor
} // end of namespace sdr

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
