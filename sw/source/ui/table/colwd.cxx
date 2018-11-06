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

#include <sfx2/dispatch.hxx>
#include <svx/dlgutil.hxx>
#include <colwd.hxx>
#include <tablemgr.hxx>
#include <wrtsh.hxx>
#include <wdocsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <usrpref.hxx>

#include <cmdid.h>

IMPL_LINK_NOARG(SwTableWidthDlg, LoseFocusHdl, weld::SpinButton&, void)
{
    sal_uInt16 nId = static_cast<sal_uInt16>(m_xColNF->get_value()) - 1;
    const SwTwips lWidth = m_rFnc.GetColWidth(nId);
    m_xWidthMF->set_max(m_xWidthMF->normalize(m_rFnc.GetMaxColWidth(nId)), FieldUnit::TWIP);
    m_xWidthMF->set_value(m_xWidthMF->normalize(lWidth), FieldUnit::TWIP);
}

SwTableWidthDlg::SwTableWidthDlg(weld::Window *pParent, SwTableFUNC &rTableFnc)
    : GenericDialogController(pParent, "modules/swriter/ui/columnwidth.ui", "ColumnWidthDialog")
    , m_rFnc(rTableFnc)
    , m_xColNF(m_xBuilder->weld_spin_button("column"))
    , m_xWidthMF(m_xBuilder->weld_metric_spin_button("width", FieldUnit::CM))
{
    bool bIsWeb = rTableFnc.GetShell()
                  && (dynamic_cast< const SwWebDocShell* >(
                                     rTableFnc.GetShell()->GetView().GetDocShell()) != nullptr );
    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref( bIsWeb )->GetMetric();
    ::SetFieldUnit(*m_xWidthMF, eFieldUnit);

    m_xColNF->set_max(m_rFnc.GetColCount() + 1);
    m_xColNF->set_value(m_rFnc.GetCurColNum() + 1);

    if (m_rFnc.GetColCount() == 0)
        m_xWidthMF->set_min(m_xWidthMF->normalize(m_rFnc.GetColWidth(0)), FieldUnit::TWIP);
    else
        m_xWidthMF->set_min(m_xWidthMF->normalize(MINLAY), FieldUnit::TWIP);
    m_xColNF->connect_value_changed(LINK(this, SwTableWidthDlg, LoseFocusHdl));
    LoseFocusHdl(*m_xColNF);
}

void SwTableWidthDlg::Apply()
{
    m_rFnc.InitTabCols();
    m_rFnc.SetColWidth(static_cast<sal_uInt16>(m_xColNF->get_value() - 1),
                       static_cast<sal_uInt16>(m_xWidthMF->denormalize(m_xWidthMF->get_value(FieldUnit::TWIP))));
}

short SwTableWidthDlg::run()
{
    short nRet = GenericDialogController::run();
    if (nRet == RET_OK)
        Apply();
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
