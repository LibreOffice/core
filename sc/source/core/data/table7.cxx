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
#include <cellvalues.hxx>
#include "olinetab.hxx"
#include <tabprotection.hxx>
#include <columniterator.hxx>

bool ScTable::IsMerged( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidCol(nCol))
        return false;

    return aCol[nCol].IsMerged(nRow);
}

sc::MultiDataCellState ScTable::HasMultipleDataCells( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const
{
    if (!ValidColRow(nCol1, nRow1) || !ValidColRow(nCol2, nRow2))
        return sc::MultiDataCellState();

    if (nCol1 > nCol2 || nRow1 > nRow2)
        // invalid range.
        return sc::MultiDataCellState();

    if (aCol.empty())
        return sc::MultiDataCellState(sc::MultiDataCellState::Empty);

    auto setFirstCell = []( sc::MultiDataCellState& rRet, SCCOL nCurCol, SCROW nCurRow )
    {
        if (rRet.mnCol1 < 0)
        {
            // First cell not yet set.  Set it.
            rRet.mnCol1 = nCurCol;
            rRet.mnRow1 = nCurRow;
        }
    };

    SCCOL nMaxCol = aCol.size()-1;
    bool bHasOne = false;
    sc::MultiDataCellState aRet(sc::MultiDataCellState::Empty);

    for (SCCOL nCol = nCol1; nCol <= nCol2 && nCol <= nMaxCol; ++nCol)
    {
        SCROW nFirstDataRow = -1;
        switch (aCol[nCol].HasDataCellsInRange(nRow1, nRow2, &nFirstDataRow))
        {
            case sc::MultiDataCellState::HasOneCell:
            {
                setFirstCell(aRet, nCol, nFirstDataRow);

                if (bHasOne)
                {
                    // We've already found one data cell in another column.
                    aRet.meState = sc::MultiDataCellState::HasMultipleCells;
                    return aRet;
                }
                bHasOne = true;
                break;
            }
            case sc::MultiDataCellState::HasMultipleCells:
            {
                setFirstCell(aRet, nCol, nFirstDataRow);

                aRet.meState = sc::MultiDataCellState::HasMultipleCells;
                return aRet;
            }
            case sc::MultiDataCellState::Empty:
            default:
                ;
        }
    }

    if (bHasOne)
        aRet.meState = sc::MultiDataCellState::HasOneCell;

    return aRet;
}

void ScTable::DeleteBeforeCopyFromClip(
    sc::CopyFromClipContext& rCxt, const ScTable& rClipTab, sc::ColumnSpanSet& rBroadcastSpans )
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
        for (SCCOL nCol = aRange.mnCol1; nCol <= aRange.mnCol2; ++nCol, ++nClipCol)
        {
            if (nClipCol > aClipRange.aEnd.Col())
                nClipCol = aClipRange.aStart.Col(); // loop through columns.

            const ScColumn& rClipCol = rClipTab.aCol[nClipCol];
            aCol[nCol].DeleteBeforeCopyFromClip(rCxt, rClipCol, rBroadcastSpans);
        }
    }

    SetStreamValid(false);
}

void ScTable::CopyOneCellFromClip(
    sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, SCROW nSrcRow, ScTable* pSrcTab )
{
    ScRange aSrcRange = rCxt.getClipDoc()->GetClipParam().getWholeRange();
    SCCOL nSrcColSize = aSrcRange.aEnd.Col() - aSrcRange.aStart.Col() + 1;

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
    {
        SCCOL nColOffset = nCol - nCol1;
        nColOffset = nColOffset % nSrcColSize;
        assert(nColOffset >= 0);
        aCol[nCol].CopyOneCellFromClip(rCxt, nRow1, nRow2, nColOffset);

        if ((rCxt.getInsertFlag() & InsertDeleteFlags::ATTRIB) && (nColOffset == 0))
        {
            for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
                CopyConditionalFormat(nCol, nRow, nCol + nSrcColSize - 1, nRow, nCol - aSrcRange.aStart.Col(),
                        nRow - nSrcRow, pSrcTab);
        }
    }

    if (nCol1 == 0 && nCol2 == MAXCOL && mpRowHeights)
        mpRowHeights->setValue(nRow1, nRow2, pSrcTab->GetOriginalHeight(nSrcRow));
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

void ScTable::ConvertFormulaToValue(
    sc::EndListeningContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    sc::TableValues* pUndo )
{
    if (!ValidCol(nCol1) || !ValidCol(nCol2) || nCol1 > nCol2)
        return;

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].ConvertFormulaToValue(rCxt, nRow1, nRow2, pUndo);
}

void ScTable::SwapNonEmpty(
    sc::TableValues& rValues, sc::StartListeningContext& rStartCxt, sc::EndListeningContext& rEndCxt )
{
    const ScRange& rRange = rValues.getRange();
    assert(rRange.IsValid());
    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
        aCol[nCol].SwapNonEmpty(rValues, rStartCxt, rEndCxt);
}

void ScTable::PreprocessRangeNameUpdate(
    sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    for (SCCOL i = 0; i < aCol.size(); ++i)
        aCol[i].PreprocessRangeNameUpdate(rEndListenCxt, rCompileCxt);
}

void ScTable::PreprocessDBDataUpdate(
    sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    for (SCCOL i = 0; i < aCol.size(); ++i)
        aCol[i].PreprocessDBDataUpdate(rEndListenCxt, rCompileCxt);
}

void ScTable::CompileHybridFormula(
    sc::StartListeningContext& rStartListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    for (SCCOL i = 0; i < aCol.size(); ++i)
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

void ScTable::SplitFormulaGroups( SCCOL nCol, std::vector<SCROW>& rRows )
{
    if (!ValidCol(nCol))
        return;

    sc::SharedFormulaUtil::splitFormulaCellGroups(aCol[nCol].maCells, rRows);
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

void ScTable::CollectListeners(
    std::vector<SvtListener*>& rListeners, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    if (nCol2 < nCol1 || !ValidCol(nCol1) || !ValidCol(nCol2))
        return;

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].CollectListeners(rListeners, nRow1, nRow2);
}

bool ScTable::HasFormulaCell( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const
{
    if (nCol2 < nCol1 || !ValidCol(nCol1) || !ValidCol(nCol2))
        return false;

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        if (aCol[nCol].HasFormulaCell(nRow1, nRow2))
            return true;

    return false;
}

void ScTable::EndListeningIntersectedGroup(
    sc::EndListeningContext& rCxt, SCCOL nCol, SCROW nRow, std::vector<ScAddress>* pGroupPos )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].EndListeningIntersectedGroup(rCxt, nRow, pGroupPos);
}

void ScTable::EndListeningIntersectedGroups(
    sc::EndListeningContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    std::vector<ScAddress>* pGroupPos )
{
    if (nCol2 < nCol1 || !ValidCol(nCol1) || !ValidCol(nCol2))
        return;

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].EndListeningIntersectedGroups(rCxt, nRow1, nRow2, pGroupPos);
}

void ScTable::EndListeningGroup( sc::EndListeningContext& rCxt, SCCOL nCol, SCROW nRow )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].EndListeningGroup(rCxt, nRow);
}

void ScTable::SetNeedsListeningGroup( SCCOL nCol, SCROW nRow )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].SetNeedsListeningGroup(nRow);
}

bool ScTable::IsEditActionAllowed(
    sc::ColRowEditAction eAction, SCCOLROW nStart, SCCOLROW nEnd ) const
{
    if (!IsProtected())
    {
        SCCOL nCol1 = 0, nCol2 = MAXCOL;
        SCROW nRow1 = 0, nRow2 = MAXROW;

        switch (eAction)
        {
            case sc::ColRowEditAction::InsertColumnsBefore:
            case sc::ColRowEditAction::InsertColumnsAfter:
            case sc::ColRowEditAction::DeleteColumns:
            {
                nCol1 = nStart;
                nCol2 = nEnd;
                break;
            }
            case sc::ColRowEditAction::InsertRowsBefore:
            case sc::ColRowEditAction::InsertRowsAfter:
            case sc::ColRowEditAction::DeleteRows:
            {
                nRow1 = nStart;
                nRow2 = nEnd;
                break;
            }
            default:
                ;
        }

        return IsBlockEditable(nCol1, nRow1, nCol2, nRow2, nullptr);
    }

    if (IsScenario())
        // TODO: I don't even know what this scenario thingie is. Perhaps we
        // should check it against the scenario ranges?
        return false;

    assert(pTabProtection);

    switch (eAction)
    {
        case sc::ColRowEditAction::InsertColumnsBefore:
        case sc::ColRowEditAction::InsertColumnsAfter:
        {
            // TODO: improve the matrix range handling for the insert-before action.
            if (HasBlockMatrixFragment(nStart, 0, nEnd, MAXROW))
                return false;

            return pTabProtection->isOptionEnabled(ScTableProtection::INSERT_COLUMNS);
        }
        case sc::ColRowEditAction::InsertRowsBefore:
        case sc::ColRowEditAction::InsertRowsAfter:
        {
            // TODO: improve the matrix range handling for the insert-before action.
            if (HasBlockMatrixFragment(0, nStart, MAXCOL, nEnd))
                return false;

            return pTabProtection->isOptionEnabled(ScTableProtection::INSERT_ROWS);
        }
        case sc::ColRowEditAction::DeleteColumns:
        {
            if (!pTabProtection->isOptionEnabled(ScTableProtection::DELETE_COLUMNS))
                return false;

            return !HasAttrib(nStart, 0, nEnd, MAXROW, HasAttrFlags::Protected);
        }
        case sc::ColRowEditAction::DeleteRows:
        {
            if (!pTabProtection->isOptionEnabled(ScTableProtection::DELETE_ROWS))
                return false;

            return !HasAttrib(0, nStart, MAXCOL, nEnd, HasAttrFlags::Protected);
        }
        default:
            ;
    }

    return false;
}

std::unique_ptr<sc::ColumnIterator> ScTable::GetColumnIterator( SCCOL nCol, SCROW nRow1, SCROW nRow2 ) const
{
    if (!ValidCol(nCol) || nCol >= aCol.size())
        return std::unique_ptr<sc::ColumnIterator>();

    return aCol[nCol].GetColumnIterator(nRow1, nRow2);
}

void ScTable::EnsureFormulaCellResults( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    if (nCol2 < nCol1 || !ValidCol(nCol1) || !ValidCol(nCol2))
        return;

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].EnsureFormulaCellResults(nRow1, nRow2);
}

void ScTable::finalizeOutlineImport()
{
    if (pOutlineTable && pRowFlags)
    {
        pOutlineTable->GetRowArray().finalizeImport(*this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
