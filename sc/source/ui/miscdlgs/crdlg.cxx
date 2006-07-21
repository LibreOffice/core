/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: crdlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:02:51 $
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

#include "crdlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScColOrRowDlg::ScColOrRowDlg( Window*       pParent,
                              const String& rStrTitle,
                              const String& rStrLabel,
                              BOOL          bColDefault )

    :   ModalDialog     ( pParent, ScResId( RID_SCDLG_COLORROW ) ),
        //
        aBtnRows        ( this, ScResId( BTN_GROUP_ROWS ) ),
        aBtnCols        ( this, ScResId( BTN_GROUP_COLS ) ),
        aFlFrame        ( this, ScResId( FL_FRAME ) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    SetText( rStrTitle );
    aFlFrame.SetText( rStrLabel );

    if ( bColDefault )
        aBtnCols.Check();
    else
        aBtnRows.Check();

    aBtnOk.SetClickHdl( LINK( this, ScColOrRowDlg, OkHdl ) );

    FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScColOrRowDlg::~ScColOrRowDlg()
{
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScColOrRowDlg, OkHdl, OKButton *, EMPTYARG )
{
    EndDialog( aBtnCols.IsChecked() ? SCRET_COLS : SCRET_ROWS );
    return 0;
}
IMPL_LINK_INLINE_END( ScColOrRowDlg, OkHdl, OKButton *, EMPTYARG )



