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


#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "namepast.hxx"
#include "scresid.hxx"
#include "docsh.hxx"
#include "miscdlgs.hrc"
#include "rangenam.hxx"
#include "viewdata.hxx"


//==================================================================

ScNamePasteDlg::ScNamePasteDlg( Window * pParent, ScDocShell* pShell, bool )
    : ModalDialog( pParent, ScResId( RID_SCDLG_NAMES_PASTE ) ),
    maBtnPasteAll    ( this, ScResId( BTN_PASTE_ALL ) ),
    maBtnPaste       ( this, ScResId( BTN_PASTE ) ),
    maHelpButton     ( this, ScResId( BTN_HELP ) ),
    maBtnClose       ( this, ScResId( BTN_CLOSE ) ),
    maFlDiv          ( this, ScResId( FL_DIV ) ),
    maCtrl           ( this, ScResId( CTRL_TABLE ) )
{
    ScDocument* pDoc = pShell->GetDocument();
    std::map<rtl::OUString, ScRangeName*> aCopyMap;
    pDoc->GetRangeNameMap(aCopyMap);
    std::map<rtl::OUString, ScRangeName*>::iterator itr = aCopyMap.begin(), itrEnd = aCopyMap.end();
    for (; itr != itrEnd; ++itr)
    {
        rtl::OUString aTemp(itr->first);
        maRangeMap.insert(aTemp, new ScRangeName(*itr->second));
    }

    ScViewData* pViewData = pShell->GetViewData();
    ScAddress aPos(pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo());
    mpTable = new ScRangeManagerTable(&maCtrl, maRangeMap, aPos);

    maBtnPaste.SetClickHdl( LINK( this, ScNamePasteDlg, ButtonHdl) );
    maBtnPasteAll.SetClickHdl( LINK( this, ScNamePasteDlg, ButtonHdl));
    maBtnClose.SetClickHdl( LINK( this, ScNamePasteDlg, ButtonHdl));

    if (!mpTable->GetEntryCount())
    {
        maBtnPaste.Disable();
        maBtnPasteAll.Disable();
    }

    FreeResource();
}

ScNamePasteDlg::~ScNamePasteDlg()
{
    delete mpTable;
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
        std::vector<ScRangeNameLine> aSelectedLines = mpTable->GetSelectedEntries();
        for (std::vector<ScRangeNameLine>::const_iterator itr = aSelectedLines.begin();
                itr != aSelectedLines.end(); ++itr)
        {
            maSelectedNames.push_back(itr->aName);
        }
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
    return maSelectedNames;
}

bool ScNamePasteDlg::IsAllSelected() const
{
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
