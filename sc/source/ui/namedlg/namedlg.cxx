/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// System - Includes ---------------------------------------------------------



// INCLUDE -------------------------------------------------------------------

#include "global.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "docfunc.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "namedlg.hrc"
#include "namedlg.hxx"
#include "viewdata.hxx"

#include "globalnames.hxx"

#include "sfx2/app.hxx"

#include <vcl/msgbox.hxx>

#include <map>
#include <memory>
#include <iostream>

// defines -------------------------------------------------------------------

#define ABS_SREF          SCA_VALID \
    | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
    | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_SREF3D      ABS_SREF | SCA_TAB_3D
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

//logic

#define ERRORBOX(s) ErrorBox(this,WinBits(WB_OK|WB_DEF_OK),s).Execute();

ScNameDlg::ScNameDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
        ScViewData*       ptrViewData,
        const ScAddress&  aCursorPos )

:   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_NAMES ),
    //
    maFtScope( this, ScResId(FT_SCOPE) ),
    maFtRange( this, ScResId(FT_RANGE) ),
    maFtName( this,  ScResId(FT_NAME) ),
    maLbScope( this, ScResId(LB_SCOPE) ),
    //
    maEdAssign       ( this, this, ScResId( ED_ASSIGN ) ),
    maEdName         ( this, ScResId( ED_NAME2 ) ),
    aRbAssign       ( this, ScResId( RB_ASSIGN ), &maEdAssign, this ),
    //
    maFlDiv          ( this, ScResId( FL_DIV ) ),
    maBtnPrintArea   ( this, ScResId( BTN_PRINTAREA ) ),
    maBtnColHeader   ( this, ScResId( BTN_COLHEADER ) ),
    maBtnCriteria    ( this, ScResId( BTN_CRITERIA ) ),
    maBtnRowHeader   ( this, ScResId( BTN_ROWHEADER ) ),
    //
    maNameMgrCtrl   ( this, ScResId( CTRL_MANAGENAMES ) ),
    //
    maBtnHelp        ( this, ScResId( BTN_HELP ) ),
    maBtnAdd         ( this, ScResId( BTN_ADD ) ),
    maBtnDelete      ( this, ScResId( BTN_DELETE ) ),
    maBtnSelect      ( this, ScResId( BTN_SELECT ) ),
    maBtnOk          ( this, ScResId( BTN_NAME_OK ) ),
    maBtnCancel      ( this, ScResId( BTN_NAME_CANCEL ) ),
    maBtnMore        ( this, ScResId( BTN_MORE ) ),
    maFtInfo         ( this, ScResId( FT_INFO ) ),
    //
    mErrMsgInvalidSym( ScResId( STR_INVALIDSYMBOL ) ),
    maErrMsgModifiedFailed( ResId::toString(ScResId( STR_MODIFYFAILED ) ) ),
    maGlobalNameStr  ( ResId::toString(ScResId(STR_GLOBAL_SCOPE)) ),
    maErrInvalidNameStr( ResId::toString(ScResId(STR_ERR_NAME_INVALID))),
    maErrNameInUse   ( ResId::toString(ScResId(STR_ERR_NAME_EXISTS))),
    maStrInfoDefault ( ResId::toString(ScResId(STR_DEFAULT_INFO))),
    //
    mpViewData       ( ptrViewData ),
    mpDoc            ( ptrViewData->GetDocument() ),
    maCursorPos      ( aCursorPos ),
    mbNeedUpdate     ( true )
{
    Init();
    FreeResource();
}

ScNameDlg::~ScNameDlg()
{
    delete mpRangeManagerTable;
}

void ScNameDlg::Init()
{
    String  aAreaStr;
    ScRange aRange;

    OSL_ENSURE( mpViewData && mpDoc, "ViewData oder Document nicht gefunden!" );

    //init UI
    std::map<rtl::OUString, ScRangeName*> aRangeMap;
    mpDoc->GetRangeNameMap(aRangeMap);
    std::map<rtl::OUString, ScRangeName*>::iterator itr = aRangeMap.begin(), itrEnd = aRangeMap.end();
    for (; itr != itrEnd; ++itr)
    {
        rtl::OUString aTemp(itr->first);
        maRangeMap.insert(aTemp, new ScRangeName(*itr->second));
        std::cout << "RangeName: " << rtl::OUStringToOString(aTemp, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    }

    mpRangeManagerTable = new ScRangeManagerTable(&maNameMgrCtrl, maRangeMap);
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

    UpdateNames();
    IsNameValid();

    mpViewData->GetSimpleArea( aRange );
    aRange.Format( aAreaStr, ABS_DREF3D, mpDoc,
            ScAddress::Details(mpDoc->GetAddressConvention(), 0, 0) );

    maCurSel = Selection( 0, SELECTION_MAX );
    maEdAssign.GrabFocus();
    maEdAssign.SetText( aAreaStr );
    maEdAssign.SetSelection( maCurSel );

    EdModifyHdl( 0 );

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
    std::cout << "Ok Pushed!" << std::endl;
    ScDocFunc aFunc(*mpViewData->GetDocShell());
    aFunc.ModifyAllRangeNames(maRangeMap);
    return DoClose( ScNameDlgWrapper::GetChildWindowId() );
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
    maBtnCriteria .Check( pData->HasType( RT_CRITERIA ) );
    maBtnPrintArea.Check( pData->HasType( RT_PRINTAREA ) );
    maBtnColHeader.Check( pData->HasType( RT_COLHEADER ) );
    maBtnRowHeader.Check( pData->HasType( RT_ROWHEADER ) );
}

bool ScNameDlg::IsNameValid()
{
    rtl::OUString aScope = maLbScope.GetSelectEntry();
    rtl::OUString aName = maEdName.GetText();
    aName = aName.trim();

    if (!aName.getLength())
        return false;

    ScRangeName* pRangeName = GetRangeName( aScope );

    if (!ScRangeData::IsNameValid( aName, mpDoc ))
    {
        maEdName.SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        maFtInfo.SetText(maErrInvalidNameStr);
        return false;
    }
    else if (pRangeName && pRangeName->findByUpperName(ScGlobal::pCharClass->upper(aName)))
    {
        maEdName.SetControlBackground(GetSettings().GetStyleSettings().GetHighlightColor());
        maFtInfo.SetText(maErrNameInUse);
        return false;
    }
    maEdName.SetControlBackground(GetSettings().GetStyleSettings().GetFieldColor());
    maFtInfo.SetText( maStrInfoDefault );
    return true;
}

bool ScNameDlg::IsFormulaValid()
{
    return true;
}

ScRangeName* ScNameDlg::GetRangeName(const rtl::OUString& rScope)
{
    if (rScope == maGlobalNameStr)
        return maRangeMap.find(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(STR_GLOBAL_RANGE_NAME)))->second;
    else
        return maRangeMap.find(rScope)->second;
}

//updates the table and the buttons
void ScNameDlg::UpdateNames()
{
    ScRangeNameLine aLine;
    mpRangeManagerTable->GetCurrentLine(aLine);
    if (aLine.aName.getLength())
    {
        maBtnDelete.Enable();
    }
    else
    {
        maBtnDelete.Disable();
    }
}

void ScNameDlg::CalcCurTableAssign( String& aAssign, ScRangeData* pRangeData )
{
    if ( pRangeData )
    {
        rtl::OUStringBuffer sBuffer;
        pRangeData->UpdateSymbol( sBuffer, maCursorPos );
        aAssign = sBuffer.makeStringAndClear();
    }
    else
    {
        aAssign.Erase();
    }
}

void ScNameDlg::ShowOptions(const ScRangeNameLine& rLine)
{
    ScRangeName* pRangeName = GetRangeName(rLine.aScope);
    ScRangeData* pData = pRangeName->findByName(rLine.aName);
    if (pData)
    {
        UpdateChecks(pData);
    }
}


bool ScNameDlg::AddPushed()
{
    if (!mpDoc)
        return false;

    rtl::OUString aName = maEdName.GetText();
    aName = aName.trim();
    if (!aName.getLength())
        return false;

    if (!ScRangeData::IsNameValid( aName, mpDoc ))
    {
        ERRORBOX( ScGlobal::GetRscString(STR_INVALIDNAME));
        return false;
    }

    rtl::OUString aScope = maLbScope.GetSelectEntry();
    rtl::OUString aExpr = maEdAssign.GetText();
    ScRangeName* pRangeName = GetRangeName(aScope);

    RangeType nType = RT_NAME |
         (maBtnRowHeader.IsChecked() ? RT_ROWHEADER : RangeType(0))
        |(maBtnColHeader.IsChecked() ? RT_COLHEADER : RangeType(0))
        |(maBtnPrintArea.IsChecked() ? RT_PRINTAREA : RangeType(0))
        |(maBtnCriteria.IsChecked()  ? RT_CRITERIA  : RangeType(0));

    ScRangeData* pNewEntry = new ScRangeData( mpDoc, aName, aExpr,
                                            maCursorPos, nType);
    if ( 0 == pNewEntry->GetErrCode() )
    {
        if (!pRangeName->insert( pNewEntry))
        {
            pNewEntry = NULL;
        }
        else
        {
            maEdName.SetText(EMPTY_STRING);
            maBtnAdd.Disable();
            UpdateNames();
            SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
        }
    }
    else
    {
        delete pNewEntry;
        ERRORBOX( mErrMsgInvalidSym );
    }
    return true;
}

void ScNameDlg::RemovePushed()
{
    std::vector<ScRangeNameLine> maEntries = mpRangeManagerTable->GetSelectedEntries();
    mpRangeManagerTable->DeleteSelectedEntries();
    for (std::vector<ScRangeNameLine>::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        ScRangeName* pRangeName = GetRangeName(itr->aScope);
        std::cout << rtl::OUStringToOString(itr->aName, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
        std::cout << rtl::OUStringToOString(ScGlobal::pCharClass->upper(itr->aName), RTL_TEXTENCODING_UTF8).getStr() << std::endl;;
        ScRangeData* pData = pRangeName->findByUpperName(ScGlobal::pCharClass->upper(itr->aName));
        OSL_ENSURE(pData, "table and model should be in sync");
        // be safe and check for possible problems
        if (pData)
            pRangeName->erase(*pData);
    }
}

void ScNameDlg::NameModified()
{
    if (!IsFormulaValid())
        return;
    ScRangeNameLine aLine;
    mpRangeManagerTable->GetCurrentLine(aLine);
    rtl::OUString aOldName = aLine.aName;
    rtl::OUString aNewName = maEdName.GetText();
    aNewName = aNewName.trim();
    if (aNewName != aOldName)
    {
        if (!IsNameValid())
            return;
    }

    rtl::OUString aOldScope = aLine.aScope;
    //empty table
    if (aOldScope.isEmpty())
        return;
    rtl::OUString aExpr = maEdAssign.GetText();
    rtl::OUString aNewScope = maLbScope.GetSelectEntry();

    ScRangeName* pOldRangeName = GetRangeName( aOldScope );
    ScRangeData* pData = pOldRangeName->findByUpperName( ScGlobal::pCharClass->upper(aOldName) );
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
        maEdName.Disable();
        maEdAssign.Disable();
        maLbScope.Disable();
        maBtnSelect.Disable();
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
        maLbScope.Enable();
        maBtnSelect.Enable();
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
    long nPixel = 85;
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
    MoveWindow(maBtnSelect, nPixel);
    MoveWindow(maFlDiv, nPixel);
}

IMPL_LINK( ScNameDlg, OkBtnHdl, void *, EMPTYARG )
{
    Close();
    return 0;
}

IMPL_LINK( ScNameDlg, CancelBtnHdl, void *, EMPTYARG )
{
    CancelPushed();
    return 0;
}

IMPL_LINK( ScNameDlg, AddBtnHdl, void *, EMPTYARG )
{
    return AddPushed();
}

IMPL_LINK( ScNameDlg, RemoveBtnHdl, void *, EMPTYARG )
{
    RemovePushed();
    return 0;
}

IMPL_LINK( ScNameDlg, EdModifyHdl, void *, EMPTYARG )
{
    NameModified();
    return 0;
}

IMPL_LINK( ScNameDlg, AssignGetFocusHdl, void *, EMPTYARG )
{
    EdModifyHdl( &maEdAssign );
    return 0;
}

IMPL_LINK( ScNameDlg, SelectionChangedHdl_Impl, void *, EMPTYARG )
{
    SelectionChanged();
    return 0;
}

IMPL_LINK( ScNameDlg, ScopeChangedHdl, void*, EMPTYARG )
{
    ScopeChanged();
    return 0;
}

IMPL_LINK( ScNameDlg, MoreBtnHdl, void*, EMPTYARG )
{
    MorePushed();
    return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
