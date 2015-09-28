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
#include <document.hxx>
#include <markdata.hxx>
#include <undobase.hxx>
#include <global.hxx>
#include <undoblk.hxx>

#include <memory>
#include <utility>

namespace sc {

bool DocFuncUtil::hasProtectedTab( const ScDocument& rDoc, const ScMarkData& rMark )
{
    SCTAB nTabCount = rDoc.GetTableCount();
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
        if (rDoc.IsTabProtected(*itr))
            return true;

    return false;
}

std::unique_ptr<ScDocument> DocFuncUtil::createDeleteContentsUndoDoc(
    ScDocument& rDoc, const ScMarkData& rMark, const ScRange& rRange,
    InsertDeleteFlags nFlags, bool bOnlyMarked )
{
    std::unique_ptr<ScDocument> pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
    SCTAB nTab = rRange.aStart.Tab();
    pUndoDoc->InitUndo(&rDoc, nTab, nTab);
    SCTAB nTabCount = rDoc.GetTableCount();
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd; ++itr)
        if (*itr != nTab)
            pUndoDoc->AddUndoTab( *itr, *itr );
    ScRange aCopyRange = rRange;
    aCopyRange.aStart.SetTab(0);
    aCopyRange.aEnd.SetTab(nTabCount-1);

    //  in case of "Format/Standard" copy all attributes, because CopyToDocument
    //  with IDF_HARDATTR only is too time-consuming:
    InsertDeleteFlags nUndoDocFlags = nFlags;
    if (nFlags & IDF_ATTRIB)
        nUndoDocFlags |= IDF_ATTRIB;
    if (nFlags & IDF_EDITATTR)          // Edit-Engine-Attribute
        nUndoDocFlags |= IDF_STRING;    // -> cells will be changed
    if (nFlags & IDF_NOTE)
        nUndoDocFlags |= IDF_CONTENTS;  // copy all cells with their notes
    // do not copy note captions to undo document
    nUndoDocFlags |= IDF_NOCAPTIONS;
    rDoc.CopyToDocument(aCopyRange, nUndoDocFlags, bOnlyMarked, pUndoDoc.get(), &rMark);

    return pUndoDoc;
}

void DocFuncUtil::addDeleteContentsUndo(
    svl::IUndoManager* pUndoMgr, ScDocShell* pDocSh, const ScMarkData& rMark,
    const ScRange& rRange, std::unique_ptr<ScDocument>&& pUndoDoc, InsertDeleteFlags nFlags,
    const std::shared_ptr<ScSimpleUndo::DataSpansType>& pSpans,
    bool bMulti, bool bDrawUndo )
{
    std::unique_ptr<ScUndoDeleteContents> pUndo(
        new ScUndoDeleteContents(
            pDocSh, rMark, rRange, std::move(pUndoDoc), bMulti, nFlags, bDrawUndo));
    pUndo->SetDataSpans(pSpans);

    pUndoMgr->AddUndoAction(pUndo.release());
}

std::unique_ptr<ScSimpleUndo::DataSpansType> DocFuncUtil::getNonEmptyCellSpans(
    const ScDocument& rDoc, const ScMarkData& rMark, const ScRange& rRange )
{
    std::unique_ptr<ScSimpleUndo::DataSpansType> pDataSpans(new ScSimpleUndo::DataSpansType);
    ScMarkData::const_iterator it = rMark.begin(), itEnd = rMark.end();
    for (; it != itEnd; ++it)
    {
        SCTAB nTab = *it;

        SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
        SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();

        std::pair<ScSimpleUndo::DataSpansType::iterator,bool> r =
            pDataSpans->insert(nTab, new sc::ColumnSpanSet(false));

        if (r.second)
        {
            sc::ColumnSpanSet* pSet = r.first->second;
            pSet->scan(rDoc, nTab, nCol1, nRow1, nCol2, nRow2, true);
        }
    }

    return pDataSpans;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
