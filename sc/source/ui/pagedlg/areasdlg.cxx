/*************************************************************************
 *
 *  $RCSfile: areasdlg.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 11:50:17 $
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

#ifndef SC_RANGELST_HXX
#include <rangelst.hxx>
#endif

#include <sfx2/dispatch.hxx>
#include <svtools/stritem.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/charclass.hxx>
#include <stdlib.h>

#define _AREASDLG_CXX
#include "areasdlg.hxx"
#undef _AREASDLG_CXX

#include "scresid.hxx"
#include "rangenam.hxx"
#include "reffact.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "globstr.hrc"
#include "pagedlg.hrc"

// STATIC DATA ---------------------------------------------------------------

// List box positions for print range (PR)
const USHORT SC_AREASDLG_PR_NONE    = 0;
const USHORT SC_AREASDLG_PR_ENTIRE  = 1;
const USHORT SC_AREASDLG_PR_USER    = 2;
const USHORT SC_AREASDLG_PR_SELECT  = 3;
const USHORT SC_AREASDLG_PR_OFFSET  = 4;

// List box positions for repeat ranges (RR)
const USHORT SC_AREASDLG_RR_NONE    = 0;
const USHORT SC_AREASDLG_RR_USER    = 1;
const USHORT SC_AREASDLG_RR_OFFSET  = 2;

//============================================================================

#define HDL(hdl)            LINK( this, ScPrintAreasDlg, hdl )
#define ERRORBOX(nId)       ErrorBox( this, WinBits(WB_OK|WB_DEF_OK), \
                            ScGlobal::GetRscString( nId ) ).Execute()
#define SWAP(x1,x2)         { int n=x1; x1=x2; x2=n; }

// globale Funktionen (->am Ende der Datei):

BOOL    lcl_CheckRepeatString( const String& rStr, BOOL bIsRow, ScRange* pRange );
void    lcl_GetRepeatRangeString( const ScRange* pRange, BOOL bIsRow, String& rStr );
void    lcl_CheckEqual( String& rStr );


//============================================================================
//  class ScPrintAreasDlg

//----------------------------------------------------------------------------

ScPrintAreasDlg::ScPrintAreasDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent )
    :   ScAnyRefDlg     ( pB, pCW, pParent, RID_SCDLG_AREAS),
        //
        aLbPrintArea    ( this, ScResId( LB_PRINTAREA ) ),
        aFlPrintArea    ( this, ScResId( FL_PRINTAREA ) ),
        aEdPrintArea    ( this, ScResId( ED_PRINTAREA ) ),
        aRbPrintArea    ( this, ScResId( RB_PRINTAREA ), &aEdPrintArea ),
        //
        aLbRepeatRow    ( this, ScResId( LB_REPEATROW ) ),
        aFlRepeatRow    ( this, ScResId( FL_REPEATROW ) ),
        aEdRepeatRow    ( this, ScResId( ED_REPEATROW ) ),
        aRbRepeatRow    ( this, ScResId( RB_REPEATROW ), &aEdRepeatRow ),
        //
        aLbRepeatCol    ( this, ScResId( LB_REPEATCOL ) ),
        aFlRepeatCol    ( this, ScResId( FL_REPEATCOL ) ),
        aEdRepeatCol    ( this, ScResId( ED_REPEATCOL ) ),
        aRbRepeatCol    ( this, ScResId( RB_REPEATCOL ), &aEdRepeatCol ),
        //
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        //
        pRefInputEdit   ( &aEdPrintArea ),
        bDlgLostFocus   ( FALSE ),
        pDoc            ( NULL ),
        pViewData       ( NULL ),
        nCurTab         ( 0 )
{
    ScTabViewShell* pScViewSh = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
    ScDocShell*     pScDocSh  = PTR_CAST( ScDocShell,     SfxObjectShell::Current() );

    DBG_ASSERT( pScDocSh, "Current DocumentShell not found :-(" );

    pDoc = pScDocSh->GetDocument();

    if ( pScViewSh )
    {
        pViewData = pScViewSh->GetViewData();
        nCurTab   = pViewData->GetTabNo();
    }

    Impl_Reset();

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();

    FreeResource();
}


//----------------------------------------------------------------------------

ScPrintAreasDlg::~ScPrintAreasDlg()
{
    // Extra-Data an ListBox-Entries abraeumen

#ifdef SINIX
    ListBox* pLb[3];
    pLb[0] = &aLbPrintArea;
    pLb[1] = &aLbRepeatRow;
    pLb[2] = &aLbRepeatCol;
#else
    ListBox* pLb[3] = { &aLbPrintArea, &aLbRepeatRow, &aLbRepeatCol };
#endif

    for ( USHORT i=0; i<3; i++ )
    {
        USHORT nCount = pLb[i]->GetEntryCount();
        for ( USHORT j=0; j<nCount; j++ )
            delete (String*)pLb[i]->GetEntryData(j);
    }
}


//----------------------------------------------------------------------------

BOOL ScPrintAreasDlg::Close()
{
    return DoClose( ScPrintAreasDlgWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------

BOOL ScPrintAreasDlg::IsTableLocked() const
{
    //  Druckbereiche gelten pro Tabelle, darum macht es keinen Sinn,
    //  bei der Eingabe die Tabelle umzuschalten

    return TRUE;
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( pRefInputEdit )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pRefInputEdit );

        String  aStr;

        if ( &aEdPrintArea == pRefInputEdit )
        {
            rRef.Format( aStr, SCR_ABS );
            lcl_CheckEqual( aStr );

//          aEdPrintArea.ReplaceSelected( aStr );

            String aVal = aEdPrintArea.GetText();
            Selection aSel = aEdPrintArea.GetSelection();
            aSel.Justify();
            aVal.Erase( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
            aVal.Insert( aStr, (xub_StrLen)aSel.Min() );
            Selection aNewSel( aSel.Min(), aSel.Min()+aStr.Len() );
            aEdPrintArea.SetRefString( aVal );
            aEdPrintArea.SetSelection( aNewSel );
        }
        else
        {
            BOOL bRow = ( &aEdRepeatRow == pRefInputEdit );
            lcl_GetRepeatRangeString( &rRef, bRow, aStr );
            pRefInputEdit->SetRefString( aStr );
        }
    }

    Impl_ModifyHdl( pRefInputEdit );
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::AddRefEntry()
{
    if ( pRefInputEdit == &aEdPrintArea )
    {
        String aVal = aEdPrintArea.GetText();
        aVal += ';';
        aEdPrintArea.SetText(aVal);

        xub_StrLen nLen = aVal.Len();
        aEdPrintArea.SetSelection( Selection( nLen, nLen ) );

        Impl_ModifyHdl( &aEdPrintArea );
    }
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::Deactivate()
{
    bDlgLostFocus = TRUE;
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = FALSE;

        if ( pRefInputEdit )
        {
            pRefInputEdit->GrabFocus();
            Impl_ModifyHdl( pRefInputEdit );
        }
    }
    else
        GrabFocus();

    RefInputDone();
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::Impl_Reset()
{
    String          aStrRange;
    const ScRange*  pRepeatColRange = pDoc->GetRepeatColRange( nCurTab );
    const ScRange*  pRepeatRowRange = pDoc->GetRepeatRowRange( nCurTab );

    aEdPrintArea.SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    aEdRepeatRow.SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    aEdRepeatCol.SetModifyHdl   ( HDL(Impl_ModifyHdl) );
    aEdPrintArea.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aEdRepeatRow.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aEdRepeatCol.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aLbPrintArea.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aLbRepeatRow.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aLbRepeatCol.SetGetFocusHdl( HDL(Impl_GetFocusHdl) );
    aLbPrintArea.SetSelectHdl   ( HDL(Impl_SelectHdl) );
    aLbRepeatRow.SetSelectHdl   ( HDL(Impl_SelectHdl) );
    aLbRepeatCol.SetSelectHdl   ( HDL(Impl_SelectHdl) );
    aBtnOk      .SetClickHdl    ( HDL(Impl_BtnHdl)    );
    aBtnCancel  .SetClickHdl    ( HDL(Impl_BtnHdl)    );

    Impl_FillLists();

    //-------------------------
    // Druckbereich
    //-------------------------
    aStrRange.Erase();
    String aOne;
    USHORT nRangeCount = pDoc->GetPrintRangeCount( nCurTab );
    for (USHORT i=0; i<nRangeCount; i++)
    {
        const ScRange* pPrintRange = pDoc->GetPrintRange( nCurTab, i );
        if (pPrintRange)
        {
            if ( aStrRange.Len() )
                aStrRange += ';';
            pPrintRange->Format( aOne, SCR_ABS );
            lcl_CheckEqual( aOne );
            aStrRange += aOne;
        }
    }
    aEdPrintArea.SetText( aStrRange );

    //-------------------------------
    // Wiederholungszeile
    //-------------------------------
    lcl_GetRepeatRangeString( pRepeatRowRange, TRUE, aStrRange );
    aEdRepeatRow.SetText( aStrRange );

    //--------------------------------
    // Wiederholungsspalte
    //--------------------------------
    lcl_GetRepeatRangeString( pRepeatColRange, FALSE, aStrRange );
    aEdRepeatCol.SetText( aStrRange );

    Impl_ModifyHdl( &aEdPrintArea );
    Impl_ModifyHdl( &aEdRepeatRow );
    Impl_ModifyHdl( &aEdRepeatCol );
    if( pDoc->IsPrintEntireSheet( nCurTab ) )
        aLbPrintArea.SelectEntryPos( SC_AREASDLG_PR_ENTIRE );

    aEdPrintArea.SaveValue();   // fuer FillItemSet() merken:
    aEdRepeatRow.SaveValue();
    aEdRepeatCol.SaveValue();
}


//----------------------------------------------------------------------------

BOOL ScPrintAreasDlg::Impl_GetItem( Edit* pEd, SfxStringItem& rItem )
{
    String  aRangeStr = pEd->GetText();
    BOOL    bDataChanged = (pEd->GetSavedValue() != aRangeStr);

    if ( (aRangeStr.Len() > 0) && &aEdPrintArea != pEd )
    {
        ScRange aRange;
        lcl_CheckRepeatString( aRangeStr, &aEdRepeatRow == pEd, &aRange );
        aRange.Format( aRangeStr, SCR_ABS );
        lcl_CheckEqual( aRangeStr );
    }

    rItem.SetValue( aRangeStr );

    return bDataChanged;
}


//----------------------------------------------------------------------------

BOOL ScPrintAreasDlg::Impl_CheckRefStrings()
{
    BOOL        bOk = FALSE;
    String      aStrPrintArea   = aEdPrintArea.GetText();
    String      aStrRepeatRow   = aEdRepeatRow.GetText();
    String      aStrRepeatCol   = aEdRepeatCol.GetText();

    BOOL bPrintAreaOk = TRUE;
    if ( aStrPrintArea.Len() )
    {
        ScRange aRange;
        xub_StrLen nTCount = aStrPrintArea.GetTokenCount();
        for ( xub_StrLen i=0; i<nTCount && bPrintAreaOk; i++ )
        {
            String aOne = aStrPrintArea.GetToken(i);
            lcl_CheckEqual( aOne );
            USHORT nResult = aRange.ParseAny( aOne, pDoc );
            if (!(nResult & SCA_VALID))
                bPrintAreaOk = FALSE;
        }
    }

    BOOL bRepeatRowOk = (aStrRepeatRow.Len() == 0);
    if ( !bRepeatRowOk )
        bRepeatRowOk = lcl_CheckRepeatString( aStrRepeatRow, TRUE, NULL );

    BOOL bRepeatColOk = (aStrRepeatCol.Len() == 0);
    if ( !bRepeatColOk )
        bRepeatColOk = lcl_CheckRepeatString( aStrRepeatCol, FALSE, NULL );

    // Fehlermeldungen

    bOk = (bPrintAreaOk && bRepeatRowOk && bRepeatColOk);

    if ( !bOk )
    {
        Edit* pEd = NULL;

             if ( !bPrintAreaOk ) pEd = &aEdPrintArea;
        else if ( !bRepeatRowOk ) pEd = &aEdRepeatRow;
        else if ( !bRepeatColOk ) pEd = &aEdRepeatCol;

        ERRORBOX( STR_INVALID_TABREF );
        pEd->GrabFocus();
    }

    return bOk;
}


//----------------------------------------------------------------------------

void ScPrintAreasDlg::Impl_FillLists()
{
    //------------------------------------------------------
    // Selektion holen und String in PrintArea-ListBox merken
    //------------------------------------------------------
    ScRange aRange;
    String  aStrRange;
    BOOL bSimple = TRUE;

    if ( pViewData )
        bSimple = pViewData->GetSimpleArea( aRange );

    if ( bSimple )
        aRange.Format( aStrRange, SCR_ABS, pDoc );
    else
    {
        ScRangeListRef aList( new ScRangeList );
        pViewData->GetMarkData().FillRangeListWithMarks( aList, FALSE );
        aList->Format( aStrRange, SCR_ABS, pDoc );
    }

    aLbPrintArea.SetEntryData( SC_AREASDLG_PR_SELECT, new String( aStrRange ) );

    //------------------------------------------------------
    // Ranges holen und in ListBoxen merken
    //------------------------------------------------------
    ScRangeName*    pRangeNames = pDoc->GetRangeName();
    const USHORT    nCount      = pRangeNames ? pRangeNames->GetCount() : 0;

    if ( nCount > 0 )
    {
        String          aName;
        String          aSymbol;
        ScRange         aRange;
        ScRangeData*    pData = NULL;

        for ( USHORT i=0; i<nCount; i++ )
        {
            pData = (ScRangeData*)(pRangeNames->At( i ));
            if ( pData )
            {
                if (   pData->HasType( RT_ABSAREA )
                    || pData->HasType( RT_REFAREA )
                    || pData->HasType( RT_ABSPOS ) )
                {
                    pData->GetName( aName );
                    pData->GetSymbol( aSymbol );
                    if ( aRange.ParseAny( aSymbol, pDoc ) & SCA_VALID )
                    {
                        if ( pData->HasType( RT_PRINTAREA ) )
                        {
                            aRange.Format( aSymbol, SCR_ABS, pDoc );
                            aLbPrintArea.SetEntryData(
                                aLbPrintArea.InsertEntry( aName ),
                                new String( aSymbol ) );
                        }

                        if ( pData->HasType( RT_ROWHEADER ) )
                        {
                            lcl_GetRepeatRangeString( &aRange, TRUE, aSymbol );
                            aLbRepeatRow.SetEntryData(
                                aLbRepeatRow.InsertEntry( aName ),
                                new String( aSymbol ) );
                        }

                        if ( pData->HasType( RT_COLHEADER ) )
                        {
                            lcl_GetRepeatRangeString( &aRange, FALSE, aSymbol );
                            aLbRepeatCol.SetEntryData(
                                aLbRepeatCol.InsertEntry( aName ),
                                new String( aSymbol ) );
                        }
                    }
                }
            }
        }
    }
}


//----------------------------------------------------------------------------
// Handler:
//----------------------------------------------------------------------------

IMPL_LINK( ScPrintAreasDlg, Impl_BtnHdl, PushButton*, pBtn )
{
    if ( &aBtnOk == pBtn )
    {
        if ( Impl_CheckRefStrings() )
        {
            BOOL            bDataChanged = FALSE;
            String          aStr;
            SfxStringItem   aPrintArea( SID_CHANGE_PRINTAREA, aStr );
            SfxStringItem   aRepeatRow( FN_PARAM_2, aStr );
            SfxStringItem   aRepeatCol( FN_PARAM_3, aStr );

            //-------------------------
            // Druckbereich veraendert?
            //-------------------------

            // first try the list box, if "Entite sheet" is selected
            BOOL bEntireSheet = (aLbPrintArea.GetSelectEntryPos() == SC_AREASDLG_PR_ENTIRE);
            SfxBoolItem aEntireSheet( FN_PARAM_4, bEntireSheet );

            bDataChanged = bEntireSheet != pDoc->IsPrintEntireSheet( nCurTab );
            if( !bEntireSheet )
            {
                // if new list box selection is not "Entire sheet", get the edit field contents
                bDataChanged |= Impl_GetItem( &aEdPrintArea, aPrintArea );
            }

            //-------------------------------
            // Wiederholungszeile veraendert?
            //-------------------------------
            bDataChanged |= Impl_GetItem( &aEdRepeatRow, aRepeatRow );

            //--------------------------------
            // Wiederholungsspalte veraendert?
            //--------------------------------
            bDataChanged |= Impl_GetItem( &aEdRepeatCol, aRepeatCol );

            if ( bDataChanged )
            {
                SetDispatcherLock( FALSE );
                SwitchToDocument();
                GetBindings().GetDispatcher()->Execute( SID_CHANGE_PRINTAREA,
                                          SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                          &aPrintArea, &aRepeatRow, &aRepeatCol, &aEntireSheet, 0L );
            }

            Close();
        }
    }
    else if ( &aBtnCancel == pBtn )
        Close();

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScPrintAreasDlg, Impl_GetFocusHdl, Control*, pCtr )
{
    if ( pCtr ==(Control *) &aEdPrintArea ||
         pCtr ==(Control *) &aEdRepeatRow ||
         pCtr ==(Control *) &aEdRepeatCol)
    {
         pRefInputEdit = (ScRefEdit*) pCtr;
    }
    else if ( pCtr ==(Control *) &aLbPrintArea)
    {
        pRefInputEdit = &aEdPrintArea;
    }
    else if ( pCtr ==(Control *) &aLbRepeatRow)
    {
        pRefInputEdit = &aEdRepeatRow;
    }
    else if ( pCtr ==(Control *) &aLbRepeatCol)
    {
        pRefInputEdit = &aEdRepeatCol;
    }

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScPrintAreasDlg, Impl_SelectHdl, ListBox*, pLb )
{
    USHORT nSelPos = pLb->GetSelectEntryPos();
    Edit* pEd = NULL;

    // list box positions of specific entries, default to "repeat row/column" list boxes
    USHORT nAllSheetPos = SC_AREASDLG_RR_NONE;
    USHORT nUserDefPos = SC_AREASDLG_RR_USER;
    USHORT nFirstCustomPos = SC_AREASDLG_RR_OFFSET;

    // find edit field for list box, and list box positions
    if( pLb == &aLbPrintArea )
    {
        pEd = &aEdPrintArea;
        nAllSheetPos = SC_AREASDLG_PR_ENTIRE;
        nUserDefPos = SC_AREASDLG_PR_USER;
        nFirstCustomPos = SC_AREASDLG_PR_SELECT;    // "Selection" and following
    }
    else if( pLb == &aLbRepeatCol )
        pEd = &aEdRepeatCol;
    else if( pLb == &aLbRepeatRow )
        pEd = &aEdRepeatRow;
    else
        return 0;

    // fill edit field according to list box selection
    if( (nSelPos == 0) || (nSelPos == nAllSheetPos) )
        pEd->SetText( EMPTY_STRING );
    else if( nSelPos == nUserDefPos && !pLb->IsTravelSelect() && pEd->GetText().Len() == 0 )
        pLb->SelectEntryPos( 0 );
    else if( nSelPos >= nFirstCustomPos )
        pEd->SetText( *static_cast< String* >( pLb->GetEntryData( nSelPos ) ) );

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScPrintAreasDlg, Impl_ModifyHdl, ScRefEdit*, pEd )
{
    ListBox* pLb = NULL;

    // list box positions of specific entries, default to "repeat row/column" list boxes
    USHORT nUserDefPos = SC_AREASDLG_RR_USER;
    USHORT nFirstCustomPos = SC_AREASDLG_RR_OFFSET;

    if( pEd == &aEdPrintArea )
    {
        pLb = &aLbPrintArea;
        nUserDefPos = SC_AREASDLG_PR_USER;
        nFirstCustomPos = SC_AREASDLG_PR_SELECT;    // "Selection" and following
    }
    else if( pEd == &aEdRepeatCol )
        pLb = &aLbRepeatCol;
    else if( pEd == &aEdRepeatRow )
        pLb = &aLbRepeatRow;
    else
        return 0;

    // set list box selection according to edit field
    USHORT  nEntryCount = pLb->GetEntryCount();
    String  aStrEd( pEd->GetText() );
    String  aEdUpper = aStrEd;
    aEdUpper.ToUpperAscii();

    if ( (nEntryCount > nFirstCustomPos) && aStrEd.Len() > 0 )
    {
        BOOL    bFound  = FALSE;
        String* pSymbol = NULL;
        USHORT i;

        for ( i=nFirstCustomPos; i<nEntryCount && !bFound; i++ )
        {
            pSymbol = (String*)pLb->GetEntryData( i );
            bFound  = ( (*pSymbol == aStrEd) || (*pSymbol == aEdUpper) );
        }

        pLb->SelectEntryPos( bFound ? i-1 : nUserDefPos );
    }
    else
        pLb->SelectEntryPos( aStrEd.Len() ? nUserDefPos : 0 );

    return 0;
}


//============================================================================
// globale Funktionen:

//----------------------------------------------------------------------------

BOOL lcl_CheckRepeatOne( const String& rStr, BOOL bIsRow, SCCOLROW& rVal )
{
    // Zulaessige Syntax fuer rStr:
    // Row: [$]1-MAXTAB
    // Col: [$]A-IV

    String  aStr    = rStr;
    xub_StrLen nLen = aStr.Len();
    SCCOLROW    nNum    = 0;
    BOOL    bStrOk  = ( nLen > 0 ) && ( bIsRow ? ( nLen < 6 ) : ( nLen < 4 ) );

    if ( bStrOk )
    {
        if ( '$' == aStr.GetChar(0) )
            aStr.Erase( 0, 1 );

        if ( bIsRow )
        {
            bStrOk = CharClass::isAsciiNumeric(aStr);

            if ( bStrOk )
            {
                sal_Int32 n = aStr.ToInt32();

                if ( bStrOk = (n > 0) && ( n <= MAXROWCOUNT ) )
                    nNum = static_cast<SCCOLROW>(n - 1);
            }
        }
        else
        {
            SCCOL nCol = 0;
            bStrOk = ::AlphaToCol( nCol, aStr);
            nNum = nCol;
        }
    }

    if ( bStrOk )
        rVal = nNum;

    return bStrOk;
}


//----------------------------------------------------------------------------
//  doppelte Referenz testen


BOOL lcl_CheckRepeatString( const String& rStr, BOOL bIsRow, ScRange* pRange )
{
    // Zulaessige Syntax fuer rStr:
    // Row: $1, $1:$2
    // Col: $A, $A:$B
    // und alles auch ohne $

    BOOL bOk = FALSE;
    SCCOLROW nStart = 0;
    SCCOLROW nEnd = 0;
    xub_StrLen nCount = rStr.GetTokenCount(':');
    if (nCount == 1)
    {
        bOk = lcl_CheckRepeatOne( rStr, bIsRow, nStart );
        nEnd = nStart;
    }
    else if (nCount == 2)
    {
        String aFirst  = rStr.GetToken( 0, ':' );
        String aSecond = rStr.GetToken( 1, ':' );
        bOk = lcl_CheckRepeatOne( aFirst, bIsRow, nStart );
        if (bOk)
            bOk = lcl_CheckRepeatOne( aSecond, bIsRow, nEnd );
    }


    if ( bOk && pRange )
    {
        ScAddress& rStart = pRange->aStart;
        ScAddress& rEnd   = pRange->aEnd;

        if ( bIsRow )
        {
            rStart.SetCol( 0 );
            rEnd  .SetCol( 0 );
            rStart.SetRow( nStart );
            rEnd  .SetRow( nEnd );
        }
        else
        {
            rStart.SetCol( static_cast<SCCOL>(nStart) );
            rEnd  .SetCol( static_cast<SCCOL>(nEnd) );
            rStart.SetRow( 0 );
            rEnd  .SetRow( 0 );
        }
    }

    return bOk;
}


//----------------------------------------------------------------------------

void lcl_GetRepeatRangeString( const ScRange* pRange, BOOL bIsRow, String& rStr )
{
    if ( pRange )
    {
        // In rStr wird die pRange im folgenden Format ausgegeben
        // Row: $1, $1:$2
        // Col: $A, $A:$B

        const ScAddress& rStart = pRange->aStart;
        const ScAddress& rEnd = pRange->aEnd;

        rStr  = '$';
        if ( bIsRow )
        {
            rStr += String::CreateFromInt32( rStart.Row()+1 );
            if ( rStart.Row() != rEnd.Row() )
            {
                rStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ":$" ));
                rStr += String::CreateFromInt32( rEnd.Row()+1 );
            }
        }
        else
        {
            rStr += ::ColToAlpha( rStart.Col() );
            if ( rStart.Col() != rEnd.Col() )
            {
                rStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ":$" ));
                rStr += ::ColToAlpha( rEnd.Col() );
            }
        }
    }
    else
        rStr.Erase();
}


//----------------------------------------------------------------------------

void lcl_CheckEqual( String& rStr )
{
    if ( STRING_NOTFOUND == rStr.Search( ':' ) )
    {
        String aStrTmp = rStr;
        rStr += ':';
        rStr += aStrTmp;
    }
}







