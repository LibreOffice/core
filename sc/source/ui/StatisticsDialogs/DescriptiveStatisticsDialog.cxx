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

#include <boost/random.hpp>

#include "DescriptiveStatisticsDialog.hxx"

namespace
{

struct StatisticCalculation {
    sal_Int16 aCalculationNameId;
    const char*     aFormula;
};

static const StatisticCalculation lclCalcDefinitions[] =
{
    { SID_CALC_MEAN,           "=AVERAGE(%RANGE%)" },
    { SID_CALC_STD_ERROR,      "=SQRT(VAR(%RANGE%)/COUNT(%RANGE%))"},
    { SID_CALC_MODE,           "=MODE(%RANGE%)"},
    { SID_CALC_MEDIAN,         "=MEDIAN(%RANGE%)"},
    { SID_CALC_VARIANCE,       "=VAR(%RANGE%)"},
    { SID_CALC_STD_DEVIATION,  "=STDEV(%RANGE%)"},
    { SID_CALC_KURTOSIS,       "=KURT(%RANGE%)"},
    { SID_CALC_SKEWNESS,       "=SKEW(%RANGE%)"},
    { SID_CALC_RANGE,          "=MAX(%RANGE%)-MIN(%RANGE%)"},
    { SID_CALC_MIN,            "=MIN(%RANGE%)"},
    { SID_CALC_MAX,            "=MAX(%RANGE%)"},
    { SID_CALC_SUM,            "=SUM(%RANGE%)"},
    { SID_CALC_COUNT,          "=COUNT(%RANGE%)" },
    { 0,                       NULL }
};

static const OUString lclWildcardRange("%RANGE%");

}

ScDescriptiveStatisticsDialog::ScDescriptiveStatisticsDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData ) :
    ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "DescriptiveStatisticsDialog", "modules/scalc/ui/descriptivestatisticsdialog.ui" )
{}

ScDescriptiveStatisticsDialog::~ScDescriptiveStatisticsDialog()
{}

sal_Bool ScDescriptiveStatisticsDialog::Close()
{
    return DoClose( ScDescriptiveStatisticsDialogWrapper::GetChildWindowId() );
}

void ScDescriptiveStatisticsDialog::CalculateInputAndWriteToOutput( )
{
    OUString aUndo(SC_RESSTR(STR_DESCRIPTIVE_STATISTICS_UNDO_NAME));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    ScAddress aStart = mInputRange.aStart;
    ScAddress aEnd   = mInputRange.aEnd;

    SCTAB outTab = mOutputAddress.Tab();
    SCCOL outCol = mOutputAddress.Col();
    SCROW outRow = mOutputAddress.Row();

    OUString aReferenceString;
    ScAddress aAddress;

    for (SCROW inTab = aStart.Tab(); inTab <= aEnd.Tab(); inTab++)
    {
        outCol = mOutputAddress.Col();

        for(sal_Int32 i = 0; lclCalcDefinitions[i].aFormula != NULL; i++)
        {
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aCalculationName(SC_RESSTR(lclCalcDefinitions[i].aCalculationNameId));
            pDocShell->GetDocFunc().SetStringCell(aAddress, aCalculationName, true);
            outRow++;
        }
        outCol++;

        for (SCCOL inCol = aStart.Col(); inCol <= aEnd.Col(); inCol++)
        {
            outRow = mOutputAddress.Row();
            ScRange aColumnRange (
                ScAddress(inCol, mInputRange.aStart.Row(), inTab),
                ScAddress(inCol, mInputRange.aEnd.Row(), inTab)
            );

            aReferenceString = aColumnRange.Format(SCR_ABS, mDocument, mAddressDetails);
            OUString aFormulaString;
            OUString aFormulaTemplate;

            for(sal_Int32 i = 0; lclCalcDefinitions[i].aFormula != NULL; i++)
            {
                aAddress = ScAddress(outCol, outRow, outTab);
                aFormulaTemplate = OUString::createFromAscii(lclCalcDefinitions[i].aFormula);
                aFormulaString = aFormulaTemplate.replaceAll(lclWildcardRange, aReferenceString);
                pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
                outRow++;
            }
            outCol++;
        }
        outTab++;
    }

    ScRange aOutputRange(mOutputAddress, ScAddress(outTab, outRow, outTab) );
    pUndoManager->LeaveListAction();
    pDocShell->PostPaint( aOutputRange, PAINT_GRID );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
