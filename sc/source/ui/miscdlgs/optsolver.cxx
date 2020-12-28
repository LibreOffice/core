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

#include <rangelst.hxx>
#include <sfx2/bindings.hxx>
#include <svl/zforlist.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>

#include <reffact.hxx>
#include <docsh.hxx>
#include <docfunc.hxx>
#include <rangeutl.hxx>
#include <convuno.hxx>
#include <unonames.hxx>
#include <solveroptions.hxx>
#include <solverutil.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

#include <optsolver.hxx>

#include <com/sun/star/sheet/SolverConstraint.hpp>
#include <com/sun/star/sheet/SolverConstraintOperator.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>
#include <com/sun/star/sheet/XSolver.hpp>

using namespace com::sun::star;

ScSolverProgressDialog::ScSolverProgressDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/scalc/ui/solverprogressdialog.ui",
                              "SolverProgressDialog")
    , m_xFtTime(m_xBuilder->weld_label("progress"))
{
}

ScSolverProgressDialog::~ScSolverProgressDialog()
{
}

void ScSolverProgressDialog::HideTimeLimit()
{
    m_xFtTime->hide();
}

void ScSolverProgressDialog::SetTimeLimit( sal_Int32 nSeconds )
{
    OUString aOld = m_xFtTime->get_label();
    OUString aNew = aOld.replaceFirst("#", OUString::number(nSeconds));
    m_xFtTime->set_label(aNew);
}

ScSolverNoSolutionDialog::ScSolverNoSolutionDialog(weld::Window* pParent, const OUString& rErrorText)
    : GenericDialogController(pParent, "modules/scalc/ui/nosolutiondialog.ui", "NoSolutionDialog")
    , m_xFtErrorText(m_xBuilder->weld_label("error"))
{
    m_xFtErrorText->set_label(rErrorText);
}

ScSolverNoSolutionDialog::~ScSolverNoSolutionDialog()
{
}

ScSolverSuccessDialog::ScSolverSuccessDialog(weld::Window* pParent, std::u16string_view rSolution)
    : GenericDialogController(pParent, "modules/scalc/ui/solversuccessdialog.ui", "SolverSuccessDialog")
    , m_xFtResult(m_xBuilder->weld_label("result"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
{
    m_xBtnOk->connect_clicked(LINK(this, ScSolverSuccessDialog, ClickHdl));
    m_xBtnCancel->connect_clicked(LINK(this, ScSolverSuccessDialog, ClickHdl));
    OUString aMessage = m_xFtResult->get_label() + " " + rSolution;
    m_xFtResult->set_label(aMessage);
}

ScSolverSuccessDialog::~ScSolverSuccessDialog()
{
}

IMPL_LINK(ScSolverSuccessDialog, ClickHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == m_xBtnOk.get())
        m_xDialog->response(RET_OK);
    else
        m_xDialog->response(RET_CANCEL);
}

ScCursorRefEdit::ScCursorRefEdit(std::unique_ptr<weld::Entry> xControl)
    : formula::RefEdit(std::move(xControl))
{
    xEntry->connect_key_press(Link<const KeyEvent&, bool>()); //acknowledge we first remove the old one
    xEntry->connect_key_press(LINK(this, ScCursorRefEdit, KeyInputHdl));
}

void ScCursorRefEdit::SetCursorLinks( const Link<ScCursorRefEdit&,void>& rUp, const Link<ScCursorRefEdit&,void>& rDown )
{
    maCursorUpLink = rUp;
    maCursorDownLink = rDown;
}

IMPL_LINK(ScCursorRefEdit, KeyInputHdl, const KeyEvent&, rKEvt, bool)
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
        return true;
    }
    return formula::RefEdit::KeyInput(rKEvt);
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

ScOptSolverDlg::ScOptSolverDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                               ScDocShell* pDocSh, const ScAddress& aCursorPos)
    : ScAnyRefDlgController(pB, pCW, pParent, "modules/scalc/ui/solverdlg.ui", "SolverDialog")
    , maInputError(ScResId(STR_INVALIDINPUT))
    , maConditionError(ScResId(STR_INVALIDCONDITION))

    , mpDocShell(pDocSh)
    , mrDoc(pDocSh->GetDocument())
    , mnCurTab(aCursorPos.Tab())
    , mbDlgLostFocus(false)
    , nScrollPos(0)
    , mpEdActive(nullptr)
    , m_xFtObjectiveCell(m_xBuilder->weld_label("targetlabel"))
    , m_xEdObjectiveCell(new formula::RefEdit(m_xBuilder->weld_entry("targetedit")))
    , m_xRBObjectiveCell(new formula::RefButton(m_xBuilder->weld_button("targetbutton")))
    , m_xRbMax(m_xBuilder->weld_radio_button("max"))
    , m_xRbMin(m_xBuilder->weld_radio_button("min"))
    , m_xRbValue(m_xBuilder->weld_radio_button("value"))
    , m_xEdTargetValue(new formula::RefEdit(m_xBuilder->weld_entry("valueedit")))
    , m_xRBTargetValue(new formula::RefButton(m_xBuilder->weld_button("valuebutton")))
    , m_xFtVariableCells(m_xBuilder->weld_label("changelabel"))
    , m_xEdVariableCells(new formula::RefEdit(m_xBuilder->weld_entry("changeedit")))
    , m_xRBVariableCells(new formula::RefButton(m_xBuilder->weld_button("changebutton")))
    , m_xFtCellRef(m_xBuilder->weld_label("cellreflabel"))
    , m_xEdLeft1(new ScCursorRefEdit(m_xBuilder->weld_entry("ref1edit")))
    , m_xRBLeft1(new formula::RefButton(m_xBuilder->weld_button("ref1button")))
    , m_xFtOperator(m_xBuilder->weld_label("oplabel"))
    , m_xLbOp1(m_xBuilder->weld_combo_box("op1list"))
    , m_xFtConstraint(m_xBuilder->weld_label("constraintlabel"))
    , m_xEdRight1(new ScCursorRefEdit(m_xBuilder->weld_entry("val1edit")))
    , m_xRBRight1(new formula::RefButton(m_xBuilder->weld_button("val1button")))
    , m_xBtnDel1(m_xBuilder->weld_button("del1"))
    , m_xEdLeft2(new ScCursorRefEdit(m_xBuilder->weld_entry("ref2edit")))
    , m_xRBLeft2(new formula::RefButton(m_xBuilder->weld_button("ref2button")))
    , m_xLbOp2(m_xBuilder->weld_combo_box("op2list"))
    , m_xEdRight2(new ScCursorRefEdit(m_xBuilder->weld_entry("val2edit")))
    , m_xRBRight2(new formula::RefButton(m_xBuilder->weld_button("val2button")))
    , m_xBtnDel2(m_xBuilder->weld_button("del2"))
    , m_xEdLeft3(new ScCursorRefEdit(m_xBuilder->weld_entry("ref3edit")))
    , m_xRBLeft3(new formula::RefButton(m_xBuilder->weld_button("ref3button")))
    , m_xLbOp3(m_xBuilder->weld_combo_box("op3list"))
    , m_xEdRight3(new ScCursorRefEdit(m_xBuilder->weld_entry("val3edit")))
    , m_xRBRight3(new formula::RefButton(m_xBuilder->weld_button("val3button")))
    , m_xBtnDel3(m_xBuilder->weld_button("del3"))
    , m_xEdLeft4(new ScCursorRefEdit(m_xBuilder->weld_entry("ref4edit")))
    , m_xRBLeft4(new formula::RefButton(m_xBuilder->weld_button("ref4button")))
    , m_xLbOp4(m_xBuilder->weld_combo_box("op4list"))
    , m_xEdRight4(new ScCursorRefEdit(m_xBuilder->weld_entry("val4edit")))
    , m_xRBRight4(new formula::RefButton(m_xBuilder->weld_button("val4button")))
    , m_xBtnDel4(m_xBuilder->weld_button("del4"))
    , m_xScrollBar(m_xBuilder->weld_scrolled_window("scrollbar", true))
    , m_xBtnOpt(m_xBuilder->weld_button("options"))
    , m_xBtnCancel(m_xBuilder->weld_button("close"))
    , m_xBtnSolve(m_xBuilder->weld_button("ok"))
    , m_xBtnResetAll(m_xBuilder->weld_button("resetall"))
    , m_xResultFT(m_xBuilder->weld_label("result"))
    , m_xContents(m_xBuilder->weld_widget("grid"))
{
    m_xEdObjectiveCell->SetReferences(this, m_xFtObjectiveCell.get());
    m_xRBObjectiveCell->SetReferences(this, m_xEdObjectiveCell.get());
    m_xEdTargetValue->SetReferences(this, m_xResultFT.get());
    m_xRBTargetValue->SetReferences(this, m_xEdTargetValue.get());
    m_xEdVariableCells->SetReferences(this, m_xFtVariableCells.get());
    m_xRBVariableCells->SetReferences(this, m_xEdVariableCells.get());
    m_xEdLeft1->SetReferences(this, m_xFtCellRef.get());
    m_xRBLeft1->SetReferences(this, m_xEdLeft1.get());
    m_xEdRight1->SetReferences(this, m_xFtConstraint.get());
    m_xRBRight1->SetReferences(this, m_xEdRight1.get());
    m_xEdLeft2->SetReferences(this, m_xFtCellRef.get());
    m_xRBLeft2->SetReferences(this, m_xEdLeft2.get());
    m_xEdRight2->SetReferences(this, m_xFtConstraint.get());
    m_xRBRight2->SetReferences(this, m_xEdRight2.get());
    m_xEdLeft3->SetReferences(this, m_xFtCellRef.get());
    m_xRBLeft3->SetReferences(this, m_xEdLeft3.get());
    m_xEdRight3->SetReferences(this, m_xFtConstraint.get());
    m_xRBRight3->SetReferences(this, m_xEdRight3.get());
    m_xEdLeft4->SetReferences(this, m_xFtCellRef.get());
    m_xRBLeft4->SetReferences(this, m_xEdLeft4.get());
    m_xEdRight4->SetReferences(this, m_xFtConstraint.get());
    m_xRBRight4->SetReferences(this, m_xEdRight4.get());

    mpLeftEdit[0]    = m_xEdLeft1.get();
    mpLeftButton[0]  = m_xRBLeft1.get();
    mpRightEdit[0]   = m_xEdRight1.get();
    mpRightButton[0] = m_xRBRight1.get();
    mpOperator[0]    = m_xLbOp1.get();
    mpDelButton[0]   = m_xBtnDel1.get();

    mpLeftEdit[1]    = m_xEdLeft2.get();
    mpLeftButton[1]  = m_xRBLeft2.get();
    mpRightEdit[1]   = m_xEdRight2.get();
    mpRightButton[1] = m_xRBRight2.get();
    mpOperator[1]    = m_xLbOp2.get();
    mpDelButton[1]   = m_xBtnDel2.get();

    mpLeftEdit[2]    = m_xEdLeft3.get();
    mpLeftButton[2]  = m_xRBLeft3.get();
    mpRightEdit[2]   = m_xEdRight3.get();
    mpRightButton[2] = m_xRBRight3.get();
    mpOperator[2]    = m_xLbOp3.get();
    mpDelButton[2]   = m_xBtnDel3.get();

    mpLeftEdit[3]    = m_xEdLeft4.get();
    mpLeftButton[3]  = m_xRBLeft4.get();
    mpRightEdit[3]   = m_xEdRight4.get();
    mpRightButton[3] = m_xRBRight4.get();
    mpOperator[3]    = m_xLbOp4.get();
    mpDelButton[3]   = m_xBtnDel4.get();

    Init( aCursorPos );
}

ScOptSolverDlg::~ScOptSolverDlg()
{
}

void ScOptSolverDlg::Init(const ScAddress& rCursorPos)
{
    uno::Reference<frame::XFrame> xFrame = GetBindings().GetActiveFrame();
    auto xDelNm = vcl::CommandInfoProvider::GetXGraphicForCommand(".uno:DeleteRows", xFrame);
    for (weld::Button* pButton : mpDelButton)
        pButton->set_image(xDelNm);

    m_xBtnOpt->connect_clicked( LINK( this, ScOptSolverDlg, BtnHdl ) );
    m_xBtnCancel->connect_clicked( LINK( this, ScOptSolverDlg, BtnHdl ) );
    m_xBtnSolve->connect_clicked( LINK( this, ScOptSolverDlg, BtnHdl ) );
    m_xBtnResetAll->connect_clicked( LINK( this, ScOptSolverDlg, BtnHdl ) );

    Link<formula::RefEdit&,void> aEditLink = LINK( this, ScOptSolverDlg, GetEditFocusHdl );
    Link<formula::RefButton&,void> aButtonLink = LINK( this, ScOptSolverDlg, GetButtonFocusHdl );
    m_xEdObjectiveCell->SetGetFocusHdl( aEditLink );
    m_xRBObjectiveCell->SetGetFocusHdl( aButtonLink );
    m_xEdTargetValue->SetGetFocusHdl( aEditLink );
    m_xRBTargetValue->SetGetFocusHdl( aButtonLink );
    m_xEdVariableCells->SetGetFocusHdl( aEditLink );
    m_xRBVariableCells->SetGetFocusHdl( aButtonLink );
    Link<weld::Widget&,void> aLink = LINK(this, ScOptSolverDlg, GetFocusHdl);
    m_xRbValue->connect_focus_in(aLink);
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        mpLeftEdit[nRow]->SetGetFocusHdl( aEditLink );
        mpLeftButton[nRow]->SetGetFocusHdl( aButtonLink );
        mpRightEdit[nRow]->SetGetFocusHdl( aEditLink );
        mpRightButton[nRow]->SetGetFocusHdl( aButtonLink );
        mpOperator[nRow]->connect_focus_in(aLink);
    }

    aEditLink = LINK( this, ScOptSolverDlg, LoseEditFocusHdl );
    aButtonLink = LINK( this, ScOptSolverDlg, LoseButtonFocusHdl );
    m_xEdObjectiveCell->SetLoseFocusHdl( aEditLink );
    m_xRBObjectiveCell->SetLoseFocusHdl( aButtonLink );
    m_xEdTargetValue->SetLoseFocusHdl( aEditLink );
    m_xRBTargetValue-> SetLoseFocusHdl( aButtonLink );
    m_xEdVariableCells->SetLoseFocusHdl( aEditLink );
    m_xRBVariableCells->SetLoseFocusHdl( aButtonLink );
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        mpLeftEdit[nRow]->SetLoseFocusHdl( aEditLink );
        mpLeftButton[nRow]->SetLoseFocusHdl( aButtonLink );
        mpRightEdit[nRow]->SetLoseFocusHdl( aEditLink );
        mpRightButton[nRow]->SetLoseFocusHdl( aButtonLink );
    }

    Link<ScCursorRefEdit&,void> aCursorUp = LINK( this, ScOptSolverDlg, CursorUpHdl );
    Link<ScCursorRefEdit&,void> aCursorDown = LINK( this, ScOptSolverDlg, CursorDownHdl );
    Link<formula::RefEdit&,void> aCondModify = LINK( this, ScOptSolverDlg, CondModifyHdl );
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        mpLeftEdit[nRow]->SetCursorLinks( aCursorUp, aCursorDown );
        mpRightEdit[nRow]->SetCursorLinks( aCursorUp, aCursorDown );
        mpLeftEdit[nRow]->SetModifyHdl( aCondModify );
        mpRightEdit[nRow]->SetModifyHdl( aCondModify );
        mpDelButton[nRow]->connect_clicked( LINK( this, ScOptSolverDlg, DelBtnHdl ) );
        mpOperator[nRow]->connect_changed( LINK( this, ScOptSolverDlg, SelectHdl ) );
    }
    m_xEdTargetValue->SetModifyHdl( LINK( this, ScOptSolverDlg, TargetModifyHdl ) );

    Size aSize(m_xContents->get_preferred_size());
    m_xContents->set_size_request(aSize.Width(), aSize.Height());
    m_xScrollBar->connect_vadjustment_changed( LINK( this, ScOptSolverDlg, ScrollHdl ) );

    m_xScrollBar->vadjustment_set_page_increment( EDIT_ROW_COUNT );
    m_xScrollBar->vadjustment_set_page_size( EDIT_ROW_COUNT );
    // Range is set in ShowConditions

    // get available solver implementations
    //! sort by descriptions?
    ScSolverUtil::GetImplementations( maImplNames, maDescriptions );
    bool bImplHasElements = maImplNames.hasElements();

    const ScOptSolverSave* pOldData = mpDocShell->GetSolverSaveData();
    if ( pOldData )
    {
        m_xEdObjectiveCell->SetRefString( pOldData->GetObjective() );
        m_xRbMax->set_active( pOldData->GetMax() );
        m_xRbMin->set_active( pOldData->GetMin() );
        m_xRbValue->set_active( pOldData->GetValue() );
        m_xEdTargetValue->SetRefString( pOldData->GetTarget() );
        m_xEdVariableCells->SetRefString( pOldData->GetVariable() );
        maConditions = pOldData->GetConditions();
        maEngine = pOldData->GetEngine();
        maProperties = pOldData->GetProperties();
    }
    else
    {
        m_xRbMax->set_active(true);
        OUString aCursorStr;
        if ( !mrDoc.GetRangeAtBlock( ScRange(rCursorPos), &aCursorStr ) )
            aCursorStr = rCursorPos.Format(ScRefFlags::ADDR_ABS, nullptr, mrDoc.GetAddressConvention());
        m_xEdObjectiveCell->SetRefString( aCursorStr );
        if ( bImplHasElements )
            maEngine = maImplNames[0];  // use first implementation
    }
    ShowConditions();

    m_xEdObjectiveCell->GrabFocus();
    mpEdActive = m_xEdObjectiveCell.get();
}

void ScOptSolverDlg::ReadConditions()
{
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        ScOptConditionRow aRowEntry;
        aRowEntry.aLeftStr = mpLeftEdit[nRow]->GetText();
        aRowEntry.aRightStr = mpRightEdit[nRow]->GetText();
        aRowEntry.nOperator = mpOperator[nRow]->get_active();

        tools::Long nVecPos = nScrollPos + nRow;
        if ( nVecPos >= static_cast<tools::Long>(maConditions.size()) && !aRowEntry.IsDefault() )
            maConditions.resize( nVecPos + 1 );

        if ( nVecPos < static_cast<tools::Long>(maConditions.size()) )
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

        tools::Long nVecPos = nScrollPos + nRow;
        if ( nVecPos < static_cast<tools::Long>(maConditions.size()) )
            aRowEntry = maConditions[nVecPos];

        mpLeftEdit[nRow]->SetRefString( aRowEntry.aLeftStr );
        mpRightEdit[nRow]->SetRefString( aRowEntry.aRightStr );
        mpOperator[nRow]->set_active( aRowEntry.nOperator );
    }

    // allow to scroll one page behind the visible or stored rows
    tools::Long nVisible = nScrollPos + EDIT_ROW_COUNT;
    tools::Long nMax = std::max( nVisible, static_cast<tools::Long>(maConditions.size()) );
    m_xScrollBar->vadjustment_configure(nScrollPos, 0, nMax + EDIT_ROW_COUNT, 1,
                                        EDIT_ROW_COUNT - 1, EDIT_ROW_COUNT);

    EnableButtons();
}

void ScOptSolverDlg::EnableButtons()
{
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        tools::Long nVecPos = nScrollPos + nRow;
        mpDelButton[nRow]->set_sensitive(nVecPos < static_cast<tools::Long>(maConditions.size()));
    }
}

void ScOptSolverDlg::Close()
{
    if (m_xOptDlg)
        m_xOptDlg->response(RET_CANCEL);
    assert(!m_xOptDlg);
    DoClose( ScOptSolverDlgWrapper::GetChildWindowId() );
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
        m_xDialog->grab_focus();
    }
    RefInputDone();
}

void ScOptSolverDlg::SetReference( const ScRange& rRef, ScDocument& rDocP )
{
    if( !mpEdActive )
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart(mpEdActive);

    // "target"/"value": single cell
    bool bSingle = ( mpEdActive == m_xEdObjectiveCell.get() || mpEdActive == m_xEdTargetValue.get() );

    OUString aStr;
    ScAddress aAdr = rRef.aStart;
    ScRange aNewRef( rRef );
    if ( bSingle )
        aNewRef.aEnd = aAdr;

    OUString aName;
    if ( rDocP.GetRangeAtBlock( aNewRef, &aName ) )            // named range: show name
        aStr = aName;
    else                                                        // format cell/range reference
    {
        ScRefFlags nFmt = ( aAdr.Tab() == mnCurTab ) ? ScRefFlags::ADDR_ABS : ScRefFlags::ADDR_ABS_3D;
        if ( bSingle )
            aStr = aAdr.Format(nFmt, &rDocP, rDocP.GetAddressConvention());
        else
            aStr = rRef.Format(rDocP, nFmt | ScRefFlags::RANGE_ABS, rDocP.GetAddressConvention());
    }

    // variable cells can be several ranges, so only the selection is replaced
    if ( mpEdActive == m_xEdVariableCells.get() )
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
    if ( mpEdActive == m_xEdTargetValue.get() )
        m_xRbValue->set_active(true);
}

bool ScOptSolverDlg::IsRefInputMode() const
{
    return mpEdActive != nullptr;
}

// Handler:

IMPL_LINK(ScOptSolverDlg, BtnHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == m_xBtnSolve.get() || &rBtn == m_xBtnCancel.get())
    {
        bool bSolve = ( &rBtn == m_xBtnSolve.get() );

        SetDispatcherLock( false );
        SwitchToDocument();

        bool bClose = true;
        if ( bSolve )
            bClose = CallSolver();

        if ( bClose )
        {
            // Close: write dialog settings to DocShell for subsequent calls
            ReadConditions();
            std::unique_ptr<ScOptSolverSave> pSave( new ScOptSolverSave(
                m_xEdObjectiveCell->GetText(), m_xRbMax->get_active(), m_xRbMin->get_active(), m_xRbValue->get_active(),
                m_xEdTargetValue->GetText(), m_xEdVariableCells->GetText(), maConditions, maEngine, maProperties ) );
            mpDocShell->SetSolverSaveData( std::move(pSave) );
            response(RET_CLOSE);
        }
        else
        {
            // no solution -> dialog is kept open
            SetDispatcherLock( true );
        }
    }
    else if (&rBtn == m_xBtnOpt.get())
    {
        //! move options dialog to UI lib?
        m_xOptDlg = std::make_shared<ScSolverOptionsDialog>(m_xDialog.get(), maImplNames, maDescriptions, maEngine, maProperties);
        weld::DialogController::runAsync(m_xOptDlg, [this](sal_Int32 nResult){
            if (nResult == RET_OK)
            {
                maEngine = m_xOptDlg->GetEngine();
                maProperties = m_xOptDlg->GetProperties();
            }
            m_xOptDlg.reset();
        });
    }
    else if (&rBtn == m_xBtnResetAll.get())
    {
        OUString sEmpty;
        m_xEdObjectiveCell->SetText(sEmpty);
        m_xEdTargetValue->SetText(sEmpty);
        m_xEdVariableCells->SetText(sEmpty);

        // Get default property values of solver implementations
        maEngine = maImplNames[0];
        maProperties = ScSolverUtil::GetDefaults( maEngine );

        // Clear all conditions (Constraints)
        maConditions.clear();
        std::unique_ptr<ScOptSolverSave> pEmpty( new ScOptSolverSave(
                        sEmpty, true, false, false,
                        sEmpty, sEmpty, maConditions, maEngine, maProperties ) );
        mpDocShell->SetSolverSaveData( std::move(pEmpty) );
        ShowConditions();

        m_xRbMax->set_active(true);
        m_xEdObjectiveCell->GrabFocus();
        mpEdActive = m_xEdObjectiveCell.get();
    }
}

IMPL_LINK( ScOptSolverDlg, GetEditFocusHdl, formula::RefEdit&, rCtrl, void )
{
    formula::RefEdit* pEdit = nullptr;
    mpEdActive = nullptr;

    if( &rCtrl == m_xEdObjectiveCell.get() )
        pEdit = mpEdActive = m_xEdObjectiveCell.get();
    else if( &rCtrl == m_xEdTargetValue.get() )
        pEdit = mpEdActive = m_xEdTargetValue.get();
    else if( &rCtrl == m_xEdVariableCells.get() )
        pEdit = mpEdActive = m_xEdVariableCells.get();
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        if( &rCtrl == mpLeftEdit[nRow]  )
            pEdit = mpEdActive = mpLeftEdit[nRow];
        else if( &rCtrl == mpRightEdit[nRow]  )
            pEdit = mpEdActive = mpRightEdit[nRow];
    }

    if( pEdit )
        pEdit->SelectAll();
}

IMPL_LINK( ScOptSolverDlg, GetButtonFocusHdl, formula::RefButton&, rCtrl, void )
{
    formula::RefEdit* pEdit = nullptr;
    mpEdActive = nullptr;

    if( &rCtrl == m_xRBObjectiveCell.get() )
        pEdit = mpEdActive = m_xEdObjectiveCell.get();
    else if( &rCtrl == m_xRBTargetValue.get() )
        pEdit = mpEdActive = m_xEdTargetValue.get();
    else if( &rCtrl == m_xRBVariableCells.get() )
        pEdit = mpEdActive = m_xEdVariableCells.get();
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
    {
        if( &rCtrl == mpLeftButton[nRow] )
            pEdit = mpEdActive = mpLeftEdit[nRow];
        else if( &rCtrl == mpRightButton[nRow] )
            pEdit = mpEdActive = mpRightEdit[nRow];
    }

    if( pEdit )
        pEdit->SelectAll();
}


IMPL_LINK(ScOptSolverDlg, GetFocusHdl, weld::Widget&, rCtrl, void)
{
    if( &rCtrl == m_xRbValue.get() )                   // focus on "Value of" radio button
        mpEdActive = m_xEdTargetValue.get();          // use value edit for ref input, but don't change selection
    else
    {
        for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
        {
            if( &rCtrl == mpOperator[nRow] )    // focus on "operator" list box
                mpEdActive = mpRightEdit[nRow];     // use right edit for ref input, but don't change selection
        }
    }
}

IMPL_LINK_NOARG(ScOptSolverDlg, LoseEditFocusHdl, formula::RefEdit&, void)
{
    mbDlgLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScOptSolverDlg, LoseButtonFocusHdl, formula::RefButton&, void)
{
    mbDlgLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK(ScOptSolverDlg, DelBtnHdl, weld::Button&, rBtn, void)
{
    for ( sal_uInt16 nRow = 0; nRow < EDIT_ROW_COUNT; ++nRow )
        if (&rBtn == mpDelButton[nRow])
        {
            bool bHadFocus = rBtn.has_focus();

            ReadConditions();
            tools::Long nVecPos = nScrollPos + nRow;
            if ( nVecPos < static_cast<tools::Long>(maConditions.size()) )
            {
                maConditions.erase( maConditions.begin() + nVecPos );
                ShowConditions();

                if ( bHadFocus && !rBtn.get_sensitive() )
                {
                    // If the button is disabled, focus would normally move to the next control,
                    // (left edit of the next row). Move it to left edit of this row instead.

                    mpEdActive = mpLeftEdit[nRow];
                    mpEdActive->GrabFocus();
                }
            }
        }
}

IMPL_LINK_NOARG(ScOptSolverDlg, TargetModifyHdl, formula::RefEdit&, void)
{
    // modify handler for the target edit:
    //  select "Value of" if something is input into the edit
    if ( !m_xEdTargetValue->GetText().isEmpty() )
        m_xRbValue->set_active(true);
}

IMPL_LINK_NOARG(ScOptSolverDlg, CondModifyHdl, formula::RefEdit&, void)
{
    // modify handler for the condition edits, just to enable/disable "delete" buttons
    ReadConditions();
    EnableButtons();
}

IMPL_LINK_NOARG(ScOptSolverDlg, SelectHdl, weld::ComboBox&, void)
{
    // select handler for operator list boxes, just to enable/disable "delete" buttons
    ReadConditions();
    EnableButtons();
}

IMPL_LINK_NOARG(ScOptSolverDlg, ScrollHdl, weld::ScrolledWindow&, void)
{
    ReadConditions();
    nScrollPos = m_xScrollBar->vadjustment_get_value();
    ShowConditions();
    if( mpEdActive )
        mpEdActive->SelectAll();
}

IMPL_LINK( ScOptSolverDlg, CursorUpHdl, ScCursorRefEdit&, rEdit, void )
{
    if ( &rEdit == mpLeftEdit[0] || &rEdit == mpRightEdit[0] )
    {
        if ( nScrollPos > 0 )
        {
            ReadConditions();
            --nScrollPos;
            ShowConditions();
            if( mpEdActive )
                mpEdActive->SelectAll();
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

IMPL_LINK( ScOptSolverDlg, CursorDownHdl, ScCursorRefEdit&, rEdit, void )
{
    if ( &rEdit == mpLeftEdit[EDIT_ROW_COUNT-1] || &rEdit == mpRightEdit[EDIT_ROW_COUNT-1] )
    {
        //! limit scroll position?
        ReadConditions();
        ++nScrollPos;
        ShowConditions();
        if( mpEdActive )
            mpEdActive->SelectAll();
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
    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                              VclMessageType::Warning, VclButtonsType::Ok,
                                              aMessage));
    xBox->run();
    if (pFocus)
    {
        mpEdActive = pFocus;
        pFocus->GrabFocus();
    }
}

bool ScOptSolverDlg::ParseRef( ScRange& rRange, const OUString& rInput, bool bAllowRange )
{
    ScAddress::Details aDetails(mrDoc.GetAddressConvention(), 0, 0);
    ScRefFlags nFlags = rRange.ParseAny( rInput, mrDoc, aDetails );
    if ( nFlags & ScRefFlags::VALID )
    {
        if ( (nFlags & ScRefFlags::TAB_3D) == ScRefFlags::ZERO)
            rRange.aStart.SetTab( mnCurTab );
        if ( (nFlags & ScRefFlags::TAB2_3D) == ScRefFlags::ZERO)
            rRange.aEnd.SetTab( rRange.aStart.Tab() );
        return ( bAllowRange || rRange.aStart == rRange.aEnd );
    }
    else if ( ScRangeUtil::MakeRangeFromName( rInput, mrDoc, mnCurTab, rRange, RUTL_NAMES, aDetails ) )
        return ( bAllowRange || rRange.aStart == rRange.aEnd );

    return false;   // not recognized
}

bool ScOptSolverDlg::FindTimeout( sal_Int32& rTimeout )
{
    bool bFound = false;

    if ( !maProperties.hasElements() )
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

    auto xProgress = std::make_shared<ScSolverProgressDialog>(m_xDialog.get());
    sal_Int32 nTimeout = 0;
    if ( FindTimeout( nTimeout ) )
        xProgress->SetTimeLimit( nTimeout );
    else
        xProgress->HideTimeLimit();

    weld::DialogController::runAsync(xProgress, [](sal_Int32 /*nResult*/){});

    // try to make sure the progress dialog is painted before continuing
    Application::Reschedule(true);

    // collect solver parameters

    ReadConditions();

    uno::Reference<sheet::XSpreadsheetDocument> xDocument( mpDocShell->GetModel(), uno::UNO_QUERY );

    ScRange aObjRange;
    if ( !ParseRef( aObjRange, m_xEdObjectiveCell->GetText(), false ) )
    {
        ShowError( false, m_xEdObjectiveCell.get() );
        return false;
    }
    table::CellAddress aObjective( aObjRange.aStart.Tab(), aObjRange.aStart.Col(), aObjRange.aStart.Row() );

    // "changing cells" can be several ranges
    ScRangeList aVarRanges;
    if ( !ParseWithNames( aVarRanges, m_xEdVariableCells->GetText(), mrDoc ) )
    {
        ShowError( false, m_xEdVariableCells.get() );
        return false;
    }
    uno::Sequence<table::CellAddress> aVariables;
    sal_Int32 nVarPos = 0;

    for ( size_t nRangePos=0, nRange = aVarRanges.size(); nRangePos < nRange; ++nRangePos )
    {
        ScRange aRange( aVarRanges[ nRangePos ] );
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
    for ( const auto& rConstr : maConditions )
    {
        if ( !rConstr.aLeftStr.isEmpty() )
        {
            sheet::SolverConstraint aConstraint;
            // order of list box entries must match enum values
            aConstraint.Operator = static_cast<sheet::SolverConstraintOperator>(rConstr.nOperator);

            ScRange aLeftRange;
            if ( !ParseRef( aLeftRange, rConstr.aLeftStr, true ) )
            {
                ShowError( true, nullptr );
                return false;
            }

            bool bIsRange = false;
            ScRange aRightRange;
            if ( ParseRef( aRightRange, rConstr.aRightStr, true ) )
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
                if ( mrDoc.GetFormatTable()->IsNumberFormat( rConstr.aRightStr, nFormat, fValue ) )
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

    bool bMaximize = m_xRbMax->get_active();
    if ( m_xRbValue->get_active() )
    {
        // handle "value of" with an additional constraint (and then minimize)

        sheet::SolverConstraint aConstraint;
        aConstraint.Left     = aObjective;
        aConstraint.Operator = sheet::SolverConstraintOperator_EQUAL;

        OUString aValStr = m_xEdTargetValue->GetText();
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
                ShowError( false, m_xEdTargetValue.get() );
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
    std::transform(aVariables.begin(), aVariables.end(), aOldValues.begin(),
        [this](const table::CellAddress& rVariable) -> double {
            ScAddress aCellPos;
            ScUnoConversion::FillScAddress( aCellPos, rVariable );
            return mrDoc.GetValue( aCellPos );
        });

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
        for (const beans::PropertyValue& rValue : std::as_const(maProperties))
        {
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

    xProgress->response(RET_CLOSE);

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

        ScSolverSuccessDialog aDialog(m_xDialog.get(), aResultStr);
        if (aDialog.run() == RET_OK)
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
        ScSolverNoSolutionDialog aDialog(m_xDialog.get(), aError);
        aDialog.run();
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
