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

#include "rangelst.hxx"
#include "scitems.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/imagemgr.hxx>
#include <svl/zforlist.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include "uiitems.hxx"
#include "reffact.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "formulacell.hxx"
#include "rangeutl.hxx"
#include "scresid.hxx"
#include "convuno.hxx"
#include "unonames.hxx"
#include "solveroptions.hxx"
#include "solverutil.hxx"
#include "globstr.hrc"

#include "optsolver.hxx"

#include <com/sun/star/sheet/Solver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>

using namespace com::sun::star;

ScSolverProgressDialog::ScSolverProgressDialog(vcl::Window* pParent)
    : ModelessDialog(pParent, "SolverProgressDialog",
        "modules/scalc/ui/solverprogressdialog.ui")
{
    get(m_pFtTime, "progress");
}

ScSolverProgressDialog::~ScSolverProgressDialog()
{
    disposeOnce();
}

void ScSolverProgressDialog::dispose()
{
    m_pFtTime.clear();
    ModelessDialog::dispose();
}


void ScSolverProgressDialog::HideTimeLimit()
{
    m_pFtTime->Hide();
}

void ScSolverProgressDialog::SetTimeLimit( sal_Int32 nSeconds )
{
    OUString aOld = m_pFtTime->GetText();
    OUString aNew = aOld.replaceFirst("#", OUString::number(nSeconds));
    m_pFtTime->SetText( aNew );
}

ScSolverNoSolutionDialog::ScSolverNoSolutionDialog( vcl::Window* pParent, const OUString& rErrorText )
    : ModalDialog(pParent, "NoSolutionDialog", "modules/scalc/ui/nosolutiondialog.ui")
{
    get(m_pFtErrorText, "error");
    m_pFtErrorText->SetText(rErrorText);
}

ScSolverNoSolutionDialog::~ScSolverNoSolutionDialog()
{
    disposeOnce();
}

void ScSolverNoSolutionDialog::dispose()
{
    m_pFtErrorText.clear();
    ModalDialog::dispose();
}

ScSolverSuccessDialog::ScSolverSuccessDialog( vcl::Window* pParent, const OUString& rSolution )
    : ModalDialog(pParent, "SolverSuccessDialog", "modules/scalc/ui/solversuccessdialog.ui")
{
    get(m_pFtResult, "result");
    get(m_pBtnOk, "ok");
    m_pBtnOk->SetClickHdl(LINK(this, ScSolverSuccessDialog, ClickHdl));
    get(m_pBtnCancel, "cancel");
    m_pBtnCancel->SetClickHdl(LINK(this, ScSolverSuccessDialog, ClickHdl));
    OUString aMessage = m_pFtResult->GetText() + " " + rSolution;
    m_pFtResult->SetText(aMessage);
}

ScSolverSuccessDialog::~ScSolverSuccessDialog()
{
    disposeOnce();
}

void ScSolverSuccessDialog::dispose()
{
    m_pFtResult.clear();
    m_pBtnOk.clear();
    m_pBtnCancel.clear();
    ModalDialog::dispose();
}

IMPL_LINK_TYPED( ScSolverSuccessDialog, ClickHdl, Button*, pBtn, void )
{
    if (pBtn == m_pBtnOk)
        EndDialog(RET_OK);
    else
        EndDialog();
}

ScCursorRefEdit::ScCursorRefEdit( vcl::Window* pParent, vcl::Window *pLabel )
    : formula::RefEdit( pParent, pLabel )
{
}

VCL_BUILDER_FACTORY_ARGS(ScCursorRefEdit, nullptr)

void ScCursorRefEdit::SetCursorLinks( const Link<ScCursorRefEdit&,void>& rUp, const Link<ScCursorRefEdit&,void>& rDown )
{
    maCursorUpLink = rUp;
    maCursorDownLink = rDown;
}

void ScCursorRefEdit::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    bool bUp = (aCode.GetCode() == KEY_UP);
    bool bDown = (aCode.GetCode() == KEY_DOWN);
    if ( !aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() && ( bUp || bDown ) )
    {
        if ( bUp )
            maCursorUpLink.Call( *this );
        else
            maCursorDownLink.Call( *this );
    }
    else
        formula::RefEdit::KeyInput( rKEvt );
}

ScOptSolverSave::ScOptSolverSave( const OUString& rObjective, bool bMax, bool bMin, bool bValue,
                             const OUString& rTarget, const OUString& rVariable,
                             const std::vector<ScOptConditionRow>& rConditions,
                             const OUString& rEngine,
                             const uno::Sequence<beans::PropertyValue>& rProperties ) :
    maObjective( rObjective ),
    mbMax( bMax ),
    mbMin( bMin ),
    mbValue( bValue ),
    maTarget( rTarget ),
    maVariable( rVariable ),
    maConditions( rConditions ),
    maEngine( rEngine ),
    maProperties( rProperties )
{
}

//  class ScOptSolverDlg

ScOptSolverDlg::ScOptSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                          ScDocShell* pDocSh, ScAddress aCursorPos )

    : ScAnyRefDlg(pB, pCW, pParent, "SolverDialog", "modules/scalc/ui/solverdlg.ui")
    , maInputError(ScGlobal::GetRscString(STR_INVALIDINPUT))
    , maConditionError(ScGlobal::GetRscString(STR_INVALIDCONDITION))

    , mpDocShell(pDocSh)
    , mrDoc(pDocSh->GetDocument())
    , mnCurTab(aCursorPos.Tab())
    , mpEdActive(nullptr)
    , mbDlgLostFocus(false)
    , nScrollPos(0)
{
    get(m_pFtObjectiveCell, "targetlabel");
    get(m_pEdObjectiveCell, "targetedit");
    m_pEdObjectiveCell->SetReferences(this, m_pFtObjectiveCell);
    get(m_pRBObjectiveCell, "targetbutton");
    m_pRBObjectiveCell->SetReferences(this, m_pEdObjectiveCell);
    get(m_pRbMax, "max");
    get(m_pRbMin, "min");
    get(m_pRbValue, "value");
    get(m_pEdTargetValue, "valueedit");
    m_pEdTargetValue->SetReferences(this, get<FixedText>("result"));
    get(m_pRBTargetValue, "valuebutton");
    m_pRBTargetValue->SetReferences(this, m_pEdTargetValue);
    get(m_pFtVariableCells, "changelabel");
    get(m_pEdVariableCells, "changeedit");
    m_pEdVariableCells->SetReferences(this, m_pFtVariableCells);
    get(m_pRBVariableCells, "changebutton");
    m_pRBVariableCells->SetReferences(this, m_pEdVariableCells);
    get(m_pFtCellRef, "cellreflabel");
    get(m_pEdLeft1, "ref1edit");
    m_pEdLeft1->SetReferences(this, m_pFtCellRef);
    get(m_pRBLeft1, "ref1button");
    m_pRBLeft1->SetReferences(this, m_pEdLeft1);
    get(m_pFtOperator, "oplabel");
    get(m_pLbOp1, "op1list");
    get(m_pFtConstraint, "constraintlabel");
    get(m_pEdRight1, "val1edit");
    m_pEdRight1->SetReferences(this, m_pFtConstraint);
    get(m_pRBRight1, "val1button");
    m_pRBRight1->SetReferences(this, m_pEdRight1);
    get(m_pBtnDel1, "del1");
    get(m_pEdLeft2, "ref2edit");
    m_pEdLeft2->SetReferences(this, m_pFtCellRef);
    get(m_pRBLeft2, "ref2button");
    m_pRBLeft2->SetReferences(this, m_pEdLeft2);
    get(m_pLbOp2, "op2list");
    get(m_pEdRight2, "val2edit");
    m_pEdRight2->SetReferences(this, m_pFtConstraint);
    get(m_pRBRight2, "val2button");
    m_pRBRight2->SetReferences(this, m_pEdRight2);
    get(m_pBtnDel2, "del2");
    get(m_pEdLeft3, "ref3edit");
    m_pEdLeft3->SetReferences(this, m_pFtCellRef);
    get(m_pRBLeft3, "ref3button");
    m_pRBLeft3->SetReferences(this, m_pEdLeft3);
    get(m_pLbOp3, "op3list");
    get(m_pEdRight3, "val3edit");
    m_pEdRight3->SetReferences(this, m_pFtConstraint);
    get(m_pRBRight3, "val3button");
    m_pRBRight3->SetReferences(this, m_pEdRight3);
    get(m_pBtnDel3, "del3");
    get(m_pEdLeft4, "ref4edit");
    m_pEdLeft4->SetReferences(this, m_pFtCellRef);
    get(m_pRBLeft4, "ref4button");
    m_pRBLeft4->SetReferences(this, m_pEdLeft4);
    get(m_pLbOp4, "op4list");
    get(m_pEdRight4, "val4edit");
    m_pEdRight4->SetReferences(this, m_pFtConstraint);
    get(m_pRBRight4, "val4button");
    m_pRBRight4->SetReferences(this, m_pEdRight4);
    get(m_pBtnDel4, "del4");
    get(m_pScrollBar, "scrollbar");
    get(m_pBtnOpt, "options");
    get(m_pBtnCancel, "close");
    get(m_pBtnSolve, "solve");

    mpLeftEdit[0]    = m_pEdLeft1;
    mpLeftButton[0]  = m_pRBLeft1;
    mpRightEdit[0]   = m_pEdRight1;
    mpRightButton[0] = m_pRBRight1;
    mpOperator[0]    = m_pLbOp1;
    mpDelButton[0]   = m_pBtnDel1;

    mpLeftEdit[1]    = m_pEdLeft2;
    mpLeftButton[1]  = m_pRBLeft2;
    mpRightEdit[1]   = m_pEdRight2;
    mpRightButton[1] = m_pRBRight2;
    mpOperator[1]    = m_pLbOp2;
    mpDelButton[1]   = m_pBtnDel2;

    mpLeftEdit[2]    = m_pEdLeft3;
    mpLeftButton[2]  = m_pRBLeft3;
    mpRightEdit[2]   = m_pEdRight3;
    mpRightButton[2] = m_pRBRight3;
    mpOperator[2]    = m_pLbOp3;
    mpDelButton[2]   = m_pBtnDel3;

    mpLeftEdit[3]    = m_pEdLeft4;
    mpLeftButton[3]  = m_pRBLeft4;
    mpRightEdit[3]   = m_pEdRight4;
    mpRightButton[3] = m_pRBRight4;
    mpOperator[3]    = m_pLbOp4;
    mpDelButton[3]   = m_pBtnDel4;

    m_pEdLeft2->SetAccessibleName(m_pFtCellRef->GetText());
    m_pLbOp2->SetAccessibleName(m_pFtOperator->GetText());
    m_pEdRight2->SetAccessibleName(m_pFtConstraint->GetText());
    m_pEdLeft3->SetAccessibleName(m_pFtCellRef->GetText());
    m_pLbOp3->SetAccessibleName(m_pFtOperator->GetText());
    m_pEdRight3->SetAccessibleName(m_pFtConstraint->GetText());
    m_pEdLeft4->SetAccessibleName(m_pFtCellRef->GetText());
    m_pLbOp4->SetAccessibleName(m_pFtOperator->GetText());
    m_pEdRight4->SetAccessibleName(m_pFtConstraint->GetText());

    Init( aCursorPos );
}

ScOptSolverDlg::~ScOptSolverDlg()
{
    disposeOnce();
}

void ScOptSolverDlg::dispose()
{
    m_pFtObjectiveCell.clear();
    m_pEdObjectiveCell.clear();
    m_pRBObjectiveCell.clear();
    m_pRbMax.clear();
    m_pRbMin.clear();
    m_pRbValue.clear();
    m_pEdTargetValue.clear();
    m_pRBTargetValue.clear();
    m_pFtVariableCells.clear();
    m_pEdVariableCells.clear();
    m_pRBVariableCells.clear();
    m_pFtCellRef.clear();
    m_pEdLeft1.clear();
    m_pRBLeft1.clear();
    m_pFtOperator.clear();
    m_pLbOp1.clear();
    m_pFtConstraint.clear();
    m_pEdRight1.clear();
    m_pRBRight1.clear();
    m_pBtnDel1.clear();
    m_pEdLeft2.clear();
    m_pRBLeft2.clear();
    m_pLbOp2.clear();
    m_pEdRight2.clear();
    m_pRBRight2.clear();
    m_pBtnDel2.clear();
    m_pEdLeft3.clear();
    m_pRBLeft3.clear();
    m_pLbOp3.clear();
    m_pEdRight3.clear();
    m_pRBRight3.clear();
    m_pBtnDel3.clear();
    m_pEdLeft4.clear();
    m_pRBLeft4.clear();
    m_pLbOp4.clear();
    m_pEdRight4.clear();
    m_pRBRight4.clear();
    m_pBtnDel4.clear();
    m_pScrollBar.clear();
    m_pBtnOpt.clear();
    m_pBtnCancel.clear();
    m_pBtnSolve.clear();
    mpEdActive.clear();
    for (auto& p : mpLeftButton)
        p.clear();
    for (auto& p : mpRightButton)
        p.clear();
    for (auto& p : mpOperator)
        p.clear();
    for (auto& p : mpDelButton)
        p.clear();
    ScAnyRefDlg::dispose();
}

void ScOptSolverDlg::Init(const ScAddress& rCursorPos)
{
    // Get the "Delete Rows" commandimagelist images from sfx instead of
    // adding a second copy to sc (see ScTbxInsertCtrl::StateChanged)

    OUString aSlotURL( "slot:" );
    aSlotURL += OUString::number( SID_DEL_ROWS );
    uno::Reference<frame::XFrame> xFrame = GetBindings().GetActiveFrame();
    Image aDelNm = ::GetImage( xFrame, aSlotURL, false );

    for (VclPtr<PushButton> & pButton : mpDelButton)
    {
        pButton->SetModeImage( aDelNm );
    }

    m_pBtnOpt->SetClickHdl( LINK( this, ScOptSolverDlg, BtnHdl ) );
    m_pBtnCancel->SetClickHdl( LINK( this, ScOptSolverDlg, BtnHdl ) );
    m_pBtnSolve->SetClickHdl( LINK( this, ScOptSolverDlg, BtnHdl ) );

    Link<Control&,void> aLink = LINK( this, ScOptSolverDlg, GetFocusHdl );
    m_pEdObjectiveCell->SetGetFocusHdl( aLink );
    m_pRBObjectiveCell->SetGetFocusHdl( aLink );
    m_pEdTargetValue->SetGetFocusHdl( aLink );
    m_pRBTargetValue->SetGetFocusHdl( aLink );
    m_pEdVariableCells->SetGetFocusHdl( aLink );
    m_pRBVariableCells->SetGetFocusHdl( aLink );
    m_pRbValue->SetGetFocusHdl( aLink );
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        mpLeftEdit[nRow]->SetGetFocusHdl( aLink );
        mpLeftButton[nRow]->SetGetFocusHdl( aLink );
        mpRightEdit[nRow]->SetGetFocusHdl( aLink );
        mpRightButton[nRow]->SetGetFocusHdl( aLink );
        mpOperator[nRow]->SetGetFocusHdl( aLink );
    }

    aLink = LINK( this, ScOptSolverDlg, LoseFocusHdl );
    m_pEdObjectiveCell->SetLoseFocusHdl( aLink );
    m_pRBObjectiveCell->SetLoseFocusHdl( aLink );
    m_pEdTargetValue->SetLoseFocusHdl( aLink );
    m_pRBTargetValue-> SetLoseFocusHdl( aLink );
    m_pEdVariableCells->SetLoseFocusHdl( aLink );
    m_pRBVariableCells->SetLoseFocusHdl( aLink );
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        mpLeftEdit[nRow]->SetLoseFocusHdl( aLink );
        mpLeftButton[nRow]->SetLoseFocusHdl( aLink );
        mpRightEdit[nRow]->SetLoseFocusHdl( aLink );
        mpRightButton[nRow]->SetLoseFocusHdl( aLink );
    }

    Link<ScCursorRefEdit&,void> aCursorUp = LINK( this, ScOptSolverDlg, CursorUpHdl );
    Link<ScCursorRefEdit&,void> aCursorDown = LINK( this, ScOptSolverDlg, CursorDownHdl );
    Link<Edit&,void> aCondModify = LINK( this, ScOptSolverDlg, CondModifyHdl );
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        mpLeftEdit[nRow]->SetCursorLinks( aCursorUp, aCursorDown );
        mpRightEdit[nRow]->SetCursorLinks( aCursorUp, aCursorDown );
        mpLeftEdit[nRow]->SetModifyHdl( aCondModify );
        mpRightEdit[nRow]->SetModifyHdl( aCondModify );
        mpDelButton[nRow]->SetClickHdl( LINK( this, ScOptSolverDlg, DelBtnHdl ) );
        mpOperator[nRow]->SetSelectHdl( LINK( this, ScOptSolverDlg, SelectHdl ) );
    }
    m_pEdTargetValue->SetModifyHdl( LINK( this, ScOptSolverDlg, TargetModifyHdl ) );

    m_pScrollBar->SetEndScrollHdl( LINK( this, ScOptSolverDlg, ScrollHdl ) );
    m_pScrollBar->SetScrollHdl( LINK( this, ScOptSolverDlg, ScrollHdl ) );

    m_pScrollBar->SetPageSize( EDIT_ROW_COUNT );
    m_pScrollBar->SetVisibleSize( EDIT_ROW_COUNT );
    m_pScrollBar->SetLineSize( 1 );
    // Range is set in ShowConditions

    // get available solver implementations
    //! sort by descriptions?
    ScSolverUtil::GetImplementations( maImplNames, maDescriptions );
    sal_Int32 nImplCount = maImplNames.getLength();

    const ScOptSolverSave* pOldData = mpDocShell->GetSolverSaveData();
    if ( pOldData )
    {
        m_pEdObjectiveCell->SetRefString( pOldData->GetObjective() );
        m_pRbMax->Check( pOldData->GetMax() );
        m_pRbMin->Check( pOldData->GetMin() );
        m_pRbValue->Check( pOldData->GetValue() );
        m_pEdTargetValue->SetRefString( pOldData->GetTarget() );
        m_pEdVariableCells->SetRefString( pOldData->GetVariable() );
        maConditions = pOldData->GetConditions();
        maEngine = pOldData->GetEngine();
        maProperties = pOldData->GetProperties();
    }
    else
    {
        m_pRbMax->Check();
        OUString aCursorStr;
        if ( !mrDoc.GetRangeAtBlock( ScRange(rCursorPos), &aCursorStr ) )
            aCursorStr = rCursorPos.Format(ScRefFlags::ADDR_ABS, nullptr, mrDoc.GetAddressConvention());
        m_pEdObjectiveCell->SetRefString( aCursorStr );
        if ( nImplCount > 0 )
            maEngine = maImplNames[0];  // use first implementation
    }
    ShowConditions();

    m_pEdObjectiveCell->GrabFocus();
    mpEdActive = m_pEdObjectiveCell;
}

void ScOptSolverDlg::ReadConditions()
{
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        ScOptConditionRow aRowEntry;
        aRowEntry.aLeftStr = mpLeftEdit[nRow]->GetText();
        aRowEntry.aRightStr = mpRightEdit[nRow]->GetText();
        aRowEntry.nOperator = mpOperator[nRow]->GetSelectEntryPos();

        long nVecPos = nScrollPos + nRow;
        if ( nVecPos >= (long)maConditions.size() && !aRowEntry.IsDefault() )
            maConditions.resize( nVecPos + 1 );

        if ( nVecPos < (long)maConditions.size() )
            maConditions[nVecPos] = aRowEntry;

        // remove default entries at the end
        size_t nSize = maConditions.size();
        while ( nSize > 0 && maConditions[ nSize-1 ].IsDefault() )
            --nSize;
        maConditions.resize( nSize );
    }
}

void ScOptSolverDlg::ShowConditions()
{
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        ScOptConditionRow aRowEntry;

        long nVecPos = nScrollPos + nRow;
        if ( nVecPos < (long)maConditions.size() )
            aRowEntry = maConditions[nVecPos];

        mpLeftEdit[nRow]->SetRefString( aRowEntry.aLeftStr );
        mpRightEdit[nRow]->SetRefString( aRowEntry.aRightStr );
        mpOperator[nRow]->SelectEntryPos( aRowEntry.nOperator );
    }

    // allow to scroll one page behind the visible or stored rows
    long nVisible = nScrollPos + EDIT_ROW_COUNT;
    long nMax = std::max( nVisible, (long) maConditions.size() );
    m_pScrollBar->SetRange( Range( 0, nMax + EDIT_ROW_COUNT ) );
    m_pScrollBar->SetThumbPos( nScrollPos );

    EnableButtons();
}

void ScOptSolverDlg::EnableButtons()
{
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        long nVecPos = nScrollPos + nRow;
        mpDelButton[nRow]->Enable( nVecPos < (long)maConditions.size() );
    }
}

bool ScOptSolverDlg::Close()
{
    return DoClose( ScOptSolverDlgWrapper::GetChildWindowId() );
}

void ScOptSolverDlg::SetActive()
{
    if ( mbDlgLostFocus )
    {
        mbDlgLostFocus = false;
        if( mpEdActive )
            mpEdActive->GrabFocus();
    }
    else
    {
        GrabFocus();
    }
    RefInputDone();
}

void ScOptSolverDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if( mpEdActive )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(mpEdActive);

        // "target"/"value": single cell
        bool bSingle = ( mpEdActive == m_pEdObjectiveCell || mpEdActive == m_pEdTargetValue );

        OUString aStr;
        ScAddress aAdr = rRef.aStart;
        ScRange aNewRef( rRef );
        if ( bSingle )
            aNewRef.aEnd = aAdr;

        OUString aName;
        if ( pDocP->GetRangeAtBlock( aNewRef, &aName ) )            // named range: show name
            aStr = aName;
        else                                                        // format cell/range reference
        {
            ScRefFlags nFmt = ( aAdr.Tab() == mnCurTab ) ? ScRefFlags::ADDR_ABS : ScRefFlags::ADDR_ABS_3D;
            if ( bSingle )
                aStr = aAdr.Format(nFmt, pDocP, pDocP->GetAddressConvention());
            else
                aStr = rRef.Format(nFmt | ScRefFlags::RANGE_ABS, pDocP, pDocP->GetAddressConvention());
        }

        // variable cells can be several ranges, so only the selection is replaced
        if ( mpEdActive == m_pEdVariableCells )
        {
            OUString aVal = mpEdActive->GetText();
            Selection aSel = mpEdActive->GetSelection();
            aSel.Justify();
            aVal = aVal.replaceAt( aSel.Min(), aSel.Len(), aStr );
            Selection aNewSel( aSel.Min(), aSel.Min()+aStr.getLength() );
            mpEdActive->SetRefString( aVal );
            mpEdActive->SetSelection( aNewSel );
        }
        else
            mpEdActive->SetRefString( aStr );

        ReadConditions();
        EnableButtons();

        // select "Value of" if a ref is input into "target" edit
        if ( mpEdActive == m_pEdTargetValue )
            m_pRbValue->Check();
    }
}

bool ScOptSolverDlg::IsRefInputMode() const
{
    return mpEdActive != nullptr;
}

// Handler:

IMPL_LINK_TYPED( ScOptSolverDlg, BtnHdl, Button*, pBtn, void )
{
    if ( pBtn == m_pBtnSolve || pBtn == m_pBtnCancel )
    {
        bool bSolve = ( pBtn == m_pBtnSolve );

        SetDispatcherLock( false );
        SwitchToDocument();

        bool bClose = true;
        if ( bSolve )
            bClose = CallSolver();

        if ( bClose )
        {
            // Close: write dialog settings to DocShell for subsequent calls
            ReadConditions();
            ScOptSolverSave aSave(
                m_pEdObjectiveCell->GetText(), m_pRbMax->IsChecked(), m_pRbMin->IsChecked(), m_pRbValue->IsChecked(),
                m_pEdTargetValue->GetText(), m_pEdVariableCells->GetText(), maConditions, maEngine, maProperties );
            mpDocShell->SetSolverSaveData( aSave );
            Close();
        }
        else
        {
            // no solution -> dialog is kept open
            SetDispatcherLock( true );
        }
    }
    else if ( pBtn == m_pBtnOpt )
    {
        //! move options dialog to UI lib?
        ScopedVclPtr<ScSolverOptionsDialog> pOptDlg(
            VclPtr<ScSolverOptionsDialog>::Create( this, maImplNames, maDescriptions, maEngine, maProperties ));
        if ( pOptDlg->Execute() == RET_OK )
        {
            maEngine = pOptDlg->GetEngine();
            maProperties = pOptDlg->GetProperties();
        }
    }
}

IMPL_LINK_TYPED( ScOptSolverDlg, GetFocusHdl, Control&, rCtrl, void )
{
    Edit* pEdit = nullptr;
    mpEdActive = nullptr;

    if( &rCtrl == m_pEdObjectiveCell || &rCtrl == m_pRBObjectiveCell )
        pEdit = mpEdActive = m_pEdObjectiveCell;
    else if( &rCtrl == m_pEdTargetValue || &rCtrl == m_pRBTargetValue )
        pEdit = mpEdActive = m_pEdTargetValue;
    else if( &rCtrl == m_pEdVariableCells || &rCtrl == m_pRBVariableCells )
        pEdit = mpEdActive = m_pEdVariableCells;
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        if( &rCtrl == mpLeftEdit[nRow] || &rCtrl == mpLeftButton[nRow] )
            pEdit = mpEdActive = mpLeftEdit[nRow].get();
        else if( &rCtrl == mpRightEdit[nRow] || &rCtrl == mpRightButton[nRow] )
            pEdit = mpEdActive = mpRightEdit[nRow].get();
        else if( &rCtrl == mpOperator[nRow] )    // focus on "operator" list box
            mpEdActive = mpRightEdit[nRow].get();     // use right edit for ref input, but don't change selection
    }
    if( &rCtrl == m_pRbValue )                   // focus on "Value of" radio button
        mpEdActive = m_pEdTargetValue;          // use value edit for ref input, but don't change selection

    if( pEdit )
        pEdit->SetSelection( Selection( 0, SELECTION_MAX ) );
}

IMPL_LINK_NOARG_TYPED(ScOptSolverDlg, LoseFocusHdl, Control&, void)
{
    mbDlgLostFocus = !IsActive();
}

IMPL_LINK_TYPED( ScOptSolverDlg, DelBtnHdl, Button*, pBtn, void )
{
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
        if( pBtn == mpDelButton[nRow] )
        {
            bool bHadFocus = pBtn->HasFocus();

            ReadConditions();
            long nVecPos = nScrollPos + nRow;
            if ( nVecPos < (long)maConditions.size() )
            {
                maConditions.erase( maConditions.begin() + nVecPos );
                ShowConditions();

                if ( bHadFocus && !pBtn->IsEnabled() )
                {
                    // If the button is disabled, focus would normally move to the next control,
                    // (left edit of the next row). Move it to left edit of this row instead.

                    mpEdActive = mpLeftEdit[nRow].get();
                    mpEdActive->GrabFocus();
                }
            }
        }
}

IMPL_LINK_NOARG_TYPED(ScOptSolverDlg, TargetModifyHdl, Edit&, void)
{
    // modify handler for the target edit:
    //  select "Value of" if something is input into the edit
    if ( !m_pEdTargetValue->GetText().isEmpty() )
        m_pRbValue->Check();
}

IMPL_LINK_NOARG_TYPED(ScOptSolverDlg, CondModifyHdl, Edit&, void)
{
    // modify handler for the condition edits, just to enable/disable "delete" buttons
    ReadConditions();
    EnableButtons();
}

IMPL_LINK_NOARG_TYPED(ScOptSolverDlg, SelectHdl, ListBox&, void)
{
    // select handler for operator list boxes, just to enable/disable "delete" buttons
    ReadConditions();
    EnableButtons();
}

IMPL_LINK_NOARG_TYPED(ScOptSolverDlg, ScrollHdl, ScrollBar*, void)
{
    ReadConditions();
    nScrollPos = m_pScrollBar->GetThumbPos();
    ShowConditions();
    if( mpEdActive )
        mpEdActive->SetSelection( Selection( 0, SELECTION_MAX ) );
}

IMPL_LINK_TYPED( ScOptSolverDlg, CursorUpHdl, ScCursorRefEdit&, rEdit, void )
{
    if ( &rEdit == mpLeftEdit[0] || &rEdit == mpRightEdit[0] )
    {
        if ( nScrollPos > 0 )
        {
            ReadConditions();
            --nScrollPos;
            ShowConditions();
            if( mpEdActive )
                mpEdActive->SetSelection( Selection( 0, SELECTION_MAX ) );
        }
    }
    else
    {
        formula::RefEdit* pFocus = nullptr;
        for ( sal_uInt16 nRow = 1; nRow < EDIT_ROW_COUNT; ++nRow )      // second row or below: move focus
        {
            if ( &rEdit == mpLeftEdit[nRow] )
                pFocus = mpLeftEdit[nRow-1];
            else if ( &rEdit == mpRightEdit[nRow] )
                pFocus = mpRightEdit[nRow-1];
        }
        if (pFocus)
        {
            mpEdActive = pFocus;
            pFocus->GrabFocus();
        }
    }
}

IMPL_LINK_TYPED( ScOptSolverDlg, CursorDownHdl, ScCursorRefEdit&, rEdit, void )
{
    if ( &rEdit == mpLeftEdit[EDIT_ROW_COUNT-1] || &rEdit == mpRightEdit[EDIT_ROW_COUNT-1] )
    {
        //! limit scroll position?
        ReadConditions();
        ++nScrollPos;
        ShowConditions();
        if( mpEdActive )
            mpEdActive->SetSelection( Selection( 0, SELECTION_MAX ) );
    }
    else
    {
        formula::RefEdit* pFocus = nullptr;
        for ( sal_uInt16 nRow = 0; nRow+1 < EDIT_ROW_COUNT; ++nRow )      // before last row: move focus
        {
            if ( &rEdit == mpLeftEdit[nRow] )
                pFocus = mpLeftEdit[nRow+1];
            else if ( &rEdit == mpRightEdit[nRow] )
                pFocus = mpRightEdit[nRow+1];
        }
        if (pFocus)
        {
            mpEdActive = pFocus;
            pFocus->GrabFocus();
        }
    }
}

void ScOptSolverDlg::ShowError( bool bCondition, formula::RefEdit* pFocus )
{
    OUString aMessage = bCondition ? maConditionError : maInputError;
    ScopedVclPtrInstance<MessageDialog>(this, aMessage)->Execute();
    if (pFocus)
    {
        mpEdActive = pFocus;
        pFocus->GrabFocus();
    }
}

bool ScOptSolverDlg::ParseRef( ScRange& rRange, const OUString& rInput, bool bAllowRange )
{
    ScRangeUtil aRangeUtil;
    ScAddress::Details aDetails(mrDoc.GetAddressConvention(), 0, 0);
    ScRefFlags nFlags = rRange.ParseAny( rInput, &mrDoc, aDetails );
    if ( nFlags & ScRefFlags::VALID )
    {
        if ( (nFlags & ScRefFlags::TAB_3D) == ScRefFlags::ZERO)
            rRange.aStart.SetTab( mnCurTab );
        if ( (nFlags & ScRefFlags::TAB2_3D) == ScRefFlags::ZERO)
            rRange.aEnd.SetTab( rRange.aStart.Tab() );
        return ( bAllowRange || rRange.aStart == rRange.aEnd );
    }
    else if ( ScRangeUtil::MakeRangeFromName( rInput, &mrDoc, mnCurTab, rRange, RUTL_NAMES, aDetails ) )
        return ( bAllowRange || rRange.aStart == rRange.aEnd );

    return false;   // not recognized
}

bool ScOptSolverDlg::FindTimeout( sal_Int32& rTimeout )
{
    bool bFound = false;

    if ( !maProperties.getLength() )
        maProperties = ScSolverUtil::GetDefaults( maEngine );   // get property defaults from component

    sal_Int32 nPropCount = maProperties.getLength();
    for (sal_Int32 nProp=0; nProp<nPropCount && !bFound; ++nProp)
    {
        const beans::PropertyValue& rValue = maProperties[nProp];
        if ( rValue.Name == SC_UNONAME_TIMEOUT )
            bFound = ( rValue.Value >>= rTimeout );
    }
    return bFound;
}

bool ScOptSolverDlg::CallSolver()       // return true -> close dialog after calling
{
    // show progress dialog

    ScopedVclPtrInstance< ScSolverProgressDialog > aProgress( this );
    sal_Int32 nTimeout = 0;
    if ( FindTimeout( nTimeout ) )
        aProgress->SetTimeLimit( nTimeout );
    else
        aProgress->HideTimeLimit();
    aProgress->Show();
    aProgress->Update();
    aProgress->Flush();
    // try to make sure the progress dialog is painted before continuing
    Application::Reschedule(true);

    // collect solver parameters

    ReadConditions();

    uno::Reference<sheet::XSpreadsheetDocument> xDocument( mpDocShell->GetModel(), uno::UNO_QUERY );

    ScRange aObjRange;
    if ( !ParseRef( aObjRange, m_pEdObjectiveCell->GetText(), false ) )
    {
        ShowError( false, m_pEdObjectiveCell );
        return false;
    }
    table::CellAddress aObjective( aObjRange.aStart.Tab(), aObjRange.aStart.Col(), aObjRange.aStart.Row() );

    // "changing cells" can be several ranges
    ScRangeList aVarRanges;
    if ( !ParseWithNames( aVarRanges, m_pEdVariableCells->GetText(), &mrDoc ) )
    {
        ShowError( false, m_pEdVariableCells );
        return false;
    }
    uno::Sequence<table::CellAddress> aVariables;
    sal_Int32 nVarPos = 0;

    for ( size_t nRangePos=0, nRange = aVarRanges.size(); nRangePos < nRange; ++nRangePos )
    {
        ScRange aRange(*aVarRanges[ nRangePos ] );
        aRange.PutInOrder();
        SCTAB nTab = aRange.aStart.Tab();

        // resolve into single cells

        sal_Int32 nAdd = ( aRange.aEnd.Col() - aRange.aStart.Col() + 1 ) *
                         ( aRange.aEnd.Row() - aRange.aStart.Row() + 1 );
        aVariables.realloc( nVarPos + nAdd );

        for (SCROW nRow = aRange.aStart.Row(); nRow <= aRange.aEnd.Row(); ++nRow)
            for (SCCOL nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
                aVariables[nVarPos++] = table::CellAddress( nTab, nCol, nRow );
    }

    uno::Sequence<sheet::SolverConstraint> aConstraints;
    sal_Int32 nConstrPos = 0;
    for ( std::vector<ScOptConditionRow>::const_iterator aConstrIter = maConditions.begin();
          aConstrIter != maConditions.end(); ++aConstrIter )
    {
        if ( !aConstrIter->aLeftStr.isEmpty() )
        {
            sheet::SolverConstraint aConstraint;
            // order of list box entries must match enum values
            aConstraint.Operator = static_cast<sheet::SolverConstraintOperator>(aConstrIter->nOperator);

            ScRange aLeftRange;
            if ( !ParseRef( aLeftRange, aConstrIter->aLeftStr, true ) )
            {
                ShowError( true, nullptr );
                return false;
            }

            bool bIsRange = false;
            ScRange aRightRange;
            if ( ParseRef( aRightRange, aConstrIter->aRightStr, true ) )
            {
                if ( aRightRange.aStart == aRightRange.aEnd )
                    aConstraint.Right <<= table::CellAddress( aRightRange.aStart.Tab(),
                                                              aRightRange.aStart.Col(), aRightRange.aStart.Row() );
                else if ( aRightRange.aEnd.Col()-aRightRange.aStart.Col() == aLeftRange.aEnd.Col()-aLeftRange.aStart.Col() &&
                          aRightRange.aEnd.Row()-aRightRange.aStart.Row() == aLeftRange.aEnd.Row()-aLeftRange.aStart.Row() )
                    bIsRange = true;    // same size as "left" range, resolve into single cells
                else
                {
                    ShowError( true, nullptr );
                    return false;
                }
            }
            else
            {
                sal_uInt32 nFormat = 0;     //! explicit language?
                double fValue = 0.0;
                if ( mrDoc.GetFormatTable()->IsNumberFormat( aConstrIter->aRightStr, nFormat, fValue ) )
                    aConstraint.Right <<= fValue;
                else if ( aConstraint.Operator != sheet::SolverConstraintOperator_INTEGER &&
                          aConstraint.Operator != sheet::SolverConstraintOperator_BINARY )
                {
                    ShowError( true, nullptr );
                    return false;
                }
            }

            // resolve into single cells

            sal_Int32 nAdd = ( aLeftRange.aEnd.Col() - aLeftRange.aStart.Col() + 1 ) *
                             ( aLeftRange.aEnd.Row() - aLeftRange.aStart.Row() + 1 );
            aConstraints.realloc( nConstrPos + nAdd );

            for (SCROW nRow = aLeftRange.aStart.Row(); nRow <= aLeftRange.aEnd.Row(); ++nRow)
                for (SCCOL nCol = aLeftRange.aStart.Col(); nCol <= aLeftRange.aEnd.Col(); ++nCol)
                {
                    aConstraint.Left = table::CellAddress( aLeftRange.aStart.Tab(), nCol, nRow );
                    if ( bIsRange )
                        aConstraint.Right <<= table::CellAddress( aRightRange.aStart.Tab(),
                            aRightRange.aStart.Col() + ( nCol - aLeftRange.aStart.Col() ),
                            aRightRange.aStart.Row() + ( nRow - aLeftRange.aStart.Row() ) );

                    aConstraints[nConstrPos++] = aConstraint;
                }
        }
    }

    bool bMaximize = m_pRbMax->IsChecked();
    if ( m_pRbValue->IsChecked() )
    {
        // handle "value of" with an additional constraint (and then minimize)

        sheet::SolverConstraint aConstraint;
        aConstraint.Left     = aObjective;
        aConstraint.Operator = sheet::SolverConstraintOperator_EQUAL;

        OUString aValStr = m_pEdTargetValue->GetText();
        ScRange aRightRange;
        if ( ParseRef( aRightRange, aValStr, false ) )
            aConstraint.Right <<= table::CellAddress( aRightRange.aStart.Tab(),
                                                      aRightRange.aStart.Col(), aRightRange.aStart.Row() );
        else
        {
            sal_uInt32 nFormat = 0;     //! explicit language?
            double fValue = 0.0;
            if ( mrDoc.GetFormatTable()->IsNumberFormat( aValStr, nFormat, fValue ) )
                aConstraint.Right <<= fValue;
            else
            {
                ShowError( false, m_pEdTargetValue );
                return false;
            }
        }

        aConstraints.realloc( nConstrPos + 1 );
        aConstraints[nConstrPos++] = aConstraint;
    }

    // copy old document values

    sal_Int32 nVarCount = aVariables.getLength();
    uno::Sequence<double> aOldValues;
    aOldValues.realloc( nVarCount );
    for (nVarPos=0; nVarPos<nVarCount; ++nVarPos)
    {
        ScAddress aCellPos;
        ScUnoConversion::FillScAddress( aCellPos, aVariables[nVarPos] );
        aOldValues[nVarPos] = mrDoc.GetValue( aCellPos );
    }

    // create and initialize solver

    uno::Reference<sheet::XSolver> xSolver = ScSolverUtil::GetSolver( maEngine );
    OSL_ENSURE( xSolver.is(), "can't get solver component" );
    if ( !xSolver.is() )
        return false;

    xSolver->setDocument( xDocument );
    xSolver->setObjective( aObjective );
    xSolver->setVariables( aVariables );
    xSolver->setConstraints( aConstraints );
    xSolver->setMaximize( bMaximize );

    // set options
    uno::Reference<beans::XPropertySet> xOptProp(xSolver, uno::UNO_QUERY);
    if ( xOptProp.is() )
    {
        sal_Int32 nPropCount = maProperties.getLength();
        for (sal_Int32 nProp=0; nProp<nPropCount; ++nProp)
        {
            const beans::PropertyValue& rValue = maProperties[nProp];
            try
            {
                xOptProp->setPropertyValue( rValue.Name, rValue.Value );
            }
            catch ( uno::Exception & )
            {
                OSL_FAIL("Exception in solver option property");
            }
        }
    }

    xSolver->solve();
    bool bSuccess = xSolver->getSuccess();

    aProgress->Hide();
    bool bClose = false;
    bool bRestore = true;   // restore old values unless a solution is accepted
    if ( bSuccess )
    {
        // put solution into document so it is visible when asking
        uno::Sequence<double> aSolution = xSolver->getSolution();
        if ( aSolution.getLength() == nVarCount )
        {
            mpDocShell->LockPaint();
            ScDocFunc &rFunc = mpDocShell->GetDocFunc();
            for (nVarPos=0; nVarPos<nVarCount; ++nVarPos)
            {
                ScAddress aCellPos;
                ScUnoConversion::FillScAddress( aCellPos, aVariables[nVarPos] );
                rFunc.SetValueCell(aCellPos, aSolution[nVarPos], false);
            }
            mpDocShell->UnlockPaint();
        }
        //! else error?

        // take formatted result from document (result value from component is ignored)
        OUString aResultStr = mrDoc.GetString(
            static_cast<SCCOL>(aObjective.Column), static_cast<SCROW>(aObjective.Row),
            static_cast<SCTAB>(aObjective.Sheet));

        ScopedVclPtrInstance< ScSolverSuccessDialog > aDialog( this, aResultStr );
        if ( aDialog->Execute() == RET_OK )
        {
            // keep results and close dialog
            bRestore = false;
            bClose = true;
        }
    }
    else
    {
        OUString aError;
        uno::Reference<sheet::XSolverDescription> xDesc( xSolver, uno::UNO_QUERY );
        if ( xDesc.is() )
            aError = xDesc->getStatusDescription();         // error description from component
        ScopedVclPtrInstance< ScSolverNoSolutionDialog > aDialog( this, aError );
        aDialog->Execute();
    }

    if ( bRestore )         // restore old values
    {
        mpDocShell->LockPaint();
        ScDocFunc &rFunc = mpDocShell->GetDocFunc();
        for (nVarPos=0; nVarPos<nVarCount; ++nVarPos)
        {
            ScAddress aCellPos;
            ScUnoConversion::FillScAddress( aCellPos, aVariables[nVarPos] );
            rFunc.SetValueCell(aCellPos, aOldValues[nVarPos], false);
        }
        mpDocShell->UnlockPaint();
    }

    return bClose;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
