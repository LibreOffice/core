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






namespace
{
    /** Return <TRUE/> when the given polygon is rectiliner and oriented so that
        all sides are either horizontal or vertical.
    */
    bool ImplIsPolygonRectilinear (const PolyPolygon& rPolyPoly)
    {
        
        const sal_uInt16 nPolyCount = rPolyPoly.Count();
        for (sal_uInt16 nPoly = 0; nPoly < nPolyCount; ++nPoly)
        {
            const Polygon&  aPoly = rPolyPoly.GetObject(nPoly);

            
            const sal_uInt16 nSize = aPoly.GetSize();

            if (nSize < 2)
                continue;
            Point aPoint (aPoly.GetPoint(0));
            const Point aLastPoint (aPoint);
            for (sal_uInt16 nPoint = 1; nPoint < nSize; ++nPoint)
            {
                const Point aNextPoint (aPoly.GetPoint(nPoint));
                
                
                
                if (aPoint.X() != aNextPoint.X() && aPoint.Y() != aNextPoint.Y())
                    return false;

                aPoint = aNextPoint;
            }
            
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

        
        RegionBand* pRegionBand = new RegionBand();
        long nLineId = 0L;

        
        const sal_uInt16 nPolyCount = rPolyPoly.Count();
        for (sal_uInt16 nPoly = 0; nPoly < nPolyCount; ++nPoly)
        {
            const Polygon&  aPoly = rPolyPoly.GetObject(nPoly);

            
            const sal_uInt16 nSize = aPoly.GetSize();
            if (nSize < 2)
                continue;
            
            
            Point aStart (aPoly.GetPoint(0));
            Point aEnd;
            for (sal_uInt16 nPoint = 1; nPoint <= nSize; ++nPoint, aStart=aEnd)
            {
                
                
                aEnd = aPoly.GetPoint(nPoint%nSize);
                if (aStart.Y() == aEnd.Y())
                {
                    
                    continue;
                }

                
                OSL_ASSERT(aStart.X() == aEnd.X());

                
                
                
                const long nTop (::std::min(aStart.Y(), aEnd.Y()));
                const long nBottom (::std::max(aStart.Y(), aEnd.Y()));
                const LineType eLineType (aStart.Y() > aEnd.Y() ? LINE_DESCENDING : LINE_ASCENDING);

                
                pRegionBand->ImplAddMissingBands(nTop,nBottom);

                
                ImplRegionBand* pBand = pRegionBand->ImplGetFirstRegionBand();
                while (pBand!=NULL && pBand->mnYBottom < nTop)
                    pBand = pBand->mpNextBand;
                ImplRegionBand* pTopBand = pBand;
                
                
                if (pBand!=NULL
                       
                    && pBand->mnYTop<nTop
                       
                    && pBand->mnYBottom>=nTop
                       
                    && pBand->mnYTop<pBand->mnYBottom)
                {
                    
                    pTopBand = pBand->SplitBand(nTop);
                }

                
                while (pBand!=NULL && pBand->mnYBottom < nBottom)
                    pBand = pBand->mpNextBand;
                
                
                if (pBand!=NULL
                       
                    && pBand->mnYTop<=nBottom
                       
                    && pBand->mnYBottom>nBottom
                       
                    && pBand->mnYTop<pBand->mnYBottom)
                {
                    
                    pBand->SplitBand(nBottom+1);
                }

                
                
                

                
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

        
        RegionBand* pRegionBand = new RegionBand();
        pRegionBand->CreateBandRange(rPolygonBoundingBox.Top(), rPolygonBoundingBox.Bottom());

        
        const sal_uInt16 nPolyCount = rPolyPoly.Count();

        for ( sal_uInt16 nPoly = 0; nPoly < nPolyCount; nPoly++ )
        {
            
            const Polygon&  aPoly = rPolyPoly.GetObject( nPoly );
            const sal_uInt16    nSize = aPoly.GetSize();

            
            if ( nSize <= 2 )
                continue;

            
            for ( sal_uInt16 nPoint = 1; nPoint < nSize; nPoint++ )
            {
                pRegionBand->InsertLine( aPoly.GetPoint(nPoint-1), aPoly.GetPoint(nPoint), nLineID++ );
            }

            
            const Point rLastPoint = aPoly.GetPoint(nSize-1);
            const Point rFirstPoint = aPoly.GetPoint(0);

            if ( rLastPoint != rFirstPoint )
            {
                pRegionBand->InsertLine( rLastPoint, rFirstPoint, nLineID++ );
            }
        }

        return pRegionBand;
    }
} 



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
        
        
        PolyPolygon aPolyPolygon;

        rPolyPolygon.AdaptiveSubdivide(aPolyPolygon);

        if(aPolyPolygon.Count())
        {
            const Rectangle aRect(aPolyPolygon.GetBoundRect());

            if(!aRect.IsEmpty())
            {
                if(ImplIsPolygonRectilinear(aPolyPolygon))
                {
                    
                    pRetval = ImplRectilinearPolygonToBands(aPolyPolygon);
                }
                else
                {
                    pRetval = ImplGeneralPolygonToBands(aPolyPolygon, aRect);
                }

                
                if(pRetval)
                {
                    pRetval->processPoints();

                    
                    
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
        
        const Rectangle aRect(rPolyPoly.GetBoundRect());

        if(!aRect.IsEmpty())
        {
            
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
        
        return;
    }

    if(!nHorzMove && !nVertMove)
    {
        
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
        
        return;
    }

    if(basegfx::fTools::equalZero(fScaleX) && basegfx::fTools::equalZero(fScaleY))
    {
        
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
        
        return true;
    }

    if(IsEmpty())
    {
        
        *this = rRect;
        return true;
    }

    if(HasPolyPolygonOrB2DPolyPolygon())
    {
        
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation(aThisPolyPoly);

        if(!aThisPolyPoly.count())
        {
            
            *this = rRect;
        }
        else
        {
            
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

    
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        
        *this = rRect;
        return true;
    }

    RegionBand* pNew = new RegionBand(*pCurrent);

    
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));

    
    pNew->InsertBands(nTop, nBottom);

    
    pNew->Union(nLeft, nTop, nRight, nBottom);

    
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
        
        SetEmpty();
        return true;
    }

    if(IsNull())
    {
        
        *this = rRect;
        return true;
    }

    if(IsEmpty())
    {
        
        return true;
    }

    if(HasPolyPolygonOrB2DPolyPolygon())
    {
        
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
        else 
        {
            PolyPolygon aPoly(*getPolyPolygon());

            
            
            
            aPoly.Clip(rRect);

            mpB2DPolyPolygon.reset();
            mpPolyPolygon.reset(aPoly.Count() ? new PolyPolygon(aPoly) : 0);
            mpRegionBand.reset();
        }

        return true;
    }

    
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        
        return true;
    }

    RegionBand* pNew = new RegionBand(*pCurrent);

    
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));

    
    pNew->InsertBands(nTop, nBottom);

    
    pNew->Intersect(nLeft, nTop, nRight, nBottom);

    
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
        
        return true;
    }

    if(IsEmpty())
    {
        
        return true;
    }

    if(IsNull())
    {
        
        
        OSL_ENSURE(false, "Region::Exclude error: Cannot exclude from null region (!)");
        return true;
    }

    if( HasPolyPolygonOrB2DPolyPolygon() )
    {
        
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation(aThisPolyPoly);

        if(!aThisPolyPoly.count())
        {
            
            return true;
        }

        
        const basegfx::B2DPolygon aRectPoly(
            basegfx::tools::createPolygonFromRect(
                basegfx::B2DRectangle(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom())));
        const basegfx::B2DPolyPolygon aOtherPolyPoly(aRectPoly);
        const basegfx::B2DPolyPolygon aClip = basegfx::tools::solvePolygonOperationDiff(aThisPolyPoly, aOtherPolyPoly);

        *this = Region(aClip);

        return true;
    }

    
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        
        return true;
    }

    RegionBand* pNew = new RegionBand(*pCurrent);

    
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));

    
    pNew->InsertBands(nTop, nBottom);

    
    pNew->Exclude(nLeft, nTop, nRight, nBottom);

    
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
        
        return true;
    }

    if(IsEmpty())
    {
        
        *this = rRect;
        return true;
    }

    if(IsNull())
    {
        
        
        OSL_ENSURE(false, "Region::XOr error: Cannot XOr with null region (!)");
        return true;
    }

    if( HasPolyPolygonOrB2DPolyPolygon() )
    {
        
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation( aThisPolyPoly );

        if(!aThisPolyPoly.count())
        {
            
            *this = rRect;
            return true;
        }

        
        const basegfx::B2DPolygon aRectPoly(
            basegfx::tools::createPolygonFromRect(
                basegfx::B2DRectangle(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom())));
        const basegfx::B2DPolyPolygon aOtherPolyPoly(aRectPoly);
        const basegfx::B2DPolyPolygon aClip = basegfx::tools::solvePolygonOperationXor(aThisPolyPoly, aOtherPolyPoly);

        *this = Region(aClip);

        return true;
    }

    
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        
        *this = rRect;
        return true;
    }

    
    RegionBand* pNew = new RegionBand(*getRegionBand());

    
    const long nLeft(std::min(rRect.Left(), rRect.Right()));
    const long nTop(std::min(rRect.Top(), rRect.Bottom()));
    const long nRight(std::max(rRect.Left(), rRect.Right()));
    const long nBottom(std::max(rRect.Top(), rRect.Bottom()));

    
    pNew->InsertBands(nTop, nBottom);

    
    pNew->XOr(nLeft, nTop, nRight, nBottom);

    
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
        
        return true;
    }

    if(rRegion.IsNull())
    {
        
        *this = Region(true);
        return true;
    }

    if(IsEmpty())
    {
        
        *this = rRegion;
        return true;
    }

    if(IsNull())
    {
        
        return true;
    }

    if( rRegion.HasPolyPolygonOrB2DPolyPolygon() || HasPolyPolygonOrB2DPolyPolygon() )
    {
        
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation(aThisPolyPoly);

        if(!aThisPolyPoly.count())
        {
            
            *this = rRegion;
            return true;
        }

        
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());
        aOtherPolyPoly = basegfx::tools::prepareForPolygonOperation(aOtherPolyPoly);

        
        basegfx::B2DPolyPolygon aClip(basegfx::tools::solvePolygonOperationOr(aThisPolyPoly, aOtherPolyPoly));

        *this = Region( aClip );
        return true;
    }

    
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        
        *this = rRegion;
        return true;
    }

    const RegionBand* pSource = rRegion.getRegionBand();

    if(!pSource)
    {
        
        return true;
    }

    
    RegionBand* pNew = new RegionBand(*pCurrent);

    
    pNew->Union(*pSource);

    
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
        
        return true;
    }

    if(IsNull())
    {
        
        *this = rRegion;
        return true;
    }

    if(rRegion.IsEmpty())
    {
        
        SetEmpty();
        return true;
    }

    if(IsEmpty())
    {
        
        return true;
    }

    if( rRegion.HasPolyPolygonOrB2DPolyPolygon() || HasPolyPolygonOrB2DPolyPolygon() )
    {
        
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        if(!aThisPolyPoly.count())
        {
            
            return true;
        }

        
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());

        if(!aOtherPolyPoly.count())
        {
            
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

    
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        
        return true;
    }

    const RegionBand* pSource = rRegion.getRegionBand();

    if(!pSource)
    {
        
        SetEmpty();
        return true;
    }

    
    if(pCurrent->getRectangleCount() + 2 < pSource->getRectangleCount())
    {
        
        Region aTempRegion = rRegion;
        aTempRegion.Intersect( *this );
        *this = aTempRegion;
    }
    else
    {
        
        RegionBand* pNew = new RegionBand(*pCurrent);

        
        pNew->Intersect(*pSource);

        
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
        
        return true;
    }

    if ( rRegion.IsNull() )
    {
        
        SetEmpty();
        return true;
    }

    if(IsEmpty())
    {
        
        return true;
    }

    if(IsNull())
    {
        
        
        OSL_ENSURE(false, "Region::Exclude error: Cannot exclude from null region (!)");
        return true;
    }

    if( rRegion.HasPolyPolygonOrB2DPolyPolygon() || HasPolyPolygonOrB2DPolyPolygon() )
    {
        
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        if(!aThisPolyPoly.count())
        {
            
            return true;
        }

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation( aThisPolyPoly );

        
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());
        aOtherPolyPoly = basegfx::tools::prepareForPolygonOperation( aOtherPolyPoly );

        basegfx::B2DPolyPolygon aClip = basegfx::tools::solvePolygonOperationDiff( aThisPolyPoly, aOtherPolyPoly );
        *this = Region( aClip );
        return true;
    }

    
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        
        return true;
    }

    const RegionBand* pSource = rRegion.getRegionBand();

    if(!pSource)
    {
        
        return true;
    }

    
    RegionBand* pNew = new RegionBand(*pCurrent);

    
    const bool bSuccess(pNew->Exclude(*pSource));

    
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
        
        return true;
    }

    if ( rRegion.IsNull() )
    {
        
        
        OSL_ENSURE(false, "Region::XOr error: Cannot XOr with null region (!)");
        return true;
    }

    if(IsEmpty())
    {
        
        *this = rRegion;
        return true;
    }

    if(IsNull())
    {
        
        
        OSL_ENSURE(false, "Region::XOr error: Cannot XOr with null region (!)");
        return false;
    }

    if( rRegion.HasPolyPolygonOrB2DPolyPolygon() || HasPolyPolygonOrB2DPolyPolygon() )
    {
        
        basegfx::B2DPolyPolygon aThisPolyPoly(GetAsB2DPolyPolygon());

        if(!aThisPolyPoly.count())
        {
            
            *this = rRegion;
            return true;
        }

        aThisPolyPoly = basegfx::tools::prepareForPolygonOperation( aThisPolyPoly );

        
        basegfx::B2DPolyPolygon aOtherPolyPoly(rRegion.GetAsB2DPolyPolygon());
        aOtherPolyPoly = basegfx::tools::prepareForPolygonOperation( aOtherPolyPoly );

        basegfx::B2DPolyPolygon aClip = basegfx::tools::solvePolygonOperationXor( aThisPolyPoly, aOtherPolyPoly );
        *this = Region( aClip );
        return true;
    }

    
    const RegionBand* pCurrent = getRegionBand();

    if(!pCurrent)
    {
        
        *this = rRegion;
        return true;
    }

    const RegionBand* pSource = rRegion.getRegionBand();

    if(!pSource)
    {
        
        return true;
    }

    
    RegionBand* pNew = new RegionBand(*pCurrent);

    
    pNew->XOr(*pSource);

    
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
        
        return Rectangle();
    }

    if(IsNull())
    {
        
        
        return Rectangle();
    }

    
    if(getB2DPolyPolygon())
    {
        const basegfx::B2DRange aRange(basegfx::tools::getRange(*getB2DPolyPolygon()));

        if(aRange.isEmpty())
        {
            
            return Rectangle();
        }
        else
        {
            
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
        
        const PolyPolygon aPolyPolgon(*getB2DPolyPolygon());
        const_cast< Region* >(this)->mpPolyPolygon.reset(new PolyPolygon(aPolyPolgon));

        return *getPolyPolygon();
    }

    if(getRegionBand())
    {
        
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
        
        const basegfx::B2DPolyPolygon aB2DPolyPolygon(getPolyPolygon()->getB2DPolyPolygon());
        const_cast< Region* >(this)->mpB2DPolyPolygon.reset(new basegfx::B2DPolyPolygon(aB2DPolyPolygon));

        return *getB2DPolyPolygon();
    }

    if(getRegionBand())
    {
        
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
            
            const_cast< Region* >(this)->mpRegionBand.reset(ImplCreateRegionBandFromPolyPolygon(PolyPolygon(*getB2DPolyPolygon())));
        }
        else if(getPolyPolygon())
        {
            
            const_cast< Region* >(this)->mpRegionBand.reset(ImplCreateRegionBandFromPolyPolygon(*getPolyPolygon()));
        }
    }

    return getRegionBand();
}

bool Region::IsInside( const Point& rPoint ) const
{
    if(IsEmpty())
    {
        
        return false;
    }

    if(IsNull())
    {
        
        return true;
    }

    
    
    
    
    

    
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
        
        return false;
    }

    if(IsNull())
    {
        
        return true;
    }

    if ( rRect.IsEmpty() )
    {
        
        return false;
    }

    
    Region aRegion(rRect);
    aRegion.Exclude(*this);

    
    return aRegion.IsEmpty();
}



bool Region::IsOver( const Rectangle& rRect ) const
{
    if(IsEmpty())
    {
        
        return false;
    }

    if(IsNull())
    {
        
        return true;
    }

    
    
    
    Region aRegion(rRect);
    aRegion.Intersect( *this );

    
    return !aRegion.IsEmpty();
}

void Region::SetNull()
{
    
    mpB2DPolyPolygon.reset();
    mpPolyPolygon.reset();
    mpRegionBand.reset();
    mbIsNull = true;
}

void Region::SetEmpty()
{
    
    mpB2DPolyPolygon.reset();
    mpPolyPolygon.reset();
    mpRegionBand.reset();
    mbIsNull = false;
}

Region& Region::operator=( const Region& rRegion )
{
    
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
        
        return true;
    }

    if(IsEmpty() && rRegion.IsEmpty())
    {
        
        return true;
    }

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
        
        
        const_cast< Region* >(this)->GetAsB2DPolyPolygon();
        const_cast< Region& >(rRegion).GetAsB2DPolyPolygon();

        return *rRegion.getB2DPolyPolygon() == *getB2DPolyPolygon();
    }

    if(rRegion.getPolyPolygon() || getPolyPolygon())
    {
        
        
        const_cast< Region* >(this)->GetAsPolyPolygon();
        const_cast< Region& >(rRegion).GetAsPolyPolygon();

        return *rRegion.getPolyPolygon() == *getPolyPolygon();
    }

    
    
    
    if(rRegion.getRegionBand() && getRegionBand())
    {
        return *rRegion.getRegionBand() == *getRegionBand();
    }

    
    return false;
}

SvStream& ReadRegion(SvStream& rIStrm, Region& rRegion)
{
    VersionCompat aCompat(rIStrm, STREAM_READ);
    sal_uInt16 nVersion(0);
    sal_uInt16 nTmp16(0);

    
    rRegion.SetEmpty();

    
    rIStrm.ReadUInt16( nVersion );

    
    rIStrm.ReadUInt16( nTmp16 );

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
                bool bHasPolyPolygon(false);

                rIStrm.ReadCharAsBool( bHasPolyPolygon );

                if(bHasPolyPolygon)
                {
                    PolyPolygon* pNewPoly = new PolyPolygon();
                    ReadPolyPolygon( rIStrm, *pNewPoly );
                    rRegion.mpPolyPolygon.reset(pNewPoly);
                }
            }

            break;
        }
    }

    return rIStrm;
}

SvStream& WriteRegion( SvStream& rOStrm, const Region& rRegion )
{
    const sal_uInt16 nVersion(2);
    VersionCompat aCompat(rOStrm, STREAM_WRITE, nVersion);

    
    rOStrm.WriteUInt16( nVersion );

    
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

    rOStrm.WriteUInt16( (sal_uInt16)aRegionType );

    
    const RegionBand* pRegionBand = rRegion.getRegionBand();

    if(pRegionBand)
    {
        pRegionBand->save(rOStrm);
    }
    else
    {
        
        
        const RegionBand aRegionBand;

        aRegionBand.save(rOStrm);
    }

    
    const bool bHasPolyPolygon(rRegion.HasPolyPolygonOrB2DPolyPolygon());
    rOStrm.WriteUChar( bHasPolyPolygon );

    if(bHasPolyPolygon)
    {
        
        PolyPolygon aNoCurvePolyPolygon;
        rRegion.GetAsPolyPolygon().AdaptiveSubdivide(aNoCurvePolyPolygon);

        WritePolyPolygon( rOStrm, aNoCurvePolyPolygon );
    }

    return rOStrm;
}

void Region::GetRegionRectangles(RectangleVector& rTarget) const
{
    
    rTarget.clear();

    
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
