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
#include <boost/random.hpp>
#include <boost/scoped_ptr.hpp>

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

#include "TTestDialog.hxx"

namespace
{

static const OUString strWildcardVariable1Range("%VAR1_RANGE%");
static const OUString strWildcardVariable2Range("%VAR2_RANGE%");

}

ScTTestDialog::ScTTestDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData ) :
    ScStatisticsTwoVariableDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "TTestDialog", "modules/scalc/ui/ttestdialog.ui" )
{}

ScTTestDialog::~ScTTestDialog()
{}

bool ScTTestDialog::Close()
{
    return DoClose( ScTTestDialogWrapper::GetChildWindowId() );
}

sal_Int16 ScTTestDialog::GetUndoNameId()
{
    return STR_TTEST_UNDO_NAME;
}

ScRange ScTTestDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter aOutput(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar( formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(mDocument, mAddressDetails);

    boost::scoped_ptr<DataRangeIterator> pVariable1Iterator;
    if (mGroupedBy == BY_COLUMN)
        pVariable1Iterator.reset(new DataRangeByColumnIterator(mVariable1Range));
    else
        pVariable1Iterator.reset(new DataRangeByRowIterator(mVariable1Range));

    boost::scoped_ptr<DataRangeIterator> pVariable2Iterator;
    if (mGroupedBy == BY_COLUMN)
        pVariable2Iterator.reset(new DataRangeByColumnIterator(mVariable2Range));
    else
        pVariable2Iterator.reset(new DataRangeByRowIterator(mVariable2Range));

    aTemplate.autoReplaceRange(strWildcardVariable1Range, pVariable1Iterator->get());
    aTemplate.autoReplaceRange(strWildcardVariable2Range, pVariable2Iterator->get());

    aOutput.nextRow();
    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STRID_CALC_MEAN));
    aOutput.nextRow();
    aOutput.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STRID_CALC_VARIANCE));
    aOutput.nextRow();
    aOutput.writeString("Observations");
    aOutput.nextRow();
    aOutput.writeString("Pearson Correlation");
    aOutput.nextRow();
    aOutput.writeString("Hypothesized Mean Difference");
    aOutput.nextRow();
    aOutput.writeString("Observed Mean Difference");
    aOutput.nextRow();
    aOutput.writeString("Variance of the Differences");
    aOutput.nextRow();
    aOutput.writeString("df");
    aOutput.nextRow();
    aOutput.writeString("t Stat");
    aOutput.nextRow();
    aOutput.writeString("P (T<=t) one-tail");
    aOutput.nextRow();
    aOutput.writeString("t Critical one-tail");
    aOutput.nextRow();
    aOutput.writeString("P (T<=t) two-tail");
    aOutput.nextRow();
    aOutput.writeString("t Critical two-tail");
    aOutput.resetRow();

    aOutput.nextColumn();

    aOutput.writeString("Variable 1");
    aOutput.nextRow();

    aTemplate.setTemplate("=AVERAGE(%VAR1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextRow();

    aTemplate.setTemplate("=VAR(%VAR1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextRow();

    aTemplate.setTemplate("=COUNT(%VAR1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextRow();

    aTemplate.setTemplate("=CORREL(%VAR1_RANGE%;%VAR2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextRow();

    aOutput.writeValue(2);
    aTemplate.autoReplaceAddress("%HYPOTHESIZED_MEAN_DIFFERENCE%", aOutput.current());
    aOutput.nextRow();

    aTemplate.setTemplate("=AVERAGE(IF(ISODD(IF(ISNUMBER(%VAR1_RANGE%); 1; 0) * IF(ISNUMBER(%VAR2_RANGE%); 1; 0)); %VAR1_RANGE% - %VAR2_RANGE%; \"NA\"))");
    aOutput.writeMatrixFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%OBSERVED_MEAN_DIFFERENCE%", aOutput.current());
    aOutput.nextRow();

    aTemplate.setTemplate("=VAR(IF(ISODD(IF(ISNUMBER(%VAR1_RANGE%); 1; 0) * IF(ISNUMBER(%VAR2_RANGE%); 1; 0)); %VAR1_RANGE% - %VAR2_RANGE%; \"NA\"))");
    aOutput.writeMatrixFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VARIANCE_OF_DIFFERENCES%", aOutput.current());
    aOutput.nextRow();

    aTemplate.setTemplate("=SUM(IF(ISNUMBER(%VAR1_RANGE%); 1; 0) * IF(ISNUMBER(%VAR2_RANGE%); 1; 0)) - 1");
    aOutput.writeMatrixFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%DEGREE_OF_FREEDOM%", aOutput.current());
    aOutput.nextRow();

    aTemplate.setTemplate("=(%OBSERVED_MEAN_DIFFERENCE% - %HYPOTHESIZED_MEAN_DIFFERENCE%) / (%VARIANCE_OF_DIFFERENCES% / ( %DEGREE_OF_FREEDOM% + 1)) ^ 0.5");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%T_STAT%", aOutput.current());
    aOutput.nextRow();

    aTemplate.setTemplate("=TDIST(ABS(%T_STAT%); %DEGREE_OF_FREEDOM%; 1)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextRow();

    aTemplate.setTemplate("=TINV(2*0.05; %DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextRow();

    aTemplate.setTemplate("=TDIST(ABS(%T_STAT%); %DEGREE_OF_FREEDOM%; 2)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextRow();

    aTemplate.setTemplate("=TINV(0.05; %DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextRow();

    aOutput.resetRow();

    aOutput.nextColumn();

    aOutput.writeString("Variable 2");
    aOutput.nextRow();

    aTemplate.setTemplate("=AVERAGE(%VAR2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextRow();

    aTemplate.setTemplate("=VAR(%VAR2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextRow();

    aTemplate.setTemplate("=COUNT(%VAR2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextRow();

    aOutput.resetRow();

    return ScRange(aOutput.mMinimumAddress, aOutput.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
