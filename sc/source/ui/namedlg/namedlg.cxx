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
#include "namedlg.hrc"
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
        const ScAddress&  aCursorPos, boost::ptr_map<rtl::OUString, ScRangeName>* pRangeMap  )

:   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_NAMES ),
    //
    maEdName         ( this, ScResId( ED_NAME2 ) ),
    maEdAssign       ( this, this, ScResId( ED_ASSIGN ) ),
    aRbAssign       ( this, ScResId( RB_ASSIGN ), &maEdAssign, this ),
    maLbScope( this, ScResId(LB_SCOPE) ),
    //
    maBtnMore        ( this, ScResId( BTN_MORE ) ),
    maBtnPrintArea   ( this, ScResId( BTN_PRINTAREA ) ),
    maBtnColHeader   ( this, ScResId( BTN_COLHEADER ) ),
    maBtnCriteria    ( this, ScResId( BTN_CRITERIA ) ),
    maBtnRowHeader   ( this, ScResId( BTN_ROWHEADER ) ),
    //
    maBtnAdd         ( this, ScResId( BTN_ADD ) ),
    maBtnDelete      ( this, ScResId( BTN_DELETE ) ),
    maBtnHelp        ( this, ScResId( BTN_HELP ) ),
    maBtnOk          ( this, ScResId( BTN_NAME_OK ) ),
    maBtnCancel      ( this, ScResId( BTN_NAME_CANCEL ) ),
    //
    maFtScope( this, ScResId(FT_SCOPE) ),
    maFtRange( this, ScResId(FT_RANGE) ),
    maFtName( this,  ScResId(FT_NAME) ),
    maFlDiv          ( this, ScResId( FL_DIV ) ),
    maFtInfo         ( this, ScResId( FT_INFO ) ),
    //
    maNameMgrCtrl   ( this, ScResId( CTRL_MANAGENAMES ) ),
    //
    maGlobalNameStr  ( ScGlobal::GetRscString(STR_GLOBAL_SCOPE) ),
    maErrInvalidNameStr( ScGlobal::GetRscString(STR_ERR_NAME_INVALID)),
    maErrNameInUse   ( ScGlobal::GetRscString(STR_ERR_NAME_EXISTS)),
    maStrInfoDefault ( SC_RESSTR(STR_DEFAULT_INFO)),
    maStrMultiSelect ( SC_RESSTR(STR_MULTI_SELECT)),
    //
    mpViewData       ( ptrViewData ),
    mpDoc            ( ptrViewData->GetDocument() ),
    maCursorPos      ( aCursorPos ),
    mbNeedUpdate     ( true ),
    mbDataChanged    ( false ),
    mbCloseWithoutUndo( false )
{
    if (!pRangeMap)
    {
        std::map<rtl::OUString, ScRangeName*> aRangeMap;
        mpDoc->GetRangeNameMap(aRangeMap);
        std::map<rtl::OUString, ScRangeName*>::iterator itr = aRangeMap.begin(), itrEnd = aRangeMap.end();
        for (; itr != itrEnd; ++itr)
        {
            rtl::OUString aTemp(itr->first);
            maRangeMap.insert(aTemp, new ScRangeName(*itr->second));
        }
    }
    else
    {
        maRangeMap.swap(*pRangeMap);
    }
    Init();
    FreeResource();
}

ScNameDlg::~ScNameDlg()
{
    delete mpRangeManagerTable;
}

void ScNameDlg::Init()
{
    ScRange aRange;

    OSL_ENSURE( mpViewData && mpDoc, "ViewData oder Document nicht gefunden!" );

    //init UI
    maFtInfo.SetStyle(WB_VCENTER);

    mpRangeManagerTable = new ScRangeManagerTable(&maNameMgrCtrl, maRangeMap, maCursorPos);
    mpRangeManagerTable->SetSelectHdl( LINK( this, ScNameDlg, SelectionChangedHdl_Impl ) );
    mpRangeManagerTable->SetDeselectHdl( LINK( this, ScNameDlg, SelectionChangedHdl_Impl ) );

    maBtnOk.SetClickHdl  ( LINK( this, ScNameDlg, OkBtnHdl ) );
    maBtnCancel.SetClickHdl  ( LINK( this, ScNameDlg, CancelBtnHdl ) );
    maBtnAdd.SetClickHdl     ( LINK( this, ScNameDlg, AddBtnHdl ) );
    maEdAssign.SetGetFocusHdl( LINK( this, ScNameDlg, AssignGetFocusHdl ) );
    maEdAssign.SetModifyHdl  ( LINK( this, ScNameDlg, EdModifyHdl ) );
    maEdName.SetModifyHdl ( LINK( this, ScNameDlg, EdModifyHdl ) );
    maLbScope.SetSelectHdl( LINK(this, ScNameDlg, ScopeChangedHdl) );
    maBtnDelete.SetClickHdl ( LINK( this, ScNameDlg, RemoveBtnHdl ) );
    maBtnMore.SetClickHdl   ( LINK( this, ScNameDlg, MoreBtnHdl ) );
    maBtnPrintArea.SetToggleHdl( LINK(this, ScNameDlg, EdModifyHdl ) );
    maBtnCriteria.SetToggleHdl( LINK(this, ScNameDlg, EdModifyHdl ) );
    maBtnRowHeader.SetToggleHdl( LINK(this, ScNameDlg, EdModifyHdl ) );
    maBtnColHeader.SetToggleHdl( LINK(this, ScNameDlg, EdModifyHdl ) );

    maBtnCriteria .Hide();
    maBtnPrintArea.Hide();
    maBtnColHeader.Hide();
    maBtnRowHeader.Hide();

    // Initialize scope list.
    maLbScope.InsertEntry(maGlobalNameStr);
    maLbScope.SelectEntryPos(0);
    SCTAB n = mpDoc->GetTableCount();
    for (SCTAB i = 0; i < n; ++i)
    {
        rtl::OUString aTabName;
        mpDoc->GetName(i, aTabName);
        maLbScope.InsertEntry(aTabName);
    }



    if (mpRangeManagerTable->GetSelectionCount())
    {
        SelectionChanged();
    }

    CheckForEmptyTable();

}

sal_Bool ScNameDlg::IsRefInputMode() const
{
    return maEdAssign.IsEnabled();
}

void ScNameDlg::RefInputDone( sal_Bool bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    EdModifyHdl(&maEdAssign);
}

void ScNameDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( maEdAssign.IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(&maEdAssign);
        String aRefStr;
        rRef.Format( aRefStr, ABS_DREF3D, pDocP,
                ScAddress::Details(pDocP->GetAddressConvention(), 0, 0) );
        maEdAssign.SetRefString( aRefStr );
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
    if (!mpRangeManagerTable->GetEntryCount())
    {
        maBtnDelete.Disable();
        maEdAssign.Disable();
        aRbAssign.Disable();
        maEdName.Disable();
        maLbScope.Disable();

        maBtnCriteria.Disable();
        maBtnPrintArea.Disable();
        maBtnColHeader.Disable();
        maBtnRowHeader.Disable();
    }
    else
    {
        maBtnDelete.Enable();
        maEdAssign.Enable();
        aRbAssign.Enable();
        maEdName.Enable();
        maLbScope.Enable();


        maBtnCriteria.Enable();
        maBtnPrintArea.Enable();
        maBtnColHeader.Enable();
        maBtnRowHeader.Enable();
    }
}

void ScNameDlg::CancelPushed()
{
    DoClose( ScNameDlgWrapper::GetChildWindowId() );
}

void ScNameDlg::SetActive()
{
    maEdAssign.GrabFocus();
    RefInputDone();
}

void ScNameDlg::UpdateChecks(ScRangeData* pData)
{
    // remove handlers, we only want the handlers to process
    // user input and not when we are syncing the controls  with our internal
    // model ( also UpdateChecks is called already from some other event
    // handlers, triggering handlers while already processing a handler can
    // ( and does in this case ) corrupt the internal data

    maBtnCriteria.SetToggleHdl( Link() );
    maBtnPrintArea.SetToggleHdl( Link() );
    maBtnColHeader.SetToggleHdl( Link() );
    maBtnRowHeader.SetToggleHdl( Link() );

    maBtnCriteria .Check( pData->HasType( RT_CRITERIA ) );
    maBtnPrintArea.Check( pData->HasType( RT_PRINTAREA ) );
    maBtnColHeader.Check( pData->HasType( RT_COLHEADER ) );
    maBtnRowHeader.Check( pData->HasType( RT_ROWHEADER ) );

    // Restore handlers so user input is processed again
    Link aToggleHandler = LINK( this, ScNameDlg, EdModifyHdl );
    maBtnCriteria.SetToggleHdl( aToggleHandler );
    maBtnPrintArea.SetToggleHdl( aToggleHandler );
    maBtnColHeader.SetToggleHdl( aToggleHandler );
    maBtnRowHeader.SetToggleHdl( aToggleHandler );
}

bool ScNameDlg::IsNameValid()
{
    rtl::OUString aScope = maLbScope.GetSelectEntry();
    rtl::OUString aName = maEdName.GetText();
    aName = aName.trim();

    if (aName.isEmpty())
        return false;

    ScRangeName* pRangeName = GetRangeName( aScope );

    if (!ScRangeData::IsNameValid( aName, mpDoc ))
    {
        maFtInfo.SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        maFtInfo.SetText(maErrInvalidNameStr);
        return false;
    }
    else if (pRangeName && pRangeName->findByUpperName(ScGlobal::pCharClass->uppercase(aName)))
    {
        maFtInfo.SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        maFtInfo.SetText(maErrNameInUse);
        return false;
    }
    maFtInfo.SetText( maStrInfoDefault );
    return true;
}

bool ScNameDlg::IsFormulaValid()
{
    ScCompiler aComp( mpDoc, maCursorPos);
    aComp.SetGrammar( mpDoc->GetGrammar() );
    ScTokenArray* pCode = aComp.CompileString(maEdAssign.GetText());
    if (pCode->GetCodeError())
    {
        maFtInfo.SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        delete pCode;
        return false;
    }
    else
    {
        delete pCode;
        return true;
    }
}

ScRangeName* ScNameDlg::GetRangeName(const rtl::OUString& rScope)
{
    if (rScope == maGlobalNameStr)
        return maRangeMap.find(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(STR_GLOBAL_RANGE_NAME)))->second;
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

void ScNameDlg::SetEntry(const rtl::OUString& rName, const rtl::OUString& rScope)
{
    if (!rName.isEmpty())
    {
        mbDataChanged = true;
        ScRangeNameLine aLine;
        aLine.aName = rName;
        aLine.aScope = rScope;
        mpRangeManagerTable->SetEntry(aLine);
    }
}

void ScNameDlg::RemovePushed()
{
    std::vector<ScRangeNameLine> maEntries = mpRangeManagerTable->GetSelectedEntries();
    mpRangeManagerTable->DeleteSelectedEntries();
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
    mpRangeManagerTable->GetCurrentLine(aLine);
    rtl::OUString aOldName = aLine.aName;
    rtl::OUString aNewName = maEdName.GetText();
    aNewName = aNewName.trim();
    maFtInfo.SetControlBackground(GetSettings().GetStyleSettings().GetDialogColor());
    if (aNewName != aOldName)
    {
        if (!IsNameValid())
            return;
    }
    else
    {
        maFtInfo.SetText( maStrInfoDefault );
    }

    if (!IsFormulaValid())
    {
        //TODO: implement an info text
        return;
    }

    rtl::OUString aOldScope = aLine.aScope;
    //empty table
    if (aOldScope.isEmpty())
        return;
    rtl::OUString aExpr = maEdAssign.GetText();
    rtl::OUString aNewScope = maLbScope.GetSelectEntry();

    ScRangeName* pOldRangeName = GetRangeName( aOldScope );
    ScRangeData* pData = pOldRangeName->findByUpperName( ScGlobal::pCharClass->uppercase(aOldName) );
    ScRangeName* pNewRangeName = GetRangeName( aNewScope );
    OSL_ENSURE(pData, "model and table should be in sync");
    // be safe and check for range data
    if (pData)
    {
        pOldRangeName->erase(*pData);
        mbNeedUpdate = false;
        mpRangeManagerTable->DeleteSelectedEntries();
        RangeType nType = RT_NAME |
            (maBtnRowHeader.IsChecked() ? RT_ROWHEADER : RangeType(0))
            |(maBtnColHeader.IsChecked() ? RT_COLHEADER : RangeType(0))
            |(maBtnPrintArea.IsChecked() ? RT_PRINTAREA : RangeType(0))
            |(maBtnCriteria.IsChecked()  ? RT_CRITERIA  : RangeType(0));

        ScRangeData* pNewEntry = new ScRangeData( mpDoc, aNewName, aExpr,
                maCursorPos, nType);
        pNewRangeName->insert(pNewEntry);
        aLine.aName = aNewName;
        aLine.aExpression = aExpr;
        aLine.aScope = aNewScope;
        mpRangeManagerTable->addEntry(aLine);
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

    if (mpRangeManagerTable->IsMultiSelection())
    {
        maEdName.SetText(maStrMultiSelect);
        maEdAssign.SetText(maStrMultiSelect);

        maEdName.Disable();
        maEdAssign.Disable();
        aRbAssign.Disable();
        maLbScope.Disable();
        maBtnRowHeader.Disable();
        maBtnColHeader.Disable();
        maBtnPrintArea.Disable();
        maBtnCriteria.Disable();
    }
    else
    {
        ScRangeNameLine aLine;
        mpRangeManagerTable->GetCurrentLine(aLine);
        maEdAssign.SetText(aLine.aExpression);
        maEdName.SetText(aLine.aName);
        maLbScope.SelectEntry(aLine.aScope);
        ShowOptions(aLine);
        maBtnDelete.Enable();
        maEdName.Enable();
        maEdAssign.Enable();
        aRbAssign.Enable();
        maLbScope.Enable();
        maBtnRowHeader.Enable();
        maBtnColHeader.Enable();
        maBtnPrintArea.Enable();
        maBtnCriteria.Enable();
    }
}

void ScNameDlg::ScopeChanged()
{
    NameModified();
}

namespace {

void MoveWindow( Window& rButton, long nPixel)
{
    Point aPoint = rButton.GetPosPixel();
    aPoint.Y() += nPixel;
    rButton.SetPosPixel(aPoint);
}

}

void ScNameDlg::MorePushed()
{
    Size nSize = GetSizePixel();

    //depending on the state of the button, move all elements below up/down
    long nPixel = 60;
    if (!maBtnMore.GetState())
    {
        nPixel *= -1;
        maBtnRowHeader.Hide();
        maBtnColHeader.Hide();
        maBtnPrintArea.Hide();
        maBtnCriteria.Hide();
    }
    else
    {
        maBtnRowHeader.Show();
        maBtnColHeader.Show();
        maBtnPrintArea.Show();
        maBtnCriteria.Show();
    }
    nSize.Height() += nPixel;
    SetSizePixel(nSize);
    MoveWindow(maBtnAdd, nPixel);
    MoveWindow(maBtnDelete, nPixel);
    MoveWindow(maBtnHelp, nPixel);
    MoveWindow(maBtnOk, nPixel);
    MoveWindow(maBtnCancel, nPixel);
    MoveWindow(maFlDiv, nPixel);
}

void ScNameDlg::GetRangeNames(boost::ptr_map<rtl::OUString, ScRangeName>& rRangeMap)
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
    EdModifyHdl( &maEdAssign );
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

IMPL_LINK_NOARG(ScNameDlg, MoreBtnHdl)
{
    MorePushed();
    return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
