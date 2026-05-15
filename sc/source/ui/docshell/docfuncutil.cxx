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

#include <docfuncutil.hxx>
#include <docfunc.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <markdata.hxx>
#include <undobase.hxx>
#include <global.hxx>
#include <undoblk.hxx>
#include <columnspanset.hxx>

#include <memory>
#include <utility>

namespace sc {

bool DocFuncUtil::demolishMatrixMaster(ScDocFunc& rDocFunc, const ScDocument& rDoc,
                                       const ScAddress& rPos, bool bApi)
{
    const ScFormulaCell* pCell = rDoc.GetFormulaCell(rPos);
    if (!pCell || pCell->GetMatrixFlag() != ScMatrixMode::Formula)
        return false;
    SCCOL nCols = 0;
    SCROW nRows = 0;
    pCell->GetMatColsRows(nCols, nRows);
    if (nCols <= 1 && nRows <= 1)
        return false;

    ScMarkData aMark(rDoc.GetSheetLimits());
    aMark.SelectOneTable(rPos.Tab());
    aMark.SetMarkArea(ScRange(rPos));
    rDocFunc.DeleteContents(aMark, InsertDeleteFlags::CONTENTS,
                            /*bRecord*/ true, bApi);
    return true;
}

bool DocFuncUtil::hasProtectedTab( const ScDocument& rDoc, const ScMarkData& rMark )
{
    SCTAB nTabCount = rDoc.GetTableCount();
    for (const auto& rTab : rMark)
    {
        if (rTab >= nTabCount)
            break;

        if (rDoc.IsTabProtected(rTab))
            return true;
    }

    return false;
}

ScDocumentUniquePtr DocFuncUtil::createDeleteContentsUndoDoc(
    ScDocument& rDoc, const ScMarkData& rMark, const ScRange& rRange,
    InsertDeleteFlags nFlags, bool bOnlyMarked )
{
    ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
    SCTAB nTab = rRange.aStart.Tab();
    pUndoDoc->InitUndo(rDoc, nTab, nTab);
    SCTAB nTabCount = rDoc.GetTableCount();
    for (const auto& rTab : rMark)
        if (rTab != nTab)
            pUndoDoc->AddUndoTab( rTab, rTab );
    ScRange aCopyRange = rRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);

    //  in case of "Format/Standard" copy all attributes, because CopyToDocument
    //  with InsertDeleteFlags::HARDATTR only is too time-consuming:
    InsertDeleteFlags nUndoDocFlags = nFlags;
    if (nFlags & InsertDeleteFlags::ATTRIB)
        nUndoDocFlags |= InsertDeleteFlags::ATTRIB;
    if (nFlags & InsertDeleteFlags::EDITATTR)          // Edit-Engine-Attribute
        nUndoDocFlags |= InsertDeleteFlags::STRING;    // -> cells will be changed
    if (nFlags & InsertDeleteFlags::NOTE)
        nUndoDocFlags |= InsertDeleteFlags::CONTENTS;  // copy all cells with their notes
    // do not copy note captions to undo document
    nUndoDocFlags |= InsertDeleteFlags::NOCAPTIONS;
    rDoc.CopyToDocument(aCopyRange, nUndoDocFlags, bOnlyMarked, *pUndoDoc, &rMark);

    return pUndoDoc;
}

void DocFuncUtil::addDeleteContentsUndo(
    SfxUndoManager* pUndoMgr, ScDocShell& rDocSh, const ScMarkData& rMark,
    const ScRange& rRange, ScDocumentUniquePtr&& pUndoDoc, InsertDeleteFlags nFlags,
    const std::shared_ptr<ScSimpleUndo::DataSpansType>& pSpans,
    bool bMulti, bool bDrawUndo,
    std::vector<ScAddress> const& rRestoreExpandedMatrices)
{
    std::unique_ptr<ScUndoDeleteContents> pUndo(
        new ScUndoDeleteContents(
            rDocSh, rMark, rRange, std::move(pUndoDoc), bMulti, nFlags, bDrawUndo));
    pUndo->SetDataSpans(pSpans);
    pUndo->SetRestoreExpandedMatrices(rRestoreExpandedMatrices);

    pUndoMgr->AddUndoAction(std::move(pUndo));
}

std::shared_ptr<ScSimpleUndo::DataSpansType> DocFuncUtil::getNonEmptyCellSpans(
    const ScDocument& rDoc, const ScMarkData& rMark, const ScRange& rRange )
{
    auto pDataSpans = std::make_shared<ScSimpleUndo::DataSpansType>();
    for (const SCTAB nTab : rMark)
    {
        SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
        SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();

        std::pair<ScSimpleUndo::DataSpansType::iterator,bool> r =
            pDataSpans->insert(std::make_pair(nTab, std::make_unique<sc::ColumnSpanSet>()));

        if (r.second)
        {
            sc::ColumnSpanSet *const pSet = r.first->second.get();
            pSet->scan(rDoc, nTab, nCol1, nRow1, nCol2, nRow2, true);
        }
    }

    return pDataSpans;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
