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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

#include "optHeaderTabListbox.hxx"
#include <vcl/svapp.hxx>
#include <svtools/headbar.hxx>


namespace svx
{
// class OptLBoxString_Impl ----------------------------------------------

class OptLBoxString_Impl : public SvLBoxString
{
public:
    OptLBoxString_Impl( SvLBoxEntry* pEntry, sal_uInt16 nFlags, const String& rTxt ) :
        SvLBoxString( pEntry, nFlags, rTxt ) {}

    virtual void Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry );
};

// -----------------------------------------------------------------------

void OptLBoxString_Impl::Paint( const Point& rPos, SvLBox& rDev, sal_uInt16, SvLBoxEntry* pEntry )
{
    Font aOldFont( rDev.GetFont() );
    Font aFont( aOldFont );
    //detect readonly state by asking for a valid Image
    if(pEntry && !(!((OptHeaderTabListBox&)rDev).GetCollapsedEntryBmp(pEntry)))
        aFont.SetColor( Application::GetSettings().GetStyleSettings().GetDeactiveTextColor() );
    rDev.SetFont( aFont );
    rDev.DrawText( rPos, GetText() );
    rDev.SetFont( aOldFont );
}
// -----------------------------------------------------------------------------

OptHeaderTabListBox::OptHeaderTabListBox( Window* pParent, WinBits nWinStyle ) :

    SvHeaderTabListBox( pParent, nWinStyle )
{
}

// -----------------------------------------------------------------------
void OptHeaderTabListBox::InitEntry( SvLBoxEntry* pEntry, const XubString& rTxt,
                                     const Image& rImg1, const Image& rImg2,
                                     SvLBoxButtonKind eButtonKind )
{
    SvTabListBox::InitEntry( pEntry, rTxt, rImg1, rImg2, eButtonKind );
    sal_uInt16 _nTabCount = TabCount();

    for ( sal_uInt16 nCol = 1; nCol < _nTabCount; ++nCol )
    {
        // alle Spalten mit eigener Klasse initialisieren (Spalte 0 == Bitmap)
        SvLBoxString* pCol = (SvLBoxString*)pEntry->GetItem( nCol );
        OptLBoxString_Impl* pStr = new OptLBoxString_Impl( pEntry, 0, pCol->GetText() );
        pEntry->ReplaceItem( pStr, nCol );
    }
}

} // namespace svx
