/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sharedformula.hxx>
#include <calcmacros.hxx>
#include <tokenarray.hxx>
#include <listenercontext.hxx>
#include <document.hxx>
#include <grouparealistener.hxx>
#include <refdata.hxx>

namespace sc {

const ScFormulaCell* SharedFormulaUtil::getSharedTopFormulaCell(const CellStoreType::position_type& aPos)
{
    if (aPos.first->type != sc::element_type_formula)
        // Not a formula cell block.
        return nullptr;

    sc::formula_block::iterator it = sc::formula_block::begin(*aPos.first->data);
    std::advance(it, aPos.second);
    const ScFormulaCell* pCell = *it;
    if (!pCell->IsShared())
        // Not a shared formula.
        return nullptr;

    return pCell->GetCellGroup()->mpTopCell;
}

bool SharedFormulaUtil::splitFormulaCellGroup(const CellStoreType::position_type& aPos, sc::EndListeningContext* pCxt)
{
    SCROW nRow = aPos.first->position + aPos.second;

    if (aPos.first->type != sc::element_type_formula)
        // Not a formula cell block.
        return false;

    if (aPos.second == 0)
        // Split position coincides with the block border. Nothing to do.
        return false;

    sc::formula_block::iterator it = sc::formula_block::begin(*aPos.first->data);
    std::advance(it, aPos.second);
    ScFormulaCell& rTop = **it;
    if (!rTop.IsShared())
        // Not a shared formula.
        return false;

    if (nRow == rTop.GetSharedTopRow())
        // Already the top cell of a shared group.
        return false;

    ScFormulaCellGroupRef xGroup = rTop.GetCellGroup();

    SCROW nLength2 = xGroup->mpTopCell->aPos.Row() + xGroup->mnLength - nRow;
    ScFormulaCellGroupRef xGroup2;
    if (nLength2 > 1)
    {
        xGroup2.reset(new ScFormulaCellGroup);
        xGroup2->mbInvariant = xGroup->mbInvariant;
        xGroup2->mpTopCell = &rTop;
        xGroup2->mnLength = nLength2;
        xGroup2->mpCode = xGroup->mpCode->Clone();
    }

    xGroup->mnLength = nRow - xGroup->mpTopCell->aPos.Row();
    ScFormulaCell& rPrevTop = *sc::formula_block::at(*aPos.first->data, aPos.second - xGroup->mnLength);

#if USE_FORMULA_GROUP_LISTENER
    // At least group area listeners will have to be adapted. As long as
    // there's no update mechanism and no separated handling of group area and
    // other listeners, all listeners of this group's top cell are to be reset.
    if (nLength2)
    {
        // If a context exists it has to be used to not interfere with
        // ScColumn::maBroadcasters iterators, which the EndListeningTo()
        // without context would do when removing a broadcaster that had its
        // last listener removed.
        if (pCxt)
            rPrevTop.EndListeningTo(*pCxt);
        else
            rPrevTop.EndListeningTo( rPrevTop.GetDocument(), nullptr, ScAddress( ScAddress::UNINITIALIZED));
        rPrevTop.SetNeedsListening(true);

        // The new group or remaining single cell needs a new listening.
        rTop.SetNeedsListening(true);
    }
#endif

    if (xGroup->mnLength == 1)
    {
        // The top group consists of only one cell. Ungroup this.
        ScFormulaCellGroupRef xNone;
        rPrevTop.SetCellGroup(xNone);
    }

    // Apply the lower group object to the lower cells.
#if DEBUG_COLUMN_STORAGE
    if (xGroup2->mpTopCell->aPos.Row() + size_t(xGroup2->mnLength) > aPos.first->position + aPos.first->size)
    {
        cerr << "ScColumn::SplitFormulaCellGroup: Shared formula region goes beyond the formula block. Not good." << endl;
        cerr.flush();
        abort();
    }
#endif
    sc::formula_block::iterator itEnd = it;
    std::advance(itEnd, nLength2);
    for (; it != itEnd; ++it)
    {
        ScFormulaCell& rCell = **it;
        rCell.SetCellGroup(xGroup2);
    }

    return true;
}

bool SharedFormulaUtil::splitFormulaCellGroups(CellStoreType& rCells, std::vector<SCROW>& rBounds)
{
    if (rBounds.empty())
        return false;

    // Sort and remove duplicates.
    std::sort(rBounds.begin(), rBounds.end());
    std::vector<SCROW>::iterator it = std::unique(rBounds.begin(), rBounds.end());
    rBounds.erase(it, rBounds.end());

    it = rBounds.begin();
    SCROW nRow = *it;
    CellStoreType::position_type aPos = rCells.position(nRow);
    if (aPos.first == rCells.end())
        return false;

    bool bSplit = splitFormulaCellGroup(aPos, nullptr);
    std::vector<SCROW>::iterator itEnd = rBounds.end();
    for (++it; it != itEnd; ++it)
    {
        nRow = *it;
        if (ValidRow(nRow))
        {
            aPos = rCells.position(aPos.first, nRow);
            if (aPos.first == rCells.end())
                return bSplit;
            bSplit |= splitFormulaCellGroup(aPos, nullptr);
        }
    }
    return bSplit;
}

bool SharedFormulaUtil::joinFormulaCells(
    const CellStoreType::position_type& rPos, ScFormulaCell& rCell1, ScFormulaCell& rCell2 )
{
    if( rCell1.GetDocument()->IsDelayedFormulaGrouping())
    {
        rCell1.GetDocument()->AddDelayedFormulaGroupingCell( &rCell1 );
        rCell1.GetDocument()->AddDelayedFormulaGroupingCell( &rCell2 );
        return false;
    }

    ScFormulaCell::CompareState eState = rCell1.CompareByTokenArray(rCell2);
    if (eState == ScFormulaCell::NotEqual)
        return false;

    // Formula tokens equal those of the previous formula cell.
    ScFormulaCellGroupRef xGroup1 = rCell1.GetCellGroup();
    ScFormulaCellGroupRef xGroup2 = rCell2.GetCellGroup();
    if (xGroup1)
    {
        if (xGroup2)
        {
            // Both cell 1 and cell 2 are shared. Merge them together.
            if (xGroup1.get() == xGroup2.get())
                // They belong to the same group.
                return false;

            // Set the group object from cell 1 to all cells in group 2.
            xGroup1->mnLength += xGroup2->mnLength;
            size_t nOffset = rPos.second + 1; // position of cell 2
            for (size_t i = 0, n = xGroup2->mnLength; i < n; ++i)
            {
                ScFormulaCell& rCell = *sc::formula_block::at(*rPos.first->data, nOffset+i);
                rCell.SetCellGroup(xGroup1);
            }
        }
        else
        {
            // cell 1 is shared but cell 2 is not.
            rCell2.SetCellGroup(xGroup1);
            ++xGroup1->mnLength;
        }
    }
    else
    {
        if (xGroup2)
        {
            // cell 1 is not shared, but cell 2 is already shared.
            rCell1.SetCellGroup(xGroup2);
            xGroup2->mpTopCell = &rCell1;
            ++xGroup2->mnLength;
        }
        else
        {
            // neither cells are shared.
            assert(rCell1.aPos.Row() == static_cast<SCROW>(rPos.first->position + rPos.second));
            xGroup1 = rCell1.CreateCellGroup(2, eState == ScFormulaCell::EqualInvariant);
            rCell2.SetCellGroup(xGroup1);
        }
    }

    return true;
}

bool SharedFormulaUtil::joinFormulaCellAbove( const CellStoreType::position_type& aPos )
{
    if (aPos.first->type != sc::element_type_formula)
        // This is not a formula cell.
        return false;

    if (aPos.second == 0)
        // This cell is already the top cell in a formula block; the previous
        // cell is not a formula cell.
        return false;

    ScFormulaCell& rPrev = *sc::formula_block::at(*aPos.first->data, aPos.second-1);
    ScFormulaCell& rCell = *sc::formula_block::at(*aPos.first->data, aPos.second);
    sc::CellStoreType::position_type aPosPrev = aPos;
    --aPosPrev.second;
    return joinFormulaCells(aPosPrev, rPrev, rCell);
}

void SharedFormulaUtil::unshareFormulaCell(const CellStoreType::position_type& aPos, ScFormulaCell& rCell)
{
    if (!rCell.IsShared())
        return;

    ScFormulaCellGroupRef xNone;
    sc::CellStoreType::iterator it = aPos.first;

    // This formula cell is shared. Adjust the shared group.
    if (rCell.aPos.Row() == rCell.GetSharedTopRow())
    {
        // Top of the shared range.
        const ScFormulaCellGroupRef& xGroup = rCell.GetCellGroup();
        if (xGroup->mnLength == 2)
        {
            // Group consists of only two cells. Mark the second one non-shared.
#if DEBUG_COLUMN_STORAGE
            if (aPos.second+1 >= aPos.first->size)
            {
                cerr << "ScColumn::UnshareFormulaCell: There is no next formula cell but there should be!" << endl;
                cerr.flush();
                abort();
            }
#endif
            ScFormulaCell& rNext = *sc::formula_block::at(*it->data, aPos.second+1);
            rNext.SetCellGroup(xNone);
        }
        else
        {
            // Move the top cell to the next formula cell down.
            ScFormulaCell& rNext = *sc::formula_block::at(*it->data, aPos.second+1);
            xGroup->mpTopCell = &rNext;
        }
        --xGroup->mnLength;
    }
    else if (rCell.aPos.Row() == rCell.GetSharedTopRow() + rCell.GetSharedLength() - 1)
    {
        // Bottom of the shared range.
        const ScFormulaCellGroupRef& xGroup = rCell.GetCellGroup();
        if (xGroup->mnLength == 2)
        {
            // Mark the top cell non-shared.
#if DEBUG_COLUMN_STORAGE
            if (aPos.second == 0)
            {
                cerr << "ScColumn::UnshareFormulaCell: There is no previous formula cell but there should be!" << endl;
                cerr.flush();
                abort();
            }
#endif
            ScFormulaCell& rPrev = *sc::formula_block::at(*it->data, aPos.second-1);
            rPrev.SetCellGroup(xNone);
        }
        else
        {
            // Just shorten the shared range length by one.
            --xGroup->mnLength;
        }
    }
    else
    {
        // In the middle of the shared range. Split it into two groups.
        ScFormulaCellGroupRef xGroup = rCell.GetCellGroup();
        SCROW nEndRow = xGroup->mpTopCell->aPos.Row() + xGroup->mnLength - 1;
        xGroup->mnLength = rCell.aPos.Row() - xGroup->mpTopCell->aPos.Row(); // Shorten the top group.
        if (xGroup->mnLength == 1)
        {
            // Make the top cell non-shared.
#if DEBUG_COLUMN_STORAGE
            if (aPos.second == 0)
            {
                cerr << "ScColumn::UnshareFormulaCell: There is no previous formula cell but there should be!" << endl;
                cerr.flush();
                abort();
            }
#endif
            ScFormulaCell& rPrev = *sc::formula_block::at(*it->data, aPos.second-1);
            rPrev.SetCellGroup(xNone);
        }

        SCROW nLength2 = nEndRow - rCell.aPos.Row();
        if (nLength2 >= 2)
        {
            ScFormulaCellGroupRef xGroup2;
            xGroup2.reset(new ScFormulaCellGroup);
            ScFormulaCell& rNext = *sc::formula_block::at(*it->data, aPos.second+1);
            xGroup2->mpTopCell = &rNext;
            xGroup2->mnLength = nLength2;
            xGroup2->mbInvariant = xGroup->mbInvariant;
            xGroup2->mpCode = xGroup->mpCode->Clone();
#if DEBUG_COLUMN_STORAGE
            if (xGroup2->mpTopCell->aPos.Row() + size_t(xGroup2->mnLength) > it->position + it->size)
            {
                cerr << "ScColumn::UnshareFormulaCell: Shared formula region goes beyond the formula block. Not good." << endl;
                cerr.flush();
                abort();
            }
#endif
            sc::formula_block::iterator itCell = sc::formula_block::begin(*it->data);
            std::advance(itCell, aPos.second+1);
            sc::formula_block::iterator itCellEnd = itCell;
            std::advance(itCellEnd, xGroup2->mnLength);
            for (; itCell != itCellEnd; ++itCell)
            {
                ScFormulaCell& rCell2 = **itCell;
                rCell2.SetCellGroup(xGroup2);
            }
        }
        else
        {
            // Make the next cell non-shared.
            sc::formula_block::iterator itCell = sc::formula_block::begin(*it->data);
            std::advance(itCell, aPos.second+1);
            ScFormulaCell& rCell2 = **itCell;
            rCell2.SetCellGroup(xNone);
        }
    }

    rCell.SetCellGroup(xNone);
}

void SharedFormulaUtil::unshareFormulaCells(CellStoreType& rCells, std::vector<SCROW>& rRows)
{
    if (rRows.empty())
        return;

    // Sort and remove duplicates.
    std::sort(rRows.begin(), rRows.end());
    rRows.erase(std::unique(rRows.begin(), rRows.end()), rRows.end());

    // Add next cell positions to the list (to ensure that each position becomes a single cell).
    std::vector<SCROW> aRows2;
    for (const auto& rRow : rRows)
    {
        if (rRow > MAXROW)
            break;

        aRows2.push_back(rRow);

        if (rRow < MAXROW)
            aRows2.push_back(rRow+1);
    }

    // Remove duplicates again (the vector should still be sorted).
    aRows2.erase(std::unique(aRows2.begin(), aRows2.end()), aRows2.end());

    splitFormulaCellGroups(rCells, aRows2);
}

void SharedFormulaUtil::startListeningAsGroup( sc::StartListeningContext& rCxt, ScFormulaCell** ppSharedTop )
{
    ScFormulaCell& rTopCell = **ppSharedTop;
    assert(rTopCell.IsSharedTop());

#if USE_FORMULA_GROUP_LISTENER
    ScDocument& rDoc = rCxt.getDoc();
    rDoc.SetDetectiveDirty(true);

    ScFormulaCellGroupRef xGroup = rTopCell.GetCellGroup();
    const ScTokenArray* pCode = xGroup->mpCode.get();
    assert(pCode == rTopCell.GetCode());
    if (pCode->IsRecalcModeAlways())
    {
        rDoc.StartListeningArea(
            BCA_LISTEN_ALWAYS, false,
            xGroup->getAreaListener(ppSharedTop, BCA_LISTEN_ALWAYS, true, true));
    }

    formula::FormulaToken** p = pCode->GetCode();
    formula::FormulaToken** pEnd = p + pCode->GetCodeLen();
    for (; p != pEnd; ++p)
    {
        const formula::FormulaToken* t = *p;
        switch (t->GetType())
        {
            case formula::svSingleRef:
            {
                const ScSingleRefData* pRef = t->GetSingleRef();
                ScAddress aPos = pRef->toAbs(rTopCell.aPos);
                ScFormulaCell** pp = ppSharedTop;
                ScFormulaCell** ppEnd = ppSharedTop + xGroup->mnLength;
                for (; pp != ppEnd; ++pp)
                {
                    if (!aPos.IsValid())
                        break;

                    rDoc.StartListeningCell(rCxt, aPos, **pp);
                    if (pRef->IsRowRel())
                        aPos.IncRow();
                }
            }
            break;
            case formula::svDoubleRef:
            {
                const ScSingleRefData& rRef1 = *t->GetSingleRef();
                const ScSingleRefData& rRef2 = *t->GetSingleRef2();
                ScAddress aPos1 = rRef1.toAbs(rTopCell.aPos);
                ScAddress aPos2 = rRef2.toAbs(rTopCell.aPos);

                ScRange aOrigRange = ScRange(aPos1, aPos2);
                ScRange aListenedRange = aOrigRange;
                if (rRef2.IsRowRel())
                    aListenedRange.aEnd.IncRow(xGroup->mnLength-1);

                if (aPos1.IsValid() && aPos2.IsValid())
                {
                    rDoc.StartListeningArea(
                        aListenedRange, true,
                        xGroup->getAreaListener(ppSharedTop, aOrigRange, !rRef1.IsRowRel(), !rRef2.IsRowRel()));
                }
            }
            break;
            default:
                ;
        }
    }

    ScFormulaCell** pp = ppSharedTop;
    ScFormulaCell** ppEnd = ppSharedTop + xGroup->mnLength;
    for (; pp != ppEnd; ++pp)
    {
        ScFormulaCell& rCell = **pp;
        rCell.SetNeedsListening(false);
    }

#else
    ScFormulaCell** pp = ppSharedTop;
    ScFormulaCell** ppEnd = ppSharedTop + rTopCell.GetSharedLength();
    for (; pp != ppEnd; ++pp)
    {
        ScFormulaCell& rFC = **pp;
        rFC.StartListeningTo(rCxt);
    }
#endif
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
