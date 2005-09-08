/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lbseldlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:10:45 $
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

#undef SC_DLLIMPLEMENTATION

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include <vcl/msgbox.hxx>

#include "lbseldlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScSelEntryDlg::ScSelEntryDlg(      Window*  pParent,
                                   USHORT   nResId,
                             const String&  aTitle,
                             const String&  aLbTitle,
                                   List&    aEntryList ) :
    ModalDialog     ( pParent, ScResId( nResId ) ),
    //
    aFlLbTitle      ( this, ScResId( FL_ENTRYLIST ) ),
    aLb             ( this, ScResId( LB_ENTRYLIST ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    SetText( aTitle );
    aFlLbTitle.SetText( aLbTitle );
    aLb.Clear();
    aLb.SetDoubleClickHdl( LINK( this, ScSelEntryDlg, DblClkHdl ) );

    void*   pListEntry = aEntryList.First();
    while ( pListEntry )
    {
        aLb.InsertEntry( *((String*)pListEntry ) );
        pListEntry = aEntryList.Next();
    }

    if ( aLb.GetEntryCount() > 0 )
        aLb.SelectEntryPos( 0 );

    //-------------
    FreeResource();
}

//------------------------------------------------------------------------

String ScSelEntryDlg::GetSelectEntry() const
{
    return aLb.GetSelectEntry();
}

//------------------------------------------------------------------------

USHORT ScSelEntryDlg::GetSelectEntryPos() const
{
    return aLb.GetSelectEntryPos();
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScSelEntryDlg, DblClkHdl, void *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( ScSelEntryDlg, DblClkHdl, void *, EMPTYARG )

//------------------------------------------------------------------------

__EXPORT ScSelEntryDlg::~ScSelEntryDlg()
{
}



