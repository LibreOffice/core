/*************************************************************************
 *
 *  $RCSfile: region.cxx,v $
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

#define _SV_REGION_CXX

#include <limits.h>

#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _REGION_H
#include <region.h>
#endif
#ifndef _REGION_HXX
#include <region.hxx>
#endif
#ifndef _REGBAND_HXX
#include <regband.hxx>
#endif

// =======================================================================
//
// ImplRegionBand
//
// Die Klassen RegionBand/ImplRegionBand speichert Regionen in Form von
// Rechtecken ab. Die Region ist in Y-Richtung in Baendern unterteilt, die
// wiederum ein oder mehrere Rechtecke mit der Hoehe des Bandes enthalten.
//
// Leere Baender werden entfernt.
//
// Polygone und PolyPolygone werden ebenfalls in Rechtecke zerlegt und in
// der Baendern abgelegt. Hierzu werden alle Punkte der einzelnen Polygone
// mit dem Bresenham-Algorithmus berechnet und in die Baender eingetragen.
// Nach der vollstaendigen Berechung aller Kanten werden die entsprechenden
// Rechntecke berechnet

// =======================================================================

static ImplRegionBase aImplNullRegion   = { 0, 0, NULL };
static ImplRegionBase aImplEmptyRegion  = { 0, 0, NULL };

// =======================================================================

DBG_NAME( Region );
DBG_NAMEEX( Polygon );
DBG_NAMEEX( PolyPolygon );

// -----------------------------------------------------------------------

#ifdef DBG_UTIL
const char* ImplDbgTestRegion( const void* pObj )
{
    Region*       pRegion = (Region*)pObj;
    ImplRegion*   pImplRegion = pRegion->ImplGetImplRegion();

    if ( aImplNullRegion.mnRefCount )
        return "Null-Region-RefCount modified";
    if ( aImplNullRegion.mnRectCount )
        return "Null-Region-RectCount modified";
    if ( aImplNullRegion.mpPolyPoly )
        return "Null-Region-PolyPoly modified";
    if ( aImplEmptyRegion.mnRefCount )
        return "Emptry-Region-RefCount modified";
    if ( aImplEmptyRegion.mnRectCount )
        return "Emptry-Region-RectCount modified";
    if ( aImplEmptyRegion.mpPolyPoly )
        return "Emptry-Region-PolyPoly modified";

    if ( (pImplRegion != &aImplEmptyRegion) && (pImplRegion != &aImplNullRegion) )
    {
        ULONG                   nCount = 0;
        const ImplRegionBand*   pBand = pImplRegion->ImplGetFirstRegionBand();
        while ( pBand )
        {
            if ( pBand->mnYBottom < pBand->mnYTop )
                return "YBottom < YTop";
            if ( pBand->mpNextBand )
            {
                if ( pBand->mnYBottom >= pBand->mpNextBand->mnYTop )
                    return "overlapping bands in region";
            }
            if ( pBand->mbTouched > 1 )
                return "Band-mbTouched overwrite";

            ImplRegionBandSep* pSep = pBand->mpFirstSep;
            while ( pSep )
            {
                if ( pSep->mnXRight < pSep->mnXLeft )
                    return "XLeft < XRight";
                if ( pSep->mpNextSep )
                {
                    if ( pSep->mnXRight >= pSep->mpNextSep->mnXLeft )
                        return "overlapping separations in region";
                }
                if ( pSep->mbRemoved > 1 )
                    return "Sep-mbRemoved overwrite";

                nCount++;
                pSep = pSep->mpNextSep;
            }

            pBand = pBand->mpNextBand;
        }

        if ( pImplRegion->mnRectCount != nCount )
            return "mnRetCount is not valid";
    }

    return NULL;
}
#endif

// =======================================================================

inline void Region::ImplPolyPolyRegionToBandRegion()
{
    if ( mpImplRegion->mpPolyPoly )
        ImplPolyPolyRegionToBandRegionFunc();
}

// =======================================================================

ImplRegion::ImplRegion()
{
    mnRefCount          = 1;
    mnRectCount         = 0;
    mpPolyPoly          = NULL;
    mpFirstBand         = NULL;
    mpLastCheckedBand   = NULL;
}

// ------------------------------------------------------------------------

ImplRegion::ImplRegion( const PolyPolygon& rPolyPoly )
{
    mnRefCount          = 1;
    mnRectCount         = 0;
    mpFirstBand         = NULL;
    mpLastCheckedBand   = NULL;
    mpPolyPoly          = new PolyPolygon( rPolyPoly );
}

// -----------------------------------------------------------------------

ImplRegion::ImplRegion( const ImplRegion& rImplRegion )
{
    mnRefCount          = 1;
    mnRectCount         = rImplRegion.mnRectCount;
    mpFirstBand         = NULL;
    mpLastCheckedBand   = NULL;
    if ( rImplRegion.mpPolyPoly )
        mpPolyPoly = new PolyPolygon( *rImplRegion.mpPolyPoly );
    else
        mpPolyPoly = NULL;

    // insert band(s) into the list
    ImplRegionBand* pNewBand;
    ImplRegionBand* pPrevBand;
    ImplRegionBand* pBand = rImplRegion.mpFirstBand;
    while ( pBand )
    {
        pNewBand = new ImplRegionBand( *pBand );

        // first element? -> set as first into the list
        if ( pBand == rImplRegion.mpFirstBand )
            mpFirstBand = pNewBand;
        else
            pPrevBand->mpNextBand = pNewBand;

        pPrevBand = pNewBand;
        pBand = pBand->mpNextBand;
    }
}

// -----------------------------------------------------------------------

ImplRegion::~ImplRegion()
{
    DBG_ASSERT( (this != &aImplEmptyRegion) && (this != &aImplNullRegion),
                "ImplRegion::~ImplRegion() - Empty oder NULL-Region" )

    ImplRegionBand* pBand = mpFirstBand;
    while ( pBand )
    {
        ImplRegionBand* pTempBand = pBand->mpNextBand;
        delete pBand;
        pBand = pTempBand;
    }

    delete mpPolyPoly;
}

// -----------------------------------------------------------------------
//
// create complete range of bands in single steps

void ImplRegion::CreateBandRange( long nYTop, long nYBottom )
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
            pNewBand->mpPrevBand = pBand;

        pBand = pBand->mpNextBand;
    }
}

// -----------------------------------------------------------------------

BOOL ImplRegion::InsertLine( const Point& rStartPt, const Point& rEndPt,
                             long nLineId )
{
    long nX, nY;

    // lines consisting of a single point do not interest here
    if ( rStartPt == rEndPt )
        return TRUE;

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

        // last point
        InsertPoint( Point( nEndX, nEndY ), nLineId, TRUE, eLineType );
    }

    return TRUE;
}

// -----------------------------------------------------------------------
//
// search for appropriate place for the new point

BOOL ImplRegion::InsertPoint( const Point &rPoint, long nLineID,
                              BOOL bEndPoint, LineType eLineType )
{
    DBG_ASSERT( mpFirstBand != NULL, "ImplRegion::InsertPoint - no bands available!" );

    if ( rPoint.Y() == mpLastCheckedBand->mnYTop )
    {
        mpLastCheckedBand->InsertPoint( rPoint.X(), nLineID, bEndPoint, eLineType );
        return TRUE;
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
                return TRUE;
            }

            mpLastCheckedBand = mpLastCheckedBand->mpNextBand;
        }

        DBG_ERROR( "ImplRegion::InsertPoint reached the end of the list!" );
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
                return TRUE;
            }

            mpLastCheckedBand = mpLastCheckedBand->mpPrevBand;
        }

        DBG_ERROR( "ImplRegion::InsertPoint reached the beginning of the list!" );
    }

    DBG_ERROR( "ImplRegion::InsertPoint point not inserted!" );

    // reinitialize pointer (should never be reached!)
    mpLastCheckedBand = mpFirstBand;

    return FALSE;
}

// -----------------------------------------------------------------------
//
// search for appropriate places for the new bands

void ImplRegion::InsertBands( long nTop, long nBottom )
{
    // region empty? -> set rectagle as first entry!
    if ( !mpFirstBand )
    {
        // add band with boundaries of the rectangle
        mpFirstBand = new ImplRegionBand( nTop, nBottom );
        return;
    }

    // find/insert bands for the boundaries of the rectangle
    BOOL bTopBoundaryInserted = FALSE;
    BOOL bTop2BoundaryInserted = FALSE;
    BOOL bBottomBoundaryInserted = FALSE;

    // special case: top boundary is above the first band
    ImplRegionBand* pNewBand;
    if ( nTop < mpFirstBand->mnYTop )
    {
        // create new band above the first in the list
        pNewBand = new ImplRegionBand( nTop, mpFirstBand->mnYTop );
        if ( nBottom < mpFirstBand->mnYTop )
            pNewBand->mnYBottom = nBottom;

        // insert band into the list
        pNewBand->mpNextBand = mpFirstBand;
        mpFirstBand = pNewBand;

        bTopBoundaryInserted = TRUE;
    }

    // insert band(s) into the list
    ImplRegionBand* pBand = mpFirstBand;
    while ( pBand )
    {
        // Insert Bands if possible
        if ( !bTopBoundaryInserted )
            bTopBoundaryInserted = InsertSingleBand( pBand, nTop - 1 );

        if ( !bTop2BoundaryInserted )
            bTop2BoundaryInserted = InsertSingleBand( pBand, nTop );

        if ( !bBottomBoundaryInserted && (nTop != nBottom) )
            bBottomBoundaryInserted = InsertSingleBand( pBand, nBottom );

        // both boundaries inserted? -> nothing more to do
        if ( bTopBoundaryInserted && bTop2BoundaryInserted && bBottomBoundaryInserted )
            break;

        // insert bands between two bands if neccessary
        if ( pBand->mpNextBand )
        {
            if ( (pBand->mnYBottom + 1) < pBand->mpNextBand->mnYTop )
            {
                // copy band with list and set new boundary
                pNewBand = new ImplRegionBand( pBand->mnYBottom+1,
                                               pBand->mpNextBand->mnYTop-1 );

                // insert band into the list
                pNewBand->mpNextBand = pBand->mpNextBand;
                pBand->mpNextBand = pNewBand;
            }
        }

        pBand = pBand->mpNextBand;
    }
}

// -----------------------------------------------------------------------
//
// create new band and insert it into the list

BOOL ImplRegion::InsertSingleBand( ImplRegionBand* pBand,
                                   long nYBandPosition )
{
    // boundary already included in band with height 1? -> nothing to do!
    if ( (pBand->mnYTop == pBand->mnYBottom) &&
         (nYBandPosition == pBand->mnYTop) )
        return TRUE;

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

        return TRUE;
    }

    // top of new rectangle within the current band? -> insert new band and copy data
    if ( (nYBandPosition > pBand->mnYTop) &&
         (nYBandPosition < pBand->mnYBottom) )
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

        return TRUE;
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
            return TRUE;
        }

        if ( nYBandPosition > pBand->mnYBottom )
        {
            // create new band
            pNewBand = new ImplRegionBand( pBand->mnYBottom + 1, nYBandPosition );

            // append band to the list
            pBand->mpNextBand = pNewBand;
            return TRUE;
        }
    }

    return FALSE;
}

// ------------------------------------------------------------------------

void ImplRegion::Union( long nLeft, long nTop, long nRight, long nBottom )
{
    DBG_ASSERT( nLeft <= nRight, "ImplRegion::Union() - nLeft > nRight" );
    DBG_ASSERT( nTop <= nBottom, "ImplRegion::Union() - nTop > nBottom" );

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
                        DBG_ERROR( "ImplRegion::Union() - Bands not sorted!" );
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

// -----------------------------------------------------------------------

void ImplRegion::Exclude( long nLeft, long nTop, long nRight, long nBottom )
{
    DBG_ASSERT( nLeft <= nRight, "ImplRegion::Exclude() - nLeft > nRight" );
    DBG_ASSERT( nTop <= nBottom, "ImplRegion::Exclude() - nTop > nBottom" );

    // process exclude
    ImplRegionBand* pBand = mpFirstBand;
    while ( pBand )
    {
        if ( pBand->mnYTop >= nTop )
        {
            if ( pBand->mnYBottom <= nBottom )
                pBand->Exclude( nLeft, nRight );
            else
            {
#ifdef DBG_UTIL
                long nCurY = pBand->mnYBottom;
                pBand = pBand->mpNextBand;
                while ( pBand )
                {
                    if ( (pBand->mnYTop < nCurY) || (pBand->mnYBottom < nCurY) )
                    {
                        DBG_ERROR( "ImplRegion::Exclude() - Bands not sorted!" );
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

// -----------------------------------------------------------------------

void ImplRegion::XOr( long nLeft, long nTop, long nRight, long nBottom )
{
    DBG_ASSERT( nLeft <= nRight, "ImplRegion::Exclude() - nLeft > nRight" );
    DBG_ASSERT( nTop <= nBottom, "ImplRegion::Exclude() - nTop > nBottom" );

    // process xor
    ImplRegionBand* pBand = mpFirstBand;
    while ( pBand )
    {
        if ( pBand->mnYTop >= nTop )
        {
            if ( pBand->mnYBottom <= nBottom )
                pBand->XOr( nLeft, nRight );
            else
            {
#ifdef DBG_UTIL
                long nCurY = pBand->mnYBottom;
                pBand = pBand->mpNextBand;
                while ( pBand )
                {
                    if ( (pBand->mnYTop < nCurY) || (pBand->mnYBottom < nCurY) )
                    {
                        DBG_ERROR( "ImplRegion::XOr() - Bands not sorted!" );
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

// -----------------------------------------------------------------------
//
// remove empty bands

BOOL ImplRegion::OptimizeBandList()
{
    DBG_ASSERT( (this != &aImplNullRegion) && (this != &aImplEmptyRegion),
                "ImplRegion::OptimizeBandList() - Empty oder NULL-Region" );

    mnRectCount = 0;

    ImplRegionBand* pPrevBand;
    ImplRegionBand* pBand = mpFirstBand;
    while ( pBand )
    {
        const BOOL bBTEqual = pBand->mpNextBand &&
                              (pBand->mnYBottom == pBand->mpNextBand->mnYTop);

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
                    mnRectCount++;
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
        DBG_ASSERT( pBand->mpFirstSep != NULL,
                    "Exiting ImplRegion::OptimizeBandList(): empty band in region!" );

        if ( pBand->mnYBottom < pBand->mnYTop )
            DBG_ERROR( "ImplRegion::OptimizeBandList(): YBottomBoundary < YTopBoundary" );

        if ( pBand->mpNextBand )
        {
            if ( pBand->mnYBottom >= pBand->mpNextBand->mnYTop )
                DBG_ERROR( "ImplRegion::OptimizeBandList(): overlapping bands in region!" );
        }

        pBand = pBand->mpNextBand;
    }
#endif

    return (mnRectCount != 0);
}

// =======================================================================

void Region::ImplCopyData()
{
    mpImplRegion->mnRefCount--;
    mpImplRegion = new ImplRegion( *mpImplRegion );
}

// =======================================================================

Region::Region()
{
    DBG_CTOR( Region, ImplDbgTestRegion );

    mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
}

// -----------------------------------------------------------------------

Region::Region( RegionType eType )
{
    DBG_CTOR( Region, ImplDbgTestRegion );
    DBG_ASSERT( (eType == REGION_NULL) || (eType == REGION_EMPTY),
                "Region( RegionType ) - RegionType != EMPTY/NULL" );

    if ( eType == REGION_NULL )
        mpImplRegion = (ImplRegion*)(&aImplNullRegion);
    else
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
}

// -----------------------------------------------------------------------

Region::Region( const Rectangle& rRect )
{
    DBG_CTOR( Region, ImplDbgTestRegion );

    ImplCreateRectRegion( rRect );
}

// -----------------------------------------------------------------------

Region::Region( const Polygon& rPolygon )
{
    DBG_CTOR( Region, ImplDbgTestRegion );
    DBG_CHKOBJ( &rPolygon, Polygon, NULL );

    ImplCreatePolyPolyRegion( rPolygon );
}

// -----------------------------------------------------------------------

Region::Region( const PolyPolygon& rPolyPoly )
{
    DBG_CTOR( Region, ImplDbgTestRegion );
    DBG_CHKOBJ( &rPolyPoly, PolyPolygon, NULL );

    ImplCreatePolyPolyRegion( rPolyPoly );
}

// -----------------------------------------------------------------------

Region::Region( const Region& rRegion )
{
    DBG_CTOR( Region, ImplDbgTestRegion );
    DBG_CHKOBJ( &rRegion, Region, ImplDbgTestRegion );
    DBG_ASSERT( rRegion.mpImplRegion->mnRefCount < 0xFFFFFFFE, "Region: RefCount overflow" );

    // copy pointer to instance of implementation
    mpImplRegion = rRegion.mpImplRegion;
    if ( mpImplRegion->mnRefCount )
        mpImplRegion->mnRefCount++;
}

// -----------------------------------------------------------------------

Region::~Region()
{
    DBG_DTOR( Region, ImplDbgTestRegion );

    // statische Object haben RefCount von 0
    if ( mpImplRegion->mnRefCount )
    {
        if ( mpImplRegion->mnRefCount > 1 )
            mpImplRegion->mnRefCount--;
        else
            delete mpImplRegion;
    }
}

// -----------------------------------------------------------------------

void Region::ImplCreateRectRegion( const Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
    else
    {
        // get justified rectangle
        long nTop       = Min( rRect.Top(), rRect.Bottom() );
        long nBottom    = Max( rRect.Top(), rRect.Bottom() );
        long nLeft      = Min( rRect.Left(), rRect.Right() );
        long nRight     = Max( rRect.Left(), rRect.Right() );

        // create instance of implementation class
        mpImplRegion = new ImplRegion();

        // add band with boundaries of the rectangle
        mpImplRegion->mpFirstBand = new ImplRegionBand( nTop, nBottom );

        // Set left and right boundaries of the band
        mpImplRegion->mpFirstBand->Union( nLeft, nRight );
        mpImplRegion->mnRectCount = 1;
    }
}

// -----------------------------------------------------------------------

void Region::ImplCreatePolyPolyRegion( const PolyPolygon& rPolyPoly )
{
    const USHORT nPolyCount = rPolyPoly.Count();
    if ( nPolyCount )
    {
        // polypolygon empty? -> empty region
        const Rectangle aRect( rPolyPoly.GetBoundRect() );

        if ( !aRect.IsEmpty() )
        {
            // width OR height == 1 ? => Rectangular region
            if ( (aRect.GetWidth() == 1) || (aRect.GetHeight() == 1) )
                ImplCreateRectRegion( aRect );
            else
                mpImplRegion = new ImplRegion( rPolyPoly );
        }
        else
            mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
    }
    else
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
}

// -----------------------------------------------------------------------

void Region::ImplPolyPolyRegionToBandRegionFunc()
{
    const PolyPolygon aPolyPoly( *mpImplRegion->mpPolyPoly );

    if ( mpImplRegion->mnRefCount > 1 )
        mpImplRegion->mnRefCount--;
    else
        delete mpImplRegion;

    const USHORT nPolyCount = aPolyPoly.Count();
    if ( nPolyCount )
    {
        // polypolygon empty? -> empty region
        const Rectangle aRect( aPolyPoly.GetBoundRect() );

        if ( !aRect.IsEmpty() )
        {
            long nLineID = 0L;

            // initialisation and creation of Bands
            mpImplRegion = new ImplRegion();
            mpImplRegion->CreateBandRange( aRect.Top(), aRect.Bottom() );

            // insert polygons
            for ( USHORT nPoly = 0; nPoly < nPolyCount; nPoly++ )
            {
                // get reference to current polygon
                const Polygon&  aPoly = aPolyPoly.GetObject( nPoly );
                const USHORT    nSize = aPoly.GetSize();

                // not enough points ( <= 2 )? -> nothing to do!
                if ( nSize <= 2 )
                    continue;

                // band the polygon
                for ( USHORT nPoint = 1; nPoint < nSize; nPoint++ )
                    mpImplRegion->InsertLine( aPoly.GetPoint(nPoint-1), aPoly.GetPoint(nPoint), nLineID++ );

                // close polygon with line from first point to last point, if neccesary
                const Point rLastPoint = aPoly.GetPoint(nSize-1);
                const Point rFirstPoint = aPoly.GetPoint(0);
                if ( rLastPoint != rFirstPoint )
                    mpImplRegion->InsertLine( rLastPoint, rFirstPoint, nLineID++ );
            }

            // process bands with lines
            ImplRegionBand* pRegionBand = mpImplRegion->mpFirstBand;
            while ( pRegionBand )
            {
                // generate separations from the lines and process union
                pRegionBand->ProcessPoints();
                pRegionBand = pRegionBand->mpNextBand;
            }

            // cleanup
            if ( !mpImplRegion->OptimizeBandList() )
            {
                delete mpImplRegion;
                mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
            }
        }
        else
            mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
    }
    else
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
}

// -----------------------------------------------------------------------

void Region::Move( long nHorzMove, long nVertMove )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // no region data? -> nothing to do
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return;

    // no own instance data? -> make own copy!
    if ( mpImplRegion->mnRefCount > 1 )
        ImplCopyData();

    if ( mpImplRegion->mpPolyPoly )
        mpImplRegion->mpPolyPoly->Move( nHorzMove, nVertMove );
    else
    {
        ImplRegionBand* pBand = mpImplRegion->mpFirstBand;
        while ( pBand )
        {
            // process the vertical move
            if ( nVertMove != 0)
            {
                pBand->mnYTop = pBand->mnYTop + nVertMove;
                pBand->mnYBottom = pBand->mnYBottom + nVertMove;
            }

            // process the horizontal move
            if ( nHorzMove != 0)
                pBand->MoveX( nHorzMove );

            pBand = pBand->mpNextBand;
        }
    }
}

// -----------------------------------------------------------------------

void Region::Scale( double fScaleX, double fScaleY )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // no region data? -> nothing to do
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return;

    // no own instance data? -> make own copy!
    if ( mpImplRegion->mnRefCount > 1 )
        ImplCopyData();

    if ( mpImplRegion->mpPolyPoly )
        mpImplRegion->mpPolyPoly->Scale( fScaleX, fScaleY );
    else
    {
        ImplRegionBand* pBand = mpImplRegion->mpFirstBand;
        while ( pBand )
        {
            // process the vertical move
            if ( fScaleY != 0.0 )
            {
                pBand->mnYTop = FRound( pBand->mnYTop * fScaleY );
                pBand->mnYBottom = FRound( pBand->mnYBottom * fScaleY );
            }

            // process the horizontal move
            if ( fScaleX != 0.0 )
                pBand->ScaleX( fScaleX );

            pBand = pBand->mpNextBand;
        }
    }
}

// -----------------------------------------------------------------------

BOOL Region::Union( const Rectangle& rRect )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // is rectangle empty? -> nothing to do
    if ( rRect.IsEmpty() )
        return TRUE;

    ImplPolyPolyRegionToBandRegion();

    // no instance data? -> create!
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        mpImplRegion = new ImplRegion();

    // no own instance data? -> make own copy!
    if ( mpImplRegion->mnRefCount > 1 )
        ImplCopyData();

    // get justified rectangle
    long nLeft      = Min( rRect.Left(), rRect.Right() );
    long nTop       = Min( rRect.Top(), rRect.Bottom() );
    long nRight     = Max( rRect.Left(), rRect.Right() );
    long nBottom    = Max( rRect.Top(), rRect.Bottom() );

    // insert bands if the boundaries are not allready in the list
    mpImplRegion->InsertBands( nTop, nBottom );

    // process union
    mpImplRegion->Union( nLeft, nTop, nRight, nBottom );

    // cleanup
    if ( !mpImplRegion->OptimizeBandList() )
    {
        delete mpImplRegion;
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Region::Intersect( const Rectangle& rRect )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // is rectangle empty? -> nothing to do
    if ( rRect.IsEmpty() )
    {
        // statische Object haben RefCount von 0
        if ( mpImplRegion->mnRefCount )
        {
            if ( mpImplRegion->mnRefCount > 1 )
                mpImplRegion->mnRefCount--;
            else
                delete mpImplRegion;
        }
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
        return TRUE;
    }

    ImplPolyPolyRegionToBandRegion();

    // is region empty? -> nothing to do!
    if ( mpImplRegion == &aImplEmptyRegion )
        return TRUE;

    // get justified rectangle
    long nLeft      = Min( rRect.Left(), rRect.Right() );
    long nTop       = Min( rRect.Top(), rRect.Bottom() );
    long nRight     = Max( rRect.Left(), rRect.Right() );
    long nBottom    = Max( rRect.Top(), rRect.Bottom() );

    // is own region NULL-region? -> copy data!
    if ( mpImplRegion == &aImplNullRegion )
    {
        // create instance of implementation class
        mpImplRegion = new ImplRegion();

        // add band with boundaries of the rectangle
        mpImplRegion->mpFirstBand = new ImplRegionBand( nTop, nBottom );

        // Set left and right boundaries of the band
        mpImplRegion->mpFirstBand->Union( nLeft, nRight );
        mpImplRegion->mnRectCount = 1;

        return TRUE;
    }

    // no own instance data? -> make own copy!
    if ( mpImplRegion->mnRefCount > 1 )
        ImplCopyData();

    // insert bands if the boundaries are not allready in the list
    mpImplRegion->InsertBands( nTop, nBottom );

    // process intersections
    ImplRegionBand* pPrevBand;
    ImplRegionBand* pBand = mpImplRegion->mpFirstBand;
    while ( pBand )
    {
        // band within intersection boundary? -> process. otherwise remove
        if ( (pBand->mnYTop >= nTop) &&
             (pBand->mnYBottom <= nBottom) )
        {
            // process intersection
            pBand->Intersect( nLeft, nRight );

            pPrevBand = pBand;
            pBand = pBand->mpNextBand;
        }
        else
        {
            ImplRegionBand* pOldBand = pBand;
            if ( pBand == mpImplRegion->mpFirstBand )
                mpImplRegion->mpFirstBand = pBand->mpNextBand;
            else
                pPrevBand->mpNextBand = pBand->mpNextBand;
            pBand = pBand->mpNextBand;
            delete pOldBand;
        }
    }

    // cleanup
    if ( !mpImplRegion->OptimizeBandList() )
    {
        delete mpImplRegion;
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Region::Exclude( const Rectangle& rRect )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // is rectangle empty? -> nothing to do
    if ( rRect.IsEmpty() )
        return TRUE;

    ImplPolyPolyRegionToBandRegion();

    // no instance data? -> create!
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return TRUE;

    // no own instance data? -> make own copy!
    if ( mpImplRegion->mnRefCount > 1 )
        ImplCopyData();

    // get justified rectangle
    long nLeft      = Min( rRect.Left(), rRect.Right() );
    long nTop       = Min( rRect.Top(), rRect.Bottom() );
    long nRight     = Max( rRect.Left(), rRect.Right() );
    long nBottom    = Max( rRect.Top(), rRect.Bottom() );

    // insert bands if the boundaries are not allready in the list
    mpImplRegion->InsertBands( nTop, nBottom );

    // process exclude
    mpImplRegion->Exclude( nLeft, nTop, nRight, nBottom );

    // cleanup
    if ( !mpImplRegion->OptimizeBandList() )
    {
        delete mpImplRegion;
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Region::XOr( const Rectangle& rRect )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // is rectangle empty? -> nothing to do
    if ( rRect.IsEmpty() )
        return TRUE;

    ImplPolyPolyRegionToBandRegion();

    // no instance data? -> create!
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        mpImplRegion = new ImplRegion();

    // no own instance data? -> make own copy!
    if ( mpImplRegion->mnRefCount > 1 )
        ImplCopyData();

    // get justified rectangle
    long nLeft      = Min( rRect.Left(), rRect.Right() );
    long nTop       = Min( rRect.Top(), rRect.Bottom() );
    long nRight     = Max( rRect.Left(), rRect.Right() );
    long nBottom    = Max( rRect.Top(), rRect.Bottom() );

    // insert bands if the boundaries are not allready in the list
    mpImplRegion->InsertBands( nTop, nBottom );

    // process xor
    mpImplRegion->XOr( nLeft, nTop, nRight, nBottom );

    // cleanup
    if ( !mpImplRegion->OptimizeBandList() )
    {
        delete mpImplRegion;
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Region::Union( const Region& rRegion )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ImplPolyPolyRegionToBandRegion();
    ((Region*)&rRegion)->ImplPolyPolyRegionToBandRegion();

    // is region empty or null? -> nothing to do
    if ( (rRegion.mpImplRegion == &aImplEmptyRegion) || (rRegion.mpImplRegion == &aImplNullRegion) )
        return TRUE;

    // no instance data? -> create!
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        mpImplRegion = new ImplRegion();

    // no own instance data? -> make own copy!
    if ( mpImplRegion->mnRefCount > 1 )
        ImplCopyData();

    // Alle Rechtecke aus der uebergebenen Region auf diese Region anwenden
    ImplRegionBand* pBand = rRegion.mpImplRegion->mpFirstBand;
    while ( pBand )
    {
        // insert bands if the boundaries are not allready in the list
        mpImplRegion->InsertBands( pBand->mnYTop, pBand->mnYBottom );

        // process all elements of the list
        ImplRegionBandSep* pSep = pBand->mpFirstSep;
        while ( pSep )
        {
            mpImplRegion->Union( pSep->mnXLeft, pBand->mnYTop,
                                 pSep->mnXRight, pBand->mnYBottom );
            pSep = pSep->mpNextSep;
        }

        pBand = pBand->mpNextBand;
    }

    // cleanup
    if ( !mpImplRegion->OptimizeBandList() )
    {
        delete mpImplRegion;
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Region::Intersect( const Region& rRegion )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // same instance data? -> nothing to do!
    if ( mpImplRegion == rRegion.mpImplRegion )
        return TRUE;

    ImplPolyPolyRegionToBandRegion();
    ((Region*)&rRegion)->ImplPolyPolyRegionToBandRegion();

    if ( mpImplRegion == &aImplEmptyRegion )
        return TRUE;

    // is region null? -> nothing to do
    if ( rRegion.mpImplRegion == &aImplNullRegion )
        return TRUE;

    // is rectangle empty? -> nothing to do
    if ( rRegion.mpImplRegion == &aImplEmptyRegion )
    {
        // statische Object haben RefCount von 0
        if ( mpImplRegion->mnRefCount )
        {
            if ( mpImplRegion->mnRefCount > 1 )
                mpImplRegion->mnRefCount--;
            else
                delete mpImplRegion;
        }
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
        return TRUE;
    }

    // is own region NULL-region? -> copy data!
    if ( mpImplRegion == &aImplNullRegion)
    {
        mpImplRegion = rRegion.mpImplRegion;
        rRegion.mpImplRegion->mnRefCount++;
        return TRUE;
    }

    // Wenn wir weniger Rechtecke haben, drehen wir den Intersect-Aufruf um
    if ( mpImplRegion->mnRectCount+2 < rRegion.mpImplRegion->mnRectCount )
    {
        Region aTempRegion = rRegion;
        aTempRegion.Intersect( *this );
        *this = aTempRegion;
    }
    else
    {
        // no own instance data? -> make own copy!
        if ( mpImplRegion->mnRefCount > 1 )
            ImplCopyData();

        // mark all bands as untouched
        ImplRegionBand* pBand = mpImplRegion->mpFirstBand;
        while ( pBand )
        {
            pBand->mbTouched = FALSE;
            pBand = pBand->mpNextBand;
        }

        pBand = rRegion.mpImplRegion->mpFirstBand;
        while ( pBand )
        {
            // insert bands if the boundaries are not allready in the list
            mpImplRegion->InsertBands( pBand->mnYTop, pBand->mnYBottom );

            // process all elements of the list
            ImplRegionBandSep* pSep = pBand->mpFirstSep;
            while ( pSep )
            {
                // left boundary?
                if ( pSep == pBand->mpFirstSep )
                {
                    // process intersection and do not remove untouched bands
                    mpImplRegion->Exclude( LONG_MIN+1, pBand->mnYTop,
                                           pSep->mnXLeft-1, pBand->mnYBottom );
                }

                // right boundary?
                if ( pSep->mpNextSep == NULL )
                {
                    // process intersection and do not remove untouched bands
                    mpImplRegion->Exclude( pSep->mnXRight+1, pBand->mnYTop,
                                           LONG_MAX-1, pBand->mnYBottom );
                }
                else
                {
                    // process intersection and do not remove untouched bands
                    mpImplRegion->Exclude( pSep->mnXRight+1, pBand->mnYTop,
                                           pSep->mpNextSep->mnXLeft-1, pBand->mnYBottom );
                }

                pSep = pSep->mpNextSep;
            }

            pBand = pBand->mpNextBand;
        }

        // remove all untouched bands if bands allready left
        ImplRegionBand* pPrevBand;
        pBand = mpImplRegion->mpFirstBand;
        while ( pBand )
        {
            if ( !pBand->mbTouched )
            {
                // save pointer
                ImplRegionBand* pOldBand = pBand;

                // previous element of the list
                if ( pBand == mpImplRegion->mpFirstBand )
                    mpImplRegion->mpFirstBand = pBand->mpNextBand;
                else
                    pPrevBand->mpNextBand = pBand->mpNextBand;

                pBand = pBand->mpNextBand;
                delete pOldBand;
            }
            else
            {
                pPrevBand = pBand;
                pBand = pBand->mpNextBand;
            }
        }

        // cleanup
        if ( !mpImplRegion->OptimizeBandList() )
        {
            delete mpImplRegion;
            mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
        }
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Region::Exclude( const Region& rRegion )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ImplPolyPolyRegionToBandRegion();
    ((Region*)&rRegion)->ImplPolyPolyRegionToBandRegion();

    // is region empty or null? -> nothing to do
    if ( (rRegion.mpImplRegion == &aImplEmptyRegion) || (rRegion.mpImplRegion == &aImplNullRegion) )
        return TRUE;

    // no instance data? -> nothing to do
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return TRUE;

    // no own instance data? -> make own copy!
    if ( mpImplRegion->mnRefCount > 1 )
        ImplCopyData();

    // Alle Rechtecke aus der uebergebenen Region auf diese Region anwenden
    ImplRegionBand* pBand = rRegion.mpImplRegion->mpFirstBand;
    while ( pBand )
    {
        // insert bands if the boundaries are not allready in the list
        mpImplRegion->InsertBands( pBand->mnYTop, pBand->mnYBottom );

        // process all elements of the list
        ImplRegionBandSep* pSep = pBand->mpFirstSep;
        while ( pSep )
        {
            mpImplRegion->Exclude( pSep->mnXLeft, pBand->mnYTop,
                                   pSep->mnXRight, pBand->mnYBottom );
            pSep = pSep->mpNextSep;
        }

        // Wir optimieren schon in der Schleife, da wir davon
        // ausgehen, das wir insgesammt weniger Baender ueberpruefen
        // muessen
        if ( !mpImplRegion->OptimizeBandList() )
        {
            delete mpImplRegion;
            mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
            break;
        }

        pBand = pBand->mpNextBand;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Region::XOr( const Region& rRegion )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ImplPolyPolyRegionToBandRegion();
    ((Region*)&rRegion)->ImplPolyPolyRegionToBandRegion();

    // is region empty or null? -> nothing to do
    if ( (rRegion.mpImplRegion == &aImplEmptyRegion) || (rRegion.mpImplRegion == &aImplNullRegion) )
        return TRUE;

    // no instance data? -> nothing to do
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return TRUE;

    // no own instance data? -> make own copy!
    if ( mpImplRegion->mnRefCount > 1 )
        ImplCopyData();

    // Alle Rechtecke aus der uebergebenen Region auf diese Region anwenden
    ImplRegionBand* pBand = rRegion.mpImplRegion->mpFirstBand;
    while ( pBand )
    {
        // insert bands if the boundaries are not allready in the list
        mpImplRegion->InsertBands( pBand->mnYTop, pBand->mnYBottom );

        // process all elements of the list
        ImplRegionBandSep* pSep = pBand->mpFirstSep;
        while ( pSep )
        {
            mpImplRegion->XOr( pSep->mnXLeft, pBand->mnYTop,
                               pSep->mnXRight, pBand->mnYBottom );
            pSep = pSep->mpNextSep;
        }

        pBand = pBand->mpNextBand;
    }

    // cleanup
    if ( !mpImplRegion->OptimizeBandList() )
    {
        delete mpImplRegion;
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
    }

    return TRUE;
}

// -----------------------------------------------------------------------

Rectangle Region::GetBoundRect() const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    Rectangle aRect;

    // no internal data? -> region is empty!
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return aRect;

    // PolyPolygon data im Imp structure?
    if ( mpImplRegion->mpPolyPoly )
        return mpImplRegion->mpPolyPoly->GetBoundRect();

    // no band in the list? -> region is empty!
    if ( !mpImplRegion->mpFirstBand )
        return aRect;

    // get the boundaries of the first band
    long nYTop    = mpImplRegion->mpFirstBand->mnYTop;
    long nYBottom = mpImplRegion->mpFirstBand->mnYBottom;
    long nXLeft   = mpImplRegion->mpFirstBand->GetXLeftBoundary();
    long nXRight  = mpImplRegion->mpFirstBand->GetXRightBoundary();

    // look in the band list (don't test first band again!)
    ImplRegionBand* pBand = mpImplRegion->mpFirstBand->mpNextBand;
    while ( pBand )
    {
        nYBottom    = pBand->mnYBottom;
        nXLeft      = Min( nXLeft, pBand->GetXLeftBoundary() );
        nXRight     = Max( nXRight, pBand->GetXRightBoundary() );

        pBand = pBand->mpNextBand;
    }

    // set rectangle
    aRect = Rectangle( nXLeft, nYTop, nXRight, nYBottom );
    return aRect;
}

// -----------------------------------------------------------------------

BOOL Region::ImplGetFirstRect( ImplRegionInfo& rImplRegionInfo,
                               long& rX, long& rY,
                               long& rWidth, long& rHeight ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ((Region*)this)->ImplPolyPolyRegionToBandRegion();

    // no internal data? -> region is empty!
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return FALSE;

    // no band in the list? -> region is empty!
    if ( mpImplRegion->mpFirstBand == NULL )
        return FALSE;

    // initialise pointer for first access
    ImplRegionBand*     pCurrRectBand = mpImplRegion->mpFirstBand;
    ImplRegionBandSep*  pCurrRectBandSep = pCurrRectBand->mpFirstSep;

    DBG_ASSERT( pCurrRectBandSep != NULL, "Erstes Band wurde nicht optimiert." );
    if ( !pCurrRectBandSep )
        return FALSE;

    // get boundaries of current rectangle
    rX      = pCurrRectBandSep->mnXLeft;
    rY      = pCurrRectBand->mnYTop;
    rWidth  = pCurrRectBandSep->mnXRight - pCurrRectBandSep->mnXLeft + 1;
    rHeight = pCurrRectBand->mnYBottom - pCurrRectBand->mnYTop + 1;

    // save pointers
    rImplRegionInfo.mpVoidCurrRectBand = (void*)pCurrRectBand;
    rImplRegionInfo.mpVoidCurrRectBandSep = (void*)pCurrRectBandSep;

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Region::ImplGetNextRect( ImplRegionInfo& rImplRegionInfo,
                              long& rX, long& rY,
                              long& rWidth, long& rHeight ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // no internal data? -> region is empty!
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return FALSE;

    // get last pointers
    ImplRegionBand*     pCurrRectBand = (ImplRegionBand*)rImplRegionInfo.mpVoidCurrRectBand;
    ImplRegionBandSep*  pCurrRectBandSep = (ImplRegionBandSep*)rImplRegionInfo.mpVoidCurrRectBandSep;

    // get next separation from current band
    pCurrRectBandSep = pCurrRectBandSep->mpNextSep;

    // no separation found? -> go to next band!
    if ( !pCurrRectBandSep )
    {
        // get next band
        pCurrRectBand = pCurrRectBand->mpNextBand;

        // no band found? -> not further rectangles!
        if( !pCurrRectBand )
            return FALSE;

        // get first separation in current band
        pCurrRectBandSep = pCurrRectBand->mpFirstSep;
    }

    // get boundaries of current rectangle
    rX      = pCurrRectBandSep->mnXLeft;
    rY      = pCurrRectBand->mnYTop;
    rWidth  = pCurrRectBandSep->mnXRight - pCurrRectBandSep->mnXLeft + 1;
    rHeight = pCurrRectBand->mnYBottom - pCurrRectBand->mnYTop + 1;

    // save new pointers
    rImplRegionInfo.mpVoidCurrRectBand = (void*)pCurrRectBand;
    rImplRegionInfo.mpVoidCurrRectBandSep = (void*)pCurrRectBandSep;

    return TRUE;
}

// -----------------------------------------------------------------------

RegionType Region::GetType() const
{
    if ( mpImplRegion == &aImplEmptyRegion )
        return REGION_EMPTY;
    else if ( mpImplRegion == &aImplNullRegion )
        return REGION_NULL;
    else if ( mpImplRegion->mnRectCount == 1 )
        return REGION_RECTANGLE;
    else
        return REGION_COMPLEX;
}

// -----------------------------------------------------------------------

BOOL Region::IsInside( const Point& rPoint ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // PolyPolygon data im Imp structure?
    ((Region*)this)->ImplPolyPolyRegionToBandRegion();
/*
    if ( mpImplRegion->mpPolyPoly )
        return mpImplRegion->mpPolyPoly->IsInside( rPoint );
*/

    // no instance data? -> not inside
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return FALSE;

    // search band list
    ImplRegionBand* pBand = mpImplRegion->mpFirstBand;
    while ( pBand )
    {
        // is point within band?
        if ( (pBand->mnYTop <= rPoint.Y()) &&
             (pBand->mnYBottom >= rPoint.Y()) )
        {
            // is point within separation of the band?
            if ( pBand->IsInside( rPoint.X() ) )
                return TRUE;
            else
                return FALSE;
        }

        pBand = pBand->mpNextBand;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Region::IsInside( const Rectangle& rRect ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // is rectangle empty? -> not inside
    if ( rRect.IsEmpty() )
        return FALSE;

    // no instance data? -> not inside
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return FALSE;

    // create region from rectangle and intersect own region
    Region aRegion = rRect;
    aRegion.Exclude( *this );

    // rectangle is inside if exclusion is empty
    return aRegion.IsEmpty();
}

// -----------------------------------------------------------------------

BOOL Region::IsOver( const Rectangle& rRect ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return FALSE;

    // Can we optimize this ??? - is used in StarDraw for brushes pointers
    // Why we have no IsOver for Regions ???
    // create region from rectangle and intersect own region
    Region aRegion = rRect;
    aRegion.Intersect( *this );

    // rectangle is over if include is not empty
    return !aRegion.IsEmpty();
}

// -----------------------------------------------------------------------

void Region::SetNull()
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // statische Object haben RefCount von 0
    if ( mpImplRegion->mnRefCount )
    {
        if ( mpImplRegion->mnRefCount > 1 )
            mpImplRegion->mnRefCount--;
        else
            delete mpImplRegion;
    }

    // set new type
    mpImplRegion = (ImplRegion*)(&aImplNullRegion);
}

// -----------------------------------------------------------------------

void Region::SetEmpty()
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // statische Object haben RefCount von 0
    if ( mpImplRegion->mnRefCount )
    {
        if ( mpImplRegion->mnRefCount > 1 )
            mpImplRegion->mnRefCount--;
        else
            delete mpImplRegion;
    }

    // set new type
    mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
}

// -----------------------------------------------------------------------

Region& Region::operator=( const Region& rRegion )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );
    DBG_CHKOBJ( &rRegion, Region, ImplDbgTestRegion );
    DBG_ASSERT( rRegion.mpImplRegion->mnRefCount < 0xFFFFFFFE, "Region: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    // RefCount == 0 fuer statische Objekte
    if ( rRegion.mpImplRegion->mnRefCount )
        rRegion.mpImplRegion->mnRefCount++;

    // statische Object haben RefCount von 0
    if ( mpImplRegion->mnRefCount )
    {
        if ( mpImplRegion->mnRefCount > 1 )
            mpImplRegion->mnRefCount--;
        else
            delete mpImplRegion;
    }

    mpImplRegion = rRegion.mpImplRegion;
    return *this;
}

// -----------------------------------------------------------------------

Region& Region::operator=( const Rectangle& rRect )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // statische Object haben RefCount von 0
    if ( mpImplRegion->mnRefCount )
    {
        if ( mpImplRegion->mnRefCount > 1 )
            mpImplRegion->mnRefCount--;
        else
            delete mpImplRegion;
    }

    ImplCreateRectRegion( rRect );
    return *this;
}

// -----------------------------------------------------------------------

BOOL Region::operator==( const Region& rRegion ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );
    DBG_CHKOBJ( &rRegion, Region, ImplDbgTestRegion );

    // reference to same object? -> equal!
    if ( mpImplRegion == rRegion.mpImplRegion )
        return TRUE;

    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return FALSE;

    if ( (rRegion.mpImplRegion == &aImplEmptyRegion) || (rRegion.mpImplRegion == &aImplNullRegion) )
        return FALSE;

    if ( rRegion.mpImplRegion->mpPolyPoly && mpImplRegion->mpPolyPoly )
        return *rRegion.mpImplRegion->mpPolyPoly == *mpImplRegion->mpPolyPoly;
    else
    {
        ((Region*)this)->ImplPolyPolyRegionToBandRegion();
        ((Region*)&rRegion)->ImplPolyPolyRegionToBandRegion();

        // Eine der beiden Regions kann jetzt Empty sein
        if ( mpImplRegion == rRegion.mpImplRegion )
            return TRUE;

        if ( mpImplRegion == &aImplEmptyRegion )
            return FALSE;

        if ( rRegion.mpImplRegion == &aImplEmptyRegion )
            return FALSE;
    }

    // initialise pointers
    ImplRegionBand*      pOwnRectBand = mpImplRegion->mpFirstBand;
    ImplRegionBandSep*   pOwnRectBandSep = pOwnRectBand->mpFirstSep;
    ImplRegionBand*      pSecondRectBand = rRegion.mpImplRegion->mpFirstBand;
    ImplRegionBandSep*   pSecondRectBandSep = pSecondRectBand->mpFirstSep;
    while ( pOwnRectBandSep && pSecondRectBandSep )
    {
        // get boundaries of current rectangle
        long nOwnXLeft = pOwnRectBandSep->mnXLeft;
        long nSecondXLeft = pSecondRectBandSep->mnXLeft;
        if ( nOwnXLeft != nSecondXLeft )
            return FALSE;

        long nOwnYTop = pOwnRectBand->mnYTop;
        long nSecondYTop = pSecondRectBand->mnYTop;
        if ( nOwnYTop != nSecondYTop )
            return FALSE;

        long nOwnXRight = pOwnRectBandSep->mnXRight;
        long nSecondXRight = pSecondRectBandSep->mnXRight;
        if ( nOwnXRight != nSecondXRight )
            return FALSE;

        long nOwnYBottom = pOwnRectBand->mnYBottom;
        long nSecondYBottom = pSecondRectBand->mnYBottom;
        if ( nOwnYBottom != nSecondYBottom )
            return FALSE;

        // get next separation from current band
        pOwnRectBandSep = pOwnRectBandSep->mpNextSep;

        // no separation found? -> go to next band!
        if ( !pOwnRectBandSep )
        {
            // get next band
            pOwnRectBand = pOwnRectBand->mpNextBand;

            // get first separation in current band
            if( pOwnRectBand )
                pOwnRectBandSep = pOwnRectBand->mpFirstSep;
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
                pSecondRectBandSep = pSecondRectBand->mpFirstSep;
        }

        if ( pOwnRectBandSep && !pSecondRectBandSep )
            return FALSE;

        if ( !pOwnRectBandSep && pSecondRectBandSep )
            return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

enum StreamEntryType { STREAMENTRY_BANDHEADER, STREAMENTRY_SEPARATION, STREAMENTRY_END };

SvStream& operator>>( SvStream& rIStrm, Region& rRegion )
{
    DBG_CHKOBJ( &rRegion, Region, ImplDbgTestRegion );

    VersionCompat   aCompat( rIStrm, STREAM_READ );
    UINT16          nVersion;
    UINT16          nTmp16;

    // statische Object haben RefCount von 0
    if ( rRegion.mpImplRegion->mnRefCount )
    {
        if ( rRegion.mpImplRegion->mnRefCount > 1 )
            rRegion.mpImplRegion->mnRefCount--;
        else
            delete rRegion.mpImplRegion;
    }

    // get version of streamed region
    rIStrm >> nVersion;

    // get type of region
    rIStrm >> nTmp16;

    RegionType meStreamedType = (RegionType)nTmp16;

    switch( meStreamedType )
    {
        case REGION_NULL:
            rRegion.mpImplRegion = (ImplRegion*)&aImplNullRegion;
            break;

        case REGION_EMPTY:
            rRegion.mpImplRegion = (ImplRegion*)&aImplEmptyRegion;
            break;

        default:
            // create instance of implementation class
            rRegion.mpImplRegion = new ImplRegion();

            // get header from first element
            rIStrm >> nTmp16;

            // get all bands
            rRegion.mpImplRegion->mnRectCount = 0;
            ImplRegionBand* pCurrBand = NULL;
            while ( (StreamEntryType)nTmp16 != STREAMENTRY_END )
            {
                // insert new band or new separation?
                if ( (StreamEntryType)nTmp16 == STREAMENTRY_BANDHEADER )
                {
                    long nYTop;
                    long nYBottom;

                    rIStrm >> nYTop;
                    rIStrm >> nYBottom;

                    // create band
                    ImplRegionBand* pNewBand = new ImplRegionBand( nYTop, nYBottom );

                    // first element? -> set as first into the list
                    if ( !pCurrBand )
                        rRegion.mpImplRegion->mpFirstBand = pNewBand;
                    else
                        pCurrBand->mpNextBand = pNewBand;

                    // save pointer for next creation
                    pCurrBand = pNewBand;
                }
                else
                {
                    long nXLeft;
                    long nXRight;

                    rIStrm >> nXLeft;
                    rIStrm >> nXRight;

                    // add separation
                    if ( pCurrBand )
                    {
                        pCurrBand->Union( nXLeft, nXRight );
                        rRegion.mpImplRegion->mnRectCount++;
                    }
                }

                // get next header
                rIStrm >> nTmp16;
            }
            break;
    }

    return rIStrm;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStrm, const Region& rRegion )
{
    DBG_CHKOBJ( &rRegion, Region, ImplDbgTestRegion );

    VersionCompat   aCompat( rOStrm, STREAM_WRITE );
    UINT16          nVersion = 1;

    ((Region*)&rRegion)->ImplPolyPolyRegionToBandRegion();

    // put version
    rOStrm << nVersion;

    // put type
    rOStrm << (UINT16)rRegion.GetType();

    // put all bands if not null or empty
    if ( (rRegion.mpImplRegion != &aImplEmptyRegion) && (rRegion.mpImplRegion != &aImplNullRegion) )
    {
        ImplRegionBand* pBand = rRegion.mpImplRegion->mpFirstBand;
        while ( pBand )
        {
            // put boundaries
            rOStrm << (UINT16) STREAMENTRY_BANDHEADER;
            rOStrm << pBand->mnYTop;
            rOStrm << pBand->mnYBottom;

            // put separations of current band
            ImplRegionBandSep* pSep = pBand->mpFirstSep;
            while ( pSep )
            {
                // put separation
                rOStrm << (UINT16) STREAMENTRY_SEPARATION;
                rOStrm << pSep->mnXLeft;
                rOStrm << pSep->mnXRight;

                // next separation from current band
                pSep = pSep->mpNextSep;
            }

            pBand = pBand->mpNextBand;
        }

        // put endmarker
        rOStrm << (UINT16) STREAMENTRY_END;
    }

    return rOStrm;
}

// -----------------------------------------------------------------------

RegionOverlapType Region::GetOverlapType( const Rectangle& rRect ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // is rectangle empty? -> not inside
    if ( rRect.IsEmpty() )
        return REGION_OUTSIDE;

    ((Region*)this)->ImplPolyPolyRegionToBandRegion();

    // no instance data? -> not inside
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return REGION_OUTSIDE;

    // resolve pointer
    ImplRegionBand*     pBand   = mpImplRegion->mpFirstBand;
    ImplRegionBandSep*  pSep    = pBand->mpFirstSep;

    // complex region? don't check for now. This may change in the future...
    if ( pBand->mpNextBand || pSep->mpNextSep )
        return REGION_OVER;

    // get justified rectangle
    long nLeft      = Min( rRect.Left(), rRect.Right() );
    long nTop       = Min( rRect.Top(), rRect.Bottom() );
    long nRight     = Max( rRect.Left(), rRect.Right() );
    long nBottom    = Max( rRect.Top(), rRect.Bottom() );

    // check rectangle region
    BOOL boLeft   = (nLeft >= pSep->mnXLeft) && (nLeft < pSep->mnXRight);
    BOOL boRight  = (nRight <= pSep->mnXRight) && (nRight > pSep->mnXLeft);
    BOOL boTop    = (nTop >= pBand->mnYTop) && (nTop < pBand->mnYBottom);
    BOOL boBottom = (nBottom <= pBand->mnYBottom) && (nBottom > pBand->mnYTop);
    if ( boLeft && boRight && boTop && boBottom )
        return REGION_INSIDE;
    if ( boLeft || boRight || boTop || boBottom )
        return REGION_OVER;

    return REGION_OUTSIDE;
}

// -----------------------------------------------------------------------

void Region::ImplBeginAddRect()
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // statische Object haben RefCount von 0
    if ( mpImplRegion->mnRefCount )
    {
        if ( mpImplRegion->mnRefCount > 1 )
            mpImplRegion->mnRefCount--;
        else
            delete mpImplRegion;
    }

    // create fresh region
    mpImplRegion = new ImplRegion();
}

// -----------------------------------------------------------------------

BOOL Region::ImplAddRect( const Rectangle& rRect )
{
    // Hier kein CheckThis, da nicht alle Daten auf Stand

    if ( rRect.IsEmpty() )
        return TRUE;

    // get justified rectangle
    long nTop;
    long nBottom;
    long nLeft;
    long nRight;
    if ( rRect.Top() <= rRect.Bottom() )
    {
        nTop = rRect.Top();
        nBottom = rRect.Bottom();
    }
    else
    {
        nTop = rRect.Bottom();
        nBottom = rRect.Top();
    }
    if ( rRect.Left() <= rRect.Right() )
    {
        nLeft = rRect.Left();
        nRight = rRect.Right();
    }
    else
    {
        nLeft = rRect.Right();
        nRight = rRect.Left();
    }

    if ( !mpImplRegion->mpLastCheckedBand )
    {
        // create new band
        mpImplRegion->mpLastCheckedBand = new ImplRegionBand( nTop, nBottom );

        // set band as current
        mpImplRegion->mpFirstBand = mpImplRegion->mpLastCheckedBand;
        mpImplRegion->mpLastCheckedBand->Union( nLeft, nRight );
    }
    else
    {
        DBG_ASSERT( nTop >= mpImplRegion->mpLastCheckedBand->mnYTop,
                    "Region::ImplAddRect() - nTopY < nLastTopY" );

        // new band? create it!
        if ( (nTop != mpImplRegion->mpLastCheckedBand->mnYTop) ||
             (nBottom != mpImplRegion->mpLastCheckedBand->mnYBottom) )
        {
            // create new band
            ImplRegionBand* pNewRegionBand = new ImplRegionBand( nTop, nBottom );

            // append band to the end
            mpImplRegion->mpLastCheckedBand->mpNextBand = pNewRegionBand;

            // skip to the new band
            mpImplRegion->mpLastCheckedBand = mpImplRegion->mpLastCheckedBand->mpNextBand;
        }

        // Insert Sep
        mpImplRegion->mpLastCheckedBand->Union( nLeft, nRight );
    }

    return TRUE;
}

// -----------------------------------------------------------------------

void Region::ImplEndAddRect()
{
    // check if we are empty
    if ( !mpImplRegion->mpFirstBand )
    {
        delete mpImplRegion;
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
        return;
    }

    // check if we have somthing to optimize
    if ( !mpImplRegion->mpFirstBand->mpNextBand )
    {
        // update mpImplRegion->mnRectCount, because no OptimizeBandList is called
        ImplRegionBandSep* pSep = mpImplRegion->mpFirstBand->mpFirstSep;
        mpImplRegion->mnRectCount = 0;
        while( pSep )
        {
            mpImplRegion->mnRectCount++;
            pSep = pSep->mpNextSep;
        }

        // Erst hier testen, da hier die Daten wieder stimmen
        DBG_CHKTHIS( Region, ImplDbgTestRegion );
        return;
    }

    // have to revert list? -> do it now!
    if ( mpImplRegion->mpFirstBand->mnYTop >
         mpImplRegion->mpFirstBand->mpNextBand->mnYTop )
    {
        ImplRegionBand * pNewFirstRegionBand;

        // initialize temp list with first element
        pNewFirstRegionBand = mpImplRegion->mpFirstBand;
        mpImplRegion->mpFirstBand = mpImplRegion->mpFirstBand->mpNextBand;
        pNewFirstRegionBand->mpNextBand = NULL;

        // insert elements to the temp list
        while ( mpImplRegion->mpFirstBand )
        {
            ImplRegionBand * pSavedRegionBand = pNewFirstRegionBand;
            pNewFirstRegionBand = mpImplRegion->mpFirstBand;
            mpImplRegion->mpFirstBand = mpImplRegion->mpFirstBand->mpNextBand;
            pNewFirstRegionBand->mpNextBand = pSavedRegionBand;
        }

        // set temp list as new list
        mpImplRegion->mpFirstBand = pNewFirstRegionBand;
    }

    // cleanup
    if ( !mpImplRegion->OptimizeBandList() )
    {
        delete mpImplRegion;
        mpImplRegion = (ImplRegion*)(&aImplEmptyRegion);
    }

    // Erst hier testen, da hier die Daten wieder stimmen
    DBG_CHKTHIS( Region, ImplDbgTestRegion );
}

// -----------------------------------------------------------------------

ULONG Region::GetRectCount() const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ((Region*)this)->ImplPolyPolyRegionToBandRegion();

#ifdef DBG_UTIL
    ULONG nCount = 0;

    // all bands if not null or empty
    if ( (mpImplRegion != &aImplEmptyRegion) && (mpImplRegion != &aImplNullRegion) )
    {
        ImplRegionBand* pBand = mpImplRegion->mpFirstBand;
        while ( pBand )
        {
            ImplRegionBandSep* pSep = pBand->mpFirstSep;
            while( pSep )
            {
                nCount++;
                pSep = pSep->mpNextSep;
            }

            pBand = pBand->mpNextBand;
        }
    }

    DBG_ASSERT( mpImplRegion->mnRectCount == nCount, "Region: invalid mnRectCount!" );
#endif

    return mpImplRegion->mnRectCount;
}

// -----------------------------------------------------------------------

RegionHandle Region::BeginEnumRects()
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ImplPolyPolyRegionToBandRegion();

    // no internal data? -> region is empty!
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return NULL;

    // no band in the list? -> region is empty!
    if ( mpImplRegion->mpFirstBand == NULL )
    {
        DBG_ASSERT( mpImplRegion->mpFirstBand, "Region::BeginEnumRects() First Band is Empty!" );
        return NULL;
    }

    ImplRegionHandle* pData = new ImplRegionHandle;
    pData->mpRegion = new Region( *this );
    pData->mbFirst  = TRUE;

    // save pointers
    pData->mpCurrRectBand = pData->mpRegion->mpImplRegion->mpFirstBand;
    pData->mpCurrRectBandSep = pData->mpCurrRectBand->mpFirstSep;

    return (RegionHandle)pData;
}

// -----------------------------------------------------------------------

BOOL Region::GetEnumRects( RegionHandle pVoidData, Rectangle& rRect )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ImplRegionHandle* pData = (ImplRegionHandle*)pVoidData;
    if ( !pData )
        return FALSE;

    if ( pData->mbFirst )
        pData->mbFirst = FALSE;
    else
    {
        // get next separation from current band
        pData->mpCurrRectBandSep = pData->mpCurrRectBandSep->mpNextSep;

        // no separation found? -> go to next band!
        if ( !pData->mpCurrRectBandSep )
        {
            // get next band
            pData->mpCurrRectBand = pData->mpCurrRectBand->mpNextBand;

            // no band found? -> not further rectangles!
            if ( !pData->mpCurrRectBand )
                return FALSE;

            // get first separation in current band
            pData->mpCurrRectBandSep = pData->mpCurrRectBand->mpFirstSep;
        }
    }

    // get boundaries of current rectangle
    rRect.Top()     = pData->mpCurrRectBand->mnYTop;
    rRect.Bottom()  = pData->mpCurrRectBand->mnYBottom;
    rRect.Left()    = pData->mpCurrRectBandSep->mnXLeft;
    rRect.Right()   = pData->mpCurrRectBandSep->mnXRight;
    return TRUE;
}

// -----------------------------------------------------------------------

void Region::EndEnumRects( RegionHandle pVoidData )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ImplRegionHandle* pData = (ImplRegionHandle*)pVoidData;
    if ( !pData )
        return;

    // cleanup
    delete pData->mpRegion;
    delete pData;
}
