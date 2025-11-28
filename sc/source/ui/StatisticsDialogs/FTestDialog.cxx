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
#include <FTestDialog.hxx>
#include <scresid.hxx>
#include <strings.hrc>

ScFTestDialog::ScFTestDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    weld::Window* pParent, ScViewData& rViewData ) :
    ScStatisticsTwoVariableDialog(
            pSfxBindings, pChildWindow, pParent, rViewData,
            u"modules/scalc/ui/ttestdialog.ui"_ustr, u"TTestDialog"_ustr )
{
    m_xDialog->set_title(ScResId(STR_FTEST));
}

ScFTestDialog::~ScFTestDialog()
{}

void ScFTestDialog::Close()
{
    DoClose( ScFTestDialogWrapper::GetChildWindowId() );
}

TranslateId ScFTestDialog::GetUndoNameId()
{
    return STR_FTEST_UNDO_NAME;
}

ScRange ScFTestDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter aOutput(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar(formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(&mDocument);

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

    aTemplate.autoReplaceRange(u"%VARIABLE1_RANGE%"_ustr, pVariable1Iterator->get());
    aTemplate.autoReplaceRange(u"%VARIABLE2_RANGE%"_ustr, pVariable2Iterator->get());

    aOutput.writeBoldString(ScResId(STR_FTEST_UNDO_NAME));
    aOutput.newLine();

    // Alpha
    aOutput.writeString(ScResId(STR_LABEL_ALPHA));
    aOutput.nextColumn();
    aOutput.writeValue(0.05);
    aTemplate.autoReplaceAddress(u"%ALPHA%"_ustr, aOutput.current());
    aOutput.newLine();

    aOutput.nextColumn();
    aOutput.writeBoldString(ScResId(STR_VARIABLE_1_LABEL));
    aOutput.nextColumn();
    aOutput.writeBoldString(ScResId(STR_VARIABLE_2_LABEL));
    aOutput.newLine();

    aOutput.writeString(ScResId(STRID_CALC_MEAN));
    aOutput.nextColumn();
    aTemplate.setTemplate("=AVERAGE(%VARIABLE1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextColumn();
    aTemplate.setTemplate("=AVERAGE(%VARIABLE2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(ScResId(STRID_CALC_VARIANCE));
    aOutput.nextColumn();
    aTemplate.setTemplate("=VAR(%VARIABLE1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress(u"%VARIABLE1_VARIANCE%"_ustr, aOutput.current());
    aOutput.nextColumn();
    aTemplate.setTemplate("=VAR(%VARIABLE2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress(u"%VARIABLE2_VARIANCE%"_ustr, aOutput.current());
    aOutput.newLine();

    aOutput.writeString(ScResId(STR_OBSERVATIONS_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=COUNT(%VARIABLE1_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress(u"%VARIABLE1_OBSERVATIONS%"_ustr, aOutput.current());
    aOutput.nextColumn();
    aTemplate.setTemplate("=COUNT(%VARIABLE2_RANGE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress(u"%VARIABLE2_OBSERVATIONS%"_ustr, aOutput.current());
    aOutput.newLine();

    aOutput.writeString(ScResId(STR_ANOVA_LABEL_DF));
    aOutput.nextColumn();
    aTemplate.setTemplate("=%VARIABLE1_OBSERVATIONS% - 1");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress(u"%VARIABLE1_DEGREE_OF_FREEDOM%"_ustr, aOutput.current());
    aOutput.nextColumn();
    aTemplate.setTemplate("=%VARIABLE2_OBSERVATIONS% - 1");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress(u"%VARIABLE2_DEGREE_OF_FREEDOM%"_ustr, aOutput.current());
    aOutput.newLine();

    aOutput.writeString(ScResId(STR_ANOVA_LABEL_F));
    aOutput.nextColumn();
    aTemplate.setTemplate("=%VARIABLE1_VARIANCE% / %VARIABLE2_VARIANCE%");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress(u"%F_VALUE%"_ustr, aOutput.current());
    aOutput.newLine();

    aOutput.writeString(ScResId(STR_FTEST_P_RIGHT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FDIST(%F_VALUE%; %VARIABLE1_DEGREE_OF_FREEDOM%; %VARIABLE2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress(u"%P_RIGHT_TAIL_VALUE%"_ustr, aOutput.current());
    aOutput.newLine();

    aOutput.writeString(ScResId(STR_FTEST_F_CRITICAL_RIGHT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(%ALPHA%; %VARIABLE1_DEGREE_OF_FREEDOM%; %VARIABLE2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(ScResId(STR_FTEST_P_LEFT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=1 - %P_RIGHT_TAIL_VALUE%");
    aOutput.writeFormula(aTemplate.getTemplate());
    aTemplate.autoReplaceAddress(u"%P_LEFT_TAIL_VALUE%"_ustr, aOutput.current());
    aOutput.newLine();

    aOutput.writeString(ScResId(STR_FTEST_F_CRITICAL_LEFT_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(1-%ALPHA%; %VARIABLE1_DEGREE_OF_FREEDOM%; %VARIABLE2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(ScResId(STR_FTEST_P_TWO_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=2*MIN(%P_RIGHT_TAIL_VALUE%; %P_LEFT_TAIL_VALUE%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    aOutput.writeString(ScResId(STR_FTEST_F_CRITICAL_TWO_TAIL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(1-(%ALPHA%/2); %VARIABLE1_DEGREE_OF_FREEDOM%; %VARIABLE2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.nextColumn();
    aTemplate.setTemplate("=FINV(%ALPHA%/2; %VARIABLE1_DEGREE_OF_FREEDOM%; %VARIABLE2_DEGREE_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());

    return ScRange(aOutput.mMinimumAddress, aOutput.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
