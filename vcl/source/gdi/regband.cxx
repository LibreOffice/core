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
#include "precompiled_vcl.hxx"
#include <tools/debug.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/regband.hxx>

#include <algorithm>


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
    mbTouched           = sal_False;
}

// -----------------------------------------------------------------------

ImplRegionBand::ImplRegionBand(
    const ImplRegionBand& rRegionBand,
    const bool bIgnorePoints)
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
    ImplRegionBandSep* pPrevSep = 0;
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

    if ( ! bIgnorePoints)
    {
        // Copy points.
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

sal_Bool ImplRegionBand::InsertPoint( long nX, long nLineId,
                                  sal_Bool bEndPoint, LineType eLineType )
{
    if ( !mpFirstBandPoint )
    {
        mpFirstBandPoint                  = new ImplRegionBandPoint;
        mpFirstBandPoint->mnX             = nX;
        mpFirstBandPoint->mnLineId        = nLineId;
        mpFirstBandPoint->mbEndPoint      = bEndPoint;
        mpFirstBandPoint->meLineType      = eLineType;
        mpFirstBandPoint->mpNextBandPoint = NULL;
        return sal_True;
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
                        pRegionBandPoint->mbEndPoint = sal_True;
                        return sal_True;
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
                return sal_False;
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

            return sal_True;
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

    return sal_True;
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

sal_Bool ImplRegionBand::OptimizeBand()
{
    ImplRegionBandSep* pPrevSep = 0;
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

    return sal_True;
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
        mpFirstSep->mbRemoved   = sal_False;
        mpFirstSep->mpNextSep   = NULL;
        return;
    }

    // process real union
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep = 0;
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
            pNewSep->mbRemoved  = sal_False;

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
            pNewSep->mbRemoved  = sal_False;

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
    mbTouched = sal_True;

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
            pSep->mbRemoved = sal_True;

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
    mbTouched = sal_True;

    // band empty? -> nothing to do
    if ( !mpFirstSep )
        return;

    // process real exclusion
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep = 0;
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep  )
    {
        sal_Bool bSepProcessed = sal_False;

        // new separation completely overlapping? -> remove separation
        if ( (nXLeft <= pSep->mnXLeft) && (nXRight >= pSep->mnXRight) )
        {
            // will be removed from the optimizer
            pSep->mbRemoved = sal_True;
            bSepProcessed = sal_True;
        }

        // new separation overlaping from left? -> reduce boundary
        if ( !bSepProcessed )
        {
            if ( (nXRight >= pSep->mnXLeft) && (nXLeft <= pSep->mnXLeft) )
            {
                pSep->mnXLeft = nXRight+1;
                bSepProcessed = sal_True;
            }
        }

        // new separation overlaping from right? -> reduce boundary
        if ( !bSepProcessed )
        {
            if ( (nXLeft <= pSep->mnXRight) && (nXRight > pSep->mnXRight) )
            {
                pSep->mnXRight = nXLeft-1;
                bSepProcessed = sal_True;
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
                pNewSep->mbRemoved  = sal_False;

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

    // #i46602# Reworked rectangle Xor
    //
    // In general, we can distinguish 11 cases of intersection
    // (details below). The old implementation explicitely handled 7
    // cases (numbered in the order of appearance, use CVS to get your
    // hands on the old version), therefore, I've sticked to that
    // order, and added four more cases. The code below references
    // those numbers via #1, #2, etc.
    //
    // Num Mnem        newX:oldX newY:oldY  Description                                             Result          Can quit?
    //
    // #1  Empty band      -         -      The band is empty, thus, simply add new bandSep         just add        Yes
    //
    // #2  apart           -         -      The rectangles are disjunct, add new one as is          just add        Yes
    //
    // #3  atop            ==        ==     The rectangles are _exactly_ the same, remove existing  just remove     Yes
    //
    // #4  around          <         >      The new rectangle extends the old to both sides         intersect       No
    //
    // #5  left            <         <      The new rectangle is left of the old (but intersects)   intersect       Yes
    //
    // #5b left-atop       <         ==     The new is left of the old, and coincides on the right  intersect       Yes
    //
    // #6  right           >         >      The new is right of the old (but intersects)            intersect       No
    //
    // #6b right-atop      ==        >      The new is right of the old, and coincides on the left  intersect       No
    //
    // #7 inside           >         <      The new is fully inside the old                         intersect       Yes
    //
    // #8 inside-right     >         ==     The new is fully inside the old, coincides on the right intersect       Yes
    //
    // #9 inside-left      ==        <      The new is fully inside the old, coincides on the left  intersect       Yes
    //
    //
    // Then, to correctly perform XOr, the segment that's switched off
    // (i.e. the overlapping part of the old and the new segment) must
    // be extended by one pixel value at each border:
    //           1   1
    // 0   4     0   4
    // 111100000001111
    //
    // Clearly, the leading band sep now goes from 0 to 3, and the
    // trailing band sep from 11 to 14. This mimicks the xor look of a
    // bitmap operation.
    //

    // band empty? -> add element
    if ( !mpFirstSep )
    {
        mpFirstSep              = new ImplRegionBandSep;
        mpFirstSep->mnXLeft     = nXLeft;
        mpFirstSep->mnXRight    = nXRight;
        mpFirstSep->mbRemoved   = sal_False;
        mpFirstSep->mpNextSep   = NULL;
        return;
    }

    // process real xor
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep = 0;
    ImplRegionBandSep* pSep = mpFirstSep;

    while ( pSep  )
    {
        long nOldLeft( pSep->mnXLeft );
        long nOldRight( pSep->mnXRight );

        // did the current segment actually touch the new rect? If
        // not, skip all comparisons, go on, loop and try to find
        // intersecting bandSep
        if( nXLeft <= nOldRight )
        {
            if( nXRight < nOldLeft )
            {
                // #2

                // add _before_ current bandSep
                pNewSep             = new ImplRegionBandSep;
                pNewSep->mnXLeft    = nXLeft;
                pNewSep->mnXRight   = nXRight;
                pNewSep->mpNextSep  = pSep;
                pNewSep->mbRemoved  = sal_False;

                // connections from the new separation
                pNewSep->mpNextSep = pSep;

                // connections to the new separation
                if ( pSep == mpFirstSep )
                    mpFirstSep = pNewSep;
                else
                    pPrevSep->mpNextSep = pNewSep;
                pPrevSep = NULL; // do not run accidentally into the "right" case when breaking the loop
                break;
            }
            else if( nXLeft == nOldLeft && nXRight == nOldRight )
            {
                // #3
                pSep->mbRemoved = sal_True;
                pPrevSep = NULL; // do not run accidentally into the "right" case when breaking the loop
                break;
            }
            else if( nXLeft != nOldLeft && nXRight == nOldRight )
            {
                // # 5b, 8
                if( nXLeft < nOldLeft )
                {
                    nXRight = nOldLeft; // 5b
                }
                else
                {
                    nXRight = nXLeft; // 8
                    nXLeft = nOldLeft;
                }

                pSep->mnXLeft = nXLeft;
                pSep->mnXRight = nXRight-1;

                pPrevSep = NULL; // do not run accidentally into the "right" case when breaking the loop
                break;
            }
            else if( nXLeft == nOldLeft && nXRight != nOldRight )
            {
                // # 6b, 9

                if( nXRight > nOldRight )
                {
                    nXLeft = nOldRight+1; // 6b

                    // cannot break here, simply mark segment as removed,
                    // and go on with adapted nXLeft/nXRight
                    pSep->mbRemoved = sal_True;
                }
                else
                {
                    pSep->mnXLeft = nXRight+1; // 9

                    pPrevSep = NULL; // do not run accidentally into the "right" case when breaking the loop
                    break;
                }
            }
            else // if( nXLeft != nOldLeft && nXRight != nOldRight ) follows automatically
            {
                // #4,5,6,7
                DBG_ASSERT( nXLeft != nOldLeft && nXRight != nOldRight,
                            "ImplRegionBand::XOr(): Case 4,5,6,7 expected all coordinates to be not equal!" );

                // The plain-jane check would look like this:
                //
                // if( nXLeft < nOldLeft )
                // {
                //     // #4,5
                //     if( nXRight > nOldRight )
                //     {
                //        // #4
                //     }
                //     else
                //     {
                //         // #5 done!
                //     }
                // }
                // else
                // {
                //     // #6,7
                //     if( nXRight > nOldRight )
                //     {
                //         // #6
                //     }
                //     else
                //     {
                //         // #7 done!
                //     }
                // }
                //
                // but since we generally don't have to care whether
                // it's 4 or 6 (only that we must not stop processing
                // here), condensed that in such a way that only the
                // coordinates get shuffled into correct ordering.

                if( nXLeft < nOldLeft )
                    ::std::swap( nOldLeft, nXLeft );

                bool bDone( false );

                if( nXRight < nOldRight )
                {
                    ::std::swap( nOldRight, nXRight );
                    bDone = true;
                }

                // now, nOldLeft<nXLeft<=nOldRight<nXRight always
                // holds. Note that we need the nXLeft<=nOldRight here, as
                // the intersection part might be only one pixel (original
                // nXLeft==nXRight)
                DBG_ASSERT( nOldLeft<nXLeft && nXLeft<=nOldRight && nOldRight<nXRight,
                            "ImplRegionBand::XOr(): Case 4,5,6,7 expected coordinates to be ordered now!" );

                pSep->mnXLeft = nOldLeft;
                pSep->mnXRight = nXLeft-1;

                nXLeft = nOldRight+1;
                // nxRight is already setup correctly

                if( bDone )
                {
                    // add behind current bandSep
                    pNewSep = new ImplRegionBandSep;

                    pNewSep->mnXLeft    = nXLeft;
                    pNewSep->mnXRight   = nXRight;
                    pNewSep->mpNextSep  = pSep->mpNextSep;
                    pNewSep->mbRemoved  = sal_False;

                    // connections from the new separation
                    pSep->mpNextSep = pNewSep;

                    pPrevSep = NULL; // do not run accidentally into the "right" case when breaking the loop
                    break;
                }
            }
        }

        pPrevSep = pSep;
        pSep = pSep->mpNextSep;
    }

    // new separation completely right of existing bandSeps ?
    if( pPrevSep && nXLeft >= pPrevSep->mnXRight )
    {
        pNewSep             = new ImplRegionBandSep;
        pNewSep->mnXLeft    = nXLeft;
        pNewSep->mnXRight   = nXRight;
        pNewSep->mpNextSep  = NULL;
        pNewSep->mbRemoved  = sal_False;

        // connections from the new separation
        pPrevSep->mpNextSep = pNewSep;
    }

    OptimizeBand();
}

// -----------------------------------------------------------------------

sal_Bool ImplRegionBand::IsInside( long nX )
{
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        if ( (pSep->mnXLeft <= nX) && (pSep->mnXRight >= nX) )
            return sal_True;

        pSep = pSep->mpNextSep;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool ImplRegionBand::IsOver( long nLeft, long nRight )
{
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        if ( (pSep->mnXLeft < nRight) && (pSep->mnXRight > nLeft) )
            return sal_True;

        pSep = pSep->mpNextSep;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool ImplRegionBand::IsInside( long nLeft, long nRight )
{
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        if ( (pSep->mnXLeft >= nLeft) && (nRight <= pSep->mnXRight) )
            return sal_True;

        pSep = pSep->mpNextSep;
    }

    return sal_False;
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

sal_Bool ImplRegionBand::operator==( const ImplRegionBand& rRegionBand ) const
{
    ImplRegionBandSep*   pOwnRectBandSep = mpFirstSep;
    ImplRegionBandSep*   pSecondRectBandSep = rRegionBand.mpFirstSep;
    while ( pOwnRectBandSep && pSecondRectBandSep )
    {
        // get boundaries of current rectangle
        long nOwnXLeft = pOwnRectBandSep->mnXLeft;
        long nSecondXLeft = pSecondRectBandSep->mnXLeft;
        if ( nOwnXLeft != nSecondXLeft )
            return sal_False;

        long nOwnXRight = pOwnRectBandSep->mnXRight;
        long nSecondXRight = pSecondRectBandSep->mnXRight;
        if ( nOwnXRight != nSecondXRight )
            return sal_False;

        // get next separation from current band
        pOwnRectBandSep = pOwnRectBandSep->mpNextSep;

        // get next separation from current band
        pSecondRectBandSep = pSecondRectBandSep->mpNextSep;
    }

    // differnt number of separations?
    if ( pOwnRectBandSep || pSecondRectBandSep )
        return sal_False;

    return sal_True;
}

// -----------------------------------------------------------------------

ImplRegionBand* ImplRegionBand::SplitBand (const sal_Int32 nY)
{
    OSL_ASSERT(nY>mnYTop);
    OSL_ASSERT(nY<=mnYBottom);

    // Create a copy of the given band (we tell the constructor to copy the points together
    // with the seps.)
    ImplRegionBand* pLowerBand = new ImplRegionBand(*this, false);

    // Adapt vertical coordinates.
    mnYBottom = nY-1;
    pLowerBand->mnYTop = nY;

    // Insert new band into list of bands.
    pLowerBand->mpNextBand = mpNextBand;
    mpNextBand = pLowerBand;
    pLowerBand->mpPrevBand = this;
    if (pLowerBand->mpNextBand != NULL)
        pLowerBand->mpNextBand->mpPrevBand = pLowerBand;

    return pLowerBand;
}
