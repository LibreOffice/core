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
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include "dialmgr.hxx"
#include "splitcelldlg.hxx"
#include "cuires.hrc"

SvxSplitTableDlg::SvxSplitTableDlg( Window *pParent, bool bIsTableVertical,
    long nMaxVertical, long nMaxHorizontal )
    : SvxStandardDialog(pParent, "SplitCellsDialog", "cui/ui/splitcellsdialog.ui")
    , mnMaxVertical(nMaxVertical)
    , mnMaxHorizontal(nMaxHorizontal)
{
    get(m_pCountEdit, "countnf");
    get(m_pHorzBox, "hori");
    get(m_pVertBox, "vert");
    get(m_pPropCB, "prop");
    m_pHorzBox->SetClickHdl( LINK( this, SvxSplitTableDlg, ClickHdl ));
    m_pPropCB->SetClickHdl( LINK( this, SvxSplitTableDlg, ClickHdl ));
    m_pVertBox->SetClickHdl( LINK( this, SvxSplitTableDlg, ClickHdl ));

    if( mnMaxVertical < 2 )
        m_pVertBox->Enable(sal_False);

    //exchange the meaning of horizontal and vertical for vertical text
    if(bIsTableVertical)
    {
        Image aTmpImg(m_pHorzBox->GetModeRadioImage());
        String sTmp(m_pHorzBox->GetText());
        m_pHorzBox->SetText(m_pVertBox->GetText());
        m_pHorzBox->SetModeRadioImage(m_pVertBox->GetModeRadioImage());
        m_pVertBox->SetText(sTmp);
        m_pVertBox->SetModeRadioImage(aTmpImg);
    }
}

SvxSplitTableDlg::~SvxSplitTableDlg()
{
}

IMPL_LINK( SvxSplitTableDlg, ClickHdl, Button *, pButton )
{
    const bool bIsVert =  pButton == m_pVertBox ;
    long nMax = bIsVert ? mnMaxVertical : mnMaxHorizontal;
    m_pPropCB->Enable(!bIsVert);
    m_pCountEdit->SetMax( nMax );
    return 0;
}

bool SvxSplitTableDlg::IsHorizontal() const
{
    return m_pHorzBox->IsChecked();
}

bool SvxSplitTableDlg::IsProportional() const
{
    return m_pPropCB->IsChecked() && m_pHorzBox->IsChecked();
}

long SvxSplitTableDlg::GetCount() const
{
    return sal::static_int_cast<long>( m_pCountEdit->GetValue() );
}

short SvxSplitTableDlg::Execute()
{
    return SvxStandardDialog::Execute();
}

void SvxSplitTableDlg::Apply()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
