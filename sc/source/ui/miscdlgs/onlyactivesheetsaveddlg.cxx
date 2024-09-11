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
#include <onlyactivesheetsaveddlg.hxx>
#include <scmod.hxx>
#include <inputopt.hxx>

ScOnlyActiveSheetSavedDlg::ScOnlyActiveSheetSavedDlg(weld::Window* pParent)
    : GenericDialogController(pParent, u"modules/scalc/ui/onlyactivesheetsaveddialog.ui"_ustr,
                              u"OnlyActiveSheetSavedDialog"_ustr)
    , m_xWarningOnBox(m_xBuilder->weld_check_button("cbShow"))
{
    m_xDialog->set_default_response(RET_YES);
}

short ScOnlyActiveSheetSavedDlg::run()
{
    short nRet = RET_YES;
    ScModule* pScMod = SC_MOD();
    assert(pScMod);
    if (pScMod->GetInputOptions().GetWarnActiveSheet())
    {
        m_xDialog->run();
        if (!m_xWarningOnBox->get_active())
        {
            ScInputOptions aInputOpt(pScMod->GetInputOptions());
            aInputOpt.SetWarnActiveSheet(false);
            pScMod->SetInputOptions(aInputOpt);
        }
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
