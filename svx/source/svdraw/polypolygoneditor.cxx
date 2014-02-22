/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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
            
            basegfx::B2DPolygon aCandidate(maPolyPolygon.getB2DPolygon(nPolyNum));
            bool bCandidateChanged(false);
            const sal_uInt32 nCount(aCandidate.count());

            if(nCount && (nPntNum + 1 < nCount || aCandidate.isClosed()))
            {
                
                const sal_uInt32 nNextIndex((nPntNum + 1) % nCount);
                const bool bContolUsed(aCandidate.areControlPointsUsed()
                    && (aCandidate.isNextControlPointUsed(nPntNum) || aCandidate.isPrevControlPointUsed(nNextIndex)));

                if(bContolUsed)
                {
                    if(SDRPATHSEGMENT_TOGGLE == eKind || SDRPATHSEGMENT_LINE == eKind)
                    {
                        
                        aCandidate.resetNextControlPoint(nPntNum);
                        aCandidate.resetPrevControlPoint(nNextIndex);
                        bCandidateChanged = true;
                    }
                }
                else
                {
                    if(SDRPATHSEGMENT_TOGGLE == eKind || SDRPATHSEGMENT_CURVE == eKind)
                    {
                        
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
            
            basegfx::B2DPolygon aCandidate(maPolyPolygon.getB2DPolygon(nPolyNum));

            
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
