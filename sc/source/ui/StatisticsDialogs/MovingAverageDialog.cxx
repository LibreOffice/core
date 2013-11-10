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

#include "MovingAverageDialog.hxx"

ScMovingAverageDialog::ScMovingAverageDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData ) :
    ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "MovingAverageDialog", "modules/scalc/ui/movingaveragedialog.ui" )
{
    get(mpIntervalSpin, "interval-spin");
}

ScMovingAverageDialog::~ScMovingAverageDialog()
{}

sal_Bool ScMovingAverageDialog::Close()
{
    return DoClose( ScMovingAverageDialogWrapper::GetChildWindowId() );
}

void ScMovingAverageDialog::CalculateInputAndWriteToOutput( )
{
    OUString aUndo(SC_STRLOAD(RID_STATISTICS_DLGS, STR_MOVING_AVERAGE_UNDO_NAME));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    AddressWalkerWriter output(mOutputAddress, pDocShell, mDocument);
    FormulaTemplate aTemplate(mDocument, mAddressDetails);

    boost::scoped_ptr<DataRangeIterator> pIterator;
    if (mGroupedBy == BY_COLUMN)
        pIterator.reset(new DataRangeByColumnIterator(mInputRange));
    else
        pIterator.reset(new DataRangeByRowIterator(mInputRange));

    sal_Int32 aIntervalSize = mpIntervalSpin->GetValue();
    bool aCentral = true;

    for( ; pIterator->hasNext(); pIterator->next() )
    {
        output.resetRow();

        // Write column label
        aTemplate.setTemplate("Column %NUMBER%");
        aTemplate.applyNumber("%NUMBER%", pIterator->index() + 1);
        output.writeString(aTemplate.getTemplate());
        output.nextRow();

        DataCellIterator aDataCellIterator = pIterator->iterateCells();

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
                output.writeFormula(aTemplate.getTemplate());
            }
            else
            {
                output.writeString("N/A");
            }
            output.nextRow();
        }
        output.nextColumn();
    }

    pUndoManager->LeaveListAction();
    ScRange aOutputRange(output.mMinimumAddress, output.mMaximumAddress);
    pDocShell->PostPaint( aOutputRange, PAINT_GRID );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
