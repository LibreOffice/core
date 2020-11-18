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

static sal_uInt8 nInsItemChecked = 0;

ScInsertCellDlg::ScInsertCellDlg(weld::Window* pParent, bool bDisallowCellMove)
    : GenericDialogController(pParent, "modules/scalc/ui/insertcells.ui", "InsertCellsDialog")
    , m_xBtnCellsDown(m_xBuilder->weld_radio_button("down"))
    , m_xBtnCellsRight(m_xBuilder->weld_radio_button("right"))
    , m_xBtnInsRow(m_xBuilder->weld_radio_button("rows"))
    , m_xBtnInsCol(m_xBuilder->weld_radio_button("cols"))
{
    if (bDisallowCellMove)
    {
        m_xBtnCellsDown->set_sensitive(false);
        m_xBtnCellsRight->set_sensitive(false);
        m_xBtnInsRow->set_active(true);

        switch (nInsItemChecked)
        {
            case 2:
                m_xBtnInsRow->set_active(true);
                break;
            case 3:
                m_xBtnInsCol->set_active(true);
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
                break;
            case 3:
                m_xBtnInsCol->set_active(true);
                break;
        }
    }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
