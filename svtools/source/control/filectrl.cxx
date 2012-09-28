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

#include <svtools/svtresid.hxx>
#include <svtools/filectrl.hxx>
#include <filectrl.hrc>

// =======================================================================

FileControl::FileControl( Window* pParent, WinBits nStyle, FileControlMode nFlags ) :
    Window( pParent, nStyle|WB_DIALOGCONTROL ),
    maEdit( this, (nStyle&(~WB_BORDER))|WB_NOTABSTOP ),
    maButton( this, (nStyle&(~WB_BORDER))|WB_NOLIGHTBORDER|WB_NOPOINTERFOCUS|WB_NOTABSTOP ),
    maButtonText( SVT_RESSTR(STR_FILECTRL_BUTTONTEXT) ),
    mnFlags( nFlags ),
    mnInternalFlags( FILECTRL_ORIGINALBUTTONTEXT )
{
    maButton.SetClickHdl( LINK( this, FileControl, ButtonHdl ) );
    mbOpenDlg = sal_True;

    maButton.Show();
    maEdit.Show();

    SetCompoundControl( sal_True );

    SetStyle( ImplInitStyle( GetStyle() ) );
}

// -----------------------------------------------------------------------

WinBits FileControl::ImplInitStyle( WinBits nStyle )
{
    if ( !( nStyle & WB_NOTABSTOP ) )
    {
        maEdit.SetStyle( (maEdit.GetStyle()|WB_TABSTOP)&(~WB_NOTABSTOP) );
        maButton.SetStyle( (maButton.GetStyle()|WB_TABSTOP)&(~WB_NOTABSTOP) );
    }
    else
    {
        maEdit.SetStyle( (maEdit.GetStyle()|WB_NOTABSTOP)&(~WB_TABSTOP) );
        maButton.SetStyle( (maButton.GetStyle()|WB_NOTABSTOP)&(~WB_TABSTOP) );
    }

    const WinBits nAlignmentStyle = ( WB_TOP | WB_VCENTER | WB_BOTTOM );
    maEdit.SetStyle( ( maEdit.GetStyle() & ~nAlignmentStyle ) | ( nStyle & nAlignmentStyle ) );

    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;

    if ( !(nStyle & WB_NOBORDER ) )
        nStyle |= WB_BORDER;

    nStyle &= ~WB_TABSTOP;

    return nStyle;
}

// -----------------------------------------------------------------------

FileControl::~FileControl()
{
}

// -----------------------------------------------------------------------

void FileControl::SetText( const XubString& rStr )
{
    maEdit.SetText( rStr );
    if ( mnFlags & FILECTRL_RESIZEBUTTONBYPATHLEN )
        Resize();
}

// -----------------------------------------------------------------------

XubString FileControl::GetText() const
{
    return maEdit.GetText();
}

// -----------------------------------------------------------------------

void FileControl::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_ENABLE )
    {
        maEdit.Enable( IsEnabled() );
        maButton.Enable( IsEnabled() );
    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        GetEdit().SetZoom( GetZoom() );
        GetButton().SetZoom( GetZoom() );
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        SetStyle( ImplInitStyle( GetStyle() ) );
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        GetEdit().SetControlFont( GetControlFont() );
        // Fuer den Button nur die Hoehe uebernehmen, weil in
        // HTML immer Courier eingestellt wird.
        Font aFont = GetButton().GetControlFont();
        aFont.SetSize( GetControlFont().GetSize() );
        GetButton().SetControlFont( aFont );
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        GetEdit().SetControlForeground( GetControlForeground() );
        GetButton().SetControlForeground( GetControlForeground() );
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        GetEdit().SetControlBackground( GetControlBackground() );
        GetButton().SetControlBackground( GetControlBackground() );
    }
    Window::StateChanged( nType );
}

// -----------------------------------------------------------------------

void FileControl::Resize()
{
    static long ButtonBorder = 10;

    if( mnInternalFlags & FILECTRL_INRESIZE )
        return;
    mnInternalFlags |= FILECTRL_INRESIZE;//InResize = sal_True

    Size aOutSz = GetOutputSizePixel();
    long nButtonTextWidth = maButton.GetTextWidth( maButtonText );
    if ( ((mnInternalFlags & FILECTRL_ORIGINALBUTTONTEXT) == 0) ||
        ( nButtonTextWidth < aOutSz.Width()/3 &&
        ( mnFlags & FILECTRL_RESIZEBUTTONBYPATHLEN
        ? ( maEdit.GetTextWidth( maEdit.GetText() )
            <= aOutSz.Width() - nButtonTextWidth - ButtonBorder )
        : sal_True ) )
       )
    {
        maButton.SetText( maButtonText );
    }
    else
    {
        rtl::OUString aSmallText( "..." );
        maButton.SetText( aSmallText );
        nButtonTextWidth = maButton.GetTextWidth( aSmallText );
    }

    long nButtonWidth = nButtonTextWidth+ButtonBorder;
    maEdit.setPosSizePixel( 0, 0, aOutSz.Width()-nButtonWidth, aOutSz.Height() );
    maButton.setPosSizePixel( aOutSz.Width()-nButtonWidth, 0, nButtonWidth, aOutSz.Height() );

    mnInternalFlags &= ~FILECTRL_INRESIZE; //InResize = sal_False
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(FileControl, ButtonHdl)
{
    ImplBrowseFile( );

    return 0;
}

// -----------------------------------------------------------------------

void FileControl::GetFocus()
{
    maEdit.GrabFocus();
}

// -----------------------------------------------------------------------

void FileControl::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    WinBits nOldEditStyle = GetEdit().GetStyle();
    if ( GetStyle() & WB_BORDER )
        GetEdit().SetStyle( nOldEditStyle|WB_BORDER );
    GetEdit().Draw( pDev, rPos, rSize, nFlags );
    if ( GetStyle() & WB_BORDER )
        GetEdit().SetStyle( nOldEditStyle );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
