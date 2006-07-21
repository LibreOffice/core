/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shtabdlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:09:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include <vcl/msgbox.hxx>

#include "shtabdlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScShowTabDlg::ScShowTabDlg( Window* pParent ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_SHOW_TAB ) ),
    aLb             ( this, ScResId( LB_ENTRYLIST ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
    aFtLbTitle      ( this, ScResId( FT_LABEL ) )
{
    aLb.Clear();
    aLb.SetDoubleClickHdl( LINK( this, ScShowTabDlg, DblClkHdl ) );

    //-------------
    FreeResource();
}

//------------------------------------------------------------------------

void ScShowTabDlg::SetDescription(
        const String& rTitle, const String& rFixedText,
        ULONG nDlgHelpId, ULONG nLbHelpId )
{
    SetText( rTitle );
    aFtLbTitle.SetText( rFixedText );
    SetHelpId( nDlgHelpId );
    aLb.SetHelpId( nLbHelpId );
}

void ScShowTabDlg::Insert( const String& rString, BOOL bSelected )
{
    aLb.InsertEntry( rString );
    if( bSelected )
        aLb.SelectEntryPos( aLb.GetEntryCount() - 1 );
}

//------------------------------------------------------------------------

USHORT ScShowTabDlg::GetSelectEntryCount() const
{
    return aLb.GetSelectEntryCount();
}

String ScShowTabDlg::GetSelectEntry(USHORT nPos) const
{
    return aLb.GetSelectEntry(nPos);
}

USHORT ScShowTabDlg::GetSelectEntryPos(USHORT nPos) const
{
    return aLb.GetSelectEntryPos(nPos);
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScShowTabDlg, DblClkHdl, void *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( ScShowTabDlg, DblClkHdl, void *, EMPTYARG )

__EXPORT ScShowTabDlg::~ScShowTabDlg()
{
}



