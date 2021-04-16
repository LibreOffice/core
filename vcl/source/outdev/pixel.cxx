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

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <drawmode.hxx>
#include <salgdi.hxx>

Color OutputDevice::GetPixel(const Point& rPoint) const
{
    Color aColor;

    if (mpGraphics || AcquireGraphics())
    {
        assert(mpGraphics);
        if (mbInitClipRegion)
            const_cast<OutputDevice*>(this)->InitClipRegion();

        if (!mbOutputClipped)
        {
            const tools::Long nX = ImplLogicXToDevicePixel(rPoint.X());
            const tools::Long nY = ImplLogicYToDevicePixel(rPoint.Y());
            aColor = mpGraphics->GetPixel(nX, nY, *this);

            if (mpAlphaVDev)
            {
                Color aAlphaColor = mpAlphaVDev->GetPixel(rPoint);
                aColor.SetAlpha(aAlphaColor.GetBlue());
            }
        }
    }
    return aColor;
}

void OutputDevice::DrawPixel( const Point& rPt )
{
    assert(!is_double_buffered_window());

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPointAction( rPt ) );

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ImplIsRecordLayout() )
        return;

    Point aPt = ImplLogicToDevicePixel( rPt );

    if ( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        InitLineColor();

    mpGraphics->DrawPixel( aPt.X(), aPt.Y(), *this );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPixel( rPt );
}

void OutputDevice::DrawPixel( const Point& rPt, const Color& rColor )
{
    assert(!is_double_buffered_window());

    Color aColor = vcl::drawmode::GetLineColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings());

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPixelAction( rPt, aColor ) );

    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    Point aPt = ImplLogicToDevicePixel( rPt );

    if ( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    mpGraphics->DrawPixel( aPt.X(), aPt.Y(), aColor, *this );

    if (mpAlphaVDev)
    {
        Color aAlphaColor(rColor.GetAlpha(), rColor.GetAlpha(), rColor.GetAlpha());
        mpAlphaVDev->DrawPixel(rPt, aAlphaColor);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
