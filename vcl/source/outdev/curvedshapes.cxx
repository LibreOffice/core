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

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

#include <salgdi.hxx>

void OutputDevice::DrawEllipse( const Rectangle& rRect )
{
    assert(!dynamic_cast<vcl::Window*>(this) || !dynamic_cast<vcl::Window*>(this)->SupportsDoubleBuffering());

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
        if ( !AcquireGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        InitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    Polygon aRectPoly( aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1 );
    if ( aRectPoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = reinterpret_cast<const SalPoint*>(aRectPoly.GetConstPointAry());
        if ( !mbFillColor )
            mpGraphics->DrawPolyLine( aRectPoly.GetSize(), pPtAry, this );
        else
        {
            if ( mbInitFillColor )
                InitFillColor();
            mpGraphics->DrawPolygon( aRectPoly.GetSize(), pPtAry, this );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawEllipse( rRect );
}

void OutputDevice::DrawArc( const Rectangle& rRect,
                            const Point& rStartPt, const Point& rEndPt )
{
    assert(!dynamic_cast<vcl::Window*>(this) || !dynamic_cast<vcl::Window*>(this)->SupportsDoubleBuffering());

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
        if ( !AcquireGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        InitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    const Point     aStart( ImplLogicToDevicePixel( rStartPt ) );
    const Point     aEnd( ImplLogicToDevicePixel( rEndPt ) );
    Polygon         aArcPoly( aRect, aStart, aEnd, POLY_ARC );

    if ( aArcPoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = reinterpret_cast<const SalPoint*>(aArcPoly.GetConstPointAry());
        mpGraphics->DrawPolyLine( aArcPoly.GetSize(), pPtAry, this );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawArc( rRect, rStartPt, rEndPt );
}

void OutputDevice::DrawPie( const Rectangle& rRect,
                            const Point& rStartPt, const Point& rEndPt )
{
    assert(!dynamic_cast<vcl::Window*>(this) || !dynamic_cast<vcl::Window*>(this)->SupportsDoubleBuffering());

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
        if ( !AcquireGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        InitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    const Point     aStart( ImplLogicToDevicePixel( rStartPt ) );
    const Point     aEnd( ImplLogicToDevicePixel( rEndPt ) );
    Polygon         aPiePoly( aRect, aStart, aEnd, POLY_PIE );

    if ( aPiePoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = reinterpret_cast<const SalPoint*>(aPiePoly.GetConstPointAry());
        if ( !mbFillColor )
            mpGraphics->DrawPolyLine( aPiePoly.GetSize(), pPtAry, this );
        else
        {
            if ( mbInitFillColor )
                InitFillColor();
            mpGraphics->DrawPolygon( aPiePoly.GetSize(), pPtAry, this );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPie( rRect, rStartPt, rEndPt );
}

void OutputDevice::DrawChord( const Rectangle& rRect,
                              const Point& rStartPt, const Point& rEndPt )
{
    assert(!dynamic_cast<vcl::Window*>(this) || !dynamic_cast<vcl::Window*>(this)->SupportsDoubleBuffering());

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
        if ( !AcquireGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        InitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    const Point     aStart( ImplLogicToDevicePixel( rStartPt ) );
    const Point     aEnd( ImplLogicToDevicePixel( rEndPt ) );
    Polygon         aChordPoly( aRect, aStart, aEnd, POLY_CHORD );

    if ( aChordPoly.GetSize() >= 2 )
    {
        const SalPoint* pPtAry = reinterpret_cast<const SalPoint*>(aChordPoly.GetConstPointAry());
        if ( !mbFillColor )
            mpGraphics->DrawPolyLine( aChordPoly.GetSize(), pPtAry, this );
        else
        {
            if ( mbInitFillColor )
                InitFillColor();
            mpGraphics->DrawPolygon( aChordPoly.GetSize(), pPtAry, this );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawChord( rRect, rStartPt, rEndPt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
