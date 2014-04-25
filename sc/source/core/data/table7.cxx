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

void ScTable::PostprocessRangeNameUpdate(
    sc::StartListeningContext& rStartListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    for (SCCOL i = 0; i <= MAXCOL; ++i)
        aCol[i].PostprocessRangeNameUpdate(rStartListenCxt, rCompileCxt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
