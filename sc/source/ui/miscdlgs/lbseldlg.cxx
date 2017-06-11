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

#include "lbseldlg.hxx"
#include "scresid.hxx"
#include "strings.hrc"

ScSelEntryDlg::ScSelEntryDlg(vcl::Window*  pParent, const std::vector<OUString> &rEntryList)
    : ModalDialog(pParent, "SelectRangeDialog", "modules/scalc/ui/selectrange.ui")
{
    get(m_pLb, "treeview");
    m_pLb->SetDropDownLineCount(8);
    m_pLb->set_width_request(m_pLb->approximate_char_width() * 32);
    m_pLb->SetDoubleClickHdl( LINK( this, ScSelEntryDlg, DblClkHdl ) );

    std::vector<OUString>::const_iterator pIter;
    for ( pIter = rEntryList.begin(); pIter != rEntryList.end(); ++pIter )
        m_pLb->InsertEntry(*pIter);

    if ( m_pLb->GetEntryCount() > 0 )
        m_pLb->SelectEntryPos( 0 );
}

ScSelEntryDlg::~ScSelEntryDlg()
{
    disposeOnce();
}

void ScSelEntryDlg::dispose()
{
    m_pLb.clear();
    ModalDialog::dispose();
}


OUString ScSelEntryDlg::GetSelectEntry() const
{
    return m_pLb->GetSelectEntry();
}

IMPL_LINK_NOARG(ScSelEntryDlg, DblClkHdl, ListBox&, void)
{
    EndDialog( RET_OK );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
