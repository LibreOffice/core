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

#include <tpsubt.hxx>
#include <subtdlg.hxx>
#include <scui_def.hxx>

ScSubTotalDlg::ScSubTotalDlg(weld::Window* pParent, const SfxItemSet* pArgSet)
    : SfxTabDialogController(pParent, "modules/scalc/ui/subtotaldialog.ui", "SubTotalDialog", pArgSet)
    , m_xBtnRemove(m_xBuilder->weld_button("remove"))
{

    AddTabPage("1stgroup",  ScTpSubTotalGroup1::Create, nullptr);
    AddTabPage("2ndgroup",  ScTpSubTotalGroup2::Create, nullptr);
    AddTabPage("3rdgroup",  ScTpSubTotalGroup3::Create, nullptr);
    AddTabPage("options", ScTpSubTotalOptions::Create, nullptr);
    m_xBtnRemove->connect_clicked( LINK( this, ScSubTotalDlg, RemoveHdl ) );
}

ScSubTotalDlg::~ScSubTotalDlg()
{
}

IMPL_LINK_NOARG(ScSubTotalDlg, RemoveHdl, weld::Button&, void)
{
    m_xDialog->response(SCRET_REMOVE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
