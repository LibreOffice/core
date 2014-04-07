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
        const ScPatternAttr* pAttr = pClipDoc->GetPattern(aSrcPos);
        rCxt.setSingleCellPattern(pAttr);

        // Check the paste flag to see whether we want to paste this cell.  If the
        // flag says we don't want to paste this cell, we'll return with true.
        sal_uInt16 nFlags = rCxt.getInsertFlag();
        bool bNumeric  = (nFlags & IDF_VALUE) != 0;
        bool bDateTime = (nFlags & IDF_DATETIME) != 0;
        bool bString   = (nFlags & IDF_STRING) != 0;
        bool bBoolean  = (nFlags & IDF_SPECIAL_BOOLEAN) != 0;
        bool bFormula  = (nFlags & IDF_FORMULA) != 0;

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

    if ((rCxt.getInsertFlag() & (IDF_NOTE | IDF_ADDNOTES)) != 0)
        rCxt.setSingleCellNote(pClipDoc->GetNote(aSrcPos));

    // All good. Proceed with the pasting.

    SCTAB nTabEnd = rCxt.getTabEnd();
    for (SCTAB i = rCxt.getTabStart(); i <= nTabEnd && i < static_cast<SCTAB>(maTabs.size()); ++i)
        maTabs[i]->CopyOneCellFromClip(rCxt, nCol1, nRow1, nCol2, nRow2);

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

void ScDocument::SetCalcConfig( const ScCalcConfig& rConfig )
{
    maCalcConfig = rConfig;
}

const ScCalcConfig& ScDocument::GetCalcConfig() const
{
    return maCalcConfig;
}

void ScDocument::PreprocessRangeNameUpdate()
{
    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* p = *it;
        p->PreprocessRangeNameUpdate();
    }
}

void ScDocument::PostprocessRangeNameUpdate()
{
    TableContainer::iterator it = maTabs.begin(), itEnd = maTabs.end();
    for (; it != itEnd; ++it)
    {
        ScTable* p = *it;
        p->PostprocessRangeNameUpdate();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
