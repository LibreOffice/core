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

#include <osl/diagnose.h>
#include <tools/line.hxx>
#include <tools/helpers.hxx>
#include <unotools/configmgr.hxx>

#include <vcl/hatch.hxx>
#include <vcl/metaact.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>

#include <drawmode.hxx>
#include <salgdi.hxx>

#include <cassert>
#include <cstdlib>
#include <memory>

#define HATCH_MAXPOINTS             1024

extern "C" {

static int HatchCmpFnc( const void* p1, const void* p2 )
{
    const tools::Long nX1 = static_cast<Point const *>(p1)->X();
    const tools::Long nX2 = static_cast<Point const *>(p2)->X();
    const tools::Long nY1 = static_cast<Point const *>(p1)->Y();
    const tools::Long nY2 = static_cast<Point const *>(p2)->Y();

    return ( nX1 > nX2 ? 1 : nX1 == nX2 ? nY1 > nY2 ? 1: nY1 == nY2 ? 0 : -1 : -1 );
}

}

void OutputDevice::DrawHatch( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
    assert(!is_double_buffered_window());

    Hatch aHatch( rHatch );
    aHatch.SetColor(vcl::drawmode::GetHatchColor(rHatch.GetColor(), GetDrawMode(), GetSettings().GetStyleSettings()));

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaHatchAction( rPolyPoly, aHatch ) );

    if( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    if( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if( mbInitClipRegion )
        InitClipRegion();

    if( mbOutputClipped )
        return;

    if( rPolyPoly.Count() )
    {
        tools::PolyPolygon     aPolyPoly( LogicToPixel( rPolyPoly ) );
        GDIMetaFile*    pOldMetaFile = mpMetaFile;
        bool            bOldMap = mbMap;

        aPolyPoly.Optimize( PolyOptimizeFlags::NO_SAME );
        aHatch.SetDistance( ImplLogicWidthToDevicePixel( aHatch.GetDistance() ) );

        mpMetaFile = nullptr;
        EnableMapMode( false );
        Push( vcl::PushFlags::LINECOLOR );
        SetLineColor( aHatch.GetColor() );
        InitLineColor();
        DrawHatch( aPolyPoly, aHatch, false );
        Pop();
        EnableMapMode( bOldMap );
        mpMetaFile = pOldMetaFile;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawHatch( rPolyPoly, rHatch );
}

void OutputDevice::AddHatchActions( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch,
                                    GDIMetaFile& rMtf )
{

    tools::PolyPolygon aPolyPoly( rPolyPoly );
    aPolyPoly.Optimize( PolyOptimizeFlags::NO_SAME | PolyOptimizeFlags::CLOSE );

    if( aPolyPoly.Count() )
    {
        GDIMetaFile* pOldMtf = mpMetaFile;

        mpMetaFile = &rMtf;
        mpMetaFile->AddAction( new MetaPushAction( vcl::PushFlags::ALL ) );
        mpMetaFile->AddAction( new MetaLineColorAction( rHatch.GetColor(), true ) );
        DrawHatch( aPolyPoly, rHatch, true );
        mpMetaFile->AddAction( new MetaPopAction() );
        mpMetaFile = pOldMtf;
    }
}

static bool HasSaneNSteps(const Point& rPt1, const Point& rEndPt1, const Size& rInc)
{
    tools::Long nVertSteps = -1;
    if (rInc.Height())
    {
        bool bFail = o3tl::checked_sub(rEndPt1.Y(), rPt1.Y(), nVertSteps);
        if (bFail)
            nVertSteps = std::numeric_limits<tools::Long>::max();
        else
            nVertSteps = nVertSteps / rInc.Height();
    }
    tools::Long nHorzSteps = -1;
    if (rInc.Width())
    {
        bool bFail = o3tl::checked_sub(rEndPt1.X(), rPt1.X(), nHorzSteps);
        if (bFail)
            nHorzSteps = std::numeric_limits<tools::Long>::max();
        else
            nHorzSteps = nHorzSteps / rInc.Width();
    }
    auto nSteps = std::max(nVertSteps, nHorzSteps);
    if (nSteps > 1024)
    {
        SAL_WARN("vcl.gdi", "skipping slow hatch with " << nSteps << " steps");
        return false;
    }
    return true;
}

void OutputDevice::DrawHatch( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch, bool bMtf )
{
    assert(!is_double_buffered_window());

    if(!rPolyPoly.Count())
        return;

    // #i115630# DrawHatch does not work with beziers included in the polypolygon, take care of that
    bool bIsCurve(false);

    for(sal_uInt16 a(0); !bIsCurve && a < rPolyPoly.Count(); a++)
    {
        if(rPolyPoly[a].HasFlags())
        {
            bIsCurve = true;
        }
    }

    if(bIsCurve)
    {
        OSL_ENSURE(false, "DrawHatch does *not* support curves, falling back to AdaptiveSubdivide()...");
        tools::PolyPolygon aPolyPoly;

        rPolyPoly.AdaptiveSubdivide(aPolyPoly);
        DrawHatch(aPolyPoly, rHatch, bMtf);
    }
    else
    {
        tools::Rectangle   aRect( rPolyPoly.GetBoundRect() );
        const tools::Long  nLogPixelWidth = ImplDevicePixelToLogicWidth( 1 );
        const tools::Long  nWidth = ImplDevicePixelToLogicWidth( std::max( ImplLogicWidthToDevicePixel( rHatch.GetDistance() ), tools::Long(3) ) );
        std::unique_ptr<Point[]> pPtBuffer(new Point[ HATCH_MAXPOINTS ]);
        Point       aPt1, aPt2, aEndPt1;
        Size        aInc;

        // Single hatch
        aRect.AdjustLeft( -nLogPixelWidth ); aRect.AdjustTop( -nLogPixelWidth ); aRect.AdjustRight(nLogPixelWidth ); aRect.AdjustBottom(nLogPixelWidth );
        CalcHatchValues( aRect, nWidth, rHatch.GetAngle(), aPt1, aPt2, aInc, aEndPt1 );
        if (utl::ConfigManager::IsFuzzing() && !HasSaneNSteps(aPt1, aEndPt1, aInc))
            return;

        if (aInc.Width() <= 0 && aInc.Height() <= 0)
            SAL_WARN("vcl.gdi", "invalid increment");
        else
        {
            do
            {
                DrawHatchLine( tools::Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer.get(), bMtf );
                aPt1.AdjustX(aInc.Width() ); aPt1.AdjustY(aInc.Height() );
                aPt2.AdjustX(aInc.Width() ); aPt2.AdjustY(aInc.Height() );
            }
            while( ( aPt1.X() <= aEndPt1.X() ) && ( aPt1.Y() <= aEndPt1.Y() ) );
        }

        if( ( rHatch.GetStyle() == HatchStyle::Double ) || ( rHatch.GetStyle() == HatchStyle::Triple ) )
        {
            // Double hatch
            CalcHatchValues( aRect, nWidth, rHatch.GetAngle() + 900_deg10, aPt1, aPt2, aInc, aEndPt1 );
            if (utl::ConfigManager::IsFuzzing() && !HasSaneNSteps(aPt1, aEndPt1, aInc))
                return;

            do
            {
                DrawHatchLine( tools::Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer.get(), bMtf );
                aPt1.AdjustX(aInc.Width() ); aPt1.AdjustY(aInc.Height() );
                aPt2.AdjustX(aInc.Width() ); aPt2.AdjustY(aInc.Height() );
            }
            while( ( aPt1.X() <= aEndPt1.X() ) && ( aPt1.Y() <= aEndPt1.Y() ) );

            if( rHatch.GetStyle() == HatchStyle::Triple )
            {
                // Triple hatch
                CalcHatchValues( aRect, nWidth, rHatch.GetAngle() + 450_deg10, aPt1, aPt2, aInc, aEndPt1 );
                if (utl::ConfigManager::IsFuzzing() && !HasSaneNSteps(aPt1, aEndPt1, aInc))
                    return;

                do
                {
                    DrawHatchLine( tools::Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer.get(), bMtf );
                    aPt1.AdjustX(aInc.Width() ); aPt1.AdjustY(aInc.Height() );
                    aPt2.AdjustX(aInc.Width() ); aPt2.AdjustY(aInc.Height() );
                }
                while( ( aPt1.X() <= aEndPt1.X() ) && ( aPt1.Y() <= aEndPt1.Y() ) );
            }
        }
    }
}

void OutputDevice::CalcHatchValues( const tools::Rectangle& rRect, tools::Long nDist, Degree10 nAngle10,
                                    Point& rPt1, Point& rPt2, Size& rInc, Point& rEndPt1 )
{
    Point   aRef;
    Degree10    nAngle = nAngle10 % 1800_deg10;
    tools::Long    nOffset = 0;

    if( nAngle > 900_deg10 )
        nAngle -= 1800_deg10;

    aRef = ( !IsRefPoint() ? rRect.TopLeft() : GetRefPoint() );

    if( 0_deg10 == nAngle )
    {
        rInc = Size( 0, nDist );
        rPt1 = rRect.TopLeft();
        rPt2 = rRect.TopRight();
        rEndPt1 = rRect.BottomLeft();

        if( aRef.Y() <= rRect.Top() )
            nOffset = ( ( rRect.Top() - aRef.Y() ) % nDist );
        else
            nOffset = ( nDist - ( ( aRef.Y() - rRect.Top() ) % nDist ) );

        rPt1.AdjustY( -nOffset );
        rPt2.AdjustY( -nOffset );
    }
    else if( 900_deg10 == nAngle )
    {
        rInc = Size( nDist, 0 );
        rPt1 = rRect.TopLeft();
        rPt2 = rRect.BottomLeft();
        rEndPt1 = rRect.TopRight();

        if( aRef.X() <= rRect.Left() )
            nOffset = ( rRect.Left() - aRef.X() ) % nDist;
        else
            nOffset = nDist - ( ( aRef.X() - rRect.Left() ) % nDist );

        rPt1.AdjustX( -nOffset );
        rPt2.AdjustX( -nOffset );
    }
    else if( nAngle >= Degree10(-450) && nAngle <= 450_deg10 )
    {
        const double    fAngle = std::abs( toRadians(nAngle) );
        const double    fTan = tan( fAngle );
        const tools::Long      nYOff = basegfx::fround<tools::Long>( ( rRect.Right() - rRect.Left() ) * fTan );
        tools::Long            nPY;

        nDist = basegfx::fround<tools::Long>(nDist / cos(fAngle));
        rInc = Size( 0, nDist );

        if( nAngle > 0_deg10 )
        {
            rPt1 = rRect.TopLeft();
            rPt2 = Point( rRect.Right(), rRect.Top() - nYOff );
            rEndPt1 = Point( rRect.Left(), rRect.Bottom() + nYOff );
            nPY = basegfx::fround<tools::Long>(aRef.Y() - ((rPt1.X() - aRef.X()) * fTan));
        }
        else
        {
            rPt1 = rRect.TopRight();
            rPt2 = Point( rRect.Left(), rRect.Top() - nYOff );
            rEndPt1 = Point( rRect.Right(), rRect.Bottom() + nYOff );
            nPY = basegfx::fround<tools::Long>(aRef.Y() + ((rPt1.X() - aRef.X()) * fTan));
        }

        if( nPY <= rPt1.Y() )
            nOffset = ( rPt1.Y() - nPY ) % nDist;
        else
            nOffset = nDist - ( ( nPY - rPt1.Y() ) % nDist );

        rPt1.AdjustY( -nOffset );
        rPt2.AdjustY( -nOffset );
    }
    else
    {
        const double fAngle = std::abs( toRadians(nAngle) );
        const double fTan = tan( fAngle );
        const tools::Long   nXOff = basegfx::fround<tools::Long>( (static_cast<double>(rRect.Bottom()) - rRect.Top()) / fTan );
        tools::Long         nPX;

        nDist = basegfx::fround<tools::Long>(nDist / sin(fAngle));
        rInc = Size( nDist, 0 );

        if( nAngle > 0_deg10 )
        {
            rPt1 = rRect.TopLeft();
            rPt2 = Point( rRect.Left() - nXOff, rRect.Bottom() );
            rEndPt1 = Point( rRect.Right() + nXOff, rRect.Top() );
            nPX = basegfx::fround<tools::Long>( aRef.X() - ( (static_cast<double>(rPt1.Y()) - aRef.Y()) / fTan ) );
        }
        else
        {
            rPt1 = rRect.BottomLeft();
            rPt2 = Point( rRect.Left() - nXOff, rRect.Top() );
            rEndPt1 = Point( rRect.Right() + nXOff, rRect.Bottom() );
            nPX = basegfx::fround<tools::Long>( aRef.X() + ( (static_cast<double>(rPt1.Y()) - aRef.Y()) / fTan ) );
        }

        if( nPX <= rPt1.X() )
            nOffset = ( rPt1.X() - nPX ) % nDist;
        else
            nOffset = nDist - ( ( nPX - rPt1.X() ) % nDist );

        rPt1.AdjustX( -nOffset );
        rPt2.AdjustX( -nOffset );
    }
}

void OutputDevice::DrawHatchLine( const tools::Line& rLine, const tools::PolyPolygon& rPolyPoly,
                                      Point* pPtBuffer, bool bMtf )
{
    assert(!is_double_buffered_window());

    double  fX, fY;
    tools::Long    nAdd, nPCounter = 0;

    for( tools::Long nPoly = 0, nPolyCount = rPolyPoly.Count(); nPoly < nPolyCount; nPoly++ )
    {
        const tools::Polygon& rPoly = rPolyPoly[ static_cast<sal_uInt16>(nPoly) ];

        if( rPoly.GetSize() > 1 )
        {
            tools::Line aCurSegment( rPoly[ 0 ], Point() );

            for( tools::Long i = 1, nCount = rPoly.GetSize(); i <= nCount; i++ )
            {
                aCurSegment.SetEnd( rPoly[ static_cast<sal_uInt16>( i % nCount ) ] );
                nAdd = 0;

                if( rLine.Intersection( aCurSegment, fX, fY ) )
                {
                    if( ( fabs( fX - aCurSegment.GetStart().X() ) <= 0.0000001 ) &&
                        ( fabs( fY - aCurSegment.GetStart().Y() ) <= 0.0000001 ) )
                    {
                        const tools::Line aPrevSegment( rPoly[ static_cast<sal_uInt16>( ( i > 1 ) ? ( i - 2 ) : ( nCount - 1 ) ) ], aCurSegment.GetStart() );
                        const double    fPrevDistance = rLine.GetDistance( aPrevSegment.GetStart() );
                        const double    fCurDistance = rLine.GetDistance( aCurSegment.GetEnd() );

                        if( ( fPrevDistance <= 0.0 && fCurDistance > 0.0 ) ||
                            ( fPrevDistance > 0.0 && fCurDistance < 0.0 ) )
                        {
                            nAdd = 1;
                        }
                    }
                    else if( ( fabs( fX - aCurSegment.GetEnd().X() ) <= 0.0000001 ) &&
                             ( fabs( fY - aCurSegment.GetEnd().Y() ) <= 0.0000001 ) )
                    {
                        const tools::Line aNextSegment( aCurSegment.GetEnd(), rPoly[ static_cast<sal_uInt16>( ( i + 1 ) % nCount ) ] );

                        if( ( fabs( rLine.GetDistance( aNextSegment.GetEnd() ) ) <= 0.0000001 ) &&
                            ( rLine.GetDistance( aCurSegment.GetStart() ) > 0.0 ) )
                        {
                            nAdd = 1;
                        }
                    }
                    else
                        nAdd = 1;

                    if( nAdd )
                    {
                        if (nPCounter == HATCH_MAXPOINTS)
                        {
                            SAL_WARN("vcl.gdi", "too many hatch points");
                            return;
                        }
                        pPtBuffer[nPCounter++] = Point(basegfx::fround<tools::Long>(fX),
                                                       basegfx::fround<tools::Long>(fY));
                    }
                }

                aCurSegment.SetStart( aCurSegment.GetEnd() );
            }
        }
    }

    if( nPCounter <= 1 )
        return;

    qsort( pPtBuffer, nPCounter, sizeof( Point ), HatchCmpFnc );

    if( nPCounter & 1 )
        nPCounter--;

    if( bMtf )
    {
        for( tools::Long i = 0; i < nPCounter; i += 2 )
            mpMetaFile->AddAction( new MetaLineAction( pPtBuffer[ i ], pPtBuffer[ i + 1 ] ) );
    }
    else
    {
        for( tools::Long i = 0; i < nPCounter; i += 2 )
            DrawHatchLine_DrawLine(pPtBuffer[i], pPtBuffer[i+1]);
    }
}

void OutputDevice::DrawHatchLine_DrawLine(const Point& rStartPoint, const Point& rEndPoint)
{
    Point aPt1{ImplLogicToDevicePixel(rStartPoint)}, aPt2{ImplLogicToDevicePixel(rEndPoint)};
    mpGraphics->DrawLine(aPt1.X(), aPt1.Y(), aPt2.X(), aPt2.Y(), *this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
