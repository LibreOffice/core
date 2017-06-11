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

#include "inscldlg.hxx"
#include "scresid.hxx"
#include "strings.hrc"

static sal_uInt8 nInsItemChecked=0;

ScInsertCellDlg::ScInsertCellDlg( vcl::Window* pParent,bool bDisallowCellMove) :
    ModalDialog     ( pParent, "InsertCellsDialog", "modules/scalc/ui/insertcells.ui")
{
    get(m_pBtnCellsDown, "down");
    get(m_pBtnCellsRight, "right");
    get(m_pBtnInsRow, "rows");
    get(m_pBtnInsCol, "cols");

    if (bDisallowCellMove)
    {
        m_pBtnCellsDown->Disable();
        m_pBtnCellsRight->Disable();
        m_pBtnInsRow->Check();

        switch(nInsItemChecked)
        {
            case 2: m_pBtnInsRow->Check();break;
            case 3: m_pBtnInsCol->Check();break;
            default:m_pBtnInsRow->Check();break;
        }
    }
    else
    {
        switch(nInsItemChecked)
        {
            case 0: m_pBtnCellsDown->Check();break;
            case 1: m_pBtnCellsRight->Check();break;
            case 2: m_pBtnInsRow->Check();break;
            case 3: m_pBtnInsCol->Check();break;
        }
    }
}

ScInsertCellDlg::~ScInsertCellDlg()
{
    disposeOnce();
}

void ScInsertCellDlg::dispose()
{
    m_pBtnCellsDown.clear();
    m_pBtnCellsRight.clear();
    m_pBtnInsRow.clear();
    m_pBtnInsCol.clear();
    ModalDialog::dispose();
}

InsCellCmd ScInsertCellDlg::GetInsCellCmd() const
{
    InsCellCmd nReturn = INS_NONE;

    if ( m_pBtnCellsDown->IsChecked() )
    {
        nInsItemChecked=0;
        nReturn = INS_CELLSDOWN;
    }
    else if ( m_pBtnCellsRight->IsChecked())
    {
        nInsItemChecked=1;
        nReturn = INS_CELLSRIGHT;
    }
    else if ( m_pBtnInsRow->IsChecked()   )
    {
        nInsItemChecked=2;
        nReturn = INS_INSROWS_BEFORE;
    }
    else if ( m_pBtnInsCol->IsChecked()   )
    {
        nInsItemChecked=3;
        nReturn = INS_INSCOLS_BEFORE;
    }

    return nReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
