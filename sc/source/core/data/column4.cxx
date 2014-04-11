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

class PreRangeNameUpdateHandler
{
    ScDocument* mpDoc;
    sc::EndListeningContext& mrEndListenCxt;
    sc::CompileFormulaContext& mrCompileFormulaCxt;

public:
    PreRangeNameUpdateHandler( ScDocument* pDoc, sc::EndListeningContext& rEndListenCxt, sc::CompileFormulaContext& rCompileCxt ) :
        mpDoc(pDoc),
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

        boost::unordered_set<OpCode> aOps;
        aOps.insert(ocBad);
        aOps.insert(ocColRowName);
        aOps.insert(ocName);
        bool bRecompile = pCode->HasOpCodes(aOps);

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

class PostRangeNameUpdateHandler
{
    ScDocument* mpDoc;
    sc::CompileFormulaContext& mrCompileFormulaCxt;

public:
    PostRangeNameUpdateHandler( ScDocument* pDoc, sc::CompileFormulaContext& rCompileCxt ) :
        mpDoc(pDoc),
        mrCompileFormulaCxt(rCompileCxt) {}

    void operator() ( sc::FormulaGroupEntry& rEntry )
    {
        if (rEntry.mbShared)
        {
            ScFormulaCell* pTop = *rEntry.mpCells;
            OUString aFormula = pTop->GetHybridFormula();

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
                p->SetDirty();
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

    PreRangeNameUpdateHandler aFunc(pDocument, rEndListenCxt, rCompileCxt);
    std::for_each(aGroups.begin(), aGroups.end(), aFunc);
}

void ScColumn::PostprocessRangeNameUpdate( sc::CompileFormulaContext& rCompileCxt )
{
    // Collect all formula groups.
    std::vector<sc::FormulaGroupEntry> aGroups = GetFormulaGroupEntries();

    PostRangeNameUpdateHandler aFunc(pDocument, rCompileCxt);
    std::for_each(aGroups.begin(), aGroups.end(), aFunc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
