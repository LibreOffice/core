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

#include <vcl/msgbox.hxx>

#include <shtabdlg.hxx>
#include <scresid.hxx>
#include <strings.hrc>

ScShowTabDlg::ScShowTabDlg(vcl::Window* pParent)
    : ModalDialog(pParent, "ShowSheetDialog", "modules/scalc/ui/showsheetdialog.ui")
{
    get(m_pFrame, "frame");
    get(m_pLb, "treeview");

    m_pLb->Clear();
    m_pLb->EnableMultiSelection(true);
    m_pLb->set_height_request(m_pLb->GetTextHeight() * 10);
    m_pLb->SetDoubleClickHdl( LINK( this, ScShowTabDlg, DblClkHdl ) );
}

ScShowTabDlg::~ScShowTabDlg()
{
    disposeOnce();
}

void ScShowTabDlg::dispose()
{
    m_pFrame.clear();
    m_pLb.clear();
    ModalDialog::dispose();
}

void ScShowTabDlg::SetDescription(
        const OUString& rTitle, const OUString& rFixedText,
        const OString& rDlgHelpId, const OString& sLbHelpId )
{
    SetText(rTitle);
    m_pFrame->set_label(rFixedText);
    SetHelpId( rDlgHelpId );
    m_pLb->SetHelpId( sLbHelpId );
}

void ScShowTabDlg::Insert( const OUString& rString, bool bSelected )
{
    m_pLb->InsertEntry( rString );
    if( bSelected )
        m_pLb->SelectEntryPos( m_pLb->GetEntryCount() - 1 );
}

sal_Int32 ScShowTabDlg::GetSelectedEntryCount() const
{
    return m_pLb->GetSelectedEntryCount();
}

OUString ScShowTabDlg::GetSelectedEntry(sal_Int32 nPos) const
{
    return m_pLb->GetSelectedEntry(nPos);
}

sal_Int32 ScShowTabDlg::GetSelectedEntryPos(sal_Int32 nPos) const
{
    return m_pLb->GetSelectedEntryPos(nPos);
}

IMPL_LINK_NOARG(ScShowTabDlg, DblClkHdl, ListBox&, void)
{
    EndDialog( RET_OK );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
