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

#include "global.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "docfunc.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "namedlg.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"

#include "globalnames.hxx"
#include "tokenarray.hxx"

#include <sfx2/app.hxx>

#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>

#include <o3tl/make_unique.hxx>

#include <map>

//logic

ScNameDlg::ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
        ScViewData*       ptrViewData,
        const ScAddress&  aCursorPos,
        std::map<OUString, std::unique_ptr<ScRangeName>> *const pRangeMap)
    : ScAnyRefDlg(pB, pCW, pParent, "ManageNamesDialog", "modules/scalc/ui/managenamesdialog.ui")

    , maGlobalNameStr(ScGlobal::GetRscString(STR_GLOBAL_SCOPE))
    , maErrInvalidNameStr(ScGlobal::GetRscString(STR_ERR_NAME_INVALID))
    , maErrNameInUse(ScGlobal::GetRscString(STR_ERR_NAME_EXISTS))
    , maStrMultiSelect(ScGlobal::GetRscString(STR_MULTI_SELECT))

    , mpViewData(ptrViewData)
    , mpDoc(ptrViewData->GetDocument())
    , maCursorPos(aCursorPos)
    , mbNeedUpdate(true)
    , mbDataChanged(false)
    , mbCloseWithoutUndo(false)
{
    get(m_pEdName, "name");
    get(m_pEdAssign, "range");
    m_pEdAssign->SetReferences(this, m_pEdName);
    get(m_pRbAssign, "assign");
    m_pRbAssign->SetReferences(this, m_pEdAssign);
    get(m_pLbScope, "scope");
    get(m_pBtnPrintArea, "printrange");
    get(m_pBtnColHeader, "colheader");
    get(m_pBtnCriteria, "filter");
    get(m_pBtnRowHeader, "rowheader");
    get(m_pBtnAdd, "add");
    get(m_pBtnDelete, "delete");
    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");
    get(m_pFtInfo, "info");
    maStrInfoDefault = m_pFtInfo->GetText();
    m_pFtInfo->SetText(OUString());

    if (!pRangeMap)
    {
        std::map<OUString, ScRangeName*> aRangeMap;
        mpDoc->GetRangeNameMap(aRangeMap);
        std::map<OUString, ScRangeName*>::iterator itr = aRangeMap.begin(), itrEnd = aRangeMap.end();
        for (; itr != itrEnd; ++itr)
        {
            OUString aTemp(itr->first);
            m_RangeMap.insert(std::make_pair(aTemp,
                    o3tl::make_unique<ScRangeName>(*itr->second)));
        }
    }
    else
    {
        m_RangeMap.swap(*pRangeMap);
    }
    Init();
}

ScNameDlg::~ScNameDlg()
{
    disposeOnce();
}

void ScNameDlg::dispose()
{
    m_pRangeManagerTable.disposeAndClear();
    m_pEdName.clear();
    m_pEdAssign.clear();
    m_pRbAssign.clear();
    m_pLbScope.clear();
    m_pBtnPrintArea.clear();
    m_pBtnColHeader.clear();
    m_pBtnCriteria.clear();
    m_pBtnRowHeader.clear();
    m_pBtnAdd.clear();
    m_pBtnDelete.clear();
    m_pBtnOk.clear();
    m_pBtnCancel.clear();
    m_pFtInfo.clear();
    ScAnyRefDlg::dispose();
}

void ScNameDlg::Init()
{
    ScRange aRange;

    OSL_ENSURE( mpViewData && mpDoc, "ViewData oder Document nicht gefunden!" );

    //init UI
    m_pFtInfo->SetStyle(WB_VCENTER);

    SvSimpleTableContainer *pCtrl = get<SvSimpleTableContainer>("names");
    pCtrl->set_height_request(pCtrl->GetTextHeight()*12);

    m_pRangeManagerTable = VclPtr<ScRangeManagerTable>::Create(*pCtrl, m_RangeMap, maCursorPos);
    m_pRangeManagerTable->setInitListener(this);
    m_pRangeManagerTable->SetSelectHdl( LINK( this, ScNameDlg, SelectionChangedHdl_Impl ) );
    m_pRangeManagerTable->SetDeselectHdl( LINK( this, ScNameDlg, SelectionChangedHdl_Impl ) );

    m_pBtnOk->SetClickHdl  ( LINK( this, ScNameDlg, OkBtnHdl ) );
    m_pBtnCancel->SetClickHdl  ( LINK( this, ScNameDlg, CancelBtnHdl ) );
    m_pBtnAdd->SetClickHdl     ( LINK( this, ScNameDlg, AddBtnHdl ) );
    m_pEdAssign->SetGetFocusHdl( LINK( this, ScNameDlg, AssignGetFocusHdl ) );
    m_pEdAssign->SetModifyHdl  ( LINK( this, ScNameDlg, EdModifyHdl ) );
    m_pEdName->SetModifyHdl ( LINK( this, ScNameDlg, EdModifyHdl ) );
    m_pLbScope->SetSelectHdl( LINK(this, ScNameDlg, ScopeChangedHdl) );
    m_pBtnDelete->SetClickHdl ( LINK( this, ScNameDlg, RemoveBtnHdl ) );
    m_pBtnPrintArea->SetToggleHdl( LINK(this, ScNameDlg, EdModifyCheckBoxHdl ) );
    m_pBtnCriteria->SetToggleHdl( LINK(this, ScNameDlg, EdModifyCheckBoxHdl ) );
    m_pBtnRowHeader->SetToggleHdl( LINK(this, ScNameDlg, EdModifyCheckBoxHdl ) );
    m_pBtnColHeader->SetToggleHdl( LINK(this, ScNameDlg, EdModifyCheckBoxHdl ) );

    // Initialize scope list.
    m_pLbScope->InsertEntry(maGlobalNameStr);
    m_pLbScope->SelectEntryPos(0);
    SCTAB n = mpDoc->GetTableCount();
    for (SCTAB i = 0; i < n; ++i)
    {
        OUString aTabName;
        mpDoc->GetName(i, aTabName);
        m_pLbScope->InsertEntry(aTabName);
    }

    CheckForEmptyTable();
}

bool ScNameDlg::IsRefInputMode() const
{
    return m_pEdAssign->IsEnabled();
}

void ScNameDlg::RefInputDone( bool bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    EdModifyHdl(*m_pEdAssign);
}

void ScNameDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( m_pEdAssign->IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(m_pEdAssign);
        OUString aRefStr(rRef.Format(ScRefFlags::RANGE_ABS_3D, pDocP,
                ScAddress::Details(pDocP->GetAddressConvention(), 0, 0)));
        m_pEdAssign->SetRefString( aRefStr );
    }
}

bool ScNameDlg::Close()
{
    if (mbDataChanged && !mbCloseWithoutUndo)
        mpViewData->GetDocFunc().ModifyAllRangeNames(m_RangeMap);
    return DoClose( ScNameDlgWrapper::GetChildWindowId() );
}

void ScNameDlg::tableInitialized()
{
    if (m_pRangeManagerTable->GetSelectionCount())
        SelectionChanged();
}

void ScNameDlg::CheckForEmptyTable()
{
    if (!m_pRangeManagerTable->GetEntryCount())
    {
        m_pBtnDelete->Disable();
        m_pEdAssign->Disable();
        m_pRbAssign->Disable();
        m_pEdName->Disable();
        m_pLbScope->Disable();

        m_pBtnCriteria->Disable();
        m_pBtnPrintArea->Disable();
        m_pBtnColHeader->Disable();
        m_pBtnRowHeader->Disable();
    }
    else
    {
        m_pBtnDelete->Enable();
        m_pEdAssign->Enable();
        m_pRbAssign->Enable();
        m_pEdName->Enable();
        m_pLbScope->Enable();

        m_pBtnCriteria->Enable();
        m_pBtnPrintArea->Enable();
        m_pBtnColHeader->Enable();
        m_pBtnRowHeader->Enable();
    }
}

void ScNameDlg::CancelPushed()
{
    DoClose( ScNameDlgWrapper::GetChildWindowId() );
}

void ScNameDlg::SetActive()
{
    m_pEdAssign->GrabFocus();
    RefInputDone();
}

void ScNameDlg::UpdateChecks(ScRangeData* pData)
{
    // remove handlers, we only want the handlers to process
    // user input and not when we are syncing the controls  with our internal
    // model ( also UpdateChecks is called already from some other event
    // handlers, triggering handlers while already processing a handler can
    // ( and does in this case ) corrupt the internal data

    m_pBtnCriteria->SetToggleHdl( Link<CheckBox&,void>() );
    m_pBtnPrintArea->SetToggleHdl( Link<CheckBox&,void>() );
    m_pBtnColHeader->SetToggleHdl( Link<CheckBox&,void>() );
    m_pBtnRowHeader->SetToggleHdl( Link<CheckBox&,void>() );

    m_pBtnCriteria->Check( pData->HasType( ScRangeData::Type::Criteria ) );
    m_pBtnPrintArea->Check( pData->HasType( ScRangeData::Type::PrintArea ) );
    m_pBtnColHeader->Check( pData->HasType( ScRangeData::Type::ColHeader ) );
    m_pBtnRowHeader->Check( pData->HasType( ScRangeData::Type::RowHeader ) );

    // Restore handlers so user input is processed again
    Link<CheckBox&,void> aToggleHandler = LINK( this, ScNameDlg, EdModifyCheckBoxHdl );
    m_pBtnCriteria->SetToggleHdl( aToggleHandler );
    m_pBtnPrintArea->SetToggleHdl( aToggleHandler );
    m_pBtnColHeader->SetToggleHdl( aToggleHandler );
    m_pBtnRowHeader->SetToggleHdl( aToggleHandler );
}

bool ScNameDlg::IsNameValid()
{
    OUString aScope = m_pLbScope->GetSelectEntry();
    OUString aName = m_pEdName->GetText();
    aName = aName.trim();

    if (aName.isEmpty())
        return false;

    ScRangeName* pRangeName = GetRangeName( aScope );

    if (!ScRangeData::IsNameValid( aName, mpDoc ))
    {
        m_pFtInfo->SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        m_pFtInfo->SetText(maErrInvalidNameStr);
        return false;
    }
    else if (pRangeName && pRangeName->findByUpperName(ScGlobal::pCharClass->uppercase(aName)))
    {
        m_pFtInfo->SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        m_pFtInfo->SetText(maErrNameInUse);
        return false;
    }
    m_pFtInfo->SetText( maStrInfoDefault );
    return true;
}

bool ScNameDlg::IsFormulaValid()
{
    ScCompiler aComp( mpDoc, maCursorPos);
    aComp.SetGrammar( mpDoc->GetGrammar() );
    ScTokenArray* pCode = aComp.CompileString(m_pEdAssign->GetText());
    if (pCode->GetCodeError())
    {
        m_pFtInfo->SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        delete pCode;
        return false;
    }
    else
    {
        delete pCode;
        return true;
    }
}

ScRangeName* ScNameDlg::GetRangeName(const OUString& rScope)
{
    if (rScope == maGlobalNameStr)
        return m_RangeMap.find(OUString(STR_GLOBAL_RANGE_NAME))->second.get();
    else
        return m_RangeMap.find(rScope)->second.get();
}

void ScNameDlg::ShowOptions(const ScRangeNameLine& rLine)
{
    ScRangeName* pRangeName = GetRangeName(rLine.aScope);
    ScRangeData* pData = pRangeName->findByUpperName(ScGlobal::pCharClass->uppercase(rLine.aName));
    if (pData)
    {
        UpdateChecks(pData);
    }
}

void ScNameDlg::AddPushed()
{
    mbCloseWithoutUndo = true;
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    pViewSh->SwitchBetweenRefDialogs(this);
}

void ScNameDlg::SetEntry(const OUString& rName, const OUString& rScope)
{
    if (!rName.isEmpty())
    {
        mbDataChanged = true;
        ScRangeNameLine aLine;
        aLine.aName = rName;
        aLine.aScope = rScope;
        m_pRangeManagerTable->SetEntry(aLine);
    }
}

void ScNameDlg::RemovePushed()
{
    std::vector<ScRangeNameLine> aEntries = m_pRangeManagerTable->GetSelectedEntries();
    m_pRangeManagerTable->DeleteSelectedEntries();
    for (std::vector<ScRangeNameLine>::iterator itr = aEntries.begin(); itr != aEntries.end(); ++itr)
    {
        ScRangeName* pRangeName = GetRangeName(itr->aScope);
        ScRangeData* pData = pRangeName->findByUpperName(ScGlobal::pCharClass->uppercase(itr->aName));
        OSL_ENSURE(pData, "table and model should be in sync");
        // be safe and check for possible problems
        if (pData)
            pRangeName->erase(*pData);

        mbDataChanged = true;
    }
    CheckForEmptyTable();
}

void ScNameDlg::NameModified()
{
    ScRangeNameLine aLine;
    m_pRangeManagerTable->GetCurrentLine(aLine);
    OUString aOldName = aLine.aName;
    OUString aNewName = m_pEdName->GetText();
    aNewName = aNewName.trim();
    m_pFtInfo->SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());
    if (aNewName != aOldName)
    {
        if (!IsNameValid())
            return;
    }
    else
    {
        m_pFtInfo->SetText( maStrInfoDefault );
    }

    if (!IsFormulaValid())
    {
        //TODO: implement an info text
        return;
    }

    OUString aOldScope = aLine.aScope;
    //empty table
    if (aOldScope.isEmpty())
        return;
    OUString aExpr = m_pEdAssign->GetText();
    OUString aNewScope = m_pLbScope->GetSelectEntry();

    ScRangeName* pOldRangeName = GetRangeName( aOldScope );
    ScRangeData* pData = pOldRangeName->findByUpperName( ScGlobal::pCharClass->uppercase(aOldName) );
    ScRangeName* pNewRangeName = GetRangeName( aNewScope );
    OSL_ENSURE(pData, "model and table should be in sync");
    // be safe and check for range data
    if (pData)
    {
        // Assign new index (0) only if the scope is changed, else keep the
        // existing index.
        sal_uInt16 nIndex = (aNewScope != aOldScope ? 0 : pData->GetIndex());

        pOldRangeName->erase(*pData);
        mbNeedUpdate = false;
        m_pRangeManagerTable->DeleteSelectedEntries();
        ScRangeData::Type nType = ScRangeData::Type::Name;
        if ( m_pBtnRowHeader->IsChecked() ) nType |= ScRangeData::Type::RowHeader;
        if ( m_pBtnColHeader->IsChecked() ) nType |= ScRangeData::Type::ColHeader;
        if ( m_pBtnPrintArea->IsChecked() ) nType |= ScRangeData::Type::PrintArea;
        if ( m_pBtnCriteria->IsChecked()  ) nType |= ScRangeData::Type::Criteria;

        ScRangeData* pNewEntry = new ScRangeData( mpDoc, aNewName, aExpr,
                maCursorPos, nType);
        pNewEntry->SetIndex( nIndex);
        pNewRangeName->insert(pNewEntry, false /*bReuseFreeIndex*/);
        aLine.aName = aNewName;
        aLine.aExpression = aExpr;
        aLine.aScope = aNewScope;
        m_pRangeManagerTable->addEntry(aLine);
        mbNeedUpdate = true;
        mbDataChanged = true;
    }
}

void ScNameDlg::SelectionChanged()
{
    //don't update if we have just modified due to user input
    if (!mbNeedUpdate)
    {
        return;
    }

    if (m_pRangeManagerTable->IsMultiSelection())
    {
        m_pEdName->SetText(maStrMultiSelect);
        m_pEdAssign->SetText(maStrMultiSelect);

        m_pEdName->Disable();
        m_pEdAssign->Disable();
        m_pRbAssign->Disable();
        m_pLbScope->Disable();
        m_pBtnRowHeader->Disable();
        m_pBtnColHeader->Disable();
        m_pBtnPrintArea->Disable();
        m_pBtnCriteria->Disable();
    }
    else
    {
        ScRangeNameLine aLine;
        m_pRangeManagerTable->GetCurrentLine(aLine);
        m_pEdAssign->SetText(aLine.aExpression);
        m_pEdName->SetText(aLine.aName);
        m_pLbScope->SelectEntry(aLine.aScope);
        ShowOptions(aLine);
        m_pBtnDelete->Enable();
        m_pEdName->Enable();
        m_pEdAssign->Enable();
        m_pRbAssign->Enable();
        m_pLbScope->Enable();
        m_pBtnRowHeader->Enable();
        m_pBtnColHeader->Enable();
        m_pBtnPrintArea->Enable();
        m_pBtnCriteria->Enable();
    }
}

void ScNameDlg::ScopeChanged()
{
    NameModified();
}

void ScNameDlg::GetRangeNames(std::map<OUString, std::unique_ptr<ScRangeName>>& rRangeMap)
{
    m_RangeMap.swap(rRangeMap);
}

IMPL_LINK_NOARG_TYPED(ScNameDlg, OkBtnHdl, Button*, void)
{
    Close();
}

IMPL_LINK_NOARG_TYPED(ScNameDlg, CancelBtnHdl, Button*, void)
{
    CancelPushed();
}

IMPL_LINK_NOARG_TYPED(ScNameDlg, AddBtnHdl, Button*, void)
{
    AddPushed();
}

IMPL_LINK_NOARG_TYPED(ScNameDlg, RemoveBtnHdl, Button*, void)
{
    RemovePushed();
}

IMPL_LINK_NOARG_TYPED(ScNameDlg, EdModifyCheckBoxHdl, CheckBox&, void)
{
    NameModified();
}

IMPL_LINK_NOARG_TYPED(ScNameDlg, EdModifyHdl, Edit&, void)
{
    NameModified();
}

IMPL_LINK_NOARG_TYPED(ScNameDlg, AssignGetFocusHdl, Control&, void)
{
    EdModifyHdl(*m_pEdAssign);
}

IMPL_LINK_NOARG_TYPED(ScNameDlg, SelectionChangedHdl_Impl, SvTreeListBox*, void)
{
    SelectionChanged();
}

IMPL_LINK_NOARG_TYPED(ScNameDlg, ScopeChangedHdl, ListBox&, void)
{
    ScopeChanged();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
