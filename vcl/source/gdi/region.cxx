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

#include <memory>
#include <limits.h>
#include <tools/vcompat.hxx>
#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <vcl/region.hxx>
#include <regionband.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <tools/poly.hxx>

namespace
{
    /** Return <TRUE/> when the given polygon is rectilinear and oriented so that
        all sides are either horizontal or vertical.
    */
    bool ImplIsPolygonRectilinear (const tools::PolyPolygon& rPolyPoly)
    {
        // Iterate over all polygons.
        const sal_uInt16 nPolyCount = rPolyPoly.Count();
        for (sal_uInt16 nPoly = 0; nPoly < nPolyCount; ++nPoly)
        {
            const tools::Polygon&  aPoly = rPolyPoly.GetObject(nPoly);

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
    std::unique_ptr<RegionBand> ImplRectilinearPolygonToBands(const tools::PolyPolygon& rPolyPoly)
    {
        OSL_ASSERT(ImplIsPolygonRectilinear (rPolyPoly));

        // Create a new RegionBand object as container of the bands.
        std::unique_ptr<RegionBand> pRegionBand( std::make_unique<RegionBand>() );
        long nLineId = 0;

        // Iterate over all polygons.
        const sal_uInt16 nPolyCount = rPolyPoly.Count();
        for (sal_uInt16 nPoly = 0; nPoly < nPolyCount; ++nPoly)
        {
            const tools::Polygon&  aPoly = rPolyPoly.GetObject(nPoly);

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
                // direction separately.  The direction is calculated as it is
                // in other places (but seems to be the wrong way.)
                const long nTop (::std::min(aStart.Y(), aEnd.Y()));
                const long nBottom (::std::max(aStart.Y(), aEnd.Y()));
                const LineType eLineType (aStart.Y() > aEnd.Y() ? LineType::Descending : LineType::Ascending);

                // Make sure that the current line is covered by bands.
                pRegionBand->ImplAddMissingBands(nTop,nBottom);

                // Find top-most band that may contain nTop.
                ImplRegionBand* pBand = pRegionBand->ImplGetFirstRegionBand();
                while (pBand!=nullptr && pBand->mnYBottom < nTop)
                    pBand = pBand->mpNextBand;
                ImplRegionBand* pTopBand = pBand;
                // If necessary split the band at nTop so that nTop is contained
                // in the lower band.
                if (pBand!=nullptr
                       // Prevent the current band from becoming 0 pixel high
                    && pBand->mnYTop<nTop
                       // this allows the lowest pixel of the band to be split off
                    && pBand->mnYBottom>=nTop
                       // do not split a band that is just one pixel high
                    && pBand->mnYTop<pBand->mnYBottom-1)
                {
                    // Split the top band.
                    pTopBand = pBand->SplitBand(nTop);
                }

                // Advance to band that may contain nBottom.
                while (pBand!=nullptr && pBand->mnYBottom < nBottom)
                    pBand = pBand->mpNextBand;
                // The lowest band may have to be split at nBottom so that
                // nBottom itself remains in the upper band.
                if (pBand!=nullptr
                       // allow the current band becoming 1 pixel high
                    && pBand->mnYTop<=nBottom
                       // prevent splitting off a band that is 0 pixel high
                    && pBand->mnYBottom>nBottom
                       // do not split a band that is just one pixel high
                    && pBand->mnYTop<pBand->mnYBottom-1)
                {
                    // Split the bottom band.
                    pBand->SplitBand(nBottom+1);
                }

                // Note that we remember the top band (in pTopBand) but not the
                // bottom band.  The later can be determined by comparing y
                // coordinates.

                // Add the x-value as point to all bands in the nTop->nBottom range.
                for (pBand=pTopBand; pBand!=nullptr&&pBand->mnYTop<=nBottom; pBand=pBand->mpNextBand)
                    pBand->InsertPoint(aStart.X(), nLineId++, true, eLineType);
            }
        }

        return pRegionBand;
    }

    /** Convert a general polygon (one for which ImplIsPolygonRectilinear()
        returns <FALSE/>) to bands.
    */
    std::unique_ptr<RegionBand> ImplGeneralPolygonToBands(const tools::PolyPolygon& rPolyPoly, const tools::Rectangle& rPolygonBoundingBox)
    {
        long nLineID = 0;

        // initialisation and creation of Bands
        std::unique_ptr<RegionBand> pRegionBand( std::make_unique<RegionBand>() );
        pRegionBand->CreateBandRange(rPolygonBoundingBox.Top(), rPolygonBoundingBox.Bottom());

        // insert polygons
        const sal_uInt16 nPolyCount = rPolyPoly.Count();

        for ( sal_uInt16 nPoly = 0; nPoly < nPolyCount; nPoly++ )
        {
            // get reference to current polygon
            const tools::Polygon&  aPoly = rPolyPoly.GetObject( nPoly );
            const sal_uInt16    nSize = aPoly.GetSize();

            // not enough points ( <= 2 )? -> nothing to do!
            if ( nSize <= 2 )
                continue;

            // band the polygon
            for ( sal_uInt16 nPoint = 1; nPoint < nSize; nPoint++ )
            {
                pRegionBand->InsertLine( aPoly.GetPoint(nPoint-1), aPoly.GetPoint(nPoint), nLineID++ );
            }

            // close polygon with line from first point to last point, if necessary
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

namespace vcl {

bool vcl::Region::IsEmpty() const
{
    return !mbIsNull && !mpB2DPolyPolygon.get() && !mpPolyPolygon.get() && !mpRegionBand.get();
}


static std::unique_ptr<RegionBand> ImplCreateRegionBandFromPolyPolygon(const tools::PolyPolygon& rPolyPolygon)
{
    std::unique_ptr<RegionBand> pRetval;

    if(rPolyPolygon.Count())
    {
        // ensure to subdivide when bezier segments are used, it's going to
        // be expanded to rectangles
        tools::PolyPolygon aPolyPolygon;

        rPolyPolygon.AdaptiveSubdivide(aPolyPolygon);

        if(aPolyPolygon.Count())
        {
            const tools::Rectangle aRect(aPolyPolygon.GetBoundRect());

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
                        pRetval.reset();
                    }
                }
            }
        }
    }

    return pRetval;
}

tools::PolyPolygon vcl::Region::ImplCreatePolyPolygonFromRegionBand() const
{
    tools::PolyPolygon aRetval;

    if(getRegionBand())
    {
        RectangleVector aRectangles;
        GetRegionRectangles(aRectangles);

        for (auto const& rectangle : aRectangles)
        {
            aRetval.Insert( tools::Polygon(rectangle) );
        }
    }
    else
    {
        OSL_ENSURE(false, "Called with no local RegionBand (!)");
    }

    return aRetval;
}

basegfx::B2DPolyPolygon vcl::Region::ImplCreateB2DPolyPolygonFromRegionBand() const
{
    tools::PolyPolygon aPoly(ImplCreatePolyPolygonFromRegionBand());

    return aPoly.getB2DPolyPolygon();
}

Region::Region(bool bIsNull)
:   mpB2DPolyPolygon(),
    mpPolyPolygon(),
    mpRegionBand(),
    mbIsNull(bIsNull)
{
}

Region::Region(const tools::Rectangle& rRect)
:   mpB2DPolyPolygon(),
    mpPolyPolygon(),
    mpRegionBand(),
    mbIsNull(false)
{
    mpRegionBand.reset(rRect.IsEmpty() ? nullptr : new RegionBand(rRect));
}

Region::Region(const tools::Polygon& rPolygon)
:   mpB2DPolyPolygon(),
    mpPolyPolygon(),
    mpRegionBand(),
    mbIsNull(false)
{

    if(rPolygon.GetSize())
    {
        ImplCreatePolyPolyRegion(rPolygon);
    }
}

Region::Region(const tools::PolyPolygon& rPolyPoly)
:   mpB2DPolyPolygon(),
    mpPolyPolygon(),
    mpRegionBand(),
    mbIsNull(false)
{

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

    if(rPolyPoly.count())
    {
        ImplCreatePolyPolyRegion(rPolyPoly);
    }
}

Region::Region(const vcl::Region&) = default;

Region::Region(vcl::Region&& rRegion)
:   mpB2DPolyPolygon(std::move(rRegion.mpB2DPolyPolygon)),
    mpPolyPolygon(std::move(rRegion.mpPolyPolygon)),
    mpRegionBand(std::move(rRegion.mpRegionBand)),
    mbIsNull(rRegion.mbIsNull)
{
    rRegion.mbIsNull = true;
}

Region::~Region() = default;

void vcl::Region::ImplCreatePolyPolyRegion( const tools::PolyPolygon& rPolyPoly )
{
    const sal_uInt16 nPolyCount = rPolyPoly.Count();

    if(nPolyCount)
    {
        // polypolygon empty? -> empty region
        const tools::Rectangle aRect(rPolyPoly.GetBoundRect());

        if(!aRect.IsEmpty())
        {
            // width OR height == 1 ? => Rectangular region
            if((1 == aRect.GetWidth()) || (1 == aRect.GetHeight()) || rPolyPoly.IsRect())
            {
                mpRegionBand.reset(new RegionBand(aRect));
            }
            else
            {
                mpPolyPolygon.reset(new tools::PolyPolygon(rPolyPoly));
            }

            mbIsNull = false;
        }
    }
}

void vcl::Region::ImplCreatePolyPolyRegion( const basegfx::B2DPolyPolygon& rPolyPoly )
{
    if(rPolyPoly.count() && !rPolyPoly.getB2DRange().isEmpty())
    {
        mpB2DPolyPolygon.reset(new basegfx::B2DPolyPolygon(rPolyPoly));
        mbIsNull = false;
    }
}

void vcl::Region::Move( long nHorzMove, long nVertMove )
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

        aPoly.transform(basegfx::utils::createTranslateB2DHomMatrix(nHorzMove, nVertMove));
        mpB2DPolyPolygon.reset(aPoly.count() ? new basegfx::B2DPolyPolygon(aPoly) : nullptr);
        mpPolyPolygon.reset();
        mpRegionBand.reset();
    }
    else if(getPolyPolygon())
    {
        tools::PolyPolygon aPoly(*getPolyPolygon());

        aPoly.Move(nHorzMove, nVertMove);
        mpB2DPolyPolygon.reset();
        mpPolyPolygon.reset(aPoly.Count() ? new tools::PolyPolygon(aPoly) : nullptr);
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

void vcl::Region::Scale( double fScaleX, double fScaleY )
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

        aPoly.transform(basegfx::utils::createScaleB2DHomMatrix(fScaleX, fScaleY));
        mpB2DPolyPolygon.reset(aPoly.count() ? new basegfx::B2DPolyPolygon(aPoly) : nullptr);
        mpPolyPolygon.reset();
        mpRegionBand.reset();
    }
    else if(getPolyPolygon())
    {
        tools::PolyPolygon aPoly(*getPolyPolygon());

        aPoly.Scale(fScaleX, fScaleY);
        mpB2DPolyPolygon.reset();
        mpPolyPolygon.reset(aPoly.Count() ? new tools::PolyPolygon(aPoly) : nullptr);
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

void vcl::Region::Union( const tools::Rectangle& rRect )
{
    if(rRect.IsEmpty())
    {
        // empty rectangle will not expand the existing union, nothing to do
        return;
    }

    if(IsEmpty())
    {
        // no local data, the union will be equal to source. Create using rectangle
        *this = rRect;
        return;
    }

    if(HasPolyPolygonOrB2DPolyPolygon())
    {
        // get this B2DPolyPolygon, solve on polygon base
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::utils::prepareForPolygonOperation(aThisPolyPoly);

        if(!aThisPolyPoly.count())
        {
            // no local polygon, use the rectangle as new region
            *this = rRect;
        }
        else
        {
            // get the other B2DPolyPolygon and use logical Or-Operation
            const basegfx::B2DPolygon aRectPoly(
                basegfx::utils::createPolygonFromRect(
                    basegfx::B2DRectangle(
                        rRect.Left(),
                        rRect.Top(),
                        rRect.Right(),
                        rRect.Bottom())));
            const basegfx::B2DPolyPolygon aClip(
                basegfx::utils::solvePolygonOperationOr(
                    aThisPolyPoly,
                    basegfx::B2DPolyPolygon(aRectPoly)));
            *this = vcl::Region(aClip);
        }

        return;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // no region band, create using the rectangle
        *this = rRect;
        return;
    }

    std::unique_ptr<RegionBand> pNew( std::make_unique<RegionBand>(*pCurrent));

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
        pNew.reset();
    }

    mpRegionBand = std::move(pNew);
}

void vcl::Region::Intersect( const tools::Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
    {
        // empty rectangle will create empty region
        SetEmpty();
        return;
    }

    if(IsNull())
    {
        // null region (everything) intersect with rect will give rect
        *this = rRect;
        return;
    }

    if(IsEmpty())
    {
        // no content, cannot get more empty
        return;
    }

    if(HasPolyPolygonOrB2DPolyPolygon())
    {
        // if polygon data prefer double precision, the other will be lost (if buffered)
        if(getB2DPolyPolygon())
        {
            const basegfx::B2DPolyPolygon aPoly(
                basegfx::utils::clipPolyPolygonOnRange(
                    *getB2DPolyPolygon(),
                    basegfx::B2DRange(
                        rRect.Left(),
                        rRect.Top(),
                        rRect.Right() + 1,
                        rRect.Bottom() + 1),
                    true,
                    false));

            mpB2DPolyPolygon.reset(aPoly.count() ? new basegfx::B2DPolyPolygon(aPoly) : nullptr);
            mpPolyPolygon.reset();
            mpRegionBand.reset();
        }
        else // if(getPolyPolygon())
        {
            tools::PolyPolygon aPoly(*getPolyPolygon());

            // use the PolyPolygon::Clip method for rectangles, this is
            // fairly simple (does not even use GPC) and saves us from
            // unnecessary banding
            aPoly.Clip(rRect);

            mpB2DPolyPolygon.reset();
            mpPolyPolygon.reset(aPoly.Count() ? new tools::PolyPolygon(aPoly) : nullptr);
            mpRegionBand.reset();
        }

        return;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // region is empty -> nothing to do!
        return;
    }

    std::unique_ptr<RegionBand> pNew( std::make_unique<RegionBand>(*pCurrent));

    // get justified rectangle
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));

    // insert bands if the boundaries are not already in the list
    pNew->InsertBands(nTop, nBottom);

    // process intersect
    pNew->Intersect(nLeft, nTop, nRight, nBottom);

    // cleanup
    if(!pNew->OptimizeBandList())
    {
        pNew.reset();
    }

    mpRegionBand = std::move(pNew);
}

void vcl::Region::Exclude( const tools::Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
    {
        // excluding nothing will do no change
        return;
    }

    if(IsEmpty())
    {
        // cannot exclude from empty, done
        return;
    }

    if(IsNull())
    {
        // error; cannot exclude from null region since this is not representable
        // in the data
        OSL_ENSURE(false, "Region::Exclude error: Cannot exclude from null region (!)");
        return;
    }

    if( HasPolyPolygonOrB2DPolyPolygon() )
    {
        // get this B2DPolyPolygon
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::utils::prepareForPolygonOperation(aThisPolyPoly);

        if(!aThisPolyPoly.count())
        {
            // when local polygon is empty, nothing can be excluded
            return;
        }

        // get the other B2DPolyPolygon
        const basegfx::B2DPolygon aRectPoly(
            basegfx::utils::createPolygonFromRect(
                basegfx::B2DRectangle(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom())));
        const basegfx::B2DPolyPolygon aOtherPolyPoly(aRectPoly);
        const basegfx::B2DPolyPolygon aClip = basegfx::utils::solvePolygonOperationDiff(aThisPolyPoly, aOtherPolyPoly);

        *this = vcl::Region(aClip);

        return;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // empty? -> done!
        return;
    }

    std::unique_ptr<RegionBand> pNew( std::make_unique<RegionBand>(*pCurrent));

    // get justified rectangle
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));

    // insert bands if the boundaries are not already in the list
    pNew->InsertBands(nTop, nBottom);

    // process exclude
    pNew->Exclude(nLeft, nTop, nRight, nBottom);

    // cleanup
    if(!pNew->OptimizeBandList())
    {
        pNew.reset();
    }

    mpRegionBand = std::move(pNew);
}

void vcl::Region::XOr( const tools::Rectangle& rRect )
{
    if ( rRect.IsEmpty() )
    {
        // empty rectangle will not change local content
        return;
    }

    if(IsEmpty())
    {
        // rRect will be the xored-form (local off, rect on)
        *this = rRect;
        return;
    }

    if(IsNull())
    {
        // error; cannot exclude from null region since this is not representable
        // in the data
        OSL_ENSURE(false, "Region::XOr error: Cannot XOr with null region (!)");
        return;
    }

    if( HasPolyPolygonOrB2DPolyPolygon() )
    {
        // get this B2DPolyPolygon
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::utils::prepareForPolygonOperation( aThisPolyPoly );

        if(!aThisPolyPoly.count())
        {
            // no local content, XOr will be equal to rectangle
            *this = rRect;
            return;
        }

        // get the other B2DPolyPolygon
        const basegfx::B2DPolygon aRectPoly(
            basegfx::utils::createPolygonFromRect(
                basegfx::B2DRectangle(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom())));
        const basegfx::B2DPolyPolygon aOtherPolyPoly(aRectPoly);
        const basegfx::B2DPolyPolygon aClip = basegfx::utils::solvePolygonOperationXor(aThisPolyPoly, aOtherPolyPoly);

        *this = vcl::Region(aClip);

        return;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // rRect will be the xored-form (local off, rect on)
        *this = rRect;
        return;
    }

    // only region band mode possibility left here or null/empty
    std::unique_ptr<RegionBand> pNew( std::make_unique<RegionBand>(*getRegionBand()));

    // get justified rectangle
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));

    // insert bands if the boundaries are not already in the list
    pNew->InsertBands(nTop, nBottom);

    // process xor
    pNew->XOr(nLeft, nTop, nRight, nBottom);

    // cleanup
    if(!pNew->OptimizeBandList())
    {
        pNew.reset();
    }

    mpRegionBand = std::move(pNew);
}

void vcl::Region::Union( const vcl::Region& rRegion )
{
    if(rRegion.IsEmpty())
    {
        // no extension at all
        return;
    }

    if(rRegion.IsNull())
    {
        // extending with null region -> null region
        *this = vcl::Region(true);
        return;
    }

    if(IsEmpty())
    {
        // local is empty, union will give source region
        *this = rRegion;
        return;
    }

    if(IsNull())
    {
        // already fully expanded (is null region), cannot be extended
        return;
    }

    if( rRegion.HasPolyPolygonOrB2DPolyPolygon() || HasPolyPolygonOrB2DPolyPolygon() )
    {
        // get this B2DPolyPolygon
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::utils::prepareForPolygonOperation(aThisPolyPoly);

        if(!aThisPolyPoly.count())
        {
            // when no local content, union will be equal to rRegion
            *this = rRegion;
            return;
        }

        // get the other B2DPolyPolygon
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());
        aOtherPolyPoly = basegfx::utils::prepareForPolygonOperation(aOtherPolyPoly);

        // use logical OR operation
        basegfx::B2DPolyPolygon aClip(basegfx::utils::solvePolygonOperationOr(aThisPolyPoly, aOtherPolyPoly));

        *this = vcl::Region( aClip );
        return;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // local is empty, union will give source region
        *this = rRegion;
        return;
    }

    const RegionBand* pSource = rRegion.getRegionBand();

    if(!pSource)
    {
        // no extension at all
        return;
    }

    // prepare source and target
    std::unique_ptr<RegionBand> pNew( std::make_unique<RegionBand>(*pCurrent));

    // union with source
    pNew->Union(*pSource);

    // cleanup
    if(!pNew->OptimizeBandList())
    {
        pNew.reset();
    }

    mpRegionBand = std::move(pNew);
}

void vcl::Region::Intersect( const vcl::Region& rRegion )
{
    // same instance data? -> nothing to do!
    if(getB2DPolyPolygon() && getB2DPolyPolygon() == rRegion.getB2DPolyPolygon())
    {
        return;
    }

    if(getPolyPolygon() && getPolyPolygon() == rRegion.getPolyPolygon())
    {
        return;
    }

    if(getRegionBand() && getRegionBand() == rRegion.getRegionBand())
    {
        return;
    }

    if(rRegion.IsNull())
    {
        // source region is null-region, intersect will not change local region
        return;
    }

    if(IsNull())
    {
        // when local region is null-region, intersect will be equal to source
        *this = rRegion;
        return;
    }

    if(rRegion.IsEmpty())
    {
        // source region is empty, intersection will always be empty
        SetEmpty();
        return;
    }

    if(IsEmpty())
    {
        // local region is empty, cannot get more empty than that. Nothing to do
        return;
    }

    if( rRegion.HasPolyPolygonOrB2DPolyPolygon() || HasPolyPolygonOrB2DPolyPolygon() )
    {
        // get this B2DPolyPolygon
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        if(!aThisPolyPoly.count())
        {
            // local region is empty, cannot get more empty than that. Nothing to do
            return;
        }

        // get the other B2DPolyPolygon
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());

        if(!aOtherPolyPoly.count())
        {
            // source region is empty, intersection will always be empty
            SetEmpty();
            return;
        }

        const basegfx::B2DPolyPolygon aClip(
            basegfx::utils::clipPolyPolygonOnPolyPolygon(
                aOtherPolyPoly,
                aThisPolyPoly,
                true,
                false));
        *this = vcl::Region( aClip );
        return;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // local region is empty, cannot get more empty than that. Nothing to do
        return;
    }

    const RegionBand* pSource = rRegion.getRegionBand();

    if(!pSource)
    {
        // source region is empty, intersection will always be empty
        SetEmpty();
        return;
    }

    // both RegionBands exist and are not empty
    if(pCurrent->getRectangleCount() + 2 < pSource->getRectangleCount())
    {
        // when we have less rectangles, turn around the call
        vcl::Region aTempRegion = rRegion;
        aTempRegion.Intersect( *this );
        *this = aTempRegion;
    }
    else
    {
        // prepare new regionBand
        std::unique_ptr<RegionBand> pNew( std::make_unique<RegionBand>(*pCurrent));

        // intersect with source
        pNew->Intersect(*pSource);

        // cleanup
        if(!pNew->OptimizeBandList())
        {
            pNew.reset();
        }

        mpRegionBand = std::move(pNew);
    }
}

void vcl::Region::Exclude( const vcl::Region& rRegion )
{
    if ( rRegion.IsEmpty() )
    {
        // excluding nothing will do no change
        return;
    }

    if ( rRegion.IsNull() )
    {
        // excluding everything will create empty region
        SetEmpty();
        return;
    }

    if(IsEmpty())
    {
        // cannot exclude from empty, done
        return;
    }

    if(IsNull())
    {
        // error; cannot exclude from null region since this is not representable
        // in the data
        OSL_ENSURE(false, "Region::Exclude error: Cannot exclude from null region (!)");
        return;
    }

    if( rRegion.HasPolyPolygonOrB2DPolyPolygon() || HasPolyPolygonOrB2DPolyPolygon() )
    {
        // get this B2DPolyPolygon
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        if(!aThisPolyPoly.count())
        {
            // cannot exclude from empty, done
            return;
        }

        aThisPolyPoly = basegfx::utils::prepareForPolygonOperation( aThisPolyPoly );

        // get the other B2DPolyPolygon
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());
        aOtherPolyPoly = basegfx::utils::prepareForPolygonOperation( aOtherPolyPoly );

        basegfx::B2DPolyPolygon aClip = basegfx::utils::solvePolygonOperationDiff( aThisPolyPoly, aOtherPolyPoly );
        *this = vcl::Region( aClip );
        return;
    }

    // only region band mode possibility left here or null/empty
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        // cannot exclude from empty, done
        return;
    }

    const RegionBand* pSource = rRegion.getRegionBand();

    if(!pSource)
    {
        // excluding nothing will do no change
        return;
    }

    // prepare source and target
    std::unique_ptr<RegionBand> pNew( std::make_unique<RegionBand>(*pCurrent));

    // union with source
    const bool bSuccess(pNew->Exclude(*pSource));

    // cleanup
    if(!bSuccess)
    {
        pNew.reset();
    }

    mpRegionBand = std::move(pNew);
}

bool vcl::Region::XOr( const vcl::Region& rRegion )
{
    if ( rRegion.IsEmpty() )
    {
        // empty region will not change local content
        return true;
    }

    if ( rRegion.IsNull() )
    {
        // error; cannot exclude null region from local since this is not representable
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
        // error: cannot exclude from null region since this is not representable
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

        aThisPolyPoly = basegfx::utils::prepareForPolygonOperation( aThisPolyPoly );

        // get the other B2DPolyPolygon
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());
        aOtherPolyPoly = basegfx::utils::prepareForPolygonOperation( aOtherPolyPoly );

        basegfx::B2DPolyPolygon aClip = basegfx::utils::solvePolygonOperationXor( aThisPolyPoly, aOtherPolyPoly );
        *this = vcl::Region( aClip );
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
    std::unique_ptr<RegionBand> pNew( std::make_unique<RegionBand>(*pCurrent));

    // union with source
    pNew->XOr(*pSource);

    // cleanup
    if(!pNew->OptimizeBandList())
    {
        pNew.reset();
    }

    mpRegionBand = std::move(pNew);

    return true;
}

tools::Rectangle vcl::Region::GetBoundRect() const
{
    if(IsEmpty())
    {
        // no internal data? -> region is empty!
        return tools::Rectangle();
    }

    if(IsNull())
    {
        // error; null region has no BoundRect
        // OSL_ENSURE(false, "Region::GetBoundRect error: null region has unlimited bound rect, not representable (!)");
        return tools::Rectangle();
    }

    // prefer double precision source
    if(getB2DPolyPolygon())
    {
        const basegfx::B2DRange aRange(basegfx::utils::getRange(*getB2DPolyPolygon()));

        if(aRange.isEmpty())
        {
            // emulate PolyPolygon::GetBoundRect() when empty polygon
            return tools::Rectangle();
        }
        else
        {
            // #i122149# corrected rounding, no need for ceil() and floor() here
            return tools::Rectangle(
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

    return tools::Rectangle();
}

const tools::PolyPolygon vcl::Region::GetAsPolyPolygon() const
{
    if(getPolyPolygon())
    {
        return *getPolyPolygon();
    }

    if(getB2DPolyPolygon())
    {
        // the polygon needs to be converted, buffer the down conversion
        const tools::PolyPolygon aPolyPolgon(*getB2DPolyPolygon());
        const_cast< vcl::Region* >(this)->mpPolyPolygon.reset(new tools::PolyPolygon(aPolyPolgon));

        return *getPolyPolygon();
    }

    if(getRegionBand())
    {
        // the BandRegion needs to be converted, buffer the conversion
        const tools::PolyPolygon aPolyPolgon(ImplCreatePolyPolygonFromRegionBand());
        const_cast< vcl::Region* >(this)->mpPolyPolygon.reset(new tools::PolyPolygon(aPolyPolgon));

        return *getPolyPolygon();
    }

    return tools::PolyPolygon();
}

const basegfx::B2DPolyPolygon vcl::Region::GetAsB2DPolyPolygon() const
{
    if(getB2DPolyPolygon())
    {
        return *getB2DPolyPolygon();
    }

    if(getPolyPolygon())
    {
        // the polygon needs to be converted, buffer the up conversion. This will be preferred from now.
        const basegfx::B2DPolyPolygon aB2DPolyPolygon(getPolyPolygon()->getB2DPolyPolygon());
        const_cast< vcl::Region* >(this)->mpB2DPolyPolygon.reset(new basegfx::B2DPolyPolygon(aB2DPolyPolygon));

        return *getB2DPolyPolygon();
    }

    if(getRegionBand())
    {
        // the BandRegion needs to be converted, buffer the conversion
        const basegfx::B2DPolyPolygon aB2DPolyPolygon(ImplCreateB2DPolyPolygonFromRegionBand());
        const_cast< vcl::Region* >(this)->mpB2DPolyPolygon.reset(new basegfx::B2DPolyPolygon(aB2DPolyPolygon));

        return *getB2DPolyPolygon();
    }

    return basegfx::B2DPolyPolygon();
}

const RegionBand* vcl::Region::GetAsRegionBand() const
{
    if(!getRegionBand())
    {
        if(getB2DPolyPolygon())
        {
            // convert B2DPolyPolygon to RegionBand, buffer it and return it
            const_cast< vcl::Region* >(this)->mpRegionBand = ImplCreateRegionBandFromPolyPolygon(tools::PolyPolygon(*getB2DPolyPolygon()));
        }
        else if(getPolyPolygon())
        {
            // convert B2DPolyPolygon to RegionBand, buffer it and return it
            const_cast< vcl::Region* >(this)->mpRegionBand = ImplCreateRegionBandFromPolyPolygon(*getPolyPolygon());
        }
    }

    return getRegionBand();
}

bool vcl::Region::IsInside( const Point& rPoint ) const
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

    // ensure RegionBand existence
    const RegionBand* pRegionBand = GetAsRegionBand();

    if(pRegionBand)
    {
        return pRegionBand->IsInside(rPoint);
    }

    return false;
}

bool vcl::Region::IsOver( const tools::Rectangle& rRect ) const
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
    vcl::Region aRegion(rRect);
    aRegion.Intersect( *this );

    // rectangle is over if include is not empty
    return !aRegion.IsEmpty();
}

bool vcl::Region::IsRectangle() const
{
    if( IsEmpty() || IsNull() )
        return false;

    if( getB2DPolyPolygon() )
        return basegfx::utils::isRectangle( *getB2DPolyPolygon() );

    if( getPolyPolygon() )
        return getPolyPolygon()->IsRect();

    if( getRegionBand() )
        return (getRegionBand()->getRectangleCount() == 1);

    return false;
}

void vcl::Region::SetNull()
{
    // reset all content
    mpB2DPolyPolygon.reset();
    mpPolyPolygon.reset();
    mpRegionBand.reset();
    mbIsNull = true;
}

void vcl::Region::SetEmpty()
{
    // reset all content
    mpB2DPolyPolygon.reset();
    mpPolyPolygon.reset();
    mpRegionBand.reset();
    mbIsNull = false;
}

Region& vcl::Region::operator=( const vcl::Region& ) = default;

Region& vcl::Region::operator=( vcl::Region&& rRegion )
{
    mpB2DPolyPolygon = std::move(rRegion.mpB2DPolyPolygon);
    mpPolyPolygon = std::move(rRegion.mpPolyPolygon);
    mpRegionBand = std::move(rRegion.mpRegionBand);
    mbIsNull = rRegion.mbIsNull;
    rRegion.mbIsNull = true;

    return *this;
}

Region& vcl::Region::operator=( const tools::Rectangle& rRect )
{
    mpB2DPolyPolygon.reset();
    mpPolyPolygon.reset();
    mpRegionBand.reset(rRect.IsEmpty() ? nullptr : new RegionBand(rRect));
    mbIsNull = false;

    return *this;
}

bool vcl::Region::operator==( const vcl::Region& rRegion ) const
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
        GetAsB2DPolyPolygon();
        rRegion.GetAsB2DPolyPolygon();

        return *rRegion.getB2DPolyPolygon() == *getB2DPolyPolygon();
    }

    if(rRegion.getPolyPolygon() || getPolyPolygon())
    {
        // one of both has a B2DPolyPolygon based region, ensure both have it
        // by evtl. conversion
        GetAsPolyPolygon();
        rRegion.GetAsPolyPolygon();

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

SvStream& ReadRegion(SvStream& rIStrm, vcl::Region& rRegion)
{
    VersionCompat aCompat(rIStrm, StreamMode::READ);
    sal_uInt16 nVersion(0);
    sal_uInt16 nTmp16(0);

    // clear region to be loaded
    rRegion.SetEmpty();

    // get version of streamed region
    rIStrm.ReadUInt16( nVersion );

    // get type of region
    rIStrm.ReadUInt16( nTmp16 );

    enum RegionType { REGION_NULL, REGION_EMPTY, REGION_RECTANGLE, REGION_COMPLEX };
    RegionType meStreamedType = static_cast<RegionType>(nTmp16);

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
            bool bSuccess = pNewRegionBand->load(rIStrm);
            rRegion.mpRegionBand.reset(pNewRegionBand);

            bool bHasPolyPolygon(false);
            if (aCompat.GetVersion() >= 2)
            {
                rIStrm.ReadCharAsBool( bHasPolyPolygon );

                if (bHasPolyPolygon)
                {
                    tools::PolyPolygon* pNewPoly = new tools::PolyPolygon();
                    ReadPolyPolygon( rIStrm, *pNewPoly );
                    rRegion.mpPolyPolygon.reset(pNewPoly);
                }
            }

            if (!bSuccess && !bHasPolyPolygon)
            {
                SAL_WARN("vcl.gdi", "bad region band:" << bHasPolyPolygon);
                rRegion.SetNull();
            }

            break;
        }
    }

    return rIStrm;
}

SvStream& WriteRegion( SvStream& rOStrm, const vcl::Region& rRegion )
{
    const sal_uInt16 nVersion(2);
    VersionCompat aCompat(rOStrm, StreamMode::WRITE, nVersion);

    // put version
    rOStrm.WriteUInt16( nVersion );

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

    rOStrm.WriteUInt16( aRegionType );

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
    const bool bHasPolyPolygon(rRegion.HasPolyPolygonOrB2DPolyPolygon());
    rOStrm.WriteBool( bHasPolyPolygon );

    if(bHasPolyPolygon)
    {
        // #i105373#
        tools::PolyPolygon aNoCurvePolyPolygon;
        rRegion.GetAsPolyPolygon().AdaptiveSubdivide(aNoCurvePolyPolygon);

        WritePolyPolygon( rOStrm, aNoCurvePolyPolygon );
    }

    return rOStrm;
}

void vcl::Region::GetRegionRectangles(RectangleVector& rTarget) const
{
    // clear returnvalues
    rTarget.clear();

    // ensure RegionBand existence
    const RegionBand* pRegionBand = GetAsRegionBand();

    if(pRegionBand)
    {
        pRegionBand->GetRegionRectangles(rTarget);
    }
}

static bool ImplPolygonRectTest( const tools::Polygon& rPoly, tools::Rectangle* pRectOut = nullptr )
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

                pRectOut->SetLeft( nX1 );
                pRectOut->SetRight( nX2 );
                pRectOut->SetTop( nY1 );
                pRectOut->SetBottom( nY2 );
            }
        }
    }

    return bIsRect;
}

vcl::Region vcl::Region::GetRegionFromPolyPolygon( const tools::PolyPolygon& rPolyPoly )
{
    //return vcl::Region( rPolyPoly );

    // check if it's worth extracting the XOr'ing the Rectangles
    // empiricism shows that break even between XOr'ing rectangles separately
    // and ImplCreateRegionBandFromPolyPolygon is at half rectangles/half polygons
    int nPolygonRects = 0, nPolygonPolygons = 0;
    int nPolygons = rPolyPoly.Count();

    for( int i = 0; i < nPolygons; i++ )
    {
        const tools::Polygon& rPoly = rPolyPoly[i];

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
        return vcl::Region( rPolyPoly );
    }

    vcl::Region aResult;
    tools::Rectangle aRect;

    for( int i = 0; i < nPolygons; i++ )
    {
        const tools::Polygon& rPoly = rPolyPoly[i];

        if( ImplPolygonRectTest( rPoly, &aRect ) )
        {
            aResult.XOr( aRect );
        }
        else
        {
            aResult.XOr( vcl::Region(rPoly) );
        }
    }

    return aResult;
}

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
