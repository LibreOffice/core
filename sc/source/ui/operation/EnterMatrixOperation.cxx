/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/EnterMatrixOperation.hxx>

#include <compiler.hxx>
#include <docsh.hxx>
#include <editable.hxx>
#include <markdata.hxx>
#include <tokenarray.hxx>
#include <undoblk.hxx>
#include <viewdata.hxx>

#include <vcl/weld.hxx>

#include <memory>

namespace sc
{
EnterMatrixOperation::EnterMatrixOperation(ScDocShell& rDocShell, const ScRange& rRange,
                                           const ScMarkData* pTabMark,
                                           const ScTokenArray* pTokenArray, const OUString& rString,
                                           bool bApi, bool bEnglish, const OUString& rFormulaNmsp,
                                           formula::FormulaGrammar::Grammar eGrammar)
    : Operation(OperationType::EnterMatrix, true, bApi)
    , mrDocShell(rDocShell)
    , maRange(rRange)
    , mpTabMark(pTabMark)
    , mpTokenArray(pTokenArray)
    , maString(rString)
    , mbEnglish(bEnglish)
    , maFormulaNmsp(rFormulaNmsp)
    , meGrammar(eGrammar)
{
}

bool EnterMatrixOperation::runImplementation()
{
    if (ScViewData::SelectionFillDOOM(maRange))
        return false;

    ScDocShellModificator aModificator(mrDocShell);

    bool bSuccess = false;
    ScDocument& rDoc = mrDocShell.GetDocument();
    SCCOL nStartCol = maRange.aStart.Col();
    SCROW nStartRow = maRange.aStart.Row();
    SCTAB nStartTab = maRange.aStart.Tab();
    SCCOL nEndCol = maRange.aEnd.Col();
    SCROW nEndRow = maRange.aEnd.Row();
    SCTAB nEndTab = maRange.aEnd.Tab();

    ScMarkData aMark(rDoc.GetSheetLimits());
    if (mpTabMark)
        aMark = *mpTabMark;
    else
    {
        for (SCTAB nTab = nStartTab; nTab <= nEndTab; nTab++)
            aMark.SelectTable(nTab, true);
    }

    if (!checkSheetViewProtection())
        return false;

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelectedBlock(
        rDoc, nStartCol, nStartRow, nEndCol, nEndRow, aMark);
    if (aTester.IsEditable())
    {
        weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

        ScDocumentUniquePtr pUndoDoc;

        if (mbRecord && !rDoc.IsUndoEnabled())
            mbRecord = false;

        if (mbRecord)
        {
            //! take selected sheets into account also when undoing
            pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
            pUndoDoc->InitUndo(rDoc, nStartTab, nEndTab);
            rDoc.CopyToDocument(maRange, InsertDeleteFlags::ALL & ~InsertDeleteFlags::NOTE, false,
                                *pUndoDoc);
        }

        // use TokenArray if given, string (and flags) otherwise
        if (mpTokenArray)
        {
            rDoc.InsertMatrixFormula(nStartCol, nStartRow, nEndCol, nEndRow, aMark, OUString(),
                                     mpTokenArray, meGrammar);
        }
        else if (rDoc.IsImportingXML())
        {
            ScTokenArray aCode(rDoc);
            aCode.AssignXMLString(maString, ((meGrammar == formula::FormulaGrammar::GRAM_EXTERNAL)
                                                 ? maFormulaNmsp
                                                 : OUString()));
            rDoc.InsertMatrixFormula(nStartCol, nStartRow, nEndCol, nEndRow, aMark, OUString(),
                                     &aCode, meGrammar);
            rDoc.IncXMLImportedFormulaCount(maString.getLength());
        }
        else if (mbEnglish)
        {
            ScCompiler aComp(rDoc, maRange.aStart, meGrammar);
            std::unique_ptr<ScTokenArray> pCode = aComp.CompileString(maString);
            rDoc.InsertMatrixFormula(nStartCol, nStartRow, nEndCol, nEndRow, aMark, OUString(),
                                     pCode.get(), meGrammar);
        }
        else
            rDoc.InsertMatrixFormula(nStartCol, nStartRow, nEndCol, nEndRow, aMark, maString,
                                     nullptr, meGrammar);

        if (mbRecord)
        {
            //! take selected sheets into account also when undoing
            mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoEnterMatrix>(
                &mrDocShell, maRange, std::move(pUndoDoc), maString));
        }

        //  Err522 painting of DDE-Formulas will be intercepted during interpreting
        mrDocShell.PostPaint(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                             PaintPartFlags::Grid);
        aModificator.SetDocumentModified();

        bSuccess = true;
    }
    else if (!mbApi)
        mrDocShell.ErrorMessage(aTester.GetMessageId());

    return bSuccess;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
