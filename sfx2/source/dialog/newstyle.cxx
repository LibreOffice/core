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
#include "precompiled_sfx2.hxx"

// INCLUDE ---------------------------------------------------------------
#include <svl/style.hxx>

#include <sfx2/newstyle.hxx>
#include "dialog.hrc"
#include "newstyle.hrc"
#include "sfx2/sfxresid.hxx"

// PRIVATE METHODES ------------------------------------------------------

IMPL_LINK( SfxNewStyleDlg, OKHdl, Control *, pControl )
{
    (void)pControl; //unused
    const String aName( aColBox.GetText() );
    SfxStyleSheetBase* pStyle = rPool.Find( aName, rPool.GetSearchFamily(), SFXSTYLEBIT_ALL );
    if ( pStyle )
    {
        if ( !pStyle->IsUserDefined() )
        {
            InfoBox( this, SfxResId( MSG_POOL_STYLE_NAME ) ).Execute();
            return 0;
        }

        if ( RET_YES == aQueryOverwriteBox.Execute() )
            EndDialog( RET_OK );
    }
    else
        EndDialog( RET_OK );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxNewStyleDlg, ModifyHdl, ComboBox *, pBox )
{
    aOKBtn.Enable( pBox->GetText().EraseAllChars().Len() > 0 );
    return 0;
}
IMPL_LINK_INLINE_END( SfxNewStyleDlg, ModifyHdl, ComboBox *, pBox )

// CTOR / DTOR -----------------------------------------------------------

SfxNewStyleDlg::SfxNewStyleDlg( Window* pParent, SfxStyleSheetBasePool& rInPool ) :

    ModalDialog( pParent, SfxResId( DLG_NEW_STYLE_BY_EXAMPLE ) ),

    aColFL              ( this, SfxResId( FL_COL ) ),
    aColBox             ( this, SfxResId( LB_COL ) ),
    aOKBtn              ( this, SfxResId( BT_OK ) ),
    aCancelBtn          ( this, SfxResId( BT_CANCEL ) ),
    aQueryOverwriteBox  ( this, SfxResId( MSG_OVERWRITE ) ),

    rPool( rInPool )

{
    FreeResource();

    aOKBtn.SetClickHdl(LINK(this, SfxNewStyleDlg, OKHdl));
    aColBox.SetModifyHdl(LINK(this, SfxNewStyleDlg, ModifyHdl));
    aColBox.SetDoubleClickHdl(LINK(this, SfxNewStyleDlg, OKHdl));
    aColBox.SetAccessibleName(SfxResId(FL_COL));

    SfxStyleSheetBase *pStyle = rPool.First();
    while ( pStyle )
    {
        aColBox.InsertEntry(pStyle->GetName());
        pStyle = rPool.Next();
    }
}

// -----------------------------------------------------------------------

SfxNewStyleDlg::~SfxNewStyleDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
