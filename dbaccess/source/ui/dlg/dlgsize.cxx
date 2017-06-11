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

#include "dlgsize.hxx"
#include "dbu_dlg.hxx"
#include "core_resource.hxx"

namespace dbaui
{

#define DEF_ROW_HEIGHT  45
#define DEF_COL_WIDTH   227

DlgSize::DlgSize( vcl::Window* pParent, sal_Int32 nVal, bool bRow, sal_Int32 _nAlternativeStandard )
    : ModalDialog(pParent, bRow ? OUString("RowHeightDialog") : OUString("ColWidthDialog"),
        bRow ? OUString("dbaccess/ui/rowheightdialog.ui") : OUString("dbaccess/ui/colwidthdialog.ui"))
    , m_nPrevValue(nVal)
    , m_nStandard(bRow ? DEF_ROW_HEIGHT : DEF_COL_WIDTH)
{
    get(m_pMF_VALUE, "value");
    get(m_pCB_STANDARD, "automatic");

    if ( _nAlternativeStandard > 0 )
        m_nStandard = _nAlternativeStandard;
    m_pCB_STANDARD->SetClickHdl(LINK(this,DlgSize,CbClickHdl));

    m_pMF_VALUE->EnableEmptyFieldValue(true);
    bool bDefault = -1 == nVal;
    m_pCB_STANDARD->Check(bDefault);
    if (bDefault)
    {
        SetValue(m_nStandard);
        m_nPrevValue = m_nStandard;
    }
    LINK(this,DlgSize,CbClickHdl).Call(m_pCB_STANDARD);
}

DlgSize::~DlgSize()
{
    disposeOnce();
}

void DlgSize::dispose()
{
    m_pMF_VALUE.clear();
    m_pCB_STANDARD.clear();
    ModalDialog::dispose();
}


void DlgSize::SetValue( sal_Int32 nVal )
{
    m_pMF_VALUE->SetValue(nVal, FUNIT_CM );
}

sal_Int32 DlgSize::GetValue()
{
    if (m_pCB_STANDARD->IsChecked())
        return -1;
    return (sal_Int32)m_pMF_VALUE->GetValue( FUNIT_CM );
}

IMPL_LINK( DlgSize, CbClickHdl, Button *, pButton, void )
{
    if( pButton == m_pCB_STANDARD )
    {
        m_pMF_VALUE->Enable(!m_pCB_STANDARD->IsChecked());
        if (m_pCB_STANDARD->IsChecked())
        {
            m_nPrevValue = static_cast<sal_Int32>(m_pMF_VALUE->GetValue(FUNIT_CM));
                // don't use getValue as this will use m_pCB_STANDARD->to determine if we're standard
            m_pMF_VALUE->SetEmptyFieldValue();
        }
        else
        {
            SetValue( m_nPrevValue );
        }
    }
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
