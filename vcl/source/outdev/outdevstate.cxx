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

#include "outdev.h"
#include "outdata.hxx"
#include "salgdi.hxx"
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


void OutputDevice::Push( sal_uInt16 nFlags )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPushAction( nFlags ) );

    OutDevState* pState = new OutDevState;

    pState->mnFlags = nFlags;

    if ( nFlags & PUSH_LINECOLOR )
    {
        if ( mbLineColor )
            pState->mpLineColor = new Color( maLineColor );
        else
            pState->mpLineColor = NULL;
    }
    if ( nFlags & PUSH_FILLCOLOR )
    {
        if ( mbFillColor )
            pState->mpFillColor = new Color( maFillColor );
        else
            pState->mpFillColor = NULL;
    }
    if ( nFlags & PUSH_FONT )
        pState->mpFont = new Font( maFont );
    if ( nFlags & PUSH_TEXTCOLOR )
        pState->mpTextColor = new Color( GetTextColor() );
    if ( nFlags & PUSH_TEXTFILLCOLOR )
    {
        if ( IsTextFillColor() )
            pState->mpTextFillColor = new Color( GetTextFillColor() );
        else
            pState->mpTextFillColor = NULL;
    }
    if ( nFlags & PUSH_TEXTLINECOLOR )
    {
        if ( IsTextLineColor() )
            pState->mpTextLineColor = new Color( GetTextLineColor() );
        else
            pState->mpTextLineColor = NULL;
    }
    if ( nFlags & PUSH_OVERLINECOLOR )
    {
        if ( IsOverlineColor() )
            pState->mpOverlineColor = new Color( GetOverlineColor() );
        else
            pState->mpOverlineColor = NULL;
    }
    if ( nFlags & PUSH_TEXTALIGN )
        pState->meTextAlign = GetTextAlign();
    if( nFlags & PUSH_TEXTLAYOUTMODE )
        pState->mnTextLayoutMode = GetLayoutMode();
    if( nFlags & PUSH_TEXTLANGUAGE )
        pState->meTextLanguage = GetDigitLanguage();
    if ( nFlags & PUSH_RASTEROP )
        pState->meRasterOp = GetRasterOp();
    if ( nFlags & PUSH_MAPMODE )
    {
        pState->mpMapMode = new MapMode( maMapMode );
        pState->mbMapActive = mbMap;
    }
    if ( nFlags & PUSH_CLIPREGION )
    {
        if ( mbClipRegion )
            pState->mpClipRegion = new Region( maRegion );
        else
            pState->mpClipRegion = NULL;
    }
    if ( nFlags & PUSH_REFPOINT )
    {
        if ( mbRefPoint )
            pState->mpRefPoint = new Point( maRefPoint );
        else
            pState->mpRefPoint = NULL;
    }

    mpOutDevStateStack->push_back( pState );

    if( mpAlphaVDev )
        mpAlphaVDev->Push();
}

void OutputDevice::Pop()
{

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPopAction() );

    GDIMetaFile* pOldMetaFile = mpMetaFile;
    mpMetaFile = NULL;

    if ( mpOutDevStateStack->empty() )
    {
        SAL_WARN( "vcl.gdi", "OutputDevice::Pop() without OutputDevice::Push()" );
        return;
    }
    const OutDevState& rState = mpOutDevStateStack->back();

    if( mpAlphaVDev )
        mpAlphaVDev->Pop();

    if ( rState.mnFlags & PUSH_LINECOLOR )
    {
        if ( rState.mpLineColor )
            SetLineColor( *rState.mpLineColor );
        else
            SetLineColor();
    }

    if ( rState.mnFlags & PUSH_FILLCOLOR )
    {
        if ( rState.mpFillColor )
            SetFillColor( *rState.mpFillColor );
        else
            SetFillColor();
    }

    if ( rState.mnFlags & PUSH_FONT )
        SetFont( *rState.mpFont );

    if ( rState.mnFlags & PUSH_TEXTCOLOR )
        SetTextColor( *rState.mpTextColor );

    if ( rState.mnFlags & PUSH_TEXTFILLCOLOR )
    {
        if ( rState.mpTextFillColor )
            SetTextFillColor( *rState.mpTextFillColor );
        else
            SetTextFillColor();
    }

    if ( rState.mnFlags & PUSH_TEXTLINECOLOR )
    {
        if ( rState.mpTextLineColor )
            SetTextLineColor( *rState.mpTextLineColor );
        else
            SetTextLineColor();
    }

    if ( rState.mnFlags & PUSH_OVERLINECOLOR )
    {
        if ( rState.mpOverlineColor )
            SetOverlineColor( *rState.mpOverlineColor );
        else
            SetOverlineColor();
    }

    if ( rState.mnFlags & PUSH_TEXTALIGN )
        SetTextAlign( rState.meTextAlign );

    if( rState.mnFlags & PUSH_TEXTLAYOUTMODE )
        SetLayoutMode( rState.mnTextLayoutMode );

    if( rState.mnFlags & PUSH_TEXTLANGUAGE )
        SetDigitLanguage( rState.meTextLanguage );

    if ( rState.mnFlags & PUSH_RASTEROP )
        SetRasterOp( rState.meRasterOp );

    if ( rState.mnFlags & PUSH_MAPMODE )
    {
        if ( rState.mpMapMode )
            SetMapMode( *rState.mpMapMode );
        else
            SetMapMode();
        mbMap = rState.mbMapActive;
    }

    if ( rState.mnFlags & PUSH_CLIPREGION )
        SetDeviceClipRegion( rState.mpClipRegion );

    if ( rState.mnFlags & PUSH_REFPOINT )
    {
        if ( rState.mpRefPoint )
            SetRefPoint( *rState.mpRefPoint );
        else
            SetRefPoint();
    }

    mpOutDevStateStack->pop_back();

    mpMetaFile = pOldMetaFile;
}

sal_uInt32 OutputDevice::GetGCStackDepth() const
{
    return mpOutDevStateStack->size();
}

void OutputDevice::EnableOutput( bool bEnable )
{
    mbOutput = bEnable;

    if( mpAlphaVDev )
        mpAlphaVDev->EnableOutput( bEnable );
}

void OutputDevice::SetAntialiasing( sal_uInt16 nMode )
{
    if ( mnAntialiasing != nMode )
    {
        mnAntialiasing = nMode;
        mbInitFont = true;

        if(mpGraphics)
        {
            mpGraphics->setAntiAliasB2DDraw(mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW);
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetAntialiasing( nMode );
}

void OutputDevice::SetDrawMode( sal_uLong nDrawMode )
{

    mnDrawMode = nDrawMode;

    if( mpAlphaVDev )
        mpAlphaVDev->SetDrawMode( nDrawMode );
}

void OutputDevice::SetLayoutMode( ComplexTextLayoutMode nTextLayoutMode )
{
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaLayoutModeAction( nTextLayoutMode ) );

    mnTextLayoutMode = nTextLayoutMode;

    if( mpAlphaVDev )
        mpAlphaVDev->SetLayoutMode( nTextLayoutMode );
}

void OutputDevice::SetDigitLanguage( LanguageType eTextLanguage )
{
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLanguageAction( eTextLanguage ) );

    meTextLanguage = eTextLanguage;

    if( mpAlphaVDev )
        mpAlphaVDev->SetDigitLanguage( eTextLanguage );
}

void OutputDevice::SetRasterOp( RasterOp eRasterOp )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRasterOpAction( eRasterOp ) );

    if ( meRasterOp != eRasterOp )
    {
        meRasterOp = eRasterOp;
        mbInitLineColor = mbInitFillColor = true;

        if( mpGraphics || AcquireGraphics() )
            mpGraphics->SetXORMode( (ROP_INVERT == meRasterOp) || (ROP_XOR == meRasterOp), ROP_INVERT == meRasterOp );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetRasterOp( eRasterOp );
}


void OutputDevice::SetFillColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaFillColorAction( Color(), false ) );

    if ( mbFillColor )
    {
        mbInitFillColor = true;
        mbFillColor = false;
        maFillColor = Color( COL_TRANSPARENT );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetFillColor();
}

void OutputDevice::SetFillColor( const Color& rColor )
{

    Color aColor( rColor );

    if( mnDrawMode & ( DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL |
                       DRAWMODE_GRAYFILL | DRAWMODE_NOFILL |
                       DRAWMODE_GHOSTEDFILL | DRAWMODE_SETTINGSFILL ) )
    {
        if( !ImplIsColorTransparent( aColor ) )
        {
            if( mnDrawMode & DRAWMODE_BLACKFILL )
            {
                aColor = Color( COL_BLACK );
            }
            else if( mnDrawMode & DRAWMODE_WHITEFILL )
            {
                aColor = Color( COL_WHITE );
            }
            else if( mnDrawMode & DRAWMODE_GRAYFILL )
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }
            else if( mnDrawMode & DRAWMODE_NOFILL )
            {
                aColor = Color( COL_TRANSPARENT );
            }
            else if( mnDrawMode & DRAWMODE_SETTINGSFILL )
            {
                aColor = GetSettings().GetStyleSettings().GetWindowColor();
            }

            if( mnDrawMode & DRAWMODE_GHOSTEDFILL )
            {
                aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                                (aColor.GetGreen() >> 1) | 0x80,
                                (aColor.GetBlue() >> 1) | 0x80);
            }
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaFillColorAction( aColor, true ) );

    if ( ImplIsColorTransparent( aColor ) )
    {
        if ( mbFillColor )
        {
            mbInitFillColor = true;
            mbFillColor = false;
            maFillColor = Color( COL_TRANSPARENT );
        }
    }
    else
    {
        if ( maFillColor != aColor )
        {
            mbInitFillColor = true;
            mbFillColor = true;
            maFillColor = aColor;
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetFillColor( COL_BLACK );
}

void OutputDevice::SetLineColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineColorAction( Color(), false ) );

    if ( mbLineColor )
    {
        mbInitLineColor = true;
        mbLineColor = false;
        maLineColor = Color( COL_TRANSPARENT );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetLineColor();
}

void OutputDevice::SetLineColor( const Color& rColor )
{

    Color aColor = ImplDrawModeToColor( rColor );

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineColorAction( aColor, true ) );

    if( ImplIsColorTransparent( aColor ) )
    {
        if ( mbLineColor )
        {
            mbInitLineColor = true;
            mbLineColor = false;
            maLineColor = Color( COL_TRANSPARENT );
        }
    }
    else
    {
        if( maLineColor != aColor )
        {
            mbInitLineColor = true;
            mbLineColor = true;
            maLineColor = aColor;
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetLineColor( COL_BLACK );
}

void OutputDevice::SetBackground()
{

    maBackground = Wallpaper();
    mbBackground = false;

    if( mpAlphaVDev )
        mpAlphaVDev->SetBackground();
}

void OutputDevice::SetBackground( const Wallpaper& rBackground )
{

    maBackground = rBackground;

    if( rBackground.GetStyle() == WALLPAPER_NULL )
        mbBackground = false;
    else
        mbBackground = true;

    if( mpAlphaVDev )
        mpAlphaVDev->SetBackground( rBackground );
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


void OutputDevice::InitLineColor()
{
    DBG_TESTSOLARMUTEX();

    if( mbLineColor )
    {
        if( ROP_0 == meRasterOp )
            mpGraphics->SetROPLineColor( SAL_ROP_0 );
        else if( ROP_1 == meRasterOp )
            mpGraphics->SetROPLineColor( SAL_ROP_1 );
        else if( ROP_INVERT == meRasterOp )
            mpGraphics->SetROPLineColor( SAL_ROP_INVERT );
        else
            mpGraphics->SetLineColor( ImplColorToSal( maLineColor ) );
    }
    else
        mpGraphics->SetLineColor();

    mbInitLineColor = false;
}


void OutputDevice::InitFillColor()
{
    DBG_TESTSOLARMUTEX();

    if( mbFillColor )
    {
        if( ROP_0 == meRasterOp )
            mpGraphics->SetROPFillColor( SAL_ROP_0 );
        else if( ROP_1 == meRasterOp )
            mpGraphics->SetROPFillColor( SAL_ROP_1 );
        else if( ROP_INVERT == meRasterOp )
            mpGraphics->SetROPFillColor( SAL_ROP_INVERT );
        else
            mpGraphics->SetFillColor( ImplColorToSal( maFillColor ) );
    }
    else
        mpGraphics->SetFillColor();

    mbInitFillColor = false;
}

void OutputDevice::ImplReleaseFonts()
{
    mpGraphics->ReleaseFonts();
    mbNewFont = true;
    mbInitFont = true;

    if ( mpFontEntry )
    {
        mpFontCache->Release( mpFontEntry );
        mpFontEntry = NULL;
    }

    if ( mpGetDevFontList )
    {
        delete mpGetDevFontList;
        mpGetDevFontList = NULL;
    }

    if ( mpGetDevSizeList )
    {
        delete mpGetDevSizeList;
        mpGetDevSizeList = NULL;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
