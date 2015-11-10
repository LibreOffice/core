/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <undosort.hxx>
#include <globstr.hrc>
#include <global.hxx>
#include <undoutil.hxx>

namespace sc {

UndoSort::UndoSort( ScDocShell* pDocSh, const ReorderParam& rParam ) :
    ScSimpleUndo(pDocSh), maParam(rParam) {}

OUString UndoSort::GetComment() const
{
    return ScGlobal::GetRscString(STR_UNDO_SORT);
}

void UndoSort::Undo()
{
    BeginUndo();
    Execute(true);
    EndUndo();
}

void UndoSort::Redo()
{
    BeginRedo();
    Execute(false);
    EndRedo();
}

void UndoSort::Execute( bool bUndo )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    sc::ReorderParam aParam = maParam;
    if (bUndo)
        aParam.reverse();
    rDoc.Reorder(aParam, nullptr);

    if (maParam.mbHasHeaders)
    {
        ScRange aMarkRange( maParam.maSortRange);
        if (maParam.mbByRow)
        {
            if (aMarkRange.aStart.Row() > 0)
                aMarkRange.aStart.IncRow(-1);
        }
        else
        {
            if (aMarkRange.aStart.Col() > 0)
                aMarkRange.aStart.IncCol(-1);
        }
        ScUndoUtil::MarkSimpleBlock(pDocShell, aMarkRange);
    }
    else
    {
        ScUndoUtil::MarkSimpleBlock(pDocShell, maParam.maSortRange);
    }

    rDoc.SetDirty(maParam.maSortRange, true);
    if (!aParam.mbUpdateRefs)
        rDoc.BroadcastCells(aParam.maSortRange, SC_HINT_DATACHANGED);

    pDocShell->PostPaint(maParam.maSortRange, PAINT_GRID);
    pDocShell->PostDataChanged();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
