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

#include <column.hxx>

#include <scitems.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <attarray.hxx>
#include <patattr.hxx>
#include <cellform.hxx>
#include <typedstrdata.hxx>
#include <formula/errorcodes.hxx>
#include <formula/token.hxx>
#include <brdcst.hxx>
#include <docoptio.hxx>
#include <subtotal.hxx>
#include <markdata.hxx>
#include <detfunc.hxx>
#include <postit.hxx>
#include <stringutil.hxx>
#include <docpool.hxx>
#include <globalnames.hxx>
#include <cellvalue.hxx>
#include <tokenarray.hxx>
#include <stlalgorithm.hxx>
#include <clipcontext.hxx>
#include <columnspanset.hxx>
#include <mtvcellfunc.hxx>
#include <scopetools.hxx>
#include <editutil.hxx>
#include <sharedformula.hxx>
#include <listenercontext.hxx>
#include <filterentries.hxx>

#include <com/sun/star/i18n/LocaleDataItem2.hpp>

#include <memory>

#include <mdds/flat_segment_tree.hpp>

#include <sfx2/objsh.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/broadcast.hxx>
#include <svl/sharedstringpool.hxx>
#include <editeng/editstat.hxx>

#include <cstdio>

using ::com::sun::star::i18n::LocaleDataItem2;

using namespace formula;

void ScColumn::Broadcast( SCROW nRow )
{
    ScHint aHint(SfxHintId::ScDataChanged, ScAddress(nCol, nRow, nTab));
    pDocument->Broadcast(aHint);
}

void ScColumn::BroadcastCells( const std::vector<SCROW>& rRows, SfxHintId nHint )
{
    if (rRows.empty())
        return;

    // Broadcast the changes.
    ScHint aHint(nHint, ScAddress(nCol, 0, nTab));
    std::vector<SCROW>::const_iterator itRow = rRows.begin(), itRowEnd = rRows.end();
    for (; itRow != itRowEnd; ++itRow)
    {
        aHint.GetAddress().SetRow(*itRow);
        pDocument->Broadcast(aHint);
    }
}

void ScColumn::BroadcastRows( SCROW nStartRow, SCROW nEndRow, SfxHintId nHint )
{
    sc::SingleColumnSpanSet aSpanSet;
    aSpanSet.scan(*this, nStartRow, nEndRow);
    std::vector<SCROW> aRows;
    aSpanSet.getRows(aRows);
    BroadcastCells(aRows, nHint);
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
    if (!ValidRow(nRow1) || !ValidRow(nRow2) || nRow1 > nRow2)
        return;

    DirtyCellInterpreter aFunc;
    sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aFunc);
}

void ScColumn::DeleteContent( SCROW nRow, bool bBroadcast )
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

    if (bBroadcast)
    {
        Broadcast(nRow);
        CellStorageModified();
    }
}

void ScColumn::Delete( SCROW nRow )
{
    DeleteContent(nRow, false);
    maCellTextAttrs.set_empty(nRow, nRow);
    maCellNotes.set_empty(nRow, nRow);

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
    maCellNotes.clear();
    maCellNotes.resize(MAXROWCOUNT);
    CellStorageModified();
}

void ScColumn::FreeNotes()
{
    maCellNotes.clear();
    maCellNotes.resize(MAXROWCOUNT);
}

namespace {

class ShiftFormulaPosHandler
{
public:

    void operator() (size_t nRow, ScFormulaCell* pCell)
    {
        pCell->aPos.SetRow(nRow);
    }
};

}

void ScColumn::DeleteRow( SCROW nStartRow, SCSIZE nSize, std::vector<ScAddress>* pGroupPos )
{
    pAttrArray->DeleteRow( nStartRow, nSize );

    SCROW nEndRow = nStartRow + nSize - 1;

    maBroadcasters.erase(nStartRow, nEndRow);
    maBroadcasters.resize(MAXROWCOUNT);

    CellNotesDeleting(nStartRow, nEndRow, false);
    maCellNotes.erase(nStartRow, nEndRow);
    maCellNotes.resize(MAXROWCOUNT);

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
    if (nEndRow < MAXROW) //only makes sense to do this if there *is* a row after the end row
    {
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
    }

    sc::SingleColumnSpanSet aNonEmptySpans;
    if (bShiftCells)
    {
        // Mark all non-empty cell positions below the end row.
        sc::ColumnBlockConstPosition aBlockPos;
        aBlockPos.miCellPos = itCell;
        aNonEmptySpans.scan(aBlockPos, *this, nEndRow+1, MAXROW);
    }

    sc::AutoCalcSwitch aACSwitch(*pDocument, false);

    // Remove the cells.
    maCells.erase(nStartRow, nEndRow);
    maCells.resize(MAXROWCOUNT);

    // Get the position again after the container change.
    aPos = maCells.position(nStartRow);

    // Shift the formula cell positions below the start row.
    ShiftFormulaPosHandler aShiftFormulaFunc;
    sc::ProcessFormula(aPos.first, maCells, nStartRow, MAXROW, aShiftFormulaFunc);

    bool bJoined = sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
    if (bJoined && pGroupPos)
        pGroupPos->push_back(ScAddress(nCol, nStartRow, nTab));

    // Shift the text attribute array too (before the broadcast).
    maCellTextAttrs.erase(nStartRow, nEndRow);
    maCellTextAttrs.resize(MAXROWCOUNT);

    CellStorageModified();
}

sc::CellStoreType::iterator ScColumn::GetPositionToInsert( SCROW nRow )
{
    return GetPositionToInsert(maCells.begin(), nRow);
}

void ScColumn::JoinNewFormulaCell(
    const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell )
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

class AttachFormulaCellsHandler
{
    sc::StartListeningContext& mrCxt;

public:
    explicit AttachFormulaCellsHandler(sc::StartListeningContext& rCxt)
        : mrCxt(rCxt) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        pCell->StartListeningTo(mrCxt);
    }
};

class DetachFormulaCellsHandler
{
    ScDocument* mpDoc;
    sc::EndListeningContext* mpCxt;

public:
    DetachFormulaCellsHandler( ScDocument* pDoc, sc::EndListeningContext* pCxt ) :
        mpDoc(pDoc), mpCxt(pCxt) {}

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        if (mpCxt)
            pCell->EndListeningTo(*mpCxt);
        else
            pCell->EndListeningTo(mpDoc);
    }
};

}

void ScColumn::DetachFormulaCells(
    const sc::CellStoreType::position_type& aPos, size_t nLength )
{
    // Split formula grouping at the top and bottom boundaries.
    sc::SharedFormulaUtil::splitFormulaCellGroup(aPos, nullptr);
    size_t nRow = aPos.first->position + aPos.second;
    size_t nNextTopRow = nRow + nLength; // start row of next formula group.
    if (ValidRow(nNextTopRow))
    {
        sc::CellStoreType::position_type aPos2 = maCells.position(aPos.first, nNextTopRow);
        sc::SharedFormulaUtil::splitFormulaCellGroup(aPos2, nullptr);
    }

    if (pDocument->IsClipOrUndo())
        return;

    DetachFormulaCellsHandler aFunc(pDocument, nullptr);
    sc::ProcessFormula(aPos.first, maCells, nRow, nNextTopRow-1, aFunc);
}

void ScColumn::AttachFormulaCells( sc::StartListeningContext& rCxt, SCROW nRow1, SCROW nRow2 )
{
    sc::CellStoreType::position_type aPos = maCells.position(nRow1);
    sc::CellStoreType::iterator it = aPos.first;

    sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
    if (ValidRow(nRow2+1))
    {
        aPos = maCells.position(it, nRow2+1);
        sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
    }

    if (pDocument->IsClipOrUndo())
        return;

    AttachFormulaCellsHandler aFunc(rCxt);
    sc::ProcessFormula(it, maCells, nRow1, nRow2, aFunc);
}

void ScColumn::DetachFormulaCells( sc::EndListeningContext& rCxt, SCROW nRow1, SCROW nRow2 )
{
    sc::CellStoreType::position_type aPos = maCells.position(nRow1);
    sc::CellStoreType::iterator it = aPos.first;

    // Split formula grouping at the top and bottom boundaries.
    sc::SharedFormulaUtil::splitFormulaCellGroup(aPos, &rCxt);
    if (ValidRow(nRow2+1))
    {
        aPos = maCells.position(it, nRow2+1);
        sc::SharedFormulaUtil::splitFormulaCellGroup(aPos, &rCxt);
    }

    if (pDocument->IsClipOrUndo())
        return;

    DetachFormulaCellsHandler aFunc(pDocument, &rCxt);
    sc::ProcessFormula(it, maCells, nRow1, nRow2, aFunc);
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

void ScColumn::AttachNewFormulaCell(
    const sc::CellStoreType::iterator& itPos, SCROW nRow, ScFormulaCell& rCell,
    bool bJoin, sc::StartListeningType eListenType )
{
    AttachNewFormulaCell(maCells.position(itPos, nRow), rCell, bJoin, eListenType);
}

void ScColumn::AttachNewFormulaCell(
    const sc::CellStoreType::position_type& aPos, ScFormulaCell& rCell,
    bool bJoin, sc::StartListeningType eListenType )
{
    if (bJoin)
        // See if this new formula cell can join an existing shared formula group.
        JoinNewFormulaCell(aPos, rCell);

    // When we insert from the Clipboard we still have wrong (old) References!
    // First they are rewired in CopyBlockFromClip via UpdateReference and the
    // we call StartListeningFromClip and BroadcastFromClip.
    // If we insert into the Clipboard/andoDoc, we do not use a Broadcast.
    // After Import we call CalcAfterLoad and in there Listening.
    if (pDocument->IsClipOrUndo() || pDocument->IsInsertingFromOtherDoc())
        return;

    switch (eListenType)
    {
        case sc::ConvertToGroupListening:
        {
            std::shared_ptr<sc::ColumnBlockPositionSet> pPosSet(new sc::ColumnBlockPositionSet(*pDocument));
            sc::StartListeningContext aStartCxt(*pDocument, pPosSet);
            sc::EndListeningContext aEndCxt(*pDocument, pPosSet);
            SCROW nRow = aPos.first->position + aPos.second;
            StartListeningFormulaCells(aStartCxt, aEndCxt, nRow, nRow);
        }
        break;
        case sc::SingleCellListening:
            rCell.StartListeningTo(pDocument);
        break;
        case sc::NoListening:
        default:
            ;

    }

    if (!pDocument->IsCalcingAfterLoad())
        rCell.SetDirty();
}

void ScColumn::AttachNewFormulaCells( const sc::CellStoreType::position_type& aPos, size_t nLength )
{
    // Make sure the whole length consists of formula cells.
    if (aPos.first->type != sc::element_type_formula)
        return;

    if (aPos.first->size < aPos.second + nLength)
        // Block is shorter than specified length.
        return;

    // Join the top and bottom cells only.
    ScFormulaCell* pCell = sc::formula_block::at(*aPos.first->data, aPos.second);
    JoinNewFormulaCell(aPos, *pCell);

    sc::CellStoreType::position_type aPosLast = aPos;
    aPosLast.second += nLength - 1;
    pCell = sc::formula_block::at(*aPosLast.first->data, aPosLast.second);
    JoinNewFormulaCell(aPosLast, *pCell);

    if (!pDocument->IsClipOrUndo() && !pDocument->IsInsertingFromOtherDoc())
    {
        sc::StartListeningContext aCxt(*pDocument);
        ScFormulaCell** pp = &sc::formula_block::at(*aPos.first->data, aPos.second);
        ScFormulaCell** ppEnd = pp + nLength;
        for (; pp != ppEnd; ++pp)
        {
            pCell = *pp;
            pCell->StartListeningTo(aCxt);
            if (!pDocument->IsCalcingAfterLoad())
                pCell->SetDirty();
        }
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

bool ScColumn::UpdateScriptType( sc::CellTextAttr& rAttr, SCROW nRow, const sc::CellStoreType::iterator& itr )
{
    if (rAttr.mnScriptType != SvtScriptType::UNKNOWN)
        // Already updated. Nothing to do.
        return false;

    // Script type not yet determined. Determine the real script
    // type, and store it.
    const ScPatternAttr* pPattern = GetPattern(nRow);
    if (!pPattern)
        return false;

    sc::CellStoreType::position_type pos = maCells.position(itr, nRow);
    sc::CellStoreType::iterator itr2 = pos.first;
    size_t nOffset = pos.second;
    ScRefCellValue aCell = GetCellValue( itr2, nOffset );
    ScAddress aPos(nCol, nRow, nTab);

    const SfxItemSet* pCondSet = nullptr;
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
    bool mbDateTime:1;
    ScColumn& mrCol;

public:
    DeleteAreaHandler(ScDocument& rDoc, InsertDeleteFlags nDelFlag, ScColumn& rCol) :
        mrDoc(rDoc),
        mbNumeric(nDelFlag & InsertDeleteFlags::VALUE),
        mbString(nDelFlag & InsertDeleteFlags::STRING),
        mbFormula(nDelFlag & InsertDeleteFlags::FORMULA),
        mbDateTime(nDelFlag & InsertDeleteFlags::DATETIME),
        mrCol(rCol) {}

    void operator() (const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize)
    {
        switch (node.type)
        {
            case sc::element_type_numeric:
                // Numeric type target datetime and number, thus we have a dedicated function
                if (!mbNumeric && !mbDateTime)
                    return;

                // If numeric and datetime selected, delete full range
                if (mbNumeric && mbDateTime)
                    break;

                deleteNumeric(node, nOffset, nDataSize);
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

    void deleteNumeric(const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize)
    {
        size_t nStart = node.position + nOffset;
        size_t nElements = 1;
        bool bLastTypeDateTime = isDateTime(nStart); // true = datetime, false = numeric
        size_t nCount = nStart + nDataSize;

        for (size_t i = nStart + 1; i < nCount; i++)
        {
            bool bIsDateTime = isDateTime(i);

            // same type as previous
            if (bIsDateTime == bLastTypeDateTime)
            {
                nElements++;
            }
            // type switching
            else
            {
                deleteNumberOrDateTime(nStart, nStart + nElements - 1, bLastTypeDateTime);
                nStart += nElements;
                nElements = 1;
            }

            bLastTypeDateTime = bIsDateTime;
        }

        // delete last cells
        deleteNumberOrDateTime(nStart, nStart + nElements - 1, bLastTypeDateTime);
    }

    void deleteNumberOrDateTime(SCROW nRow1, SCROW nRow2, bool dateTime)
    {
        if (!dateTime && !mbNumeric) // numeric flag must be selected
            return;
        if (dateTime && !mbDateTime) // datetime flag must be selected
            return;
        maDeleteRanges.set(nRow1, nRow2, true);
    }

    bool isDateTime(size_t position)
    {
        short nType = mrDoc.GetFormatTable()->GetType(static_cast<const SfxUInt32Item&>(
                          mrCol.GetAttr(position, ATTR_VALUE_FORMAT)).GetValue());

        return (nType == css::util::NumberFormat::DATE) || (nType == css::util::NumberFormat::TIME) ||
               (nType == css::util::NumberFormat::DATETIME);
    }

    void endFormulas()
    {
        mrDoc.EndListeningFormulaCells(maFormulaCells);
    }

    sc::SingleColumnSpanSet& getSpans()
    {
        return maDeleteRanges;
    }
};

class EmptyCells
{
    ScColumn& mrColumn;
    sc::ColumnBlockPosition& mrPos;

    static void splitFormulaGrouping(const sc::CellStoreType::position_type& rPos)
    {
        if (rPos.first->type == sc::element_type_formula)
        {
            ScFormulaCell& rCell = *sc::formula_block::at(*rPos.first->data, rPos.second);
            sc::SharedFormulaUtil::unshareFormulaCell(rPos, rCell);
        }
    }

public:
    EmptyCells( sc::ColumnBlockPosition& rPos, ScColumn& rColumn ) :
        mrColumn(rColumn), mrPos(rPos) {}

    void operator() (const sc::RowSpan& rSpan)
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

void ScColumn::DeleteCells(
    sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2, InsertDeleteFlags nDelFlag,
    sc::SingleColumnSpanSet& rDeleted )
{
    // Determine which cells to delete based on the deletion flags.
    DeleteAreaHandler aFunc(*pDocument, nDelFlag, *this);
    sc::CellStoreType::iterator itPos = maCells.position(rBlockPos.miCellPos, nRow1).first;
    sc::ProcessBlock(itPos, maCells, aFunc, nRow1, nRow2);
    aFunc.endFormulas(); // Have the formula cells stop listening.

    // Get the deletion spans.
    sc::SingleColumnSpanSet::SpansType aSpans;
    aFunc.getSpans().getSpans(aSpans);

    // Delete the cells for real.
    std::for_each(aSpans.begin(), aSpans.end(), EmptyCells(rBlockPos, *this));
    CellStorageModified();

    aFunc.getSpans().swap(rDeleted);
}

void ScColumn::DeleteArea(
    SCROW nStartRow, SCROW nEndRow, InsertDeleteFlags nDelFlag, bool bBroadcast,
    sc::ColumnSpanSet* pBroadcastSpans )
{
    InsertDeleteFlags nContMask = InsertDeleteFlags::CONTENTS;
    // InsertDeleteFlags::NOCAPTIONS needs to be passed too, if InsertDeleteFlags::NOTE is set
    if( nDelFlag & InsertDeleteFlags::NOTE )
        nContMask |= InsertDeleteFlags::NOCAPTIONS;
    InsertDeleteFlags nContFlag = nDelFlag & nContMask;

    sc::SingleColumnSpanSet aDeletedRows;

    sc::ColumnBlockPosition aBlockPos;
    InitBlockPosition(aBlockPos);

    if (!IsEmptyData() && nContFlag != InsertDeleteFlags::NONE)
    {
        DeleteCells(aBlockPos, nStartRow, nEndRow, nDelFlag, aDeletedRows);
        if (pBroadcastSpans)
        {
            sc::SingleColumnSpanSet::SpansType aSpans;
            aDeletedRows.getSpans(aSpans);
            sc::SingleColumnSpanSet::SpansType::const_iterator it = aSpans.begin(), itEnd = aSpans.end();
            for (; it != itEnd; ++it)
                pBroadcastSpans->set(nTab, nCol, it->mnRow1, it->mnRow2, true);
        }
    }

    if (nDelFlag & InsertDeleteFlags::NOTE)
    {
        bool bForgetCaptionOwnership = ((nDelFlag & InsertDeleteFlags::FORGETCAPTIONS) != InsertDeleteFlags::NONE);
        DeleteCellNotes(aBlockPos, nStartRow, nEndRow, bForgetCaptionOwnership);
    }

    if ( nDelFlag & InsertDeleteFlags::EDITATTR )
    {
        OSL_ENSURE( nContFlag == InsertDeleteFlags::NONE, "DeleteArea: Wrong Flags" );
        RemoveEditAttribs( nStartRow, nEndRow );
    }

    // Delete attributes just now
    if ((nDelFlag & InsertDeleteFlags::ATTRIB) == InsertDeleteFlags::ATTRIB)
        pAttrArray->DeleteArea( nStartRow, nEndRow );
    else if ((nDelFlag & InsertDeleteFlags::HARDATTR) == InsertDeleteFlags::HARDATTR)
        pAttrArray->DeleteHardAttr( nStartRow, nEndRow );

    if (bBroadcast)
    {
        // Broadcast on only cells that were deleted; no point broadcasting on
        // cells that were already empty before the deletion.
        std::vector<SCROW> aRows;
        aDeletedRows.getRows(aRows);
        BroadcastCells(aRows, SfxHintId::ScDataChanged);
    }
}

bool ScColumn::InitBlockPosition( sc::ColumnBlockPosition& rBlockPos )
{
    rBlockPos.miBroadcasterPos = maBroadcasters.begin();
    rBlockPos.miCellNotePos = maCellNotes.begin();
    rBlockPos.miCellTextAttrPos = maCellTextAttrs.begin();
    rBlockPos.miCellPos = maCells.begin();
    return true;
}

void ScColumn::InitBlockPosition( sc::ColumnBlockConstPosition& rBlockPos ) const
{
    rBlockPos.miBroadcasterPos = maBroadcasters.begin();
    rBlockPos.miCellNotePos = maCellNotes.begin();
    rBlockPos.miCellTextAttrPos = maCellTextAttrs.begin();
    rBlockPos.miCellPos = maCells.begin();
}

namespace {

class CopyAttrArrayByRange
{
    ScAttrArray& mrDestAttrArray;
    ScAttrArray& mrSrcAttrArray;
    long mnRowOffset;
public:
    CopyAttrArrayByRange(ScAttrArray& rDestAttrArray, ScAttrArray& rSrcAttrArray, long nRowOffset) :
        mrDestAttrArray(rDestAttrArray), mrSrcAttrArray(rSrcAttrArray), mnRowOffset(nRowOffset) {}

    void operator() (const sc::RowSpan& rSpan)
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
    SCTAB mnSrcTab;
    SCCOL mnSrcCol;
    long mnRowOffset;
    sc::ColumnBlockPosition maDestBlockPos;
    sc::ColumnBlockPosition* mpDestBlockPos; // to save it for next iteration.
    svl::SharedStringPool* mpSharedStringPool;

    void insertRefCell(SCROW nSrcRow, SCROW nDestRow)
    {
        ScAddress aSrcPos(mnSrcCol, nSrcRow, mnSrcTab);
        ScAddress aDestPos(mnCol, nDestRow, mnTab);
        ScSingleRefData aRef;
        aRef.InitAddress(aSrcPos);
        aRef.SetFlag3D(true);

        ScTokenArray aArr;
        aArr.AddSingleReference(aRef);

        mrDestCol.SetFormulaCell(
            maDestBlockPos, nDestRow, new ScFormulaCell(&mrDestCol.GetDoc(), aDestPos, aArr));
    }

    void duplicateNotes(SCROW nStartRow, size_t nDataSize, bool bCloneCaption )
    {
        mrSrcCol.DuplicateNotes(nStartRow, nDataSize, mrDestCol, maDestBlockPos, bCloneCaption, mnRowOffset);
    }

public:
    CopyCellsFromClipHandler(sc::CopyFromClipContext& rCxt, ScColumn& rSrcCol, ScColumn& rDestCol, SCTAB nDestTab, SCCOL nDestCol, long nRowOffset, svl::SharedStringPool* pSharedStringPool) :
        mrCxt(rCxt),
        mrSrcCol(rSrcCol),
        mrDestCol(rDestCol),
        mnTab(nDestTab),
        mnCol(nDestCol),
        mnSrcTab(rSrcCol.GetTab()),
        mnSrcCol(rSrcCol.GetCol()),
        mnRowOffset(nRowOffset),
        mpDestBlockPos(mrCxt.getBlockPosition(nDestTab, nDestCol)),
        mpSharedStringPool(pSharedStringPool)
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
        SCROW nSrcRow1 = node.position + nOffset;
        bool bCopyCellNotes = mrCxt.isCloneNotes();

        InsertDeleteFlags nFlags = mrCxt.getInsertFlag();

        if (node.type == sc::element_type_empty)
        {
            if (bCopyCellNotes && !mrCxt.isSkipAttrForEmptyCells())
            {
                bool bCloneCaption = (nFlags & InsertDeleteFlags::NOCAPTIONS) == InsertDeleteFlags::NONE;
                duplicateNotes(nSrcRow1, nDataSize, bCloneCaption );
            }
            return;
        }

        bool bNumeric = (nFlags & InsertDeleteFlags::VALUE) != InsertDeleteFlags::NONE;
        bool bDateTime = (nFlags & InsertDeleteFlags::DATETIME) != InsertDeleteFlags::NONE;
        bool bString   = (nFlags & InsertDeleteFlags::STRING) != InsertDeleteFlags::NONE;
        bool bBoolean  = (nFlags & InsertDeleteFlags::SPECIAL_BOOLEAN) != InsertDeleteFlags::NONE;
        bool bFormula  = (nFlags & InsertDeleteFlags::FORMULA) != InsertDeleteFlags::NONE;

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
                    bool bCopy = mrCxt.isDateCell(mrSrcCol, nSrcRow) ? bDateTime : bNumeric;
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
                    break;

                sc::string_block::const_iterator it = sc::string_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::string_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                for (SCROW nSrcRow = nSrcRow1; it != itEnd; ++it, ++nSrcRow)
                {
                    if (bAsLink)
                        insertRefCell(nSrcRow, nSrcRow + mnRowOffset);
                    else if (mpSharedStringPool)
                    {
                        // Re-intern the string if source is a different document.
                        svl::SharedString aInterned = mpSharedStringPool->intern( (*it).getString());
                        mrDestCol.SetRawString(maDestBlockPos, nSrcRow + mnRowOffset, aInterned);
                    }
                    else
                        mrDestCol.SetRawString(maDestBlockPos, nSrcRow + mnRowOffset, *it);
                }
            }
            break;
            case sc::element_type_edittext:
            {
                if (!bString)
                    break;

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
                    ScFormulaCell& rSrcCell = **it;
                    bool bForceFormula = false;
                    if (bBoolean)
                    {
                        // See if the formula consists of =TRUE() or =FALSE().
                        ScTokenArray* pCode = rSrcCell.GetCode();
                        if (pCode && pCode->GetLen() == 1)
                        {
                            const formula::FormulaToken* p = pCode->FirstToken();
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

                        FormulaError nErr = rSrcCell.GetErrCode();
                        if (nErr != FormulaError::NONE)
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
                        else if (rSrcCell.IsEmptyDisplayedAsString())
                        {
                            // Empty stays empty and doesn't become 0.
                            continue;
                        }
                        else if (rSrcCell.IsValue())
                        {
                            bool bCopy = mrCxt.isDateCell(mrSrcCol, nSrcRow) ? bDateTime : bNumeric;
                            if (!bCopy)
                                continue;

                            if (bAsLink)
                                insertRefCell(nSrcRow, nSrcRow + mnRowOffset);
                            else
                                mrDestCol.SetValue(maDestBlockPos, nSrcRow + mnRowOffset, rSrcCell.GetValue());
                        }
                        else if (bString)
                        {
                            svl::SharedString aStr = rSrcCell.GetString();
                            if (aStr.isEmpty())
                                // do not clone empty string
                                continue;

                            if (bAsLink)
                                insertRefCell(nSrcRow, nSrcRow + mnRowOffset);
                            else if (rSrcCell.IsMultilineResult())
                            {
                                // Clone as an edit text object.
                                ScFieldEditEngine& rEngine = mrDestCol.GetDoc().GetEditEngine();
                                rEngine.SetText(aStr.getString());
                                mrDestCol.SetEditText(maDestBlockPos, nSrcRow + mnRowOffset, rEngine.CreateTextObject());
                            }
                            else if (mpSharedStringPool)
                            {
                                // Re-intern the string if source is a different document.
                                svl::SharedString aInterned = mpSharedStringPool->intern( aStr.getString());
                                mrDestCol.SetRawString(maDestBlockPos, nSrcRow + mnRowOffset, aInterned);
                            }
                            else
                            {
                                mrDestCol.SetRawString(maDestBlockPos, nSrcRow + mnRowOffset, aStr);
                            }
                        }
                    }
                }
            }
            break;
            default:
                ;
        }
        if (bCopyCellNotes)
        {
            bool bCloneCaption = (nFlags & InsertDeleteFlags::NOCAPTIONS) == InsertDeleteFlags::NONE;
            duplicateNotes(nSrcRow1, nDataSize, bCloneCaption );
        }
    }
};

class CopyTextAttrsFromClipHandler
{
    sc::CellTextAttrStoreType& mrAttrs;
    sc::CellTextAttrStoreType::iterator miPos;
    size_t mnDelta;

public:
    CopyTextAttrsFromClipHandler( sc::CellTextAttrStoreType& rAttrs, size_t nDelta ) :
        mrAttrs(rAttrs), miPos(mrAttrs.begin()), mnDelta(nDelta) {}

    void operator() ( const sc::CellTextAttrStoreType::value_type& aNode, size_t nOffset, size_t nDataSize )
    {
        if (aNode.type != sc::element_type_celltextattr)
            return;

        sc::celltextattr_block::const_iterator it = sc::celltextattr_block::begin(*aNode.data);
        std::advance(it, nOffset);
        sc::celltextattr_block::const_iterator itEnd = it;
        std::advance(itEnd, nDataSize);

        size_t nPos = aNode.position + nOffset + mnDelta;
        miPos = mrAttrs.set(miPos, nPos, it, itEnd);
    }
};

}

//  rColumn = source
//  nRow1, nRow2 = target position

void ScColumn::CopyFromClip(
    sc::CopyFromClipContext& rCxt, SCROW nRow1, SCROW nRow2, long nDy, ScColumn& rColumn )
{
    if ((rCxt.getInsertFlag() & InsertDeleteFlags::ATTRIB) != InsertDeleteFlags::NONE)
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
    if ((rCxt.getInsertFlag() & InsertDeleteFlags::CONTENTS) == InsertDeleteFlags::NONE)
        return;

    if (rCxt.isAsLink() && rCxt.getInsertFlag() == InsertDeleteFlags::ALL)
    {
        // We also reference empty cells for "ALL"
        // InsertDeleteFlags::ALL must always contain more flags when compared to "Insert contents" as
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
            SetFormulaCell(nDestRow, new ScFormulaCell(pDocument, aDestPos, aArr));
        }

        // Don't forget to copy the cell text attributes.
        CopyTextAttrsFromClipHandler aFunc(maCellTextAttrs, nDy);
        sc::ParseBlock(rColumn.maCellTextAttrs.begin(), rColumn.maCellTextAttrs, aFunc, nRow1-nDy, nRow2-nDy);

        return;
    }

    // Compare the ScDocumentPool* to determine if we are copying within the
    // same document. If not, re-intern shared strings.
    svl::SharedStringPool* pSharedStringPool = (rColumn.pDocument->GetPool() != pDocument->GetPool()) ?
        &pDocument->GetSharedStringPool() : nullptr;

    // nRow1 to nRow2 is for destination (this) column. Subtract nDy to get the source range.
    // Copy all cells in the source column (rColumn) from nRow1-nDy to nRow2-nDy to this column.
    {
        CopyCellsFromClipHandler aFunc(rCxt, rColumn, *this, nTab, nCol, nDy, pSharedStringPool);
        sc::ParseBlock(rColumn.maCells.begin(), rColumn.maCells, aFunc, nRow1-nDy, nRow2-nDy);
    }

    {
        // Don't forget to copy the cell text attributes.
        CopyTextAttrsFromClipHandler aFunc(maCellTextAttrs, nDy);
        sc::ParseBlock(rColumn.maCellTextAttrs.begin(), rColumn.maCellTextAttrs, aFunc, nRow1-nDy, nRow2-nDy);
    }
}

void ScColumn::MixMarked(
    sc::MixDocContext& rCxt, const ScMarkData& rMark, ScPasteFunc nFunction,
    bool bSkipEmpty, const ScColumn& rSrcCol )
{
    SCROW nRow1, nRow2;

    if (rMark.IsMultiMarked())
    {
        ScMultiSelIter aIter( rMark.GetMultiSelData(), nCol );
        while (aIter.Next( nRow1, nRow2 ))
            MixData(rCxt, nRow1, nRow2, nFunction, bSkipEmpty, rSrcCol);
    }
}

namespace {

// Result in rVal1
bool lcl_DoFunction( double& rVal1, double nVal2, ScPasteFunc nFunction )
{
    bool bOk = false;
    switch (nFunction)
    {
        case ScPasteFunc::ADD:
            bOk = SubTotal::SafePlus( rVal1, nVal2 );
            break;
        case ScPasteFunc::SUB:
            nVal2 = -nVal2;     // FIXME: Can we do this always without error?
            bOk = SubTotal::SafePlus( rVal1, nVal2 );
            break;
        case ScPasteFunc::MUL:
            bOk = SubTotal::SafeMult( rVal1, nVal2 );
            break;
        case ScPasteFunc::DIV:
            bOk = SubTotal::SafeDiv( rVal1, nVal2 );
            break;
        default: break;
    }
    return bOk;
}

void lcl_AddCode( ScTokenArray& rArr, const ScFormulaCell* pCell )
{
    rArr.AddOpCode(ocOpen);

    ScTokenArray* pCode = const_cast<ScFormulaCell*>(pCell)->GetCode();
    if (pCode)
    {
        FormulaTokenArrayPlainIterator aIter(*pCode);
        const formula::FormulaToken* pToken = aIter.First();
        while (pToken)
        {
            rArr.AddToken( *pToken );
            pToken = aIter.Next();
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
    ScPasteFunc mnFunction;

    bool mbSkipEmpty;

    void doFunction( size_t nDestRow, double fVal1, double fVal2 )
    {
        bool bOk = lcl_DoFunction(fVal1, fVal2, mnFunction);

        if (bOk)
            miNewCellsPos = maNewCells.set(miNewCellsPos, nDestRow-mnRowOffset, fVal1);
        else
        {
            ScAddress aPos(mrDestColumn.GetCol(), nDestRow, mrDestColumn.GetTab());

            ScFormulaCell* pFC = new ScFormulaCell(&mrDestColumn.GetDoc(), aPos);
            pFC->SetErrCode(FormulaError::NoValue);

            miNewCellsPos = maNewCells.set(miNewCellsPos, nDestRow-mnRowOffset, pFC);
        }
    }

public:
    MixDataHandler(
        sc::ColumnBlockPosition& rBlockPos,
        ScColumn& rDestColumn,
        SCROW nRow1, SCROW nRow2,
        ScPasteFunc nFunction, bool bSkipEmpty) :
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
            case sc::element_type_empty:
            case sc::element_type_numeric:
            {
                double fSrcVal = 0.0;
                if (aPos.first->type == sc::element_type_numeric)
                    fSrcVal = sc::numeric_block::at(*aPos.first->data, aPos.second);

                // Both src and dest are of numeric type.
                doFunction(nRow, f, fSrcVal);
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
                    case ScPasteFunc::ADD: eOp = ocAdd; break;
                    case ScPasteFunc::SUB: eOp = ocSub; break;
                    case ScPasteFunc::MUL: eOp = ocMul; break;
                    case ScPasteFunc::DIV: eOp = ocDiv; break;
                    default: break;
                }
                aArr.AddOpCode(eOp); // Function

                // Second row
                ScFormulaCell* pDest = sc::formula_block::at(*aPos.first->data, aPos.second);
                lcl_AddCode(aArr, pDest);

                miNewCellsPos = maNewCells.set(
                    miNewCellsPos, nRow-mnRowOffset,
                    new ScFormulaCell(
                        &mrDestColumn.GetDoc(), ScAddress(mrDestColumn.GetCol(), nRow, mrDestColumn.GetTab()), aArr));
            }
            break;
            case sc::element_type_string:
            case sc::element_type_edittext:
            {
                // Destination cell is not a number. Just take the source cell.
                miNewCellsPos = maNewCells.set(miNewCellsPos, nRow-mnRowOffset, f);
            }
            break;
            default:
                ;
        }
    }

    void operator() (size_t nRow, const svl::SharedString& rStr)
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
                    case ScPasteFunc::ADD: eOp = ocAdd; break;
                    case ScPasteFunc::SUB: eOp = ocSub; break;
                    case ScPasteFunc::MUL: eOp = ocMul; break;
                    case ScPasteFunc::DIV: eOp = ocDiv; break;
                    default: break;
                }
                aArr.AddOpCode(eOp); // Function

                // Second row
                aArr.AddDouble(sc::numeric_block::at(*aPos.first->data, aPos.second));

                miNewCellsPos = maNewCells.set(
                    miNewCellsPos, nRow-mnRowOffset,
                    new ScFormulaCell(
                        &mrDestColumn.GetDoc(), ScAddress(mrDestColumn.GetCol(), nRow, mrDestColumn.GetTab()), aArr));
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
                    case ScPasteFunc::ADD: eOp = ocAdd; break;
                    case ScPasteFunc::SUB: eOp = ocSub; break;
                    case ScPasteFunc::MUL: eOp = ocMul; break;
                    case ScPasteFunc::DIV: eOp = ocDiv; break;
                    default: break;
                }
                aArr.AddOpCode(eOp); // Function

                // Second row
                ScFormulaCell* pDest = sc::formula_block::at(*aPos.first->data, aPos.second);
                lcl_AddCode(aArr, pDest);

                miNewCellsPos = maNewCells.set(
                    miNewCellsPos, nRow-mnRowOffset,
                    new ScFormulaCell(
                        &mrDestColumn.GetDoc(), ScAddress(mrDestColumn.GetCol(), nRow, mrDestColumn.GetTab()), aArr));
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
                {
                    double fVal2 = sc::numeric_block::at(*aPos.first->data, aPos.second);
                    doFunction(nDestRow, 0.0, fVal2);
                }
                break;
                case sc::element_type_string:
                {
                    const svl::SharedString& aVal = sc::string_block::at(*aPos.first->data, aPos.second);
                    miNewCellsPos = maNewCells.set(
                            miNewCellsPos, nDestRow-mnRowOffset, aVal);
                }
                break;
                case sc::element_type_edittext:
                {
                    EditTextObject* pObj = sc::edittext_block::at(*aPos.first->data, aPos.second);
                    miNewCellsPos = maNewCells.set(
                            miNewCellsPos, nDestRow-mnRowOffset, pObj->Clone());
                }
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
                        case ScPasteFunc::ADD: eOp = ocAdd; break;
                        case ScPasteFunc::SUB: eOp = ocSub; break;
                        case ScPasteFunc::MUL: eOp = ocMul; break;
                        case ScPasteFunc::DIV: eOp = ocDiv; break;
                        default: break;
                    }

                    aArr.AddOpCode(eOp); // Function
                    aArr.AddDouble(0.0);

                    miNewCellsPos = maNewCells.set(
                        miNewCellsPos, nDestRow-mnRowOffset,
                        new ScFormulaCell(
                            &mrDestColumn.GetDoc(), ScAddress(mrDestColumn.GetCol(), nDestRow, mrDestColumn.GetTab()), aArr));
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
    sc::MixDocContext& rCxt, SCROW nRow1, SCROW nRow2, ScPasteFunc nFunction,
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
    return new ScAttrIterator( pAttrArray, nStartRow, nEndRow, pDocument->GetDefPattern() );
}

namespace {

class StartListenersHandler
{
    sc::StartListeningContext* mpCxt;
    bool mbAllListeners;

public:
    StartListenersHandler( sc::StartListeningContext& rCxt, bool bAllListeners ) :
        mpCxt(&rCxt), mbAllListeners(bAllListeners) {}

    void operator() ( sc::CellStoreType::value_type& aBlk )
    {
        if (aBlk.type != sc::element_type_formula)
            return;

        ScFormulaCell** pp = &sc::formula_block::at(*aBlk.data, 0);
        ScFormulaCell** ppEnd = pp + aBlk.size;

        for (; pp != ppEnd; ++pp)
        {
            ScFormulaCell& rFC = **pp;
            if (!mbAllListeners && !rFC.NeedsListening())
                continue;

            if (rFC.IsSharedTop())
            {
                sc::SharedFormulaUtil::startListeningAsGroup(*mpCxt, pp);
                pp += rFC.GetSharedLength() - 1; // Move to the last cell in the group.
            }
            else
                rFC.StartListeningTo(*mpCxt);
        }
    }
};

}

void ScColumn::StartListeners( sc::StartListeningContext& rCxt, bool bAll )
{
    std::for_each(maCells.begin(), maCells.end(), StartListenersHandler(rCxt, bAll));
}

namespace {

void applyTextNumFormat( ScColumn& rCol, SCROW nRow, SvNumberFormatter* pFormatter )
{
    sal_uInt32 nFormat = pFormatter->GetStandardFormat(css::util::NumberFormat::TEXT);
    ScPatternAttr aNewAttrs(rCol.GetDoc().GetPool());
    SfxItemSet& rSet = aNewAttrs.GetItemSet();
    rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nFormat));
    rCol.ApplyPattern(nRow, aNewAttrs);
}

}

bool ScColumn::ParseString(
    ScCellValue& rCell, SCROW nRow, SCTAB nTabP, const OUString& rString,
    formula::FormulaGrammar::AddressConvention eConv,
    const ScSetStringParam* pParam )
{
    if (rString.isEmpty())
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

    nIndex = nOldIndex = GetNumberFormat( pDocument->GetNonThreadedContext(), nRow );
    if ( rString.getLength() > 1
            && aParam.mpNumFormatter->GetType(nIndex) != css::util::NumberFormat::TEXT )
        cFirstChar = rString[0];
    else
        cFirstChar = 0; // Text

    svl::SharedStringPool& rPool = pDocument->GetSharedStringPool();

    if ( cFirstChar == '=' )
    {
        if ( rString.getLength() == 1 ) // = Text
        {
            rCell.set(rPool.intern(rString));
        }
        else if (aParam.meSetTextNumFormat == ScSetStringParam::Always)
        {
            // Set the cell format type to Text.
            applyTextNumFormat(*this, nRow, aParam.mpNumFormatter);
            rCell.set(rPool.intern(rString));
        }
        else // = Formula
            rCell.set(
                new ScFormulaCell(
                    pDocument, ScAddress(nCol, nRow, nTabP), rString,
                    formula::FormulaGrammar::mergeToGrammar(formula::FormulaGrammar::GRAM_DEFAULT, eConv),
                    ScMatrixMode::NONE));
    }
    else if ( cFirstChar == '\'') // 'Text
    {
        bool bNumeric = false;
        if (aParam.mbHandleApostrophe)
        {
            // Cell format is not 'Text', and the first char
            // is an apostrophe. Check if the input is considered a number.
            OUString aTest = rString.copy(1);
            double fTest;
            bNumeric = aParam.mpNumFormatter->IsNumberFormat(aTest, nIndex, fTest);
            if (bNumeric)
                // This is a number. Strip out the first char.
                rCell.set(rPool.intern(aTest));
        }
        if (!bNumeric)
            // This is normal text. Take it as-is.
            rCell.set(rPool.intern(rString));
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

                // convert back to the original language if a built-in format was detected
                const SvNumberformat* pOldFormat = aParam.mpNumFormatter->GetEntry( nOldIndex );
                if ( pOldFormat )
                    nIndex = aParam.mpNumFormatter->GetFormatForLanguageIfBuiltIn( nIndex, pOldFormat->GetLanguage() );

                rCell.set(nVal);
                if ( nIndex != nOldIndex)
                {
                    // #i22345# New behavior: Apply the detected number format only if
                    // the old one was the default number, date, time or boolean format.
                    // Exception: If the new format is boolean, always apply it.

                    bool bOverwrite = false;
                    if ( pOldFormat )
                    {
                        short nOldType = pOldFormat->GetMaskedType();
                        if ( nOldType == css::util::NumberFormat::NUMBER || nOldType == css::util::NumberFormat::DATE ||
                             nOldType == css::util::NumberFormat::TIME || nOldType == css::util::NumberFormat::LOGICAL )
                        {
                            if ( nOldIndex == aParam.mpNumFormatter->GetStandardFormat(
                                                nOldType, pOldFormat->GetLanguage() ) )
                            {
                                bOverwrite = true; // default of these types can be overwritten
                            }
                        }
                    }
                    if ( !bOverwrite && aParam.mpNumFormatter->GetType( nIndex ) == css::util::NumberFormat::LOGICAL )
                    {
                        bOverwrite = true; // overwrite anything if boolean was detected
                    }

                    if ( bOverwrite )
                    {
                        ApplyAttr( nRow, SfxUInt32Item( ATTR_VALUE_FORMAT,
                            nIndex) );
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

                const LocaleDataItem2& aLocaleItem = pLocale->getLocaleItem();
                const OUString& rDecSep = aLocaleItem.decimalSeparator;
                const OUString& rGroupSep = aLocaleItem.thousandSeparator;
                const OUString& rDecSepAlt = aLocaleItem.decimalSeparatorAlternative;
                if (rDecSep.getLength() != 1 || rGroupSep.getLength() != 1 || rDecSepAlt.getLength() > 1)
                    break;

                sal_Unicode dsep = rDecSep[0];
                sal_Unicode gsep = rGroupSep[0];
                sal_Unicode dsepa = rDecSepAlt.toChar();

                if (!ScStringUtil::parseSimpleNumber(rString, dsep, gsep, dsepa, nVal))
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
                applyTextNumFormat(*this, nRow, aParam.mpNumFormatter);
            }

            rCell.set(rPool.intern(rString));
        }
    }

    return bNumFmtSet;
}

/**
 * Returns true if the cell format was set as well
 */
bool ScColumn::SetString( SCROW nRow, SCTAB nTabP, const OUString& rString,
                          formula::FormulaGrammar::AddressConvention eConv,
                          ScSetStringParam* pParam )
{
    if (!ValidRow(nRow))
        return false;

    ScCellValue aNewCell;
    bool bNumFmtSet = ParseString(aNewCell, nRow, nTabP, rString, eConv, pParam);
    if (pParam)
        aNewCell.release(*this, nRow, pParam->meStartListening);
    else
        aNewCell.release(*this, nRow);

    // Do not set Formats and Formulas here anymore!
    // These are queried during output

    return bNumFmtSet;
}

void ScColumn::SetEditText( SCROW nRow, EditTextObject* pEditText )
{
    pEditText->NormalizeString(pDocument->GetSharedStringPool());
    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    maCells.set(it, nRow, pEditText);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());
    CellStorageModified();

    BroadcastNewCell(nRow);
}

void ScColumn::SetEditText( sc::ColumnBlockPosition& rBlockPos, SCROW nRow, EditTextObject* pEditText )
{
    pEditText->NormalizeString(pDocument->GetSharedStringPool());
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

    // rats, yet another "spool"
    // Sadly there is no other way to change the Pool than to
    // "spool" the Object through a corresponding Engine
    EditEngine& rEngine = pDocument->GetEditEngine();
    rEngine.SetText(rEditText);
    SetEditText(rBlockPos, nRow, rEngine.CreateTextObject());
}

void ScColumn::SetEditText( SCROW nRow, const EditTextObject& rEditText, const SfxItemPool* pEditPool )
{
    if (pEditPool && pDocument->GetEditPool() == pEditPool)
    {
        SetEditText(nRow, rEditText.Clone());
        return;
    }

    // rats, yet another "spool"
    // Sadly there is no other way to change the Pool than to
    // "spool" the Object through a corresponding Engine
    EditEngine& rEngine = pDocument->GetEditEngine();
    rEngine.SetText(rEditText);
    SetEditText(nRow, rEngine.CreateTextObject());
}

void ScColumn::SetFormula( SCROW nRow, const ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGram )
{
    ScAddress aPos(nCol, nRow, nTab);

    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    ScFormulaCell* pCell = new ScFormulaCell(pDocument, aPos, rArray, eGram);
    sal_uInt32 nCellFormat = GetNumberFormat(pDocument->GetNonThreadedContext(), nRow);
    if( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        pCell->SetNeedNumberFormat(true);
    it = maCells.set(it, nRow, pCell);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());

    CellStorageModified();

    AttachNewFormulaCell(it, nRow, *pCell);
}

void ScColumn::SetFormula( SCROW nRow, const OUString& rFormula, formula::FormulaGrammar::Grammar eGram )
{
    ScAddress aPos(nCol, nRow, nTab);

    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    ScFormulaCell* pCell = new ScFormulaCell(pDocument, aPos, rFormula, eGram);
    sal_uInt32 nCellFormat = GetNumberFormat(pDocument->GetNonThreadedContext(), nRow);
    if( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        pCell->SetNeedNumberFormat(true);
    it = maCells.set(it, nRow, pCell);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());

    CellStorageModified();

    AttachNewFormulaCell(it, nRow, *pCell);
}

ScFormulaCell* ScColumn::SetFormulaCell(
    SCROW nRow, ScFormulaCell* pCell, sc::StartListeningType eListenType )
{
    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    sal_uInt32 nCellFormat = GetNumberFormat(pDocument->GetNonThreadedContext(), nRow);
    if( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        pCell->SetNeedNumberFormat(true);
    it = maCells.set(it, nRow, pCell);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());

    CellStorageModified();

    AttachNewFormulaCell(it, nRow, *pCell, true, eListenType);
    return pCell;
}

void ScColumn::SetFormulaCell(
    sc::ColumnBlockPosition& rBlockPos, SCROW nRow, ScFormulaCell* pCell,
    sc::StartListeningType eListenType )
{
    rBlockPos.miCellPos = GetPositionToInsert(rBlockPos.miCellPos, nRow);
    sal_uInt32 nCellFormat = GetNumberFormat(pDocument->GetNonThreadedContext(), nRow);
    if( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        pCell->SetNeedNumberFormat(true);
    rBlockPos.miCellPos = maCells.set(rBlockPos.miCellPos, nRow, pCell);
    rBlockPos.miCellTextAttrPos = maCellTextAttrs.set(
        rBlockPos.miCellTextAttrPos, nRow, sc::CellTextAttr());

    CellStorageModified();

    AttachNewFormulaCell(rBlockPos.miCellPos, nRow, *pCell, true, eListenType);
}

bool ScColumn::SetFormulaCells( SCROW nRow, std::vector<ScFormulaCell*>& rCells )
{
    if (!ValidRow(nRow))
        return false;

    SCROW nEndRow = nRow + rCells.size() - 1;
    if (!ValidRow(nEndRow))
        return false;

    sc::CellStoreType::position_type aPos = maCells.position(nRow);

    // Detach all formula cells that will be overwritten.
    DetachFormulaCells(aPos, rCells.size());

    if (!pDocument->IsClipOrUndo())
    {
        for (size_t i = 0, n = rCells.size(); i < n; ++i)
        {
            SCROW nThisRow = nRow + i;
            sal_uInt32 nFmt = GetNumberFormat(pDocument->GetNonThreadedContext(), nThisRow);
            if ((nFmt % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
                rCells[i]->SetNeedNumberFormat(true);
        }
    }

    std::vector<sc::CellTextAttr> aDefaults(rCells.size(), sc::CellTextAttr());
    maCellTextAttrs.set(nRow, aDefaults.begin(), aDefaults.end());

    maCells.set(aPos.first, nRow, rCells.begin(), rCells.end());

    CellStorageModified();

    AttachNewFormulaCells(aPos, rCells.size());

    return true;
}

svl::SharedString ScColumn::GetSharedString( SCROW nRow ) const
{
    sc::CellStoreType::const_position_type aPos = maCells.position(nRow);
    switch (aPos.first->type)
    {
        case sc::element_type_string:
            return sc::string_block::at(*aPos.first->data, aPos.second);
        case sc::element_type_edittext:
        {
            const EditTextObject* pObj = sc::edittext_block::at(*aPos.first->data, aPos.second);
            std::vector<svl::SharedString> aSSs = pObj->GetSharedStrings();
            if (aSSs.size() != 1)
                // We don't handle multiline content for now.
                return svl::SharedString();

            return aSSs[0];
        }
        break;
        default:
            ;
    }
    return svl::SharedString();
}

namespace {

class FilterEntriesHandler
{
    ScColumn& mrColumn;
    ScFilterEntries& mrFilterEntries;

    void processCell(SCROW nRow, ScRefCellValue& rCell)
    {
        SvNumberFormatter* pFormatter = mrColumn.GetDoc().GetFormatTable();
        OUString aStr;
        sal_uLong nFormat = mrColumn.GetNumberFormat(mrColumn.GetDoc().GetNonThreadedContext(), nRow);
        ScCellFormat::GetInputString(rCell, nFormat, aStr, *pFormatter, &mrColumn.GetDoc());

        if (rCell.hasString())
        {
            mrFilterEntries.push_back(ScTypedStrData(aStr));
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
                FormulaError nErr = pFC->GetErrCode();
                if (nErr != FormulaError::NONE)
                {
                    // Error cell is evaluated as string (for now).
                    OUString aErr = ScGlobal::GetErrorString(nErr);
                    if (!aErr.isEmpty())
                    {
                        mrFilterEntries.push_back(ScTypedStrData(aErr));
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
        bool bDate = false;
        if ((nType & css::util::NumberFormat::DATE) && !(nType & css::util::NumberFormat::TIME))
        {
            // special case for date values.  Disregard the time
            // element if the number format is of date type.
            fVal = rtl::math::approxFloor(fVal);
            mrFilterEntries.mbHasDates = true;
            bDate = true;
            // Convert string representation to ISO 8601 date to eliminate
            // locale dependent behaviour later when filtering for dates.
            sal_uInt32 nIndex = pFormatter->GetFormatIndex( NF_DATE_DIN_YYYYMMDD);
            pFormatter->GetInputLineString( fVal, nIndex, aStr);
        }
        // maybe extend ScTypedStrData enum is also an option here
        mrFilterEntries.push_back(ScTypedStrData(aStr, fVal, ScTypedStrData::Value,bDate));
    }

public:
    FilterEntriesHandler(ScColumn& rColumn, ScFilterEntries& rFilterEntries) :
        mrColumn(rColumn), mrFilterEntries(rFilterEntries) {}

    void operator() (size_t nRow, double fVal)
    {
        ScRefCellValue aCell(fVal);
        processCell(nRow, aCell);
    }

    void operator() (size_t nRow, const svl::SharedString& rStr)
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

    void operator() (const int nElemType, size_t nRow, size_t /* nDataSize */)
    {
        if ( nElemType == sc::element_type_empty )
        {
            if (!mrFilterEntries.mbHasEmpties)
            {
                mrFilterEntries.push_back(ScTypedStrData(OUString()));
                mrFilterEntries.mbHasEmpties = true;
            }
            return;
        }
        ScRefCellValue aCell = mrColumn.GetCellValue(nRow);
        processCell(nRow, aCell);
    }
};

}

void ScColumn::GetFilterEntries(
    sc::ColumnBlockConstPosition& rBlockPos, SCROW nStartRow, SCROW nEndRow,
    ScFilterEntries& rFilterEntries )
{
    FilterEntriesHandler aFunc(*this, rFilterEntries);
    rBlockPos.miCellPos =
        sc::ParseAll(rBlockPos.miCellPos, maCells, nStartRow, nEndRow, aFunc, aFunc);
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
        miBeg(rCells.begin()), miEnd(rCells.end()), mpDoc(pDoc)
    {
        if (ValidRow(nStart))
            maPos = rCells.position(nStart);
        else
            // Make this iterator invalid.
            maPos.first = miEnd;
    }

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
                return sc::string_block::at(*maPos.first->data, maPos.second).getString();
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

// GetDataEntries - Strings from continuous Section around nRow

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
        Entry(SCROW nRow, const svl::SharedString& rStr) : mnRow(nRow), maValue(rStr) {}
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
                break;
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

    ScAttrIterator aAttrIter( pAttrArray, nStartRow, nEndRow, pDocument->GetDefPattern() );
    SCROW nTop = -1;
    SCROW nBottom = -1;
    const ScPatternAttr* pPattern = aAttrIter.Next( nTop, nBottom );
    while (pPattern)
    {
        const ScProtectionAttr* pAttr = static_cast<const ScProtectionAttr*>(&pPattern->GetItem(ATTR_PROTECTION));
        if ( pAttr->GetHideCell() )
            DeleteArea( nTop, nBottom, InsertDeleteFlags::CONTENTS );
        else if ( pAttr->GetHideFormula() )
        {
            // Replace all formula cells between nTop and nBottom with raw value cells.
            itPos = sc::ParseFormula(itPos, maCells, nTop, nBottom, aFunc);
        }

        pPattern = aAttrIter.Next( nTop, nBottom );
    }

    aFunc.commitCells(*this);
}

void ScColumn::SetError( SCROW nRow, const FormulaError nError)
{
    if (!ValidRow(nRow))
        return;

    ScFormulaCell* pCell = new ScFormulaCell(pDocument, ScAddress(nCol, nRow, nTab));
    pCell->SetErrCode(nError);

    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    it = maCells.set(it, nRow, pCell);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());

    CellStorageModified();

    AttachNewFormulaCell(it, nRow, *pCell);
}

void ScColumn::SetRawString( SCROW nRow, const OUString& rStr )
{
    if (!ValidRow(nRow))
        return;

    svl::SharedString aSS = pDocument->GetSharedStringPool().intern(rStr);
    if (!aSS.getData())
        return;

    SetRawString(nRow, aSS);
}

void ScColumn::SetRawString( SCROW nRow, const svl::SharedString& rStr )
{
    if (!ValidRow(nRow))
        return;

    sc::CellStoreType::iterator it = GetPositionToInsert(nRow);
    maCells.set(it, nRow, rStr);
    maCellTextAttrs.set(nRow, sc::CellTextAttr());

    CellStorageModified();

    BroadcastNewCell(nRow);
}

void ScColumn::SetRawString(
    sc::ColumnBlockPosition& rBlockPos, SCROW nRow, const svl::SharedString& rStr, bool bBroadcast )
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

    sal_uLong nFormat = GetNumberFormat(pDocument->GetNonThreadedContext(), nRow);
    Color* pColor = nullptr;
    ScCellFormat::GetString(aCell, nFormat, rString, &pColor, *(pDocument->GetFormatTable()), pDocument);
}

double* ScColumn::GetValueCell( SCROW nRow )
{
    std::pair<sc::CellStoreType::iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it == maCells.end())
        return nullptr;

    if (it->type != sc::element_type_numeric)
        return nullptr;

    return &sc::numeric_block::at(*it->data, aPos.second);
}

void ScColumn::GetInputString( SCROW nRow, OUString& rString ) const
{
    ScRefCellValue aCell = GetCellValue(nRow);
    sal_uLong nFormat = GetNumberFormat(pDocument->GetNonThreadedContext(), nRow);
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
        return nullptr;

    if (it->type != sc::element_type_edittext)
        return nullptr;

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
    aFunc = std::for_each(maCells.begin(), maCells.end(), aFunc);
    return aFunc.getCount();
}

FormulaError ScColumn::GetErrCode( SCROW nRow ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        return FormulaError::NONE;

    if (it->type != sc::element_type_formula)
        return FormulaError::NONE;

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
    rtl_TextEncoding meCharSet;
    bool mbOctetEncoding;

    void processCell(size_t nRow, ScRefCellValue& rCell)
    {
        Color* pColor;
        OUString aString;
        sal_uInt32 nFormat = static_cast<const SfxUInt32Item&>(mrColumn.GetAttr(nRow, ATTR_VALUE_FORMAT)).GetValue();
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
    MaxStringLenHandler(const ScColumn& rColumn, rtl_TextEncoding eCharSet) :
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

    void operator() (size_t nRow, const svl::SharedString& rStr)
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

sal_Int32 ScColumn::GetMaxStringLen( SCROW nRowStart, SCROW nRowEnd, rtl_TextEncoding eCharSet ) const
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
    sal_uInt16 mnMaxGeneralPrecision;
    bool mbHaveSigned;

    void processCell(size_t nRow, ScRefCellValue& rCell)
    {
        sal_uInt16 nCellPrecision = mnMaxGeneralPrecision;
        if (rCell.meType == CELLTYPE_FORMULA)
        {
            if (!rCell.mpFormula->IsValue())
                return;

            // Limit unformatted formula cell precision to precision
            // encountered so far, if any, otherwise we'd end up with 15 just
            // because of =1/3 ...  If no precision yet then arbitrarily limit
            // to a maximum of 4 unless a maximum general precision is set.
            if (mnPrecision)
                nCellPrecision = mnPrecision;
            else
                nCellPrecision = (mnMaxGeneralPrecision >= 15) ? 4 : mnMaxGeneralPrecision;
        }

        double fVal = rCell.getValue();
        if (!mbHaveSigned && fVal < 0.0)
            mbHaveSigned = true;

        OUString aString;
        OUString aSep;
        sal_uInt16 nPrec;
        sal_uInt32 nFormat = static_cast<const SfxUInt32Item&>(
                mrColumn.GetAttr(nRow, ATTR_VALUE_FORMAT)).GetValue();
        if (nFormat % SV_COUNTRY_LANGUAGE_OFFSET)
        {
            aSep = mpFormatter->GetFormatDecimalSep(nFormat);
            ScCellFormat::GetInputString(rCell, nFormat, aString, *mpFormatter, &mrColumn.GetDoc());
            const SvNumberformat* pEntry = mpFormatter->GetEntry(nFormat);
            if (pEntry)
            {
                bool bThousand, bNegRed;
                sal_uInt16 nLeading;
                pEntry->GetFormatSpecialInfo(bThousand, bNegRed, nPrec, nLeading);
            }
            else
                nPrec = mpFormatter->GetFormatPrecision(nFormat);
        }
        else
        {
            if (mnPrecision >= mnMaxGeneralPrecision)
                return;     // early bail out for nothing changes here

            if (!fVal)
            {
                // 0 doesn't change precision, but set a maximum length if none yet.
                if (!mnMaxLen)
                    mnMaxLen = 1;
                return;
            }

            // Simple number string with at most 15 decimals and trailing
            // decimal zeros eliminated.
            aSep = ".";
            aString = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_F, nCellPrecision, '.', true);
            nPrec = SvNumberFormatter::UNLIMITED_PRECISION;
        }

        sal_Int32 nLen = aString.getLength();
        if (nLen <= 0)
            // Ignore empty string.
            return;

        if (nPrec == SvNumberFormatter::UNLIMITED_PRECISION && mnPrecision < mnMaxGeneralPrecision)
        {
            if (nFormat % SV_COUNTRY_LANGUAGE_OFFSET)
            {
                // For some reason we couldn't obtain a precision from the
                // format, retry with simple number string.
                aSep = ".";
                aString = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_F, nCellPrecision, '.', true);
                nLen = aString.getLength();
            }
            sal_Int32 nSep = aString.indexOf( aSep);
            if (nSep != -1)
                nPrec = aString.getLength() - nSep - 1;

        }

        if (nPrec != SvNumberFormatter::UNLIMITED_PRECISION && nPrec > mnPrecision)
            mnPrecision = nPrec;

        if (mnPrecision)
        {   // less than mnPrecision in string => widen it
            // more => shorten it
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

        // Enlarge for sign if necessary. Bear in mind that
        // GetMaxNumberStringLen() is for determining dBase decimal field width
        // and precision where the overall field width must include the sign.
        // Fitting -1 into "#.##" (width 4, 2 decimals) does not work.
        if (mbHaveSigned && fVal >= 0.0)
            ++nLen;

        if (mnMaxLen < nLen)
            mnMaxLen = nLen;
    }

public:
    MaxNumStringLenHandler(const ScColumn& rColumn, sal_uInt16 nMaxGeneralPrecision) :
        mrColumn(rColumn), mpFormatter(rColumn.GetDoc().GetFormatTable()),
        mnMaxLen(0), mnPrecision(0), mnMaxGeneralPrecision(nMaxGeneralPrecision),
        mbHaveSigned(false)
    {
        // Limit the decimals passed to doubleToUString().
        // Also, the dBaseIII maximum precision is 15.
        if (mnMaxGeneralPrecision > 15)
            mnMaxGeneralPrecision = 15;
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

sal_Int32 ScColumn::GetMaxNumberStringLen(
    sal_uInt16& nPrecision, SCROW nRowStart, SCROW nRowEnd ) const
{
    sal_uInt16 nMaxGeneralPrecision = pDocument->GetDocOptions().GetStdPrecision();
    MaxNumStringLenHandler aFunc(*this, nMaxGeneralPrecision);
    sc::ParseFormulaNumeric(maCells.begin(), maCells, nRowStart, nRowEnd, aFunc);
    nPrecision = aFunc.getPrecision();
    return aFunc.getMaxLen();
}

namespace {

class GroupFormulaCells
{
    std::vector<ScAddress>* mpGroupPos;

public:
    explicit GroupFormulaCells(std::vector<ScAddress>* pGroupPos)
        : mpGroupPos(pGroupPos) {}

    void operator() (sc::CellStoreType::value_type& node)
    {
        if (node.type != sc::element_type_formula)
            // We are only interested in formula cells.
            return;

        size_t nRow = node.position; // start row position.

        sc::formula_block::iterator it = sc::formula_block::begin(*node.data);
        sc::formula_block::iterator itEnd = sc::formula_block::end(*node.data);

        // This block should never be empty.

        ScFormulaCell* pPrev = *it;
        ScFormulaCellGroupRef xPrevGrp = pPrev->GetCellGroup();
        if (xPrevGrp)
        {
            // Move to the cell after the last cell of the current group.
            std::advance(it, xPrevGrp->mnLength);
            nRow += xPrevGrp->mnLength;
        }
        else
        {
            ++it;
            ++nRow;
        }

        ScFormulaCell* pCur = nullptr;
        ScFormulaCellGroupRef xCurGrp;
        for (; it != itEnd; pPrev = pCur, xPrevGrp = xCurGrp)
        {
            pCur = *it;
            xCurGrp = pCur->GetCellGroup();

            ScFormulaCell::CompareState eCompState = pPrev->CompareByTokenArray(*pCur);
            if (eCompState == ScFormulaCell::NotEqual)
            {
                // different formula tokens.
                if (xCurGrp)
                {
                    // Move to the cell after the last cell of the current group.
                    std::advance(it, xCurGrp->mnLength);
                    nRow += xCurGrp->mnLength;
                }
                else
                {
                    ++it;
                    ++nRow;
                }

                continue;
            }

            // Formula tokens equal those of the previous formula cell or cell group.
            if (xPrevGrp)
            {
                // Previous cell is a group.
                if (xCurGrp)
                {
                    // The current cell is a group.  Merge these two groups.
                    xPrevGrp->mnLength += xCurGrp->mnLength;
                    pCur->SetCellGroup(xPrevGrp);
                    sc::formula_block::iterator itGrpEnd = it;
                    std::advance(itGrpEnd, xCurGrp->mnLength);
                    for (++it; it != itGrpEnd; ++it)
                    {
                        ScFormulaCell* pCell = *it;
                        pCell->SetCellGroup(xPrevGrp);
                    }
                    nRow += xCurGrp->mnLength;
                }
                else
                {
                    // Add this cell to the previous group.
                    pCur->SetCellGroup(xPrevGrp);
                    ++xPrevGrp->mnLength;
                    ++nRow;
                    ++it;
                }

            }
            else if (xCurGrp)
            {
                // Previous cell is a regular cell and current cell is a group.
                nRow += xCurGrp->mnLength;
                std::advance(it, xCurGrp->mnLength);
                pPrev->SetCellGroup(xCurGrp);
                xCurGrp->mpTopCell = pPrev;
                ++xCurGrp->mnLength;
                xPrevGrp = xCurGrp;
            }
            else
            {
                // Both previous and current cells are regular cells.
                assert(pPrev->aPos.Row() == (SCROW)(nRow - 1));
                xPrevGrp = pPrev->CreateCellGroup(2, eCompState == ScFormulaCell::EqualInvariant);
                pCur->SetCellGroup(xPrevGrp);
                ++nRow;
                ++it;
            }

            if (mpGroupPos)
                mpGroupPos->push_back(pCur->aPos);

            pCur = pPrev;
            xCurGrp = xPrevGrp;
        }
    }
};

}

void ScColumn::RegroupFormulaCells( std::vector<ScAddress>* pGroupPos )
{
    // re-build formula groups.
    std::for_each(maCells.begin(), maCells.end(), GroupFormulaCells(pGroupPos));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
