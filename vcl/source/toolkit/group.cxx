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

#include <vcl/event.hxx>
#include <vcl/toolkit/group.hxx>
#include <vcl/settings.hxx>

#include <controldata.hxx>

#define GROUP_BORDER            12
#define GROUP_TEXT_BORDER       2

#define GROUP_VIEW_STYLE        (WB_3DLOOK | WB_NOLABEL)

void GroupBox::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, nullptr );
    SetMouseTransparent( true );
    ImplInitSettings( true );
}

WinBits GroupBox::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

const vcl::Font& GroupBox::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetGroupFont();
}

const Color& GroupBox::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetGroupTextColor();
}

void GroupBox::ImplInitSettings( bool bBackground )
{
    Control::ImplInitSettings();

    if ( !bBackground )
        return;

    vcl::Window* pParent = GetParent();
    if ( (pParent->IsChildTransparentModeEnabled() ||
          !(pParent->GetStyle() & WB_CLIPCHILDREN) ) &&
         !IsControlBackground() )
    {
        EnableChildTransparentMode();
        SetParentClipMode( ParentClipMode::NoClip );
        SetPaintTransparent( true );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( false );
        SetParentClipMode();
        SetPaintTransparent( false );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

GroupBox::GroupBox( vcl::Window* pParent, WinBits nStyle ) :
    Control( WindowType::GROUPBOX )
{
    ImplInit( pParent, nStyle );
}

void GroupBox::ImplDraw( OutputDevice* pDev, DrawFlags nDrawFlags,
                         const Point& rPos, const Size& rSize, bool bLayout )
{
    tools::Long                    nTop;
    tools::Long                    nTextOff;
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    OUString                aText( GetText() );
    tools::Rectangle               aRect( rPos, rSize );
    DrawTextFlags           nTextStyle = DrawTextFlags::Left | DrawTextFlags::Top | DrawTextFlags::EndEllipsis | DrawTextFlags::Mnemonic;

    if ( GetStyle() & WB_NOLABEL )
        nTextStyle &= ~DrawTextFlags::Mnemonic;
    if ( !IsEnabled() )
        nTextStyle |= DrawTextFlags::Disable;
    if ( (nDrawFlags & DrawFlags::Mono) ||
         (rStyleSettings.GetOptions() & StyleSettingsOptions::Mono) )
    {
        nTextStyle |= DrawTextFlags::Mono;
        nDrawFlags |= DrawFlags::Mono;
    }

    if (aText.isEmpty())
    {
        nTop = rPos.Y();
        nTextOff = 0;
    }
    else
    {
        aRect.AdjustLeft(GROUP_BORDER );
        aRect.AdjustRight( -(GROUP_BORDER) );
        aRect = pDev->GetTextRect( aRect, aText, nTextStyle );
        nTop = rPos.Y();
        nTop += aRect.GetHeight() / 2;
        nTextOff = GROUP_TEXT_BORDER;
    }

    if( ! bLayout )
    {
        if ( nDrawFlags & DrawFlags::Mono )
            pDev->SetLineColor( COL_BLACK );
        else
            pDev->SetLineColor( rStyleSettings.GetShadowColor() );

        if (aText.isEmpty())
            pDev->DrawLine( Point( rPos.X(), nTop ), Point( rPos.X()+rSize.Width()-2, nTop ) );
        else
        {
            pDev->DrawLine( Point( rPos.X(), nTop ), Point( aRect.Left()-nTextOff, nTop ) );
            pDev->DrawLine( Point( aRect.Right()+nTextOff, nTop ), Point( rPos.X()+rSize.Width()-2, nTop ) );
        }
        pDev->DrawLine( Point( rPos.X(), nTop ), Point( rPos.X(), rPos.Y()+rSize.Height()-2 ) );
        pDev->DrawLine( Point( rPos.X(), rPos.Y()+rSize.Height()-2 ), Point( rPos.X()+rSize.Width()-2, rPos.Y()+rSize.Height()-2 ) );
        pDev->DrawLine( Point( rPos.X()+rSize.Width()-2, rPos.Y()+rSize.Height()-2 ), Point( rPos.X()+rSize.Width()-2, nTop ) );

        bool bIsPrinter = OUTDEV_PRINTER == pDev->GetOutDevType();
        // if we're drawing onto a printer, spare the 3D effect #i46986#

        if ( !bIsPrinter && !(nDrawFlags & DrawFlags::Mono) )
        {
            pDev->SetLineColor( rStyleSettings.GetLightColor() );
            if (aText.isEmpty())
                pDev->DrawLine( Point( rPos.X()+1, nTop+1 ), Point( rPos.X()+rSize.Width()-3, nTop+1 ) );
            else
            {
                pDev->DrawLine( Point( rPos.X()+1, nTop+1 ), Point( aRect.Left()-nTextOff, nTop+1 ) );
                pDev->DrawLine( Point( aRect.Right()+nTextOff, nTop+1 ), Point( rPos.X()+rSize.Width()-3, nTop+1 ) );
            }
            pDev->DrawLine( Point( rPos.X()+1, nTop+1 ), Point( rPos.X()+1, rPos.Y()+rSize.Height()-3 ) );
            pDev->DrawLine( Point( rPos.X(), rPos.Y()+rSize.Height()-1 ), Point( rPos.X()+rSize.Width()-1, rPos.Y()+rSize.Height()-1 ) );
            pDev->DrawLine( Point( rPos.X()+rSize.Width()-1, rPos.Y()+rSize.Height()-1 ), Point( rPos.X()+rSize.Width()-1, nTop ) );
        }
    }

    std::vector< tools::Rectangle >* pVector = bLayout ? &mpControlData->mpLayoutData->m_aUnicodeBoundRects : nullptr;
    OUString* pDisplayText = bLayout ? &mpControlData->mpLayoutData->m_aDisplayText : nullptr;
    DrawControlText( *pDev, aRect, aText, nTextStyle, pVector, pDisplayText );
}

void GroupBox::FillLayoutData() const
{
    mpControlData->mpLayoutData.reset( new vcl::ControlLayoutData );
    const_cast<GroupBox*>(this)->ImplDraw( const_cast<GroupBox*>(this), DrawFlags::NONE, Point(), GetOutputSizePixel(), true );
}

void GroupBox::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    ImplDraw(&rRenderContext, DrawFlags::NONE, Point(), GetOutputSizePixel());
}

void GroupBox::Draw( OutputDevice* pDev, const Point& rPos,
                     DrawFlags nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = GetSizePixel();
    vcl::Font   aFont = GetDrawPixelFont( pDev );

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    if ( nFlags & DrawFlags::Mono )
        pDev->SetTextColor( COL_BLACK );
    else
        pDev->SetTextColor( GetTextColor() );
    pDev->SetTextFillColor();

    ImplDraw( pDev, nFlags, aPos, aSize );
    pDev->Pop();
}

void GroupBox::Resize()
{
    Control::Resize();
    Invalidate();
}

void GroupBox::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == StateChangedType::Enable) ||
         (nType == StateChangedType::Text) ||
         (nType == StateChangedType::UpdateMode) )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::Style )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & GROUP_VIEW_STYLE) !=
             (GetStyle() & GROUP_VIEW_STYLE) )
            Invalidate();
    }
    else if ( (nType == StateChangedType::Zoom)  ||
              (nType == StateChangedType::ControlFont) )
    {
        ImplInitSettings( false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlForeground )
    {
        ImplInitSettings( false );
        Invalidate();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings( true );
        Invalidate();
    }
}

void GroupBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings( true );
        Invalidate();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
