/*************************************************************************
 *
 *  $RCSfile: tabopdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:42:21 $
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

//----------------------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>

#include "uiitems.hxx"
#include "global.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "reffact.hxx"
#include "tabopdlg.hrc"

#define _TABOPDLG_CXX
#include "tabopdlg.hxx"


//============================================================================
//  class ScTabOpDlg
//----------------------------------------------------------------------------

ScTabOpDlg::ScTabOpDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                        ScDocument*         pDocument,
                        const ScRefTripel&  rCursorPos )

    :   ScAnyRefDlg         ( pB, pCW, pParent, RID_SCDLG_TABOP ),
        //
        aFtFormulaRange     ( this, ScResId( FT_FORMULARANGE ) ),
        aEdFormulaRange     ( this, ScResId( ED_FORMULARANGE ) ),
        aRBFormulaRange     ( this, ScResId( RB_FORMULARANGE ), &aEdFormulaRange ),
        aFtRowCell          ( this, ScResId( FT_ROWCELL ) ),
        aEdRowCell          ( this, ScResId( ED_ROWCELL ) ),
        aRBRowCell          ( this, ScResId( RB_ROWCELL ), &aEdRowCell ),
        aFtColCell          ( this, ScResId( FT_COLCELL ) ),
        aEdColCell          ( this, ScResId( ED_COLCELL ) ),
        aRBColCell          ( this, ScResId( RB_COLCELL ), &aEdColCell ),
        aGbVariables        ( this, ScResId( GB_VARIABLES ) ),
        aBtnOk              ( this, ScResId( BTN_OK ) ),
        aBtnCancel          ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp            ( this, ScResId( BTN_HELP ) ),
        errMsgNoFormula     ( ScResId( STR_NOFORMULA ) ),
        errMsgNoColRow      ( ScResId( STR_NOCOLROW ) ),
        errMsgWrongFormula  ( ScResId( STR_WRONGFORMULA ) ),
        errMsgWrongRowCol   ( ScResId( STR_WRONGROWCOL ) ),
        errMsgNoColFormula  ( ScResId( STR_NOCOLFORMULA ) ),
        errMsgNoRowFormula  ( ScResId( STR_NOROWFORMULA ) ),
        //
        pDoc                ( pDocument ),
        theFormulaCell      ( rCursorPos ),
        nCurTab             ( theFormulaCell.GetTab() ),
        pEdActive           ( NULL ),
        bDlgLostFocus       ( FALSE )
{
    Init();
    FreeResource();
}

//----------------------------------------------------------------------------

__EXPORT ScTabOpDlg::~ScTabOpDlg()
{
    Hide();
}

//----------------------------------------------------------------------------

void __EXPORT ScTabOpDlg::Init()
{
    aBtnOk.         SetClickHdl     ( LINK( this, ScTabOpDlg, BtnHdl ) );
    aBtnCancel.     SetClickHdl     ( LINK( this, ScTabOpDlg, BtnHdl ) );
    aEdFormulaRange.SetGetFocusHdl  ( LINK( this, ScTabOpDlg, EdGetFocusHdl ) );
    aEdRowCell.     SetGetFocusHdl  ( LINK( this, ScTabOpDlg, EdGetFocusHdl ) );
    aEdColCell.     SetGetFocusHdl  ( LINK( this, ScTabOpDlg, EdGetFocusHdl ) );
    aEdFormulaRange.SetLoseFocusHdl ( LINK( this, ScTabOpDlg, EdLoseFocusHdl ) );
    aEdRowCell.     SetLoseFocusHdl ( LINK( this, ScTabOpDlg, EdLoseFocusHdl ) );
    aEdColCell.     SetLoseFocusHdl ( LINK( this, ScTabOpDlg, EdLoseFocusHdl ) );

    aEdFormulaRange.GrabFocus();
    pEdActive = &aEdFormulaRange;

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();
}

//----------------------------------------------------------------------------

BOOL __EXPORT ScTabOpDlg::Close()
{
    return DoClose( ScTabOpDlgWrapper::GetChildWindowId() );
}

//----------------------------------------------------------------------------

void ScTabOpDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = FALSE;
        pEdActive->GrabFocus();
    }
    else
        GrabFocus();

    RefInputDone();
}

//----------------------------------------------------------------------------

void ScTabOpDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( pEdActive )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(pEdActive);

        USHORT nStartCol = rRef.aStart.Col();       // fuer RefTripel
        USHORT nStartRow = rRef.aStart.Row();
        USHORT nStartTab = rRef.aStart.Tab();
        USHORT nEndCol = rRef.aEnd.Col();
        USHORT nEndRow = rRef.aEnd.Row();
        USHORT nEndTab = rRef.aEnd.Tab();

        String      aStr;
        USHORT      nFmt = ( nStartTab == nCurTab )
                                ? SCR_ABS
                                : SCR_ABS_3D;

        if ( pEdActive == &aEdFormulaRange )
        {
            theFormulaCell = ScRefTripel( nStartCol, nStartRow, nStartTab, FALSE, FALSE, FALSE );
            theFormulaEnd  = ScRefTripel( nEndCol,   nEndRow,   nEndTab,   FALSE, FALSE, FALSE );
            rRef.Format( aStr, nFmt, pDoc );
        }
        else if ( pEdActive == &aEdRowCell )
        {
            theRowCell = ScRefTripel( nStartCol, nStartRow, nStartTab, FALSE, FALSE, FALSE );
            rRef.aStart.Format( aStr, nFmt, pDoc );
        }
        else if ( pEdActive == &aEdColCell )
        {
            theColCell = ScRefTripel( nStartCol, nStartRow, nStartTab, FALSE, FALSE, FALSE );
            rRef.aStart.Format( aStr, nFmt, pDoc );
        }

        pEdActive->SetRefString( aStr );
    }
}

//----------------------------------------------------------------------------

void ScTabOpDlg::RaiseError( ScTabOpErr eError )
{
    const String* pMsg = &errMsgNoFormula;
    Edit*         pEd  = &aEdFormulaRange;

    switch ( eError )
    {
        case TABOPERR_NOFORMULA:
            pMsg = &errMsgNoFormula;
            pEd  = &aEdFormulaRange;
            break;

        case TABOPERR_NOCOLROW:
            pMsg = &errMsgNoColRow;
            pEd  = &aEdRowCell;
            break;

        case TABOPERR_WRONGFORMULA:
            pMsg = &errMsgWrongFormula;
            pEd  = &aEdFormulaRange;
            break;

        case TABOPERR_WRONGROW:
            pMsg = &errMsgWrongRowCol;
            pEd  = &aEdRowCell;
            break;

        case TABOPERR_NOCOLFORMULA:
            pMsg = &errMsgNoColFormula;
            pEd  = &aEdFormulaRange;
            break;

        case TABOPERR_WRONGCOL:
            pMsg = &errMsgWrongRowCol;
            pEd  = &aEdColCell;
            break;

        case TABOPERR_NOROWFORMULA:
            pMsg = &errMsgNoRowFormula;
            pEd  = &aEdFormulaRange;
            break;
    }

    ErrorBox( this, WinBits( WB_OK_CANCEL | WB_DEF_OK), *pMsg ).Execute();
    pEd->GrabFocus();
}

//----------------------------------------------------------------------------

BOOL lcl_Parse( const String& rString, ScDocument* pDoc, USHORT nCurTab,
                ScRefTripel& rStart, ScRefTripel& rEnd )
{
    BOOL bRet = FALSE;
    if ( rString.Search(':') != STRING_NOTFOUND )
        bRet = ConvertDoubleRef( pDoc, rString, nCurTab, rStart, rEnd );
    else
    {
        bRet = ConvertSingleRef( pDoc, rString, nCurTab, rStart );
        rEnd = rStart;
    }
    return bRet;
}

//----------------------------------------------------------------------------
// Handler:

IMPL_LINK( ScTabOpDlg, BtnHdl, PushButton*, pBtn )
{
    if ( pBtn == &aBtnOk )
    {
        BYTE nMode = 3;
        USHORT nError = 0;

        // Zu ueberpruefen:
        // 1. enthalten die Strings korrekte Tabellenkoordinaten/def.Namen?
        // 2. IstFormelRang Zeile bei leerer Zeile bzw. Spalte bei leerer Spalte
        //    bzw. Einfachreferenz bei beidem?
        // 3. Ist mindestens Zeile oder Spalte und Formel voll?

        if (aEdFormulaRange.GetText().Len() == 0)
            nError = TABOPERR_NOFORMULA;
        else if (aEdRowCell.GetText().Len() == 0 &&
                 aEdColCell.GetText().Len() == 0)
            nError = TABOPERR_NOCOLROW;
        else if ( !lcl_Parse( aEdFormulaRange.GetText(), pDoc, nCurTab,
                                theFormulaCell, theFormulaEnd ) )
            nError = TABOPERR_WRONGFORMULA;
        else
        {
            if (aEdRowCell.GetText().Len() > 0)
            {
                if (!ConvertSingleRef( pDoc, aEdRowCell.GetText(), nCurTab, theRowCell ))
                    nError = TABOPERR_WRONGROW;
                else
                {
                    if (aEdColCell.GetText().Len() == 0 &&
                        theFormulaCell.GetCol() != theFormulaEnd.GetCol())
                        nError = TABOPERR_NOCOLFORMULA;
                    else
                        nMode = 1;
                }
            }
            if (aEdColCell.GetText().Len() > 0)
            {
                if (!ConvertSingleRef( pDoc, aEdColCell.GetText(), nCurTab,
                                       theColCell ))
                    nError = TABOPERR_WRONGCOL;
                else
                {
                    if (nMode == 1)                         // beides
                    {
                        nMode = 2;
                        ConvertSingleRef( pDoc, aEdFormulaRange.GetText(), nCurTab,
                                          theFormulaCell );
                    }
                    else if (theFormulaCell.GetRow() != theFormulaEnd.GetRow())
                        nError = TABOPERR_NOROWFORMULA;
                    else
                        nMode = 0;
                }
            }
        }

        if (nError)
            RaiseError( (ScTabOpErr) nError );
        else
        {
            ScTabOpParam aOutParam( theFormulaCell,
                                    theFormulaEnd,
                                    theRowCell,
                                    theColCell,
                                    nMode );
            ScTabOpItem  aOutItem( SID_TABOP, &aOutParam );

            SFX_APP()->LockDispatcher( FALSE );
            SwitchToDocument();
            GetBindings().GetDispatcher()->Execute( SID_TABOP,
                                      SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                      &aOutItem, 0L, 0L );
            Close();
        }
    }
    else if ( pBtn == &aBtnCancel )
        Close();

    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScTabOpDlg, EdGetFocusHdl, ScRefEdit*, pEd )
{
    pEdActive       = pEd;
    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();
    pEd->SetSelection( Selection(0,SELECTION_MAX) );
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScTabOpDlg, EdLoseFocusHdl, ScRefEdit*, pEd )
{
    bDlgLostFocus = !IsActive();
    return 0;
}





