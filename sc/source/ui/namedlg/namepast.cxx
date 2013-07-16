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
    : ModalDialog( pParent, "InsertNameDialog", "modules/scalc/ui/insertname.ui" )
{
    get(m_pBtnPasteAll, "pasteall");
    get(m_pBtnPaste, "paste");
    get(m_pBtnClose, "close");

    ScDocument* pDoc = pShell->GetDocument();
    std::map<OUString, ScRangeName*> aCopyMap;
    pDoc->GetRangeNameMap(aCopyMap);
    std::map<OUString, ScRangeName*>::iterator itr = aCopyMap.begin(), itrEnd = aCopyMap.end();
    for (; itr != itrEnd; ++itr)
    {
        OUString aTemp(itr->first);
        maRangeMap.insert(aTemp, new ScRangeName(*itr->second));
    }

    ScViewData* pViewData = pShell->GetViewData();
    ScAddress aPos(pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo());
    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("ctrl");
    Size aControlSize(210, 0);
    aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
    pContainer->set_width_request(aControlSize.Width());
    pContainer->set_height_request(10 * GetTextHeight());
    mpTable = new ScRangeManagerTable(*pContainer, maRangeMap, aPos);

    m_pBtnPaste->SetClickHdl( LINK( this, ScNamePasteDlg, ButtonHdl) );
    m_pBtnPasteAll->SetClickHdl( LINK( this, ScNamePasteDlg, ButtonHdl));
    m_pBtnClose->SetClickHdl( LINK( this, ScNamePasteDlg, ButtonHdl));

    if (!mpTable->GetEntryCount())
    {
        m_pBtnPaste->Disable();
        m_pBtnPasteAll->Disable();
    }
}

ScNamePasteDlg::~ScNamePasteDlg()
{
    delete mpTable;
}

//------------------------------------------------------------------

IMPL_LINK( ScNamePasteDlg, ButtonHdl, Button *, pButton )
{
    if( pButton == m_pBtnPasteAll )
    {
        EndDialog( BTN_PASTE_LIST );
    }
    else if( pButton == m_pBtnPaste )
    {
        std::vector<ScRangeNameLine> aSelectedLines = mpTable->GetSelectedEntries();
        for (std::vector<ScRangeNameLine>::const_iterator itr = aSelectedLines.begin();
                itr != aSelectedLines.end(); ++itr)
        {
            maSelectedNames.push_back(itr->aName);
        }
        EndDialog( BTN_PASTE_NAME );
    }
    else if( pButton == m_pBtnClose )
    {
        EndDialog( BTN_PASTE_CLOSE );
    }
    return 0;
}

//------------------------------------------------------------------

std::vector<OUString> ScNamePasteDlg::GetSelectedNames() const
{
    return maSelectedNames;
}

bool ScNamePasteDlg::IsAllSelected() const
{
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
