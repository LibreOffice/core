/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <column.hxx>
#include <clipcontext.hxx>
#include <clipparam.hxx>
#include <cellvalue.hxx>
#include <attarray.hxx>
#include <document.hxx>
#include <columnspanset.hxx>
#include <listenercontext.hxx>
#include <tokenstringcontext.hxx>
#include <mtvcellfunc.hxx>
#include <clipcontext.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <conditio.hxx>
#include <formulagroup.hxx>
#include <tokenarray.hxx>
#include <globalnames.hxx>
#include <scitems.hxx>
#include <cellform.hxx>
#include <sharedformula.hxx>

#include <svl/sharedstringpool.hxx>

#include <vector>
#include <cassert>

#include <boost/shared_ptr.hpp>

void ScColumn::DeleteBeforeCopyFromClip( sc::CopyFromClipContext& rCxt, const ScColumn& rClipCol )
{
    sc::CopyFromClipContext::Range aRange = rCxt.getDestRange();
    if (!ValidRow(aRange.mnRow1) || !ValidRow(aRange.mnRow2))
        return;

    ScRange aClipRange = rCxt.getClipDoc()->GetClipParam().getWholeRange();
    SCROW nClipRow1 = aClipRange.aStart.Row();
    SCROW nClipRow2 = aClipRange.aEnd.Row();
    SCROW nClipRowLen = nClipRow2 - nClipRow1 + 1;

    // Check for non-empty cell ranges in the clip column.
    sc::SingleColumnSpanSet aSpanSet;
    aSpanSet.scan(rClipCol, nClipRow1, nClipRow2);
    sc::SingleColumnSpanSet::SpansType aSpans;
    aSpanSet.getSpans(aSpans);

    if (aSpans.empty())
        // All cells in the range in the clip are empty.  Nothing to delete.
        return;

    // Translate the clip column spans into the destination column, and repeat as needed.
    std::vector<sc::RowSpan> aDestSpans;
    SCROW nDestOffset = aRange.mnRow1 - nClipRow1;
    bool bContinue = true;
    while (bContinue)
    {
        sc::SingleColumnSpanSet::SpansType::const_iterator it = aSpans.begin(), itEnd = aSpans.end();
        for (; it != itEnd && bContinue; ++it)
        {
            const sc::RowSpan& r = *it;
            SCROW nDestRow1 = r.mnRow1 + nDestOffset;
            SCROW nDestRow2 = r.mnRow2 + nDestOffset;

            if (nDestRow1 > aRange.mnRow2)
            {
                // We're done.
                bContinue = false;
                continue;
            }

            if (nDestRow2 > aRange.mnRow2)
            {
                // Truncate this range, and set it as the last span.
                nDestRow2 = aRange.mnRow2;
                bContinue = false;
            }

            aDestSpans.push_back(sc::RowSpan(nDestRow1, nDestRow2));
        }

        nDestOffset += nClipRowLen;
    }

    std::vector<SCROW> aDeletedRows;
    sal_uInt16 nDelFlag = rCxt.getDeleteFlag();
    sc::ColumnBlockPosition aBlockPos;
    InitBlockPosition(aBlockPos);

    std::vector<sc::RowSpan>::const_iterator it = aDestSpans.begin(), itEnd = aDestSpans.end();
    for (; it != itEnd; ++it)
    {
        SCROW nRow1 = it->mnRow1;
        SCROW nRow2 = it->mnRow2;

        if (nDelFlag & IDF_CONTENTS)
            DeleteCells(aBlockPos, nRow1, nRow2, nDelFlag, aDeletedRows);

        if (nDelFlag & IDF_NOTE)
            DeleteCellNotes(aBlockPos, nRow1, nRow2);

        if (nDelFlag & IDF_EDITATTR)
            RemoveEditAttribs(nRow1, nRow2);

        // Delete attributes just now
        if (nDelFlag & IDF_ATTRIB)
        {
            pAttrArray->DeleteArea(nRow1, nRow2);

            if (rCxt.isTableProtected())
            {
                ScPatternAttr aPattern(pDocument->GetPool());
                aPattern.GetItemSet().Put(ScProtectionAttr(false));
                ApplyPatternArea(nRow1, nRow2, aPattern);
            }

            ScConditionalFormatList* pCondList = rCxt.getCondFormatList();
            if (pCondList)
                pCondList->DeleteArea(nCol, nRow1, nCol, nRow2);
        }
        else if ((nDelFlag & IDF_HARDATTR) == IDF_HARDATTR)
            pAttrArray->DeleteHardAttr(nRow1, nRow2);
    }

    BroadcastCells(aDeletedRows, SC_HINT_DATACHANGED);
}

namespace {

class RecompileByOpcodeHandler
{
    ScDocument* mpDoc;
    const boost::unordered_set<OpCode>& mrOps;
    sc::EndListeningContext& mrEndListenCxt;
    sc::CompileFormulaContext& mrCompileFormulaCxt;

public:
    RecompileByOpcodeHandler(
        ScDocument* pDoc, const boost::unordered_set<OpCode>& rOps,
        sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt ) :
        mpDoc(pDoc),
        mrOps(rOps),
        mrEndListenCxt(rEndListenCxt),
        mrCompileFormulaCxt(rCompileCxt) {}

    void operator() ( sc::FormulaGroupEntry& rEntry )
    {
        // Perform end listening, remove from formula tree, and set them up
        // for re-compilation.

        ScFormulaCell* pTop = NULL;

        if (rEntry.mbShared)
        {
            // Only inspect the code from the top cell.
            pTop = *rEntry.mpCells;
        }
        else
            pTop = rEntry.mpCell;

        ScTokenArray* pCode = pTop->GetCode();
        bool bRecompile = pCode->HasOpCodes(mrOps);

        if (bRecompile)
        {
            // Get the formula string.
            OUString aFormula = pTop->GetFormula(mrCompileFormulaCxt);
            sal_Int32 n = aFormula.getLength();
            if (pTop->GetMatrixFlag() != MM_NONE && n > 0)
            {
                if (aFormula[0] == '{' && aFormula[n-1] == '}')
                    aFormula = aFormula.copy(1, n-2);
            }

            if (rEntry.mbShared)
            {
                ScFormulaCell** pp = rEntry.mpCells;
                ScFormulaCell** ppEnd = pp + rEntry.mnLength;
                for (; pp != ppEnd; ++pp)
                {
                    ScFormulaCell* p = *pp;
                    p->EndListeningTo(mrEndListenCxt);
                    mpDoc->RemoveFromFormulaTree(p);
                }
            }
            else
            {
                rEntry.mpCell->EndListeningTo(mrEndListenCxt);
                mpDoc->RemoveFromFormulaTree(rEntry.mpCell);
            }

            pCode->Clear();
            pTop->SetHybridFormula(aFormula, mpDoc->GetGrammar());
        }
    }
};

class CompileHybridFormulaHandler
{
    ScDocument* mpDoc;
    sc::StartListeningContext& mrStartListenCxt;
    sc::CompileFormulaContext& mrCompileFormulaCxt;

public:
    CompileHybridFormulaHandler( ScDocument* pDoc, sc::StartListeningContext& rStartListenCxt, sc::CompileFormulaContext& rCompileCxt ) :
        mpDoc(pDoc),
        mrStartListenCxt(rStartListenCxt),
        mrCompileFormulaCxt(rCompileCxt) {}

    void operator() ( sc::FormulaGroupEntry& rEntry )
    {
        if (rEntry.mbShared)
        {
            ScFormulaCell* pTop = *rEntry.mpCells;
            OUString aFormula = pTop->GetHybridFormula();

            if (!aFormula.isEmpty())
            {
                // Create a new token array from the hybrid formula string, and
                // set it to the group.
                ScCompiler aComp(mrCompileFormulaCxt, pTop->aPos);
                ScTokenArray* pNewCode = aComp.CompileString(aFormula);
                ScFormulaCellGroupRef xGroup = pTop->GetCellGroup();
                assert(xGroup);
                xGroup->setCode(pNewCode);
                xGroup->compileCode(*mpDoc, pTop->aPos, mpDoc->GetGrammar());

                // Propagate the new token array to all formula cells in the group.
                ScFormulaCell** pp = rEntry.mpCells;
                ScFormulaCell** ppEnd = pp + rEntry.mnLength;
                for (; pp != ppEnd; ++pp)
                {
                    ScFormulaCell* p = *pp;
                    p->SyncSharedCode();
                    p->StartListeningTo(mrStartListenCxt);
                    p->SetDirty();
                }
            }
        }
        else
        {
            ScFormulaCell* pCell = rEntry.mpCell;
            OUString aFormula = pCell->GetHybridFormula();

            if (!aFormula.isEmpty())
            {
                // Create token array from formula string.
                ScCompiler aComp(mrCompileFormulaCxt, pCell->aPos);
                ScTokenArray* pNewCode = aComp.CompileString(aFormula);

                // Generate RPN tokens.
                ScCompiler aComp2(mpDoc, pCell->aPos, *pNewCode);
                aComp2.CompileTokenArray();

                pCell->SetCode(pNewCode);
                pCell->StartListeningTo(mrStartListenCxt);
                pCell->SetDirty();
            }
        }
    }
};

}

void ScColumn::PreprocessRangeNameUpdate(
    sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    // Collect all formula groups.
    std::vector<sc::FormulaGroupEntry> aGroups = GetFormulaGroupEntries();

    boost::unordered_set<OpCode> aOps;
    aOps.insert(ocBad);
    aOps.insert(ocColRowName);
    aOps.insert(ocName);
    RecompileByOpcodeHandler aFunc(pDocument, aOps, rEndListenCxt, rCompileCxt);
    std::for_each(aGroups.begin(), aGroups.end(), aFunc);
}

void ScColumn::PreprocessDBDataUpdate(
    sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    // Collect all formula groups.
    std::vector<sc::FormulaGroupEntry> aGroups = GetFormulaGroupEntries();

    boost::unordered_set<OpCode> aOps;
    aOps.insert(ocBad);
    aOps.insert(ocColRowName);
    aOps.insert(ocDBArea);
    RecompileByOpcodeHandler aFunc(pDocument, aOps, rEndListenCxt, rCompileCxt);
    std::for_each(aGroups.begin(), aGroups.end(), aFunc);
}

void ScColumn::CompileHybridFormula(
    sc::StartListeningContext& rStartListenCxt, sc::CompileFormulaContext& rCompileCxt )
{
    // Collect all formula groups.
    std::vector<sc::FormulaGroupEntry> aGroups = GetFormulaGroupEntries();

    CompileHybridFormulaHandler aFunc(pDocument, rStartListenCxt, rCompileCxt);
    std::for_each(aGroups.begin(), aGroups.end(), aFunc);
}

namespace {

class ScriptTypeUpdater
{
    ScColumn& mrCol;
    sc::CellTextAttrStoreType& mrTextAttrs;
    sc::CellTextAttrStoreType::iterator miPosAttr;
    ScConditionalFormatList* mpCFList;
    SvNumberFormatter* mpFormatter;
    ScAddress maPos;
    bool mbUpdated;

private:
    void updateScriptType( size_t nRow, ScRefCellValue& rCell )
    {
        sc::CellTextAttrStoreType::position_type aAttrPos = mrTextAttrs.position(miPosAttr, nRow);
        miPosAttr = aAttrPos.first;

        if (aAttrPos.first->type != sc::element_type_celltextattr)
            return;

        sc::CellTextAttr& rAttr = sc::celltextattr_block::at(*aAttrPos.first->data, aAttrPos.second);
        if (rAttr.mnScriptType != SC_SCRIPTTYPE_UNKNOWN)
            // Script type already deteremined.  Skip it.
            return;

        const ScPatternAttr* pPat = mrCol.GetPattern(nRow);
        if (!pPat)
            // In theory this should never return NULL. But let's be safe.
            return;

        const SfxItemSet* pCondSet = NULL;
        if (mpCFList)
        {
            maPos.SetRow(nRow);
            const ScCondFormatItem& rItem =
                static_cast<const ScCondFormatItem&>(pPat->GetItem(ATTR_CONDITIONAL));
            const std::vector<sal_uInt32>& rData = rItem.GetCondFormatData();
            pCondSet = mrCol.GetDoc().GetCondResult(rCell, maPos, *mpCFList, rData);
        }

        OUString aStr;
        Color* pColor;
        sal_uLong nFormat = pPat->GetNumberFormat(mpFormatter, pCondSet);
        ScCellFormat::GetString(rCell, nFormat, aStr, &pColor, *mpFormatter, &mrCol.GetDoc());

        rAttr.mnScriptType = mrCol.GetDoc().GetStringScriptType(aStr);
        mbUpdated = true;
    }

public:
    ScriptTypeUpdater( ScColumn& rCol ) :
        mrCol(rCol),
        mrTextAttrs(rCol.GetCellAttrStore()),
        miPosAttr(mrTextAttrs.begin()),
        mpCFList(rCol.GetDoc().GetCondFormList(rCol.GetTab())),
        mpFormatter(rCol.GetDoc().GetFormatTable()),
        maPos(rCol.GetCol(), 0, rCol.GetTab()),
        mbUpdated(false)
    {}

    void operator() ( size_t nRow, double fVal )
    {
        ScRefCellValue aCell(fVal);
        updateScriptType(nRow, aCell);
    }

    void operator() ( size_t nRow, const svl::SharedString& rStr )
    {
        ScRefCellValue aCell(&rStr);
        updateScriptType(nRow, aCell);
    }

    void operator() ( size_t nRow, const EditTextObject* pText )
    {
        ScRefCellValue aCell(pText);
        updateScriptType(nRow, aCell);
    }

    void operator() ( size_t nRow, const ScFormulaCell* pCell )
    {
        ScRefCellValue aCell(const_cast<ScFormulaCell*>(pCell));
        updateScriptType(nRow, aCell);
    }

    bool isUpdated() const { return mbUpdated; }
};

}

void ScColumn::UpdateScriptTypes( SCROW nRow1, SCROW nRow2 )
{
    if (!ValidRow(nRow1) || !ValidRow(nRow2) || nRow1 > nRow2)
        return;

    ScriptTypeUpdater aFunc(*this);
    sc::ParseAllNonEmpty(maCells.begin(), maCells, nRow1, nRow2, aFunc);
    if (aFunc.isUpdated())
        CellStorageModified();
}

void ScColumn::Swap( ScColumn& rOther, SCROW nRow1, SCROW nRow2, bool bPattern )
{
    maCells.swap(nRow1, nRow2, rOther.maCells, nRow1);
    maCellTextAttrs.swap(nRow1, nRow2, rOther.maCellTextAttrs, nRow1);
    maCellNotes.swap(nRow1, nRow2, rOther.maCellNotes, nRow1);
    maBroadcasters.swap(nRow1, nRow2, rOther.maBroadcasters, nRow1);

    if (bPattern)
    {
        for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
        {
            const ScPatternAttr* pPat1 = GetPattern(nRow);
            const ScPatternAttr* pPat2 = rOther.GetPattern(nRow);
            if (pPat1 != pPat2)
            {
                SetPattern(nRow, *pPat2, true);
                rOther.SetPattern(nRow, *pPat1, true);
            }
        }
    }

    CellStorageModified();
    rOther.CellStorageModified();
}

namespace {

class FormulaColPosSetter
{
    SCCOL mnCol;
public:
    FormulaColPosSetter( SCCOL nCol ) : mnCol(nCol) {}

    void operator() ( size_t nRow, ScFormulaCell* pCell )
    {
        if (!pCell->IsShared() || pCell->IsSharedTop())
        {
            // Ensure that the references still point to the same locations
            // after the position change.
            ScAddress aOldPos = pCell->aPos;
            pCell->aPos.SetCol(mnCol);
            pCell->aPos.SetRow(nRow);
            pCell->GetCode()->AdjustReferenceOnMovedOrigin(aOldPos, pCell->aPos);
        }
        else
        {
            pCell->aPos.SetCol(mnCol);
            pCell->aPos.SetRow(nRow);
        }
    }
};

}

void ScColumn::ResetFormulaCellPositions( SCROW nRow1, SCROW nRow2 )
{
    FormulaColPosSetter aFunc(nCol);
    sc::ProcessFormula(maCells.begin(), maCells, nRow1, nRow2, aFunc);
}

namespace {

class RelativeRefBoundChecker
{
    std::vector<SCROW> maBounds;
    ScRange maBoundRange;

public:
    RelativeRefBoundChecker( const ScRange& rBoundRange ) :
        maBoundRange(rBoundRange) {}

    void operator() ( size_t /*nRow*/, ScFormulaCell* pCell )
    {
        if (!pCell->IsSharedTop())
            return;

        pCell->GetCode()->CheckRelativeReferenceBounds(
            pCell->aPos, pCell->GetSharedLength(), maBoundRange, maBounds);
    }

    void swapBounds( std::vector<SCROW>& rBounds )
    {
        rBounds.swap(maBounds);
    }
};

}

void ScColumn::SplitFormulaGroupByRelativeRef( const ScRange& rBoundRange )
{
    std::vector<SCROW> aBounds;

    // Cut at row boundaries first.
    aBounds.push_back(rBoundRange.aStart.Row());
    aBounds.push_back(rBoundRange.aEnd.Row()+1);
    sc::SharedFormulaUtil::splitFormulaCellGroups(maCells, aBounds);

    RelativeRefBoundChecker aFunc(rBoundRange);
    sc::ProcessFormula(
        maCells.begin(), maCells, rBoundRange.aStart.Row(), rBoundRange.aEnd.Row(), aFunc);
    aFunc.swapBounds(aBounds);
    sc::SharedFormulaUtil::splitFormulaCellGroups(maCells, aBounds);
}

namespace {

class ListenerCollector
{
    std::vector<SvtListener*>& mrListeners;
public:
    ListenerCollector( std::vector<SvtListener*>& rListener ) :
        mrListeners(rListener) {}

    void operator() ( size_t /*nRow*/, SvtBroadcaster* p )
    {
        SvtBroadcaster::ListenersType& rLis = p->GetAllListeners();
        std::copy(rLis.begin(), rLis.end(), std::back_inserter(mrListeners));
    }
};

}

void ScColumn::CollectListeners( std::vector<SvtListener*>& rListeners, SCROW nRow1, SCROW nRow2 )
{
    if (nRow2 < nRow1 || !ValidRow(nRow1) || !ValidRow(nRow2))
        return;

    ListenerCollector aFunc(rListeners);
    sc::ProcessBroadcaster(maBroadcasters.begin(), maBroadcasters, nRow1, nRow2, aFunc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
