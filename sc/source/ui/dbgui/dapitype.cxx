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
#include "dapitype.hrc"

using namespace com::sun::star;



ScDataPilotSourceTypeDlg::ScDataPilotSourceTypeDlg(Window* pParent, bool bEnableExternal)
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
    sal_uInt16 nPos = m_pLbNamedRange->GetSelectEntryPos();
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

IMPL_LINK( ScDataPilotSourceTypeDlg, RadioClickHdl, RadioButton*, pBtn )
{
    m_pLbNamedRange->Enable(pBtn == m_pBtnNamedRange);
    return 0;
}



ScDataPilotServiceDlg::ScDataPilotServiceDlg( Window* pParent,
                                const uno::Sequence<OUString>& rServices ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DAPISERVICE ) ),
    //
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
    aFtService      ( this, ScResId( FT_SERVICE ) ),
    aLbService      ( this, ScResId( LB_SERVICE ) ),
    aFtSource       ( this, ScResId( FT_SOURCE ) ),
    aEdSource       ( this, ScResId( ED_SOURCE ) ),
    aFtName         ( this, ScResId( FT_NAME ) ),
    aEdName         ( this, ScResId( ED_NAME ) ),
    aFtUser         ( this, ScResId( FT_USER ) ),
    aEdUser         ( this, ScResId( ED_USER ) ),
    aFtPasswd       ( this, ScResId( FT_PASSWD ) ),
    aEdPasswd       ( this, ScResId( ED_PASSWD ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    long nCount = rServices.getLength();
    const OUString* pArray = rServices.getConstArray();
    for (long i=0; i<nCount; i++)
    {
        OUString aName = pArray[i];
        aLbService.InsertEntry( aName );
    }
    aLbService.SelectEntryPos( 0 );

    FreeResource();
}

ScDataPilotServiceDlg::~ScDataPilotServiceDlg()
{
}

OUString ScDataPilotServiceDlg::GetServiceName() const
{
    return aLbService.GetSelectEntry();
}

OUString ScDataPilotServiceDlg::GetParSource() const
{
    return aEdSource.GetText();
}

OUString ScDataPilotServiceDlg::GetParName() const
{
    return aEdName.GetText();
}

OUString ScDataPilotServiceDlg::GetParUser() const
{
    return aEdUser.GetText();
}

OUString ScDataPilotServiceDlg::GetParPass() const
{
    return aEdPasswd.GetText();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
