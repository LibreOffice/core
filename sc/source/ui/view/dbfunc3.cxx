/*************************************************************************
 *
 *  $RCSfile: dbfunc3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:08 $
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

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/bindings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>
#include <vcl/waitobj.hxx>

#include "dbfunc.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "sc.hrc"
#include "undotab.hxx"
#include "undodat.hxx"
#include "dbcolect.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "docsh.hxx"
#include "olinetab.hxx"
#include "consoli.hxx"
#include "olinefun.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dbdocfun.hxx"

// STATIC DATA -----------------------------------------------------------


//==================================================================

//
//          Outliner
//

void lcl_InvalidateOutliner()
{
    SfxBindings& rBindings = SFX_BINDINGS();
    rBindings.Invalidate( SID_OUTLINE_SHOW );
    rBindings.Invalidate( SID_OUTLINE_HIDE );
    rBindings.Invalidate( SID_OUTLINE_REMOVE );

    rBindings.Invalidate( SID_STATUS_SUM );         // wegen ein-/ausblenden
    rBindings.Invalidate( SID_ATTR_SIZE );
}

//  Outline-Gruppierung erzeugen

void ScDBFunc::MakeOutline( BOOL bColumns, BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.MakeOutline( aRange, bColumns, bRecord, FALSE );
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

//  Outline-Gruppierung loeschen

void ScDBFunc::RemoveOutline( BOOL bColumns, BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScOutlineDocFunc aFunc(*pDocSh);
        aFunc.RemoveOutline( aRange, bColumns, bRecord, FALSE );
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

//  Menue-Status: Outlines loeschen

void ScDBFunc::TestRemoveOutline( BOOL& rCol, BOOL& rRow )
{
    BOOL bColFound = FALSE;
    BOOL bRowFound = FALSE;

    USHORT nStartCol, nStartRow, nStartTab;
    USHORT nEndCol, nEndRow, nEndTab;
    if (GetViewData()->GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab))
    {
        USHORT nTab = nStartTab;
        ScDocument* pDoc = GetViewData()->GetDocument();
        ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
        if (pTable)
        {
            ScOutlineArray* pArray;
            ScOutlineEntry* pEntry;
            USHORT nStart;
            USHORT nEnd;
            BOOL bColMarked = ( nStartRow == 0 && nEndRow == MAXROW );
            BOOL bRowMarked = ( nStartCol == 0 && nEndCol == MAXCOL );

            //  Spalten

            if ( !bRowMarked || bColMarked )        // nicht wenn ganze Zeilen markiert
            {
                pArray = pTable->GetColArray();
                ScSubOutlineIterator aColIter( pArray );
                while ((pEntry=aColIter.GetNext()) != NULL && !bColFound)
                {
                    nStart = pEntry->GetStart();
                    nEnd   = pEntry->GetEnd();
                    if ( nStartCol<=nEnd && nEndCol>=nStart )
                        bColFound = TRUE;
                }
            }

            //  Zeilen

            if ( !bColMarked || bRowMarked )        // nicht wenn ganze Spalten markiert
            {
                pArray = pTable->GetRowArray();
                ScSubOutlineIterator aRowIter( pArray );
                while ((pEntry=aRowIter.GetNext()) != NULL && !bRowFound)
                {
                    nStart = pEntry->GetStart();
                    nEnd   = pEntry->GetEnd();
                    if ( nStartRow<=nEnd && nEndRow>=nStart )
                        bRowFound = TRUE;
                }
            }
        }
    }

    rCol = bColFound;
    rRow = bRowFound;
}

void ScDBFunc::RemoveAllOutlines( BOOL bRecord )
{
    USHORT nTab = GetViewData()->GetTabNo();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScOutlineDocFunc aFunc(*pDocSh);

    HideCursor();
    BOOL bOk = aFunc.RemoveAllOutlines( nTab, bRecord, FALSE );
    ShowCursor();

    if (bOk)
        UpdateScrollBars();
}

//  Auto-Outlines

void ScDBFunc::AutoOutline( BOOL bRecord )
{
    USHORT nTab = GetViewData()->GetTabNo();
    ScRange aRange( 0,0,nTab, MAXCOL,MAXROW,nTab );     // ganze Tabelle, wenn nichts markiert
    ScMarkData& rMark = GetViewData()->GetMarkData();
    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        rMark.MarkToMulti();
        rMark.GetMultiMarkArea( aRange );
    }

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScOutlineDocFunc aFunc(*pDocSh);
    aFunc.AutoOutline( aRange, bRecord, FALSE );
}

//  Outline-Ebene auswaehlen

void ScDBFunc::SelectLevel( BOOL bColumns, USHORT nLevel, BOOL bRecord, BOOL bPaint )
{
    USHORT nTab = GetViewData()->GetTabNo();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScOutlineDocFunc aFunc(*pDocSh);

    HideCursor();
    BOOL bOk = aFunc.SelectLevel( nTab, bColumns, nLevel, bRecord, bPaint, FALSE );
    ShowCursor();

    if (bOk)
        UpdateScrollBars();
}

//  einzelne Outline-Gruppe einblenden

void ScDBFunc::ShowOutline( BOOL bColumns, USHORT nLevel, USHORT nEntry, BOOL bRecord, BOOL bPaint )
{
    USHORT nTab = GetViewData()->GetTabNo();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScOutlineDocFunc aFunc(*pDocSh);

    HideCursor();
    BOOL bOk = aFunc.ShowOutline( nTab, bColumns, nLevel, nEntry, bRecord, bPaint, FALSE );
    ShowCursor();

    if ( bOk && bPaint )
        UpdateScrollBars();
}

//  einzelne Outline-Gruppe ausblenden

void ScDBFunc::HideOutline( BOOL bColumns, USHORT nLevel, USHORT nEntry, BOOL bRecord, BOOL bPaint )
{
    USHORT nTab = GetViewData()->GetTabNo();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScOutlineDocFunc aFunc(*pDocSh);

    HideCursor();
    BOOL bOk = aFunc.HideOutline( nTab, bColumns, nLevel, nEntry, bRecord, bPaint, FALSE );
    ShowCursor();

    if ( bOk && bPaint )
        UpdateScrollBars();
}

//  Menue-Status: markierten Bereich ein-/ausblenden

BOOL ScDBFunc::OutlinePossible(BOOL bHide)
{
    BOOL bEnable = FALSE;

    USHORT nStartCol;
    USHORT nStartRow;
    USHORT nStartTab;
    USHORT nEndCol;
    USHORT nEndRow;
    USHORT nEndTab;

    if (GetViewData()->GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab))
    {
        ScDocument* pDoc = GetViewData()->GetDocument();
        USHORT nTab = GetViewData()->GetTabNo();
        ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
        if (pTable)
        {
            ScOutlineArray* pArray;
            ScOutlineEntry* pEntry;
            USHORT nStart;
            USHORT nEnd;

            //  Spalten

            pArray = pTable->GetColArray();
            ScSubOutlineIterator aColIter( pArray );
            while ((pEntry=aColIter.GetNext()) != NULL && !bEnable)
            {
                nStart = pEntry->GetStart();
                nEnd   = pEntry->GetEnd();
                if ( bHide )
                {
                    if ( nStartCol<=nEnd && nEndCol>=nStart )
                        if (!pEntry->IsHidden())
                            bEnable = TRUE;
                }
                else
                {
                    if ( nStart>=nStartCol && nEnd<=nEndCol )
                        if (pEntry->IsHidden())
                            bEnable = TRUE;
                }
            }

            //  Zeilen

            pArray = pTable->GetRowArray();
            ScSubOutlineIterator aRowIter( pArray );
            while ((pEntry=aRowIter.GetNext()) != NULL)
            {
                nStart = pEntry->GetStart();
                nEnd   = pEntry->GetEnd();
                if ( bHide )
                {
                    if ( nStartRow<=nEnd && nEndRow>=nStart )
                        if (!pEntry->IsHidden())
                            bEnable = TRUE;
                }
                else
                {
                    if ( nStart>=nStartRow && nEnd<=nEndRow )
                        if (pEntry->IsHidden())
                            bEnable = TRUE;
                }
            }
        }
    }

    return bEnable;
}

//  markierten Bereich einblenden

void ScDBFunc::ShowMarkedOutlines( BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScOutlineDocFunc aFunc(*pDocSh);
        HideCursor();
        BOOL bDone = aFunc.ShowMarkedOutlines( aRange, bRecord, FALSE );
        ShowCursor();
        if (bDone)
            UpdateScrollBars();
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

//  markierten Bereich ausblenden

void ScDBFunc::HideMarkedOutlines( BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange))
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScOutlineDocFunc aFunc(*pDocSh);
        HideCursor();
        BOOL bDone = aFunc.HideMarkedOutlines( aRange, bRecord, FALSE );
        ShowCursor();
        if (bDone)
            UpdateScrollBars();
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

//  --------------------------------------------------------------------------

//
//          Teilergebnisse
//

void ScDBFunc::DoSubTotals( const ScSubTotalParam& rParam, BOOL bRecord,
                            const ScSortParam* pForceNewSort )
{
    BOOL bDo = !rParam.bRemoveOnly;                         // FALSE = nur loeschen

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    USHORT nTab = GetViewData()->GetTabNo();

    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rParam.nCol1, rParam.nRow1,
                                                rParam.nCol2, rParam.nRow2 );
    if (!pDBData)
    {
        DBG_ERROR( "SubTotals: keine DBData" );
        return;
    }

    if (!pDoc->IsBlockEditable( nTab, 0,rParam.nRow1+1, MAXCOL,MAXROW ))
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    if (pDoc->HasAttrib( rParam.nCol1, rParam.nRow1+1, nTab,
                         rParam.nCol2, rParam.nRow2, nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ))
    {
        ErrorMessage(STR_MSSG_INSERTCELLS_0);   // nicht in zusammengefasste einfuegen
        return;
    }

    WaitObject aWait( GetViewData()->GetDialogParent() );
    BOOL bOk = TRUE;
    BOOL bDelete = FALSE;
    if (rParam.bReplace)
        if (pDoc->TestRemoveSubTotals( nTab, rParam ))
        {
            bDelete = TRUE;
            bOk = ( MessBox( GetViewData()->GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                // "StarCalc" "Daten loeschen?"
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_1 ) ).Execute()
                == RET_YES );
        }

    if (bOk)
    {
        ScDocShellModificator aModificator( *pDocSh );

        ScSubTotalParam aNewParam( rParam );        // Bereichsende wird veraendert
        ScDocument*     pUndoDoc = NULL;
        ScOutlineTable* pUndoTab = NULL;
        ScRangeName*    pUndoRange = NULL;
        ScDBCollection* pUndoDB = NULL;
        USHORT          nTabCount = 0;              // fuer Referenz-Undo

        if (bRecord)                                        // alte Daten sichern
        {
            BOOL bOldFilter = bDo && rParam.bDoSort;

            nTabCount = pDoc->GetTableCount();
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
            if (pTable)
            {
                pUndoTab = new ScOutlineTable( *pTable );

                USHORT nOutStartCol;                            // Zeilen/Spaltenstatus
                USHORT nOutStartRow;
                USHORT nOutEndCol;
                USHORT nOutEndRow;
                pTable->GetColArray()->GetRange( nOutStartCol, nOutEndCol );
                pTable->GetRowArray()->GetRange( nOutStartRow, nOutEndRow );

                pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
                pDoc->CopyToDocument( nOutStartCol, 0, nTab, nOutEndCol, MAXROW, nTab, IDF_NONE, FALSE, pUndoDoc );
                pDoc->CopyToDocument( 0, nOutStartRow, nTab, MAXCOL, nOutEndRow, nTab, IDF_NONE, FALSE, pUndoDoc );
            }
            else
                pUndoDoc->InitUndo( pDoc, nTab, nTab, FALSE, bOldFilter );

            //  Datenbereich sichern - incl. Filter-Ergebnis
            pDoc->CopyToDocument( 0,rParam.nRow1+1,nTab, MAXCOL,rParam.nRow2,nTab,
                                    IDF_ALL, FALSE, pUndoDoc );

            //  alle Formeln wegen Referenzen
            pDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1,
                                        IDF_FORMULA, FALSE, pUndoDoc );

            //  DB- und andere Bereiche
            ScRangeName* pDocRange = pDoc->GetRangeName();
            if (pDocRange->GetCount())
                pUndoRange = new ScRangeName( *pDocRange );
            ScDBCollection* pDocDB = pDoc->GetDBCollection();
            if (pDocDB->GetCount())
                pUndoDB = new ScDBCollection( *pDocDB );
        }

//      pDoc->SetOutlineTable( nTab, NULL );
        ScOutlineTable* pOut = pDoc->GetOutlineTable( nTab );
        if (pOut)
            pOut->GetRowArray()->RemoveAll();       // nur Zeilen-Outlines loeschen

        if (rParam.bReplace)
            pDoc->RemoveSubTotals( nTab, aNewParam );
        BOOL bSuccess = TRUE;
        if (bDo)
        {
            // Sortieren
            if ( rParam.bDoSort || pForceNewSort )
            {
                pDBData->SetArea( nTab, aNewParam.nCol1,aNewParam.nRow1, aNewParam.nCol2,aNewParam.nRow2 );

                //  Teilergebnis-Felder vor die Sortierung setzen
                //  (doppelte werden weggelassen, kann darum auch wieder aufgerufen werden)

                ScSortParam aOldSort;
                pDBData->GetSortParam( aOldSort );
                ScSortParam aSortParam( aNewParam, pForceNewSort ? *pForceNewSort : aOldSort );
                Sort( aSortParam, FALSE, FALSE );
            }

            bSuccess = pDoc->DoSubTotals( nTab, aNewParam );
        }
        ScRange aDirtyRange( aNewParam.nCol1, aNewParam.nRow1, nTab,
            aNewParam.nCol2, aNewParam.nRow2, nTab );
        pDoc->SetDirty( aDirtyRange );

        if (bRecord)
        {
//          ScDBData* pUndoDBData = pDBData ? new ScDBData( *pDBData ) : NULL;
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoSubTotals( pDocSh, nTab,
                                        rParam, aNewParam.nRow2,
                                        pUndoDoc, pUndoTab, // pUndoDBData,
                                        pUndoRange, pUndoDB ) );
        }

        if (!bSuccess)
        {
            // "Kann keine Zeilen einfuegen"
            ErrorMessage(STR_MSSG_DOSUBTOTALS_2);
        }

                                                    // merken
        pDBData->SetSubTotalParam( aNewParam );
        pDBData->SetArea( nTab, aNewParam.nCol1,aNewParam.nRow1, aNewParam.nCol2,aNewParam.nRow2 );
        pDoc->CompileDBFormula();

        DoneBlockMode();
        InitOwnBlockMode();
        rMark.SetMarkArea( ScRange( aNewParam.nCol1,aNewParam.nRow1,nTab,
                                    aNewParam.nCol2,aNewParam.nRow2,nTab ) );

        pDocSh->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab,
                                                PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE );

        aModificator.SetDocumentModified();

        SelectionChanged();
    }
}

//
//          Consolidate
//

void ScDBFunc::Consolidate( const ScConsolidateParam& rParam, BOOL bRecord )
{
    ScDocShell* pDocShell = GetViewData()->GetDocShell();
    pDocShell->DoConsolidate( rParam, bRecord );
    SetTabNo( rParam.nTab, TRUE );
}

//
//          Pivot
//

String lcl_MakePivotTabName( const String& rPrefix, USHORT nNumber )
{
    String aName = rPrefix;
    aName += String::CreateFromInt32( nNumber );
    return aName;
}

void ScDBFunc::MakePivotTable( const ScDPSaveData& rData, const ScRange& rDest, BOOL bNewTable,
                                const ScDPObject& rSource, BOOL bApi )
{
    //  #70096# error message if no fields are set
    //  this must be removed when drag&drop of fields from a toolbox is available

    if ( rData.IsEmpty() && !bApi )
    {
        ErrorMessage(STR_PIVOT_NODATA);
        return;
    }

    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScDocument* pDoc    = GetViewData()->GetDocument();

    ScRange aDestRange = rDest;
    if ( bNewTable )
    {
        USHORT nSrcTab = GetViewData()->GetTabNo();

        String aName( ScGlobal::GetRscString(STR_PIVOT_TABLE) );
        String aStr;

        pDoc->GetName( nSrcTab, aStr );
        aName += '_';
        aName += aStr;
        aName += '_';

        USHORT nNewTab = nSrcTab+1;

        USHORT i=1;
        while ( !pDoc->InsertTab( nNewTab, lcl_MakePivotTabName( aName, i ) ) && i <= MAXTAB )
            i++;

        BOOL bAppend = ( nNewTab+1 == pDoc->GetTableCount() );
        pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoInsertTab( pDocSh, nNewTab, bAppend, lcl_MakePivotTabName( aName, i ) ));

        GetViewData()->InsertTab( nNewTab );
        SetTabNo( nNewTab, TRUE );

        aDestRange = ScRange( 0, 0, nNewTab );
    }

    ScDPObject* pDPObj = pDoc->GetDPAtCursor(
                            aDestRange.aStart.Col(), aDestRange.aStart.Row(), aDestRange.aStart.Tab() );

    ScDPObject aObj( rSource );
    aObj.SetOutRange( aDestRange );
    aObj.SetSaveData( rData );

    ScDBDocFunc aFunc( *pDocSh );
    aFunc.DataPilotUpdate( pDPObj, &aObj, TRUE, FALSE );

    CursorPosChanged();     // shells may be switched

    if ( bNewTable )
    {
        pDocSh->PostPaintExtras();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    }
}

void ScDBFunc::DeletePivotTable()
{
    ScDocShell* pDocSh    = GetViewData()->GetDocShell();
    ScDocument* pDoc      = pDocSh->GetDocument();
    ScDPObject* pDPObj    = pDoc->GetDPAtCursor( GetViewData()->GetCurX(),
                                                  GetViewData()->GetCurY(),
                                                  GetViewData()->GetTabNo() );
    if ( pDPObj )
    {
        ScDBDocFunc aFunc( *pDocSh );
        aFunc.DataPilotUpdate( pDPObj, NULL, TRUE, FALSE );
        CursorPosChanged();     // shells may be switched
    }
    else
        ErrorMessage(STR_PIVOT_NOTFOUND);
}

void ScDBFunc::RecalcPivotTable()
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScDocument* pDoc    = GetViewData()->GetDocument();

    //  old pivot not used any more

    ScDPObject* pDPObj  = pDoc->GetDPAtCursor( GetViewData()->GetCurX(),
                                                  GetViewData()->GetCurY(),
                                                  GetViewData()->GetTabNo() );
    if ( pDPObj )
    {
        ScDBDocFunc aFunc( *pDocSh );
        aFunc.DataPilotUpdate( pDPObj, pDPObj, TRUE, FALSE );
        CursorPosChanged();     // shells may be switched
    }
    else
        ErrorMessage(STR_PIVOT_NOTFOUND);
}


//
//          DB-Operationen (Sortieren, Filtern, Teilergebnisse) wiederholen
//

void ScDBFunc::RepeatDB( BOOL bRecord )
{
    USHORT nCurX = GetViewData()->GetCurX();
    USHORT nCurY = GetViewData()->GetCurY();
    USHORT nTab = GetViewData()->GetTabNo();
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScDBData* pDBData = GetDBData();

    ScQueryParam aQueryParam;
    pDBData->GetQueryParam( aQueryParam );
    BOOL bQuery = aQueryParam.GetEntry(0).bDoQuery;

    ScSortParam aSortParam;
    pDBData->GetSortParam( aSortParam );
    BOOL bSort = aSortParam.bDoSort[0];

    ScSubTotalParam aSubTotalParam;
    pDBData->GetSubTotalParam( aSubTotalParam );
    BOOL bSubTotal = aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly;

    if ( bQuery || bSort || bSubTotal )
    {
        BOOL bQuerySize = FALSE;
        ScRange aOldQuery;
        ScRange aNewQuery;
        if (bQuery && !aQueryParam.bInplace)
        {
            ScDBData* pDest = pDoc->GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                    aQueryParam.nDestTab, TRUE );
            if (pDest && pDest->IsDoSize())
            {
                pDest->GetArea( aOldQuery );
                bQuerySize = TRUE;
            }
        }

        USHORT nDummy;
        USHORT nStartCol;
        USHORT nStartRow;
        USHORT nEndCol;
        USHORT nEndRow;
        pDBData->GetArea( nDummy, nStartCol, nStartRow, nEndCol, nEndRow );

        //!     Undo nur benoetigte Daten ?

        ScDocument* pUndoDoc = NULL;
        ScOutlineTable* pUndoTab = NULL;
        ScRangeName* pUndoRange = NULL;
        ScDBCollection* pUndoDB = NULL;

        if (bRecord)
        {
            USHORT nTabCount = pDoc->GetTableCount();
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
            if (pTable)
            {
                pUndoTab = new ScOutlineTable( *pTable );

                USHORT nOutStartCol;                            // Zeilen/Spaltenstatus
                USHORT nOutStartRow;
                USHORT nOutEndCol;
                USHORT nOutEndRow;
                pTable->GetColArray()->GetRange( nOutStartCol, nOutEndCol );
                pTable->GetRowArray()->GetRange( nOutStartRow, nOutEndRow );

                pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
                pDoc->CopyToDocument( nOutStartCol, 0, nTab, nOutEndCol, MAXROW, nTab, IDF_NONE, FALSE, pUndoDoc );
                pDoc->CopyToDocument( 0, nOutStartRow, nTab, MAXCOL, nOutEndRow, nTab, IDF_NONE, FALSE, pUndoDoc );
            }
            else
                pUndoDoc->InitUndo( pDoc, nTab, nTab, FALSE, TRUE );

            //  Datenbereich sichern - incl. Filter-Ergebnis
            pDoc->CopyToDocument( 0,nStartRow,nTab, MAXCOL,nEndRow,nTab, IDF_ALL, FALSE, pUndoDoc );

            //  alle Formeln wegen Referenzen
            pDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1, IDF_FORMULA, FALSE, pUndoDoc );

            //  DB- und andere Bereiche
            ScRangeName* pDocRange = pDoc->GetRangeName();
            if (pDocRange->GetCount())
                pUndoRange = new ScRangeName( *pDocRange );
            ScDBCollection* pDocDB = pDoc->GetDBCollection();
            if (pDocDB->GetCount())
                pUndoDB = new ScDBCollection( *pDocDB );
        }

        if (bSort && bSubTotal)
        {
            //  Sortieren ohne SubTotals

            aSubTotalParam.bRemoveOnly = TRUE;      // wird unten wieder zurueckgesetzt
            DoSubTotals( aSubTotalParam, FALSE );
        }

        if (bSort)
        {
            pDBData->GetSortParam( aSortParam );            // Bereich kann sich geaendert haben
            Sort( aSortParam, FALSE, FALSE);
        }
        if (bQuery)
        {
            pDBData->GetQueryParam( aQueryParam );          // Bereich kann sich geaendert haben
            ScRange aAdvSource;
            if (pDBData->GetAdvancedQuerySource(aAdvSource))
                Query( aQueryParam, &aAdvSource, FALSE );
            else
                Query( aQueryParam, NULL, FALSE );

            //  bei nicht-inplace kann die Tabelle umgestellt worden sein
            if ( !aQueryParam.bInplace && aQueryParam.nDestTab != nTab )
                SetTabNo( nTab );
        }
        if (bSubTotal)
        {
            pDBData->GetSubTotalParam( aSubTotalParam );    // Bereich kann sich geaendert haben
            aSubTotalParam.bRemoveOnly = FALSE;
            DoSubTotals( aSubTotalParam, FALSE );
        }

        if (bRecord)
        {
            USHORT nDummy;
            USHORT nNewEndRow;
            pDBData->GetArea( nDummy, nDummy,nDummy, nDummy,nNewEndRow );

            const ScRange* pOld = NULL;
            const ScRange* pNew = NULL;
            if (bQuerySize)
            {
                ScDBData* pDest = pDoc->GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                        aQueryParam.nDestTab, TRUE );
                if (pDest)
                {
                    pDest->GetArea( aNewQuery );
                    pOld = &aOldQuery;
                    pNew = &aNewQuery;
                }
            }

            GetViewData()->GetDocShell()->GetUndoManager()->AddUndoAction(
                new ScUndoRepeatDB( GetViewData()->GetDocShell(), nTab,
                                        nStartCol, nStartRow, nEndCol, nEndRow,
                                        nNewEndRow,
                                        nCurX, nCurY,
                                        pUndoDoc, pUndoTab,
                                        pUndoRange, pUndoDB,
                                        pOld, pNew ) );
        }

        GetViewData()->GetDocShell()->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab,
                                                    PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE );
    }
    else        // "Keine Operationen auszufuehren"
        ErrorMessage(STR_MSSG_REPEATDB_0);
}




