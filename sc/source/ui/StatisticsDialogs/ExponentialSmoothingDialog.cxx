/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sfx2/dispatch.hxx>
#include <svl/zforlist.hxx>
#include <svl/undo.hxx>

#include "formulacell.hxx"
#include "rangelst.hxx"
#include "scitems.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "reffact.hxx"
#include "strload.hxx"
#include "random.hxx"
#include "docfunc.hxx"
#include "StatisticsDialogs.hrc"
#include "TableFillingAndNavigationTools.hxx"

#include "ExponentialSmoothingDialog.hxx"

ScExponentialSmoothingDialog::ScExponentialSmoothingDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData ) :
    ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "ExponentialSmoothingDialog", "modules/scalc/ui/exponentialsmoothingdialog.ui" )
{
    get(mpSmoothingFactor, "smoothing-factor-spin");
}

ScExponentialSmoothingDialog::~ScExponentialSmoothingDialog()
{}

sal_Bool ScExponentialSmoothingDialog::Close()
{
    return DoClose( ScExponentialSmoothingDialogWrapper::GetChildWindowId() );
}

void ScExponentialSmoothingDialog::CalculateInputAndWriteToOutput( )
{
    OUString aUndo(SC_STRLOAD(RID_STATISTICS_DLGS, STR_EXPONENTIAL_SMOOTHING_UNDO_NAME));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    ScAddress aStart = mInputRange.aStart;
    ScAddress aEnd   = mInputRange.aEnd;

    AddressWalkerWriter output(mOutputAddress, pDocShell, mDocument);
    FormulaTemplate aTemplate(mDocument, mAddressDetails);

    SCROW inTab = aStart.Tab();

    // Smoothing factor
    double aSmoothingFactor = mpSmoothingFactor->GetValue() / 100.0;

    ScAddress aSmoothingFactorAddress = output.current();
    output.writeValue(aSmoothingFactor);
    output.nextRow();

    // Exponential Smoothing
    output.push();

    for (SCCOL inCol = aStart.Col(); inCol <= aEnd.Col(); inCol++)
    {
        output.resetRow();

        SCROW inRow = aStart.Row();

        if (false)
        {
            ScRange aColumnRange (
                    ScAddress(inCol, mInputRange.aStart.Row(), inTab),
                    ScAddress(inCol, mInputRange.aEnd.Row(), inTab));

            aTemplate.setTemplate("=AVERAGE(%RANGE%)");
            aTemplate.applyRange("%RANGE%", aColumnRange);
            output.writeFormula(aTemplate.getTemplate());
        }
        else
        {
            ScAddress aFirstValueAddress(inCol, mInputRange.aStart.Row(), inTab);

            aTemplate.setTemplate("=%VAR%");
            aTemplate.applyAddress("%VAR%", aFirstValueAddress);
            output.writeFormula(aTemplate.getTemplate());
        }

        output.nextRow();

        for (inRow = aStart.Row() + 1; inRow <= aEnd.Row(); inRow++)
        {
            aTemplate.setTemplate("=%VALUE% * %PREVIOUS_INPUT% + (1 - %VALUE%) * %PREVIOUS_OUTPUT%");
            aTemplate.applyAddress("%PREVIOUS_INPUT%",  ScAddress(inCol, inRow - 1, inTab));
            aTemplate.applyAddress("%PREVIOUS_OUTPUT%", output.current(0, -1));
            aTemplate.applyAddress("%VALUE%",           aSmoothingFactorAddress);

            output.writeFormula(aTemplate.getTemplate());
            output.nextRow();
        }
        output.nextColumn();
    }

    pUndoManager->LeaveListAction();
    ScRange aOutputRange(output.mMinimumAddress, output.mMaximumAddress);
    pDocShell->PostPaint( aOutputRange, PAINT_GRID );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
