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

#include <shtabdlg.hxx>

ScShowTabDlg::ScShowTabDlg(weld::Window* pParent)
    : GenericDialogController(pParent, u"modules/scalc/ui/showsheetdialog.ui"_ustr,
                              u"ShowSheetDialog"_ustr)
    , m_xFrame(m_xBuilder->weld_frame(u"frame"_ustr))
    , m_xLb(m_xBuilder->weld_tree_view(u"treeview"_ustr))
{
    m_xLb->set_selection_mode(SelectionMode::Multiple);
    m_xLb->set_size_request(-1, m_xLb->get_height_rows(10));
    m_xLb->connect_row_activated(LINK(this, ScShowTabDlg, DblClkHdl));
}

ScShowTabDlg::~ScShowTabDlg() {}

void ScShowTabDlg::SetDescription(const OUString& rTitle, const OUString& rFixedText,
                                  const OUString& rDlgHelpId, const OUString& sLbHelpId)
{
    m_xDialog->set_title(rTitle);
    m_xFrame->set_label(rFixedText);
    m_xDialog->set_help_id(rDlgHelpId);
    m_xLb->set_help_id(sLbHelpId);
}

void ScShowTabDlg::Insert(const OUString& rString, bool bSelected)
{
    m_xLb->append_text(rString);
    if (bSelected)
        m_xLb->select(m_xLb->n_children() - 1);
}

std::vector<sal_Int32> ScShowTabDlg::GetSelectedRows() const
{
    auto aTmp = m_xLb->get_selected_rows();
    return std::vector<sal_Int32>(aTmp.begin(), aTmp.end());
}

OUString ScShowTabDlg::GetEntry(sal_Int32 nIndex) const { return m_xLb->get_text(nIndex); }

IMPL_LINK_NOARG(ScShowTabDlg, DblClkHdl, weld::TreeView&, bool)
{
    m_xDialog->response(RET_OK);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
