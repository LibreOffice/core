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
    ScDocument& rDoc = *pDocShell->GetDocument();
    sc::ReorderParam aParam = maParam;
    if (bUndo)
        aParam.reverse();
    rDoc.Reorder(aParam, NULL);

    ScUndoUtil::MarkSimpleBlock(pDocShell, maParam.maSortRange);

    pDocShell->PostPaint(maParam.maSortRange, PAINT_GRID);
    pDocShell->PostDataChanged();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
