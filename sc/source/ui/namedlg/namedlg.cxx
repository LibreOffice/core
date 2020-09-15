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

#include <memory>
#include <global.hxx>
#include <reffact.hxx>
#include <compiler.hxx>
#include <document.hxx>
#include <docfunc.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <namedlg.hxx>
#include <viewdata.hxx>
#include <tabvwsh.hxx>

#include <globalnames.hxx>
#include <tokenarray.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <formula/errorcodes.hxx>
#include <unotools/charclass.hxx>

#include <map>

//logic

ScNameDlg::ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
        ScViewData*       ptrViewData,
        const ScAddress&  aCursorPos,
        std::map<OUString, std::unique_ptr<ScRangeName>> *const pRangeMap)
    : ScAnyRefDlgController(pB, pCW, pParent, "modules/scalc/ui/managenamesdialog.ui",
                            "ManageNamesDialog")

    , maGlobalNameStr(ScResId(STR_GLOBAL_SCOPE))
    , maErrInvalidNameStr(ScResId(STR_ERR_NAME_INVALID))
    , maErrNameInUse(ScResId(STR_ERR_NAME_EXISTS))
    , maStrMultiSelect(ScResId(STR_MULTI_SELECT))

    , mpViewData(ptrViewData)
    , mrDoc(ptrViewData->GetDocument())
    , maCursorPos(aCursorPos)
    , mbDataChanged(false)
    , mbCloseWithoutUndo(false)

    , m_xEdName(m_xBuilder->weld_entry("name"))
    , m_xFtAssign(m_xBuilder->weld_label("label3"))
    , m_xEdAssign(new formula::RefEdit(m_xBuilder->weld_entry("range")))
    , m_xRbAssign(new formula::RefButton(m_xBuilder->weld_button("assign")))
    , m_xLbScope(m_xBuilder->weld_combo_box("scope"))
    , m_xBtnPrintArea(m_xBuilder->weld_check_button("printrange"))
    , m_xBtnColHeader(m_xBuilder->weld_check_button("colheader"))
    , m_xBtnCriteria(m_xBuilder->weld_check_button("filter"))
    , m_xBtnRowHeader(m_xBuilder->weld_check_button("rowheader"))
    , m_xBtnAdd(m_xBuilder->weld_button("add"))
    , m_xBtnDelete(m_xBuilder->weld_button("delete"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
    , m_xFtInfo(m_xBuilder->weld_label("info"))
    , m_xExpander(m_xBuilder->weld_expander("more"))
{
    m_xEdAssign->SetReferences(this, m_xFtAssign.get());
    m_xRbAssign->SetReferences(this, m_xEdAssign.get());
    maStrInfoDefault = m_xFtInfo->get_label();
    m_xFtInfo->set_label(OUString());

    if (!pRangeMap)
    {
        std::map<OUString, ScRangeName*> aRangeMap;
        mrDoc.GetRangeNameMap(aRangeMap);
        for (const auto& [aTemp, pRangeName] : aRangeMap)
        {
            m_RangeMap.insert(std::make_pair(aTemp, std::make_unique<ScRangeName>(*pRangeName)));
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
}

void ScNameDlg::Init()
{
    //init UI

    std::unique_ptr<weld::TreeView> xTreeView(m_xBuilder->weld_tree_view("names"));
    xTreeView->set_size_request(xTreeView->get_approximate_digit_width() * 75,
                                xTreeView->get_height_rows(10));
    m_xRangeManagerTable.reset(new ScRangeManagerTable(std::move(xTreeView), m_RangeMap, maCursorPos));

    if (m_xRangeManagerTable->n_children())
    {
        m_xRangeManagerTable->set_cursor(0);
        m_xRangeManagerTable->CheckForFormulaString();
        SelectionChanged();
    }

    m_xRangeManagerTable->connect_changed( LINK( this, ScNameDlg, SelectionChangedHdl_Impl ) );

    m_xBtnOk->connect_clicked( LINK( this, ScNameDlg, OkBtnHdl ) );
    m_xBtnCancel->connect_clicked( LINK( this, ScNameDlg, CancelBtnHdl ) );
    m_xBtnAdd->connect_clicked( LINK( this, ScNameDlg, AddBtnHdl ) );
    m_xEdAssign->SetGetFocusHdl( LINK( this, ScNameDlg, AssignGetFocusHdl ) );
    m_xEdAssign->SetModifyHdl  ( LINK( this, ScNameDlg, RefEdModifyHdl ) );
    m_xEdName->connect_changed( LINK( this, ScNameDlg, EdModifyHdl ) );
    m_xLbScope->connect_changed( LINK(this, ScNameDlg, ScopeChangedHdl) );
    m_xBtnDelete->connect_clicked( LINK( this, ScNameDlg, RemoveBtnHdl ) );
    m_xBtnPrintArea->connect_toggled( LINK(this, ScNameDlg, EdModifyCheckBoxHdl ) );
    m_xBtnCriteria->connect_toggled( LINK(this, ScNameDlg, EdModifyCheckBoxHdl ) );
    m_xBtnRowHeader->connect_toggled( LINK(this, ScNameDlg, EdModifyCheckBoxHdl ) );
    m_xBtnColHeader->connect_toggled( LINK(this, ScNameDlg, EdModifyCheckBoxHdl ) );

    // Initialize scope list.
    m_xLbScope->append_text(maGlobalNameStr);
    m_xLbScope->set_active(0);
    SCTAB n = mrDoc.GetTableCount();
    for (SCTAB i = 0; i < n; ++i)
    {
        OUString aTabName;
        mrDoc.GetName(i, aTabName);
        m_xLbScope->append_text(aTabName);
    }

    CheckForEmptyTable();
}

bool ScNameDlg::IsRefInputMode() const
{
    return m_xEdAssign->GetWidget()->get_sensitive();
}

void ScNameDlg::RefInputDone( bool bForced)
{
    ScAnyRefDlgController::RefInputDone(bForced);
    RefEdModifyHdl(*m_xEdAssign);
}

void ScNameDlg::SetReference( const ScRange& rRef, ScDocument& rDocP )
{
    if (m_xEdAssign->GetWidget()->get_sensitive())
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(m_xEdAssign.get());
        OUString aRefStr(rRef.Format(rDocP, ScRefFlags::RANGE_ABS_3D,
                ScAddress::Details(rDocP.GetAddressConvention(), 0, 0)));
        m_xEdAssign->SetRefString( aRefStr );
    }
}

void ScNameDlg::Close()
{
    if (mbDataChanged && !mbCloseWithoutUndo)
        mpViewData->GetDocFunc().ModifyAllRangeNames(m_RangeMap);
    DoClose(ScNameDlgWrapper::GetChildWindowId());
}

void ScNameDlg::CheckForEmptyTable()
{
    if (!m_xRangeManagerTable->n_children())
    {
        m_xBtnDelete->set_sensitive(false);
        m_xEdAssign->GetWidget()->set_sensitive(false);
        m_xRbAssign->GetWidget()->set_sensitive(false);
        m_xEdName->set_sensitive(false);
        m_xLbScope->set_sensitive(false);

        m_xBtnCriteria->set_sensitive(false);
        m_xBtnPrintArea->set_sensitive(false);
        m_xBtnColHeader->set_sensitive(false);
        m_xBtnRowHeader->set_sensitive(false);
    }
    else
    {
        m_xBtnDelete->set_sensitive(true);
        m_xEdAssign->GetWidget()->set_sensitive(true);
        m_xRbAssign->GetWidget()->set_sensitive(true);
        m_xEdName->set_sensitive(true);
        m_xLbScope->set_sensitive(true);

        m_xBtnCriteria->set_sensitive(true);
        m_xBtnPrintArea->set_sensitive(true);
        m_xBtnColHeader->set_sensitive(true);
        m_xBtnRowHeader->set_sensitive(true);
    }
}

void ScNameDlg::SetActive()
{
    m_xEdAssign->GrabFocus();
    RefInputDone();
}

void ScNameDlg::UpdateChecks(const ScRangeData* pData)
{
    // remove handlers, we only want the handlers to process
    // user input and not when we are syncing the controls  with our internal
    // model ( also UpdateChecks is called already from some other event
    // handlers, triggering handlers while already processing a handler can
    // ( and does in this case ) corrupt the internal data

    m_xBtnCriteria->connect_toggled( Link<weld::ToggleButton&,void>() );
    m_xBtnPrintArea->connect_toggled( Link<weld::ToggleButton&,void>() );
    m_xBtnColHeader->connect_toggled( Link<weld::ToggleButton&,void>() );
    m_xBtnRowHeader->connect_toggled( Link<weld::ToggleButton&,void>() );

    m_xBtnCriteria->set_active( pData->HasType( ScRangeData::Type::Criteria ) );
    m_xBtnPrintArea->set_active( pData->HasType( ScRangeData::Type::PrintArea ) );
    m_xBtnColHeader->set_active( pData->HasType( ScRangeData::Type::ColHeader ) );
    m_xBtnRowHeader->set_active( pData->HasType( ScRangeData::Type::RowHeader ) );

    // Restore handlers so user input is processed again
    Link<weld::ToggleButton&,void> aToggleHandler = LINK( this, ScNameDlg, EdModifyCheckBoxHdl );
    m_xBtnCriteria->connect_toggled( aToggleHandler );
    m_xBtnPrintArea->connect_toggled( aToggleHandler );
    m_xBtnColHeader->connect_toggled( aToggleHandler );
    m_xBtnRowHeader->connect_toggled( aToggleHandler );
}

bool ScNameDlg::IsNameValid()
{
    OUString aScope = m_xLbScope->get_active_text();
    OUString aName = m_xEdName->get_text();
    aName = aName.trim();

    if (aName.isEmpty())
        return false;

    ScRangeName* pRangeName = GetRangeName( aScope );

    if (ScRangeData::IsNameValid( aName, mrDoc ) != ScRangeData::NAME_VALID)
    {
        m_xFtInfo->set_label_type(weld::LabelType::Error);
        m_xFtInfo->set_label(maErrInvalidNameStr);
        return false;
    }
    else if (pRangeName && pRangeName->findByUpperName(ScGlobal::getCharClassPtr()->uppercase(aName)))
    {
        m_xFtInfo->set_label_type(weld::LabelType::Error);
        m_xFtInfo->set_label(maErrNameInUse);
        return false;
    }
    m_xFtInfo->set_label( maStrInfoDefault );
    return true;
}

bool ScNameDlg::IsFormulaValid()
{
    ScCompiler aComp( &mrDoc, maCursorPos, mrDoc.GetGrammar());
    std::unique_ptr<ScTokenArray> pCode = aComp.CompileString(m_xEdAssign->GetText());
    if (pCode->GetCodeError() != FormulaError::NONE)
    {
        m_xFtInfo->set_label_type(weld::LabelType::Error);
        return false;
    }
    else
    {
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
    ScRangeData* pData = pRangeName->findByUpperName(ScGlobal::getCharClassPtr()->uppercase(rLine.aName));
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
        m_xRangeManagerTable->SetEntry(aLine);
    }
}

void ScNameDlg::RemovePushed()
{
    std::vector<ScRangeNameLine> aEntries = m_xRangeManagerTable->GetSelectedEntries();
    m_xRangeManagerTable->DeleteSelectedEntries();
    for (const auto& rEntry : aEntries)
    {
        ScRangeName* pRangeName = GetRangeName(rEntry.aScope);
        ScRangeData* pData = pRangeName->findByUpperName(ScGlobal::getCharClassPtr()->uppercase(rEntry.aName));
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
    m_xRangeManagerTable->GetCurrentLine(aLine);
    OUString aOldName = aLine.aName;
    OUString aNewName = m_xEdName->get_text();
    aNewName = aNewName.trim();
    m_xFtInfo->set_label_type(weld::LabelType::Normal);
    if (aNewName != aOldName)
    {
        if (!IsNameValid())
            return;
    }
    else
    {
        m_xFtInfo->set_label( maStrInfoDefault );
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
    OUString aExpr = m_xEdAssign->GetText();
    OUString aNewScope = m_xLbScope->get_active_text();

    ScRangeName* pOldRangeName = GetRangeName( aOldScope );
    ScRangeData* pData = pOldRangeName->findByUpperName( ScGlobal::getCharClassPtr()->uppercase(aOldName) );
    ScRangeName* pNewRangeName = GetRangeName( aNewScope );
    OSL_ENSURE(pData, "model and table should be in sync");
    // be safe and check for range data
    if (!pData)
        return;

    // Assign new index (0) only if the scope is changed, else keep the
    // existing index.
    sal_uInt16 nIndex = (aNewScope != aOldScope ? 0 : pData->GetIndex());

    pOldRangeName->erase(*pData);
    m_xRangeManagerTable->BlockUpdate();
    m_xRangeManagerTable->DeleteSelectedEntries();
    ScRangeData::Type nType = ScRangeData::Type::Name;
    if ( m_xBtnRowHeader->get_active() ) nType |= ScRangeData::Type::RowHeader;
    if ( m_xBtnColHeader->get_active() ) nType |= ScRangeData::Type::ColHeader;
    if ( m_xBtnPrintArea->get_active() ) nType |= ScRangeData::Type::PrintArea;
    if ( m_xBtnCriteria->get_active()  ) nType |= ScRangeData::Type::Criteria;

    ScRangeData* pNewEntry = new ScRangeData( mrDoc, aNewName, aExpr,
            maCursorPos, nType);
    pNewEntry->SetIndex( nIndex);
    pNewRangeName->insert(pNewEntry, false /*bReuseFreeIndex*/);
    aLine.aName = aNewName;
    aLine.aExpression = aExpr;
    aLine.aScope = aNewScope;
    m_xRangeManagerTable->addEntry(aLine, true);
    // tdf#128137 process pending async row change events while UpdatesBlocked in place
    Application::Reschedule(true);
    m_xRangeManagerTable->UnblockUpdate();
    mbDataChanged = true;
}

void ScNameDlg::SelectionChanged()
{
    //don't update if we have just modified due to user input
    if (m_xRangeManagerTable->UpdatesBlocked())
    {
        return;
    }

    if (m_xRangeManagerTable->IsMultiSelection())
    {
        m_xEdName->set_text(maStrMultiSelect);
        m_xEdAssign->SetText(maStrMultiSelect);

        m_xEdName->set_sensitive(false);
        m_xEdAssign->GetWidget()->set_sensitive(false);
        m_xRbAssign->GetWidget()->set_sensitive(false);
        m_xLbScope->set_sensitive(false);
        m_xBtnRowHeader->set_sensitive(false);
        m_xBtnColHeader->set_sensitive(false);
        m_xBtnPrintArea->set_sensitive(false);
        m_xBtnCriteria->set_sensitive(false);
    }
    else
    {
        ScRangeNameLine aLine;
        m_xRangeManagerTable->GetCurrentLine(aLine);
        m_xEdAssign->SetText(aLine.aExpression);
        m_xEdName->set_text(aLine.aName);
        m_xLbScope->set_active_text(aLine.aScope);
        ShowOptions(aLine);
        m_xBtnDelete->set_sensitive(true);
        m_xEdName->set_sensitive(true);
        m_xEdAssign->GetWidget()->set_sensitive(true);
        m_xRbAssign->GetWidget()->set_sensitive(true);
        m_xLbScope->set_sensitive(true);
        m_xBtnRowHeader->set_sensitive(true);
        m_xBtnColHeader->set_sensitive(true);
        m_xBtnPrintArea->set_sensitive(true);
        m_xBtnCriteria->set_sensitive(true);
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

IMPL_LINK_NOARG(ScNameDlg, OkBtnHdl, weld::Button&, void)
{
    response(RET_OK);
}

IMPL_LINK_NOARG(ScNameDlg, CancelBtnHdl, weld::Button&, void)
{
    response(RET_CANCEL);
}

IMPL_LINK_NOARG(ScNameDlg, AddBtnHdl, weld::Button&, void)
{
    AddPushed();
}

IMPL_LINK_NOARG(ScNameDlg, RemoveBtnHdl, weld::Button&, void)
{
    RemovePushed();
}

IMPL_LINK_NOARG(ScNameDlg, EdModifyCheckBoxHdl, weld::ToggleButton&, void)
{
    NameModified();
}

IMPL_LINK_NOARG(ScNameDlg, EdModifyHdl, weld::Entry&, void)
{
    NameModified();
}

IMPL_LINK_NOARG(ScNameDlg, RefEdModifyHdl, formula::RefEdit&, void)
{
    NameModified();
}

IMPL_LINK_NOARG(ScNameDlg, AssignGetFocusHdl, formula::RefEdit&, void)
{
    RefEdModifyHdl(*m_xEdAssign);
}

IMPL_LINK_NOARG(ScNameDlg, SelectionChangedHdl_Impl, weld::TreeView&, void)
{
    SelectionChanged();
}

IMPL_LINK_NOARG(ScNameDlg, ScopeChangedHdl, weld::ComboBox&, void)
{
    ScopeChanged();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
