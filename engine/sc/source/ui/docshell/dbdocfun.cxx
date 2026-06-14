/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <unotools/charclass.hxx>
#include <comphelper/kit.hxx>
#include <osl/diagnose.h>

#include <dbdocfun.hxx>
#include <dbdata.hxx>
#include <undodat.hxx>
#include <docsh.hxx>
#include <docfunc.hxx>
#include <global.hxx>
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
#include <operation/CreatePivotTableOperation.hxx>
#include <operation/QueryOperation.hxx>
#include <operation/RemovePivotTableOperation.hxx>
#include <operation/ReplacePivotTableOperation.hxx>
#include <operation/SortOperation.hxx>
#include <operation/SubTotalsOperation.hxx>
#include <operation/UpdatePivotTableOperation.hxx>

#include <algorithm>
#include <memory>

using namespace ::com::sun::star;

bool ScDBDocFunc::AddDBTable(const OUString& rName, const ScRange& rRange, bool bHeader,
                             bool bRecord, bool bApi, const OUString& rStyleName)
{
    ScDocShellModificator aModificator(rDocShell);

    ScDocument& rDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = rDoc.GetDBCollection();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    if (rStyleName.isEmpty())
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_TABLE_ERR_ADD);
        return false;
    }

    std::unique_ptr<ScDBCollection> pUndoColl;
    if (bRecord)
        pUndoColl.reset(new ScDBCollection(*pDocColl));

    // If a database range already covers exactly this area, reuse it and
    // promote it into a table rather than layering a second, overlapping
    // range on top.
    ScDBData* pExisting = pDocColl->GetDBAtArea(rRange.aStart.Tab(), rRange.aStart.Col(),
                                                rRange.aStart.Row(), rRange.aEnd.Col(),
                                                rRange.aEnd.Row());

    // The exact range is already a styled table; don't create a duplicate.
    if (pExisting && pExisting->GetTableStyleInfo())
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_TABLE_ERR_ADD);
        return false;
    }

    bool bReuseNamed
        = pExisting
          && pDocColl->getNamedDBs().findByPointer(pExisting) != pDocColl->getNamedDBs().end();
    bool bReuseAnon
        = pExisting && !bReuseNamed && rDoc.GetAnonymousDBData(rRange.aStart.Tab()) == pExisting;

    ScTableStyleParam aStyleParam;
    aStyleParam.maStyleID = rStyleName;

    bool bOk = true;

    if (bReuseNamed)
    {
        pExisting->SetHeader(bHeader);
        pExisting->SetAutoFilter(true);
        pExisting->SetTableStyleInfo(aStyleParam);
        rDoc.ApplyFlagsTab(rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(),
                           rRange.aStart.Row(), rRange.aStart.Tab(), ScMF::Auto);
    }
    else
    {
        std::unique_ptr<ScDBData> pNew;
        if (bReuseAnon)
        {
            // Migrate the anonymous range into the named range
            pNew.reset(new ScDBData(rName, *pExisting));
            pNew->SetHeader(bHeader);
            pNew->SetTableStyleInfo(aStyleParam);
        }
        else
        {
            pNew.reset(new ScDBData(rName, rRange.aStart.Tab(), rRange.aStart.Col(),
                                    rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row(),
                                    true, bHeader, false, u""_ustr, rStyleName));
        }
        pNew->SetAutoFilter(true);
        rDoc.ApplyFlagsTab(rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(),
                           rRange.aStart.Row(), rRange.aStart.Tab(), ScMF::Auto);

        if (bReuseAnon)
            rDoc.SetAnonymousDBData(rRange.aStart.Tab(), nullptr);

        bool bCompile = !rDoc.IsImportingXML();
        if (bCompile)
            rDoc.PreprocessDBDataUpdate();

        bOk = pDocColl->getNamedDBs().insert(std::move(pNew));
        if (bCompile)
            rDoc.CompileHybridFormula();
    }

    if (!bOk && !bApi)
    {
        rDocShell.ErrorMessage(STR_TABLE_ERR_ADD);
        return false;
    }

    rDocShell.PostPaint(rRange, PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top | PaintPartFlags::Size);

    if (bRecord)
    {
        ViewShellId nViewShellId(-1);
        if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
            nViewShellId = pViewSh->GetViewShellId();
        rDocShell.GetUndoManager()->EnterListAction(u""_ustr, u""_ustr, 0, nViewShellId);

        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoDBTable>(rDocShell, rName, true/*bInsert*/, std::move(pUndoColl),
                                           std::make_unique<ScDBCollection>(*pDocColl)));
    }

    if (bHeader)
    {
        if (ScDBData* pInserted = pDocColl->getNamedDBs().findByName(rName))
            FillEmptyHeaderColumnNames(*pInserted);
    }

    if (bRecord)
        rDocShell.GetUndoManager()->LeaveListAction();

    aModificator.SetDocumentModified();
    SfxGetpApp()->Broadcast(SfxHint(SfxHintId::ScDbAreasChanged));
    return true;
}

void ScDBDocFunc::FillEmptyHeaderColumnNames(ScDBData& rData, SCCOL nFirstCol, SCCOL nLastCol)
{
    if (!rData.HasHeader())
        return;

    ScDocument& rDoc = rDocShell.GetDocument();
    const ScRange aHeader = rData.GetHeaderArea();
    if (!aHeader.IsValid())
        return;

    const SCTAB nTab = aHeader.aStart.Tab();
    const SCROW nRow = aHeader.aStart.Row();
    const SCCOL nHdrCol1 = aHeader.aStart.Col();
    const SCCOL nHdrCol2 = aHeader.aEnd.Col();

    const SCCOL nCol1 = (nFirstCol < 0) ? nHdrCol1 : std::max(nFirstCol, nHdrCol1);
    const SCCOL nCol2 = (nLastCol < 0) ? nHdrCol2 : std::min(nLastCol, nHdrCol2);
    if (nCol1 > nCol2)
        return;

    std::vector<OUString> aExisting;
    aExisting.reserve(static_cast<size_t>(nHdrCol2 - nHdrCol1 + 1));
    for (SCCOL nCol = nHdrCol1; nCol <= nHdrCol2; ++nCol)
        aExisting.push_back(rDoc.GetString(nCol, nRow, nTab));

    // A header name collides case-insensitively, matching the engine's own
    // de-duplication (ScDBData::SetTableColumnName via GetTransliteration); a
    // plain == compare would wrongly let "Column1"/"column1" coexist.
    auto bNameTaken = [&aExisting](const OUString& rName) {
        return std::any_of(aExisting.begin(), aExisting.end(), [&rName](const OUString& rSeen)
                           { return ScGlobal::GetTransliteration().isEqual(rName, rSeen); });
    };

    const OUString aColumn(ScResId(STR_COLUMN));
    ScDocFunc& rFunc = rDocShell.GetDocFunc();
    bool bWrote = false;
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
    {
        if (rDoc.GetCellType(ScAddress(nCol, nRow, nTab)) != CELLTYPE_NONE)
            continue;

        // Get the already cached name, if exists
        OUString aName = rData.GetTableColumnName(nCol);
        bool bGenerated = false;
        if (aName.isEmpty() || bNameTaken(aName))
        {
            sal_Int32 nNumber = nCol - nHdrCol1 + 1;
            do
            {
                aName = aColumn + OUString::number(nNumber);
                ++nNumber;
            } while (bNameTaken(aName));
            bGenerated = true;
        }

        rFunc.SetStringCell(ScAddress(nCol, nRow, nTab), aName, false);
        aExisting[nCol - nHdrCol1] = aName;
        if (bGenerated)
            rData.AddGeneratedHeaderName(aName);
        bWrote = true;
    }

    if (bWrote)
        rData.RefreshTableColumnNames(&rDoc);
}

void ScDBDocFunc::FillInsertedColumnHeaders(SCTAB nTab, SCCOL nCol1, SCCOL nCol2)
{
    if (nCol1 > nCol2)
        return;

    ScDBCollection* pColl = rDocShell.GetDocument().GetDBCollection();
    if (!pColl)
        return;

    for (const auto& rxData : pColl->getNamedDBs())
    {
        ScDBData* pData = rxData.get();
        if (!pData->GetTableStyleInfo() || !pData->HasHeader())
            continue;
        ScRange aArea;
        pData->GetArea(aArea);
        if (aArea.aStart.Tab() != nTab)
            continue;

        FillEmptyHeaderColumnNames(*pData, nCol1, nCol2);
    }
}

void ScDBDocFunc::RestoreEmptyHeaderColumnNames(ScDBData& rData)
{
    if (!rData.GetTableStyleInfo() || !rData.HasHeader())
        return;

    const ScRange aHeader = rData.GetHeaderArea();
    if (!aHeader.IsValid())
        return;

    ScDocument& rDoc = rDocShell.GetDocument();
    ScDocFunc& rFunc = rDocShell.GetDocFunc();
    const SCTAB nTab = aHeader.aStart.Tab();
    const SCROW nRow = aHeader.aStart.Row();

    // If a header cell is cleared by user, put back the cached value
    for (SCCOL nCol = aHeader.aStart.Col(); nCol <= aHeader.aEnd.Col(); ++nCol)
    {
        if (rDoc.GetCellType(ScAddress(nCol, nRow, nTab)) != CELLTYPE_NONE)
            continue;

        const OUString& aPrev = rData.GetTableColumnName(nCol);
        if (!aPrev.isEmpty())
            rFunc.SetStringCell(ScAddress(nCol, nRow, nTab), aPrev, false);
    }
}

bool ScDBDocFunc::DeleteDBTable(const ScDBData* pDBObj, bool bRecord, bool bApi)
{
    bool bDone = false;
    ScDocument& rDoc = rDocShell.GetDocument();
    ScDBCollection* pDocColl = rDoc.GetDBCollection();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScDBCollection::NamedDBs& rDBs = pDocColl->getNamedDBs();
    auto const iter = rDBs.findByPointer(pDBObj);
    if (iter != rDBs.end())
    {
        ScDocShellModificator aModificator(rDocShell);

        OUString aTableName = iter->get()->GetName();
        ScRange aRange;
        iter->get()->GetArea(aRange);

        // Collect the header cells we auto-generated that still hold the
        // generated value, so removing the table also removes the placeholder
        // "Column#" names we added.
        const ScDBData* pDel = iter->get();
        const ScRange aHeaderArea = pDel->GetHeaderArea();
        std::vector<SCCOL> aClearCols;
        if (aHeaderArea.IsValid())
        {
            for (SCCOL nCol = aHeaderArea.aStart.Col(); nCol <= aHeaderArea.aEnd.Col(); ++nCol)
            {
                if (pDel->IsGeneratedHeaderName(
                        rDoc.GetString(nCol, aHeaderArea.aStart.Row(), aHeaderArea.aStart.Tab())))
                    aClearCols.push_back(nCol);
            }
        }

        const bool bGroup = bRecord && !aClearCols.empty();
        if (bGroup)
        {
            ViewShellId nViewShellId(-1);
            if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
                nViewShellId = pViewSh->GetViewShellId();
            rDocShell.GetUndoManager()->EnterListAction(u""_ustr, u""_ustr, 0, nViewShellId);
        }

        if (!aClearCols.empty())
        {
            ScMarkData aMark(rDoc.GetSheetLimits());
            aMark.SelectTable(aHeaderArea.aStart.Tab(), true);
            for (SCCOL nCol : aClearCols)
                aMark.SetMultiMarkArea(
                    ScRange(nCol, aHeaderArea.aStart.Row(), aHeaderArea.aStart.Tab()));
            rDocShell.GetDocFunc().DeleteContents(aMark, InsertDeleteFlags::CONTENTS, bRecord,
                                                  bApi);
        }

        std::unique_ptr<ScDBCollection> pUndoColl;
        if (bRecord)
            pUndoColl.reset(new ScDBCollection(*pDocColl));

        rDoc.PreprocessDBDataUpdate();
        rDBs.erase(iter);
        rDoc.CompileHybridFormula();

        if (aRange.IsValid())
        {
            rDoc.RemoveFlagsTab(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
                                aRange.aStart.Row(), aRange.aStart.Tab(), ScMF::Auto);
            rDocShell.PostPaint(aRange, PaintPartFlags::Grid | PaintPartFlags::Left
                                            | PaintPartFlags::Top | PaintPartFlags::Size);
        }

        if (bRecord)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoDBTable>(rDocShell, aTableName, false/*bInsert*/, std::move(pUndoColl),
                                                std::make_unique<ScDBCollection>(*pDocColl)));
        }

        if (bGroup)
            rDocShell.GetUndoManager()->LeaveListAction();

        aModificator.SetDocumentModified();
        SfxGetpApp()->Broadcast(SfxHint(SfxHintId::ScDbAreasChanged));
        bDone = true;
    }
    else if (!bApi)
    {
        rDocShell.ErrorMessage(STR_TABLE_ERR_DEL);
    }

    return bDone;
}

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
                        std::make_unique<ScUndoDBData>( rDocShell, rName, std::move(pUndoColl),
                            rName, std::make_unique<ScDBCollection>( *pDocColl ) ) );
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
    auto const iter = rDBs.findByUpperName2(ScGlobal::getCharClass().uppercase(rName));
    if (iter != rDBs.end())
    {
        ScDocShellModificator aModificator( rDocShell );

        std::unique_ptr<ScDBCollection> pUndoColl;
        if (bUndo)
            pUndoColl.reset( new ScDBCollection( *pDocColl ) );

        ScRange aRange;
        iter->get()->GetArea(aRange);

        rDoc.PreprocessDBDataUpdate();
        rDBs.erase(iter);
        rDoc.CompileHybridFormula();

        if (bUndo)
        {
            rDocShell.GetUndoManager()->AddUndoAction(
                            std::make_unique<ScUndoDBData>( rDocShell, rName, std::move(pUndoColl),
                                rName, std::make_unique<ScDBCollection>( *pDocColl ) ) );
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
    auto const iterOld = rDBs.findByUpperName2(ScGlobal::getCharClass().uppercase(rOld));
    const ScDBData* pNew = rDBs.findByUpperName(ScGlobal::getCharClass().uppercase(rNew));
    if (iterOld != rDBs.end() && !pNew)
    {
        ScDocShellModificator aModificator( rDocShell );

        std::unique_ptr<ScDBData> pNewData(new ScDBData(rNew, **iterOld));
        OUString aUndoName = rOld;
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
                                std::make_unique<ScUndoDBData>( rDocShell, aUndoName, std::move(pUndoColl),
                                    rNew, std::make_unique<ScDBCollection>( *pDocColl ) ) );
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

    if (!pData)
        return;

    ScDocShellModificator aModificator( rDocShell );
    ScRange aOldRange, aNewRange;
    pData->GetArea(aOldRange);
    rNewData.GetArea(aNewRange);
    bool bAreaChanged = ( aOldRange != aNewRange );     // then a recompilation is needed
    bool bOldAutoFilter = pData->HasAutoFilter();
    bool bNewAutoFilter = rNewData.HasAutoFilter();
    const bool bOldHasHeader = pData->HasHeader();
    const bool bNewHasHeader = rNewData.HasHeader();

    std::unique_ptr<ScDBCollection> pUndoColl;
    OUString sOldName;
    if (bUndo)
    {
        pUndoColl.reset( new ScDBCollection( *pDocColl ) );
        sOldName = pData->GetName();
    }

    *pData = rNewData;
    if (bAreaChanged || bOldAutoFilter != bNewAutoFilter) {
        if (bAreaChanged)
            rDoc.CompileDBFormula();
        if (bOldAutoFilter && !bNewAutoFilter)
        {
            rDoc.RemoveFlagsTab(aOldRange.aStart.Col(), aOldRange.aStart.Row(), aOldRange.aEnd.Col(), aOldRange.aEnd.Row(), aOldRange.aStart.Tab(), ScMF::Auto);
        }
        else if (bOldAutoFilter && bNewAutoFilter)
        {
            rDoc.RemoveFlagsTab(aOldRange.aStart.Col(), aOldRange.aStart.Row(), aOldRange.aEnd.Col(), aOldRange.aEnd.Row(), aOldRange.aStart.Tab(), ScMF::Auto);
            rDoc.ApplyFlagsTab(aNewRange.aStart.Col(), aNewRange.aStart.Row(), aNewRange.aEnd.Col(), aNewRange.aStart.Row(), aNewRange.aStart.Tab(), ScMF::Auto);
        }
        else if (!bOldAutoFilter && bNewAutoFilter)
        {
            rDoc.ApplyFlagsTab(aNewRange.aStart.Col(), aNewRange.aStart.Row(), aNewRange.aEnd.Col(), aNewRange.aStart.Row(), aNewRange.aStart.Tab(), ScMF::Auto);
        }
    }

    rDocShell.PostPaint(ScRange(0, 0, aOldRange.aStart.Tab(), rDoc.MaxCol(), rDoc.MaxRow(), aOldRange.aEnd.Tab()),
        PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top | PaintPartFlags::Size);

    const bool bStyled = pData->GetTableStyleInfo() != nullptr;
    const bool bHeaderDisabled = bStyled && bOldHasHeader && !bNewHasHeader;
    const bool bHeaderEnabled = bStyled && !bOldHasHeader && bNewHasHeader;
    const bool bColumnsChanged = bStyled && bNewHasHeader
                                 && (aNewRange.aStart.Col() != aOldRange.aStart.Col()
                                     || aNewRange.aEnd.Col() != aOldRange.aEnd.Col());

    // Get auto-generated header names
    std::vector<SCCOL> aClearCols;
    if (bHeaderDisabled)
    {
        const SCTAB nTab = aOldRange.aStart.Tab();
        const SCROW nRow = aOldRange.aStart.Row();
        for (SCCOL nCol = aOldRange.aStart.Col(); nCol <= aOldRange.aEnd.Col(); ++nCol)
        {
            if (pData->IsGeneratedHeaderName(rDoc.GetString(nCol, nRow, nTab)))
                aClearCols.push_back(nCol);
        }
    }
    else if (bColumnsChanged)
    {
        // Table shrank in size, remove auto-generated header names
        const SCTAB nTab = aOldRange.aStart.Tab();
        const SCROW nRow = aOldRange.aStart.Row();
        for (SCCOL nCol = aOldRange.aStart.Col(); nCol < aNewRange.aStart.Col(); ++nCol)
        {
            if (pData->IsGeneratedHeaderName(rDoc.GetString(nCol, nRow, nTab)))
                aClearCols.push_back(nCol);
        }
        for (SCCOL nCol = static_cast<SCCOL>(aNewRange.aEnd.Col() + 1);
             nCol <= aOldRange.aEnd.Col(); ++nCol)
        {
            if (pData->IsGeneratedHeaderName(rDoc.GetString(nCol, nRow, nTab)))
                aClearCols.push_back(nCol);
        }
    }

    // Group the cell writes with the range/setting-change undo so one Undo reverts both.
    const bool bGroup = bUndo && (bHeaderEnabled || bColumnsChanged || !aClearCols.empty());

    if (bUndo)
    {
        if (bGroup)
        {
            ViewShellId nViewShellId(-1);
            if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
                nViewShellId = pViewSh->GetViewShellId();
            rDocShell.GetUndoManager()->EnterListAction(u""_ustr, u""_ustr, 0, nViewShellId);
        }

        rDocShell.GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoDBData>( rDocShell, sOldName, std::move(pUndoColl),
                            rNewData.GetName(), std::make_unique<ScDBCollection>( *pDocColl ) ) );
    }

    if (!aClearCols.empty())
    {
        ScMarkData aMark(rDoc.GetSheetLimits());
        aMark.SelectTable(aOldRange.aStart.Tab(), true);
        for (SCCOL nCol : aClearCols)
            aMark.SetMultiMarkArea(ScRange(nCol, aOldRange.aStart.Row(), aOldRange.aStart.Tab()));
        rDocShell.GetDocFunc().DeleteContents(aMark, InsertDeleteFlags::CONTENTS, bUndo, false);
    }
    else if (bHeaderEnabled)
    {
        FillEmptyHeaderColumnNames(*pData);
    }
    else if (bColumnsChanged)
    {
        if (aNewRange.aStart.Col() < aOldRange.aStart.Col())
            FillEmptyHeaderColumnNames(*pData, aNewRange.aStart.Col(),
                                       static_cast<SCCOL>(aOldRange.aStart.Col() - 1));
        if (aNewRange.aEnd.Col() > aOldRange.aEnd.Col())
            FillEmptyHeaderColumnNames(*pData, static_cast<SCCOL>(aOldRange.aEnd.Col() + 1),
                                       aNewRange.aEnd.Col());
    }

    if (bGroup)
        rDocShell.GetUndoManager()->LeaveListAction();

    aModificator.SetDocumentModified();
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
    rDocShell.PostPaint(ScRange(0, 0, 0, rDoc.MaxCol(), rDoc.MaxRow(), MAXTAB), PaintPartFlags::Grid);
    aModificator.SetDocumentModified();
    SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScDbAreasChanged ) );

    if (bRecord)
    {
        rDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoDBData>(rDocShell, u""_ustr, std::move(pUndoColl),
                 u""_ustr, std::make_unique<ScDBCollection>(rNewColl)));
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
            pDBData = pColl->getNamedDBs().findByUpperName(ScGlobal::getCharClass().uppercase(rDBName));
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
        bool bSubTotal = aSubTotalParam.aGroups[0].bActive && !aSubTotalParam.bRemoveOnly;

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

                    pUndoDoc->InitUndo( rDoc, nTab, nTab, true, true );
                    rDoc.CopyToDocument(static_cast<SCCOL>(nOutStartCol), 0,
                                        nTab, static_cast<SCCOL>(nOutEndCol), rDoc.MaxRow(), nTab,
                                        InsertDeleteFlags::NONE, false, *pUndoDoc);
                    rDoc.CopyToDocument(0, static_cast<SCROW>(nOutStartRow),
                                        nTab, rDoc.MaxCol(), static_cast<SCROW>(nOutEndRow), nTab,
                                        InsertDeleteFlags::NONE, false, *pUndoDoc);
                }
                else
                    pUndoDoc->InitUndo( rDoc, nTab, nTab, false, true );

                //  secure data range - incl. filtering result
                rDoc.CopyToDocument(0, nStartRow, nTab, rDoc.MaxCol(), nEndRow, nTab, InsertDeleteFlags::ALL, false, *pUndoDoc);

                //  all formulas because of references
                rDoc.CopyToDocument(0, 0, 0, rDoc.MaxCol(), rDoc.MaxRow(), nTabCount-1, InsertDeleteFlags::FORMULA, false, *pUndoDoc);

                //  ranges of DB and other
                ScRangeName& rDocRange = rDoc.GetRangeName();
                if (!rDocRange.empty())
                    pUndoRange.reset(new ScRangeName( rDocRange ));
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
                (void)SortTab( nTab, aSortParam, false, false, bApi );
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
                    std::make_unique<ScUndoRepeatDB>( rDocShell, nTab,
                                            nStartCol, nStartRow, nEndCol, nEndRow,
                                            nNewEndRow,
                                            //nCurX, nCurY,
                                            nStartCol, nStartRow,
                                            std::move(pUndoDoc), std::move(pUndoTab),
                                            std::move(pUndoRange), std::move(pUndoDB),
                                            pOld, pNew ) );
            }

            rDocShell.PostPaint(ScRange(0, 0, nTab, rDoc.MaxCol(), rDoc.MaxRow(), nTab),
                                PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top | PaintPartFlags::Size);
            bDone = true;
        }
        else if (!bApi)     // "Don't execute any operations"
            rDocShell.ErrorMessage(STR_MSSG_REPEATDB_0);
    }

    return bDone;
}

bool ScDBDocFunc::SortTab(SCTAB nTab, const ScSortParam& rSortParam, bool bRecord, bool bPaint, bool bApi)
{
    sc::SortOperation aOperation(rDocShell, nTab, rSortParam, bRecord, bPaint, bApi);
    return aOperation.run();
}

bool ScDBDocFunc::Query( SCTAB nTab, const ScQueryParam& rQueryParam,
                        const ScRange* pAdvSource, bool bRecord, bool bApi )
{
    sc::QueryOperation aOperation(rDocShell, nTab, rQueryParam, pAdvSource, bRecord, bApi);
    return aOperation.run();
}

void ScDBDocFunc::DoSubTotals( SCTAB nTab, const ScSubTotalParam& rParam,
                               bool bRecord, bool bApi )
{
    sc::SubTotalsOperation aOperation(rDocShell, nTab, rParam, bRecord, bApi);
    aOperation.run();
}

void ScDBDocFunc::DoTableSubTotals( SCTAB nTab, const ScDBData& rNewData, const ScSubTotalParam& rParam,
                                    bool bRecord, bool bApi )
{
    bool bDo = !rParam.bRemoveOnly; // false = only delete
    // A resize (extend/shrink) relocates the total row: bReplace && bDo, unlike the toggle
    // (ON: bReplace=false; OFF: bRemoveOnly=true). It is net-0 rows, which drives the tear
    // refusal and Undo/Redo handling below.
    const bool bResize = rParam.bReplace && bDo;

    ScDocument& rDoc = rDocShell.GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    ScDBData* pDBData = rDoc.GetDBAtArea(nTab, rParam.nCol1, rParam.nRow1, rParam.nCol2, rParam.nRow2);
    if (!pDBData)
    {
        OSL_FAIL("SubTotals: no DBData");
        return;
    }

    ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(rDoc, nTab, 0, rParam.nRow1 + 1, rDoc.MaxCol(), rDoc.MaxRow());
    if (!aTester.IsEditable())
    {
        if (!bApi)
            rDocShell.ErrorMessageAsync(aTester.GetMessageId());
        return;
    }

    if (rDoc.HasAttrib(rParam.nCol1, rParam.nRow1 + 1, nTab, rParam.nCol2, rParam.nRow2, nTab,
                       HasAttrFlags::Merged | HasAttrFlags::Overlapped))
    {
        if (!bApi)
            rDocShell.ErrorMessageAsync(STR_MSSG_INSERTCELLS_0); // don't insert into merged
        return;
    }

    // Would a column-bounded row shift at the would-be total row tear a straddling structure
    // below? If so, a resize (which must shift to relocate the total) is refused outright; a
    // toggle may write the total cells in-place when the band is empty, and is refused only
    // when they are non-empty.
    const SCROW nShiftRow = rParam.nRow2 + 1;
    const bool bTearRisk = pDBData->HasTearRiskAtBand(rDoc, nShiftRow);
    bool bInPlace = false;
    if (bTearRisk)
    {
        if (bResize || (bDo && pDBData->IsBandBlockedAtRow(rDoc, nShiftRow)))
        {
            if (!bApi)
                rDocShell.ErrorMessageAsync(STR_MSSG_TABLE_STRADDLE);
            return;
        }
        bInPlace = true;
    }

    // A resize must not reach into another structure: HasTearRiskAtBand catches only
    // wider/straddling ones from afar, WouldResizeOverlap catches a same-width/within-column
    // one once the new area reaches it.
    if (bResize)
    {
        ScRange aNewArea;
        rNewData.GetArea(aNewArea);
        if (pDBData->WouldResizeOverlap(rDoc, aNewArea))
        {
            if (!bApi)
                rDocShell.ErrorMessageAsync(STR_MSSG_TABLE_OVERLAP);
            return;
        }
    }

    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());
    ScDocShellModificator aModificator(rDocShell);

    ScSubTotalParam aNewParam;
    rNewData.GetSubTotalParam(aNewParam); // end of range is being changed (new bottom)

    // Snapshot down to the lower of the old/new bottom, so content absorbed below the old
    // total row is captured for undo.
    const SCROW nCaptureEndRow
        = (bResize && aNewParam.nRow2 > rParam.nRow2) ? aNewParam.nRow2 : rParam.nRow2;

    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScDBCollection> pUndoDB;

    if (bRecord) // secure old data
    {
        bool bOldFilter = bDo && rParam.bDoSort;

        SCTAB nTabCount = rDoc.GetTableCount();
        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo(rDoc, nTab, nTab, false, bOldFilter);

        //  secure data range - incl. filtering result
        rDoc.CopyToDocument(rParam.nCol1, rParam.nRow1 + 1, nTab, rParam.nCol2, nCaptureEndRow, nTab,
                            InsertDeleteFlags::ALL, false, *pUndoDoc);

        //  all formulas because of references
        rDoc.CopyToDocument(0, 0, 0, rDoc.MaxCol(), rDoc.MaxRow(), nTabCount - 1,
                            InsertDeleteFlags::FORMULA, false, *pUndoDoc);

        //  ranges of DB
        ScDBCollection* pDocDB = rDoc.GetDBCollection();
        if (!pDocDB->empty())
            pUndoDB.reset(new ScDBCollection(*pDocDB));
    }

    if (rParam.bReplace)
        rDoc.RemoveTableSubTotals(nTab, aNewParam, rParam, bInPlace);
    bool bSuccess = true;
    if (bDo)
    {
        bSuccess = rDoc.DoTableSubTotals(nTab, aNewParam, bInPlace);
        rDoc.SetDrawPageSize(nTab);
    }
    ScRange aDirtyRange(aNewParam.nCol1, aNewParam.nRow1, nTab, aNewParam.nCol2, aNewParam.nRow2,
                        nTab);
    rDoc.SetDirty(aDirtyRange, true);

    // Need to store with the new values
    *pDBData = rNewData;
    if (bRecord)
    {
        ScDBCollection* pDocDB = rDoc.GetDBCollection();
        rDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoTableTotals>(
            rDocShell, nTab, rParam, aNewParam.nRow2, std::move(pUndoDoc),
            std::move(pUndoDB), std::make_unique<ScDBCollection>(*pDocDB), bInPlace));
    }

    if (!bSuccess)
    {
        // "Cannot insert rows"
        if (!bApi)
            rDocShell.ErrorMessageAsync(STR_MSSG_DOSUBTOTALS_2);
    }

    // store
    pDBData->SetSubTotalParam(aNewParam);
    pDBData->SetArea(nTab, aNewParam.nCol1, aNewParam.nRow1, aNewParam.nCol2, aNewParam.nRow2);
    rDoc.CompileDBFormula();

    rDocShell.PostPaint(ScRange(0, 0, nTab, rDoc.MaxCol(), rDoc.MaxRow(), nTab),
                        PaintPartFlags::Grid | PaintPartFlags::Left | PaintPartFlags::Top
                            | PaintPartFlags::Size);
    aModificator.SetDocumentModified();
}

bool ScDBDocFunc::DataPilotUpdate( ScDPObject* pOldObj, const ScDPObject* pNewObj,
                                   bool bRecord, bool bApi, bool bAllowMove )
{
    if (!pOldObj)
    {
        if (!pNewObj)
            return false;

        sc::CreatePivotTableOperation aOperation(rDocShell, *pNewObj, bRecord, bApi);
        return aOperation.run();
    }

    if (!pNewObj)
    {
        sc::RemovePivotTableOperation aOperation(rDocShell, *pOldObj, bRecord, bApi);
        return aOperation.run();
    }

    if (pOldObj == pNewObj)
    {
        sc::UpdatePivotTableOperation aOperation(rDocShell, *pOldObj, bRecord, bApi);
        return aOperation.run();
    }

    sc::ReplacePivotTableOperation aOperation(rDocShell, pOldObj, pNewObj, bRecord, bApi,
                                              bAllowMove);
    return aOperation.run();
}

bool ScDBDocFunc::RemovePivotTable(const ScDPObject& rDPObj, bool bRecord, bool bApi)
{
    sc::RemovePivotTableOperation aOperation(rDocShell, rDPObj, bRecord, bApi);
    return aOperation.run();
}

bool ScDBDocFunc::CreatePivotTable(const ScDPObject& rDPObj, bool bRecord, bool bApi)
{
    sc::CreatePivotTableOperation aOperation(rDocShell, rDPObj, bRecord, bApi);
    return aOperation.run();
}

bool ScDBDocFunc::UpdatePivotTable(ScDPObject& rDPObj, bool bRecord, bool bApi)
{
    sc::UpdatePivotTableOperation aOperation(rDocShell, rDPObj, bRecord, bApi);
    return aOperation.run();
}

void ScDBDocFunc::RefreshPivotTables(const ScDPObject* pDPObj, bool bApi)
{
    ScDPCollection* pDPs = rDocShell.GetDocument().GetDPCollection();
    if (!pDPs)
        return;

    o3tl::sorted_vector<ScDPObject*> aRefs;
    TranslateId pErrId = pDPs->ReloadCache(pDPObj, aRefs);
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
    o3tl::sorted_vector<ScDPObject*> aRefs;
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
    const ScDBData* pData = rDBColl.getNamedDBs().findByUpperName(ScGlobal::getCharClass().uppercase(rTarget));
    if (!pData)
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(ScDocShell::GetActiveDialogParent(),
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
    if (!pViewSh)
        return;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
