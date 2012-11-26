/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "svx/fontlb.hxx"
#include <vcl/svapp.hxx>

// ============================================================================

SvLBoxFontString::SvLBoxFontString() :
    SvLBoxString()
{
}

SvLBoxFontString::SvLBoxFontString(
        SvLBoxEntry* pEntry, sal_uInt16 nFlags, const XubString& rString,
        const Font& rFont, const Color* pColor ) :
    SvLBoxString( pEntry, nFlags, rString ),
    maFont( rFont ),
    mbUseColor( pColor != NULL )
{
    SetText( pEntry, rString );
    if( pColor )
        maFont.SetColor( *pColor );
}

SvLBoxFontString::~SvLBoxFontString()
{
}


SvLBoxItem* SvLBoxFontString::Create() const
{
    return new SvLBoxFontString;
}

void SvLBoxFontString::Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry )
{
    Font aOldFont( rDev.GetFont() );
    Font aNewFont( maFont );
    bool bSel = (nFlags & SVLISTENTRYFLAG_SELECTED) != 0;
//  if( !mbUseColor )               // selection gets font color, if available
    if( !mbUseColor || bSel )       // selection always gets highlight color
    {
        const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();
        aNewFont.SetColor( bSel ? rSett.GetHighlightTextColor() : rSett.GetFieldTextColor() );
    }

    rDev.SetFont( aNewFont );
    SvLBoxString::Paint( rPos, rDev, nFlags, pEntry );
    rDev.SetFont( aOldFont );
}

void SvLBoxFontString::InitViewData( SvLBox* pView, SvLBoxEntry* pEntry, SvViewDataItem* pViewData )
{
    Font aOldFont( pView->GetFont() );
    pView->SetFont( maFont );
    SvLBoxString::InitViewData( pView, pEntry, pViewData);
    pView->SetFont( aOldFont );
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

#if ENABLE_LAYOUT

namespace layout
{

SvxFontListBox::~SvxFontListBox ()
{
}

sal_uInt16 SvxFontListBox::InsertFontEntry (String const& entry, Font const&, Color const*)
{
    return InsertEntry (entry);
}

SvxFontListBox::SvxFontListBox( Context* pParent, const char* pFile)
: ListBox( pParent, pFile )
{
}

/*IMPL_IMPL (SvxFontListBox, ListBox);
IMPL_CONSTRUCTORS (SvxFontListBox, ListBox, "svxfontlistbox");
IMPL_GET_IMPL (SvxFontListBox);
IMPL_GET_WINDOW (SvxFontListBox);*/

};

#endif

// ============================================================================

