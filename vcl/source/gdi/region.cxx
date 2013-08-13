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

#include <limits.h>
#include <tools/vcompat.hxx>
#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <vcl/region.hxx>
#include <regionband.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

//////////////////////////////////////////////////////////////////////////////

DBG_NAME( Region )
DBG_NAMEEX( Polygon )
DBG_NAMEEX( PolyPolygon )

//////////////////////////////////////////////////////////////////////////////

namespace
{
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

    /** Convert a rectilinear polygon (that is oriented along the primary axes)
        to a list of bands.  For this special form of polygon we can use an
        optimization that prevents the creation of one band per y value.
        However, it still is possible that some temporary bands are created that
        later can be optimized away.
        @param rPolyPolygon
            A set of zero, one, or more polygons, nested or not, that are
            converted into a list of bands.
        @return
            A new RegionBand object is returned that contains the bands that
            represent the given poly-polygon.
    */
    RegionBand* ImplRectilinearPolygonToBands(const PolyPolygon& rPolyPoly)
    {
        OSL_ASSERT(ImplIsPolygonRectilinear (rPolyPoly));

        // Create a new RegionBand object as container of the bands.
        RegionBand* pRegionBand = new RegionBand();
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
                pRegionBand->ImplAddMissingBands(nTop,nBottom);

                // Find top-most band that may contain nTop.
                ImplRegionBand* pBand = pRegionBand->ImplGetFirstRegionBand();
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
                    pBand->InsertPoint(aStart.X(), nLineId++, true, eLineType);
            }
        }

        return pRegionBand;
    }

    /** Convert a general polygon (one for which ImplIsPolygonRectilinear()
        returns <FALSE/>) to bands.
    */
    RegionBand* ImplGeneralPolygonToBands(const PolyPolygon& rPolyPoly, const Rectangle& rPolygonBoundingBox)
    {
        long nLineID = 0L;

        // initialisation and creation of Bands
        RegionBand* pRegionBand = new RegionBand();
        pRegionBand->CreateBandRange(rPolygonBoundingBox.Top(), rPolygonBoundingBox.Bottom());

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
            {
                pRegionBand->InsertLine( aPoly.GetPoint(nPoint-1), aPoly.GetPoint(nPoint), nLineID++ );
            }

            // close polygon with line from first point to last point, if neccesary
            const Point rLastPoint = aPoly.GetPoint(nSize-1);
            const Point rFirstPoint = aPoly.GetPoint(0);

            if ( rLastPoint != rFirstPoint )
            {
                pRegionBand->InsertLine( rLastPoint, rFirstPoint, nLineID++ );
            }
        }

        return pRegionBand;
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

bool Region::IsEmpty() const
{
    return !mbIsNull && !mpB2DPolyPolygon.get() && !mpPolyPolygon.get() && !mpRegionBand.get();
}

bool Region::IsNull() const
{
    return mbIsNull;
}

RegionBand* ImplCreateRegionBandFromPolyPolygon(const PolyPolygon& rPolyPolygon)
{
    RegionBand* pRetval = 0;

    if(rPolyPolygon.Count())
    {
        // ensure to subdivide when bezier segemnts are used, it's going to
        // be expanded to rectangles
        PolyPolygon aPolyPolygon;

        rPolyPolygon.AdaptiveSubdivide(aPolyPolygon);

        if(aPolyPolygon.Count())
        {
            const Rectangle aRect(aPolyPolygon.GetBoundRect());

            if(!aRect.IsEmpty())
            {
                if(ImplIsPolygonRectilinear(aPolyPolygon))
                {
                    // For rectilinear polygons there is an optimized band conversion.
                    pRetval = ImplRectilinearPolygonToBands(aPolyPolygon);
                }
                else
                {
                    pRetval = ImplGeneralPolygonToBands(aPolyPolygon, aRect);
                }

                // Convert points into seps.
                if(pRetval)
                {
                    pRetval->processPoints();

                    // Optimize list of bands.  Adjacent bands with identical lists
                    // of seps are joined.
                    if(!pRetval->OptimizeBandList())
                    {
                        delete pRetval;
                        pRetval = 0;
                    }
                }
            }
        }
    }

    return pRetval;
}

PolyPolygon Region::ImplCreatePolyPolygonFromRegionBand() const
{
    PolyPolygon aRetval;

    if(getRegionBand())
    {
        RectangleVector aRectangles;
        GetRegionRectangles(aRectangles);

        for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            aRetval.Insert(Polygon(*aRectIter));
        }
    }
    else
    {
        OSL_ENSURE(false, "Called with no local RegionBand (!)");
    }

    return aRetval;
}

basegfx::B2DPolyPolygon Region::ImplCreateB2DPolyPolygonFromRegionBand() const
{
    PolyPolygon aPoly(ImplCreatePolyPolygonFromRegionBand());

    return aPoly.getB2DPolyPolygon();
}

Region::Region(bool bIsNull)
:   mpB2DPolyPolygon(),
    mpPolyPolygon(),
    mpRegionBand(),
    mbIsNull(bIsNull)
{
}

Region::Region(const Rectangle& rRect)
:   mpB2DPolyPolygon(),
    mpPolyPolygon(),
    mpRegionBand(),
    mbIsNull(false)
{
    mpRegionBand.reset(rRect.IsEmpty() ? 0 : new RegionBand(rRect));
}

Region::Region(const Polygon& rPolygon)
:   mpB2DPolyPolygon(),
    mpPolyPolygon(),
    mpRegionBand(),
    mbIsNull(false)
{
    DBG_CHKOBJ( &rPolygon, Polygon, NULL );

    if(rPolygon.GetSize())
    {
        ImplCreatePolyPolyRegion(rPolygon);
    }
}

Region::Region(const PolyPolygon& rPolyPoly)
:   mpB2DPolyPolygon(),
    mpPolyPolygon(),
    mpRegionBand(),
    mbIsNull(false)
{
    DBG_CHKOBJ( &rPolyPoly, PolyPolygon, NULL );

    if(rPolyPoly.Count())
    {
        ImplCreatePolyPolyRegion(rPolyPoly);
    }
}

Region::Region(const basegfx::B2DPolyPolygon& rPolyPoly)
:   mpB2DPolyPolygon(),
    mpPolyPolygon(),
    mpRegionBand(),
    mbIsNull(false)
{
    DBG_CHKOBJ( &rPolyPoly, PolyPolygon, NULL );

    if(rPolyPoly.count())
    {
        ImplCreatePolyPolyRegion(rPolyPoly);
    }
}

Region::Region(const Region& rRegion)
:   mpB2DPolyPolygon(rRegion.mpB2DPolyPolygon),
    mpPolyPolygon(rRegion.mpPolyPolygon),
    mpRegionBand(rRegion.mpRegionBand),
    mbIsNull(rRegion.mbIsNull)
{
}

Region::~Region()
{
}

void Region::ImplCreatePolyPolyRegion( const PolyPolygon& rPolyPoly )
{
    const sal_uInt16 nPolyCount = rPolyPoly.Count();

    if(nPolyCount)
    {
        // polypolygon empty? -> empty region
        const Rectangle aRect(rPolyPoly.GetBoundRect());

        if(!aRect.IsEmpty())
        {
            // width OR height == 1 ? => Rectangular region
            if((1 == aRect.GetWidth()) || (1 == aRect.GetHeight()) || rPolyPoly.IsRect())
            {
                mpRegionBand.reset(new RegionBand(aRect));
            }
            else
            {
                mpPolyPolygon.reset(new PolyPolygon(rPolyPoly));
            }

            mbIsNull = false;
        }
    }
}

void Region::ImplCreatePolyPolyRegion( const basegfx::B2DPolyPolygon& rPolyPoly )
{
    if(rPolyPoly.count() && !rPolyPoly.getB2DRange().isEmpty())
    {
        mpB2DPolyPolygon.reset(new basegfx::B2DPolyPolygon(rPolyPoly));
        mbIsNull = false;
    }
}

void Region::Move( long nHorzMove, long nVertMove )
{
    if(IsNull() || IsEmpty())
    {
        // empty or null need no move
        return;
    }

    if(!nHorzMove && !nVertMove)
    {
        // no move defined
        return;
    }

    if(getB2DPolyPolygon())
    {
        basegfx::B2DPolyPolygon aPoly(*getB2DPolyPolygon());

        aPoly.transform(basegfx::tools::createTranslateB2DHomMatrix(nHorzMove, nVertMove));
        mpB2DPolyPolygon.reset(aPoly.count() ? new basegfx::B2DPolyPolygon(aPoly) : 0);
        mpPolyPolygon.reset();
        mpRegionBand.reset();
    }
    else if(getPolyPolygon())
    {
        PolyPolygon aPoly(*getPolyPolygon());

        aPoly.Move(nHorzMove, nVertMove);
        mpB2DPolyPolygon.reset();
        mpPolyPolygon.reset(aPoly.Count() ? new PolyPolygon(aPoly) : 0);
        mpRegionBand.reset();
    }
    else if(getRegionBand())
    {
        RegionBand* pNew = new RegionBand(*getRegionBand());

        pNew->Move(nHorzMove, nVertMove);
        mpB2DPolyPolygon.reset();
        mpPolyPolygon.reset();
        mpRegionBand.reset(pNew);
    }
    else
    {
        OSL_ENSURE(false, "Region::Move error: impossible combination (!)");
    }
}

void Region::Scale( double fScaleX, double fScaleY )
{
    if(IsNull() || IsEmpty())
    {
        // empty or null need no scale
        return;
    }

    if(basegfx::fTools::equalZero(fScaleX) && basegfx::fTools::equalZero(fScaleY))
    {
        // no scale defined
        return;
    }

    if(getB2DPolyPolygon())
    {
        basegfx::B2DPolyPolygon aPoly(*getB2DPolyPolygon());

        aPoly.transform(basegfx::tools::createScaleB2DHomMatrix(fScaleX, fScaleY));
        mpB2DPolyPolygon.reset(aPoly.count() ? new basegfx::B2DPolyPolygon(aPoly) : 0);
        mpPolyPolygon.reset();
        mpRegionBand.reset();
    }
    else if(getPolyPolygon())
    {
        PolyPolygon aPoly(*getPolyPolygon());

        aPoly.Scale(fScaleX, fScaleY);
        mpB2DPolyPolygon.reset();
        mpPolyPolygon.reset(aPoly.Count() ? new PolyPolygon(aPoly) : 0);
        mpRegionBand.reset();
    }
    else if(getRegionBand())
    {
        RegionBand* pNew = new RegionBand(*getRegionBand());

        pNew->Scale(fScaleX, fScaleY);
        mpB2DPolyPolygon.reset();
        mpPolyPolygon.reset();
        mpRegionBand.reset(pNew);
    }
    else
    {
        OSL_ENSURE(false, "Region::Scale error: impossible combination (!)");
    }
}

bool Region::Union( const Rectangle& rRect )
{
    if(rRect.IsEmpty())
    {
        // empty rectangle will not expand the existing union, nothing to do
        return true;
    }

    if(IsEmpty())
    {
        // no local data, the union will be equal to source. Create using rectangle
        *this = rRect;
        return true;
    }

    if(HasPolyPolygonOrB2DPolyPolygon())
    {
        // get this B2DPolyPolygon, solve on polygon base
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation(aThisPolyPoly);

        if(!aThisPolyPoly.count())
        {
            // no local polygon, use the rectangle as new region
            *this = rRect;
        }
        else
        {
            // get the other B2DPolyPolygon and use logical Or-Operation
            const basegfx::B2DPolygon aRectPoly(
                basegfx::tools::createPolygonFromRect(
                    basegfx::B2DRectangle(
                        rRect.Left(),
                        rRect.Top(),
                        rRect.Right(),
                        rRect.Bottom())));
            const basegfx::B2DPolyPolygon aClip(
                basegfx::tools::solvePolygonOperationOr(
                    aThisPolyPoly,
                    basegfx::B2DPolyPolygon(aRectPoly)));
            *this = Region(aClip);
        }

        return true;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // no region band, create using the rectangle
        *this = rRect;
        return true;
    }

    RegionBand* pNew = new RegionBand(*pCurrent);

    // get justified rectangle
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));

    // insert bands if the boundaries are not already in the list
    pNew->InsertBands(nTop, nBottom);

    // process union
    pNew->Union(nLeft, nTop, nRight, nBottom);

    // cleanup
    if(!pNew->OptimizeBandList())
    {
        delete pNew;
        pNew = 0;
    }

    mpRegionBand.reset(pNew);
    return true;
}

bool Region::Intersect( const Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
    {
        // empty rectangle will create empty region
        SetEmpty();
        return true;
    }

    if(IsNull())
    {
        // null region (everything) intersect with rect will give rect
        *this = rRect;
        return true;
    }

    if(IsEmpty())
    {
        // no content, cannot get more empty
        return true;
    }

    if(HasPolyPolygonOrB2DPolyPolygon())
    {
        // if polygon data prefer double precision, the other will be lost (if buffered)
        if(getB2DPolyPolygon())
        {
            const basegfx::B2DPolyPolygon aPoly(
                basegfx::tools::clipPolyPolygonOnRange(
                    *getB2DPolyPolygon(),
                    basegfx::B2DRange(
                        rRect.Left(),
                        rRect.Top(),
                        rRect.Right() + 1,
                        rRect.Bottom() + 1),
                    true,
                    false));

            mpB2DPolyPolygon.reset(aPoly.count() ? new basegfx::B2DPolyPolygon(aPoly) : 0);
            mpPolyPolygon.reset();
            mpRegionBand.reset();
        }
        else // if(getPolyPolygon())
        {
            PolyPolygon aPoly(*getPolyPolygon());

            // use the PolyPolygon::Clip method for rectangles, this is
            // fairly simple (does not even use GPC) and saves us from
            // unnecessary banding
            aPoly.Clip(rRect);

            mpB2DPolyPolygon.reset();
            mpPolyPolygon.reset(aPoly.Count() ? new PolyPolygon(aPoly) : 0);
            mpRegionBand.reset();
        }

        return true;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // region is empty -> nothing to do!
        return true;
    }

    RegionBand* pNew = new RegionBand(*pCurrent);

    // get justified rectangle
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));

    // insert bands if the boundaries are not allready in the list
    pNew->InsertBands(nTop, nBottom);

    // process intersect
    pNew->Intersect(nLeft, nTop, nRight, nBottom);

    // cleanup
    if(!pNew->OptimizeBandList())
    {
        delete pNew;
        pNew = 0;
    }

    mpRegionBand.reset(pNew);
    return true;
}

bool Region::Exclude( const Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
    {
        // excluding nothing will do no change
        return true;
    }

    if(IsEmpty())
    {
        // cannot exclude from empty, done
        return true;
    }

    if(IsNull())
    {
        // error; cannnot exclude from null region since this is not representable
        // in the data
        OSL_ENSURE(false, "Region::Exclude error: Cannot exclude from null region (!)");
        return true;
    }

    if( HasPolyPolygonOrB2DPolyPolygon() )
    {
        // get this B2DPolyPolygon
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation(aThisPolyPoly);

        if(!aThisPolyPoly.count())
        {
            // when local polygon is empty, nothing can be excluded
            return true;
        }

        // get the other B2DPolyPolygon
        const basegfx::B2DPolygon aRectPoly(
            basegfx::tools::createPolygonFromRect(
                basegfx::B2DRectangle(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom())));
        const basegfx::B2DPolyPolygon aOtherPolyPoly(aRectPoly);
        const basegfx::B2DPolyPolygon aClip = basegfx::tools::solvePolygonOperationDiff(aThisPolyPoly, aOtherPolyPoly);

        *this = Region(aClip);

        return true;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // empty? -> done!
        return true;
    }

    RegionBand* pNew = new RegionBand(*pCurrent);

    // get justified rectangle
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));

    // insert bands if the boundaries are not allready in the list
    pNew->InsertBands(nTop, nBottom);

    // process exclude
    pNew->Exclude(nLeft, nTop, nRight, nBottom);

    // cleanup
    if(!pNew->OptimizeBandList())
    {
        delete pNew;
        pNew = 0;
    }

    mpRegionBand.reset(pNew);
    return true;
}

bool Region::XOr( const Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
    {
        // empty rectangle will not change local content
        return true;
    }

    if(IsEmpty())
    {
        // rRect will be the xored-form (local off, rect on)
        *this = rRect;
        return true;
    }

    if(IsNull())
    {
        // error; cannnot exclude from null region since this is not representable
        // in the data
        OSL_ENSURE(false, "Region::XOr error: Cannot XOr with null region (!)");
        return true;
    }

    if( HasPolyPolygonOrB2DPolyPolygon() )
    {
        // get this B2DPolyPolygon
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation( aThisPolyPoly );

        if(!aThisPolyPoly.count())
        {
            // no local content, XOr will be equal to rectangle
            *this = rRect;
            return true;
        }

        // get the other B2DPolyPolygon
        const basegfx::B2DPolygon aRectPoly(
            basegfx::tools::createPolygonFromRect(
                basegfx::B2DRectangle(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom())));
        const basegfx::B2DPolyPolygon aOtherPolyPoly(aRectPoly);
        const basegfx::B2DPolyPolygon aClip = basegfx::tools::solvePolygonOperationXor(aThisPolyPoly, aOtherPolyPoly);

        *this = Region(aClip);

        return true;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // rRect will be the xored-form (local off, rect on)
        *this = rRect;
        return true;
    }

    // only region band mode possibility left here or null/empty
    RegionBand* pNew = new RegionBand(*getRegionBand());

    // get justified rectangle
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));

    // insert bands if the boundaries are not allready in the list
    pNew->InsertBands(nTop, nBottom);

    // process xor
    pNew->XOr(nLeft, nTop, nRight, nBottom);

    // cleanup
    if(!pNew->OptimizeBandList())
    {
        delete pNew;
        pNew = 0;
    }

    mpRegionBand.reset(pNew);
    return true;
}

bool Region::Union( const Region& rRegion )
{
    if(rRegion.IsEmpty())
    {
        // no extension at all
        return true;
    }

    if(rRegion.IsNull())
    {
        // extending with null region -> null region
        *this = Region(true);
        return true;
    }

    if(IsEmpty())
    {
        // local is empty, union will give source region
        *this = rRegion;
        return true;
    }

    if(IsNull())
    {
        // already fully expanded (is null region), cannot be extended
        return true;
    }

    if( rRegion.HasPolyPolygonOrB2DPolyPolygon() || HasPolyPolygonOrB2DPolyPolygon() )
    {
        // get this B2DPolyPolygon
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation(aThisPolyPoly);

        if(!aThisPolyPoly.count())
        {
            // when no local content, union will be equal to rRegion
            *this = rRegion;
            return true;
        }

        // get the other B2DPolyPolygon
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());
        aOtherPolyPoly = basegfx::tools::prepareForPolygonOperation(aOtherPolyPoly);

        // use logical OR operation
        basegfx::B2DPolyPolygon aClip(basegfx::tools::solvePolygonOperationOr(aThisPolyPoly, aOtherPolyPoly));

        *this = Region( aClip );
        return true;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // local is empty, union will give source region
        *this = rRegion;
        return true;
    }

    const RegionBand* pSource = rRegion.getRegionBand();

    if(!pSource)
    {
        // no extension at all
        return true;
    }

    // prepare source and target
    RegionBand* pNew = new RegionBand(*pCurrent);

    // union with source
    pNew->Union(*pSource);

    // cleanup
    if(!pNew->OptimizeBandList())
    {
        delete pNew;
        pNew = 0;
    }

    mpRegionBand.reset(pNew);
    return true;
}

bool Region::Intersect( const Region& rRegion )
{
    // same instance data? -> nothing to do!
    if(getB2DPolyPolygon() && getB2DPolyPolygon() == rRegion.getB2DPolyPolygon())
    {
        return true;
    }

    if(getPolyPolygon() && getPolyPolygon() == rRegion.getPolyPolygon())
    {
        return true;
    }

    if(getRegionBand() && getRegionBand() == rRegion.getRegionBand())
    {
        return true;
    }

    if(rRegion.IsNull())
    {
        // source region is null-region, intersect will not change local region
        return true;
    }

    if(IsNull())
    {
        // when local region is null-region, intersect will be equal to source
        *this = rRegion;
        return true;
    }

    if(rRegion.IsEmpty())
    {
        // source region is empty, intersection will always be empty
        SetEmpty();
        return true;
    }

    if(IsEmpty())
    {
        // local region is empty, cannot get more emty than that. Nothing to do
        return true;
    }

    if( rRegion.HasPolyPolygonOrB2DPolyPolygon() || HasPolyPolygonOrB2DPolyPolygon() )
    {
        // get this B2DPolyPolygon
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        if(!aThisPolyPoly.count())
        {
            // local region is empty, cannot get more emty than that. Nothing to do
            return true;
        }

        // get the other B2DPolyPolygon
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());

        if(!aOtherPolyPoly.count())
        {
            // source region is empty, intersection will always be empty
            SetEmpty();
            return true;
        }

        const basegfx::B2DPolyPolygon aClip(
            basegfx::tools::clipPolyPolygonOnPolyPolygon(
                aOtherPolyPoly,
                aThisPolyPoly,
                true,
                false));
        *this = Region( aClip );
        return true;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // local region is empty, cannot get more emty than that. Nothing to do
        return true;
    }

    const RegionBand* pSource = rRegion.getRegionBand();

    if(!pSource)
    {
        // source region is empty, intersection will always be empty
        SetEmpty();
        return true;
    }

    // both RegionBands exist and are not empty
    if(pCurrent->getRectangleCount() + 2 < pSource->getRectangleCount())
    {
        // when we have less rectangles, turn around the call
        Region aTempRegion = rRegion;
        aTempRegion.Intersect( *this );
        *this = aTempRegion;
    }
    else
    {
        // prepare new regionBand
        RegionBand* pNew = pCurrent ? new RegionBand(*pCurrent) : new RegionBand();

        // intersect with source
        pNew->Intersect(*pSource);

        // cleanup
        if(!pNew->OptimizeBandList())
        {
            delete pNew;
            pNew = 0;
        }

        mpRegionBand.reset(pNew);
    }

    return true;
}

bool Region::Exclude( const Region& rRegion )
{
    if ( rRegion.IsEmpty() )
    {
        // excluding nothing will do no change
        return true;
    }

    if ( rRegion.IsNull() )
    {
        // excluding everything will create empty region
        SetEmpty();
        return true;
    }

    if(IsEmpty())
    {
        // cannot exclude from empty, done
        return true;
    }

    if(IsNull())
    {
        // error; cannnot exclude from null region since this is not representable
        // in the data
        OSL_ENSURE(false, "Region::Exclude error: Cannot exclude from null region (!)");
        return true;
    }

    if( rRegion.HasPolyPolygonOrB2DPolyPolygon() || HasPolyPolygonOrB2DPolyPolygon() )
    {
        // get this B2DPolyPolygon
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        if(!aThisPolyPoly.count())
        {
            // cannot exclude from empty, done
            return true;
        }

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation( aThisPolyPoly );

        // get the other B2DPolyPolygon
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());
        aOtherPolyPoly = basegfx::tools::prepareForPolygonOperation( aOtherPolyPoly );

        basegfx::B2DPolyPolygon aClip = basegfx::tools::solvePolygonOperationDiff( aThisPolyPoly, aOtherPolyPoly );
        *this = Region( aClip );
        return true;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // cannot exclude from empty, done
        return true;
    }

    const RegionBand* pSource = rRegion.getRegionBand();

    if(!pSource)
    {
        // excluding nothing will do no change
        return true;
    }

    // prepare source and target
    RegionBand* pNew = new RegionBand(*pCurrent);

    // union with source
    const bool bSuccess(pNew->Exclude(*pSource));

    // cleanup
    if(!bSuccess)
    {
        delete pNew;
        pNew = 0;
    }

    mpRegionBand.reset(pNew);
    return true;
}

bool Region::XOr( const Region& rRegion )
{
    if ( rRegion.IsEmpty() )
    {
        // empty region will not change local content
        return true;
    }

    if ( rRegion.IsNull() )
    {
        // error; cannnot exclude null region from local since this is not representable
        // in the data
        OSL_ENSURE(false, "Region::XOr error: Cannot XOr with null region (!)");
        return true;
    }

    if(IsEmpty())
    {
        // rRect will be the xored-form (local off, rect on)
        *this = rRegion;
        return true;
    }

    if(IsNull())
    {
        // error; cannnot exclude from null region since this is not representable
        // in the data
        OSL_ENSURE(false, "Region::XOr error: Cannot XOr with null region (!)");
        return false;
    }

    if( rRegion.HasPolyPolygonOrB2DPolyPolygon() || HasPolyPolygonOrB2DPolyPolygon() )
    {
        // get this B2DPolyPolygon
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        if(!aThisPolyPoly.count())
        {
            // rRect will be the xored-form (local off, rect on)
            *this = rRegion;
            return true;
        }

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation( aThisPolyPoly );

        // get the other B2DPolyPolygon
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());
        aOtherPolyPoly = basegfx::tools::prepareForPolygonOperation( aOtherPolyPoly );

        basegfx::B2DPolyPolygon aClip = basegfx::tools::solvePolygonOperationXor( aThisPolyPoly, aOtherPolyPoly );
        *this = Region( aClip );
        return true;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // rRect will be the xored-form (local off, rect on)
        *this = rRegion;
        return true;
    }

    const RegionBand* pSource = rRegion.getRegionBand();

    if(!pSource)
    {
        // empty region will not change local content
        return true;
    }

    // prepare source and target
    RegionBand* pNew = new RegionBand(*pCurrent);

    // union with source
    pNew->XOr(*pSource);

    // cleanup
    if(!pNew->OptimizeBandList())
    {
        delete pNew;
        pNew = 0;
    }

    mpRegionBand.reset(pNew);

    return true;
}

Rectangle Region::GetBoundRect() const
{
    if(IsEmpty())
    {
        // no internal data? -> region is empty!
        return Rectangle();
    }

    if(IsNull())
    {
        // error; null region has no BoundRect
        // OSL_ENSURE(false, "Region::GetBoundRect error: null region has unlimitied bound rect, not representable (!)");
        return Rectangle();
    }

    // prefer double precision source
    if(getB2DPolyPolygon())
    {
        const basegfx::B2DRange aRange(basegfx::tools::getRange(*getB2DPolyPolygon()));

        if(aRange.isEmpty())
        {
            // emulate PolyPolygon::GetBoundRect() when empty polygon
            return Rectangle();
        }
        else
        {
            // #i122149# corrected rounding, no need for ceil() and floor() here
            return Rectangle(
                basegfx::fround(aRange.getMinX()), basegfx::fround(aRange.getMinY()),
                basegfx::fround(aRange.getMaxX()), basegfx::fround(aRange.getMaxY()));
        }
    }

    if(getPolyPolygon())
    {
        return getPolyPolygon()->GetBoundRect();
    }

    if(getRegionBand())
    {
        return getRegionBand()->GetBoundRect();
    }

    return Rectangle();
}

const PolyPolygon Region::GetAsPolyPolygon() const
{
    if(getPolyPolygon())
    {
        return *getPolyPolygon();
    }

    if(getB2DPolyPolygon())
    {
        // the polygon needs to be converted, buffer the down converion
        const PolyPolygon aPolyPolgon(*getB2DPolyPolygon());
        const_cast< Region* >(this)->mpPolyPolygon.reset(new PolyPolygon(aPolyPolgon));

        return *getPolyPolygon();
    }

    if(getRegionBand())
    {
        // the BandRegion needs to be converted, buffer the converion
        const PolyPolygon aPolyPolgon(ImplCreatePolyPolygonFromRegionBand());
        const_cast< Region* >(this)->mpPolyPolygon.reset(new PolyPolygon(aPolyPolgon));

        return *getPolyPolygon();
    }

    return PolyPolygon();
}

const basegfx::B2DPolyPolygon Region::GetAsB2DPolyPolygon() const
{
    if(getB2DPolyPolygon())
    {
        return *getB2DPolyPolygon();
    }

    if(getPolyPolygon())
    {
        // the polygon needs to be converted, buffer the up conversion. This will be preferred from now.
        const basegfx::B2DPolyPolygon aB2DPolyPolygon(getPolyPolygon()->getB2DPolyPolygon());
        const_cast< Region* >(this)->mpB2DPolyPolygon.reset(new basegfx::B2DPolyPolygon(aB2DPolyPolygon));

        return *getB2DPolyPolygon();
    }

    if(getRegionBand())
    {
        // the BandRegion needs to be converted, buffer the converion
        const basegfx::B2DPolyPolygon aB2DPolyPolygon(ImplCreateB2DPolyPolygonFromRegionBand());
        const_cast< Region* >(this)->mpB2DPolyPolygon.reset(new basegfx::B2DPolyPolygon(aB2DPolyPolygon));

        return *getB2DPolyPolygon();
    }

    return basegfx::B2DPolyPolygon();
}

const RegionBand* Region::GetAsRegionBand() const
{
    if(!getRegionBand())
    {
        if(getB2DPolyPolygon())
        {
            // convert B2DPolyPolygon to RegionBand, buffer it and return it
            const_cast< Region* >(this)->mpRegionBand.reset(ImplCreateRegionBandFromPolyPolygon(PolyPolygon(*getB2DPolyPolygon())));
        }
        else if(getPolyPolygon())
        {
            // convert B2DPolyPolygon to RegionBand, buffer it and return it
            const_cast< Region* >(this)->mpRegionBand.reset(ImplCreateRegionBandFromPolyPolygon(*getPolyPolygon()));
        }
    }

    return getRegionBand();
}

bool Region::IsInside( const Point& rPoint ) const
{
    if(IsEmpty())
    {
        // no point can be in empty region
        return false;
    }

    if(IsNull())
    {
        // all points are inside null-region
        return true;
    }

    // Too expensive (?)
    //if(mpImplRegion->getRegionPolyPoly())
    //{
    //  return mpImplRegion->getRegionPolyPoly()->IsInside( rPoint );
    //}

    // ensure RegionBand existance
    const RegionBand* pRegionBand = GetAsRegionBand();

    if(pRegionBand)
    {
        return pRegionBand->IsInside(rPoint);
    }

    return false;
}

bool Region::IsInside( const Rectangle& rRect ) const
{
    if(IsEmpty())
    {
        // no rectangle can be in empty region
        return false;
    }

    if(IsNull())
    {
        // rectangle always inside null-region
        return true;
    }

    if ( rRect.IsEmpty() )
    {
        // is rectangle empty? -> not inside
        return false;
    }

    // create region from rectangle and intersect own region
    Region aRegion(rRect);
    aRegion.Exclude(*this);

    // rectangle is inside if exclusion is empty
    return aRegion.IsEmpty();
}

// -----------------------------------------------------------------------

bool Region::IsOver( const Rectangle& rRect ) const
{
    if(IsEmpty())
    {
        // nothing can be over something empty
        return false;
    }

    if(IsNull())
    {
        // everything is over null region
        return true;
    }

    // Can we optimize this ??? - is used in StarDraw for brushes pointers
    // Why we have no IsOver for Regions ???
    // create region from rectangle and intersect own region
    Region aRegion(rRect);
    aRegion.Intersect( *this );

    // rectangle is over if include is not empty
    return !aRegion.IsEmpty();
}

void Region::SetNull()
{
    // reset all content
    mpB2DPolyPolygon.reset();
    mpPolyPolygon.reset();
    mpRegionBand.reset();
    mbIsNull = true;
}

void Region::SetEmpty()
{
    // reset all content
    mpB2DPolyPolygon.reset();
    mpPolyPolygon.reset();
    mpRegionBand.reset();
    mbIsNull = false;
}

Region& Region::operator=( const Region& rRegion )
{
    // reset all content
    mpB2DPolyPolygon = rRegion.mpB2DPolyPolygon;
    mpPolyPolygon = rRegion.mpPolyPolygon;
    mpRegionBand = rRegion.mpRegionBand;
    mbIsNull = rRegion.mbIsNull;

    return *this;
}

Region& Region::operator=( const Rectangle& rRect )
{
    mpB2DPolyPolygon.reset();
    mpPolyPolygon.reset();
    mpRegionBand.reset(rRect.IsEmpty() ? 0 : new RegionBand(rRect));
    mbIsNull = false;

    return *this;
}

bool Region::operator==( const Region& rRegion ) const
{
    if(IsNull() && rRegion.IsNull())
    {
        // both are null region
        return true;
    }

    if(IsEmpty() && rRegion.IsEmpty())
    {
        // both are empty
        return true;
    }

    if(getB2DPolyPolygon() && getB2DPolyPolygon() == rRegion.getB2DPolyPolygon())
    {
        // same instance data? -> equal
        return true;
    }

    if(getPolyPolygon() && getPolyPolygon() == rRegion.getPolyPolygon())
    {
        // same instance data? -> equal
        return true;
    }

    if(getRegionBand() && getRegionBand() == rRegion.getRegionBand())
    {
        // same instance data? -> equal
        return true;
    }

    if(IsNull() || IsEmpty())
    {
        return false;
    }

    if(rRegion.IsNull() || rRegion.IsEmpty())
    {
        return false;
    }

    if(rRegion.getB2DPolyPolygon() || getB2DPolyPolygon())
    {
        // one of both has a B2DPolyPolygon based region, ensure both have it
        // by evtl. conversion
        const_cast< Region* >(this)->GetAsB2DPolyPolygon();
        const_cast< Region& >(rRegion).GetAsB2DPolyPolygon();

        return *rRegion.getB2DPolyPolygon() == *getB2DPolyPolygon();
    }

    if(rRegion.getPolyPolygon() || getPolyPolygon())
    {
        // one of both has a B2DPolyPolygon based region, ensure both have it
        // by evtl. conversion
        const_cast< Region* >(this)->GetAsPolyPolygon();
        const_cast< Region& >(rRegion).GetAsPolyPolygon();

        return *rRegion.getPolyPolygon() == *getPolyPolygon();
    }

    // both are not empty or null (see above) and if content supported polygon
    // data the comparison is already done. Only both on RegionBand base can be left,
    // but better check
    if(rRegion.getRegionBand() && getRegionBand())
    {
        return *rRegion.getRegionBand() == *getRegionBand();
    }

    // should not happen, but better deny equality
    return false;
}

SvStream& operator>>(SvStream& rIStrm, Region& rRegion)
{
    VersionCompat aCompat(rIStrm, STREAM_READ);
    sal_uInt16 nVersion(0);
    sal_uInt16 nTmp16(0);

    // clear region to be loaded
    rRegion.SetEmpty();

    // get version of streamed region
    rIStrm >> nVersion;

    // get type of region
    rIStrm >> nTmp16;

    enum RegionType { REGION_NULL, REGION_EMPTY, REGION_RECTANGLE, REGION_COMPLEX };
    RegionType meStreamedType = (RegionType)nTmp16;

    switch(meStreamedType)
    {
        case REGION_NULL:
        {
            rRegion.SetNull();
            break;
        }

        case REGION_EMPTY:
        {
            rRegion.SetEmpty();
            break;
        }

        default:
        {
            RegionBand* pNewRegionBand = new RegionBand();
            pNewRegionBand->load(rIStrm);
            rRegion.mpRegionBand.reset(pNewRegionBand);

            if(aCompat.GetVersion() >= 2)
            {
                sal_Bool bHasPolyPolygon(sal_False);

                rIStrm >> bHasPolyPolygon;

                if(bHasPolyPolygon)
                {
                    PolyPolygon* pNewPoly = new PolyPolygon();
                    rIStrm >> *pNewPoly;
                    rRegion.mpPolyPolygon.reset(pNewPoly);
                }
            }

            break;
        }
    }

    return rIStrm;
}

SvStream& operator<<( SvStream& rOStrm, const Region& rRegion )
{
    const sal_uInt16 nVersion(2);
    VersionCompat aCompat(rOStrm, STREAM_WRITE, nVersion);

    // put version
    rOStrm << nVersion;

    // put type
    enum RegionType { REGION_NULL, REGION_EMPTY, REGION_RECTANGLE, REGION_COMPLEX };
    RegionType aRegionType(REGION_COMPLEX);
    bool bEmpty(rRegion.IsEmpty());

    if(!bEmpty && rRegion.getB2DPolyPolygon() && 0 == rRegion.getB2DPolyPolygon()->count())
    {
        OSL_ENSURE(false, "Region with empty B2DPolyPolygon, should not be created (!)");
        bEmpty = true;
    }

    if(!bEmpty && rRegion.getPolyPolygon() && 0 == rRegion.getPolyPolygon()->Count())
    {
        OSL_ENSURE(false, "Region with empty PolyPolygon, should not be created (!)");
        bEmpty = true;
    }

    if(bEmpty)
    {
        aRegionType = REGION_EMPTY;
    }
    else if(rRegion.IsNull())
    {
        aRegionType = REGION_NULL;
    }
    else if(rRegion.getRegionBand() && rRegion.getRegionBand()->isSingleRectangle())
    {
        aRegionType = REGION_RECTANGLE;
    }

    rOStrm << (sal_uInt16)aRegionType;

    // get RegionBand
    const RegionBand* pRegionBand = rRegion.getRegionBand();

    if(pRegionBand)
    {
        pRegionBand->save(rOStrm);
    }
    else
    {
        // for compatibility, write an empty RegionBand (will only write
        // the end marker STREAMENTRY_END, but this *is* needed)
        const RegionBand aRegionBand;

        aRegionBand.save(rOStrm);
    }

    // write polypolygon if available
    const sal_Bool bHasPolyPolygon(rRegion.HasPolyPolygonOrB2DPolyPolygon());
    rOStrm << bHasPolyPolygon;

    if(bHasPolyPolygon)
    {
        // #i105373#
        PolyPolygon aNoCurvePolyPolygon;
        rRegion.GetAsPolyPolygon().AdaptiveSubdivide(aNoCurvePolyPolygon);

        rOStrm << aNoCurvePolyPolygon;
    }

    return rOStrm;
}

void Region::GetRegionRectangles(RectangleVector& rTarget) const
{
    // clear returnvalues
    rTarget.clear();

    // ensure RegionBand existance
    const RegionBand* pRegionBand = GetAsRegionBand();

    if(pRegionBand)
    {
        pRegionBand->GetRegionRectangles(rTarget);
    }
}

static inline bool ImplPolygonRectTest( const Polygon& rPoly, Rectangle* pRectOut = NULL )
{
    bool bIsRect = false;
    const Point* pPoints = rPoly.GetConstPointAry();
    sal_uInt16 nPoints = rPoly.GetSize();

    if( nPoints == 4 || (nPoints == 5 && pPoints[0] == pPoints[4]) )
    {
        long nX1 = pPoints[0].X(), nX2 = pPoints[2].X(), nY1 = pPoints[0].Y(), nY2 = pPoints[2].Y();

        if( ( (pPoints[1].X() == nX1 && pPoints[3].X() == nX2) && (pPoints[1].Y() == nY2 && pPoints[3].Y() == nY1) )
         || ( (pPoints[1].X() == nX2 && pPoints[3].X() == nX1) && (pPoints[1].Y() == nY1 && pPoints[3].Y() == nY2) ) )
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
                {
                    nX2--;
                }

                if( nY2 != nY1 )
                {
                    nY2--;
                }

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
    // and ImplCreateRegionBandFromPolyPolygon is at half rectangles/half polygons
    int nPolygonRects = 0, nPolygonPolygons = 0;
    int nPolygons = rPolyPoly.Count();

    for( sal_uInt16 i = 0; i < nPolygons; i++ )
    {
        const Polygon& rPoly = rPolyPoly[i];

        if( ImplPolygonRectTest( rPoly ) )
        {
            nPolygonRects++;
        }
        else
        {
            nPolygonPolygons++;
        }
    }

    if( nPolygonPolygons > nPolygonRects )
    {
        return Region( rPolyPoly );
    }

    Region aResult;
    Rectangle aRect;

    for( sal_uInt16 i = 0; i < nPolygons; i++ )
    {
        const Polygon& rPoly = rPolyPoly[i];

        if( ImplPolygonRectTest( rPoly, &aRect ) )
        {
            aResult.XOr( aRect );
        }
        else
        {
            aResult.XOr( Region(rPoly) );
        }
    }

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
