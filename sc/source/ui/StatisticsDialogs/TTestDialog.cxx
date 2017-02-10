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
#include "docfunc.hxx"
#include "TableFillingAndNavigationTools.hxx"

#include "TTestDialog.hxx"

ScTTestDialog::ScTTestDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    vcl::Window* pParent, ScViewData* pViewData ) :
    ScStatisticsTwoVariableDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "TTestDialog", "modules/scalc/ui/ttestdialog.ui" )
{
    SetText(SC_RESSTR(STR_TTEST));
}

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
    FormulaTemplate aTemplate(mDocument);

    std::unique_ptr<DataRangeIterator> pVariable1Iterator;
    if (mGroupedBy == BY_COLUMN)
        pVariable1Iterator.reset(new DataRangeByColumnIterator(mVariable1Range));
    else
        pVariable1Iterator.reset(new DataRangeByRowIterator(mVariable1Range));

    std::unique_ptr<DataRangeIterator> pVariable2Iterator;
    if (mGroupedBy == BY_COLUMN)
        pVariable2Iterator.reset(new DataRangeByColumnIterator(mVariable2Range));
    else
        pVariable2Iterator.reset(new DataRangeByRowIterator(mVariable2Range));

    aTemplate.autoReplaceRange("%VARIABLE1_RANGE%", pVariable1Iterator->get());
    aTemplate.autoReplaceRange("%VARIABLE2_RANGE%", pVariable2Iterator->get());

    aOutput.writeBoldString(SC_RESSTR(STR_TTEST_UNDO_NAME));
    aOutput.newLine();

    // Alpha
    aOutput.writeString(SC_RESSTR(STR_LABEL_ALPHA));
    aOutput.nextColumn();
    aOutput.writeValue(0.05);
    aTemplate.autoReplaceAddress("%ALPHA%", aOutput.current());
    aOutput.newLine();

    // Hypothesized mean difference
    aOutput.writeString(SC_RESSTR(STR_HYPOTHESIZED_MEAN_DIFFERENCE_LABEL));
    aOutput.nextColumn();
    aOutput.writeValue(0);
    aTemplate.autoReplaceAddress("%HYPOTHESIZED_MEAN_DIFFERENCE%", aOutput.current());
    aOutput.newLine();

    aOutput.nextColumn();
    aOutput.writeBoldString(SC_RESSTR(STR_VARIABLE_1_LABEL));
    aOutput.nextColumn();
    aOutput.writeBoldString(SC_RESSTR(STR_VARIABLE_2_LABEL));
    aOutput.newLine();

    aOutput.writeString(SC_RESSTR(STRID_CALC_MEAN));
    aOutput.nextColumn();
    aTemplate.setTemplate("=AVERAGE(%VARIABLE1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextColumn();
    aTemplate.setTemplate("=AVERAGE(%VARIABLE2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(SC_RESSTR(STRID_CALC_VARIANCE));
    aOutput.nextColumn();
    aTemplate.setTemplate("=VAR(%VARIABLE1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextColumn();
    aTemplate.setTemplate("=VAR(%VARIABLE2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // Observations
    aOutput.writeString(SC_RESSTR(STR_OBSERVATIONS_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=COUNT(%VARIABLE1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextColumn();
    aTemplate.setTemplate("=COUNT(%VARIABLE2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // Pearson Correlation
    aOutput.writeString(SC_RESSTR(STR_TTEST_PEARSON_CORRELATION));
    aOutput.nextColumn();
    aTemplate.setTemplate("=CORREL(%VARIABLE1_RANGE%;%VARIABLE2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // Observed mean difference
    aOutput.writeString(SC_RESSTR(STR_OBSERVED_MEAN_DIFFERENCE_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=AVERAGE(IF(ISODD(IF(ISNUMBER(%VARIABLE1_RANGE%); 1; 0) * IF(ISNUMBER(%VARIABLE2_RANGE%); 1; 0)); %VARIABLE1_RANGE% - %VARIABLE2_RANGE%; \"NA\"))");
    aOutput.writeMatrixFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%OBSERVED_MEAN_DIFFERENCE%", aOutput.current());
    aOutput.newLine();

    // Variance of the Differences
    aOutput.writeString(SC_RESSTR(STR_TTEST_VARIANCE_OF_THE_DIFFERENCES));
    aOutput.nextColumn();
    aTemplate.setTemplate("=VAR(IF(ISODD(IF(ISNUMBER(%VARIABLE1_RANGE%); 1; 0) * IF(ISNUMBER(%VARIABLE2_RANGE%); 1; 0)); %VARIABLE1_RANGE% - %VARIABLE2_RANGE%; \"NA\"))");
    aOutput.writeMatrixFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VARIANCE_OF_DIFFERENCES%", aOutput.current());
    aOutput.newLine();

    // df
    aOutput.writeString(SC_RESSTR(STR_ANOVA_LABEL_DF));
    aOutput.nextColumn();
    aTemplate.setTemplate("=SUM(IF(ISNUMBER(%VARIABLE1_RANGE%); 1; 0) * IF(ISNUMBER(%VARIABLE2_RANGE%); 1; 0)) - 1");
    aOutput.writeMatrixFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%DEGREE_OF_FREEDOM%", aOutput.current());
    aOutput.newLine();

    // t stat
    aOutput.writeString(SC_RESSTR(STR_TTEST_T_STAT));
    aOutput.nextColumn();
    aTemplate.setTemplate("=(%OBSERVED_MEAN_DIFFERENCE% - %HYPOTHESIZED_MEAN_DIFFERENCE%) / (%VARIANCE_OF_DIFFERENCES% / ( %DEGREE_OF_FREEDOM% + 1)) ^ 0.5");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%T_STAT%", aOutput.current());
    aOutput.newLine();

    // P one-tail
    aOutput.writeString(SC_RESSTR(STR_TTEST_P_ONE_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=TDIST(ABS(%T_STAT%); %DEGREE_OF_FREEDOM%; 1)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // T critical one-tail
    aOutput.writeString(SC_RESSTR(STR_TTEST_T_CRITICAL_ONE_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=TINV(2*%ALPHA%; %DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // P two-tail
    aOutput.writeString(SC_RESSTR(STR_TTEST_P_TWO_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=TDIST(ABS(%T_STAT%); %DEGREE_OF_FREEDOM%; 2)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // T critical two-tail
    aOutput.writeString(SC_RESSTR(STR_TTEST_T_CRITICAL_TWO_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=TINV(%ALPHA%; %DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());

    return ScRange(aOutput.mMinimumAddress, aOutput.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
