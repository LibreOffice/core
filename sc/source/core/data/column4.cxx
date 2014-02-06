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
#include <mtvcellfunc.hxx>
#include <clipcontext.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <conditio.hxx>

#include <svl/sharedstringpool.hxx>

#include <vector>
#include <cassert>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
