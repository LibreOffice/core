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

#include <lbseldlg.hxx>

ScSelEntryDlg::ScSelEntryDlg(weld::Window* pParent, const std::vector<OUString> &rEntryList)
    : GenericDialogController(pParent, "modules/scalc/ui/selectrange.ui", "SelectRangeDialog")
    , m_xLb(m_xBuilder->weld_tree_view("treeview"))
{
    m_xLb->set_size_request(m_xLb->get_approximate_digit_width() * 32,
                            m_xLb->get_height_rows(8));
    m_xLb->connect_row_activated(LINK(this, ScSelEntryDlg, DblClkHdl));

    for (const auto& rEntry : rEntryList)
        m_xLb->append_text(rEntry);

    if (m_xLb->n_children() > 0)
        m_xLb->select(0);
}

ScSelEntryDlg::~ScSelEntryDlg()
{
}

OUString ScSelEntryDlg::GetSelectedEntry() const
{
    return m_xLb->get_selected_text();
}

IMPL_LINK_NOARG(ScSelEntryDlg, DblClkHdl, weld::TreeView&, void)
{
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
