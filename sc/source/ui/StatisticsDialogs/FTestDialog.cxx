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

#include "FTestDialog.hxx"

ScFTestDialog::ScFTestDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    vcl::Window* pParent, ScViewData* pViewData ) :
    ScStatisticsTwoVariableDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "TTestDialog", "modules/scalc/ui/ttestdialog.ui" )
{
    SetText(SC_RESSTR(STR_FTEST));
}

ScFTestDialog::~ScFTestDialog()
{}

bool ScFTestDialog::Close()
{
    return DoClose( ScFTestDialogWrapper::GetChildWindowId() );
}

sal_Int16 ScFTestDialog::GetUndoNameId()
{
    return STR_FTEST_UNDO_NAME;
}

ScRange ScFTestDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter aOutput(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar(formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
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

    aOutput.writeBoldString(SC_RESSTR(STR_FTEST_UNDO_NAME));
    aOutput.newLine();

    // Alpha
    aOutput.writeString(SC_RESSTR(STR_LABEL_ALPHA));
    aOutput.nextColumn();
    aOutput.writeValue(0.05);
    aTemplate.autoReplaceAddress("%ALPHA%", aOutput.current());
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
    aTemplate.autoReplaceAddress("%VARIABLE1_VARIANCE%", aOutput.current());
    aOutput.nextColumn();
    aTemplate.setTemplate("=VAR(%VARIABLE2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VARIABLE2_VARIANCE%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_RESSTR(STR_OBSERVATIONS_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=COUNT(%VARIABLE1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VARIABLE1_OBSERVATIONS%", aOutput.current());
    aOutput.nextColumn();
    aTemplate.setTemplate("=COUNT(%VARIABLE2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VARIABLE2_OBSERVATIONS%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_RESSTR(STR_ANOVA_LABEL_DF));
    aOutput.nextColumn();
    aTemplate.setTemplate("=%VARIABLE1_OBSERVATIONS% - 1");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VARIABLE1_DEGREE_OF_FREEDOM%", aOutput.current());
    aOutput.nextColumn();
    aTemplate.setTemplate("=%VARIABLE2_OBSERVATIONS% - 1");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%VARIABLE2_DEGREE_OF_FREEDOM%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_RESSTR(STR_ANOVA_LABEL_F));
    aOutput.nextColumn();
    aTemplate.setTemplate("=%VARIABLE1_VARIANCE% / %VARIABLE2_VARIANCE%");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%F_VALUE%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_RESSTR(STR_FTEST_P_RIGHT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FDIST(%F_VALUE%; %VARIABLE1_DEGREE_OF_FREEDOM%; %VARIABLE2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%P_RIGHT_TAIL_VALUE%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_RESSTR(STR_FTEST_F_CRITICAL_RIGHT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(%ALPHA%; %VARIABLE1_DEGREE_OF_FREEDOM%; %VARIABLE2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(SC_RESSTR(STR_FTEST_P_LEFT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=1 - %P_RIGHT_TAIL_VALUE%");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress("%P_LEFT_TAIL_VALUE%", aOutput.current());
    aOutput.newLine();

    aOutput.writeString(SC_RESSTR(STR_FTEST_F_CRITICAL_LEFT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(1-%ALPHA%; %VARIABLE1_DEGREE_OF_FREEDOM%; %VARIABLE2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(SC_RESSTR(STR_FTEST_P_TWO_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=2*MIN(%P_RIGHT_TAIL_VALUE%; %P_LEFT_TAIL_VALUE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(SC_RESSTR(STR_FTEST_F_CRITICAL_TWO_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(1-(%ALPHA%/2); %VARIABLE1_DEGREE_OF_FREEDOM%; %VARIABLE2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(%ALPHA%/2; %VARIABLE1_DEGREE_OF_FREEDOM%; %VARIABLE2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());

    return ScRange(aOutput.mMinimumAddress, aOutput.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
