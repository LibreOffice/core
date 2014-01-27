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
        switch (rSrcCell.meType)
        {
            case CELLTYPE_VALUE:
            {
                std::vector<double> aVals(nDestSize, rSrcCell.mfValue);
                pBlockPos->miCellPos =
                    maCells.set(pBlockPos->miCellPos, nRow1, aVals.begin(), aVals.end());
                CellStorageModified();
            }
            break;
            case CELLTYPE_STRING:
            {
                std::vector<svl::SharedString> aStrs(nDestSize, *rSrcCell.mpString);
                maCells.set(pBlockPos->miCellPos, nRow1, aStrs.begin(), aStrs.end());
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
                CellStorageModified();
            }
            break;
            case CELLTYPE_FORMULA:
            {
                std::vector<ScFormulaCell*> aFormulas;
                ScAddress aPos(nCol, nRow1, nTab);
                aFormulas.reserve(nDestSize);
                ScFormulaCellGroupRef xGroup(new ScFormulaCellGroup);
                xGroup->setCode(*rSrcCell.mpFormula->GetCode());
                xGroup->compileCode(*pDocument, aPos, pDocument->GetGrammar());
                for (size_t i = 0; i < nDestSize; ++i)
                {
                    ScFormulaCell* pCell = new ScFormulaCell(pDocument, aPos, xGroup);
                    if (i == 0)
                    {
                        xGroup->mpTopCell = pCell;
                        xGroup->mnLength = nDestSize;
                    }
                    aFormulas.push_back(pCell);
                    aPos.IncRow();
                }

                pBlockPos->miCellPos =
                    maCells.set(pBlockPos->miCellPos, nRow1, aFormulas.begin(), aFormulas.end());

                // Join the top and bottom of the pasted formula cells as needed.
                sc::CellStoreType::position_type aPosObj =
                    maCells.position(pBlockPos->miCellPos, nRow1);

                assert(aPosObj.first->type == sc::element_type_formula);
                ScFormulaCell* pCell = sc::formula_block::at(*aPosObj.first->data, aPosObj.second);
                JoinNewFormulaCell(aPosObj, *pCell);

                aPosObj = maCells.position(aPosObj.first, nRow2);
                assert(aPosObj.first->type == sc::element_type_formula);
                pCell = sc::formula_block::at(*aPosObj.first->data, aPosObj.second);
                JoinNewFormulaCell(aPosObj, *pCell);

                CellStorageModified();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
