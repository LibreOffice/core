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

#include <gototabdlg.hxx>

ScGoToTabDlg::ScGoToTabDlg(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/scalc/ui/gotosheetdialog.ui", "GoToSheetDialog")
    , m_xFrame(m_xBuilder->weld_frame("frame"))
    , m_xLb(m_xBuilder->weld_tree_view("treeview"))
    , m_xEdNameMask(m_xBuilder->weld_entry("name-mask"))
{
    m_xLb->set_selection_mode(SelectionMode::Single);
    m_xLb->set_size_request(-1, m_xLb->get_height_rows(10));
    m_xLb->connect_row_activated(LINK(this, ScGoToTabDlg, DblClkHdl));
    m_xEdNameMask->connect_changed(LINK(this, ScGoToTabDlg, FindNameHdl));
}

ScGoToTabDlg::~ScGoToTabDlg() {}

void ScGoToTabDlg::SetDescription(const OUString& rTitle, const OUString& rFixedText,
                                  const OString& rDlgHelpId, const OString& sLbHelpId)
{
    m_xDialog->set_title(rTitle);
    m_xFrame->set_label(rFixedText);
    m_xDialog->set_help_id(rDlgHelpId);
    m_xLb->set_help_id(sLbHelpId);
}

void ScGoToTabDlg::Insert(const OUString& rString, bool bSelected)
{
    maCacheSheetsNames.push_back(rString);
    m_xLb->append_text(rString);
    if (bSelected)
        m_xLb->select(m_xLb->n_children() - 1);
}

OUString ScGoToTabDlg::GetSelectedEntry() const { return m_xLb->get_selected_text(); }

IMPL_LINK_NOARG(ScGoToTabDlg, DblClkHdl, weld::TreeView&, bool)
{
    m_xDialog->response(RET_OK);
    return true;
}

IMPL_LINK_NOARG(ScGoToTabDlg, FindNameHdl, weld::Entry&, void)
{
    const OUString aMask = m_xEdNameMask->get_text();
    m_xLb->clear();
    if (aMask.isEmpty())
    {
        for (const OUString& s : maCacheSheetsNames)
        {
            m_xLb->append_text(s);
        }
    }
    else
    {
        for (const OUString& s : maCacheSheetsNames)
        {
            if (s.indexOf(aMask) >= 0)
            {
                m_xLb->append_text(s);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
