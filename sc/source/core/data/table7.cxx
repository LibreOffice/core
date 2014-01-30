/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <table.hxx>

bool ScTable::IsMerged( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidCol(nCol))
        return false;

    return aCol[nCol].IsMerged(nRow);
}

void ScTable::CopyOneCellFromClip(
    sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].CopyOneCellFromClip(rCxt, nRow1, nRow2);
}

void ScTable::SetValues( SCCOL nCol, SCROW nRow, const std::vector<double>& rVals )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].SetValues(nRow, rVals);
}

void ScTable::TransferCellValuesTo( SCCOL nCol, SCROW nRow, size_t nLen, sc::CellValues& rDest )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].TransferCellValuesTo(nRow, nLen, rDest);
}

void ScTable::CopyCellValuesFrom( SCCOL nCol, SCROW nRow, const sc::CellValues& rSrc )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].CopyCellValuesFrom(nRow, rSrc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
