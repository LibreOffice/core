/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <reffact.hxx>
#include <TableFillingAndNavigationTools.hxx>
#include <ChiSquareTestDialog.hxx>
#include <scresid.hxx>
#include <strings.hrc>

ScChiSquareTestDialog::ScChiSquareTestDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    weld::Window* pParent, ScViewData& rViewData ) :
    ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, rViewData,
            u"modules/scalc/ui/chisquaretestdialog.ui"_ustr, u"ChiSquareTestDialog"_ustr)
{
    m_xDialog->set_title(ScResId(STR_CHI_SQUARE_TEST));
}

ScChiSquareTestDialog::~ScChiSquareTestDialog()
{}

void ScChiSquareTestDialog::Close()
{
    DoClose(ScChiSquareTestDialogWrapper::GetChildWindowId());
}

TranslateId ScChiSquareTestDialog::GetUndoNameId()
{
    return STR_CHI_SQUARE_TEST;
}

ScRange ScChiSquareTestDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter aOutput(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar( formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(&mDocument);

    aTemplate.autoReplaceRange(u"%RANGE%"_ustr, mInputRange);

    aOutput.writeBoldString(ScResId(STR_CHI_SQUARE_TEST));
    aOutput.newLine();

    // Alpha
    aOutput.writeString(ScResId(STR_LABEL_ALPHA));
    aOutput.nextColumn();
    aOutput.writeValue(0.05);
    aTemplate.autoReplaceAddress(u"%ALPHA%"_ustr, aOutput.current());
    aOutput.newLine();

    // DF
    aOutput.writeString(ScResId(STR_DEGREES_OF_FREEDOM_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=(COLUMNS(%RANGE%) - 1) * (ROWS(%RANGE%) - 1)");
    aTemplate.autoReplaceAddress(u"%DEGREES_OF_FREEDOM%"_ustr, aOutput.current());
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // p Value
    aOutput.writeString(ScResId(STR_P_VALUE_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=CHITEST(%RANGE%; MMULT(MMULT(%RANGE%;TRANSPOSE(IF(COLUMN(%RANGE%))));MMULT(TRANSPOSE(IF(ROW(%RANGE%)));%RANGE%)) / SUM(%RANGE%))");
    aTemplate.autoReplaceAddress(u"%P_VALUE%"_ustr, aOutput.current());
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // Test Statistic
    aOutput.writeString(ScResId(STR_TEST_STATISTIC_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=CHIINV(%P_VALUE%; %DEGREES_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // Critical value
    aOutput.writeString(ScResId(STR_CRITICAL_VALUE_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=CHIINV(%ALPHA%; %DEGREES_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    return ScRange(aOutput.mMinimumAddress, aOutput.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
