/*************************************************************************
 *
 *  $RCSfile: condfrmt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:53 $
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

#ifndef PCH
#include <segmentc.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#include "tabvwsh.hxx"
#include "reffact.hxx"
#include "conditio.hxx"
#include "stlpool.hxx"
#include "uiitems.hxx"
#include "document.hxx"
#include "scresid.hxx"

#include "condfrmt.hrc"
#include "globstr.hrc"

#define _CONDFRMT_CXX
#include "condfrmt.hxx"
#undef _CONDFRMT_CXX


SEG_EOFGLOBALS()


//============================================================================
//  class ScConditionalFormat

//----------------------------------------------------------------------------
// Konstruktor
#pragma SEG_FUNCDEF(condfrmt_01)

ScConditionalFormatDlg::ScConditionalFormatDlg(
            SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
            ScDocument* pCurDoc,
            const ScConditionalFormat* pCurrentFormat )
    :   ScAnyRefDlg         ( pB, pCW, pParent, RID_SCDLG_CONDFORMAT ),

        pDoc                ( pCurDoc ),

        aCbxCond1           ( this, ScResId( CBX_COND1 ) ),
        aLbCond11           ( this, ScResId( LB_COND1_1 ) ),
        aLbCond12           ( this, ScResId( LB_COND1_2 ) ),
        aEdtCond11          ( this, ScResId( EDT_COND1_1 ) ),
        aRbCond11           ( this, ScResId( RB_COND1_1 ), &aEdtCond11 ),
        aFtCond1And         ( this, ScResId( FT_COND1_AND ) ),
        aEdtCond12          ( this, ScResId( EDT_COND1_2 ) ),
        aRbCond12           ( this, ScResId( RB_COND1_2 ), &aEdtCond12 ),
        aFtCond1Template    ( this, ScResId( FT_COND1_TEMPLATE ) ),
        aLbCond1Template    ( this, ScResId( LB_COND1_TEMPLATE ) ),

        aCbxCond2           ( this, ScResId( CBX_COND2 ) ),
        aLbCond21           ( this, ScResId( LB_COND2_1 ) ),
        aLbCond22           ( this, ScResId( LB_COND2_2 ) ),
        aEdtCond21          ( this, ScResId( EDT_COND2_1 ) ),
        aRbCond21           ( this, ScResId( RB_COND2_1 ), &aEdtCond21 ),
        aFtCond2And         ( this, ScResId( FT_COND2_AND ) ),
        aEdtCond22          ( this, ScResId( EDT_COND2_2 ) ),
        aRbCond22           ( this, ScResId( RB_COND2_2 ), &aEdtCond22 ),
        aFtCond2Template    ( this, ScResId( FT_COND2_TEMPLATE ) ),
        aLbCond2Template    ( this, ScResId( LB_COND2_TEMPLATE ) ),

        aCbxCond3           ( this, ScResId( CBX_COND3 ) ),
        aLbCond31           ( this, ScResId( LB_COND3_1 ) ),
        aLbCond32           ( this, ScResId( LB_COND3_2 ) ),
        aEdtCond31          ( this, ScResId( EDT_COND3_1 ) ),
        aRbCond31           ( this, ScResId( RB_COND3_1 ), &aEdtCond31 ),
        aFtCond3And         ( this, ScResId( FT_COND3_AND ) ),
        aEdtCond32          ( this, ScResId( EDT_COND3_2 ) ),
        aRbCond32           ( this, ScResId( RB_COND3_2 ), &aEdtCond32 ),
        aFtCond3Template    ( this, ScResId( FT_COND3_TEMPLATE ) ),
        aLbCond3Template    ( this, ScResId( LB_COND3_TEMPLATE ) ),

        aBtnOk              ( this, ScResId( BTN_OK ) ),
        aBtnCancel          ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp            ( this, ScResId( BTN_HELP ) ),
        pEdActive           ( NULL ),
        bDlgLostFocus       ( FALSE )
{
    Point aPos;
    String aName;
    SfxStyleSheetBase* pStyle;

    FreeResource();

    // Handler setzen
    aCbxCond1.SetClickHdl ( LINK( this, ScConditionalFormatDlg, ClickCond1Hdl ) );
    aLbCond11.SetSelectHdl( LINK( this, ScConditionalFormatDlg, ChangeCond11Hdl ) );
    aLbCond12.SetSelectHdl( LINK( this, ScConditionalFormatDlg, ChangeCond12Hdl ) );

    aCbxCond2.SetClickHdl ( LINK( this, ScConditionalFormatDlg, ClickCond2Hdl ) );
    aLbCond21.SetSelectHdl( LINK( this, ScConditionalFormatDlg, ChangeCond21Hdl ) );
    aLbCond22.SetSelectHdl( LINK( this, ScConditionalFormatDlg, ChangeCond22Hdl ) );

    aCbxCond3.SetClickHdl ( LINK( this, ScConditionalFormatDlg, ClickCond3Hdl ) );
    aLbCond31.SetSelectHdl( LINK( this, ScConditionalFormatDlg, ChangeCond31Hdl ) );
    aLbCond32.SetSelectHdl( LINK( this, ScConditionalFormatDlg, ChangeCond32Hdl ) );

    aBtnOk.SetClickHdl    ( LINK( this, ScConditionalFormatDlg, BtnHdl ) );
//? aBtnCancel.SetClickHdl( LINK( this, ScConditionalFormatDlg, BtnHdl ) );

    aEdtCond11.SetGetFocusHdl ( LINK( this, ScConditionalFormatDlg, EdGetFocusHdl ) );
    aEdtCond12.SetGetFocusHdl ( LINK( this, ScConditionalFormatDlg, EdGetFocusHdl ) );
    aEdtCond21.SetGetFocusHdl ( LINK( this, ScConditionalFormatDlg, EdGetFocusHdl ) );
    aEdtCond22.SetGetFocusHdl ( LINK( this, ScConditionalFormatDlg, EdGetFocusHdl ) );
    aEdtCond31.SetGetFocusHdl ( LINK( this, ScConditionalFormatDlg, EdGetFocusHdl ) );
    aEdtCond32.SetGetFocusHdl ( LINK( this, ScConditionalFormatDlg, EdGetFocusHdl ) );
    aEdtCond11.SetLoseFocusHdl ( LINK( this, ScConditionalFormatDlg, EdLoseFocusHdl ) );
    aEdtCond12.SetLoseFocusHdl ( LINK( this, ScConditionalFormatDlg, EdLoseFocusHdl ) );
    aEdtCond21.SetLoseFocusHdl ( LINK( this, ScConditionalFormatDlg, EdLoseFocusHdl ) );
    aEdtCond22.SetLoseFocusHdl ( LINK( this, ScConditionalFormatDlg, EdLoseFocusHdl ) );
    aEdtCond31.SetLoseFocusHdl ( LINK( this, ScConditionalFormatDlg, EdLoseFocusHdl ) );
    aEdtCond32.SetLoseFocusHdl ( LINK( this, ScConditionalFormatDlg, EdLoseFocusHdl ) );

    // Condition 1
    aCond1Pos1 = aLbCond12.GetPosPixel();       // Position Edit ohne Listbox
    aCond1Pos2 = aEdtCond11.GetPosPixel();      // Position Edit mit Listbox
    aRBtn1Pos1 = aRbCond11.GetPosPixel();
    aRBtn1Pos2 = aRbCond12.GetPosPixel();
    aPos = aEdtCond12.GetPosPixel();
    aPos.X() += aEdtCond12.GetSizePixel().Width();  // rechter Rand
    aCond1Size3 = aEdtCond11.GetSizePixel();
    aCond1Size2 = Size( aPos.X() - aCond1Pos2.X(), aCond1Size3.Height() );
    aCond1Size1 = Size( aPos.X() - aCond1Pos1.X(), aCond1Size3.Height() );

    aCbxCond1.Check();
    aLbCond11.SelectEntryPos( 0 );
    aLbCond12.SelectEntryPos( 0 );

    // Condition 2
    aCond2Pos1  = aLbCond22.GetPosPixel();      // Position Edit ohne Listbox
    aCond2Pos2  = aEdtCond21.GetPosPixel();     // Position Edit mit Listbox
    aRBtn2Pos1 = aRbCond21.GetPosPixel();
    aRBtn2Pos2 = aRbCond22.GetPosPixel();
    aPos = aEdtCond22.GetPosPixel();
    aPos.X() += aEdtCond22.GetSizePixel().Width();  // rechter Rand
    aCond2Size3 = aEdtCond21.GetSizePixel();
    aCond2Size2 = Size( aPos.X() - aCond2Pos2.X(), aCond2Size3.Height() );
    aCond2Size1 = Size( aPos.X() - aCond2Pos1.X(), aCond2Size3.Height() );

    aCbxCond2.Check( FALSE );
    aLbCond21.SelectEntryPos( 0 );
    aLbCond22.SelectEntryPos( 0 );

    // Condition 3
    aCond3Pos1  = aLbCond32.GetPosPixel();      // Position Edit ohne Listbox
    aCond3Pos2  = aEdtCond31.GetPosPixel();     // Position Edit mit Listbox
    aRBtn3Pos1 = aRbCond31.GetPosPixel();
    aRBtn3Pos2 = aRbCond32.GetPosPixel();
    aPos = aEdtCond32.GetPosPixel();
    aPos.X() += aEdtCond32.GetSizePixel().Width();  // rechter Rand
    aCond3Size3 = aEdtCond31.GetSizePixel();
    aCond3Size2 = Size( aPos.X() - aCond3Pos2.X(), aCond3Size3.Height() );
    aCond3Size1 = Size( aPos.X() - aCond3Pos1.X(), aCond3Size3.Height() );

    aCbxCond3.Check( FALSE );
    aLbCond31.SelectEntryPos( 0 );
    aLbCond32.SelectEntryPos( 0 );

    // Vorlagen aus pDoc holen
    SfxStyleSheetIterator aStyleIter( pDoc->GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
    for ( pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
    {
        aName = pStyle->GetName();
        aLbCond1Template.InsertEntry( aName );
        aLbCond2Template.InsertEntry( aName );
        aLbCond3Template.InsertEntry( aName );
    }

    // Vorlagen eintragen
//! pStyle = pDoc->GetSelectionStyle( /* ??? const ScMarkData& rMark ??? */ );
    pStyle = NULL;  //!
    if (pStyle)
        aName = pStyle->GetName();
    else
        aName = ScGlobal::GetRscString(STR_STYLENAME_STANDARD);
    aLbCond1Template.SelectEntry( aName );
    aLbCond2Template.SelectEntry( aName );
    aLbCond3Template.SelectEntry( aName );

    ScAddress aCurPos;
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        ScViewData* pData = pViewShell->GetViewData();
        aCurPos = ScAddress( pData->GetCurX(), pData->GetCurY(), pData->GetTabNo() );
    }

    // Inhalt aus ConditionalFormat holen
    if ( pCurrentFormat )
    {
        const ScCondFormatEntry* pEntry;
        if ( pCurrentFormat->Count() > 0 )
        {
            pEntry= pCurrentFormat->GetEntry( 0 );
            aEdtCond11.SetText( pEntry->GetExpression( aCurPos, 0 ) );
            aLbCond1Template.SelectEntry( pEntry->GetStyle() );

            ScConditionMode eMode = pEntry->GetOperation();
            if ( eMode == SC_COND_DIRECT )          // via Formel
            {
                aLbCond11.SelectEntryPos( 1 );
                ChangeCond11Hdl( NULL );
            }
            else if ( eMode == SC_COND_NONE )       // ???
                ;
            else                                    // via Werte
            {
                aLbCond12.SelectEntryPos( eMode );
                if ( ( eMode == SC_COND_BETWEEN ) || ( eMode == SC_COND_NOTBETWEEN ) )
                    aEdtCond12.SetText( pEntry->GetExpression( aCurPos, 1 ) );
            }
        }


        if ( pCurrentFormat->Count() > 1 )
        {
            aCbxCond2.Check( TRUE );
            pEntry= pCurrentFormat->GetEntry( 1 );
            aEdtCond21.SetText( pEntry->GetExpression( aCurPos, 0 ) );
            aLbCond2Template.SelectEntry( pEntry->GetStyle() );

            ScConditionMode eMode = pEntry->GetOperation();
            if ( eMode == SC_COND_DIRECT )          // via Formel
            {
                aLbCond21.SelectEntryPos( 1 );
                ChangeCond21Hdl( NULL );
            }
            else if ( eMode == SC_COND_NONE )       // ???
                ;
            else                                    // via Werte
            {
                aLbCond22.SelectEntryPos( eMode );
                if ( ( eMode == SC_COND_BETWEEN ) || ( eMode == SC_COND_NOTBETWEEN ) )
                    aEdtCond22.SetText( pEntry->GetExpression( aCurPos, 1 ) );
            }
        }

        if ( pCurrentFormat->Count() > 2 )
        {
            aCbxCond3.Check( TRUE );
            pEntry= pCurrentFormat->GetEntry( 2 );
            aEdtCond31.SetText( pEntry->GetExpression( aCurPos, 0 ) );
            aLbCond3Template.SelectEntry( pEntry->GetStyle() );

            ScConditionMode eMode = pEntry->GetOperation();
            if ( eMode == SC_COND_DIRECT )          // via Formel
            {
                aLbCond31.SelectEntryPos( 1 );
                ChangeCond31Hdl( NULL );
            }
            else if ( eMode == SC_COND_NONE )       // ???
                ;
            else                                    // via Werte
            {
                aLbCond32.SelectEntryPos( eMode );
                if ( ( eMode == SC_COND_BETWEEN ) || ( eMode == SC_COND_NOTBETWEEN ) )
                    aEdtCond32.SetText( pEntry->GetExpression( aCurPos, 1 ) );
            }
        }
    }

    ClickCond1Hdl( NULL );
    ClickCond2Hdl( NULL );
    ClickCond3Hdl( NULL );

    ChangeCond12Hdl( NULL );
    ChangeCond22Hdl( NULL );
    ChangeCond32Hdl( NULL );

    aEdtCond11.GrabFocus();
    pEdActive = &aEdtCond11;
    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();      // Ref-Feld hat Focus
//  SFX_APPWINDOW->Disable();
}


//----------------------------------------------------------------------------
// Destruktor
#pragma SEG_FUNCDEF(condfrmt_0d)

__EXPORT ScConditionalFormatDlg::~ScConditionalFormatDlg()
{
}


//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(condfrmt_0e)

void ScConditionalFormatDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( pEdActive )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(pEdActive);

        String aStr;
        rRef.Format( aStr, SCR_ABS_3D, pDoc );
        String aVal( pEdActive->GetText() );
        Selection aSel( pEdActive->GetSelection() );
        aVal.Erase( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
        aVal.Insert( aStr, (xub_StrLen)aSel.Min() );
        Selection aNewSel( aSel.Min(), aSel.Min()+aStr.Len() );
        pEdActive->SetRefString( aVal );
        pEdActive->SetSelection( aNewSel );
//      pEdActive->SetModifyFlag();
    }
}

//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(condfrmt_15)

void ScConditionalFormatDlg::AddRefEntry()
{
    if ( pEdActive )
    {
        String aVal = pEdActive->GetText();
        aVal += ';';
        pEdActive->SetText(aVal);

        xub_StrLen nLen = aVal.Len();
        pEdActive->SetSelection( Selection( nLen, nLen ) );
//      pEdActive->SetModifyFlag();
    }
}

//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(condfrmt_0f)

BOOL ScConditionalFormatDlg::IsRefInputMode()
{
    return (pEdActive != NULL);
}


//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(condfrmt_10)

void ScConditionalFormatDlg::SetActive()
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
// Holt die ausgewaehlte bedingte Formatierung ab
#pragma SEG_FUNCDEF(condfrmt_11)

void ScConditionalFormatDlg::GetConditionalFormat( ScConditionalFormat& rCndFmt )
{
    ScConditionMode eOper;
    String sExpr1;
    String sExpr2;
    String sStyle;
    ScAddress aCurPos;

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        ScViewData* pData = pViewShell->GetViewData();
        aCurPos = ScAddress( pData->GetCurX(), pData->GetCurY(), pData->GetTabNo() );
    }

    if ( aCbxCond1.IsChecked() )
    {
        if ( aLbCond11.GetSelectEntryPos() == 1 )   // via Formel
            eOper = SC_COND_DIRECT;
        else
            eOper = (ScConditionMode)aLbCond12.GetSelectEntryPos();
        sExpr1 = aEdtCond11.GetText();
        sExpr2 = aEdtCond12.GetText();
        sStyle = aLbCond1Template.GetSelectEntry();
        ScCondFormatEntry aNewEntry( eOper, sExpr1, sExpr2, pDoc, aCurPos, sStyle );
        rCndFmt.AddEntry( aNewEntry );
    }

    if ( aCbxCond2.IsChecked() )
    {
        if ( aLbCond21.GetSelectEntryPos() == 1 )   // via Formel???
            eOper = SC_COND_DIRECT;
        else
            eOper = (ScConditionMode)aLbCond22.GetSelectEntryPos();
        sExpr1 = aEdtCond21.GetText();
        sExpr2 = aEdtCond22.GetText();
        sStyle = aLbCond2Template.GetSelectEntry();
        ScCondFormatEntry aNewEntry( eOper, sExpr1, sExpr2, pDoc, aCurPos, sStyle );
        rCndFmt.AddEntry( aNewEntry );
    }

    if ( aCbxCond3.IsChecked() )
    {
        if ( aLbCond31.GetSelectEntryPos() == 1 )   // via Formel???
            eOper = SC_COND_DIRECT;
        else
            eOper = (ScConditionMode)aLbCond32.GetSelectEntryPos();
        sExpr1 = aEdtCond31.GetText();
        sExpr2 = aEdtCond32.GetText();
        sStyle = aLbCond3Template.GetSelectEntry();
        ScCondFormatEntry aNewEntry( eOper, sExpr1, sExpr2, pDoc, aCurPos, sStyle );
        rCndFmt.AddEntry( aNewEntry );
    }
}


//----------------------------------------------------------------------------
// Zerstoert den Dialog
#pragma SEG_FUNCDEF(condfrmt_03)

BOOL ScConditionalFormatDlg::Close()
{
    return DoClose( ScCondFormatDlgWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------
// Handler:

//----------------------------------------------------------------------------
// Enabled/Disabled Condition1-Controls
#pragma SEG_FUNCDEF(condfrmt_04)

IMPL_LINK( ScConditionalFormatDlg, ClickCond1Hdl, void *, EMPTYARG )
{
    BOOL bChecked = aCbxCond1.IsChecked();

    aLbCond11.Enable( bChecked );
    aLbCond12.Enable( bChecked );
    aEdtCond11.Enable( bChecked );
    aRbCond11.Enable( bChecked );
    aFtCond1And.Enable( bChecked );
    aEdtCond12.Enable( bChecked );
    aRbCond12.Enable( bChecked );
    aFtCond1Template.Enable( bChecked );
    aLbCond1Template.Enable( bChecked );

    return( 0L );
}


//----------------------------------------------------------------------------
// Zellwert/Formel
#pragma SEG_FUNCDEF(condfrmt_05)

IMPL_LINK( ScConditionalFormatDlg, ChangeCond11Hdl, void *, EMPTYARG )
{
    USHORT nPos = aLbCond11.GetSelectEntryPos();

    if( nPos == 0 ) // Zellwert
    {
        aLbCond12.Show();
        aEdtCond11.SetPosPixel( aCond1Pos2 );
    }
    else // Formel
    {
        aLbCond12.Hide();
        aFtCond1And.Hide();
        aEdtCond12.Hide();
        aRbCond12.Hide();
        aRbCond11.SetPosPixel( aRBtn1Pos2 );
        aEdtCond11.SetPosSizePixel( aCond1Pos1, aCond1Size1 );
    }

    ChangeCond12Hdl( NULL );

    return( 0L );
}


//----------------------------------------------------------------------------
// zwischen, gleich, groesser, ...
#pragma SEG_FUNCDEF(condfrmt_06)

IMPL_LINK( ScConditionalFormatDlg, ChangeCond12Hdl, void *, EMPTYARG )
{
    if( aLbCond12.IsVisible() )
    {
        USHORT nPos = aLbCond12.GetSelectEntryPos();

        if( nPos == 6  || nPos == 7 ) // zwischen, n. zwischen
        {
            aEdtCond11.SetSizePixel( aCond1Size3 );
            aRbCond11.SetPosPixel( aRBtn1Pos1 );
            aFtCond1And.Show();
            aEdtCond12.Show();
            aRbCond12.Show();
        }
        else // gleich, n. gleich ...
        {
            aEdtCond12.Hide();
            aRbCond12.Hide();
            aFtCond1And.Hide();
            aRbCond11.SetPosPixel( aRBtn1Pos2 );
            aEdtCond11.SetSizePixel( aCond1Size2 );
        }
    }

    return( 0L );
}


//----------------------------------------------------------------------------
// Enabled/Disabled Condition2-Controls
#pragma SEG_FUNCDEF(condfrmt_07)

IMPL_LINK( ScConditionalFormatDlg, ClickCond2Hdl, void *, EMPTYARG )
{
    BOOL bChecked = aCbxCond2.IsChecked();

    aLbCond21.Enable( bChecked );
    aLbCond22.Enable( bChecked );
    aEdtCond21.Enable( bChecked );
    aRbCond21.Enable( bChecked );
    aFtCond2And.Enable( bChecked );
    aEdtCond22.Enable( bChecked );
    aRbCond22.Enable( bChecked );
    aFtCond2Template.Enable( bChecked );
    aLbCond2Template.Enable( bChecked );

    return( 0L );
}


//----------------------------------------------------------------------------
// Zellwert/Formel
#pragma SEG_FUNCDEF(condfrmt_08)

IMPL_LINK( ScConditionalFormatDlg, ChangeCond21Hdl, void *, EMPTYARG )
{
    USHORT nPos = aLbCond21.GetSelectEntryPos();

    if( nPos == 0 ) // Zellwert
    {
        aLbCond22.Show();
        aEdtCond21.SetPosPixel( aCond2Pos2 );
    }
    else // Formel
    {
        aLbCond22.Hide();
        aFtCond2And.Hide();
        aEdtCond22.Hide();
        aRbCond22.Hide();
        aRbCond21.SetPosPixel( aRBtn2Pos2 );
        aEdtCond21.SetPosSizePixel( aCond2Pos1, aCond2Size1 );
    }

    ChangeCond22Hdl( NULL );

    return( 0L );
}


//----------------------------------------------------------------------------
// zwischen, gleich, groesser, ...
#pragma SEG_FUNCDEF(condfrmt_09)

IMPL_LINK( ScConditionalFormatDlg, ChangeCond22Hdl, void *, EMPTYARG )
{
    if( aLbCond22.IsVisible() )
    {
        USHORT nPos = aLbCond22.GetSelectEntryPos();

        if( nPos == 6  || nPos == 7 ) // zwischen, n. zwischen
        {
            aEdtCond21.SetSizePixel( aCond2Size3 );
            aRbCond21.SetPosPixel( aRBtn2Pos1 );
            aFtCond2And.Show();
            aEdtCond22.Show();
            aRbCond22.Show();
        }
        else // gleich, n. gleich ...
        {
            aEdtCond22.Hide();
            aRbCond22.Hide();
            aFtCond2And.Hide();
            aRbCond21.SetPosPixel( aRBtn2Pos2 );
            aEdtCond21.SetSizePixel( aCond2Size2 );
        }
    }

    return( 0L );
}


//----------------------------------------------------------------------------
// Enabled/Disabled Condition3-Controls
#pragma SEG_FUNCDEF(condfrmt_0a)

IMPL_LINK( ScConditionalFormatDlg, ClickCond3Hdl, void *, EMPTYARG )
{
    BOOL bChecked = aCbxCond3.IsChecked();

    aLbCond31.Enable( bChecked );
    aLbCond32.Enable( bChecked );
    aEdtCond31.Enable( bChecked );
    aRbCond31.Enable( bChecked );
    aFtCond3And.Enable( bChecked );
    aEdtCond32.Enable( bChecked );
    aRbCond32.Enable( bChecked );
    aFtCond3Template.Enable( bChecked );
    aLbCond3Template.Enable( bChecked );

    return( 0L );
}


//----------------------------------------------------------------------------
// Zellwert/Formel
#pragma SEG_FUNCDEF(condfrmt_0b)

IMPL_LINK( ScConditionalFormatDlg, ChangeCond31Hdl, void *, EMPTYARG )
{
    USHORT nPos = aLbCond31.GetSelectEntryPos();

    if( nPos == 0 ) // Zellwert
    {
        aLbCond32.Show();
        aEdtCond31.SetPosPixel( aCond3Pos2 );
    }
    else // Formel
    {
        aLbCond32.Hide();
        aFtCond3And.Hide();
        aEdtCond32.Hide();
        aRbCond32.Hide();
        aRbCond31.SetPosPixel( aRBtn3Pos2 );
        aEdtCond31.SetPosSizePixel( aCond3Pos1, aCond3Size1 );
    }

    ChangeCond32Hdl( NULL );

    return( 0L );
}


//----------------------------------------------------------------------------
// zwischen, gleich, groesser, ...
#pragma SEG_FUNCDEF(condfrmt_0c)

IMPL_LINK( ScConditionalFormatDlg, ChangeCond32Hdl, void *, EMPTYARG )
{
    if( aLbCond32.IsVisible() )
    {
        USHORT nPos = aLbCond32.GetSelectEntryPos();

        if( nPos == 6  || nPos == 7 ) // zwischen, n. zwischen
        {
            aEdtCond31.SetSizePixel( aCond3Size3 );
            aRbCond31.SetPosPixel( aRBtn3Pos1 );
            aFtCond3And.Show();
            aEdtCond32.Show();
            aRbCond32.Show();
        }
        else // gleich, n. gleich ...
        {
            aEdtCond32.Hide();
            aRbCond32.Hide();
            aFtCond3And.Hide();
            aRbCond31.SetPosPixel( aRBtn3Pos2 );
            aEdtCond31.SetSizePixel( aCond3Size2 );
        }
    }

    return( 0L );
}


//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(condfrmt_12)

IMPL_LINK( ScConditionalFormatDlg, EdGetFocusHdl, ScRefEdit*, pEd )
{
    pEdActive = pEd;
    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();
    pEd->SetSelection( Selection(0,SELECTION_MAX) );
    return( 0L );
}


//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(condfrmt_13)

IMPL_LINK( ScConditionalFormatDlg, EdLoseFocusHdl, ScRefEdit*, pEd )
{
    bDlgLostFocus = !IsActive();
    return( 0L );
}


//----------------------------------------------------------------------------
//  [OK], [Cancel]
#pragma SEG_FUNCDEF(condfrmt_14)

IMPL_LINK( ScConditionalFormatDlg, BtnHdl, PushButton*, pBtn )
{
    if ( pBtn == &aBtnOk )
    {
        ScConditionalFormat aCondFrmt( 0, pDoc );
        GetConditionalFormat( aCondFrmt );
        ScCondFrmtItem aOutItem( FID_CONDITIONAL_FORMAT, aCondFrmt );

        SFX_APP()->LockDispatcher( FALSE );
        SwitchToDocument();
        SFX_DISPATCHER().Execute( FID_CONDITIONAL_FORMAT,
                                  SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                  &aOutItem, 0L, 0L );
        Close();
    }
    else if ( pBtn == &aBtnCancel )
        Close();

    return( 0L );
}


/*****************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.20  2000/09/17 14:08:55  willem.vandorp
      OpenOffice header added.

      Revision 1.19  2000/08/31 16:38:19  willem.vandorp
      Header and footer replaced

      Revision 1.18  2000/04/17 13:29:42  nn
      unicode changes

      Revision 1.17  2000/01/17 19:23:39  nn
      #67336# SwitchToDocument

      Revision 1.16  1999/06/09 16:08:20  NN
      Sfx-appwin removed


      Rev 1.15   09 Jun 1999 18:08:20   NN
   Sfx-appwin removed

      Rev 1.14   26 May 1999 14:29:02   NN
   #66123# Name fuer Standard-Vorlage kommt aus Resource

      Rev 1.13   10 Dec 1998 20:40:42   ANK
   #51738# Umstellung auf ScRefEdit zum Anzeigen von Referenzen

      Rev 1.12   06 Aug 1998 21:20:52   ANK
   #54702# Enablen/Disablen der Applikation nur noch in ScAnyRefDlg

      Rev 1.11   16 Sep 1997 19:37:48   NN
   #42538# bedingte Formatierung/Gueltigkeit mit relativen Referenzen

      Rev 1.10   04 Sep 1997 20:32:54   RG
   change header

      Rev 1.9   03 Sep 1997 15:11:14   RG
   change header

      Rev 1.8   28 Nov 1996 18:49:40   NN
   mehrfache und 3D-Referenzen zulassen

      Rev 1.7   13 Nov 1996 20:12:04   NN
   #33143# AutoHide nur bei Bereichsreferenz

      Rev 1.6   05 Nov 1996 21:15:46   NN
   #32969# Focus initial auf ersten Wert

      Rev 1.5   05 Nov 1996 14:51:10   NN
   ScApplication gibts nicht mehr

      Rev 1.4   01 Nov 1996 06:30:38   MH
   chg: includes, defines

      Rev 1.3   29 Oct 1996 14:04:36   NN
   ueberall ScResId statt ResId

      Rev 1.2   22 Oct 1996 17:58:14   RJ
   Buttons fuer Referenzeingabe

      Rev 1.1   16 Oct 1996 16:33:16   RJ
   Dialog fuer bedingte Formatierung mit Leben gefuellt

      Rev 1.0   14 Oct 1996 15:09:42   SOH
   Initial revision.


*****************************************************************************/

#pragma SEG_EOFMODULE

