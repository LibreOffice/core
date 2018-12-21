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

#include <swtypes.hxx>
#include <multmrk.hxx>
#include <toxmgr.hxx>
#include <wrtsh.hxx>

SwMultiTOXMarkDlg::SwMultiTOXMarkDlg(weld::Window* pParent, SwTOXMgr& rTOXMgr)
    : GenericDialogController(pParent, "modules/swriter/ui/selectindexdialog.ui", "SelectIndexDialog")
    , m_rMgr(rTOXMgr)
    , m_nPos(0)
    , m_xTextFT(m_xBuilder->weld_label("type"))
    , m_xTOXLB(m_xBuilder->weld_tree_view("treeview"))
{
    m_xTOXLB->set_size_request(m_xTOXLB->get_approximate_digit_width() * 32,
                               m_xTOXLB->get_height_rows(8));

    m_xTOXLB->connect_changed(LINK(this, SwMultiTOXMarkDlg, SelectHdl));

    sal_uInt16 nSize = m_rMgr.GetTOXMarkCount();
    for(sal_uInt16 i=0; i < nSize; ++i)
        m_xTOXLB->append_text(m_rMgr.GetTOXMark(i)->GetText(m_rMgr.GetShell()->GetLayout()));

    m_xTOXLB->select(0);
    m_xTextFT->set_label(m_rMgr.GetTOXMark(0)->GetTOXType()->GetTypeName());
}

IMPL_LINK( SwMultiTOXMarkDlg, SelectHdl, weld::TreeView&, rBox, void )
{
    if (rBox.get_selected_index() != -1)
    {
        SwTOXMark* pMark = m_rMgr.GetTOXMark(rBox.get_selected_index());
        m_xTextFT->set_label(pMark->GetTOXType()->GetTypeName());
        m_nPos = rBox.get_selected_index();
    }
}

void SwMultiTOXMarkDlg::Apply()
{
    m_rMgr.SetCurTOXMark(m_nPos);
}

SwMultiTOXMarkDlg::~SwMultiTOXMarkDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
