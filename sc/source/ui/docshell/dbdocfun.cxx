/*************************************************************************
 *
 *  $RCSfile: dbdocfun.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:55 $
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

#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>

#include "dbdocfun.hxx"
#include "sc.hrc"
#include "dbcolect.hxx"
#include "undodat.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "olinetab.hxx"
#include "dpobject.hxx"

// -----------------------------------------------------------------

BOOL ScDBDocFunc::AddDBRange( const String& rName, const ScRange& rRange, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();

    ScDBCollection* pUndoColl = new ScDBCollection( *pDocColl );

    ScDBData* pNew = new ScDBData( rName, rRange.aStart.Tab(),
                                    rRange.aStart.Col(), rRange.aStart.Row(),
                                    rRange.aEnd.Col(), rRange.aEnd.Row() );

    pDoc->CompileDBFormula( TRUE );     // CreateFormulaString
    BOOL bOk = pDocColl->Insert( pNew );
    pDoc->CompileDBFormula( FALSE );    // CompileFormulaString
    if (!bOk)
    {
        delete pNew;
        delete pUndoColl;
        return FALSE;
    }

    ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
    rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );

    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
    return TRUE;
}

BOOL ScDBDocFunc::DeleteDBRange( const String& rName, BOOL bApi )
{
    BOOL bDone = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();

    USHORT nPos = 0;
    if (pDocColl->SearchName( rName, nPos ))
    {
        ScDocShellModificator aModificator( rDocShell );

        ScDBCollection* pUndoColl = new ScDBCollection( *pDocColl );

        pDoc->CompileDBFormula( TRUE );     // CreateFormulaString
        pDocColl->AtFree( nPos );
        pDoc->CompileDBFormula( FALSE );    // CompileFormulaString

        ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
        rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );

        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
        bDone = TRUE;
    }

    return bDone;
}

BOOL ScDBDocFunc::RenameDBRange( const String& rOld, const String& rNew, BOOL bApi )
{
    BOOL bDone = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();

    USHORT nPos = 0;
    USHORT nDummy = 0;
    if ( pDocColl->SearchName( rOld, nPos ) &&
         !pDocColl->SearchName( rNew, nDummy ) )
    {
        ScDocShellModificator aModificator( rDocShell );

        ScDBData* pData = (*pDocColl)[nPos];
        ScDBData* pNewData = new ScDBData(*pData);
        pNewData->SetName(rNew);

        ScDBCollection* pUndoColl = new ScDBCollection( *pDocColl );

        pDoc->CompileDBFormula( TRUE );             // CreateFormulaString
        pDocColl->AtFree( nPos );
        BOOL bInserted = pDocColl->Insert( pNewData );
        if (!bInserted)                             // Fehler -> alten Zustand wiederherstellen
        {
            delete pNewData;
            pDoc->SetDBCollection( pUndoColl );     // gehoert dann dem Dokument
        }
        pDoc->CompileDBFormula( FALSE );            // CompileFormulaString

        if (bInserted)                              // Einfuegen hat geklappt
        {
            ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
            rDocShell.GetUndoManager()->AddUndoAction(
                            new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );

            aModificator.SetDocumentModified();
            SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
            bDone = TRUE;
        }
    }

    return bDone;
}

BOOL ScDBDocFunc::ModifyDBData( const ScDBData& rNewData, BOOL bApi )
{
    BOOL bDone = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();

    USHORT nPos = 0;
    if (pDocColl->SearchName( rNewData.GetName(), nPos ))
    {
        ScDocShellModificator aModificator( rDocShell );

        ScDBData* pData = (*pDocColl)[nPos];

        ScRange aOldRange, aNewRange;
        pData->GetArea(aOldRange);
        rNewData.GetArea(aNewRange);
        BOOL bAreaChanged = ( aOldRange != aNewRange );     // dann muss neu compiliert werden

        ScDBCollection* pUndoColl = new ScDBCollection( *pDocColl );

        *pData = rNewData;
        if (bAreaChanged)
            pDoc->CompileDBFormula();

        ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
        rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );

        aModificator.SetDocumentModified();
        bDone = TRUE;
    }

    return bDone;
}

// -----------------------------------------------------------------

BOOL ScDBDocFunc::RepeatDB( const String& rDBName, BOOL bRecord, BOOL bApi )
{
    //! auch fuer ScDBFunc::RepeatDB benutzen!

    BOOL bDone = FALSE;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pColl = pDoc->GetDBCollection();
    USHORT nIndex;
    if ( pColl && pColl->SearchName( rDBName, nIndex ) )
    {
        ScDBData* pDBData = (*pColl)[nIndex];

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

            USHORT nTab;
            USHORT nStartCol;
            USHORT nStartRow;
            USHORT nEndCol;
            USHORT nEndRow;
            pDBData->GetArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );

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
                DoSubTotals( nTab, aSubTotalParam, NULL, FALSE, bApi );
            }

            if (bSort)
            {
                pDBData->GetSortParam( aSortParam );            // Bereich kann sich geaendert haben
                Sort( nTab, aSortParam, FALSE, FALSE, bApi );
            }
            if (bQuery)
            {
                pDBData->GetQueryParam( aQueryParam );          // Bereich kann sich geaendert haben
                ScRange aAdvSource;
                if (pDBData->GetAdvancedQuerySource(aAdvSource))
                    Query( nTab, aQueryParam, &aAdvSource, FALSE, bApi );
                else
                    Query( nTab, aQueryParam, NULL, FALSE, bApi );

                //  bei nicht-inplace kann die Tabelle umgestellt worden sein
//              if ( !aQueryParam.bInplace && aQueryParam.nDestTab != nTab )
//                  SetTabNo( nTab );
            }
            if (bSubTotal)
            {
                pDBData->GetSubTotalParam( aSubTotalParam );    // Bereich kann sich geaendert haben
                aSubTotalParam.bRemoveOnly = FALSE;
                DoSubTotals( nTab, aSubTotalParam, NULL, FALSE, bApi );
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

                rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoRepeatDB( &rDocShell, nTab,
                                            nStartCol, nStartRow, nEndCol, nEndRow,
                                            nNewEndRow,
                                            //nCurX, nCurY,
                                            nStartCol, nStartRow,
                                            pUndoDoc, pUndoTab,
                                            pUndoRange, pUndoDB,
                                            pOld, pNew ) );
            }

            rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab,
                                    PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE );
            bDone = TRUE;
        }
        else if (!bApi)     // "Keine Operationen auszufuehren"
            rDocShell.ErrorMessage(STR_MSSG_REPEATDB_0);
    }

    return bDone;
}

// -----------------------------------------------------------------

BOOL ScDBDocFunc::Sort( USHORT nTab, const ScSortParam& rSortParam,
                            BOOL bRecord, BOOL bPaint, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    USHORT nSrcTab = nTab;

    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rSortParam.nCol1, rSortParam.nRow1,
                                                    rSortParam.nCol2, rSortParam.nRow2 );
    if (!pDBData)
    {
        DBG_ERROR( "Sort: keine DBData" );
        return FALSE;
    }

    ScDBData* pDestData = NULL;
    ScRange aOldDest;
    BOOL bCopy = !rSortParam.bInplace;
    if ( bCopy && rSortParam.nDestCol == rSortParam.nCol1 &&
                  rSortParam.nDestRow == rSortParam.nRow1 && rSortParam.nDestTab == nTab )
        bCopy = FALSE;
    ScSortParam aLocalParam( rSortParam );
    if ( bCopy )
    {
        aLocalParam.MoveToDest();
        nTab = rSortParam.nDestTab;
        pDestData = pDoc->GetDBAtCursor( rSortParam.nDestCol, rSortParam.nDestRow,
                                            rSortParam.nDestTab, TRUE );
        if (pDestData)
            pDestData->GetArea(aOldDest);
    }

    if (!pDoc->IsBlockEditable(nTab, aLocalParam.nCol1,aLocalParam.nRow1,
                                        aLocalParam.nCol2,aLocalParam.nRow2))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    if ( aLocalParam.bIncludePattern && pDoc->HasAttrib(
                                        aLocalParam.nCol1, aLocalParam.nRow1, nTab,
                                        aLocalParam.nCol2, aLocalParam.nRow2, nTab,
                                        HASATTR_MERGED | HASATTR_OVERLAPPED ) )
    {
        //  Merge-Attribute wuerden beim Sortieren durcheinanderkommen
        if (!bApi)
            rDocShell.ErrorMessage(STR_SORT_ERR_MERGED);
        return FALSE;
    }


    //      ausfuehren

    WaitObject aWait( rDocShell.GetDialogParent() );

    BOOL bRepeatQuery = FALSE;                          // bestehenden Filter wiederholen?
    ScQueryParam aQueryParam;
    pDBData->GetQueryParam( aQueryParam );
    if ( aQueryParam.GetEntry(0).bDoQuery )
        bRepeatQuery = TRUE;

    if (bRepeatQuery && bCopy)
    {
        if ( aQueryParam.bInplace ||
                aQueryParam.nDestCol != rSortParam.nDestCol ||
                aQueryParam.nDestRow != rSortParam.nDestRow ||
                aQueryParam.nDestTab != rSortParam.nDestTab )       // Query auf selben Zielbereich?
            bRepeatQuery = FALSE;
    }

    if ( bRecord )
    {
        //  Referenzen ausserhalb des Bereichs werden nicht veraendert !

        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        //  Zeilenhoehen immer (wegen automatischer Anpassung)
        //! auf ScBlockUndo umstellen
        pUndoDoc->InitUndo( pDoc, nTab, nTab, FALSE, TRUE );
        pDoc->CopyToDocument( aLocalParam.nCol1, aLocalParam.nRow1, nTab,
                                aLocalParam.nCol2, aLocalParam.nRow2, nTab,
                                IDF_ALL, FALSE, pUndoDoc );

        const ScRange* pR = 0;
        if (pDestData)
        {
            pDoc->CopyToDocument( aOldDest, IDF_ALL, FALSE, pUndoDoc );
            pR = &aOldDest;
        }

        //  Zeilenhoehen immer (wegen automatischer Anpassung)
        //! auf ScBlockUndo umstellen
//      if (bRepeatQuery)
            pDoc->CopyToDocument( 0, aLocalParam.nRow1, nTab, MAXCOL, aLocalParam.nRow2, nTab,
                                    IDF_NONE, FALSE, pUndoDoc );

        ScDBCollection* pUndoDB = NULL;
        ScDBCollection* pDocDB = pDoc->GetDBCollection();
        if (pDocDB->GetCount())
            pUndoDB = new ScDBCollection( *pDocDB );


        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoSort( &rDocShell, nTab,
                            rSortParam, bRepeatQuery, pUndoDoc, pUndoDB, pR ) );
    }

    if ( bCopy )
    {
        if (pDestData)
            pDoc->DeleteAreaTab(aOldDest, IDF_CONTENTS);            // Zielbereich vorher loeschen

        ScRange aSource( rSortParam.nCol1,rSortParam.nRow1,nSrcTab,
                            rSortParam.nCol2,rSortParam.nRow2,nSrcTab );
        ScAddress aDest( rSortParam.nDestCol, rSortParam.nDestRow, rSortParam.nDestTab );

        rDocShell.GetDocFunc().MoveBlock( aSource, aDest, FALSE, FALSE, FALSE, TRUE );
    }
    pDoc->Sort( nTab, aLocalParam, bRepeatQuery );

    BOOL bSave = TRUE;
    if (bCopy)
    {
        ScSortParam aOldSortParam;
        pDBData->GetSortParam( aOldSortParam );
        if ( aOldSortParam.bDoSort[0] && aOldSortParam.bInplace )   // Inplace-Sortierung gemerkt?
        {
            bSave = FALSE;
            aOldSortParam.nDestCol = rSortParam.nDestCol;
            aOldSortParam.nDestRow = rSortParam.nDestRow;
            aOldSortParam.nDestTab = rSortParam.nDestTab;
            pDBData->SetSortParam( aOldSortParam );                 // dann nur DestPos merken
        }
    }
    if (bSave)                                              // Parameter merken
    {
        pDBData->SetSortParam( rSortParam );
        pDBData->SetHeader( rSortParam.bHasHeader );        //! ???
        pDBData->SetByRow( rSortParam.bByRow );             //! ???
    }

    if (bCopy)                                          // neuen DB-Bereich merken
    {
        //  Tabelle umschalten von aussen (View)
        //! SetCursor ??!?!

        ScRange aDestPos( aLocalParam.nCol1, aLocalParam.nRow1, nTab,
                            aLocalParam.nCol2, aLocalParam.nRow2, nTab );
        ScDBData* pNewData;
        if (pDestData)
            pNewData = pDestData;               // Bereich vorhanden -> anpassen
        else                                    // Bereich ab Cursor/Markierung wird angelegt
            pNewData = rDocShell.GetDBData(aDestPos, SC_DB_MAKE, TRUE );
        if (pNewData)
        {
            pNewData->SetArea( nTab,
                                aLocalParam.nCol1,aLocalParam.nRow1,
                                aLocalParam.nCol2,aLocalParam.nRow2 );
            pNewData->SetSortParam( aLocalParam );
            pNewData->SetHeader( aLocalParam.bHasHeader );      //! ???
            pNewData->SetByRow( aLocalParam.bByRow );
        }
        else
            DBG_ERROR("Zielbereich nicht da");
    }

    ScRange aDirtyRange( aLocalParam.nCol1, aLocalParam.nRow1, nTab,
        aLocalParam.nCol2, aLocalParam.nRow2, nTab );
    pDoc->SetDirty( aDirtyRange );

    if (bPaint)
    {
        USHORT nPaint = PAINT_GRID;
        USHORT nStartX = aLocalParam.nCol1;
        USHORT nStartY = aLocalParam.nRow1;
        USHORT nEndX = aLocalParam.nCol2;
        USHORT nEndY = aLocalParam.nRow2;
        if ( bRepeatQuery )
        {
            nPaint |= PAINT_LEFT;
            nStartX = 0;
            nEndX = MAXCOL;
        }
        if (pDestData)
        {
            if ( nEndX < aOldDest.aEnd.Col() )
                nEndX = aOldDest.aEnd.Col();
            if ( nEndY < aOldDest.aEnd.Row() )
                nEndY = aOldDest.aEnd.Row();
        }
        rDocShell.PostPaint( nStartX, nStartY, nTab, nEndX, nEndY, nTab, nPaint );
    }

//  AdjustRowHeight( aLocalParam.nRow1, aLocalParam.nRow2, bPaint );
    rDocShell.AdjustRowHeight( aLocalParam.nRow1, aLocalParam.nRow2, nTab );

    aModificator.SetDocumentModified();

    return TRUE;
}

// -----------------------------------------------------------------

BOOL ScDBDocFunc::Query( USHORT nTab, const ScQueryParam& rQueryParam,
                        const ScRange* pAdvSource, BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rQueryParam.nCol1, rQueryParam.nRow1,
                                                    rQueryParam.nCol2, rQueryParam.nRow2 );
    if (!pDBData)
    {
        DBG_ERROR( "Query: keine DBData" );
        return FALSE;
    }

    //  Wechsel von Inplace auf nicht-Inplace, dann erst Inplace aufheben:
    //  (nur, wenn im Dialog "Persistent" ausgewaehlt ist)

    if ( !rQueryParam.bInplace && pDBData->HasQueryParam() && rQueryParam.bDestPers )
    {
        ScQueryParam aOldQuery;
        pDBData->GetQueryParam(aOldQuery);
        if (aOldQuery.bInplace)
        {
            //  alte Filterung aufheben

            USHORT nEC = aOldQuery.GetEntryCount();
            for (USHORT i=0; i<nEC; i++)
                aOldQuery.GetEntry(i).bDoQuery = FALSE;
            aOldQuery.bDuplicate = TRUE;
            Query( nTab, aOldQuery, NULL, bRecord, bApi );
        }
    }

    ScQueryParam aLocalParam( rQueryParam );        // fuer Paint / Zielbereich
    BOOL bCopy = !rQueryParam.bInplace;             // kopiert wird in Table::Query
    ScDBData* pDestData = NULL;                     // Bereich, in den kopiert wird
    BOOL bDoSize = FALSE;                           // Zielgroesse anpassen (einf./loeschen)
    USHORT nFormulaCols = 0;                        // nur bei bDoSize
    BOOL bKeepFmt = FALSE;
    ScRange aOldDest;
    ScRange aDestTotal;
    if ( bCopy && rQueryParam.nDestCol == rQueryParam.nCol1 &&
                  rQueryParam.nDestRow == rQueryParam.nRow1 && rQueryParam.nDestTab == nTab )
        bCopy = FALSE;
    USHORT nDestTab = nTab;
    if ( bCopy )
    {
        aLocalParam.MoveToDest();
        nDestTab = rQueryParam.nDestTab;

        if (!pDoc->IsBlockEditable(nDestTab, aLocalParam.nCol1,aLocalParam.nRow1,
                                                aLocalParam.nCol2,aLocalParam.nRow2))
        {
            if (!bApi)
                rDocShell.ErrorMessage(STR_PROTECTIONERR);
            return FALSE;
        }

        pDestData = pDoc->GetDBAtCursor( rQueryParam.nDestCol, rQueryParam.nDestRow,
                                            rQueryParam.nDestTab, TRUE );
        if (pDestData)
        {
            pDestData->GetArea( aOldDest );
            aDestTotal=ScRange( rQueryParam.nDestCol,
                                rQueryParam.nDestRow,
                                nDestTab,
                                rQueryParam.nDestCol + rQueryParam.nCol2 - rQueryParam.nCol1,
                                rQueryParam.nDestRow + rQueryParam.nRow2 - rQueryParam.nRow1,
                                nDestTab );

            bDoSize = pDestData->IsDoSize();
            //  Test, ob Formeln aufgefuellt werden muessen (nFormulaCols):
            if ( bDoSize && aOldDest.aEnd.Col() == aDestTotal.aEnd.Col() )
            {
                USHORT nTestCol = aOldDest.aEnd.Col() + 1;      // neben dem Bereich
                USHORT nTestRow = rQueryParam.nDestRow +
                                    ( aLocalParam.bHasHeader ? 1 : 0 );
                while ( nTestCol <= MAXCOL &&
                        pDoc->GetCellType(ScAddress( nTestCol, nTestRow, nTab )) == CELLTYPE_FORMULA )
                    ++nTestCol, ++nFormulaCols;
            }

            bKeepFmt = pDestData->IsKeepFmt();
            if ( bDoSize && !pDoc->CanFitBlock( aOldDest, aDestTotal ) )
            {
                if (!bApi)
                    rDocShell.ErrorMessage(STR_MSSG_DOSUBTOTALS_2);     // kann keine Zeilen einfuegen
                return FALSE;
            }
        }
    }

    //      ausfuehren

    WaitObject aWait( rDocShell.GetDialogParent() );

    BOOL bKeepSub = FALSE;                          // bestehende Teilergebnisse wiederholen?
    ScSubTotalParam aSubTotalParam;
    if (rQueryParam.GetEntry(0).bDoQuery)           // nicht beim Aufheben
    {
        pDBData->GetSubTotalParam( aSubTotalParam );    // Teilergebnisse vorhanden?

        if ( aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly )
            bKeepSub = TRUE;
    }

    if ( bRecord )
    {
        const ScRange* pOld = 0;
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bCopy)
        {
            pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab, FALSE, TRUE );
            pDoc->CopyToDocument( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                    aLocalParam.nCol2, aLocalParam.nRow2, nDestTab,
                                    IDF_ALL, FALSE, pUndoDoc );
            //  Attribute sichern, falls beim Filtern mitkopiert

            if (pDestData)
            {
                pDoc->CopyToDocument( aOldDest, IDF_ALL, FALSE, pUndoDoc );
                pOld = &aOldDest;
            }
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, FALSE, TRUE );
            pDoc->CopyToDocument( 0, rQueryParam.nRow1, nTab, MAXCOL, rQueryParam.nRow2, nTab,
                                        IDF_NONE, FALSE, pUndoDoc );
        }

        ScDBCollection* pUndoDB = NULL;
        ScDBCollection* pDocDB = pDoc->GetDBCollection();
        if (pDocDB->GetCount())
            pUndoDB = new ScDBCollection( *pDocDB );

        rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoQuery( &rDocShell, nTab, rQueryParam, pUndoDoc, pUndoDB,
                                        pOld, bDoSize, pAdvSource ) );
    }

    ScDocument* pAttribDoc = NULL;
    ScRange aAttribRange;
    if (pDestData)                                      // Zielbereich loeschen
    {
        if ( bKeepFmt )
        {
            //  kleinere der End-Spalten, Header+1 Zeile
            aAttribRange = aOldDest;
            if ( aAttribRange.aEnd.Col() > aDestTotal.aEnd.Col() )
                aAttribRange.aEnd.SetCol( aDestTotal.aEnd.Col() );
            aAttribRange.aEnd.SetRow( aAttribRange.aStart.Row() +
                                        ( aLocalParam.bHasHeader ? 1 : 0 ) );

            //  auch fuer aufgefuellte Formeln
            aAttribRange.aEnd.SetCol( aAttribRange.aEnd.Col() + nFormulaCols );

            pAttribDoc = new ScDocument( SCDOCMODE_UNDO );
            pAttribDoc->InitUndo( pDoc, nDestTab, nDestTab, FALSE, TRUE );
            pDoc->CopyToDocument( aAttribRange, IDF_ATTRIB, FALSE, pAttribDoc );
        }

        if ( bDoSize )
            pDoc->FitBlock( aOldDest, aDestTotal );
        else
            pDoc->DeleteAreaTab(aOldDest, IDF_ALL);         // einfach loeschen
    }

    //  Filtern am Dokument ausfuehren
    USHORT nCount = pDoc->Query( nTab, rQueryParam, bKeepSub );
    if (bCopy)
    {
        aLocalParam.nRow2 = aLocalParam.nRow1 + nCount;
        if (!aLocalParam.bHasHeader && nCount)
            --aLocalParam.nRow2;

        if ( bDoSize )
        {
            //  auf wirklichen Ergebnis-Bereich anpassen
            //  (das hier ist immer eine Verkleinerung)

            ScRange aNewDest( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                aLocalParam.nCol2, aLocalParam.nRow2, nDestTab );
            pDoc->FitBlock( aDestTotal, aNewDest, FALSE );      // FALSE - nicht loeschen

            if ( nFormulaCols )
            {
                //  Formeln ausfuellen
                //! Undo (Query und Repeat) !!!

                ScRange aNewForm( aLocalParam.nCol2+1, aLocalParam.nRow1, nDestTab,
                                  aLocalParam.nCol2+nFormulaCols, aLocalParam.nRow2, nDestTab );
                ScRange aOldForm = aNewForm;
                aOldForm.aEnd.SetRow( aOldDest.aEnd.Row() );
                pDoc->FitBlock( aOldForm, aNewForm, FALSE );

                ScMarkData aMark;
                aMark.SelectOneTable(nDestTab);
                USHORT nFStartY = aLocalParam.nRow1 + ( aLocalParam.bHasHeader ? 1 : 0 );
                pDoc->Fill( aLocalParam.nCol2+1, nFStartY,
                            aLocalParam.nCol2+nFormulaCols, nFStartY, aMark,
                            aLocalParam.nRow2 - nFStartY,
                            FILL_TO_BOTTOM, FILL_SIMPLE );
            }
        }

        if ( pAttribDoc )       // gemerkte Attribute zurueckkopieren
        {
            //  Header
            if (aLocalParam.bHasHeader)
            {
                ScRange aHdrRange = aAttribRange;
                aHdrRange.aEnd.SetRow( aHdrRange.aStart.Row() );
                pAttribDoc->CopyToDocument( aHdrRange, IDF_ATTRIB, FALSE, pDoc );
            }

            //  Daten
            USHORT nAttrEndCol = aAttribRange.aEnd.Col();
            USHORT nAttrRow = aAttribRange.aStart.Row() + ( aLocalParam.bHasHeader ? 1 : 0 );
            for (USHORT nCol = aAttribRange.aStart.Col(); nCol<=nAttrEndCol; nCol++)
            {
                const ScPatternAttr* pSrcPattern = pAttribDoc->GetPattern(
                                                    nCol, nAttrRow, nDestTab );
                DBG_ASSERT(pSrcPattern,"Pattern ist 0");
                if (pSrcPattern)
                    pDoc->ApplyPatternAreaTab( nCol, nAttrRow, nCol, aLocalParam.nRow2,
                                                    nDestTab, *pSrcPattern );
                const ScStyleSheet* pStyle = pSrcPattern->GetStyleSheet();
                if (pStyle)
                    pDoc->ApplyStyleAreaTab( nCol, nAttrRow, nCol, aLocalParam.nRow2,
                                                    nDestTab, *pStyle );
            }

            delete pAttribDoc;
        }
    }

    //  speichern: Inplace immer, sonst je nach Einstellung
    //             alter Inplace-Filter ist ggf. schon aufgehoben

    BOOL bSave = rQueryParam.bInplace || rQueryParam.bDestPers;
    if (bSave)                                                  // merken
    {
        pDBData->SetQueryParam( rQueryParam );
        pDBData->SetHeader( rQueryParam.bHasHeader );       //! ???
        pDBData->SetAdvancedQuerySource( pAdvSource );      // after SetQueryParam
    }

    if (bCopy)                                              // neuen DB-Bereich merken
    {
        //  selektieren wird hinterher von aussen (dbfunc)
        //  momentan ueber DB-Bereich an der Zielposition, darum muss dort
        //  auf jeden Fall ein Bereich angelegt werden.

        ScDBData* pNewData;
        if (pDestData)
            pNewData = pDestData;               // Bereich vorhanden -> anpassen (immer!)
        else                                    // Bereich anlegen
            pNewData = rDocShell.GetDBData(
                            ScRange( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                     aLocalParam.nCol2, aLocalParam.nRow2, nDestTab ),
                            SC_DB_MAKE, TRUE );

        if (pNewData)
        {
            pNewData->SetArea( nDestTab, aLocalParam.nCol1, aLocalParam.nRow1,
                                            aLocalParam.nCol2, aLocalParam.nRow2 );

            //  Query-Param wird am Ziel nicht mehr eingestellt, fuehrt nur zu Verwirrung
            //  und Verwechslung mit dem Query-Param am Quellbereich (#37187#)
        }
        else
            DBG_ERROR("Zielbereich nicht da");
    }

    if (!bCopy)
        pDoc->UpdatePageBreaks( nTab );

    ScRange aDirtyRange( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
        aLocalParam.nCol2, aLocalParam.nRow2, nDestTab );
    pDoc->SetDirty( aDirtyRange );

    if (bCopy)
    {
        USHORT nEndX = aLocalParam.nCol2;
        USHORT nEndY = aLocalParam.nRow2;
        if (pDestData)
        {
            if ( aOldDest.aEnd.Col() > nEndX )
                nEndX = aOldDest.aEnd.Col();
            if ( aOldDest.aEnd.Row() > nEndY )
                nEndY = aOldDest.aEnd.Row();
        }
        if (bDoSize)
            nEndY = MAXROW;
        rDocShell.PostPaint( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                    nEndX, nEndY, nDestTab, PAINT_GRID );
    }
    else
        rDocShell.PostPaint( 0, rQueryParam.nRow1, nTab, MAXCOL, MAXROW, nTab,
                                                PAINT_GRID | PAINT_LEFT );
    aModificator.SetDocumentModified();

    return TRUE;
}

// -----------------------------------------------------------------

BOOL ScDBDocFunc::DoSubTotals( USHORT nTab, const ScSubTotalParam& rParam,
                                const ScSortParam* pForceNewSort, BOOL bRecord, BOOL bApi )
{
    //! auch fuer ScDBFunc::DoSubTotals benutzen!
    //  dann bleibt aussen:
    //  - neuen Bereich (aus DBData) markieren
    //  - SelectionChanged (?)

    BOOL bDo = !rParam.bRemoveOnly;                         // FALSE = nur loeschen
    BOOL bRet = FALSE;

    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rParam.nCol1, rParam.nRow1,
                                                rParam.nCol2, rParam.nRow2 );
    if (!pDBData)
    {
        DBG_ERROR( "SubTotals: keine DBData" );
        return FALSE;
    }

    if (!pDoc->IsBlockEditable( nTab, 0,rParam.nRow1+1, MAXCOL,MAXROW ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return FALSE;
    }

    if (pDoc->HasAttrib( rParam.nCol1, rParam.nRow1+1, nTab,
                         rParam.nCol2, rParam.nRow2, nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0); // nicht in zusammengefasste einfuegen
        return FALSE;
    }

    BOOL bOk = TRUE;
    BOOL bDelete = FALSE;
    if (rParam.bReplace)
        if (pDoc->TestRemoveSubTotals( nTab, rParam ))
        {
            bDelete = TRUE;
            bOk = ( MessBox( rDocShell.GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                // "StarCalc" "Daten loeschen?"
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_1 ) ).Execute()
                == RET_YES );
        }

    if (bOk)
    {
        WaitObject aWait( rDocShell.GetDialogParent() );
        ScDocShellModificator aModificator( rDocShell );

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
                Sort( nTab, aSortParam, FALSE, FALSE, bApi );
            }

            bSuccess = pDoc->DoSubTotals( nTab, aNewParam );
        }
        ScRange aDirtyRange( aNewParam.nCol1, aNewParam.nRow1, nTab,
            aNewParam.nCol2, aNewParam.nRow2, nTab );
        pDoc->SetDirty( aDirtyRange );

        if (bRecord)
        {
//          ScDBData* pUndoDBData = pDBData ? new ScDBData( *pDBData ) : NULL;
            rDocShell.GetUndoManager()->AddUndoAction(
                new ScUndoSubTotals( &rDocShell, nTab,
                                        rParam, aNewParam.nRow2,
                                        pUndoDoc, pUndoTab, // pUndoDBData,
                                        pUndoRange, pUndoDB ) );
        }

        if (!bSuccess)
        {
            // "Kann keine Zeilen einfuegen"
            if (!bApi)
                rDocShell.ErrorMessage(STR_MSSG_DOSUBTOTALS_2);
        }

                                                    // merken
        pDBData->SetSubTotalParam( aNewParam );
        pDBData->SetArea( nTab, aNewParam.nCol1,aNewParam.nRow1, aNewParam.nCol2,aNewParam.nRow2 );
        pDoc->CompileDBFormula();

        rDocShell.PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab,
                                                PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE );
        aModificator.SetDocumentModified();

        bRet = bSuccess;
    }
    return bRet;
}

//==================================================================

BOOL lcl_BlockEditable( ScDocument* pDoc, const ScRange& rRange )
{
    return pDoc->IsBlockEditable( rRange.aStart.Tab(),
                                  rRange.aStart.Col(), rRange.aStart.Row(),
                                  rRange.aEnd.Col(), rRange.aEnd.Row() );
}

BOOL ScDBDocFunc::DataPilotUpdate( ScDPObject* pOldObj, const ScDPObject* pNewObj,
                                        BOOL bRecord, BOOL bApi )
{
    ScDocShellModificator aModificator( rDocShell );
    WaitObject aWait( rDocShell.GetDialogParent() );

    BOOL bDone = FALSE;
    BOOL bUndoSelf = FALSE;
    USHORT nErrId = 0;

    ScDocument* pOldUndoDoc = NULL;
    ScDocument* pNewUndoDoc = NULL;
    ScDPObject* pUndoDPObj = NULL;
    if ( bRecord && pOldObj )
        pUndoDPObj = new ScDPObject( *pOldObj );    // copy old settings for undo

    ScDocument* pDoc = rDocShell.GetDocument();
    if ( rDocShell.IsReadOnly() || pDoc->GetChangeTrack() )
    {
        //  not recorded -> disallow
        //! different error messages?

        nErrId = STR_PROTECTIONERR;
    }
    if ( pOldObj && !nErrId )
    {
        ScRange aOldOut = pOldObj->GetOutRange();
        if ( !lcl_BlockEditable( pDoc, aOldOut ) )
            nErrId = STR_PROTECTIONERR;
    }
    if ( pNewObj && !nErrId )
    {
        //  at least one cell at the output position must be editable
        //  -> check in advance
        //  (start of output range in pNewObj is valid)

        ScRange aNewStart( pNewObj->GetOutRange().aStart );
        if ( !lcl_BlockEditable( pDoc, aNewStart ) )
            nErrId = STR_PROTECTIONERR;
    }

    ScDPObject* pDestObj = NULL;
    if ( !nErrId )
    {
        if ( pOldObj && !pNewObj )
        {
            //  delete table

            ScRange aRange = pOldObj->GetOutRange();
            USHORT nTab = aRange.aStart.Tab();

            if ( bRecord )
            {
                pOldUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pOldUndoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pOldUndoDoc );
            }

            pDoc->DeleteAreaTab( aRange.aStart.Col(), aRange.aStart.Row(),
                                 aRange.aEnd.Col(),   aRange.aEnd.Row(),
                                 nTab, IDF_ALL );

            pDoc->GetDPCollection()->Free( pOldObj );   // object is deleted here

            rDocShell.PostPaintGridAll();   //! only necessary parts
            rDocShell.PostPaint( aRange.aStart.Col(), aRange.aStart.Row(), nTab,
                                 aRange.aEnd.Col(),   aRange.aEnd.Row(),   nTab,
                                 PAINT_GRID );
            bDone = TRUE;
        }
        else if ( pNewObj )
        {
            if ( pOldObj )
            {
                if ( bRecord )
                {
                    ScRange aRange = pOldObj->GetOutRange();
                    USHORT nTab = aRange.aStart.Tab();
                    pOldUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                    pOldUndoDoc->InitUndo( pDoc, nTab, nTab );
                    pDoc->CopyToDocument( aRange, IDF_ALL, FALSE, pOldUndoDoc );
                }

                if ( pNewObj == pOldObj )
                {
                    //  refresh only - no settings modified
                }
                else
                {
                    pNewObj->WriteSourceDataTo( *pOldObj );     // copy source data

                    ScDPSaveData* pData = pNewObj->GetSaveData();
                    DBG_ASSERT( pData, "no SaveData from living DPObject" );
                    if ( pData )
                        pOldObj->SetSaveData( *pData );     // copy SaveData
                }

                pDestObj = pOldObj;
            }
            else
            {
                //  output range must be set at pNewObj

                pDestObj = new ScDPObject( *pNewObj );
                pDestObj->SetAlive(TRUE);
                if ( !pDoc->GetDPCollection()->Insert(pDestObj) )
                {
                    DBG_ERROR("cannot insert DPObject");
                    DELETEZ( pDestObj );
                }
            }
            if ( pDestObj )
            {
                pDestObj->InvalidateData();             // before getting the new output area

                //  make sure the table has a name (not set by dialog)
                if ( !pDestObj->GetName().Len() )
                    pDestObj->SetName( pDoc->GetDPCollection()->CreateNewName() );

                ScRange aNewOut = pDestObj->GetNewOutputRange();
                if ( !lcl_BlockEditable( pDoc, aNewOut ) )
                {
                    //  destination area isn't editable
                    //! reverse everything done so far, don't proceed

                    //  quick solution: proceed to end, use undo action
                    //  to reverse everything:
                    DBG_ASSERT( bRecord, "DataPilotUpdate: can't undo" );
                    bUndoSelf = TRUE;
                    nErrId = STR_PROTECTIONERR;
                }

                if ( bRecord )
                {
                    USHORT nTab = aNewOut.aStart.Tab();
                    pNewUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                    pNewUndoDoc->InitUndo( pDoc, nTab, nTab );
                    pDoc->CopyToDocument( aNewOut, IDF_ALL, FALSE, pNewUndoDoc );
                }

                //! test for overlap with other data pilot tables

                //! test if output area is empty and query before overwriting

                pDestObj->Output();

                rDocShell.PostPaintGridAll();           //! only necessary parts
                bDone = TRUE;
            }
        }
        // else nothing (no old, no new)
    }

    if ( bRecord && bDone )
    {
        SfxUndoAction* pAction = new ScUndoDataPilot( &rDocShell,
                                    pOldUndoDoc, pNewUndoDoc, pUndoDPObj, pDestObj );
        pOldUndoDoc = NULL;
        pNewUndoDoc = NULL;     // pointers are used in undo action
        // pUndoDPObj is copied

        if (bUndoSelf)
        {
            //  use undo action to restore original state
            //! prevent setting the document modified? (ScDocShellModificator)

            pAction->Undo();
            delete pAction;
            bDone = FALSE;
        }
        else
            rDocShell.GetUndoManager()->AddUndoAction( pAction );
    }

    delete pOldUndoDoc;     // if not used for undo
    delete pNewUndoDoc;
    delete pUndoDPObj;

    if (bDone)
        aModificator.SetDocumentModified();

    if ( nErrId && !bApi )
        rDocShell.ErrorMessage( nErrId );

    return bDone;
}

//==================================================================
//
//      Datenbank-Import...

void ScDBDocFunc::UpdateImport( const String& rTarget, const String& rDBName,
                                    const String& rTableName, const String& rStatement,
                                    BOOL bNative, BYTE nType,
                                    const SbaSelectionList* pSelection )
{
    //  Target ist jetzt einfach der Bereichsname

    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection& rDBColl = *pDoc->GetDBCollection();
    ScDBData* pData;
    ScImportParam aImportParam;
    BOOL bFound = FALSE;
    USHORT nCount = rDBColl.GetCount();
    for (USHORT i=0; i<nCount && !bFound; i++)
    {
        pData = rDBColl[i];
        if (pData->GetName() == rTarget)
            bFound = TRUE;
    }
    if (!bFound)
    {
        InfoBox aInfoBox(rDocShell.GetDialogParent(),
                    ScGlobal::GetRscString( STR_TARGETNOTFOUND ) );
        aInfoBox.Execute();
        return;
    }

    USHORT nTab;
    USHORT nDummy;
    pData->GetArea( nTab, nDummy,nDummy,nDummy,nDummy );
    pData->GetImportParam( aImportParam );

    BOOL bSql = ( rStatement.Len() != 0 );

    aImportParam.aDBName    = rDBName;
    aImportParam.bSql       = bSql;
    aImportParam.aStatement = bSql ? rStatement : rTableName;
    aImportParam.bNative    = bNative;
    aImportParam.nType      = nType;
    aImportParam.bImport    = TRUE;
    BOOL bContinue = DoImport( nTab, aImportParam, pSelection, TRUE );

    //  DB-Operationen wiederholen

    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();
    if (pViewSh)
    {
        ScRange aRange;
        pData->GetArea(aRange);
        pViewSh->MarkRange(aRange);         // selektieren

        if ( bContinue )        // #41905# Fehler beim Import -> Abbruch
        {
            //  interne Operationen, wenn welche gespeichert

            if ( pData->HasQueryParam() || pData->HasSortParam() || pData->HasSubTotalParam() )
                pViewSh->RepeatDB();

            //  Pivottabellen die den Bereich als Quelldaten haben

            rDocShell.RefreshPivotTables(aRange);
        }
    }
}




