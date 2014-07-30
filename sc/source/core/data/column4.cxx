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
#include <tokenstringcontext.hxx>
#include <mtvcellfunc.hxx>
#include <clipcontext.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <conditio.hxx>
#include <formulagroup.hxx>
#include <tokenarray.hxx>
#include <globalnames.hxx>
#include <scitems.hxx>
#include <cellform.hxx>
#include <sharedformula.hxx>

#include <svl/sharedstringpool.hxx>

#include <vector>
#include <cassert>

#include <boost/shared_ptr.hpp>

bool ScColumn::IsMerged( SCROW nRow ) const
{
    return pAttrArray->IsMerged(nRow);
}

void ScColumn::DeleteBeforeCopyFromClip( sc::CopyFromClipContext& rCxt, const ScColumn& rClipCol )
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
    std::vector<sc::RowSpan> aDestSpans;
    SCROW nDestOffset = aRange.mnRow1 - nClipRow1;
    bool bContinue = true;
    while (bContinue)
    {
        sc::SingleColumnSpanSet::SpansType::const_iterator it = aSpans.begin(), itEnd = aSpans.end();
        for (; it != itEnd && bContinue; ++it)
        {
            const sc::RowSpan& r = *it;
            SCROW nDestRow1 = r.mnRow1 + nDestOffset;
            SCROW nDestRow2 = r.mnRow2 + nDestOffset;

            if (nDestRow1 > aRange.mnRow2)
            {
                // We're done.
                bContinue = false;
                continue;
            }

            if (nDestRow2 > aRange.mnRow2)
            {
                // Truncate this range, and set it as the last span.
                nDestRow2 = aRange.mnRow2;
                bContinue = false;
            }

            aDestSpans.push_back(sc::RowSpan(nDestRow1, nDestRow2));
        }

        nDestOffset += nClipRowLen;
    }

    std::vector<SCROW> aDeletedRows;
    sal_uInt16 nDelFlag = rCxt.getDeleteFlag();
    sc::ColumnBlockPosition aBlockPos;
    InitBlockPosition(aBlockPos);

    std::vector<sc::RowSpan>::const_iterator it = aDestSpans.begin(), itEnd = aDestSpans.end();
    for (; it != itEnd; ++it)
    {
        SCROW nRow1 = it->mnRow1;
        SCROW nRow2 = it->mnRow2;

        if (nDelFlag & IDF_CONTENTS)
            DeleteCells(aBlockPos, nRow1, nRow2, nDelFlag, aDeletedRows);

        if (nDelFlag & IDF_NOTE)
            DeleteCellNotes(aBlockPos, nRow1, nRow2);

        if (nDelFlag & IDF_EDITATTR)
            RemoveEditAttribs(nRow1, nRow2);

        // Delete attributes just now
        if (nDelFlag & IDF_ATTRIB)
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
        else if ((nDelFlag & IDF_HARDATTR) == IDF_HARDATTR)
            pAttrArray->DeleteHardAttr(nRow1, nRow2);
    }

    BroadcastCells(aDeletedRows, SC_HINT_DATACHANGED);
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
                // Compare the ScDocumentPool* to determine if we are copying within the
                // same document. If not, re-intern shared strings.
                svl::SharedStringPool* pSharedStringPool = (rCxt.getClipDoc()->GetPool() != pDocument->GetPool()) ?
                    &pDocument->GetSharedStringPool() : NULL;
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
        assert(nLen > 0);
        aFormulas.clear();
        aFormulas.reserve(nLen);

        ScAddress aPos(nCol, nRow1, nTab);

        if (nLen == 1)
        {
            // Single, ungrouped formula cell.
            ScFormulaCell* pCell = new ScFormulaCell(rSrc, *pDocument, aPos);
            pCell->StartListeningTo(aCxt);
            pCell->SetDirty();
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

ScPostIt* ScColumn::ReleaseNote( SCROW nRow )
{
    if (!ValidRow(nRow))
        return NULL;

    ScPostIt* p = NULL;
    maCellNotes.release(nRow, p);
    return p;
}

size_t ScColumn::GetNoteCount() const
{
    size_t nCount = 0;
    sc::CellNoteStoreType::const_iterator it = maCellNotes.begin(), itEnd = maCellNotes.end();
    for (; it != itEnd; ++it)
    {
        if (it->type != sc::element_type_cellnote)
            continue;

        nCount += it->size;
    }

    return nCount;
}

namespace {

class NoteCaptionCreator
{
    ScAddress maPos;
public:
    NoteCaptionCreator( SCTAB nTab, SCCOL nCol ) : maPos(nCol,0,nTab) {}

    void operator() ( size_t nRow, ScPostIt* p )
    {
        maPos.SetRow(nRow);
        p->GetOrCreateCaption(maPos);
    }
};

struct NoteCaptionCleaner
{
    void operator() ( size_t /*nRow*/, ScPostIt* p )
    {
        p->ForgetCaption();
    }
};

}

void ScColumn::CreateAllNoteCaptions()
{
    NoteCaptionCreator aFunc(nTab, nCol);
    sc::ProcessNote(maCellNotes, aFunc);
}

void ScColumn::ForgetNoteCaptions( SCROW nRow1, SCROW nRow2 )
{
    if (!ValidRow(nRow1) || !ValidRow(nRow2))
        return;

    NoteCaptionCleaner aFunc;
    sc::CellNoteStoreType::iterator it = maCellNotes.begin();
    sc::ProcessNote(it, maCellNotes, nRow1, nRow2, aFunc);
}

SCROW ScColumn::GetNotePosition( size_t nIndex ) const
{
    // Return the row position of the nth note in the column.

    sc::CellNoteStoreType::const_iterator it = maCellNotes.begin(), itEnd = maCellNotes.end();

    size_t nCount = 0; // Number of notes encountered so far.
    for (; it != itEnd; ++it)
    {
        if (it->type != sc::element_type_cellnote)
            // Skip the empty blocks.
            continue;

        if (nIndex < nCount + it->size)
        {
            // Index falls within this block.
            size_t nOffset = nIndex - nCount;
            return it->position + nOffset;
        }

        nCount += it->size;
    }

    return -1;
}

namespace {

class NoteEntryCollector
{
    std::vector<sc::NoteEntry>& mrNotes;
    SCTAB mnTab;
    SCCOL mnCol;
    SCROW mnStartRow;
    SCROW mnEndRow;
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
            mrNotes.push_back(sc::NoteEntry(aPos, *it));
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

    std::for_each(it, itEnd, NoteEntryCollector(rNotes, nTab, nCol, nStartRow, nEndRow));
}

namespace {

class RecompileByOpcodeHandler
{
    ScDocument* mpDoc;
    const boost::unordered_set<OpCode>& mrOps;
    sc::EndListeningContext& mrEndListenCxt;
    sc::CompileFormulaContext& mrCompileFormulaCxt;

public:
    RecompileByOpcodeHandler(
        ScDocument* pDoc, const boost::unordered_set<OpCode>& rOps,
        sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt ) :
        mpDoc(pDoc),
        mrOps(rOps),
        mrEndListenCxt(rEndListenCxt),
        mrCompileFormulaCxt(rCompileCxt) {}

    void operator() ( sc::FormulaGroupEntry& rEntry )
    {
        // Perform end listening, remove from formula tree, and set them up
        // for re-compilation.

        ScFormulaCell* pTop = NULL;

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
            if (pTop->GetMatrixFlag() != MM_NONE && n > 0)
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
                ScTokenArray* pNewCode = aComp.CompileString(aFormula);
                ScFormulaCellGroupRef xGroup = pTop->GetCellGroup();
                assert(xGroup);
                xGroup->setCode(pNewCode);
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
                ScTokenArray* pNewCode = aComp.CompileString(aFormula);

                // Generate RPN tokens.
                ScCompiler aComp2(mpDoc, pCell->aPos, *pNewCode);
                aComp2.CompileTokenArray();

                pCell->SetCode(pNewCode);
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

    boost::unordered_set<OpCode> aOps;
    aOps.insert(ocBad);
    aOps.insert(ocColRowName);
    aOps.insert(ocName);
    RecompileByOpcodeHandler aFunc(pDocument, aOps, rEndListenCxt, rCompileCxt);
    std::for_each(aGroups.begin(), aGroups.end(), aFunc);
}

void ScColumn::PreprocessDBDataUpdate(
    sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    // Collect all formula groups.
    std::vector<sc::FormulaGroupEntry> aGroups = GetFormulaGroupEntries();

    boost::unordered_set<OpCode> aOps;
    aOps.insert(ocBad);
    aOps.insert(ocColRowName);
    aOps.insert(ocDBArea);
    RecompileByOpcodeHandler aFunc(pDocument, aOps, rEndListenCxt, rCompileCxt);
    std::for_each(aGroups.begin(), aGroups.end(), aFunc);
}

void ScColumn::CompileHybridFormula(
    sc::StartListeningContext& rStartListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    // Collect all formula groups.
    std::vector<sc::FormulaGroupEntry> aGroups = GetFormulaGroupEntries();

    CompileHybridFormulaHandler aFunc(pDocument, rStartListenCxt, rCompileCxt);
    std::for_each(aGroups.begin(), aGroups.end(), aFunc);
}

namespace {

class ScriptTypeUpdater
{
    ScColumn& mrCol;
    sc::CellTextAttrStoreType& mrTextAttrs;
    sc::CellTextAttrStoreType::iterator miPosAttr;
    ScConditionalFormatList* mpCFList;
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
        if (rAttr.mnScriptType != SC_SCRIPTTYPE_UNKNOWN)
            // Script type already deteremined.  Skip it.
            return;

        const ScPatternAttr* pPat = mrCol.GetPattern(nRow);
        if (!pPat)
            // In theory this should never return NULL. But let's be safe.
            return;

        const SfxItemSet* pCondSet = NULL;
        if (mpCFList)
        {
            maPos.SetRow(nRow);
            const ScCondFormatItem& rItem =
                static_cast<const ScCondFormatItem&>(pPat->GetItem(ATTR_CONDITIONAL));
            const std::vector<sal_uInt32>& rData = rItem.GetCondFormatData();
            pCondSet = mrCol.GetDoc().GetCondResult(rCell, maPos, *mpCFList, rData);
        }

        OUString aStr;
        Color* pColor;
        sal_uLong nFormat = pPat->GetNumberFormat(mpFormatter, pCondSet);
        ScCellFormat::GetString(rCell, nFormat, aStr, &pColor, *mpFormatter, &mrCol.GetDoc());

        rAttr.mnScriptType = mrCol.GetDoc().GetStringScriptType(aStr);
        mbUpdated = true;
    }

public:
    ScriptTypeUpdater( ScColumn& rCol ) :
        mrCol(rCol),
        mrTextAttrs(rCol.GetCellAttrStore()),
        miPosAttr(mrTextAttrs.begin()),
        mpCFList(rCol.GetDoc().GetCondFormList(rCol.GetTab())),
        mpFormatter(rCol.GetDoc().GetFormatTable()),
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

    if (bPattern)
    {
        for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
        {
            const ScPatternAttr* pPat1 = GetPattern(nRow);
            const ScPatternAttr* pPat2 = rOther.GetPattern(nRow);
            if (pPat1 != pPat2)
            {
                SetPattern(nRow, *pPat2, true);
                rOther.SetPattern(nRow, *pPat1, true);
            }
        }
    }

    CellStorageModified();
    rOther.CellStorageModified();
}

namespace {

class FormulaColPosSetter
{
    SCCOL mnCol;
public:
    FormulaColPosSetter( SCCOL nCol ) : mnCol(nCol) {}

    void operator() ( size_t nRow, ScFormulaCell* pCell )
    {
        if (!pCell->IsShared() || pCell->IsSharedTop())
        {
            // Ensure that the references still point to the same locations
            // after the position change.
            ScAddress aOldPos = pCell->aPos;
            pCell->aPos.SetCol(mnCol);
            pCell->aPos.SetRow(nRow);
            pCell->GetCode()->AdjustReferenceOnMovedOrigin(aOldPos, pCell->aPos);
        }
        else
        {
            pCell->aPos.SetCol(mnCol);
            pCell->aPos.SetRow(nRow);
        }
    }
};

}

void ScColumn::ResetFormulaCellPositions( SCROW nRow1, SCROW nRow2 )
{
    FormulaColPosSetter aFunc(nCol);
    sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aFunc);
}

namespace {

class RelativeRefBoundChecker
{
    std::vector<SCROW> maBounds;
    ScRange maBoundRange;

public:
    RelativeRefBoundChecker( const ScRange& rBoundRange ) :
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
    std::vector<SCROW> aBounds;

    // Cut at row boundaries first.
    aBounds.push_back(rBoundRange.aStart.Row());
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
    ListenerCollector( std::vector<SvtListener*>& rListener ) :
        mrListeners(rListener) {}

    void operator() ( size_t /*nRow*/, SvtBroadcaster* p )
    {
        SvtBroadcaster::ListenersType& rLis = p->GetAllListeners();
        std::copy(rLis.begin(), rLis.end(), std::back_inserter(mrListeners));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
