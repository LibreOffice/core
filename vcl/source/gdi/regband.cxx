/*************************************************************************
 *
 *  $RCSfile: regband.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_REGBAND_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_REGBAND_HXX
#include <regband.hxx>
#endif

// =======================================================================
//
// ImplRegionBand
//
// Jedes Band enthaelt die zwischen der enthaltenen Ober- und Untergrenze
// enthaltenen Rechtecke. Bei den Operationen Union, Intersect, XOr und
// Exclude werden immer Rechtecke der gleichen Hoehe ausgewerte; die
// Grenzen der Baender sind immer so zu waehlen, dasz dies moeglich ist.
//
// Die Rechtecke in den Baendern werden nach Moeglichkeit zusammengefaszt.
//
// Bei der Umwandlung von Polygonen werden alle Punkte des Polygons
// in die einzelnen Baender eingetragen (sie stehen fuer jedes Band als
// Punkte in einer Liste). Nach dem Eintragen der Punkte werden diese
// in Rechtecke umgewandelt und die Liste der Punkte geloescht.
//
// -----------------------------------------------------------------------

ImplRegionBand::ImplRegionBand( long nTop, long nBottom )
{
    // save boundaries
    mnYTop              = nTop;
    mnYBottom           = nBottom;

    // initialize lists
    mpNextBand          = NULL;
    mpPrevBand          = NULL;
    mpFirstSep          = NULL;
    mpFirstBandPoint    = NULL;
    mbTouched           = FALSE;
}

// -----------------------------------------------------------------------

ImplRegionBand::ImplRegionBand( const ImplRegionBand& rRegionBand )
{
    // copy boundaries
    mnYTop              = rRegionBand.mnYTop;
    mnYBottom           = rRegionBand.mnYBottom;
    mbTouched           = rRegionBand.mbTouched;

    // initialisation
    mpNextBand          = NULL;
    mpPrevBand          = NULL;
    mpFirstSep          = NULL;
    mpFirstBandPoint    = NULL;

    // copy all elements of the list with separations
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep;
    ImplRegionBandSep* pSep = rRegionBand.mpFirstSep;
    while ( pSep )
    {
        // create new and copy data
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
}

// -----------------------------------------------------------------------

ImplRegionBand::~ImplRegionBand()
{
    DBG_ASSERT( mpFirstBandPoint == NULL, "ImplRegionBand::~ImplRegionBand -> pointlist not empty" );

    // delete elements of the list
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        ImplRegionBandSep* pTempSep = pSep->mpNextSep;
        delete pSep;
        pSep = pTempSep;
    }

    // delete elements of the list
    ImplRegionBandPoint* pPoint = mpFirstBandPoint;
    while ( pPoint )
    {
        ImplRegionBandPoint* pTempPoint = pPoint->mpNextBandPoint;
        delete pPoint;
        pPoint = pTempPoint;
    }
}

// -----------------------------------------------------------------------
//
// generate separations from lines and process union with existing
// separations

void ImplRegionBand::ProcessPoints()
{
    // check Pointlist
    ImplRegionBandPoint* pRegionBandPoint = mpFirstBandPoint;
    while ( pRegionBandPoint )
    {
        // within list?
        if ( pRegionBandPoint && pRegionBandPoint->mpNextBandPoint )
        {
            // start/stop?
            if ( pRegionBandPoint->mbEndPoint && pRegionBandPoint->mpNextBandPoint->mbEndPoint )
            {
                // same direction? -> remove next point!
                if ( pRegionBandPoint->meLineType == pRegionBandPoint->mpNextBandPoint->meLineType )
                {
                    ImplRegionBandPoint* pSaveRegionBandPoint = pRegionBandPoint->mpNextBandPoint;
                    pRegionBandPoint->mpNextBandPoint = pRegionBandPoint->mpNextBandPoint->mpNextBandPoint;
                    delete pSaveRegionBandPoint;
                }
            }
        }

        // continue with next element in the list
        pRegionBandPoint = pRegionBandPoint->mpNextBandPoint;
    }

    pRegionBandPoint = mpFirstBandPoint;
    while ( pRegionBandPoint && pRegionBandPoint->mpNextBandPoint )
    {
        Union( pRegionBandPoint->mnX, pRegionBandPoint->mpNextBandPoint->mnX );

        ImplRegionBandPoint* pNextBandPoint = pRegionBandPoint->mpNextBandPoint->mpNextBandPoint;

        // remove allready processed points
        delete pRegionBandPoint->mpNextBandPoint;
        delete pRegionBandPoint;

        // continue with next element in the list
        pRegionBandPoint = pNextBandPoint;
    }

    // remove last element if necessary
    if ( pRegionBandPoint )
        delete pRegionBandPoint;

    // list is now empty
    mpFirstBandPoint = NULL;
}

// -----------------------------------------------------------------------
//
// generate separations from lines and process union with existing
// separations

BOOL ImplRegionBand::InsertPoint( long nX, long nLineId,
                                  BOOL bEndPoint, LineType eLineType )
{
    if ( !mpFirstBandPoint )
    {
        mpFirstBandPoint                  = new ImplRegionBandPoint;
        mpFirstBandPoint->mnX             = nX;
        mpFirstBandPoint->mnLineId        = nLineId;
        mpFirstBandPoint->mbEndPoint      = bEndPoint;
        mpFirstBandPoint->meLineType      = eLineType;
        mpFirstBandPoint->mpNextBandPoint = NULL;
        return TRUE;
    }

    // look if line allready touched the band
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
                    // remove old band point
                    if( !mpFirstBandPoint->mpNextBandPoint )
                    {
                        // if we've only got one point => replace first point
                        pRegionBandPoint->mnX = nX;
                        pRegionBandPoint->mbEndPoint = TRUE;
                        return TRUE;
                    }
                    else
                    {
                        // remove current point
                        if( !pLastTestedRegionBandPoint )
                        {
                            // remove and delete old first point
                            ImplRegionBandPoint* pSaveBandPoint = mpFirstBandPoint;
                            mpFirstBandPoint = mpFirstBandPoint->mpNextBandPoint;
                            delete pSaveBandPoint;
                        }
                        else
                        {
                            // remove and delete current band point
                            pLastTestedRegionBandPoint->mpNextBandPoint = pRegionBandPoint->mpNextBandPoint;
                            delete pRegionBandPoint;
                        }

                        break;
                    }
                }
            }
            else
                return FALSE;
        }

        // use next element
        pLastTestedRegionBandPoint = pRegionBandPoint;
        pRegionBandPoint = pRegionBandPoint->mpNextBandPoint;
    }

    // search appropriate position and insert point into the list
    ImplRegionBandPoint* pNewRegionBandPoint;

    pRegionBandPoint = mpFirstBandPoint;
    pLastTestedRegionBandPoint = NULL;
    while ( pRegionBandPoint )
    {
        // new point completly left? -> insert as first point
        if ( nX <= pRegionBandPoint->mnX )
        {
            pNewRegionBandPoint                     = new ImplRegionBandPoint;
            pNewRegionBandPoint->mnX                = nX;
            pNewRegionBandPoint->mnLineId           = nLineId;
            pNewRegionBandPoint->mbEndPoint         = bEndPoint;
            pNewRegionBandPoint->meLineType         = eLineType;
            pNewRegionBandPoint->mpNextBandPoint    = pRegionBandPoint;

            // connections to the new point
            if ( !pLastTestedRegionBandPoint )
                mpFirstBandPoint = pNewRegionBandPoint;
            else
                pLastTestedRegionBandPoint->mpNextBandPoint = pNewRegionBandPoint;

            return TRUE;
        }

        // use next element
        pLastTestedRegionBandPoint = pRegionBandPoint;
        pRegionBandPoint = pRegionBandPoint->mpNextBandPoint;
    }

    // not inserted -> add to the end of the list
    pNewRegionBandPoint                     = new ImplRegionBandPoint;
    pNewRegionBandPoint->mnX                = nX;
    pNewRegionBandPoint->mnLineId           = nLineId;
    pNewRegionBandPoint->mbEndPoint         = bEndPoint;
    pNewRegionBandPoint->meLineType         = eLineType;
    pNewRegionBandPoint->mpNextBandPoint    = NULL;

    // connections to the new point
    pLastTestedRegionBandPoint->mpNextBandPoint = pNewRegionBandPoint;

    return TRUE;
}

// -----------------------------------------------------------------------

void ImplRegionBand::MoveX( long nHorzMove )
{
    // move all x-separations
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        pSep->mnXLeft  += nHorzMove;
        pSep->mnXRight += nHorzMove;
        pSep = pSep->mpNextSep;
    }
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------
//
// combine overlaping sparations

BOOL ImplRegionBand::OptimizeBand()
{
    ImplRegionBandSep* pPrevSep;
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        // remove?
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

        // overlaping separations? -> combine!
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

    return TRUE;
}

// -----------------------------------------------------------------------

void ImplRegionBand::Union( long nXLeft, long nXRight )
{
    DBG_ASSERT( nXLeft <= nXRight, "ImplRegionBand::Union(): nxLeft > nXRight" );

    // band empty? -> add element
    if ( !mpFirstSep )
    {
        mpFirstSep              = new ImplRegionBandSep;
        mpFirstSep->mnXLeft     = nXLeft;
        mpFirstSep->mnXRight    = nXRight;
        mpFirstSep->mbRemoved   = FALSE;
        mpFirstSep->mpNextSep   = NULL;
        return;
    }

    // process real union
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep;
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        // new separation completely inside? nothing to do!
        if ( (nXLeft >= pSep->mnXLeft) && (nXRight <= pSep->mnXRight) )
            return;

        // new separation completly left? -> new separation!
        if ( nXRight < pSep->mnXLeft )
        {
            pNewSep             = new ImplRegionBandSep;
            pNewSep->mnXLeft    = nXLeft;
            pNewSep->mnXRight   = nXRight;
            pNewSep->mbRemoved  = FALSE;

            pNewSep->mpNextSep = pSep;
            if ( pSep == mpFirstSep )
                mpFirstSep = pNewSep;
            else
                pPrevSep->mpNextSep = pNewSep;
            break;
        }

        // new separation overlaping from left? -> extend boundary
        if ( (nXRight >= pSep->mnXLeft) && (nXLeft <= pSep->mnXLeft) )
            pSep->mnXLeft = nXLeft;

        // new separation overlaping from right? -> extend boundary
        if ( (nXLeft <= pSep->mnXRight) && (nXRight > pSep->mnXRight) )
        {
            pSep->mnXRight = nXRight;
            break;
        }

        // not inserted, but last element? -> add to the end of the list
        if ( !pSep->mpNextSep && (nXLeft > pSep->mnXRight) )
        {
            pNewSep             = new ImplRegionBandSep;
            pNewSep->mnXLeft    = nXLeft;
            pNewSep->mnXRight   = nXRight;
            pNewSep->mbRemoved  = FALSE;

            pSep->mpNextSep     = pNewSep;
            pNewSep->mpNextSep  = NULL;
            break;
        }

        pPrevSep = pSep;
        pSep = pSep->mpNextSep;
    }

    OptimizeBand();
}

// -----------------------------------------------------------------------

void ImplRegionBand::Intersect( long nXLeft, long nXRight )
{
    DBG_ASSERT( nXLeft <= nXRight, "ImplRegionBand::Intersect(): nxLeft > nXRight" );

    // band has been touched
    mbTouched = TRUE;

    // band empty? -> nothing to do
    if ( !mpFirstSep )
        return;

    // process real intersection
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        // new separation completly outside? -> remove separation
        if ( (nXRight < pSep->mnXLeft) || (nXLeft > pSep->mnXRight) )
            // will be removed from the optimizer
            pSep->mbRemoved = TRUE;

        // new separation overlaping from left? -> reduce right boundary
        if ( (nXLeft <= pSep->mnXLeft) &&
             (nXRight <= pSep->mnXRight) &&
             (nXRight >= pSep->mnXLeft) )
            pSep->mnXRight = nXRight;

        // new separation overlaping from right? -> reduce right boundary
        if ( (nXLeft >= pSep->mnXLeft) &&
             (nXLeft <= pSep->mnXRight) &&
             (nXRight >= pSep->mnXRight) )
            pSep->mnXLeft = nXLeft;

        // new separation within the actual one? -> reduce both boundaries
        if ( (nXLeft >= pSep->mnXLeft) && (nXRight <= pSep->mnXRight) )
        {
            pSep->mnXRight = nXRight;
            pSep->mnXLeft = nXLeft;
        }

        pSep = pSep->mpNextSep;
    }

    OptimizeBand();
}

// -----------------------------------------------------------------------

void ImplRegionBand::Exclude( long nXLeft, long nXRight )
{
    DBG_ASSERT( nXLeft <= nXRight, "ImplRegionBand::Exclude(): nxLeft > nXRight" );

    // band has been touched
    mbTouched = TRUE;

    // band empty? -> nothing to do
    if ( !mpFirstSep )
        return;

    // process real exclusion
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep;
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep  )
    {
        BOOL bSepProcessed = FALSE;

        // new separation completely overlapping? -> remove separation
        if ( (nXLeft <= pSep->mnXLeft) && (nXRight >= pSep->mnXRight) )
        {
            // will be removed from the optimizer
            pSep->mbRemoved = TRUE;
            bSepProcessed = TRUE;
        }

        // new separation overlaping from left? -> reduce boundary
        if ( !bSepProcessed )
        {
            if ( (nXRight >= pSep->mnXLeft) && (nXLeft <= pSep->mnXLeft) )
            {
                pSep->mnXLeft = nXRight+1;
                bSepProcessed = TRUE;
            }
        }

        // new separation overlaping from right? -> reduce boundary
        if ( !bSepProcessed )
        {
            if ( (nXLeft <= pSep->mnXRight) && (nXRight > pSep->mnXRight) )
            {
                pSep->mnXRight = nXLeft-1;
                bSepProcessed = TRUE;
            }
        }

        // new separation within the actual one? -> reduce boundary
        // and add new entry for reminder
        if ( !bSepProcessed )
        {
            if ( (nXLeft >= pSep->mnXLeft) && (nXRight <= pSep->mnXRight) )
            {
                pNewSep             = new ImplRegionBandSep;
                pNewSep->mnXLeft    = pSep->mnXLeft;
                pNewSep->mnXRight   = nXLeft-1;
                pNewSep->mbRemoved  = FALSE;

                pSep->mnXLeft = nXRight+1;

                // connections from the new separation
                pNewSep->mpNextSep = pSep;

                // connections to the new separation
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

// -----------------------------------------------------------------------

void ImplRegionBand::XOr( long nXLeft, long nXRight )
{
    DBG_ASSERT( nXLeft <= nXRight, "ImplRegionBand::XOr(): nxLeft > nXRight" );

    // band empty? -> add element
    if ( !mpFirstSep )
    {
        mpFirstSep              = new ImplRegionBandSep;
        mpFirstSep->mnXLeft     = nXLeft;
        mpFirstSep->mnXRight    = nXRight;
        mpFirstSep->mbRemoved   = FALSE;
        mpFirstSep->mpNextSep   = NULL;
        return;
    }

    // process real xor
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep;
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep  )
    {
        // new separation completely overlapping?
        //   -> move boundaries to left remainder
        //   -> reduce boundaries of new separation
        if ( (nXLeft <= pSep->mnXLeft) && (nXRight >= pSep->mnXRight) )
        {
            int iOldXRight = pSep->mnXRight;
            pSep->mnXRight = pSep->mnXLeft;
            pSep->mnXLeft = nXLeft;
            nXLeft = pSep->mnXRight;
        }

        // new separation overlaping from left?
        //   -> move boundaries to left remainder
        //   -> set boundaries of new separation to right remainder
        if ( (nXRight >= pSep->mnXLeft) && (nXLeft <= pSep->mnXLeft) )
        {
            int iOldXRight = pSep->mnXRight;
            pSep->mnXRight = pSep->mnXLeft;
            pSep->mnXLeft = nXLeft;
            nXLeft = pSep->mnXRight;
        }

        // new separation overlaping from right? -> reduce boundary
        if ( (nXLeft <= pSep->mnXRight) && (nXRight > pSep->mnXRight) )
            pSep->mnXRight = nXLeft;

        // new separation within the actual one? -> reduce boundary
        // and add new entry for reminder
        if ( (nXLeft >= pSep->mnXLeft) && (nXRight <= pSep->mnXRight) )
        {
            pNewSep             = new ImplRegionBandSep;
            pNewSep->mnXLeft    = pSep->mnXLeft;
            pNewSep->mnXRight   = nXLeft;

            pSep->mnXLeft = nXRight;

            // connections from the new separation
            pNewSep->mpNextSep = pSep;

            // connections to the new separation
            if ( pSep == mpFirstSep )
                mpFirstSep = pNewSep;
            else
                pPrevSep->mpNextSep = pNewSep;
        }

        pPrevSep = pSep;
        pSep = pSep->mpNextSep;
    }

    OptimizeBand();
}

// -----------------------------------------------------------------------

BOOL ImplRegionBand::IsInside( long nX )
{
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        if ( (pSep->mnXLeft <= nX) && (pSep->mnXRight >= nX) )
            return TRUE;

        pSep = pSep->mpNextSep;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ImplRegionBand::IsOver( long nLeft, long nRight )
{
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        if ( (pSep->mnXLeft < nRight) && (pSep->mnXRight > nLeft) )
            return TRUE;

        pSep = pSep->mpNextSep;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ImplRegionBand::IsInside( long nLeft, long nRight )
{
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        if ( (pSep->mnXLeft >= nLeft) && (nRight <= pSep->mnXRight) )
            return TRUE;

        pSep = pSep->mpNextSep;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

long ImplRegionBand::GetXLeftBoundary() const
{
    DBG_ASSERT( mpFirstSep != NULL, "ImplRegionBand::XLeftBoundary -> no separation in band!" );

    return mpFirstSep->mnXLeft;
}

// -----------------------------------------------------------------------

long ImplRegionBand::GetXRightBoundary() const
{
    DBG_ASSERT( mpFirstSep != NULL, "ImplRegionBand::XRightBoundary -> no separation in band!" );

    // search last separation
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep->mpNextSep )
        pSep = pSep->mpNextSep;
    return pSep->mnXRight;
}

// -----------------------------------------------------------------------

BOOL ImplRegionBand::operator==( const ImplRegionBand& rRegionBand ) const
{
    ImplRegionBandSep*   pOwnRectBandSep = mpFirstSep;
    ImplRegionBandSep*   pSecondRectBandSep = rRegionBand.mpFirstSep;
    while ( pOwnRectBandSep && pSecondRectBandSep )
    {
        // get boundaries of current rectangle
        long nOwnXLeft = pOwnRectBandSep->mnXLeft;
        long nSecondXLeft = pSecondRectBandSep->mnXLeft;
        if ( nOwnXLeft != nSecondXLeft )
            return FALSE;

        long nOwnXRight = pOwnRectBandSep->mnXRight;
        long nSecondXRight = pSecondRectBandSep->mnXRight;
        if ( nOwnXRight != nSecondXRight )
            return FALSE;

        // get next separation from current band
        pOwnRectBandSep = pOwnRectBandSep->mpNextSep;

        // get next separation from current band
        pSecondRectBandSep = pSecondRectBandSep->mpNextSep;
    }

    // differnt number of separations?
    if ( pOwnRectBandSep || pSecondRectBandSep )
        return FALSE;

    return TRUE;
}
