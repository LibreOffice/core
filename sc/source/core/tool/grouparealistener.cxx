/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <grouparealistener.hxx>
#include <brdcst.hxx>
#include <formulacell.hxx>
#include <bulkdatahint.hxx>
#include <columnspanset.hxx>
#include <column.hxx>
#include <listenerquery.hxx>
#include <listenerqueryids.hxx>

namespace sc {

namespace {

class Notifier : std::unary_function<ScFormulaCell*, void>
{
    const SfxHint& mrHint;
public:
    Notifier( const SfxHint& rHint ) : mrHint(rHint) {}

    void operator() ( ScFormulaCell* pCell )
    {
        pCell->Notify(mrHint);
    }
};

class CollectCellAction : public sc::ColumnSpanSet::ColumnAction
{
    const FormulaGroupAreaListener& mrAreaListener;
    ScAddress maPos;
    std::vector<ScFormulaCell*> maCells;

public:
    CollectCellAction( const FormulaGroupAreaListener& rAreaListener ) :
        mrAreaListener(rAreaListener) {}

    virtual void startColumn( ScColumn* pCol ) SAL_OVERRIDE
    {
        maPos.SetTab(pCol->GetTab());
        maPos.SetCol(pCol->GetCol());
    }

    virtual void execute( SCROW nRow1, SCROW nRow2, bool bVal ) SAL_OVERRIDE
    {
        if (!bVal)
            return;

        mrAreaListener.collectFormulaCells(maPos.Tab(), maPos.Col(), nRow1, nRow2, maCells);
    };

    void swapCells( std::vector<ScFormulaCell*>& rCells )
    {
        // Remove duplicate before the swap.
        std::sort(maCells.begin(), maCells.end());
        std::vector<ScFormulaCell*>::iterator it = std::unique(maCells.begin(), maCells.end());
        maCells.erase(it, maCells.end());

        rCells.swap(maCells);
    }
};

}

FormulaGroupAreaListener::FormulaGroupAreaListener(
    const ScRange& rRange, ScFormulaCell** ppTopCell, SCROW nGroupLen, bool bStartFixed, bool bEndFixed ) :
    maRange(rRange),
    mppTopCell(ppTopCell),
    mnGroupLen(nGroupLen),
    mbStartFixed(bStartFixed),
    mbEndFixed(bEndFixed)
{
    assert(mppTopCell); // This can't be NULL.
}

ScRange FormulaGroupAreaListener::getListeningRange() const
{
    ScRange aRet = maRange;
    if (!mbEndFixed)
        aRet.aEnd.IncRow(mnGroupLen-1);
    return aRet;
}

void FormulaGroupAreaListener::Notify( const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if (!pSimpleHint)
        return;

    switch (pSimpleHint->GetId())
    {
        case SC_HINT_DATACHANGED:
            notifyCellChange(rHint, static_cast<const ScHint*>(pSimpleHint)->GetAddress());
        break;
        case SC_HINT_BULK_DATACHANGED:
        {
            const BulkDataHint& rBulkHint = static_cast<const BulkDataHint&>(*pSimpleHint);
            notifyBulkChange(rBulkHint);
        }
        break;
        default:
            ;
    }
}

void FormulaGroupAreaListener::Query( QueryBase& rQuery ) const
{
    switch (rQuery.getId())
    {
        case SC_LISTENER_QUERY_FORMULA_GROUP_RANGE:
        {
            ScFormulaCell* pTop = *mppTopCell;
            ScRange aRange(pTop->aPos);
            aRange.aEnd.IncRow(mnGroupLen-1);
            QueryRange& rQR = static_cast<QueryRange&>(rQuery);
            rQR.add(aRange);
        }
        break;
        default:
            ;
    }
}

void FormulaGroupAreaListener::notifyBulkChange( const BulkDataHint& rHint )
{
    const ColumnSpanSet* pSpans = rHint.getSpans();
    if (!pSpans)
        return;

    ScDocument& rDoc = const_cast<BulkDataHint&>(rHint).getDoc();

    CollectCellAction aAction(*this);
    pSpans->executeColumnAction(rDoc, aAction);

    std::vector<ScFormulaCell*> aCells;
    aAction.swapCells(aCells);
    ScHint aHint(SC_HINT_DATACHANGED, ScAddress());
    std::for_each(aCells.begin(), aCells.end(), Notifier(aHint));
}

void FormulaGroupAreaListener::collectFormulaCells(
    SCTAB nTab, SCCOL nCol, SCROW nRow1, SCROW nRow2, std::vector<ScFormulaCell*>& rCells ) const
{
    PutInOrder(nRow1, nRow2);

    if (nTab < maRange.aStart.Tab() || maRange.aEnd.Tab() < nTab)
        // Wrong sheet.
        return;

    if (nCol < maRange.aStart.Col() || maRange.aEnd.Col() < nCol)
        // Outside the column range.
        return;

    collectFormulaCells(nRow1, nRow2, rCells);
}

void FormulaGroupAreaListener::collectFormulaCells(
    SCROW nRow1, SCROW nRow2, std::vector<ScFormulaCell*>& rCells ) const
{
    ScFormulaCell** pp = mppTopCell;
    ScFormulaCell** ppEnd = pp + mnGroupLen;

    if (mbStartFixed)
    {
        if (mbEndFixed)
        {
            // Both top and bottom row positions are absolute.  Use the original range as-is.
            SCROW nRefRow1 = maRange.aStart.Row();
            SCROW nRefRow2 = maRange.aEnd.Row();
            if (nRow2 < nRefRow1 || nRefRow2 < nRow1)
                return;

            for (; pp != ppEnd; ++pp)
                rCells.push_back(*pp);
        }
        else
        {
            // Only the end row is relative.
            SCROW nRefRow1 = maRange.aStart.Row();
            SCROW nRefRow2 = maRange.aEnd.Row();
            SCROW nMaxRefRow = nRefRow2 + mnGroupLen - 1;
            if (nRow2 < nRefRow1 || nMaxRefRow < nRow1)
                return;

            if (nRefRow2 < nRow1)
            {
                // Skip ahead to the first hit.
                SCROW nSkip = nRow1 - nRefRow2;
                pp += nSkip;
                nRefRow2 += nSkip;
            }

            assert(nRow1 <= nRefRow2);

            // Notify the first hit cell and all subsequent ones.
            for (; pp != ppEnd; ++pp)
                rCells.push_back(*pp);
        }
    }
    else if (mbEndFixed)
    {
        // Only the start row is relative.
        SCROW nRefRow1 = maRange.aStart.Row();
        SCROW nRefRow2 = maRange.aEnd.Row();

        if (nRow2 < nRefRow1 || nRefRow2 < nRow1)
            return;

        for (; pp != ppEnd && nRefRow1 <= nRefRow2; ++pp, ++nRefRow1)
            rCells.push_back(*pp);
    }
    else
    {
        // Both top and bottom row positions are relative.
        SCROW nRefRow1 = maRange.aStart.Row();
        SCROW nRefRow2 = maRange.aEnd.Row();
        SCROW nMaxRefRow = nRefRow2 + mnGroupLen - 1;
        if (nMaxRefRow < nRow1)
            return;

        if (nRefRow2 < nRow1)
        {
            // The ref row range is above the changed row span.  Skip ahead.
            SCROW nSkip = nRow1 - nRefRow2;
            pp += nSkip;
            nRefRow1 += nSkip;
            nRefRow2 += nSkip;
        }

        // At this point the initial ref row range should be overlapping the
        // dirty cell range.
        assert(nRow1 <= nRefRow2);

        // Keep sliding down until the top ref row position is below the
        // bottom row of the dirty cell range.
        for (; pp != ppEnd && nRefRow1 <= nRow2; ++pp, ++nRefRow1, ++nRefRow2)
            rCells.push_back(*pp);
    }
}

ScAddress FormulaGroupAreaListener::getTopCellPos() const
{
    const ScFormulaCell& rFC = **mppTopCell;
    return rFC.aPos;
}

const ScRange& FormulaGroupAreaListener::getRange() const
{
    return maRange;
}

SCROW FormulaGroupAreaListener::getGroupLength() const
{
    return mnGroupLen;
}

bool FormulaGroupAreaListener::isStartFixed() const
{
    return mbStartFixed;
}

bool FormulaGroupAreaListener::isEndFixed() const
{
    return mbEndFixed;
}

void FormulaGroupAreaListener::notifyCellChange( const SfxHint& rHint, const ScAddress& rPos )
{
    // Determine which formula cells within the group need to be notified of this change.
    std::vector<ScFormulaCell*> aCells;
    collectFormulaCells(rPos.Tab(), rPos.Col(), rPos.Row(), rPos.Row(), aCells);
    std::for_each(aCells.begin(), aCells.end(), Notifier(rHint));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
