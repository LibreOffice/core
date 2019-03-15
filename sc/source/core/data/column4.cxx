/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <column.hxx>
#include <clipparam.hxx>
#include <cellvalue.hxx>
#include <attarray.hxx>
#include <document.hxx>
#include <cellvalues.hxx>
#include <columnspanset.hxx>
#include <columniterator.hxx>
#include <mtvcellfunc.hxx>
#include <clipcontext.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <conditio.hxx>
#include <formulagroup.hxx>
#include <tokenarray.hxx>
#include <scitems.hxx>
#include <cellform.hxx>
#include <sharedformula.hxx>
#include <drwlayer.hxx>
#include <compiler.hxx>
#include <recursionhelper.hxx>

#include <svl/sharedstringpool.hxx>
#include <sal/log.hxx>

#include <numeric>
#include <vector>
#include <cassert>

bool ScColumn::IsMerged( SCROW nRow ) const
{
    return pAttrArray->IsMerged(nRow);
}

sc::MultiDataCellState::StateType ScColumn::HasDataCellsInRange(
    SCROW nRow1, SCROW nRow2, SCROW* pRow1 ) const
{
    sc::CellStoreType::const_position_type aPos = maCells.position(nRow1);
    sc::CellStoreType::const_iterator it = aPos.first;
    size_t nOffset = aPos.second;
    SCROW nRow = nRow1;
    bool bHasOne = false; // whether or not we have found a non-empty block of size one.

    for (; it != maCells.end() && nRow <= nRow2; ++it)
    {
        if (it->type != sc::element_type_empty)
        {
            // non-empty block found.
            assert(it->size > 0); // mtv should never contain a block of zero length.
            size_t nSize = it->size - nOffset;

            SCROW nLastRow = nRow + nSize - 1;
            if (nLastRow > nRow2)
                // shrink the size to avoid exceeding the specified last row position.
                nSize -= nLastRow - nRow2;

            if (nSize == 1)
            {
                // this block is of size one.
                if (bHasOne)
                    return sc::MultiDataCellState::HasMultipleCells;

                bHasOne = true;
                if (pRow1)
                    *pRow1 = nRow;
            }
            else
            {
                // size of this block is greater than one.
                if (pRow1)
                    *pRow1 = nRow;
                return sc::MultiDataCellState::HasMultipleCells;
            }
        }

        nRow += it->size - nOffset;
        nOffset = 0;
    }

    return bHasOne ? sc::MultiDataCellState::HasOneCell : sc::MultiDataCellState::Empty;
}

void ScColumn::DeleteBeforeCopyFromClip(
    sc::CopyFromClipContext& rCxt, const ScColumn& rClipCol, sc::ColumnSpanSet& rBroadcastSpans )
{
    sc::CopyFromClipContext::Range aRange = rCxt.getDestRange();
    if (!ValidRow(aRange.mnRow1) || !ValidRow(aRange.mnRow2))
        return;

    ScRange aClipRange = rCxt.getClipDoc()->GetClipParam().getWholeRange();
    SCROW nClipRow1 = aClipRange.aStart.Row();
    SCROW nClipRow2 = aClipRange.aEnd.Row();
    SCROW nClipRowLen = nClipRow2 - nClipRow1 + 1;

    // Check for non-empty cell ranges in the clip column.
    sc::SingleColumnSpanSet aSpanSet;
    aSpanSet.scan(rClipCol, nClipRow1, nClipRow2);
    sc::SingleColumnSpanSet::SpansType aSpans;
    aSpanSet.getSpans(aSpans);

    if (aSpans.empty())
        // All cells in the range in the clip are empty.  Nothing to delete.
        return;

    // Translate the clip column spans into the destination column, and repeat as needed.
    ScDocument* pDocument = GetDoc();
    std::vector<sc::RowSpan> aDestSpans;
    SCROW nDestOffset = aRange.mnRow1 - nClipRow1;
    bool bContinue = true;
    while (bContinue)
    {
        for (const sc::RowSpan& r : aSpans)
        {
            SCROW nDestRow1 = r.mnRow1 + nDestOffset;
            SCROW nDestRow2 = r.mnRow2 + nDestOffset;

            if (nDestRow1 > aRange.mnRow2)
            {
                // We're done.
                bContinue = false;
                break;
            }

            if (nDestRow2 > aRange.mnRow2)
            {
                // Truncate this range, and set it as the last span.
                nDestRow2 = aRange.mnRow2;
                bContinue = false;
            }

            aDestSpans.emplace_back(nDestRow1, nDestRow2);

            if (!bContinue)
                break;
        }

        nDestOffset += nClipRowLen;
    }

    InsertDeleteFlags nDelFlag = rCxt.getDeleteFlag();
    sc::ColumnBlockPosition aBlockPos;
    InitBlockPosition(aBlockPos);

    for (const auto& rDestSpan : aDestSpans)
    {
        SCROW nRow1 = rDestSpan.mnRow1;
        SCROW nRow2 = rDestSpan.mnRow2;

        if (nDelFlag & InsertDeleteFlags::CONTENTS)
        {
            sc::SingleColumnSpanSet aDeletedRows;
            DeleteCells(aBlockPos, nRow1, nRow2, nDelFlag, aDeletedRows);
            rBroadcastSpans.set(nTab, nCol, aDeletedRows, true);
        }

        if (nDelFlag & InsertDeleteFlags::NOTE)
            DeleteCellNotes(aBlockPos, nRow1, nRow2, false);

        if (nDelFlag & InsertDeleteFlags::EDITATTR)
            RemoveEditAttribs(nRow1, nRow2);

        // Delete attributes just now
        if (nDelFlag & InsertDeleteFlags::ATTRIB)
        {
            pAttrArray->DeleteArea(nRow1, nRow2);

            if (rCxt.isTableProtected())
            {
                ScPatternAttr aPattern(pDocument->GetPool());
                aPattern.GetItemSet().Put(ScProtectionAttr(false));
                ApplyPatternArea(nRow1, nRow2, aPattern);
            }

            ScConditionalFormatList* pCondList = rCxt.getCondFormatList();
            if (pCondList)
                pCondList->DeleteArea(nCol, nRow1, nCol, nRow2);
        }
        else if ((nDelFlag & InsertDeleteFlags::HARDATTR) == InsertDeleteFlags::HARDATTR)
            pAttrArray->DeleteHardAttr(nRow1, nRow2);
    }
}

void ScColumn::CopyOneCellFromClip( sc::CopyFromClipContext& rCxt, SCROW nRow1, SCROW nRow2, size_t nColOffset )
{
    assert(nRow1 <= nRow2);

    size_t nDestSize = nRow2 - nRow1 + 1;
    sc::ColumnBlockPosition* pBlockPos = rCxt.getBlockPosition(nTab, nCol);
    if (!pBlockPos)
        return;

    ScDocument* pDocument = GetDoc();
    bool bSameDocPool = (rCxt.getClipDoc()->GetPool() == pDocument->GetPool());

    ScCellValue& rSrcCell = rCxt.getSingleCell(nColOffset);
    sc::CellTextAttr& rSrcAttr = rCxt.getSingleCellAttr(nColOffset);

    InsertDeleteFlags nFlags = rCxt.getInsertFlag();

    if ((nFlags & InsertDeleteFlags::ATTRIB) != InsertDeleteFlags::NONE)
    {
        if (!rCxt.isSkipAttrForEmptyCells() || rSrcCell.meType != CELLTYPE_NONE)
        {
            const ScPatternAttr* pAttr = (bSameDocPool ? rCxt.getSingleCellPattern(nColOffset) :
                    rCxt.getSingleCellPattern(nColOffset)->PutInPool( pDocument, rCxt.getClipDoc()));

            ScPatternAttr aNewPattern(*pAttr);
            sal_uInt16 pItems[2];
            pItems[0] = ATTR_CONDITIONAL;
            pItems[1] = 0;
            aNewPattern.ClearItems(pItems);
            pAttrArray->SetPatternArea(nRow1, nRow2, &aNewPattern, true);
        }
    }

    if ((nFlags & InsertDeleteFlags::CONTENTS) != InsertDeleteFlags::NONE)
    {
        std::vector<sc::CellTextAttr> aTextAttrs(nDestSize, rSrcAttr);

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
                // Compare the ScDocumentPool* to determine if we are copying within the
                // same document. If not, re-intern shared strings.
                svl::SharedStringPool* pSharedStringPool = (bSameDocPool ? nullptr : &pDocument->GetSharedStringPool());
                svl::SharedString aStr = (pSharedStringPool ?
                        pSharedStringPool->intern( rSrcCell.mpString->getString()) :
                        *rSrcCell.mpString);

                std::vector<svl::SharedString> aStrs(nDestSize, aStr);
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
                    aStrs.push_back(rSrcCell.mpEditText->Clone().release());

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
                aRanges.emplace_back(nRow1, nRow2);
                CloneFormulaCell(*rSrcCell.mpFormula, rSrcAttr, aRanges);
            }
            break;
            default:
                ;
        }
    }

    const ScPostIt* pNote = rCxt.getSingleCellNote(nColOffset);
    if (pNote && (nFlags & (InsertDeleteFlags::NOTE | InsertDeleteFlags::ADDNOTES)) != InsertDeleteFlags::NONE)
    {
        // Duplicate the cell note over the whole pasted range.

        ScDocument* pClipDoc = rCxt.getClipDoc();
        const ScAddress& rSrcPos = pClipDoc->GetClipParam().getWholeRange().aStart;
        std::vector<ScPostIt*> aNotes;
        ScAddress aDestPos(nCol, nRow1, nTab);
        aNotes.reserve(nDestSize);
        for (size_t i = 0; i < nDestSize; ++i)
        {
            bool bCloneCaption = (nFlags & InsertDeleteFlags::NOCAPTIONS) == InsertDeleteFlags::NONE;
            aNotes.push_back(pNote->Clone(rSrcPos, *pDocument, aDestPos, bCloneCaption).release());
            aDestPos.IncRow();
        }

        pBlockPos->miCellNotePos =
            maCellNotes.set(
                pBlockPos->miCellNotePos, nRow1, aNotes.begin(), aNotes.end());
    }
}

void ScColumn::SetValues( const SCROW nRow, const std::vector<double>& rVals )
{
    if (!ValidRow(nRow))
        return;

    SCROW nLastRow = nRow + rVals.size() - 1;
    if (nLastRow > MAXROW)
        // Out of bound. Do nothing.
        return;

    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    std::vector<SCROW> aNewSharedRows;
    DetachFormulaCells(aPos, rVals.size(), &aNewSharedRows);

    maCells.set(nRow, rVals.begin(), rVals.end());
    std::vector<sc::CellTextAttr> aDefaults(rVals.size());
    maCellTextAttrs.set(nRow, aDefaults.begin(), aDefaults.end());

    CellStorageModified();

    StartListeningUnshared( aNewSharedRows);

    std::vector<SCROW> aRows;
    aRows.reserve(rVals.size());
    for (SCROW i = nRow; i <= nLastRow; ++i)
        aRows.push_back(i);

    BroadcastCells(aRows, SfxHintId::ScDataChanged);
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
    DetachFormulaCells(aPos, nLen, nullptr);

    rDest.transferFrom(*this, nRow, nLen);

    CellStorageModified();

    std::vector<SCROW> aRows;
    aRows.reserve(nLen);
    for (SCROW i = nRow; i <= nLastRow; ++i)
        aRows.push_back(i);

    BroadcastCells(aRows, SfxHintId::ScDataChanged);
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
    DetachFormulaCells(aPos, rSrc.size(), nullptr);

    rSrc.copyTo(*this, nRow);

    CellStorageModified();

    std::vector<SCROW> aRows;
    aRows.reserve(rSrc.size());
    for (SCROW i = nRow; i <= nLastRow; ++i)
        aRows.push_back(i);

    BroadcastCells(aRows, SfxHintId::ScDataChanged);
}

namespace {

class ConvertFormulaToValueHandler
{
    sc::CellValues maResValues;
    bool mbModified;

public:
    ConvertFormulaToValueHandler() :
        mbModified(false)
    {
        maResValues.reset(MAXROWCOUNT);
    }

    void operator() ( size_t nRow, const ScFormulaCell* pCell )
    {
        sc::FormulaResultValue aRes = pCell->GetResult();
        switch (aRes.meType)
        {
            case sc::FormulaResultValue::Value:
                maResValues.setValue(nRow, aRes.mfValue);
            break;
            case sc::FormulaResultValue::String:
                maResValues.setValue(nRow, aRes.maString);
            break;
            case sc::FormulaResultValue::Error:
            case sc::FormulaResultValue::Invalid:
            default:
                maResValues.setValue(nRow, svl::SharedString::getEmptyString());
        }

        mbModified = true;
    }

    bool isModified() const { return mbModified; }

    sc::CellValues& getResValues() { return maResValues; }
};

}

void ScColumn::ConvertFormulaToValue(
    sc::EndListeningContext& rCxt, SCROW nRow1, SCROW nRow2, sc::TableValues* pUndo )
{
    if (!ValidRow(nRow1) || !ValidRow(nRow2) || nRow1 > nRow2)
        return;

    std::vector<SCROW> aBounds;
    aBounds.push_back(nRow1);
    if (nRow2 < MAXROW-1)
        aBounds.push_back(nRow2+1);

    // Split formula cell groups at top and bottom boundaries (if applicable).
    sc::SharedFormulaUtil::splitFormulaCellGroups(maCells, aBounds);

    // Parse all formulas within the range and store their results into temporary storage.
    ConvertFormulaToValueHandler aFunc;
    sc::ParseFormula(maCells.begin(), maCells, nRow1, nRow2, aFunc);
    if (!aFunc.isModified())
        // No formula cells encountered.
        return;

    DetachFormulaCells(rCxt, nRow1, nRow2, nullptr);

    // Undo storage to hold static values which will get swapped to the cell storage later.
    sc::CellValues aUndoCells;
    aFunc.getResValues().swap(aUndoCells);
    aUndoCells.swapNonEmpty(*this);
    if (pUndo)
        pUndo->swap(nTab, nCol, aUndoCells);
}

namespace {

class StartListeningHandler
{
    sc::StartListeningContext& mrCxt;

public:
    explicit StartListeningHandler( sc::StartListeningContext& rCxt ) :
        mrCxt(rCxt) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->StartListeningTo(mrCxt);
    }
};

class EndListeningHandler
{
    sc::EndListeningContext& mrCxt;

public:
    explicit EndListeningHandler( sc::EndListeningContext& rCxt ) :
        mrCxt(rCxt) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->EndListeningTo(mrCxt);
    }
};

}

void ScColumn::SwapNonEmpty(
    sc::TableValues& rValues, sc::StartListeningContext& rStartCxt, sc::EndListeningContext& rEndCxt )
{
    const ScRange& rRange = rValues.getRange();
    std::vector<SCROW> aBounds;
    aBounds.push_back(rRange.aStart.Row());
    if (rRange.aEnd.Row() < MAXROW-1)
        aBounds.push_back(rRange.aEnd.Row()+1);

    // Split formula cell groups at top and bottom boundaries (if applicable).
    sc::SharedFormulaUtil::splitFormulaCellGroups(maCells, aBounds);
    std::vector<sc::CellValueSpan> aSpans = rValues.getNonEmptySpans(nTab, nCol);

    // Detach formula cells within the spans (if any).
    EndListeningHandler aEndLisFunc(rEndCxt);
    sc::CellStoreType::iterator itPos = maCells.begin();
    for (const auto& rSpan : aSpans)
    {
        SCROW nRow1 = rSpan.mnRow1;
        SCROW nRow2 = rSpan.mnRow2;
        itPos = sc::ProcessFormula(itPos, maCells, nRow1, nRow2, aEndLisFunc);
    }

    rValues.swapNonEmpty(nTab, nCol, *this);
    RegroupFormulaCells();

    // Attach formula cells within the spans (if any).
    StartListeningHandler aStartLisFunc(rStartCxt);
    itPos = maCells.begin();
    for (const auto& rSpan : aSpans)
    {
        SCROW nRow1 = rSpan.mnRow1;
        SCROW nRow2 = rSpan.mnRow2;
        itPos = sc::ProcessFormula(itPos, maCells, nRow1, nRow2, aStartLisFunc);
    }

    CellStorageModified();
}

void ScColumn::DeleteRanges( const std::vector<sc::RowSpan>& rRanges, InsertDeleteFlags nDelFlag )
{
    for (const auto& rSpan : rRanges)
        DeleteArea(rSpan.mnRow1, rSpan.mnRow2, nDelFlag, false/*bBroadcast*/);
}

void ScColumn::CloneFormulaCell(
    const ScFormulaCell& rSrc, const sc::CellTextAttr& rAttr,
    const std::vector<sc::RowSpan>& rRanges )
{
    sc::CellStoreType::iterator itPos = maCells.begin();
    sc::CellTextAttrStoreType::iterator itAttrPos = maCellTextAttrs.begin();

    SCCOL nMatrixCols = 0;
    SCROW nMatrixRows = 0;
    ScMatrixMode nMatrixFlag = rSrc.GetMatrixFlag();
    if (nMatrixFlag == ScMatrixMode::Formula)
    {
        rSrc.GetMatColsRows( nMatrixCols, nMatrixRows);
        SAL_WARN_IF( nMatrixCols != 1 || nMatrixRows != 1, "sc.core",
                "ScColumn::CloneFormulaCell - cloning array/matrix with not exactly one column or row as single cell");
    }

    ScDocument* pDocument = GetDoc();
    std::vector<ScFormulaCell*> aFormulas;
    for (const auto& rSpan : rRanges)
    {
        SCROW nRow1 = rSpan.mnRow1, nRow2 = rSpan.mnRow2;
        size_t nLen = nRow2 - nRow1 + 1;
        assert(nLen > 0);
        aFormulas.clear();
        aFormulas.reserve(nLen);

        ScAddress aPos(nCol, nRow1, nTab);

        if (nLen == 1)
        {
            // Single, ungrouped formula cell.
            ScFormulaCell* pCell = new ScFormulaCell(rSrc, *pDocument, aPos);
            aFormulas.push_back(pCell);
        }
        else
        {
            // Create a group of formula cells.
            ScFormulaCellGroupRef xGroup(new ScFormulaCellGroup);
            xGroup->setCode(*rSrc.GetCode());
            xGroup->compileCode(*pDocument, aPos, pDocument->GetGrammar());
            for (size_t i = 0; i < nLen; ++i, aPos.IncRow())
            {
                ScFormulaCell* pCell = new ScFormulaCell(pDocument, aPos, xGroup, pDocument->GetGrammar(), nMatrixFlag);
                if (nMatrixFlag == ScMatrixMode::Formula)
                    pCell->SetMatColsRows( nMatrixCols, nMatrixRows);
                if (i == 0)
                {
                    xGroup->mpTopCell = pCell;
                    xGroup->mnLength = nLen;
                }
                aFormulas.push_back(pCell);
            }
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

        std::vector<sc::CellTextAttr> aTextAttrs(nLen, rAttr);
        itAttrPos = maCellTextAttrs.set(itAttrPos, nRow1, aTextAttrs.begin(), aTextAttrs.end());
    }

    CellStorageModified();
}

std::unique_ptr<ScPostIt> ScColumn::ReleaseNote( SCROW nRow )
{
    if (!ValidRow(nRow))
        return nullptr;

    ScPostIt* p = nullptr;
    maCellNotes.release(nRow, p);
    return std::unique_ptr<ScPostIt>(p);
}

size_t ScColumn::GetNoteCount() const
{
    return std::accumulate(maCellNotes.begin(), maCellNotes.end(), size_t(0),
        [](const size_t& rCount, const auto& rCellNote) {
            if (rCellNote.type != sc::element_type_cellnote)
                return rCount;
            return rCount + rCellNote.size;
        });
}

namespace {

class NoteCaptionCreator
{
    ScAddress maPos;
public:
    NoteCaptionCreator( SCTAB nTab, SCCOL nCol ) : maPos(nCol,0,nTab) {}

    void operator() ( size_t nRow, const ScPostIt* p )
    {
        maPos.SetRow(nRow);
        p->GetOrCreateCaption(maPos);
    }
};

class NoteCaptionCleaner
{
    bool const mbPreserveData;
public:
    explicit NoteCaptionCleaner( bool bPreserveData ) : mbPreserveData(bPreserveData) {}

    void operator() ( size_t /*nRow*/, ScPostIt* p )
    {
        p->ForgetCaption(mbPreserveData);
    }
};

}

void ScColumn::CreateAllNoteCaptions()
{
    NoteCaptionCreator aFunc(nTab, nCol);
    sc::ProcessNote(maCellNotes, aFunc);
}

void ScColumn::ForgetNoteCaptions( SCROW nRow1, SCROW nRow2, bool bPreserveData )
{
    if (maCellNotes.empty())
        return;

    if (!ValidRow(nRow1) || !ValidRow(nRow2))
        return;

    NoteCaptionCleaner aFunc(bPreserveData);
    sc::CellNoteStoreType::iterator it = maCellNotes.begin();
    sc::ProcessNote(it, maCellNotes, nRow1, nRow2, aFunc);
}

SCROW ScColumn::GetNotePosition( size_t nIndex ) const
{
    // Return the row position of the nth note in the column.

    size_t nCount = 0; // Number of notes encountered so far.
    for (const auto& rCellNote : maCellNotes)
    {
        if (rCellNote.type != sc::element_type_cellnote)
            // Skip the empty blocks.
            continue;

        if (nIndex < nCount + rCellNote.size)
        {
            // Index falls within this block.
            size_t nOffset = nIndex - nCount;
            return rCellNote.position + nOffset;
        }

        nCount += rCellNote.size;
    }

    return -1;
}

namespace {

class NoteEntryCollector
{
    std::vector<sc::NoteEntry>& mrNotes;
    SCTAB const mnTab;
    SCCOL const mnCol;
    SCROW const mnStartRow;
    SCROW const mnEndRow;
public:
    NoteEntryCollector( std::vector<sc::NoteEntry>& rNotes, SCTAB nTab, SCCOL nCol,
            SCROW nStartRow = 0, SCROW nEndRow = MAXROW) :
        mrNotes(rNotes), mnTab(nTab), mnCol(nCol),
        mnStartRow(nStartRow), mnEndRow(nEndRow) {}

    void operator() (const sc::CellNoteStoreType::value_type& node) const
    {
        if (node.type != sc::element_type_cellnote)
            return;

        size_t nTopRow = node.position;
        sc::cellnote_block::const_iterator it = sc::cellnote_block::begin(*node.data);
        sc::cellnote_block::const_iterator itEnd = sc::cellnote_block::end(*node.data);
        size_t nOffset = 0;
        if(nTopRow < size_t(mnStartRow))
        {
            std::advance(it, mnStartRow - nTopRow);
            nOffset = mnStartRow - nTopRow;
        }

        for (; it != itEnd && nTopRow + nOffset <= size_t(mnEndRow);
                ++it, ++nOffset)
        {
            ScAddress aPos(mnCol, nTopRow + nOffset, mnTab);
            mrNotes.emplace_back(aPos, *it);
        }
    }
};

}

void ScColumn::GetAllNoteEntries( std::vector<sc::NoteEntry>& rNotes ) const
{
    std::for_each(maCellNotes.begin(), maCellNotes.end(), NoteEntryCollector(rNotes, nTab, nCol));
}

void ScColumn::GetNotesInRange(SCROW nStartRow, SCROW nEndRow,
        std::vector<sc::NoteEntry>& rNotes ) const
{
    std::pair<sc::CellNoteStoreType::const_iterator,size_t> aPos = maCellNotes.position(nStartRow);
    sc::CellNoteStoreType::const_iterator it = aPos.first;
    if (it == maCellNotes.end())
        // Invalid row number.
        return;

    std::pair<sc::CellNoteStoreType::const_iterator,size_t> aEndPos =
        maCellNotes.position(nEndRow);
    sc::CellNoteStoreType::const_iterator itEnd = aEndPos.first;

    std::for_each(it, ++itEnd, NoteEntryCollector(rNotes, nTab, nCol, nStartRow, nEndRow));
}

void ScColumn::GetUnprotectedCells( SCROW nStartRow, SCROW nEndRow, ScRangeList& rRangeList ) const
{
    SCROW nTmpStartRow = nStartRow, nTmpEndRow = nEndRow;
    const ScPatternAttr* pPattern = pAttrArray->GetPatternRange(nTmpStartRow, nTmpEndRow, nStartRow);
    bool bProtection = pPattern->GetItem(ATTR_PROTECTION).GetProtection();
    if (!bProtection)
    {
        // Limit the span to the range in question.
        if (nTmpStartRow < nStartRow)
            nTmpStartRow = nStartRow;
        if (nTmpEndRow > nEndRow)
            nTmpEndRow = nEndRow;
        rRangeList.Join( ScRange( nCol, nTmpStartRow, nTab, nCol, nTmpEndRow, nTab));
    }
    while (nEndRow > nTmpEndRow)
    {
        nStartRow = nTmpEndRow + 1;
        pPattern = pAttrArray->GetPatternRange(nTmpStartRow, nTmpEndRow, nStartRow);
        bool bTmpProtection = pPattern->GetItem(ATTR_PROTECTION).GetProtection();
        if (!bTmpProtection)
        {
            // Limit the span to the range in question.
            // Only end row needs to be checked as we enter here only for spans
            // below the original nStartRow.
            if (nTmpEndRow > nEndRow)
                nTmpEndRow = nEndRow;
            rRangeList.Join( ScRange( nCol, nTmpStartRow, nTab, nCol, nTmpEndRow, nTab));
        }
    }
}

namespace {

class RecompileByOpcodeHandler
{
    ScDocument* mpDoc;
    const formula::unordered_opcode_set& mrOps;
    sc::EndListeningContext& mrEndListenCxt;
    sc::CompileFormulaContext& mrCompileFormulaCxt;

public:
    RecompileByOpcodeHandler(
        ScDocument* pDoc, const formula::unordered_opcode_set& rOps,
        sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt ) :
        mpDoc(pDoc),
        mrOps(rOps),
        mrEndListenCxt(rEndListenCxt),
        mrCompileFormulaCxt(rCompileCxt) {}

    void operator() ( sc::FormulaGroupEntry& rEntry )
    {
        // Perform end listening, remove from formula tree, and set them up
        // for re-compilation.

        ScFormulaCell* pTop = nullptr;

        if (rEntry.mbShared)
        {
            // Only inspect the code from the top cell.
            pTop = *rEntry.mpCells;
        }
        else
            pTop = rEntry.mpCell;

        ScTokenArray* pCode = pTop->GetCode();
        bool bRecompile = pCode->HasOpCodes(mrOps);

        if (bRecompile)
        {
            // Get the formula string.
            OUString aFormula = pTop->GetFormula(mrCompileFormulaCxt);
            sal_Int32 n = aFormula.getLength();
            if (pTop->GetMatrixFlag() != ScMatrixMode::NONE && n > 0)
            {
                if (aFormula[0] == '{' && aFormula[n-1] == '}')
                    aFormula = aFormula.copy(1, n-2);
            }

            if (rEntry.mbShared)
            {
                ScFormulaCell** pp = rEntry.mpCells;
                ScFormulaCell** ppEnd = pp + rEntry.mnLength;
                for (; pp != ppEnd; ++pp)
                {
                    ScFormulaCell* p = *pp;
                    p->EndListeningTo(mrEndListenCxt);
                    mpDoc->RemoveFromFormulaTree(p);
                }
            }
            else
            {
                rEntry.mpCell->EndListeningTo(mrEndListenCxt);
                mpDoc->RemoveFromFormulaTree(rEntry.mpCell);
            }

            pCode->Clear();
            pTop->SetHybridFormula(aFormula, mpDoc->GetGrammar());
        }
    }
};

class CompileHybridFormulaHandler
{
    ScDocument* mpDoc;
    sc::StartListeningContext& mrStartListenCxt;
    sc::CompileFormulaContext& mrCompileFormulaCxt;

public:
    CompileHybridFormulaHandler( ScDocument* pDoc, sc::StartListeningContext& rStartListenCxt, sc::CompileFormulaContext& rCompileCxt ) :
        mpDoc(pDoc),
        mrStartListenCxt(rStartListenCxt),
        mrCompileFormulaCxt(rCompileCxt) {}

    void operator() ( sc::FormulaGroupEntry& rEntry )
    {
        if (rEntry.mbShared)
        {
            ScFormulaCell* pTop = *rEntry.mpCells;
            OUString aFormula = pTop->GetHybridFormula();

            if (!aFormula.isEmpty())
            {
                // Create a new token array from the hybrid formula string, and
                // set it to the group.
                ScCompiler aComp(mrCompileFormulaCxt, pTop->aPos);
                std::unique_ptr<ScTokenArray> pNewCode = aComp.CompileString(aFormula);
                ScFormulaCellGroupRef xGroup = pTop->GetCellGroup();
                assert(xGroup);
                xGroup->setCode(std::move(pNewCode));
                xGroup->compileCode(*mpDoc, pTop->aPos, mpDoc->GetGrammar());

                // Propagate the new token array to all formula cells in the group.
                ScFormulaCell** pp = rEntry.mpCells;
                ScFormulaCell** ppEnd = pp + rEntry.mnLength;
                for (; pp != ppEnd; ++pp)
                {
                    ScFormulaCell* p = *pp;
                    p->SyncSharedCode();
                    p->StartListeningTo(mrStartListenCxt);
                    p->SetDirty();
                }
            }
        }
        else
        {
            ScFormulaCell* pCell = rEntry.mpCell;
            OUString aFormula = pCell->GetHybridFormula();

            if (!aFormula.isEmpty())
            {
                // Create token array from formula string.
                ScCompiler aComp(mrCompileFormulaCxt, pCell->aPos);
                std::unique_ptr<ScTokenArray> pNewCode = aComp.CompileString(aFormula);

                // Generate RPN tokens.
                ScCompiler aComp2(mpDoc, pCell->aPos, *pNewCode, formula::FormulaGrammar::GRAM_UNSPECIFIED,
                                  true, pCell->GetMatrixFlag() != ScMatrixMode::NONE);
                aComp2.CompileTokenArray();

                pCell->SetCode(std::move(pNewCode));
                pCell->StartListeningTo(mrStartListenCxt);
                pCell->SetDirty();
            }
        }
    }
};

}

void ScColumn::PreprocessRangeNameUpdate(
    sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    // Collect all formula groups.
    std::vector<sc::FormulaGroupEntry> aGroups = GetFormulaGroupEntries();

    formula::unordered_opcode_set aOps;
    aOps.insert(ocBad);
    aOps.insert(ocColRowName);
    aOps.insert(ocName);
    RecompileByOpcodeHandler aFunc(GetDoc(), aOps, rEndListenCxt, rCompileCxt);
    std::for_each(aGroups.begin(), aGroups.end(), aFunc);
}

void ScColumn::PreprocessDBDataUpdate(
    sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    // Collect all formula groups.
    std::vector<sc::FormulaGroupEntry> aGroups = GetFormulaGroupEntries();

    formula::unordered_opcode_set aOps;
    aOps.insert(ocBad);
    aOps.insert(ocColRowName);
    aOps.insert(ocDBArea);
    aOps.insert(ocTableRef);
    RecompileByOpcodeHandler aFunc(GetDoc(), aOps, rEndListenCxt, rCompileCxt);
    std::for_each(aGroups.begin(), aGroups.end(), aFunc);
}

void ScColumn::CompileHybridFormula(
    sc::StartListeningContext& rStartListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    // Collect all formula groups.
    std::vector<sc::FormulaGroupEntry> aGroups = GetFormulaGroupEntries();

    CompileHybridFormulaHandler aFunc(GetDoc(), rStartListenCxt, rCompileCxt);
    std::for_each(aGroups.begin(), aGroups.end(), aFunc);
}

namespace {

class ScriptTypeUpdater
{
    ScColumn& mrCol;
    sc::CellTextAttrStoreType& mrTextAttrs;
    sc::CellTextAttrStoreType::iterator miPosAttr;
    ScConditionalFormatList* const mpCFList;
    SvNumberFormatter* mpFormatter;
    ScAddress maPos;
    bool mbUpdated;

private:
    void updateScriptType( size_t nRow, ScRefCellValue& rCell )
    {
        sc::CellTextAttrStoreType::position_type aAttrPos = mrTextAttrs.position(miPosAttr, nRow);
        miPosAttr = aAttrPos.first;

        if (aAttrPos.first->type != sc::element_type_celltextattr)
            return;

        sc::CellTextAttr& rAttr = sc::celltextattr_block::at(*aAttrPos.first->data, aAttrPos.second);
        if (rAttr.mnScriptType != SvtScriptType::UNKNOWN)
            // Script type already determined.  Skip it.
            return;

        const ScPatternAttr* pPat = mrCol.GetPattern(nRow);
        if (!pPat)
            // In theory this should never return NULL. But let's be safe.
            return;

        const SfxItemSet* pCondSet = nullptr;
        if (mpCFList)
        {
            maPos.SetRow(nRow);
            const ScCondFormatItem& rItem = pPat->GetItem(ATTR_CONDITIONAL);
            const std::vector<sal_uInt32>& rData = rItem.GetCondFormatData();
            pCondSet = mrCol.GetDoc()->GetCondResult(rCell, maPos, *mpCFList, rData);
        }

        OUString aStr;
        Color* pColor;
        sal_uInt32 nFormat = pPat->GetNumberFormat(mpFormatter, pCondSet);
        ScCellFormat::GetString(rCell, nFormat, aStr, &pColor, *mpFormatter, mrCol.GetDoc());

        rAttr.mnScriptType = mrCol.GetDoc()->GetStringScriptType(aStr);
        mbUpdated = true;
    }

public:
    explicit ScriptTypeUpdater( ScColumn& rCol ) :
        mrCol(rCol),
        mrTextAttrs(rCol.GetCellAttrStore()),
        miPosAttr(mrTextAttrs.begin()),
        mpCFList(rCol.GetDoc()->GetCondFormList(rCol.GetTab())),
        mpFormatter(rCol.GetDoc()->GetFormatTable()),
        maPos(rCol.GetCol(), 0, rCol.GetTab()),
        mbUpdated(false)
    {}

    void operator() ( size_t nRow, double fVal )
    {
        ScRefCellValue aCell(fVal);
        updateScriptType(nRow, aCell);
    }

    void operator() ( size_t nRow, const svl::SharedString& rStr )
    {
        ScRefCellValue aCell(&rStr);
        updateScriptType(nRow, aCell);
    }

    void operator() ( size_t nRow, const EditTextObject* pText )
    {
        ScRefCellValue aCell(pText);
        updateScriptType(nRow, aCell);
    }

    void operator() ( size_t nRow, const ScFormulaCell* pCell )
    {
        ScRefCellValue aCell(const_cast<ScFormulaCell*>(pCell));
        updateScriptType(nRow, aCell);
    }

    bool isUpdated() const { return mbUpdated; }
};

}

void ScColumn::UpdateScriptTypes( SCROW nRow1, SCROW nRow2 )
{
    if (!ValidRow(nRow1) || !ValidRow(nRow2) || nRow1 > nRow2)
        return;

    ScriptTypeUpdater aFunc(*this);
    sc::ParseAllNonEmpty(maCells.begin(), maCells, nRow1, nRow2, aFunc);
    if (aFunc.isUpdated())
        CellStorageModified();
}

void ScColumn::Swap( ScColumn& rOther, SCROW nRow1, SCROW nRow2, bool bPattern )
{
    maCells.swap(nRow1, nRow2, rOther.maCells, nRow1);
    maCellTextAttrs.swap(nRow1, nRow2, rOther.maCellTextAttrs, nRow1);
    maCellNotes.swap(nRow1, nRow2, rOther.maCellNotes, nRow1);
    maBroadcasters.swap(nRow1, nRow2, rOther.maBroadcasters, nRow1);

    // Update draw object anchors
    ScDrawLayer* pDrawLayer = GetDoc()->GetDrawLayer();
    if (pDrawLayer)
    {
        std::map<SCROW, std::vector<SdrObject*>> aThisColRowDrawObjects
            = pDrawLayer->GetObjectsAnchoredToRange(GetTab(), GetCol(), nRow1, nRow2);
        std::map<SCROW, std::vector<SdrObject*>> aOtherColRowDrawObjects
            = pDrawLayer->GetObjectsAnchoredToRange(GetTab(), rOther.GetCol(), nRow1, nRow2);
        for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
        {
            std::vector<SdrObject*>& rThisCellDrawObjects = aThisColRowDrawObjects[nRow];
            if (!rThisCellDrawObjects.empty())
                UpdateDrawObjectsForRow(rThisCellDrawObjects, rOther.GetCol(), nRow);
            std::vector<SdrObject*>& rOtherCellDrawObjects = aOtherColRowDrawObjects[nRow];
            if (!rOtherCellDrawObjects.empty())
                rOther.UpdateDrawObjectsForRow(rOtherCellDrawObjects, GetCol(), nRow);
        }
    }

    if (bPattern)
    {
        for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
        {
            const ScPatternAttr* pPat1 = GetPattern(nRow);
            const ScPatternAttr* pPat2 = rOther.GetPattern(nRow);
            if (pPat1 != pPat2)
            {
                SetPattern(nRow, *pPat2);
                rOther.SetPattern(nRow, *pPat1);
            }
        }
    }

    CellStorageModified();
    rOther.CellStorageModified();
}

namespace {

class FormulaColPosSetter
{
    SCCOL const mnCol;
    bool const  mbUpdateRefs;
public:
    FormulaColPosSetter( SCCOL nCol, bool bUpdateRefs ) : mnCol(nCol), mbUpdateRefs(bUpdateRefs) {}

    void operator() ( size_t nRow, ScFormulaCell* pCell )
    {
        if (!pCell->IsShared() || pCell->IsSharedTop())
        {
            // Ensure that the references still point to the same locations
            // after the position change.
            ScAddress aOldPos = pCell->aPos;
            pCell->aPos.SetCol(mnCol);
            pCell->aPos.SetRow(nRow);
            if (mbUpdateRefs)
                pCell->GetCode()->AdjustReferenceOnMovedOrigin(aOldPos, pCell->aPos);
            else
                pCell->GetCode()->AdjustReferenceOnMovedOriginIfOtherSheet(aOldPos, pCell->aPos);
        }
        else
        {
            pCell->aPos.SetCol(mnCol);
            pCell->aPos.SetRow(nRow);
        }
    }
};

}

void ScColumn::ResetFormulaCellPositions( SCROW nRow1, SCROW nRow2, bool bUpdateRefs )
{
    FormulaColPosSetter aFunc(nCol, bUpdateRefs);
    sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aFunc);
}

namespace {

class RelativeRefBoundChecker
{
    std::vector<SCROW> maBounds;
    ScRange const maBoundRange;

public:
    explicit RelativeRefBoundChecker( const ScRange& rBoundRange ) :
        maBoundRange(rBoundRange) {}

    void operator() ( size_t /*nRow*/, ScFormulaCell* pCell )
    {
        if (!pCell->IsSharedTop())
            return;

        pCell->GetCode()->CheckRelativeReferenceBounds(
            pCell->aPos, pCell->GetSharedLength(), maBoundRange, maBounds);
    }

    void swapBounds( std::vector<SCROW>& rBounds )
    {
        rBounds.swap(maBounds);
    }
};

}

void ScColumn::SplitFormulaGroupByRelativeRef( const ScRange& rBoundRange )
{
    if (rBoundRange.aStart.Row() >= MAXROW)
        // Nothing to split.
        return;

    std::vector<SCROW> aBounds;

    // Cut at row boundaries first.
    aBounds.push_back(rBoundRange.aStart.Row());
    if (rBoundRange.aEnd.Row() < MAXROW)
        aBounds.push_back(rBoundRange.aEnd.Row()+1);
    sc::SharedFormulaUtil::splitFormulaCellGroups(maCells, aBounds);

    RelativeRefBoundChecker aFunc(rBoundRange);
    sc::ProcessFormula(
        maCells.begin(), maCells, rBoundRange.aStart.Row(), rBoundRange.aEnd.Row(), aFunc);
    aFunc.swapBounds(aBounds);
    sc::SharedFormulaUtil::splitFormulaCellGroups(maCells, aBounds);
}

namespace {

class ListenerCollector
{
    std::vector<SvtListener*>& mrListeners;
public:
    explicit ListenerCollector( std::vector<SvtListener*>& rListener ) :
        mrListeners(rListener) {}

    void operator() ( size_t /*nRow*/, SvtBroadcaster* p )
    {
        SvtBroadcaster::ListenersType& rLis = p->GetAllListeners();
        mrListeners.reserve(mrListeners.size() + rLis.size());
        std::copy(rLis.begin(), rLis.end(), std::back_inserter(mrListeners));
    }
};

class FormulaCellCollector
{
    std::vector<ScFormulaCell*>& mrCells;
public:
    explicit FormulaCellCollector( std::vector<ScFormulaCell*>& rCells ) : mrCells(rCells) {}

    void operator() ( size_t /*nRow*/, ScFormulaCell* p )
    {
        mrCells.push_back(p);
    }
};

}

void ScColumn::CollectListeners( std::vector<SvtListener*>& rListeners, SCROW nRow1, SCROW nRow2 )
{
    if (nRow2 < nRow1 || !ValidRow(nRow1) || !ValidRow(nRow2))
        return;

    ListenerCollector aFunc(rListeners);
    sc::ProcessBroadcaster(maBroadcasters.begin(), maBroadcasters, nRow1, nRow2, aFunc);
}

void ScColumn::CollectFormulaCells( std::vector<ScFormulaCell*>& rCells, SCROW nRow1, SCROW nRow2 )
{
    if (nRow2 < nRow1 || !ValidRow(nRow1) || !ValidRow(nRow2))
        return;

    FormulaCellCollector aFunc(rCells);
    sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aFunc);
}

bool ScColumn::HasFormulaCell() const
{
    return mnBlkCountFormula != 0;
}

namespace {

struct FindAnyFormula
{
    bool operator() ( size_t /*nRow*/, const ScFormulaCell* /*pCell*/ ) const
    {
        return true;
    }
};

}

bool ScColumn::HasFormulaCell( SCROW nRow1, SCROW nRow2 ) const
{
    if (!mnBlkCountFormula)
        return false;

    if (nRow2 < nRow1 || !ValidRow(nRow1) || !ValidRow(nRow2))
        return false;

    if (nRow1 == 0 && nRow2 == MAXROW)
        return HasFormulaCell();

    FindAnyFormula aFunc;
    std::pair<sc::CellStoreType::const_iterator, size_t> aRet =
        sc::FindFormula(maCells, nRow1, nRow2, aFunc);

    return aRet.first != maCells.end();
}

namespace {

void endListening( sc::EndListeningContext& rCxt, ScFormulaCell** pp, ScFormulaCell** ppEnd )
{
    for (; pp != ppEnd; ++pp)
    {
        ScFormulaCell& rFC = **pp;
        rFC.EndListeningTo(rCxt);
    }
}

class StartListeningFormulaCellsHandler
{
    sc::StartListeningContext& mrStartCxt;
    sc::EndListeningContext& mrEndCxt;
    SCROW mnStartRow;

public:
    StartListeningFormulaCellsHandler( sc::StartListeningContext& rStartCxt, sc::EndListeningContext& rEndCxt ) :
        mrStartCxt(rStartCxt), mrEndCxt(rEndCxt), mnStartRow(-1) {}

    void operator() ( const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize )
    {
        if (node.type != sc::element_type_formula)
            // We are only interested in formulas.
            return;

        mnStartRow = node.position + nOffset;

        ScFormulaCell** ppBeg = &sc::formula_block::at(*node.data, nOffset);
        ScFormulaCell** ppEnd = ppBeg + nDataSize;

        ScFormulaCell** pp = ppBeg;

        // If the first formula cell belongs to a group and it's not the top
        // cell, move up to the top cell of the group, and have all the extra
        // formula cells stop listening.

        ScFormulaCell* pFC = *pp;
        if (pFC->IsShared() && !pFC->IsSharedTop())
        {
            SCROW nBackTrackSize = pFC->aPos.Row() - pFC->GetSharedTopRow();
            if (nBackTrackSize > 0)
            {
                assert(static_cast<size_t>(nBackTrackSize) <= nOffset);
                for (SCROW i = 0; i < nBackTrackSize; ++i)
                    --pp;
                endListening(mrEndCxt, pp, ppBeg);
                mnStartRow -= nBackTrackSize;
            }
        }

        for (; pp != ppEnd; ++pp)
        {
            pFC = *pp;

            if (!pFC->IsSharedTop())
            {
                assert(!pFC->IsShared());
                pFC->StartListeningTo(mrStartCxt);
                continue;
            }

            // If This is the last group in the range, see if the group
            // extends beyond the range, in which case have the excess
            // formula cells stop listening.
            size_t nEndGroupPos = (pp - ppBeg) + pFC->GetSharedLength();
            if (nEndGroupPos > nDataSize)
            {
                size_t nExcessSize = nEndGroupPos - nDataSize;
                ScFormulaCell** ppGrpEnd = pp + pFC->GetSharedLength();
                ScFormulaCell** ppGrp = ppGrpEnd - nExcessSize;
                endListening(mrEndCxt, ppGrp, ppGrpEnd);

                // Register formula cells as a group.
                sc::SharedFormulaUtil::startListeningAsGroup(mrStartCxt, pp);
                pp = ppEnd - 1; // Move to the one before the end position.
            }
            else
            {
                // Register formula cells as a group.
                sc::SharedFormulaUtil::startListeningAsGroup(mrStartCxt, pp);
                pp += pFC->GetSharedLength() - 1; // Move to the last one in the group.
            }
        }
    }

};

class EndListeningFormulaCellsHandler
{
    sc::EndListeningContext& mrEndCxt;
    SCROW mnStartRow;
    SCROW mnEndRow;

public:
    explicit EndListeningFormulaCellsHandler( sc::EndListeningContext& rEndCxt ) :
        mrEndCxt(rEndCxt), mnStartRow(-1), mnEndRow(-1) {}

    void operator() ( const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize )
    {
        if (node.type != sc::element_type_formula)
            // We are only interested in formulas.
            return;

        mnStartRow = node.position + nOffset;

        ScFormulaCell** ppBeg = &sc::formula_block::at(*node.data, nOffset);
        ScFormulaCell** ppEnd = ppBeg + nDataSize;

        ScFormulaCell** pp = ppBeg;

        // If the first formula cell belongs to a group and it's not the top
        // cell, move up to the top cell of the group.

        ScFormulaCell* pFC = *pp;
        if (pFC->IsShared() && !pFC->IsSharedTop())
        {
            SCROW nBackTrackSize = pFC->aPos.Row() - pFC->GetSharedTopRow();
            if (nBackTrackSize > 0)
            {
                assert(static_cast<size_t>(nBackTrackSize) <= nOffset);
                for (SCROW i = 0; i < nBackTrackSize; ++i)
                    --pp;
                mnStartRow -= nBackTrackSize;
            }
        }

        for (; pp != ppEnd; ++pp)
        {
            pFC = *pp;

            if (!pFC->IsSharedTop())
            {
                assert(!pFC->IsShared());
                pFC->EndListeningTo(mrEndCxt);
                continue;
            }

            size_t nEndGroupPos = (pp - ppBeg) + pFC->GetSharedLength();
            mnEndRow = node.position + nOffset + nEndGroupPos - 1; // absolute row position of the last one in the group.

            ScFormulaCell** ppGrpEnd = pp + pFC->GetSharedLength();
            endListening(mrEndCxt, pp, ppGrpEnd);

            if (nEndGroupPos > nDataSize)
            {
                // The group goes beyond the specified end row.  Move to the
                // one before the end position to finish the loop.
                pp = ppEnd - 1;
            }
            else
            {
                // Move to the last one in the group.
                pp += pFC->GetSharedLength() - 1;
            }
        }
    }

    SCROW getStartRow() const
    {
        return mnStartRow;
    }

    SCROW getEndRow() const
    {
        return mnEndRow;
    }
};

}

void ScColumn::StartListeningFormulaCells(
    sc::StartListeningContext& rStartCxt, sc::EndListeningContext& rEndCxt,
    SCROW nRow1, SCROW nRow2 )
{
    if (!HasFormulaCell())
        return;

    StartListeningFormulaCellsHandler aFunc(rStartCxt, rEndCxt);
    sc::ProcessBlock(maCells.begin(), maCells, aFunc, nRow1, nRow2);
}

void ScColumn::EndListeningFormulaCells(
    sc::EndListeningContext& rCxt, SCROW nRow1, SCROW nRow2,
    SCROW* pStartRow, SCROW* pEndRow )
{
    if (!HasFormulaCell())
        return;

    EndListeningFormulaCellsHandler aFunc(rCxt);
    sc::ProcessBlock(maCells.begin(), maCells, aFunc, nRow1, nRow2);

    if (pStartRow)
        *pStartRow = aFunc.getStartRow();

    if (pEndRow)
        *pEndRow = aFunc.getEndRow();
}

void ScColumn::EndListeningIntersectedGroup(
    sc::EndListeningContext& rCxt, SCROW nRow, std::vector<ScAddress>* pGroupPos )
{
    if (!ValidRow(nRow))
        return;

    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it->type != sc::element_type_formula)
        // Only interested in a formula block.
        return;

    ScFormulaCell* pFC = sc::formula_block::at(*it->data, aPos.second);
    ScFormulaCellGroupRef xGroup = pFC->GetCellGroup();
    if (!xGroup)
        // Not a formula group.
        return;

    // End listening.
    pFC->EndListeningTo(rCxt);

    if (pGroupPos)
    {
        if (!pFC->IsSharedTop())
            // Record the position of the top cell of the group.
            pGroupPos->push_back(xGroup->mpTopCell->aPos);

        SCROW nGrpLastRow = pFC->GetSharedTopRow() + pFC->GetSharedLength() - 1;
        if (nRow < nGrpLastRow)
            // Record the last position of the group.
            pGroupPos->push_back(ScAddress(nCol, nGrpLastRow, nTab));
    }
}

void ScColumn::EndListeningIntersectedGroups(
    sc::EndListeningContext& rCxt, SCROW nRow1, SCROW nRow2, std::vector<ScAddress>* pGroupPos )
{
    // Only end the intersected group.
    sc::CellStoreType::position_type aPos = maCells.position(nRow1);
    sc::CellStoreType::iterator it = aPos.first;
    if (it->type == sc::element_type_formula)
    {
        ScFormulaCell* pFC = sc::formula_block::at(*it->data, aPos.second);
        ScFormulaCellGroupRef xGroup = pFC->GetCellGroup();
        if (xGroup)
        {
            if (!pFC->IsSharedTop())
                // End listening.
                pFC->EndListeningTo(rCxt);

            if (pGroupPos)
                // Record the position of the top cell of the group.
                pGroupPos->push_back(xGroup->mpTopCell->aPos);
        }
    }

    aPos = maCells.position(it, nRow2);
    it = aPos.first;
    if (it->type == sc::element_type_formula)
    {
        ScFormulaCell* pFC = sc::formula_block::at(*it->data, aPos.second);
        ScFormulaCellGroupRef xGroup = pFC->GetCellGroup();
        if (xGroup)
        {
            if (!pFC->IsSharedTop())
                // End listening.
                pFC->EndListeningTo(rCxt);

            if (pGroupPos)
            {
                // Record the position of the bottom cell of the group.
                ScAddress aPosLast = xGroup->mpTopCell->aPos;
                aPosLast.IncRow(xGroup->mnLength-1);
                pGroupPos->push_back(aPosLast);
            }
        }
    }
}

void ScColumn::EndListeningGroup( sc::EndListeningContext& rCxt, SCROW nRow )
{
    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    if (aPos.first->type != sc::element_type_formula)
        // not a formula cell.
        return;

    ScFormulaCell** pp = &sc::formula_block::at(*aPos.first->data, aPos.second);

    ScFormulaCellGroupRef xGroup = (*pp)->GetCellGroup();
    if (!xGroup)
    {
        // not a formula group.
        (*pp)->EndListeningTo(rCxt);
        return;
    }

    // Move back to the top cell.
    SCROW nTopDelta = (*pp)->aPos.Row() - xGroup->mpTopCell->aPos.Row();
    assert(nTopDelta >= 0);
    if (nTopDelta > 0)
        pp -= nTopDelta;

    // Set the needs listening flag to all cells in the group.
    assert(*pp == xGroup->mpTopCell);
    ScFormulaCell** ppEnd = pp + xGroup->mnLength;
    for (; pp != ppEnd; ++pp)
        (*pp)->EndListeningTo(rCxt);
}

void ScColumn::SetNeedsListeningGroup( SCROW nRow )
{
    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    if (aPos.first->type != sc::element_type_formula)
        // not a formula cell.
        return;

    ScFormulaCell** pp = &sc::formula_block::at(*aPos.first->data, aPos.second);

    ScFormulaCellGroupRef xGroup = (*pp)->GetCellGroup();
    if (!xGroup)
    {
        // not a formula group.
        (*pp)->SetNeedsListening(true);
        return;
    }

    // Move back to the top cell.
    SCROW nTopDelta = (*pp)->aPos.Row() - xGroup->mpTopCell->aPos.Row();
    assert(nTopDelta >= 0);
    if (nTopDelta > 0)
        pp -= nTopDelta;

    // Set the needs listening flag to all cells in the group.
    assert(*pp == xGroup->mpTopCell);
    ScFormulaCell** ppEnd = pp + xGroup->mnLength;
    for (; pp != ppEnd; ++pp)
        (*pp)->SetNeedsListening(true);
}

std::unique_ptr<sc::ColumnIterator> ScColumn::GetColumnIterator( SCROW nRow1, SCROW nRow2 ) const
{
    if (!ValidRow(nRow1) || !ValidRow(nRow2) || nRow1 > nRow2)
        return std::unique_ptr<sc::ColumnIterator>();

    return std::make_unique<sc::ColumnIterator>(maCells, nRow1, nRow2);
}

static bool lcl_InterpretSpan(sc::formula_block::const_iterator& rSpanIter, SCROW nStartOffset, SCROW nEndOffset,
                              const ScFormulaCellGroupRef& mxParentGroup, bool& bAllowThreading)
{
    bAllowThreading = true;
    ScFormulaCell* pCellStart = nullptr;
    SCROW nSpanStart = -1;
    SCROW nSpanEnd = -1;
    sc::formula_block::const_iterator itSpanStart;
    bool bAnyDirty = false;
    for (SCROW nFGOffset = nStartOffset; nFGOffset <= nEndOffset; ++rSpanIter, ++nFGOffset)
    {
        bool bThisDirty = (*rSpanIter)->NeedsInterpret();
        if (!pCellStart && bThisDirty)
        {
            pCellStart = *rSpanIter;
            itSpanStart = rSpanIter;
            nSpanStart = nFGOffset;
            bAnyDirty = true;
        }

        if (pCellStart && (!bThisDirty || nFGOffset == nEndOffset))
        {
            nSpanEnd = bThisDirty ? nFGOffset : nFGOffset - 1;
            assert(nSpanStart >= nStartOffset && nSpanStart <= nSpanEnd && nSpanEnd <= nEndOffset);

            // Found a completely dirty sub span [nSpanStart, nSpanEnd] inside the required span [nStartOffset, nEndOffset]
            bool bGroupInterpreted = pCellStart->Interpret(nSpanStart, nSpanEnd);

            // child cell's Interpret could result in calling dependency calc
            // and that could detect a cycle involving mxGroup
            // and do early exit in that case.
            if (mxParentGroup && mxParentGroup->mbPartOfCycle)
            {
                // Set pCellStart as dirty as pCellStart may be interpreted in InterpretTail()
                pCellStart->SetDirtyVar();
                bAllowThreading = false;
                return bAnyDirty;
            }

            if (!bGroupInterpreted)
            {
                // Evaluate from second cell in non-grouped style (no point in trying group-interpret again).
                ++itSpanStart;
                for (SCROW nIdx = nSpanStart+1; nIdx <= nSpanEnd; ++nIdx, ++itSpanStart)
                    (*itSpanStart)->Interpret(); // We know for sure that this cell is dirty so directly call Interpret().
            }

            pCellStart = nullptr; // For next sub span start detection.
        }
    }

    return bAnyDirty;
}

static void lcl_EvalDirty(sc::CellStoreType& rCells, SCROW nRow1, SCROW nRow2, ScDocument& rDoc,
                          const ScFormulaCellGroupRef& mxGroup, bool bThreadingDepEval, bool bSkipRunning,
                          bool& bIsDirty, bool& bAllowThreading)
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = rCells.position(nRow1);
    sc::CellStoreType::const_iterator it = aPos.first;
    size_t nOffset = aPos.second;
    SCROW nRow = nRow1;

    bIsDirty = false;

    for (;it != rCells.end() && nRow <= nRow2; ++it, nOffset = 0)
    {
        switch( it->type )
        {
            case sc::element_type_edittext:
                // These require EditEngine (in ScEditUtils::GetString()), which is probably
                // too complex for use in threads.
                if (bThreadingDepEval)
                {
                    bAllowThreading = false;
                    return;
                }
                break;
            case sc::element_type_formula:
            {
                size_t nRowsToRead = nRow2 - nRow + 1;
                const size_t nEnd = std::min(it->size, nOffset+nRowsToRead); // last row + 1
                sc::formula_block::const_iterator itCell = sc::formula_block::begin(*it->data);
                std::advance(itCell, nOffset);

                // Loop inside the formula block.
                size_t nCellIdx = nOffset;
                while (nCellIdx < nEnd)
                {
                    const ScFormulaCellGroupRef& mxGroupChild = (*itCell)->GetCellGroup();
                    ScFormulaCell* pChildTopCell = mxGroupChild ? mxGroupChild->mpTopCell : *itCell;

                    // Check if itCell is already in path.
                    // If yes use a cycle guard to mark all elements of the cycle
                    // and return false
                    if (bThreadingDepEval && pChildTopCell->GetSeenInPath())
                    {
                        ScRecursionHelper& rRecursionHelper = rDoc.GetRecursionHelper();
                        ScFormulaGroupCycleCheckGuard aCycleCheckGuard(rRecursionHelper, pChildTopCell);
                        bAllowThreading = false;
                        return;
                    }

                    if (bSkipRunning && (*itCell)->IsRunning())
                    {
                        ++itCell;
                        nCellIdx += 1;
                        nRow += 1;
                        nRowsToRead -= 1;
                        continue;
                    }

                    if (mxGroupChild)
                    {
                        // It is a Formula-group, evaluate the necessary parts of it (spans).
                        const SCROW nFGStartOffset = (*itCell)->aPos.Row() - pChildTopCell->aPos.Row();
                        const SCROW nFGEndOffset = std::min(nFGStartOffset + static_cast<SCROW>(nRowsToRead) - 1, mxGroupChild->mnLength - 1);
                        assert(nFGEndOffset >= nFGStartOffset);
                        const SCROW nSpanLen = nFGEndOffset - nFGStartOffset + 1;
                        // The (main) span required to be evaluated is [nFGStartOffset, nFGEndOffset], but this span may contain
                        // non-dirty cells, so split this into sets of completely-dirty spans and try evaluate each of them in grouped-style.

                        bool bAnyDirtyInSpan = lcl_InterpretSpan(itCell, nFGStartOffset, nFGEndOffset, mxGroup, bAllowThreading);
                        if (!bAllowThreading)
                            return;
                        // itCell will now point to cell just after the end of span [nFGStartOffset, nFGEndOffset].
                        bIsDirty = bIsDirty || bAnyDirtyInSpan;

                        // update the counters by nSpanLen.
                        // itCell already got updated.
                        nCellIdx += nSpanLen;
                        nRow += nSpanLen;
                        nRowsToRead -= nSpanLen;
                    }
                    else
                    {
                        // No formula-group here.
                        bool bDirtyFlag = (*itCell)->MaybeInterpret();
                        bIsDirty = bIsDirty || bDirtyFlag;

                        // child cell's Interpret could result in calling dependency calc
                        // and that could detect a cycle involving mxGroup
                        // and do early exit in that case.
                        if (bThreadingDepEval && mxGroup && mxGroup->mbPartOfCycle)
                        {
                            // Set itCell as dirty as itCell may be interpreted in InterpretTail()
                            (*itCell)->SetDirtyVar();
                            bAllowThreading = false;
                            return;
                        }

                        // update the counters by 1.
                        nCellIdx += 1;
                        nRow += 1;
                        nRowsToRead -= 1;
                        ++itCell;
                    }
                }
                break;
            }
            default:
                // Skip this block.
                nRow += it->size - nOffset;
                continue;
        }
    }

    if (bThreadingDepEval)
        bAllowThreading = true;

}

// Returns true if at least one FC is dirty.
bool ScColumn::EnsureFormulaCellResults( SCROW nRow1, SCROW nRow2, bool bSkipRunning )
{
    if (!ValidRow(nRow1) || !ValidRow(nRow2) || nRow1 > nRow2)
        return false;

    if (!HasFormulaCell(nRow1, nRow2))
        return false;

    bool bAnyDirty = false, bTmp = false;
    lcl_EvalDirty(maCells, nRow1, nRow2, *GetDoc(), nullptr, false, bSkipRunning, bAnyDirty, bTmp);
    return bAnyDirty;
}

bool ScColumn::HandleRefArrayForParallelism( SCROW nRow1, SCROW nRow2, const ScFormulaCellGroupRef& mxGroup )
{
    if (nRow1 > nRow2)
        return false;

    bool bAllowThreading = true, bTmp = false;
    lcl_EvalDirty(maCells, nRow1, nRow2, *GetDoc(), mxGroup, true, false, bTmp, bAllowThreading);

    return bAllowThreading;
}

namespace {

class StoreToCacheFunc
{
    SvStream& mrStrm;
public:

    StoreToCacheFunc(SvStream& rStrm):
        mrStrm(rStrm)
    {
    }

    void operator() ( const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize )
    {
        SCROW nStartRow = node.position + nOffset;
        mrStrm.WriteUInt64(nStartRow);
        mrStrm.WriteUInt64(nDataSize);
        switch (node.type)
        {
            case sc::element_type_empty:
            {
                mrStrm.WriteUChar(0);
            }
            break;
            case sc::element_type_numeric:
            {
                mrStrm.WriteUChar(1);
                sc::numeric_block::const_iterator it = sc::numeric_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::numeric_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                for (; it != itEnd; ++it)
                {
                    mrStrm.WriteDouble(*it);
                }
            }
            break;
            case sc::element_type_string:
            {
                mrStrm.WriteUChar(2);
                sc::string_block::const_iterator it = sc::string_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::string_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                for (; it != itEnd; ++it)
                {
                    OString aStr = OUStringToOString(it->getString(), RTL_TEXTENCODING_UTF8);
                    sal_Int32 nStrLength = aStr.getLength();
                    mrStrm.WriteInt32(nStrLength);
                    mrStrm.WriteCharPtr(aStr.getStr());
                }
            }
            break;
            case sc::element_type_formula:
            {
                mrStrm.WriteUChar(3);
                sc::formula_block::const_iterator it = sc::formula_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::formula_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                for (; it != itEnd; /* incrementing through std::advance*/)
                {
                    const ScFormulaCell* pCell = *it;
                    OUString aFormula;
                    pCell->GetFormula(aFormula, formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1);
                    const auto& xCellGroup = pCell->GetCellGroup();
                    sal_uInt64 nGroupLength = 0;
                    if (xCellGroup)
                    {
                        nGroupLength = xCellGroup->mnLength;
                    }
                    else
                    {
                        nGroupLength = 1;
                    }
                    mrStrm.WriteUInt64(nGroupLength);
                    mrStrm.WriteInt32(aFormula.getLength());
                    mrStrm.WriteCharPtr(OUStringToOString(aFormula, RTL_TEXTENCODING_UTF8).getStr());

                    // incrementing the iterator
                    std::advance(it, nGroupLength);
                }
            }
            break;
        }
    }
};

}

void ScColumn::StoreToCache(SvStream& rStrm) const
{
    rStrm.WriteUInt64(nCol);
    SCROW nLastRow = GetLastDataPos();
    rStrm.WriteUInt64(nLastRow + 1); // the rows are zero based

    StoreToCacheFunc aFunc(rStrm);
    sc::ParseBlock(maCells.begin(), maCells, aFunc, SCROW(0), nLastRow);
}

void ScColumn::RestoreFromCache(SvStream& rStrm)
{
    sal_uInt64 nStoredCol = 0;
    rStrm.ReadUInt64(nStoredCol);
    if (nStoredCol != static_cast<sal_uInt64>(nCol))
        throw std::exception();

    sal_uInt64 nLastRow = 0;
    rStrm.ReadUInt64(nLastRow);
    sal_uInt64 nReadRow = 0;
    ScDocument* pDocument = GetDoc();
    while (nReadRow < nLastRow)
    {
        sal_uInt64 nStartRow = 0;
        sal_uInt64 nDataSize = 0;
        rStrm.ReadUInt64(nStartRow);
        rStrm.ReadUInt64(nDataSize);
        sal_uInt8 nType = 0;
        rStrm.ReadUChar(nType);
        switch (nType)
        {
            case 0:
                // nothing to do
                maCells.set_empty(nStartRow, nDataSize);
            break;
            case 1:
            {
                // nDataSize double values
                std::vector<double> aValues(nDataSize);
                for (auto& rValue : aValues)
                {
                    rStrm.ReadDouble(rValue);
                }
                maCells.set(nStartRow, aValues.begin(), aValues.end());
            }
            break;
            case 2:
            {
                std::vector<svl::SharedString> aStrings(nDataSize);
                svl::SharedStringPool& rPool = pDocument->GetSharedStringPool();
                for (auto& rString : aStrings)
                {
                    sal_Int32 nStrLength = 0;
                    rStrm.ReadInt32(nStrLength);
                    std::unique_ptr<char[]> pStr(new char[nStrLength]);
                    rStrm.ReadBytes(pStr.get(), nStrLength);
                    OString aOStr(pStr.get(), nStrLength);
                    OUString aStr = OStringToOUString(aOStr, RTL_TEXTENCODING_UTF8);
                    rString = rPool.intern(aStr);
                }
                maCells.set(nStartRow, aStrings.begin(), aStrings.end());

            }
            break;
            case 3:
            {
                std::vector<ScFormulaCell*> aFormulaCells(nDataSize);

                ScAddress aAddr(nCol, nStartRow, nTab);
                const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1;
                for (SCROW nRow = 0; nRow < static_cast<SCROW>(nDataSize);)
                {
                    sal_uInt64 nFormulaGroupSize = 0;
                    rStrm.ReadUInt64(nFormulaGroupSize);
                    sal_Int32 nStrLength = 0;
                    rStrm.ReadInt32(nStrLength);
                    std::unique_ptr<char[]> pStr(new char[nStrLength]);
                    rStrm.ReadBytes(pStr.get(), nStrLength);
                    OString aOStr(pStr.get(), nStrLength);
                    OUString aStr = OStringToOUString(aOStr, RTL_TEXTENCODING_UTF8);
                    for (sal_uInt64 i = 0; i < nFormulaGroupSize; ++i)
                    {
                        aFormulaCells[nRow + i] = new ScFormulaCell(pDocument, aAddr, aStr, eGrammar);
                        aAddr.IncRow();
                    }

                    nRow += nFormulaGroupSize;
                }

                maCells.set(nStartRow, aFormulaCells.begin(), aFormulaCells.end());
            }
            break;
        }

        nReadRow += nDataSize;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
