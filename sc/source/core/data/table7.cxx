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
#include <segmenttree.hxx>
#include <sharedformula.hxx>
#include <cellvalues.hxx>
#include <olinetab.hxx>
#include <tabprotection.hxx>
#include <columniterator.hxx>
#include <drwlayer.hxx>
#include <compressedarray.hxx>

#include <sal/log.hxx>
#include <tools/stream.hxx>

bool ScTable::IsMerged( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidCol(nCol) || nCol >= GetAllocatedColumnsCount() )
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
    sc::CopyFromClipContext& rCxt, const SCCOL nCol1, const SCROW nRow1, const SCCOL nCol2, const SCROW nRow2, const SCROW nSrcRow, const ScTable* pSrcTab )
{
    ScRange aSrcRange = rCxt.getClipDoc()->GetClipParam().getWholeRange();
    SCCOL nSrcColSize = aSrcRange.aEnd.Col() - aSrcRange.aStart.Col() + 1;

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
    {
        SCCOL nColOffset = nCol - nCol1;
        nColOffset = nColOffset % nSrcColSize;
        assert(nColOffset >= 0);
        CreateColumnIfNotExists(nCol).CopyOneCellFromClip(rCxt, nRow1, nRow2, nColOffset);

        if (rCxt.getInsertFlag() & InsertDeleteFlags::ATTRIB)
        {
            for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
                CopyConditionalFormat(nCol, nRow, nCol, nRow, nCol - aSrcRange.aStart.Col() - nColOffset,
                        nRow - nSrcRow, pSrcTab);
        }
    }

    if (nCol1 == 0 && nCol2 == rDocument.MaxCol() && mpRowHeights)
    {
        mpRowHeights->setValue(nRow1, nRow2, pSrcTab->GetOriginalHeight(nSrcRow));

        if (pRowFlags && pSrcTab->pRowFlags) {
           if (pSrcTab->pRowFlags->GetValue(nSrcRow) & CRFlags::ManualSize)
               pRowFlags->OrValue(nRow1, CRFlags::ManualSize);
           else
               pRowFlags->AndValue(nRow1, ~CRFlags::ManualSize);
        }
    }

    // Copy graphics over too
    bool bCopyGraphics
        = (rCxt.getInsertFlag() & InsertDeleteFlags::OBJECTS) != InsertDeleteFlags::NONE;
    if (!(bCopyGraphics && rCxt.getClipDoc()->mpDrawLayer))
        return;

    ScDrawLayer* pDrawLayer = GetDoc().GetDrawLayer();
    OSL_ENSURE(pDrawLayer, "No drawing layer");
    if (pDrawLayer)
    {
        const ScAddress aSrcStartPos
            = rCxt.getClipDoc()->GetClipParam().getWholeRange().aStart;
        const ScAddress aSrcEndPos = rCxt.getClipDoc()->GetClipParam().getWholeRange().aEnd;
        tools::Rectangle aSourceRect = rCxt.getClipDoc()->GetMMRect(
            aSrcStartPos.Col(), aSrcStartPos.Row(), aSrcEndPos.Col(), aSrcEndPos.Row(),
            aSrcStartPos.Tab());
        tools::Rectangle aDestRect = GetDoc().GetMMRect(nCol1, nRow1, nCol2, nRow2, nTab);
        pDrawLayer->CopyFromClip(rCxt.getClipDoc()->mpDrawLayer.get(), aSrcStartPos.Tab(),
                                 aSourceRect, ScAddress(nCol1, nRow1, nTab), aDestRect);
    }
}

void ScTable::SetValues( const SCCOL nCol, const SCROW nRow, const std::vector<double>& rVals )
{
    if (!ValidCol(nCol))
        return;

    CreateColumnIfNotExists(nCol).SetValues(nRow, rVals);
}

void ScTable::TransferCellValuesTo( const SCCOL nCol, SCROW nRow, size_t nLen, sc::CellValues& rDest )
{
    if (!ValidCol(nCol))
        return;

    CreateColumnIfNotExists(nCol).TransferCellValuesTo(nRow, nLen, rDest);
}

void ScTable::CopyCellValuesFrom( const SCCOL nCol, SCROW nRow, const sc::CellValues& rSrc )
{
    if (!ValidCol(nCol))
        return;

    CreateColumnIfNotExists(nCol).CopyCellValuesFrom(nRow, rSrc);
}

void ScTable::ConvertFormulaToValue(
    sc::EndListeningContext& rCxt, const SCCOL nCol1, const SCROW nRow1, const SCCOL nCol2, const SCROW nRow2,
    sc::TableValues* pUndo )
{
    if (!ValidCol(nCol1) || !ValidCol(nCol2) || nCol1 > nCol2)
        return;

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
         CreateColumnIfNotExists(nCol).ConvertFormulaToValue(rCxt, nRow1, nRow2, pUndo);
}

void ScTable::SwapNonEmpty(
    sc::TableValues& rValues, sc::StartListeningContext& rStartCxt, sc::EndListeningContext& rEndCxt )
{
    const ScRange& rRange = rValues.getRange();
    assert(rRange.IsValid());
    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
        CreateColumnIfNotExists(nCol).SwapNonEmpty(rValues, rStartCxt, rEndCxt);
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

void ScTable::UpdateScriptTypes( const SCCOL nCol1, SCROW nRow1, const SCCOL nCol2, SCROW nRow2 )
{
    if (!IsColValid(nCol1) || !ValidCol(nCol2) || nCol1 > nCol2)
        return;

    const SCCOL nMaxCol2 = std::min<SCCOL>( nCol2, aCol.size() - 1 );

    for (SCCOL nCol = nCol1; nCol <= nMaxCol2; ++nCol)
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
    if (!IsColValid(nCol))
        return;

    sc::SharedFormulaUtil::splitFormulaCellGroups(&GetDoc(), aCol[nCol].maCells, rRows);
}

void ScTable::UnshareFormulaCells( SCCOL nCol, std::vector<SCROW>& rRows )
{
    if (!IsColValid(nCol))
        return;

    sc::SharedFormulaUtil::unshareFormulaCells(&rDocument, aCol[nCol].maCells, rRows);
}

void ScTable::RegroupFormulaCells( SCCOL nCol )
{
    if (!IsColValid(nCol))
        return;

    aCol[nCol].RegroupFormulaCells();
}

bool ScTable::HasFormulaCell( const SCCOL nCol1, SCROW nRow1, const SCCOL nCol2, SCROW nRow2 ) const
{
    if (nCol2 < nCol1 || !IsColValid(nCol1) || !ValidCol(nCol2))
        return false;

    const SCCOL nMaxCol2 = std::min<SCCOL>( nCol2, aCol.size() - 1 );

    for (SCCOL nCol = nCol1; nCol <= nMaxCol2; ++nCol)
        if (aCol[nCol].HasFormulaCell(nRow1, nRow2))
            return true;

    return false;
}

void ScTable::EndListeningIntersectedGroup(
    sc::EndListeningContext& rCxt, SCCOL nCol, SCROW nRow, std::vector<ScAddress>* pGroupPos )
{
    if (!IsColValid(nCol))
        return;

    aCol[nCol].EndListeningIntersectedGroup(rCxt, nRow, pGroupPos);
}

void ScTable::EndListeningIntersectedGroups(
    sc::EndListeningContext& rCxt, const SCCOL nCol1, SCROW nRow1, const SCCOL nCol2, SCROW nRow2,
    std::vector<ScAddress>* pGroupPos )
{
    if (nCol2 < nCol1 || !IsColValid(nCol1) || !ValidCol(nCol2))
        return;

    for (SCCOL nCol : GetColumnsRange(nCol1, nCol2))
        aCol[nCol].EndListeningIntersectedGroups(rCxt, nRow1, nRow2, pGroupPos);
}

void ScTable::EndListeningGroup( sc::EndListeningContext& rCxt, const SCCOL nCol, SCROW nRow )
{
    if (!IsColValid(nCol))
        return;

    aCol[nCol].EndListeningGroup(rCxt, nRow);
}

void ScTable::SetNeedsListeningGroup( SCCOL nCol, SCROW nRow )
{
    if (!ValidCol(nCol))
        return;

    CreateColumnIfNotExists(nCol).SetNeedsListeningGroup(nRow);
}

bool ScTable::IsEditActionAllowed(
    sc::ColRowEditAction eAction, SCCOLROW nStart, SCCOLROW nEnd ) const
{
    if (!IsProtected())
    {
        SCCOL nCol1 = 0, nCol2 = aCol.size() - 1;
        SCROW nRow1 = 0, nRow2 = rDocument.MaxRow();

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
            if (HasBlockMatrixFragment(nStart, 0, nEnd, rDocument.MaxRow()))
                return false;

            return pTabProtection->isOptionEnabled(ScTableProtection::INSERT_COLUMNS);
        }
        case sc::ColRowEditAction::InsertRowsBefore:
        case sc::ColRowEditAction::InsertRowsAfter:
        {
            // TODO: improve the matrix range handling for the insert-before action.
            if (HasBlockMatrixFragment(0, nStart, rDocument.MaxCol(), nEnd))
                return false;

            return pTabProtection->isOptionEnabled(ScTableProtection::INSERT_ROWS);
        }
        case sc::ColRowEditAction::DeleteColumns:
        {
            if (!pTabProtection->isOptionEnabled(ScTableProtection::DELETE_COLUMNS))
                return false;

            return !HasAttrib(nStart, 0, nEnd, rDocument.MaxRow(), HasAttrFlags::Protected);
        }
        case sc::ColRowEditAction::DeleteRows:
        {
            if (!pTabProtection->isOptionEnabled(ScTableProtection::DELETE_ROWS))
                return false;

            return !HasAttrib(0, nStart, rDocument.MaxCol(), nEnd, HasAttrFlags::Protected);
        }
        default:
            ;
    }

    return false;
}

std::unique_ptr<sc::ColumnIterator> ScTable::GetColumnIterator( SCCOL nCol, SCROW nRow1, SCROW nRow2 ) const
{
    if (!ValidCol(nCol))
        return std::unique_ptr<sc::ColumnIterator>();

    return CreateColumnIfNotExists(nCol).GetColumnIterator(nRow1, nRow2);
}

bool ScTable::EnsureFormulaCellResults( const SCCOL nCol1, SCROW nRow1, const SCCOL nCol2, SCROW nRow2, bool bSkipRunning )
{
    if (nCol2 < nCol1 || !IsColValid(nCol1) || !ValidCol(nCol2))
        return false;

    const SCCOL nMaxCol2 = std::min<SCCOL>( nCol2, aCol.size() - 1 );

    bool bAnyDirty = false;

    for (SCCOL nCol = nCol1; nCol <= nMaxCol2; ++nCol)
    {
        bool bRet = aCol[nCol].EnsureFormulaCellResults(nRow1, nRow2, bSkipRunning);
        bAnyDirty = bAnyDirty || bRet;
    }

    return bAnyDirty;
}

void ScTable::finalizeOutlineImport()
{
    if (pOutlineTable && pRowFlags)
    {
        pOutlineTable->GetRowArray().finalizeImport(*this);
    }
}

void ScTable::StoreToCache(SvStream& rStrm) const
{
    SCCOL nStartCol = 0;
    SCCOL nEndCol = rDocument.MaxCol();
    SCROW nStartRow = 0;
    SCROW nEndRow = rDocument.MaxRow();

    GetDataArea(nStartCol, nStartRow, nEndCol, nEndRow, false, false);

    rStrm.WriteUInt64(nEndCol + 1);
    for (SCCOL nCol = 0; nCol <= nEndCol; ++nCol)
    {
        aCol[nCol].StoreToCache(rStrm);
    }
}

void ScTable::RestoreFromCache(SvStream& rStrm)
{
    sal_uInt64 nCols = 0;
    rStrm.ReadUInt64(nCols);
    for (SCCOL nCol = 0; nCol < static_cast<SCCOL>(nCols); ++nCol)
    {
        aCol[nCol].RestoreFromCache(rStrm);
    }
}

OString ScTable::dumpSheetGeomData(bool bColumns, SheetGeomType eGeomType)
{
    switch (eGeomType)
    {
        case SheetGeomType::SIZES:
            // returns a non-empty space separated list of spans with trailing space.
            // The format of the span is <size of any row/col in the span in print twips>:<last row/col of the span>
            // Example (for columns with three spans if MAXCOL is 1023):   "1280:3 1049:50 1280:1023"
            return dumpColumnRowSizes(bColumns);
        case SheetGeomType::HIDDEN:
            // returns a non-empty space separated list of spans with trailing space.
            // The format of the span is:
            // 1) First span:         <1 (span is hidden) / 0 (not hidden)>:<last row/col of the span>
            // 2) Rest of the spans:  <last row/col of the span>
            // The hidden state of the spans after the first can be inferred from the first span's flag as no adjacent
            // spans can have the same state by definition of span.
            return dumpHiddenFiltered(bColumns, /*bHidden*/ true);
        case SheetGeomType::FILTERED:
            // has exactly the same format as 'hidden'.
            return dumpHiddenFiltered(bColumns, /*bHidden*/ false);
        case SheetGeomType::GROUPS:
            // returns a space separated list of 'levels' with trailing space.
            // A 'level' is a comma separated list of groups(outline entries) with trailing comma.
            // format of a group is:
            // <start row/col of group>:<number of rows/cols in the group>:<1/0(group is hidden?)>:<1/0(control is visible?)>
            return dumpColumnRowGroups(bColumns);
        default:
            ;
    }

    return "";
}

OString ScTable::dumpColumnRowSizes(bool bColumns)
{
    // If the data-structures are not available, just report that all
    // rows/cols have the default sizes.
    static const OString aDefaultForCols
        = OString::number(STD_COL_WIDTH) + ":" + OString::number(MAXCOL) + " ";
    static const OString aDefaultForRows
        = OString::number(ScGlobal::nStdRowHeight) + ":" + OString::number(MAXROW) + " ";

    // ScCompressedArray is a template class and we don't want to impose
    // the restriction that its value type should be string serializable,
    // instead just operate on the specialized object.
    typedef ScCompressedArray<SCCOL, sal_uInt16> ColWidthsType;
    auto dumpColWidths = [](const ColWidthsType& rWidths) -> OString {
        OString aOutput;
        OString aSegment;
        SCCOL nStartCol = 0;
        const SCCOL nMaxCol = std::min(rWidths.GetLastPos(), MAXCOL);
        size_t nDummy = 0;
        while (nStartCol <= nMaxCol)
        {
            SCCOL nEndCol;
            sal_uInt16 nWidth = rWidths.GetValue(nStartCol, nDummy, nEndCol);
            // The last span nEndCol is always MAXCOL+1 for some reason, and we don't want that.
            if (nEndCol > nMaxCol)
                nEndCol = nMaxCol;
            aSegment = OString::number(nWidth) + ":" + OString::number(nEndCol) + " ";
            aOutput += aSegment;
            nStartCol = nEndCol + 1;
        }

        return aOutput;
    };

    if (bColumns)
        return mpColWidth ? dumpColWidths(*mpColWidth) : aDefaultForCols;

    return mpRowHeights ? mpRowHeights->dumpAsString() : aDefaultForRows;
}

OString ScTable::dumpHiddenFiltered(bool bColumns, bool bHidden)
{
    // defaults to no hidden/filtered row/cols.
    static const OString aDefaultForCols = "0:" + OString::number(MAXCOL) + " ";
    static const OString aDefaultForRows = "0:" + OString::number(MAXROW) + " ";

    if (bHidden)
    {
        if (bColumns)
            return mpHiddenCols ? mpHiddenCols->dumpAsString() : aDefaultForCols;

        return mpHiddenRows ? mpHiddenRows->dumpAsString() : aDefaultForRows;
    }

    if (bColumns)
        return mpFilteredCols ? mpFilteredCols->dumpAsString() : aDefaultForCols;

    return mpFilteredRows ? mpFilteredRows->dumpAsString() : aDefaultForRows;
}

OString ScTable::dumpColumnRowGroups(bool bColumns) const
{
    if (!pOutlineTable)
        return "";

    if (bColumns)
        return pOutlineTable->GetColArray().dumpAsString();

    return pOutlineTable->GetRowArray().dumpAsString();
}

SCCOL ScTable::GetLOKFreezeCol() const
{
    return maLOKFreezeCell.Col();
}

SCROW ScTable::GetLOKFreezeRow() const
{
    return maLOKFreezeCell.Row();
}

bool ScTable::SetLOKFreezeCol(SCCOL nFreezeCol)
{
    if (!ValidCol(nFreezeCol))
    {
        SAL_WARN("sc.core", "ScTable::SetLOKFreezeCol : invalid nFreezeCol = " << nFreezeCol);
        return false;
    }

    if (maLOKFreezeCell.Col() != nFreezeCol)
    {
        maLOKFreezeCell.SetCol(nFreezeCol);
        return true;
    }

    return false;
}

bool ScTable::SetLOKFreezeRow(SCROW nFreezeRow)
{
    if (!ValidRow(nFreezeRow))
    {
        SAL_WARN("sc.core", "ScTable::SetLOKFreezeRow : invalid nFreezeRow = " << nFreezeRow);
        return false;
    }

    if (maLOKFreezeCell.Row() != nFreezeRow)
    {
        maLOKFreezeCell.SetRow(nFreezeRow);
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
