/*************************************************************************
 *
 *  $RCSfile: group.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_GROUP_CXX

#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <group.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif

#pragma hdrstop

// =======================================================================

#define GROUP_BORDER            12
#define GROUP_TEXT_BORDER       2

#define GROUP_VIEW_STYLE        (WB_3DLOOK | WB_NOLABEL)

// =======================================================================

void GroupBox::ImplInit( Window* pParent, WinBits nStyle )
{
    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );
    SetMouseTransparent( TRUE );
    ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

WinBits GroupBox::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void GroupBox::ImplInitSettings( BOOL bFont,
                                 BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetGroupFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( bForeground || bFont )
    {
        Color aColor;
        if ( IsControlForeground() )
            aColor = GetControlForeground();
        else
            aColor = rStyleSettings.GetGroupTextColor();
        SetTextColor( aColor );
        SetTextFillColor();
    }

    if ( bBackground )
    {
        Window* pParent = GetParent();
        if ( (pParent->IsChildTransparentModeEnabled() ||
              !(pParent->GetStyle() & WB_CLIPCHILDREN) ) &&
             !IsControlBackground() )
        {
            EnableChildTransparentMode( TRUE );
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            SetPaintTransparent( TRUE );
            SetBackground();
        }
        else
        {
            EnableChildTransparentMode( FALSE );
            SetParentClipMode( 0 );
            SetPaintTransparent( FALSE );

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

void GroupBox::ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
                         const Point& rPos, const Size& rSize )
{
    long                    nTop;
    long                    nTextOff;
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    XubString               aText( GetText() );
    Rectangle               aRect( rPos, rSize );
    USHORT                  nTextStyle = TEXT_DRAW_LEFT | TEXT_DRAW_TOP | TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_MNEMONIC;

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

    if ( !(nDrawFlags & WINDOW_DRAW_MONO) )
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

    pDev->DrawText( aRect, aText, nTextStyle );
}

// -----------------------------------------------------------------------

void GroupBox::Paint( const Rectangle& )
{
    ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void GroupBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                     ULONG nFlags )
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
        ImplInitSettings( TRUE, FALSE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( FALSE, TRUE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( FALSE, FALSE, TRUE );
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
        ImplInitSettings( TRUE, TRUE, TRUE );
        Invalidate();
    }
}

