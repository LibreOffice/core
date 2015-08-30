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

#include "dapitype.hxx"
#include "scresid.hxx"
#include "sc.hrc"

using namespace com::sun::star;

ScDataPilotSourceTypeDlg::ScDataPilotSourceTypeDlg(vcl::Window* pParent, bool bEnableExternal)
    : ModalDialog( pParent, "SelectSourceDialog", "modules/scalc/ui/selectsource.ui" )
{
    get(m_pBtnSelection, "selection");
    get(m_pBtnNamedRange, "namedrange");
    get(m_pBtnDatabase, "database");
    get(m_pBtnExternal, "external");
    get(m_pLbNamedRange, "rangelb");
    m_pBtnSelection->SetClickHdl( LINK(this, ScDataPilotSourceTypeDlg, RadioClickHdl) );
    m_pBtnNamedRange->SetClickHdl( LINK(this, ScDataPilotSourceTypeDlg, RadioClickHdl) );
    m_pBtnDatabase->SetClickHdl( LINK(this, ScDataPilotSourceTypeDlg, RadioClickHdl) );
    m_pBtnExternal->SetClickHdl( LINK(this, ScDataPilotSourceTypeDlg, RadioClickHdl) );

    if (!bEnableExternal)
        m_pBtnExternal->Disable();

    m_pBtnSelection->Check();

    // Disabled unless at least one named range exists.
    m_pLbNamedRange->Disable();
    m_pBtnNamedRange->Disable();

    // Intentionally hide this button to see if anyone complains.
    m_pBtnExternal->Show(false);
}

ScDataPilotSourceTypeDlg::~ScDataPilotSourceTypeDlg()
{
    disposeOnce();
}

void ScDataPilotSourceTypeDlg::dispose()
{
    m_pBtnSelection.clear();
    m_pBtnNamedRange.clear();
    m_pBtnDatabase.clear();
    m_pBtnExternal.clear();
    m_pLbNamedRange.clear();
    ModalDialog::dispose();
}

bool ScDataPilotSourceTypeDlg::IsDatabase() const
{
    return m_pBtnDatabase->IsChecked();
}

bool ScDataPilotSourceTypeDlg::IsExternal() const
{
    return m_pBtnExternal->IsChecked();
}

bool ScDataPilotSourceTypeDlg::IsNamedRange() const
{
    return m_pBtnNamedRange->IsChecked();
}

OUString ScDataPilotSourceTypeDlg::GetSelectedNamedRange() const
{
    const sal_Int32 nPos = m_pLbNamedRange->GetSelectEntryPos();
    return m_pLbNamedRange->GetEntry(nPos);
}

void ScDataPilotSourceTypeDlg::AppendNamedRange(const OUString& rName)
{
    m_pLbNamedRange->InsertEntry(rName);
    if (m_pLbNamedRange->GetEntryCount() == 1)
    {
        // Select position 0 only for the first time.
        m_pLbNamedRange->SelectEntryPos(0);
        m_pBtnNamedRange->Enable();
    }
}

IMPL_LINK_TYPED( ScDataPilotSourceTypeDlg, RadioClickHdl, Button*, pBtn, void )
{
    m_pLbNamedRange->Enable(pBtn == m_pBtnNamedRange);
}

ScDataPilotServiceDlg::ScDataPilotServiceDlg( vcl::Window* pParent,
                                const uno::Sequence<OUString>& rServices ) :
    ModalDialog     ( pParent, "DapiserviceDialog", "modules/scalc/ui/dapiservicedialog.ui" )
{
    get(m_pLbService, "service");
    get(m_pEdSource, "source");
    get(m_pEdName, "name");
    get(m_pEdUser, "user");
    get(m_pEdPasswd, "password");

    long nCount = rServices.getLength();
    const OUString* pArray = rServices.getConstArray();
    for (long i=0; i<nCount; i++)
    {
        OUString aName = pArray[i];
        m_pLbService->InsertEntry( aName );
    }
    m_pLbService->SelectEntryPos( 0 );
}

ScDataPilotServiceDlg::~ScDataPilotServiceDlg()
{
    disposeOnce();
}

void ScDataPilotServiceDlg::dispose()
{
    m_pLbService.clear();
    m_pEdSource.clear();
    m_pEdName.clear();
    m_pEdUser.clear();
    m_pEdPasswd.clear();
    ModalDialog::dispose();
}


OUString ScDataPilotServiceDlg::GetServiceName() const
{
    return m_pLbService->GetSelectEntry();
}

OUString ScDataPilotServiceDlg::GetParSource() const
{
    return m_pEdSource->GetText();
}

OUString ScDataPilotServiceDlg::GetParName() const
{
    return m_pEdName->GetText();
}

OUString ScDataPilotServiceDlg::GetParUser() const
{
    return m_pEdUser->GetText();
}

OUString ScDataPilotServiceDlg::GetParPass() const
{
    return m_pEdPasswd->GetText();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
