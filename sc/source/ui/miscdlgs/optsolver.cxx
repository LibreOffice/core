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

ScCursorRefEdit::ScCursorRefEdit( ScAnyRefDlg* pParent, const ResId& rResId ) :
    formula::RefEdit( pParent, pParent, rResId )
{
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

    :   ScAnyRefDlg         ( pB, pCW, pParent, RID_SCDLG_OPTSOLVER ),
        //
        maFtObjectiveCell   ( this, ScResId( FT_OBJECTIVECELL ) ),
        maEdObjectiveCell   ( this, this, ScResId( ED_OBJECTIVECELL ) ),
        maRBObjectiveCell   ( this, ScResId( IB_OBJECTIVECELL ), &maEdObjectiveCell, &maFtObjectiveCell, this ),
        maFtDirection       ( this, ScResId( FT_DIRECTION ) ),
        maRbMax             ( this, ScResId( RB_MAX ) ),
        maRbMin             ( this, ScResId( RB_MIN ) ),
        maRbValue           ( this, ScResId( RB_VALUE ) ),
        maEdTargetValue     ( this, this, ScResId( ED_TARGET ) ),
        maRBTargetValue     ( this, ScResId( IB_TARGET ), &maEdTargetValue, NULL, this ),
        maFtVariableCells   ( this, ScResId( FT_VARIABLECELLS ) ),
        maEdVariableCells   ( this, this, ScResId( ED_VARIABLECELLS ) ),
        maRBVariableCells   ( this, ScResId( IB_VARIABLECELLS ), &maEdVariableCells, &maFtVariableCells, this),
        maFlConditions      ( this, ScResId( FL_CONDITIONS ) ),
        maFtCellRef         ( this, ScResId( FT_CELLREF ) ),
        maEdLeft1           ( this, ScResId( ED_LEFT1 ) ),
        maRBLeft1           ( this, ScResId( IB_LEFT1 ), &maEdLeft1, NULL, this ),
        maFtOperator        ( this, ScResId( FT_OPERATOR ) ),
        maLbOp1             ( this, ScResId( LB_OP1 ) ),
        maFtConstraint      ( this, ScResId( FT_CONSTRAINT ) ),
        maEdRight1          ( this, ScResId( ED_RIGHT1 ) ),
        maRBRight1          ( this, ScResId( IB_RIGHT1 ), &maEdRight1, NULL, this ),
        maBtnDel1           ( this, ScResId( IB_DELETE1 ) ),
        maEdLeft2           ( this, ScResId( ED_LEFT2 ) ),
        maRBLeft2           ( this, ScResId( IB_LEFT2 ), &maEdLeft2, NULL, this ),
        maLbOp2             ( this, ScResId( LB_OP2 ) ),
        maEdRight2          ( this, ScResId( ED_RIGHT2 ) ),
        maRBRight2          ( this, ScResId( IB_RIGHT2 ), &maEdRight2, NULL, this ),
        maBtnDel2           ( this, ScResId( IB_DELETE2 ) ),
        maEdLeft3           ( this, ScResId( ED_LEFT3 ) ),
        maRBLeft3           ( this, ScResId( IB_LEFT3 ), &maEdLeft3, NULL, this ),
        maLbOp3             ( this, ScResId( LB_OP3 ) ),
        maEdRight3          ( this, ScResId( ED_RIGHT3 ) ),
        maRBRight3          ( this, ScResId( IB_RIGHT3 ), &maEdRight3, NULL, this ),
        maBtnDel3           ( this, ScResId( IB_DELETE3 ) ),
        maEdLeft4           ( this, ScResId( ED_LEFT4 ) ),
        maRBLeft4           ( this, ScResId( IB_LEFT4 ), &maEdLeft4, NULL, this ),
        maLbOp4             ( this, ScResId( LB_OP4 ) ),
        maEdRight4          ( this, ScResId( ED_RIGHT4 ) ),
        maRBRight4          ( this, ScResId( IB_RIGHT4 ), &maEdRight4, NULL, this ),
        maBtnDel4           ( this, ScResId( IB_DELETE4 ) ),
        maScrollBar         ( this, ScResId( SB_SCROLL ) ),
        maFlButtons         ( this, ScResId( FL_BUTTONS ) ),
        maBtnOpt            ( this, ScResId( BTN_OPTIONS ) ),
        maBtnHelp           ( this, ScResId( BTN_HELP ) ),
        maBtnCancel         ( this, ScResId( BTN_CLOSE ) ),
        maBtnSolve          ( this, ScResId( BTN_SOLVE ) ),
        maInputError        ( ScResId( STR_INVALIDINPUT ) ),
        maConditionError    ( ScResId( STR_INVALIDCONDITION ) ),
        //
        mpDocShell          ( pDocSh ),
        mpDoc               ( pDocSh->GetDocument() ),
        mnCurTab            ( aCursorPos.Tab() ),
        mpEdActive          ( NULL ),
        mbDlgLostFocus      ( false ),
        nScrollPos          ( 0 )
{
    mpLeftEdit[0]    = &maEdLeft1;
    mpLeftButton[0]  = &maRBLeft1;
    mpRightEdit[0]   = &maEdRight1;
    mpRightButton[0] = &maRBRight1;
    mpOperator[0]    = &maLbOp1;
    mpDelButton[0]   = &maBtnDel1;

    mpLeftEdit[1]    = &maEdLeft2;
    mpLeftButton[1]  = &maRBLeft2;
    mpRightEdit[1]   = &maEdRight2;
    mpRightButton[1] = &maRBRight2;
    mpOperator[1]    = &maLbOp2;
    mpDelButton[1]   = &maBtnDel2;

    mpLeftEdit[2]    = &maEdLeft3;
    mpLeftButton[2]  = &maRBLeft3;
    mpRightEdit[2]   = &maEdRight3;
    mpRightButton[2] = &maRBRight3;
    mpOperator[2]    = &maLbOp3;
    mpDelButton[2]   = &maBtnDel3;

    mpLeftEdit[3]    = &maEdLeft4;
    mpLeftButton[3]  = &maRBLeft4;
    mpRightEdit[3]   = &maEdRight4;
    mpRightButton[3] = &maRBRight4;
    mpOperator[3]    = &maLbOp4;
    mpDelButton[3]   = &maBtnDel4;

    maRbMax.SetAccessibleRelationMemberOf(&maFtDirection);
    maRbMin.SetAccessibleRelationMemberOf(&maFtDirection);
    maRbValue.SetAccessibleRelationMemberOf(&maFtDirection);
    maEdLeft2.SetAccessibleName(maFtCellRef.GetText());
    maLbOp2.SetAccessibleName(maFtOperator.GetText());
    maEdRight2.SetAccessibleName(maFtConstraint.GetText());
    maEdLeft3.SetAccessibleName(maFtCellRef.GetText());
    maLbOp3.SetAccessibleName(maFtOperator.GetText());
    maEdRight3.SetAccessibleName(maFtConstraint.GetText());
    maEdLeft4.SetAccessibleName(maFtCellRef.GetText());
    maLbOp4.SetAccessibleName(maFtOperator.GetText());
    maEdRight4.SetAccessibleName(maFtConstraint.GetText());

    Init( aCursorPos );
    FreeResource();
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
    aSlotURL += OUString::valueOf( sal_Int32( SID_DEL_ROWS ) );
    uno::Reference<frame::XFrame> xFrame = GetBindings().GetActiveFrame();
    Image aDelNm = ::GetImage( xFrame, aSlotURL, false );

    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        mpDelButton[nRow]->SetModeImage( aDelNm );
    }

    maBtnOpt.SetClickHdl( LINK( this, ScOptSolverDlg, BtnHdl ) );
    maBtnCancel.SetClickHdl( LINK( this, ScOptSolverDlg, BtnHdl ) );
    maBtnSolve.SetClickHdl( LINK( this, ScOptSolverDlg, BtnHdl ) );

    Link aLink = LINK( this, ScOptSolverDlg, GetFocusHdl );
    maEdObjectiveCell.SetGetFocusHdl( aLink );
    maRBObjectiveCell.SetGetFocusHdl( aLink );
    maEdTargetValue.SetGetFocusHdl( aLink );
    maRBTargetValue.SetGetFocusHdl( aLink );
    maEdVariableCells.SetGetFocusHdl( aLink );
    maRBVariableCells.SetGetFocusHdl( aLink );
    maRbValue.SetGetFocusHdl( aLink );
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        mpLeftEdit[nRow]->SetGetFocusHdl( aLink );
        mpLeftButton[nRow]->SetGetFocusHdl( aLink );
        mpRightEdit[nRow]->SetGetFocusHdl( aLink );
        mpRightButton[nRow]->SetGetFocusHdl( aLink );
        mpOperator[nRow]->SetGetFocusHdl( aLink );
    }

    aLink = LINK( this, ScOptSolverDlg, LoseFocusHdl );
    maEdObjectiveCell.SetLoseFocusHdl( aLink );
    maRBObjectiveCell.SetLoseFocusHdl( aLink );
    maEdTargetValue.  SetLoseFocusHdl( aLink );
    maRBTargetValue.  SetLoseFocusHdl( aLink );
    maEdVariableCells.SetLoseFocusHdl( aLink );
    maRBVariableCells.SetLoseFocusHdl( aLink );
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
    maEdTargetValue.SetModifyHdl( LINK( this, ScOptSolverDlg, TargetModifyHdl ) );

    maScrollBar.SetEndScrollHdl( LINK( this, ScOptSolverDlg, ScrollHdl ) );
    maScrollBar.SetScrollHdl( LINK( this, ScOptSolverDlg, ScrollHdl ) );

    maScrollBar.SetPageSize( EDIT_ROW_COUNT );
    maScrollBar.SetVisibleSize( EDIT_ROW_COUNT );
    maScrollBar.SetLineSize( 1 );
    // Range is set in ShowConditions

    // get available solver implementations
    //! sort by descriptions?
    ScSolverUtil::GetImplementations( maImplNames, maDescriptions );
    sal_Int32 nImplCount = maImplNames.getLength();

    const ScOptSolverSave* pOldData = mpDocShell->GetSolverSaveData();
    if ( pOldData )
    {
        maEdObjectiveCell.SetRefString( pOldData->GetObjective() );
        maRbMax.Check( pOldData->GetMax() );
        maRbMin.Check( pOldData->GetMin() );
        maRbValue.Check( pOldData->GetValue() );
        maEdTargetValue.SetRefString( pOldData->GetTarget() );
        maEdVariableCells.SetRefString( pOldData->GetVariable() );
        maConditions = pOldData->GetConditions();
        maEngine = pOldData->GetEngine();
        maProperties = pOldData->GetProperties();
    }
    else
    {
        maRbMax.Check();
        OUString aCursorStr;
        if ( !mpDoc->GetRangeAtBlock( ScRange(rCursorPos), &aCursorStr ) )
            rCursorPos.Format( aCursorStr, SCA_ABS, NULL, mpDoc->GetAddressConvention() );
        maEdObjectiveCell.SetRefString( aCursorStr );
        if ( nImplCount > 0 )
            maEngine = maImplNames[0];  // use first implementation
    }
    ShowConditions();

    maEdObjectiveCell.GrabFocus();
    mpEdActive = &maEdObjectiveCell;
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
    maScrollBar.SetRange( Range( 0, nMax + EDIT_ROW_COUNT ) );
    maScrollBar.SetThumbPos( nScrollPos );

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
        bool bSingle = ( mpEdActive == &maEdObjectiveCell || mpEdActive == &maEdTargetValue );

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
        if ( mpEdActive == &maEdVariableCells )
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
        if ( mpEdActive == &maEdTargetValue )
            maRbValue.Check();
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
    if ( pBtn == &maBtnSolve || pBtn == &maBtnCancel )
    {
        bool bSolve = ( pBtn == &maBtnSolve );

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
                maEdObjectiveCell.GetText(), maRbMax.IsChecked(), maRbMin.IsChecked(), maRbValue.IsChecked(),
                maEdTargetValue.GetText(), maEdVariableCells.GetText(), maConditions, maEngine, maProperties );
            mpDocShell->SetSolverSaveData( aSave );
            Close();
        }
        else
        {
            // no solution -> dialog is kept open
            SetDispatcherLock( sal_True );
        }
    }
    else if ( pBtn == &maBtnOpt )
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

    if( pCtrl == &maEdObjectiveCell || pCtrl == &maRBObjectiveCell )
        pEdit = mpEdActive = &maEdObjectiveCell;
    else if( pCtrl == &maEdTargetValue || pCtrl == &maRBTargetValue )
        pEdit = mpEdActive = &maEdTargetValue;
    else if( pCtrl == &maEdVariableCells || pCtrl == &maRBVariableCells )
        pEdit = mpEdActive = &maEdVariableCells;
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        if( pCtrl == mpLeftEdit[nRow] || pCtrl == mpLeftButton[nRow] )
            pEdit = mpEdActive = mpLeftEdit[nRow];
        else if( pCtrl == mpRightEdit[nRow] || pCtrl == mpRightButton[nRow] )
            pEdit = mpEdActive = mpRightEdit[nRow];
        else if( pCtrl == mpOperator[nRow] )    // focus on "operator" list box
            mpEdActive = mpRightEdit[nRow];     // use right edit for ref input, but don't change selection
    }
    if( pCtrl == &maRbValue )                   // focus on "Value of" radio button
        mpEdActive = &maEdTargetValue;          // use value edit for ref input, but don't change selection

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
    if ( !maEdTargetValue.GetText().isEmpty() )
        maRbValue.Check();
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
    nScrollPos = maScrollBar.GetThumbPos();
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
    if ( !ParseRef( aObjRange, maEdObjectiveCell.GetText(), false ) )
    {
        ShowError( false, &maEdObjectiveCell );
        return false;
    }
    table::CellAddress aObjective( aObjRange.aStart.Tab(), aObjRange.aStart.Col(), aObjRange.aStart.Row() );

    // "changing cells" can be several ranges
    ScRangeList aVarRanges;
    if ( !ParseWithNames( aVarRanges, maEdVariableCells.GetText(), mpDoc ) )
    {
        ShowError( false, &maEdVariableCells );
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

    sal_Bool bMaximize = maRbMax.IsChecked();
    if ( maRbValue.IsChecked() )
    {
        // handle "value of" with an additional constraint (and then minimize)

        sheet::SolverConstraint aConstraint;
        aConstraint.Left     = aObjective;
        aConstraint.Operator = sheet::SolverConstraintOperator_EQUAL;

        String aValStr = maEdTargetValue.GetText();
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
                ShowError( false, &maEdTargetValue );
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
