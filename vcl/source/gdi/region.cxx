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

#include <limits.h>
#include <tools/vcompat.hxx>
#include <vcl/salbtype.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#ifndef _REGION_H
#include <vcl/region.h>
#endif
#ifndef _REGION_HXX
#include <vcl/region.hxx>
#endif
#ifndef _REGBAND_HXX
#include <vcl/regband.hxx>
#endif

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

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

static ImplRegionBase aImplNullRegion( 0 );
static ImplRegionBase aImplEmptyRegion( 0 );

// =======================================================================

DBG_NAME( Region )
DBG_NAMEEX( Polygon )
DBG_NAMEEX( PolyPolygon )

namespace {

/** Return <TRUE/> when the given polygon is rectiliner and oriented so that
    all sides are either horizontal or vertical.
*/
bool ImplIsPolygonRectilinear (const PolyPolygon& rPolyPoly)
{
    // Iterate over all polygons.
    const sal_uInt16 nPolyCount = rPolyPoly.Count();
    for (sal_uInt16 nPoly = 0; nPoly < nPolyCount; ++nPoly)
    {
        const Polygon&  aPoly = rPolyPoly.GetObject(nPoly);

        // Iterate over all edges of the current polygon.
        const sal_uInt16 nSize = aPoly.GetSize();

        if (nSize < 2)
            continue;
        Point aPoint (aPoly.GetPoint(0));
        const Point aLastPoint (aPoint);
        for (sal_uInt16 nPoint = 1; nPoint < nSize; ++nPoint)
        {
            const Point aNextPoint (aPoly.GetPoint(nPoint));
            // When there is at least one edge that is neither vertical nor
            // horizontal then the entire polygon is not rectilinear (and
            // oriented along primary axes.)
            if (aPoint.X() != aNextPoint.X() && aPoint.Y() != aNextPoint.Y())
                return false;

            aPoint = aNextPoint;
        }
        // Compare closing edge.
        if (aLastPoint.X() != aPoint.X() && aLastPoint.Y() != aPoint.Y())
            return false;
    }
    return true;
}



/** This function is similar to the ImplRegion::InsertBands() method.
    It creates a minimal set of missing bands so that the entire vertical
    interval from nTop to nBottom is covered by bands.
*/
void ImplAddMissingBands (
    ImplRegion* pImplRegion,
    const long nTop,
    const long nBottom)
{
    // Iterate over already existing bands and add missing bands atop the
    // first and between two bands.
    ImplRegionBand* pPreviousBand = NULL;
    ImplRegionBand* pBand = pImplRegion->ImplGetFirstRegionBand();
    long nCurrentTop (nTop);
    while (pBand != NULL && nCurrentTop<nBottom)
    {
        if (nCurrentTop < pBand->mnYTop)
        {
            // Create new band above the current band.
            ImplRegionBand* pAboveBand = new ImplRegionBand(
                nCurrentTop,
                ::std::min(nBottom,pBand->mnYTop-1));
            pImplRegion->InsertBand(pPreviousBand, pAboveBand);
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
        pImplRegion->InsertBand(
            pPreviousBand,
            new ImplRegionBand(
                nCurrentTop,
                nBottom));
    }
}



/** Convert a rectilinear polygon (that is oriented along the primary axes)
    to a list of bands.  For this special form of polygon we can use an
    optimization that prevents the creation of one band per y value.
    However, it still is possible that some temporary bands are created that
    later can be optimized away.
    @param rPolyPolygon
        A set of zero, one, or more polygons, nested or not, that are
        converted into a list of bands.
    @return
        A new ImplRegion object is returned that contains the bands that
        represent the given poly-polygon.
*/
ImplRegion* ImplRectilinearPolygonToBands (const PolyPolygon& rPolyPoly)
{
    OSL_ASSERT(ImplIsPolygonRectilinear (rPolyPoly));

    // Create a new ImplRegion object as container of the bands.
    ImplRegion* pImplRegion = new ImplRegion();
    long nLineId = 0L;

    // Iterate over all polygons.
    const sal_uInt16 nPolyCount = rPolyPoly.Count();
    for (sal_uInt16 nPoly = 0; nPoly < nPolyCount; ++nPoly)
    {
        const Polygon&  aPoly = rPolyPoly.GetObject(nPoly);

        // Iterate over all edges of the current polygon.
        const sal_uInt16 nSize = aPoly.GetSize();
        if (nSize < 2)
            continue;
        // Avoid fetching every point twice (each point is the start point
        // of one and the end point of another edge.)
        Point aStart (aPoly.GetPoint(0));
        Point aEnd;
        for (sal_uInt16 nPoint = 1; nPoint <= nSize; ++nPoint, aStart=aEnd)
        {
            // We take the implicit closing edge into account by mapping
            // index nSize to 0.
            aEnd = aPoly.GetPoint(nPoint%nSize);
            if (aStart.Y() == aEnd.Y())
            {
                // Horizontal lines are ignored.
                continue;
            }

            // At this point the line has to be vertical.
            OSL_ASSERT(aStart.X() == aEnd.X());

            // Sort y-coordinates to simplify the algorithm and store the
            // direction seperately.  The direction is calculated as it is
            // in other places (but seems to be the wrong way.)
            const long nTop (::std::min(aStart.Y(), aEnd.Y()));
            const long nBottom (::std::max(aStart.Y(), aEnd.Y()));
            const LineType eLineType (aStart.Y() > aEnd.Y() ? LINE_DESCENDING : LINE_ASCENDING);

            // Make sure that the current line is covered by bands.
            ImplAddMissingBands(pImplRegion, nTop,nBottom);

            // Find top-most band that may contain nTop.
            ImplRegionBand* pBand = pImplRegion->ImplGetFirstRegionBand();
            while (pBand!=NULL && pBand->mnYBottom < nTop)
                pBand = pBand->mpNextBand;
            ImplRegionBand* pTopBand = pBand;
            // If necessary split the band at nTop so that nTop is contained
            // in the lower band.
            if (pBand!=NULL
                   // Prevent the current band from becoming 0 pixel high
                && pBand->mnYTop<nTop
                   // this allows the lowest pixel of the band to be split off
                && pBand->mnYBottom>=nTop
                   // do not split a band that is just one pixel high
                && pBand->mnYTop<pBand->mnYBottom)
            {
                // Split the top band.
                pTopBand = pBand->SplitBand(nTop);
            }

            // Advance to band that may contain nBottom.
            while (pBand!=NULL && pBand->mnYBottom < nBottom)
                pBand = pBand->mpNextBand;
            // The lowest band may have to be split at nBottom so that
            // nBottom itself remains in the upper band.
            if (pBand!=NULL
                   // allow the current band becoming 1 pixel high
                && pBand->mnYTop<=nBottom
                   // prevent splitting off a band that is 0 pixel high
                && pBand->mnYBottom>nBottom
                   // do not split a band that is just one pixel high
                && pBand->mnYTop<pBand->mnYBottom)
            {
                // Split the bottom band.
                pBand->SplitBand(nBottom+1);
            }

            // Note that we remember the top band (in pTopBand) but not the
            // bottom band.  The later can be determined by comparing y
            // coordinates.

            // Add the x-value as point to all bands in the nTop->nBottom range.
            for (pBand=pTopBand; pBand!=NULL&&pBand->mnYTop<=nBottom; pBand=pBand->mpNextBand)
                pBand->InsertPoint(aStart.X(), nLineId++, sal_True, eLineType);
        }
    }

    return pImplRegion;
}




/** Convert a general polygon (one for which ImplIsPolygonRectilinear()
    returns <FALSE/>) to bands.
*/
ImplRegion* ImplGeneralPolygonToBands (
    const PolyPolygon& rPolyPoly,
    const Rectangle& rPolygonBoundingBox)
{
    long nLineID = 0L;

    // initialisation and creation of Bands
    ImplRegion* pImplRegion = new ImplRegion();
    pImplRegion->CreateBandRange( rPolygonBoundingBox.Top(), rPolygonBoundingBox.Bottom() );

    // insert polygons
    const sal_uInt16 nPolyCount = rPolyPoly.Count();
    for ( sal_uInt16 nPoly = 0; nPoly < nPolyCount; nPoly++ )
    {
        // get reference to current polygon
        const Polygon&  aPoly = rPolyPoly.GetObject( nPoly );
        const sal_uInt16    nSize = aPoly.GetSize();

        // not enough points ( <= 2 )? -> nothing to do!
        if ( nSize <= 2 )
            continue;

        // band the polygon
        for ( sal_uInt16 nPoint = 1; nPoint < nSize; nPoint++ )
            pImplRegion->InsertLine( aPoly.GetPoint(nPoint-1), aPoly.GetPoint(nPoint), nLineID++ );

        // close polygon with line from first point to last point, if neccesary
        const Point rLastPoint = aPoly.GetPoint(nSize-1);
        const Point rFirstPoint = aPoly.GetPoint(0);
        if ( rLastPoint != rFirstPoint )
            pImplRegion->InsertLine( rLastPoint, rFirstPoint, nLineID++ );
    }

    return pImplRegion;
}


} // end of anonymous namespace


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
        sal_uLong                   nCount = 0;
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

void TraceBands (const ImplRegionBand* pFirstBand)
{
    int nBandIndex  (0);
    const ImplRegionBand* pBand = pFirstBand;
    while (pBand != NULL)
    {
        OSL_TRACE("            band %d  %d->%d : ", nBandIndex++,
            pBand->mnYTop, pBand->mnYBottom);

        ImplRegionBandPoint* pPoint = pBand->mpFirstBandPoint;
        while (pPoint != NULL)
        {
            OSL_TRACE(" %d ", pPoint->mnX);
            pPoint = pPoint->mpNextBandPoint;
        }
        OSL_TRACE("  |  ");

        ImplRegionBandSep* pSep = pBand->mpFirstSep;
        while (pSep != NULL)
        {
            OSL_TRACE(" %d->%d ", pSep->mnXLeft, pSep->mnXRight);
            pSep = pSep->mpNextSep;
        }
        OSL_TRACE("\n");

        pBand = pBand->mpNextBand;
    }
}
#endif

// =======================================================================

inline void Region::ImplPolyPolyRegionToBandRegion()
{
    if( mpImplRegion->mpPolyPoly || mpImplRegion->mpB2DPolyPoly )
        ImplPolyPolyRegionToBandRegionFunc();
}

// =======================================================================

ImplRegionBase::ImplRegionBase( int nRefCount )
:   mnRefCount( nRefCount )
,   mnRectCount( 0 )
,   mpPolyPoly( NULL )
,   mpB2DPolyPoly( NULL )
{}

// ------------------------------------------------------------------------

ImplRegion::ImplRegion()
{
    mpFirstBand         = NULL;
    mpLastCheckedBand   = NULL;
}

// ------------------------------------------------------------------------

ImplRegion::ImplRegion( const PolyPolygon& rPolyPoly )
{
    mpFirstBand         = NULL;
    mpLastCheckedBand   = NULL;
    mpPolyPoly          = new PolyPolygon( rPolyPoly );
}

// ------------------------------------------------------------------------

ImplRegion::ImplRegion( const basegfx::B2DPolyPolygon& rPolyPoly )
{
    mpFirstBand = NULL;
    mpLastCheckedBand = NULL;
    mpB2DPolyPoly = new basegfx::B2DPolyPolygon( rPolyPoly );
}

// -----------------------------------------------------------------------

ImplRegion::ImplRegion( const ImplRegion& rImplRegion )
:   ImplRegionBase()
{
    mpFirstBand = NULL;
    mpLastCheckedBand = NULL;
    mnRectCount = rImplRegion.mnRectCount;

    if ( rImplRegion.mpPolyPoly )
        mpPolyPoly = new PolyPolygon( *rImplRegion.mpPolyPoly );
    else if( rImplRegion.mpB2DPolyPoly )
        mpB2DPolyPoly = new basegfx::B2DPolyPolygon( *rImplRegion.mpB2DPolyPoly );

    // insert band(s) into the list
    ImplRegionBand* pNewBand;
    ImplRegionBand* pPrevBand = 0;
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
                "ImplRegion::~ImplRegion() - Empty oder NULL-Region" );

    ImplRegionBand* pBand = mpFirstBand;
    while ( pBand )
    {
        ImplRegionBand* pTempBand = pBand->mpNextBand;
        delete pBand;
        pBand = pTempBand;
    }
}

// -----------------------------------------------------------------------

ImplRegionBase::~ImplRegionBase()
{
    delete mpPolyPoly;
    delete mpB2DPolyPoly;
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

sal_Bool ImplRegion::InsertLine( const Point& rStartPt, const Point& rEndPt,
                             long nLineId )
{
    long nX, nY;

    // lines consisting of a single point do not interest here
    if ( rStartPt == rEndPt )
        return sal_True;

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
        InsertPoint( Point( nEndX, nEndY ), nLineId, sal_True, eLineType );
    }

    return sal_True;
}

// -----------------------------------------------------------------------
//
// search for appropriate place for the new point

sal_Bool ImplRegion::InsertPoint( const Point &rPoint, long nLineID,
                              sal_Bool bEndPoint, LineType eLineType )
{
    DBG_ASSERT( mpFirstBand != NULL, "ImplRegion::InsertPoint - no bands available!" );

    if ( rPoint.Y() == mpLastCheckedBand->mnYTop )
    {
        mpLastCheckedBand->InsertPoint( rPoint.X(), nLineID, bEndPoint, eLineType );
        return sal_True;
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
                return sal_True;
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
                return sal_True;
            }

            mpLastCheckedBand = mpLastCheckedBand->mpPrevBand;
        }

        DBG_ERROR( "ImplRegion::InsertPoint reached the beginning of the list!" );
    }

    DBG_ERROR( "ImplRegion::InsertPoint point not inserted!" );

    // reinitialize pointer (should never be reached!)
    mpLastCheckedBand = mpFirstBand;

    return sal_False;
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
    sal_Bool bTopBoundaryInserted = sal_False;
    sal_Bool bTop2BoundaryInserted = sal_False;
    sal_Bool bBottomBoundaryInserted = sal_False;

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

        bTopBoundaryInserted = sal_True;
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

sal_Bool ImplRegion::InsertSingleBand( ImplRegionBand* pBand,
                                   long nYBandPosition )
{
    // boundary already included in band with height 1? -> nothing to do!
    if ( (pBand->mnYTop == pBand->mnYBottom) &&
         (nYBandPosition == pBand->mnYTop) )
        return sal_True;

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

        return sal_True;
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

        return sal_True;
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
            return sal_True;
        }

        if ( nYBandPosition > pBand->mnYBottom )
        {
            // create new band
            pNewBand = new ImplRegionBand( pBand->mnYBottom + 1, nYBandPosition );

            // append band to the list
            pBand->mpNextBand = pNewBand;
            return sal_True;
        }
    }

    return sal_False;
}

// ------------------------------------------------------------------------

void ImplRegion::InsertBand (ImplRegionBand* pPreviousBand, ImplRegionBand* pBandToInsert)
{
    OSL_ASSERT(pBandToInsert!=NULL);

    if (pPreviousBand == NULL)
    {
        // Insert band before all others.
        if (mpFirstBand != NULL)
            mpFirstBand->mpPrevBand = pBandToInsert;
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

sal_Bool ImplRegion::OptimizeBandList()
{
    DBG_ASSERT( (this != &aImplNullRegion) && (this != &aImplEmptyRegion),
                "ImplRegion::OptimizeBandList() - Empty oder NULL-Region" );

    mnRectCount = 0;

    ImplRegionBand* pPrevBand = 0;
    ImplRegionBand* pBand = mpFirstBand;
    while ( pBand )
    {
        const sal_Bool bBTEqual = pBand->mpNextBand &&
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

Region::Region( const basegfx::B2DPolyPolygon& rPolyPoly )
{
    DBG_CTOR( Region, ImplDbgTestRegion );
    DBG_CHKOBJ( &rPolyPoly, PolyPolygon, NULL );

    mpImplRegion = new ImplRegion( rPolyPoly );
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
    const sal_uInt16 nPolyCount = rPolyPoly.Count();
    if ( nPolyCount )
    {
        // polypolygon empty? -> empty region
        const Rectangle aRect( rPolyPoly.GetBoundRect() );

        if ( !aRect.IsEmpty() )
        {
            // width OR height == 1 ? => Rectangular region
            if ( (aRect.GetWidth() == 1)
                || (aRect.GetHeight() == 1)
                || rPolyPoly.IsRect() )
            {
                ImplCreateRectRegion( aRect );
            }
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
    // ensure to subdivide when bezier segemnts are used, it's going to
    // be expanded to rectangles
    PolyPolygon aPolyPoly;
    GetPolyPolygon().AdaptiveSubdivide(aPolyPoly);

    if ( mpImplRegion->mnRefCount > 1 )
        mpImplRegion->mnRefCount--;
    else
        delete mpImplRegion;

    if ( aPolyPoly.Count() )
    {
        // polypolygon empty? -> empty region
        const Rectangle aRect( aPolyPoly.GetBoundRect() );

        if ( !aRect.IsEmpty() )
        {
            if (ImplIsPolygonRectilinear(aPolyPoly))
            {
                // For rectilinear polygons there is an optimized band conversion.
                mpImplRegion = ImplRectilinearPolygonToBands(aPolyPoly);
            }
            else
            {
                mpImplRegion = ImplGeneralPolygonToBands(aPolyPoly, aRect);
            }

            // Convert points into seps.
            ImplRegionBand* pRegionBand = mpImplRegion->mpFirstBand;
            while ( pRegionBand )
            {
                // generate separations from the lines and process union
                pRegionBand->ProcessPoints();
                pRegionBand = pRegionBand->mpNextBand;
            }

            // Optimize list of bands.  Adjacent bands with identical lists
            // of seps are joined.
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
    else if( mpImplRegion->mpB2DPolyPoly )
    {
        mpImplRegion->mpB2DPolyPoly->transform(basegfx::tools::createTranslateB2DHomMatrix(nHorzMove, nVertMove));
    }
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
    else if( mpImplRegion->mpB2DPolyPoly )
    {
        mpImplRegion->mpB2DPolyPoly->transform(basegfx::tools::createScaleB2DHomMatrix(fScaleX, fScaleY));
    }
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

sal_Bool Region::Union( const Rectangle& rRect )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // is rectangle empty? -> nothing to do
    if ( rRect.IsEmpty() )
        return sal_True;

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

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool Region::Intersect( const Rectangle& rRect )
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
        return sal_True;
    }

    // #103137# Avoid banding for special cases
    if ( mpImplRegion->mpPolyPoly )
    {
        // #127431# make ImplRegion unique, if not already.
        if( mpImplRegion->mnRefCount > 1 )
        {
            mpImplRegion->mnRefCount--;
            mpImplRegion = new ImplRegion( *mpImplRegion->mpPolyPoly );
        }

        // use the PolyPolygon::Clip method for rectangles, this is
        // fairly simple (does not even use GPC) and saves us from
        // unnecessary banding
        mpImplRegion->mpPolyPoly->Clip( rRect );

        return sal_True;
    }
    else
        ImplPolyPolyRegionToBandRegion();

    // is region empty? -> nothing to do!
    if ( mpImplRegion == &aImplEmptyRegion )
        return sal_True;

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

        return sal_True;
    }

    // no own instance data? -> make own copy!
    if ( mpImplRegion->mnRefCount > 1 )
        ImplCopyData();

    // insert bands if the boundaries are not allready in the list
    mpImplRegion->InsertBands( nTop, nBottom );

    // process intersections
    ImplRegionBand* pPrevBand = 0;
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

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool Region::Exclude( const Rectangle& rRect )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // is rectangle empty? -> nothing to do
    if ( rRect.IsEmpty() )
        return sal_True;

    ImplPolyPolyRegionToBandRegion();

    // no instance data? -> create!
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return sal_True;

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

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool Region::XOr( const Rectangle& rRect )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // is rectangle empty? -> nothing to do
    if ( rRect.IsEmpty() )
        return sal_True;

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

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool Region::Union( const Region& rRegion )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ImplPolyPolyRegionToBandRegion();
    ((Region*)&rRegion)->ImplPolyPolyRegionToBandRegion();

    // is region empty or null? -> nothing to do
    if ( (rRegion.mpImplRegion == &aImplEmptyRegion) || (rRegion.mpImplRegion == &aImplNullRegion) )
        return sal_True;

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

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool Region::Intersect( const Region& rRegion )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // same instance data? -> nothing to do!
    if ( mpImplRegion == rRegion.mpImplRegion )
        return sal_True;

    ImplPolyPolyRegionToBandRegion();
    ((Region*)&rRegion)->ImplPolyPolyRegionToBandRegion();

    if ( mpImplRegion == &aImplEmptyRegion )
        return sal_True;

    // is region null? -> nothing to do
    if ( rRegion.mpImplRegion == &aImplNullRegion )
        return sal_True;

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
        return sal_True;
    }

    // is own region NULL-region? -> copy data!
    if ( mpImplRegion == &aImplNullRegion)
    {
        mpImplRegion = rRegion.mpImplRegion;
        rRegion.mpImplRegion->mnRefCount++;
        return sal_True;
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
            pBand->mbTouched = sal_False;
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
        ImplRegionBand* pPrevBand = 0;
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

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool Region::Exclude( const Region& rRegion )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ImplPolyPolyRegionToBandRegion();
    ((Region*)&rRegion)->ImplPolyPolyRegionToBandRegion();

    // is region empty or null? -> nothing to do
    if ( (rRegion.mpImplRegion == &aImplEmptyRegion) || (rRegion.mpImplRegion == &aImplNullRegion) )
        return sal_True;

    // no instance data? -> nothing to do
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return sal_True;

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

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool Region::XOr( const Region& rRegion )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ImplPolyPolyRegionToBandRegion();
    ((Region*)&rRegion)->ImplPolyPolyRegionToBandRegion();

    // is region empty or null? -> nothing to do
    if ( (rRegion.mpImplRegion == &aImplEmptyRegion) || (rRegion.mpImplRegion == &aImplNullRegion) )
        return sal_True;

    // no own instance data? -> XOr = copy
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
    {
        *this = rRegion;
        return sal_True;
    }

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

    return sal_True;
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
    if( mpImplRegion->mpB2DPolyPoly )
    {
        const basegfx::B2DRange aRange = basegfx::tools::getRange( *mpImplRegion->mpB2DPolyPoly );
        aRect.SetPos( Point( (int)aRange.getMinX(), (int)aRange.getMinY() ) );
        aRect.SetSize( Size( (int)aRange.getWidth(), (int)aRange.getHeight() ) );
        return aRect;
    }

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

sal_Bool Region::HasPolyPolygon() const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );
    if( !mpImplRegion )
        return false;
    if( mpImplRegion->mpPolyPoly )
        return true;
    if( mpImplRegion->mpB2DPolyPoly )
        return true;
    return false;
}

// -----------------------------------------------------------------------

PolyPolygon Region::GetPolyPolygon() const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    PolyPolygon aRet;

    if( mpImplRegion->mpPolyPoly )
        aRet = *mpImplRegion->mpPolyPoly;
    else if( mpImplRegion->mpB2DPolyPoly )
    {
        // the polygon needs to be converted
        aRet = PolyPolygon( *mpImplRegion->mpB2DPolyPoly );
        // TODO: cache the converted polygon?
        // mpImplRegion->mpB2DPolyPoly = aRet;
    }

    return aRet;
}

// -----------------------------------------------------------------------

const basegfx::B2DPolyPolygon Region::GetB2DPolyPolygon() const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    basegfx::B2DPolyPolygon aRet;

    if( mpImplRegion->mpB2DPolyPoly )
        aRet = *mpImplRegion->mpB2DPolyPoly;
    else if( mpImplRegion->mpPolyPoly )
    {
        // the polygon needs to be converted
        aRet = mpImplRegion->mpPolyPoly->getB2DPolyPolygon();
        // TODO: cache the converted polygon?
        // mpImplRegion->mpB2DPolyPoly = aRet;
    }

    return aRet;
}

// -----------------------------------------------------------------------

basegfx::B2DPolyPolygon Region::ConvertToB2DPolyPolygon()
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    basegfx::B2DPolyPolygon aRet;

    if( HasPolyPolygon() )
        aRet = GetB2DPolyPolygon();
    else
    {
        RegionHandle aHdl = BeginEnumRects();
        Rectangle aSubRect;
        while( GetNextEnumRect( aHdl, aSubRect ) )
        {
            basegfx::B2DPolygon aPoly( basegfx::tools::createPolygonFromRect(
                 basegfx::B2DRectangle( aSubRect.Left(), aSubRect.Top(), aSubRect.Right(), aSubRect.Bottom() ) ) );
            aRet.append( aPoly );
        }
        EndEnumRects( aHdl );
    }

    return aRet;
}

// -----------------------------------------------------------------------

sal_Bool Region::ImplGetFirstRect( ImplRegionInfo& rImplRegionInfo,
                               long& rX, long& rY,
                               long& rWidth, long& rHeight ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ((Region*)this)->ImplPolyPolyRegionToBandRegion();

    // no internal data? -> region is empty!
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return sal_False;

    // no band in the list? -> region is empty!
    if ( mpImplRegion->mpFirstBand == NULL )
        return sal_False;

    // initialise pointer for first access
    ImplRegionBand*     pCurrRectBand = mpImplRegion->mpFirstBand;
    ImplRegionBandSep*  pCurrRectBandSep = pCurrRectBand->mpFirstSep;

    DBG_ASSERT( pCurrRectBandSep != NULL, "Erstes Band wurde nicht optimiert." );
    if ( !pCurrRectBandSep )
        return sal_False;

    // get boundaries of current rectangle
    rX      = pCurrRectBandSep->mnXLeft;
    rY      = pCurrRectBand->mnYTop;
    rWidth  = pCurrRectBandSep->mnXRight - pCurrRectBandSep->mnXLeft + 1;
    rHeight = pCurrRectBand->mnYBottom - pCurrRectBand->mnYTop + 1;

    // save pointers
    rImplRegionInfo.mpVoidCurrRectBand = (void*)pCurrRectBand;
    rImplRegionInfo.mpVoidCurrRectBandSep = (void*)pCurrRectBandSep;

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool Region::ImplGetNextRect( ImplRegionInfo& rImplRegionInfo,
                              long& rX, long& rY,
                              long& rWidth, long& rHeight ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // no internal data? -> region is empty!
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return sal_False;

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
            return sal_False;

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

    return sal_True;
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

sal_Bool Region::IsInside( const Point& rPoint ) const
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
        return sal_False;

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
                return sal_True;
            else
                return sal_False;
        }

        pBand = pBand->mpNextBand;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Region::IsInside( const Rectangle& rRect ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    // is rectangle empty? -> not inside
    if ( rRect.IsEmpty() )
        return sal_False;

    // no instance data? -> not inside
    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return sal_False;

    // create region from rectangle and intersect own region
    Region aRegion = rRect;
    aRegion.Exclude( *this );

    // rectangle is inside if exclusion is empty
    return aRegion.IsEmpty();
}

// -----------------------------------------------------------------------

sal_Bool Region::IsOver( const Rectangle& rRect ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return sal_False;

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

sal_Bool Region::operator==( const Region& rRegion ) const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );
    DBG_CHKOBJ( &rRegion, Region, ImplDbgTestRegion );

    // reference to same object? -> equal!
    if ( mpImplRegion == rRegion.mpImplRegion )
        return sal_True;

    if ( (mpImplRegion == &aImplEmptyRegion) || (mpImplRegion == &aImplNullRegion) )
        return sal_False;

    if ( (rRegion.mpImplRegion == &aImplEmptyRegion) || (rRegion.mpImplRegion == &aImplNullRegion) )
        return sal_False;

    if ( rRegion.mpImplRegion->mpPolyPoly && mpImplRegion->mpPolyPoly )
        return *rRegion.mpImplRegion->mpPolyPoly == *mpImplRegion->mpPolyPoly;
    else
    {
        ((Region*)this)->ImplPolyPolyRegionToBandRegion();
        ((Region*)&rRegion)->ImplPolyPolyRegionToBandRegion();

        // Eine der beiden Regions kann jetzt Empty sein
        if ( mpImplRegion == rRegion.mpImplRegion )
            return sal_True;

        if ( mpImplRegion == &aImplEmptyRegion )
            return sal_False;

        if ( rRegion.mpImplRegion == &aImplEmptyRegion )
            return sal_False;
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
            return sal_False;

        long nOwnYTop = pOwnRectBand->mnYTop;
        long nSecondYTop = pSecondRectBand->mnYTop;
        if ( nOwnYTop != nSecondYTop )
            return sal_False;

        long nOwnXRight = pOwnRectBandSep->mnXRight;
        long nSecondXRight = pSecondRectBandSep->mnXRight;
        if ( nOwnXRight != nSecondXRight )
            return sal_False;

        long nOwnYBottom = pOwnRectBand->mnYBottom;
        long nSecondYBottom = pSecondRectBand->mnYBottom;
        if ( nOwnYBottom != nSecondYBottom )
            return sal_False;

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
            return sal_False;

        if ( !pOwnRectBandSep && pSecondRectBandSep )
            return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

enum StreamEntryType { STREAMENTRY_BANDHEADER, STREAMENTRY_SEPARATION, STREAMENTRY_END };

SvStream& operator>>( SvStream& rIStrm, Region& rRegion )
{
    DBG_CHKOBJ( &rRegion, Region, ImplDbgTestRegion );

    VersionCompat   aCompat( rIStrm, STREAM_READ );
    sal_uInt16          nVersion;
    sal_uInt16          nTmp16;

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
        {
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

                if( rIStrm.IsEof() )
                {
                    DBG_ERROR( "premature end of region stream" );
                    delete rRegion.mpImplRegion;
                    rRegion.mpImplRegion = (ImplRegion*)&aImplEmptyRegion;
                    return rIStrm;
                }

                // get next header
                rIStrm >> nTmp16;
            }

            if( aCompat.GetVersion() >= 2 )
            {
                sal_Bool bHasPolyPolygon;

                rIStrm >> bHasPolyPolygon;

                if( bHasPolyPolygon )
                {
                    delete rRegion.mpImplRegion->mpPolyPoly;
                    rRegion.mpImplRegion->mpPolyPoly = new PolyPolygon;
                    rIStrm >> *( rRegion.mpImplRegion->mpPolyPoly );
                }
            }
        }
        break;
    }

    return rIStrm;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStrm, const Region& rRegion )
{
    DBG_CHKOBJ( &rRegion, Region, ImplDbgTestRegion );

    sal_uInt16          nVersion = 2;
    VersionCompat   aCompat( rOStrm, STREAM_WRITE, nVersion );
    Region          aTmpRegion( rRegion );

    // use tmp region to avoid destruction of internal region (polypolygon) of rRegion
    aTmpRegion.ImplPolyPolyRegionToBandRegion();

    // put version
    rOStrm << nVersion;

    // put type
    rOStrm << (sal_uInt16)aTmpRegion.GetType();

    // put all bands if not null or empty
    if ( (aTmpRegion.mpImplRegion != &aImplEmptyRegion) && (aTmpRegion.mpImplRegion != &aImplNullRegion) )
    {
        ImplRegionBand* pBand = aTmpRegion.mpImplRegion->mpFirstBand;
        while ( pBand )
        {
            // put boundaries
            rOStrm << (sal_uInt16) STREAMENTRY_BANDHEADER;
            rOStrm << pBand->mnYTop;
            rOStrm << pBand->mnYBottom;

            // put separations of current band
            ImplRegionBandSep* pSep = pBand->mpFirstSep;
            while ( pSep )
            {
                // put separation
                rOStrm << (sal_uInt16) STREAMENTRY_SEPARATION;
                rOStrm << pSep->mnXLeft;
                rOStrm << pSep->mnXRight;

                // next separation from current band
                pSep = pSep->mpNextSep;
            }

            pBand = pBand->mpNextBand;
        }

        // put endmarker
        rOStrm << (sal_uInt16) STREAMENTRY_END;

        // write polypolygon if available
        const sal_Bool bHasPolyPolygon = rRegion.HasPolyPolygon();
        rOStrm << bHasPolyPolygon;

        if( bHasPolyPolygon )
        {
            // #i105373#
            PolyPolygon aNoCurvePolyPolygon;
            rRegion.GetPolyPolygon().AdaptiveSubdivide(aNoCurvePolyPolygon);

            rOStrm << aNoCurvePolyPolygon;
        }
    }

    return rOStrm;
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

sal_Bool Region::ImplAddRect( const Rectangle& rRect )
{
    // Hier kein CheckThis, da nicht alle Daten auf Stand

    if ( rRect.IsEmpty() )
        return sal_True;

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

    return sal_True;
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

sal_uLong Region::GetRectCount() const
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ((Region*)this)->ImplPolyPolyRegionToBandRegion();

#ifdef DBG_UTIL
    sal_uLong nCount = 0;

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
        return 0;

    // no band in the list? -> region is empty!
    if ( mpImplRegion->mpFirstBand == NULL )
    {
        DBG_ASSERT( mpImplRegion->mpFirstBand, "Region::BeginEnumRects() First Band is Empty!" );
        return 0;
    }

    ImplRegionHandle* pData = new ImplRegionHandle;
    pData->mpRegion = new Region( *this );
    pData->mbFirst  = sal_True;

    // save pointers
    pData->mpCurrRectBand = pData->mpRegion->mpImplRegion->mpFirstBand;
    pData->mpCurrRectBandSep = pData->mpCurrRectBand->mpFirstSep;

    return (RegionHandle)pData;
}

// -----------------------------------------------------------------------

sal_Bool Region::GetEnumRects( RegionHandle pVoidData, Rectangle& rRect )
{
    DBG_CHKTHIS( Region, ImplDbgTestRegion );

    ImplRegionHandle* pData = (ImplRegionHandle*)pVoidData;
    if ( !pData )
        return sal_False;

    if ( pData->mbFirst )
        pData->mbFirst = sal_False;
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
                return sal_False;

            // get first separation in current band
            pData->mpCurrRectBandSep = pData->mpCurrRectBand->mpFirstSep;
        }
    }

    // get boundaries of current rectangle
    rRect.Top()     = pData->mpCurrRectBand->mnYTop;
    rRect.Bottom()  = pData->mpCurrRectBand->mnYBottom;
    rRect.Left()    = pData->mpCurrRectBandSep->mnXLeft;
    rRect.Right()   = pData->mpCurrRectBandSep->mnXRight;
    return sal_True;
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

// -----------------------------------------------------------------------

static inline bool ImplPolygonRectTest( const Polygon& rPoly, Rectangle* pRectOut = NULL )
{
    bool bIsRect = false;
    const Point* pPoints = rPoly.GetConstPointAry();
    sal_uInt16 nPoints = rPoly.GetSize();
    if( nPoints == 4 || (nPoints == 5 && pPoints[0] == pPoints[4]) )
    {
        long nX1 = pPoints[0].X(), nX2 = pPoints[2].X(),
        nY1 = pPoints[0].Y(), nY2 = pPoints[2].Y();
        if( ( (pPoints[1].X() == nX1 && pPoints[3].X() == nX2) &&
            (pPoints[1].Y() == nY2 && pPoints[3].Y() == nY1) )
        ||
        ( (pPoints[1].X() == nX2 && pPoints[3].X() == nX1) &&
        (pPoints[1].Y() == nY1 && pPoints[3].Y() == nY2) ) )
        {
            bIsRect = true;
            if( pRectOut )
            {
                long nSwap;
                if( nX2 < nX1 )
                {
                    nSwap = nX2;
                    nX2 = nX1;
                    nX1 = nSwap;
                }
                if( nY2 < nY1 )
                {
                    nSwap = nY2;
                    nY2 = nY1;
                    nY1 = nSwap;
                }
                if( nX2 != nX1 )
                    nX2--;
                if( nY2 != nY1 )
                    nY2--;
                pRectOut->Left()    = nX1;
                pRectOut->Right()   = nX2;
                pRectOut->Top()     = nY1;
                pRectOut->Bottom()  = nY2;
            }
        }
    }
    return bIsRect;
}

Region Region::GetRegionFromPolyPolygon( const PolyPolygon& rPolyPoly )
{
    //return Region( rPolyPoly );

    // check if it's worth extracting the XOr'ing the Rectangles
    // empiricism shows that break even between XOr'ing rectangles separately
    // and ImplPolyPolyRegionToBandRegion is at half rectangles/half polygons
    int nPolygonRects = 0, nPolygonPolygons = 0;
    int nPolygons = rPolyPoly.Count();

    for( sal_uInt16 i = 0; i < nPolygons; i++ )
    {
        const Polygon& rPoly = rPolyPoly[i];
        if( ImplPolygonRectTest( rPoly ) )
            nPolygonRects++;
        else
            nPolygonPolygons++;
    }
    if( nPolygonPolygons > nPolygonRects )
        return Region( rPolyPoly );

    Region aResult;
    Rectangle aRect;
    for( sal_uInt16 i = 0; i < nPolygons; i++ )
    {
        const Polygon& rPoly = rPolyPoly[i];
        if( ImplPolygonRectTest( rPoly, &aRect ) )
            aResult.XOr( aRect );
        else
            aResult.XOr( Region(rPoly) );
    }
    return aResult;
}
