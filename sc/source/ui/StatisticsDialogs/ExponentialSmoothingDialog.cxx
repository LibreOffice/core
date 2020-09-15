/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>

#include <reffact.hxx>
#include <TableFillingAndNavigationTools.hxx>
#include <ExponentialSmoothingDialog.hxx>
#include <scresid.hxx>
#include <strings.hrc>

ScExponentialSmoothingDialog::ScExponentialSmoothingDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    weld::Window* pParent, ScViewData* pViewData )
    : ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "modules/scalc/ui/exponentialsmoothingdialog.ui",
            "ExponentialSmoothingDialog")
    , mxSmoothingFactor(m_xBuilder->weld_spin_button("smoothing-factor-spin"))
{
}

ScExponentialSmoothingDialog::~ScExponentialSmoothingDialog()
{
}

void ScExponentialSmoothingDialog::Close()
{
    DoClose( ScExponentialSmoothingDialogWrapper::GetChildWindowId() );
}

const char* ScExponentialSmoothingDialog::GetUndoNameId()
{
    return STR_EXPONENTIAL_SMOOTHING_UNDO_NAME;
}

ScRange ScExponentialSmoothingDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter output(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar( formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(&mDocument);

    // Smoothing factor
    double aSmoothingFactor = mxSmoothingFactor->get_value() / 100.0;

    // Alpha
    output.writeBoldString(ScResId(STR_LABEL_ALPHA));
    output.nextRow();

    // Alpha Value
    ScAddress aSmoothingFactorAddress = output.current();
    output.writeValue(aSmoothingFactor);
    output.nextRow();

    // Exponential Smoothing
    output.push();

    std::unique_ptr<DataRangeIterator> pIterator;
    if (mGroupedBy == BY_COLUMN)
        pIterator.reset(new DataRangeByColumnIterator(mInputRange));
    else
        pIterator.reset(new DataRangeByRowIterator(mInputRange));

    for( ; pIterator->hasNext(); pIterator->next() )
    {
        output.resetRow();

        ScRange aCurrentRange = pIterator->get();

        // Write column label
        if (mGroupedBy == BY_COLUMN)
            aTemplate.setTemplate(ScResId(STR_COLUMN_LABEL_TEMPLATE));
        else
            aTemplate.setTemplate(ScResId(STR_ROW_LABEL_TEMPLATE));
        aTemplate.applyNumber("%NUMBER%", pIterator->index() + 1);
        output.writeBoldString(aTemplate.getTemplate());
        output.nextRow();

        // Initial value
        if ((false))
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

    return ScRange (output.mMinimumAddress, output.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
