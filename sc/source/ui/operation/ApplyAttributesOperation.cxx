/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ApplyAttributesOperation.hxx>

#include <docfuncutil.hxx>
#include <docfunc.hxx>
#include <address.hxx>
#include <editable.hxx>
#include <undoblk.hxx>
#include <validat.hxx>
#include <globstr.hrc>

#include <memory>

namespace sc
{
ApplyAttributesOperation::ApplyAttributesOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                                   const ScMarkData& rMark,
                                                   const ScPatternAttr& rPattern, bool bApi)
    : Operation(OperationType::ApplyAttributes, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrMark(rMark)
    , mrPattern(rPattern)
{
}

bool ApplyAttributesOperation::runImplementation()
{
    ScDocument& rDoc = mrDocShell.GetDocument();
    bool bRecord = true;
    if (!rDoc.IsUndoEnabled())
        bRecord = false;

    ScMarkData aMark = mrMark;

    bool bImportingXML = rDoc.IsImportingXML();
    bool bImportingXLSX = rDoc.IsImportingXLSX();
    // Cell formats can still be set if the range isn't editable only because of matrix formulas.
    // #i62483# When loading XML, the check can be skipped altogether.
    bool bOnlyNotBecauseOfMatrix;
    if (!bImportingXML && !rDoc.IsSelectionEditable(aMark, &bOnlyNotBecauseOfMatrix)
        && !bOnlyNotBecauseOfMatrix)
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_PROTECTIONERR);
        return false;
    }

    ScDocShellModificator aModificator(mrDocShell);

    //! Border

    ScRange aMultiRange;
    bool bMulti = aMark.IsMultiMarked();
    if (bMulti)
        aMultiRange = aMark.GetMultiMarkArea();
    else
        aMultiRange = aMark.GetMarkArea();

    if (bRecord)
    {
        ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo(rDoc, aMultiRange.aStart.Tab(), aMultiRange.aEnd.Tab());
        rDoc.CopyToDocument(aMultiRange, InsertDeleteFlags::ATTRIB, bMulti, *pUndoDoc, &aMark);

        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoSelectionAttr>(
            &mrDocShell, aMark, aMultiRange.aStart.Col(), aMultiRange.aStart.Row(),
            aMultiRange.aStart.Tab(), aMultiRange.aEnd.Col(), aMultiRange.aEnd.Row(),
            aMultiRange.aEnd.Tab(), std::move(pUndoDoc), bMulti, &mrPattern));
    }

    // While loading XML it is not necessary to ask HasAttrib. It needs too much time.
    sal_uInt16 nExtFlags = 0;
    if (!bImportingXML && !bImportingXLSX)
        mrDocShell.UpdatePaintExt(nExtFlags, aMultiRange); // content before the change

    bool bChanged = false;
    rDoc.ApplySelectionPattern(mrPattern, aMark, nullptr, &bChanged);

    if (bChanged)
    {
        if (!bImportingXML && !bImportingXLSX)
            mrDocShell.UpdatePaintExt(nExtFlags, aMultiRange); // content after the change

        if (!mrDocFunc.AdjustRowHeight(aMultiRange, true, mbApi))
            mrDocShell.PostPaint(aMultiRange, PaintPartFlags::Grid, nExtFlags);
        else if (nExtFlags & SC_PF_LINES)
            ScDocFunc::PaintAbove(mrDocShell, aMultiRange); // because of lines above the range

        aModificator.SetDocumentModified();
    }

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
