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

#include "ChiSquareTestDialog.hxx"

ScChiSquareTestDialog::ScChiSquareTestDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    vcl::Window* pParent, ScViewData* pViewData ) :
    ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "ChiSquareTestDialog", "modules/scalc/ui/chisquaretestdialog.ui" )
{
    SetText(SC_RESSTR(STR_CHI_SQUARE_TEST));
}

ScChiSquareTestDialog::~ScChiSquareTestDialog()
{}

bool ScChiSquareTestDialog::Close()
{
    return DoClose(ScChiSquareTestDialogWrapper::GetChildWindowId());
}

sal_Int16 ScChiSquareTestDialog::GetUndoNameId()
{
    return STR_CHI_SQUARE_TEST;
}

ScRange ScChiSquareTestDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter aOutput(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar( formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(mDocument);

    aTemplate.autoReplaceRange("%RANGE%", mInputRange);

    aOutput.writeBoldString(SC_RESSTR(STR_CHI_SQUARE_TEST));
    aOutput.newLine();

    // Alpha
    aOutput.writeString(SC_RESSTR(STR_LABEL_ALPHA));
    aOutput.nextColumn();
    aOutput.writeValue(0.05);
    aTemplate.autoReplaceAddress("%ALPHA%", aOutput.current());
    aOutput.newLine();

    // DF
    aOutput.writeString(SC_RESSTR(STR_DEGREES_OF_FREEDOM_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=(COLUMNS(%RANGE%) - 1) * (ROWS(%RANGE%) - 1)");
    aTemplate.autoReplaceAddress("%DEGREES_OF_FREEDOM%", aOutput.current());
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // p Value
    aOutput.writeString(SC_RESSTR(STR_P_VALUE_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=CHITEST(%RANGE%; MMULT(MMULT(%RANGE%;TRANSPOSE(IF(COLUMN(%RANGE%))));MMULT(TRANSPOSE(IF(ROW(%RANGE%)));%RANGE%)) / SUM(%RANGE%))");
    aTemplate.autoReplaceAddress("%P_VALUE%", aOutput.current());
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // Test Statistic
    aOutput.writeString(SC_RESSTR(STR_TEST_STATISTIC_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=CHIINV(%P_VALUE%; %DEGREES_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    // Critical value
    aOutput.writeString(SC_RESSTR(STR_CRITICAL_VALUE_LABEL));
    aOutput.nextColumn();
    aTemplate.setTemplate("=CHIINV(%ALPHA%; %DEGREES_OF_FREEDOM%)");
    aOutput.writeFormula(aTemplate.getTemplate());
    aOutput.newLine();

    return ScRange(aOutput.mMinimumAddress, aOutput.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
