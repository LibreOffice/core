/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <column.hxx>
#include <clipcontext.hxx>
#include <clipparam.hxx>
#include <cellvalue.hxx>
#include <attarray.hxx>
#include <document.hxx>
#include <cellvalues.hxx>
#include <columnspanset.hxx>
#include <listenercontext.hxx>

#include <svl/sharedstring.hxx>

#include <vector>
#include <cassert>

bool ScColumn::IsMerged( SCROW nRow ) const
{
    return pAttrArray->IsMerged(nRow);
}

void ScColumn::CopyOneCellFromClip( sc::CopyFromClipContext& rCxt, SCROW nRow1, SCROW nRow2 )
{
    assert(nRow1 <= nRow2);

    size_t nDestSize = nRow2 - nRow1 + 1;
    sc::ColumnBlockPosition* pBlockPos = rCxt.getBlockPosition(nTab, nCol);
    if (!pBlockPos)
        return;

    ScCellValue& rSrcCell = rCxt.getSingleCell();

    sal_uInt16 nFlags = rCxt.getInsertFlag();

    if ((nFlags & IDF_ATTRIB) != 0)
    {
        if (!rCxt.isSkipAttrForEmptyCells() || rSrcCell.meType != CELLTYPE_NONE)
        {
            const ScPatternAttr* pAttr = rCxt.getSingleCellPattern();
            pAttrArray->SetPatternArea(nRow1, nRow2, pAttr, true);
        }
    }

    if ((nFlags & IDF_CONTENTS) != 0)
    {
        std::vector<sc::CellTextAttr> aTextAttrs(nDestSize);

        switch (rSrcCell.meType)
        {
            case CELLTYPE_VALUE:
            {
                std::vector<double> aVals(nDestSize, rSrcCell.mfValue);
                pBlockPos->miCellPos =
                    maCells.set(pBlockPos->miCellPos, nRow1, aVals.begin(), aVals.end());
                pBlockPos->miCellTextAttrPos =
                    maCellTextAttrs.set(pBlockPos->miCellTextAttrPos, nRow1, aTextAttrs.begin(), aTextAttrs.end());
                CellStorageModified();
            }
            break;
            case CELLTYPE_STRING:
            {
                std::vector<svl::SharedString> aStrs(nDestSize, *rSrcCell.mpString);
                pBlockPos->miCellPos =
                    maCells.set(pBlockPos->miCellPos, nRow1, aStrs.begin(), aStrs.end());
                pBlockPos->miCellTextAttrPos =
                    maCellTextAttrs.set(pBlockPos->miCellTextAttrPos, nRow1, aTextAttrs.begin(), aTextAttrs.end());
                CellStorageModified();
            }
            break;
            case CELLTYPE_EDIT:
            {
                std::vector<EditTextObject*> aStrs;
                aStrs.reserve(nDestSize);
                for (size_t i = 0; i < nDestSize; ++i)
                    aStrs.push_back(rSrcCell.mpEditText->Clone());

                pBlockPos->miCellPos =
                    maCells.set(pBlockPos->miCellPos, nRow1, aStrs.begin(), aStrs.end());
                pBlockPos->miCellTextAttrPos =
                    maCellTextAttrs.set(pBlockPos->miCellTextAttrPos, nRow1, aTextAttrs.begin(), aTextAttrs.end());
                CellStorageModified();
            }
            break;
            case CELLTYPE_FORMULA:
            {
                std::vector<sc::RowSpan> aRanges;
                aRanges.reserve(1);
                aRanges.push_back(sc::RowSpan(nRow1, nRow2));
                CloneFormulaCell(*rSrcCell.mpFormula, aRanges);
            }
            break;
            default:
                ;
        }
    }

    const ScPostIt* pNote = rCxt.getSingleCellNote();
    if (pNote && (nFlags & (IDF_NOTE | IDF_ADDNOTES)) != 0)
    {
        // Duplicate the cell note over the whole pasted range.

        ScDocument* pClipDoc = rCxt.getClipDoc();
        const ScAddress& rSrcPos = pClipDoc->GetClipParam().getWholeRange().aStart;
        std::vector<ScPostIt*> aNotes;
        ScAddress aDestPos(nCol, nRow1, nTab);
        aNotes.reserve(nDestSize);
        for (size_t i = 0; i < nDestSize; ++i)
        {
            bool bCloneCaption = (nFlags & IDF_NOCAPTIONS) == 0;
            aNotes.push_back(pNote->Clone(rSrcPos, *pDocument, aDestPos, bCloneCaption));
            aDestPos.IncRow();
        }

        pBlockPos->miCellNotePos =
            maCellNotes.set(
                pBlockPos->miCellNotePos, nRow1, aNotes.begin(), aNotes.end());
    }
}

void ScColumn::SetValues( SCROW nRow, const std::vector<double>& rVals )
{
    if (!ValidRow(nRow))
        return;

    SCROW nLastRow = nRow + rVals.size() - 1;
    if (nLastRow > MAXROW)
        // Out of bound. Do nothing.
        return;

    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    DetachFormulaCells(aPos, rVals.size());

    maCells.set(nRow, rVals.begin(), rVals.end());
    std::vector<sc::CellTextAttr> aDefaults(rVals.size());
    maCellTextAttrs.set(nRow, aDefaults.begin(), aDefaults.end());

    CellStorageModified();

    std::vector<SCROW> aRows;
    aRows.reserve(rVals.size());
    for (SCROW i = nRow; i <= nLastRow; ++i)
        aRows.push_back(i);

    BroadcastCells(aRows, SC_HINT_DATACHANGED);
}

void ScColumn::TransferCellValuesTo( SCROW nRow, size_t nLen, sc::CellValues& rDest )
{
    if (!ValidRow(nRow))
        return;

    SCROW nLastRow = nRow + nLen - 1;
    if (nLastRow > MAXROW)
        // Out of bound. Do nothing.
        return;

    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    DetachFormulaCells(aPos, nLen);

    rDest.transferFrom(*this, nRow, nLen);

    std::vector<sc::CellTextAttr> aDefaults(nLen);
    maCellTextAttrs.set(nRow, aDefaults.begin(), aDefaults.end());

    CellStorageModified();

    std::vector<SCROW> aRows;
    aRows.reserve(nLen);
    for (SCROW i = nRow; i <= nLastRow; ++i)
        aRows.push_back(i);

    BroadcastCells(aRows, SC_HINT_DATACHANGED);
}

void ScColumn::CopyCellValuesFrom( SCROW nRow, const sc::CellValues& rSrc )
{
    if (!ValidRow(nRow))
        return;

    SCROW nLastRow = nRow + rSrc.size() - 1;
    if (nLastRow > MAXROW)
        // Out of bound. Do nothing
        return;

    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    DetachFormulaCells(aPos, rSrc.size());

    rSrc.copyTo(*this, nRow);

    std::vector<sc::CellTextAttr> aDefaults(rSrc.size());
    maCellTextAttrs.set(nRow, aDefaults.begin(), aDefaults.end());

    CellStorageModified();

    std::vector<SCROW> aRows;
    aRows.reserve(rSrc.size());
    for (SCROW i = nRow; i <= nLastRow; ++i)
        aRows.push_back(i);

    BroadcastCells(aRows, SC_HINT_DATACHANGED);
}

void ScColumn::DeleteRanges( const std::vector<sc::RowSpan>& rRanges, sal_uInt16 nDelFlag, bool bBroadcast )
{
    std::vector<sc::RowSpan>::const_iterator itSpan = rRanges.begin(), itSpanEnd = rRanges.end();
    for (; itSpan != itSpanEnd; ++itSpan)
        DeleteArea(itSpan->mnRow1, itSpan->mnRow2, nDelFlag, bBroadcast);
}

void ScColumn::CloneFormulaCell( const ScFormulaCell& rSrc, const std::vector<sc::RowSpan>& rRanges )
{
    sc::CellStoreType::iterator itPos = maCells.begin();
    sc::CellTextAttrStoreType::iterator itAttrPos = maCellTextAttrs.begin();
    sc::StartListeningContext aCxt(*pDocument);

    std::vector<ScFormulaCell*> aFormulas;
    std::vector<sc::RowSpan>::const_iterator itSpan = rRanges.begin(), itSpanEnd = rRanges.end();
    for (; itSpan != itSpanEnd; ++itSpan)
    {
        SCROW nRow1 = itSpan->mnRow1, nRow2 = itSpan->mnRow2;
        size_t nLen = nRow2 - nRow1 + 1;
        aFormulas.clear();
        aFormulas.reserve(nLen);

        ScAddress aPos(nCol, nRow1, nTab);
        ScFormulaCellGroupRef xGroup(new ScFormulaCellGroup);
        xGroup->setCode(*rSrc.GetCode());
        xGroup->compileCode(*pDocument, aPos, pDocument->GetGrammar());
        for (size_t i = 0; i < nLen; ++i, aPos.IncRow())
        {
            ScFormulaCell* pCell = new ScFormulaCell(pDocument, aPos, xGroup);
            if (i == 0)
            {
                xGroup->mpTopCell = pCell;
                xGroup->mnLength = nLen;
            }
            pCell->StartListeningTo(aCxt);
            pCell->SetDirty();
            aFormulas.push_back(pCell);
        }

        itPos = maCells.set(itPos, nRow1, aFormulas.begin(), aFormulas.end());

        // Join the top and bottom of the pasted formula cells as needed.
        sc::CellStoreType::position_type aPosObj = maCells.position(itPos, nRow1);

        assert(aPosObj.first->type == sc::element_type_formula);
        ScFormulaCell* pCell = sc::formula_block::at(*aPosObj.first->data, aPosObj.second);
        JoinNewFormulaCell(aPosObj, *pCell);

        aPosObj = maCells.position(aPosObj.first, nRow2);
        assert(aPosObj.first->type == sc::element_type_formula);
        pCell = sc::formula_block::at(*aPosObj.first->data, aPosObj.second);
        JoinNewFormulaCell(aPosObj, *pCell);

        std::vector<sc::CellTextAttr> aTextAttrs(nLen);
        itAttrPos = maCellTextAttrs.set(itAttrPos, nRow1, aTextAttrs.begin(), aTextAttrs.end());
    }

    CellStorageModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
