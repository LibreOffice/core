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

#include "RegressionDialog.hxx"

namespace
{
    sal_Int16 constRegressionModel[] =
    {
        STR_LABEL_LINEAR,
        STR_LABEL_LOGARITHMIC,
        STR_LABEL_POWER
    };

    OUString constTemplateRSQUARED[] =
    {
        "=RSQ(%VARIABLE2_RANGE% ; %VARIABLE1_RANGE%)",
        "=RSQ(%VARIABLE2_RANGE% ; LN(%VARIABLE1_RANGE%))",
        "=RSQ(LN(%VARIABLE2_RANGE%) ; LN(%VARIABLE1_RANGE%))"
    };

    OUString constTemplatesSTDERR[] =
    {
        "=STEYX(%VARIABLE2_RANGE% ; %VARIABLE1_RANGE%)",
        "=STEYX(%VARIABLE2_RANGE% ; LN(%VARIABLE1_RANGE%))",
        "=STEYX(LN(%VARIABLE2_RANGE%) ; LN(%VARIABLE1_RANGE%))"
    };

    OUString constTemplatesSLOPE[] =
    {
        "=SLOPE(%VARIABLE2_RANGE% ; %VARIABLE1_RANGE%)",
        "=SLOPE(%VARIABLE2_RANGE% ; LN(%VARIABLE1_RANGE%))",
        "=SLOPE(LN(%VARIABLE2_RANGE%) ; LN(%VARIABLE1_RANGE%))"
    };

    OUString constTemplatesINTERCEPT[] =
    {
        "=INTERCEPT(%VARIABLE2_RANGE% ; %VARIABLE1_RANGE%)",
        "=INTERCEPT(%VARIABLE2_RANGE% ; LN(%VARIABLE1_RANGE%))",
        "=EXP(INTERCEPT(LN(%VARIABLE2_RANGE%) ; LN(%VARIABLE1_RANGE%)))"
    };

    OUString constRegressionFormula[] =
    {
        "=%A% * %ADDRESS% + %B%",
        "=%A% * LN(%ADDRESS%) + %B%",
        "=%B% * %ADDRESS% ^ %A%"
    };

} // end anonymous namespace

ScRegressionDialog::ScRegressionDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    vcl::Window* pParent, ScViewData* pViewData ) :
    ScStatisticsTwoVariableDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "RegressionDialog", "modules/scalc/ui/regressiondialog.ui" )
{
    get(mpLinearCheckBox, "linear-check");
    get(mpLogarithmicCheckBox, "logarithmic-check");
    get(mpPowerCheckBox, "power-check");
}

ScRegressionDialog::~ScRegressionDialog()
{
    disposeOnce();
}

bool ScRegressionDialog::Close()
{
    return DoClose(ScRegressionDialogWrapper::GetChildWindowId());
}

void ScRegressionDialog::dispose()
{
    mpLinearCheckBox.disposeAndClear();
    mpLogarithmicCheckBox.disposeAndClear();
    mpPowerCheckBox.disposeAndClear();
    ScStatisticsTwoVariableDialog::dispose();
}

sal_Int16 ScRegressionDialog::GetUndoNameId()
{
    return STR_REGRESSION_UNDO_NAME;
}

ScRange ScRegressionDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter aOutput(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar( formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(mDocument);
    aTemplate.autoReplaceUses3D(false);

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

    aOutput.writeBoldString(SC_RESSTR(STR_REGRESSION));
    aOutput.newLine();
    aOutput.newLine();
    aOutput.push();

    // REGRESSION MODEL
    aOutput.writeString(SC_RESSTR(STR_LABEL_REGRESSION_MODEL));
    aOutput.nextRow();

    // RSQUARED
    aOutput.writeString(SC_RESSTR(STR_LABEL_RSQUARED));
    aOutput.nextRow();

    // Standard Error
    aOutput.writeString(SC_RESSTR(STRID_CALC_STD_ERROR));
    aOutput.nextRow();

    aOutput.nextRow();

    // Slope
    aOutput.writeString(SC_RESSTR(STR_LABEL_SLOPE));
    aOutput.nextRow();

    // Intercept
    aOutput.writeString(SC_RESSTR(STR_LABEL_INTERCEPT));
    aOutput.nextRow();

    aOutput.nextRow();

    size_t nVariable1Size = pVariable1Iterator->size();

    OUString sFormula;
    if (mGroupedBy == BY_COLUMN)
        sFormula = "=INDEX(%VARIABLE1_RANGE%; %VAR1_CELL_INDEX%; 1)";
    else
        sFormula = "=INDEX(%VARIABLE1_RANGE%; 1; %VAR1_CELL_INDEX%)";

    for (size_t i = 0; i < nVariable1Size; i++)
    {
        aTemplate.setTemplate(sFormula);
        aTemplate.applyNumber("%VAR1_CELL_INDEX%", i + 1);
        aOutput.writeFormula(aTemplate.getTemplate());
        aOutput.nextRow();
    }

    aOutput.reset();

    bool aEnabledRegressionTypes[3];

    aEnabledRegressionTypes[0] = mpLinearCheckBox->IsChecked();
    aEnabledRegressionTypes[1] = mpLogarithmicCheckBox->IsChecked();
    aEnabledRegressionTypes[2] = mpPowerCheckBox->IsChecked();

    sal_Int16 nColumn = 0;

    for (size_t nRegressionIndex = 0; nRegressionIndex < SAL_N_ELEMENTS(aEnabledRegressionTypes); ++nRegressionIndex)
    {
        if (!aEnabledRegressionTypes[nRegressionIndex])
            continue;

        aOutput.nextColumn();
        nColumn += 1;

        // REGRESSION MODEL
        aOutput.writeString(SC_RESSTR(constRegressionModel[nRegressionIndex]));
        aOutput.nextRow();

        // RSQUARED
        aTemplate.setTemplate(constTemplateRSQUARED[nRegressionIndex]);
        aOutput.writeMatrixFormula(aTemplate.getTemplate());
        aTemplate.autoReplaceAddress("%RSQUARED%", aOutput.current());
        aOutput.nextRow();

        // Standard Error
        aTemplate.setTemplate(constTemplatesSTDERR[nRegressionIndex]);
        aOutput.writeMatrixFormula(aTemplate.getTemplate());
        aTemplate.autoReplaceAddress("%STD_ERROR%", aOutput.current());
        aOutput.nextRow();

        aOutput.nextRow();

        // Slope
        aTemplate.setTemplate(constTemplatesSLOPE[nRegressionIndex]);
        aOutput.writeMatrixFormula(aTemplate.getTemplate());
        aTemplate.autoReplaceAddress("%A%", aOutput.current());
        aOutput.nextRow();

        // Intercept
        aTemplate.setTemplate(constTemplatesINTERCEPT[nRegressionIndex]);
        aOutput.writeMatrixFormula(aTemplate.getTemplate());
        aTemplate.autoReplaceAddress("%B%", aOutput.current());
        aOutput.nextRow();

        aOutput.nextRow();

        for (size_t i = 0; i < nVariable1Size; i++)
        {
            aTemplate.setTemplate(constRegressionFormula[nRegressionIndex]);
            aTemplate.applyAddress("%ADDRESS%", aOutput.current(-nColumn), false);
            aOutput.writeFormula(aTemplate.getTemplate());

            aOutput.nextRow();
        }

        aOutput.resetRow();
    }

    return ScRange(aOutput.mMinimumAddress, aOutput.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
