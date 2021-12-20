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

#include <cassert>
#include <cstdlib>

#include <osl/diagnose.h>
#include <tools/line.hxx>
#include <tools/helpers.hxx>

#include <vcl/hatch.hxx>
#include <vcl/metaact.hxx>
#include <vcl/settings.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <drawmode.hxx>
#include <salgdi.hxx>

#include <memory>

#define HATCH_MAXPOINTS             1024

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

        tools::Rectangle aRect(rPolyPoly.GetBoundRect());
        DrawHatch(aPolyPoly, aHatch, !IsRefPoint() ? aRect.TopLeft() : GetRefPoint(), false);

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

        tools::Rectangle aRect(rPolyPoly.GetBoundRect());
        DrawHatch( aPolyPoly, rHatch, !IsRefPoint() ? aRect.TopLeft() : GetRefPoint(), true );

        mpMetaFile->AddAction( new MetaPopAction() );
        mpMetaFile = pOldMtf;
    }
}

void OutputDevice::DrawHatch( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch, Point const& rRef, bool bMtf )
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
        DrawHatch(aPolyPoly, rHatch, rRef, bMtf);
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
        Hatch::CalcHatchValues( aRect, nWidth, rHatch.GetAngle(), aPt1, aPt2, aInc, aEndPt1, rRef);
        do
        {
            DrawHatchLines( tools::Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer.get(), bMtf );
            aPt1.AdjustX(aInc.Width() ); aPt1.AdjustY(aInc.Height() );
            aPt2.AdjustX(aInc.Width() ); aPt2.AdjustY(aInc.Height() );
        }
        while( ( aPt1.X() <= aEndPt1.X() ) && ( aPt1.Y() <= aEndPt1.Y() ) );

        if( ( rHatch.GetStyle() == HatchStyle::Double ) || ( rHatch.GetStyle() == HatchStyle::Triple ) )
        {
            // Double hatch
            Hatch::CalcHatchValues( aRect, nWidth, rHatch.GetAngle() + 900_deg10, aPt1, aPt2, aInc, aEndPt1, rRef);
            do
            {
                DrawHatchLines( tools::Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer.get(), bMtf );
                aPt1.AdjustX(aInc.Width() ); aPt1.AdjustY(aInc.Height() );
                aPt2.AdjustX(aInc.Width() ); aPt2.AdjustY(aInc.Height() );
            }
            while( ( aPt1.X() <= aEndPt1.X() ) && ( aPt1.Y() <= aEndPt1.Y() ) );

            if( rHatch.GetStyle() == HatchStyle::Triple )
            {
                // Triple hatch
                Hatch::CalcHatchValues( aRect, nWidth, rHatch.GetAngle() + 450_deg10, aPt1, aPt2, aInc, aEndPt1, rRef);
                do
                {
                    DrawHatchLines( tools::Line( aPt1, aPt2 ), rPolyPoly, pPtBuffer.get(), bMtf );
                    aPt1.AdjustX(aInc.Width() ); aPt1.AdjustY(aInc.Height() );
                    aPt2.AdjustX(aInc.Width() ); aPt2.AdjustY(aInc.Height() );
                }
                while( ( aPt1.X() <= aEndPt1.X() ) && ( aPt1.Y() <= aEndPt1.Y() ) );
            }
        }
    }
}

void OutputDevice::DrawHatchLines(tools::Line const& rLine, tools::PolyPolygon const& rPolyPoly,
                                  Point* pPtBuffer, bool bMtf)
{
    assert(!is_double_buffered_window());

    tools::Long nPCounter = 0;

    Hatch::GenerateHatchLinePoints(rLine, rPolyPoly, nPCounter, pPtBuffer);

    if (nPCounter <= 1)
        return;

    if( bMtf )
    {
        for( tools::Long i = 0; i < nPCounter; i += 2 )
        {
            mpMetaFile->AddAction( new MetaLineAction( pPtBuffer[ i ], pPtBuffer[ i + 1 ] ) );
        }
    }
    else
    {
        for( tools::Long i = 0; i < nPCounter; i += 2 )
        {
            DrawHatchLine(pPtBuffer[i], pPtBuffer[i+1]);
        }
    }
}

void OutputDevice::DrawHatchLine(const Point& rStartPoint, const Point& rEndPoint)
{
    Point aPt1{ImplLogicToDevicePixel(rStartPoint)}, aPt2{ImplLogicToDevicePixel(rEndPoint)};
    mpGraphics->DrawLine(aPt1.X(), aPt1.Y(), aPt2.X(), aPt2.Y(), *this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
