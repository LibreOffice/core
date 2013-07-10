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
#include <sfx2/dispatch.hxx>
#include <svl/zforlist.hxx>
#include <vcl/msgbox.hxx>

#include "uiitems.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "sc.hrc"

#define _SOLVRDLG_CXX
#include "solvrdlg.hxx"
#undef  _SOLVRDLG_CXX

namespace
{
    void lclErrorDialog( Window* pParent, OUString aString )
    {
        ErrorBox( pParent, WinBits( WB_OK | WB_DEF_OK), aString ).Execute();
    }
}

ScSolverDlg::ScSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                          ScDocument* pDocument,
                          ScAddress aCursorPos )

    : ScAnyRefDlg(pB, pCW, pParent, "GoalSeekDialog", "modules/scalc/ui/goalseekdlg.ui")
    , theFormulaCell(aCursorPos)
    , theVariableCell(aCursorPos)
    , pDoc(pDocument)
    , nCurTab(aCursorPos.Tab())
    , pEdActive(NULL)
    , bDlgLostFocus(false)
    , errMsgInvalidVar(ScGlobal::GetRscString(STR_INVALIDVAR))
    , errMsgInvalidForm(ScGlobal::GetRscString(STR_INVALIDFORM))
    , errMsgNoFormula(ScGlobal::GetRscString(STR_NOFORMULA))
    , errMsgInvalidVal(ScGlobal::GetRscString(STR_INVALIDVAL))
{
    get(m_pFtFormulaCell, "formulatext");
    get(m_pEdFormulaCell, "formulaedit");
    m_pEdFormulaCell->SetReferences(this, m_pFtFormulaCell);
    get(m_pRBFormulaCell, "formulabutton");
    m_pRBFormulaCell->SetReferences(this, m_pEdFormulaCell),
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

//----------------------------------------------------------------------------

ScSolverDlg::~ScSolverDlg()
{
}

//----------------------------------------------------------------------------

void ScSolverDlg::Init()
{
    String          aStr;

    m_pBtnOk->SetClickHdl( LINK( this, ScSolverDlg, BtnHdl ) );
    m_pBtnCancel->SetClickHdl( LINK( this, ScSolverDlg, BtnHdl ) );

    Link aLink = LINK( this, ScSolverDlg, GetFocusHdl );
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

    theFormulaCell.Format( aStr, SCA_ABS, NULL, pDoc->GetAddressConvention() );

    m_pEdFormulaCell->SetText( aStr );
    m_pEdFormulaCell->GrabFocus();
    pEdActive = m_pEdFormulaCell;
}

//----------------------------------------------------------------------------

sal_Bool ScSolverDlg::Close()
{
    return DoClose( ScSolverDlgWrapper::GetChildWindowId() );
}

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

void ScSolverDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if( pEdActive )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(pEdActive);

        String      aStr;
        ScAddress   aAdr = rRef.aStart;
        sal_uInt16      nFmt = ( aAdr.Tab() == nCurTab )
                                ? SCA_ABS
                                : SCA_ABS_3D;

        aAdr.Format( aStr, nFmt, pDocP, pDocP->GetAddressConvention() );
        pEdActive->SetRefString( aStr );

        if ( pEdActive == m_pEdFormulaCell )
            theFormulaCell = aAdr;
        else if ( pEdActive == m_pEdVariableCell )
            theVariableCell = aAdr;
    }
}

//----------------------------------------------------------------------------

void ScSolverDlg::RaiseError( ScSolverErr eError )
{
    switch ( eError )
    {
        case SOLVERR_NOFORMULA:
            lclErrorDialog( this, errMsgNoFormula );
            m_pEdFormulaCell->GrabFocus();
            break;

        case SOLVERR_INVALID_FORMULA:
            lclErrorDialog( this, errMsgInvalidForm );
            m_pEdFormulaCell->GrabFocus();
            break;

        case SOLVERR_INVALID_VARIABLE:
            lclErrorDialog( this, errMsgInvalidVar );
            m_pEdVariableCell->GrabFocus();
            break;

        case SOLVERR_INVALID_TARGETVALUE:
            lclErrorDialog( this, errMsgInvalidVal );
            m_pEdTargetVal->GrabFocus();
            break;
    }
}

//----------------------------------------------------------------------------

sal_Bool ScSolverDlg::IsRefInputMode() const
{
    return pEdActive != NULL;
}

//----------------------------------------------------------------------------

sal_Bool ScSolverDlg::CheckTargetValue( String& rStrVal )
{
    sal_uInt32 n1 = 0;
    double n2;

    return pDoc->GetFormatTable()->IsNumberFormat( rStrVal, n1, n2 );
}

//----------------------------------------------------------------------------
// Handler:

IMPL_LINK( ScSolverDlg, BtnHdl, PushButton*, pBtn )
{
    if (pBtn == m_pBtnOk)
    {
        theTargetValStr = m_pEdTargetVal->GetText();

        // Zu ueberpruefen:
        // 1. enthalten die Strings korrekte Tabellenkoordinaten/def.Namen?
        // 2. verweist die Formel-Koordinate wirklich auf eine Formelzelle?
        // 3. wurde ein korrekter Zielwert eingegeben

        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        sal_uInt16  nRes1 = theFormulaCell .Parse( m_pEdFormulaCell->GetText(),  pDoc, eConv );
        sal_uInt16  nRes2 = theVariableCell.Parse( m_pEdVariableCell->GetText(), pDoc, eConv );

        if ( SCA_VALID == ( nRes1 & SCA_VALID ) )
        {
            if ( SCA_VALID == ( nRes2 & SCA_VALID ) )
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
                        GetBindings().GetDispatcher()->Execute( SID_SOLVE,
                                                  SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                                  &aOutItem, 0L, 0L );
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

    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScSolverDlg, GetFocusHdl, Control*, pCtrl )
{
    Edit* pEdit = NULL;
    pEdActive = NULL;

    if( (pCtrl == (Control*)m_pEdFormulaCell) || (pCtrl == (Control*)m_pRBFormulaCell) )
        pEdit = pEdActive = m_pEdFormulaCell;
    else if( (pCtrl == (Control*)m_pEdVariableCell) || (pCtrl == (Control*)m_pRBVariableCell) )
        pEdit = pEdActive = m_pEdVariableCell;
    else if( pCtrl == (Control*)m_pEdTargetVal )
        pEdit = m_pEdTargetVal;

    if( pEdit )
        pEdit->SetSelection( Selection( 0, SELECTION_MAX ) );

    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK_NOARG(ScSolverDlg, LoseFocusHdl)
{
    bDlgLostFocus = !IsActive();
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
