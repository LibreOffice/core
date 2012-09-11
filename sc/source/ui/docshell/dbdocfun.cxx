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

#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <svx/dataaccessdescriptor.hxx>

#include <com/sun/star/sdb/CommandType.hpp>

#include "dbdocfun.hxx"
#include "sc.hrc"
#include "dbdata.hxx"
#include "undodat.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "globstr.hrc"
#include "globalnames.hxx"
#include "tabvwsh.hxx"
#include "patattr.hxx"
#include "rangenam.hxx"
#include "olinetab.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dociter.hxx"      // for lcl_EmptyExcept
#include "cell.hxx"         // for lcl_EmptyExcept
#include "editable.hxx"
#include "attrib.hxx"
#include "drwlayer.hxx"
#include "dpshttab.hxx"
#include "hints.hxx"
#include "queryentry.hxx"
#include "markdata.hxx"
#include "progress.hxx"

#include <set>

using namespace ::com::sun::star;

// -----------------------------------------------------------------

bool ScDBDocFunc::AddDBRange( const ::rtl::OUString& rName, const ScRange& rRange, sal_Bool /* bApi */ )
{

    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();
    sal_Bool bUndo (pDoc->IsUndoEnabled());

    ScDBCollection* pUndoColl = NULL;
    if (bUndo)
        pUndoColl = new ScDBCollection( *pDocColl );

    ScDBData* pNew = new ScDBData( rName, rRange.aStart.Tab(),
                                    rRange.aStart.Col(), rRange.aStart.Row(),
                                    rRange.aEnd.Col(), rRange.aEnd.Row() );

    // #i55926# While loading XML, formula cells only have a single string token,
    // so CompileDBFormula would never find any name (index) tokens, and would
    // unnecessarily loop through all cells.
    bool bCompile = !pDoc->IsImportingXML();
    bool bOk;
    if ( bCompile )
        pDoc->CompileDBFormula( sal_True );     // CreateFormulaString
    if ( rName == STR_DB_LOCAL_NONAME )
    {
        pDoc->SetAnonymousDBData(rRange.aStart.Tab() , pNew);
        bOk = true;
    }
    else
    {
        bOk = pDocColl->getNamedDBs().insert(pNew);
    }
    if ( bCompile )
        pDoc->CompileDBFormula( false );    // CompileFormulaString

    if (!bOk)
    {
        delete pNew;
        delete pUndoColl;
        return false;
    }

    if (bUndo)
    {
        ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
        rDocShell.GetUndoManager()->AddUndoAction(
                        new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );
    }

    aModificator.SetDocumentModified();
    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
    return true;
}

bool ScDBDocFunc::DeleteDBRange(const ::rtl::OUString& rName)
{
    bool bDone = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();
    bool bUndo = pDoc->IsUndoEnabled();

    ScDBCollection::NamedDBs& rDBs = pDocColl->getNamedDBs();
    const ScDBData* p = rDBs.findByName(rName);
    if (p)
    {
        ScDocShellModificator aModificator( rDocShell );

        ScDBCollection* pUndoColl = NULL;
        if (bUndo)
            pUndoColl = new ScDBCollection( *pDocColl );

        pDoc->CompileDBFormula( true );     // CreateFormulaString
        rDBs.erase(*p);
        pDoc->CompileDBFormula( false );    // CompileFormulaString

        if (bUndo)
        {
            ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
            rDocShell.GetUndoManager()->AddUndoAction(
                            new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );
        }

        aModificator.SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
        bDone = true;
    }

    return bDone;
}

bool ScDBDocFunc::RenameDBRange( const String& rOld, const String& rNew )
{
    bool bDone = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();
    bool bUndo = pDoc->IsUndoEnabled();
    ScDBCollection::NamedDBs& rDBs = pDocColl->getNamedDBs();
    const ScDBData* pOld = rDBs.findByName(rOld);
    const ScDBData* pNew = rDBs.findByName(rNew);
    if (pOld && !pNew)
    {
        ScDocShellModificator aModificator( rDocShell );

        ScDBData* pNewData = new ScDBData(rNew, *pOld);

        ScDBCollection* pUndoColl = new ScDBCollection( *pDocColl );

        pDoc->CompileDBFormula(true);               // CreateFormulaString
        rDBs.erase(*pOld);
        bool bInserted = rDBs.insert(pNewData);
        if (!bInserted)                             // Fehler -> alten Zustand wiederherstellen
            pDoc->SetDBCollection(pUndoColl);       // gehoert dann dem Dokument
                                                    //
        pDoc->CompileDBFormula( false );            // CompileFormulaString

        if (bInserted)                              // Einfuegen hat geklappt
        {
            if (bUndo)
            {
                ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
                rDocShell.GetUndoManager()->AddUndoAction(
                                new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );
            }
            else
                delete pUndoColl;

            aModificator.SetDocumentModified();
            SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
            bDone = true;
        }
    }

    return bDone;
}

bool ScDBDocFunc::ModifyDBData( const ScDBData& rNewData )
{
    bool bDone = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = pDoc->GetDBCollection();
    bool bUndo = pDoc->IsUndoEnabled();

    ScDBData* pData = NULL;
    if (rNewData.GetName().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(STR_DB_LOCAL_NONAME)))
    {
        ScRange aRange;
        rNewData.GetArea(aRange);
        SCTAB nTab = aRange.aStart.Tab();
        pData = pDoc->GetAnonymousDBData(nTab);
    }
    else
        pData = pDocColl->getNamedDBs().findByName(rNewData.GetName());

    if (pData)
    {
        ScDocShellModificator aModificator( rDocShell );
        ScRange aOldRange, aNewRange;
        pData->GetArea(aOldRange);
        rNewData.GetArea(aNewRange);
        bool bAreaChanged = ( aOldRange != aNewRange );     // dann muss neu compiliert werden

        ScDBCollection* pUndoColl = NULL;
        if (bUndo)
            pUndoColl = new ScDBCollection( *pDocColl );

        *pData = rNewData;
        if (bAreaChanged)
            pDoc->CompileDBFormula();

        if (bUndo)
        {
            ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
            rDocShell.GetUndoManager()->AddUndoAction(
                            new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );
        }

        aModificator.SetDocumentModified();
        bDone = true;
    }

    return bDone;
}

// -----------------------------------------------------------------

bool ScDBDocFunc::RepeatDB( const ::rtl::OUString& rDBName, bool bRecord, bool bApi, bool bIsUnnamed, SCTAB aTab )
{
    //! auch fuer ScDBFunc::RepeatDB benutzen!

    bool bDone = false;
    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    ScDBData* pDBData = NULL;
    if (bIsUnnamed)
    {
        pDBData = pDoc->GetAnonymousDBData( aTab );
    }
    else
    {
        ScDBCollection* pColl = pDoc->GetDBCollection();
        if (pColl)
            pDBData = pColl->getNamedDBs().findByName(rDBName);
    }

    if ( pDBData )
    {
        ScQueryParam aQueryParam;
        pDBData->GetQueryParam( aQueryParam );
        sal_Bool bQuery = aQueryParam.GetEntry(0).bDoQuery;

        ScSortParam aSortParam;
        pDBData->GetSortParam( aSortParam );
        sal_Bool bSort = aSortParam.maKeyState[0].bDoSort;

        ScSubTotalParam aSubTotalParam;
        pDBData->GetSubTotalParam( aSubTotalParam );
        sal_Bool bSubTotal = aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly;

        if ( bQuery || bSort || bSubTotal )
        {
            sal_Bool bQuerySize = false;
            ScRange aOldQuery;
            ScRange aNewQuery;
            if (bQuery && !aQueryParam.bInplace)
            {
                ScDBData* pDest = pDoc->GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                        aQueryParam.nDestTab, sal_True );
                if (pDest && pDest->IsDoSize())
                {
                    pDest->GetArea( aOldQuery );
                    bQuerySize = sal_True;
                }
            }

            SCTAB nTab;
            SCCOL nStartCol;
            SCROW nStartRow;
            SCCOL nEndCol;
            SCROW nEndRow;
            pDBData->GetArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );

            //!     Undo nur benoetigte Daten ?

            ScDocument* pUndoDoc = NULL;
            ScOutlineTable* pUndoTab = NULL;
            ScRangeName* pUndoRange = NULL;
            ScDBCollection* pUndoDB = NULL;

            if (bRecord)
            {
                SCTAB nTabCount = pDoc->GetTableCount();
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
                if (pTable)
                {
                    pUndoTab = new ScOutlineTable( *pTable );

                    // column/row state
                    SCCOLROW nOutStartCol, nOutEndCol;
                    SCCOLROW nOutStartRow, nOutEndRow;
                    pTable->GetColArray()->GetRange( nOutStartCol, nOutEndCol );
                    pTable->GetRowArray()->GetRange( nOutStartRow, nOutEndRow );

                    pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, sal_True );
                    pDoc->CopyToDocument( static_cast<SCCOL>(nOutStartCol), 0,
                            nTab, static_cast<SCCOL>(nOutEndCol), MAXROW, nTab,
                            IDF_NONE, false, pUndoDoc );
                    pDoc->CopyToDocument( 0, static_cast<SCROW>(nOutStartRow),
                            nTab, MAXCOL, static_cast<SCROW>(nOutEndRow), nTab,
                            IDF_NONE, false, pUndoDoc );
                }
                else
                    pUndoDoc->InitUndo( pDoc, nTab, nTab, false, sal_True );

                //  Datenbereich sichern - incl. Filter-Ergebnis
                pDoc->CopyToDocument( 0,nStartRow,nTab, MAXCOL,nEndRow,nTab, IDF_ALL, false, pUndoDoc );

                //  alle Formeln wegen Referenzen
                pDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1, IDF_FORMULA, false, pUndoDoc );

                //  DB- und andere Bereiche
                ScRangeName* pDocRange = pDoc->GetRangeName();
                if (!pDocRange->empty())
                    pUndoRange = new ScRangeName( *pDocRange );
                ScDBCollection* pDocDB = pDoc->GetDBCollection();
                if (!pDocDB->empty())
                    pUndoDB = new ScDBCollection( *pDocDB );
            }

            if (bSort && bSubTotal)
            {
                //  Sortieren ohne SubTotals

                aSubTotalParam.bRemoveOnly = sal_True;      // wird unten wieder zurueckgesetzt
                DoSubTotals( nTab, aSubTotalParam, NULL, false, bApi );
            }

            if (bSort)
            {
                pDBData->GetSortParam( aSortParam );            // Bereich kann sich geaendert haben
                Sort( nTab, aSortParam, false, false, bApi );
            }
            if (bQuery)
            {
                pDBData->GetQueryParam( aQueryParam );          // Bereich kann sich geaendert haben
                ScRange aAdvSource;
                if (pDBData->GetAdvancedQuerySource(aAdvSource))
                    Query( nTab, aQueryParam, &aAdvSource, false, bApi );
                else
                    Query( nTab, aQueryParam, NULL, false, bApi );

                //  bei nicht-inplace kann die Tabelle umgestellt worden sein
//              if ( !aQueryParam.bInplace && aQueryParam.nDestTab != nTab )
//                  SetTabNo( nTab );
            }
            if (bSubTotal)
            {
                pDBData->GetSubTotalParam( aSubTotalParam );    // Bereich kann sich geaendert haben
                aSubTotalParam.bRemoveOnly = false;
                DoSubTotals( nTab, aSubTotalParam, NULL, false, bApi );
            }

            if (bRecord)
            {
                SCTAB nDummyTab;
                SCCOL nDummyCol;
                SCROW nDummyRow;
                SCROW nNewEndRow;
                pDBData->GetArea( nDummyTab, nDummyCol,nDummyRow, nDummyCol,nNewEndRow );

                const ScRange* pOld = NULL;
                const ScRange* pNew = NULL;
                if (bQuerySize)
                {
                    ScDBData* pDest = pDoc->GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                            aQueryParam.nDestTab, sal_True );
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

            rDocShell.PostPaint(ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab),
                                PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE);
            bDone = sal_True;
        }
        else if (!bApi)     // "Keine Operationen auszufuehren"
            rDocShell.ErrorMessage(STR_MSSG_REPEATDB_0);
    }

    return bDone;
}

// -----------------------------------------------------------------

sal_Bool ScDBDocFunc::Sort( SCTAB nTab, const ScSortParam& rSortParam,
                            sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    SCTAB nSrcTab = nTab;
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();

    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rSortParam.nCol1, rSortParam.nRow1,
                                                    rSortParam.nCol2, rSortParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "Sort: keine DBData" );
        return false;
    }

    ScDBData* pDestData = NULL;
    ScRange aOldDest;
    sal_Bool bCopy = !rSortParam.bInplace;
    if ( bCopy && rSortParam.nDestCol == rSortParam.nCol1 &&
                  rSortParam.nDestRow == rSortParam.nRow1 && rSortParam.nDestTab == nTab )
        bCopy = false;
    ScSortParam aLocalParam( rSortParam );
    if ( bCopy )
    {
        aLocalParam.MoveToDest();
        if ( !ValidColRow( aLocalParam.nCol2, aLocalParam.nRow2 ) )
        {
            if (!bApi)
                rDocShell.ErrorMessage(STR_PASTE_FULL);
            return false;
        }

        nTab = rSortParam.nDestTab;
        pDestData = pDoc->GetDBAtCursor( rSortParam.nDestCol, rSortParam.nDestRow,
                                            rSortParam.nDestTab, sal_True );
        if (pDestData)
            pDestData->GetArea(aOldDest);
    }

    ScEditableTester aTester( pDoc, nTab, aLocalParam.nCol1,aLocalParam.nRow1,
                                        aLocalParam.nCol2,aLocalParam.nRow2 );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if ( aLocalParam.bIncludePattern && pDoc->HasAttrib(
                                        aLocalParam.nCol1, aLocalParam.nRow1, nTab,
                                        aLocalParam.nCol2, aLocalParam.nRow2, nTab,
                                        HASATTR_MERGED | HASATTR_OVERLAPPED ) )
    {
        //  Merge-Attribute wuerden beim Sortieren durcheinanderkommen
        if (!bApi)
            rDocShell.ErrorMessage(STR_SORT_ERR_MERGED);
        return false;
    }


    //      ausfuehren

    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    sal_Bool bRepeatQuery = false;                          // bestehenden Filter wiederholen?
    ScQueryParam aQueryParam;
    pDBData->GetQueryParam( aQueryParam );
    if ( aQueryParam.GetEntry(0).bDoQuery )
        bRepeatQuery = sal_True;

    if (bRepeatQuery && bCopy)
    {
        if ( aQueryParam.bInplace ||
                aQueryParam.nDestCol != rSortParam.nDestCol ||
                aQueryParam.nDestRow != rSortParam.nDestRow ||
                aQueryParam.nDestTab != rSortParam.nDestTab )       // Query auf selben Zielbereich?
            bRepeatQuery = false;
    }

    ScUndoSort* pUndoAction = 0;
    if ( bRecord )
    {
        //  Referenzen ausserhalb des Bereichs werden nicht veraendert !

        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        //  Zeilenhoehen immer (wegen automatischer Anpassung)
        //! auf ScBlockUndo umstellen
        pUndoDoc->InitUndo( pDoc, nTab, nTab, false, sal_True );

        /*  #i59745# Do not copy note captions to undo document. All existing
            caption objects will be repositioned while sorting which is tracked
            in drawing undo. When undo is executed, the old positions will be
            restored, and the cells with the old notes (which still refer to the
            existing captions) will be copied back into the source document. */
        pDoc->CopyToDocument( aLocalParam.nCol1, aLocalParam.nRow1, nTab,
                                aLocalParam.nCol2, aLocalParam.nRow2, nTab,
                                IDF_ALL|IDF_NOCAPTIONS, false, pUndoDoc );

        const ScRange* pR = 0;
        if (pDestData)
        {
            /*  #i59745# Do not copy note captions from destination range to
                undo document. All existing caption objects will be removed
                which is tracked in drawing undo. When undo is executed, the
                caption objects are reinserted with drawing undo, and the cells
                with the old notes (which still refer to the existing captions)
                will be copied back into the source document. */
            pDoc->CopyToDocument( aOldDest, IDF_ALL|IDF_NOCAPTIONS, false, pUndoDoc );
            pR = &aOldDest;
        }

        //  Zeilenhoehen immer (wegen automatischer Anpassung)
        //! auf ScBlockUndo umstellen
//        if (bRepeatQuery)
            pDoc->CopyToDocument( 0, aLocalParam.nRow1, nTab, MAXCOL, aLocalParam.nRow2, nTab,
                                    IDF_NONE, false, pUndoDoc );

        ScDBCollection* pUndoDB = NULL;
        ScDBCollection* pDocDB = pDoc->GetDBCollection();
        if (!pDocDB->empty())
            pUndoDB = new ScDBCollection( *pDocDB );

        pUndoAction = new ScUndoSort( &rDocShell, nTab, rSortParam, pUndoDoc, pUndoDB, pR );
        rDocShell.GetUndoManager()->AddUndoAction( pUndoAction );

        // #i59745# collect all drawing undo actions affecting cell note captions
        if( pDrawLayer )
            pDrawLayer->BeginCalcUndo();
    }

    if ( bCopy )
    {
        if (pDestData)
            pDoc->DeleteAreaTab(aOldDest, IDF_CONTENTS);            // Zielbereich vorher loeschen

        ScRange aSource( rSortParam.nCol1,rSortParam.nRow1,nSrcTab,
                            rSortParam.nCol2,rSortParam.nRow2,nSrcTab );
        ScAddress aDest( rSortParam.nDestCol, rSortParam.nDestRow, rSortParam.nDestTab );

        rDocShell.GetDocFunc().MoveBlock( aSource, aDest, false, false, false, sal_True );
    }

    // don't call ScDocument::Sort with an empty SortParam (may be empty here if bCopy is set)
    if (aLocalParam.GetSortKeyCount() && aLocalParam.maKeyState[0].bDoSort)
    {
        ScProgress aProgress(&rDocShell, ScGlobal::GetRscString(STR_PROGRESS_SORTING), 0);
        pDoc->Sort( nTab, aLocalParam, bRepeatQuery, &aProgress );
    }

    sal_Bool bSave = sal_True;
    if (bCopy)
    {
        ScSortParam aOldSortParam;
        pDBData->GetSortParam( aOldSortParam );
        if (aOldSortParam.GetSortKeyCount() &&
            aOldSortParam.maKeyState[0].bDoSort && aOldSortParam.bInplace)
        {
            bSave = false;
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
            pNewData = rDocShell.GetDBData(aDestPos, SC_DB_MAKE, SC_DBSEL_FORCE_MARK );
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
        {
            OSL_FAIL("Zielbereich nicht da");
        }
    }

    ScRange aDirtyRange( aLocalParam.nCol1, aLocalParam.nRow1, nTab,
        aLocalParam.nCol2, aLocalParam.nRow2, nTab );
    pDoc->SetDirty( aDirtyRange );

    if (bPaint)
    {
        sal_uInt16 nPaint = PAINT_GRID;
        SCCOL nStartX = aLocalParam.nCol1;
        SCROW nStartY = aLocalParam.nRow1;
        SCCOL nEndX = aLocalParam.nCol2;
        SCROW nEndY = aLocalParam.nRow2;
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
        rDocShell.PostPaint(ScRange(nStartX, nStartY, nTab, nEndX, nEndY, nTab), nPaint);
    }

    //  AdjustRowHeight( aLocalParam.nRow1, aLocalParam.nRow2, bPaint );
    rDocShell.AdjustRowHeight( aLocalParam.nRow1, aLocalParam.nRow2, nTab );

    // #i59745# set collected drawing undo actions at sorting undo action
    if( pUndoAction && pDrawLayer )
        pUndoAction->SetDrawUndoAction( pDrawLayer->GetCalcUndo() );

    aModificator.SetDocumentModified();

    return sal_True;
}

// -----------------------------------------------------------------

sal_Bool ScDBDocFunc::Query( SCTAB nTab, const ScQueryParam& rQueryParam,
                        const ScRange* pAdvSource, sal_Bool bRecord, sal_Bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rQueryParam.nCol1, rQueryParam.nRow1,
                                                    rQueryParam.nCol2, rQueryParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "Query: keine DBData" );
        return false;
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

            SCSIZE nEC = aOldQuery.GetEntryCount();
            for (SCSIZE i=0; i<nEC; i++)
                aOldQuery.GetEntry(i).bDoQuery = false;
            aOldQuery.bDuplicate = sal_True;
            Query( nTab, aOldQuery, NULL, bRecord, bApi );
        }
    }

    ScQueryParam aLocalParam( rQueryParam );        // fuer Paint / Zielbereich
    sal_Bool bCopy = !rQueryParam.bInplace;             // kopiert wird in Table::Query
    ScDBData* pDestData = NULL;                     // Bereich, in den kopiert wird
    sal_Bool bDoSize = false;                           // Zielgroesse anpassen (einf./loeschen)
    SCCOL nFormulaCols = 0;                     // nur bei bDoSize
    sal_Bool bKeepFmt = false;
    ScRange aOldDest;
    ScRange aDestTotal;
    if ( bCopy && rQueryParam.nDestCol == rQueryParam.nCol1 &&
                  rQueryParam.nDestRow == rQueryParam.nRow1 && rQueryParam.nDestTab == nTab )
        bCopy = false;
    SCTAB nDestTab = nTab;
    if ( bCopy )
    {
        aLocalParam.MoveToDest();
        nDestTab = rQueryParam.nDestTab;
        if ( !ValidColRow( aLocalParam.nCol2, aLocalParam.nRow2 ) )
        {
            if (!bApi)
                rDocShell.ErrorMessage(STR_PASTE_FULL);
            return false;
        }

        ScEditableTester aTester( pDoc, nDestTab, aLocalParam.nCol1,aLocalParam.nRow1,
                                                aLocalParam.nCol2,aLocalParam.nRow2);
        if (!aTester.IsEditable())
        {
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());
            return false;
        }

        pDestData = pDoc->GetDBAtCursor( rQueryParam.nDestCol, rQueryParam.nDestRow,
                                            rQueryParam.nDestTab, sal_True );
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
                SCCOL nTestCol = aOldDest.aEnd.Col() + 1;       // neben dem Bereich
                SCROW nTestRow = rQueryParam.nDestRow +
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
                return false;
            }
        }
    }

    //      ausfuehren

    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    sal_Bool bKeepSub = false;                          // bestehende Teilergebnisse wiederholen?
    ScSubTotalParam aSubTotalParam;
    if (rQueryParam.GetEntry(0).bDoQuery)           // nicht beim Aufheben
    {
        pDBData->GetSubTotalParam( aSubTotalParam );    // Teilergebnisse vorhanden?

        if ( aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly )
            bKeepSub = sal_True;
    }

    ScDocument* pUndoDoc = NULL;
    ScDBCollection* pUndoDB = NULL;
    const ScRange* pOld = NULL;

    if ( bRecord )
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bCopy)
        {
            pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab, false, sal_True );
            pDoc->CopyToDocument( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                    aLocalParam.nCol2, aLocalParam.nRow2, nDestTab,
                                    IDF_ALL, false, pUndoDoc );
            //  Attribute sichern, falls beim Filtern mitkopiert

            if (pDestData)
            {
                pDoc->CopyToDocument( aOldDest, IDF_ALL, false, pUndoDoc );
                pOld = &aOldDest;
            }
        }
        else
        {
            pUndoDoc->InitUndo( pDoc, nTab, nTab, false, sal_True );
            pDoc->CopyToDocument( 0, rQueryParam.nRow1, nTab, MAXCOL, rQueryParam.nRow2, nTab,
                                        IDF_NONE, false, pUndoDoc );
        }

        ScDBCollection* pDocDB = pDoc->GetDBCollection();
        if (!pDocDB->empty())
            pUndoDB = new ScDBCollection( *pDocDB );

        pDoc->BeginDrawUndo();
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
            pAttribDoc->InitUndo( pDoc, nDestTab, nDestTab, false, sal_True );
            pDoc->CopyToDocument( aAttribRange, IDF_ATTRIB, false, pAttribDoc );
        }

        if ( bDoSize )
            pDoc->FitBlock( aOldDest, aDestTotal );
        else
            pDoc->DeleteAreaTab(aOldDest, IDF_ALL);         // einfach loeschen
    }

    //  Filtern am Dokument ausfuehren
    SCSIZE nCount = pDoc->Query( nTab, rQueryParam, bKeepSub );
    if (bCopy)
    {
        aLocalParam.nRow2 = aLocalParam.nRow1 + nCount;
        if (!aLocalParam.bHasHeader && nCount > 0)
            --aLocalParam.nRow2;

        if ( bDoSize )
        {
            //  auf wirklichen Ergebnis-Bereich anpassen
            //  (das hier ist immer eine Verkleinerung)

            ScRange aNewDest( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                aLocalParam.nCol2, aLocalParam.nRow2, nDestTab );
            pDoc->FitBlock( aDestTotal, aNewDest, false );      // sal_False - nicht loeschen

            if ( nFormulaCols > 0 )
            {
                //  Formeln ausfuellen
                //! Undo (Query und Repeat) !!!

                ScRange aNewForm( aLocalParam.nCol2+1, aLocalParam.nRow1, nDestTab,
                                  aLocalParam.nCol2+nFormulaCols, aLocalParam.nRow2, nDestTab );
                ScRange aOldForm = aNewForm;
                aOldForm.aEnd.SetRow( aOldDest.aEnd.Row() );
                pDoc->FitBlock( aOldForm, aNewForm, false );

                ScMarkData aMark;
                aMark.SelectOneTable(nDestTab);
                SCROW nFStartY = aLocalParam.nRow1 + ( aLocalParam.bHasHeader ? 1 : 0 );

                sal_uLong nProgCount = nFormulaCols;
                nProgCount *= aLocalParam.nRow2 - nFStartY;
                ScProgress aProgress( pDoc->GetDocumentShell(),
                        ScGlobal::GetRscString(STR_FILL_SERIES_PROGRESS), nProgCount );

                pDoc->Fill( aLocalParam.nCol2+1, nFStartY,
                            aLocalParam.nCol2+nFormulaCols, nFStartY, &aProgress, aMark,
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
                pAttribDoc->CopyToDocument( aHdrRange, IDF_ATTRIB, false, pDoc );
            }

            //  Daten
            SCCOL nAttrEndCol = aAttribRange.aEnd.Col();
            SCROW nAttrRow = aAttribRange.aStart.Row() + ( aLocalParam.bHasHeader ? 1 : 0 );
            for (SCCOL nCol = aAttribRange.aStart.Col(); nCol<=nAttrEndCol; nCol++)
            {
                const ScPatternAttr* pSrcPattern = pAttribDoc->GetPattern(
                                                    nCol, nAttrRow, nDestTab );
                OSL_ENSURE(pSrcPattern,"Pattern ist 0");
                if (pSrcPattern)
                {
                    pDoc->ApplyPatternAreaTab( nCol, nAttrRow, nCol, aLocalParam.nRow2,
                                                    nDestTab, *pSrcPattern );
                    const ScStyleSheet* pStyle = pSrcPattern->GetStyleSheet();
                    if (pStyle)
                        pDoc->ApplyStyleAreaTab( nCol, nAttrRow, nCol, aLocalParam.nRow2,
                                                    nDestTab, *pStyle );
                }
            }

            delete pAttribDoc;
        }
    }

    //  speichern: Inplace immer, sonst je nach Einstellung
    //             alter Inplace-Filter ist ggf. schon aufgehoben

    sal_Bool bSave = rQueryParam.bInplace || rQueryParam.bDestPers;
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
                            SC_DB_MAKE, SC_DBSEL_FORCE_MARK );

        if (pNewData)
        {
            pNewData->SetArea( nDestTab, aLocalParam.nCol1, aLocalParam.nRow1,
                                            aLocalParam.nCol2, aLocalParam.nRow2 );

            //  Query-Param wird am Ziel nicht mehr eingestellt, fuehrt nur zu Verwirrung
            //  und Verwechslung mit dem Query-Param am Quellbereich (#37187#)
        }
        else
        {
            OSL_FAIL("Zielbereich nicht da");
        }
    }

    if (!bCopy)
    {
        pDoc->InvalidatePageBreaks(nTab);
        pDoc->UpdatePageBreaks( nTab );
    }

    // #i23299# Subtotal functions depend on cell's filtered states.
    ScRange aDirtyRange(0 , aLocalParam.nRow1, nDestTab, MAXCOL, aLocalParam.nRow2, nDestTab);
    pDoc->SetSubTotalCellsDirty(aDirtyRange);

    if ( bRecord )
    {
        // create undo action after executing, because of drawing layer undo
        rDocShell.GetUndoManager()->AddUndoAction(
                    new ScUndoQuery( &rDocShell, nTab, rQueryParam, pUndoDoc, pUndoDB,
                                        pOld, bDoSize, pAdvSource ) );
    }


    if (bCopy)
    {
        SCCOL nEndX = aLocalParam.nCol2;
        SCROW nEndY = aLocalParam.nRow2;
        if (pDestData)
        {
            if ( aOldDest.aEnd.Col() > nEndX )
                nEndX = aOldDest.aEnd.Col();
            if ( aOldDest.aEnd.Row() > nEndY )
                nEndY = aOldDest.aEnd.Row();
        }
        if (bDoSize)
            nEndY = MAXROW;
        rDocShell.PostPaint(
            ScRange(aLocalParam.nCol1, aLocalParam.nRow1, nDestTab, nEndX, nEndY, nDestTab),
            PAINT_GRID);
    }
    else
        rDocShell.PostPaint(
            ScRange(0, rQueryParam.nRow1, nTab, MAXCOL, MAXROW, nTab),
            PAINT_GRID | PAINT_LEFT);
    aModificator.SetDocumentModified();

    return sal_True;
}

// -----------------------------------------------------------------

sal_Bool ScDBDocFunc::DoSubTotals( SCTAB nTab, const ScSubTotalParam& rParam,
                                const ScSortParam* pForceNewSort, sal_Bool bRecord, sal_Bool bApi )
{
    //! auch fuer ScDBFunc::DoSubTotals benutzen!
    //  dann bleibt aussen:
    //  - neuen Bereich (aus DBData) markieren
    //  - SelectionChanged (?)

    sal_Bool bDo = !rParam.bRemoveOnly;                         // sal_False = nur loeschen
    sal_Bool bRet = false;

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rParam.nCol1, rParam.nRow1,
                                                rParam.nCol2, rParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "SubTotals: keine DBData" );
        return false;
    }

    ScEditableTester aTester( pDoc, nTab, 0,rParam.nRow1+1, MAXCOL,MAXROW );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if (pDoc->HasAttrib( rParam.nCol1, rParam.nRow1+1, nTab,
                         rParam.nCol2, rParam.nRow2, nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0); // nicht in zusammengefasste einfuegen
        return false;
    }

    sal_Bool bOk = true;
    if (rParam.bReplace)
        if (pDoc->TestRemoveSubTotals( nTab, rParam ))
        {
            bOk = ( MessBox( rDocShell.GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                // "StarCalc" "Daten loeschen?"
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_1 ) ).Execute()
                == RET_YES );
        }

    if (bOk)
    {
        WaitObject aWait( rDocShell.GetActiveDialogParent() );
        ScDocShellModificator aModificator( rDocShell );

        ScSubTotalParam aNewParam( rParam );        // Bereichsende wird veraendert
        ScDocument*     pUndoDoc = NULL;
        ScOutlineTable* pUndoTab = NULL;
        ScRangeName*    pUndoRange = NULL;
        ScDBCollection* pUndoDB = NULL;

        if (bRecord)                                        // alte Daten sichern
        {
            sal_Bool bOldFilter = bDo && rParam.bDoSort;

            SCTAB nTabCount = pDoc->GetTableCount();
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            ScOutlineTable* pTable = pDoc->GetOutlineTable( nTab );
            if (pTable)
            {
                pUndoTab = new ScOutlineTable( *pTable );

                // column/row state
                SCCOLROW nOutStartCol, nOutEndCol;
                SCCOLROW nOutStartRow, nOutEndRow;
                pTable->GetColArray()->GetRange( nOutStartCol, nOutEndCol );
                pTable->GetRowArray()->GetRange( nOutStartRow, nOutEndRow );

                pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, sal_True );
                pDoc->CopyToDocument( static_cast<SCCOL>(nOutStartCol), 0, nTab, static_cast<SCCOL>(nOutEndCol), MAXROW, nTab, IDF_NONE, false, pUndoDoc );
                pDoc->CopyToDocument( 0, nOutStartRow, nTab, MAXCOL, nOutEndRow, nTab, IDF_NONE, false, pUndoDoc );
            }
            else
                pUndoDoc->InitUndo( pDoc, nTab, nTab, false, bOldFilter );

            //  Datenbereich sichern - incl. Filter-Ergebnis
            pDoc->CopyToDocument( 0,rParam.nRow1+1,nTab, MAXCOL,rParam.nRow2,nTab,
                                    IDF_ALL, false, pUndoDoc );

            //  alle Formeln wegen Referenzen
            pDoc->CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1,
                                        IDF_FORMULA, false, pUndoDoc );

            //  DB- und andere Bereiche
            ScRangeName* pDocRange = pDoc->GetRangeName();
            if (!pDocRange->empty())
                pUndoRange = new ScRangeName( *pDocRange );
            ScDBCollection* pDocDB = pDoc->GetDBCollection();
            if (!pDocDB->empty())
                pUndoDB = new ScDBCollection( *pDocDB );
        }

//      pDoc->SetOutlineTable( nTab, NULL );
        ScOutlineTable* pOut = pDoc->GetOutlineTable( nTab );
        if (pOut)
            pOut->GetRowArray()->RemoveAll();       // nur Zeilen-Outlines loeschen

        if (rParam.bReplace)
            pDoc->RemoveSubTotals( nTab, aNewParam );
        sal_Bool bSuccess = sal_True;
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
                Sort( nTab, aSortParam, false, false, bApi );
            }

            pDoc->InitializeNoteCaptions(nTab);
            bSuccess = pDoc->DoSubTotals( nTab, aNewParam );
            pDoc->SetDrawPageSize(nTab);
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

        rDocShell.PostPaint(ScRange(0, 0, nTab, MAXCOL,MAXROW,nTab),
                            PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE);
        aModificator.SetDocumentModified();

        bRet = bSuccess;
    }
    return bRet;
}

//==================================================================

sal_Bool lcl_EmptyExcept( ScDocument* pDoc, const ScRange& rRange, const ScRange& rExcept )
{
    ScCellIterator aIter( pDoc, rRange );
    ScBaseCell* pCell = aIter.GetFirst();
    while (pCell)
    {
        if ( !pCell->IsBlank() )      // real content?
        {
            if ( !rExcept.In( ScAddress( aIter.GetCol(), aIter.GetRow(), aIter.GetTab() ) ) )
                return false;       // cell found
        }
        pCell = aIter.GetNext();
    }

    return sal_True;        // nothing found - empty
}

bool ScDBDocFunc::DataPilotUpdate( ScDPObject* pOldObj, const ScDPObject* pNewObj,
                                   bool bRecord, bool bApi, bool bAllowMove )
{
    ScDocShellModificator aModificator( rDocShell );
    WaitObject aWait( rDocShell.GetActiveDialogParent() );

    bool bDone = false;
    bool bUndoSelf = false;
    sal_uInt16 nErrId = 0;

    ScDocument* pOldUndoDoc = NULL;
    ScDocument* pNewUndoDoc = NULL;
    ScDPObject* pUndoDPObj = NULL;
    if ( bRecord && pOldObj )
        pUndoDPObj = new ScDPObject( *pOldObj );    // copy old settings for undo

    ScDocument* pDoc = rDocShell.GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = false;
    if ( !rDocShell.IsEditable() || pDoc->GetChangeTrack() )
    {
        //  not recorded -> disallow
        //! different error messages?

        nErrId = STR_PROTECTIONERR;
    }
    if ( pOldObj && !nErrId )
    {
        ScRange aOldOut = pOldObj->GetOutRange();
        ScEditableTester aTester( pDoc, aOldOut );
        if ( !aTester.IsEditable() )
            nErrId = aTester.GetMessageId();
    }
    if ( pNewObj && !nErrId )
    {
        //  at least one cell at the output position must be editable
        //  -> check in advance
        //  (start of output range in pNewObj is valid)

        ScRange aNewStart( pNewObj->GetOutRange().aStart );
        ScEditableTester aTester( pDoc, aNewStart );
        if ( !aTester.IsEditable() )
            nErrId = aTester.GetMessageId();
    }

    ScDPObject* pDestObj = NULL;
    if ( !nErrId )
    {
        if ( pOldObj && !pNewObj )
        {
            //  delete table

            ScRange aRange = pOldObj->GetOutRange();
            SCTAB nTab = aRange.aStart.Tab();

            if ( bRecord )
            {
                pOldUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pOldUndoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( aRange, IDF_ALL, false, pOldUndoDoc );
            }

            pDoc->DeleteAreaTab( aRange.aStart.Col(), aRange.aStart.Row(),
                                 aRange.aEnd.Col(),   aRange.aEnd.Row(),
                                 nTab, IDF_ALL );
            pDoc->RemoveFlagsTab( aRange.aStart.Col(), aRange.aStart.Row(),
                                  aRange.aEnd.Col(),   aRange.aEnd.Row(),
                                  nTab, SC_MF_AUTO );

            pDoc->GetDPCollection()->FreeTable( pOldObj );  // object is deleted here

            rDocShell.PostPaintGridAll();   //! only necessary parts
            rDocShell.PostPaint(aRange, PAINT_GRID);
            bDone = true;
        }
        else if ( pNewObj )
        {
            if ( pOldObj )
            {
                if ( bRecord )
                {
                    ScRange aRange = pOldObj->GetOutRange();
                    SCTAB nTab = aRange.aStart.Tab();
                    pOldUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                    pOldUndoDoc->InitUndo( pDoc, nTab, nTab );
                    pDoc->CopyToDocument( aRange, IDF_ALL, false, pOldUndoDoc );
                }

                if ( pNewObj == pOldObj )
                {
                    //  refresh only - no settings modified
                }
                else
                {
                    pNewObj->WriteSourceDataTo( *pOldObj );     // copy source data

                    ScDPSaveData* pData = pNewObj->GetSaveData();
                    OSL_ENSURE( pData, "no SaveData from living DPObject" );
                    if ( pData )
                        pOldObj->SetSaveData( *pData );     // copy SaveData
                }

                pDestObj = pOldObj;
                pDestObj->SetAllowMove( bAllowMove );
            }
            else
            {
                //  output range must be set at pNewObj

                pDestObj = new ScDPObject( *pNewObj );

                // #i94570# When changing the output position in the dialog, a new table is created
                // with the settings from the old table, including the name.
                // So we have to check for duplicate names here (before inserting).
                if ( pDoc->GetDPCollection()->GetByName(pDestObj->GetName()) )
                    pDestObj->SetName( String() );      // ignore the invalid name, create a new name below

                pDestObj->SetAlive(sal_True);
                if ( !pDoc->GetDPCollection()->InsertNewTable(pDestObj) )
                {
                    OSL_FAIL("cannot insert DPObject");
                    DELETEZ( pDestObj );
                }
            }
            if ( pDestObj )
            {
                pDestObj->ReloadGroupTableData();
                pDestObj->InvalidateData();             // before getting the new output area

                //  make sure the table has a name (not set by dialog)
                if (pDestObj->GetName().isEmpty())
                    pDestObj->SetName( pDoc->GetDPCollection()->CreateNewName() );

                bool bOverflow = false;
                ScRange aNewOut = pDestObj->GetNewOutputRange( bOverflow );

                //! test for overlap with other data pilot tables
                if( pOldObj )
                {
                    const ScSheetSourceDesc* pSheetDesc = pOldObj->GetSheetDesc();
                    if( pSheetDesc && pSheetDesc->GetSourceRange().Intersects( aNewOut ) )
                    {
                        ScRange aOldRange = pOldObj->GetOutRange();
                        SCsROW nDiff = aOldRange.aStart.Row()-aNewOut.aStart.Row();
                        aNewOut.aStart.SetRow( aOldRange.aStart.Row() );
                        aNewOut.aEnd.SetRow( aNewOut.aEnd.Row()+nDiff );
                        if( !ValidRow( aNewOut.aStart.Row() ) || !ValidRow( aNewOut.aEnd.Row() ) )
                            bOverflow = sal_True;
                    }
                }

                if ( bOverflow )
                {
                    //  like with STR_PROTECTIONERR, use undo to reverse everything
                    OSL_ENSURE( bRecord, "DataPilotUpdate: can't undo" );
                    bUndoSelf = true;
                    nErrId = STR_PIVOT_ERROR;
                }
                else
                {
                    ScEditableTester aTester( pDoc, aNewOut );
                    if ( !aTester.IsEditable() )
                    {
                        //  destination area isn't editable
                        //! reverse everything done so far, don't proceed

                        //  quick solution: proceed to end, use undo action
                        //  to reverse everything:
                        OSL_ENSURE( bRecord, "DataPilotUpdate: can't undo" );
                        bUndoSelf = sal_True;
                        nErrId = aTester.GetMessageId();
                    }
                }

                //  test if new output area is empty except for old area
                if ( !bApi )
                {
                    bool bEmpty;
                    if ( pOldObj )  // OutRange of pOldObj (pDestObj) is still old area
                        bEmpty = lcl_EmptyExcept( pDoc, aNewOut, pOldObj->GetOutRange() );
                    else
                        bEmpty = pDoc->IsBlockEmpty( aNewOut.aStart.Tab(),
                                            aNewOut.aStart.Col(), aNewOut.aStart.Row(),
                                            aNewOut.aEnd.Col(), aNewOut.aEnd.Row() );

                    if ( !bEmpty )
                    {
                        QueryBox aBox( rDocShell.GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                                         ScGlobal::GetRscString(STR_PIVOT_NOTEMPTY) );
                        if (aBox.Execute() == RET_NO)
                        {
                            //! like above (not editable), use undo to reverse everything
                            OSL_ENSURE( bRecord, "DataPilotUpdate: can't undo" );
                            bUndoSelf = true;
                        }
                    }
                }

                if ( bRecord )
                {
                    SCTAB nTab = aNewOut.aStart.Tab();
                    pNewUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                    pNewUndoDoc->InitUndo( pDoc, nTab, nTab );
                    pDoc->CopyToDocument( aNewOut, IDF_ALL, false, pNewUndoDoc );
                }

                pDestObj->Output( aNewOut.aStart );

                rDocShell.PostPaintGridAll();           //! only necessary parts
                bDone = true;
            }
        }
        // else nothing (no old, no new)
    }

    if ( bRecord && bDone )
    {
        SfxUndoAction* pAction = new ScUndoDataPilot( &rDocShell,
                                    pOldUndoDoc, pNewUndoDoc, pUndoDPObj, pDestObj, bAllowMove );
        pOldUndoDoc = NULL;
        pNewUndoDoc = NULL;     // pointers are used in undo action
        // pUndoDPObj is copied

        if (bUndoSelf)
        {
            //  use undo action to restore original state
            //! prevent setting the document modified? (ScDocShellModificator)

            pAction->Undo();
            delete pAction;
            bDone = false;
        }
        else
            rDocShell.GetUndoManager()->AddUndoAction( pAction );
    }

    delete pOldUndoDoc;     // if not used for undo
    delete pNewUndoDoc;
    delete pUndoDPObj;

    if (bDone)
    {
        // notify API objects
        if (pDestObj)
            pDoc->BroadcastUno( ScDataPilotModifiedHint( pDestObj->GetName() ) );
        aModificator.SetDocumentModified();
    }

    if ( nErrId && !bApi )
        rDocShell.ErrorMessage( nErrId );

    return bDone;
}

sal_uLong ScDBDocFunc::RefreshPivotTables(ScDPObject* pDPObj, bool bApi)
{
    ScDPCollection* pDPs = rDocShell.GetDocument()->GetDPCollection();
    if (!pDPs)
        return 0;

    std::set<ScDPObject*> aRefs;
    sal_uLong nErrId = pDPs->ReloadCache(pDPObj, aRefs);
    if (nErrId)
        return nErrId;

    std::set<ScDPObject*>::iterator it = aRefs.begin(), itEnd = aRefs.end();
    for (; it != itEnd; ++it)
    {
        ScDPObject* pObj = *it;
        if (!pObj->SyncAllDimensionMembers())
            continue;

        // This action is intentionally not undoable since it modifies cache.
        DataPilotUpdate(pObj, pObj, false, bApi);
    }

    return 0;
}

void ScDBDocFunc::RefreshPivotTableGroups(ScDPObject* pDPObj)
{
    if (!pDPObj)
        return;

    ScDPCollection* pDPs = rDocShell.GetDocument()->GetDPCollection();
    if (!pDPs)
        return;

    ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    if (!pSaveData)
        return;

    std::set<ScDPObject*> aRefs;
    if (!pDPs->ReloadGroupsInCache(pDPObj, aRefs))
        return;

    // We allow pDimData being NULL.
    const ScDPDimensionSaveData* pDimData = pSaveData->GetExistingDimensionData();
    std::set<ScDPObject*>::iterator it = aRefs.begin(), itEnd = aRefs.end();
    for (; it != itEnd; ++it)
    {
        ScDPObject* pObj = *it;
        if (pObj != pDPObj)
        {
            pSaveData = pObj->GetSaveData();
            if (pSaveData)
                pSaveData->SetDimensionData(pDimData);
        }

        // This action is intentionally not undoable since it modifies cache.
        DataPilotUpdate(pObj, pObj, false, false);
    }
}

//==================================================================
//
//      database import

void ScDBDocFunc::UpdateImport( const String& rTarget, const svx::ODataAccessDescriptor& rDescriptor )
{
    // rTarget is the name of a database range

    ScDocument* pDoc = rDocShell.GetDocument();
    ScDBCollection& rDBColl = *pDoc->GetDBCollection();
    const ScDBData* pData = rDBColl.getNamedDBs().findByName(rTarget);
    if (!pData)
    {
        InfoBox aInfoBox(rDocShell.GetActiveDialogParent(),
                    ScGlobal::GetRscString( STR_TARGETNOTFOUND ) );
        aInfoBox.Execute();
        return;
    }

    SCTAB nTab;
    SCCOL nDummyCol;
    SCROW nDummyRow;
    pData->GetArea( nTab, nDummyCol,nDummyRow,nDummyCol,nDummyRow );

    ScImportParam aImportParam;
    pData->GetImportParam( aImportParam );

    rtl::OUString sDBName;
    rtl::OUString sDBTable;
    sal_Int32 nCommandType = 0;
    rDescriptor[svx::daDataSource]  >>= sDBName;
    rDescriptor[svx::daCommand]     >>= sDBTable;
    rDescriptor[svx::daCommandType] >>= nCommandType;

    aImportParam.aDBName    = sDBName;
    aImportParam.bSql       = ( nCommandType == sdb::CommandType::COMMAND );
    aImportParam.aStatement = sDBTable;
    aImportParam.bNative    = false;
    aImportParam.nType      = static_cast<sal_uInt8>( ( nCommandType == sdb::CommandType::QUERY ) ? ScDbQuery : ScDbTable );
    aImportParam.bImport    = true;

    bool bContinue = DoImport( nTab, aImportParam, &rDescriptor, true );

    //  DB-Operationen wiederholen

    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();
    if (pViewSh)
    {
        ScRange aRange;
        pData->GetArea(aRange);
        pViewSh->MarkRange(aRange);         // selektieren

        if ( bContinue )        // Fehler beim Import -> Abbruch
        {
            //  interne Operationen, wenn welche gespeichert

            if ( pData->HasQueryParam() || pData->HasSortParam() || pData->HasSubTotalParam() )
                pViewSh->RepeatDB();

            //  Pivottabellen die den Bereich als Quelldaten haben

            rDocShell.RefreshPivotTables(aRange);
        }
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
