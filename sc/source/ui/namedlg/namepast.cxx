/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
