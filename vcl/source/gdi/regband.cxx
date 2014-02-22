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

#include <algorithm>

#include <tools/debug.hxx>
#include <tools/helpers.hxx>
#include <osl/diagnose.hxx>

#include "regband.hxx"


//

//




//

//




//
//


ImplRegionBand::ImplRegionBand( long nTop, long nBottom )
{
    
    mnYTop              = nTop;
    mnYBottom           = nBottom;

    
    mpNextBand          = NULL;
    mpPrevBand          = NULL;
    mpFirstSep          = NULL;
    mpFirstBandPoint    = NULL;
    mbTouched           = false;
}



ImplRegionBand::ImplRegionBand(
    const ImplRegionBand& rRegionBand,
    const bool bIgnorePoints)
{
    
    mnYTop              = rRegionBand.mnYTop;
    mnYBottom           = rRegionBand.mnYBottom;
    mbTouched           = rRegionBand.mbTouched;

    
    mpNextBand          = NULL;
    mpPrevBand          = NULL;
    mpFirstSep          = NULL;
    mpFirstBandPoint    = NULL;

    
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep = 0;
    ImplRegionBandSep* pSep = rRegionBand.mpFirstSep;
    while ( pSep )
    {
        
        pNewSep             = new ImplRegionBandSep;
        pNewSep->mnXLeft    = pSep->mnXLeft;
        pNewSep->mnXRight   = pSep->mnXRight;
        pNewSep->mbRemoved  = pSep->mbRemoved;
        pNewSep->mpNextSep  = NULL;
        if ( pSep == rRegionBand.mpFirstSep )
            mpFirstSep = pNewSep;
        else
            pPrevSep->mpNextSep = pNewSep;

        pPrevSep = pNewSep;
        pSep = pSep->mpNextSep;
    }

    if ( ! bIgnorePoints)
    {
        
        ImplRegionBandPoint* pPoint = rRegionBand.mpFirstBandPoint;
        ImplRegionBandPoint* pPrevPointCopy = NULL;
        while (pPoint != NULL)
        {
            ImplRegionBandPoint* pPointCopy = new ImplRegionBandPoint();
            pPointCopy->mnX = pPoint->mnX;
            pPointCopy->mnLineId = pPoint->mnLineId;
            pPointCopy->mbEndPoint = pPoint->mbEndPoint;
            pPointCopy->meLineType = pPoint->meLineType;

            if (pPrevPointCopy != NULL)
                pPrevPointCopy->mpNextBandPoint = pPointCopy;
            else
                mpFirstBandPoint = pPointCopy;

            pPrevPointCopy = pPointCopy;
            pPoint = pPoint->mpNextBandPoint;
        }
    }
}



ImplRegionBand::~ImplRegionBand()
{
    DBG_ASSERT( mpFirstBandPoint == NULL, "ImplRegionBand::~ImplRegionBand -> pointlist not empty" );

    
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        ImplRegionBandSep* pTempSep = pSep->mpNextSep;
        delete pSep;
        pSep = pTempSep;
    }

    
    ImplRegionBandPoint* pPoint = mpFirstBandPoint;
    while ( pPoint )
    {
        ImplRegionBandPoint* pTempPoint = pPoint->mpNextBandPoint;
        delete pPoint;
        pPoint = pTempPoint;
    }
}


//



void ImplRegionBand::ProcessPoints()
{
    
    ImplRegionBandPoint* pRegionBandPoint = mpFirstBandPoint;
    while ( pRegionBandPoint )
    {
        
        if ( pRegionBandPoint->mpNextBandPoint )
        {
            
            if ( pRegionBandPoint->mbEndPoint && pRegionBandPoint->mpNextBandPoint->mbEndPoint )
            {
                
                if ( pRegionBandPoint->meLineType == pRegionBandPoint->mpNextBandPoint->meLineType )
                {
                    ImplRegionBandPoint* pSaveRegionBandPoint = pRegionBandPoint->mpNextBandPoint;
                    pRegionBandPoint->mpNextBandPoint = pRegionBandPoint->mpNextBandPoint->mpNextBandPoint;
                    delete pSaveRegionBandPoint;
                }
            }
        }

        
        pRegionBandPoint = pRegionBandPoint->mpNextBandPoint;
    }

    pRegionBandPoint = mpFirstBandPoint;
    while ( pRegionBandPoint && pRegionBandPoint->mpNextBandPoint )
    {
        Union( pRegionBandPoint->mnX, pRegionBandPoint->mpNextBandPoint->mnX );

        ImplRegionBandPoint* pNextBandPoint = pRegionBandPoint->mpNextBandPoint->mpNextBandPoint;

        
        delete pRegionBandPoint->mpNextBandPoint;
        delete pRegionBandPoint;

        
        pRegionBandPoint = pNextBandPoint;
    }

    
    delete pRegionBandPoint;

    
    mpFirstBandPoint = NULL;
}


//



bool ImplRegionBand::InsertPoint( long nX, long nLineId,
                                  bool bEndPoint, LineType eLineType )
{
    if ( !mpFirstBandPoint )
    {
        mpFirstBandPoint                  = new ImplRegionBandPoint;
        mpFirstBandPoint->mnX             = nX;
        mpFirstBandPoint->mnLineId        = nLineId;
        mpFirstBandPoint->mbEndPoint      = bEndPoint;
        mpFirstBandPoint->meLineType      = eLineType;
        mpFirstBandPoint->mpNextBandPoint = NULL;
        return true;
    }

    
    ImplRegionBandPoint* pRegionBandPoint = mpFirstBandPoint;
    ImplRegionBandPoint* pLastTestedRegionBandPoint = NULL;
    while( pRegionBandPoint )
    {
        if ( pRegionBandPoint->mnLineId == nLineId )
        {
            if ( bEndPoint )
            {
                if( !pRegionBandPoint->mbEndPoint )
                {
                    
                    if( !mpFirstBandPoint->mpNextBandPoint )
                    {
                        
                        pRegionBandPoint->mnX = nX;
                        pRegionBandPoint->mbEndPoint = true;
                        return true;
                    }
                    else
                    {
                        
                        if( !pLastTestedRegionBandPoint )
                        {
                            
                            ImplRegionBandPoint* pSaveBandPoint = mpFirstBandPoint;
                            mpFirstBandPoint = mpFirstBandPoint->mpNextBandPoint;
                            delete pSaveBandPoint;
                        }
                        else
                        {
                            
                            pLastTestedRegionBandPoint->mpNextBandPoint = pRegionBandPoint->mpNextBandPoint;
                            delete pRegionBandPoint;
                        }

                        break;
                    }
                }
            }
            else
                return false;
        }

        
        pLastTestedRegionBandPoint = pRegionBandPoint;
        pRegionBandPoint = pRegionBandPoint->mpNextBandPoint;
    }

    
    ImplRegionBandPoint* pNewRegionBandPoint;

    pRegionBandPoint = mpFirstBandPoint;
    pLastTestedRegionBandPoint = NULL;
    while ( pRegionBandPoint )
    {
        
        if ( nX <= pRegionBandPoint->mnX )
        {
            pNewRegionBandPoint                     = new ImplRegionBandPoint;
            pNewRegionBandPoint->mnX                = nX;
            pNewRegionBandPoint->mnLineId           = nLineId;
            pNewRegionBandPoint->mbEndPoint         = bEndPoint;
            pNewRegionBandPoint->meLineType         = eLineType;
            pNewRegionBandPoint->mpNextBandPoint    = pRegionBandPoint;

            
            if ( !pLastTestedRegionBandPoint )
                mpFirstBandPoint = pNewRegionBandPoint;
            else
                pLastTestedRegionBandPoint->mpNextBandPoint = pNewRegionBandPoint;

            return true;
        }

        
        pLastTestedRegionBandPoint = pRegionBandPoint;
        pRegionBandPoint = pRegionBandPoint->mpNextBandPoint;
    }

    
    pNewRegionBandPoint                     = new ImplRegionBandPoint;
    pNewRegionBandPoint->mnX                = nX;
    pNewRegionBandPoint->mnLineId           = nLineId;
    pNewRegionBandPoint->mbEndPoint         = bEndPoint;
    pNewRegionBandPoint->meLineType         = eLineType;
    pNewRegionBandPoint->mpNextBandPoint    = NULL;

    
    pLastTestedRegionBandPoint->mpNextBandPoint = pNewRegionBandPoint;

    return true;
}



void ImplRegionBand::MoveX( long nHorzMove )
{
    
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        pSep->mnXLeft  += nHorzMove;
        pSep->mnXRight += nHorzMove;
        pSep = pSep->mpNextSep;
    }
}



void ImplRegionBand::ScaleX( double fHorzScale )
{
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        pSep->mnXLeft   = FRound( pSep->mnXLeft * fHorzScale );
        pSep->mnXRight  = FRound( pSep->mnXRight * fHorzScale );
        pSep = pSep->mpNextSep;
    }
}


//


bool ImplRegionBand::OptimizeBand()
{
    ImplRegionBandSep* pPrevSep = 0;
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        
        if ( pSep->mbRemoved || (pSep->mnXRight < pSep->mnXLeft) )
        {
            ImplRegionBandSep* pOldSep = pSep;
            if ( pSep == mpFirstSep )
                mpFirstSep = pSep->mpNextSep;
            else
                pPrevSep->mpNextSep = pSep->mpNextSep;
            pSep = pSep->mpNextSep;
            delete pOldSep;
            continue;
        }

        
        if ( pSep->mpNextSep )
        {
            if ( (pSep->mnXRight+1) >= pSep->mpNextSep->mnXLeft )
            {
                if ( pSep->mpNextSep->mnXRight > pSep->mnXRight )
                    pSep->mnXRight = pSep->mpNextSep->mnXRight;

                ImplRegionBandSep* pOldSep = pSep->mpNextSep;
                pSep->mpNextSep = pOldSep->mpNextSep;
                delete pOldSep;
                continue;
            }
        }

        pPrevSep = pSep;
        pSep = pSep->mpNextSep;
    }

    return true;
}



void ImplRegionBand::Union( long nXLeft, long nXRight )
{
    DBG_ASSERT( nXLeft <= nXRight, "ImplRegionBand::Union(): nxLeft > nXRight" );

    
    if ( !mpFirstSep )
    {
        mpFirstSep              = new ImplRegionBandSep;
        mpFirstSep->mnXLeft     = nXLeft;
        mpFirstSep->mnXRight    = nXRight;
        mpFirstSep->mbRemoved   = false;
        mpFirstSep->mpNextSep   = NULL;
        return;
    }

    
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep = 0;
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        
        if ( (nXLeft >= pSep->mnXLeft) && (nXRight <= pSep->mnXRight) )
            return;

        
        if ( nXRight < pSep->mnXLeft )
        {
            pNewSep             = new ImplRegionBandSep;
            pNewSep->mnXLeft    = nXLeft;
            pNewSep->mnXRight   = nXRight;
            pNewSep->mbRemoved  = false;

            pNewSep->mpNextSep = pSep;
            if ( pSep == mpFirstSep )
                mpFirstSep = pNewSep;
            else
                pPrevSep->mpNextSep = pNewSep;
            break;
        }

        
        if ( (nXRight >= pSep->mnXLeft) && (nXLeft <= pSep->mnXLeft) )
            pSep->mnXLeft = nXLeft;

        
        if ( (nXLeft <= pSep->mnXRight) && (nXRight > pSep->mnXRight) )
        {
            pSep->mnXRight = nXRight;
            break;
        }

        
        if ( !pSep->mpNextSep && (nXLeft > pSep->mnXRight) )
        {
            pNewSep             = new ImplRegionBandSep;
            pNewSep->mnXLeft    = nXLeft;
            pNewSep->mnXRight   = nXRight;
            pNewSep->mbRemoved  = false;

            pSep->mpNextSep     = pNewSep;
            pNewSep->mpNextSep  = NULL;
            break;
        }

        pPrevSep = pSep;
        pSep = pSep->mpNextSep;
    }

    OptimizeBand();
}



void ImplRegionBand::Intersect( long nXLeft, long nXRight )
{
    DBG_ASSERT( nXLeft <= nXRight, "ImplRegionBand::Intersect(): nxLeft > nXRight" );

    
    mbTouched = true;

    
    if ( !mpFirstSep )
        return;

    
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        
        if ( (nXRight < pSep->mnXLeft) || (nXLeft > pSep->mnXRight) )
            
            pSep->mbRemoved = true;

        
        if ( (nXLeft <= pSep->mnXLeft) &&
             (nXRight <= pSep->mnXRight) &&
             (nXRight >= pSep->mnXLeft) )
            pSep->mnXRight = nXRight;

        
        if ( (nXLeft >= pSep->mnXLeft) &&
             (nXLeft <= pSep->mnXRight) &&
             (nXRight >= pSep->mnXRight) )
            pSep->mnXLeft = nXLeft;

        
        if ( (nXLeft >= pSep->mnXLeft) && (nXRight <= pSep->mnXRight) )
        {
            pSep->mnXRight = nXRight;
            pSep->mnXLeft = nXLeft;
        }

        pSep = pSep->mpNextSep;
    }

    OptimizeBand();
}



void ImplRegionBand::Exclude( long nXLeft, long nXRight )
{
    DBG_ASSERT( nXLeft <= nXRight, "ImplRegionBand::Exclude(): nxLeft > nXRight" );

    
    mbTouched = true;

    
    if ( !mpFirstSep )
        return;

    
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep = 0;
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep  )
    {
        bool bSepProcessed = false;

        
        if ( (nXLeft <= pSep->mnXLeft) && (nXRight >= pSep->mnXRight) )
        {
            
            pSep->mbRemoved = true;
            bSepProcessed = true;
        }

        
        if ( !bSepProcessed )
        {
            if ( (nXRight >= pSep->mnXLeft) && (nXLeft <= pSep->mnXLeft) )
            {
                pSep->mnXLeft = nXRight+1;
                bSepProcessed = true;
            }
        }

        
        if ( !bSepProcessed )
        {
            if ( (nXLeft <= pSep->mnXRight) && (nXRight > pSep->mnXRight) )
            {
                pSep->mnXRight = nXLeft-1;
                bSepProcessed = true;
            }
        }

        
        
        if ( !bSepProcessed )
        {
            if ( (nXLeft >= pSep->mnXLeft) && (nXRight <= pSep->mnXRight) )
            {
                pNewSep             = new ImplRegionBandSep;
                pNewSep->mnXLeft    = pSep->mnXLeft;
                pNewSep->mnXRight   = nXLeft-1;
                pNewSep->mbRemoved  = false;

                pSep->mnXLeft = nXRight+1;

                
                pNewSep->mpNextSep = pSep;

                
                if ( pSep == mpFirstSep )
                    mpFirstSep = pNewSep;
                else
                    pPrevSep->mpNextSep = pNewSep;
            }
        }

        pPrevSep = pSep;
        pSep = pSep->mpNextSep;
    }

    OptimizeBand();
}



void ImplRegionBand::XOr( long nXLeft, long nXRight )
{
    DBG_ASSERT( nXLeft <= nXRight, "ImplRegionBand::XOr(): nxLeft > nXRight" );

    
    //
    
    
    
    
    
    
    //
    
    //
    
    //
    
    //
    
    //
    
    //
    
    //
    
    //
    
    //
    
    //
    
    //
    
    //
    
    //
    //
    
    
    
    
    
    
    //
    
    
    
    //

    
    if ( !mpFirstSep )
    {
        mpFirstSep              = new ImplRegionBandSep;
        mpFirstSep->mnXLeft     = nXLeft;
        mpFirstSep->mnXRight    = nXRight;
        mpFirstSep->mbRemoved   = false;
        mpFirstSep->mpNextSep   = NULL;
        return;
    }

    
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep = 0;
    ImplRegionBandSep* pSep = mpFirstSep;

    while ( pSep  )
    {
        long nOldLeft( pSep->mnXLeft );
        long nOldRight( pSep->mnXRight );

        
        
        
        if( nXLeft <= nOldRight )
        {
            if( nXRight < nOldLeft )
            {
                

                
                pNewSep             = new ImplRegionBandSep;
                pNewSep->mnXLeft    = nXLeft;
                pNewSep->mnXRight   = nXRight;
                pNewSep->mpNextSep  = pSep;
                pNewSep->mbRemoved  = false;

                
                pNewSep->mpNextSep = pSep;

                
                if ( pSep == mpFirstSep )
                    mpFirstSep = pNewSep;
                else
                    pPrevSep->mpNextSep = pNewSep;
                pPrevSep = NULL; 
                break;
            }
            else if( nXLeft == nOldLeft && nXRight == nOldRight )
            {
                
                pSep->mbRemoved = true;
                pPrevSep = NULL; 
                break;
            }
            else if( nXLeft != nOldLeft && nXRight == nOldRight )
            {
                
                if( nXLeft < nOldLeft )
                {
                    nXRight = nOldLeft; 
                }
                else
                {
                    nXRight = nXLeft; 
                    nXLeft = nOldLeft;
                }

                pSep->mnXLeft = nXLeft;
                pSep->mnXRight = nXRight-1;

                pPrevSep = NULL; 
                break;
            }
            else if( nXLeft == nOldLeft && nXRight != nOldRight )
            {
                

                if( nXRight > nOldRight )
                {
                    nXLeft = nOldRight+1; 

                    
                    
                    pSep->mbRemoved = true;
                }
                else
                {
                    pSep->mnXLeft = nXRight+1; 

                    pPrevSep = NULL; 
                    break;
                }
            }
            else 
            {
                
                DBG_ASSERT( nXLeft != nOldLeft && nXRight != nOldRight,
                            "ImplRegionBand::XOr(): Case 4,5,6,7 expected all coordinates to be not equal!" );

                
                //
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                //
                
                
                
                

                if( nXLeft < nOldLeft )
                    ::std::swap( nOldLeft, nXLeft );

                bool bDone( false );

                if( nXRight < nOldRight )
                {
                    ::std::swap( nOldRight, nXRight );
                    bDone = true;
                }

                
                
                
                
                DBG_ASSERT( nOldLeft<nXLeft && nXLeft<=nOldRight && nOldRight<nXRight,
                            "ImplRegionBand::XOr(): Case 4,5,6,7 expected coordinates to be ordered now!" );

                pSep->mnXLeft = nOldLeft;
                pSep->mnXRight = nXLeft-1;

                nXLeft = nOldRight+1;
                

                if( bDone )
                {
                    
                    pNewSep = new ImplRegionBandSep;

                    pNewSep->mnXLeft    = nXLeft;
                    pNewSep->mnXRight   = nXRight;
                    pNewSep->mpNextSep  = pSep->mpNextSep;
                    pNewSep->mbRemoved  = false;

                    
                    pSep->mpNextSep = pNewSep;

                    pPrevSep = NULL; 
                    break;
                }
            }
        }

        pPrevSep = pSep;
        pSep = pSep->mpNextSep;
    }

    
    if( pPrevSep && nXLeft >= pPrevSep->mnXRight )
    {
        pNewSep             = new ImplRegionBandSep;
        pNewSep->mnXLeft    = nXLeft;
        pNewSep->mnXRight   = nXRight;
        pNewSep->mpNextSep  = NULL;
        pNewSep->mbRemoved  = false;

        
        pPrevSep->mpNextSep = pNewSep;
    }

    OptimizeBand();
}



bool ImplRegionBand::IsInside( long nX )
{
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        if ( (pSep->mnXLeft <= nX) && (pSep->mnXRight >= nX) )
            return true;

        pSep = pSep->mpNextSep;
    }

    return false;
}



bool ImplRegionBand::IsOver( long nLeft, long nRight )
{
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        if ( (pSep->mnXLeft < nRight) && (pSep->mnXRight > nLeft) )
            return true;

        pSep = pSep->mpNextSep;
    }

    return false;
}



bool ImplRegionBand::IsInside( long nLeft, long nRight )
{
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        if ( (pSep->mnXLeft >= nLeft) && (nRight <= pSep->mnXRight) )
            return true;

        pSep = pSep->mpNextSep;
    }

    return false;
}



long ImplRegionBand::GetXLeftBoundary() const
{
    DBG_ASSERT( mpFirstSep != NULL, "ImplRegionBand::XLeftBoundary -> no separation in band!" );

    return mpFirstSep->mnXLeft;
}



long ImplRegionBand::GetXRightBoundary() const
{
    DBG_ASSERT( mpFirstSep != NULL, "ImplRegionBand::XRightBoundary -> no separation in band!" );

    
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep->mpNextSep )
        pSep = pSep->mpNextSep;
    return pSep->mnXRight;
}



bool ImplRegionBand::operator==( const ImplRegionBand& rRegionBand ) const
{
    ImplRegionBandSep*   pOwnRectBandSep = mpFirstSep;
    ImplRegionBandSep*   pSecondRectBandSep = rRegionBand.mpFirstSep;
    while ( pOwnRectBandSep && pSecondRectBandSep )
    {
        
        long nOwnXLeft = pOwnRectBandSep->mnXLeft;
        long nSecondXLeft = pSecondRectBandSep->mnXLeft;
        if ( nOwnXLeft != nSecondXLeft )
            return false;

        long nOwnXRight = pOwnRectBandSep->mnXRight;
        long nSecondXRight = pSecondRectBandSep->mnXRight;
        if ( nOwnXRight != nSecondXRight )
            return false;

        
        pOwnRectBandSep = pOwnRectBandSep->mpNextSep;

        
        pSecondRectBandSep = pSecondRectBandSep->mpNextSep;
    }

    
    if ( pOwnRectBandSep || pSecondRectBandSep )
        return false;

    return true;
}



ImplRegionBand* ImplRegionBand::SplitBand (const sal_Int32 nY)
{
    OSL_ASSERT(nY>mnYTop);
    OSL_ASSERT(nY<=mnYBottom);

    
    
    ImplRegionBand* pLowerBand = new ImplRegionBand(*this, false);

    
    mnYBottom = nY-1;
    pLowerBand->mnYTop = nY;

    
    pLowerBand->mpNextBand = mpNextBand;
    mpNextBand = pLowerBand;
    pLowerBand->mpPrevBand = this;
    if (pLowerBand->mpNextBand != NULL)
        pLowerBand->mpNextBand->mpPrevBand = pLowerBand;

    return pLowerBand;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
