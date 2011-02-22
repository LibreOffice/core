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

// System - Includes ---------------------------------------------------------



// INCLUDE -------------------------------------------------------------------

#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "crnrdlg.hrc"
#include "docsh.hxx"

#define _CRNRDLG_CXX
#include "crnrdlg.hxx"
#undef _CRNRDLG_CXX
#include <vcl/msgbox.hxx>


//============================================================================

#define ERRORBOX(s) ErrorBox(this,WinBits(WB_OK|WB_DEF_OK),s).Execute()
#define QUERYBOX(m) QueryBox(this,WinBits(WB_YES_NO|WB_DEF_YES),m).Execute()

const sal_uLong nEntryDataCol = 0;
const sal_uLong nEntryDataRow = 1;
const sal_uLong nEntryDataDelim = 2;


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
        aFlAssign       ( this, ScResId( FL_ASSIGN ) ),
        aLbRange        ( this, ScResId( LB_RANGE ) ),

        aEdAssign       ( this, this, ScResId( ED_AREA ) ),
        aRbAssign       ( this, ScResId( RB_AREA ), &aEdAssign, this ),
        aBtnColHead     ( this, ScResId( BTN_COLHEAD ) ),
        aBtnRowHead     ( this, ScResId( BTN_ROWHEAD ) ),
        aFtAssign2      ( this, ScResId( FT_DATA_LABEL ) ),
        aEdAssign2      ( this, this, ScResId( ED_DATA ) ),
        aRbAssign2      ( this, ScResId( RB_DATA ), &aEdAssign2, this ),

        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnAdd         ( this, ScResId( BTN_ADD ) ),
        aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),

        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData->GetDocument() ),

        pEdActive       ( NULL ),
        bDlgLostFocus   ( sal_False )
{
    xColNameRanges = pDoc->GetColNameRanges()->Clone();
    xRowNameRanges = pDoc->GetRowNameRanges()->Clone();
    Init();
    FreeResource();

    aRbAssign.SetAccessibleRelationMemberOf(&aEdAssign);
    aRbAssign2.SetAccessibleRelationMemberOf(&aEdAssign);
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
    SCCOL   nStartCol   = 0;
    SCROW   nStartRow   = 0;
    SCTAB   nStartTab   = 0;
    SCCOL   nEndCol     = 0;
    SCROW   nEndRow     = 0;
    SCTAB   nEndTab     = 0;

    aBtnOk.SetClickHdl      ( LINK( this, ScColRowNameRangesDlg, OkBtnHdl ) );
    aBtnCancel.SetClickHdl  ( LINK( this, ScColRowNameRangesDlg, CancelBtnHdl ) );
    aBtnAdd.SetClickHdl     ( LINK( this, ScColRowNameRangesDlg, AddBtnHdl ) );
    aBtnRemove.SetClickHdl  ( LINK( this, ScColRowNameRangesDlg, RemoveBtnHdl ) );
    aLbRange.SetSelectHdl   ( LINK( this, ScColRowNameRangesDlg, Range1SelectHdl ) );
    aEdAssign.SetModifyHdl  ( LINK( this, ScColRowNameRangesDlg, Range1DataModifyHdl ) );
    aBtnColHead.SetClickHdl ( LINK( this, ScColRowNameRangesDlg, ColClickHdl ) );
    aBtnRowHead.SetClickHdl ( LINK( this, ScColRowNameRangesDlg, RowClickHdl ) );
    aEdAssign2.SetModifyHdl ( LINK( this, ScColRowNameRangesDlg, Range2DataModifyHdl ) );

    Link aLink = LINK( this, ScColRowNameRangesDlg, GetFocusHdl );
    aEdAssign.SetGetFocusHdl( aLink );
    aRbAssign.SetGetFocusHdl( aLink );
    aEdAssign2.SetGetFocusHdl( aLink );
    aRbAssign2.SetGetFocusHdl( aLink );

    aLink = LINK( this, ScColRowNameRangesDlg, LoseFocusHdl );
    aEdAssign.SetLoseFocusHdl( aLink );
    aRbAssign.SetLoseFocusHdl( aLink );
    aEdAssign2.SetLoseFocusHdl( aLink );
    aRbAssign2.SetLoseFocusHdl( aLink );

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
        aBtnColHead.Check( sal_True );
        aBtnRowHead.Check( sal_False );
        aEdAssign.SetText( EMPTY_STRING );
        aEdAssign2.SetText( EMPTY_STRING );
    }

    aLbRange.SetBorderStyle( WINDOW_BORDER_MONO );
    aBtnColHead.Enable();
    aBtnRowHead.Enable();
    aEdAssign.Enable();
    aEdAssign.GrabFocus();
    aRbAssign.Enable();
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

void ScColRowNameRangesDlg::SetColRowData( const ScRange& rLabelRange,sal_Bool bRef)
{
    theCurData = theCurArea = rLabelRange;
    sal_Bool bValid = sal_True;
    SCCOL nCol1 = theCurArea.aStart.Col();
    SCCOL nCol2 = theCurArea.aEnd.Col();
    SCROW nRow1 = theCurArea.aStart.Row();
    SCROW nRow2 = theCurArea.aEnd.Row();
    if ( (static_cast<SCCOLROW>(nCol2 - nCol1) >= nRow2 - nRow1) || (nCol1 == 0 && nCol2 == MAXCOL) )
    {   // Spaltenkoepfe und Grenzfall gesamte Tabelle
        aBtnColHead.Check( sal_True );
        aBtnRowHead.Check( sal_False );
        if ( nRow2 == MAXROW  )
        {
            if ( nRow1 == 0 )
                bValid = sal_False;     // Grenzfall gesamte Tabelle
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
        aBtnRowHead.Check( sal_True );
        aBtnColHead.Check( sal_False );
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
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        String aStr;
        theCurArea.Format( aStr, SCR_ABS_3D, pDoc, eConv );

        if(bRef)
            aEdAssign.SetRefString( aStr );
        else
            aEdAssign.SetText( aStr );

        aEdAssign.SetSelection( Selection( SELECTION_MAX, SELECTION_MAX ) );
        theCurData.Format( aStr, SCR_ABS_3D, pDoc, eConv );

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
        aRbAssign2.Disable();
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

void ScColRowNameRangesDlg::AdjustColRowData( const ScRange& rDataRange,sal_Bool bRef)
{
    theCurData = rDataRange;
    if ( aBtnColHead.IsChecked() )
    {   // Datenbereich gleiche Spalten wie Koepfe
        theCurData.aStart.SetCol( theCurArea.aStart.Col() );
        theCurData.aEnd.SetCol( theCurArea.aEnd.Col() );
        if ( theCurData.Intersects( theCurArea ) )
        {
            SCROW nRow1 = theCurArea.aStart.Row();
            SCROW nRow2 = theCurArea.aEnd.Row();
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
            SCCOL nCol1 = theCurArea.aStart.Col();
            SCCOL nCol2 = theCurArea.aEnd.Col();
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
    theCurData.Format( aStr, SCR_ABS_3D, pDoc, pDoc->GetAddressConvention() );

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

void ScColRowNameRangesDlg::SetReference( const ScRange& rRef, ScDocument* /* pDoc */ )
{
    if ( pEdActive )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pEdActive );

        String aRefStr;
        if ( pEdActive == &aEdAssign )
            SetColRowData( rRef, sal_True );
        else
            AdjustColRowData( rRef, sal_True );
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

sal_Bool __EXPORT ScColRowNameRangesDlg::Close()
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
        bDlgLostFocus = sal_False;
        if( pEdActive )
            pEdActive->GrabFocus();
    }
    else
        GrabFocus();

    if( pEdActive == &aEdAssign )
        Range1DataModifyHdl( 0 );
    else if( pEdActive == &aEdAssign2 )
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
    aLbRange.SetUpdateMode( sal_False );
    //-----------------------------------------------------------
    aLbRange.Clear();
    aRangeMap.clear();
    aEdAssign.SetText( EMPTY_STRING );

    sal_uLong nCount, j;
    sal_uInt16 nPos; //@008 Hilfsvariable q eingefuegt

    SCCOL nCol1;    //@008 04.09.97
    SCROW nRow1;    //Erweiterung fuer Bereichsnamen
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    String rString;
    String strShow;
    const ScAddress::Details aDetails(pDoc->GetAddressConvention());

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
            const ScRange aRange(ppSortArray[j]->GetRange(0));
            aRange.Format( aString, SCR_ABS_3D, pDoc, aDetails );

            //@008 Hole Bereichsparameter aus Dok
            ppSortArray[j]->GetRange(0).GetVars( nCol1, nRow1, nTab1,
                                            nCol2, nRow2, nTab2 );
            SCCOL q=nCol1+3;
            if(q>nCol2) q=nCol2;
            //@008 Baue String zusammen
            strShow.AssignAscii(RTL_CONSTASCII_STRINGPARAM(" ["));
            if(pDoc!=NULL)
            {
                pDoc->GetString(nCol1, nRow1, nTab1,rString);
                strShow +=rString;
                for(SCCOL i=nCol1+1;i<=q;i++)
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
            aRangeMap.insert( NameRangeMap::value_type(aInsStr, aRange) );
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
            const ScRange aRange(ppSortArray[j]->GetRange(0));
            aRange.Format( aString, SCR_ABS_3D, pDoc, aDetails );

            //@008 Ab hier baue String fuer Zeilen
            ppSortArray[j]->GetRange(0).GetVars( nCol1, nRow1, nTab1,
                                            nCol2, nRow2, nTab2 );
            SCROW q=nRow1+3;
            if(q>nRow2) q=nRow2;
            strShow.AssignAscii(RTL_CONSTASCII_STRINGPARAM(" ["));
            if(pDoc!=NULL)
            {
                pDoc->GetString(nCol1, nRow1, nTab1,rString);
                strShow += rString;
                for(SCROW i=nRow1+1;i<=q;i++)
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
            aRangeMap.insert( NameRangeMap::value_type(aInsStr, aRange) );
            aLbRange.SetEntryData( nPos, (void*)nEntryDataRow );
        }
        delete [] ppSortArray;
    }
    //-----------------------------------------------------------
    aLbRange.SetUpdateMode( sal_True );
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

void ScColRowNameRangesDlg::UpdateRangeData( const ScRange& rRange, sal_Bool bColName )
{
    ScRangePair* pPair = NULL;
    sal_Bool bFound = sal_False;
    if ( bColName && (pPair = xColNameRanges->Find( rRange )) != NULL )
        bFound = sal_True;
    else if ( !bColName && (pPair = xRowNameRanges->Find( rRange )) != NULL )
        bFound = sal_True;

    if ( bFound )
    {
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        String aStr;
        theCurArea = rRange;
        theCurArea.Format( aStr, SCR_ABS_3D, pDoc, eConv );
        aEdAssign.SetText( aStr );
        aBtnAdd.Disable();
        aBtnRemove.Enable();
        aBtnColHead.Check( bColName );
        aBtnRowHead.Check( !bColName );
        theCurData = pPair->GetRange(1);
        theCurData.Format( aStr, SCR_ABS_3D, pDoc, eConv );
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
    aRbAssign2.Enable();
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

sal_Bool ScColRowNameRangesDlg::IsRefInputMode() const
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
#*  Funktion:   Wird ausgeloest, wenn der OK- Button gedrueckt wurde.
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
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        ScRange aRange1, aRange2;
        sal_Bool bOk1;
        if ( (bOk1 = ((aRange1.ParseAny( aNewArea, pDoc, eConv ) & SCA_VALID) == SCA_VALID)) != sal_False
          && ((aRange2.ParseAny( aNewData, pDoc, eConv ) & SCA_VALID) == SCA_VALID) )
        {
            theCurArea = aRange1;
            AdjustColRowData( aRange2 );
            ScRangePair* pPair;
            if ( ( pPair = xColNameRanges->Find( theCurArea ) ) != NULL )
            {
                xColNameRanges->Remove( pPair );
                delete pPair;
            }
            if ( ( pPair = xRowNameRanges->Find( theCurArea ) ) != NULL )
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
            aBtnColHead.Check( sal_True );
            aBtnRowHead.Check( sal_False );
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
    sal_uInt16 nSelectPos = aLbRange.GetSelectEntryPos();
    sal_Bool bColName =
        ((sal_uLong)aLbRange.GetEntryData( nSelectPos ) == nEntryDataCol);
    NameRangeMap::const_iterator itr = aRangeMap.find(aRangeStr);
    if (itr == aRangeMap.end())
        return 0;
    const ScRange& rRange = itr->second;

    ScRangePair* pPair = NULL;
    sal_Bool bFound = sal_False;
    if ( bColName && (pPair = xColNameRanges->Find( rRange )) != NULL )
        bFound = sal_True;
    else if ( !bColName && (pPair = xRowNameRanges->Find( rRange )) != NULL )
        bFound = sal_True;
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
            sal_uInt16 nCnt = aLbRange.GetEntryCount();
            if ( nSelectPos >= nCnt )
            {
                if ( nCnt )
                    nSelectPos = nCnt - 1;
                else
                    nSelectPos = 0;
            }
            aLbRange.SelectEntryPos( nSelectPos );
            if ( nSelectPos &&
                    (sal_uLong)aLbRange.GetEntryData( nSelectPos ) == nEntryDataDelim )
                aLbRange.SelectEntryPos( --nSelectPos );    // ---Zeile---

            aLbRange.GrabFocus();
            aBtnAdd.Disable();
            aBtnRemove.Disable();
            aEdAssign.SetText( EMPTY_STRING );
            theCurArea = theCurData = ScRange();
            aBtnColHead.Check( sal_True );
            aBtnRowHead.Check( sal_False );
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
    sal_uInt16 nSelectPos = aLbRange.GetSelectEntryPos();
    sal_uInt16 nCnt = aLbRange.GetEntryCount();
    sal_uInt16 nMoves = 0;
    while ( nSelectPos < nCnt
            && (sal_uLong)aLbRange.GetEntryData( nSelectPos ) == nEntryDataDelim )
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
    NameRangeMap::const_iterator itr = aRangeMap.find(aRangeStr);
    if ( itr != aRangeMap.end() )
    {
        sal_Bool bColName =
            ((sal_uLong)aLbRange.GetEntryData( nSelectPos ) == nEntryDataCol);
        UpdateRangeData( itr->second, bColName );
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
            aRbAssign2.Enable();
        }
        else
        {
            aBtnAdd.Disable();
            aBtnColHead.Disable();
            aBtnRowHead.Disable();
            aEdAssign2.Disable();
            aRbAssign2.Disable();
        }
        aBtnRemove.Disable();
        aEdAssign.GrabFocus();
    }

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
    sal_Bool bValid = sal_False;
    if ( aNewArea.Len() > 0 )
    {
        ScRange aRange;
        if ( (aRange.ParseAny( aNewArea, pDoc, pDoc->GetAddressConvention() ) & SCA_VALID) == SCA_VALID )
        {
            SetColRowData( aRange );
            bValid = sal_True;
        }
    }
    if ( bValid )
    {
        aBtnAdd.Enable();
        aBtnColHead.Enable();
        aBtnRowHead.Enable();
        aEdAssign2.Enable();
        aRbAssign2.Enable();
    }
    else
    {
        aBtnAdd.Disable();
        aBtnColHead.Disable();
        aBtnRowHead.Disable();
        aEdAssign2.Disable();
        aRbAssign2.Disable();
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
        if ( (aRange.ParseAny( aNewData, pDoc, pDoc->GetAddressConvention() ) & SCA_VALID) == SCA_VALID )
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
        aBtnColHead.Check( sal_True );
        aBtnRowHead.Check( sal_False );
        if ( theCurArea.aStart.Row() == 0 && theCurArea.aEnd.Row() == MAXROW )
        {
            theCurArea.aEnd.SetRow( MAXROW - 1 );
            String aStr;
            theCurArea.Format( aStr, SCR_ABS_3D, pDoc, pDoc->GetAddressConvention() );
            aEdAssign.SetText( aStr );
        }
        ScRange aRange( theCurData );
        aRange.aStart.SetRow( Min( (long)(theCurArea.aEnd.Row() + 1), (long)MAXROW ) );
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
        aBtnRowHead.Check( sal_True );
        aBtnColHead.Check( sal_False );
        if ( theCurArea.aStart.Col() == 0 && theCurArea.aEnd.Col() == MAXCOL )
        {
            theCurArea.aEnd.SetCol( MAXCOL - 1 );
            String aStr;
            theCurArea.Format( aStr, SCR_ABS_3D, pDoc, pDoc->GetAddressConvention() );
            aEdAssign.SetText( aStr );
        }
        ScRange aRange( theCurData );
        aRange.aStart.SetCol( static_cast<SCCOL>(Min( (long)(theCurArea.aEnd.Col() + 1), (long)MAXCOL )) );
        aRange.aEnd.SetCol( MAXCOL );
        AdjustColRowData( aRange );
    }
    return 0;
}


IMPL_LINK( ScColRowNameRangesDlg, GetFocusHdl, Control*, pCtrl )
{
    if( (pCtrl == (Control*)&aEdAssign) || (pCtrl == (Control*)&aRbAssign) )
        pEdActive = &aEdAssign;
    else if( (pCtrl == (Control*)&aEdAssign2) || (pCtrl == (Control*)&aRbAssign2) )
        pEdActive = &aEdAssign2;
    else
        pEdActive = NULL;

    if( pEdActive )
        pEdActive->SetSelection( Selection( 0, SELECTION_MAX ) );

    return 0;
}


IMPL_LINK( ScColRowNameRangesDlg, LoseFocusHdl, Control*, EMPTYARG )
{
    bDlgLostFocus = !IsActive();
    return 0;
}
