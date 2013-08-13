/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "column.hxx"

#include "scitems.hxx"
#include "formulacell.hxx"
#include "document.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "typedstrdata.hxx"
#include "formula/errorcodes.hxx"
#include "formula/token.hxx"
#include "brdcst.hxx"
#include "docoptio.hxx"         // GetStdPrecision for GetMaxNumberStringLen
#include "subtotal.hxx"
#include "markdata.hxx"
#include "detfunc.hxx"          // For Notes for DeleteRange
#include "postit.hxx"
#include "stringutil.hxx"
#include "docpool.hxx"
#include "globalnames.hxx"
#include "cellvalue.hxx"
#include "tokenarray.hxx"
#include "stlalgorithm.hxx"
#include "clipcontext.hxx"
#include "columnspanset.hxx"
#include "mtvcellfunc.hxx"
#include "scopetools.hxx"
#include "editutil.hxx"
#include "sharedformula.hxx"

#include <com/sun/star/i18n/LocaleDataItem.hpp>

#include <boost/scoped_ptr.hpp>

#include <mdds/flat_segment_tree.hpp>

#include <sfx2/objsh.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/broadcast.hxx>
#include "editeng/editstat.hxx"

#include <cstdio>

using ::com::sun::star::i18n::LocaleDataItem;

// Err527 Workaroand
extern const ScFormulaCell* pLastFormulaTreeTop; // in cellform.cxx
using namespace formula;

void ScColumn::Broadcast( SCROW nRow )
{
    ScHint aHint(SC_HINT_DATACHANGED, ScAddress(nCol, nRow, nTab));
    pDocument->Broadcast(aHint);
}

void ScColumn::BroadcastCells( const std::vector<SCROW>& rRows )
{
    if (rRows.empty())
        return;

    // Broadcast the changes.
    ScHint aHint(SC_HINT_DATACHANGED, ScAddress(nCol, 0, nTab));
    std::vector<SCROW>::const_iterator itRow = rRows.begin(), itRowEnd = rRows.end();
    for (; itRow != itRowEnd; ++itRow)
    {
        aHint.GetAddress().SetRow(*itRow);
        pDocument->Broadcast(aHint);
    }
}

struct DirtyCellInterpreter
{
    void operator() (size_t, ScFormulaCell* p)
    {
        if (p->GetDirty())
            p->Interpret();
    }
};

void ScColumn::InterpretDirtyCells( SCROW nRow1, SCROW nRow2 )
{
    DirtyCellInterpreter aFunc;
    sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aFunc);
}

void ScColumn::Delete( SCROW nRow )
{
    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it == maCells.end())
        return;

    if (it->type == sc::element_type_formula)
    {
        ScFormulaCell* p = sc::formula_block::at(*it->data, aPos.second);
        p->EndListeningTo(pDocument);
        sc::SharedFormulaUtil::unshareFormulaCell(aPos, *p);
    }
    maCells.set_empty(nRow, nRow);
    maCellTextAttrs.set_empty(nRow, nRow);

    Broadcast(nRow);
    CellStorageModified();
}

void ScColumn::FreeAll()
{
    // Keep a logical empty range of 0-MAXROW at all times.
    maCells.clear();
    maCells.resize(MAXROWCOUNT);
    maCellTextAttrs.clear();
    maCellTextAttrs.resize(MAXROWCOUNT);
    CellStorageModified();
}

namespace {

/**
 * Collect all formula cells for later mass-unregistration. Also tag row
 * positions of all non-empty cells in the range.
 */
class DeleteRowsHandler
{
    ScDocument& mrDoc;
    std::vector<SCROW> maRows;
    std::vector<ScFormulaCell*> maFormulaCells;
public:
    DeleteRowsHandler(ScDocument& rDoc) : mrDoc(rDoc) {}

    void operator() (size_t nRow, ScFormulaCell* pCell)
    {
        maFormulaCells.push_back(pCell);
        maRows.push_back(nRow);
    }

    void operator() (mdds::mtv::element_t nType, size_t nTopRow, size_t nDataSize)
    {
        if (nType == sc::element_type_empty)
            // Ignore empty cells.
            return;

        for (size_t i = 0; i < nDataSize; ++i)
            // Tag all non-empty cells.
            maRows.push_back(i + nTopRow);
    }

    void endFormulas()
    {
        mrDoc.EndListeningFormulaCells(maFormulaCells);
    }

    const std::vector<SCROW>& getNonEmptyRows() const
    {
        return maRows;
    }
};

class ShiftFormulaPosHandler
{
public:

    void operator() (size_t nRow, ScFormulaCell* pCell)
    {
        pCell->aPos.SetRow(nRow);
    }
};

class RangeBroadcaster
{
    ScDocument& mrDoc;
    ScHint maHint;
public:
    RangeBroadcaster(ScDocument& rDoc, SCTAB nTab, SCCOL nCol) :
        mrDoc(rDoc),
        maHint(SC_HINT_DATACHANGED, ScAddress(nCol, 0, nTab)) {}

    void operator() (const sc::SingleColumnSpanSet::Span& rSpan)
    {
        SCROW nRow1 = rSpan.mnRow1, nRow2 = rSpan.mnRow2;
        maHint.GetAddress().SetRow(nRow1);
        ScRange aRange(maHint.GetAddress());
        aRange.aEnd.SetRow(nRow2);
        mrDoc.AreaBroadcastInRange(aRange, maHint);
    }
};

}

void ScColumn::DeleteRow( SCROW nStartRow, SCSIZE nSize )
{
    pAttrArray->DeleteRow( nStartRow, nSize );

    SCROW nEndRow = nStartRow + nSize - 1;

    maBroadcasters.erase(nStartRow, nEndRow);
    maBroadcasters.resize(MAXROWCOUNT);

    // See if we have any cells that would get deleted or shifted by deletion.
    sc::CellStoreType::position_type aPos = maCells.position(nStartRow);
    sc::CellStoreType::iterator itCell = aPos.first;
    if (itCell->type == sc::element_type_empty)
    {
        // This is an empty block. If this is the last block, then there is no cells to delete or shift.
        sc::CellStoreType::iterator itTest = itCell;
        ++itTest;
        if (itTest == maCells.end())
        {
            // No cells are affected by this deletion. Bail out.
            CellStorageModified(); // broadcast array has been modified.
            return;
        }
    }

    // Check if there are any cells below the end row that will get shifted.
    bool bShiftCells = false;
    aPos = maCells.position(itCell, nEndRow+1);
    itCell = aPos.first;
    if (itCell->type == sc::element_type_empty)
    {
        // This block is empty. See if there is any block that follows.
        sc::CellStoreType::iterator itTest = itCell;
        ++itTest;
        if (itTest != maCells.end())
            // Non-empty block follows -> cells that will get shifted.
            bShiftCells = true;
    }
    else
        bShiftCells = true;

    sc::SingleColumnSpanSet aNonEmptySpans;
    if (bShiftCells)
    {
        // Mark all non-empty cell positions below the end row.
        sc::ColumnBlockConstPosition aBlockPos;
        aBlockPos.miCellPos = itCell;
        aNonEmptySpans.scan(aBlockPos, *this, nEndRow+1, MAXROW);
    }

    sc::AutoCalcSwitch aACSwitch(*pDocument, false);

    // Parse all non-empty cells in the range to pick up their row positions,
    // and end all formula cells.
    DeleteRowsHandler aDeleteRowsFunc(*pDocument);
    sc::ProcessFormula(itCell, maCells, nStartRow, nEndRow, aDeleteRowsFunc, aDeleteRowsFunc);
    aDeleteRowsFunc.endFormulas();

    // Remove the cells.
    maCells.erase(nStartRow, nEndRow);
    maCells.resize(MAXROWCOUNT);

    // Get the position again after the container change.
    aPos = maCells.position(nStartRow);

    // Shift the formula cell positions below the start row.
    ShiftFormulaPosHandler aShiftFormulaFunc;
    sc::ProcessFormula(aPos.first, maCells, nStartRow, MAXROW, aShiftFormulaFunc);

    sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);

    // Single cell broadcasts on deleted cells.
    BroadcastCells(aDeleteRowsFunc.getNonEmptyRows());

    // Shift the text attribute array too (before the broadcast).
    maCellTextAttrs.erase(nStartRow, nEndRow);
    maCellTextAttrs.resize(MAXROWCOUNT);

    CellStorageModified();

    if (!bShiftCells)
        return;

    // Do area broadcast on the old non-empty cell ranges prior to the shift.
    sc::SingleColumnSpanSet::SpansType aSpans;
    aNonEmptySpans.getSpans(aSpans);
    std::for_each(aSpans.begin(), aSpans.end(), RangeBroadcaster(*pDocument, nTab, nCol));
}

sc::CellStoreType::iterator ScColumn::GetPositionToInsert( SCROW nRow )
{
    return GetPositionToInsert(maCells.begin(), nRow);
}

void ScColumn::JoinNewFormulaCell(
    const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell ) const
{
    // Check the previous row position for possible grouping.
    if (aPos.first->type == sc::element_type_formula && aPos.second > 0)
    {
        ScFormulaCell& rPrev = *sc::formula_block::at(*aPos.first->data, aPos.second-1);
        sc::CellStoreType::position_type aPosPrev = aPos;
        --aPosPrev.second;
        sc::SharedFormulaUtil::joinFormulaCells(aPosPrev, rPrev, rCell);
    }

    // Check the next row position for possible grouping.
    if (aPos.first->type == sc::element_type_formula && aPos.second+1 < aPos.first->size)
    {
        ScFormulaCell& rNext = *sc::formula_block::at(*aPos.first->data, aPos.second+1);
        sc::SharedFormulaUtil::joinFormulaCells(aPos, rCell, rNext);
    }
}

void ScColumn::DetachFormulaCell(
    const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell )
{
    if (!pDocument->IsClipOrUndo())
        // Have the dying formula cell stop listening.
        rCell.EndListeningTo(pDocument);

    sc::SharedFormulaUtil::unshareFormulaCell(aPos, rCell);
}

namespace {

class DetachFormulaCellsHandler
{
    ScDocument* mpDoc;
public:
    DetachFormulaCellsHandler(ScDocument* pDoc) : mpDoc(pDoc) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->EndListeningTo(mpDoc);
    }
};

}

void ScColumn::DetachFormulaCells(
    const sc::CellStoreType::position_type& aPos, size_t nLength )
{
    // Split formula grouping at the top and bottom boundaries.
    sc::SharedFormulaUtil::splitFormulaCellGroup(aPos);
    size_t nRow = aPos.first->position + aPos.second;
    size_t nNextTopRow = nRow + nLength; // start row of next formula group.
    if (ValidRow(nNextTopRow))
    {
        sc::CellStoreType::position_type aPos2 = maCells.position(aPos.first, nNextTopRow);
        sc::SharedFormulaUtil::splitFormulaCellGroup(aPos2);
    }

    if (pDocument->IsClipOrUndo())
        return;

    DetachFormulaCellsHandler aFunc(pDocument);
    sc::ProcessFormula(aPos.first, maCells, nRow, nNextTopRow-1, aFunc);
}

sc::CellStoreType::iterator ScColumn::GetPositionToInsert( const sc::CellStoreType::iterator& it, SCROW nRow )
{
    // See if we are overwriting an existing formula cell.
    sc::CellStoreType::position_type aPos = maCells.position(it, nRow);
    sc::CellStoreType::iterator itRet = aPos.first;
    if (itRet->type == sc::element_type_formula)
    {
        ScFormulaCell& rCell = *sc::formula_block::at(*itRet->data, aPos.second);
        DetachFormulaCell(aPos, rCell);
    }

    return itRet;
}

void ScColumn::ActivateNewFormulaCell(
    const sc::CellStoreType::iterator& itPos, SCROW nRow, ScFormulaCell& rCell, bool bJoin )
{
    ActivateNewFormulaCell(maCells.position(itPos, nRow), rCell, bJoin);
}

void ScColumn::ActivateNewFormulaCell(
    const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell, bool bJoin )
{
    if (bJoin)
        // See if this new formula cell can join an existing shared formula group.
        JoinNewFormulaCell(aPos, rCell);

    // When we insert from the Clipboard we still have wrong (old) References!
    // First they are rewired in CopyBlockFromClip via UpdateReference and the
    // we call StartListeningFromClip and BroadcastFromClip.
    // If we insert into the Clipboard/andoDoc, we do not use a Broadcast.
    // After Import we call CalcAfterLoad and in there Listening.
    if (!pDocument->IsClipOrUndo() && !pDocument->IsInsertingFromOtherDoc())
    {
        rCell.StartListeningTo(pDocument);
        if (!pDocument->IsCalcingAfterLoad())
            rCell.SetDirty();
    }
}

void ScColumn::BroadcastNewCell( SCROW nRow )
{
    // When we insert from the Clipboard we still have wrong (old) References!
    // First they are rewired in CopyBlockFromClip via UpdateReference and the
    // we call StartListeningFromClip and BroadcastFromClip.
    // If we insert into the Clipboard/andoDoc, we do not use a Broadcast.
    // After Import we call CalcAfterLoad and in there Listening.
    if (pDocument->IsClipOrUndo() || pDocument->IsInsertingFromOtherDoc() || pDocument->IsCalcingAfterLoad())
        return;

    Broadcast(nRow);
}

bool ScColumn::UpdateScriptType( sc::CellTextAttr& rAttr, SCROW nRow )
{
    if (rAttr.mnScriptType != SC_SCRIPTTYPE_UNKNOWN)
        // Already updated. Nothing to do.
        return false;

    // Script type not yet determined. Determine the real script
    // type, and store it.
    const ScPatternAttr* pPattern = GetPattern(nRow);
    if (!pPattern)
        return false;

    ScRefCellValue aCell;
    ScAddress aPos(nCol, nRow, nTab);
    aCell.assign(*pDocument, aPos);

    const SfxItemSet* pCondSet = NULL;
    ScConditionalFormatList* pCFList = pDocument->GetCondFormList(nTab);
    if (pCFList)
    {
        const ScCondFormatItem& rItem =
            static_cast<const ScCondFormatItem&>(pPattern->GetItem(ATTR_CONDITIONAL));
        const std::vector<sal_uInt32>& rData = rItem.GetCondFormatData();
        pCondSet = pDocument->GetCondResult(aCell, aPos, *pCFList, rData);
    }

    SvNumberFormatter* pFormatter = pDocument->GetFormatTable();

    OUString aStr;
    Color* pColor;
    sal_uLong nFormat = pPattern->GetNumberFormat(pFormatter, pCondSet);
    ScCellFormat::GetString(aCell, nFormat, aStr, &pColor, *pFormatter, pDocument);

    // Store the real script type to the array.
    rAttr.mnScriptType = pDocument->GetStringScriptType(aStr);
    return true;
}

namespace {

class DeleteAreaHandler
{
    ScDocument& mrDoc;
    std::vector<ScFormulaCell*> maFormulaCells;
    sc::SingleColumnSpanSet maDeleteRanges;

    bool mbNumeric:1;
    bool mbString:1;
    bool mbFormula:1;

public:
    DeleteAreaHandler(ScDocument& rDoc, sal_uInt16 nDelFlag) :
        mrDoc(rDoc),
        mbNumeric(nDelFlag & IDF_VALUE),
        mbString(nDelFlag & IDF_STRING),
        mbFormula(nDelFlag & IDF_FORMULA) {}

    void operator() (const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize)
    {
        switch (node.type)
        {
            case sc::element_type_numeric:
                if (!mbNumeric)
                    return;
            break;
            case sc::element_type_string:
            case sc::element_type_edittext:
                if (!mbString)
                    return;
            break;
            case sc::element_type_formula:
            {
                if (!mbFormula)
                    return;

                sc::formula_block::iterator it = sc::formula_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::formula_block::iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                for (; it != itEnd; ++it)
                    maFormulaCells.push_back(*it);
            }
            break;
            case sc::element_type_empty:
            default:
                return;
        }

        // Tag these cells for deletion.
        SCROW nRow1 = node.position + nOffset;
        SCROW nRow2 = nRow1 + nDataSize - 1;
        maDeleteRanges.set(nRow1, nRow2, true);
    }

    void endFormulas()
    {
        mrDoc.EndListeningFormulaCells(maFormulaCells);
    }

    const sc::SingleColumnSpanSet& getSpans() const
    {
        return maDeleteRanges;
    }
};

class EmptyCells
{
    ScColumn& mrColumn;
    sc::ColumnBlockPosition& mrPos;

    void splitFormulaGrouping(const sc::CellStoreType::position_type& rPos)
    {
        if (rPos.first->type == sc::element_type_formula)
        {
            ScFormulaCell& rCell = *sc::formula_block::at(*rPos.first->data, rPos.second);
            sc::SharedFormulaUtil::unshareFormulaCell(rPos, rCell);
        }
    }

public:
    EmptyCells(sc::ColumnBlockPosition& rPos, ScColumn& rColumn) :
        mrColumn(rColumn), mrPos(rPos) {}

    void operator() (const sc::SingleColumnSpanSet::Span& rSpan)
    {
        sc::CellStoreType& rCells = mrColumn.GetCellStore();

        // First, split formula grouping at the top and bottom boundaries
        // before emptying the cells.
        sc::CellStoreType::position_type aPos = rCells.position(mrPos.miCellPos, rSpan.mnRow1);
        splitFormulaGrouping(aPos);
        aPos = rCells.position(aPos.first, rSpan.mnRow2);
        splitFormulaGrouping(aPos);

        mrPos.miCellPos = rCells.set_empty(mrPos.miCellPos, rSpan.mnRow1, rSpan.mnRow2);
        mrPos.miCellTextAttrPos = mrColumn.GetCellAttrStore().set_empty(mrPos.miCellTextAttrPos, rSpan.mnRow1, rSpan.mnRow2);
    }
};

}

void ScColumn::DeleteArea(SCROW nStartRow, SCROW nEndRow, sal_uInt16 nDelFlag)
{
    sal_uInt16 nContMask = IDF_CONTENTS;
    // IDF_NOCAPTIONS needs to be passed too, if IDF_NOTE is set
    if( nDelFlag & IDF_NOTE )
        nContMask |= IDF_NOCAPTIONS;
    sal_uInt16 nContFlag = nDelFlag & nContMask;

    std::vector<SCROW> aDeletedRows;

    if (!IsEmptyData() && nContFlag)
    {
        // There are cells to delete.  Determine which cells to delete based on the deletion flags.
        DeleteAreaHandler aFunc(*pDocument, nDelFlag);
        sc::CellStoreType::iterator itPos = maCells.position(nStartRow).first;
        sc::ProcessBlock(itPos, maCells, aFunc, nStartRow, nEndRow);
        aFunc.endFormulas(); // Have the formula cells stop listening.
        aFunc.getSpans().getRows(aDeletedRows);

        // Get the deletion spans.
        sc::SingleColumnSpanSet::SpansType aSpans;
        aFunc.getSpans().getSpans(aSpans);

        sc::ColumnBlockPosition aBlockPos;
        aBlockPos.miCellPos = itPos;
        aBlockPos.miCellTextAttrPos = maCellTextAttrs.begin();

        // Delete the cells for real.
        std::for_each(aSpans.begin(), aSpans.end(), EmptyCells(aBlockPos, *this));
        CellStorageModified();
    }

    if ( nDelFlag & IDF_EDITATTR )
    {
        OSL_ENSURE( nContFlag == 0, "DeleteArea: Wrong Flags" );
        RemoveEditAttribs( nStartRow, nEndRow );
    }

    // Delete attributes just now
    if ((nDelFlag & IDF_ATTRIB) == IDF_ATTRIB)
        pAttrArray->DeleteArea( nStartRow, nEndRow );
    else if ((nDelFlag & IDF_ATTRIB) != 0)
        pAttrArray->DeleteHardAttr( nStartRow, nEndRow );

    // Broadcast on only cells that were deleted; no point broadcasting on
    // cells that were already empty before the deletion.
    BroadcastCells(aDeletedRows);
}

bool ScColumn::InitBlockPosition( sc::ColumnBlockPosition& rBlockPos )
{
    rBlockPos.miBroadcasterPos = maBroadcasters.begin();
    rBlockPos.miCellTextAttrPos = maCellTextAttrs.begin();
    rBlockPos.miCellPos = maCells.begin();
    return true;
}

bool ScColumn::InitBlockPosition( sc::ColumnBlockConstPosition& rBlockPos ) const
{
    rBlockPos.miBroadcasterPos = maBroadcasters.begin();
    rBlockPos.miCellTextAttrPos = maCellTextAttrs.begin();
    rBlockPos.miCellPos = maCells.begin();
    return true;
}

namespace {

class CopyAttrArrayByRange : std::unary_function<sc::SingleColumnSpanSet::Span, void>
{
    ScAttrArray& mrDestAttrArray;
    ScAttrArray& mrSrcAttrArray;
    long mnRowOffset;
public:
    CopyAttrArrayByRange(ScAttrArray& rDestAttrArray, ScAttrArray& rSrcAttrArray, long nRowOffset) :
        mrDestAttrArray(rDestAttrArray), mrSrcAttrArray(rSrcAttrArray), mnRowOffset(nRowOffset) {}

    void operator() (const sc::SingleColumnSpanSet::Span& rSpan)
    {
        mrDestAttrArray.CopyAreaSafe(
            rSpan.mnRow1+mnRowOffset, rSpan.mnRow2+mnRowOffset, mnRowOffset, mrSrcAttrArray);
    }
};

class CopyCellsFromClipHandler
{
    sc::CopyFromClipContext& mrCxt;
    ScColumn& mrSrcCol;
    ScColumn& mrDestCol;
    SCTAB mnTab;
    SCCOL mnCol;
    long mnRowOffset;
    sc::ColumnBlockPosition maDestBlockPos;
    sc::ColumnBlockPosition* mpDestBlockPos; // to save it for next iteration.

    bool isDateCell(SCROW nSrcRow) const
    {
        ScDocument* pSrcDoc = mrCxt.getClipDoc(); // clip document is the source.
        sal_uLong nNumIndex = static_cast<const SfxUInt32Item*>(mrSrcCol.GetAttr(nSrcRow, ATTR_VALUE_FORMAT))->GetValue();
        short nType = pSrcDoc->GetFormatTable()->GetType(nNumIndex);
        return (nType == NUMBERFORMAT_DATE) || (nType == NUMBERFORMAT_TIME) || (nType == NUMBERFORMAT_DATETIME);
    }

    void insertRefCell(SCROW nSrcRow, SCROW nDestRow)
    {
        ScAddress aSrcPos(mnCol, nSrcRow, mnTab);
        ScAddress aDestPos(mnCol, nDestRow, mnTab);
        ScSingleRefData aRef;
        aRef.InitAddress(aSrcPos);
        aRef.SetFlag3D(true);

        ScTokenArray aArr;
        aArr.AddSingleReference(aRef);

        mrDestCol.SetFormulaCell(
            maDestBlockPos, nDestRow, new ScFormulaCell(&mrDestCol.GetDoc(), aDestPos, &aArr));
    }

public:
    CopyCellsFromClipHandler(sc::CopyFromClipContext& rCxt, ScColumn& rSrcCol, ScColumn& rDestCol, SCTAB nDestTab, SCCOL nDestCol, long nRowOffset) :
        mrCxt(rCxt),
        mrSrcCol(rSrcCol),
        mrDestCol(rDestCol),
        mnTab(nDestTab),
        mnCol(nDestCol),
        mnRowOffset(nRowOffset),
        mpDestBlockPos(mrCxt.getBlockPosition(nDestTab, nDestCol))
    {
        if (mpDestBlockPos)
            maDestBlockPos = *mpDestBlockPos;
        else
            mrDestCol.InitBlockPosition(maDestBlockPos);
    }

    ~CopyCellsFromClipHandler()
    {
        if (mpDestBlockPos)
            // Don't forget to save this to the context!
            *mpDestBlockPos = maDestBlockPos;
    }

    void operator() (const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize)
    {
        if (node.type == sc::element_type_empty)
            return;

        SCROW nSrcRow1 = node.position + nOffset;

        sal_uInt16 nFlags = mrCxt.getInsertFlag();
        bool bNumeric = (nFlags & IDF_VALUE) != 0;
        bool bDateTime = (nFlags & IDF_DATETIME) != 0;
        bool bString   = (nFlags & IDF_STRING) != 0;
        bool bBoolean  = (nFlags & IDF_SPECIAL_BOOLEAN) != 0;
        bool bFormula  = (nFlags & IDF_FORMULA) != 0;

        bool bAsLink = mrCxt.isAsLink();

        switch (node.type)
        {
            case sc::element_type_numeric:
            {
                // We need to copy numeric cells individually because of date type check.
                sc::numeric_block::const_iterator it = sc::numeric_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::numeric_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                for (SCROW nSrcRow = nSrcRow1; it != itEnd; ++it, ++nSrcRow)
                {
                    bool bCopy = isDateCell(nSrcRow) ? bDateTime : bNumeric;
                    if (!bCopy)
                        continue;

                    if (bAsLink)
                        insertRefCell(nSrcRow, nSrcRow + mnRowOffset);
                    else
                        mrDestCol.SetValue(maDestBlockPos, nSrcRow + mnRowOffset, *it);
                }
            }
            break;
            case sc::element_type_string:
            {
                if (!bString)
                    return;

                sc::string_block::const_iterator it = sc::string_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::string_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                for (SCROW nSrcRow = nSrcRow1; it != itEnd; ++it, ++nSrcRow)
                {
                    if (bAsLink)
                        insertRefCell(nSrcRow, nSrcRow + mnRowOffset);
                    else
                        mrDestCol.SetRawString(maDestBlockPos, nSrcRow + mnRowOffset, *it);
                }
            }
            break;
            case sc::element_type_edittext:
            {
                if (!bString)
                    return;

                sc::edittext_block::const_iterator it = sc::edittext_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::edittext_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                for (SCROW nSrcRow = nSrcRow1; it != itEnd; ++it, ++nSrcRow)
                {

                    if (bAsLink)
                        insertRefCell(nSrcRow, nSrcRow + mnRowOffset);
                    else
                        mrDestCol.SetEditText(maDestBlockPos, nSrcRow + mnRowOffset, **it);
                }
            }
            break;
            case sc::element_type_formula:
            {
                sc::formula_block::const_iterator it = sc::formula_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::formula_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                for (SCROW nSrcRow = nSrcRow1; it != itEnd; ++it, ++nSrcRow)
                {
                    ScFormulaCell& rSrcCell = const_cast<ScFormulaCell&>(**it);
                    bool bForceFormula = false;
                    if (bBoolean)
                    {
                        // See if the formula consists of =TRUE() or =FALSE().
                        ScTokenArray* pCode = rSrcCell.GetCode();
                        if (pCode && pCode->GetLen() == 1)
                        {
                            const formula::FormulaToken* p = pCode->First();
                            if (p->GetOpCode() == ocTrue || p->GetOpCode() == ocFalse)
                                // This is a boolean formula.
                                bForceFormula = true;
                        }
                    }

                    ScAddress aDestPos(mnCol, nSrcRow + mnRowOffset, mnTab);
                    if (bFormula || bForceFormula)
                    {
                        if (bAsLink)
                            insertRefCell(nSrcRow, nSrcRow + mnRowOffset);
                        else
                        {
                            mrDestCol.SetFormulaCell(
                                maDestBlockPos, nSrcRow + mnRowOffset,
                                new ScFormulaCell(rSrcCell, mrDestCol.GetDoc(), aDestPos));
                        }
                    }
                    else if (bNumeric || bDateTime || bString)
                    {
                        // Always just copy the original row to the Undo Documen;
                        // do not create Value/string cells from formulas

                        sal_uInt16 nErr = rSrcCell.GetErrCode();
                        if (nErr)
                        {
                            // error codes are cloned with values
                            if (bNumeric)
                            {
                                if (bAsLink)
                                    insertRefCell(nSrcRow, nSrcRow + mnRowOffset);
                                else
                                {
                                    ScFormulaCell* pErrCell = new ScFormulaCell(&mrDestCol.GetDoc(), aDestPos);
                                    pErrCell->SetErrCode(nErr);
                                    mrDestCol.SetFormulaCell(
                                        maDestBlockPos, nSrcRow + mnRowOffset, pErrCell);
                                }
                            }
                        }
                        else if (rSrcCell.IsValue())
                        {
                            bool bCopy = isDateCell(nSrcRow) ? bDateTime : bNumeric;
                            if (!bCopy)
                                continue;

                            if (bAsLink)
                                insertRefCell(nSrcRow, nSrcRow + mnRowOffset);
                            else
                                mrDestCol.SetValue(maDestBlockPos, nSrcRow + mnRowOffset, rSrcCell.GetValue());
                        }
                        else if (bString)
                        {
                            OUString aStr = rSrcCell.GetString();
                            if (aStr.isEmpty())
                                // do not clone empty string
                                continue;

                            if (bAsLink)
                                insertRefCell(nSrcRow, nSrcRow + mnRowOffset);
                            else if (rSrcCell.IsMultilineResult())
                            {
                                // Clone as an edit text object.
                                ScFieldEditEngine& rEngine = mrDestCol.GetDoc().GetEditEngine();
                                rEngine.SetText(aStr);
                                mrDestCol.SetEditText(maDestBlockPos, nSrcRow + mnRowOffset, rEngine.CreateTextObject());
                            }
                            else
                                mrDestCol.SetRawString(maDestBlockPos, nSrcRow + mnRowOffset, aStr);
                        }
                    }
                }
            }
            break;
            default:
                ;
        }
    }
};

}

//  rColumn = source
//  nRow1, nRow2 = target position

void ScColumn::CopyFromClip(
    sc::CopyFromClipContext& rCxt, SCROW nRow1, SCROW nRow2, long nDy, ScColumn& rColumn )
{
    if ((rCxt.getInsertFlag() & IDF_ATTRIB) != 0)
    {
        if (rCxt.isSkipAttrForEmptyCells())
        {
            //  copy only attributes for non-empty cells between nRow1-nDy and nRow2-nDy.
            sc::SingleColumnSpanSet aSpanSet;
            aSpanSet.scan(rColumn, nRow1-nDy, nRow2-nDy);
            sc::SingleColumnSpanSet::SpansType aSpans;
            aSpanSet.getSpans(aSpans);
            std::for_each(
                aSpans.begin(), aSpans.end(), CopyAttrArrayByRange(*rColumn.pAttrArray, *pAttrArray, nDy));
        }
        else
            rColumn.pAttrArray->CopyAreaSafe( nRow1, nRow2, nDy, *pAttrArray );
    }
    if ((rCxt.getInsertFlag() & IDF_CONTENTS) == 0)
        return;

    if (rCxt.isAsLink() && rCxt.getInsertFlag() == IDF_ALL)
    {
        // We also reference empty cells for "ALL"
        // IDF_ALL must always contain more flags when compared to "Insert contents" as
        // contents can be selected one by one!

        ScAddress aDestPos( nCol, 0, nTab ); // Adapt Row

        //  Create reference (Source Position)
        ScSingleRefData aRef;
        aRef.InitFlags(); // -> All absolute
        aRef.SetAbsCol(rColumn.nCol);
        aRef.SetAbsTab(rColumn.nTab);
        aRef.SetFlag3D(true);

        for (SCROW nDestRow = nRow1; nDestRow <= nRow2; nDestRow++)
        {
            aRef.SetAbsRow(nDestRow - nDy); // Source row
            aDestPos.SetRow( nDestRow );

            ScTokenArray aArr;
            aArr.AddSingleReference( aRef );
            SetFormulaCell(nDestRow, new ScFormulaCell(pDocument, aDestPos, &aArr));
        }

        return;
    }

    // nRow1 to nRow2 is for destination (this) column. Subtract nDy to get the source range.

    // Copy all cells in the source column (rColumn) from nRow1-nDy to nRow2-nDy to this column.
    CopyCellsFromClipHandler aFunc(rCxt, rColumn, *this, nTab, nCol, nDy);
    sc::ParseBlock(rColumn.maCells.begin(), rColumn.maCells, aFunc, nRow1-nDy, nRow2-nDy);
}

void ScColumn::MixMarked(
    sc::MixDocContext& rCxt, const ScMarkData& rMark, sal_uInt16 nFunction,
    bool bSkipEmpty, const ScColumn& rSrcCol )
{
    SCROW nRow1, nRow2;

    if (rMark.IsMultiMarked())
    {
        ScMarkArrayIter aIter( rMark.GetArray()+nCol );
        while (aIter.Next( nRow1, nRow2 ))
            MixData(rCxt, nRow1, nRow2, nFunction, bSkipEmpty, rSrcCol);
    }
}

namespace {

// Result in rVal1
bool lcl_DoFunction( double& rVal1, double nVal2, sal_uInt16 nFunction )
{
    bool bOk = false;
    switch (nFunction)
    {
        case PASTE_ADD:
            bOk = SubTotal::SafePlus( rVal1, nVal2 );
            break;
        case PASTE_SUB:
            nVal2 = -nVal2;     // FIXME: Can we do this alwyas without error?
            bOk = SubTotal::SafePlus( rVal1, nVal2 );
            break;
        case PASTE_MUL:
            bOk = SubTotal::SafeMult( rVal1, nVal2 );
            break;
        case PASTE_DIV:
            bOk = SubTotal::SafeDiv( rVal1, nVal2 );
            break;
    }
    return bOk;
}

void lcl_AddCode( ScTokenArray& rArr, const ScFormulaCell* pCell )
{
    rArr.AddOpCode(ocOpen);

    ScTokenArray* pCode = const_cast<ScFormulaCell*>(pCell)->GetCode();
    if (pCode)
    {
        const formula::FormulaToken* pToken = pCode->First();
        while (pToken)
        {
            rArr.AddToken( *pToken );
            pToken = pCode->Next();
        }
    }

    rArr.AddOpCode(ocClose);
}

class MixDataHandler
{
    ScColumn& mrDestColumn;
    sc::ColumnBlockPosition& mrBlockPos;

    sc::CellStoreType maNewCells;
    sc::CellStoreType::iterator miNewCellsPos;

    size_t mnRowOffset;
    sal_uInt16 mnFunction;

    bool mbSkipEmpty;

public:
    MixDataHandler(
        sc::ColumnBlockPosition& rBlockPos,
        ScColumn& rDestColumn,
        SCROW nRow1, SCROW nRow2,
        sal_uInt16 nFunction, bool bSkipEmpty) :
        mrDestColumn(rDestColumn),
        mrBlockPos(rBlockPos),
        maNewCells(nRow2 - nRow1 + 1),
        miNewCellsPos(maNewCells.begin()),
        mnRowOffset(nRow1),
        mnFunction(nFunction),
        mbSkipEmpty(bSkipEmpty)
    {
    }

    void operator() (size_t nRow, double f)
    {
        sc::CellStoreType::position_type aPos = mrDestColumn.GetCellStore().position(mrBlockPos.miCellPos, nRow);
        mrBlockPos.miCellPos = aPos.first;
        switch (aPos.first->type)
        {
            case sc::element_type_numeric:
            {
                // Both src and dest are of numeric type.
                bool bOk = lcl_DoFunction(f, sc::numeric_block::at(*aPos.first->data, aPos.second), mnFunction);

                if (bOk)
                    miNewCellsPos = maNewCells.set(miNewCellsPos, nRow-mnRowOffset, f);
                else
                {
                    ScFormulaCell* pFC =
                        new ScFormulaCell(
                            &mrDestColumn.GetDoc(), ScAddress(mrDestColumn.GetCol(), nRow, mrDestColumn.GetTab()));

                    pFC->SetErrCode(errNoValue);
                    miNewCellsPos = maNewCells.set(miNewCellsPos, nRow-mnRowOffset, pFC);
                }
            }
            break;
            case sc::element_type_formula:
            {
                // Combination of value and at least one formula -> Create formula
                ScTokenArray aArr;

                // First row
                aArr.AddDouble(f);

                // Operator
                OpCode eOp = ocAdd;
                switch (mnFunction)
                {
                    case PASTE_ADD: eOp = ocAdd; break;
                    case PASTE_SUB: eOp = ocSub; break;
                    case PASTE_MUL: eOp = ocMul; break;
                    case PASTE_DIV: eOp = ocDiv; break;
                }
                aArr.AddOpCode(eOp); // Function

                // Second row
                ScFormulaCell* pDest = sc::formula_block::at(*aPos.first->data, aPos.second);
                lcl_AddCode(aArr, pDest);

                miNewCellsPos = maNewCells.set(
                    miNewCellsPos, nRow-mnRowOffset,
                    new ScFormulaCell(
                        &mrDestColumn.GetDoc(), ScAddress(mrDestColumn.GetCol(), nRow, mrDestColumn.GetTab()), &aArr));
            }
            break;
            case sc::element_type_string:
            case sc::element_type_edittext:
            case sc::element_type_empty:
            {
                // Destination cell is not a number. Just take the source cell.
                miNewCellsPos = maNewCells.set(miNewCellsPos, nRow-mnRowOffset, f);
            }
            break;
            default:
                ;
        }
    }

    void operator() (size_t nRow, const OUString& rStr)
    {
        miNewCellsPos = maNewCells.set(miNewCellsPos, nRow-mnRowOffset, rStr);
    }

    void operator() (size_t nRow, const EditTextObject* p)
    {
        miNewCellsPos = maNewCells.set(miNewCellsPos, nRow-mnRowOffset, p->Clone());
    }

    void operator() (size_t nRow, const ScFormulaCell* p)
    {
        sc::CellStoreType::position_type aPos = mrDestColumn.GetCellStore().position(mrBlockPos.miCellPos, nRow);
        mrBlockPos.miCellPos = aPos.first;
        switch (aPos.first->type)
        {
            case sc::element_type_numeric:
            {
                // Source is formula, and dest is value.
                ScTokenArray aArr;

                // First row
                lcl_AddCode(aArr, p);

                // Operator
                OpCode eOp = ocAdd;
                switch (mnFunction)
                {
                    case PASTE_ADD: eOp = ocAdd; break;
                    case PASTE_SUB: eOp = ocSub; break;
                    case PASTE_MUL: eOp = ocMul; break;
                    case PASTE_DIV: eOp = ocDiv; break;
                }
                aArr.AddOpCode(eOp); // Function

                // Second row
                aArr.AddDouble(sc::numeric_block::at(*aPos.first->data, aPos.second));

                miNewCellsPos = maNewCells.set(
                    miNewCellsPos, nRow-mnRowOffset,
                    new ScFormulaCell(
                        &mrDestColumn.GetDoc(), ScAddress(mrDestColumn.GetCol(), nRow, mrDestColumn.GetTab()), &aArr));
            }
            break;
            case sc::element_type_formula:
            {
                // Both are formulas.
                ScTokenArray aArr;

                // First row
                lcl_AddCode(aArr, p);

                // Operator
                OpCode eOp = ocAdd;
                switch (mnFunction)
                {
                    case PASTE_ADD: eOp = ocAdd; break;
                    case PASTE_SUB: eOp = ocSub; break;
                    case PASTE_MUL: eOp = ocMul; break;
                    case PASTE_DIV: eOp = ocDiv; break;
                }
                aArr.AddOpCode(eOp); // Function

                // Second row
                ScFormulaCell* pDest = sc::formula_block::at(*aPos.first->data, aPos.second);
                lcl_AddCode(aArr, pDest);

                miNewCellsPos = maNewCells.set(
                    miNewCellsPos, nRow-mnRowOffset,
                    new ScFormulaCell(
                        &mrDestColumn.GetDoc(), ScAddress(mrDestColumn.GetCol(), nRow, mrDestColumn.GetTab()), &aArr));
            }
            break;
            case sc::element_type_string:
            case sc::element_type_edittext:
            case sc::element_type_empty:
            {
                // Destination cell is not a number. Just take the source cell.
                ScAddress aDestPos(mrDestColumn.GetCol(), nRow, mrDestColumn.GetTab());
                miNewCellsPos = maNewCells.set(
                    miNewCellsPos, nRow-mnRowOffset, new ScFormulaCell(*p, mrDestColumn.GetDoc(), aDestPos));
            }
            break;
            default:
                ;
        }
    }

    /**
     * Empty cell series in the source (clip) document.
     */
    void operator() (mdds::mtv::element_t, size_t nTopRow, size_t nDataSize)
    {
        if (mbSkipEmpty)
            return;

        // Source cells are empty. Treat them as if they have a value of 0.0.
        for (size_t i = 0; i < nDataSize; ++i)
        {
            size_t nDestRow = nTopRow + i;
            sc::CellStoreType::position_type aPos = mrDestColumn.GetCellStore().position(mrBlockPos.miCellPos, nDestRow);
            mrBlockPos.miCellPos = aPos.first;
            switch (aPos.first->type)
            {
                case sc::element_type_numeric:
                case sc::element_type_string:
                case sc::element_type_edittext:
                    // Dont' do anything.
                break;
                case sc::element_type_formula:
                {
                    ScTokenArray aArr;

                    // First row
                    ScFormulaCell* pSrc = sc::formula_block::at(*aPos.first->data, aPos.second);
                    lcl_AddCode( aArr, pSrc);

                    // Operator
                    OpCode eOp = ocAdd;
                    switch (mnFunction)
                    {
                        case PASTE_ADD: eOp = ocAdd; break;
                        case PASTE_SUB: eOp = ocSub; break;
                        case PASTE_MUL: eOp = ocMul; break;
                        case PASTE_DIV: eOp = ocDiv; break;
                    }

                    aArr.AddOpCode(eOp); // Function
                    aArr.AddDouble(0.0);

                    miNewCellsPos = maNewCells.set(
                        miNewCellsPos, nDestRow-mnRowOffset,
                        new ScFormulaCell(
                            &mrDestColumn.GetDoc(), ScAddress(mrDestColumn.GetCol(), nDestRow, mrDestColumn.GetTab()), &aArr));
                }
                break;
                default:
                    ;
            }
        }
    }

    /**
     * Set the new cells to the destination (this) column.
     */
    void commit()
    {
        sc::CellStoreType& rDestCells = mrDestColumn.GetCellStore();

        // Stop all formula cells in the destination range first.
        sc::CellStoreType::position_type aPos = rDestCells.position(mrBlockPos.miCellPos, mnRowOffset);
        mrDestColumn.DetachFormulaCells(aPos, maNewCells.size());

        // Move the new cells to the destination range.
        sc::CellStoreType::iterator& itDestPos = mrBlockPos.miCellPos;
        sc::CellTextAttrStoreType::iterator& itDestAttrPos = mrBlockPos.miCellTextAttrPos;

        sc::CellStoreType::iterator it = maNewCells.begin(), itEnd = maNewCells.end();
        for (; it != itEnd; ++it)
        {
            bool bHasContent = true;
            size_t nDestRow = mnRowOffset + it->position;

            switch (it->type)
            {
                case sc::element_type_numeric:
                {
                    sc::numeric_block::iterator itData = sc::numeric_block::begin(*it->data);
                    sc::numeric_block::iterator itDataEnd = sc::numeric_block::end(*it->data);
                    itDestPos = mrDestColumn.GetCellStore().set(itDestPos, nDestRow, itData, itDataEnd);
                }
                break;
                case sc::element_type_string:
                {
                    sc::string_block::iterator itData = sc::string_block::begin(*it->data);
                    sc::string_block::iterator itDataEnd = sc::string_block::end(*it->data);
                    itDestPos = rDestCells.set(itDestPos, nDestRow, itData, itDataEnd);
                }
                break;
                case sc::element_type_edittext:
                {
                    sc::edittext_block::iterator itData = sc::edittext_block::begin(*it->data);
                    sc::edittext_block::iterator itDataEnd = sc::edittext_block::end(*it->data);
                    itDestPos = rDestCells.set(itDestPos, nDestRow, itData, itDataEnd);
                }
                break;
                case sc::element_type_formula:
                {
                    sc::formula_block::iterator itData = sc::formula_block::begin(*it->data);
                    sc::formula_block::iterator itDataEnd = sc::formula_block::end(*it->data);

                    // Group new formula cells before inserting them.
                    sc::SharedFormulaUtil::groupFormulaCells(itData, itDataEnd);

                    // Insert the formula cells to the column.
                    itDestPos = rDestCells.set(itDestPos, nDestRow, itData, itDataEnd);

                    // Merge with the previous formula group (if any).
                    aPos = rDestCells.position(itDestPos, nDestRow);
                    sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);

                    // Merge with the next formula group (if any).
                    size_t nNextRow = nDestRow + it->size;
                    if (ValidRow(nNextRow))
                    {
                        aPos = rDestCells.position(aPos.first, nNextRow);
                        sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
                    }
                }
                break;
                case sc::element_type_empty:
                {
                    itDestPos = rDestCells.set_empty(itDestPos, nDestRow, nDestRow+it->size-1);
                    bHasContent = false;
                }
                break;
                default:
                    ;
            }

            sc::CellTextAttrStoreType& rDestAttrs = mrDestColumn.GetCellAttrStore();
            if (bHasContent)
            {
                std::vector<sc::CellTextAttr> aAttrs(it->size, sc::CellTextAttr());
                itDestAttrPos = rDestAttrs.set(itDestAttrPos, nDestRow, aAttrs.begin(), aAttrs.end());
            }
            else
                itDestAttrPos = rDestAttrs.set_empty(itDestAttrPos, nDestRow, nDestRow+it->size-1);
        }

        maNewCells.release();
    }
};

}

void ScColumn::MixData(
    sc::MixDocContext& rCxt, SCROW nRow1, SCROW nRow2, sal_uInt16 nFunction,
    bool bSkipEmpty, const ScColumn& rSrcCol )
{
    // destination (this column) block position.

    sc::ColumnBlockPosition* p = rCxt.getBlockPosition(nTab, nCol);
    if (!p)
        return;

    MixDataHandler aFunc(*p, *this, nRow1, nRow2, nFunction, bSkipEmpty);
    sc::ParseAll(rSrcCol.maCells.begin(), rSrcCol.maCells, nRow1, nRow2, aFunc, aFunc);

    aFunc.commit();
    CellStorageModified();
}


ScAttrIterator* ScColumn::CreateAttrIterator( SCROW nStartRow, SCROW nEndRow ) const
{
    return new ScAttrIterator( pAttrArray, nStartRow, nEndRow );
}

namespace {

class StartAllListenersHandler
{
    ScDocument* mpDoc;
public:
    StartAllListenersHandler(ScDocument* pDoc) : mpDoc(pDoc) {}

    void operator() (size_t, ScFormulaCell* p)
    {
        p->StartListeningTo(mpDoc);
    }
};

class StartNeededListenerHandler
{
    ScDocument* mpDoc;
public:
    StartNeededListenerHandler(ScDocument* pDoc) : mpDoc(pDoc) {}

    void operator() (size_t, ScFormulaCell* p)
    {
        if (p->NeedsListening())
            p->StartListeningTo(mpDoc);
    }
};

}

void ScColumn::StartAllListeners()
{
    StartAllListenersHandler aFunc(pDocument);
    sc::ProcessFormula(maCells, aFunc);
}


void ScColumn::StartNeededListeners()
{
    StartNeededListenerHandler aFunc(pDocument);
    sc::ProcessFormula(maCells, aFunc);
}

namespace {

class StartListeningInAreaHandler
{
    sc::StartListeningContext& mrCxt;
public:
    StartListeningInAreaHandler(sc::StartListeningContext& rCxt) : mrCxt(rCxt) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* p)
    {
        p->StartListeningTo(mrCxt);
    }
};

}

void ScColumn::StartListeningInArea( sc::StartListeningContext& rCxt, SCROW nRow1, SCROW nRow2 )
{
    StartListeningInAreaHandler aFunc(rCxt);
    sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aFunc);
}

bool ScColumn::ParseString(
    ScCellValue& rCell, SCROW nRow, SCTAB nTabP, const String& rString,
    formula::FormulaGrammar::AddressConvention eConv,
    ScSetStringParam* pParam )
{
    if (!rString.Len())
        return false;

    bool bNumFmtSet = false;

    ScSetStringParam aParam;

    if (pParam)
        aParam = *pParam;

    sal_uInt32 nIndex = 0;
    sal_uInt32 nOldIndex = 0;
    sal_Unicode cFirstChar;
    if (!aParam.mpNumFormatter)
        aParam.mpNumFormatter = pDocument->GetFormatTable();

    nIndex = nOldIndex = GetNumberFormat( nRow );
    if ( rString.Len() > 1
            && aParam.mpNumFormatter->GetType(nIndex) != NUMBERFORMAT_TEXT )
        cFirstChar = rString.GetChar(0);
    else
        cFirstChar = 0; // Text

    if ( cFirstChar == '=' )
    {
        if ( rString.Len() == 1 ) // = Text
            rCell.set(rString);
        else // = Formula
            rCell.set(
                new ScFormulaCell(
                    pDocument, ScAddress(nCol, nRow, nTabP), rString,
                    formula::FormulaGrammar::mergeToGrammar(formula::FormulaGrammar::GRAM_DEFAULT, eConv),
                    MM_NONE));
    }
    else if ( cFirstChar == '\'') // 'Text
    {
        bool bNumeric = false;
        if (aParam.mbHandleApostrophe)
        {
            // Cell format is not 'Text', and the first char
            // is an apostrophe. Check if the input is considered a number.
            String aTest = rString.Copy(1);
            double fTest;
            bNumeric = aParam.mpNumFormatter->IsNumberFormat(aTest, nIndex, fTest);
            if (bNumeric)
                // This is a number. Strip out the first char.
                rCell.set(aTest);
        }
        if (!bNumeric)
            // This is normal text. Take it as-is.
            rCell.set(rString);
    }
    else
    {
        double nVal;

        do
        {
            if (aParam.mbDetectNumberFormat)
            {
                if (!aParam.mpNumFormatter->IsNumberFormat(rString, nIndex, nVal))
                    break;

                if ( aParam.mpNumFormatter )
                {
                    // convert back to the original language if a built-in format was detected
                    const SvNumberformat* pOldFormat = aParam.mpNumFormatter->GetEntry( nOldIndex );
                    if ( pOldFormat )
                        nIndex = aParam.mpNumFormatter->GetFormatForLanguageIfBuiltIn( nIndex, pOldFormat->GetLanguage() );
                }

                rCell.set(nVal);
                if ( nIndex != nOldIndex)
                {
                    // #i22345# New behavior: Apply the detected number format only if
                    // the old one was the default number, date, time or boolean format.
                    // Exception: If the new format is boolean, always apply it.

                    bool bOverwrite = false;
                    const SvNumberformat* pOldFormat = aParam.mpNumFormatter->GetEntry( nOldIndex );
                    if ( pOldFormat )
                    {
                        short nOldType = pOldFormat->GetType() & ~NUMBERFORMAT_DEFINED;
                        if ( nOldType == NUMBERFORMAT_NUMBER || nOldType == NUMBERFORMAT_DATE ||
                             nOldType == NUMBERFORMAT_TIME || nOldType == NUMBERFORMAT_LOGICAL )
                        {
                            if ( nOldIndex == aParam.mpNumFormatter->GetStandardFormat(
                                                nOldType, pOldFormat->GetLanguage() ) )
                            {
                                bOverwrite = true; // default of these types can be overwritten
                            }
                        }
                    }
                    if ( !bOverwrite && aParam.mpNumFormatter->GetType( nIndex ) == NUMBERFORMAT_LOGICAL )
                    {
                        bOverwrite = true; // overwrite anything if boolean was detected
                    }

                    if ( bOverwrite )
                    {
                        ApplyAttr( nRow, SfxUInt32Item( ATTR_VALUE_FORMAT,
                            (sal_uInt32) nIndex) );
                        bNumFmtSet = true;
                    }
                }
            }
            else if (aParam.meSetTextNumFormat != ScSetStringParam::Always)
            {
                // Only check if the string is a regular number.
                const LocaleDataWrapper* pLocale = aParam.mpNumFormatter->GetLocaleData();
                if (!pLocale)
                    break;

                LocaleDataItem aLocaleItem = pLocale->getLocaleItem();
                const OUString& rDecSep = aLocaleItem.decimalSeparator;
                const OUString& rGroupSep = aLocaleItem.thousandSeparator;
                if (rDecSep.getLength() != 1 || rGroupSep.getLength() != 1)
                    break;

                sal_Unicode dsep = rDecSep.getStr()[0];
                sal_Unicode gsep = rGroupSep.getStr()[0];

                if (!ScStringUtil::parseSimpleNumber(rString, dsep, gsep, nVal))
                    break;

                rCell.set(nVal);
            }
        }
        while (false);

        if (rCell.meType == CELLTYPE_NONE)
        {
            if (aParam.meSetTextNumFormat != ScSetStringParam::Never && aParam.mpNumFormatter->IsNumberFormat(rString, nIndex, nVal))
            {
                // Set the cell format type to Text.
                sal_uInt32 nFormat = aParam.mpNumFormatter->GetStandardFormat(NUMBERFORMAT_TEXT);
                ScPatternAttr aNewAttrs(pDocument->GetPool());
                SfxItemSet& rSet = aNewAttrs.GetItemSet();
                rSet.Put( SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat) );
                ApplyPattern(nRow, aNewAttrs);
            }

            rCell.set(rString);
        }
    }

    return bNumFmtSet;
}

/**
 * Returns true if the cell format was set as well
 */
bool ScColumn::SetString( SCROW nRow, SCTAB nTabP, const String& rString,
                          formula::FormulaGrammar::AddressConvention eConv,
                          ScSetStringParam* pParam )
{
    if (!ValidRow(nRow))
        return false;

    ScCellValue aNewCell;
    bool bNumFmtSet = ParseString(aNewCell, nRow, nTabP, rString, eConv, pParam);
    aNewCell.release(*this, nRow);

    // Do not set Formats and Formulas here anymore!
    // These are queried during output

    return bNumFmtSet;
}

void ScColumn::SetEditText( SCROW nRow, EditTextObject* pEditText )
{
    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    maCells.set(it, nRow, pEditText);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());
    CellStorageModified();

    BroadcastNewCell(nRow);
}

void ScColumn::SetEditText( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, EditTextObject* pEditText )
{
    rBlockPos.miCellPos = GetPositionToInsert(rBlockPos.miCellPos, nRow);
    rBlockPos.miCellPos = maCells.set(rBlockPos.miCellPos, nRow, pEditText);
    rBlockPos.miCellTextAttrPos = maCellTextAttrs.set(
        rBlockPos.miCellTextAttrPos, nRow, sc::CellTextAttr());
    CellStorageModified();

    BroadcastNewCell(nRow);
}

void ScColumn::SetEditText( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, const EditTextObject& rEditText )
{
    if (pDocument->GetEditPool() == rEditText.GetPool())
    {
        SetEditText(rBlockPos, nRow, rEditText.Clone());
        return;
    }

    //! another "spool"
    // Sadly there is no other way to change the Pool than to
    // "spool" the Object through a corresponding Engine
    EditEngine& rEngine = pDocument->GetEditEngine();
    if (!rEditText.HasOnlineSpellErrors())
    {
        rEngine.SetText(rEditText);
        SetEditText(rBlockPos, nRow, rEngine.CreateTextObject());
        return;
    }

    sal_uLong nControl = rEngine.GetControlWord();
    const sal_uLong nSpellControl = EE_CNTRL_ONLINESPELLING | EE_CNTRL_ALLOWBIGOBJS;
    bool bNewControl = (nControl & nSpellControl) != nSpellControl;
    if (bNewControl)
        rEngine.SetControlWord(nControl | nSpellControl);
    rEngine.SetText(rEditText);
    EditTextObject* pData = rEngine.CreateTextObject();
    if (bNewControl)
        rEngine.SetControlWord(nControl);

    SetEditText(rBlockPos, nRow, pData);
}

void ScColumn::SetEditText( SCROW nRow, const EditTextObject& rEditText, const SfxItemPool* pEditPool )
{
    if (pEditPool && pDocument->GetEditPool() == pEditPool)
    {
        SetEditText(nRow, rEditText.Clone());
        return;
    }

    //! another "spool"
    // Sadly there is no other way to change the Pool than to
    // "spool" the Object through a corresponding Engine
    EditEngine& rEngine = pDocument->GetEditEngine();
    if (!rEditText.HasOnlineSpellErrors())
    {
        rEngine.SetText(rEditText);
        SetEditText(nRow, rEngine.CreateTextObject());
        return;
    }

    sal_uLong nControl = rEngine.GetControlWord();
    const sal_uLong nSpellControl = EE_CNTRL_ONLINESPELLING | EE_CNTRL_ALLOWBIGOBJS;
    bool bNewControl = (nControl & nSpellControl) != nSpellControl;
    if (bNewControl)
        rEngine.SetControlWord(nControl | nSpellControl);
    rEngine.SetText(rEditText);
    EditTextObject* pData = rEngine.CreateTextObject();
    if (bNewControl)
        rEngine.SetControlWord(nControl);

    SetEditText(nRow, pData);
}

void ScColumn::SetFormula( SCROW nRow, const ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGram )
{
    ScAddress aPos(nCol, nRow, nTab);

    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    ScFormulaCell* pCell = new ScFormulaCell(pDocument, aPos, &rArray, eGram);
    sal_uInt32 nCellFormat = GetNumberFormat(nRow);
    if( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        pCell->SetNeedNumberFormat(true);
    it = maCells.set(it, nRow, pCell);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());
    CellStorageModified();

    ActivateNewFormulaCell(it, nRow, *pCell);
}

void ScColumn::SetFormula( SCROW nRow, const OUString& rFormula, formula::FormulaGrammar::Grammar eGram )
{
    ScAddress aPos(nCol, nRow, nTab);

    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    ScFormulaCell* pCell = new ScFormulaCell(pDocument, aPos, rFormula, eGram);
    sal_uInt32 nCellFormat = GetNumberFormat(nRow);
    if( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        pCell->SetNeedNumberFormat(true);
    it = maCells.set(it, nRow, pCell);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());
    CellStorageModified();

    ActivateNewFormulaCell(it, nRow, *pCell);
}

ScFormulaCell* ScColumn::SetFormulaCell( SCROW nRow, ScFormulaCell* pCell )
{
    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    sal_uInt32 nCellFormat = GetNumberFormat(nRow);
    if( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        pCell->SetNeedNumberFormat(true);
    it = maCells.set(it, nRow, pCell);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());
    CellStorageModified();

    ActivateNewFormulaCell(it, nRow, *pCell);
    return pCell;
}

ScFormulaCell* ScColumn::SetFormulaCell( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, ScFormulaCell* pCell )
{
    rBlockPos.miCellPos = GetPositionToInsert(rBlockPos.miCellPos, nRow);
    sal_uInt32 nCellFormat = GetNumberFormat(nRow);
    if( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        pCell->SetNeedNumberFormat(true);
    rBlockPos.miCellPos = maCells.set(rBlockPos.miCellPos, nRow, pCell);
    rBlockPos.miCellTextAttrPos = maCellTextAttrs.set(
        rBlockPos.miCellTextAttrPos, nRow, sc::CellTextAttr());
    CellStorageModified();

    ActivateNewFormulaCell(rBlockPos.miCellPos, nRow, *pCell);
    return pCell;
}

bool ScColumn::SetGroupFormulaCell( SCROW nRow, ScFormulaCell* pCell )
{
    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    sal_uInt32 nCellFormat = GetNumberFormat(nRow);
    if( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        pCell->SetNeedNumberFormat(true);
    it = maCells.set(it, nRow, pCell);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());
    CellStorageModified();

    ActivateNewFormulaCell(it, nRow, *pCell, false);
    return true;
}

namespace {

class FilterEntriesHandler
{
    ScColumn& mrColumn;
    std::vector<ScTypedStrData>& mrStrings;
    bool mbHasDates;

    void processCell(SCROW nRow, ScRefCellValue& rCell)
    {
        SvNumberFormatter* pFormatter = mrColumn.GetDoc().GetFormatTable();
        OUString aStr;
        sal_uLong nFormat = mrColumn.GetNumberFormat(nRow);
        ScCellFormat::GetInputString(rCell, nFormat, aStr, *pFormatter, &mrColumn.GetDoc());

        if (rCell.hasString())
        {
            mrStrings.push_back(ScTypedStrData(aStr));
            return;
        }

        double fVal = 0.0;

        switch (rCell.meType)
        {
            case CELLTYPE_VALUE:
                fVal = rCell.mfValue;
                break;

            case CELLTYPE_FORMULA:
            {
                ScFormulaCell* pFC = rCell.mpFormula;
                sal_uInt16 nErr = pFC->GetErrCode();
                if (nErr)
                {
                    // Error cell is evaluated as string (for now).
                    OUString aErr = ScGlobal::GetErrorString(nErr);
                    if (!aErr.isEmpty())
                    {
                        mrStrings.push_back(ScTypedStrData(aErr));
                        return;
                    }
                }
                else
                    fVal = pFC->GetValue();
            }
            break;
            default:
                ;
        }

        short nType = pFormatter->GetType(nFormat);
        if ((nType & NUMBERFORMAT_DATE) && !(nType & NUMBERFORMAT_TIME))
        {
            // special case for date values.  Disregard the time
            // element if the number format is of date type.
            fVal = rtl::math::approxFloor(fVal);
            mbHasDates = true;
        }

        mrStrings.push_back(ScTypedStrData(aStr, fVal, ScTypedStrData::Value));
    }

public:
    FilterEntriesHandler(ScColumn& rColumn, std::vector<ScTypedStrData>& rStrings) :
        mrColumn(rColumn), mrStrings(rStrings), mbHasDates(false) {}

    void operator() (size_t nRow, double fVal)
    {
        ScRefCellValue aCell(fVal);
        processCell(nRow, aCell);
    }

    void operator() (size_t nRow, const OUString& rStr)
    {
        ScRefCellValue aCell(&rStr);
        processCell(nRow, aCell);
    }

    void operator() (size_t nRow, const EditTextObject* p)
    {
        ScRefCellValue aCell(p);
        processCell(nRow, aCell);
    }

    void operator() (size_t nRow, const ScFormulaCell* p)
    {
        ScRefCellValue aCell(const_cast<ScFormulaCell*>(p));
        processCell(nRow, aCell);
    }

    bool hasDates() const { return mbHasDates; }
};

}

void ScColumn::GetFilterEntries(SCROW nStartRow, SCROW nEndRow, std::vector<ScTypedStrData>& rStrings, bool& rHasDates)
{
    FilterEntriesHandler aFunc(*this, rStrings);
    sc::ParseAllNonEmpty(maCells.begin(), maCells, nStartRow, nEndRow, aFunc);
    rHasDates = aFunc.hasDates();
}

namespace {

/**
 * Iterate over only string and edit-text cells.
 */
class StrCellIterator
{
    typedef std::pair<sc::CellStoreType::const_iterator,size_t> PosType;
    PosType maPos;
    sc::CellStoreType::const_iterator miBeg;
    sc::CellStoreType::const_iterator miEnd;
    const ScDocument* mpDoc;
public:
    StrCellIterator(const sc::CellStoreType& rCells, SCROW nStart, const ScDocument* pDoc) :
        maPos(rCells.position(nStart)), miBeg(rCells.begin()), miEnd(rCells.end()), mpDoc(pDoc) {}

    bool valid() const { return (maPos.first != miEnd); }

    bool has() const
    {
        return (maPos.first->type == sc::element_type_string || maPos.first->type == sc::element_type_edittext);
    }

    bool prev()
    {
        if (!has())
        {
            // Not in a string block. Move back until we hit a string block.
            while (!has())
            {
                if (maPos.first == miBeg)
                    return false;

                --maPos.first; // move to the preceding block.
                maPos.second = maPos.first->size - 1; // last cell in the block.
            }
            return true;
        }

        // We are in a string block.
        if (maPos.second > 0)
        {
            // Move back one cell in the same block.
            --maPos.second;
        }
        else
        {
            // Move back to the preceding string block.
            while (true)
            {
                if (maPos.first == miBeg)
                    return false;

                // Move to the last cell of the previous block.
                --maPos.first;
                maPos.second = maPos.first->size - 1;
                if (has())
                    break;
            }
        }
        return true;
    }

    bool next()
    {
        if (!has())
        {
            // Not in a string block. Move forward until we hit a string block.
            while (!has())
            {
                ++maPos.first;
                if (maPos.first == miEnd)
                    return false;

                maPos.second = 0; // First cell in this block.
            }
            return true;
        }

        // We are in a string block.
        ++maPos.second;
        if (maPos.second >= maPos.first->size)
        {
            // Move to the next string block.
            while (true)
            {
                ++maPos.first;
                if (maPos.first == miEnd)
                    return false;

                maPos.second = 0;
                if (has())
                    break;
            }
        }
        return true;
    }

    OUString get() const
    {
        switch (maPos.first->type)
        {
            case sc::element_type_string:
                return sc::string_block::at(*maPos.first->data, maPos.second);
            case sc::element_type_edittext:
            {
                const EditTextObject* p = sc::edittext_block::at(*maPos.first->data, maPos.second);
                return ScEditUtil::GetString(*p, mpDoc);
            }
            default:
                ;
        }
        return OUString();
    }
};

}

//
// GetDataEntries - Strings from continuous Section around nRow
//

// DATENT_MAX      - max. number of entries in list for auto entry
// DATENT_SEARCH   - max. number of cells that get transparent - new: only count Strings
#define DATENT_MAX      200
#define DATENT_SEARCH   2000

bool ScColumn::GetDataEntries(
    SCROW nStartRow, std::set<ScTypedStrData>& rStrings, bool bLimit ) const
{
    // Start at the specified row position, and collect all string values
    // going upward and downward directions in parallel. The start position
    // cell must be skipped.

    StrCellIterator aItrUp(maCells, nStartRow, pDocument);
    StrCellIterator aItrDown(maCells, nStartRow+1, pDocument);

    bool bMoveUp = aItrUp.valid();
    if (!bMoveUp)
        // Current cell is invalid.
        return false;

    // Skip the start position cell.
    bMoveUp = aItrUp.prev(); // Find the previous string cell position.

    bool bMoveDown = aItrDown.valid();
    if (bMoveDown && !aItrDown.has())
        bMoveDown = aItrDown.next(); // Find the next string cell position.

    bool bFound = false;
    size_t nCellsSearched = 0;
    while (bMoveUp || bMoveDown)
    {
        if (bMoveUp)
        {
            // Get the current string and move up.
            OUString aStr = aItrUp.get();
            if (!aStr.isEmpty())
            {
                bool bInserted = rStrings.insert(ScTypedStrData(aStr)).second;
                if (bInserted && bLimit && rStrings.size() >= DATENT_MAX)
                    return true; // Maximum reached
                bFound = true;
            }

            if (bLimit && ++nCellsSearched >= DATENT_SEARCH)
                return bFound; // max search cell count reached.

            bMoveUp = aItrUp.prev();
        }

        if (bMoveDown)
        {
            // Get the current string and move down.
            OUString aStr = aItrDown.get();
            if (!aStr.isEmpty())
            {
                bool bInserted = rStrings.insert(ScTypedStrData(aStr)).second;
                if (bInserted && bLimit && rStrings.size() >= DATENT_MAX)
                    return true; // Maximum reached
                bFound = true;
            }

            if (bLimit && ++nCellsSearched >= DATENT_SEARCH)
                return bFound; // max search cell count reached.

            bMoveDown = aItrDown.next();
        }
    }

    return bFound;
}

namespace {

class FormulaToValueHandler
{
    struct Entry
    {
        SCROW mnRow;
        ScCellValue maValue;

        Entry(SCROW nRow, double f) : mnRow(nRow), maValue(f) {}
        Entry(SCROW nRow, const OUString& rStr) : mnRow(nRow), maValue(rStr) {}
    };

    typedef std::vector<Entry> EntriesType;
    EntriesType maEntries;

public:

    void operator() (size_t nRow, const ScFormulaCell* p)
    {
        ScFormulaCell* p2 = const_cast<ScFormulaCell*>(p);
        if (p2->IsValue())
            maEntries.push_back(Entry(nRow, p2->GetValue()));
        else
            maEntries.push_back(Entry(nRow, p2->GetString()));
    }

    void commitCells(ScColumn& rColumn)
    {
        sc::ColumnBlockPosition aBlockPos;
        rColumn.InitBlockPosition(aBlockPos);

        EntriesType::iterator it = maEntries.begin(), itEnd = maEntries.end();
        for (; it != itEnd; ++it)
        {
            Entry& r = *it;
            switch (r.maValue.meType)
            {
                case CELLTYPE_VALUE:
                    rColumn.SetValue(aBlockPos, r.mnRow, r.maValue.mfValue, false);
                break;
                case CELLTYPE_STRING:
                    rColumn.SetRawString(aBlockPos, r.mnRow, *r.maValue.mpString, false);
                default:
                    ;
            }
        }
    }
};

}

void ScColumn::RemoveProtected( SCROW nStartRow, SCROW nEndRow )
{
    FormulaToValueHandler aFunc;
    sc::CellStoreType::const_iterator itPos = maCells.begin();

    ScAttrIterator aAttrIter( pAttrArray, nStartRow, nEndRow );
    SCROW nTop = -1;
    SCROW nBottom = -1;
    const ScPatternAttr* pPattern = aAttrIter.Next( nTop, nBottom );
    while (pPattern)
    {
        const ScProtectionAttr* pAttr = (const ScProtectionAttr*)&pPattern->GetItem(ATTR_PROTECTION);
        if ( pAttr->GetHideCell() )
            DeleteArea( nTop, nBottom, IDF_CONTENTS );
        else if ( pAttr->GetHideFormula() )
        {
            // Replace all formula cells between nTop and nBottom with raw value cells.
            itPos = sc::ParseFormula(itPos, maCells, nTop, nBottom, aFunc);
        }

        pPattern = aAttrIter.Next( nTop, nBottom );
    }

    aFunc.commitCells(*this);
}


void ScColumn::SetError( SCROW nRow, const sal_uInt16 nError)
{
    if (!ValidRow(nRow))
        return;

    ScFormulaCell* pCell = new ScFormulaCell(pDocument, ScAddress(nCol, nRow, nTab));
    pCell->SetErrCode(nError);

    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    it = maCells.set(it, nRow, pCell);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());
    CellStorageModified();

    ActivateNewFormulaCell(it, nRow, *pCell);
}

void ScColumn::SetRawString( SCROW nRow, const OUString& rStr, bool bBroadcast )
{
    if (!ValidRow(nRow))
        return;

    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    maCells.set(it, nRow, rStr);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());
    CellStorageModified();

    if (bBroadcast)
        BroadcastNewCell(nRow);
}

void ScColumn::SetRawString(
    sc::ColumnBlockPosition& rBlockPos, SCROW nRow, const OUString& rStr, bool bBroadcast )
{
    if (!ValidRow(nRow))
        return;

    rBlockPos.miCellPos = GetPositionToInsert(rBlockPos.miCellPos, nRow);
    rBlockPos.miCellPos = maCells.set(rBlockPos.miCellPos, nRow, rStr);
    rBlockPos.miCellTextAttrPos = maCellTextAttrs.set(
        rBlockPos.miCellTextAttrPos, nRow, sc::CellTextAttr());
    CellStorageModified();

    if (bBroadcast)
        BroadcastNewCell(nRow);
}

void ScColumn::SetValue( SCROW nRow, double fVal )
{
    if (!ValidRow(nRow))
        return;

    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    maCells.set(it, nRow, fVal);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());
    CellStorageModified();

    BroadcastNewCell(nRow);
}

void ScColumn::SetValue(
    sc::ColumnBlockPosition& rBlockPos, SCROW nRow, double fVal, bool bBroadcast )
{
    if (!ValidRow(nRow))
        return;

    rBlockPos.miCellPos = GetPositionToInsert(rBlockPos.miCellPos, nRow);
    rBlockPos.miCellPos = maCells.set(rBlockPos.miCellPos, nRow, fVal);
    rBlockPos.miCellTextAttrPos = maCellTextAttrs.set(
        rBlockPos.miCellTextAttrPos, nRow, sc::CellTextAttr());
    CellStorageModified();

    if (bBroadcast)
        BroadcastNewCell(nRow);
}

void ScColumn::GetString( SCROW nRow, OUString& rString ) const
{
    ScRefCellValue aCell = GetCellValue(nRow);

    // ugly hack for ordering problem with GetNumberFormat and missing inherited formats
    if (aCell.meType == CELLTYPE_FORMULA)
        aCell.mpFormula->MaybeInterpret();

    sal_uLong nFormat = GetNumberFormat(nRow);
    Color* pColor = NULL;
    ScCellFormat::GetString(aCell, nFormat, rString, &pColor, *(pDocument->GetFormatTable()), pDocument);
}

const OUString* ScColumn::GetStringCell( SCROW nRow ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        return NULL;

    if (it->type != sc::element_type_string)
        return NULL;

    return &sc::string_block::at(*it->data, aPos.second);
}

double* ScColumn::GetValueCell( SCROW nRow )
{
    std::pair<sc::CellStoreType::iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it == maCells.end())
        return NULL;

    if (it->type != sc::element_type_numeric)
        return NULL;

    return &sc::numeric_block::at(*it->data, aPos.second);
}

void ScColumn::GetInputString( SCROW nRow, OUString& rString ) const
{
    ScRefCellValue aCell = GetCellValue(nRow);
    sal_uLong nFormat = GetNumberFormat(nRow);
    ScCellFormat::GetInputString(aCell, nFormat, rString, *(pDocument->GetFormatTable()), pDocument);
}

double ScColumn::GetValue( SCROW nRow ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    switch (it->type)
    {
        case sc::element_type_numeric:
            return sc::numeric_block::at(*it->data, aPos.second);
        case sc::element_type_formula:
        {
            const ScFormulaCell* p = sc::formula_block::at(*it->data, aPos.second);
            ScFormulaCell* p2 = const_cast<ScFormulaCell*>(p);
            return p2->IsValue() ? p2->GetValue() : 0.0;
        }
        default:
            ;
    }

    return 0.0;
}

const EditTextObject* ScColumn::GetEditText( SCROW nRow ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        return NULL;

    if (it->type != sc::element_type_edittext)
        return NULL;

    return sc::edittext_block::at(*it->data, aPos.second);
}

void ScColumn::RemoveEditTextCharAttribs( SCROW nRow, const ScPatternAttr& rAttr )
{
    std::pair<sc::CellStoreType::iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it == maCells.end())
        return;

    if (it->type != sc::element_type_edittext)
        return;

    EditTextObject* p = sc::edittext_block::at(*it->data, aPos.second);
    ScEditUtil::RemoveCharAttribs(*p, rAttr);
}

void ScColumn::GetFormula( SCROW nRow, OUString& rFormula ) const
{
    const ScFormulaCell* p = FetchFormulaCell(nRow);
    if (p)
        p->GetFormula(rFormula);
    else
        rFormula = EMPTY_OUSTRING;
}

const ScTokenArray* ScColumn::GetFormulaTokens( SCROW nRow ) const
{
    const ScFormulaCell* pCell = FetchFormulaCell(nRow);
    if (!pCell)
        return NULL;

    return pCell->GetCode();
}

const ScFormulaCell* ScColumn::GetFormulaCell( SCROW nRow ) const
{
    return FetchFormulaCell(nRow);
}

ScFormulaCell* ScColumn::GetFormulaCell( SCROW nRow )
{
    return const_cast<ScFormulaCell*>(FetchFormulaCell(nRow));
}

CellType ScColumn::GetCellType( SCROW nRow ) const
{
    switch (maCells.get_type(nRow))
    {
        case sc::element_type_numeric:
            return CELLTYPE_VALUE;
        case sc::element_type_string:
            return CELLTYPE_STRING;
        case sc::element_type_edittext:
            return CELLTYPE_EDIT;
        case sc::element_type_formula:
            return CELLTYPE_FORMULA;
        default:
            ;
    }
    return CELLTYPE_NONE;
}

namespace {

/**
 * Count the number of all non-empty cells.
 */
class CellCounter
{
    size_t mnCount;
public:
    CellCounter() : mnCount(0) {}

    void operator() (const sc::CellStoreType::value_type& node)
    {
        if (node.type == sc::element_type_empty)
            return;

        mnCount += node.size;
    }

    size_t getCount() const { return mnCount; }
};

}

SCSIZE ScColumn::GetCellCount() const
{
    CellCounter aFunc;
    std::for_each(maCells.begin(), maCells.end(), aFunc);
    return aFunc.getCount();
}

sal_uInt16 ScColumn::GetErrCode( SCROW nRow ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        return 0;

    if (it->type != sc::element_type_formula)
        return 0;

    const ScFormulaCell* p = sc::formula_block::at(*it->data, aPos.second);
    return const_cast<ScFormulaCell*>(p)->GetErrCode();
}


bool ScColumn::HasStringData( SCROW nRow ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    switch (aPos.first->type)
    {
        case sc::element_type_string:
        case sc::element_type_edittext:
            return true;
        case sc::element_type_formula:
        {
            const ScFormulaCell* p = sc::formula_block::at(*aPos.first->data, aPos.second);
            return !const_cast<ScFormulaCell*>(p)->IsValue();
        }
        default:
            ;
    }

    return false;
}


bool ScColumn::HasValueData( SCROW nRow ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    switch (aPos.first->type)
    {
        case sc::element_type_numeric:
            return true;
        case sc::element_type_formula:
        {
            const ScFormulaCell* p = sc::formula_block::at(*aPos.first->data, aPos.second);
            return const_cast<ScFormulaCell*>(p)->IsValue();
        }
        default:
            ;
    }

    return false;
}

/**
 * Return true if there is a string or editcell in the range
 */
bool ScColumn::HasStringCells( SCROW nStartRow, SCROW nEndRow ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nStartRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    size_t nOffset = aPos.second;
    SCROW nRow = nStartRow;
    for (; it != maCells.end() && nRow <= nEndRow; ++it)
    {
        if (it->type == sc::element_type_string || it->type == sc::element_type_edittext)
            return true;

        nRow += it->size - nOffset;
        nOffset = 0;
    }

    return false;
}

namespace {

class MaxStringLenHandler
{
    sal_Int32 mnMaxLen;
    const ScColumn& mrColumn;
    SvNumberFormatter* mpFormatter;
    CharSet meCharSet;
    bool mbOctetEncoding;

    void processCell(size_t nRow, ScRefCellValue& rCell)
    {
        Color* pColor;
        OUString aString;
        sal_uInt32 nFormat = static_cast<const SfxUInt32Item*>(mrColumn.GetAttr(nRow, ATTR_VALUE_FORMAT))->GetValue();
        ScCellFormat::GetString(rCell, nFormat, aString, &pColor, *mpFormatter, &mrColumn.GetDoc());
        sal_Int32 nLen = 0;
        if (mbOctetEncoding)
        {
            OString aOString;
            if (!aString.convertToString(&aOString, meCharSet,
                        RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
                        RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR))
            {
                // TODO: anything? this is used by the dBase export filter
                // that throws an error anyway, but in case of another
                // context we might want to indicate a conversion error
                // early.
            }
            nLen = aOString.getLength();
        }
        else
            nLen = aString.getLength() * sizeof(sal_Unicode);

        if (mnMaxLen < nLen)
            mnMaxLen = nLen;
    }

public:
    MaxStringLenHandler(const ScColumn& rColumn, CharSet eCharSet) :
        mnMaxLen(0),
        mrColumn(rColumn),
        mpFormatter(rColumn.GetDoc().GetFormatTable()),
        meCharSet(eCharSet),
        mbOctetEncoding(rtl_isOctetTextEncoding(eCharSet))
    {
    }

    void operator() (size_t nRow, double fVal)
    {
        ScRefCellValue aCell(fVal);
        processCell(nRow, aCell);
    }

    void operator() (size_t nRow, const OUString& rStr)
    {
        ScRefCellValue aCell(&rStr);
        processCell(nRow, aCell);
    }

    void operator() (size_t nRow, const EditTextObject* p)
    {
        ScRefCellValue aCell(p);
        processCell(nRow, aCell);
    }

    void operator() (size_t nRow, const ScFormulaCell* p)
    {
        ScRefCellValue aCell(const_cast<ScFormulaCell*>(p));
        processCell(nRow, aCell);
    }

    sal_Int32 getMaxLen() const { return mnMaxLen; }
};

}

sal_Int32 ScColumn::GetMaxStringLen( SCROW nRowStart, SCROW nRowEnd, CharSet eCharSet ) const
{
    MaxStringLenHandler aFunc(*this, eCharSet);
    sc::ParseAllNonEmpty(maCells.begin(), maCells, nRowStart, nRowEnd, aFunc);
    return aFunc.getMaxLen();
}

namespace {

class MaxNumStringLenHandler
{
    const ScColumn& mrColumn;
    SvNumberFormatter* mpFormatter;
    sal_Int32 mnMaxLen;
    sal_uInt16 mnPrecision;

    void processCell(size_t nRow, ScRefCellValue& rCell)
    {
        if (rCell.meType == CELLTYPE_FORMULA && !rCell.mpFormula->IsValue())
            return;

        OUString aString;
        sal_uInt32 nFormat = static_cast<const SfxUInt32Item*>(
            mrColumn.GetAttr(nRow, ATTR_VALUE_FORMAT))->GetValue();
        ScCellFormat::GetInputString(rCell, nFormat, aString, *mpFormatter, &mrColumn.GetDoc());
        sal_Int32 nLen = aString.getLength();
        if (nLen <= 0)
            // Ignore empty string.
            return;

        if (nFormat)
        {
            const SvNumberformat* pEntry = mpFormatter->GetEntry(nFormat);
            sal_uInt16 nPrec;
            if (pEntry)
            {
                bool bThousand, bNegRed;
                sal_uInt16 nLeading;
                pEntry->GetFormatSpecialInfo(bThousand, bNegRed, nPrec, nLeading);
            }
            else
                nPrec = mpFormatter->GetFormatPrecision(nFormat);

            if (nPrec != SvNumberFormatter::UNLIMITED_PRECISION && nPrec > mnPrecision)
                mnPrecision = nPrec;
        }

        if (mnPrecision)
        {   // less than mnPrecision in string => widen it
            // more => shorten it
            OUString aSep = mpFormatter->GetFormatDecimalSep(nFormat);
            sal_Int32 nTmp = aString.indexOf(aSep);
            if ( nTmp == -1 )
                nLen += mnPrecision + aSep.getLength();
            else
            {
                nTmp = aString.getLength() - (nTmp + aSep.getLength());
                if (nTmp != mnPrecision)
                    nLen += mnPrecision - nTmp;
                    // nPrecision > nTmp : nLen + Diff
                    // nPrecision < nTmp : nLen - Diff
            }
        }

        if (mnMaxLen < nLen)
            mnMaxLen = nLen;
    }

public:
    MaxNumStringLenHandler(const ScColumn& rColumn, sal_uInt16 nPrecision) :
        mrColumn(rColumn), mpFormatter(rColumn.GetDoc().GetFormatTable()),
        mnMaxLen(0), mnPrecision(nPrecision)
    {
    }

    void operator() (size_t nRow, double fVal)
    {
        ScRefCellValue aCell(fVal);
        processCell(nRow, aCell);
    }

    void operator() (size_t nRow, const ScFormulaCell* p)
    {
        ScRefCellValue aCell(const_cast<ScFormulaCell*>(p));
        processCell(nRow, aCell);
    }

    sal_Int32 getMaxLen() const { return mnMaxLen; }

    sal_uInt16 getPrecision() const { return mnPrecision; }
};

}

xub_StrLen ScColumn::GetMaxNumberStringLen(
    sal_uInt16& nPrecision, SCROW nRowStart, SCROW nRowEnd ) const
{
    nPrecision = pDocument->GetDocOptions().GetStdPrecision();
    if ( nPrecision == SvNumberFormatter::UNLIMITED_PRECISION )
        // In case of unlimited precision, use 2 instead.
        nPrecision = 2;

    MaxNumStringLenHandler aFunc(*this, nPrecision);
    sc::ParseFormulaNumeric(maCells.begin(), maCells, nRowStart, nRowEnd, aFunc);
    nPrecision = aFunc.getPrecision();
    return aFunc.getMaxLen();
}

namespace {

class CellGroupSetter
{
    ScFormulaCellGroupRef mxGroup;
public:
    CellGroupSetter(const ScFormulaCellGroupRef& xGroup) : mxGroup(xGroup) {}

    void operator() (size_t, ScFormulaCell* pCell)
    {
        pCell->SetCellGroup(mxGroup);
    }
};

class GroupFormulaCells
{
    ScFormulaCellGroupRef mxNone;

public:

    void operator() (sc::CellStoreType::value_type& node)
    {
        if (node.type != sc::element_type_formula)
            // We are only interested in formula cells.
            return;

        ScFormulaCell* pPrev = NULL;
        ScFormulaCell* pCur = NULL;
        size_t nRow = node.position; // start row position.

        sc::formula_block::iterator it = sc::formula_block::begin(*node.data);
        sc::formula_block::iterator itEnd = sc::formula_block::end(*node.data);
        for (; it != itEnd; ++it, ++nRow, pPrev = pCur)
        {
            pCur = *it;
            if (!pPrev)
                continue;

            ScFormulaCell::CompareState eCompState = pPrev->CompareByTokenArray(*pCur);
            if (eCompState == ScFormulaCell::NotEqual)
            {
                // different formula tokens.
                pCur->SetCellGroup(mxNone);
                continue;
            }

            // Formula tokens equal those of the previous formula cell.
            ScFormulaCellGroupRef xGroup = pPrev->GetCellGroup();
            if (!xGroup)
            {
                // create a new group ...
                xGroup = pPrev->CreateCellGroup(nRow - 1, 2, eCompState == ScFormulaCell::EqualInvariant);
                pCur->SetCellGroup(xGroup);
            }
            else
            {
                // existing group. extend its length.
                pCur->SetCellGroup(xGroup);
                ++xGroup->mnLength;
            }
        }
    }
};

}

// Very[!] slow way to look for and merge contiguous runs
// of similar formulae into a formulagroup
void ScColumn::RebuildFormulaGroups()
{
    if (!mbDirtyGroups)
        return;

    RegroupFormulaCells();
    mbDirtyGroups = false;
}

void ScColumn::RegroupFormulaCells()
{
    // clear previous formula groups (if any)
    ScFormulaCellGroupRef xNone;
    CellGroupSetter aFunc(xNone);
    sc::ProcessFormula(maCells, aFunc);

    // re-build formula groups.
    std::for_each(maCells.begin(), maCells.end(), GroupFormulaCells());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
