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
