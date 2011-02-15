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
                              sal_Bool          bColDefault )

    :   ModalDialog     ( pParent, ScResId( RID_SCDLG_COLORROW ) ),
        //
        aFlFrame        ( this, ScResId( FL_FRAME ) ),
        aBtnRows        ( this, ScResId( BTN_GROUP_ROWS ) ),
        aBtnCols        ( this, ScResId( BTN_GROUP_COLS ) ),
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



