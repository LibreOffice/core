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
        const rtl::OString& rDlgHelpId, const rtl::OString& sLbHelpId )
{
    SetText( rTitle );
    aFtLbTitle.SetText( rFixedText );
    SetHelpId( rDlgHelpId );
    aLb.SetHelpId( sLbHelpId );
}

void ScShowTabDlg::Insert( const String& rString, sal_Bool bSelected )
{
    aLb.InsertEntry( rString );
    if( bSelected )
        aLb.SelectEntryPos( aLb.GetEntryCount() - 1 );
}

//------------------------------------------------------------------------

sal_uInt16 ScShowTabDlg::GetSelectEntryCount() const
{
    return aLb.GetSelectEntryCount();
}

String ScShowTabDlg::GetSelectEntry(sal_uInt16 nPos) const
{
    return aLb.GetSelectEntry(nPos);
}

sal_uInt16 ScShowTabDlg::GetSelectEntryPos(sal_uInt16 nPos) const
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



