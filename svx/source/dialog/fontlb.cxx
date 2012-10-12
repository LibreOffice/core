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

#include "svx/fontlb.hxx"
#include <vcl/svapp.hxx>

// ============================================================================

DBG_NAME( SvLBoxFontString );

SvLBoxFontString::SvLBoxFontString() :
    SvLBoxString()
{
    DBG_CTOR( SvLBoxFontString, 0 );
}

SvLBoxFontString::SvLBoxFontString(
        SvLBoxEntry* pEntry, sal_uInt16 nFlags, const XubString& rString,
        const Font& rFont, const Color* pColor ) :
    SvLBoxString( pEntry, nFlags, rString ),
    maFont( rFont ),
    mbUseColor( pColor != NULL )
{
    DBG_CTOR( SvLBoxFontString, 0 );
    SetText( rString );
    if( pColor )
        maFont.SetColor( *pColor );
}

SvLBoxFontString::~SvLBoxFontString()
{
    DBG_DTOR( SvLBoxFontString, 0 );
}


SvLBoxItem* SvLBoxFontString::Create() const
{
    DBG_CHKTHIS( SvLBoxFontString, 0 );
    return new SvLBoxFontString;
}

void SvLBoxFontString::Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS( SvLBoxFontString, 0 );
    Font aOldFont( rDev.GetFont() );
    Font aNewFont( maFont );
    bool bSel = (nFlags & SVLISTENTRYFLAG_SELECTED) != 0;
    if( !mbUseColor || bSel )       // selection always gets highlight color
    {
        const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();
        aNewFont.SetColor( bSel ? rSett.GetHighlightTextColor() : rSett.GetFieldTextColor() );
    }

    rDev.SetFont( aNewFont );
    SvLBoxString::Paint( rPos, rDev, nFlags, pEntry );
    rDev.SetFont( aOldFont );
}

void SvLBoxFontString::InitViewData( SvTreeListBox* pView, SvLBoxEntry* pEntry, SvViewDataItem* pViewData )
{
    DBG_CHKTHIS( SvLBoxFontString, 0 );
    Font aOldFont( pView->GetFont() );
    pView->Control::SetFont( maFont );
    SvLBoxString::InitViewData( pView, pEntry, pViewData);
    pView->Control::SetFont( aOldFont );
}


// ============================================================================

SvxFontListBox::SvxFontListBox( Window* pParent, const ResId& rResId ) :
    SvTabListBox( pParent, rResId ),
    maStdFont( GetFont() ),
    mbUseFont( false )
{
    maStdFont.SetTransparent( sal_True );
    maEntryFont = maStdFont;
}

void SvxFontListBox::InsertFontEntry( const String& rString, const Font& rFont, const Color* pColor )
{
    mbUseFont = true;           // InitEntry() will use maEntryFont
    maEntryFont = rFont;        // font to use in InitEntry() over InsertEntry()
    mpEntryColor = pColor;      // color to use in InitEntry() over InsertEntry()
    InsertEntry( rString );
    mbUseFont = false;
}

void SvxFontListBox::SelectEntryPos( sal_uInt16 nPos, bool bSelect )
{
    SvLBoxEntry* pEntry = GetEntry( nPos );
    if( pEntry )
    {
        Select( pEntry, bSelect );
        ShowEntry( pEntry );
    }
}

void SvxFontListBox::SetNoSelection()
{
    SelectAll( sal_False, sal_True );
}

sal_uLong SvxFontListBox::GetSelectEntryPos() const
{
    SvLBoxEntry* pSvLBoxEntry = FirstSelected();
    return pSvLBoxEntry ? GetModel()->GetAbsPos( pSvLBoxEntry ) : LIST_APPEND;
}

XubString SvxFontListBox::GetSelectEntry() const
{
    return GetEntryText( GetSelectEntryPos() );
}

void SvxFontListBox::InitEntry(
        SvLBoxEntry* pEntry, const XubString& rEntryText,
        const Image& rCollImg, const Image& rExpImg,
        SvLBoxButtonKind eButtonKind )
{
    if( mbUseFont )
    {
        if( nTreeFlags & TREEFLAG_CHKBTN )
            pEntry->AddItem( new SvLBoxButton( pEntry, eButtonKind, 0,
                                               pCheckButtonData ) );
        pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, rCollImg, rExpImg, SVLISTENTRYFLAG_EXPANDED ) );
        pEntry->AddItem( new SvLBoxFontString( pEntry, 0, rEntryText, maEntryFont, mpEntryColor ) );
    }
    else
        SvTreeListBox::InitEntry( pEntry, rEntryText, rCollImg, rExpImg,
                                  eButtonKind );
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
