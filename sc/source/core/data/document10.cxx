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
#include "refupdatecontext.hxx"
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

    if (pClipDoc->maTabs.size() > 1)
        // Copying from multiple source sheets is not handled here.
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
        maTabs[i]->CopyOneCellFromClip(rCxt, nCol1, nRow1, nCol2, nRow2,  aClipRange.aStart.Row(), pSrcTab);
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
    for (sal_uInt16 nAttrib : pAttribs)
    {
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

void ScDocument::PreprocessAllRangeNamesUpdate( const std::map<OUString, std::unique_ptr<ScRangeName>>& rRangeMap )
{
    // Update all existing names with new names.
    // The prerequisites are that the name dialog preserves ScRangeData index
    // for changes and does not reuse free index slots for new names.
    // ScDocument::SetAllRangeNames() hereafter then will replace the
    // ScRangeName containers of ScRangeData instances with empty
    // ScRangeData::maNewName.
    std::map<OUString, ScRangeName*> aRangeNameMap;
    GetRangeNameMap( aRangeNameMap);
    for (const auto& itTab : aRangeNameMap)
    {
        ScRangeName* pOldRangeNames = itTab.second;
        if (!pOldRangeNames)
            continue;

        const auto& itNewTab( rRangeMap.find( itTab.first));
        if (itNewTab == rRangeMap.end())
            continue;

        const ScRangeName* pNewRangeNames = itNewTab->second.get();
        if (!pNewRangeNames)
            continue;

        for (ScRangeName::iterator it( pOldRangeNames->begin()), itEnd( pOldRangeNames->end());
                it != itEnd; ++it)
        {
            ScRangeData* pOldData = it->second.get();
            if (!pOldData)
                continue;

            const ScRangeData* pNewData = pNewRangeNames->findByIndex( pOldData->GetIndex());
            if (pNewData)
                pOldData->SetNewName( pNewData->GetName());
        }
    }

    sc::EndListeningContext aEndListenCxt(*this);
    sc::CompileFormulaContext aCompileCxt(this);

    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* p = *it;
        p->PreprocessRangeNameUpdate(aEndListenCxt, aCompileCxt);
    }
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

class StartNeededListenersHandler : public std::unary_function<ScTable*, void>
{
    std::shared_ptr<sc::StartListeningContext> mpCxt;
public:
    explicit StartNeededListenersHandler( ScDocument& rDoc ) : mpCxt(new sc::StartListeningContext(rDoc)) {}
    explicit StartNeededListenersHandler( ScDocument& rDoc, const std::shared_ptr<const sc::ColumnSet>& rpColSet ) :
        mpCxt(new sc::StartListeningContext(rDoc))
    {
        mpCxt->setColumnSet( rpColSet);
    }

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

void ScDocument::StartNeededListeners( const std::shared_ptr<const sc::ColumnSet>& rpColSet )
{
    std::for_each(maTabs.begin(), maTabs.end(), StartNeededListenersHandler(*this, rpColSet));
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

void ScDocument::finalizeOutlineImport()
{
    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* p = *it;
        p->finalizeOutlineImport();
    }
}

bool ScDocument::FindRangeNamesReferencingSheet( sc::UpdatedRangeNames& rIndexes,
        SCTAB nTokenTab, const sal_uInt16 nTokenIndex,
        SCTAB nGlobalRefTab, SCTAB nLocalRefTab, SCTAB nOldTokenTab, SCTAB nOldTokenTabReplacement,
        bool bSameDoc, int nRecursion) const
{
    if (nTokenTab < -1)
    {
        SAL_WARN("sc.core", "ScDocument::FindRangeNamesReferencingSheet - nTokenTab < -1 : " <<
                nTokenTab << ", nTokenIndex " << nTokenIndex << " Fix the creator!");
#if OSL_DEBUG_LEVEL > 0
        const ScRangeData* pData = FindRangeNameBySheetAndIndex( nTokenTab, nTokenIndex);
        SAL_WARN_IF( pData, "sc.core", "ScDocument::FindRangeNamesReferencingSheet - named expression is: " << pData->GetName());
#endif
        nTokenTab = -1;
    }
    SCTAB nRefTab = nGlobalRefTab;
    if (nTokenTab == nOldTokenTab)
    {
        nTokenTab = nOldTokenTabReplacement;
        nRefTab = nLocalRefTab;
    }
    else if (nTokenTab == nOldTokenTabReplacement)
    {
        nRefTab = nLocalRefTab;
    }

    if (rIndexes.isNameUpdated( nTokenTab, nTokenIndex))
        return true;

    ScRangeData* pData = FindRangeNameBySheetAndIndex( nTokenTab, nTokenIndex);
    if (!pData)
        return false;

    ScTokenArray* pCode = pData->GetCode();
    if (!pCode)
        return false;

    bool bRef = !bSameDoc;  // include every name used when copying to other doc
    if (nRecursion < 126)   // whatever.. 42*3
    {
        for (const formula::FormulaToken* p = pCode->First(); p; p = pCode->Next())
        {
            if (p->GetOpCode() == ocName)
            {
                bRef |= FindRangeNamesReferencingSheet( rIndexes, p->GetSheet(), p->GetIndex(),
                        nGlobalRefTab, nLocalRefTab, nOldTokenTab, nOldTokenTabReplacement, bSameDoc, nRecursion+1);
            }
        }
    }

    if (!bRef)
    {
        SCTAB nPosTab = pData->GetPos().Tab();
        if (nPosTab == nOldTokenTab)
            nPosTab = nOldTokenTabReplacement;
        bRef = pCode->ReferencesSheet( nRefTab, nPosTab);
    }
    if (bRef)
        rIndexes.setUpdatedName( nTokenTab, nTokenIndex);

    return bRef;
}

namespace {

enum MightReferenceSheet
{
    UNKNOWN,
    NONE,
    CODE,
    NAME
};

MightReferenceSheet mightRangeNameReferenceSheet( ScRangeData* pData, SCTAB nRefTab)
{
    ScTokenArray* pCode = pData->GetCode();
    if (!pCode)
        return MightReferenceSheet::NONE;

    for (const formula::FormulaToken* p = pCode->First(); p; p = pCode->Next())
    {
        if (p->GetOpCode() == ocName)
            return MightReferenceSheet::NAME;
    }

    return pCode->ReferencesSheet( nRefTab, pData->GetPos().Tab()) ?
        MightReferenceSheet::CODE : MightReferenceSheet::NONE;
}

ScRangeData* copyRangeName( const ScRangeData* pOldRangeData, ScDocument& rNewDoc, const ScDocument* pOldDoc,
        const ScAddress& rNewPos, const ScAddress& rOldPos, bool bGlobalNamesToLocal,
        SCTAB nOldSheet, const SCTAB nNewSheet, bool bSameDoc)
{
    ScAddress aRangePos( pOldRangeData->GetPos());
    if (nNewSheet >= 0)
        aRangePos.SetTab( nNewSheet);
    ScRangeData* pRangeData = new ScRangeData(*pOldRangeData, &rNewDoc, &aRangePos);
    pRangeData->SetIndex(0);    // needed for insert to assign a new index
    ScTokenArray* pRangeNameToken = pRangeData->GetCode();
    if (bSameDoc && nNewSheet >= 0)
    {
        if (bGlobalNamesToLocal && nOldSheet < 0)
        {
            nOldSheet = rOldPos.Tab();
            if (rNewPos.Tab() <= nOldSheet)
                // Sheet was inserted before and references already updated.
                ++nOldSheet;
        }
        pRangeNameToken->AdjustSheetLocalNameReferences( nOldSheet, nNewSheet);
    }
    if (!bSameDoc)
    {
        pRangeNameToken->ReadjustAbsolute3DReferences(pOldDoc, &rNewDoc, pRangeData->GetPos(), true);
        pRangeNameToken->AdjustAbsoluteRefs(pOldDoc, rOldPos, rNewPos, true);
    }

    bool bInserted;
    if (nNewSheet < 0)
        bInserted = rNewDoc.GetRangeName()->insert(pRangeData);
    else
        bInserted = rNewDoc.GetRangeName(nNewSheet)->insert(pRangeData);

    return bInserted ? pRangeData : nullptr;
}

struct SheetIndex
{
    SCTAB       mnSheet;
    sal_uInt16  mnIndex;

    SheetIndex( SCTAB nSheet, sal_uInt16 nIndex ) : mnSheet(nSheet < -1 ? -1 : nSheet), mnIndex(nIndex) {}
    bool operator<( const SheetIndex& r ) const
    {
        // Ascending order sheet, index
        if (mnSheet < r.mnSheet)
            return true;
        if (mnSheet == r.mnSheet)
            return mnIndex < r.mnIndex;
        return false;
    }
};
typedef std::map< SheetIndex, SheetIndex > SheetIndexMap;

ScRangeData* copyRangeNames( SheetIndexMap& rSheetIndexMap, std::vector<ScRangeData*>& rRangeDataVec,
        const sc::UpdatedRangeNames& rReferencingNames, SCTAB nTab,
        const ScRangeData* pOldRangeData, ScDocument& rNewDoc, const ScDocument* pOldDoc,
        const ScAddress& rNewPos, const ScAddress& rOldPos, bool bGlobalNamesToLocal,
        const SCTAB nOldSheet, const SCTAB nNewSheet, bool bSameDoc)
{
    ScRangeData* pRangeData = nullptr;
    const ScRangeName* pOldRangeName = (nTab < 0 ? pOldDoc->GetRangeName() : pOldDoc->GetRangeName(nTab));
    if (pOldRangeName)
    {
        const ScRangeName* pNewRangeName = (nNewSheet < 0 ? rNewDoc.GetRangeName() : rNewDoc.GetRangeName(nNewSheet));
        sc::UpdatedRangeNames::NameIndicesType aSet( rReferencingNames.getUpdatedNames(nTab));
        for (auto const & rIndex : aSet)
        {
            const ScRangeData* pCopyData = pOldRangeName->findByIndex(rIndex);
            if (pCopyData)
            {
                // Match the original pOldRangeData to adapt the current
                // token's values later. For that no check for an already
                // copied name is needed as we only enter here if there was
                // none.
                if (pCopyData == pOldRangeData)
                {
                    pRangeData = copyRangeName( pCopyData, rNewDoc, pOldDoc, rNewPos, rOldPos,
                            bGlobalNamesToLocal, nOldSheet, nNewSheet, bSameDoc);
                    if (pRangeData)
                    {
                        rRangeDataVec.push_back(pRangeData);
                        rSheetIndexMap.insert( std::make_pair( SheetIndex( nOldSheet, pCopyData->GetIndex()),
                                    SheetIndex( nNewSheet, pRangeData->GetIndex())));
                    }
                }
                else
                {
                    // First check if the name is already available as copy.
                    const ScRangeData* pFoundData = pNewRangeName->findByUpperName( pCopyData->GetUpperName());
                    if (pFoundData)
                    {
                        // Just add the resulting sheet/index mapping.
                        rSheetIndexMap.insert( std::make_pair( SheetIndex( nOldSheet, pCopyData->GetIndex()),
                                    SheetIndex( nNewSheet, pFoundData->GetIndex())));
                    }
                    else
                    {
                        ScRangeData* pTmpData = copyRangeName( pCopyData, rNewDoc, pOldDoc, rNewPos, rOldPos,
                                bGlobalNamesToLocal, nOldSheet, nNewSheet, bSameDoc);
                        if (pTmpData)
                        {
                            rRangeDataVec.push_back(pTmpData);
                            rSheetIndexMap.insert( std::make_pair( SheetIndex( nOldSheet, pCopyData->GetIndex()),
                                        SheetIndex( nNewSheet, pTmpData->GetIndex())));
                        }
                    }
                }
            }
        }
    }
    return pRangeData;
}

}   // namespace

bool ScDocument::CopyAdjustRangeName( SCTAB& rSheet, sal_uInt16& rIndex, ScRangeData*& rpRangeData,
        ScDocument& rNewDoc, const ScAddress& rNewPos, const ScAddress& rOldPos, const bool bGlobalNamesToLocal,
        const bool bUsedByFormula ) const
{
    const bool bSameDoc = (rNewDoc.GetPool() == const_cast<ScDocument*>(this)->GetPool());
    if (bSameDoc && ((rSheet < 0 && !bGlobalNamesToLocal) || (rSheet >= 0 && rSheet != rOldPos.Tab())))
        // Same doc and global name, if not copied to local name, or
        // sheet-local name on other sheet stays the same.
        return false;

    // Ensure we don't fiddle with the references until exit.
    const SCTAB nOldSheet = rSheet;
    const sal_uInt16 nOldIndex = rIndex;

    SAL_WARN_IF( !bSameDoc && nOldSheet >= 0 && nOldSheet != rOldPos.Tab(),
            "sc.core", "adjustCopyRangeName - sheet-local name was on other sheet in other document");
    /* TODO: can we do something about that? e.g. loop over sheets? */

    OUString aRangeName;
    ScRangeData* pOldRangeData = nullptr;

    // XXX bGlobalNamesToLocal is also a synonym for copied sheet.
    bool bInsertingBefore = (bGlobalNamesToLocal && bSameDoc && rNewPos.Tab() <= rOldPos.Tab());

    // The Tab where an old local name is to be found or that a global name
    // references. May differ below from nOldSheet if a sheet was inserted
    // before the old position. Global names and local names other than on the
    // old sheet or new sheet are already updated, local names on the old sheet
    // or inserted sheet will be updated later. Confusing stuff. Watch out.
    SCTAB nOldTab = (nOldSheet < 0 ? rOldPos.Tab() : nOldSheet);
    if (bInsertingBefore)
        // Sheet was already inserted before old position.
        ++nOldTab;

    // Search the name of the RangeName.
    if (nOldSheet >= 0)
    {
        const ScRangeName* pNames = GetRangeName(nOldTab);
        pOldRangeData = pNames ? pNames->findByIndex(nOldIndex) : nullptr;
        if (!pOldRangeData)
            return false;     // might be an error in the formula array
        aRangeName = pOldRangeData->GetUpperName();
    }
    else
    {
        pOldRangeData = GetRangeName()->findByIndex(nOldIndex);
        if (!pOldRangeData)
            return false;     // might be an error in the formula array
        aRangeName = pOldRangeData->GetUpperName();
    }

    // Find corresponding range name in new document.
    // First search for local range name then global range names.
    SCTAB nNewSheet = rNewPos.Tab();
    ScRangeName* pNewNames = rNewDoc.GetRangeName(nNewSheet);
    // Search local range names.
    if (pNewNames)
    {
        rpRangeData = pNewNames->findByUpperName(aRangeName);
    }
    // Search global range names.
    if (!rpRangeData && !bGlobalNamesToLocal)
    {
        nNewSheet = -1;
        pNewNames = rNewDoc.GetRangeName();
        if (pNewNames)
            rpRangeData = pNewNames->findByUpperName(aRangeName);
    }
    // If no range name was found copy it.
    if (!rpRangeData)
    {
        // Do not copy global name if it doesn't reference sheet or is not used
        // by a formula copied to another document.
        bool bEarlyBailOut = (nOldSheet < 0 && (bSameDoc || !bUsedByFormula));
        MightReferenceSheet eMightReference = mightRangeNameReferenceSheet( pOldRangeData, nOldTab);
        if (bEarlyBailOut && eMightReference == MightReferenceSheet::NONE)
            return false;

        if (eMightReference == MightReferenceSheet::NAME)
        {
            // Name these to clarify what is passed where.
            const SCTAB nGlobalRefTab = nOldTab;
            const SCTAB nLocalRefTab = (bInsertingBefore ? nOldTab-1 : nOldTab);
            const SCTAB nOldTokenTab = (nOldSheet < 0 ? (bInsertingBefore ? nOldTab-1 : nOldTab) : nOldSheet);
            const SCTAB nOldTokenTabReplacement = nOldTab;
            sc::UpdatedRangeNames aReferencingNames;
            FindRangeNamesReferencingSheet( aReferencingNames, nOldSheet, nOldIndex,
                    nGlobalRefTab, nLocalRefTab, nOldTokenTab, nOldTokenTabReplacement, bSameDoc, 0);
            if (bEarlyBailOut && aReferencingNames.isEmpty(-1) && aReferencingNames.isEmpty(nOldTokenTabReplacement))
                return false;

            SheetIndexMap aSheetIndexMap;
            std::vector<ScRangeData*> aRangeDataVec;
            if (!aReferencingNames.isEmpty(nOldTokenTabReplacement))
            {
                const SCTAB nTmpOldSheet = (nOldSheet < 0 ? nOldTab : nOldSheet);
                nNewSheet = rNewPos.Tab();
                rpRangeData = copyRangeNames( aSheetIndexMap, aRangeDataVec, aReferencingNames, nOldTab,
                        pOldRangeData, rNewDoc, this, rNewPos, rOldPos,
                        bGlobalNamesToLocal, nTmpOldSheet, nNewSheet, bSameDoc);
            }
            if ((bGlobalNamesToLocal || !bSameDoc) && !aReferencingNames.isEmpty(-1))
            {
                const SCTAB nTmpOldSheet = -1;
                const SCTAB nTmpNewSheet = (bGlobalNamesToLocal ? rNewPos.Tab() : -1);
                ScRangeData* pTmpData = copyRangeNames( aSheetIndexMap, aRangeDataVec, aReferencingNames, -1,
                        pOldRangeData, rNewDoc, this, rNewPos, rOldPos,
                        bGlobalNamesToLocal, nTmpOldSheet, nTmpNewSheet, bSameDoc);
                if (!rpRangeData)
                {
                    rpRangeData = pTmpData;
                    nNewSheet = nTmpNewSheet;
                }
            }

            // Adjust copied nested names to new sheet/index.
            for (auto & iRD : aRangeDataVec)
            {
                ScTokenArray* pCode = iRD->GetCode();
                if (pCode)
                {
                    for (formula::FormulaToken* p = pCode->First(); p; p = pCode->Next())
                    {
                        if (p->GetOpCode() == ocName)
                        {
                            auto it = aSheetIndexMap.find( SheetIndex( p->GetSheet(), p->GetIndex()));
                            if (it != aSheetIndexMap.end())
                            {
                                p->SetSheet( it->second.mnSheet);
                                p->SetIndex( it->second.mnIndex);
                            }
                            else if (!bSameDoc)
                            {
                                SAL_WARN("sc.core","adjustCopyRangeName - mapping to new name in other doc missing");
                                p->SetIndex(0);     // #NAME? error instead of arbitrary name.
                            }
                        }
                    }
                }
            }
        }
        else
        {
            nNewSheet = ((nOldSheet < 0 && !bGlobalNamesToLocal) ? -1 : rNewPos.Tab());
            rpRangeData = copyRangeName( pOldRangeData, rNewDoc, this, rNewPos, rOldPos, bGlobalNamesToLocal,
                    nOldSheet, nNewSheet, bSameDoc);
        }
    }
    rSheet = nNewSheet;
    rIndex = rpRangeData ? rpRangeData->GetIndex() : 0;     // 0 means not inserted
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
