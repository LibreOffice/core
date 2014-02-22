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

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <regionband.hxx>






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

    
    mpFirstBand = new ImplRegionBand(nTop, nBottom);

    
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

}

RegionBand::~RegionBand()
{
    implReset();
}

bool RegionBand::operator==( const RegionBand& rRegionBand ) const
{

    
    ImplRegionBand*      pOwnRectBand = mpFirstBand;
    ImplRegionBandSep*   pOwnRectBandSep = pOwnRectBand->mpFirstSep;
    ImplRegionBand*      pSecondRectBand = rRegionBand.mpFirstBand;
    ImplRegionBandSep*   pSecondRectBandSep = pSecondRectBand->mpFirstSep;

    while ( pOwnRectBandSep && pSecondRectBandSep )
    {
        
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

        
        pOwnRectBandSep = pOwnRectBandSep->mpNextSep;

        
        if ( !pOwnRectBandSep )
        {
            
            pOwnRectBand = pOwnRectBand->mpNextBand;

            
            if( pOwnRectBand )
            {
                pOwnRectBandSep = pOwnRectBand->mpFirstSep;
            }
        }

        
        pSecondRectBandSep = pSecondRectBandSep->mpNextSep;

        
        if ( !pSecondRectBandSep )
        {
            
            pSecondRectBand = pSecondRectBand->mpNextBand;

            
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
    
    implReset();

    
    ImplRegionBand* pCurrBand = 0;

    
    sal_uInt16 nTmp16(0);
    rIStrm.ReadUInt16( nTmp16 );

    while(STREAMENTRY_END != (StreamEntryType)nTmp16)
    {
        
        if(STREAMENTRY_BANDHEADER == (StreamEntryType)nTmp16)
        {
            sal_Int32 nYTop(0);
            sal_Int32 nYBottom(0);

            rIStrm.ReadInt32( nYTop );
            rIStrm.ReadInt32( nYBottom );

            
            ImplRegionBand* pNewBand = new ImplRegionBand( nYTop, nYBottom );

            
            if ( !pCurrBand )
            {
                mpFirstBand = pNewBand;
            }
            else
            {
                pCurrBand->mpNextBand = pNewBand;
            }

            
            pCurrBand = pNewBand;
        }
        else
        {
            sal_Int32 nXLeft(0);
            sal_Int32 nXRight(0);

            rIStrm.ReadInt32( nXLeft );
            rIStrm.ReadInt32( nXRight );

            
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

        
        rIStrm.ReadUInt16( nTmp16 );
    }

}

void RegionBand::save(SvStream& rOStrm) const
{
    ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        
        rOStrm.WriteUInt16( (sal_uInt16)STREAMENTRY_BANDHEADER );
        rOStrm.WriteInt32( static_cast<sal_Int32>(pBand->mnYTop) );
        rOStrm.WriteInt32( static_cast<sal_Int32>(pBand->mnYBottom) );

        
        ImplRegionBandSep* pSep = pBand->mpFirstSep;

        while(pSep)
        {
            
            rOStrm.WriteUInt16( (sal_uInt16)STREAMENTRY_SEPARATION );
            rOStrm.WriteInt32( static_cast<sal_Int32>(pSep->mnXLeft) );
            rOStrm.WriteInt32( static_cast<sal_Int32>(pSep->mnXRight) );

            
            pSep = pSep->mpNextSep;
        }

        pBand = pBand->mpNextBand;
    }

    
    rOStrm.WriteUInt16( (sal_uInt16)STREAMENTRY_END );
}

bool RegionBand::isSingleRectangle() const
{
    
    if(mpFirstBand && !mpFirstBand->mpNextBand)
    {
        
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
        
        if(mpFirstBand)
        {
            mpFirstBand->mpPrevBand = pBandToInsert;
        }

        pBandToInsert->mpNextBand = mpFirstBand;
        mpFirstBand = pBandToInsert;
    }
    else
    {
        
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
    
    
    ImplRegionBand* pPreviousBand = NULL;
    ImplRegionBand* pBand = ImplGetFirstRegionBand();
    long nCurrentTop (nTop);

    while (pBand != NULL && nCurrentTop<nBottom)
    {
        if (nCurrentTop < pBand->mnYTop)
        {
            
            ImplRegionBand* pAboveBand = new ImplRegionBand(
                nCurrentTop,
                ::std::min(nBottom,pBand->mnYTop-1));
            InsertBand(pPreviousBand, pAboveBand);
        }

        
        nCurrentTop = ::std::max(nTop, pBand->mnYBottom+1);

        
        pPreviousBand = pBand;
        pBand = pBand->mpNextBand;
    }

    
    
    
    if (nCurrentTop <= nBottom
        && (pBand==NULL || nBottom>pBand->mnYBottom))
    {
        
        
        InsertBand(
            pPreviousBand,
            new ImplRegionBand(
                nCurrentTop,
                nBottom));
    }

}

void RegionBand::CreateBandRange(long nYTop, long nYBottom)
{
    
    mpFirstBand = new ImplRegionBand( nYTop-1, nYTop-1 );

    
    mpLastCheckedBand = mpFirstBand;
    ImplRegionBand* pBand = mpFirstBand;

    for ( int i = nYTop; i <= nYBottom+1; i++ )
    {
        
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

    
    if ( rStartPt == rEndPt )
    {
        return true;
    }

    LineType eLineType = (rStartPt.Y() > rEndPt.Y()) ? LINE_DESCENDING : LINE_ASCENDING;
    if ( rStartPt.X() == rEndPt.X() )
    {
        
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
            const long  nDYX = ( nDY - nDX ) << 1;
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
            const long  nDYX = ( nDX - nDY ) << 1;
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
        
        while ( mpLastCheckedBand )
        {
            
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
        
        while ( mpLastCheckedBand )
        {
            
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

        
        if ( pBand->IsEmpty() || (bBTEqual && (pBand->mnYBottom == pBand->mnYTop)) )
        {
            
            ImplRegionBand* pOldBand = pBand;

            
            if ( pBand == mpFirstBand )
                mpFirstBand = pBand->mpNextBand;
            else
                pPrevBand->mpNextBand = pBand->mpNextBand;

            pBand = pBand->mpNextBand;
            delete pOldBand;
        }
        else
        {
            
            if ( bBTEqual )
                pBand->mnYBottom = pBand->mpNextBand->mnYTop-1;

            
            if ( pBand->mpNextBand &&
                 ((pBand->mnYBottom+1) == pBand->mpNextBand->mnYTop) &&
                 (*pBand == *pBand->mpNextBand) )
            {
                
                pBand->mnYBottom = pBand->mpNextBand->mnYBottom;

                
                ImplRegionBand* pDeletedBand = pBand->mpNextBand;
                pBand->mpNextBand = pDeletedBand->mpNextBand;
                delete pDeletedBand;

                
            }
            else
            {
                
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
        
        if(nVertMove)
        {
            pBand->mnYTop = pBand->mnYTop + nVertMove;
            pBand->mnYBottom = pBand->mnYBottom + nVertMove;
        }

        
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
        
        if(0.0 != fScaleY)
        {
            pBand->mnYTop = basegfx::fround(pBand->mnYTop * fScaleY);
            pBand->mnYBottom = basegfx::fround(pBand->mnYBottom * fScaleY);
        }

        
        if(0.0 != fScaleX)
        {
            pBand->ScaleX(fScaleX);
        }

        pBand = pBand->mpNextBand;
    }

}

void RegionBand::InsertBands(long nTop, long nBottom)
{
    
    if ( !mpFirstBand )
    {
        
        mpFirstBand = new ImplRegionBand( nTop, nBottom );
        return;
    }

    
    bool bTopBoundaryInserted = false;
    bool bTop2BoundaryInserted = false;
    bool bBottomBoundaryInserted = false;

    
    ImplRegionBand* pNewBand;

    if ( nTop < mpFirstBand->mnYTop )
    {
        
        pNewBand = new ImplRegionBand( nTop, mpFirstBand->mnYTop );

        if ( nBottom < mpFirstBand->mnYTop )
        {
            pNewBand->mnYBottom = nBottom;
        }

        
        pNewBand->mpNextBand = mpFirstBand;
        mpFirstBand = pNewBand;

        bTopBoundaryInserted = true;
    }

    
    ImplRegionBand* pBand = mpFirstBand;

    while ( pBand )
    {
        
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

        
        if ( bTopBoundaryInserted && bTop2BoundaryInserted && bBottomBoundaryInserted )
        {
            break;
        }

        
        if ( pBand->mpNextBand )
        {
            if ( (pBand->mnYBottom + 1) < pBand->mpNextBand->mnYTop )
            {
                
                pNewBand = new ImplRegionBand( pBand->mnYBottom+1, pBand->mpNextBand->mnYTop-1 );

                
                pNewBand->mpNextBand = pBand->mpNextBand;
                pBand->mpNextBand = pNewBand;
            }
        }

        pBand = pBand->mpNextBand;
    }

}

bool RegionBand::InsertSingleBand(ImplRegionBand* pBand, long nYBandPosition)
{
    
    if ( (pBand->mnYTop == pBand->mnYBottom) && (nYBandPosition == pBand->mnYTop) )
    {
        return true;
    }

    
    ImplRegionBand* pNewBand;

    if ( nYBandPosition == pBand->mnYTop )
    {
        
        pNewBand = new ImplRegionBand( *pBand );
        pNewBand->mnYTop = nYBandPosition+1;

        
        pNewBand->mpNextBand = pBand->mpNextBand;
        pBand->mnYBottom = nYBandPosition;
        pBand->mpNextBand = pNewBand;

        return true;
    }

    
    if ( (nYBandPosition > pBand->mnYTop) && (nYBandPosition < pBand->mnYBottom) )
    {
        
        pNewBand = new ImplRegionBand( *pBand );
        pNewBand->mnYTop = nYBandPosition;

        
        pNewBand->mpNextBand = pBand->mpNextBand;
        pBand->mnYBottom = nYBandPosition;
        pBand->mpNextBand = pNewBand;

        
        pNewBand = new ImplRegionBand( *pBand );
        pNewBand->mnYTop = nYBandPosition;

        
        pBand->mpNextBand->mnYTop = nYBandPosition+1;

        pNewBand->mpNextBand = pBand->mpNextBand;
        pBand->mnYBottom = nYBandPosition - 1;
        pBand->mpNextBand = pNewBand;

        return true;
    }

    
    if ( !pBand->mpNextBand )
    {
        if ( nYBandPosition == pBand->mnYBottom )
        {
            
            pNewBand = new ImplRegionBand( *pBand );
            pNewBand->mnYTop = pBand->mnYBottom;
            pNewBand->mnYBottom = nYBandPosition;

            pBand->mnYBottom = nYBandPosition-1;

            
            pBand->mpNextBand = pNewBand;
            return true;
        }

        if ( nYBandPosition > pBand->mnYBottom )
        {
            
            pNewBand = new ImplRegionBand( pBand->mnYBottom + 1, nYBandPosition );

            
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
    
    ImplRegionBand* pPrevBand = 0;
    ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        
        if((pBand->mnYTop >= nTop) && (pBand->mnYBottom <= nBottom))
        {
            
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
    
    ImplRegionBand* pBand = rSource.mpFirstBand;

    while ( pBand )
    {
        
        InsertBands(pBand->mnYTop, pBand->mnYBottom);

        
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
    
    ImplRegionBand* pBand = mpFirstBand;

    while ( pBand )
    {
        pBand->mbTouched = false;
        pBand = pBand->mpNextBand;
    }

    pBand = rSource.mpFirstBand;

    while ( pBand )
    {
        
        InsertBands( pBand->mnYTop, pBand->mnYBottom );

        
        ImplRegionBandSep* pSep = pBand->mpFirstSep;

        while ( pSep )
        {
            
            if ( pSep == pBand->mpFirstSep )
            {
                
                Exclude( LONG_MIN+1, pBand->mnYTop, pSep->mnXLeft-1, pBand->mnYBottom );
            }

            
            if ( pSep->mpNextSep == NULL )
            {
                
                Exclude( pSep->mnXRight+1, pBand->mnYTop, LONG_MAX-1, pBand->mnYBottom );
            }
            else
            {
                
                Exclude( pSep->mnXRight+1, pBand->mnYTop, pSep->mpNextSep->mnXLeft-1, pBand->mnYBottom );
            }

            pSep = pSep->mpNextSep;
        }

        pBand = pBand->mpNextBand;
    }

    
    ImplRegionBand* pPrevBand = 0;
    pBand = mpFirstBand;

    while ( pBand )
    {
        if ( !pBand->mbTouched )
        {
            
            ImplRegionBand* pOldBand = pBand;

            
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
    
    ImplRegionBand* pBand = rSource.mpFirstBand;

    while ( pBand )
    {
        
        InsertBands( pBand->mnYTop, pBand->mnYBottom );

        
        ImplRegionBandSep* pSep = pBand->mpFirstSep;

        while ( pSep )
        {
            Exclude( pSep->mnXLeft, pBand->mnYTop, pSep->mnXRight, pBand->mnYBottom );
            pSep = pSep->mpNextSep;
        }

        
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

    
    long nYTop(mpFirstBand->mnYTop);
    long nYBottom(mpFirstBand->mnYBottom);
    long nXLeft(mpFirstBand->GetXLeftBoundary());
    long nXRight(mpFirstBand->GetXRightBoundary());

    
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
        
        InsertBands( pBand->mnYTop, pBand->mnYBottom );

        
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

    
    ImplRegionBand* pBand = mpFirstBand;

    while(pBand)
    {
        
        if((pBand->mnYTop <= rPoint.Y()) && (pBand->mnYBottom >= rPoint.Y()))
        {
            
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

    return 0;
}

#ifdef DBG_UTIL
const char* ImplDbgTestRegionBand(const void* pObj)
{
    const RegionBand* pRegionBand = reinterpret_cast< const RegionBand* >(pObj);

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
