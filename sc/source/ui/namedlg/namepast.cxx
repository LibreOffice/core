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

#include "namepast.hxx"
#include "scresid.hxx"
#include "docsh.hxx"
#include "strings.hrc"
#include "rangenam.hxx"
#include "viewdata.hxx"

#include <o3tl/make_unique.hxx>

ScNamePasteDlg::ScNamePasteDlg( vcl::Window * pParent, ScDocShell* pShell, bool )
    : ModalDialog( pParent, "InsertNameDialog", "modules/scalc/ui/insertname.ui" )
{
    get(m_pBtnPasteAll, "pasteall");
    get(m_pBtnPaste, "paste");
    get(m_pBtnClose, "close");

    ScDocument& rDoc = pShell->GetDocument();
    std::map<OUString, ScRangeName*> aCopyMap;
    rDoc.GetRangeNameMap(aCopyMap);
    std::map<OUString, ScRangeName*>::iterator itr = aCopyMap.begin(), itrEnd = aCopyMap.end();
    for (; itr != itrEnd; ++itr)
    {
        OUString aTemp(itr->first);
        m_RangeMap.insert(std::make_pair(aTemp, o3tl::make_unique<ScRangeName>(*itr->second)));
    }

    ScViewData* pViewData = ScDocShell::GetViewData();
    ScAddress aPos(pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo());
    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("ctrl");
    Size aControlSize(210, 0);
    aControlSize = LogicToPixel(aControlSize, MapUnit::MapAppFont);
    pContainer->set_width_request(aControlSize.Width());
    pContainer->set_height_request(10 * GetTextHeight());
    mpTable = VclPtr<ScRangeManagerTable>::Create(*pContainer, m_RangeMap, aPos);

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
    disposeOnce();
}

void ScNamePasteDlg::dispose()
{
    mpTable.disposeAndClear();
    m_pBtnPasteAll.clear();
    m_pBtnPaste.clear();
    m_pBtnClose.clear();
    ModalDialog::dispose();
}

IMPL_LINK( ScNamePasteDlg, ButtonHdl, Button *, pButton, void )
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
}

const std::vector<OUString>& ScNamePasteDlg::GetSelectedNames() const
{
    return maSelectedNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
