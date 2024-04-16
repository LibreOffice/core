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

#include <svtools/unitconv.hxx>
#include <colwd.hxx>
#include <tablemgr.hxx>
#include <wrtsh.hxx>
#include <wdocsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <usrpref.hxx>

IMPL_LINK_NOARG(SwTableWidthDlg, LoseFocusHdl, weld::SpinButton&, void)
{
    sal_uInt16 nId = o3tl::narrowing<sal_uInt16>(m_xColNF->get_value()) - 1;
    const SwTwips lWidth = m_xFnc->GetColWidth(nId);
    m_xWidthMF->set_max(m_xWidthMF->normalize(m_xFnc->GetMaxColWidth(nId)), FieldUnit::TWIP);
    m_xWidthMF->set_value(m_xWidthMF->normalize(lWidth), FieldUnit::TWIP);
}

SwTableWidthDlg::SwTableWidthDlg(weld::Window *pParent, SwWrtShell *pShell)
    : GenericDialogController(pParent, "modules/swriter/ui/columnwidth.ui", "ColumnWidthDialog")
    , m_xFnc(new SwTableFUNC(pShell))
    , m_xColNF(m_xBuilder->weld_spin_button("column"))
    , m_xWidthMF(m_xBuilder->weld_metric_spin_button("width", FieldUnit::CM))
{
    m_xFnc->InitTabCols();
    bool bIsWeb = m_xFnc->GetShell()
                  && (dynamic_cast< const SwWebDocShell* >(
                                     m_xFnc->GetShell()->GetView().GetDocShell()) != nullptr );
    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref( bIsWeb )->GetMetric();
    ::SetFieldUnit(*m_xWidthMF, eFieldUnit);

    m_xColNF->set_max(m_xFnc->GetColCount() + 1);
    m_xColNF->set_value(m_xFnc->GetCurColNum() + 1);

    if (m_xFnc->GetColCount() == 0)
        m_xWidthMF->set_min(m_xWidthMF->normalize(m_xFnc->GetColWidth(0)), FieldUnit::TWIP);
    else
        m_xWidthMF->set_min(m_xWidthMF->normalize(MINLAY), FieldUnit::TWIP);
    m_xColNF->connect_value_changed(LINK(this, SwTableWidthDlg, LoseFocusHdl));
    LoseFocusHdl(*m_xColNF);
}

SwTableWidthDlg::~SwTableWidthDlg() {}

void SwTableWidthDlg::Apply()
{
    m_xFnc->InitTabCols();
    m_xFnc->SetColWidth(o3tl::narrowing<sal_uInt16>(m_xColNF->get_value() - 1),
                       o3tl::narrowing<sal_uInt16>(m_xWidthMF->denormalize(m_xWidthMF->get_value(FieldUnit::TWIP))));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
