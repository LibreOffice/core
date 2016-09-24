/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "documentstreamaccess.hxx"
#include "document.hxx"
#include "table.hxx"
#include "column.hxx"
#include "mtvelements.hxx"

#include <svl/sharedstringpool.hxx>

namespace sc {

struct DocumentStreamAccessImpl
{
    ScDocument& mrDoc;
    ColumnBlockPositionSet maBlockPosSet;

    explicit DocumentStreamAccessImpl( ScDocument& rDoc ) :
        mrDoc(rDoc),
        maBlockPosSet(rDoc)
    {}
};

DocumentStreamAccess::DocumentStreamAccess( ScDocument& rDoc ) :
    mpImpl(new DocumentStreamAccessImpl(rDoc)) {}

DocumentStreamAccess::~DocumentStreamAccess()
{
}

void DocumentStreamAccess::setNumericCell( const ScAddress& rPos, double fVal )
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    // Set the numeric value.
    CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), fVal);

    // Be sure to set the corresponding text attribute to the default value.
    CellTextAttrStoreType& rAttrs = pTab->aCol[rPos.Col()].maCellTextAttrs;
    pBlockPos->miCellTextAttrPos = rAttrs.set(pBlockPos->miCellTextAttrPos, rPos.Row(), CellTextAttr());
}

void DocumentStreamAccess::setStringCell( const ScAddress& rPos, const OUString& rStr )
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    svl::SharedString aSS = mpImpl->mrDoc.GetSharedStringPool().intern(rStr);
    if (!aSS.getData())
        return;

    // Set the string.
    CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), aSS);

    // Be sure to set the corresponding text attribute to the default value.
    CellTextAttrStoreType& rAttrs = pTab->aCol[rPos.Col()].maCellTextAttrs;
    pBlockPos->miCellTextAttrPos = rAttrs.set(pBlockPos->miCellTextAttrPos, rPos.Row(), CellTextAttr());
}

void DocumentStreamAccess::reset()
{
    mpImpl->maBlockPosSet.clear();
}

void DocumentStreamAccess::shiftRangeUp( const ScRange& rRange )
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rRange.aStart.Tab());
    if (!pTab)
        return;

    SCROW nTopRow = rRange.aStart.Row();
    SCROW nLastRow = rRange.aEnd.Row();

    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        ColumnBlockPosition* pBlockPos =
            mpImpl->maBlockPosSet.getBlockPosition(rRange.aStart.Tab(), nCol);

        if (!pBlockPos)
            return;

        CellStoreType& rCells = pTab->aCol[nCol].maCells;
        rCells.erase(nTopRow, nTopRow); // Erase the top, and shift the rest up.
        pBlockPos->miCellPos = rCells.insert_empty(nLastRow, 1);

        // Do the same for the text attribute storage.
        CellTextAttrStoreType& rAttrs = pTab->aCol[nCol].maCellTextAttrs;
        rAttrs.erase(nTopRow, nTopRow);
        pBlockPos->miCellTextAttrPos = rAttrs.insert_empty(nLastRow, 1);
    }
}

void DocumentStreamAccess::shiftRangeDown( const ScRange& rRange )
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rRange.aStart.Tab());
    if (!pTab)
        return;

    SCROW nTopRow = rRange.aStart.Row();
    SCROW nLastRow = rRange.aEnd.Row();

    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        ColumnBlockPosition* pBlockPos =
            mpImpl->maBlockPosSet.getBlockPosition(rRange.aStart.Tab(), nCol);

        if (!pBlockPos)
            return;

        CellStoreType& rCells = pTab->aCol[nCol].maCells;
        rCells.erase(nLastRow, nLastRow); // Erase the bottom.
        pBlockPos->miCellPos = rCells.insert_empty(nTopRow, 1); // insert at the top and shift everything down.

        // Do the same for the text attribute storage.
        CellTextAttrStoreType& rAttrs = pTab->aCol[nCol].maCellTextAttrs;
        rAttrs.erase(nLastRow, nLastRow);
        pBlockPos->miCellTextAttrPos = rAttrs.insert_empty(nTopRow, 1);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
