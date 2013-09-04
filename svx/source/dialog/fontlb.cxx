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

#include "svx/fontlb.hxx"
#include <vcl/builder.hxx>
#include <vcl/svapp.hxx>
#include "svtools/treelistentry.hxx"
#include "svtools/viewdataentry.hxx"

// ============================================================================

DBG_NAME( SvLBoxFontString );

SvLBoxFontString::SvLBoxFontString() :
    SvLBoxString()
{
    DBG_CTOR( SvLBoxFontString, 0 );
}

SvLBoxFontString::SvLBoxFontString(
        SvTreeListEntry* pEntry, sal_uInt16 nFlags, const OUString& rString,
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

void SvLBoxFontString::Paint(
    const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* pView, const SvTreeListEntry* pEntry)
{
    DBG_CHKTHIS( SvLBoxFontString, 0 );
    Font aOldFont( rDev.GetFont() );
    Font aNewFont( maFont );
    bool bSel = pView->IsSelected();
    if( !mbUseColor || bSel )       // selection always gets highlight color
    {
        const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();
        aNewFont.SetColor( bSel ? rSett.GetHighlightTextColor() : rSett.GetFieldTextColor() );
    }

    rDev.SetFont( aNewFont );
    SvLBoxString::Paint(rPos, rDev, pView, pEntry);
    rDev.SetFont( aOldFont );
}

void SvLBoxFontString::InitViewData( SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData )
{
    DBG_CHKTHIS( SvLBoxFontString, 0 );
    Font aOldFont( pView->GetFont() );
    pView->Control::SetFont( maFont );
    SvLBoxString::InitViewData( pView, pEntry, pViewData);
    pView->Control::SetFont( aOldFont );
}


// ============================================================================

SvxFontListBox::SvxFontListBox(Window* pParent, WinBits nStyle)
    : SvTabListBox(pParent, nStyle)
    , maStdFont(GetFont())
    , mbUseFont(false)
{
    maStdFont.SetTransparent(sal_True);
    maEntryFont = maStdFont;
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxFontListBox(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    return new SvxFontListBox(pParent, nWinStyle);
}

void SvxFontListBox::InsertFontEntry( const OUString& rString, const Font& rFont, const Color* pColor )
{
    mbUseFont = true;           // InitEntry() will use maEntryFont
    maEntryFont = rFont;        // font to use in InitEntry() over InsertEntry()
    mpEntryColor = pColor;      // color to use in InitEntry() over InsertEntry()
    InsertEntry( rString );
    mbUseFont = false;
}

void SvxFontListBox::SelectEntryPos( sal_uInt16 nPos, bool bSelect )
{
    SvTreeListEntry* pEntry = GetEntry( nPos );
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
    SvTreeListEntry* pSvLBoxEntry = FirstSelected();
    return pSvLBoxEntry ? GetModel()->GetAbsPos( pSvLBoxEntry ) : LIST_APPEND;
}

OUString SvxFontListBox::GetSelectEntry() const
{
    return GetEntryText(GetSelectEntryPos());
}

void SvxFontListBox::InitEntry(
        SvTreeListEntry* pEntry, const OUString& rEntryText,
        const Image& rCollImg, const Image& rExpImg,
        SvLBoxButtonKind eButtonKind)
{
    if( mbUseFont )
    {
        if( nTreeFlags & TREEFLAG_CHKBTN )
            pEntry->AddItem( new SvLBoxButton( pEntry, eButtonKind, 0,
                                               pCheckButtonData ) );
        pEntry->AddItem( new SvLBoxContextBmp(pEntry, 0, rCollImg, rExpImg, true) );
        pEntry->AddItem( new SvLBoxFontString( pEntry, 0, rEntryText, maEntryFont, mpEntryColor ) );
    }
    else
        SvTreeListBox::InitEntry( pEntry, rEntryText, rCollImg, rExpImg,
                                  eButtonKind );
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
