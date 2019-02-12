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

#include <namepast.hxx>
#include <docsh.hxx>
#include <rangenam.hxx>
#include <viewdata.hxx>
#include <scui_def.hxx>

ScNamePasteDlg::ScNamePasteDlg(weld::Window * pParent, ScDocShell* pShell)
    : GenericDialogController(pParent, "modules/scalc/ui/insertname.ui", "InsertNameDialog")
    , m_xBtnPasteAll(m_xBuilder->weld_button("pasteall"))
    , m_xBtnPaste(m_xBuilder->weld_button("paste"))
    , m_xBtnClose(m_xBuilder->weld_button("close"))
{
    ScDocument& rDoc = pShell->GetDocument();
    std::map<OUString, ScRangeName*> aCopyMap;
    rDoc.GetRangeNameMap(aCopyMap);
    for (const auto& [aTemp, pName] : aCopyMap)
    {
        m_RangeMap.insert(std::make_pair(aTemp, std::make_unique<ScRangeName>(*pName)));
    }

    ScViewData* pViewData = ScDocShell::GetViewData();
    ScAddress aPos(pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo());

    std::unique_ptr<weld::TreeView> xTreeView(m_xBuilder->weld_tree_view("ctrl"));
    xTreeView->set_size_request(xTreeView->get_approximate_digit_width() * 75,
                                xTreeView->get_height_rows(10));
    m_xTable.reset(new RangeManagerTable(std::move(xTreeView), m_RangeMap, aPos));

    m_xBtnPaste->connect_clicked( LINK( this, ScNamePasteDlg, ButtonHdl) );
    m_xBtnPasteAll->connect_clicked( LINK( this, ScNamePasteDlg, ButtonHdl));
    m_xBtnClose->connect_clicked( LINK( this, ScNamePasteDlg, ButtonHdl));

    if (!m_xTable->n_children())
    {
        m_xBtnPaste->set_sensitive(false);
        m_xBtnPasteAll->set_sensitive(false);
    }
}

ScNamePasteDlg::~ScNamePasteDlg()
{
}

IMPL_LINK(ScNamePasteDlg, ButtonHdl, weld::Button&, rButton, void)
{
    if (&rButton == m_xBtnPasteAll.get())
    {
        m_xDialog->response(BTN_PASTE_LIST);
    }
    else if (&rButton == m_xBtnPaste.get())
    {
        std::vector<ScRangeNameLine> aSelectedLines = m_xTable->GetSelectedEntries();
        for (const auto& rLine : aSelectedLines)
        {
            maSelectedNames.push_back(rLine.aName);
        }
        m_xDialog->response(BTN_PASTE_NAME);
    }
    else if (&rButton == m_xBtnClose.get())
    {
        m_xDialog->response(BTN_PASTE_CLOSE);
    }
}

const std::vector<OUString>& ScNamePasteDlg::GetSelectedNames() const
{
    return maSelectedNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
