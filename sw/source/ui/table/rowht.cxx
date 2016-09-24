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

#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/dlgutil.hxx>

#include <fmtfsize.hxx>
#include <swtypes.hxx>
#include <rowht.hxx>
#include <wrtsh.hxx>
#include <frmatr.hxx>
#include <wdocsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <usrpref.hxx>

#include <cmdid.h>
#include <table.hrc>

void SwTableHeightDlg::Apply()
{
    SwTwips nHeight = static_cast< SwTwips >(m_pHeightEdit->Denormalize(m_pHeightEdit->GetValue(FUNIT_TWIP)));
    SwFormatFrameSize aSz(ATT_FIX_SIZE, 0, nHeight);

    SwFrameSize eFrameSize = m_pAutoHeightCB->IsChecked() ?
        ATT_MIN_SIZE : ATT_FIX_SIZE;
    if(eFrameSize != aSz.GetHeightSizeType())
    {
        aSz.SetHeightSizeType(eFrameSize);
    }
    rSh.SetRowHeight( aSz );
}

SwTableHeightDlg::SwTableHeightDlg(vcl::Window *pParent, SwWrtShell &rS)
    : SvxStandardDialog(pParent, "RowHeightDialog", "modules/swriter/ui/rowheight.ui")
    , rSh( rS )
{
    get(m_pHeightEdit, "heightmf");
    get(m_pAutoHeightCB, "fit");

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref( dynamic_cast< const SwWebDocShell*>(
                                rSh.GetView().GetDocShell() ) != nullptr  )->GetMetric();
    ::SetFieldUnit(*m_pHeightEdit, eFieldUnit);

    m_pHeightEdit->SetMin(MINLAY, FUNIT_TWIP);
    if(!m_pHeightEdit->GetMin())
        m_pHeightEdit->SetMin(1);
    SwFormatFrameSize *pSz;
    rSh.GetRowHeight( pSz );
    if ( pSz )
    {
        long nHeight = pSz->GetHeight();
        m_pAutoHeightCB->Check(pSz->GetHeightSizeType() != ATT_FIX_SIZE);
        m_pHeightEdit->SetValue(m_pHeightEdit->Normalize(nHeight), FUNIT_TWIP);

        delete pSz;
    }
}

SwTableHeightDlg::~SwTableHeightDlg()
{
    disposeOnce();
}

void SwTableHeightDlg::dispose()
{
    m_pHeightEdit.clear();
    m_pAutoHeightCB.clear();
    SvxStandardDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
