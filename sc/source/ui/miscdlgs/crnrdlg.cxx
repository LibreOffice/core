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

#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
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
#*  Member:     ScColRowNameRangesDlg
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

    :   ScAnyRefDlg ( pB, pCW, pParent, "NameRangesDialog" , "modules/scalc/ui/namerangesdialog.ui" ),

        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData->GetDocument() ),

        pEdActive       ( NULL ),
        bDlgLostFocus   ( false )
{
    get(pLbRange,"range");

    get(pEdAssign,"edassign");
    get(pRbAssign,"rbassign");
    pRbAssign->SetReferences(this, pEdAssign);
    pEdAssign->SetReferences(this, get<VclFrame>("rangeframe")->get_label_widget());
    get(pBtnColHead,"colhead");
    get(pBtnRowHead,"rowhead");
    get(pEdAssign2,"edassign2");
    get(pRbAssign2,"rbassign2");
    pRbAssign2->SetReferences(this, pEdAssign2);
    pEdAssign2->SetReferences(this, get<FixedText>("datarange"));

    get(pBtnOk,"ok");
    get(pBtnCancel,"cancel");
    get(pBtnAdd,"add");
    get(pBtnRemove,"delete");

    xColNameRanges = pDoc->GetColNameRanges()->Clone();
    xRowNameRanges = pDoc->GetRowNameRanges()->Clone();
    Init();
}


/*************************************************************************
#*  Member:     ~ScColRowNameRangesDlg
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

ScColRowNameRangesDlg::~ScColRowNameRangesDlg()
{
}


/*************************************************************************
#*  Member:     Init
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
    pBtnOk->SetClickHdl      ( LINK( this, ScColRowNameRangesDlg, OkBtnHdl ) );
    pBtnCancel->SetClickHdl  ( LINK( this, ScColRowNameRangesDlg, CancelBtnHdl ) );
    pBtnAdd->SetClickHdl     ( LINK( this, ScColRowNameRangesDlg, AddBtnHdl ) );
    pBtnRemove->SetClickHdl  ( LINK( this, ScColRowNameRangesDlg, RemoveBtnHdl ) );
    pLbRange->SetSelectHdl   ( LINK( this, ScColRowNameRangesDlg, Range1SelectHdl ) );
    pEdAssign->SetModifyHdl  ( LINK( this, ScColRowNameRangesDlg, Range1DataModifyHdl ) );
    pBtnColHead->SetClickHdl ( LINK( this, ScColRowNameRangesDlg, ColClickHdl ) );
    pBtnRowHead->SetClickHdl ( LINK( this, ScColRowNameRangesDlg, RowClickHdl ) );
    pEdAssign2->SetModifyHdl ( LINK( this, ScColRowNameRangesDlg, Range2DataModifyHdl ) );

    Link aLink = LINK( this, ScColRowNameRangesDlg, GetFocusHdl );
    pEdAssign->SetGetFocusHdl( aLink );
    pRbAssign->SetGetFocusHdl( aLink );
    pEdAssign2->SetGetFocusHdl( aLink );
    pRbAssign2->SetGetFocusHdl( aLink );

    aLink = LINK( this, ScColRowNameRangesDlg, LoseFocusHdl );
    pEdAssign->SetLoseFocusHdl( aLink );
    pRbAssign->SetLoseFocusHdl( aLink );
    pEdAssign2->SetLoseFocusHdl( aLink );
    pRbAssign2->SetLoseFocusHdl( aLink );

    pLbRange->SetDropDownLineCount(10);

    pEdActive = pEdAssign;

    UpdateNames();

    if ( pViewData && pDoc )
    {
        SCCOL nStartCol = 0;
        SCROW nStartRow = 0;
        SCTAB nStartTab = 0;
        SCCOL nEndCol   = 0;
        SCROW nEndRow   = 0;
        SCTAB nEndTab   = 0;
        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,  nEndTab );
        SetColRowData( ScRange( ScAddress( nStartCol, nStartRow, nStartTab ),
                              ScAddress( nEndCol,   nEndRow,   nEndTab ) ) );
    }
    else
    {
        pBtnColHead->Check( sal_True );
        pBtnRowHead->Check( false );
        pEdAssign->SetText( EMPTY_STRING );
        pEdAssign2->SetText( EMPTY_STRING );
    }

    pLbRange->SetBorderStyle( WINDOW_BORDER_MONO );
    pBtnColHead->Enable();
    pBtnRowHead->Enable();
    pEdAssign->Enable();
    pEdAssign->GrabFocus();
    pRbAssign->Enable();

    Range1SelectHdl( 0 );
}


/*************************************************************************
#*  Member:     SetColRowData
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
        pBtnColHead->Check( sal_True );
        pBtnRowHead->Check( false );
        if ( nRow2 == MAXROW  )
        {
            if ( nRow1 == 0 )
                bValid = false;     // Grenzfall gesamte Tabelle
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
        pBtnRowHead->Check( sal_True );
        pBtnColHead->Check( false );
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
        OUString aStr(theCurArea.Format(SCR_ABS_3D, pDoc, eConv));

        if(bRef)
            pEdAssign->SetRefString( aStr );
        else
            pEdAssign->SetText( aStr );

        pEdAssign->SetSelection( Selection( SELECTION_MAX, SELECTION_MAX ) );
        aStr = theCurData.Format(SCR_ABS_3D, pDoc, eConv);

        if(bRef)
            pEdAssign2->SetRefString( aStr );
        else
            pEdAssign2->SetText( aStr );
    }
    else
    {
        theCurData = theCurArea = ScRange();

        if(bRef)
        {
            pEdAssign->SetRefString( EMPTY_STRING );
            pEdAssign2->SetRefString( EMPTY_STRING );
        }
        else
        {
            pEdAssign->SetText( EMPTY_STRING );
            pEdAssign2->SetText( EMPTY_STRING );
        }

        pBtnColHead->Disable();
        pBtnRowHead->Disable();
        pEdAssign2->Disable();
        pRbAssign2->Disable();
    }
}


/*************************************************************************
#*  Member:     AdjustColRowData
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
    if ( pBtnColHead->IsChecked() )
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
    OUString aStr(theCurData.Format(SCR_ABS_3D, pDoc, pDoc->GetAddressConvention()));

    if(bRef)
        pEdAssign2->SetRefString( aStr );
    else
        pEdAssign2->SetText( aStr );

    pEdAssign2->SetSelection( Selection( SELECTION_MAX, SELECTION_MAX ) );
}


/*************************************************************************
#*  Member:     SetReference
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

        if ( pEdActive == pEdAssign )
            SetColRowData( rRef, sal_True );
        else
            AdjustColRowData( rRef, sal_True );
        pBtnColHead->Enable();
        pBtnRowHead->Enable();
        pBtnAdd->Enable();
        pBtnRemove->Disable();
    }
}


/*************************************************************************
#*  Member:     Close
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

sal_Bool ScColRowNameRangesDlg::Close()
{
    return DoClose( ScColRowNameRangesDlgWrapper::GetChildWindowId() );
}


/*************************************************************************
#*  Member:     SetActive
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
        bDlgLostFocus = false;
        if( pEdActive )
            pEdActive->GrabFocus();
    }
    else
        GrabFocus();

    if( pEdActive == pEdAssign )
        Range1DataModifyHdl( 0 );
    else if( pEdActive == pEdAssign2 )
        Range2DataModifyHdl( 0 );

    RefInputDone();
}


/*************************************************************************
#*  Member:     UpdateNames
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
    pLbRange->SetUpdateMode( false );
    //-----------------------------------------------------------
    pLbRange->Clear();
    aRangeMap.clear();
    pEdAssign->SetText( EMPTY_STRING );

    size_t nCount, j;
    sal_uInt16 nPos; //@008 Hilfsvariable q eingefuegt

    SCCOL nCol1;
    SCROW nRow1;    //Erweiterung fuer Bereichsnamen
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    String rString;
    String strShow;
    const ScAddress::Details aDetails(pDoc->GetAddressConvention());

    String aString;
    OUString strDelim(" --- ");
    aString = strDelim;
    aString += ScGlobal::GetRscString( STR_COLUMN );
    aString += strDelim;
    nPos = pLbRange->InsertEntry( aString );
    pLbRange->SetEntryData( nPos, (void*)nEntryDataDelim );
    if ( (nCount = xColNameRanges->size()) > 0 )
    {
        ScRangePair** ppSortArray = xColNameRanges->CreateNameSortedArray(
            nCount, pDoc );
        for ( j=0; j < nCount; j++ )
        {
            const ScRange aRange(ppSortArray[j]->GetRange(0));
            aString = aRange.Format(SCR_ABS_3D, pDoc, aDetails);

            //@008 Hole Bereichsparameter aus Dok
            ppSortArray[j]->GetRange(0).GetVars( nCol1, nRow1, nTab1,
                                            nCol2, nRow2, nTab2 );
            SCCOL q=nCol1+3;
            if(q>nCol2) q=nCol2;
            //@008 Baue String zusammen
            strShow.AssignAscii(RTL_CONSTASCII_STRINGPARAM(" ["));
            if(pDoc!=NULL)
            {
                rString = pDoc->GetString(nCol1, nRow1, nTab1);
                strShow +=rString;
                for(SCCOL i=nCol1+1;i<=q;i++)
                {
                    strShow.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
                    rString = pDoc->GetString(i, nRow1, nTab1);
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
            nPos = pLbRange->InsertEntry( aInsStr );
            aRangeMap.insert( NameRangeMap::value_type(aInsStr, aRange) );
            pLbRange->SetEntryData( nPos, (void*)nEntryDataCol );
        }
        delete [] ppSortArray;
    }
    aString = strDelim;
    aString += ScGlobal::GetRscString( STR_ROW );
    aString += strDelim;
    nPos = pLbRange->InsertEntry( aString );
    pLbRange->SetEntryData( nPos, (void*)nEntryDataDelim );
    if ( (nCount = xRowNameRanges->size()) > 0 )
    {
        ScRangePair** ppSortArray = xRowNameRanges->CreateNameSortedArray(
            nCount, pDoc );
        for ( j=0; j < nCount; j++ )
        {
            const ScRange aRange(ppSortArray[j]->GetRange(0));
            aString = aRange.Format(SCR_ABS_3D, pDoc, aDetails);

            //@008 Ab hier baue String fuer Zeilen
            ppSortArray[j]->GetRange(0).GetVars( nCol1, nRow1, nTab1,
                                            nCol2, nRow2, nTab2 );
            SCROW q=nRow1+3;
            if(q>nRow2) q=nRow2;
            strShow.AssignAscii(RTL_CONSTASCII_STRINGPARAM(" ["));
            if(pDoc!=NULL)
            {
                rString = pDoc->GetString(nCol1, nRow1, nTab1);
                strShow += rString;
                for(SCROW i=nRow1+1;i<=q;i++)
                {
                    strShow.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
                    rString = pDoc->GetString(nCol1, i, nTab1);
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
            nPos = pLbRange->InsertEntry( aInsStr );
            aRangeMap.insert( NameRangeMap::value_type(aInsStr, aRange) );
            pLbRange->SetEntryData( nPos, (void*)nEntryDataRow );
        }
        delete [] ppSortArray;
    }
    //-----------------------------------------------------------
    pLbRange->SetUpdateMode( sal_True );
    pLbRange->Invalidate();
}


/*************************************************************************
#*  Member:     UpdateRangeData
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
    sal_Bool bFound = false;
    if ( bColName && (pPair = xColNameRanges->Find( rRange )) != NULL )
        bFound = sal_True;
    else if ( !bColName && (pPair = xRowNameRanges->Find( rRange )) != NULL )
        bFound = sal_True;

    if ( bFound )
    {
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        theCurArea = rRange;
        OUString aStr(theCurArea.Format(SCR_ABS_3D, pDoc, eConv));
        pEdAssign->SetText( aStr );
        pBtnAdd->Disable();
        pBtnRemove->Enable();
        pBtnColHead->Check( bColName );
        pBtnRowHead->Check( !bColName );
        theCurData = pPair->GetRange(1);
        aStr = theCurData.Format(SCR_ABS_3D, pDoc, eConv);
        pEdAssign2->SetText( aStr );
    }
    else
    {
        pBtnAdd->Enable();
        pBtnRemove->Disable();
    }
    pBtnColHead->Enable();
    pBtnRowHead->Enable();
    pEdAssign2->Enable();
    pRbAssign2->Enable();
}


/*************************************************************************
#*  Member:     IsRefInputMode
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
#*  Handler:    OkBtnHdl
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

IMPL_LINK_NOARG(ScColRowNameRangesDlg, OkBtnHdl)
{
    AddBtnHdl( 0 );

    // die RangeLists den Refs am Doc zuweisen
    pDoc->GetColNameRangesRef() = xColNameRanges;
    pDoc->GetRowNameRangesRef() = xRowNameRanges;
    // geaenderte Datenbereiche muessen sich auswirken
    pDoc->CompileColRowNameFormula();
    ScDocShell* pDocShell = pViewData->GetDocShell();
    pDocShell->PostPaint(ScRange(0, 0, 0, MAXCOL, MAXROW, MAXTAB), PAINT_GRID);
    pDocShell->SetDocumentModified();

    Close();
    return 0;
}


/*************************************************************************
#*  Handler:    CancelBtnHdl
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

IMPL_LINK_NOARG_INLINE_START(ScColRowNameRangesDlg, CancelBtnHdl)
{
    Close();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(ScColRowNameRangesDlg, CancelBtnHdl)


/*************************************************************************
#*  Handler:    AddBtnHdl
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

IMPL_LINK_NOARG(ScColRowNameRangesDlg, AddBtnHdl)
{
    String aNewArea( pEdAssign->GetText() );
    String aNewData( pEdAssign2->GetText() );

    if ( aNewArea.Len() > 0 && aNewData.Len() > 0 )
    {
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
        ScRange aRange1, aRange2;
        sal_Bool bOk1;
        if ( (bOk1 = ((aRange1.ParseAny( aNewArea, pDoc, eConv ) & SCA_VALID) == SCA_VALID)) != false
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
            if ( pBtnColHead->IsChecked() )
                xColNameRanges->Join( ScRangePair( theCurArea, theCurData ) );
            else
                xRowNameRanges->Join( ScRangePair( theCurArea, theCurData ) );

            UpdateNames();

            pEdAssign->GrabFocus();
            pBtnAdd->Disable();
            pBtnRemove->Disable();
            pEdAssign->SetText( EMPTY_STRING );
            pBtnColHead->Check( sal_True );
            pBtnRowHead->Check( false );
            pEdAssign2->SetText( EMPTY_STRING );
            theCurArea = ScRange();
            theCurData = theCurArea;
            Range1SelectHdl( 0 );
        }
        else
        {
            ERRORBOX( ScGlobal::GetRscString(STR_INVALIDTABNAME) );
            if ( !bOk1 )
                pEdAssign->GrabFocus();
            else
                pEdAssign2->GrabFocus();
        }
    }
    return 0;
}


/*************************************************************************
#*  Handler:    RemoveBtnHdl
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

IMPL_LINK_NOARG(ScColRowNameRangesDlg, RemoveBtnHdl)
{
    String aRangeStr = pLbRange->GetSelectEntry();
    sal_uInt16 nSelectPos = pLbRange->GetSelectEntryPos();
    sal_Bool bColName =
        ((sal_uLong)pLbRange->GetEntryData( nSelectPos ) == nEntryDataCol);
    NameRangeMap::const_iterator itr = aRangeMap.find(aRangeStr);
    if (itr == aRangeMap.end())
        return 0;
    const ScRange& rRange = itr->second;

    ScRangePair* pPair = NULL;
    sal_Bool bFound = false;
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
            sal_uInt16 nCnt = pLbRange->GetEntryCount();
            if ( nSelectPos >= nCnt )
            {
                if ( nCnt )
                    nSelectPos = nCnt - 1;
                else
                    nSelectPos = 0;
            }
            pLbRange->SelectEntryPos( nSelectPos );
            if ( nSelectPos &&
                    (sal_uLong)pLbRange->GetEntryData( nSelectPos ) == nEntryDataDelim )
                pLbRange->SelectEntryPos( --nSelectPos );    // ---Zeile---

            pLbRange->GrabFocus();
            pBtnAdd->Disable();
            pBtnRemove->Disable();
            pEdAssign->SetText( EMPTY_STRING );
            theCurArea = theCurData = ScRange();
            pBtnColHead->Check( sal_True );
            pBtnRowHead->Check( false );
            pEdAssign2->SetText( EMPTY_STRING );
            Range1SelectHdl( 0 );
        }
    }
    return 0;
}


/*************************************************************************
#*  Handler:    Range1SelectHdl
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

IMPL_LINK_NOARG(ScColRowNameRangesDlg, Range1SelectHdl)
{
    sal_uInt16 nSelectPos = pLbRange->GetSelectEntryPos();
    sal_uInt16 nCnt = pLbRange->GetEntryCount();
    sal_uInt16 nMoves = 0;
    while ( nSelectPos < nCnt
            && (sal_uLong)pLbRange->GetEntryData( nSelectPos ) == nEntryDataDelim )
    {   // skip Delimiter
        ++nMoves;
        pLbRange->SelectEntryPos( ++nSelectPos );
    }
    String aRangeStr = pLbRange->GetSelectEntry();
    if ( nMoves )
    {
        if ( nSelectPos > 1 && nSelectPos >= nCnt )
        {   // am Ende nicht auf dem " --- Zeile --- " Delimiter stehenbleiben
            // wenn davor Eintraege existieren
            nSelectPos = nCnt - 2;
            pLbRange->SelectEntryPos( nSelectPos );
            aRangeStr = pLbRange->GetSelectEntry();
        }
        else if ( nSelectPos > 2 && nSelectPos < nCnt && aRangeStr.Len()
                  && OUString(aRangeStr) == pEdAssign->GetText() )
        {   // nach oben wandern statt nach unten auf die vorherige Position
            nSelectPos -= 2;
            pLbRange->SelectEntryPos( nSelectPos );
            aRangeStr = pLbRange->GetSelectEntry();
        }
    }
    NameRangeMap::const_iterator itr = aRangeMap.find(aRangeStr);
    if ( itr != aRangeMap.end() )
    {
        sal_Bool bColName =
            ((sal_uLong)pLbRange->GetEntryData( nSelectPos ) == nEntryDataCol);
        UpdateRangeData( itr->second, bColName );
        pBtnAdd->Disable();
        pBtnRemove->Enable();
    }
    else
    {
        if ( !pEdAssign->GetText().isEmpty() )
        {
            if ( !pEdAssign2->GetText().isEmpty() )
                pBtnAdd->Enable();
            else
                pBtnAdd->Disable();
            pBtnColHead->Enable();
            pBtnRowHead->Enable();
            pEdAssign2->Enable();
            pRbAssign2->Enable();
        }
        else
        {
            pBtnAdd->Disable();
            pBtnColHead->Disable();
            pBtnRowHead->Disable();
            pEdAssign2->Disable();
            pRbAssign2->Disable();
        }
        pBtnRemove->Disable();
        pEdAssign->GrabFocus();
    }

    pEdAssign->Enable();
    pRbAssign->Enable();

    return 0;
}


/*************************************************************************
#*  Handler:    Range1DataModifyHdl
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

IMPL_LINK_NOARG(ScColRowNameRangesDlg, Range1DataModifyHdl)
{
    String aNewArea( pEdAssign->GetText() );
    sal_Bool bValid = false;
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
        pBtnAdd->Enable();
        pBtnColHead->Enable();
        pBtnRowHead->Enable();
        pEdAssign2->Enable();
        pRbAssign2->Enable();
    }
    else
    {
        pBtnAdd->Disable();
        pBtnColHead->Disable();
        pBtnRowHead->Disable();
        pEdAssign2->Disable();
        pRbAssign2->Disable();
    }
    pBtnRemove->Disable();
    return 0;
}


/*************************************************************************
#*  Handler:    Range2DataModifyHdl
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

IMPL_LINK_NOARG(ScColRowNameRangesDlg, Range2DataModifyHdl)
{
    String aNewData( pEdAssign2->GetText() );
    if ( aNewData.Len() > 0 )
    {
        ScRange aRange;
        if ( (aRange.ParseAny( aNewData, pDoc, pDoc->GetAddressConvention() ) & SCA_VALID) == SCA_VALID )
        {
            AdjustColRowData( aRange );
            pBtnAdd->Enable();
        }
        else
            pBtnAdd->Disable();
    }
    else
    {
        pBtnAdd->Disable();
    }
    return 0;
}


/*************************************************************************
#*  Handler:    ColClickHdl
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

IMPL_LINK_NOARG(ScColRowNameRangesDlg, ColClickHdl)
{
    if ( !pBtnColHead->GetSavedValue() )
    {
        pBtnColHead->Check( sal_True );
        pBtnRowHead->Check( false );
        if ( theCurArea.aStart.Row() == 0 && theCurArea.aEnd.Row() == MAXROW )
        {
            theCurArea.aEnd.SetRow( MAXROW - 1 );
            OUString aStr(theCurArea.Format(SCR_ABS_3D, pDoc, pDoc->GetAddressConvention()));
            pEdAssign->SetText( aStr );
        }
        ScRange aRange( theCurData );
        aRange.aStart.SetRow( std::min( (long)(theCurArea.aEnd.Row() + 1), (long)MAXROW ) );
        aRange.aEnd.SetRow( MAXROW );
        AdjustColRowData( aRange );
    }
    return 0;
}


/*************************************************************************
#*  Handler:    RowClickHdl
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

IMPL_LINK_NOARG(ScColRowNameRangesDlg, RowClickHdl)
{
    if ( !pBtnRowHead->GetSavedValue() )
    {
        pBtnRowHead->Check( sal_True );
        pBtnColHead->Check( false );
        if ( theCurArea.aStart.Col() == 0 && theCurArea.aEnd.Col() == MAXCOL )
        {
            theCurArea.aEnd.SetCol( MAXCOL - 1 );
            OUString aStr(theCurArea.Format(SCR_ABS_3D, pDoc, pDoc->GetAddressConvention()));
            pEdAssign->SetText( aStr );
        }
        ScRange aRange( theCurData );
        aRange.aStart.SetCol( static_cast<SCCOL>(std::min( (long)(theCurArea.aEnd.Col() + 1), (long)MAXCOL )) );
        aRange.aEnd.SetCol( MAXCOL );
        AdjustColRowData( aRange );
    }
    return 0;
}


IMPL_LINK( ScColRowNameRangesDlg, GetFocusHdl, Control*, pCtrl )
{
    if( (pCtrl == (Control*)pEdAssign) || (pCtrl == (Control*)pRbAssign) )
        pEdActive = pEdAssign;
    else if( (pCtrl == (Control*)pEdAssign2) || (pCtrl == (Control*)pRbAssign2) )
        pEdActive = pEdAssign2;
    else
        pEdActive = NULL;

    if( pEdActive )
        pEdActive->SetSelection( Selection( 0, SELECTION_MAX ) );

    return 0;
}


IMPL_LINK_NOARG(ScColRowNameRangesDlg, LoseFocusHdl)
{
    bDlgLostFocus = !IsActive();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
