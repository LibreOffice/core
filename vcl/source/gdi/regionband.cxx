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

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <regionband.hxx>
#include <osl/diagnose.h>

RegionBand::RegionBand()
:   mpFirstBand(0),
    mpLastCheckedBand(0)
{
}

RegionBand::RegionBand(const RegionBand& rRef)
:   mpFirstBand(0),
    mpLastCheckedBand(0)
{
    *this = rRef;
}

RegionBand& RegionBand::operator=(const RegionBand& rRef)
{
    ImplRegionBand* pPrevBand = 0;
    ImplRegionBand* pBand = rRef.mpFirstBand;

    while(pBand)
    {
        ImplRegionBand* pNewBand = new ImplRegionBand(*pBand);

        // first element? -> set as first into the list
        if(pBand == rRef.mpFirstBand)
        {
            mpFirstBand = pNewBand;
        }
        else
        {
            pPrevBand->mpNextBand = pNewBand;
        }

        pPrevBand = pNewBand;
        pBand = pBand->mpNextBand;
    }

    return *this;
}

RegionBand::RegionBand(const Rectangle& rRect)
:   mpFirstBand(0),
    mpLastCheckedBand(0)
{
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));

    // add band with boundaries of the rectangle
    mpFirstBand = new ImplRegionBand(nTop, nBottom);

    // Set left and right boundaries of the band
    mpFirstBand->Union(nLeft, nRight);

}

void RegionBand::implReset()
{
    ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        ImplRegionBand* pTempBand = pBand->mpNextBand;
        delete pBand;
        pBand = pTempBand;
    }

    mpLastCheckedBand = 0;
    mpFirstBand = 0;
}

RegionBand::~RegionBand()
{
    implReset();
}

bool RegionBand::operator==( const RegionBand& rRegionBand ) const
{

    // initialise pointers
    ImplRegionBand*      pOwnRectBand = mpFirstBand;
    ImplRegionBandSep*   pOwnRectBandSep = pOwnRectBand->mpFirstSep;
    ImplRegionBand*      pSecondRectBand = rRegionBand.mpFirstBand;
    ImplRegionBandSep*   pSecondRectBandSep = pSecondRectBand->mpFirstSep;

    while ( pOwnRectBandSep && pSecondRectBandSep )
    {
        // get boundaries of current rectangle
        long nOwnXLeft = pOwnRectBandSep->mnXLeft;
        long nSecondXLeft = pSecondRectBandSep->mnXLeft;

        if ( nOwnXLeft != nSecondXLeft )
        {
            return false;
        }

        long nOwnYTop = pOwnRectBand->mnYTop;
        long nSecondYTop = pSecondRectBand->mnYTop;

        if ( nOwnYTop != nSecondYTop )
        {
            return false;
        }

        long nOwnXRight = pOwnRectBandSep->mnXRight;
        long nSecondXRight = pSecondRectBandSep->mnXRight;

        if ( nOwnXRight != nSecondXRight )
        {
            return false;
        }

        long nOwnYBottom = pOwnRectBand->mnYBottom;
        long nSecondYBottom = pSecondRectBand->mnYBottom;

        if ( nOwnYBottom != nSecondYBottom )
        {
            return false;
        }

        // get next separation from current band
        pOwnRectBandSep = pOwnRectBandSep->mpNextSep;

        // no separation found? -> go to next band!
        if ( !pOwnRectBandSep )
        {
            // get next band
            pOwnRectBand = pOwnRectBand->mpNextBand;

            // get first separation in current band
            if( pOwnRectBand )
            {
                pOwnRectBandSep = pOwnRectBand->mpFirstSep;
            }
        }

        // get next separation from current band
        pSecondRectBandSep = pSecondRectBandSep->mpNextSep;

        // no separation found? -> go to next band!
        if ( !pSecondRectBandSep )
        {
            // get next band
            pSecondRectBand = pSecondRectBand->mpNextBand;

            // get first separation in current band
            if( pSecondRectBand )
            {
                pSecondRectBandSep = pSecondRectBand->mpFirstSep;
            }
        }

        if ( pOwnRectBandSep && !pSecondRectBandSep )
        {
            return false;
        }

        if ( !pOwnRectBandSep && pSecondRectBandSep )
        {
            return false;
        }
    }

    return true;
}

enum StreamEntryType { STREAMENTRY_BANDHEADER, STREAMENTRY_SEPARATION, STREAMENTRY_END };

void RegionBand::load(SvStream& rIStrm)
{
    // clear this instance data
    implReset();

    // get all bands
    ImplRegionBand* pCurrBand = 0;

    // get header from first element
    sal_uInt16 nTmp16(STREAMENTRY_END);
    rIStrm.ReadUInt16(nTmp16);

    if (STREAMENTRY_END == (StreamEntryType)nTmp16)
        return;

    size_t nRecordsPossible = rIStrm.remainingSize() / (2*sizeof(sal_Int32));
    if (!nRecordsPossible)
    {
        OSL_ENSURE(false, "premature end of region stream" );
        implReset();
        return;
    }

    do
    {
        // insert new band or new separation?
        if(STREAMENTRY_BANDHEADER == (StreamEntryType)nTmp16)
        {
            sal_Int32 nYTop(0);
            sal_Int32 nYBottom(0);

            rIStrm.ReadInt32( nYTop );
            rIStrm.ReadInt32( nYBottom );

            // create band
            ImplRegionBand* pNewBand = new ImplRegionBand( nYTop, nYBottom );

            // first element? -> set as first into the list
            if ( !pCurrBand )
            {
                mpFirstBand = pNewBand;
            }
            else
            {
                pCurrBand->mpNextBand = pNewBand;
            }

            // save pointer for next creation
            pCurrBand = pNewBand;
        }
        else
        {
            sal_Int32 nXLeft(0);
            sal_Int32 nXRight(0);

            rIStrm.ReadInt32( nXLeft );
            rIStrm.ReadInt32( nXRight );

            // add separation
            if ( pCurrBand )
            {
                pCurrBand->Union( nXLeft, nXRight );
            }
        }

        if( rIStrm.IsEof() )
        {
            OSL_ENSURE(false, "premature end of region stream" );
            implReset();
            return;
        }

        // get next header
        rIStrm.ReadUInt16( nTmp16 );
    }
    while (STREAMENTRY_END != (StreamEntryType)nTmp16 && rIStrm.good());
}

void RegionBand::save(SvStream& rOStrm) const
{
    ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        // put boundaries
        rOStrm.WriteUInt16( STREAMENTRY_BANDHEADER );
        rOStrm.WriteInt32( pBand->mnYTop );
        rOStrm.WriteInt32( pBand->mnYBottom );

        // put separations of current band
        ImplRegionBandSep* pSep = pBand->mpFirstSep;

        while(pSep)
        {
            // put separation
            rOStrm.WriteUInt16( STREAMENTRY_SEPARATION );
            rOStrm.WriteInt32( pSep->mnXLeft );
            rOStrm.WriteInt32( pSep->mnXRight );

            // next separation from current band
            pSep = pSep->mpNextSep;
        }

        pBand = pBand->mpNextBand;
    }

    // put endmarker
    rOStrm.WriteUInt16( STREAMENTRY_END );
}

bool RegionBand::isSingleRectangle() const
{
    // just one band?
    if(mpFirstBand && !mpFirstBand->mpNextBand)
    {
        // just one sep?
        if(mpFirstBand->mpFirstSep && !mpFirstBand->mpFirstSep->mpNextSep)
        {
            return true;
        }
    }

    return false;
}

void RegionBand::InsertBand(ImplRegionBand* pPreviousBand, ImplRegionBand* pBandToInsert)
{
    OSL_ASSERT(pBandToInsert!=NULL);

    if(!pPreviousBand)
    {
        // Insert band before all others.
        if(mpFirstBand)
        {
            mpFirstBand->mpPrevBand = pBandToInsert;
        }

        pBandToInsert->mpNextBand = mpFirstBand;
        mpFirstBand = pBandToInsert;
    }
    else
    {
        // Insert band directly after pPreviousBand.
        pBandToInsert->mpNextBand = pPreviousBand->mpNextBand;
        pPreviousBand->mpNextBand = pBandToInsert;
        pBandToInsert->mpPrevBand = pPreviousBand;
    }

}

void RegionBand::processPoints()
{
    ImplRegionBand* pRegionBand = mpFirstBand;

    while(pRegionBand)
    {
        // generate separations from the lines and process union
        pRegionBand->ProcessPoints();
        pRegionBand = pRegionBand->mpNextBand;
    }

}

/** This function is similar to the RegionBand::InsertBands() method.
    It creates a minimal set of missing bands so that the entire vertical
    interval from nTop to nBottom is covered by bands.
*/
void RegionBand::ImplAddMissingBands(const long nTop, const long nBottom)
{
    // Iterate over already existing bands and add missing bands atop the
    // first and between two bands.
    ImplRegionBand* pPreviousBand = NULL;
    ImplRegionBand* pBand = ImplGetFirstRegionBand();
    long nCurrentTop (nTop);

    while (pBand != NULL && nCurrentTop<nBottom)
    {
        if (nCurrentTop < pBand->mnYTop)
        {
            // Create new band above the current band.
            ImplRegionBand* pAboveBand = new ImplRegionBand(
                nCurrentTop,
                ::std::min(nBottom,pBand->mnYTop-1));
            InsertBand(pPreviousBand, pAboveBand);
        }

        // Adapt the top of the interval to prevent overlapping bands.
        nCurrentTop = ::std::max(nTop, pBand->mnYBottom+1);

        // Advance to next band.
        pPreviousBand = pBand;
        pBand = pBand->mpNextBand;
    }

    // We still have to cover two cases:
    // 1. The region does not yet contain any bands.
    // 2. The intervall nTop->nBottom extends past the bottom most band.
    if (nCurrentTop <= nBottom
        && (pBand==NULL || nBottom>pBand->mnYBottom))
    {
        // When there is no previous band then the new one will be the
        // first.  Otherwise the new band is inserted behind the last band.
        InsertBand(
            pPreviousBand,
            new ImplRegionBand(
                nCurrentTop,
                nBottom));
    }

}

void RegionBand::CreateBandRange(long nYTop, long nYBottom)
{
    // add top band
    mpFirstBand = new ImplRegionBand( nYTop-1, nYTop-1 );

    // begin first search from the first element
    mpLastCheckedBand = mpFirstBand;
    ImplRegionBand* pBand = mpFirstBand;

    for ( int i = nYTop; i <= nYBottom+1; i++ )
    {
        // create new band
        ImplRegionBand* pNewBand = new ImplRegionBand( i, i );
        pBand->mpNextBand = pNewBand;

        if ( pBand != mpFirstBand )
        {
            pNewBand->mpPrevBand = pBand;
        }

        pBand = pBand->mpNextBand;
    }

}

bool RegionBand::InsertLine(const Point& rStartPt, const Point& rEndPt, long nLineId)
{
    long nX, nY;

    // lines consisting of a single point do not interest here
    if ( rStartPt == rEndPt )
    {
        return true;
    }

    LineType eLineType = (rStartPt.Y() > rEndPt.Y()) ? LINE_DESCENDING : LINE_ASCENDING;
    if ( rStartPt.X() == rEndPt.X() )
    {
        // vertical line
        const long nEndY = rEndPt.Y();

        nX = rStartPt.X();
        nY = rStartPt.Y();

        if( nEndY > nY )
        {
            for ( ; nY <= nEndY; nY++ )
            {
                Point aNewPoint( nX, nY );
                InsertPoint( aNewPoint, nLineId,
                             (aNewPoint == rEndPt) || (aNewPoint == rStartPt),
                             eLineType );
            }
        }
        else
        {
            for ( ; nY >= nEndY; nY-- )
            {
                Point aNewPoint( nX, nY );
                InsertPoint( aNewPoint, nLineId,
                             (aNewPoint == rEndPt) || (aNewPoint == rStartPt),
                             eLineType );
            }
        }
    }
    else if ( rStartPt.Y() != rEndPt.Y() )
    {
        const long  nDX = labs( rEndPt.X() - rStartPt.X() );
        const long  nDY = labs( rEndPt.Y() - rStartPt.Y() );
        const long  nStartX = rStartPt.X();
        const long  nStartY = rStartPt.Y();
        const long  nEndX = rEndPt.X();
        const long  nEndY = rEndPt.Y();
        const long  nXInc = ( nStartX < nEndX ) ? 1L : -1L;
        const long  nYInc = ( nStartY < nEndY ) ? 1L : -1L;

        if ( nDX >= nDY )
        {
            const long  nDYX = ( nDY - nDX ) * 2;
            const long  nDY2 = nDY << 1;
            long        nD = nDY2 - nDX;

            for ( nX = nStartX, nY = nStartY; nX != nEndX; nX += nXInc )
            {
                InsertPoint( Point( nX, nY ), nLineId, nStartX == nX, eLineType );

                if ( nD < 0L )
                    nD += nDY2;
                else
                    nD += nDYX, nY += nYInc;
            }
        }
        else
        {
            const long  nDYX = ( nDX - nDY ) * 2;
            const long  nDY2 = nDX << 1;
            long        nD = nDY2 - nDY;

            for ( nX = nStartX, nY = nStartY; nY != nEndY; nY += nYInc )
            {
                InsertPoint( Point( nX, nY ), nLineId, nStartY == nY, eLineType );

                if ( nD < 0L )
                    nD += nDY2;
                else
                    nD += nDYX, nX += nXInc;
            }
        }

        // last point
        InsertPoint( Point( nEndX, nEndY ), nLineId, true, eLineType );
    }

    return true;
}

bool RegionBand::InsertPoint(const Point &rPoint, long nLineID, bool bEndPoint, LineType eLineType)
{
    DBG_ASSERT( mpFirstBand != NULL, "RegionBand::InsertPoint - no bands available!" );

    if ( rPoint.Y() == mpLastCheckedBand->mnYTop )
    {
        mpLastCheckedBand->InsertPoint( rPoint.X(), nLineID, bEndPoint, eLineType );
        return true;
    }

    if ( rPoint.Y() > mpLastCheckedBand->mnYTop )
    {
        // Search ascending
        while ( mpLastCheckedBand )
        {
            // Insert point if possible
            if ( rPoint.Y() == mpLastCheckedBand->mnYTop )
            {
                mpLastCheckedBand->InsertPoint( rPoint.X(), nLineID, bEndPoint, eLineType );
                return true;
            }

            mpLastCheckedBand = mpLastCheckedBand->mpNextBand;
        }

        OSL_ENSURE(false, "RegionBand::InsertPoint reached the end of the list!" );
    }
    else
    {
        // Search descending
        while ( mpLastCheckedBand )
        {
            // Insert point if possible
            if ( rPoint.Y() == mpLastCheckedBand->mnYTop )
            {
                mpLastCheckedBand->InsertPoint( rPoint.X(), nLineID, bEndPoint, eLineType );
                return true;
            }

            mpLastCheckedBand = mpLastCheckedBand->mpPrevBand;
        }

        OSL_ENSURE(false, "RegionBand::InsertPoint reached the beginning of the list!" );
    }

    OSL_ENSURE(false, "RegionBand::InsertPoint point not inserted!" );

    // reinitialize pointer (should never be reached!)
    mpLastCheckedBand = mpFirstBand;

    return false;
}

bool RegionBand::OptimizeBandList()
{
    ImplRegionBand* pPrevBand = 0;
    ImplRegionBand* pBand = mpFirstBand;

    while ( pBand )
    {
        const bool bBTEqual = pBand->mpNextBand && (pBand->mnYBottom == pBand->mpNextBand->mnYTop);

        // no separation? -> remove!
        if ( pBand->IsEmpty() || (bBTEqual && (pBand->mnYBottom == pBand->mnYTop)) )
        {
            // save pointer
            ImplRegionBand* pOldBand = pBand;

            // previous element of the list
            if ( pBand == mpFirstBand )
                mpFirstBand = pBand->mpNextBand;
            else
                pPrevBand->mpNextBand = pBand->mpNextBand;

            pBand = pBand->mpNextBand;
            delete pOldBand;
        }
        else
        {
            // fixup
            if ( bBTEqual )
                pBand->mnYBottom = pBand->mpNextBand->mnYTop-1;

            // this and next band with equal separations? -> combine!
            if ( pBand->mpNextBand &&
                 ((pBand->mnYBottom+1) == pBand->mpNextBand->mnYTop) &&
                 (*pBand == *pBand->mpNextBand) )
            {
                // expand current height
                pBand->mnYBottom = pBand->mpNextBand->mnYBottom;

                // remove next band from list
                ImplRegionBand* pDeletedBand = pBand->mpNextBand;
                pBand->mpNextBand = pDeletedBand->mpNextBand;
                delete pDeletedBand;

                // check band again!
            }
            else
            {
                // count rectangles within band
                ImplRegionBandSep* pSep = pBand->mpFirstSep;
                while ( pSep )
                {
                    pSep = pSep->mpNextSep;
                }

                pPrevBand = pBand;
                pBand = pBand->mpNextBand;
            }
        }
    }

#ifdef DBG_UTIL
    pBand = mpFirstBand;
    while ( pBand )
    {
        DBG_ASSERT( pBand->mpFirstSep != NULL, "Exiting RegionBand::OptimizeBandList(): empty band in region!" );

        if ( pBand->mnYBottom < pBand->mnYTop )
            OSL_ENSURE(false, "RegionBand::OptimizeBandList(): YBottomBoundary < YTopBoundary" );

        if ( pBand->mpNextBand )
        {
            if ( pBand->mnYBottom >= pBand->mpNextBand->mnYTop )
                OSL_ENSURE(false, "RegionBand::OptimizeBandList(): overlapping bands in region!" );
        }

        pBand = pBand->mpNextBand;
    }
#endif

    return (0 != mpFirstBand);
}

void RegionBand::Move(long nHorzMove, long nVertMove)
{
    ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        // process the vertical move
        if(nVertMove)
        {
            pBand->mnYTop = pBand->mnYTop + nVertMove;
            pBand->mnYBottom = pBand->mnYBottom + nVertMove;
        }

        // process the horizontal move
        if(nHorzMove)
        {
            pBand->MoveX(nHorzMove);
        }

        pBand = pBand->mpNextBand;
    }

}

void RegionBand::Scale(double fScaleX, double fScaleY)
{
    ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        // process the vertical move
        if(0.0 != fScaleY)
        {
            pBand->mnYTop = basegfx::fround(pBand->mnYTop * fScaleY);
            pBand->mnYBottom = basegfx::fround(pBand->mnYBottom * fScaleY);
        }

        // process the horizontal move
        if(0.0 != fScaleX)
        {
            pBand->ScaleX(fScaleX);
        }

        pBand = pBand->mpNextBand;
    }

}

void RegionBand::InsertBands(long nTop, long nBottom)
{
    // region empty? -> set rectangle as first entry!
    if ( !mpFirstBand )
    {
        // add band with boundaries of the rectangle
        mpFirstBand = new ImplRegionBand( nTop, nBottom );
        return;
    }

    // find/insert bands for the boundaries of the rectangle
    bool bTopBoundaryInserted = false;
    bool bTop2BoundaryInserted = false;
    bool bBottomBoundaryInserted = false;

    // special case: top boundary is above the first band
    ImplRegionBand* pNewBand;

    if ( nTop < mpFirstBand->mnYTop )
    {
        // create new band above the first in the list
        pNewBand = new ImplRegionBand( nTop, mpFirstBand->mnYTop );

        if ( nBottom < mpFirstBand->mnYTop )
        {
            pNewBand->mnYBottom = nBottom;
        }

        // insert band into the list
        pNewBand->mpNextBand = mpFirstBand;
        mpFirstBand = pNewBand;

        bTopBoundaryInserted = true;
    }

    // insert band(s) into the list
    ImplRegionBand* pBand = mpFirstBand;

    while ( pBand )
    {
        // Insert Bands if possible
        if ( !bTopBoundaryInserted )
        {
            bTopBoundaryInserted = InsertSingleBand( pBand, nTop - 1 );
        }

        if ( !bTop2BoundaryInserted )
        {
            bTop2BoundaryInserted = InsertSingleBand( pBand, nTop );
        }

        if ( !bBottomBoundaryInserted && (nTop != nBottom) )
        {
            bBottomBoundaryInserted = InsertSingleBand( pBand, nBottom );
        }

        // both boundaries inserted? -> nothing more to do
        if ( bTopBoundaryInserted && bTop2BoundaryInserted && bBottomBoundaryInserted )
        {
            break;
        }

        // insert bands between two bands if necessary
        if ( pBand->mpNextBand )
        {
            if ( (pBand->mnYBottom + 1) < pBand->mpNextBand->mnYTop )
            {
                // copy band with list and set new boundary
                pNewBand = new ImplRegionBand( pBand->mnYBottom+1, pBand->mpNextBand->mnYTop-1 );

                // insert band into the list
                pNewBand->mpNextBand = pBand->mpNextBand;
                pBand->mpNextBand = pNewBand;
            }
        }

        pBand = pBand->mpNextBand;
    }

}

bool RegionBand::InsertSingleBand(ImplRegionBand* pBand, long nYBandPosition)
{
    // boundary already included in band with height 1? -> nothing to do!
    if ( (pBand->mnYTop == pBand->mnYBottom) && (nYBandPosition == pBand->mnYTop) )
    {
        return true;
    }

    // insert single height band on top?
    ImplRegionBand* pNewBand;

    if ( nYBandPosition == pBand->mnYTop )
    {
        // copy band with list and set new boundary
        pNewBand = new ImplRegionBand( *pBand );
        pNewBand->mnYTop = nYBandPosition+1;

        // insert band into the list
        pNewBand->mpNextBand = pBand->mpNextBand;
        pBand->mnYBottom = nYBandPosition;
        pBand->mpNextBand = pNewBand;

        return true;
    }

    // top of new rectangle within the current band? -> insert new band and copy data
    if ( (nYBandPosition > pBand->mnYTop) && (nYBandPosition < pBand->mnYBottom) )
    {
        // copy band with list and set new boundary
        pNewBand = new ImplRegionBand( *pBand );
        pNewBand->mnYTop = nYBandPosition;

        // insert band into the list
        pNewBand->mpNextBand = pBand->mpNextBand;
        pBand->mnYBottom = nYBandPosition;
        pBand->mpNextBand = pNewBand;

        // copy band with list and set new boundary
        pNewBand = new ImplRegionBand( *pBand );
        pNewBand->mnYTop = nYBandPosition;

        // insert band into the list
        pBand->mpNextBand->mnYTop = nYBandPosition+1;

        pNewBand->mpNextBand = pBand->mpNextBand;
        pBand->mnYBottom = nYBandPosition - 1;
        pBand->mpNextBand = pNewBand;

        return true;
    }

    // create new band behind the current in the list
    if ( !pBand->mpNextBand )
    {
        if ( nYBandPosition == pBand->mnYBottom )
        {
            // copy band with list and set new boundary
            pNewBand = new ImplRegionBand( *pBand );
            pNewBand->mnYTop = pBand->mnYBottom;
            pNewBand->mnYBottom = nYBandPosition;

            pBand->mnYBottom = nYBandPosition-1;

            // append band to the list
            pBand->mpNextBand = pNewBand;
            return true;
        }

        if ( nYBandPosition > pBand->mnYBottom )
        {
            // create new band
            pNewBand = new ImplRegionBand( pBand->mnYBottom + 1, nYBandPosition );

            // append band to the list
            pBand->mpNextBand = pNewBand;
            return true;
        }
    }

    return false;
}

void RegionBand::Union(long nLeft, long nTop, long nRight, long nBottom)
{
    DBG_ASSERT( nLeft <= nRight, "RegionBand::Union() - nLeft > nRight" );
    DBG_ASSERT( nTop <= nBottom, "RegionBand::Union() - nTop > nBottom" );

    // process union
    ImplRegionBand* pBand = mpFirstBand;
    while ( pBand )
    {
        if ( pBand->mnYTop >= nTop )
        {
            if ( pBand->mnYBottom <= nBottom )
                pBand->Union( nLeft, nRight );
            else
            {
#ifdef DBG_UTIL
                long nCurY = pBand->mnYBottom;
                pBand = pBand->mpNextBand;
                while ( pBand )
                {
                    if ( (pBand->mnYTop < nCurY) || (pBand->mnYBottom < nCurY) )
                    {
                        OSL_ENSURE(false, "RegionBand::Union() - Bands not sorted!" );
                    }
                    pBand = pBand->mpNextBand;
                }
#endif
                break;
            }
        }

        pBand = pBand->mpNextBand;
    }

}

void RegionBand::Intersect(long nLeft, long nTop, long nRight, long nBottom)
{
    // process intersections
    ImplRegionBand* pPrevBand = 0;
    ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        // band within intersection boundary? -> process. otherwise remove
        if((pBand->mnYTop >= nTop) && (pBand->mnYBottom <= nBottom))
        {
            // process intersection
            pBand->Intersect(nLeft, nRight);
            pPrevBand = pBand;
            pBand = pBand->mpNextBand;
        }
        else
        {
            ImplRegionBand* pOldBand = pBand;

            if(pBand == mpFirstBand)
            {
                mpFirstBand = pBand->mpNextBand;
            }
            else
            {
                pPrevBand->mpNextBand = pBand->mpNextBand;
            }

            pBand = pBand->mpNextBand;
            delete pOldBand;
        }
    }

}

void RegionBand::Union(const RegionBand& rSource)
{
    // apply all rectangles from rSource to this
    ImplRegionBand* pBand = rSource.mpFirstBand;

    while ( pBand )
    {
        // insert bands if the boundaries are not already in the list
        InsertBands(pBand->mnYTop, pBand->mnYBottom);

        // process all elements of the list
        ImplRegionBandSep* pSep = pBand->mpFirstSep;

        while(pSep)
        {
            Union(pSep->mnXLeft, pBand->mnYTop, pSep->mnXRight, pBand->mnYBottom);
            pSep = pSep->mpNextSep;
        }

        pBand = pBand->mpNextBand;
    }

}

void RegionBand::Exclude(long nLeft, long nTop, long nRight, long nBottom)
{
    DBG_ASSERT( nLeft <= nRight, "RegionBand::Exclude() - nLeft > nRight" );
    DBG_ASSERT( nTop <= nBottom, "RegionBand::Exclude() - nTop > nBottom" );

    // process exclude
    ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        if(pBand->mnYTop >= nTop)
        {
            if(pBand->mnYBottom <= nBottom)
            {
                pBand->Exclude(nLeft, nRight);
            }
            else
            {
#ifdef DBG_UTIL
                long nCurY = pBand->mnYBottom;
                pBand = pBand->mpNextBand;

                while(pBand)
                {
                    if((pBand->mnYTop < nCurY) || (pBand->mnYBottom < nCurY))
                    {
                        OSL_ENSURE(false, "RegionBand::Exclude() - Bands not sorted!" );
                    }

                    pBand = pBand->mpNextBand;
                }
#endif
                break;
            }
        }

        pBand = pBand->mpNextBand;
    }

}

void RegionBand::XOr(long nLeft, long nTop, long nRight, long nBottom)
{
    DBG_ASSERT( nLeft <= nRight, "RegionBand::Exclude() - nLeft > nRight" );
    DBG_ASSERT( nTop <= nBottom, "RegionBand::Exclude() - nTop > nBottom" );

    // process xor
    ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        if(pBand->mnYTop >= nTop)
        {
            if(pBand->mnYBottom <= nBottom)
            {
                pBand->XOr(nLeft, nRight);
            }
            else
            {
#ifdef DBG_UTIL
                long nCurY = pBand->mnYBottom;
                pBand = pBand->mpNextBand;

                while(pBand)
                {
                    if((pBand->mnYTop < nCurY) || (pBand->mnYBottom < nCurY))
                    {
                        OSL_ENSURE(false, "RegionBand::XOr() - Bands not sorted!" );
                    }

                    pBand = pBand->mpNextBand;
                }
#endif
                break;
            }
        }

        pBand = pBand->mpNextBand;
    }

}

void RegionBand::Intersect(const RegionBand& rSource)
{
    // mark all bands as untouched
    ImplRegionBand* pBand = mpFirstBand;

    while ( pBand )
    {
        pBand->mbTouched = false;
        pBand = pBand->mpNextBand;
    }

    pBand = rSource.mpFirstBand;

    while ( pBand )
    {
        // insert bands if the boundaries are not already in the list
        InsertBands( pBand->mnYTop, pBand->mnYBottom );

        // process all elements of the list
        ImplRegionBandSep* pSep = pBand->mpFirstSep;

        while ( pSep )
        {
            // left boundary?
            if ( pSep == pBand->mpFirstSep )
            {
                // process intersection and do not remove untouched bands
                Exclude( LONG_MIN+1, pBand->mnYTop, pSep->mnXLeft-1, pBand->mnYBottom );
            }

            // right boundary?
            if ( pSep->mpNextSep == NULL )
            {
                // process intersection and do not remove untouched bands
                Exclude( pSep->mnXRight+1, pBand->mnYTop, LONG_MAX-1, pBand->mnYBottom );
            }
            else
            {
                // process intersection and do not remove untouched bands
                Exclude( pSep->mnXRight+1, pBand->mnYTop, pSep->mpNextSep->mnXLeft-1, pBand->mnYBottom );
            }

            pSep = pSep->mpNextSep;
        }

        pBand = pBand->mpNextBand;
    }

    // remove all untouched bands if bands already left
    ImplRegionBand* pPrevBand = 0;
    pBand = mpFirstBand;

    while ( pBand )
    {
        if ( !pBand->mbTouched )
        {
            // save pointer
            ImplRegionBand* pOldBand = pBand;

            // previous element of the list
            if ( pBand == mpFirstBand )
            {
                mpFirstBand = pBand->mpNextBand;
            }
            else
            {
                pPrevBand->mpNextBand = pBand->mpNextBand;
            }

            pBand = pBand->mpNextBand;
            delete pOldBand;
        }
        else
        {
            pPrevBand = pBand;
            pBand = pBand->mpNextBand;
        }
    }

}

bool RegionBand::Exclude(const RegionBand& rSource)
{
    // apply all rectangles to the region passed to this region
    ImplRegionBand* pBand = rSource.mpFirstBand;

    while ( pBand )
    {
        // insert bands if the boundaries are not already in the list
        InsertBands( pBand->mnYTop, pBand->mnYBottom );

        // process all elements of the list
        ImplRegionBandSep* pSep = pBand->mpFirstSep;

        while ( pSep )
        {
            Exclude( pSep->mnXLeft, pBand->mnYTop, pSep->mnXRight, pBand->mnYBottom );
            pSep = pSep->mpNextSep;
        }

        // to test less bands, already check in the loop
        if ( !OptimizeBandList() )
        {
            return false;
        }

        pBand = pBand->mpNextBand;
    }

    return true;
}

Rectangle RegionBand::GetBoundRect() const
{

    // get the boundaries of the first band
    long nYTop(mpFirstBand->mnYTop);
    long nYBottom(mpFirstBand->mnYBottom);
    long nXLeft(mpFirstBand->GetXLeftBoundary());
    long nXRight(mpFirstBand->GetXRightBoundary());

    // look in the band list (don't test first band again!)
    ImplRegionBand* pBand = mpFirstBand->mpNextBand;

    while ( pBand )
    {
        nYBottom = pBand->mnYBottom;
        nXLeft = std::min( nXLeft, pBand->GetXLeftBoundary() );
        nXRight = std::max( nXRight, pBand->GetXRightBoundary() );

        pBand = pBand->mpNextBand;
    }

    return Rectangle( nXLeft, nYTop, nXRight, nYBottom );
}

void RegionBand::XOr(const RegionBand& rSource)
{
    ImplRegionBand* pBand = rSource.mpFirstBand;

    while ( pBand )
    {
        // insert bands if the boundaries are not already in the list
        InsertBands( pBand->mnYTop, pBand->mnYBottom );

        // process all elements of the list
        ImplRegionBandSep* pSep = pBand->mpFirstSep;

        while ( pSep )
        {
            XOr( pSep->mnXLeft, pBand->mnYTop, pSep->mnXRight, pBand->mnYBottom );
            pSep = pSep->mpNextSep;
        }

        pBand = pBand->mpNextBand;
    }
}

bool RegionBand::IsInside(const Point& rPoint) const
{

    // search band list
    ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        // is point within band?
        if((pBand->mnYTop <= rPoint.Y()) && (pBand->mnYBottom >= rPoint.Y()))
        {
            // is point within separation of the band?
            if(pBand->IsInside(rPoint.X()))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        pBand = pBand->mpNextBand;
    }

    return false;
}

void RegionBand::GetRegionRectangles(RectangleVector& rTarget) const
{
    // clear result vector
    rTarget.clear();
    ImplRegionBand* mpCurrRectBand = mpFirstBand;
    Rectangle aRectangle;

    while(mpCurrRectBand)
    {
        ImplRegionBandSep* mpCurrRectBandSep = mpCurrRectBand->mpFirstSep;

        aRectangle.Top() = mpCurrRectBand->mnYTop;
        aRectangle.Bottom() = mpCurrRectBand->mnYBottom;

        while(mpCurrRectBandSep)
        {
            aRectangle.Left() = mpCurrRectBandSep->mnXLeft;
            aRectangle.Right() = mpCurrRectBandSep->mnXRight;
            rTarget.push_back(aRectangle);
            mpCurrRectBandSep = mpCurrRectBandSep->mpNextSep;
        }

        mpCurrRectBand = mpCurrRectBand->mpNextBand;
    }
}

sal_uInt32 RegionBand::getRectangleCount() const
{
    sal_uInt32 nCount = 0;
    const ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        ImplRegionBandSep* pSep = pBand->mpFirstSep;

        while(pSep)
        {
            nCount++;
            pSep = pSep->mpNextSep;
        }

        pBand = pBand->mpNextBand;
    }

    return nCount;
}

#ifdef DBG_UTIL
const char* ImplDbgTestRegionBand(const void* pObj)
{
    const RegionBand* pRegionBand = static_cast< const RegionBand* >(pObj);

    if(pRegionBand)
    {
        const ImplRegionBand* pBand = pRegionBand->ImplGetFirstRegionBand();

        while(pBand)
        {
            if(pBand->mnYBottom < pBand->mnYTop)
            {
                return "YBottom < YTop";
            }

            if(pBand->mpNextBand)
            {
                if(pBand->mnYBottom >= pBand->mpNextBand->mnYTop)
                {
                    return "overlapping bands in region";
                }
            }

            if(pBand->mbTouched)
            {
                return "Band-mbTouched overwrite";
            }

            ImplRegionBandSep* pSep = pBand->mpFirstSep;

            while(pSep)
            {
                if(pSep->mnXRight < pSep->mnXLeft)
                {
                    return "XLeft < XRight";
                }

                if(pSep->mpNextSep)
                {
                    if(pSep->mnXRight >= pSep->mpNextSep->mnXLeft)
                    {
                        return "overlapping separations in region";
                    }
                }

                if ( pSep->mbRemoved )
                {
                    return "Sep-mbRemoved overwrite";
                }

                pSep = pSep->mpNextSep;
            }

            pBand = pBand->mpNextBand;
        }
    }

    return 0;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
