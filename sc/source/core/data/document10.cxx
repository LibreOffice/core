/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <document.hxx>
#include <clipcontext.hxx>
#include <formulacell.hxx>
#include <clipparam.hxx>
#include <table.hxx>
#include <tokenarray.hxx>
#include <editutil.hxx>
#include <listenercontext.hxx>
#include <tokenstringcontext.hxx>
#include <poolhelp.hxx>

// Add totally brand-new methods to this source file.

void ScDocument::DeleteBeforeCopyFromClip( sc::CopyFromClipContext& rCxt, const ScMarkData& rMark )
{
    SCTAB nClipTab = 0;
    const TableContainer& rClipTabs = rCxt.getClipDoc()->maTabs;
    SCTAB nClipTabCount = rClipTabs.size();

    for (SCTAB nTab = rCxt.getTabStart(); nTab <= rCxt.getTabEnd(); ++nTab)
    {
        ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        if (!rMark.GetTableSelect(nTab))
            continue;

        while (!rClipTabs[nClipTab])
            nClipTab = (nClipTab+1) % nClipTabCount;

        pTab->DeleteBeforeCopyFromClip(rCxt, *rClipTabs[nClipTab]);

        nClipTab = (nClipTab+1) % nClipTabCount;
    }
}

void ScDocument::PreprocessRangeNameUpdate()
{
    sc::EndListeningContext aEndListenCxt(*this);
    sc::CompileFormulaContext aCompileCxt(this);

    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* p = *it;
        p->PreprocessRangeNameUpdate(aEndListenCxt, aCompileCxt);
    }
}

void ScDocument::PostprocessRangeNameUpdate()
{
    sc::CompileFormulaContext aCompileCxt(this);
    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* p = *it;
        p->PostprocessRangeNameUpdate(aCompileCxt);
    }
}

void ScDocument::SharePooledResources( ScDocument* pSrcDoc )
{
    xPoolHelper = pSrcDoc->xPoolHelper;
    mpCellStringPool = pSrcDoc->mpCellStringPool;
}

void ScDocument::UpdateScriptTypes( const ScAddress& rPos, SCCOL nColSize, SCROW nRowSize )
{
    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->UpdateScriptTypes(rPos.Col(), rPos.Row(), rPos.Col()+nColSize-1, rPos.Row()+nRowSize-1);
}

bool ScDocument::HasUniformRowHeight( SCTAB nTab, SCROW nRow1, SCROW nRow2 ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return false;

    return pTab->HasUniformRowHeight(nRow1, nRow2);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
