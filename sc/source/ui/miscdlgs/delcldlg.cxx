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

#include <delcldlg.hxx>

static sal_uInt8 nDelItemChecked = 0;

ScDeleteCellDlg::ScDeleteCellDlg(weld::Window* pParent, bool bDisallowCellMove)
    : GenericDialogController(pParent, u"modules/scalc/ui/deletecells.ui"_ustr,
                              u"DeleteCellsDialog"_ustr)
    , m_xBtnCellsUp(m_xBuilder->weld_radio_button(u"up"_ustr))
    , m_xBtnCellsLeft(m_xBuilder->weld_radio_button(u"left"_ustr))
    , m_xBtnDelRows(m_xBuilder->weld_radio_button(u"rows"_ustr))
    , m_xBtnDelCols(m_xBuilder->weld_radio_button(u"cols"_ustr))
{
    if (bDisallowCellMove)
    {
        m_xBtnCellsUp->set_sensitive(false);
        m_xBtnCellsLeft->set_sensitive(false);

        switch (nDelItemChecked)
        {
            case 2:
                m_xBtnDelRows->set_active(true);
                break;
            case 3:
                m_xBtnDelCols->set_active(true);
                break;
            default:
                m_xBtnDelRows->set_active(true);
                break;
        }
    }
    else
    {
        switch (nDelItemChecked)
        {
            case 0:
                m_xBtnCellsUp->set_active(true);
                break;
            case 1:
                m_xBtnCellsLeft->set_active(true);
                break;
            case 2:
                m_xBtnDelRows->set_active(true);
                break;
            case 3:
                m_xBtnDelCols->set_active(true);
                break;
        }
    }
}

ScDeleteCellDlg::~ScDeleteCellDlg() {}

DelCellCmd ScDeleteCellDlg::GetDelCellCmd() const
{
    DelCellCmd nReturn = DelCellCmd::NONE;

    if (m_xBtnCellsUp->get_active())
    {
        nDelItemChecked = 0;
        nReturn = DelCellCmd::CellsUp;
    }
    else if (m_xBtnCellsLeft->get_active())
    {
        nDelItemChecked = 1;
        nReturn = DelCellCmd::CellsLeft;
    }
    else if (m_xBtnDelRows->get_active())
    {
        nDelItemChecked = 2;
        nReturn = DelCellCmd::Rows;
    }
    else if (m_xBtnDelCols->get_active())
    {
        nDelItemChecked = 3;
        nReturn = DelCellCmd::Cols;
    }

    return nReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
