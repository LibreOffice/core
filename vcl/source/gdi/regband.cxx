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

#include <sal/config.h>

#include <basegfx/numeric/ftools.hxx>
#include <tools/helpers.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <regband.hxx>

// ImplRegionBand

// Each band contains all rectangles between upper and lower border.
// For Union, Intersect, Xor and Exclude operations rectangles of
// equal height are evaluated. The borders of the bands should always
// be chosen such that this is possible.

// If possible, rectangles within the bands are condensed.

// When converting polygons all points of the polygon are registered
// in the individual bands (for each band they are stored as
// points in a list). After registration of these points they are
// converted to rectangles and the points in the list are deleted.

ImplRegionBand::ImplRegionBand( tools::Long nTop, tools::Long nBottom )
{
    // save boundaries
    mnYTop              = nTop;
    mnYBottom           = nBottom;

    // initialize lists
    mpNextBand          = nullptr;
    mpPrevBand          = nullptr;
    mpFirstSep          = nullptr;
    mpFirstBandPoint    = nullptr;
    mbTouched           = false;
}

ImplRegionBand::ImplRegionBand(
    const ImplRegionBand& rRegionBand,
    const bool bIgnorePoints)
{
    // copy boundaries
    mnYTop              = rRegionBand.mnYTop;
    mnYBottom           = rRegionBand.mnYBottom;
    mbTouched           = rRegionBand.mbTouched;

    // initialisation
    mpNextBand          = nullptr;
    mpPrevBand          = nullptr;
    mpFirstSep          = nullptr;
    mpFirstBandPoint    = nullptr;

    // copy all elements of the list with separations
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep = nullptr;
    ImplRegionBandSep* pSep = rRegionBand.mpFirstSep;
    while ( pSep )
    {
        // create new and copy data
        pNewSep             = new ImplRegionBandSep;
        pNewSep->mnXLeft    = pSep->mnXLeft;
        pNewSep->mnXRight   = pSep->mnXRight;
        pNewSep->mbRemoved  = pSep->mbRemoved;
        pNewSep->mpNextSep  = nullptr;
        if ( pSep == rRegionBand.mpFirstSep )
            mpFirstSep = pNewSep;
        else
            pPrevSep->mpNextSep = pNewSep;

        pPrevSep = pNewSep;
        pSep = pSep->mpNextSep;
    }

    if (  bIgnorePoints)
        return;

    // Copy points.
    ImplRegionBandPoint* pPoint = rRegionBand.mpFirstBandPoint;
    ImplRegionBandPoint* pPrevPointCopy = nullptr;
    while (pPoint != nullptr)
    {
        ImplRegionBandPoint* pPointCopy = new ImplRegionBandPoint;
        pPointCopy->mpNextBandPoint = nullptr;
        pPointCopy->mnX = pPoint->mnX;
        pPointCopy->mnLineId = pPoint->mnLineId;
        pPointCopy->mbEndPoint = pPoint->mbEndPoint;
        pPointCopy->meLineType = pPoint->meLineType;

        if (pPrevPointCopy != nullptr)
            pPrevPointCopy->mpNextBandPoint = pPointCopy;
        else
            mpFirstBandPoint = pPointCopy;

        pPrevPointCopy = pPointCopy;
        pPoint = pPoint->mpNextBandPoint;
    }
}

ImplRegionBand::~ImplRegionBand()
{
    SAL_WARN_IF( mpFirstBandPoint != nullptr, "vcl", "ImplRegionBand::~ImplRegionBand -> pointlist not empty" );

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

// generate separations from lines and process union with existing
// separations

void ImplRegionBand::ProcessPoints()
{
    // check Pointlist
    ImplRegionBandPoint* pRegionBandPoint = mpFirstBandPoint;
    while ( pRegionBandPoint )
    {
        // within list?
        if ( pRegionBandPoint->mpNextBandPoint )
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

        // remove already processed points
        delete pRegionBandPoint->mpNextBandPoint;
        delete pRegionBandPoint;

        // continue with next element in the list
        pRegionBandPoint = pNextBandPoint;
    }

    // remove last element if necessary
    delete pRegionBandPoint;

    // list is now empty
    mpFirstBandPoint = nullptr;
}

// generate separations from lines and process union with existing
// separations

bool ImplRegionBand::InsertPoint( tools::Long nX, tools::Long nLineId,
                                  bool bEndPoint, LineType eLineType )
{
    if ( !mpFirstBandPoint )
    {
        mpFirstBandPoint                  = new ImplRegionBandPoint;
        mpFirstBandPoint->mnX             = nX;
        mpFirstBandPoint->mnLineId        = nLineId;
        mpFirstBandPoint->mbEndPoint      = bEndPoint;
        mpFirstBandPoint->meLineType      = eLineType;
        mpFirstBandPoint->mpNextBandPoint = nullptr;
        return true;
    }

    // look if line already touched the band
    ImplRegionBandPoint* pRegionBandPoint = mpFirstBandPoint;
    ImplRegionBandPoint* pLastTestedRegionBandPoint = nullptr;
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
                        pRegionBandPoint->mbEndPoint = true;
                        return true;
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
                return false;
        }

        // use next element
        pLastTestedRegionBandPoint = pRegionBandPoint;
        pRegionBandPoint = pRegionBandPoint->mpNextBandPoint;
    }

    // search appropriate position and insert point into the list
    ImplRegionBandPoint* pNewRegionBandPoint;

    pRegionBandPoint = mpFirstBandPoint;
    pLastTestedRegionBandPoint = nullptr;
    while ( pRegionBandPoint )
    {
        // new point completely left? -> insert as first point
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

            return true;
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
    pNewRegionBandPoint->mpNextBandPoint    = nullptr;

    // connections to the new point
    pLastTestedRegionBandPoint->mpNextBandPoint = pNewRegionBandPoint;

    return true;
}

void ImplRegionBand::MoveX( tools::Long nHorzMove )
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

void ImplRegionBand::ScaleX( double fHorzScale )
{
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        pSep->mnXLeft = basegfx::fround<tools::Long>(pSep->mnXLeft * fHorzScale);
        pSep->mnXRight = basegfx::fround<tools::Long>(pSep->mnXRight * fHorzScale);
        pSep = pSep->mpNextSep;
    }
}

// combine overlapping separations

void ImplRegionBand::OptimizeBand()
{
    ImplRegionBandSep* pPrevSep = nullptr;
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

        // overlapping separations? -> combine!
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
}

void ImplRegionBand::Union( tools::Long nXLeft, tools::Long nXRight )
{
    SAL_WARN_IF( nXLeft > nXRight, "vcl", "ImplRegionBand::Union(): nxLeft > nXRight" );

    // band empty? -> add element
    if ( !mpFirstSep )
    {
        mpFirstSep              = new ImplRegionBandSep;
        mpFirstSep->mnXLeft     = nXLeft;
        mpFirstSep->mnXRight    = nXRight;
        mpFirstSep->mbRemoved   = false;
        mpFirstSep->mpNextSep   = nullptr;
        return;
    }

    // process real union
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep = nullptr;
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        // new separation completely inside? nothing to do!
        if ( (nXLeft >= pSep->mnXLeft) && (nXRight <= pSep->mnXRight) )
            return;

        // new separation completely left? -> new separation!
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

        // new separation overlapping from left? -> extend boundary
        if ( (nXRight >= pSep->mnXLeft) && (nXLeft <= pSep->mnXLeft) )
            pSep->mnXLeft = nXLeft;

        // new separation overlapping from right? -> extend boundary
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
            pNewSep->mbRemoved  = false;

            pSep->mpNextSep     = pNewSep;
            pNewSep->mpNextSep  = nullptr;
            break;
        }

        pPrevSep = pSep;
        pSep = pSep->mpNextSep;
    }

    OptimizeBand();
}

void ImplRegionBand::Intersect( tools::Long nXLeft, tools::Long nXRight )
{
    SAL_WARN_IF( nXLeft > nXRight, "vcl", "ImplRegionBand::Intersect(): nxLeft > nXRight" );

    // band has been touched
    mbTouched = true;

    // band empty? -> nothing to do
    if ( !mpFirstSep )
        return;

    // process real intersection
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep )
    {
        // new separation completely outside? -> remove separation
        if ( (nXRight < pSep->mnXLeft) || (nXLeft > pSep->mnXRight) )
            // will be removed from the optimizer
            pSep->mbRemoved = true;

        // new separation overlapping from left? -> reduce right boundary
        if ( (nXLeft <= pSep->mnXLeft) &&
             (nXRight <= pSep->mnXRight) &&
             (nXRight >= pSep->mnXLeft) )
            pSep->mnXRight = nXRight;

        // new separation overlapping from right? -> reduce right boundary
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

void ImplRegionBand::Exclude( tools::Long nXLeft, tools::Long nXRight )
{
    SAL_WARN_IF( nXLeft > nXRight, "vcl", "ImplRegionBand::Exclude(): nxLeft > nXRight" );

    // band has been touched
    mbTouched = true;

    // band empty? -> nothing to do
    if ( !mpFirstSep )
        return;

    // process real exclusion
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep = nullptr;
    ImplRegionBandSep* pSep = mpFirstSep;
    while ( pSep  )
    {
        bool bSepProcessed = false;

        // new separation completely overlapping? -> remove separation
        if ( (nXLeft <= pSep->mnXLeft) && (nXRight >= pSep->mnXRight) )
        {
            // will be removed from the optimizer
            pSep->mbRemoved = true;
            bSepProcessed = true;
        }

        // new separation overlapping from left? -> reduce boundary
        if ( !bSepProcessed )
        {
            if ( (nXRight >= pSep->mnXLeft) && (nXLeft <= pSep->mnXLeft) )
            {
                pSep->mnXLeft = nXRight+1;
                bSepProcessed = true;
            }
        }

        // new separation overlapping from right? -> reduce boundary
        if ( !bSepProcessed )
        {
            if ( (nXLeft <= pSep->mnXRight) && (nXRight > pSep->mnXRight) )
            {
                pSep->mnXRight = nXLeft-1;
                bSepProcessed = true;
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
                pNewSep->mbRemoved  = false;

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

void ImplRegionBand::XOr( tools::Long nXLeft, tools::Long nXRight )
{
    SAL_WARN_IF( nXLeft > nXRight, "vcl", "ImplRegionBand::XOr(): nxLeft > nXRight" );

    // #i46602# Reworked rectangle Xor

    // In general, we can distinguish 11 cases of intersection
    // (details below). The old implementation explicitly handled 7
    // cases (numbered in the order of appearance, use CVS to get your
    // hands on the old version), therefore, I've sticked to that
    // order, and added four more cases. The code below references
    // those numbers via #1, #2, etc.

    // Num Mnem        newX:oldX newY:oldY  Description                                             Result          Can quit?

    // #1  Empty band      -         -      The band is empty, thus, simply add new bandSep         just add        Yes

    // #2  apart           -         -      The rectangles are disjunct, add new one as is          just add        Yes

    // #3  atop            ==        ==     The rectangles are _exactly_ the same, remove existing  just remove     Yes

    // #4  around          <         >      The new rectangle extends the old to both sides         intersect       No

    // #5  left            <         <      The new rectangle is left of the old (but intersects)   intersect       Yes

    // #5b left-atop       <         ==     The new is left of the old, and coincides on the right  intersect       Yes

    // #6  right           >         >      The new is right of the old (but intersects)            intersect       No

    // #6b right-atop      ==        >      The new is right of the old, and coincides on the left  intersect       No

    // #7 inside           >         <      The new is fully inside the old                         intersect       Yes

    // #8 inside-right     >         ==     The new is fully inside the old, coincides on the right intersect       Yes

    // #9 inside-left      ==        <      The new is fully inside the old, coincides on the left  intersect       Yes

    // Then, to correctly perform XOr, the segment that's switched off
    // (i.e. the overlapping part of the old and the new segment) must
    // be extended by one pixel value at each border:
    //           1   1
    // 0   4     0   4
    // 111100000001111

    // Clearly, the leading band sep now goes from 0 to 3, and the
    // trailing band sep from 11 to 14. This mimics the xor look of a
    // bitmap operation.

    // band empty? -> add element
    if ( !mpFirstSep )
    {
        mpFirstSep              = new ImplRegionBandSep;
        mpFirstSep->mnXLeft     = nXLeft;
        mpFirstSep->mnXRight    = nXRight;
        mpFirstSep->mbRemoved   = false;
        mpFirstSep->mpNextSep   = nullptr;
        return;
    }

    // process real xor
    ImplRegionBandSep* pNewSep;
    ImplRegionBandSep* pPrevSep = nullptr;
    ImplRegionBandSep* pSep = mpFirstSep;

    while ( pSep  )
    {
        tools::Long nOldLeft( pSep->mnXLeft );
        tools::Long nOldRight( pSep->mnXRight );

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
                pNewSep->mbRemoved  = false;

                // connections from the new separation
                pNewSep->mpNextSep = pSep;

                // connections to the new separation
                if ( pSep == mpFirstSep )
                    mpFirstSep = pNewSep;
                else
                    pPrevSep->mpNextSep = pNewSep;
                pPrevSep = nullptr; // do not run accidentally into the "right" case when breaking the loop
                break;
            }
            else if( nXLeft == nOldLeft && nXRight == nOldRight )
            {
                // #3
                pSep->mbRemoved = true;
                pPrevSep = nullptr; // do not run accidentally into the "right" case when breaking the loop
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

                pPrevSep = nullptr; // do not run accidentally into the "right" case when breaking the loop
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
                    pSep->mbRemoved = true;
                }
                else
                {
                    pSep->mnXLeft = nXRight+1; // 9

                    pPrevSep = nullptr; // do not run accidentally into the "right" case when breaking the loop
                    break;
                }
            }
            else // if( nXLeft != nOldLeft && nXRight != nOldRight ) follows automatically
            {
                // #4,5,6,7
                SAL_WARN_IF( nXLeft == nOldLeft || nXRight == nOldRight, "vcl",
                            "ImplRegionBand::XOr(): Case 4,5,6,7 expected all coordinates to be not equal!" );

                // The plain-jane check would look like this:

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
                SAL_WARN_IF( nOldLeft==nXLeft || nXLeft>nOldRight || nOldRight>=nXRight, "vcl",
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
                    pNewSep->mbRemoved  = false;

                    // connections from the new separation
                    pSep->mpNextSep = pNewSep;

                    pPrevSep = nullptr; // do not run accidentally into the "right" case when breaking the loop
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
        pNewSep->mpNextSep  = nullptr;
        pNewSep->mbRemoved  = false;

        // connections from the new separation
        pPrevSep->mpNextSep = pNewSep;
    }

    OptimizeBand();
}

bool ImplRegionBand::Contains( tools::Long nX )
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

tools::Long ImplRegionBand::GetXLeftBoundary() const
{
    assert(mpFirstSep && "ImplRegionBand::XLeftBoundary -> no separation in band!");

    return mpFirstSep->mnXLeft;
}

tools::Long ImplRegionBand::GetXRightBoundary() const
{
    SAL_WARN_IF( mpFirstSep == nullptr, "vcl", "ImplRegionBand::XRightBoundary -> no separation in band!" );

    // search last separation
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
        // get boundaries of current rectangle
        tools::Long nOwnXLeft = pOwnRectBandSep->mnXLeft;
        tools::Long nSecondXLeft = pSecondRectBandSep->mnXLeft;
        if ( nOwnXLeft != nSecondXLeft )
            return false;

        tools::Long nOwnXRight = pOwnRectBandSep->mnXRight;
        tools::Long nSecondXRight = pSecondRectBandSep->mnXRight;
        if ( nOwnXRight != nSecondXRight )
            return false;

        // get next separation from current band
        pOwnRectBandSep = pOwnRectBandSep->mpNextSep;

        // get next separation from current band
        pSecondRectBandSep = pSecondRectBandSep->mpNextSep;
    }

    // different number of separations?
    return !(pOwnRectBandSep || pSecondRectBandSep);
}

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
    if (pLowerBand->mpNextBand != nullptr)
        pLowerBand->mpNextBand->mpPrevBand = pLowerBand;

    return pLowerBand;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
