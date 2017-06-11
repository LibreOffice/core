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

#include "delcldlg.hxx"
#include "scresid.hxx"
#include "strings.hrc"

static sal_uInt8 nDelItemChecked=0;

ScDeleteCellDlg::ScDeleteCellDlg(vcl::Window* pParent, bool bDisallowCellMove)
    : ModalDialog(pParent, "DeleteCellsDialog", "modules/scalc/ui/deletecells.ui")
{
    get(m_pBtnCellsUp, "up");
    get(m_pBtnCellsLeft, "left");
    get(m_pBtnDelRows, "rows");
    get(m_pBtnDelCols, "cols");

    if (bDisallowCellMove)
    {
        m_pBtnCellsUp->Disable();
        m_pBtnCellsLeft->Disable();

        switch(nDelItemChecked)
        {
            case 2: m_pBtnDelRows->Check();break;
            case 3: m_pBtnDelCols->Check();break;
            default:m_pBtnDelRows->Check();break;
        }
    }
    else
    {
        switch(nDelItemChecked)
        {
            case 0: m_pBtnCellsUp->Check();break;
            case 1: m_pBtnCellsLeft->Check();break;
            case 2: m_pBtnDelRows->Check();break;
            case 3: m_pBtnDelCols->Check();break;
        }
    }
}

ScDeleteCellDlg::~ScDeleteCellDlg()
{
    disposeOnce();
}

void ScDeleteCellDlg::dispose()
{
    m_pBtnCellsUp.clear();
    m_pBtnCellsLeft.clear();
    m_pBtnDelRows.clear();
    m_pBtnDelCols.clear();
    ModalDialog::dispose();
}


DelCellCmd ScDeleteCellDlg::GetDelCellCmd() const
{
    DelCellCmd nReturn = DEL_NONE;

    if ( m_pBtnCellsUp->IsChecked()   )
    {
        nDelItemChecked=0;
        nReturn = DEL_CELLSUP;
    }
    else if ( m_pBtnCellsLeft->IsChecked() )
    {
        nDelItemChecked=1;
        nReturn = DEL_CELLSLEFT;
    }
    else if ( m_pBtnDelRows->IsChecked()   )
    {
        nDelItemChecked=2;
        nReturn = DEL_DELROWS;
    }
    else if ( m_pBtnDelCols->IsChecked()   )
    {
        nDelItemChecked=3;
        nReturn = DEL_DELCOLS;
    }

    return nReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
