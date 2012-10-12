/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
                        const ScRefAddress& rCursorPos )

    :   ScAnyRefDlg         ( pB, pCW, pParent, RID_SCDLG_TABOP ),
        //
        aFlVariables        ( this, ScResId( FL_VARIABLES ) ),
        aFtFormulaRange     ( this, ScResId( FT_FORMULARANGE ) ),
        aEdFormulaRange     ( this, this, ScResId( ED_FORMULARANGE ) ),
        aRBFormulaRange     ( this, ScResId( RB_FORMULARANGE ), &aEdFormulaRange, this ),
        aFtRowCell          ( this, ScResId( FT_ROWCELL ) ),
        aEdRowCell          ( this, this, ScResId( ED_ROWCELL ) ),
        aRBRowCell          ( this, ScResId( RB_ROWCELL ), &aEdRowCell, this ),
        aFtColCell          ( this, ScResId( FT_COLCELL ) ),
        aEdColCell          ( this, this, ScResId( ED_COLCELL ) ),
        aRBColCell          ( this, ScResId( RB_COLCELL ), &aEdColCell, this ),
        aBtnOk              ( this, ScResId( BTN_OK ) ),
        aBtnCancel          ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp            ( this, ScResId( BTN_HELP ) ),
        //
        theFormulaCell      ( rCursorPos ),
        pDoc                ( pDocument ),
        nCurTab             ( theFormulaCell.Tab() ),
        pEdActive           ( NULL ),
        bDlgLostFocus       ( false ),
        errMsgNoFormula     ( ScResId( STR_NOFORMULA ) ),
        errMsgNoColRow      ( ScResId( STR_NOCOLROW ) ),
        errMsgWrongFormula  ( ScResId( STR_WRONGFORMULA ) ),
        errMsgWrongRowCol   ( ScResId( STR_WRONGROWCOL ) ),
        errMsgNoColFormula  ( ScResId( STR_NOCOLFORMULA ) ),
        errMsgNoRowFormula  ( ScResId( STR_NOROWFORMULA ) )
{
    Init();
    FreeResource();
}

//----------------------------------------------------------------------------

ScTabOpDlg::~ScTabOpDlg()
{
    Hide();
}

//----------------------------------------------------------------------------

void ScTabOpDlg::Init()
{
    aBtnOk.         SetClickHdl     ( LINK( this, ScTabOpDlg, BtnHdl ) );
    aBtnCancel.     SetClickHdl     ( LINK( this, ScTabOpDlg, BtnHdl ) );

    Link aLink = LINK( this, ScTabOpDlg, GetFocusHdl );
    aEdFormulaRange.SetGetFocusHdl( aLink );
    aRBFormulaRange.SetGetFocusHdl( aLink );
    aEdRowCell.     SetGetFocusHdl( aLink );
    aRBRowCell.     SetGetFocusHdl( aLink );
    aEdColCell.     SetGetFocusHdl( aLink );
    aRBColCell.     SetGetFocusHdl( aLink );

    aLink = LINK( this, ScTabOpDlg, LoseFocusHdl );
    aEdFormulaRange.SetLoseFocusHdl( aLink );
    aRBFormulaRange.SetLoseFocusHdl( aLink );
    aEdRowCell.     SetLoseFocusHdl( aLink );
    aRBRowCell.     SetLoseFocusHdl( aLink );
    aEdColCell.     SetLoseFocusHdl( aLink );
    aRBColCell.     SetLoseFocusHdl( aLink );

    aEdFormulaRange.GrabFocus();
    pEdActive = &aEdFormulaRange;

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();
}

//----------------------------------------------------------------------------

sal_Bool ScTabOpDlg::Close()
{
    return DoClose( ScTabOpDlgWrapper::GetChildWindowId() );
}

//----------------------------------------------------------------------------

void ScTabOpDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;
        if( pEdActive )
            pEdActive->GrabFocus();
    }
    else
        GrabFocus();

    RefInputDone();
}

//----------------------------------------------------------------------------

void ScTabOpDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( pEdActive )
    {
        ScAddress::Details aDetails(pDocP->GetAddressConvention(), 0, 0);

        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(pEdActive);

        String      aStr;
        sal_uInt16      nFmt = ( rRef.aStart.Tab() == nCurTab )
                                ? SCR_ABS
                                : SCR_ABS_3D;

        if ( pEdActive == &aEdFormulaRange )
        {
            theFormulaCell.Set( rRef.aStart, false, false, false);
            theFormulaEnd.Set( rRef.aEnd, false, false, false);
            rRef.Format( aStr, nFmt, pDocP, aDetails );
        }
        else if ( pEdActive == &aEdRowCell )
        {
            theRowCell.Set( rRef.aStart, false, false, false);
            rRef.aStart.Format( aStr, nFmt, pDocP, aDetails );
        }
        else if ( pEdActive == &aEdColCell )
        {
            theColCell.Set( rRef.aStart, false, false, false);
            rRef.aStart.Format( aStr, nFmt, pDocP, aDetails );
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

static sal_Bool lcl_Parse( const String& rString, ScDocument* pDoc, SCTAB nCurTab,
                ScRefAddress& rStart, ScRefAddress& rEnd )
{
    sal_Bool bRet = false;
    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    if ( rString.Search(':') != STRING_NOTFOUND )
        bRet = ConvertDoubleRef( pDoc, rString, nCurTab, rStart, rEnd, eConv );
    else
    {
        bRet = ConvertSingleRef( pDoc, rString, nCurTab, rStart, eConv );
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
        sal_uInt8 nMode = 3;
        sal_uInt16 nError = 0;

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
            const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
            if (aEdRowCell.GetText().Len() > 0)
            {
                if (!ConvertSingleRef( pDoc, aEdRowCell.GetText(), nCurTab,
                                       theRowCell, eConv ))
                    nError = TABOPERR_WRONGROW;
                else
                {
                    if (aEdColCell.GetText().Len() == 0 &&
                        theFormulaCell.Col() != theFormulaEnd.Col())
                        nError = TABOPERR_NOCOLFORMULA;
                    else
                        nMode = 1;
                }
            }
            if (aEdColCell.GetText().Len() > 0)
            {
                if (!ConvertSingleRef( pDoc, aEdColCell.GetText(), nCurTab,
                                       theColCell, eConv ))
                    nError = TABOPERR_WRONGCOL;
                else
                {
                    if (nMode == 1)                         // beides
                    {
                        nMode = 2;
                        ConvertSingleRef( pDoc, aEdFormulaRange.GetText(), nCurTab,
                                          theFormulaCell, eConv );
                    }
                    else if (theFormulaCell.Row() != theFormulaEnd.Row())
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

            SetDispatcherLock( false );
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

IMPL_LINK( ScTabOpDlg, GetFocusHdl, Control*, pCtrl )
{
    if( (pCtrl == (Control*)&aEdFormulaRange) || (pCtrl == (Control*)&aRBFormulaRange) )
        pEdActive = &aEdFormulaRange;
    else if( (pCtrl == (Control*)&aEdRowCell) || (pCtrl == (Control*)&aRBRowCell) )
        pEdActive = &aEdRowCell;
    else if( (pCtrl == (Control*)&aEdColCell) || (pCtrl == (Control*)&aRBColCell) )
        pEdActive = &aEdColCell;
    else
        pEdActive = NULL;

    if( pEdActive )
        pEdActive->SetSelection( Selection( 0, SELECTION_MAX ) );

    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK_NOARG(ScTabOpDlg, LoseFocusHdl)
{
    bDlgLostFocus = !IsActive();
    return 0;
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
