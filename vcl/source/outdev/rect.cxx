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

#include <tools/poly.hxx>

#include <vcl/virdev.hxx>
#include <vcl/outdev.hxx>

#include <salgdi.hxx>

void OutputDevice::DrawRect( const Rectangle& rRect )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRectAction( rRect ) );

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || ImplIsRecordLayout() )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;
    aRect.Justify();

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
        InitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();

    mpGraphics->DrawRect( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), this );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawRect( rRect );
}

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
        InitLineColor();
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
