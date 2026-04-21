/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <undo/UndoSpillPivotTable.hxx>
#include <docsh.hxx>
#include <dpobject.hxx>
#include <document.hxx>
#include <formula/errorcodes.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

namespace sc
{
UndoSpillPivotTable::UndoSpillPivotTable(ScDocShell& rNewDocShell, const OUString& rDPName,
                                         const ScRange& rOutputRange)
    : ScSimpleUndo(rNewDocShell)
    , maDPName(rDPName)
    , maOutputRange(rOutputRange)
{
}

OUString UndoSpillPivotTable::GetComment() const { return ScResId(STR_UNDO_PIVOT_MODIFY); }

void UndoSpillPivotTable::Undo()
{
    // Undo: go from rendered pivot table back to #SPILL! state.
    BeginUndo();

    ScDocument& rDocument = rDocShell.GetDocument();
    ScDPObject* pDPObject = rDocument.GetDPCollection()->GetByName(maDPName);
    if (pDPObject)
    {
        // Clear the rendered output.
        rDocument.DeleteAreaTab(maOutputRange, InsertDeleteFlags::ALL);
        rDocument.RemoveFlagsTab(maOutputRange.aStart.Col(), maOutputRange.aStart.Row(),
                                 maOutputRange.aEnd.Col(), maOutputRange.aEnd.Row(),
                                 maOutputRange.aStart.Tab(), ScMF::Auto);

        // Write #SPILL! into the origin cell.
        ScAddress aOrigin = maOutputRange.aStart;
        rDocument.SetError(aOrigin.Col(), aOrigin.Row(), aOrigin.Tab(), FormulaError::Spill);
        pDPObject->SetOutRange(ScRange(aOrigin));
        pDPObject->SetSpillError(true);

        rDocShell.PostPaint(maOutputRange, PaintPartFlags::Grid);
    }

    EndUndo();
}

void UndoSpillPivotTable::Redo()
{
    // Redo: resolve the spill and render the pivot table.
    BeginRedo();

    ScDocument& rDocument = rDocShell.GetDocument();
    ScDPObject* pDPObject = rDocument.GetDPCollection()->GetByName(maDPName);
    if (pDPObject)
    {
        ScAddress aOrigin = pDPObject->GetOutRange().aStart;
        pDPObject->InvalidateData();
        pDPObject->Output(aOrigin, true);
        rDocShell.PostPaintGridAll();
    }

    EndRedo();
}

void UndoSpillPivotTable::Repeat(SfxRepeatTarget& /*rTarget*/) {}

bool UndoSpillPivotTable::CanRepeat(SfxRepeatTarget& /*rTarget*/) const { return false; }

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
