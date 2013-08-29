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
#include "scresid.hxx"
#include "random.hxx"
#include "docfunc.hxx"
#include "globstr.hrc"
#include "sc.hrc"

#include "CovarianceDialog.hxx"

namespace
{

static const OUString lclWildcardRow1("%ROW1%");
static const OUString lclWildcardRow2("%ROW2%");
static const OUString lclCovarTemplate("=COVAR(%ROW1%, %ROW2%)");
static const OUString lclWildcardNumber("%NUMBER%");
static const OUString lclRowLabelTemplate("Row %NUMBER%");

}

ScCovarianceDialog::ScCovarianceDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData ) :
    ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "DescriptiveStatisticsDialog", "modules/scalc/ui/descriptivestatisticsdialog.ui" )
{}

ScCovarianceDialog::~ScCovarianceDialog()
{}

sal_Bool ScCovarianceDialog::Close()
{
    return DoClose( ScCovarianceDialogWrapper::GetChildWindowId() );
}

void ScCovarianceDialog::CalculateInputAndWriteToOutput( )
{
    OUString aUndo(SC_RESSTR(STR_COVARIANCE_UNDO_NAME));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    ScAddress aStart = mInputRange.aStart;
    ScAddress aEnd   = mInputRange.aEnd;

    SCTAB outTab = mOutputAddress.Tab();
    SCCOL outCol = mOutputAddress.Col();
    SCROW outRow = mOutputAddress.Row();

    ScAddress aAddress;
    SCCOL aMaxCol = 0;
    SCROW aMaxRow = 0;

    for (SCTAB inTab = aStart.Tab(); inTab <= aEnd.Tab(); inTab++)
    {
        ScRangeList aRangeList = MakeRowRangeList(inTab, aStart, aEnd);

        // row labels
        aAddress = ScAddress(outCol, outRow, outTab);
        pDocShell->GetDocFunc().SetStringCell(aAddress, OUString("Covariances"), true);
        outCol++;
        aMaxCol = outCol > aMaxCol ? outCol : aMaxCol;

        // write labels to columns
        for (size_t i = 0; i < aRangeList.size(); i++)
        {
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aRowLabel = lclRowLabelTemplate.replaceAll(lclWildcardNumber, OUString::number(i + 1));
            pDocShell->GetDocFunc().SetStringCell(aAddress, aRowLabel, true);
            outCol++;
            aMaxCol = outCol > aMaxCol ? outCol : aMaxCol;
        }

        // write labels to rows
        outCol = mOutputAddress.Col();
        outRow++;
        for (size_t i = 0; i < aRangeList.size(); i++)
        {
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aRowLabel = lclRowLabelTemplate.replaceAll(lclWildcardNumber, OUString::number(i + 1));
            pDocShell->GetDocFunc().SetStringCell(aAddress, aRowLabel, true);
            outRow++;
            aMaxRow = outRow > aMaxRow ? outRow : aMaxRow;
        }

        // write correlation formulas
        aAddress = ScAddress(
                    mOutputAddress.Col() + 1,
                    mOutputAddress.Row() + 1,
                    inTab);

        WriteCovarianceFormulas(aAddress, aRangeList);
        outTab++;
    }
    ScAddress aLastAddress = ScAddress(
                                mOutputAddress.Col() + aMaxCol,
                                mOutputAddress.Row() + aMaxRow,
                                outTab);

    ScRange aOutputRange(mOutputAddress, aLastAddress);
    pUndoManager->LeaveListAction();
    pDocShell->PostPaint(aOutputRange, PAINT_GRID);
}

void ScCovarianceDialog::WriteCovarianceFormulas(ScAddress aOutputAddress, ScRangeList aRangeList)
{
    ScDocShell* pDocShell = mViewData->GetDocShell();
    ScAddress aAddress;

    SCTAB outTab = aOutputAddress.Tab();
    SCCOL outCol = aOutputAddress.Col();

    OUString aFormulaString;

    for (size_t i = 0; i < aRangeList.size(); i++)
    {
        SCROW outRow = aOutputAddress.Row();
        for (size_t j = 0; j < aRangeList.size(); j++)
        {
            if (j >= i)
            {
                OUString aRowString1(aRangeList[i]->Format(SCR_ABS, mDocument, mAddressDetails));
                OUString aRowString2(aRangeList[j]->Format(SCR_ABS, mDocument, mAddressDetails));

                aAddress = ScAddress(outCol, outRow, outTab);
                aFormulaString = lclCovarTemplate.replaceAll(lclWildcardRow1, aRowString1);
                aFormulaString = aFormulaString.replaceAll(lclWildcardRow2, aRowString2);
                pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
            }
            outRow++;
        }
        outCol++;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
