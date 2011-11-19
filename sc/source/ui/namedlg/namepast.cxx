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

#include "namepast.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"
#include "rangenam.hxx"


//==================================================================

ScNamePasteDlg::ScNamePasteDlg( Window * pParent, const ScRangeName* pList, const ScRangeName* pLocalList, bool bInsList )
    : ModalDialog( pParent, ScResId( RID_SCDLG_NAMES_PASTE ) ),
    maHelpButton     ( this, ScResId( BTN_HELP ) ),
    maBtnClose       ( this, ScResId( BTN_CLOSE ) ),
    maBtnPaste       ( this, ScResId( BTN_PASTE ) ),
    maBtnPasteAll    ( this, ScResId( BTN_PASTE_ALL ) )
{

    maBtnPaste.SetClickHdl( LINK( this, ScNamePasteDlg, ButtonHdl) );
    maBtnPasteAll.SetClickHdl( LINK( this, ScNamePasteDlg, ButtonHdl));
    maBtnClose.SetClickHdl( LINK( this, ScNamePasteDlg, ButtonHdl));

    FreeResource();
}

//------------------------------------------------------------------

IMPL_LINK( ScNamePasteDlg, ButtonHdl, Button *, pButton )
{
    if( pButton == &maBtnPasteAll )
    {
        EndDialog( BTN_PASTE_LIST );
    }
    else if( pButton == &maBtnPaste )
    {
        EndDialog( BTN_PASTE_NAME );
    }
    else if( pButton == &maBtnClose )
    {
        EndDialog( BTN_PASTE_CLOSE );
    }
    return 0;
}

//------------------------------------------------------------------

std::vector<rtl::OUString> ScNamePasteDlg::GetSelectedNames() const
{
    std::vector<rtl::OUString> aSelectedNames;
    //aSelectedNames.push_back(aNameList.GetSelectEntry());
    return aSelectedNames;
}

bool ScNamePasteDlg::IsAllSelected() const
{
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
