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
#include <vcl/weld.hxx>
#include <vcl/waitobj.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <unotools/charclass.hxx>

#include <dbdocfun.hxx>
#include <dbdata.hxx>
#include <undodat.hxx>
#include <docsh.hxx>
#include <docfunc.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <globalnames.hxx>
#include <tabvwsh.hxx>
#include <patattr.hxx>
#include <rangenam.hxx>
#include <olinetab.hxx>
#include <dpobject.hxx>
#include <dpsave.hxx>
#include <dociter.hxx>
#include <editable.hxx>
#include <attrib.hxx>
#include <drwlayer.hxx>
#include <dpshttab.hxx>
#include <hints.hxx>
#include <queryentry.hxx>
#include <markdata.hxx>
#include <progress.hxx>
#include <undosort.hxx>
#include <inputopt.hxx>
#include <scmod.hxx>

#include <chartlis.hxx>
#include <ChartTools.hxx>

#include <set>
#include <memory>

using namespace ::com::sun::star;

bool ScDBDocFunc::AddDBRange( const OUString& rName, const ScRange& rRange )
{

    ScDocShellModificator aModificator( rDocShell );

    ScDocument& rDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = rDoc.GetDBCollection();
    bool bUndo (rDoc.IsUndoEnabled());

    std::unique_ptr<ScDBCollection> pUndoColl;
    if (bUndo)
        pUndoColl.reset( new ScDBCollection( *pDocColl ) );

    std::unique_ptr<ScDBData> pNew(new ScDBData( rName, rRange.aStart.Tab(),
                                    rRange.aStart.Col(), rRange.aStart.Row(),
                                    rRange.aEnd.Col(), rRange.aEnd.Row() ));

    // #i55926# While loading XML, formula cells only have a single string token,
    // so CompileDBFormula would never find any name (index) tokens, and would
    // unnecessarily loop through all cells.
    bool bCompile = !rDoc.IsImportingXML();
    bool bOk;
    if ( bCompile )
        rDoc.PreprocessDBDataUpdate();
    if ( rName == STR_DB_LOCAL_NONAME )
    {
        rDoc.SetAnonymousDBData(rRange.aStart.Tab(), std::move(pNew));
        bOk = true;
    }
    else
    {
        bOk = pDocColl->getNamedDBs().insert(std::move(pNew));
    }
    if ( bCompile )
        rDoc.CompileHybridFormula();

    if (!bOk)
    {
        return false;
    }

    if (bUndo)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoDBData>( &rDocShell, std::move(pUndoColl),
                            std::make_unique<ScDBCollection>( *pDocColl ) ) );
    }

    aModificator.SetDocumentModified();
    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScDbAreasChanged ) );
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

        std::unique_ptr<ScDBCollection> pUndoColl;
        if (bUndo)
            pUndoColl.reset( new ScDBCollection( *pDocColl ) );

        rDoc.PreprocessDBDataUpdate();
        rDBs.erase(iter);
        rDoc.CompileHybridFormula();

        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                            std::make_unique<ScUndoDBData>( &rDocShell, std::move(pUndoColl),
                                std::make_unique<ScDBCollection>( *pDocColl ) ) );
        }

        aModificator.SetDocumentModified();
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScDbAreasChanged ) );
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

        std::unique_ptr<ScDBData> pNewData(new ScDBData(rNew, **iterOld));

        std::unique_ptr<ScDBCollection> pUndoColl( new ScDBCollection( *pDocColl ) );

        rDoc.PreprocessDBDataUpdate();
        rDBs.erase(iterOld);
        bool bInserted = rDBs.insert(std::move(pNewData));
        if (!bInserted)                             // error -> restore old state
        {
            rDoc.SetDBCollection(std::move(pUndoColl));       // belongs to the document then
        }

        rDoc.CompileHybridFormula();

        if (bInserted)                              // insertion worked
        {
            if (bUndo)
            {
                rDocShell.GetUndoManager()->AddUndoAction(
                                std::make_unique<ScUndoDBData>( &rDocShell, std::move(pUndoColl),
                                    std::make_unique<ScDBCollection>( *pDocColl ) ) );
            }
            else
                pUndoColl.reset();

            aModificator.SetDocumentModified();
            SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScDbAreasChanged ) );
            bDone = true;
        }
    }

    return bDone;
}

void ScDBDocFunc::ModifyDBData( const ScDBData& rNewData )
{
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
        bool bAreaChanged = ( aOldRange != aNewRange );     // then a recompilation is needed

        std::unique_ptr<ScDBCollection> pUndoColl;
        if (bUndo)
            pUndoColl.reset( new ScDBCollection( *pDocColl ) );

        *pData = rNewData;
        if (bAreaChanged)
            rDoc.CompileDBFormula();

        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                            std::make_unique<ScUndoDBData>( &rDocShell, std::move(pUndoColl),
                                std::make_unique<ScDBCollection>( *pDocColl ) ) );
        }

        aModificator.SetDocumentModified();
    }
}

void ScDBDocFunc::ModifyAllDBData( const ScDBCollection& rNewColl, const std::vector<ScRange>& rDelAreaList )
{
    ScDocShellModificator aModificator(rDocShell);
    ScDocument& rDoc = rDocShell.GetDocument();
    ScDBCollection* pOldColl = rDoc.GetDBCollection();
    std::unique_ptr<ScDBCollection> pUndoColl;
    bool bRecord = rDoc.IsUndoEnabled();

    for (const auto& rDelArea : rDelAreaList)
    {
        // unregistering target in SBA no longer necessary
        const ScAddress& rStart = rDelArea.aStart;
        const ScAddress& rEnd   = rDelArea.aEnd;
        rDocShell.DBAreaDeleted(
            rStart.Tab(), rStart.Col(), rStart.Row(), rEnd.Col());
    }

    if (bRecord)
        pUndoColl.reset( new ScDBCollection( *pOldColl ) );

    //  register target in SBA no longer necessary

    rDoc.PreprocessDBDataUpdate();
    rDoc.SetDBCollection( std::unique_ptr<ScDBCollection>(new ScDBCollection( rNewColl )) );
    rDoc.CompileHybridFormula();
    pOldColl = nullptr;
    rDocShell.PostPaint(ScRange(0, 0, 0, MAXCOL, MAXROW, MAXTAB), PaintPartFlags::Grid);
    aModificator.SetDocumentModified();
    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScDbAreasChanged ) );

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoDBData>(&rDocShell, std::move(pUndoColl),
                std::make_unique<ScDBCollection>(rNewColl)));
    }
}

bool ScDBDocFunc::RepeatDB( const OUString& rDBName, bool bApi, bool bIsUnnamed, SCTAB aTab )
{
    //! use also for ScDBFunc::RepeatDB !

    bool bDone = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
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

            //!     Undo needed data only ?

            ScDocumentUniquePtr pUndoDoc;
            std::unique_ptr<ScOutlineTable> pUndoTab;
            std::unique_ptr<ScRangeName> pUndoRange;
            std::unique_ptr<ScDBCollection> pUndoDB;

            if (bRecord)
            {
                SCTAB nTabCount = rDoc.GetTableCount();
                pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
                ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );
                if (pTable)
                {
                    pUndoTab.reset(new ScOutlineTable( *pTable ));

                    // column/row state
                    SCCOLROW nOutStartCol, nOutEndCol;
                    SCCOLROW nOutStartRow, nOutEndRow;
                    pTable->GetColArray().GetRange( nOutStartCol, nOutEndCol );
                    pTable->GetRowArray().GetRange( nOutStartRow, nOutEndRow );

                    pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
                    rDoc.CopyToDocument(static_cast<SCCOL>(nOutStartCol), 0,
                                        nTab, static_cast<SCCOL>(nOutEndCol), MAXROW, nTab,
                                        InsertDeleteFlags::NONE, false, *pUndoDoc);
                    rDoc.CopyToDocument(0, static_cast<SCROW>(nOutStartRow),
                                        nTab, MAXCOL, static_cast<SCROW>(nOutEndRow), nTab,
                                        InsertDeleteFlags::NONE, false, *pUndoDoc);
                }
                else
                    pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, true );

                //  secure data range - incl. filtering result
                rDoc.CopyToDocument(0, nStartRow, nTab, MAXCOL, nEndRow, nTab, InsertDeleteFlags::ALL, false, *pUndoDoc);

                //  all formulas because of references
                rDoc.CopyToDocument(0, 0, 0, MAXCOL, MAXROW, nTabCount-1, InsertDeleteFlags::FORMULA, false, *pUndoDoc);

                //  ranges of DB and other
                ScRangeName* pDocRange = rDoc.GetRangeName();
                if (!pDocRange->empty())
                    pUndoRange.reset(new ScRangeName( *pDocRange ));
                ScDBCollection* pDocDB = rDoc.GetDBCollection();
                if (!pDocDB->empty())
                    pUndoDB.reset(new ScDBCollection( *pDocDB ));
            }

            if (bSort && bSubTotal)
            {
                //  sort without SubTotals

                aSubTotalParam.bRemoveOnly = true;      // will be reset again further down
                DoSubTotals( nTab, aSubTotalParam, false, bApi );
            }

            if (bSort)
            {
                pDBData->GetSortParam( aSortParam );            // range may have changed
                (void)Sort( nTab, aSortParam, false, false, bApi );
            }
            if (bQuery)
            {
                pDBData->GetQueryParam( aQueryParam );          // range may have changed
                ScRange aAdvSource;
                if (pDBData->GetAdvancedQuerySource(aAdvSource))
                    Query( nTab, aQueryParam, &aAdvSource, false, bApi );
                else
                    Query( nTab, aQueryParam, nullptr, false, bApi );

                //  at not-inplace the table may have been converted
//              if ( !aQueryParam.bInplace && aQueryParam.nDestTab != nTab )
//                  SetTabNo( nTab );
            }
            if (bSubTotal)
            {
                pDBData->GetSubTotalParam( aSubTotalParam );    // range may have changed
                aSubTotalParam.bRemoveOnly = false;
                DoSubTotals( nTab, aSubTotalParam, false, bApi );
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
                    std::make_unique<ScUndoRepeatDB>( &rDocShell, nTab,
                                            nStartCol, nStartRow, nEndCol, nEndRow,
                                            nNewEndRow,
                                            //nCurX, nCurY,
                                            nStartCol, nStartRow,
                                            std::move(pUndoDoc), std::move(pUndoTab),
                                            std::move(pUndoRange), std::move(pUndoDB),
                                            pOld, pNew ) );
            }

            rDocShell.PostPaint(ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab),
                                PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top | PaintPartFlags::Size);
            bDone = true;
        }
        else if (!bApi)     // "Don't execute any operations"
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
        OSL_FAIL( "Sort: no DBData" );
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

    // tdf#119804: If there is a header row/column, it won't be affected by
    // sorting; so we can exclude it from the test.
    SCROW nStartingRowToEdit = aLocalParam.nRow1;
    SCROW nStartingColToEdit = aLocalParam.nCol1;
    if ( aLocalParam.bHasHeader )
    {
        if ( aLocalParam.bByRow )
            nStartingRowToEdit++;
        else
            nStartingColToEdit++;
    }
    ScEditableTester aTester( &rDoc, nTab, nStartingColToEdit,nStartingRowToEdit,
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
            aLocalParam.nCol2, aLocalParam.nRow2, false, aLocalParam.bByRow, !aLocalParam.bByRow,
            aLocalParam.bIncludeComments, aLocalParam.bIncludeGraphicObjects );

    SCROW nStartRow = aLocalParam.nRow1;
    if (aLocalParam.bByRow && aLocalParam.bHasHeader && nStartRow < aLocalParam.nRow2)
        ++nStartRow;

    if ( aLocalParam.bIncludePattern && rDoc.HasAttrib(
                                        aLocalParam.nCol1, nStartRow        , nTab,
                                        aLocalParam.nCol2, aLocalParam.nRow2, nTab,
                                        HasAttrFlags::Merged | HasAttrFlags::Overlapped ) )
    {
        //  merge attributes would be mixed up during sorting
        if (!bApi)
            rDocShell.ErrorMessage(STR_SORT_ERR_MERGED);
        return false;
    }

    //      execute

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

    bool bRepeatQuery = false;                          // repeat existing filter?
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
        ScProgress aProgress(&rDocShell, ScResId(STR_PROGRESS_SORTING), 0, true);
        rDoc.Sort(nTab, aLocalParam, bRepeatQuery, bUpdateRefs, &aProgress, &aUndoParam);
    }

    if (bRecord)
    {
        // Set up an undo object.
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<sc::UndoSort>(&rDocShell, aUndoParam));
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
        PaintPartFlags nPaint = PaintPartFlags::Grid;
        SCCOL nStartX = aLocalParam.nCol1;
        SCROW nStartY = aLocalParam.nRow1;
        SCCOL nEndX = aLocalParam.nCol2;
        SCROW nEndY = aLocalParam.nRow2;
        if ( bRepeatQuery )
        {
            nPaint |= PaintPartFlags::Left;
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

    if (ScTabViewShell::isAnyEditViewInRange(/*bColumns*/ false, rQueryParam.nRow1, rQueryParam.nRow2))
    {
        return false;
    }

    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    ScDBData* pDBData = rDoc.GetDBAtArea( nTab, rQueryParam.nCol1, rQueryParam.nRow1,
                                                    rQueryParam.nCol2, rQueryParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "Query: no DBData" );
        return false;
    }

    //  Change from Inplace to non-Inplace, only then cancel Inplace:
    //  (only if "Persistent"  is selected in the dialog)

    if ( !rQueryParam.bInplace && pDBData->HasQueryParam() && rQueryParam.bDestPers )
    {
        ScQueryParam aOldQuery;
        pDBData->GetQueryParam(aOldQuery);
        if (aOldQuery.bInplace)
        {
            //  cancel old filtering

            SCSIZE nEC = aOldQuery.GetEntryCount();
            for (SCSIZE i=0; i<nEC; i++)
                aOldQuery.GetEntry(i).bDoQuery = false;
            aOldQuery.bDuplicate = true;
            Query( nTab, aOldQuery, nullptr, bRecord, bApi );
        }
    }

    ScQueryParam aLocalParam( rQueryParam );        // for Paint / destination range
    bool bCopy = !rQueryParam.bInplace;             // copied in Table::Query
    ScDBData* pDestData = nullptr;                  // range to be copied to
    bool bDoSize = false;                           // adjust destination size (insert/delete)
    SCCOL nFormulaCols = 0;                         // only at bDoSize
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
            //  test if formulas need to be filled in (nFormulaCols):
            if ( bDoSize && aOldDest.aEnd.Col() == aDestTotal.aEnd.Col() )
            {
                SCCOL nTestCol = aOldDest.aEnd.Col() + 1;       // next to the range
                SCROW nTestRow = rQueryParam.nDestRow +
                                    ( aLocalParam.bHasHeader ? 1 : 0 );
                while ( nTestCol <= MAXCOL &&
                        rDoc.GetCellType(ScAddress( nTestCol, nTestRow, nTab )) == CELLTYPE_FORMULA )
                {
                    ++nTestCol;
                    ++nFormulaCols;
                }
            }

            bKeepFmt = pDestData->IsKeepFmt();
            if ( bDoSize && !rDoc.CanFitBlock( aOldDest, aDestTotal ) )
            {
                if (!bApi)
                    rDocShell.ErrorMessage(STR_MSSG_DOSUBTOTALS_2);     // cannot insert rows
                return false;
            }
        }
    }

    //      execute

    WaitObject aWait( ScDocShell::GetActiveDialogParent() );

    bool bKeepSub = false;                          // repeat existing partial results?
    ScSubTotalParam aSubTotalParam;
    if (rQueryParam.GetEntry(0).bDoQuery)           // not at cancellation
    {
        pDBData->GetSubTotalParam( aSubTotalParam );    // partial results exist?

        if ( aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly )
            bKeepSub = true;
    }

    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScDBCollection> pUndoDB;
    const ScRange* pOld = nullptr;

    if ( bRecord )
    {
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        if (bCopy)
        {
            pUndoDoc->InitUndo( &rDoc, nDestTab, nDestTab, false, true );
            rDoc.CopyToDocument(aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                aLocalParam.nCol2, aLocalParam.nRow2, nDestTab,
                                InsertDeleteFlags::ALL, false, *pUndoDoc);
            //  secure attributes in case they were copied along

            if (pDestData)
            {
                rDoc.CopyToDocument(aOldDest, InsertDeleteFlags::ALL, false, *pUndoDoc);
                pOld = &aOldDest;
            }
        }
        else
        {
            pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, true );
            rDoc.CopyToDocument(0, rQueryParam.nRow1, nTab, MAXCOL, rQueryParam.nRow2, nTab,
                                InsertDeleteFlags::NONE, false, *pUndoDoc);
        }

        ScDBCollection* pDocDB = rDoc.GetDBCollection();
        if (!pDocDB->empty())
            pUndoDB.reset(new ScDBCollection( *pDocDB ));

        rDoc.BeginDrawUndo();
    }

    std::unique_ptr<ScDocument> pAttribDoc;
    ScRange aAttribRange;
    if (pDestData)                                      // delete destination range
    {
        if ( bKeepFmt )
        {
            //  smaller of the end columns, header+1 row
            aAttribRange = aOldDest;
            if ( aAttribRange.aEnd.Col() > aDestTotal.aEnd.Col() )
                aAttribRange.aEnd.SetCol( aDestTotal.aEnd.Col() );
            aAttribRange.aEnd.SetRow( aAttribRange.aStart.Row() +
                                        ( aLocalParam.bHasHeader ? 1 : 0 ) );

            //  also for filled-in formulas
            aAttribRange.aEnd.SetCol( aAttribRange.aEnd.Col() + nFormulaCols );

            pAttribDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            pAttribDoc->InitUndo( &rDoc, nDestTab, nDestTab, false, true );
            rDoc.CopyToDocument(aAttribRange, InsertDeleteFlags::ATTRIB, false, *pAttribDoc);
        }

        if ( bDoSize )
            rDoc.FitBlock( aOldDest, aDestTotal );
        else
            rDoc.DeleteAreaTab(aOldDest, InsertDeleteFlags::ALL);         // simply delete
    }

    //  execute filtering on the document
    SCSIZE nCount = rDoc.Query( nTab, rQueryParam, bKeepSub );
    pDBData->CalcSaveFilteredCount( nCount );
    if (bCopy)
    {
        aLocalParam.nRow2 = aLocalParam.nRow1 + nCount;
        if (!aLocalParam.bHasHeader && nCount > 0)
            --aLocalParam.nRow2;

        if ( bDoSize )
        {
            //  adjust to the real result range
            //  (this here is always a reduction)

            ScRange aNewDest( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                aLocalParam.nCol2, aLocalParam.nRow2, nDestTab );
            rDoc.FitBlock( aDestTotal, aNewDest, false );      // sal_False - don't delete

            if ( nFormulaCols > 0 )
            {
                //  fill in formulas
                //! Undo (Query and Repeat) !!!

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
                        ScResId(STR_FILL_SERIES_PROGRESS), nProgCount, true );

                rDoc.Fill( aLocalParam.nCol2+1, nFStartY,
                            aLocalParam.nCol2+nFormulaCols, nFStartY, &aProgress, aMark,
                            aLocalParam.nRow2 - nFStartY,
                            FILL_TO_BOTTOM, FILL_SIMPLE );
            }
        }

        if ( pAttribDoc )       // copy back the memorized attributes
        {
            //  Header
            if (aLocalParam.bHasHeader)
            {
                ScRange aHdrRange = aAttribRange;
                aHdrRange.aEnd.SetRow( aHdrRange.aStart.Row() );
                pAttribDoc->CopyToDocument(aHdrRange, InsertDeleteFlags::ATTRIB, false, rDoc);
            }

            //  Data
            SCCOL nAttrEndCol = aAttribRange.aEnd.Col();
            SCROW nAttrRow = aAttribRange.aStart.Row() + ( aLocalParam.bHasHeader ? 1 : 0 );
            for (SCCOL nCol = aAttribRange.aStart.Col(); nCol<=nAttrEndCol; nCol++)
            {
                const ScPatternAttr* pSrcPattern = pAttribDoc->GetPattern(
                                                    nCol, nAttrRow, nDestTab );
                OSL_ENSURE(pSrcPattern,"Pattern is 0");
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
        }
    }

    //  saving: Inplace always, otherwise depending on setting
    //          old Inplace-Filter may have already been removed

    bool bSave = rQueryParam.bInplace || rQueryParam.bDestPers;
    if (bSave)                                                  // memorize
    {
        pDBData->SetQueryParam( rQueryParam );
        pDBData->SetHeader( rQueryParam.bHasHeader );       //! ???
        pDBData->SetAdvancedQuerySource( pAdvSource );      // after SetQueryParam
    }

    if (bCopy)                                              // memorize new DB range
    {
        //  Selection is done afterwards from outside (dbfunc).
        //  Currently through the DB area at the destination position,
        //  so a range must be created there in any case.

        ScDBData* pNewData;
        if (pDestData)
            pNewData = pDestData;               // range exists -> adjust (always!)
        else                                    // create range
            pNewData = rDocShell.GetDBData(
                            ScRange( aLocalParam.nCol1, aLocalParam.nRow1, nDestTab,
                                     aLocalParam.nCol2, aLocalParam.nRow2, nDestTab ),
                            SC_DB_MAKE, ScGetDBSelection::ForceMark );

        if (pNewData)
        {
            pNewData->SetArea( nDestTab, aLocalParam.nCol1, aLocalParam.nRow1,
                                            aLocalParam.nCol2, aLocalParam.nRow2 );

            //  query parameter is no longer set at the destination, only leads to confusion
            //  and mistakes with the query parameter at the source range (#37187#)
        }
        else
        {
            OSL_FAIL("Target are not available");
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
                    std::make_unique<ScUndoQuery>( &rDocShell, nTab, rQueryParam, std::move(pUndoDoc), std::move(pUndoDB),
                                        pOld, bDoSize, pAdvSource ) );
    }

    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();
    if ( pViewSh )
    {
        // could there be horizontal autofilter ?
        // maybe it would be better to set bColumns to !rQueryParam.bByRow ?
        // anyway at the beginning the value of bByRow is 'false'
        // then after the first sort action it becomes 'true'
        pViewSh->OnLOKShowHideColRow(/*bColumns*/ false, rQueryParam.nRow1 - 1);
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
            PaintPartFlags::Grid);
    }
    else
        rDocShell.PostPaint(
            ScRange(0, rQueryParam.nRow1, nTab, MAXCOL, MAXROW, nTab),
            PaintPartFlags::Grid | PaintPartFlags::Left);
    aModificator.SetDocumentModified();

    return true;
}

void ScDBDocFunc::DoSubTotals( SCTAB nTab, const ScSubTotalParam& rParam,
                               bool bRecord, bool bApi )
{
    //! use also for ScDBFunc::DoSubTotals !
    //  then stays outside:
    //  - mark new range (from DBData)
    //  - SelectionChanged (?)

    bool bDo = !rParam.bRemoveOnly;                         // sal_False = only delete

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    ScDBData* pDBData = rDoc.GetDBAtArea( nTab, rParam.nCol1, rParam.nRow1,
                                                rParam.nCol2, rParam.nRow2 );
    if (!pDBData)
    {
        OSL_FAIL( "SubTotals: no DBData" );
        return;
    }

    ScEditableTester aTester( &rDoc, nTab, 0,rParam.nRow1+1, MAXCOL,MAXROW );
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessage(aTester.GetMessageId());
        return;
    }

    if (rDoc.HasAttrib( rParam.nCol1, rParam.nRow1+1, nTab,
                         rParam.nCol2, rParam.nRow2, nTab, HasAttrFlags::Merged | HasAttrFlags::Overlapped ))
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0); // don't insert into merged
        return;
    }

    bool bOk = true;
    if (rParam.bReplace)
    {
        if (rDoc.TestRemoveSubTotals( nTab, rParam ))
        {
            vcl::Window* pWin = ScDocShell::GetActiveDialogParent();
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                      VclMessageType::Question,
                                                      VclButtonsType::YesNo, ScResId(STR_MSSG_DOSUBTOTALS_1))); // "Delete Data?"
            xBox->set_title(ScResId(STR_MSSG_DOSUBTOTALS_0)); // "StarCalc"
            bOk = xBox->run() == RET_YES;
        }
    }

    if (bOk)
    {
        WaitObject aWait( ScDocShell::GetActiveDialogParent() );
        ScDocShellModificator aModificator( rDocShell );

        ScSubTotalParam aNewParam( rParam );        // end of range is being changed
        ScDocumentUniquePtr pUndoDoc;
        std::unique_ptr<ScOutlineTable> pUndoTab;
        std::unique_ptr<ScRangeName> pUndoRange;
        std::unique_ptr<ScDBCollection> pUndoDB;

        if (bRecord)                                        // secure old data
        {
            bool bOldFilter = bDo && rParam.bDoSort;

            SCTAB nTabCount = rDoc.GetTableCount();
            pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
            ScOutlineTable* pTable = rDoc.GetOutlineTable( nTab );
            if (pTable)
            {
                pUndoTab.reset(new ScOutlineTable( *pTable ));

                // column/row state
                SCCOLROW nOutStartCol, nOutEndCol;
                SCCOLROW nOutStartRow, nOutEndRow;
                pTable->GetColArray().GetRange( nOutStartCol, nOutEndCol );
                pTable->GetRowArray().GetRange( nOutStartRow, nOutEndRow );

                pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
                rDoc.CopyToDocument(static_cast<SCCOL>(nOutStartCol), 0, nTab, static_cast<SCCOL>(nOutEndCol), MAXROW, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);
                rDoc.CopyToDocument(0, nOutStartRow, nTab, MAXCOL, nOutEndRow, nTab, InsertDeleteFlags::NONE, false, *pUndoDoc);
            }
            else
                pUndoDoc->InitUndo( &rDoc, nTab, nTab, false, bOldFilter );

            //  secure data range - incl. filtering result
            rDoc.CopyToDocument(0, rParam.nRow1+1,nTab, MAXCOL,rParam.nRow2,nTab,
                                InsertDeleteFlags::ALL, false, *pUndoDoc);

            //  all formulas because of references
            rDoc.CopyToDocument(0, 0, 0, MAXCOL,MAXROW,nTabCount-1,
                                InsertDeleteFlags::FORMULA, false, *pUndoDoc);

            //  ranges of DB and other
            ScRangeName* pDocRange = rDoc.GetRangeName();
            if (!pDocRange->empty())
                pUndoRange.reset(new ScRangeName( *pDocRange ));
            ScDBCollection* pDocDB = rDoc.GetDBCollection();
            if (!pDocDB->empty())
                pUndoDB.reset(new ScDBCollection( *pDocDB ));
        }

//      rDoc.SetOutlineTable( nTab, NULL );
        ScOutlineTable* pOut = rDoc.GetOutlineTable( nTab );
        if (pOut)
            pOut->GetRowArray().RemoveAll();       // only delete row outlines

        if (rParam.bReplace)
            rDoc.RemoveSubTotals( nTab, aNewParam );
        bool bSuccess = true;
        if (bDo)
        {
            // sort
            if ( rParam.bDoSort )
            {
                pDBData->SetArea( nTab, aNewParam.nCol1,aNewParam.nRow1, aNewParam.nCol2,aNewParam.nRow2 );

                //  set partial result field to before the sorting
                //  (Duplicates are omitted, so can be called again)

                ScSortParam aOldSort;
                pDBData->GetSortParam( aOldSort );
                ScSortParam aSortParam( aNewParam, aOldSort );
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
                std::make_unique<ScUndoSubTotals>( &rDocShell, nTab,
                                        rParam, aNewParam.nRow2,
                                        std::move(pUndoDoc), std::move(pUndoTab), // pUndoDBData,
                                        std::move(pUndoRange), std::move(pUndoDB) ) );
        }

        if (!bSuccess)
        {
            // "Cannot insert rows"
            if (!bApi)
                rDocShell.ErrorMessage(STR_MSSG_DOSUBTOTALS_2);
        }

                                                    // memorize
        pDBData->SetSubTotalParam( aNewParam );
        pDBData->SetArea( nTab, aNewParam.nCol1,aNewParam.nRow1, aNewParam.nCol2,aNewParam.nRow2 );
        rDoc.CompileDBFormula();

        rDocShell.PostPaint(ScRange(0, 0, nTab, MAXCOL,MAXROW,nTab),
                            PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top | PaintPartFlags::Size);
        aModificator.SetDocumentModified();
    }
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
        const ScRange & r = rRanges[i];
        ScEditableTester aTester(&rDoc, r);
        if (!aTester.IsEditable())
        {
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());

            return false;
        }
    }

    return true;
}

void createUndoDoc(ScDocumentUniquePtr& pUndoDoc, ScDocument* pDoc, const ScRange& rRange)
{
    SCTAB nTab = rRange.aStart.Tab();
    pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
    pUndoDoc->InitUndo(pDoc, nTab, nTab);
    pDoc->CopyToDocument(rRange, InsertDeleteFlags::ALL, false, *pUndoDoc);
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
        SCROW nDiff = aOldRange.aStart.Row() - rNewOut.aStart.Row();
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

    if (!pNewObj)
        return RemovePivotTable(*pOldObj, bRecord, bApi);

    if (pOldObj == pNewObj)
        return UpdatePivotTable(*pOldObj, bRecord, bApi);

    OSL_ASSERT(pOldObj && pNewObj && pOldObj != pNewObj);

    ScDocShellModificator aModificator( rDocShell );
    WaitObject aWait( ScDocShell::GetActiveDialogParent() );

    ScRangeList aRanges;
    aRanges.push_back(pOldObj->GetOutRange());
    aRanges.push_back(pNewObj->GetOutRange().aStart); // at least one cell in the output position must be editable.
    if (!isEditable(rDocShell, aRanges, bApi))
        return false;

    ScDocumentUniquePtr pOldUndoDoc;
    ScDocumentUniquePtr pNewUndoDoc;

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
            vcl::Window* pWin = ScDocShell::GetActiveDialogParent();
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                           ScResId(STR_PIVOT_NOTEMPTY)));
            xQueryBox->set_default_response(RET_YES);
            if (xQueryBox->run() == RET_NO)
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
            std::make_unique<ScUndoDataPilot>(
                &rDocShell, std::move(pOldUndoDoc), std::move(pNewUndoDoc), &aUndoDPObj, pOldObj, bAllowMove));
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

    ScDocument& rDoc = rDocShell.GetDocument();

    if (!bApi)
    {
        // If we come from GUI - ask to delete the associated pivot charts too..
        std::vector<SdrOle2Obj*> aListOfObjects =
                    sc::tools::getAllPivotChartsConntectedTo(rDPObj.GetName(), &rDocShell);

        ScDrawLayer* pModel = rDoc.GetDrawLayer();

        if (pModel && !aListOfObjects.empty())
        {
            vcl::Window* pWin = ScDocShell::GetActiveDialogParent();
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                           ScResId(STR_PIVOT_REMOVE_PIVOTCHART)));
            xQueryBox->set_default_response(RET_YES);
            if (xQueryBox->run() == RET_NO)
            {
                return false;
            }
            else
            {
                for (SdrOle2Obj* pChartObject : aListOfObjects)
                {
                    rDoc.GetChartListenerCollection()->removeByName(pChartObject->GetName());
                    pModel->AddUndo(std::make_unique<SdrUndoDelObj>(*pChartObject));
                    pChartObject->getSdrPageFromSdrObject()->RemoveObject(pChartObject->GetOrdNum());
                }
            }
        }
    }

    ScDocumentUniquePtr pOldUndoDoc;
    std::unique_ptr<ScDPObject> pUndoDPObj;

    if (bRecord)
        pUndoDPObj.reset(new ScDPObject(rDPObj));    // copy old settings for undo

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
    rDocShell.PostPaint(aRange, PaintPartFlags::Grid);

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoDataPilot>(
                &rDocShell, std::move(pOldUndoDoc), nullptr, pUndoDPObj.get(), nullptr, false));

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

    ScDocumentUniquePtr pNewUndoDoc;

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

    // Synchronize groups between linked tables
    {
        const ScDPDimensionSaveData* pGroups = nullptr;
        bool bRefFound = rDoc.GetDPCollection()->GetReferenceGroups(rDestObj, &pGroups);
        if (bRefFound)
        {
            ScDPSaveData* pSaveData = rDestObj.GetSaveData();
            if (pSaveData)
                pSaveData->SetDimensionData(pGroups);
        }
    }

    rDoc.GetDPCollection()->InsertNewTable(std::move(pDestObj));

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
            vcl::Window* pWin = ScDocShell::GetActiveDialogParent();
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                           ScResId(STR_PIVOT_NOTEMPTY)));
            xQueryBox->set_default_response(RET_YES);
            if (xQueryBox->run() == RET_NO)
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
            std::make_unique<ScUndoDataPilot>(&rDocShell, nullptr, std::move(pNewUndoDoc), nullptr, &rDestObj, false));
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

    ScDocumentUniquePtr pOldUndoDoc;
    ScDocumentUniquePtr pNewUndoDoc;

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
            vcl::Window* pWin = ScDocShell::GetActiveDialogParent();
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                           ScResId(STR_PIVOT_NOTEMPTY)));
            xQueryBox->set_default_response(RET_YES);
            if (xQueryBox->run() == RET_NO)
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
            std::make_unique<ScUndoDataPilot>(
                &rDocShell, std::move(pOldUndoDoc), std::move(pNewUndoDoc), &aUndoDPObj, &rDPObj, false));
    }

    // notify API objects
    rDoc.BroadcastUno( ScDataPilotModifiedHint(rDPObj.GetName()) );
    aModificator.SetDocumentModified();
    return true;
}

void ScDBDocFunc::RefreshPivotTables(const ScDPObject* pDPObj, bool bApi)
{
    ScDPCollection* pDPs = rDocShell.GetDocument().GetDPCollection();
    if (!pDPs)
        return;

    std::set<ScDPObject*> aRefs;
    const char* pErrId = pDPs->ReloadCache(pDPObj, aRefs);
    if (pErrId)
        return;

    for (ScDPObject* pObj : aRefs)
    {
        // This action is intentionally not undoable since it modifies cache.
        UpdatePivotTable(*pObj, false, bApi);
    }
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

    if (!pDPs->HasTable(pDPObj))
    {
        // This table is under construction so no need for a whole update (UpdatePivotTable()).
        pDPObj->ReloadGroupTableData();
        return;
    }

    // Update all linked tables, if this table is part of the cache (ScDPCollection)
    std::set<ScDPObject*> aRefs;
    if (!pDPs->ReloadGroupsInCache(pDPObj, aRefs))
        return;

    // We allow pDimData being NULL.
    const ScDPDimensionSaveData* pDimData = pSaveData->GetExistingDimensionData();
    for (ScDPObject* pObj : aRefs)
    {
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
        vcl::Window* pWin = ScDocShell::GetActiveDialogParent();
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      ScResId(STR_TARGETNOTFOUND)));
        xInfoBox->run();
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
    rDescriptor[svx::DataAccessDescriptorProperty::Command]     >>= sDBTable;
    rDescriptor[svx::DataAccessDescriptorProperty::CommandType] >>= nCommandType;

    aImportParam.aDBName    = sDBName;
    aImportParam.bSql       = ( nCommandType == sdb::CommandType::COMMAND );
    aImportParam.aStatement = sDBTable;
    aImportParam.bNative    = false;
    aImportParam.nType      = static_cast<sal_uInt8>( ( nCommandType == sdb::CommandType::QUERY ) ? ScDbQuery : ScDbTable );
    aImportParam.bImport    = true;

    bool bContinue = DoImport( nTab, aImportParam, &rDescriptor );

    //  repeat DB operations

    ScTabViewShell* pViewSh = rDocShell.GetBestViewShell();
    if (pViewSh)
    {
        ScRange aRange;
        pData->GetArea(aRange);
        pViewSh->MarkRange(aRange);         // select

        if ( bContinue )        // error at import -> abort
        {
            //  internal operations, if some are saved

            if ( pData->HasQueryParam() || pData->HasSortParam() || pData->HasSubTotalParam() )
                pViewSh->RepeatDB();

            //  pivot tables which have the range as source data

            rDocShell.RefreshPivotTables(aRange);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
