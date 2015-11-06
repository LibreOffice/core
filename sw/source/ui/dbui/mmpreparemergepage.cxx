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

#include <mmpreparemergepage.hxx>

#include <comphelper/propertysequence.hxx>
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <dbui.hrc>
#include <swtypes.hxx>
#include <view.hxx>
#include <dbmgr.hxx>
#include <wrtsh.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <swabstdlg.hxx>

#include <unomid.h>

using namespace css::uno;
using namespace css::beans;
using namespace css::sdbc;

SwMailMergePrepareMergePage::SwMailMergePrepareMergePage( SwMailMergeWizard* _pParent)
    : svt::OWizardPage(_pParent, "MMPreparePage", "modules/swriter/ui/mmpreparepage.ui")
    , m_pWizard(_pParent)
{
    get(m_pFirstPB, "first");
    get(m_pPrevPB, "prev");
    get(m_pRecordED, "record-nospin");
    get(m_pNextPB, "next");
    get(m_pLastPB, "last");
    get(m_pExcludeCB, "exclude");
    get(m_pEditPB, "edit");

    m_pEditPB->SetClickHdl( LINK( this, SwMailMergePrepareMergePage, EditDocumentHdl_Impl));
    Link<Button*,void> aMoveLink(LINK( this, SwMailMergePrepareMergePage, MoveClickHdl_Impl));
    m_pFirstPB->SetClickHdl( aMoveLink );
    m_pPrevPB->SetClickHdl( aMoveLink );
    m_pNextPB->SetClickHdl( aMoveLink );
    m_pLastPB->SetClickHdl( aMoveLink );
    m_pRecordED->SetModifyHdl( LINK( this, SwMailMergePrepareMergePage, MoveEditHdl_Impl) );
    m_pExcludeCB->SetClickHdl(LINK(this, SwMailMergePrepareMergePage, ExcludeHdl_Impl));
    MoveHdl_Impl(m_pRecordED);
}

SwMailMergePrepareMergePage::~SwMailMergePrepareMergePage()
{
    disposeOnce();
}

void SwMailMergePrepareMergePage::dispose()
{
    m_pFirstPB.clear();
    m_pPrevPB.clear();
    m_pRecordED.clear();
    m_pNextPB.clear();
    m_pLastPB.clear();
    m_pExcludeCB.clear();
    m_pEditPB.clear();
    m_pWizard.clear();
    svt::OWizardPage::dispose();
}

IMPL_LINK_NOARG_TYPED(SwMailMergePrepareMergePage, EditDocumentHdl_Impl, Button*, void)
{
    m_pWizard->SetRestartPage(MM_PREPAREMERGEPAGE);
    m_pWizard->EndDialog(RET_EDIT_DOC);
}

IMPL_LINK_TYPED( SwMailMergePrepareMergePage, MoveClickHdl_Impl, Button*, pCtrl, void)
{
    MoveHdl_Impl(pCtrl);
}
IMPL_LINK_TYPED( SwMailMergePrepareMergePage, MoveEditHdl_Impl, Edit&, rEdit, void)
{
    MoveHdl_Impl(&rEdit);
}
void SwMailMergePrepareMergePage:: MoveHdl_Impl(Control* pCtrl)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    sal_Int32 nPos = rConfigItem.GetResultSetPosition();
    if (pCtrl == m_pFirstPB)
    {
        rConfigItem.MoveResultSet(1);
    }
    else if (pCtrl == m_pPrevPB)
    {
        rConfigItem.MoveResultSet(nPos - 1);
    }
    else if (pCtrl == m_pRecordED)
    {
        rConfigItem.MoveResultSet( static_cast< sal_Int32 >(m_pRecordED->GetValue()) );
    }
    else if (pCtrl == m_pNextPB)
        rConfigItem.MoveResultSet(nPos + 1);
    else if (pCtrl == m_pLastPB)
        rConfigItem.MoveResultSet(-1);

    nPos = rConfigItem.GetResultSetPosition();
    m_pRecordED->SetValue(nPos);
    bool bIsFirst;
    bool bIsLast;
    bool bValid = rConfigItem.IsResultSetFirstLast(bIsFirst, bIsLast);
    m_pFirstPB->Enable(bValid && !bIsFirst);
    m_pPrevPB->Enable(bValid && !bIsFirst);
    m_pNextPB->Enable(bValid && !bIsLast);
    m_pLastPB->Enable(bValid && !bIsLast);
    m_pExcludeCB->Check(rConfigItem.IsRecordExcluded( rConfigItem.GetResultSetPosition() ));
    //now the record has to be merged into the source document
    const SwDBData& rDBData = rConfigItem.GetCurrentDBData();
    Sequence<Any> vSelection = { makeAny(rConfigItem.GetResultSetPosition()) };
    auto aArgs(::comphelper::InitPropertySequence({
        {"Selection",        makeAny(vSelection)},
        {"DataSourceName",  makeAny(rDBData.sDataSource)},
        {"Command",          makeAny(rDBData.sCommand)},
        {"CommandType",      makeAny(rDBData.nCommandType)},
        {"ActiveConnection", makeAny(rConfigItem.GetConnection().getTyped())},
        {"Filter",           makeAny(rConfigItem.GetFilter())},
        {"Cursor",           makeAny(rConfigItem.GetResultSet())}
    }));
    svx::ODataAccessDescriptor aDescriptor(aArgs);
    SwWrtShell& rSh = m_pWizard->GetSwView()->GetWrtShell();
    SwMergeDescriptor aMergeDesc( DBMGR_MERGE, rSh, aDescriptor );
    rSh.GetDBManager()->MergeNew(aMergeDesc);
}

IMPL_LINK_TYPED( SwMailMergePrepareMergePage, ExcludeHdl_Impl, Button*, pBox, void)
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    rConfigItem.ExcludeRecord( rConfigItem.GetResultSetPosition(), static_cast<CheckBox*>(pBox)->IsChecked());
};

void  SwMailMergePrepareMergePage::ActivatePage()
{
    MoveHdl_Impl(m_pRecordED);
}

// merge the data into a new file
bool SwMailMergePrepareMergePage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if(::svt::WizardTypes::eTravelForward == _eReason && !rConfigItem.IsMergeDone())
    {
        m_pWizard->CreateTargetDocument();
        m_pWizard->SetRestartPage(MM_MERGEPAGE);
        m_pWizard->EndDialog(RET_TARGET_CREATED);
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
