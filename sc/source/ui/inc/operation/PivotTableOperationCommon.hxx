/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <address.hxx>
#include <dociter.hxx>
#include <docsh.hxx>
#include <dpobject.hxx>
#include <dpshttab.hxx>
#include <editable.hxx>
#include <globstr.hrc>
#include <rangelst.hxx>

namespace sc::pivot
{
/** Can the input range be edited. */
inline bool isEditable(ScDocShell& rDocShell, const ScRangeList& rRanges, bool bApi,
                       sc::EditAction eAction = sc::EditAction::Unknown)
{
    ScDocument& rDoc = rDocShell.GetDocument();
    if (!rDocShell.IsEditable() || rDoc.GetChangeTrack())
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PROTECTIONERR);
        return false;
    }

    for (size_t i = 0, n = rRanges.size(); i < n; ++i)
    {
        const ScRange& r = rRanges[i];
        ScEditableTester aTester = ScEditableTester::CreateAndTestRange(rDoc, r, eAction);
        if (!aTester.IsEditable())
        {
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());
            return false;
        }
    }

    return true;
}

/** Create undo and copy the data to the undo document. */
inline void createUndoDoc(ScDocumentUniquePtr& pUndoDoc, ScDocument& rDocument,
                          const ScRange& rRange)
{
    SCTAB nTab = rRange.aStart.Tab();
    pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
    pUndoDoc->InitUndo(rDocument, nTab, nTab);
    rDocument.CopyToDocument(rRange, InsertDeleteFlags::ALL, false, *pUndoDoc);
}

inline bool lcl_EmptyExcept(ScDocument& rDoc, const ScRange& rRange, const ScRange& rExcept)
{
    ScCellIterator aIter(rDoc, rRange);
    for (bool bHasCell = aIter.first(); bHasCell; bHasCell = aIter.next())
    {
        if (!aIter.isEmpty()) // real content?
        {
            if (!rExcept.Contains(aIter.GetPos()))
                return false; // cell found
        }
    }

    return true; // nothing found - empty
}

inline bool checkNewOutputRange(ScDPObject& rDPObj, ScDocShell& rDocShell, ScRange& rNewOut,
                                bool bApi, sc::EditAction eAction = sc::EditAction::Unknown)
{
    ScDocument& rDoc = rDocShell.GetDocument();

    bool bOverflow = false;
    rNewOut = rDPObj.GetNewOutputRange(bOverflow);

    // Test for overlap with source data range.
    // TODO: Check with other pivot tables as well.
    const ScSheetSourceDesc* pSheetDesc = rDPObj.GetSheetDesc();
    if (pSheetDesc && pSheetDesc->GetSourceRange().Intersects(rNewOut))
    {
        // New output range intersteps with the source data. Move it up to
        // where the old range is and see if that works.
        ScRange aOldRange = rDPObj.GetOutRange();
        SCROW nDiff = aOldRange.aStart.Row() - rNewOut.aStart.Row();
        rNewOut.aStart.SetRow(aOldRange.aStart.Row());
        rNewOut.aEnd.IncRow(nDiff);
        if (!rDoc.ValidRow(rNewOut.aStart.Row()) || !rDoc.ValidRow(rNewOut.aEnd.Row()))
            bOverflow = true;
    }

    if (bOverflow)
    {
        if (!bApi)
            rDocShell.ErrorMessage(STR_PIVOT_ERROR);

        return false;
    }

    if (!rDoc.IsImportingXML())
    {
        ScEditableTester aTester = ScEditableTester::CreateAndTestRange(rDoc, rNewOut, eAction);
        if (!aTester.IsEditable())
        {
            //  destination area isn't editable
            if (!bApi)
                rDocShell.ErrorMessage(aTester.GetMessageId());

            return false;
        }
    }

    return true;
}
} // end sc::pivot namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
