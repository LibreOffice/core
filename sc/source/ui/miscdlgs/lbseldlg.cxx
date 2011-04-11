/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "lbseldlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"


//==================================================================

ScSelEntryDlg::ScSelEntryDlg(      Window*  pParent,
                                   sal_uInt16   nResId,
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

IMPL_LINK_INLINE_START( ScSelEntryDlg, DblClkHdl, void *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( ScSelEntryDlg, DblClkHdl, void *, EMPTYARG )

//------------------------------------------------------------------------

ScSelEntryDlg::~ScSelEntryDlg()
{
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
