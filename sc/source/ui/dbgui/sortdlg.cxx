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

#include <scui_def.hxx>
#include <tpsort.hxx>
#include <sortdlg.hxx>
#include <unotools/viewoptions.hxx>

ScSortDlg::ScSortDlg(weld::Window* pParent, const SfxItemSet* pArgSet)
    : SfxTabDialogController(pParent, u"modules/scalc/ui/sortdialog.ui"_ustr, u"SortDialog"_ustr, pArgSet)
{
    AddTabPage(u"criteria"_ustr, ScTabPageSortFields::Create, nullptr);
    AddTabPage(u"options"_ustr, ScTabPageSortOptions::Create, nullptr);

    // restore dialog size
    SvtViewOptions aDlgOpt(EViewType::Dialog, u"SortDialog"_ustr);
    if (aDlgOpt.Exists())
        m_xDialog->set_window_state(aDlgOpt.GetWindowState());
}

ScSortDlg::~ScSortDlg()
{
    // tdf#153852 - Make of sort dialog resizable (and remember size)
    SvtViewOptions aDlgOpt(EViewType::Dialog, u"SortDialog"_ustr);
    OUString sWindowState = m_xDialog->get_window_state(vcl::WindowDataMask::PosSize);
    aDlgOpt.SetWindowState(sWindowState);
}

ScSortWarningDlg::ScSortWarningDlg(weld::Window* pParent,
    std::u16string_view rExtendText, std::u16string_view rCurrentText)
    : GenericDialogController(pParent, u"modules/scalc/ui/sortwarning.ui"_ustr, u"SortWarning"_ustr)
    , m_xFtText(m_xBuilder->weld_label(u"sorttext"_ustr))
    , m_xBtnExtSort(m_xBuilder->weld_button(u"extend"_ustr))
    , m_xBtnCurSort(m_xBuilder->weld_button(u"current"_ustr))
{
    OUString sTextName = m_xFtText->get_label();
    sTextName = sTextName.replaceFirst("%1", rExtendText);
    sTextName = sTextName.replaceFirst("%2", rCurrentText);
    m_xFtText->set_label(sTextName);

    m_xBtnExtSort->connect_clicked( LINK( this, ScSortWarningDlg, BtnHdl ) );
    m_xBtnCurSort->connect_clicked( LINK( this, ScSortWarningDlg, BtnHdl ) );
}

ScSortWarningDlg::~ScSortWarningDlg()
{
}

IMPL_LINK(ScSortWarningDlg, BtnHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == m_xBtnExtSort.get())
    {
        m_xDialog->response(BTN_EXTEND_RANGE);
    }
    else if(&rBtn == m_xBtnCurSort.get())
    {
        m_xDialog->response(BTN_CURRENT_SELECTION);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
