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

#include "ZTestDialog.hxx"

ScZTestDialog::ScZTestDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    vcl::Window* pParent, ScViewData* pViewData ) :
    ScStatisticsTwoVariableDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "ZTestDialog", "modules/scalc/ui/ztestdialog.ui" )
{
    SetText(SC_RESSTR(STR_ZTEST));
}

ScZTestDialog::~ScZTestDialog()
{}

bool ScZTestDialog::Close()
{
    return DoClose( ScZTestDialogWrapper::GetChildWindowId() );
}

sal_Int16 ScZTestDialog::GetUndoNameId()
{
    return STR_ZTEST_UNDO_NAME;
}

ScRange ScZTestDialog::ApplyOutput(ScDocShell* pDocShell)
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

    aOutput.writeBoldString(SC_RESSTR(STR_ZTEST));
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

    // Variable Label
    aOutput.nextColumn();
    aOutput.writeBoldString(SC_RESSTR(STR_VARIABLE_1_LABEL));
    aOutput.nextColumn();
    aOutput.writeBoldString(SC_RESSTR(STR_VARIABLE_2_LABEL));
    aOutput.newLine();

    // Known Variance
    aOutput.writeString(SC_RESSTR(STR_ZTEST_KNOWN_VARIANCE));
    aOutput.nextColumn();
    aOutput.writeValue(0);
    aTemplate.autoReplaceAddress("%KNOWN_VARIANCE_VARIABLE1%", aOutput.current());
    aOutput.nextColumn();
    aOutput.writeValue(0);
    aTemplate.autoReplaceAddress("%KNOWN_VARIANCE_VARIABLE2%", aOutput.current());
    aOutput.newLine();

    // Mean
    aOutput.writeString(SC_RESSTR(STRID_CALC_MEAN));
    aOutput.nextColumn();
    aTemplate.setTemplate("=AVERAGE(%VARIABLE1_RANGE%)");
    aTemplate.autoReplaceAddress("%MEAN_VARIABLE1%", aOutput.current());
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextColumn();
    aTemplate.setTemplate("=AVERAGE(%VARIABLE2_RANGE%)");
    aTemplate.autoReplaceAddress("%MEAN_VARIABLE2%", aOutput.current());
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // Observations
    aOutput.writeString(SC_RESSTR(STR_OBSERVATIONS_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=COUNT(%VARIABLE1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%OBSERVATION_VARIABLE1%", aOutput.current());
    aOutput.nextColumn();
    aTemplate.setTemplate("=COUNT(%VARIABLE2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%OBSERVATION_VARIABLE2%", aOutput.current());
    aOutput.newLine();

    // Observed mean difference
    aOutput.writeString(SC_RESSTR(STR_OBSERVED_MEAN_DIFFERENCE_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=%MEAN_VARIABLE1% - %MEAN_VARIABLE2%");
    aOutput.writeMatrixFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%OBSERVED_MEAN_DIFFERENCE%", aOutput.current());
    aOutput.newLine();

    // z
    aOutput.writeString(SC_RESSTR(STR_ZTEST_Z_VALUE));
    aOutput.nextColumn();
    aTemplate.setTemplate("=(%OBSERVED_MEAN_DIFFERENCE% - %HYPOTHESIZED_MEAN_DIFFERENCE%) / SQRT( %KNOWN_VARIANCE_VARIABLE1% / %OBSERVATION_VARIABLE1% + %KNOWN_VARIANCE_VARIABLE2% / %OBSERVATION_VARIABLE2% )");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%Z_STAT%", aOutput.current());
    aOutput.newLine();

    // P one-tail
    aOutput.writeString(SC_RESSTR(STR_ZTEST_P_ONE_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=1 - NORMSDIST(ABS(%Z_STAT%))");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // z critical one-tail
    aOutput.writeString(SC_RESSTR(STR_ZTEST_Z_CRITICAL_ONE_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=-NORMSINV(%ALPHA%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // P two-tail
    aOutput.writeString(SC_RESSTR(STR_ZTEST_P_TWO_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=2 * NORMSDIST(-ABS(%Z_STAT%))");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // z critical two-tail
    aOutput.writeString(SC_RESSTR(STR_ZTEST_Z_CRITICAL_TWO_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=-NORMSINV(%ALPHA%/2)");
    aOutput.writeFormula(aTemplate.getTemplate());

    return ScRange(aOutput.mMinimumAddress, aOutput.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
