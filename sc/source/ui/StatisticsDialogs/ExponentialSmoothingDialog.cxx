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

    AddressWalkerWriter output(mOutputAddress, pDocShell, mDocument);
    FormulaTemplate aTemplate(mDocument, mAddressDetails);

    // Smoothing factor
    double aSmoothingFactor = mpSmoothingFactor->GetValue() / 100.0;

    ScAddress aSmoothingFactorAddress = output.current();
    output.writeValue(aSmoothingFactor);
    output.nextRow();

    // Exponential Smoothing
    output.push();

    boost::scoped_ptr<DataRangeIterator> pIterator;
    if (mGroupedBy == BY_COLUMN)
        pIterator.reset(new DataRangeByColumnIterator(mInputRange));
    else
        pIterator.reset(new DataRangeByRowIterator(mInputRange));

    for( ; pIterator->hasNext(); pIterator->next() )
    {
        output.resetRow();

        ScRange aCurrentRange = pIterator->get();

        if (false)
        {
            aTemplate.setTemplate("=AVERAGE(%RANGE%)");
            aTemplate.applyRange("%RANGE%", aCurrentRange);
            output.writeFormula(aTemplate.getTemplate());
        }
        else
        {
            aTemplate.setTemplate("=%VAR%");
            aTemplate.applyAddress("%VAR%", aCurrentRange.aStart);
            output.writeFormula(aTemplate.getTemplate());
        }

        output.nextRow();

        DataCellIterator aDataCellIterator = pIterator->iterateCells();

        for (; aDataCellIterator.hasNext(); aDataCellIterator.next())
        {
            aTemplate.setTemplate("=%VALUE% * %PREVIOUS_INPUT% + (1 - %VALUE%) * %PREVIOUS_OUTPUT%");
            aTemplate.applyAddress("%PREVIOUS_INPUT%",  aDataCellIterator.get());
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
