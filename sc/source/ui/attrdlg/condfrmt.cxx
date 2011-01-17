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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>

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


//============================================================================
//  class ScConditionalFormat

//----------------------------------------------------------------------------
// Konstruktor

ScConditionalFormatDlg::ScConditionalFormatDlg(
            SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
            ScDocument* pCurDoc,
            const ScConditionalFormat* pCurrentFormat )
    :   ScAnyRefDlg         ( pB, pCW, pParent, RID_SCDLG_CONDFORMAT ),

        aCbxCond1           ( this, ScResId( CBX_COND1 ) ),
        aLbCond11           ( this, ScResId( LB_COND1_1 ) ),
        aLbCond12           ( this, ScResId( LB_COND1_2 ) ),
        aEdtCond11          ( this, this, ScResId( EDT_COND1_1 ) ),
        aRbCond11           ( this, ScResId( RB_COND1_1 ), &aEdtCond11,this ),
        aFtCond1And         ( this, ScResId( FT_COND1_AND ) ),
        aEdtCond12          ( this, this, ScResId( EDT_COND1_2 ) ),
        aRbCond12           ( this, ScResId( RB_COND1_2 ), &aEdtCond12,this ),
        aFtCond1Template    ( this, ScResId( FT_COND1_TEMPLATE ) ),
        aLbCond1Template    ( this, ScResId( LB_COND1_TEMPLATE ) ),
        aBtnNew1            ( this, ScResId( BTN_COND1_NEW ) ),
        aFlSep1             ( this, ScResId( FL_SEP1 ) ),

        aCbxCond2           ( this, ScResId( CBX_COND2 ) ),
        aLbCond21           ( this, ScResId( LB_COND2_1 ) ),
        aLbCond22           ( this, ScResId( LB_COND2_2 ) ),
        aEdtCond21          ( this, this, ScResId( EDT_COND2_1 ) ),
        aRbCond21           ( this, ScResId( RB_COND2_1 ), &aEdtCond21,this ),
        aFtCond2And         ( this, ScResId( FT_COND2_AND ) ),
        aEdtCond22          ( this, this, ScResId( EDT_COND2_2 ) ),
        aRbCond22           ( this, ScResId( RB_COND2_2 ), &aEdtCond22,this ),
        aFtCond2Template    ( this, ScResId( FT_COND2_TEMPLATE ) ),
        aLbCond2Template    ( this, ScResId( LB_COND2_TEMPLATE ) ),
        aBtnNew2            ( this, ScResId( BTN_COND2_NEW ) ),
        aFlSep2             ( this, ScResId( FL_SEP2 ) ),

        aCbxCond3           ( this, ScResId( CBX_COND3 ) ),
        aLbCond31           ( this, ScResId( LB_COND3_1 ) ),
        aLbCond32           ( this, ScResId( LB_COND3_2 ) ),
        aEdtCond31          ( this, this, ScResId( EDT_COND3_1 ) ),
        aRbCond31           ( this, ScResId( RB_COND3_1 ), &aEdtCond31,this ),
        aFtCond3And         ( this, ScResId( FT_COND3_AND ) ),
        aEdtCond32          ( this, this, ScResId( EDT_COND3_2 ) ),
        aRbCond32           ( this, ScResId( RB_COND3_2 ), &aEdtCond32,this ),
        aFtCond3Template    ( this, ScResId( FT_COND3_TEMPLATE ) ),
        aLbCond3Template    ( this, ScResId( LB_COND3_TEMPLATE ) ),
        aBtnNew3            ( this, ScResId( BTN_COND3_NEW ) ),

        aBtnOk              ( this, ScResId( BTN_OK ) ),
        aBtnCancel          ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp            ( this, ScResId( BTN_HELP ) ),
        pEdActive           ( NULL ),
        bDlgLostFocus       ( sal_False ),

        pDoc                ( pCurDoc )
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

    Link aLink = LINK( this, ScConditionalFormatDlg, NewBtnHdl );
    aBtnNew1.SetClickHdl( aLink );
    aBtnNew2.SetClickHdl( aLink );
    aBtnNew3.SetClickHdl( aLink );

    aLink = LINK( this, ScConditionalFormatDlg, GetFocusHdl );
    aEdtCond11.SetGetFocusHdl( aLink );
    aEdtCond12.SetGetFocusHdl( aLink );
    aEdtCond21.SetGetFocusHdl( aLink );
    aEdtCond22.SetGetFocusHdl( aLink );
    aEdtCond31.SetGetFocusHdl( aLink );
    aEdtCond32.SetGetFocusHdl( aLink );
    aRbCond11.SetGetFocusHdl( aLink );
    aRbCond12.SetGetFocusHdl( aLink );
    aRbCond21.SetGetFocusHdl( aLink );
    aRbCond22.SetGetFocusHdl( aLink );
    aRbCond31.SetGetFocusHdl( aLink );
    aRbCond32.SetGetFocusHdl( aLink );

    aLink = LINK( this, ScConditionalFormatDlg, LoseFocusHdl );
    aEdtCond11.SetLoseFocusHdl( aLink );
    aEdtCond12.SetLoseFocusHdl( aLink );
    aEdtCond21.SetLoseFocusHdl( aLink );
    aEdtCond22.SetLoseFocusHdl( aLink );
    aEdtCond31.SetLoseFocusHdl( aLink );
    aEdtCond32.SetLoseFocusHdl( aLink );
    aRbCond11.SetLoseFocusHdl( aLink );
    aRbCond12.SetLoseFocusHdl( aLink );
    aRbCond21.SetLoseFocusHdl( aLink );
    aRbCond22.SetLoseFocusHdl( aLink );
    aRbCond31.SetLoseFocusHdl( aLink );
    aRbCond32.SetLoseFocusHdl( aLink );

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

    aCbxCond2.Check( sal_False );
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

    aCbxCond3.Check( sal_False );
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
                aLbCond12.SelectEntryPos( sal::static_int_cast<sal_uInt16>( eMode ) );
                if ( ( eMode == SC_COND_BETWEEN ) || ( eMode == SC_COND_NOTBETWEEN ) )
                    aEdtCond12.SetText( pEntry->GetExpression( aCurPos, 1 ) );
            }
        }


        if ( pCurrentFormat->Count() > 1 )
        {
            aCbxCond2.Check( sal_True );
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
                aLbCond22.SelectEntryPos( sal::static_int_cast<sal_uInt16>( eMode ) );
                if ( ( eMode == SC_COND_BETWEEN ) || ( eMode == SC_COND_NOTBETWEEN ) )
                    aEdtCond22.SetText( pEntry->GetExpression( aCurPos, 1 ) );
            }
        }

        if ( pCurrentFormat->Count() > 2 )
        {
            aCbxCond3.Check( sal_True );
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
                aLbCond32.SelectEntryPos( sal::static_int_cast<sal_uInt16>( eMode ) );
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

__EXPORT ScConditionalFormatDlg::~ScConditionalFormatDlg()
{
}


//----------------------------------------------------------------------------

void ScConditionalFormatDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( pEdActive )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(pEdActive);

        String aStr;
        rRef.Format( aStr, SCR_ABS_3D, pDocP, pDocP->GetAddressConvention () );
        String aVal( pEdActive->GetText() );
        Selection aSel( pEdActive->GetSelection() );
        aSel.Justify();
        aVal.Erase( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
        aVal.Insert( aStr, (xub_StrLen)aSel.Min() );
        Selection aNewSel( aSel.Min(), aSel.Min()+aStr.Len() );
        pEdActive->SetRefString( aVal );
        pEdActive->SetSelection( aNewSel );
//      pEdActive->SetModifyFlag();
    }
}

//----------------------------------------------------------------------------

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

sal_Bool ScConditionalFormatDlg::IsRefInputMode() const
{
    return (pEdActive != NULL);
}


//----------------------------------------------------------------------------

void ScConditionalFormatDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = sal_False;
        if( pEdActive )
            pEdActive->GrabFocus();
    }
    else
        GrabFocus();

    RefInputDone();
}


//----------------------------------------------------------------------------
// Holt die ausgewaehlte bedingte Formatierung ab

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

sal_Bool ScConditionalFormatDlg::Close()
{
    return DoClose( ScCondFormatDlgWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------
// Handler:

//----------------------------------------------------------------------------
// Enabled/Disabled Condition1-Controls

IMPL_LINK( ScConditionalFormatDlg, ClickCond1Hdl, void *, EMPTYARG )
{
    sal_Bool bChecked = aCbxCond1.IsChecked();

    aLbCond11.Enable( bChecked );
    aLbCond12.Enable( bChecked );
    aEdtCond11.Enable( bChecked );
    aRbCond11.Enable( bChecked );
    aFtCond1And.Enable( bChecked );
    aEdtCond12.Enable( bChecked );
    aRbCond12.Enable( bChecked );
    aFtCond1Template.Enable( bChecked );
    aLbCond1Template.Enable( bChecked );
    aBtnNew1.Enable( bChecked );

    return( 0L );
}


//----------------------------------------------------------------------------
// Zellwert/Formel

IMPL_LINK( ScConditionalFormatDlg, ChangeCond11Hdl, void *, EMPTYARG )
{
    sal_uInt16 nPos = aLbCond11.GetSelectEntryPos();

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

IMPL_LINK( ScConditionalFormatDlg, ChangeCond12Hdl, void *, EMPTYARG )
{
    if( aLbCond12.IsVisible() )
    {
        sal_uInt16 nPos = aLbCond12.GetSelectEntryPos();

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

IMPL_LINK( ScConditionalFormatDlg, ClickCond2Hdl, void *, EMPTYARG )
{
    sal_Bool bChecked = aCbxCond2.IsChecked();

    aLbCond21.Enable( bChecked );
    aLbCond22.Enable( bChecked );
    aEdtCond21.Enable( bChecked );
    aRbCond21.Enable( bChecked );
    aFtCond2And.Enable( bChecked );
    aEdtCond22.Enable( bChecked );
    aRbCond22.Enable( bChecked );
    aFtCond2Template.Enable( bChecked );
    aLbCond2Template.Enable( bChecked );
    aBtnNew2.Enable( bChecked );

    return( 0L );
}


//----------------------------------------------------------------------------
// Zellwert/Formel

IMPL_LINK( ScConditionalFormatDlg, ChangeCond21Hdl, void *, EMPTYARG )
{
    sal_uInt16 nPos = aLbCond21.GetSelectEntryPos();

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

IMPL_LINK( ScConditionalFormatDlg, ChangeCond22Hdl, void *, EMPTYARG )
{
    if( aLbCond22.IsVisible() )
    {
        sal_uInt16 nPos = aLbCond22.GetSelectEntryPos();

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

IMPL_LINK( ScConditionalFormatDlg, ClickCond3Hdl, void *, EMPTYARG )
{
    sal_Bool bChecked = aCbxCond3.IsChecked();

    aLbCond31.Enable( bChecked );
    aLbCond32.Enable( bChecked );
    aEdtCond31.Enable( bChecked );
    aRbCond31.Enable( bChecked );
    aFtCond3And.Enable( bChecked );
    aEdtCond32.Enable( bChecked );
    aRbCond32.Enable( bChecked );
    aFtCond3Template.Enable( bChecked );
    aLbCond3Template.Enable( bChecked );
    aBtnNew3.Enable( bChecked );

    return( 0L );
}


//----------------------------------------------------------------------------
// Zellwert/Formel

IMPL_LINK( ScConditionalFormatDlg, ChangeCond31Hdl, void *, EMPTYARG )
{
    sal_uInt16 nPos = aLbCond31.GetSelectEntryPos();

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

IMPL_LINK( ScConditionalFormatDlg, ChangeCond32Hdl, void *, EMPTYARG )
{
    if( aLbCond32.IsVisible() )
    {
        sal_uInt16 nPos = aLbCond32.GetSelectEntryPos();

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

IMPL_LINK( ScConditionalFormatDlg, GetFocusHdl, Control*, pCtrl )
{
    if( (pCtrl == (Control*)&aEdtCond11) || (pCtrl == (Control*)&aRbCond11) )
        pEdActive = &aEdtCond11;
    else if( (pCtrl == (Control*)&aEdtCond12) || (pCtrl == (Control*)&aRbCond12) )
        pEdActive = &aEdtCond12;
    else if( (pCtrl == (Control*)&aEdtCond21) || (pCtrl == (Control*)&aRbCond21) )
        pEdActive = &aEdtCond21;
    else if( (pCtrl == (Control*)&aEdtCond22) || (pCtrl == (Control*)&aRbCond22) )
        pEdActive = &aEdtCond22;
    else if( (pCtrl == (Control*)&aEdtCond31) || (pCtrl == (Control*)&aRbCond31) )
        pEdActive = &aEdtCond31;
    else if( (pCtrl == (Control*)&aEdtCond32) || (pCtrl == (Control*)&aRbCond32) )
        pEdActive = &aEdtCond32;
    else
        pEdActive = NULL;

    if( pEdActive )
        pEdActive->SetSelection( Selection( 0, SELECTION_MAX ) );

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConditionalFormatDlg, LoseFocusHdl, Control*, EMPTYARG )
{
    bDlgLostFocus = !IsActive();
    return 0;
}


//----------------------------------------------------------------------------
//  [OK], [Cancel]

IMPL_LINK( ScConditionalFormatDlg, BtnHdl, PushButton*, pBtn )
{
    if ( pBtn == &aBtnOk )
    {
        ScConditionalFormat aCondFrmt( 0, pDoc );
        GetConditionalFormat( aCondFrmt );
        ScCondFrmtItem aOutItem( FID_CONDITIONAL_FORMAT, aCondFrmt );

        SetDispatcherLock( sal_False );
        SwitchToDocument();
        GetBindings().GetDispatcher()->Execute( FID_CONDITIONAL_FORMAT,
                                  SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                  &aOutItem, 0L, 0L );
        Close();
    }
    else if ( pBtn == &aBtnCancel )
        Close();

    return( 0L );
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConditionalFormatDlg, NewBtnHdl, PushButton*, pBtn )
{
    SfxUInt16Item aFamilyItem( SID_STYLE_FAMILY, SFX_STYLE_FAMILY_PARA );
    SfxStringItem aRefItem( SID_STYLE_REFERENCE, ScGlobal::GetRscString(STR_STYLENAME_STANDARD) );

    // unlock the dispatcher so SID_STYLE_NEW can be executed
    // (SetDispatcherLock would affect all Calc documents)
    SfxDispatcher* pDisp = GetBindings().GetDispatcher();
    sal_Bool bLocked = pDisp->IsLocked();
    if (bLocked)
        pDisp->Lock(sal_False);

    // Execute the "new style" slot, complete with undo and all necessary updates.
    // The return value (SfxUInt16Item) is ignored, look for new styles instead.
    pDisp->Execute( SID_STYLE_NEW, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD | SFX_CALLMODE_MODAL,
                    &aFamilyItem,
                    &aRefItem,
                    0L );

    if (bLocked)
        pDisp->Lock(sal_True);

    // Find the new style and add it into the style list boxes
    String aNewStyle;
    SfxStyleSheetIterator aStyleIter( pDoc->GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
    for ( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
    {
        String aName = pStyle->GetName();
        if ( aLbCond1Template.GetEntryPos(aName) == LISTBOX_ENTRY_NOTFOUND )    // all lists contain the same entries
        {
            aLbCond1Template.InsertEntry( aName );
            aLbCond2Template.InsertEntry( aName );
            aLbCond3Template.InsertEntry( aName );
            // if there are several new styles (from API or a different view),
            // assume the last one is the result of the dialog
            aNewStyle = aName;
        }
    }

    // select the new style in the list box for which the button was pressed
    if ( aNewStyle.Len() )
    {
        ListBox* pListBox = &aLbCond1Template;
        if ( pBtn == &aBtnNew2 )
            pListBox = &aLbCond2Template;
        else if ( pBtn == &aBtnNew3 )
            pListBox = &aLbCond3Template;

        pListBox->SelectEntry( aNewStyle );
    }

    return 0;
}

