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
#include <vcl/builder.hxx>
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
#include "optsolver.hrc"

#include "optsolver.hxx"

#include <com/sun/star/sheet/Solver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>

using namespace com::sun::star;

//----------------------------------------------------------------------------

ScSolverProgressDialog::ScSolverProgressDialog( Window* pParent )
    : ModelessDialog( pParent, ScResId( RID_SCDLG_SOLVER_PROGRESS ) ),
    maFtProgress    ( this, ScResId( FT_PROGRESS ) ),
    maFtTime        ( this, ScResId( FT_TIMELIMIT ) ),
    maFlButtons     ( this, ScResId( FL_BUTTONS ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) )
{
    maBtnOk.Enable(false);
    FreeResource();
}

ScSolverProgressDialog::~ScSolverProgressDialog()
{
}

void ScSolverProgressDialog::HideTimeLimit()
{
    maFtTime.Hide();
}

void ScSolverProgressDialog::SetTimeLimit( sal_Int32 nSeconds )
{
    OUString aOld = maFtTime.GetText();
    OUString aNew = aOld.getToken(0,'#') + OUString::number( nSeconds ) + aOld.getToken(1,'#');
    maFtTime.SetText( aNew );
}

//----------------------------------------------------------------------------

ScSolverNoSolutionDialog::ScSolverNoSolutionDialog( Window* pParent, const String& rErrorText )
    : ModalDialog( pParent, ScResId( RID_SCDLG_SOLVER_NOSOLUTION ) ),
    maFtNoSolution  ( this, ScResId( FT_NOSOLUTION ) ),
    maFtErrorText   ( this, ScResId( FT_ERRORTEXT ) ),
    maFlButtons     ( this, ScResId( FL_BUTTONS ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) )
{
    maFtErrorText.SetText( rErrorText );
    FreeResource();
}

ScSolverNoSolutionDialog::~ScSolverNoSolutionDialog()
{
}

//----------------------------------------------------------------------------

ScSolverSuccessDialog::ScSolverSuccessDialog( Window* pParent, const String& rSolution )
    : ModalDialog( pParent, ScResId( RID_SCDLG_SOLVER_SUCCESS ) ),
    maFtSuccess     ( this, ScResId( FT_SUCCESS ) ),
    maFtResult      ( this, ScResId( FT_RESULT ) ),
    maFtQuestion    ( this, ScResId( FT_QUESTION ) ),
    maFlButtons     ( this, ScResId( FL_BUTTONS ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) ),
    maBtnCancel     ( this, ScResId( BTN_CANCEL ) )
{
    String aMessage = maFtResult.GetText();
    aMessage.Append( (sal_Char) ' ' );
    aMessage.Append( rSolution );
    maFtResult.SetText( aMessage );
    FreeResource();
}

ScSolverSuccessDialog::~ScSolverSuccessDialog()
{
}

//----------------------------------------------------------------------------

ScCursorRefEdit::ScCursorRefEdit( Window* pParent, Window *pLabel )
    : formula::RefEdit( pParent, pLabel )
{
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeScCursorRefEdit(Window *pParent,
    VclBuilder::stringmap &)
{
    return new ScCursorRefEdit(pParent, NULL);
}

void ScCursorRefEdit::SetCursorLinks( const Link& rUp, const Link& rDown )
{
    maCursorUpLink = rUp;
    maCursorDownLink = rDown;
}

void ScCursorRefEdit::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode = rKEvt.GetKeyCode();
    bool bUp = (aCode.GetCode() == KEY_UP);
    bool bDown = (aCode.GetCode() == KEY_DOWN);
    if ( !aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() && ( bUp || bDown ) )
    {
        if ( bUp )
            maCursorUpLink.Call( this );
        else
            maCursorDownLink.Call( this );
    }
    else
        formula::RefEdit::KeyInput( rKEvt );
}

//----------------------------------------------------------------------------

ScOptSolverSave::ScOptSolverSave( const String& rObjective, sal_Bool bMax, sal_Bool bMin, sal_Bool bValue,
                             const String& rTarget, const String& rVariable,
                             const std::vector<ScOptConditionRow>& rConditions,
                             const String& rEngine,
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

//============================================================================
//  class ScOptSolverDlg
//----------------------------------------------------------------------------

ScOptSolverDlg::ScOptSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                          ScDocShell* pDocSh, ScAddress aCursorPos )

    : ScAnyRefDlg(pB, pCW, pParent, "SolverDialog", "modules/scalc/ui/solverdlg.ui")
    , maInputError(ScGlobal::GetRscString(STR_INVALIDINPUT))
    , maConditionError(ScGlobal::GetRscString(STR_INVALIDCONDITION))
    //
    , mpDocShell(pDocSh)
    , mpDoc(pDocSh->GetDocument())
    , mnCurTab(aCursorPos.Tab())
    , mpEdActive(NULL)
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

//----------------------------------------------------------------------------

ScOptSolverDlg::~ScOptSolverDlg()
{
}

//----------------------------------------------------------------------------

void ScOptSolverDlg::Init(const ScAddress& rCursorPos)
{
    // Get the "Delete Rows" commandimagelist images from sfx instead of
    // adding a second copy to sc (see ScTbxInsertCtrl::StateChanged)

    OUString aSlotURL( "slot:" );
    aSlotURL += OUString::number( SID_DEL_ROWS );
    uno::Reference<frame::XFrame> xFrame = GetBindings().GetActiveFrame();
    Image aDelNm = ::GetImage( xFrame, aSlotURL, false );

    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        mpDelButton[nRow]->SetModeImage( aDelNm );
    }

    m_pBtnOpt->SetClickHdl( LINK( this, ScOptSolverDlg, BtnHdl ) );
    m_pBtnCancel->SetClickHdl( LINK( this, ScOptSolverDlg, BtnHdl ) );
    m_pBtnSolve->SetClickHdl( LINK( this, ScOptSolverDlg, BtnHdl ) );

    Link aLink = LINK( this, ScOptSolverDlg, GetFocusHdl );
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

    Link aCursorUp = LINK( this, ScOptSolverDlg, CursorUpHdl );
    Link aCursorDown = LINK( this, ScOptSolverDlg, CursorDownHdl );
    Link aCondModify = LINK( this, ScOptSolverDlg, CondModifyHdl );
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
        if ( !mpDoc->GetRangeAtBlock( ScRange(rCursorPos), &aCursorStr ) )
            rCursorPos.Format( aCursorStr, SCA_ABS, NULL, mpDoc->GetAddressConvention() );
        m_pEdObjectiveCell->SetRefString( aCursorStr );
        if ( nImplCount > 0 )
            maEngine = maImplNames[0];  // use first implementation
    }
    ShowConditions();

    m_pEdObjectiveCell->GrabFocus();
    mpEdActive = m_pEdObjectiveCell;
}

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

sal_Bool ScOptSolverDlg::Close()
{
    return DoClose( ScOptSolverDlgWrapper::GetChildWindowId() );
}

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

void ScOptSolverDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if( mpEdActive )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(mpEdActive);

        // "target"/"value": single cell
        bool bSingle = ( mpEdActive == m_pEdObjectiveCell || mpEdActive == m_pEdTargetValue );

        String aStr;
        ScAddress aAdr = rRef.aStart;
        ScRange aNewRef( rRef );
        if ( bSingle )
            aNewRef.aEnd = aAdr;

        OUString aName;
        if ( pDocP->GetRangeAtBlock( aNewRef, &aName ) )            // named range: show name
            aStr = aName;
        else                                                        // format cell/range reference
        {
            sal_uInt16 nFmt = ( aAdr.Tab() == mnCurTab ) ? SCA_ABS : SCA_ABS_3D;
            if ( bSingle )
                aAdr.Format( aStr, nFmt, pDocP, pDocP->GetAddressConvention() );
            else
                rRef.Format( aStr, nFmt | SCR_ABS, pDocP, pDocP->GetAddressConvention() );
        }

        // variable cells can be several ranges, so only the selection is replaced
        if ( mpEdActive == m_pEdVariableCells )
        {
            String aVal = mpEdActive->GetText();
            Selection aSel = mpEdActive->GetSelection();
            aSel.Justify();
            aVal.Erase( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
            aVal.Insert( aStr, (xub_StrLen)aSel.Min() );
            Selection aNewSel( aSel.Min(), aSel.Min()+aStr.Len() );
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

//----------------------------------------------------------------------------

sal_Bool ScOptSolverDlg::IsRefInputMode() const
{
    return mpEdActive != NULL;
}

//----------------------------------------------------------------------------
// Handler:

IMPL_LINK( ScOptSolverDlg, BtnHdl, PushButton*, pBtn )
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
            SetDispatcherLock( sal_True );
        }
    }
    else if ( pBtn == m_pBtnOpt )
    {
        //! move options dialog to UI lib?
        ScSolverOptionsDialog* pOptDlg =
            new ScSolverOptionsDialog( this, maImplNames, maDescriptions, maEngine, maProperties );
        if ( pOptDlg->Execute() == RET_OK )
        {
            maEngine = pOptDlg->GetEngine();
            maProperties = pOptDlg->GetProperties();
        }
        delete pOptDlg;
    }

    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScOptSolverDlg, GetFocusHdl, Control*, pCtrl )
{
    Edit* pEdit = NULL;
    mpEdActive = NULL;

    if( pCtrl == m_pEdObjectiveCell || pCtrl == m_pRBObjectiveCell )
        pEdit = mpEdActive = m_pEdObjectiveCell;
    else if( pCtrl == m_pEdTargetValue || pCtrl == m_pRBTargetValue )
        pEdit = mpEdActive = m_pEdTargetValue;
    else if( pCtrl == m_pEdVariableCells || pCtrl == m_pRBVariableCells )
        pEdit = mpEdActive = m_pEdVariableCells;
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        if( pCtrl == mpLeftEdit[nRow] || pCtrl == mpLeftButton[nRow] )
            pEdit = mpEdActive = mpLeftEdit[nRow];
        else if( pCtrl == mpRightEdit[nRow] || pCtrl == mpRightButton[nRow] )
            pEdit = mpEdActive = mpRightEdit[nRow];
        else if( pCtrl == mpOperator[nRow] )    // focus on "operator" list box
            mpEdActive = mpRightEdit[nRow];     // use right edit for ref input, but don't change selection
    }
    if( pCtrl == m_pRbValue )                   // focus on "Value of" radio button
        mpEdActive = m_pEdTargetValue;          // use value edit for ref input, but don't change selection

    if( pEdit )
        pEdit->SetSelection( Selection( 0, SELECTION_MAX ) );

    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK_NOARG(ScOptSolverDlg, LoseFocusHdl)
{
    mbDlgLostFocus = !IsActive();
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScOptSolverDlg, DelBtnHdl, PushButton*, pBtn )
{
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
        if( pBtn == mpDelButton[nRow] )
        {
            sal_Bool bHadFocus = pBtn->HasFocus();

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

                    mpEdActive = mpLeftEdit[nRow];
                    mpEdActive->GrabFocus();
                }
            }
        }

    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK_NOARG(ScOptSolverDlg, TargetModifyHdl)
{
    // modify handler for the target edit:
    //  select "Value of" if something is input into the edit
    if ( !m_pEdTargetValue->GetText().isEmpty() )
        m_pRbValue->Check();
    return 0;
}

IMPL_LINK_NOARG(ScOptSolverDlg, CondModifyHdl)
{
    // modify handler for the condition edits, just to enable/disable "delete" buttons
    ReadConditions();
    EnableButtons();
    return 0;
}

IMPL_LINK_NOARG(ScOptSolverDlg, SelectHdl)
{
    // select handler for operator list boxes, just to enable/disable "delete" buttons
    ReadConditions();
    EnableButtons();
    return 0;
}

IMPL_LINK_NOARG(ScOptSolverDlg, ScrollHdl)
{
    ReadConditions();
    nScrollPos = m_pScrollBar->GetThumbPos();
    ShowConditions();
    if( mpEdActive )
        mpEdActive->SetSelection( Selection( 0, SELECTION_MAX ) );
    return 0;
}

IMPL_LINK( ScOptSolverDlg, CursorUpHdl, ScCursorRefEdit*, pEdit )
{
    if ( pEdit == mpLeftEdit[0] || pEdit == mpRightEdit[0] )
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
        formula::RefEdit* pFocus = NULL;
        for ( sal_uInt16 nRow = 1; nRow < EDIT_ROW_COUNT; ++nRow )      // second row or below: move focus
        {
            if ( pEdit == mpLeftEdit[nRow] )
                pFocus = mpLeftEdit[nRow-1];
            else if ( pEdit == mpRightEdit[nRow] )
                pFocus = mpRightEdit[nRow-1];
        }
        if (pFocus)
        {
            mpEdActive = pFocus;
            pFocus->GrabFocus();
        }
    }

    return 0;
}

IMPL_LINK( ScOptSolverDlg, CursorDownHdl, ScCursorRefEdit*, pEdit )
{
    if ( pEdit == mpLeftEdit[EDIT_ROW_COUNT-1] || pEdit == mpRightEdit[EDIT_ROW_COUNT-1] )
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
        formula::RefEdit* pFocus = NULL;
        for ( sal_uInt16 nRow = 0; nRow+1 < EDIT_ROW_COUNT; ++nRow )      // before last row: move focus
        {
            if ( pEdit == mpLeftEdit[nRow] )
                pFocus = mpLeftEdit[nRow+1];
            else if ( pEdit == mpRightEdit[nRow] )
                pFocus = mpRightEdit[nRow+1];
        }
        if (pFocus)
        {
            mpEdActive = pFocus;
            pFocus->GrabFocus();
        }
    }

    return 0;
}

//----------------------------------------------------------------------------

void ScOptSolverDlg::ShowError( bool bCondition, formula::RefEdit* pFocus )
{
    String aMessage = bCondition ? maConditionError : maInputError;
    ErrorBox( this, WinBits( WB_OK | WB_DEF_OK ), aMessage ).Execute();
    if (pFocus)
    {
        mpEdActive = pFocus;
        pFocus->GrabFocus();
    }
}

//----------------------------------------------------------------------------

bool ScOptSolverDlg::ParseRef( ScRange& rRange, const String& rInput, bool bAllowRange )
{
    ScRangeUtil aRangeUtil;
    ScAddress::Details aDetails(mpDoc->GetAddressConvention(), 0, 0);
    sal_uInt16 nFlags = rRange.ParseAny( rInput, mpDoc, aDetails );
    if ( nFlags & SCA_VALID )
    {
        if ( (nFlags & SCA_TAB_3D) == 0 )
            rRange.aStart.SetTab( mnCurTab );
        if ( (nFlags & SCA_TAB2_3D) == 0 )
            rRange.aEnd.SetTab( rRange.aStart.Tab() );
        return ( bAllowRange || rRange.aStart == rRange.aEnd );
    }
    else if ( aRangeUtil.MakeRangeFromName( rInput, mpDoc, mnCurTab, rRange, RUTL_NAMES, aDetails ) )
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

    ScSolverProgressDialog aProgress( this );
    sal_Int32 nTimeout = 0;
    if ( FindTimeout( nTimeout ) )
        aProgress.SetTimeLimit( nTimeout );
    else
        aProgress.HideTimeLimit();
    aProgress.Show();
    aProgress.Update();
    aProgress.Sync();
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
    if ( !ParseWithNames( aVarRanges, m_pEdVariableCells->GetText(), mpDoc ) )
    {
        ShowError( false, m_pEdVariableCells );
        return false;
    }
    uno::Sequence<table::CellAddress> aVariables;
    sal_Int32 nVarPos = 0;

    for ( size_t nRangePos=0, nRange = aVarRanges.size(); nRangePos < nRange; ++nRangePos )
    {
        ScRange aRange(*aVarRanges[ nRangePos ] );
        aRange.Justify();
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
        if ( aConstrIter->aLeftStr.Len() )
        {
            sheet::SolverConstraint aConstraint;
            // order of list box entries must match enum values
            aConstraint.Operator = static_cast<sheet::SolverConstraintOperator>(aConstrIter->nOperator);

            ScRange aLeftRange;
            if ( !ParseRef( aLeftRange, aConstrIter->aLeftStr, true ) )
            {
                ShowError( true, NULL );
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
                    ShowError( true, NULL );
                    return false;
                }
            }
            else
            {
                sal_uInt32 nFormat = 0;     //! explicit language?
                double fValue = 0.0;
                if ( mpDoc->GetFormatTable()->IsNumberFormat( aConstrIter->aRightStr, nFormat, fValue ) )
                    aConstraint.Right <<= fValue;
                else if ( aConstraint.Operator != sheet::SolverConstraintOperator_INTEGER &&
                          aConstraint.Operator != sheet::SolverConstraintOperator_BINARY )
                {
                    ShowError( true, NULL );
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

    sal_Bool bMaximize = m_pRbMax->IsChecked();
    if ( m_pRbValue->IsChecked() )
    {
        // handle "value of" with an additional constraint (and then minimize)

        sheet::SolverConstraint aConstraint;
        aConstraint.Left     = aObjective;
        aConstraint.Operator = sheet::SolverConstraintOperator_EQUAL;

        String aValStr = m_pEdTargetValue->GetText();
        ScRange aRightRange;
        if ( ParseRef( aRightRange, aValStr, false ) )
            aConstraint.Right <<= table::CellAddress( aRightRange.aStart.Tab(),
                                                      aRightRange.aStart.Col(), aRightRange.aStart.Row() );
        else
        {
            sal_uInt32 nFormat = 0;     //! explicit language?
            double fValue = 0.0;
            if ( mpDoc->GetFormatTable()->IsNumberFormat( aValStr, nFormat, fValue ) )
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
        aOldValues[nVarPos] = mpDoc->GetValue( aCellPos );
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
    sal_Bool bSuccess = xSolver->getSuccess();

    aProgress.Hide();
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
        OUString aResultStr = mpDoc->GetString(
            static_cast<SCCOL>(aObjective.Column), static_cast<SCROW>(aObjective.Row),
            static_cast<SCTAB>(aObjective.Sheet));

        ScSolverSuccessDialog aDialog( this, aResultStr );
        if ( aDialog.Execute() == RET_OK )
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
        ScSolverNoSolutionDialog aDialog( this, aError );
        aDialog.Execute();
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
