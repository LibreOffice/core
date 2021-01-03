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

#include <drawmode.hxx>
#include <font/ImplDeviceFontList.hxx>
#include <font/ImplDeviceFontSizeList.hxx>
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

OutDevState::OutDevState(OutDevState&&) = default;

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

    maOutDevStateStack.emplace_back();
    OutDevState& rState = maOutDevStateStack.back();

    rState.mnFlags = nFlags;

    if (nFlags & PushFlags::LINECOLOR && IsOpaqueLineColor())
        rState.mpLineColor = GetLineColor();

    if (nFlags & PushFlags::FILLCOLOR && IsOpaqueFillColor())
        rState.mpFillColor = GetFillColor();

    if ( nFlags & PushFlags::FONT )
        rState.mpFont.reset( new vcl::Font( maFont ) );

    if ( nFlags & PushFlags::TEXTCOLOR )
        rState.mpTextColor = GetTextColor();

    if (nFlags & PushFlags::TEXTFILLCOLOR && IsOpaqueTextFillColor())
        rState.mpTextFillColor = GetTextFillColor();

    if (nFlags & PushFlags::TEXTLINECOLOR && IsOpaqueTextLineColor())
        rState.mpTextLineColor = GetTextLineColor();

    if (nFlags & PushFlags::OVERLINECOLOR && IsOpaqueOverlineColor())
        rState.mpOverlineColor = GetOverlineColor();

    if ( nFlags & PushFlags::TEXTALIGN )
        rState.meTextAlign = GetTextAlign();

    if( nFlags & PushFlags::TEXTLAYOUTMODE )
        rState.mnTextLayoutMode = GetLayoutMode();

    if( nFlags & PushFlags::TEXTLANGUAGE )
        rState.meTextLanguage = GetDigitLanguage();

    if ( nFlags & PushFlags::RASTEROP )
        rState.meRasterOp = GetRasterOp();

    if ( nFlags & PushFlags::MAPMODE )
    {
        rState.mpMapMode = maMapMode;
        rState.mbMapActive = mbMap;
    }

    if (nFlags & PushFlags::CLIPREGION && mbClipRegion)
        rState.mpClipRegion.reset( new vcl::Region( maRegion ) );

    if (nFlags & PushFlags::REFPOINT && mbRefPoint)
        rState.mpRefPoint = maRefPoint;

    if( mpAlphaVDev )
        mpAlphaVDev->Push();
}

void OutputDevice::Pop()
{

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPopAction() );

    GDIMetaFile* pOldMetaFile = mpMetaFile;
    mpMetaFile = nullptr;

    if ( maOutDevStateStack.empty() )
    {
        SAL_WARN( "vcl.gdi", "OutputDevice::Pop() without OutputDevice::Push()" );
        return;
    }
    const OutDevState& rState = maOutDevStateStack.back();

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

    maOutDevStateStack.pop_back();

    mpMetaFile = pOldMetaFile;
}

void OutputDevice::ClearStack()
{
    // the current stack depth is the number of Push() calls minus the number of Pop() calls
    // this should not normally be used since Push and Pop must always be used symmetrically
    // however this may be e.g. a help when debugging code in which this somehow is not the case

    sal_uInt32 nCount = maOutDevStateStack.size();
    while (nCount--)
        Pop();
}

void OutputDevice::SetDrawMode(DrawModeFlags nDrawMode)
{
    if (mpAlphaVDev)
        mpAlphaVDev->SetDrawMode(nDrawMode);
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

void OutputDevice::SetRasterOp(RasterOp eRasterOp)
{

    if (mpMetaFile)
        mpMetaFile->AddAction(new MetaRasterOpAction(eRasterOp));

    RenderContext2::SetRasterOp(eRasterOp);

    if (mpAlphaVDev)
        mpAlphaVDev->SetRasterOp(eRasterOp);
}


void OutputDevice::SetFillColor(const Color& rColor)
{

    Color aColor(rColor);
    aColor = GetDrawModeFillColor(aColor, GetDrawMode(), GetSettings().GetStyleSettings());

    if (mpMetaFile)
    {
        if (aColor.IsTransparent())
            mpMetaFile->AddAction(new MetaFillColorAction(Color(), false));
        else
            mpMetaFile->AddAction(new MetaFillColorAction(aColor, true));
    }

    RenderContext2::SetFillColor(rColor);

    if (mpAlphaVDev)
    {
        if (aColor.IsTransparent())
            mpAlphaVDev->SetFillColor();
        else
            mpAlphaVDev->SetFillColor(COL_BLACK);
    }
}

void OutputDevice::SetLineColor(Color const& rColor)
{
    Color aColor(rColor);

    if (mpMetaFile)
    {
        if (aColor.IsTransparent())
            mpMetaFile->AddAction(new MetaLineColorAction(Color(), false));
        else
            mpMetaFile->AddAction(new MetaLineColorAction(aColor, true));
    }

    RenderContext2::SetLineColor(rColor);

    if (mpAlphaVDev)
    {
        if (aColor.IsTransparent())
            mpAlphaVDev->SetLineColor();
        else
            mpAlphaVDev->SetLineColor(COL_BLACK);
    }
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
    {
        // Some of these are probably wrong (e.g. if the gradient has transparency),
        // but hopefully nobody uses that. If you do, feel free to implement it properly.
        if( rBackground.GetStyle() == WallpaperStyle::NONE )
            mpAlphaVDev->SetBackground( rBackground );
        else if( rBackground.IsBitmap())
        {
            BitmapEx bitmap = rBackground.GetBitmap();
            if( bitmap.IsAlpha())
                mpAlphaVDev->SetBackground( Wallpaper( BitmapEx( Bitmap( bitmap.GetAlpha()))));
            else
            {
                switch( bitmap.GetTransparentType())
                {
                    case TransparentType::NONE:
                        mpAlphaVDev->SetBackground( Wallpaper( COL_BLACK ));
                        break;
                    case TransparentType::Color:
                    {
                        AlphaMask mask( bitmap.GetBitmap().CreateMask( bitmap.GetTransparentColor()));
                        mpAlphaVDev->SetBackground( Wallpaper( BitmapEx( bitmap.GetBitmap(), mask )));
                        break;
                    }
                    case TransparentType::Bitmap:
                        mpAlphaVDev->SetBackground( Wallpaper( BitmapEx( bitmap.GetMask())));
                        break;
                }
            }
        }
        else if( rBackground.IsGradient())
            mpAlphaVDev->SetBackground( Wallpaper( COL_BLACK ));
        else
        {
            // Color background.
            int transparency = rBackground.GetColor().GetTransparency();
            mpAlphaVDev->SetBackground( Wallpaper( Color( transparency, transparency, transparency )));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
