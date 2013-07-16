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

#include "sfx2/app.hxx"

#include <vcl/msgbox.hxx>

#include <map>
#include <memory>

// defines -------------------------------------------------------------------

#define ABS_SREF          SCA_VALID \
    | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
    | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

//logic

ScNameDlg::ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
        ScViewData*       ptrViewData,
        const ScAddress&  aCursorPos, boost::ptr_map<OUString, ScRangeName>* pRangeMap  )
    : ScAnyRefDlg(pB, pCW, pParent, "ManageNamesDialog", "modules/scalc/ui/managenamesdialog.ui")

    , maGlobalNameStr(ScGlobal::GetRscString(STR_GLOBAL_SCOPE))
    , maErrInvalidNameStr(ScGlobal::GetRscString(STR_ERR_NAME_INVALID))
    , maErrNameInUse(ScGlobal::GetRscString(STR_ERR_NAME_EXISTS))
    , maStrMultiSelect(ScGlobal::GetRscString(STR_MULTI_SELECT))
    //
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
            maRangeMap.insert(aTemp, new ScRangeName(*itr->second));
        }
    }
    else
    {
        maRangeMap.swap(*pRangeMap);
    }
    Init();
}

ScNameDlg::~ScNameDlg()
{
    delete m_pRangeManagerTable;
}

void ScNameDlg::Init()
{
    ScRange aRange;

    OSL_ENSURE( mpViewData && mpDoc, "ViewData oder Document nicht gefunden!" );

    //init UI
    m_pFtInfo->SetStyle(WB_VCENTER);

    SvSimpleTableContainer *pCtrl = get<SvSimpleTableContainer>("names");
    pCtrl->set_height_request(pCtrl->GetTextHeight()*12);

    m_pRangeManagerTable = new ScRangeManagerTable(*pCtrl, maRangeMap, maCursorPos);
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
    m_pBtnPrintArea->SetToggleHdl( LINK(this, ScNameDlg, EdModifyHdl ) );
    m_pBtnCriteria->SetToggleHdl( LINK(this, ScNameDlg, EdModifyHdl ) );
    m_pBtnRowHeader->SetToggleHdl( LINK(this, ScNameDlg, EdModifyHdl ) );
    m_pBtnColHeader->SetToggleHdl( LINK(this, ScNameDlg, EdModifyHdl ) );

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



    if (m_pRangeManagerTable->GetSelectionCount())
    {
        SelectionChanged();
    }

    CheckForEmptyTable();

}

sal_Bool ScNameDlg::IsRefInputMode() const
{
    return m_pEdAssign->IsEnabled();
}

void ScNameDlg::RefInputDone( sal_Bool bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    EdModifyHdl(m_pEdAssign);
}

void ScNameDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( m_pEdAssign->IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(m_pEdAssign);
        String aRefStr;
        rRef.Format( aRefStr, ABS_DREF3D, pDocP,
                ScAddress::Details(pDocP->GetAddressConvention(), 0, 0) );
        m_pEdAssign->SetRefString( aRefStr );
    }
}

sal_Bool ScNameDlg::Close()
{
    if (mbDataChanged && !mbCloseWithoutUndo)
        mpViewData->GetDocFunc().ModifyAllRangeNames(maRangeMap);
    return DoClose( ScNameDlgWrapper::GetChildWindowId() );
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

    m_pBtnCriteria->SetToggleHdl( Link() );
    m_pBtnPrintArea->SetToggleHdl( Link() );
    m_pBtnColHeader->SetToggleHdl( Link() );
    m_pBtnRowHeader->SetToggleHdl( Link() );

    m_pBtnCriteria->Check( pData->HasType( RT_CRITERIA ) );
    m_pBtnPrintArea->Check( pData->HasType( RT_PRINTAREA ) );
    m_pBtnColHeader->Check( pData->HasType( RT_COLHEADER ) );
    m_pBtnRowHeader->Check( pData->HasType( RT_ROWHEADER ) );

    // Restore handlers so user input is processed again
    Link aToggleHandler = LINK( this, ScNameDlg, EdModifyHdl );
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
        return maRangeMap.find(OUString(STR_GLOBAL_RANGE_NAME))->second;
    else
        return maRangeMap.find(rScope)->second;
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


bool ScNameDlg::AddPushed()
{
    mbCloseWithoutUndo = true;
    ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell();
    pViewSh->SwitchBetweenRefDialogs(this);
    return false;
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
    std::vector<ScRangeNameLine> maEntries = m_pRangeManagerTable->GetSelectedEntries();
    m_pRangeManagerTable->DeleteSelectedEntries();
    for (std::vector<ScRangeNameLine>::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
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
        pOldRangeName->erase(*pData);
        mbNeedUpdate = false;
        m_pRangeManagerTable->DeleteSelectedEntries();
        RangeType nType = RT_NAME |
            (m_pBtnRowHeader->IsChecked() ? RT_ROWHEADER : RangeType(0))
            |(m_pBtnColHeader->IsChecked() ? RT_COLHEADER : RangeType(0))
            |(m_pBtnPrintArea->IsChecked() ? RT_PRINTAREA : RangeType(0))
            |(m_pBtnCriteria->IsChecked()  ? RT_CRITERIA  : RangeType(0));

        ScRangeData* pNewEntry = new ScRangeData( mpDoc, aNewName, aExpr,
                maCursorPos, nType);
        pNewRangeName->insert(pNewEntry);
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

void ScNameDlg::GetRangeNames(boost::ptr_map<OUString, ScRangeName>& rRangeMap)
{
    maRangeMap.swap(rRangeMap);
}

IMPL_LINK_NOARG(ScNameDlg, OkBtnHdl)
{
    Close();
    return 0;
}

IMPL_LINK_NOARG(ScNameDlg, CancelBtnHdl)
{
    CancelPushed();
    return 0;
}

IMPL_LINK_NOARG(ScNameDlg, AddBtnHdl)
{
    return AddPushed();
}

IMPL_LINK_NOARG(ScNameDlg, RemoveBtnHdl)
{
    RemovePushed();
    return 0;
}

IMPL_LINK_NOARG(ScNameDlg, EdModifyHdl)
{
    NameModified();
    return 0;
}

IMPL_LINK_NOARG(ScNameDlg, AssignGetFocusHdl)
{
    EdModifyHdl(m_pEdAssign);
    return 0;
}

IMPL_LINK_NOARG(ScNameDlg, SelectionChangedHdl_Impl)
{
    SelectionChanged();
    return 0;
}

IMPL_LINK_NOARG(ScNameDlg, ScopeChangedHdl)
{
    ScopeChanged();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
