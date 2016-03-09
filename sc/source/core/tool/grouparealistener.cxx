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
#include <document.hxx>
#include <table.hxx>

namespace sc {

namespace {

class Notifier : public std::unary_function<ScFormulaCell*, void>
{
    const SfxHint& mrHint;
public:
    explicit Notifier( const SfxHint& rHint ) : mrHint(rHint) {}

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
    explicit CollectCellAction( const FormulaGroupAreaListener& rAreaListener ) :
        mrAreaListener(rAreaListener) {}

    virtual void startColumn( ScColumn* pCol ) override
    {
        maPos.SetTab(pCol->GetTab());
        maPos.SetCol(pCol->GetCol());
    }

    virtual void execute( SCROW nRow1, SCROW nRow2, bool bVal ) override
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

FormulaGroupAreaListener::FormulaGroupAreaListener( const ScRange& rRange, const ScDocument& rDocument,
        const ScAddress& rTopCellPos, SCROW nGroupLen, bool bStartFixed, bool bEndFixed ) :
    maRange(rRange),
    mpColumn(nullptr),
    mnTopCellRow(rTopCellPos.Row()),
    mnGroupLen(nGroupLen),
    mbStartFixed(bStartFixed),
    mbEndFixed(bEndFixed)
{
    const ScTable* pTab = rDocument.FetchTable( rTopCellPos.Tab());
    assert(pTab);
    mpColumn = pTab->FetchColumn( rTopCellPos.Col());
    assert(mpColumn);
    SAL_INFO( "sc.core.grouparealistener",
            "FormulaGroupAreaListener ctor this " << this <<
            " range " << (maRange == BCA_LISTEN_ALWAYS ? "LISTEN-ALWAYS" : maRange.Format(ScRefFlags::VALID)) <<
            " mnTopCellRow " << mnTopCellRow << " length " << mnGroupLen <<
            ", col/tab " << mpColumn->GetCol() << "/" << mpColumn->GetTab());
}

FormulaGroupAreaListener::~FormulaGroupAreaListener()
{
    SAL_INFO( "sc.core.grouparealistener",
            "FormulaGroupAreaListener dtor this " << this);
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
            const ScFormulaCell* pTop = getTopCell();
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
    SAL_INFO( "sc.core.grouparealistener",
            "FormulaGroupAreaListener::collectFormulaCells() this " << this <<
            " range " << (maRange == BCA_LISTEN_ALWAYS ? "LISTEN-ALWAYS" : maRange.Format(ScRefFlags::VALID)) <<
            " mnTopCellRow " << mnTopCellRow << " length " << mnGroupLen <<
            ", col/tab " << mpColumn->GetCol() << "/" << mpColumn->GetTab());

    size_t nBlockSize = 0;
    ScFormulaCell* const * pp = mpColumn->GetFormulaCellBlockAddress( mnTopCellRow, nBlockSize);
    if (!pp)
    {
        SAL_WARN("sc.core", "GetFormulaCellBlockAddress not found");
        return;
    }

    /* FIXME: this is tdf#90717, when deleting a row fixed size area listeners
     * such as BCA_ALWAYS or entire row listeners are (rightly) not destroyed,
     * but mnTopCellRow and mnGroupLen also not updated, which needs fixing.
     * Until then pull things as straight as possible here in such situation
     * and prevent crash. */
    if (!(*pp)->IsSharedTop())
    {
        SCROW nRow = (*pp)->GetSharedTopRow();
        if (nRow < 0)
            SAL_WARN("sc.core", "FormulaGroupAreaListener::collectFormulaCells() no shared top");
        else
        {
            SAL_WARN("sc.core","FormulaGroupAreaListener::collectFormulaCells() syncing mnTopCellRow from " <<
                    mnTopCellRow << " to " << nRow);
            const_cast<FormulaGroupAreaListener*>(this)->mnTopCellRow = nRow;
            pp = mpColumn->GetFormulaCellBlockAddress( mnTopCellRow, nBlockSize);
            if (!pp)
            {
                SAL_WARN("sc.core", "GetFormulaCellBlockAddress not found");
                return;
            }
        }
    }
    SCROW nLen = (*pp)->GetSharedLength();
    if (nLen != mnGroupLen)
    {
        SAL_WARN("sc.core", "FormulaGroupAreaListener::collectFormulaCells() syncing mnGroupLen from " <<
                mnGroupLen << " to " << nLen);
        const_cast<FormulaGroupAreaListener*>(this)->mnGroupLen = nLen;
    }

    /* FIXME: with tdf#89957 it happened that the actual block size in column
     * AP (shifted from AO) of sheet 'w' was smaller than the remembered group
     * length and correct. This is just a very ugly workaround, the real cause
     * is yet unknown, but at least don't crash in such case. The intermediate
     * cause is that not all affected group area listeners are destroyed and
     * newly created, so mpColumn still points to the old column that then has
     * the content of a shifted column. Effectively this workaround has the
     * consequence that the group area listener is fouled up and not all
     * formula cells are notified.. */
    if (nBlockSize < static_cast<size_t>(mnGroupLen))
    {
        SAL_WARN("sc.core","FormulaGroupAreaListener::collectFormulaCells() nBlockSize " <<
                nBlockSize << " < " << mnGroupLen << " mnGroupLen");
        const_cast<FormulaGroupAreaListener*>(this)->mnGroupLen = static_cast<SCROW>(nBlockSize);
    }

    ScFormulaCell* const * ppEnd = pp + mnGroupLen;

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

const ScFormulaCell* FormulaGroupAreaListener::getTopCell() const
{
    size_t nBlockSize = 0;
    const ScFormulaCell* const * pp = mpColumn->GetFormulaCellBlockAddress( mnTopCellRow, nBlockSize);
    SAL_WARN_IF(!pp, "sc", "GetFormulaCellBlockAddress not found");
    return pp ? *pp : nullptr;
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
