/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <document.hxx>
#include <clipcontext.hxx>
#include <formulacell.hxx>
#include <clipparam.hxx>
#include <table.hxx>
#include <tokenarray.hxx>
#include <editutil.hxx>
#include <listenercontext.hxx>
#include <tokenstringcontext.hxx>
#include <poolhelp.hxx>
#include <bcaslot.hxx>
#include <cellvalues.hxx>
#include <docpool.hxx>

#include "dociter.hxx"
#include "patattr.hxx"
#include <svl/whiter.hxx>
#include <editeng/colritem.hxx>
#include "scitems.hxx"

// Add totally brand-new methods to this source file.

bool ScDocument::IsMerged( const ScAddress& rPos ) const
{
    const ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return false;

    return pTab->IsMerged(rPos.Col(), rPos.Row());
}

void ScDocument::DeleteBeforeCopyFromClip(
    sc::CopyFromClipContext& rCxt, const ScMarkData& rMark, sc::ColumnSpanSet& rBroadcastSpans )
{
    SCTAB nClipTab = 0;
    const TableContainer& rClipTabs = rCxt.getClipDoc()->maTabs;
    SCTAB nClipTabCount = rClipTabs.size();

    for (SCTAB nTab = rCxt.getTabStart(); nTab <= rCxt.getTabEnd(); ++nTab)
    {
        ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        if (!rMark.GetTableSelect(nTab))
            continue;

        while (!rClipTabs[nClipTab])
            nClipTab = (nClipTab+1) % nClipTabCount;

        pTab->DeleteBeforeCopyFromClip(rCxt, *rClipTabs[nClipTab], rBroadcastSpans);

        nClipTab = (nClipTab+1) % nClipTabCount;
    }
}

bool ScDocument::CopyOneCellFromClip(
    sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    ScDocument* pClipDoc = rCxt.getClipDoc();
    if (pClipDoc->GetClipParam().mbCutMode)
        // We don't handle cut and paste or moving of cells here.
        return false;

    ScRange aClipRange = pClipDoc->GetClipParam().getWholeRange();
    if (aClipRange.aStart.Row() != aClipRange.aEnd.Row())
        // The source is not really a single row. Bail out.
        return false;

    SCCOL nSrcColSize = aClipRange.aEnd.Col() - aClipRange.aStart.Col() + 1;
    SCCOL nDestColSize = nCol2 - nCol1 + 1;
    if (nDestColSize < nSrcColSize)
        return false;

    ScAddress aSrcPos = aClipRange.aStart;

    for (SCCOL nCol = aClipRange.aStart.Col(); nCol <= aClipRange.aEnd.Col(); ++nCol)
    {
        ScAddress aTestPos = aSrcPos;
        aTestPos.SetCol(nCol);
        if (pClipDoc->IsMerged(aTestPos))
            // We don't handle merged source cell for this.
            return false;
    }

    ScTable* pSrcTab = pClipDoc->FetchTable(aSrcPos.Tab());
    if (!pSrcTab)
        return false;

    rCxt.setSingleCellColumnSize(nSrcColSize);

    for (SCCOL nColOffset = 0; nColOffset < nSrcColSize; ++nColOffset, aSrcPos.IncCol())
    {
        const ScPatternAttr* pAttr = pClipDoc->GetPattern(aSrcPos);
        rCxt.setSingleCellPattern(nColOffset, pAttr);

        if ((rCxt.getInsertFlag() & (InsertDeleteFlags::NOTE | InsertDeleteFlags::ADDNOTES)) != InsertDeleteFlags::NONE)
            rCxt.setSingleCellNote(nColOffset, pClipDoc->GetNote(aSrcPos));

        ScColumn& rSrcCol = pSrcTab->aCol[aSrcPos.Col()];
        // Determine the script type of the copied single cell.
        rSrcCol.UpdateScriptTypes(aSrcPos.Row(), aSrcPos.Row());
        rCxt.setSingleCell(aSrcPos, rSrcCol);
    }

    // All good. Proceed with the pasting.

    SCTAB nTabEnd = rCxt.getTabEnd();
    for (SCTAB i = rCxt.getTabStart(); i <= nTabEnd && i < static_cast<SCTAB>(maTabs.size()); ++i)
    {
        maTabs[i]->CopyOneCellFromClip(rCxt, nCol1, nRow1, nCol2, nRow2);
        if (rCxt.getInsertFlag() & InsertDeleteFlags::ATTRIB)
            for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
            {
                maTabs[i]->CopyConditionalFormat(nCol1, nRow, nCol2, nRow, nCol1 - aClipRange.aStart.Col(),
                        nRow - aClipRange.aStart.Row(), pSrcTab);
            }
    }

    return true;
}

void ScDocument::SetValues( const ScAddress& rPos, const std::vector<double>& rVals )
{
    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->SetValues(rPos.Col(), rPos.Row(), rVals);
}

void ScDocument::TransferCellValuesTo( const ScAddress& rTopPos, size_t nLen, sc::CellValues& rDest )
{
    ScTable* pTab = FetchTable(rTopPos.Tab());
    if (!pTab)
        return;

    pTab->TransferCellValuesTo(rTopPos.Col(), rTopPos.Row(), nLen, rDest);
}

void ScDocument::CopyCellValuesFrom( const ScAddress& rTopPos, const sc::CellValues& rSrc )
{
    ScTable* pTab = FetchTable(rTopPos.Tab());
    if (!pTab)
        return;

    pTab->CopyCellValuesFrom(rTopPos.Col(), rTopPos.Row(), rSrc);
}

std::set<Color> ScDocument::GetDocColors()
{
    std::set<Color> aDocColors;
    ScDocumentPool *pPool = GetPool();
    const sal_uInt16 pAttribs[] = {ATTR_BACKGROUND, ATTR_FONT_COLOR};
    for (size_t i=0; i<SAL_N_ELEMENTS( pAttribs ); i++)
    {
        const sal_uInt16 nAttrib = pAttribs[i];
        const sal_uInt32 nCount = pPool->GetItemCount2(nAttrib);
        for (sal_uInt32 j=0; j<nCount; j++)
        {
            const SvxColorItem *pItem = static_cast<const SvxColorItem*>(pPool->GetItem2(nAttrib, j));
            if (pItem == nullptr)
                continue;
            Color aColor( pItem->GetValue() );
            if (COL_AUTO != aColor.GetColor())
                aDocColors.insert(aColor);
        }
    }
    return aDocColors;
}

void ScDocument::SetCalcConfig( const ScCalcConfig& rConfig )
{
    maCalcConfig = rConfig;
}

void ScDocument::ConvertFormulaToValue( const ScRange& rRange, sc::TableValues* pUndo )
{
    sc::EndListeningContext aCxt(*this);

    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
    {
        ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        pTab->ConvertFormulaToValue(
            aCxt, rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row(),
            pUndo);
    }

    aCxt.purgeEmptyBroadcasters();
}

void ScDocument::SwapNonEmpty( sc::TableValues& rValues )
{
    const ScRange& rRange = rValues.getRange();
    if (!rRange.IsValid())
        return;

    std::shared_ptr<sc::ColumnBlockPositionSet> pPosSet(new sc::ColumnBlockPositionSet(*this));
    sc::StartListeningContext aStartCxt(*this, pPosSet);
    sc::EndListeningContext aEndCxt(*this, pPosSet);

    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
    {
        ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        pTab->SwapNonEmpty(rValues, aStartCxt, aEndCxt);
    }

    aEndCxt.purgeEmptyBroadcasters();
}

void ScDocument::PreprocessRangeNameUpdate()
{
    sc::EndListeningContext aEndListenCxt(*this);
    sc::CompileFormulaContext aCompileCxt(this);

    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* p = *it;
        p->PreprocessRangeNameUpdate(aEndListenCxt, aCompileCxt);
    }
}

void ScDocument::PreprocessDBDataUpdate()
{
    sc::EndListeningContext aEndListenCxt(*this);
    sc::CompileFormulaContext aCompileCxt(this);

    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* p = *it;
        p->PreprocessDBDataUpdate(aEndListenCxt, aCompileCxt);
    }
}

void ScDocument::CompileHybridFormula()
{
    sc::StartListeningContext aStartListenCxt(*this);
    sc::CompileFormulaContext aCompileCxt(this);
    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* p = *it;
        p->CompileHybridFormula(aStartListenCxt, aCompileCxt);
    }
}

void ScDocument::SharePooledResources( ScDocument* pSrcDoc )
{
    xPoolHelper = pSrcDoc->xPoolHelper;
    mpCellStringPool = pSrcDoc->mpCellStringPool;
}

void ScDocument::UpdateScriptTypes( const ScAddress& rPos, SCCOL nColSize, SCROW nRowSize )
{
    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->UpdateScriptTypes(rPos.Col(), rPos.Row(), rPos.Col()+nColSize-1, rPos.Row()+nRowSize-1);
}

bool ScDocument::HasUniformRowHeight( SCTAB nTab, SCROW nRow1, SCROW nRow2 ) const
{
    const ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return false;

    return pTab->HasUniformRowHeight(nRow1, nRow2);
}

void ScDocument::UnshareFormulaCells( SCTAB nTab, SCCOL nCol, std::vector<SCROW>& rRows )
{
    ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return;

    pTab->UnshareFormulaCells(nCol, rRows);
}

void ScDocument::RegroupFormulaCells( SCTAB nTab, SCCOL nCol )
{
    ScTable* pTab = FetchTable(nTab);
    if (!pTab)
        return;

    pTab->RegroupFormulaCells(nCol);
}

void ScDocument::CollectAllAreaListeners(
    std::vector<SvtListener*>& rListener, const ScRange& rRange, sc::AreaOverlapType eType )
{
    if (!pBASM)
        return;

    std::vector<sc::AreaListener> aAL = pBASM->GetAllListeners(rRange, eType);
    std::vector<sc::AreaListener>::iterator it = aAL.begin(), itEnd = aAL.end();
    for (; it != itEnd; ++it)
        rListener.push_back(it->mpListener);
}

bool ScDocument::HasFormulaCell( const ScRange& rRange ) const
{
    if (!rRange.IsValid())
        return false;

    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
    {
        const ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        if (pTab->HasFormulaCell(rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row()))
            return true;
    }

    return false;
}

void ScDocument::EndListeningIntersectedGroup(
    sc::EndListeningContext& rCxt, const ScAddress& rPos, std::vector<ScAddress>* pGroupPos )
{
    ScTable* pTab = FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->EndListeningIntersectedGroup(rCxt, rPos.Col(), rPos.Row(), pGroupPos);
}

void ScDocument::EndListeningIntersectedGroups(
    sc::EndListeningContext& rCxt, const ScRange& rRange, std::vector<ScAddress>* pGroupPos )
{
    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
    {
        ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        pTab->EndListeningIntersectedGroups(
            rCxt, rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row(),
            pGroupPos);
    }
}

void ScDocument::EndListeningGroups( const std::vector<ScAddress>& rPosArray )
{
    sc::EndListeningContext aCxt(*this);
    std::vector<ScAddress>::const_iterator it = rPosArray.begin(), itEnd = rPosArray.end();
    for (; it != itEnd; ++it)
    {
        const ScAddress& rPos = *it;
        ScTable* pTab = FetchTable(rPos.Tab());
        if (!pTab)
            return;

        pTab->EndListeningGroup(aCxt, rPos.Col(), rPos.Row());
    }

    aCxt.purgeEmptyBroadcasters();
}

void ScDocument::SetNeedsListeningGroups( const std::vector<ScAddress>& rPosArray )
{
    std::vector<ScAddress>::const_iterator it = rPosArray.begin(), itEnd = rPosArray.end();
    for (; it != itEnd; ++it)
    {
        const ScAddress& rPos = *it;
        ScTable* pTab = FetchTable(rPos.Tab());
        if (!pTab)
            return;

        pTab->SetNeedsListeningGroup(rPos.Col(), rPos.Row());
    }
}

namespace {

class StartNeededListenersHandler : std::unary_function<ScTable*, void>
{
    std::shared_ptr<sc::StartListeningContext> mpCxt;
public:
    StartNeededListenersHandler( ScDocument& rDoc ) : mpCxt(new sc::StartListeningContext(rDoc)) {}

    void operator() (ScTable* p)
    {
        if (p)
            p->StartListeners(*mpCxt, false);
    }
};

}

void ScDocument::StartNeededListeners()
{
    std::for_each(maTabs.begin(), maTabs.end(), StartNeededListenersHandler(*this));
}

void ScDocument::StartAllListeners( const ScRange& rRange )
{
    std::shared_ptr<sc::ColumnBlockPositionSet> pPosSet(new sc::ColumnBlockPositionSet(*this));
    sc::StartListeningContext aStartCxt(*this, pPosSet);
    sc::EndListeningContext aEndCxt(*this, pPosSet);

    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
    {
        ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        pTab->StartListeningFormulaCells(
            aStartCxt, aEndCxt,
            rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
