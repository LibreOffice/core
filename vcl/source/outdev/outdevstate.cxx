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

#include <sal/types.h>

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

#include <vcl/mapmod.hxx>
#include <vcl/region.hxx>
#include <vcl/font.hxx>
#include <vcl/vclenum.hxx>

#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <tools/fontenum.hxx>

#include <vcl/outdevstate.hxx>
#include "sallayout.hxx"

OutDevState::~OutDevState()
{
    if ( mnFlags & PUSH_LINECOLOR )
        delete mpLineColor;
    if ( mnFlags & PUSH_FILLCOLOR )
        delete mpFillColor;
    if ( mnFlags & PUSH_FONT )
        delete mpFont;
    if ( mnFlags & PUSH_TEXTCOLOR )
        delete mpTextColor;
    if ( mnFlags & PUSH_TEXTFILLCOLOR )
        delete mpTextFillColor;
    if ( mnFlags & PUSH_TEXTLINECOLOR )
        delete mpTextLineColor;
    if ( mnFlags & PUSH_OVERLINECOLOR )
        delete mpOverlineColor;
    if ( mnFlags & PUSH_MAPMODE )
        delete mpMapMode;
    if ( mnFlags & PUSH_CLIPREGION )
        delete mpClipRegion;
    if ( mnFlags & PUSH_REFPOINT )
        delete mpRefPoint;
}

void OutputDevice::SetFont( const Font& rNewFont )
{

    Font aFont( rNewFont );
    aFont.SetLanguage(rNewFont.GetLanguage());
    if ( mnDrawMode & (DRAWMODE_BLACKTEXT | DRAWMODE_WHITETEXT | DRAWMODE_GRAYTEXT | DRAWMODE_GHOSTEDTEXT | DRAWMODE_SETTINGSTEXT |
                       DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL | DRAWMODE_GRAYFILL | DRAWMODE_NOFILL |
                       DRAWMODE_GHOSTEDFILL | DRAWMODE_SETTINGSFILL ) )
    {
        Color aTextColor( aFont.GetColor() );

        if ( mnDrawMode & DRAWMODE_BLACKTEXT )
            aTextColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITETEXT )
            aTextColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYTEXT )
        {
            const sal_uInt8 cLum = aTextColor.GetLuminance();
            aTextColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DRAWMODE_SETTINGSTEXT )
            aTextColor = GetSettings().GetStyleSettings().GetFontColor();

        if ( mnDrawMode & DRAWMODE_GHOSTEDTEXT )
        {
            aTextColor = Color( (aTextColor.GetRed() >> 1 ) | 0x80,
                                (aTextColor.GetGreen() >> 1 ) | 0x80,
                                (aTextColor.GetBlue() >> 1 ) | 0x80 );
        }

        aFont.SetColor( aTextColor );

        bool bTransFill = aFont.IsTransparent();
        if ( !bTransFill )
        {
            Color aTextFillColor( aFont.GetFillColor() );

            if ( mnDrawMode & DRAWMODE_BLACKFILL )
                aTextFillColor = Color( COL_BLACK );
            else if ( mnDrawMode & DRAWMODE_WHITEFILL )
                aTextFillColor = Color( COL_WHITE );
            else if ( mnDrawMode & DRAWMODE_GRAYFILL )
            {
                const sal_uInt8 cLum = aTextFillColor.GetLuminance();
                aTextFillColor = Color( cLum, cLum, cLum );
            }
            else if( mnDrawMode & DRAWMODE_SETTINGSFILL )
                aTextFillColor = GetSettings().GetStyleSettings().GetWindowColor();
            else if ( mnDrawMode & DRAWMODE_NOFILL )
            {
                aTextFillColor = Color( COL_TRANSPARENT );
                bTransFill = true;
            }

            if ( !bTransFill && (mnDrawMode & DRAWMODE_GHOSTEDFILL) )
            {
                aTextFillColor = Color( (aTextFillColor.GetRed() >> 1) | 0x80,
                                        (aTextFillColor.GetGreen() >> 1) | 0x80,
                                        (aTextFillColor.GetBlue() >> 1) | 0x80 );
            }

            aFont.SetFillColor( aTextFillColor );
        }
    }

    if ( mpMetaFile )
    {
        mpMetaFile->AddAction( new MetaFontAction( aFont ) );
        // the color and alignment actions don't belong here
        // TODO: get rid of them without breaking anything...
        mpMetaFile->AddAction( new MetaTextAlignAction( aFont.GetAlign() ) );
        mpMetaFile->AddAction( new MetaTextFillColorAction( aFont.GetFillColor(), !aFont.IsTransparent() ) );
    }

    if ( !maFont.IsSameInstance( aFont ) )
    {
        // Optimization MT/HDU: COL_TRANSPARENT means SetFont should ignore the font color,
        // because SetTextColor() is used for this.
        // #i28759# maTextColor might have been changed behind our back, commit then, too.
        if( aFont.GetColor() != COL_TRANSPARENT
        && (aFont.GetColor() != maFont.GetColor() || aFont.GetColor() != maTextColor ) )
        {
            maTextColor = aFont.GetColor();
            mbInitTextColor = true;
            if( mpMetaFile )
                mpMetaFile->AddAction( new MetaTextColorAction( aFont.GetColor() ) );
        }
        maFont      = aFont;
        mbNewFont   = true;

        if( mpAlphaVDev )
        {
            // #i30463#
            // Since SetFont might change the text color, apply that only
            // selectively to alpha vdev (which normally paints opaque text
            // with COL_BLACK)
            if( aFont.GetColor() != COL_TRANSPARENT )
            {
                mpAlphaVDev->SetTextColor( COL_BLACK );
                aFont.SetColor( COL_TRANSPARENT );
            }

            mpAlphaVDev->SetFont( aFont );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
