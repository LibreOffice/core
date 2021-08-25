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
#include <drawmode.hxx>
#include <salgdi.hxx>

void OutputDevice::Push(PushFlags nFlags)
{
    if (mpMetaFile)
        mpMetaFile->AddAction(new MetaPushAction(nFlags));

    maOutDevStateStack.emplace_back();
    OutDevState& rState = maOutDevStateStack.back();

    rState.mnFlags = nFlags;

    if (nFlags & PushFlags::LINECOLOR && mbLineColor)
        rState.mpLineColor = maLineColor;

    if (nFlags & PushFlags::FILLCOLOR && mbFillColor)
        rState.mpFillColor = maFillColor;

    if (nFlags & PushFlags::FONT)
        rState.mpFont = maFont;

    if (nFlags & PushFlags::TEXTCOLOR)
        rState.mpTextColor = GetTextColor();

    if (nFlags & PushFlags::TEXTFILLCOLOR && IsTextFillColor())
        rState.mpTextFillColor = GetTextFillColor();

    if (nFlags & PushFlags::TEXTLINECOLOR && IsTextLineColor())
        rState.mpTextLineColor = GetTextLineColor();

    if (nFlags & PushFlags::OVERLINECOLOR && IsOverlineColor())
        rState.mpOverlineColor = GetOverlineColor();

    if (nFlags & PushFlags::TEXTALIGN)
        rState.meTextAlign = GetTextAlign();

    if (nFlags & PushFlags::TEXTLAYOUTMODE)
        rState.mnTextLayoutMode = GetLayoutMode();

    if (nFlags & PushFlags::TEXTLANGUAGE)
        rState.meTextLanguage = GetDigitLanguage();

    if (nFlags & PushFlags::RASTEROP)
        rState.meRasterOp = GetRasterOp();

    if (nFlags & PushFlags::MAPMODE)
    {
        rState.mpMapMode = maMapMode;
        rState.mbMapActive = mbMap;
    }

    if (nFlags & PushFlags::CLIPREGION && mbClipRegion)
        rState.mpClipRegion.reset(new vcl::Region(maRegion));

    if (nFlags & PushFlags::REFPOINT && mbRefPoint)
        rState.mpRefPoint = maRefPoint;

    if (mpAlphaVDev)
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
    sal_uInt32 nCount = maOutDevStateStack.size();
    while( nCount-- )
        Pop();
}

void OutputDevice::SetAntialiasing( AntialiasingFlags nMode )
{
    if ( mnAntialiasing != nMode )
    {
        mnAntialiasing = nMode;
        mbInitFont = true;

        if(mpGraphics)
        {
            mpGraphics->setAntiAlias(bool(mnAntialiasing & AntialiasingFlags::Enable));
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetAntialiasing( nMode );
}

DrawModeFlags OutputDevice::GetDrawMode() const { return mnDrawMode; }

void OutputDevice::SetDrawMode(DrawModeFlags nDrawMode)
{
    mnDrawMode = nDrawMode;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
