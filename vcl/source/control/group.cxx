/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <tools/rc.h>

#include <vcl/event.hxx>
#include <vcl/group.hxx>

#include <controldata.hxx>

// =======================================================================

#define GROUP_BORDER            12
#define GROUP_TEXT_BORDER       2

#define GROUP_VIEW_STYLE        (WB_3DLOOK | WB_NOLABEL)

// =======================================================================

void GroupBox::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );
    SetMouseTransparent( sal_True );
    ImplInitSettings( sal_True, sal_True, sal_True );
}

// -----------------------------------------------------------------------

WinBits GroupBox::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------

const Font& GroupBox::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetGroupFont();
}

// -----------------------------------------------------------------
const Color& GroupBox::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetGroupTextColor();
}

// -----------------------------------------------------------------------

void GroupBox::ImplInitSettings( sal_Bool bFont,
                                 sal_Bool bForeground, sal_Bool bBackground )
{
    Control::ImplInitSettings( bFont, bForeground );

    if ( bBackground )
    {
        Window* pParent = GetParent();
        if ( (pParent->IsChildTransparentModeEnabled() ||
              !(pParent->GetStyle() & WB_CLIPCHILDREN) ) &&
             !IsControlBackground() )
        {
            EnableChildTransparentMode( sal_True );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( sal_True );
            SetBackground();
        }
        else
        {
            EnableChildTransparentMode( sal_False );
            SetParentClipMode( 0 );
            SetPaintTransparent( sal_False );

            if ( IsControlBackground() )
                SetBackground( GetControlBackground() );
            else
                SetBackground( pParent->GetBackground() );
        }
    }
}

// -----------------------------------------------------------------------

GroupBox::GroupBox( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_GROUPBOX )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

GroupBox::GroupBox( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_GROUPBOX )
{
    rResId.SetRT( RSC_GROUPBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void GroupBox::ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                         const Point& rPos, const Size& rSize, bool bLayout )
{
    long                    nTop;
    long                    nTextOff;
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    XubString               aText( GetText() );
    Rectangle               aRect( rPos, rSize );
    sal_uInt16                  nTextStyle = TEXT_DRAW_LEFT | TEXT_DRAW_TOP | TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_MNEMONIC;

    if ( GetStyle() & WB_NOLABEL )
        nTextStyle &= ~TEXT_DRAW_MNEMONIC;
    if ( nDrawFlags & WINDOW_DRAW_NOMNEMONIC )
    {
        if ( nTextStyle & TEXT_DRAW_MNEMONIC )
        {
            aText = GetNonMnemonicString( aText );
            nTextStyle &= ~TEXT_DRAW_MNEMONIC;
        }
    }
    if ( !(nDrawFlags & WINDOW_DRAW_NODISABLE) )
    {
        if ( !IsEnabled() )
            nTextStyle |= TEXT_DRAW_DISABLE;
    }
    if ( (nDrawFlags & WINDOW_DRAW_MONO) ||
         (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
    {
        nTextStyle |= TEXT_DRAW_MONO;
        nDrawFlags |= WINDOW_DRAW_MONO;
    }

    if ( !aText.Len() )
    {
        nTop = rPos.Y();
        nTextOff = 0;
    }
    else
    {
        aRect.Left() += GROUP_BORDER;
        aRect.Right() -= GROUP_BORDER;
        aRect = pDev->GetTextRect( aRect, aText, nTextStyle );
        nTop = rPos.Y();
        nTop += aRect.GetHeight() / 2;
        nTextOff = GROUP_TEXT_BORDER;
    }

    if( ! bLayout )
    {
        if ( nDrawFlags & WINDOW_DRAW_MONO )
            pDev->SetLineColor( Color( COL_BLACK ) );
        else
            pDev->SetLineColor( rStyleSettings.GetShadowColor() );

        if ( !aText.Len() )
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
        // if we're drawing onto a printer, spare the 3D effect
        // #i46986# / 2005-04-13 / frank.schoenheit@sun.com

        if ( !bIsPrinter && !(nDrawFlags & WINDOW_DRAW_MONO) )
        {
            pDev->SetLineColor( rStyleSettings.GetLightColor() );
            if ( !aText.Len() )
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

    MetricVector* pVector = bLayout ? &mpControlData->mpLayoutData->m_aUnicodeBoundRects : NULL;
    String* pDisplayText = bLayout ? &mpControlData->mpLayoutData->m_aDisplayText : NULL;
    DrawControlText( *pDev, aRect, aText, nTextStyle, pVector, pDisplayText );
}

// -----------------------------------------------------------------------

void GroupBox::FillLayoutData() const
{
    mpControlData->mpLayoutData = new vcl::ControlLayoutData();
    const_cast<GroupBox*>(this)->   ImplDraw( const_cast<GroupBox*>(this), 0, Point(), GetOutputSizePixel(), true );
}

// -----------------------------------------------------------------------

void GroupBox::Paint( const Rectangle& )
{
    ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void GroupBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                     sal_uLong nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );
    Font        aFont = GetDrawPixelFont( pDev );

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );
    if ( nFlags & WINDOW_DRAW_MONO )
        pDev->SetTextColor( Color( COL_BLACK ) );
    else
        pDev->SetTextColor( GetTextColor() );
    pDev->SetTextFillColor();

    ImplDraw( pDev, nFlags, aPos, aSize );
    pDev->Pop();
}

// -----------------------------------------------------------------------

void GroupBox::Resize()
{
    Control::Resize();
    Invalidate();
}

// -----------------------------------------------------------------------

void GroupBox::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == STATE_CHANGE_ENABLE) ||
         (nType == STATE_CHANGE_TEXT) ||
         (nType == STATE_CHANGE_UPDATEMODE) )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
        if ( (GetPrevStyle() & GROUP_VIEW_STYLE) !=
             (GetStyle() & GROUP_VIEW_STYLE) )
            Invalidate();
    }
    else if ( (nType == STATE_CHANGE_ZOOM)  ||
              (nType == STATE_CHANGE_CONTROLFONT) )
    {
        ImplInitSettings( sal_True, sal_False, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( sal_False, sal_True, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( sal_False, sal_False, sal_True );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void GroupBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
