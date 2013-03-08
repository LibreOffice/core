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
#include "solvrdlg.hrc"

#define _SOLVRDLG_CXX
#include "solvrdlg.hxx"
#undef _SOLVERDLG_CXX


#define ERRORBOX(s) ErrorBox( this, WinBits( WB_OK | WB_DEF_OK), s ).Execute()


//============================================================================
//  class ScSolverDlg
//----------------------------------------------------------------------------

ScSolverDlg::ScSolverDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                          ScDocument* pDocument,
                          ScAddress aCursorPos )

    :   ScAnyRefDlg         ( pB, pCW, pParent, RID_SCDLG_SOLVER ),
        //
        aFlVariables        ( this, ScResId( FL_VARIABLES ) ),
        aFtFormulaCell      ( this, ScResId( FT_FORMULACELL ) ),
        aEdFormulaCell      ( this, this, ScResId( ED_FORMULACELL ) ),
        aRBFormulaCell      ( this, ScResId( RB_FORMULACELL ), &aEdFormulaCell, &aFtFormulaCell, this ),
        aFtTargetVal        ( this, ScResId( FT_TARGETVAL ) ),
        aEdTargetVal        ( this, ScResId( ED_TARGETVAL ) ),
        aFtVariableCell     ( this, ScResId( FT_VARCELL ) ),
        aEdVariableCell     ( this, this, ScResId( ED_VARCELL ) ),
        aRBVariableCell     ( this, ScResId( RB_VARCELL ), &aEdVariableCell, &aFtVariableCell, this ),
        aBtnOk              ( this, ScResId( BTN_OK ) ),
        aBtnCancel          ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp            ( this, ScResId( BTN_HELP ) ),
        //
        theFormulaCell      ( aCursorPos ),
        theVariableCell     ( aCursorPos ),
        pDoc                ( pDocument ),
        nCurTab             ( aCursorPos.Tab() ),
        pEdActive           ( NULL ),
        bDlgLostFocus       ( false ),
        errMsgInvalidVar    ( ScResId( STR_INVALIDVAR ) ),
        errMsgInvalidForm   ( ScResId( STR_INVALIDFORM ) ),
        errMsgNoFormula     ( ScResId( STR_NOFORMULA ) ),
        errMsgInvalidVal    ( ScResId( STR_INVALIDVAL ) )
{
    Init();
    FreeResource();

    aRBFormulaCell.SetAccessibleRelationMemberOf(&aFlVariables);
    aRBVariableCell.SetAccessibleRelationMemberOf(&aFlVariables);
}

//----------------------------------------------------------------------------

ScSolverDlg::~ScSolverDlg()
{
}

//----------------------------------------------------------------------------

void ScSolverDlg::Init()
{
    String          aStr;

    aBtnOk.         SetClickHdl     ( LINK( this, ScSolverDlg, BtnHdl ) );
    aBtnCancel.     SetClickHdl     ( LINK( this, ScSolverDlg, BtnHdl ) );

    Link aLink = LINK( this, ScSolverDlg, GetFocusHdl );
    aEdFormulaCell. SetGetFocusHdl  ( aLink );
    aRBFormulaCell. SetGetFocusHdl  ( aLink );
    aEdVariableCell.SetGetFocusHdl  ( aLink );
    aRBVariableCell.SetGetFocusHdl  ( aLink );
    aEdTargetVal.   SetGetFocusHdl  ( aLink );

    aLink = LINK( this, ScSolverDlg, LoseFocusHdl );
    aEdFormulaCell. SetLoseFocusHdl ( aLink );
    aRBFormulaCell. SetLoseFocusHdl ( aLink );
    aEdVariableCell.SetLoseFocusHdl ( aLink );
    aRBVariableCell.SetLoseFocusHdl ( aLink );

    theFormulaCell.Format( aStr, SCA_ABS, NULL, pDoc->GetAddressConvention() );

    aEdFormulaCell.SetText( aStr );
    aEdFormulaCell.GrabFocus();
    pEdActive = &aEdFormulaCell;
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

        if ( pEdActive == &aEdFormulaCell )
            theFormulaCell = aAdr;
        else if ( pEdActive == &aEdVariableCell )
            theVariableCell = aAdr;
    }
}

//----------------------------------------------------------------------------

void ScSolverDlg::RaiseError( ScSolverErr eError )
{
    switch ( eError )
    {
        case SOLVERR_NOFORMULA:
            ERRORBOX( errMsgNoFormula );
            aEdFormulaCell.GrabFocus();
            break;

        case SOLVERR_INVALID_FORMULA:
            ERRORBOX( errMsgInvalidForm );
            aEdFormulaCell.GrabFocus();
            break;

        case SOLVERR_INVALID_VARIABLE:
            ERRORBOX( errMsgInvalidVar );
            aEdVariableCell.GrabFocus();
            break;

        case SOLVERR_INVALID_TARGETVALUE:
            ERRORBOX( errMsgInvalidVal );
            aEdTargetVal.GrabFocus();
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
    if ( pBtn == &aBtnOk )
    {
        theTargetValStr = aEdTargetVal.GetText();

        // Zu ueberpruefen:
        // 1. enthalten die Strings korrekte Tabellenkoordinaten/def.Namen?
        // 2. verweist die Formel-Koordinate wirklich auf eine Formelzelle?
        // 3. wurde ein korrekter Zielwert eingegeben

        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        sal_uInt16  nRes1 = theFormulaCell .Parse( aEdFormulaCell.GetText(),  pDoc, eConv );
        sal_uInt16  nRes2 = theVariableCell.Parse( aEdVariableCell.GetText(), pDoc, eConv );

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
    else if ( pBtn == &aBtnCancel )
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

    if( (pCtrl == (Control*)&aEdFormulaCell) || (pCtrl == (Control*)&aRBFormulaCell) )
        pEdit = pEdActive = &aEdFormulaCell;
    else if( (pCtrl == (Control*)&aEdVariableCell) || (pCtrl == (Control*)&aRBVariableCell) )
        pEdit = pEdActive = &aEdVariableCell;
    else if( pCtrl == (Control*)&aEdTargetVal )
        pEdit = &aEdTargetVal;

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
