/*************************************************************************
 *
 *  $RCSfile: solvrdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SOT_DTRANS_HXX
#include <sot/dtrans.hxx>
#endif

//----------------------------------------------------------------------------

#include "rangelst.hxx"
#include "scitems.hxx"
#include <sfx2/dispatch.hxx>
#include <svtools/zforlist.hxx>
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
        aFtFormulaCell      ( this, ScResId( FT_FORMULACELL ) ),
        aEdFormulaCell      ( this, ScResId( ED_FORMULACELL ) ),
        aRBFormulaCell      ( this, ScResId( RB_FORMULACELL ), &aEdFormulaCell ),
        aFtTargetVal        ( this, ScResId( FT_TARGETVAL ) ),
        aEdTargetVal        ( this, ScResId( ED_TARGETVAL ) ),
        aFtVariableCell     ( this, ScResId( FT_VARCELL ) ),
        aEdVariableCell     ( this, ScResId( ED_VARCELL ) ),
        aRBVariableCell     ( this, ScResId( RB_VARCELL ), &aEdVariableCell ),
        aGbVariables        ( this, ScResId( GB_VARIABLES ) ),
        aBtnOk              ( this, ScResId( BTN_OK ) ),
        aBtnCancel          ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp            ( this, ScResId( BTN_HELP ) ),
        errMsgInvalidVal    ( ScResId( STR_INVALIDVAL ) ),
        errMsgInvalidVar    ( ScResId( STR_INVALIDVAR ) ),
        errMsgInvalidForm   ( ScResId( STR_INVALIDFORM ) ),
        errMsgNoFormula     ( ScResId( STR_NOFORMULA ) ),
        //
        pDoc                ( pDocument ),
        theFormulaCell      ( aCursorPos ),
        theVariableCell     ( aCursorPos ),
        nCurTab             ( aCursorPos.Tab() ),
        pEdActive           ( NULL ),
        bDlgLostFocus       ( FALSE )
{
    Init();
    FreeResource();
}

//----------------------------------------------------------------------------

__EXPORT ScSolverDlg::~ScSolverDlg()
{
}

//----------------------------------------------------------------------------

void __EXPORT ScSolverDlg::Init()
{
    String          aStr;

    aBtnOk.         SetClickHdl     ( LINK( this, ScSolverDlg, BtnHdl ) );
    aBtnCancel.     SetClickHdl     ( LINK( this, ScSolverDlg, BtnHdl ) );
    aEdFormulaCell. SetGetFocusHdl  ( LINK( this, ScSolverDlg, EdGetFocusHdl ) );
    aEdVariableCell.SetGetFocusHdl  ( LINK( this, ScSolverDlg, EdGetFocusHdl ) );
    aEdTargetVal.   SetGetFocusHdl  ( LINK( this, ScSolverDlg, EdGetFocusHdl ) );
    aEdFormulaCell. SetLoseFocusHdl ( LINK( this, ScSolverDlg, EdLoseFocusHdl ) );
    aEdVariableCell.SetLoseFocusHdl ( LINK( this, ScSolverDlg, EdLoseFocusHdl ) );

    theFormulaCell.Format( aStr, SCA_ABS );

    aEdFormulaCell.SetText( aStr );
    aEdFormulaCell.GrabFocus();
    pEdActive = &aEdFormulaCell;
    bMouseInputMode = TRUE;

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();
}

//----------------------------------------------------------------------------

BOOL __EXPORT ScSolverDlg::Close()
{
    return DoClose( ScSolverDlgWrapper::GetChildWindowId() );
}

//----------------------------------------------------------------------------

void ScSolverDlg::SetActive()
{
    if ( bDlgLostFocus && bMouseInputMode )
    {
        bDlgLostFocus = FALSE;
        pEdActive->GrabFocus();
    }
    else
    {
        GrabFocus();
    }
    RefInputDone();
}

//----------------------------------------------------------------------------

void ScSolverDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( bMouseInputMode && pEdActive )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(pEdActive);

        String      aStr;
        ScAddress   aAdr = rRef.aStart;
        USHORT      nFmt = ( aAdr.Tab() == nCurTab )
                                ? SCA_ABS
                                : SCA_ABS_3D;

        aAdr.Format( aStr, nFmt, pDoc );
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

BOOL __EXPORT ScSolverDlg::CheckTargetValue( String& rStrVal )
{
    ULONG  n1 = 0;
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

        USHORT  nRes1 = theFormulaCell .Parse( aEdFormulaCell.GetText(), pDoc );
        USHORT  nRes2 = theVariableCell.Parse( aEdVariableCell.GetText(), pDoc );

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

                        SFX_APP()->LockDispatcher( FALSE );

                        SwitchToDocument();
                        SFX_DISPATCHER().Execute( SID_SOLVE,
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

    return NULL;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScSolverDlg, EdGetFocusHdl, ScRefEdit*, pEd )
{
    pEdActive       = pEd;
    bMouseInputMode = ( pEd != &aEdTargetVal );

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    /*
    if ( bMouseInputMode )
        SFX_APPWINDOW->Enable();
    else
        SFX_APPWINDOW->Disable(FALSE);      //! allgemeine Methode im ScAnyRefDlg
    */
    pEd->SetSelection( Selection(0,SELECTION_MAX) );

    return NULL;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScSolverDlg, EdLoseFocusHdl, ScRefEdit*, pEd )
{
    bDlgLostFocus = !IsActive();
    return NULL;
}




