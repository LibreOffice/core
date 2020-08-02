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

#include <scitems.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/zforlist.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <uiitems.hxx>
#include <reffact.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <sc.hrc>
#include <solvrdlg.hxx>

ScSolverDlg::ScSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                          ScDocument* pDocument,
                          const ScAddress& aCursorPos )

    : ScAnyRefDlgController(pB, pCW, pParent, "modules/scalc/ui/goalseekdlg.ui", "GoalSeekDialog")
    , theFormulaCell(aCursorPos)
    , theVariableCell(aCursorPos)
    , pDoc(pDocument)
    , nCurTab(aCursorPos.Tab())
    , bDlgLostFocus(false)
    , errMsgInvalidVar(ScResId(STR_INVALIDVAR))
    , errMsgInvalidForm(ScResId(STR_INVALIDFORM))
    , errMsgNoFormula(ScResId(STR_NOFORMULA))
    , errMsgInvalidVal(ScResId(STR_INVALIDVAL))
    , m_pEdActive(nullptr)
    , m_xFtFormulaCell(m_xBuilder->weld_label("formulatext"))
    , m_xEdFormulaCell(new formula::RefEdit(m_xBuilder->weld_entry("formulaedit")))
    , m_xRBFormulaCell(new formula::RefButton(m_xBuilder->weld_button("formulabutton")))
    , m_xEdTargetVal(m_xBuilder->weld_entry("target"))
    , m_xFtVariableCell(m_xBuilder->weld_label("vartext"))
    , m_xEdVariableCell(new formula::RefEdit(m_xBuilder->weld_entry("varedit")))
    , m_xRBVariableCell(new formula::RefButton(m_xBuilder->weld_button("varbutton")))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
{
    m_xEdFormulaCell->SetReferences(this, m_xFtFormulaCell.get());
    m_xRBFormulaCell->SetReferences(this, m_xEdFormulaCell.get());
    m_xEdVariableCell->SetReferences(this, m_xFtVariableCell.get());
    m_xRBVariableCell->SetReferences(this, m_xEdVariableCell.get());
    Init();
}

ScSolverDlg::~ScSolverDlg()
{
    if (m_xMessageBox)
        m_xMessageBox->response(RET_CANCEL);
    assert(!m_xMessageBox);
}

void ScSolverDlg::Init()
{
    m_xBtnOk->connect_clicked( LINK( this, ScSolverDlg, BtnHdl ) );
    m_xBtnCancel->connect_clicked( LINK( this, ScSolverDlg, BtnHdl ) );

    Link<formula::RefEdit&,void> aEditLink = LINK( this, ScSolverDlg, GetEditFocusHdl );
    m_xEdFormulaCell->SetGetFocusHdl( aEditLink );
    m_xEdVariableCell->SetGetFocusHdl( aEditLink );

    Link<formula::RefButton&,void> aButtonLink = LINK( this, ScSolverDlg, GetButtonFocusHdl );
    m_xRBFormulaCell->SetGetFocusHdl( aButtonLink );
    m_xRBVariableCell->SetGetFocusHdl( aButtonLink );

    m_xEdTargetVal->connect_focus_in(LINK(this, ScSolverDlg, GetFocusHdl));

    aEditLink = LINK( this, ScSolverDlg, LoseEditFocusHdl );
    m_xEdFormulaCell->SetLoseFocusHdl ( aEditLink );
    m_xEdVariableCell->SetLoseFocusHdl ( aEditLink );

    aButtonLink = LINK( this, ScSolverDlg, LoseButtonFocusHdl );
    m_xRBFormulaCell->SetLoseFocusHdl ( aButtonLink );
    m_xRBVariableCell->SetLoseFocusHdl ( aButtonLink );

    OUString aStr(theFormulaCell.Format(ScRefFlags::ADDR_ABS, nullptr, pDoc->GetAddressConvention()));

    m_xEdFormulaCell->SetText( aStr );
    m_xEdFormulaCell->GrabFocus();
    m_pEdActive = m_xEdFormulaCell.get();
}

void ScSolverDlg::Close()
{
    DoClose( ScSolverDlgWrapper::GetChildWindowId() );
}

void ScSolverDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;
        if( m_pEdActive )
            m_pEdActive->GrabFocus();
    }
    else
    {
        m_xDialog->grab_focus();
    }
    RefInputDone();
}

void ScSolverDlg::SetReference( const ScRange& rRef, ScDocument& rDocP )
{
    if( !m_pEdActive )
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart(m_pEdActive);

    ScAddress   aAdr = rRef.aStart;
    ScRefFlags      nFmt = ( aAdr.Tab() == nCurTab )
                            ? ScRefFlags::ADDR_ABS
                            : ScRefFlags::ADDR_ABS_3D;

    OUString aStr(aAdr.Format(nFmt, &rDocP, rDocP.GetAddressConvention()));
    m_pEdActive->SetRefString( aStr );

    if (m_pEdActive == m_xEdFormulaCell.get())
        theFormulaCell = aAdr;
    else if (m_pEdActive == m_xEdVariableCell.get())
        theVariableCell = aAdr;
}

void ScSolverDlg::RaiseError( ScSolverErr eError )
{
    OUString sMessage;

    switch (eError)
    {
        case SOLVERR_NOFORMULA:
            sMessage = errMsgNoFormula;
            break;
        case SOLVERR_INVALID_FORMULA:
            sMessage = errMsgInvalidForm;
            break;
        case SOLVERR_INVALID_VARIABLE:
            sMessage = errMsgInvalidVar;
            break;
        case SOLVERR_INVALID_TARGETVALUE:
            sMessage = errMsgInvalidVal;
            break;
    }

    m_xMessageBox.reset(Application::CreateMessageDialog(m_xDialog.get(),
                                                         VclMessageType::Warning, VclButtonsType::Ok,
                                                         sMessage));
    m_xMessageBox->runAsync(m_xMessageBox, [this](sal_Int32 /*nResult*/) {
        m_xEdTargetVal->grab_focus();
        m_xMessageBox.reset();
    });
}

bool ScSolverDlg::IsRefInputMode() const
{
    return m_pEdActive != nullptr;
}

bool ScSolverDlg::CheckTargetValue( const OUString& rStrVal )
{
    sal_uInt32 n1 = 0;
    double n2;

    return pDoc->GetFormatTable()->IsNumberFormat( rStrVal, n1, n2 );
}

// Handler:

IMPL_LINK(ScSolverDlg, BtnHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == m_xBtnOk.get())
    {
        theTargetValStr = m_xEdTargetVal->get_text();

        // The following code checks:
        // 1. do the strings contain correct references / defined names?
        // 2. does the formula coordinate refer to a cell containing a formula?
        // 3. has a valid target value been entered?

        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        ScRefFlags  nRes1 = theFormulaCell .Parse( m_xEdFormulaCell->GetText(),  pDoc, eConv );
        ScRefFlags  nRes2 = theVariableCell.Parse( m_xEdVariableCell->GetText(), pDoc, eConv );

        if ( (nRes1 & ScRefFlags::VALID) == ScRefFlags::VALID )
        {
            if ( (nRes2 & ScRefFlags::VALID) == ScRefFlags::VALID )
            {
                if ( CheckTargetValue( theTargetValStr ) )
                {
                    CellType eType;
                    pDoc->GetCellType( theFormulaCell.Col(),
                                       theFormulaCell.Row(),
                                       theFormulaCell.Tab(),
                                       eType );

                    if ( CELLTYPE_FORMULA  == eType )
                    {
                        ScSolveParam aOutParam( theFormulaCell,
                                                theVariableCell,
                                                theTargetValStr );
                        ScSolveItem  aOutItem( SCITEM_SOLVEDATA, &aOutParam );

                        SetDispatcherLock( false );

                        SwitchToDocument();
                        GetBindings().GetDispatcher()->ExecuteList(SID_SOLVE,
                                                  SfxCallMode::SLOT | SfxCallMode::RECORD,
                                                  { &aOutItem });
                        response(RET_OK);
                    }
                    else RaiseError( SOLVERR_NOFORMULA );
                }
                else RaiseError( SOLVERR_INVALID_TARGETVALUE );
            }
            else RaiseError( SOLVERR_INVALID_VARIABLE );
        }
        else RaiseError( SOLVERR_INVALID_FORMULA );
    }
    else if (&rBtn == m_xBtnCancel.get())
    {
        response(RET_CANCEL);
    }
}

IMPL_LINK(ScSolverDlg, GetEditFocusHdl, formula::RefEdit&, rCtrl, void)
{
    if (&rCtrl == m_xEdFormulaCell.get())
        m_pEdActive = m_xEdFormulaCell.get();
    else if (&rCtrl == m_xEdVariableCell.get())
        m_pEdActive = m_xEdVariableCell.get();

    if (m_pEdActive)
        m_pEdActive->SelectAll();
}

IMPL_LINK_NOARG(ScSolverDlg, GetFocusHdl, weld::Widget&, void)
{
    m_pEdActive = nullptr;
    m_xEdTargetVal->select_region(0, -1);
}

IMPL_LINK(ScSolverDlg, GetButtonFocusHdl, formula::RefButton&, rCtrl, void)
{
    if (&rCtrl == m_xRBFormulaCell.get())
        m_pEdActive = m_xEdFormulaCell.get();
    else if (&rCtrl == m_xRBVariableCell.get())
        m_pEdActive = m_xEdVariableCell.get();

    if (m_pEdActive)
        m_pEdActive->SelectAll();
}

IMPL_LINK_NOARG(ScSolverDlg, LoseEditFocusHdl, formula::RefEdit&, void)
{
    bDlgLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScSolverDlg, LoseButtonFocusHdl, formula::RefButton&, void)
{
    bDlgLostFocus = !m_xDialog->has_toplevel_focus();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
