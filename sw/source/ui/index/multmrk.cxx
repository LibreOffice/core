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

#include "swtypes.hxx"

#include "multmrk.hxx"
#include "toxmgr.hxx"

#include "index.hrc"


SwMultiTOXMarkDlg::SwMultiTOXMarkDlg(Window* pParent, SwTOXMgr& rTOXMgr)
    : SvxStandardDialog(pParent, "SelectIndexDialog",
        "modules/swriter/ui/selectindexdialog.ui")
    , rMgr(rTOXMgr)
    , nPos(0)
{
    get(m_pTextFT, "type");
    get(m_pTOXLB, "treeview");
    m_pTOXLB->set_height_request(m_pTOXLB->GetTextHeight() * 10);
    m_pTOXLB->set_width_request(m_pTOXLB->approximate_char_width() * 25);

    m_pTOXLB->SetSelectHdl(LINK(this, SwMultiTOXMarkDlg, SelectHdl));

    sal_uInt16 nSize = rMgr.GetTOXMarkCount();
    for(sal_uInt16 i=0; i < nSize; ++i)
        m_pTOXLB->InsertEntry(rMgr.GetTOXMark(i)->GetText());

    m_pTOXLB->SelectEntryPos(0);
    m_pTextFT->SetText(rMgr.GetTOXMark(0)->GetTOXType()->GetTypeName());
}

IMPL_LINK_INLINE_START( SwMultiTOXMarkDlg, SelectHdl, ListBox *, pBox )
{
    if(pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
    {   SwTOXMark* pMark = rMgr.GetTOXMark(pBox->GetSelectEntryPos());
        m_pTextFT->SetText(pMark->GetTOXType()->GetTypeName());
        nPos = pBox->GetSelectEntryPos();
    }
    return 0;
}
IMPL_LINK_INLINE_END( SwMultiTOXMarkDlg, SelectHdl, ListBox *, pBox )


void SwMultiTOXMarkDlg::Apply()
{
    rMgr.SetCurTOXMark(nPos);
}

/*--------------------------------------------------
 overload dtor
--------------------------------------------------*/


SwMultiTOXMarkDlg::~SwMultiTOXMarkDlg() {}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
