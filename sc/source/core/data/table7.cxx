/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <table.hxx>
#include <clipcontext.hxx>
#include <document.hxx>
#include <clipparam.hxx>
#include <bcaslot.hxx>
#include <segmenttree.hxx>
#include <sharedformula.hxx>

void ScTable::DeleteBeforeCopyFromClip( sc::CopyFromClipContext& rCxt, const ScTable& rClipTab )
{
    sc::CopyFromClipContext::Range aRange = rCxt.getDestRange();
    if (!ValidCol(aRange.mnCol1) || !ValidCol(aRange.mnCol2))
        return;

    // Pass some stuff to the columns via context.
    rCxt.setTableProtected(IsProtected());
    rCxt.setCondFormatList(mpCondFormatList.get());

    ScRange aClipRange = rCxt.getClipDoc()->GetClipParam().getWholeRange();
    SCCOL nClipCol = aClipRange.aStart.Col();
    {
        ScBulkBroadcast aBulkBroadcast(pDocument->GetBASM());

        for (SCCOL nCol = aRange.mnCol1; nCol <= aRange.mnCol2; ++nCol, ++nClipCol)
        {
            if (nClipCol > aClipRange.aEnd.Col())
                nClipCol = aClipRange.aStart.Col(); // loop through columns.

            const ScColumn& rClipCol = rClipTab.aCol[nClipCol];
            aCol[nCol].DeleteBeforeCopyFromClip(rCxt, rClipCol);
        }
    }

    SetStreamValid(false);
}

void ScTable::PreprocessRangeNameUpdate(
    sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    for (SCCOL i = 0; i <= MAXCOL; ++i)
        aCol[i].PreprocessRangeNameUpdate(rEndListenCxt, rCompileCxt);
}

void ScTable::PreprocessDBDataUpdate(
    sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    for (SCCOL i = 0; i <= MAXCOL; ++i)
        aCol[i].PreprocessDBDataUpdate(rEndListenCxt, rCompileCxt);
}

void ScTable::CompileHybridFormula(
    sc::StartListeningContext& rStartListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    for (SCCOL i = 0; i <= MAXCOL; ++i)
        aCol[i].CompileHybridFormula(rStartListenCxt, rCompileCxt);
}

void ScTable::UpdateScriptTypes( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    if (!ValidCol(nCol1) || !ValidCol(nCol2) || nCol1 > nCol2)
        return;

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].UpdateScriptTypes(nRow1, nRow2);
}

bool ScTable::HasUniformRowHeight( SCROW nRow1, SCROW nRow2 ) const
{
    if (!ValidRow(nRow1) || !ValidRow(nRow2) || nRow1 > nRow2)
        return false;

    ScFlatUInt16RowSegments::RangeData aData;
    if (!mpRowHeights->getRangeData(nRow1, aData))
        // Search failed.
        return false;

    return nRow2 <= aData.mnRow2;
}

void ScTable::UnshareFormulaCells( SCCOL nCol, std::vector<SCROW>& rRows )
{
    if (!ValidCol(nCol))
        return;

    sc::SharedFormulaUtil::unshareFormulaCells(aCol[nCol].maCells, rRows);
}

void ScTable::RegroupFormulaCells( SCCOL nCol )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].RegroupFormulaCells();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
