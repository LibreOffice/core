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

void ScDocument::DeleteBeforeCopyFromClip( sc::CopyFromClipContext& rCxt, const ScMarkData& rMark )
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

        pTab->DeleteBeforeCopyFromClip(rCxt, *rClipTabs[nClipTab]);

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
    if (aClipRange.aStart != aClipRange.aEnd)
        // The source is not really a single cell. Bail out.
        return false;

    ScAddress aSrcPos = aClipRange.aStart;
    if (pClipDoc->IsMerged(aSrcPos))
        // We don't handle merged source cell for this.
        return false;

    ScTable* pSrcTab = pClipDoc->FetchTable(aSrcPos.Tab());
    if (!pSrcTab)
        return false;

    ScCellValue& rSrcCell = rCxt.getSingleCell();
    const ScPatternAttr* pAttr = pClipDoc->GetPattern(aSrcPos);
    rCxt.setSingleCellPattern(pAttr);
    if (rCxt.isAsLink())
    {
        ScSingleRefData aRef;
        aRef.InitAddress(aSrcPos);
        aRef.SetFlag3D(true);

        ScTokenArray aArr;
        aArr.AddSingleReference(aRef);
        rSrcCell.set(new ScFormulaCell(pClipDoc, aSrcPos, aArr));
    }
    else
    {
        rSrcCell.set(pClipDoc->GetRefCellValue(aSrcPos));

        // Check the paste flag to see whether we want to paste this cell.  If the
        // flag says we don't want to paste this cell, we'll return with true.
        InsertDeleteFlags nFlags = rCxt.getInsertFlag();
        bool bNumeric  = (nFlags & IDF_VALUE) != IDF_NONE;
        bool bDateTime = (nFlags & IDF_DATETIME) != IDF_NONE;
        bool bString   = (nFlags & IDF_STRING) != IDF_NONE;
        bool bBoolean  = (nFlags & IDF_SPECIAL_BOOLEAN) != IDF_NONE;
        bool bFormula  = (nFlags & IDF_FORMULA) != IDF_NONE;

        switch (rSrcCell.meType)
        {
            case CELLTYPE_VALUE:
            {
                bool bPaste = rCxt.isDateCell(pSrcTab->aCol[aSrcPos.Col()], aSrcPos.Row()) ? bDateTime : bNumeric;
                if (!bPaste)
                    // Don't paste this.
                    rSrcCell.clear();
            }
            break;
            case CELLTYPE_STRING:
            case CELLTYPE_EDIT:
            {
                if (!bString)
                    // Skip pasting.
                    rSrcCell.clear();
            }
            break;
            case CELLTYPE_FORMULA:
            {
                if (bBoolean)
                {
                    // Check if this formula cell is a boolean cell, and if so, go ahead and paste it.
                    ScTokenArray* pCode = rSrcCell.mpFormula->GetCode();
                    if (pCode && pCode->GetLen() == 1)
                    {
                        const formula::FormulaToken* p = pCode->First();
                        if (p->GetOpCode() == ocTrue || p->GetOpCode() == ocFalse)
                            // This is a boolean formula. Good.
                            break;
                    }
                }

                if (bFormula)
                    // Good.
                    break;

                sal_uInt16 nErr = rSrcCell.mpFormula->GetErrCode();
                if (nErr)
                {
                    // error codes are cloned with values
                    if (!bNumeric)
                        // Error code is treated as numeric value. Don't paste it.
                        rSrcCell.clear();
                }
                else if (rSrcCell.mpFormula->IsValue())
                {
                    bool bPaste = rCxt.isDateCell(pSrcTab->aCol[aSrcPos.Col()], aSrcPos.Row()) ? bDateTime : bNumeric;
                    if (!bPaste)
                    {
                        // Don't paste this.
                        rSrcCell.clear();
                        break;
                    }

                    // Turn this into a numeric cell.
                    rSrcCell.set(rSrcCell.mpFormula->GetValue());
                }
                else if (bString)
                {
                    svl::SharedString aStr = rSrcCell.mpFormula->GetString();
                    if (aStr.isEmpty())
                    {
                        // do not clone empty string
                        rSrcCell.clear();
                        break;
                    }

                    // Turn this into a string or edit cell.
                    if (rSrcCell.mpFormula->IsMultilineResult())
                    {
                        // TODO : Add shared string support to the edit engine to
                        // make this process simpler.
                        ScFieldEditEngine& rEngine = GetEditEngine();
                        rEngine.SetText(rSrcCell.mpFormula->GetString().getString());
                        boost::scoped_ptr<EditTextObject> pObj(rEngine.CreateTextObject());
                        pObj->NormalizeString(GetSharedStringPool());
                        rSrcCell.set(*pObj);
                    }
                    else
                        rSrcCell.set(rSrcCell.mpFormula->GetString());
                }
                else
                    // We don't want to paste this.
                    rSrcCell.clear();
            }
            break;
            case CELLTYPE_NONE:
            default:
                // There is nothing to paste.
                rSrcCell.clear();
        }
    }

    if ((rCxt.getInsertFlag() & (IDF_NOTE | IDF_ADDNOTES)) != IDF_NONE)
        rCxt.setSingleCellNote(pClipDoc->GetNote(aSrcPos));

    // All good. Proceed with the pasting.

    SCTAB nTabEnd = rCxt.getTabEnd();
    for (SCTAB i = rCxt.getTabStart(); i <= nTabEnd && i < static_cast<SCTAB>(maTabs.size()); ++i)
    {
        maTabs[i]->CopyOneCellFromClip(rCxt, nCol1, nRow1, nCol2, nRow2);
        if (rCxt.getInsertFlag() & IDF_ATTRIB)
            maTabs[i]->CopyConditionalFormat(nCol1, nRow1, nCol2, nRow2, nCol1 - aClipRange.aStart.Col(),
                    nRow1 - aClipRange.aStart.Row(), pSrcTab);
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

std::vector<Color> ScDocument::GetDocColors()
{
    std::vector<Color> docColors;

    for( unsigned int nTabIx = 0; nTabIx < maTabs.size(); ++nTabIx )
    {
        ScUsedAreaIterator aIt(this, nTabIx, 0, 0, MAXCOLCOUNT-1, MAXROWCOUNT-1);

        for( bool bIt = aIt.GetNext(); bIt; bIt = aIt.GetNext() )
        {
            const ScPatternAttr* pPattern = aIt.GetPattern();

            if( pPattern == 0 )
                continue;

            const SfxItemSet& rItemSet = pPattern->GetItemSet();

            SfxWhichIter aIter( rItemSet );
            sal_uInt16 nWhich = aIter.FirstWhich();
            while( nWhich )
            {
                const SfxPoolItem *pItem;
                if( SfxItemState::SET == rItemSet.GetItemState( nWhich, false, &pItem ) )
                {
                    sal_uInt16 aWhich = pItem->Which();
                    switch (aWhich)
                    {
                        // attributes we want to collect
                        case ATTR_FONT_COLOR:
                        case ATTR_BACKGROUND:
                            {
                                Color aColor( ((SvxColorItem*)pItem)->GetValue() );
                                if( COL_AUTO != aColor.GetColor() &&
                                        std::find(docColors.begin(), docColors.end(), aColor) == docColors.end() )
                                {
                                    docColors.push_back( aColor );
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }

                nWhich = aIter.NextWhich();
            }
        }
    }
    return docColors;
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

    boost::shared_ptr<sc::ColumnBlockPositionSet> pPosSet(new sc::ColumnBlockPositionSet(*this));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
