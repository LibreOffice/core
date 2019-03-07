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
#include <vcl/weld.hxx>

#include <uiitems.hxx>
#include <reffact.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <sc.hrc>
#include <solvrdlg.hxx>

namespace
{
    void lclErrorDialog(weld::Window* pParent, const OUString& rString, const std::function<void(sal_Int32)>& func)
    {
        std::shared_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  rString));
        xBox->runAsync(func);
    }
}

ScSolverDlg::ScSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                          ScDocument* pDocument,
                          const ScAddress& aCursorPos )

    : ScAnyRefDlg(pB, pCW, pParent, "GoalSeekDialog", "modules/scalc/ui/goalseekdlg.ui")
    , theFormulaCell(aCursorPos)
    , theVariableCell(aCursorPos)
    , pDoc(pDocument)
    , nCurTab(aCursorPos.Tab())
    , pEdActive(nullptr)
    , bDlgLostFocus(false)
    , errMsgInvalidVar(ScResId(STR_INVALIDVAR))
    , errMsgInvalidForm(ScResId(STR_INVALIDFORM))
    , errMsgNoFormula(ScResId(STR_NOFORMULA))
    , errMsgInvalidVal(ScResId(STR_INVALIDVAL))
{
    get(m_pFtFormulaCell, "formulatext");
    get(m_pEdFormulaCell, "formulaedit");
    m_pEdFormulaCell->SetReferences(this, m_pFtFormulaCell);
    get(m_pRBFormulaCell, "formulabutton");
    m_pRBFormulaCell->SetReferences(this, m_pEdFormulaCell);
    get(m_pEdTargetVal, "target");
    get(m_pFtVariableCell, "vartext");
    get(m_pEdVariableCell, "varedit");
    m_pEdVariableCell->SetReferences(this, m_pFtVariableCell);
    get(m_pRBVariableCell, "varbutton");
    m_pRBVariableCell->SetReferences(this, m_pEdVariableCell);
    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");
    Init();
}

ScSolverDlg::~ScSolverDlg()
{
    disposeOnce();
}

void ScSolverDlg::dispose()
{
    m_pFtFormulaCell.clear();
    m_pEdFormulaCell.clear();
    m_pRBFormulaCell.clear();
    m_pEdTargetVal.clear();
    m_pFtVariableCell.clear();
    m_pEdVariableCell.clear();
    m_pRBVariableCell.clear();
    m_pBtnOk.clear();
    m_pBtnCancel.clear();
    pEdActive.clear();
    ScAnyRefDlg::dispose();
}

void ScSolverDlg::Init()
{
    m_pBtnOk->SetClickHdl( LINK( this, ScSolverDlg, BtnHdl ) );
    m_pBtnCancel->SetClickHdl( LINK( this, ScSolverDlg, BtnHdl ) );

    Link<Control&,void> aLink = LINK( this, ScSolverDlg, GetFocusHdl );
    m_pEdFormulaCell->SetGetFocusHdl( aLink );
    m_pRBFormulaCell->SetGetFocusHdl( aLink );
    m_pEdVariableCell->SetGetFocusHdl( aLink );
    m_pRBVariableCell->SetGetFocusHdl( aLink );
    m_pEdTargetVal->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScSolverDlg, LoseFocusHdl );
    m_pEdFormulaCell->SetLoseFocusHdl ( aLink );
    m_pRBFormulaCell->SetLoseFocusHdl ( aLink );
    m_pEdVariableCell->SetLoseFocusHdl ( aLink );
    m_pRBVariableCell->SetLoseFocusHdl ( aLink );

    OUString aStr(theFormulaCell.Format(ScRefFlags::ADDR_ABS, nullptr, pDoc->GetAddressConvention()));

    m_pEdFormulaCell->SetText( aStr );
    m_pEdFormulaCell->GrabFocus();
    pEdActive = m_pEdFormulaCell;
}

bool ScSolverDlg::Close()
{
    return DoClose( ScSolverDlgWrapper::GetChildWindowId() );
}

void ScSolverDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;
        if( pEdActive )
            pEdActive->GrabFocus();
    }
    else
    {
        GrabFocus();
    }
    RefInputDone();
}

void ScSolverDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if( pEdActive )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(pEdActive);

        ScAddress   aAdr = rRef.aStart;
        ScRefFlags      nFmt = ( aAdr.Tab() == nCurTab )
                                ? ScRefFlags::ADDR_ABS
                                : ScRefFlags::ADDR_ABS_3D;

        OUString aStr(aAdr.Format(nFmt, pDocP, pDocP->GetAddressConvention()));
        pEdActive->SetRefString( aStr );

        if ( pEdActive == m_pEdFormulaCell )
            theFormulaCell = aAdr;
        else if ( pEdActive == m_pEdVariableCell )
            theVariableCell = aAdr;
    }
}

void ScSolverDlg::RaiseError( ScSolverErr eError )
{
    switch ( eError )
    {
        case SOLVERR_NOFORMULA:
            lclErrorDialog(GetFrameWeld(), errMsgNoFormula,
                [this](sal_Int32 /*nResult*/) {
                    m_pEdFormulaCell->GrabFocus();
                });
            break;

        case SOLVERR_INVALID_FORMULA:
            lclErrorDialog(GetFrameWeld(), errMsgInvalidForm,
                [this](sal_Int32 /*nResult*/) {
                    m_pEdFormulaCell->GrabFocus();
                });
            break;

        case SOLVERR_INVALID_VARIABLE:
            lclErrorDialog(GetFrameWeld(), errMsgInvalidVar,
                [this](sal_Int32 /*nResult*/) {
                    m_pEdVariableCell->GrabFocus();
                });
            break;

        case SOLVERR_INVALID_TARGETVALUE:
            lclErrorDialog(GetFrameWeld(), errMsgInvalidVal,
                [this](sal_Int32 /*nResult*/) {
                    m_pEdTargetVal->GrabFocus();
                });
            break;
    }
}

bool ScSolverDlg::IsRefInputMode() const
{
    return pEdActive != nullptr;
}

bool ScSolverDlg::CheckTargetValue( const OUString& rStrVal )
{
    sal_uInt32 n1 = 0;
    double n2;

    return pDoc->GetFormatTable()->IsNumberFormat( rStrVal, n1, n2 );
}

// Handler:

IMPL_LINK( ScSolverDlg, BtnHdl, Button*, pBtn, void )
{
    if (pBtn == m_pBtnOk)
    {
        theTargetValStr = m_pEdTargetVal->GetText();

        // The following code checks:
        // 1. do the strings contain correct references / defined names?
        // 2. does the formula coordinate refer to a cell containing a formula?
        // 3. has a valid target value been entered?

        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        ScRefFlags  nRes1 = theFormulaCell .Parse( m_pEdFormulaCell->GetText(),  pDoc, eConv );
        ScRefFlags  nRes2 = theVariableCell.Parse( m_pEdVariableCell->GetText(), pDoc, eConv );

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
                        Close();
                    }
                    else RaiseError( SOLVERR_NOFORMULA );
                }
                else RaiseError( SOLVERR_INVALID_TARGETVALUE );
            }
            else RaiseError( SOLVERR_INVALID_VARIABLE );
        }
        else RaiseError( SOLVERR_INVALID_FORMULA );
    }
    else if (pBtn == m_pBtnCancel)
    {
        Close();
    }
}

IMPL_LINK( ScSolverDlg, GetFocusHdl, Control&, rCtrl, void )
{
    Edit* pEdit = nullptr;
    pEdActive = nullptr;

    if( (&rCtrl == static_cast<Control*>(m_pEdFormulaCell)) || (&rCtrl == static_cast<Control*>(m_pRBFormulaCell)) )
        pEdit = pEdActive = m_pEdFormulaCell;
    else if( (&rCtrl == static_cast<Control*>(m_pEdVariableCell)) || (&rCtrl == static_cast<Control*>(m_pRBVariableCell)) )
        pEdit = pEdActive = m_pEdVariableCell;
    else if( &rCtrl == static_cast<Control*>(m_pEdTargetVal) )
        pEdit = m_pEdTargetVal;

    if( pEdit )
        pEdit->SetSelection( Selection( 0, SELECTION_MAX ) );
}

IMPL_LINK_NOARG(ScSolverDlg, LoseFocusHdl, Control&, void)
{
    bDlgLostFocus = !IsActive();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
