/*************************************************************************
 *
 *  $RCSfile: crnrdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 14:46:47 $
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

// System - Includes ---------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE -------------------------------------------------------------------

#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "crnrdlg.hrc"
#ifndef SC_DOCSHELL_HXX
#include "docsh.hxx"
#endif

#define _CRNRDLG_CXX
#include "crnrdlg.hxx"
#undef _CRNRDLG_CXX

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif


//============================================================================

#define ERRORBOX(s) ErrorBox(this,WinBits(WB_OK|WB_DEF_OK),s).Execute()
#define QUERYBOX(m) QueryBox(this,WinBits(WB_YES_NO|WB_DEF_YES),m).Execute()

const ULONG nEntryDataCol = 0;
const ULONG nEntryDataRow = 1;
const ULONG nEntryDataDelim = 2;


//============================================================================
//  class ScColRowNameRangesDlg


/*************************************************************************
#*  Member:     ScColRowNameRangesDlg                       Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Konstruktor der Klasse ScColRowNameRangesDlg.
#*              Initialisieren der Klassen- Mitglieder,
#*              Uebernahme der Range- Angaben und Aufruf
#*              der eigentlichen Initialisierungsroutine
#*
#*  Input:      Sfx- Verknuepfungen
#*              Parent- Window
#*              SCViewData
#*
#*  Output:     ---
#*
#************************************************************************/

ScColRowNameRangesDlg::ScColRowNameRangesDlg( SfxBindings* pB,
                                SfxChildWindow* pCW,
                                Window* pParent,
                                ScViewData* ptrViewData )

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_COLROWNAMERANGES ),
        //
        aLbRange        ( this, ScResId( LB_RANGE ) ),

        aGbAssign       ( this, ScResId( GB_ASSIGN ) ),
        aEdAssign       ( this, ScResId( ED_AREA ) ),
        aRbAssign       ( this, ScResId( RB_AREA ), &aEdAssign ),
        aBtnColHead     ( this, ScResId( BTN_COLHEAD ) ),
        aBtnRowHead     ( this, ScResId( BTN_ROWHEAD ) ),
        aFtAssign2      ( this, ScResId( FT_DATA_LABEL ) ),
        aEdAssign2      ( this, ScResId( ED_DATA ) ),
        aRbAssign2      ( this, ScResId( RB_DATA ), &aEdAssign2 ),

        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnAdd         ( this, ScResId( BTN_ADD ) ),
        aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),

        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData->GetDocument() ),

        pEdActive       ( NULL ),
        bDlgLostFocus   ( FALSE )
{
    xColNameRanges = pDoc->GetColNameRanges()->Clone();
    xRowNameRanges = pDoc->GetRowNameRanges()->Clone();
    Init();
    FreeResource();
}


/*************************************************************************
#*  Member:     ~ScColRowNameRangesDlg                      Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Destruktor der Klasse
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

__EXPORT ScColRowNameRangesDlg::~ScColRowNameRangesDlg()
{
}


/*************************************************************************
#*  Member:     Init                                        Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Initialisierungs- Routine:
#*              Umlenken der Event- Handler und einstellen der
#*              Startparameter.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScColRowNameRangesDlg::Init()
{
    USHORT  nStartCol   = 0;
    USHORT  nStartRow   = 0;
    USHORT  nStartTab   = 0;
    USHORT  nEndCol     = 0;
    USHORT  nEndRow     = 0;
    USHORT  nEndTab     = 0;

    aBtnOk.SetClickHdl      ( LINK( this, ScColRowNameRangesDlg, OkBtnHdl ) );
    aBtnCancel.SetClickHdl  ( LINK( this, ScColRowNameRangesDlg, CancelBtnHdl ) );
    aBtnAdd.SetClickHdl     ( LINK( this, ScColRowNameRangesDlg, AddBtnHdl ) );
    aBtnRemove.SetClickHdl  ( LINK( this, ScColRowNameRangesDlg, RemoveBtnHdl ) );
    aLbRange.SetSelectHdl   ( LINK( this, ScColRowNameRangesDlg, Range1SelectHdl ) );
    aEdAssign.SetModifyHdl  ( LINK( this, ScColRowNameRangesDlg, Range1DataModifyHdl ) );
    aBtnColHead.SetClickHdl ( LINK( this, ScColRowNameRangesDlg, ColClickHdl ) );
    aBtnRowHead.SetClickHdl ( LINK( this, ScColRowNameRangesDlg, RowClickHdl ) );
    aEdAssign2.SetModifyHdl ( LINK( this, ScColRowNameRangesDlg, Range2DataModifyHdl ) );

    aEdAssign.SetGetFocusHdl ( LINK( this, ScColRowNameRangesDlg, EdGetFocusHdl ) );
    aEdAssign2.SetGetFocusHdl ( LINK( this, ScColRowNameRangesDlg, EdGetFocusHdl ) );
    aEdAssign.SetLoseFocusHdl ( LINK( this, ScColRowNameRangesDlg, EdLoseFocusHdl ) );
    aEdAssign2.SetLoseFocusHdl ( LINK( this, ScColRowNameRangesDlg, EdLoseFocusHdl ) );

    pEdActive = &aEdAssign;

    UpdateNames();

    if ( pViewData && pDoc )
    {
        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,  nEndTab );
        SetColRowData( ScRange( ScAddress( nStartCol, nStartRow, nStartTab ),
                              ScAddress( nEndCol,   nEndRow,   nEndTab ) ) );
    }
    else
    {
        aBtnColHead.Check( TRUE );
        aBtnRowHead.Check( FALSE );
        aEdAssign.SetText( EMPTY_STRING );
        aEdAssign2.SetText( EMPTY_STRING );
    }

    aBtnColHead.Enable();
    aBtnRowHead.Enable();
    aEdAssign.Enable();
    aEdAssign.GrabFocus();
    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();      // Ref-Feld hat Focus

    Range1SelectHdl( 0 );
}


/*************************************************************************
#*  Member:     SetColRowData                               Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   zugehoerigen Datenbereich eines Beschriftungsbereiches
#*              auf default Werte setzen und beide Referenz-Edit-Felder
#*              fuellen.
#*
#*  Input:      Einstellbereich fuer Labels
#*
#*  Output:     ---
#*
#************************************************************************/

void ScColRowNameRangesDlg::SetColRowData( const ScRange& rLabelRange,BOOL bRef)
{
    theCurData = theCurArea = rLabelRange;
    BOOL bValid = TRUE;
    USHORT nCol1 = theCurArea.aStart.Col();
    USHORT nCol2 = theCurArea.aEnd.Col();
    USHORT nRow1 = theCurArea.aStart.Row();
    USHORT nRow2 = theCurArea.aEnd.Row();
    if ( (nCol2 - nCol1 >= nRow2 - nRow1) || (nCol1 == 0 && nCol2 == MAXCOL) )
    {   // Spaltenkoepfe und Grenzfall gesamte Tabelle
        aBtnColHead.Check( TRUE );
        aBtnRowHead.Check( FALSE );
        if ( nRow2 == MAXROW  )
        {
            if ( nRow1 == 0 )
                bValid = FALSE;     // Grenzfall gesamte Tabelle
            else
            {   // Head unten, Data oben
                theCurData.aStart.SetRow( 0 );
                theCurData.aEnd.SetRow( nRow1 - 1 );
            }
        }
        else
        {   // Head oben, Data unten
            theCurData.aStart.SetRow( nRow2 + 1 );
            theCurData.aEnd.SetRow( MAXROW );
        }
    }
    else
    {   // Zeilenkoepfe
        aBtnRowHead.Check( TRUE );
        aBtnColHead.Check( FALSE );
        if ( nCol2 == MAXCOL )
        {   // Head rechts, Data links
            theCurData.aStart.SetCol( 0 );
            theCurData.aEnd.SetCol( nCol2 - 1 );
        }
        else
        {   // Head links, Data rechts
            theCurData.aStart.SetCol( nCol2 + 1 );
            theCurData.aEnd.SetCol( MAXCOL );
        }
    }
    if ( bValid )
    {
        String aStr;
        theCurArea.Format( aStr, SCR_ABS_3D, pDoc );

        if(bRef)
            aEdAssign.SetRefString( aStr );
        else
            aEdAssign.SetText( aStr );

        aEdAssign.SetSelection( Selection( SELECTION_MAX, SELECTION_MAX ) );
        theCurData.Format( aStr, SCR_ABS_3D, pDoc );

        if(bRef)
            aEdAssign2.SetRefString( aStr );
        else
            aEdAssign2.SetText( aStr );
    }
    else
    {
        theCurData = theCurArea = ScRange();

        if(bRef)
        {
            aEdAssign.SetRefString( EMPTY_STRING );
            aEdAssign2.SetRefString( EMPTY_STRING );
        }
        else
        {
            aEdAssign.SetText( EMPTY_STRING );
            aEdAssign2.SetText( EMPTY_STRING );
        }

        aBtnColHead.Disable();
        aBtnRowHead.Disable();
        aEdAssign2.Disable();
    }
}


/*************************************************************************
#*  Member:     AdjustColRowData                            Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   zugehoerigen Datenbereich eines Beschriftungsbereiches
#*              anpassen und Data-Referenz-Edit-Feld fuellen.
#*
#*  Input:      Bereich fuer Labels
#*
#*  Output:     ---
#*
#************************************************************************/

void ScColRowNameRangesDlg::AdjustColRowData( const ScRange& rDataRange,BOOL bRef)
{
    theCurData = rDataRange;
    if ( aBtnColHead.IsChecked() )
    {   // Datenbereich gleiche Spalten wie Koepfe
        theCurData.aStart.SetCol( theCurArea.aStart.Col() );
        theCurData.aEnd.SetCol( theCurArea.aEnd.Col() );
        if ( theCurData.Intersects( theCurArea ) )
        {
            USHORT nRow1 = theCurArea.aStart.Row();
            USHORT nRow2 = theCurArea.aEnd.Row();
            if ( nRow1 > 0
              && (theCurData.aEnd.Row() < nRow2 || nRow2 == MAXROW) )
            {   // Data oben
                theCurData.aEnd.SetRow( nRow1 - 1 );
                if ( theCurData.aStart.Row() > theCurData.aEnd.Row() )
                    theCurData.aStart.SetRow( theCurData.aEnd.Row() );
            }
            else
            {   // Data unten
                theCurData.aStart.SetRow( nRow2 + 1 );
                if ( theCurData.aStart.Row() > theCurData.aEnd.Row() )
                    theCurData.aEnd.SetRow( theCurData.aStart.Row() );
            }
        }
    }
    else
    {   // Datenbereich gleiche Zeilen wie Koepfe
        theCurData.aStart.SetRow( theCurArea.aStart.Row() );
        theCurData.aEnd.SetRow( theCurArea.aEnd.Row() );
        if ( theCurData.Intersects( theCurArea ) )
        {
            USHORT nCol1 = theCurArea.aStart.Col();
            USHORT nCol2 = theCurArea.aEnd.Col();
            if ( nCol1 > 0
              && (theCurData.aEnd.Col() < nCol2 || nCol2 == MAXCOL) )
            {   // Data links
                theCurData.aEnd.SetCol( nCol1 - 1 );
                if ( theCurData.aStart.Col() > theCurData.aEnd.Col() )
                    theCurData.aStart.SetCol( theCurData.aEnd.Col() );
            }
            else
            {   // Data rechts
                theCurData.aStart.SetCol( nCol2 + 1 );
                if ( theCurData.aStart.Col() > theCurData.aEnd.Col() )
                    theCurData.aEnd.SetCol( theCurData.aStart.Col() );
            }
        }
    }
    String aStr;
    theCurData.Format( aStr, SCR_ABS_3D, pDoc );

    if(bRef)
        aEdAssign2.SetRefString( aStr );
    else
        aEdAssign2.SetText( aStr );

    aEdAssign2.SetSelection( Selection( SELECTION_MAX, SELECTION_MAX ) );
}


/*************************************************************************
#*  Member:     SetReference                                Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Uebergabe eines mit der Maus selektierten Tabellen-
#*              bereiches, der dann als neue Selektion im Referenz-
#*              Fenster angezeigt wird.
#*
#*  Input:      Bereich fuer Labels
#*              Dokumentklasse
#*
#*  Output:     ---
#*
#************************************************************************/

void ScColRowNameRangesDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( pEdActive )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pEdActive );

        String aRefStr;
        if ( pEdActive == &aEdAssign )
            SetColRowData( rRef, TRUE );
        else
            AdjustColRowData( rRef, TRUE );
        aBtnColHead.Enable();
        aBtnRowHead.Enable();
        aBtnAdd.Enable();
        aBtnRemove.Disable();
    }
}


/*************************************************************************
#*  Member:     Close                                       Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Schliessen des Fensters
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

BOOL __EXPORT ScColRowNameRangesDlg::Close()
{
    return DoClose( ScColRowNameRangesDlgWrapper::GetChildWindowId() );
}


/*************************************************************************
#*  Member:     SetActive                                   Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Aktivieren des Fensters
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScColRowNameRangesDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = FALSE;
        pEdActive->GrabFocus();
    }
    else
        GrabFocus();
    if ( pEdActive == &aEdAssign )
        Range1DataModifyHdl( 0 );
    else
        Range2DataModifyHdl( 0 );
    RefInputDone();
}


/*************************************************************************
#*  Member:     UpdateNames                                 Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Aktualisieren der Namen
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScColRowNameRangesDlg::UpdateNames()
{
    aLbRange.SetUpdateMode( FALSE );
    //-----------------------------------------------------------
    aLbRange.Clear();
    aEdAssign.SetText( EMPTY_STRING );

    ULONG nCount, j;
    USHORT nPos, i, q; //@008 Hilfsvariable q eingefuegt

    USHORT nCol1;   //@008 04.09.97
    USHORT nRow1;   //Erweiterung fuer Bereichsnamen
    USHORT nTab1;
    USHORT nCol2;
    USHORT nRow2;
    USHORT nTab2;
    String rString;
    String strShow;

    String aString;
    String strDelim = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( " --- " ));
    aString = strDelim;
    aString += ScGlobal::GetRscString( STR_COLUMN );
    aString += strDelim;
    nPos = aLbRange.InsertEntry( aString );
    aLbRange.SetEntryData( nPos, (void*)nEntryDataDelim );
    if ( (nCount = xColNameRanges->Count()) > 0 )
    {
        ScRangePair** ppSortArray = xColNameRanges->CreateNameSortedArray(
            nCount, pDoc );
        for ( j=0; j < nCount; j++ )
        {
            ppSortArray[j]->GetRange(0).Format( aString, SCR_ABS_3D, pDoc );

            //@008 Hole Bereichsparameter aus Dok
            ppSortArray[j]->GetRange(0).GetVars( nCol1, nRow1, nTab1,
                                            nCol2, nRow2, nTab2 );
            q=nCol1+3;
            if(q>nCol2) q=nCol2;
            //@008 Baue String zusammen
            strShow.AssignAscii(RTL_CONSTASCII_STRINGPARAM(" ["));
            if(pDoc!=NULL)
            {
                pDoc->GetString(nCol1, nRow1, nTab1,rString);
                strShow +=rString;
                for(i=nCol1+1;i<=q;i++)
                {
                    strShow.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
                    pDoc->GetString(i, nRow1, nTab1,rString);
                    strShow += rString;
                }
            }
            if(q<nCol2) // Zu lang? Ergaenzen um ",..."
            {
                strShow.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", ..."));
            }
            strShow += ']';

            //@008 String einfuegen in Listbox
            String aInsStr = aString;
            aInsStr += strShow;
            nPos = aLbRange.InsertEntry( aInsStr );
            aLbRange.SetEntryData( nPos, (void*)nEntryDataCol );
        }
        delete [] ppSortArray;
    }
    aString = strDelim;
    aString += ScGlobal::GetRscString( STR_ROW );
    aString += strDelim;
    nPos = aLbRange.InsertEntry( aString );
    aLbRange.SetEntryData( nPos, (void*)nEntryDataDelim );
    if ( (nCount = xRowNameRanges->Count()) > 0 )
    {
        ScRangePair** ppSortArray = xRowNameRanges->CreateNameSortedArray(
            nCount, pDoc );
        for ( j=0; j < nCount; j++ )
        {
            ppSortArray[j]->GetRange(0).Format( aString, SCR_ABS_3D, pDoc );

            //@008 Ab hier baue String fuer Zeilen
            ppSortArray[j]->GetRange(0).GetVars( nCol1, nRow1, nTab1,
                                            nCol2, nRow2, nTab2 );
            q=nRow1+3;
            if(q>nRow2) q=nRow2;
            strShow.AssignAscii(RTL_CONSTASCII_STRINGPARAM(" ["));
            if(pDoc!=NULL)
            {
                pDoc->GetString(nCol1, nRow1, nTab1,rString);
                strShow += rString;
                for(i=nRow1+1;i<=q;i++)
                {
                    strShow.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
                    pDoc->GetString(nCol1, i, nTab1,rString);
                    strShow += rString;
                }
            }
            if(q<nRow2)
            {
                strShow.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", ..."));
            }
            strShow += ']';

            String aInsStr = aString;
            aInsStr += strShow;
            nPos = aLbRange.InsertEntry( aInsStr );
            aLbRange.SetEntryData( nPos, (void*)nEntryDataRow );
        }
        delete [] ppSortArray;
    }
    //-----------------------------------------------------------
    aLbRange.SetUpdateMode( TRUE );
    aLbRange.Invalidate();
}


/*************************************************************************
#*  Member:     UpdateRangeData                             Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Aktualisieren der Bereichsdaten
#*
#*  Input:      Bereichs-String
#*              Flag fuer Spalten
#*
#*  Output:     ---
#*
#************************************************************************/

void ScColRowNameRangesDlg::UpdateRangeData( const String& rRangeStr, BOOL bColName )
{
    ScRange aRange;
    String aRefString=rRangeStr;

    //@008 Suchen nach Erweiterung u. rausschmeissen
    xub_StrLen nPosExt=rRangeStr.Search( '[',0 );

    if(nPosExt!=STRING_NOTFOUND)
    {
        nPosExt--;
        aRefString.Erase(nPosExt);
    }
    aRange.ParseAny( aRefString, pDoc );

    ScRangePair* pPair;
    BOOL bFound = FALSE;
    if ( bColName && (pPair = xColNameRanges->Find( aRange )) )
        bFound = TRUE;
    else if ( !bColName && (pPair = xRowNameRanges->Find( aRange )) )
        bFound = TRUE;

    if ( bFound )
    {
        String aStr;
        theCurArea = aRange;
        theCurArea.Format( aStr, SCR_ABS_3D, pDoc );
        aEdAssign.SetText( aStr );
        aBtnAdd.Disable();
        aBtnRemove.Enable();
        aBtnColHead.Check( bColName );
        aBtnRowHead.Check( !bColName );
        theCurData = pPair->GetRange(1);
        theCurData.Format( aStr, SCR_ABS_3D, pDoc );
        aEdAssign2.SetText( aStr );
    }
    else
    {
        aBtnAdd.Enable();
        aBtnRemove.Disable();
    }
    aBtnColHead.Enable();
    aBtnRowHead.Enable();
    aEdAssign2.Enable();
}


/*************************************************************************
#*  Member:     IsRefInputMode                              Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Abfragefunktion fuer Referenz- Input- Mode.
#*
#*  Input:      Bereichs-String
#*              Flag fuer Spalten
#*
#*  Output:     true, wenn Referenz- Input- Mode
#*
#************************************************************************/

BOOL ScColRowNameRangesDlg::IsRefInputMode() const
{
    return (pEdActive != NULL);
}

//------------------------------------------------------------------------
// Handler:
// ========

/*************************************************************************
#*  Handler:    OkBtnHdl                                    Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Wird ausgeloest, wenn der OK- Button gedrückt wurde.
#*              Hinzufuegen- Button ausloesen, und die neu einge-
#*              stellten Bereiche ans Dokument uebergeben.
#*              Fensterschliessen- Anweisung ausloesen.
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( ScColRowNameRangesDlg, OkBtnHdl, void *, EMPTYARG )
{
    AddBtnHdl( 0 );

    // die RangeLists den Refs am Doc zuweisen
    pDoc->GetColNameRangesRef() = xColNameRanges;
    pDoc->GetRowNameRangesRef() = xRowNameRanges;
    // geaenderte Datenbereiche muessen sich auswirken
    pDoc->CompileColRowNameFormula();
    ScDocShell* pDocShell = pViewData->GetDocShell();
    pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
    pDocShell->SetDocumentModified();

    Close();
    return 0;
}


/*************************************************************************
#*  Handler:    CancelBtnHdl                                Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Fensterschliessen- Anweisung ausloesen.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_INLINE_START( ScColRowNameRangesDlg, CancelBtnHdl, void *, EMPTYARG )
{
    Close();
    return 0;
}
IMPL_LINK_INLINE_END( ScColRowNameRangesDlg, CancelBtnHdl, void *, EMPTYARG )


/*************************************************************************
#*  Handler:    AddBtnHdl                                   Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Nach betaetigen des Hinzufuegen- Buttons, werden
#*              die Bereichsangaben eingestellt und in der
#*              Listbox dargestellt.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( ScColRowNameRangesDlg, AddBtnHdl, void *, EMPTYARG )
{
    String aNewArea( aEdAssign.GetText() );
    String aNewData( aEdAssign2.GetText() );

    if ( aNewArea.Len() > 0 && aNewData.Len() > 0 )
    {
        ScRange aRange1, aRange2;
        BOOL bOk1;
        if ( (bOk1 = ((aRange1.ParseAny( aNewArea, pDoc ) & SCA_VALID) == SCA_VALID))
          && ((aRange2.ParseAny( aNewData, pDoc ) & SCA_VALID) == SCA_VALID) )
        {
            theCurArea = aRange1;
            AdjustColRowData( aRange2 );
            ScRangePair* pPair;
            if ( pPair = xColNameRanges->Find( theCurArea ) )
            {
                xColNameRanges->Remove( pPair );
                delete pPair;
            }
            if ( pPair = xRowNameRanges->Find( theCurArea ) )
            {
                xRowNameRanges->Remove( pPair );
                delete pPair;
            }
            if ( aBtnColHead.IsChecked() )
                xColNameRanges->Join( ScRangePair( theCurArea, theCurData ) );
            else
                xRowNameRanges->Join( ScRangePair( theCurArea, theCurData ) );

            UpdateNames();

            aEdAssign.GrabFocus();
            aBtnAdd.Disable();
            aBtnRemove.Disable();
            aEdAssign.SetText( EMPTY_STRING );
            aBtnColHead.Check( TRUE );
            aBtnRowHead.Check( FALSE );
            aEdAssign2.SetText( EMPTY_STRING );
            theCurArea = ScRange();
            theCurData = theCurArea;
            Range1SelectHdl( 0 );
        }
        else
        {
            ERRORBOX( ScGlobal::GetRscString(STR_INVALIDTABNAME) );
            if ( !bOk1 )
                aEdAssign.GrabFocus();
            else
                aEdAssign2.GrabFocus();
        }
    }
    return 0;
}


/*************************************************************************
#*  Handler:    RemoveBtnHdl                                Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Nach betaetigen des Loeschen- Buttons, wird
#*              die markierte Bereichsangabe geloescht.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( ScColRowNameRangesDlg, RemoveBtnHdl, void *, EMPTYARG )
{
    String aRangeStr = aLbRange.GetSelectEntry();
    USHORT nSelectPos = aLbRange.GetSelectEntryPos();
    BOOL bColName =
        ((ULONG)aLbRange.GetEntryData( nSelectPos ) == nEntryDataCol);
    ScRange aRange;

    //@008 Suchen nach Erweiterung u. rausschmeissen
    String aRefString=aRangeStr;

    xub_StrLen nPosExt=aRangeStr.Search( '[', 0 );

    if(nPosExt!=STRING_NOTFOUND)
    {
        nPosExt--;
        aRefString.Erase(nPosExt);
    }

    aRange.ParseAny( aRefString, pDoc );

    ScRangePair* pPair;
    BOOL bFound = FALSE;
    if ( bColName && (pPair = xColNameRanges->Find( aRange )) )
        bFound = TRUE;
    else if ( !bColName && (pPair = xRowNameRanges->Find( aRange )) )
        bFound = TRUE;
    if ( bFound )
    {
        String aStrDelMsg = ScGlobal::GetRscString( STR_QUERY_DELENTRY );
        String aMsg       = aStrDelMsg.GetToken( 0, '#' );

        aMsg += aRangeStr;
        aMsg += aStrDelMsg.GetToken( 1, '#' );

        if ( RET_YES == QUERYBOX(aMsg) )
        {
            if ( bColName )
                xColNameRanges->Remove( pPair );
            else
                xRowNameRanges->Remove( pPair );
            delete pPair;

            UpdateNames();
            USHORT nCnt = aLbRange.GetEntryCount();
            if ( nSelectPos >= nCnt )
            {
                if ( nCnt )
                    nSelectPos = nCnt - 1;
                else
                    nSelectPos = 0;
            }
            aLbRange.SelectEntryPos( nSelectPos );
            if ( nSelectPos &&
                    (ULONG)aLbRange.GetEntryData( nSelectPos ) == nEntryDataDelim )
                aLbRange.SelectEntryPos( --nSelectPos );    // ---Zeile---

            aLbRange.GrabFocus();
            aBtnAdd.Disable();
            aBtnRemove.Disable();
            aEdAssign.SetText( EMPTY_STRING );
            theCurArea = theCurData = ScRange();
            aBtnColHead.Check( TRUE );
            aBtnRowHead.Check( FALSE );
            aEdAssign2.SetText( EMPTY_STRING );
            Range1SelectHdl( 0 );
        }
    }
    return 0;
}


/*************************************************************************
#*  Handler:    Range1SelectHdl                             Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Wenn Zeile in Listbox ausgewaehlt wird,
#*              werden die Eingabefelder entsprechend
#*              eingestellt.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( ScColRowNameRangesDlg, Range1SelectHdl, void *, EMPTYARG )
{
    USHORT nSelectPos = aLbRange.GetSelectEntryPos();
    USHORT nCnt = aLbRange.GetEntryCount();
    USHORT nMoves = 0;
    while ( nSelectPos < nCnt
            && (ULONG)aLbRange.GetEntryData( nSelectPos ) == nEntryDataDelim )
    {   // skip Delimiter
        ++nMoves;
        aLbRange.SelectEntryPos( ++nSelectPos );
    }
    String aRangeStr = aLbRange.GetSelectEntry();
    if ( nMoves )
    {
        if ( nSelectPos > 1 && nSelectPos >= nCnt )
        {   // am Ende nicht auf dem " --- Zeile --- " Delimiter stehenbleiben
            // wenn davor Eintraege existieren
            nSelectPos = nCnt - 2;
            aLbRange.SelectEntryPos( nSelectPos );
            aRangeStr = aLbRange.GetSelectEntry();
        }
        else if ( nSelectPos > 2 && nSelectPos < nCnt && aRangeStr.Len()
                && aRangeStr == aEdAssign.GetText() )
        {   // nach oben wandern statt nach unten auf die vorherige Position
            nSelectPos -= 2;
            aLbRange.SelectEntryPos( nSelectPos );
            aRangeStr = aLbRange.GetSelectEntry();
        }
    }
    if ( aRangeStr.Len() && aRangeStr.GetChar(0) == '$' )
    {
        BOOL bColName =
            ((ULONG)aLbRange.GetEntryData( nSelectPos ) == nEntryDataCol);
        UpdateRangeData( aRangeStr, bColName );
        aBtnAdd.Disable();
        aBtnRemove.Enable();
    }
    else
    {
        if ( aEdAssign.GetText().Len() > 0 )
        {
            if ( aEdAssign2.GetText().Len() > 0 )
                aBtnAdd.Enable();
            else
                aBtnAdd.Disable();
            aBtnColHead.Enable();
            aBtnRowHead.Enable();
            aEdAssign2.Enable();
        }
        else
        {
            aBtnAdd.Disable();
            aBtnColHead.Disable();
            aBtnRowHead.Disable();
            aEdAssign2.Disable();
        }
        aBtnRemove.Disable();
        aEdAssign.GrabFocus();
    }

    aGbAssign.Enable();
    aEdAssign.Enable();
    aRbAssign.Enable();

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();
    return 0;
}


/*************************************************************************
#*  Handler:    Range1DataModifyHdl                         Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Wird ausgeloest, wenn in der Tabelle, der Label-
#*              Bereich geaendert wurde.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( ScColRowNameRangesDlg, Range1DataModifyHdl, void *, EMPTYARG )
{
    String aNewArea( aEdAssign.GetText() );
    BOOL bValid = FALSE;
    if ( aNewArea.Len() > 0 )
    {
        ScRange aRange;
        if ( (aRange.ParseAny( aNewArea, pDoc ) & SCA_VALID) == SCA_VALID )
        {
            SetColRowData( aRange );
            bValid = TRUE;
        }
    }
    if ( bValid )
    {
        aBtnAdd.Enable();
        aBtnColHead.Enable();
        aBtnRowHead.Enable();
        aEdAssign2.Enable();
    }
    else
    {
        aBtnAdd.Disable();
        aBtnColHead.Disable();
        aBtnRowHead.Disable();
        aEdAssign2.Disable();
    }
    aBtnRemove.Disable();
    return 0;
}


/*************************************************************************
#*  Handler:    Range2DataModifyHdl                         Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Wird ausgeloest, wenn in der Tabelle, der Daten-
#*              Bereich geaendert wurde
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( ScColRowNameRangesDlg, Range2DataModifyHdl, void *, EMPTYARG )
{
    String aNewData( aEdAssign2.GetText() );
    if ( aNewData.Len() > 0 )
    {
        ScRange aRange;
        if ( (aRange.ParseAny( aNewData, pDoc ) & SCA_VALID) == SCA_VALID )
        {
            AdjustColRowData( aRange );
            aBtnAdd.Enable();
        }
        else
            aBtnAdd.Disable();
    }
    else
    {
        aBtnAdd.Disable();
    }
    return 0;
}


/*************************************************************************
#*  Handler:    ColClickHdl                                 Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Radiobutton fuer Spalten wurde betaetigt,
#*              die entsprechenden Einstellungen werden
#*              vorgenommen
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( ScColRowNameRangesDlg, ColClickHdl, void *, EMPTYARG )
{
    if ( !aBtnColHead.GetSavedValue() )
    {
        aBtnColHead.Check( TRUE );
        aBtnRowHead.Check( FALSE );
        if ( theCurArea.aStart.Row() == 0 && theCurArea.aEnd.Row() == MAXROW )
        {
            theCurArea.aEnd.SetRow( MAXROW - 1 );
            String aStr;
            theCurArea.Format( aStr, SCR_ABS_3D, pDoc );
            aEdAssign.SetText( aStr );
        }
        ScRange aRange( theCurData );
        aRange.aStart.SetRow((USHORT) Min( (long)(theCurArea.aEnd.Row() + 1), (long)MAXROW ) );
        aRange.aEnd.SetRow( MAXROW );
        AdjustColRowData( aRange );
    }
    return 0;
}


/*************************************************************************
#*  Handler:    RowClickHdl                                 Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Radiobutton fuer Zeilen wurde betaetigt,
#*              die entsprechenden Einstellungen werden
#*              vorgenommen
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( ScColRowNameRangesDlg, RowClickHdl, void *, EMPTYARG )
{
    if ( !aBtnRowHead.GetSavedValue() )
    {
        aBtnRowHead.Check( TRUE );
        aBtnColHead.Check( FALSE );
        if ( theCurArea.aStart.Col() == 0 && theCurArea.aEnd.Col() == MAXCOL )
        {
            theCurArea.aEnd.SetCol( MAXCOL - 1 );
            String aStr;
            theCurArea.Format( aStr, SCR_ABS_3D, pDoc );
            aEdAssign.SetText( aStr );
        }
        ScRange aRange( theCurData );
        aRange.aStart.SetCol( (USHORT) Min( (long)(theCurArea.aEnd.Col() + 1), (long)MAXCOL ) );
        aRange.aEnd.SetCol( MAXCOL );
        AdjustColRowData( aRange );
    }
    return 0;
}


/*************************************************************************
#*  Handler:    EdGetFocusHdl                               Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Wird ausgeloest, wenn das Fenster einen Focus
#*              erhaelt.
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( ScColRowNameRangesDlg, EdGetFocusHdl, ScRefEdit*, pEd )
{
    pEdActive = pEd;
    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();
    pEd->SetSelection( Selection(0,SELECTION_MAX) );
    return 0;
}


/*************************************************************************
#*  Handler:    EdLoseFocusHdl                              Datum:04.09.97
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScColRowNameRangesDlg
#*
#*  Funktion:   Wird ausgeloest, wenn das Fenster seinen Focus
#*              verliert.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK( ScColRowNameRangesDlg, EdLoseFocusHdl, ScRefEdit*, pEd )
{
    bDlgLostFocus = !IsActive();
    return 0;
}
