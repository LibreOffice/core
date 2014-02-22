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


#include <tools/debug.hxx>
#include <tools/poly.hxx>

#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>
#include <svdata.hxx>
#include <outdata.hxx>
#include <outdev.h>

// =======================================================================


// =======================================================================

void OutputDevice::DrawRect( const Rectangle& rRect,
                             sal_uLong nHorzRound, sal_uLong nVertRound )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRoundRectAction( rRect, nHorzRound, nVertRound ) );

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || ImplIsRecordLayout() )
        return;

    const Rectangle aRect( ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;

    nHorzRound = ImplLogicWidthToDevicePixel( nHorzRound );
    nVertRound = ImplLogicHeightToDevicePixel( nVertRound );

    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();

    if ( !nHorzRound && !nVertRound )
        mpGraphics->DrawRect( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), this );
    else
    {
        const Polygon aRoundRectPoly( aRect, nHorzRound, nVertRound );

        if ( aRoundRectPoly.GetSize() >= 2 )
        {
            const SalPoint* pPtAry = (const SalPoint*) aRoundRectPoly.GetConstPointAry();

            if ( !mbFillColor )
                mpGraphics->DrawPolyLine( aRoundRectPoly.GetSize(), pPtAry, this );
            else
                mpGraphics->DrawPolygon( aRoundRectPoly.GetSize(), pPtAry, this );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawRect( rRect, nHorzRound, nVertRound );
}



void OutputDevice::DrawEllipse( const Rectangle& rRect )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaEllipseAction( rRect ) );

    if  ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || ImplIsRecordLayout() )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );
    if ( aRect.IsEmpty() )
        return;

    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    Polygon aRectPoly( aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1 );
    if ( aRectPoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = (const SalPoint*)aRectPoly.GetConstPointAry();
        if ( !mbFillColor )
            mpGraphics->DrawPolyLine( aRectPoly.GetSize(), pPtAry, this );
        else
        {
            if ( mbInitFillColor )
                ImplInitFillColor();
            mpGraphics->DrawPolygon( aRectPoly.GetSize(), pPtAry, this );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawEllipse( rRect );
}



void OutputDevice::DrawArc( const Rectangle& rRect,
                            const Point& rStartPt, const Point& rEndPt )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaArcAction( rRect, rStartPt, rEndPt ) );

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ImplIsRecordLayout() )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );
    if ( aRect.IsEmpty() )
        return;

    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    const Point     aStart( ImplLogicToDevicePixel( rStartPt ) );
    const Point     aEnd( ImplLogicToDevicePixel( rEndPt ) );
    Polygon         aArcPoly( aRect, aStart, aEnd, POLY_ARC );

    if ( aArcPoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = (const SalPoint*)aArcPoly.GetConstPointAry();
        mpGraphics->DrawPolyLine( aArcPoly.GetSize(), pPtAry, this );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawArc( rRect, rStartPt, rEndPt );
}



void OutputDevice::DrawPie( const Rectangle& rRect,
                            const Point& rStartPt, const Point& rEndPt )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPieAction( rRect, rStartPt, rEndPt ) );

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || ImplIsRecordLayout() )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );
    if ( aRect.IsEmpty() )
        return;

    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    const Point     aStart( ImplLogicToDevicePixel( rStartPt ) );
    const Point     aEnd( ImplLogicToDevicePixel( rEndPt ) );
    Polygon         aPiePoly( aRect, aStart, aEnd, POLY_PIE );

    if ( aPiePoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = (const SalPoint*)aPiePoly.GetConstPointAry();
        if ( !mbFillColor )
            mpGraphics->DrawPolyLine( aPiePoly.GetSize(), pPtAry, this );
        else
        {
            if ( mbInitFillColor )
                ImplInitFillColor();
            mpGraphics->DrawPolygon( aPiePoly.GetSize(), pPtAry, this );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPie( rRect, rStartPt, rEndPt );
}



void OutputDevice::DrawChord( const Rectangle& rRect,
                              const Point& rStartPt, const Point& rEndPt )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaChordAction( rRect, rStartPt, rEndPt ) );

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || ImplIsRecordLayout() )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );
    if ( aRect.IsEmpty() )
        return;

    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    const Point     aStart( ImplLogicToDevicePixel( rStartPt ) );
    const Point     aEnd( ImplLogicToDevicePixel( rEndPt ) );
    Polygon         aChordPoly( aRect, aStart, aEnd, POLY_CHORD );

    if ( aChordPoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = (const SalPoint*)aChordPoly.GetConstPointAry();
        if ( !mbFillColor )
            mpGraphics->DrawPolyLine( aChordPoly.GetSize(), pPtAry, this );
        else
        {
            if ( mbInitFillColor )
                ImplInitFillColor();
            mpGraphics->DrawPolygon( aChordPoly.GetSize(), pPtAry, this );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawChord( rRect, rStartPt, rEndPt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
