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
#include <table.hrc>

IMPL_LINK_NOARG(SwTableWidthDlg, LoseFocusHdl)
{
    sal_uInt16 nId = (sal_uInt16)m_pColNF->GetValue()-1;
    const SwTwips lWidth = rFnc.GetColWidth(nId);
    m_pWidthMF->SetMax(m_pWidthMF->Normalize(rFnc.GetMaxColWidth(nId)), FUNIT_TWIP);
    m_pWidthMF->SetValue(m_pWidthMF->Normalize(lWidth), FUNIT_TWIP);
    return 0;
}

SwTableWidthDlg::SwTableWidthDlg(vcl::Window *pParent, SwTableFUNC &rTableFnc )
    : SvxStandardDialog( pParent, "ColumnWidthDialog", "modules/swriter/ui/columnwidth.ui" )
    , rFnc(rTableFnc)
{
    get(m_pColNF, "column");
    get(m_pWidthMF, "width");

    bool bIsWeb = rTableFnc.GetShell()
                  && (dynamic_cast< const SwWebDocShell* >(
                                     rTableFnc.GetShell()->GetView().GetDocShell()) != nullptr );
    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref( bIsWeb )->GetMetric();
    ::SetFieldUnit(*m_pWidthMF, eFieldUnit);

    m_pColNF->SetValue( rFnc.GetCurColNum() +1 );
    m_pWidthMF->SetMin(m_pWidthMF->Normalize(MINLAY), FUNIT_TWIP);
    if(!m_pWidthMF->GetMin())
        m_pWidthMF->SetMin(1);

    if(rFnc.GetColCount() == 0)
        m_pWidthMF->SetMin(m_pWidthMF->Normalize(rFnc.GetColWidth(0)), FUNIT_TWIP);
    m_pColNF->SetMax(rFnc.GetColCount() +1 );
    m_pColNF->SetModifyHdl(LINK(this,SwTableWidthDlg, LoseFocusHdl));
    LoseFocusHdl();
}

SwTableWidthDlg::~SwTableWidthDlg()
{
    disposeOnce();
}

void SwTableWidthDlg::dispose()
{
    m_pColNF.clear();
    m_pWidthMF.clear();
    SvxStandardDialog::dispose();
}

void SwTableWidthDlg::Apply()
{
    rFnc.InitTabCols();
    rFnc.SetColWidth(
            static_cast< sal_uInt16 >(m_pColNF->GetValue() - 1),
            static_cast< sal_uInt16 >(m_pWidthMF->Denormalize(m_pWidthMF->GetValue(FUNIT_TWIP))));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
