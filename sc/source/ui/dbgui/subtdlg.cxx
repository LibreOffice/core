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

#include "tpsubt.hxx"
#include "scresid.hxx"
#include "subtdlg.hxx"

ScSubTotalDlg::ScSubTotalDlg(vcl::Window* pParent, const SfxItemSet* pArgSet)
    : SfxTabDialog(pParent, "SubTotalDialog",
        "modules/scalc/ui/subtotaldialog.ui", pArgSet)
{
    get(m_pBtnRemove, "remove");

    AddTabPage("1stgroup",  ScTpSubTotalGroup1::Create, nullptr);
    AddTabPage("2ndgroup",  ScTpSubTotalGroup2::Create, nullptr);
    AddTabPage("3rdgroup",  ScTpSubTotalGroup3::Create, nullptr);
    AddTabPage("options", ScTpSubTotalOptions::Create, nullptr);
    m_pBtnRemove->SetClickHdl( LINK( this, ScSubTotalDlg, RemoveHdl ) );
}

ScSubTotalDlg::~ScSubTotalDlg()
{
    disposeOnce();
}

void ScSubTotalDlg::dispose()
{
    m_pBtnRemove.clear();
    SfxTabDialog::dispose();
}

IMPL_LINK_TYPED( ScSubTotalDlg, RemoveHdl, Button *, pBtn, void )
{
    if (pBtn == m_pBtnRemove)
    {
        EndDialog( SCRET_REMOVE );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
