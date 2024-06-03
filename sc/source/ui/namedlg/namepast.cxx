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
#include <globstr.hrc>
#include <scresid.hxx>
#include <compiler.hxx>

ScNamePasteDlg::ScNamePasteDlg(weld::Window* pParent, ScDocShell* pShell)
    : GenericDialogController(pParent, u"modules/scalc/ui/insertname.ui"_ustr,
                              u"InsertNameDialog"_ustr)
    , m_xBtnPasteAll(m_xBuilder->weld_button(u"pasteall"_ustr))
    , m_xBtnPaste(m_xBuilder->weld_button(u"paste"_ustr))
    , m_xBtnClose(m_xBuilder->weld_button(u"close"_ustr))
{
    ScDocument& rDoc = pShell->GetDocument();
    m_aSheetSep = OUString(rDoc.GetSheetSeparator());
    std::map<OUString, ScRangeName*> aCopyMap;
    rDoc.GetRangeNameMap(aCopyMap);
    for (const auto & [ aTemp, pName ] : aCopyMap)
    {
        m_RangeMap.insert(std::make_pair(aTemp, *pName));
    }

    ScAddress aPos;
    if (ScViewData* pViewData = ScDocShell::GetViewData())
        aPos = ScAddress(pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo());

    std::unique_ptr<weld::TreeView> xTreeView(m_xBuilder->weld_tree_view(u"ctrl"_ustr));
    xTreeView->set_size_request(xTreeView->get_approximate_digit_width() * 75,
                                xTreeView->get_height_rows(10));
    m_xTable.reset(new ScRangeManagerTable(std::move(xTreeView), m_RangeMap, aPos));

    m_xBtnPaste->connect_clicked(LINK(this, ScNamePasteDlg, ButtonHdl));
    m_xBtnPasteAll->connect_clicked(LINK(this, ScNamePasteDlg, ButtonHdl));
    m_xBtnClose->connect_clicked(LINK(this, ScNamePasteDlg, ButtonHdl));

    if (!m_xTable->n_children())
    {
        m_xBtnPaste->set_sensitive(false);
        m_xBtnPasteAll->set_sensitive(false);
    }
}

ScNamePasteDlg::~ScNamePasteDlg() {}

IMPL_LINK(ScNamePasteDlg, ButtonHdl, weld::Button&, rButton, void)
{
    if (&rButton == m_xBtnPasteAll.get())
    {
        m_xDialog->response(BTN_PASTE_LIST);
    }
    else if (&rButton == m_xBtnPaste.get())
    {
        const OUString aGlobalScope(ScResId(STR_GLOBAL_SCOPE));
        std::vector<ScRangeNameLine> aSelectedLines = m_xTable->GetSelectedEntries();
        for (const auto& rLine : aSelectedLines)
        {
            if (rLine.aScope == aGlobalScope)
                maSelectedNames.push_back(rLine.aName);
            else
            {
                OUString aSheet(rLine.aScope);
                ScCompiler::CheckTabQuotes(aSheet);
                maSelectedNames.push_back(aSheet + m_aSheetSep + rLine.aName);
            }
        }
        m_xDialog->response(BTN_PASTE_NAME);
    }
    else if (&rButton == m_xBtnClose.get())
    {
        m_xDialog->response(BTN_PASTE_CLOSE);
    }
}

const std::vector<OUString>& ScNamePasteDlg::GetSelectedNames() const { return maSelectedNames; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
