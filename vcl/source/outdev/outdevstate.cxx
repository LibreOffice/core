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

#include <sal/config.h>
#include <sal/log.hxx>

#include <tools/debug.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdevstate.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

#include <outdev.h>
#include <outdata.hxx>
#include <outdevstatestack.hxx>
#include <salgdi.hxx>

OutDevState::OutDevState()
    : mbMapActive(false)
    , meTextAlign(ALIGN_TOP)
    , meRasterOp(RasterOp::OverPaint)
    , mnTextLayoutMode(ComplexTextLayoutFlags::Default)
    , meTextLanguage(0)
    , mnFlags(PushFlags::NONE)
{
}

OutDevState::~OutDevState()
{
    mpLineColor.reset();
    mpFillColor.reset();
    mpFont.reset();
    mpTextColor.reset();
    mpTextFillColor.reset();
    mpTextLineColor.reset();
    mpOverlineColor.reset();
    mpMapMode.reset();
    mpClipRegion.reset();
    mpRefPoint.reset();
}

void OutputDevice::Push( PushFlags nFlags )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPushAction( nFlags ) );

    OutDevState* pState = new OutDevState;

    pState->mnFlags = nFlags;

    if (nFlags & PushFlags::LINECOLOR && mbLineColor)
    {
        pState->mpLineColor = maLineColor;
    }
    if (nFlags & PushFlags::FILLCOLOR && mbFillColor)
    {
        pState->mpFillColor = maFillColor;
    }
    if ( nFlags & PushFlags::FONT )
        pState->mpFont.reset( new vcl::Font( maFont ) );
    if ( nFlags & PushFlags::TEXTCOLOR )
        pState->mpTextColor = GetTextColor();
    if (nFlags & PushFlags::TEXTFILLCOLOR && IsTextFillColor())
    {
        pState->mpTextFillColor = GetTextFillColor();
    }
    if (nFlags & PushFlags::TEXTLINECOLOR && IsTextLineColor())
    {
        pState->mpTextLineColor = GetTextLineColor();
    }
    if (nFlags & PushFlags::OVERLINECOLOR && IsOverlineColor())
    {
        pState->mpOverlineColor = GetOverlineColor();
    }
    if ( nFlags & PushFlags::TEXTALIGN )
        pState->meTextAlign = GetTextAlign();
    if( nFlags & PushFlags::TEXTLAYOUTMODE )
        pState->mnTextLayoutMode = GetLayoutMode();
    if( nFlags & PushFlags::TEXTLANGUAGE )
        pState->meTextLanguage = GetDigitLanguage();
    if ( nFlags & PushFlags::RASTEROP )
        pState->meRasterOp = GetRasterOp();
    if ( nFlags & PushFlags::MAPMODE )
    {
        pState->mpMapMode = maMapMode;
        pState->mbMapActive = mbMap;
    }
    if (nFlags & PushFlags::CLIPREGION && mbClipRegion)
    {
        pState->mpClipRegion.reset( new vcl::Region( maRegion ) );
    }
    if (nFlags & PushFlags::REFPOINT && mbRefPoint)
    {
        pState->mpRefPoint = maRefPoint;
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
    mpMetaFile = nullptr;

    if ( mpOutDevStateStack->empty() )
    {
        SAL_WARN( "vcl.gdi", "OutputDevice::Pop() without OutputDevice::Push()" );
        return;
    }
    const OutDevState& rState = mpOutDevStateStack->back();

    if( mpAlphaVDev )
        mpAlphaVDev->Pop();

    if ( rState.mnFlags & PushFlags::LINECOLOR )
    {
        if ( rState.mpLineColor )
            SetLineColor( *rState.mpLineColor );
        else
            SetLineColor();
    }

    if ( rState.mnFlags & PushFlags::FILLCOLOR )
    {
        if ( rState.mpFillColor )
            SetFillColor( *rState.mpFillColor );
        else
            SetFillColor();
    }

    if ( rState.mnFlags & PushFlags::FONT )
        SetFont( *rState.mpFont );

    if ( rState.mnFlags & PushFlags::TEXTCOLOR )
        SetTextColor( *rState.mpTextColor );

    if ( rState.mnFlags & PushFlags::TEXTFILLCOLOR )
    {
        if ( rState.mpTextFillColor )
            SetTextFillColor( *rState.mpTextFillColor );
        else
            SetTextFillColor();
    }

    if ( rState.mnFlags & PushFlags::TEXTLINECOLOR )
    {
        if ( rState.mpTextLineColor )
            SetTextLineColor( *rState.mpTextLineColor );
        else
            SetTextLineColor();
    }

    if ( rState.mnFlags & PushFlags::OVERLINECOLOR )
    {
        if ( rState.mpOverlineColor )
            SetOverlineColor( *rState.mpOverlineColor );
        else
            SetOverlineColor();
    }

    if ( rState.mnFlags & PushFlags::TEXTALIGN )
        SetTextAlign( rState.meTextAlign );

    if( rState.mnFlags & PushFlags::TEXTLAYOUTMODE )
        SetLayoutMode( rState.mnTextLayoutMode );

    if( rState.mnFlags & PushFlags::TEXTLANGUAGE )
        SetDigitLanguage( rState.meTextLanguage );

    if ( rState.mnFlags & PushFlags::RASTEROP )
        SetRasterOp( rState.meRasterOp );

    if ( rState.mnFlags & PushFlags::MAPMODE )
    {
        if ( rState.mpMapMode )
            SetMapMode( *rState.mpMapMode );
        else
            SetMapMode();
        mbMap = rState.mbMapActive;
    }

    if ( rState.mnFlags & PushFlags::CLIPREGION )
        SetDeviceClipRegion( rState.mpClipRegion.get() );

    if ( rState.mnFlags & PushFlags::REFPOINT )
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

void OutputDevice::SetAntialiasing( AntialiasingFlags nMode )
{
    if ( mnAntialiasing != nMode )
    {
        mnAntialiasing = nMode;
        mbInitFont = true;

        if(mpGraphics)
        {
            mpGraphics->setAntiAliasB2DDraw(bool(mnAntialiasing & AntialiasingFlags::EnableB2dDraw));
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetAntialiasing( nMode );
}

void OutputDevice::SetDrawMode( DrawModeFlags nDrawMode )
{

    mnDrawMode = nDrawMode;

    if( mpAlphaVDev )
        mpAlphaVDev->SetDrawMode( nDrawMode );
}

void OutputDevice::SetLayoutMode( ComplexTextLayoutFlags nTextLayoutMode )
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
            mpGraphics->SetXORMode( (RasterOp::Invert == meRasterOp) || (RasterOp::Xor == meRasterOp), RasterOp::Invert == meRasterOp );
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
        maFillColor = COL_TRANSPARENT;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetFillColor();
}

void OutputDevice::SetFillColor( const Color& rColor )
{

    Color aColor( rColor );

    if( mnDrawMode & ( DrawModeFlags::BlackFill | DrawModeFlags::WhiteFill |
                       DrawModeFlags::GrayFill | DrawModeFlags::NoFill |
                       DrawModeFlags::SettingsFill ) )
    {
        if( !ImplIsColorTransparent( aColor ) )
        {
            if( mnDrawMode & DrawModeFlags::BlackFill )
            {
                aColor = COL_BLACK;
            }
            else if( mnDrawMode & DrawModeFlags::WhiteFill )
            {
                aColor = COL_WHITE;
            }
            else if( mnDrawMode & DrawModeFlags::GrayFill )
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }
            else if( mnDrawMode & DrawModeFlags::NoFill )
            {
                aColor = COL_TRANSPARENT;
            }
            else if( mnDrawMode & DrawModeFlags::SettingsFill )
            {
                aColor = GetSettings().GetStyleSettings().GetWindowColor();
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
            maFillColor = COL_TRANSPARENT;
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
        maLineColor = COL_TRANSPARENT;
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
            maLineColor = COL_TRANSPARENT;
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

    if( rBackground.GetStyle() == WallpaperStyle::NONE )
        mbBackground = false;
    else
        mbBackground = true;

    if( mpAlphaVDev )
        mpAlphaVDev->SetBackground( rBackground );
}

void OutputDevice::SetFont( const vcl::Font& rNewFont )
{

    vcl::Font aFont( rNewFont );
    if ( mnDrawMode & (DrawModeFlags::BlackText | DrawModeFlags::WhiteText | DrawModeFlags::GrayText | DrawModeFlags::SettingsText |
                       DrawModeFlags::BlackFill | DrawModeFlags::WhiteFill | DrawModeFlags::GrayFill | DrawModeFlags::NoFill |
                       DrawModeFlags::SettingsFill ) )
    {
        Color aTextColor( aFont.GetColor() );

        if ( mnDrawMode & DrawModeFlags::BlackText )
            aTextColor = COL_BLACK;
        else if ( mnDrawMode & DrawModeFlags::WhiteText )
            aTextColor = COL_WHITE;
        else if ( mnDrawMode & DrawModeFlags::GrayText )
        {
            const sal_uInt8 cLum = aTextColor.GetLuminance();
            aTextColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DrawModeFlags::SettingsText )
            aTextColor = GetSettings().GetStyleSettings().GetFontColor();

        aFont.SetColor( aTextColor );

        bool bTransFill = aFont.IsTransparent();
        if ( !bTransFill )
        {
            Color aTextFillColor( aFont.GetFillColor() );

            if ( mnDrawMode & DrawModeFlags::BlackFill )
                aTextFillColor = COL_BLACK;
            else if ( mnDrawMode & DrawModeFlags::WhiteFill )
                aTextFillColor = COL_WHITE;
            else if ( mnDrawMode & DrawModeFlags::GrayFill )
            {
                const sal_uInt8 cLum = aTextFillColor.GetLuminance();
                aTextFillColor = Color( cLum, cLum, cLum );
            }
            else if( mnDrawMode & DrawModeFlags::SettingsFill )
                aTextFillColor = GetSettings().GetStyleSettings().GetWindowColor();
            else if ( mnDrawMode & DrawModeFlags::NoFill )
            {
                aTextFillColor = COL_TRANSPARENT;
                bTransFill = true;
            }

            aFont.SetFillColor( aTextFillColor );
        }
    }

    if ( mpMetaFile )
    {
        mpMetaFile->AddAction( new MetaFontAction( aFont ) );
        // the color and alignment actions don't belong here
        // TODO: get rid of them without breaking anything...
        mpMetaFile->AddAction( new MetaTextAlignAction( aFont.GetAlignment() ) );
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
        if( RasterOp::N0 == meRasterOp )
            mpGraphics->SetROPLineColor( SalROPColor::N0 );
        else if( RasterOp::N1 == meRasterOp )
            mpGraphics->SetROPLineColor( SalROPColor::N1 );
        else if( RasterOp::Invert == meRasterOp )
            mpGraphics->SetROPLineColor( SalROPColor::Invert );
        else
            mpGraphics->SetLineColor( maLineColor );
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
        if( RasterOp::N0 == meRasterOp )
            mpGraphics->SetROPFillColor( SalROPColor::N0 );
        else if( RasterOp::N1 == meRasterOp )
            mpGraphics->SetROPFillColor( SalROPColor::N1 );
        else if( RasterOp::Invert == meRasterOp )
            mpGraphics->SetROPFillColor( SalROPColor::Invert );
        else
            mpGraphics->SetFillColor( maFillColor );
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

    mpFontInstance.clear();
    mpDeviceFontList.reset();
    mpDeviceFontSizeList.reset();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
