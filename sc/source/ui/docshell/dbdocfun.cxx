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
#include "dociter.hxx"
#include "editable.hxx"
#include "attrib.hxx"
#include "drwlayer.hxx"
#include "dpshttab.hxx"
#include "hints.hxx"
#include "queryentry.hxx"
#include "markdata.hxx"
#include "progress.hxx"
#include <undosort.hxx>
#include <inputopt.hxx>

#include <set>
#include <memory>

using namespace ::com::sun::star;

bool ScDBDocFunc::AddDBRange( const OUString& rName, const ScRange& rRange, bool /* bApi */ )
{

    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = rDoc.GetDBCollection();
    bool bUndo (rDoc.IsUndoEnabled());

    ScDBCollection* pUndoColl = nullptr;
    if (bUndo)
        pUndoColl = new ScDBCollection( *pDocColl );

    ScDBData* pNew = new ScDBData( rName, rRange.aStart.Tab(),
                                    rRange.aStart.Col(), rRange.aStart.Row(),
                                    rRange.aEnd.Col(), rRange.aEnd.Row() );

    // #i55926# While loading XML, formula cells only have a single string token,
    // so CompileDBFormula would never find any name (index) tokens, and would
    // unnecessarily loop through all cells.
    bool bCompile = !rDoc.IsImportingXML();
    bool bOk;
    if ( bCompile )
        rDoc.PreprocessDBDataUpdate();
    if ( rName == STR_DB_LOCAL_NONAME )
    {
        rDoc.SetAnonymousDBData(rRange.aStart.Tab() , pNew);
        bOk = true;
    }
    else
    {
        bOk = pDocColl->getNamedDBs().insert(pNew);
    }
    if ( bCompile )
        rDoc.CompileHybridFormula();

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
    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
    return true;
}

bool ScDBDocFunc::DeleteDBRange(const OUString& rName)
{
    bool bDone = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = rDoc.GetDBCollection();
    bool bUndo = rDoc.IsUndoEnabled();

    ScDBCollection::NamedDBs& rDBs = pDocColl->getNamedDBs();
    auto const iter = rDBs.findByUpperName2(ScGlobal::pCharClass->uppercase(rName));
    if (iter != rDBs.end())
    {
        ScDocShellModificator aModificator( rDocShell );

        ScDBCollection* pUndoColl = nullptr;
        if (bUndo)
            pUndoColl = new ScDBCollection( *pDocColl );

        rDoc.PreprocessDBDataUpdate();
        rDBs.erase(iter);
        rDoc.CompileHybridFormula();

        if (bUndo)
        {
            ScDBCollection* pRedoColl = new ScDBCollection( *pDocColl );
            rDocShell.GetUndoManager()->AddUndoAction(
                            new ScUndoDBData( &rDocShell, pUndoColl, pRedoColl ) );
        }

        aModificator.SetDocumentModified();
        SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
        bDone = true;
    }

    return bDone;
}

bool ScDBDocFunc::RenameDBRange( const OUString& rOld, const OUString& rNew )
{
    bool bDone = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = rDoc.GetDBCollection();
    bool bUndo = rDoc.IsUndoEnabled();
    ScDBCollection::NamedDBs& rDBs = pDocColl->getNamedDBs();
    auto const iterOld = rDBs.findByUpperName2(ScGlobal::pCharClass->uppercase(rOld));
    const ScDBData* pNew = rDBs.findByUpperName(ScGlobal::pCharClass->uppercase(rNew));
    if (iterOld != rDBs.end() && !pNew)
    {
        ScDocShellModificator aModificator( rDocShell );

        ScDBData* pNewData = new ScDBData(rNew, **iterOld);

        ScDBCollection* pUndoColl = new ScDBCollection( *pDocColl );

        rDoc.PreprocessDBDataUpdate();
        rDBs.erase(iterOld);
        bool bInserted = rDBs.insert(pNewData);
        if (!bInserted)                             // Fehler -> alten Zustand wiederherstellen
        {
            delete pNewData;
            rDoc.SetDBCollection(pUndoColl);       // gehoert dann dem Dokument
        }

        rDoc.CompileHybridFormula();

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
            SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
            bDone = true;
        }
    }

    return bDone;
}

bool ScDBDocFunc::ModifyDBData( const ScDBData& rNewData )
{
    bool bDone = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = rDoc.GetDBCollection();
    bool bUndo = rDoc.IsUndoEnabled();

    ScDBData* pData = nullptr;
    if (rNewData.GetName() == STR_DB_LOCAL_NONAME)
    {
        ScRange aRange;
        rNewData.GetArea(aRange);
        SCTAB nTab = aRange.aStart.Tab();
        pData = rDoc.GetAnonymousDBData(nTab);
    }
    else
        pData = pDocColl->getNamedDBs().findByUpperName(rNewData.GetUpperName());

    if (pData)
    {
        ScDocShellModificator aModificator( rDocShell );
        ScRange aOldRange, aNewRange;
        pData->GetArea(aOldRange);
        rNewData.GetArea(aNewRange);
        bool bAreaChanged = ( aOldRange != aNewRange );     // dann muss neu compiliert werden

        ScDBCollection* pUndoColl = nullptr;
        if (bUndo)
            pUndoColl = new ScDBCollection( *pDocColl );

        *pData = rNewData;
        if (bAreaChanged)
            rDoc.CompileDBFormula();

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

void ScDBDocFunc::ModifyAllDBData( const ScDBCollection& rNewColl, const std::vector<ScRange>& rDelAreaList )
{
    ScDocShellModificator aModificator(rDocShell);
    ScDocument& rDoc = rDocShell.GetDocument();
    ScDBCollection* pOldColl = rDoc.GetDBCollection();
    ScDBCollection* pUndoColl = nullptr;
    bool bRecord = rDoc.IsUndoEnabled();

    std::vector<ScRange>::const_iterator iter;
    for (iter = rDelAreaList.begin(); iter != rDelAreaList.end(); ++iter)
    {
        // unregistering target in SBA no longer necessary
        const ScAddress& rStart = iter->aStart;
        const ScAddress& rEnd   = iter->aEnd;
        rDocShell.DBAreaDeleted(
            rStart.Tab(), rStart.Col(), rStart.Row(), rEnd.Col(), rEnd.Row());
    }

    if (bRecord)
        pUndoColl = new ScDBCollection( *pOldColl );

    //  register target in SBA no longer necessary

    rDoc.PreprocessDBDataUpdate();
    rDoc.SetDBCollection( new ScDBCollection( rNewColl ) );
    rDoc.CompileHybridFormula();
    pOldColl = nullptr;
    rDocShell.PostPaint(ScRange(0, 0, 0, MAXCOL, MAXROW, MAXTAB), PAINT_GRID);
    aModificator.SetDocumentModified();
    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );

    if (bRecord)
    {
        ScDBCollection* pRedoColl = new ScDBCollection(rNewColl);
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDBData(&rDocShell, pUndoColl, pRedoColl));
    }
}

bool ScDBDocFunc::RepeatDB( const OUString& rDBName, bool bRecord, bool bApi, bool bIsUnnamed, SCTAB aTab )
{
    //! auch fuer ScDBFunc::RepeatDB benutzen!

    bool bDone = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    ScDBData* pDBData = nullptr;
    if (bIsUnnamed)
    {
        pDBData = rDoc.GetAnonymousDBData( aTab );
    }
    else
    {
        ScDBCollection* pColl = rDoc.GetDBCollection();
        if (pColl)
            pDBData = pColl->getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(rDBName));
    }

    if ( pDBData )
    {
        ScQueryParam aQueryParam;
        pDBData->GetQueryParam( aQueryParam );
        bool bQuery = aQueryParam.GetEntry(0).bDoQuery;

        ScSortParam aSortParam;
        pDBData->GetSortParam( aSortParam );
        bool bSort = aSortParam.maKeyState[0].bDoSort;

        ScSubTotalParam aSubTotalParam;
        pDBData->GetSubTotalParam( aSubTotalParam );
        bool bSubTotal = aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly;

        if ( bQuery || bSort || bSubTotal )
        {
            bool bQuerySize = false;
            ScRange aOldQuery;
            ScRange aNewQuery;
            if (bQuery && !aQueryParam.bInplace)
            {
                ScDBData* pDest = rDoc.GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                        aQueryParam.nDestTab, ScDBDataPortion::TOP_LEFT );
                if (pDest && pDest->IsDoSize())
                {
                    pDest->GetArea( aOldQuery );
                    bQuerySize = true;
                }
            }

            SCTAB nTab;
            SCCOL nStartCol;
            SCROW nStartRow;
            SCCOL nEndCol;
            SCROW nEndRow;
            pDBData->GetArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );

            //!     Undo nur benoetigte Daten ?

            ScDocument* pUndoDoc = nullptr;
            ScOutlineTable* pUndoTab = nullptr;
            ScRangeName* pUndoRange = nullptr;
            ScDBCollection* pUndoDB = nullptr;

            if (bRecord)
            {
                SCTAB nTabCount = rDoc.GetTableCount();
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );
                if (pTable)
                {
                    pUndoTab = new ScOutlineTable( *pTable );

                    // column/row state
                    SCCOLROW nOutStartCol, nOutEndCol;
                    SCCOLROW nOutStartRow, nOutEndRow;
                    pTable->GetColArray().GetRange( nOutStartCol, nOutEndCol );
                    pTable->GetRowArray().GetRange( nOutStartRow, nOutEndRow );

                    pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
                    rDoc.CopyToDocument( static_cast<SCCOL>(nOutStartCol), 0,
                            nTab, static_cast<SCCOL>(nOutEndCol), MAXROW, nTab,
                            InsertDeleteFlags::NONE, false, pUndoDoc );
                    rDoc.CopyToDocument( 0, static_cast<SCROW>(nOutStartRow),
                            nTab, MAXCOL, static_cast<SCROW>(nOutEndRow), nTab,
                            InsertDeleteFlags::NONE, false, pUndoDoc );
                }
                else
                    pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, true );

                //  Datenbereich sichern - incl. Filter-Ergebnis
                rDoc.CopyToDocument( 0,nStartRow,nTab, MAXCOL,nEndRow,nTab, InsertDeleteFlags::ALL, false, pUndoDoc );

                //  alle Formeln wegen Referenzen
                rDoc.CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1, InsertDeleteFlags::FORMULA, false, pUndoDoc );

                //  DB- und andere Bereiche
                ScRangeName* pDocRange = rDoc.GetRangeName();
                if (!pDocRange->empty())
                    pUndoRange = new ScRangeName( *pDocRange );
                ScDBCollection* pDocDB = rDoc.GetDBCollection();
                if (!pDocDB->empty())
                    pUndoDB = new ScDBCollection( *pDocDB );
            }

            if (bSort && bSubTotal)
            {
                //  Sortieren ohne SubTotals

                aSubTotalParam.bRemoveOnly = true;      // wird unten wieder zurueckgesetzt
                DoSubTotals( nTab, aSubTotalParam, nullptr, false, bApi );
            }

            if (bSort)
            {
                pDBData->GetSortParam( aSortParam );            // Bereich kann sich geaendert haben
                (void)Sort( nTab, aSortParam, false, false, bApi );
            }
            if (bQuery)
            {
                pDBData->GetQueryParam( aQueryParam );          // Bereich kann sich geaendert haben
                ScRange aAdvSource;
                if (pDBData->GetAdvancedQuerySource(aAdvSource))
                    Query( nTab, aQueryParam, &aAdvSource, false, bApi );
                else
                    Query( nTab, aQueryParam, nullptr, false, bApi );

                //  bei nicht-inplace kann die Tabelle umgestellt worden sein
//              if ( !aQueryParam.bInplace && aQueryParam.nDestTab != nTab )
//                  SetTabNo( nTab );
            }
            if (bSubTotal)
            {
                pDBData->GetSubTotalParam( aSubTotalParam );    // Bereich kann sich geaendert haben
                aSubTotalParam.bRemoveOnly = false;
                DoSubTotals( nTab, aSubTotalParam, nullptr, false, bApi );
            }

            if (bRecord)
            {
                SCTAB nDummyTab;
                SCCOL nDummyCol;
                SCROW nDummyRow;
                SCROW nNewEndRow;
                pDBData->GetArea( nDummyTab, nDummyCol,nDummyRow, nDummyCol,nNewEndRow );

                const ScRange* pOld = nullptr;
                const ScRange* pNew = nullptr;
                if (bQuerySize)
                {
                    ScDBData* pDest = rDoc.GetDBAtCursor( aQueryParam.nDestCol, aQueryParam.nDestRow,
                                                            aQueryParam.nDestTab, ScDBDataPortion::TOP_LEFT );
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
            bDone = true;
        }
        else if (!bApi)     // "Keine Operationen auszufuehren"
            rDocShell.ErrorMessage(STR_MSSG_REPEATDB_0);
    }

    return bDone;
}

bool ScDBDocFunc::Sort( SCTAB nTab, const ScSortParam& rSortParam,
                            bool bRecord, bool bPaint, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScDBData* pDBData = rDoc.GetDBAtArea( nTab, rSortParam.nCol1, rSortParam.nRow1,
                                                    rSortParam.nCol2, rSortParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "Sort: keine DBData" );
        return false;
    }

    bool bCopy = !rSortParam.bInplace;
    if ( bCopy && rSortParam.nDestCol == rSortParam.nCol1 &&
                  rSortParam.nDestRow == rSortParam.nRow1 && rSortParam.nDestTab == nTab )
        bCopy = false;

    ScSortParam aLocalParam( rSortParam );
    if ( bCopy )
    {
        // Copy the data range to the destination then move the sort range to it.
        ScRange aSrcRange(rSortParam.nCol1, rSortParam.nRow1, nTab, rSortParam.nCol2, rSortParam.nRow2, nTab);
        ScAddress aDestPos(rSortParam.nDestCol,rSortParam.nDestRow,rSortParam.nDestTab);

        ScDocFunc& rDocFunc = rDocShell.GetDocFunc();
        bool bRet = rDocFunc.MoveBlock(aSrcRange, aDestPos, false, bRecord, bPaint, bApi);

        if (!bRet)
            return false;

        aLocalParam.MoveToDest();
        nTab = aLocalParam.nDestTab;
    }

    ScEditableTester aTester( &rDoc, nTab, aLocalParam.nCol1,aLocalParam.nRow1,
                                        aLocalParam.nCol2,aLocalParam.nRow2 );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    // Adjust aLocalParam cols/rows to used data area. Keep sticky top row or
    // column (depending on direction) in any case, not just if it has headers,
    // so empty leading cells will be sorted to the end.
    bool bShrunk = false;
    rDoc.ShrinkToUsedDataArea( bShrunk, nTab, aLocalParam.nCol1, aLocalParam.nRow1,
            aLocalParam.nCol2, aLocalParam.nRow2, false, aLocalParam.bByRow, !aLocalParam.bByRow);

    SCROW nStartRow = aLocalParam.nRow1;
    if (aLocalParam.bByRow && aLocalParam.bHasHeader && nStartRow < aLocalParam.nRow2)
        ++nStartRow;

    if ( aLocalParam.bIncludePattern && rDoc.HasAttrib(
                                        aLocalParam.nCol1, nStartRow        , nTab,
                                        aLocalParam.nCol2, aLocalParam.nRow2, nTab,
                                        HASATTR_MERGED | HASATTR_OVERLAPPED ) )
    {
        //  Merge-Attribute wuerden beim Sortieren durcheinanderkommen
        if (!bApi)
            rDocShell.ErrorMessage(STR_SORT_ERR_MERGED);
        return false;
    }

    //      ausfuehren

    WaitObject aWait( ScDocShell::GetActiveDialogParent() );

    // Calculate the script types for all cells in the sort range beforehand.
    // This will speed up the row height adjustment that takes place after the
    // sort.
    rDoc.UpdateScriptTypes(
        ScAddress(aLocalParam.nCol1,nStartRow,nTab),
        aLocalParam.nCol2-aLocalParam.nCol1+1,
        aLocalParam.nRow2-nStartRow+1);

    // No point adjusting row heights after the sort when all rows have the same height.
    bool bUniformRowHeight =
        rDoc.HasUniformRowHeight(nTab, nStartRow, aLocalParam.nRow2);

    bool bRepeatQuery = false;                          // bestehenden Filter wiederholen?
    ScQueryParam aQueryParam;
    pDBData->GetQueryParam( aQueryParam );
    if ( aQueryParam.GetEntry(0).bDoQuery )
        bRepeatQuery = true;

    sc::ReorderParam aUndoParam;

    // don't call ScDocument::Sort with an empty SortParam (may be empty here if bCopy is set)
    if (aLocalParam.GetSortKeyCount() && aLocalParam.maKeyState[0].bDoSort)
    {
        ScInputOptions aInputOption = SC_MOD()->GetInputOptions();
        bool bUpdateRefs = aInputOption.GetSortRefUpdate();
        ScProgress aProgress(&rDocShell, ScGlobal::GetRscString(STR_PROGRESS_SORTING), 0);
        rDoc.Sort(nTab, aLocalParam, bRepeatQuery, bUpdateRefs, &aProgress, &aUndoParam);
    }

    if (bRecord)
    {
        // Set up an undo object.
        sc::UndoSort* pUndoAction = new sc::UndoSort(&rDocShell, aUndoParam);
        rDocShell.GetUndoManager()->AddUndoAction(pUndoAction);
    }

    pDBData->SetSortParam(rSortParam);
    // Remember additional settings on anonymous database ranges.
    if (pDBData == rDoc.GetAnonymousDBData( nTab) || rDoc.GetDBCollection()->getAnonDBs().has( pDBData))
        pDBData->UpdateFromSortParam( rSortParam);

    if (nStartRow <= aLocalParam.nRow2)
    {
        ScRange aDirtyRange(
                aLocalParam.nCol1, nStartRow, nTab,
                aLocalParam.nCol2, aLocalParam.nRow2, nTab);
        rDoc.SetDirty( aDirtyRange, true );
    }

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
        rDocShell.PostPaint(ScRange(nStartX, nStartY, nTab, nEndX, nEndY, nTab), nPaint);
    }

    if (!bUniformRowHeight && nStartRow <= aLocalParam.nRow2)
        rDocShell.AdjustRowHeight(nStartRow, aLocalParam.nRow2, nTab);

    aModificator.SetDocumentModified();

    return true;
}

bool ScDBDocFunc::Query( SCTAB nTab, const ScQueryParam& rQueryParam,
                        const ScRange* pAdvSource, bool bRecord, bool bApi )
{
    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    ScDBData* pDBData = rDoc.GetDBAtArea( nTab, rQueryParam.nCol1, rQueryParam.nRow1,
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
            aOldQuery.bDuplicate = true;
            Query( nTab, aOldQuery, nullptr, bRecord, bApi );
        }
    }

    ScQueryParam aLocalParam( rQueryParam );        // fuer Paint / Zielbereich
    bool bCopy = !rQueryParam.bInplace;             // kopiert wird in Table::Query
    ScDBData* pDestData = nullptr;                     // Bereich, in den kopiert wird
    bool bDoSize = false;                           // Zielgroesse anpassen (einf./loeschen)
    SCCOL nFormulaCols = 0;                     // nur bei bDoSize
    bool bKeepFmt = false;
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

        ScEditableTester aTester( &rDoc, nDestTab, aLocalParam.nCol1,aLocalParam.nRow1,
                                                aLocalParam.nCol2,aLocalParam.nRow2);
        if (!aTester.IsEditable())
        {
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());
            return false;
        }

        pDestData = rDoc.GetDBAtCursor( rQueryParam.nDestCol, rQueryParam.nDestRow,
                                            rQueryParam.nDestTab, ScDBDataPortion::TOP_LEFT );
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
                        rDoc.GetCellType(ScAddress( nTestCol, nTestRow, nTab )) == CELLTYPE_FORMULA )
                    ++nTestCol, ++nFormulaCols;
            }

            bKeepFmt = pDestData->IsKeepFmt();
            if ( bDoSize && !rDoc.CanFitBlock( aOldDest, aDestTotal ) )
            {
                if (!bApi)
                    rDocShell.ErrorMessage(STR_MSSG_DOSUBTOTALS_2);     // kann keine Zeilen einfuegen
                return false;
            }
        }
    }

    //      ausfuehren

    WaitObject aWait( ScDocShell::GetActiveDialogParent() );

    bool bKeepSub = false;                          // bestehende Teilergebnisse wiederholen?
    ScSubTotalParam aSubTotalParam;
    if (rQueryParam.GetEntry(0).bDoQuery)           // nicht beim Aufheben
    {
        pDBData->GetSubTotalParam( aSubTotalParam );    // Teilergebnisse vorhanden?

        if ( aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly )
            bKeepSub = true;
    }

    ScDocument* pUndoDoc = nullptr;
    ScDBCollection* pUndoDB = nullptr;
    const ScRange* pOld = nullptr;

    if ( bRecord )
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        if (bCopy)
        {
            pUndoDoc->InitUndo( &rDoc, nDestTab, nDestTab, false, true );
            rDoc.CopyToDocument( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                    aLocalParam.nCol2, aLocalParam.nRow2, nDestTab,
                                    InsertDeleteFlags::ALL, false, pUndoDoc );
            //  Attribute sichern, falls beim Filtern mitkopiert

            if (pDestData)
            {
                rDoc.CopyToDocument( aOldDest, InsertDeleteFlags::ALL, false, pUndoDoc );
                pOld = &aOldDest;
            }
        }
        else
        {
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, true );
            rDoc.CopyToDocument( 0, rQueryParam.nRow1, nTab, MAXCOL, rQueryParam.nRow2, nTab,
                                        InsertDeleteFlags::NONE, false, pUndoDoc );
        }

        ScDBCollection* pDocDB = rDoc.GetDBCollection();
        if (!pDocDB->empty())
            pUndoDB = new ScDBCollection( *pDocDB );

        rDoc.BeginDrawUndo();
    }

    ScDocument* pAttribDoc = nullptr;
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
            pAttribDoc->InitUndo( &rDoc, nDestTab, nDestTab, false, true );
            rDoc.CopyToDocument( aAttribRange, InsertDeleteFlags::ATTRIB, false, pAttribDoc );
        }

        if ( bDoSize )
            rDoc.FitBlock( aOldDest, aDestTotal );
        else
            rDoc.DeleteAreaTab(aOldDest, InsertDeleteFlags::ALL);         // einfach loeschen
    }

    //  Filtern am Dokument ausfuehren
    SCSIZE nCount = rDoc.Query( nTab, rQueryParam, bKeepSub );
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
            rDoc.FitBlock( aDestTotal, aNewDest, false );      // sal_False - nicht loeschen

            if ( nFormulaCols > 0 )
            {
                //  Formeln ausfuellen
                //! Undo (Query und Repeat) !!!

                ScRange aNewForm( aLocalParam.nCol2+1, aLocalParam.nRow1, nDestTab,
                                  aLocalParam.nCol2+nFormulaCols, aLocalParam.nRow2, nDestTab );
                ScRange aOldForm = aNewForm;
                aOldForm.aEnd.SetRow( aOldDest.aEnd.Row() );
                rDoc.FitBlock( aOldForm, aNewForm, false );

                ScMarkData aMark;
                aMark.SelectOneTable(nDestTab);
                SCROW nFStartY = aLocalParam.nRow1 + ( aLocalParam.bHasHeader ? 1 : 0 );

                sal_uLong nProgCount = nFormulaCols;
                nProgCount *= aLocalParam.nRow2 - nFStartY;
                ScProgress aProgress( rDoc.GetDocumentShell(),
                        ScGlobal::GetRscString(STR_FILL_SERIES_PROGRESS), nProgCount );

                rDoc.Fill( aLocalParam.nCol2+1, nFStartY,
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
                pAttribDoc->CopyToDocument( aHdrRange, InsertDeleteFlags::ATTRIB, false, &rDoc );
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
                    rDoc.ApplyPatternAreaTab( nCol, nAttrRow, nCol, aLocalParam.nRow2,
                                                    nDestTab, *pSrcPattern );
                    const ScStyleSheet* pStyle = pSrcPattern->GetStyleSheet();
                    if (pStyle)
                        rDoc.ApplyStyleAreaTab( nCol, nAttrRow, nCol, aLocalParam.nRow2,
                                                    nDestTab, *pStyle );
                }
            }

            delete pAttribDoc;
        }
    }

    //  speichern: Inplace immer, sonst je nach Einstellung
    //             alter Inplace-Filter ist ggf. schon aufgehoben

    bool bSave = rQueryParam.bInplace || rQueryParam.bDestPers;
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
        rDoc.InvalidatePageBreaks(nTab);
        rDoc.UpdatePageBreaks( nTab );
    }

    // #i23299# Subtotal functions depend on cell's filtered states.
    ScRange aDirtyRange(0 , aLocalParam.nRow1, nDestTab, MAXCOL, aLocalParam.nRow2, nDestTab);
    rDoc.SetSubTotalCellsDirty(aDirtyRange);

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

    return true;
}

bool ScDBDocFunc::DoSubTotals( SCTAB nTab, const ScSubTotalParam& rParam,
                                const ScSortParam* pForceNewSort, bool bRecord, bool bApi )
{
    //! auch fuer ScDBFunc::DoSubTotals benutzen!
    //  dann bleibt aussen:
    //  - neuen Bereich (aus DBData) markieren
    //  - SelectionChanged (?)

    bool bDo = !rParam.bRemoveOnly;                         // sal_False = nur loeschen
    bool bRet = false;

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    ScDBData* pDBData = rDoc.GetDBAtArea( nTab, rParam.nCol1, rParam.nRow1,
                                                rParam.nCol2, rParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "SubTotals: keine DBData" );
        return false;
    }

    ScEditableTester aTester( &rDoc, nTab, 0,rParam.nRow1+1, MAXCOL,MAXROW );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if (rDoc.HasAttrib( rParam.nCol1, rParam.nRow1+1, nTab,
                         rParam.nCol2, rParam.nRow2, nTab, HASATTR_MERGED | HASATTR_OVERLAPPED ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0); // nicht in zusammengefasste einfuegen
        return false;
    }

    bool bOk = true;
    if (rParam.bReplace)
        if (rDoc.TestRemoveSubTotals( nTab, rParam ))
        {
            bOk = ( ScopedVclPtr<MessBox>::Create( ScDocShell::GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                // "StarCalc" "Daten loeschen?"
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
                ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_1 ) )->Execute()
                == RET_YES );
        }

    if (bOk)
    {
        WaitObject aWait( ScDocShell::GetActiveDialogParent() );
        ScDocShellModificator aModificator( rDocShell );

        ScSubTotalParam aNewParam( rParam );        // Bereichsende wird veraendert
        ScDocument*     pUndoDoc = nullptr;
        ScOutlineTable* pUndoTab = nullptr;
        ScRangeName*    pUndoRange = nullptr;
        ScDBCollection* pUndoDB = nullptr;

        if (bRecord)                                        // alte Daten sichern
        {
            bool bOldFilter = bDo && rParam.bDoSort;

            SCTAB nTabCount = rDoc.GetTableCount();
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );
            if (pTable)
            {
                pUndoTab = new ScOutlineTable( *pTable );

                // column/row state
                SCCOLROW nOutStartCol, nOutEndCol;
                SCCOLROW nOutStartRow, nOutEndRow;
                pTable->GetColArray().GetRange( nOutStartCol, nOutEndCol );
                pTable->GetRowArray().GetRange( nOutStartRow, nOutEndRow );

                pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
                rDoc.CopyToDocument( static_cast<SCCOL>(nOutStartCol), 0, nTab, static_cast<SCCOL>(nOutEndCol), MAXROW, nTab, InsertDeleteFlags::NONE, false, pUndoDoc );
                rDoc.CopyToDocument( 0, nOutStartRow, nTab, MAXCOL, nOutEndRow, nTab, InsertDeleteFlags::NONE, false, pUndoDoc );
            }
            else
                pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, bOldFilter );

            //  Datenbereich sichern - incl. Filter-Ergebnis
            rDoc.CopyToDocument( 0,rParam.nRow1+1,nTab, MAXCOL,rParam.nRow2,nTab,
                                    InsertDeleteFlags::ALL, false, pUndoDoc );

            //  alle Formeln wegen Referenzen
            rDoc.CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1,
                                        InsertDeleteFlags::FORMULA, false, pUndoDoc );

            //  DB- und andere Bereiche
            ScRangeName* pDocRange = rDoc.GetRangeName();
            if (!pDocRange->empty())
                pUndoRange = new ScRangeName( *pDocRange );
            ScDBCollection* pDocDB = rDoc.GetDBCollection();
            if (!pDocDB->empty())
                pUndoDB = new ScDBCollection( *pDocDB );
        }

//      rDoc.SetOutlineTable( nTab, NULL );
        ScOutlineTable* pOut = rDoc.GetOutlineTable( nTab );
        if (pOut)
            pOut->GetRowArray().RemoveAll();       // nur Zeilen-Outlines loeschen

        if (rParam.bReplace)
            rDoc.RemoveSubTotals( nTab, aNewParam );
        bool bSuccess = true;
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

            bSuccess = rDoc.DoSubTotals( nTab, aNewParam );
            rDoc.SetDrawPageSize(nTab);
        }
        ScRange aDirtyRange( aNewParam.nCol1, aNewParam.nRow1, nTab,
            aNewParam.nCol2, aNewParam.nRow2, nTab );
        rDoc.SetDirty( aDirtyRange, true );

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
        rDoc.CompileDBFormula();

        rDocShell.PostPaint(ScRange(0, 0, nTab, MAXCOL,MAXROW,nTab),
                            PAINT_GRID | PAINT_LEFT | PAINT_TOP | PAINT_SIZE);
        aModificator.SetDocumentModified();

        bRet = bSuccess;
    }
    return bRet;
}

namespace {

bool lcl_EmptyExcept( ScDocument* pDoc, const ScRange& rRange, const ScRange& rExcept )
{
    ScCellIterator aIter( pDoc, rRange );
    for (bool bHasCell = aIter.first(); bHasCell; bHasCell = aIter.next())
    {
        if (!aIter.isEmpty())      // real content?
        {
            if (!rExcept.In(aIter.GetPos()))
                return false;       // cell found
        }
    }

    return true;        // nothing found - empty
}

bool isEditable(ScDocShell& rDocShell, const ScRangeList& rRanges, bool bApi)
{
    ScDocument& rDoc = rDocShell.GetDocument();
    if (!rDocShell.IsEditable() || rDoc.GetChangeTrack())
    {
        //  not recorded -> disallow
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);

        return false;
    }

    for (size_t i = 0, n = rRanges.size(); i < n; ++i)
    {
        const ScRange* p = rRanges[i];
        ScEditableTester aTester(&rDoc, *p);
        if (!aTester.IsEditable())
        {
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());

            return false;
        }
    }

    return true;
}

void createUndoDoc(std::unique_ptr<ScDocument>& pUndoDoc, ScDocument* pDoc, const ScRange& rRange)
{
    SCTAB nTab = rRange.aStart.Tab();
    pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
    pUndoDoc->InitUndo(pDoc, nTab, nTab);
    pDoc->CopyToDocument(rRange, InsertDeleteFlags::ALL, false, pUndoDoc.get());
}

bool checkNewOutputRange(ScDPObject& rDPObj, ScDocShell& rDocShell, ScRange& rNewOut, bool bApi)
{
    ScDocument& rDoc = rDocShell.GetDocument();

    bool bOverflow = false;
    rNewOut = rDPObj.GetNewOutputRange(bOverflow);

    // Test for overlap with source data range.
    // TODO: Check with other pivot tables as well.
    const ScSheetSourceDesc* pSheetDesc = rDPObj.GetSheetDesc();
    if (pSheetDesc && pSheetDesc->GetSourceRange().Intersects(rNewOut))
    {
        // New output range intersteps with the source data. Move it up to
        // where the old range is and see if that works.
        ScRange aOldRange = rDPObj.GetOutRange();
        SCsROW nDiff = aOldRange.aStart.Row() - rNewOut.aStart.Row();
        rNewOut.aStart.SetRow(aOldRange.aStart.Row());
        rNewOut.aEnd.IncRow(nDiff);
        if (!ValidRow(rNewOut.aStart.Row()) || !ValidRow(rNewOut.aEnd.Row()))
            bOverflow = true;
    }

    if (bOverflow)
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PIVOT_ERROR);

        return false;
    }

    ScEditableTester aTester(&rDoc, rNewOut);
    if (!aTester.IsEditable())
    {
        //  destination area isn't editable
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());

        return false;
    }

    return true;
}

}

bool ScDBDocFunc::DataPilotUpdate( ScDPObject* pOldObj, const ScDPObject* pNewObj,
                                   bool bRecord, bool bApi, bool bAllowMove )
{
    if (!pOldObj)
    {
        if (!pNewObj)
            return false;

        return CreatePivotTable(*pNewObj, bRecord, bApi);
    }

    if (pOldObj)
    {
        if (!pNewObj)
            return RemovePivotTable(*pOldObj, bRecord, bApi);

        if (pOldObj == pNewObj)
            return UpdatePivotTable(*pOldObj, bRecord, bApi);
    }

    OSL_ASSERT(pOldObj && pNewObj && pOldObj != pNewObj);

    ScDocShellModificator aModificator( rDocShell );
    WaitObject aWait( ScDocShell::GetActiveDialogParent() );

    ScRangeList aRanges;
    aRanges.Append(pOldObj->GetOutRange());
    aRanges.Append(pNewObj->GetOutRange().aStart); // at least one cell in the output position must be editable.
    if (!isEditable(rDocShell, aRanges, bApi))
        return false;

    std::unique_ptr<ScDocument> pOldUndoDoc;
    std::unique_ptr<ScDocument> pNewUndoDoc;

    ScDPObject aUndoDPObj(*pOldObj); // for undo or revert on failure

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    if (bRecord)
        createUndoDoc(pOldUndoDoc, &rDoc, pOldObj->GetOutRange());

    pNewObj->WriteSourceDataTo(*pOldObj);     // copy source data

    ScDPSaveData* pData = pNewObj->GetSaveData();
    OSL_ENSURE( pData, "no SaveData from living DPObject" );
    if (pData)
        pOldObj->SetSaveData(*pData);     // copy SaveData

    pOldObj->SetAllowMove(bAllowMove);
    pOldObj->ReloadGroupTableData();
    pOldObj->SyncAllDimensionMembers();
    pOldObj->InvalidateData();             // before getting the new output area

    //  make sure the table has a name (not set by dialog)
    if (pOldObj->GetName().isEmpty())
        pOldObj->SetName( rDoc.GetDPCollection()->CreateNewName() );

    ScRange aNewOut;
    if (!checkNewOutputRange(*pOldObj, rDocShell, aNewOut, bApi))
    {
        *pOldObj = aUndoDPObj;
        return false;
    }

    //  test if new output area is empty except for old area
    if (!bApi)
    {
        // OutRange of pOldObj (pDestObj) is still old area
        if (!lcl_EmptyExcept(&rDoc, aNewOut, pOldObj->GetOutRange()))
        {
            ScopedVclPtrInstance<QueryBox> aBox( ScDocShell::GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                             ScGlobal::GetRscString(STR_PIVOT_NOTEMPTY) );
            if (aBox->Execute() == RET_NO)
            {
                //! like above (not editable)
                *pOldObj = aUndoDPObj;
                return false;
            }
        }
    }

    if (bRecord)
        createUndoDoc(pNewUndoDoc, &rDoc, aNewOut);

    pOldObj->Output(aNewOut.aStart);
    rDocShell.PostPaintGridAll();           //! only necessary parts

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDataPilot(
                &rDocShell, pOldUndoDoc.release(), pNewUndoDoc.release(), &aUndoDPObj, pOldObj, bAllowMove));
    }

    // notify API objects
    rDoc.BroadcastUno( ScDataPilotModifiedHint(pOldObj->GetName()) );
    aModificator.SetDocumentModified();

    return true;
}

bool ScDBDocFunc::RemovePivotTable(ScDPObject& rDPObj, bool bRecord, bool bApi)
{
    ScDocShellModificator aModificator(rDocShell);
    WaitObject aWait(ScDocShell::GetActiveDialogParent());

    if (!isEditable(rDocShell, rDPObj.GetOutRange(), bApi))
        return false;

    std::unique_ptr<ScDocument> pOldUndoDoc;
    std::unique_ptr<ScDPObject> pUndoDPObj;

    if (bRecord)
        pUndoDPObj.reset(new ScDPObject(rDPObj));    // copy old settings for undo

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    //  delete table

    ScRange aRange = rDPObj.GetOutRange();
    SCTAB nTab = aRange.aStart.Tab();

    if (bRecord)
        createUndoDoc(pOldUndoDoc, &rDoc, aRange);

    rDoc.DeleteAreaTab( aRange.aStart.Col(), aRange.aStart.Row(),
                         aRange.aEnd.Col(),   aRange.aEnd.Row(),
                         nTab, InsertDeleteFlags::ALL );
    rDoc.RemoveFlagsTab( aRange.aStart.Col(), aRange.aStart.Row(),
                          aRange.aEnd.Col(),   aRange.aEnd.Row(),
                          nTab, ScMF::Auto );

    rDoc.GetDPCollection()->FreeTable(&rDPObj);  // object is deleted here

    rDocShell.PostPaintGridAll();   //! only necessary parts
    rDocShell.PostPaint(aRange, PAINT_GRID);

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDataPilot(
                &rDocShell, pOldUndoDoc.release(), nullptr, pUndoDPObj.get(), nullptr, false));

        // pUndoDPObj is copied
    }

    aModificator.SetDocumentModified();
    return true;
}

bool ScDBDocFunc::CreatePivotTable(const ScDPObject& rDPObj, bool bRecord, bool bApi)
{
    ScDocShellModificator aModificator(rDocShell);
    WaitObject aWait(ScDocShell::GetActiveDialogParent());

    // At least one cell in the output range should be editable. Check in advance.
    if (!isEditable(rDocShell, ScRange(rDPObj.GetOutRange().aStart), bApi))
        return false;

    std::unique_ptr<ScDocument> pNewUndoDoc;

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    //  output range must be set at pNewObj
    std::unique_ptr<ScDPObject> pDestObj(new ScDPObject(rDPObj));

    ScDPObject& rDestObj = *pDestObj;

    // #i94570# When changing the output position in the dialog, a new table is created
    // with the settings from the old table, including the name.
    // So we have to check for duplicate names here (before inserting).
    if (rDoc.GetDPCollection()->GetByName(rDestObj.GetName()))
        rDestObj.SetName(OUString());      // ignore the invalid name, create a new name below

    if (!rDoc.GetDPCollection()->InsertNewTable(pDestObj.release()))
        // Insertion into collection failed.
        return false;

    rDestObj.ReloadGroupTableData();
    rDestObj.SyncAllDimensionMembers();
    rDestObj.InvalidateData();             // before getting the new output area

    //  make sure the table has a name (not set by dialog)
    if (rDestObj.GetName().isEmpty())
        rDestObj.SetName(rDoc.GetDPCollection()->CreateNewName());

    bool bOverflow = false;
    ScRange aNewOut = rDestObj.GetNewOutputRange(bOverflow);

    if (bOverflow)
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PIVOT_ERROR);

        return false;
    }

    {
        ScEditableTester aTester(&rDoc, aNewOut);
        if (!aTester.IsEditable())
        {
            //  destination area isn't editable
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());

            return false;
        }
    }

    //  test if new output area is empty except for old area
    if (!bApi)
    {
        bool bEmpty = rDoc.IsBlockEmpty(
            aNewOut.aStart.Tab(), aNewOut.aStart.Col(), aNewOut.aStart.Row(),
            aNewOut.aEnd.Col(), aNewOut.aEnd.Row());

        if (!bEmpty)
        {
            ScopedVclPtrInstance<QueryBox> aBox(
                ScDocShell::GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                ScGlobal::GetRscString(STR_PIVOT_NOTEMPTY));

            if (aBox->Execute() == RET_NO)
            {
                //! like above (not editable)
                return false;
            }
        }
    }

    if (bRecord)
        createUndoDoc(pNewUndoDoc, &rDoc, aNewOut);

    rDestObj.Output(aNewOut.aStart);
    rDocShell.PostPaintGridAll();           //! only necessary parts

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDataPilot(&rDocShell, nullptr, pNewUndoDoc.release(), nullptr, &rDestObj, false));
    }

    // notify API objects
    rDoc.BroadcastUno(ScDataPilotModifiedHint(rDestObj.GetName()));
    aModificator.SetDocumentModified();

    return true;
}

bool ScDBDocFunc::UpdatePivotTable(ScDPObject& rDPObj, bool bRecord, bool bApi)
{
    ScDocShellModificator aModificator( rDocShell );
    WaitObject aWait( ScDocShell::GetActiveDialogParent() );

    if (!isEditable(rDocShell, rDPObj.GetOutRange(), bApi))
        return false;

    std::unique_ptr<ScDocument> pOldUndoDoc;
    std::unique_ptr<ScDocument> pNewUndoDoc;

    ScDPObject aUndoDPObj(rDPObj); // For undo or revert on failure.

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    if (bRecord)
        createUndoDoc(pOldUndoDoc, &rDoc, rDPObj.GetOutRange());

    rDPObj.SetAllowMove(false);
    rDPObj.ReloadGroupTableData();
    if (!rDPObj.SyncAllDimensionMembers())
        return false;

    rDPObj.InvalidateData();             // before getting the new output area

    //  make sure the table has a name (not set by dialog)
    if (rDPObj.GetName().isEmpty())
        rDPObj.SetName( rDoc.GetDPCollection()->CreateNewName() );

    ScRange aNewOut;
    if (!checkNewOutputRange(rDPObj, rDocShell, aNewOut, bApi))
    {
        rDPObj = aUndoDPObj;
        return false;
    }

    //  test if new output area is empty except for old area
    if (!bApi)
    {
        if (!lcl_EmptyExcept(&rDoc, aNewOut, rDPObj.GetOutRange()))
        {
            ScopedVclPtrInstance<QueryBox> aBox( ScDocShell::GetActiveDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
                                                 ScGlobal::GetRscString(STR_PIVOT_NOTEMPTY) );
            if (aBox->Execute() == RET_NO)
            {
                rDPObj = aUndoDPObj;
                return false;
            }
        }
    }

    if (bRecord)
        createUndoDoc(pNewUndoDoc, &rDoc, aNewOut);

    rDPObj.Output(aNewOut.aStart);
    rDocShell.PostPaintGridAll();           //! only necessary parts

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            new ScUndoDataPilot(
                &rDocShell, pOldUndoDoc.release(), pNewUndoDoc.release(), &aUndoDPObj, &rDPObj, false));
    }

    // notify API objects
    rDoc.BroadcastUno( ScDataPilotModifiedHint(rDPObj.GetName()) );
    aModificator.SetDocumentModified();
    return true;
}

sal_uLong ScDBDocFunc::RefreshPivotTables(ScDPObject* pDPObj, bool bApi)
{
    ScDPCollection* pDPs = rDocShell.GetDocument().GetDPCollection();
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

        // This action is intentionally not undoable since it modifies cache.
        UpdatePivotTable(*pObj, false, bApi);
    }

    return 0;
}

void ScDBDocFunc::RefreshPivotTableGroups(ScDPObject* pDPObj)
{
    if (!pDPObj)
        return;

    ScDPCollection* pDPs = rDocShell.GetDocument().GetDPCollection();
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
        UpdatePivotTable(*pObj, false, false);
    }
}

//      database import

void ScDBDocFunc::UpdateImport( const OUString& rTarget, const svx::ODataAccessDescriptor& rDescriptor )
{
    // rTarget is the name of a database range

    ScDocument& rDoc = rDocShell.GetDocument();
    ScDBCollection& rDBColl = *rDoc.GetDBCollection();
    const ScDBData* pData = rDBColl.getNamedDBs().findByUpperName(ScGlobal::pCharClass->uppercase(rTarget));
    if (!pData)
    {
        ScopedVclPtrInstance<InfoBox> aInfoBox( ScDocShell::GetActiveDialogParent(),
                                                ScGlobal::GetRscString( STR_TARGETNOTFOUND ) );
        aInfoBox->Execute();
        return;
    }

    SCTAB nTab;
    SCCOL nDummyCol;
    SCROW nDummyRow;
    pData->GetArea( nTab, nDummyCol,nDummyRow,nDummyCol,nDummyRow );

    ScImportParam aImportParam;
    pData->GetImportParam( aImportParam );

    OUString sDBName;
    OUString sDBTable;
    sal_Int32 nCommandType = 0;
    sDBName = rDescriptor.getDataSource();
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
