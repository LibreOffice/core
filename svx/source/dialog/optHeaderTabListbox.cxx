/*************************************************************************
 *
 *  $RCSfile: optHeaderTabListbox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:40:14 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef SVX_OPTHEADERTABLISTBOX_HXX
#include "optHeaderTabListbox.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif


namespace svx
{
// class OptLBoxString_Impl ----------------------------------------------

class OptLBoxString_Impl : public SvLBoxString
{
public:
    OptLBoxString_Impl( SvLBoxEntry* pEntry, USHORT nFlags, const String& rTxt ) :
        SvLBoxString( pEntry, nFlags, rTxt ) {}

    virtual void Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags, SvLBoxEntry* pEntry );
};

// -----------------------------------------------------------------------

void OptLBoxString_Impl::Paint( const Point& rPos, SvLBox& rDev, USHORT, SvLBoxEntry* pEntry )
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

    SvTabListBox( pParent, nWinStyle ),

    m_bFirstPaint( TRUE ),
    m_pHeaderBar( NULL )
{
}

// -----------------------------------------------------------------------

void OptHeaderTabListBox::Paint( const Rectangle& rRect )
{
    if ( m_bFirstPaint )
    {
        m_bFirstPaint = FALSE;
        RepaintScrollBars();
    }

    SvTabListBox::Paint( rRect );
}

void OptHeaderTabListBox::InitHeaderBar( HeaderBar* _pHeaderBar )
{
    m_pHeaderBar = _pHeaderBar;
    SetScrolledHdl( LINK( this, OptHeaderTabListBox, TabBoxScrollHdl_Impl ) );
}

void OptHeaderTabListBox::InitEntry( SvLBoxEntry* pEntry, const XubString& rTxt,
                                     const Image& rImg1, const Image& rImg2 )
{
    SvTabListBox::InitEntry( pEntry, rTxt, rImg1, rImg2 );
    USHORT nTabCount = TabCount();

    for ( USHORT nCol = 1; nCol < nTabCount; ++nCol )
    {
        // alle Spalten mit eigener Klasse initialisieren (Spalte 0 == Bitmap)
        SvLBoxString* pCol = (SvLBoxString*)pEntry->GetItem( nCol );
        OptLBoxString_Impl* pStr = new OptLBoxString_Impl( pEntry, 0, pCol->GetText() );
        pEntry->ReplaceItem( pStr, nCol );
    }
}

IMPL_LINK( OptHeaderTabListBox, TabBoxScrollHdl_Impl, SvTabListBox*, pList )
{
    m_pHeaderBar->SetOffset( -GetXOffset() );
    return 0;
}

} // namespace svx
