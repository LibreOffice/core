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
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include "svtools/treelistentry.hxx"
#include "svtools/viewdataentry.hxx"

SvLBoxFontString::SvLBoxFontString()
    : SvLBoxString()
    , mbUseColor(false)
{
}

SvLBoxFontString::SvLBoxFontString(
        SvTreeListEntry* pEntry, sal_uInt16 nFlags, const OUString& rString,
        const vcl::Font& rFont, const Color* pColor ) :
    SvLBoxString( pEntry, nFlags, rString ),
    maFont( rFont ),
    mbUseColor( pColor != nullptr )
{
    SetText(rString);
    if(pColor)
        maFont.SetColor(*pColor);
}

SvLBoxFontString::~SvLBoxFontString()
{
}

SvLBoxItem* SvLBoxFontString::Create() const
{
    return new SvLBoxFontString;
}

void SvLBoxFontString::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                             const SvViewDataEntry* pView, const SvTreeListEntry& rEntry)
{
    rRenderContext.Push(PushFlags::FONT);
    vcl::Font aNewFont(maFont);
    bool bSel = pView->IsSelected();
    if (!mbUseColor || bSel)       // selection always gets highlight color
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        aNewFont.SetColor(bSel ? rStyleSettings.GetHighlightTextColor() : rStyleSettings.GetFieldTextColor());
    }

    rRenderContext.SetFont(aNewFont);
    SvLBoxString::Paint(rPos, rDev, rRenderContext, pView, rEntry);
    rRenderContext.Pop();
}

void SvLBoxFontString::InitViewData( SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData )
{
    vcl::Font aOldFont( pView->GetFont() );
    pView->Control::SetFont( maFont );
    SvLBoxString::InitViewData( pView, pEntry, pViewData);
    pView->Control::SetFont( aOldFont );
}




SvxFontListBox::SvxFontListBox(vcl::Window* pParent, WinBits nStyle)
    : SvTabListBox(pParent, nStyle)
    , maStdFont(GetFont())
    , mpEntryColor(nullptr)
    , mbUseFont(false)
{
    maStdFont.SetTransparent(true);
    maEntryFont = maStdFont;
}

VCL_BUILDER_DECL_FACTORY(SvxFontListBox)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<SvxFontListBox>::Create(pParent, nWinStyle);
}

void SvxFontListBox::InsertFontEntry( const OUString& rString, const vcl::Font& rFont, const Color* pColor )
{
    mbUseFont = true;           // InitEntry() will use maEntryFont
    maEntryFont = rFont;        // font to use in InitEntry() over InsertEntry()
    mpEntryColor = pColor;      // color to use in InitEntry() over InsertEntry()
    InsertEntry( rString );
    mbUseFont = false;
}

void SvxFontListBox::SelectEntryPos( sal_uLong nPos, bool bSelect )
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
    SelectAll( false );
}

sal_uLong SvxFontListBox::GetSelectEntryPos() const
{
    SvTreeListEntry* pSvLBoxEntry = FirstSelected();
    return pSvLBoxEntry ? GetModel()->GetAbsPos( pSvLBoxEntry ) : TREELIST_ENTRY_NOTFOUND;
}

void SvxFontListBox::InitEntry(
        SvTreeListEntry* pEntry, const OUString& rEntryText,
        const Image& rCollImg, const Image& rExpImg,
        SvLBoxButtonKind eButtonKind)
{
    if( mbUseFont )
    {
        if( nTreeFlags & SvTreeFlags::CHKBTN )
            pEntry->AddItem(std::unique_ptr<SvLBoxButton>(new SvLBoxButton(
                        pEntry, eButtonKind, 0, pCheckButtonData)));
        pEntry->AddItem(std::unique_ptr<SvLBoxContextBmp>(new SvLBoxContextBmp(
                        pEntry, 0, rCollImg, rExpImg, true)));
        pEntry->AddItem(std::unique_ptr<SvLBoxFontString>(new SvLBoxFontString(
                        pEntry, 0, rEntryText, maEntryFont, mpEntryColor)));
    }
    else
        SvTreeListBox::InitEntry( pEntry, rEntryText, rCollImg, rExpImg,
                                  eButtonKind );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
