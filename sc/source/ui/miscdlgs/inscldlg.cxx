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

#include <inscldlg.hxx>
#include <viewdata.hxx>
#include <strings.hrc>
#include <scresid.hxx>

static sal_uInt8 nInsItemChecked = 0;

ScInsertCellDlg::ScInsertCellDlg(weld::Window* pParent, bool bDisallowCellMove)
    : GenericDialogController(pParent, "modules/scalc/ui/insertcells.ui", "InsertCellsDialog")
    , m_xBtnCellsDown(m_xBuilder->weld_radio_button("down"))
    , m_xBtnCellsRight(m_xBuilder->weld_radio_button("right"))
    , m_xBtnInsRow(m_xBuilder->weld_radio_button("rows"))
    , m_xBtnInsCol(m_xBuilder->weld_radio_button("cols"))
    , m_xNumberOfRows(m_xBuilder->weld_spin_button("number_of_rows"))
    , m_xNumberOfCols(m_xBuilder->weld_spin_button("number_of_columns"))
{
    const ScViewData* pViewData = ScDocShell::GetViewData();
    if (pViewData && pViewData->GetDocument().IsLayoutRTL(pViewData->GetTabNo()))
        m_xBtnCellsRight->set_label(ScResId(SCSTR_INSERT_RTL));

    m_xNumberOfRows->set_range(1, MAX_INS_ROWS);
    m_xNumberOfRows->set_value(1);
    m_xNumberOfCols->set_range(1, MAX_INS_COLS);
    m_xNumberOfCols->set_value(1);

    m_xBtnInsRow->connect_toggled(LINK(this, ScInsertCellDlg, RadioButtonsHdl));
    m_xBtnInsCol->connect_toggled(LINK(this, ScInsertCellDlg, RadioButtonsHdl));

    bool bColCount = false;
    bool bRowsCount = false;
    if (bDisallowCellMove)
    {
        m_xBtnCellsDown->set_sensitive(false);
        m_xBtnCellsRight->set_sensitive(false);
        m_xBtnInsRow->set_active(true);

        bRowsCount = true;
        switch (nInsItemChecked)
        {
            case 2:
                m_xBtnInsRow->set_active(true);
                break;
            case 3:
                m_xBtnInsCol->set_active(true);
                bRowsCount = false;
                bColCount = true;
                break;
            default:
                m_xBtnInsRow->set_active(true);
                break;
        }
    }
    else
    {
        switch (nInsItemChecked)
        {
            case 0:
                m_xBtnCellsDown->set_active(true);
                break;
            case 1:
                m_xBtnCellsRight->set_active(true);
                break;
            case 2:
                m_xBtnInsRow->set_active(true);
                bRowsCount = true;
                bColCount = false;
                break;
            case 3:
                m_xBtnInsCol->set_active(true);
                bRowsCount = false;
                bColCount = true;
                break;
        }
    }

    // if some cells are selected, then disable the SpinButtons
    const bool bMarked = pViewData && pViewData->GetMarkData().IsMarked();
    m_xNumberOfCols->set_sensitive(bColCount && !bMarked);
    m_xNumberOfRows->set_sensitive(bRowsCount && !bMarked);
}

ScInsertCellDlg::~ScInsertCellDlg() {}

InsCellCmd ScInsertCellDlg::GetInsCellCmd() const
{
    InsCellCmd nReturn = INS_NONE;

    if (m_xBtnCellsDown->get_active())
    {
        nInsItemChecked = 0;
        nReturn = INS_CELLSDOWN;
    }
    else if (m_xBtnCellsRight->get_active())
    {
        nInsItemChecked = 1;
        nReturn = INS_CELLSRIGHT;
    }
    else if (m_xBtnInsRow->get_active())
    {
        nInsItemChecked = 2;
        nReturn = INS_INSROWS_BEFORE;
    }
    else if (m_xBtnInsCol->get_active())
    {
        nInsItemChecked = 3;
        nReturn = INS_INSCOLS_BEFORE;
    }

    return nReturn;
}

size_t ScInsertCellDlg::GetCount() const
{
    switch (nInsItemChecked)
    {
        case 2:
            return m_xNumberOfRows->get_value() - 1;
        case 3:
            return m_xNumberOfCols->get_value() - 1;
        default:
            return 0;
    }
}

IMPL_LINK_NOARG(ScInsertCellDlg, RadioButtonsHdl, weld::Toggleable&, void)
{
    m_xNumberOfRows->set_sensitive(m_xBtnInsRow->get_active());
    m_xNumberOfCols->set_sensitive(m_xBtnInsCol->get_active());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
