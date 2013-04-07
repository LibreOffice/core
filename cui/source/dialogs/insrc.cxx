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

#include <dialmgr.hxx>
#include <svx/svxdlg.hxx>
#include <cuires.hrc>
#include "insrc.hxx"

bool SvxInsRowColDlg::isInsertBefore() const
{
    return !m_pAfterBtn->IsChecked();
}

sal_uInt16 SvxInsRowColDlg::getInsertCount() const
{
    return static_cast< sal_uInt16 >( m_pCountEdit->GetValue() );
}

SvxInsRowColDlg::SvxInsRowColDlg(Window* pParent, bool bCol, const OString& sHelpId )
    : ModalDialog(pParent, "InsertRowColumnDialog", "cui/ui/insertrowcolumn.ui")
    , aRow(CUI_RESSTR(RID_SVXSTR_ROW))
    , aCol(CUI_RESSTR(RID_SVXSTR_COL))
    , bColumn(bCol)
{
    get(m_pCountEdit, "insert_number");
    get(m_pBeforeBtn, "insert_before");
    get(m_pAfterBtn,  "insert_after");
    SetText( bColumn ? aCol : aRow );
    SetHelpId( sHelpId );
}

short SvxInsRowColDlg::Execute(void)
{
    return ModalDialog::Execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
