/*************************************************************************
 *
 *  $RCSfile: fontlb.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dr $ $Date: 2002-07-23 10:50:17 $
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

#ifndef SVX_FONTLB_HXX
#include "fontlb.hxx"
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#pragma hdrstop

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
    SetText( pEntry, rString );
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

void SvLBoxFontString::Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS( SvLBoxFontString, 0 );
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
    DBG_CHKTHIS( SvLBoxFontString, 0 );
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
    maStdFont.SetTransparent( TRUE );
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
    SelectAll( FALSE, TRUE );
}

sal_uInt32 SvxFontListBox::GetSelectEntryPos() const
{
    SvLBoxEntry* pSvLBoxEntry = FirstSelected();
    return pSvLBoxEntry ? GetModel()->GetAbsPos( pSvLBoxEntry ) : LIST_ENTRY_NOTFOUND;
}

XubString SvxFontListBox::GetSelectEntry() const
{
    return GetEntryText( GetSelectEntryPos() );
}

void SvxFontListBox::InitEntry(
        SvLBoxEntry* pEntry, const XubString& rEntryText,
        const Image& rCollImg, const Image& rExpImg )
{
    if( mbUseFont )
    {
        if( nTreeFlags & TREEFLAG_CHKBTN )
            pEntry->AddItem( new SvLBoxButton( pEntry, 0, pCheckButtonData ) );
        pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, rCollImg, rExpImg, SVLISTENTRYFLAG_EXPANDED ) );
        pEntry->AddItem( new SvLBoxFontString( pEntry, 0, rEntryText, maEntryFont, mpEntryColor ) );
    }
    else
        SvTreeListBox::InitEntry( pEntry, rEntryText, rCollImg, rExpImg );
}


// ============================================================================

