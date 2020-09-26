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
#include <MovingAverageDialog.hxx>
#include <scresid.hxx>
#include <strings.hrc>

ScMovingAverageDialog::ScMovingAverageDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    weld::Window* pParent, ScViewData& rViewData )
    : ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, rViewData,
            "modules/scalc/ui/movingaveragedialog.ui",
            "MovingAverageDialog")
    , mxTrimRangeCheck(m_xBuilder->weld_check_button("trimrange-check"))
    , mxIntervalSpin(m_xBuilder->weld_spin_button("interval-spin"))
{
}

ScMovingAverageDialog::~ScMovingAverageDialog()
{
}

void ScMovingAverageDialog::Close()
{
    DoClose( ScMovingAverageDialogWrapper::GetChildWindowId() );
}

const char* ScMovingAverageDialog::GetUndoNameId()
{
    return STR_MOVING_AVERAGE_UNDO_NAME;
}

ScRange ScMovingAverageDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter output(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar( formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(&mDocument);

    if (mxTrimRangeCheck->get_active())
        mDocument.GetDataAreaSubrange(mInputRange);

    std::unique_ptr<DataRangeIterator> pIterator;
    if (mGroupedBy == BY_COLUMN)
        pIterator.reset(new DataRangeByColumnIterator(mInputRange));
    else
        pIterator.reset(new DataRangeByRowIterator(mInputRange));

    sal_Int32 aIntervalSize = mxIntervalSpin->get_value();
    const bool aCentral = true; //to-do add support to change this to the dialog

    for( ; pIterator->hasNext(); pIterator->next() )
    {
        output.resetRow();

        // Write label
        if (mGroupedBy == BY_COLUMN)
            aTemplate.setTemplate(ScResId(STR_COLUMN_LABEL_TEMPLATE));
        else
            aTemplate.setTemplate(ScResId(STR_ROW_LABEL_TEMPLATE));

        aTemplate.applyNumber("%NUMBER%", pIterator->index() + 1);
        output.writeBoldString(aTemplate.getTemplate());
        output.nextRow();

        DataCellIterator aDataCellIterator = pIterator->iterateCells();
        std::vector<OUString> aFormulas;

        for (; aDataCellIterator.hasNext(); aDataCellIterator.next())
        {
            ScAddress aIntervalStart;
            ScAddress aIntervalEnd;

            if (aCentral)
            {
                sal_Int32 aHalf = aIntervalSize / 2;
                sal_Int32 aHalfRemainder = aIntervalSize % 2;
                aIntervalStart = aDataCellIterator.getRelative(-aHalf);
                aIntervalEnd = aDataCellIterator.getRelative(aHalf - 1 + aHalfRemainder);
            }
            else
            {
                aIntervalStart = aDataCellIterator.getRelative(-aIntervalSize);
                aIntervalEnd = aDataCellIterator.getRelative(0);
            }

            if(aIntervalStart.IsValid() && aIntervalEnd.IsValid())
            {
                aTemplate.setTemplate("=AVERAGE(%RANGE%)");
                aTemplate.applyRange("%RANGE%", ScRange(aIntervalStart, aIntervalEnd));
                aFormulas.push_back(aTemplate.getTemplate());
            }
            else
            {
                aFormulas.push_back("=#N/A");
            }
        }

        output.writeFormulas(aFormulas);
        output.nextColumn();
    }
    return ScRange(output.mMinimumAddress, output.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
