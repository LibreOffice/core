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

#include "AnalysisOfVarianceDialog.hxx"

namespace
{

struct StatisticCalculation {
    sal_Int16   aLabelId;
    const char* aFormula;
    const char* aResultRangeName;
};

static StatisticCalculation lclBasicStatistics[] =
{
    { STR_ANOVA_LABEL_GROUPS, nullptr,                nullptr          },
    { STRID_CALC_COUNT,       "=COUNT(%RANGE%)",   "COUNT_RANGE" },
    { STRID_CALC_SUM,         "=SUM(%RANGE%)",     "SUM_RANGE"   },
    { STRID_CALC_MEAN,        "=AVERAGE(%RANGE%)", "MEAN_RANGE"  },
    { STRID_CALC_VARIANCE,    "=VAR(%RANGE%)",     "VAR_RANGE"   },
    { 0,                      nullptr,                nullptr          }
};

static sal_Int16 lclAnovaLabels[] =
{
    STR_ANOVA_LABEL_SOURCE_OF_VARIATION,
    STR_ANOVA_LABEL_SS,
    STR_ANOVA_LABEL_DF,
    STR_ANOVA_LABEL_MS,
    STR_ANOVA_LABEL_F,
    STR_ANOVA_LABEL_P_VALUE,
    STR_ANOVA_LABEL_F_CRITICAL,
    0
};

static const char strWildcardRange[] = "%RANGE%";

OUString lclCreateMultiParameterFormula(
            ScRangeList&        aRangeList, const OUString& aFormulaTemplate,
            const OUString&     aWildcard,  ScDocument*     pDocument,
            ScAddress::Details& aAddressDetails)
{
    OUString aResult;
    for (size_t i = 0; i < aRangeList.size(); i++)
    {
        OUString aRangeString(aRangeList[i]->Format(ScRefFlags::RANGE_ABS, pDocument, aAddressDetails));
        OUString aFormulaString = aFormulaTemplate.replaceAll(aWildcard, aRangeString);
        aResult += aFormulaString;
        if(i != aRangeList.size() - 1) // Not Last
            aResult+= ";";
    }
    return aResult;
}

void lclMakeSubRangesList(ScRangeList& rRangeList, ScRange& rInputRange, ScStatisticsInputOutputDialog::GroupedBy aGroupedBy)
{
    std::unique_ptr<DataRangeIterator> pIterator;
    if (aGroupedBy == ScStatisticsInputOutputDialog::BY_COLUMN)
        pIterator.reset(new DataRangeByColumnIterator(rInputRange));
    else
        pIterator.reset(new DataRangeByRowIterator(rInputRange));

    for( ; pIterator->hasNext(); pIterator->next() )
    {
        ScRange aRange = pIterator->get();
        rRangeList.Append(aRange);
    }
}

}

ScAnalysisOfVarianceDialog::ScAnalysisOfVarianceDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    vcl::Window* pParent, ScViewData* pViewData ) :
    ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "AnalysisOfVarianceDialog", "modules/scalc/ui/analysisofvariancedialog.ui" ),
    meFactor(SINGLE_FACTOR)
{
    get(mpAlphaField,         "alpha-spin");
    get(mpSingleFactorRadio,  "radio-single-factor");
    get(mpTwoFactorRadio,     "radio-two-factor");
    get(mpRowsPerSampleField, "rows-per-sample-spin");

    mpSingleFactorRadio->SetToggleHdl( LINK( this, ScAnalysisOfVarianceDialog, FactorChanged ) );
    mpTwoFactorRadio->SetToggleHdl( LINK( this, ScAnalysisOfVarianceDialog, FactorChanged ) );

    mpSingleFactorRadio->Check();
    mpTwoFactorRadio->Check(false);

    FactorChanged();
}

ScAnalysisOfVarianceDialog::~ScAnalysisOfVarianceDialog()
{
    disposeOnce();
}

void ScAnalysisOfVarianceDialog::dispose()
{
    mpAlphaField.clear();
    mpSingleFactorRadio.clear();
    mpTwoFactorRadio.clear();
    mpRowsPerSampleField.clear();
    ScStatisticsInputOutputDialog::dispose();
}

bool ScAnalysisOfVarianceDialog::Close()
{
    return DoClose( ScAnalysisOfVarianceDialogWrapper::GetChildWindowId() );
}

sal_Int16 ScAnalysisOfVarianceDialog::GetUndoNameId()
{
    return STR_ANALYSIS_OF_VARIANCE_UNDO_NAME;
}

IMPL_LINK_NOARG( ScAnalysisOfVarianceDialog, FactorChanged, RadioButton&, void )
{
    FactorChanged();
}

void ScAnalysisOfVarianceDialog::FactorChanged()
{
    if (mpSingleFactorRadio->IsChecked())
    {
        mpGroupByRowsRadio->Enable();
        mpGroupByColumnsRadio->Enable();
        mpRowsPerSampleField->Enable(false);
        meFactor = SINGLE_FACTOR;
    }
    else if (mpTwoFactorRadio->IsChecked())
    {
        mpGroupByRowsRadio->Enable(false);
        mpGroupByColumnsRadio->Enable(false);
        mpRowsPerSampleField->Enable(false); // Rows per sample not yet implemented
        meFactor = TWO_FACTOR;
    }
}

void ScAnalysisOfVarianceDialog::RowColumn(ScRangeList& rRangeList, AddressWalkerWriter& aOutput, FormulaTemplate& aTemplate,
                                           OUString& sFormula, GroupedBy aGroupedBy, ScRange* pResultRange)
{
    if (pResultRange != nullptr)
        pResultRange->aStart = aOutput.current();
    if (!sFormula.isEmpty())
    {
        for (size_t i = 0; i < rRangeList.size(); i++)
        {
            ScRange* pRange = rRangeList[i];
            aTemplate.setTemplate(sFormula);
            aTemplate.applyRange(strWildcardRange, *pRange);
            aOutput.writeFormula(aTemplate.getTemplate());
            if (pResultRange != nullptr)
                pResultRange->aEnd = aOutput.current();
            aOutput.nextRow();
        }
    }
    else
    {
        sal_Int16 aLabelId = (aGroupedBy == BY_COLUMN) ? STR_COLUMN_LABEL_TEMPLATE : STR_ROW_LABEL_TEMPLATE;
        OUString aLabelTemplate(SC_RESSTR(aLabelId));

        for (size_t i = 0; i < rRangeList.size(); i++)
        {
            aTemplate.setTemplate(aLabelTemplate);
            aTemplate.applyNumber("%NUMBER%", i + 1);
            aOutput.writeString(aTemplate.getTemplate());
            if (pResultRange != nullptr)
                pResultRange->aEnd = aOutput.current();
            aOutput.nextRow();
        }
    }
}

void ScAnalysisOfVarianceDialog::AnovaSingleFactor(AddressWalkerWriter& output, FormulaTemplate& aTemplate)
{
    output.writeBoldString(SC_RESSTR(STR_ANOVA_SINGLE_FACTOR_LABEL));
    output.newLine();

    double aAlphaValue = mpAlphaField->GetValue() / 100.0;
    output.writeString(SC_RESSTR(STR_LABEL_ALPHA));
    output.nextColumn();
    output.writeValue(aAlphaValue);
    aTemplate.autoReplaceAddress("%ALPHA%", output.current());
    output.newLine();
    output.newLine();

    // Write labels
    for(sal_Int32 i = 0; lclBasicStatistics[i].aLabelId != 0; i++)
    {
        output.writeString(SC_RESSTR(lclBasicStatistics[i].aLabelId));
        output.nextColumn();
    }
    output.newLine();

    // Collect aRangeList
    ScRangeList aRangeList;
    lclMakeSubRangesList(aRangeList, mInputRange, mGroupedBy);

    output.push();

    // Write values
    for(sal_Int32 i = 0; lclBasicStatistics[i].aLabelId != 0; i++)
    {
        output.resetRow();
        ScRange aResultRange;
        OUString sFormula = OUString::createFromAscii(lclBasicStatistics[i].aFormula);
        RowColumn(aRangeList, output, aTemplate, sFormula, mGroupedBy, &aResultRange);
        output.nextColumn();
        if (lclBasicStatistics[i].aResultRangeName != nullptr)
        {
            OUString sResultRangeName = OUString::createFromAscii(lclBasicStatistics[i].aResultRangeName);
            aTemplate.autoReplaceRange("%" + sResultRangeName + "%", aResultRange);
        }
    }

    output.nextRow(); // Blank row

    // Write ANOVA labels
    output.resetColumn();
    for(sal_Int32 i = 0; lclAnovaLabels[i] != 0; i++)
    {
        output.writeString(SC_RESSTR(lclAnovaLabels[i]));
        output.nextColumn();
    }
    output.nextRow();

    aTemplate.autoReplaceRange("%FIRST_COLUMN%", *aRangeList[0]);

    // Between Groups
    {
        // Label
        output.resetColumn();
        output.writeString(SC_RESSTR(STR_ANOVA_LABEL_BETWEEN_GROUPS));
        output.nextColumn();

        // Sum of Squares

        aTemplate.setTemplate("=SUMPRODUCT(%SUM_RANGE%;%MEAN_RANGE%)-SUM(%SUM_RANGE%)^2/SUM(%COUNT_RANGE%)");
        aTemplate.autoReplaceAddress("%BETWEEN_SS%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // Degree of freedom
        aTemplate.setTemplate("=COUNT(%SUM_RANGE%)-1");
        aTemplate.autoReplaceAddress("%BETWEEN_DF%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // MS
        aTemplate.setTemplate("=%BETWEEN_SS% / %BETWEEN_DF%");
        aTemplate.autoReplaceAddress("%BETWEEN_MS%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // F
        aTemplate.setTemplate("=%BETWEEN_MS% / %WITHIN_MS%");
        aTemplate.applyAddress("%WITHIN_MS%",  output.current(-1, 1));
        aTemplate.autoReplaceAddress("%F_VAL%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // P-value
        aTemplate.setTemplate("=FDIST(%F_VAL%; %BETWEEN_DF%; %WITHIN_DF%");
        aTemplate.applyAddress("%WITHIN_DF%",   output.current(-3, 1));
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // F critical
        aTemplate.setTemplate("=FINV(%ALPHA%; %BETWEEN_DF%; %WITHIN_DF%");
        aTemplate.applyAddress("%WITHIN_DF%",  output.current(-4, 1));
        output.writeFormula(aTemplate.getTemplate());
    }
    output.nextRow();

    // Within Groups
    {
        // Label
        output.resetColumn();
        output.writeString(SC_RESSTR(STR_ANOVA_LABEL_WITHIN_GROUPS));
        output.nextColumn();

        // Sum of Squares
        OUString aSSPart = lclCreateMultiParameterFormula(aRangeList, "DEVSQ(%RANGE%)", strWildcardRange, mDocument, mAddressDetails);
        aTemplate.setTemplate("=SUM(%RANGE%)");
        aTemplate.applyString(strWildcardRange, aSSPart);
        aTemplate.autoReplaceAddress("%WITHIN_SS%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // Degree of freedom
        aTemplate.setTemplate("=SUM(%COUNT_RANGE%)-COUNT(%COUNT_RANGE%)");
        aTemplate.autoReplaceAddress("%WITHIN_DF%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // MS
        aTemplate.setTemplate("=%WITHIN_SS% / %WITHIN_DF%");
        output.writeFormula(aTemplate.getTemplate());
    }
    output.nextRow();

    // Total
    {
        // Label
        output.resetColumn();
        output.writeString(SC_RESSTR(STR_ANOVA_LABEL_TOTAL));
        output.nextColumn();

        // Sum of Squares
        aTemplate.setTemplate("=DEVSQ(%RANGE_LIST%)");
        aTemplate.applyRangeList("%RANGE_LIST%", aRangeList);
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // Degree of freedom
        aTemplate.setTemplate("=SUM(%COUNT_RANGE%) - 1");
        output.writeFormula(aTemplate.getTemplate());
    }
    output.nextRow();
}

void ScAnalysisOfVarianceDialog::AnovaTwoFactor(AddressWalkerWriter& output, FormulaTemplate& aTemplate)
{
    output.writeBoldString(SC_RESSTR(STR_ANOVA_TWO_FACTOR_LABEL));
    output.newLine();

    double aAlphaValue = mpAlphaField->GetValue() / 100.0;
    output.writeString("Alpha");
    output.nextColumn();
    output.writeValue(aAlphaValue);
    aTemplate.autoReplaceAddress("%ALPHA%", output.current());
    output.newLine();
    output.newLine();

    // Write labels
    for(sal_Int32 i = 0; lclBasicStatistics[i].aLabelId != 0; i++)
    {
        output.writeString(SC_RESSTR(lclBasicStatistics[i].aLabelId));
        output.nextColumn();
    }
    output.newLine();

    ScRangeList aColumnRangeList;
    ScRangeList aRowRangeList;

    lclMakeSubRangesList(aColumnRangeList, mInputRange, BY_COLUMN);
    lclMakeSubRangesList(aRowRangeList, mInputRange, BY_ROW);

    // Write ColumnX values
    output.push();
    for(sal_Int32 i = 0; lclBasicStatistics[i].aLabelId != 0; i++)
    {
        output.resetRow();
        ScRange aResultRange;
        OUString sFormula = OUString::createFromAscii(lclBasicStatistics[i].aFormula);
        RowColumn(aColumnRangeList, output, aTemplate, sFormula, BY_COLUMN, &aResultRange);
        if (lclBasicStatistics[i].aResultRangeName != nullptr)
        {
            OUString sResultRangeName = OUString::createFromAscii(lclBasicStatistics[i].aResultRangeName);
            aTemplate.autoReplaceRange("%" + sResultRangeName + "_COLUMN%", aResultRange);
        }
        output.nextColumn();
    }
    output.newLine();

    // Write RowX values
    output.push();
    for(sal_Int32 i = 0; lclBasicStatistics[i].aLabelId != 0; i++)
    {
        output.resetRow();
        ScRange aResultRange;
        OUString sFormula = OUString::createFromAscii(lclBasicStatistics[i].aFormula);
        RowColumn(aRowRangeList, output, aTemplate, sFormula, BY_ROW, &aResultRange);

        if (lclBasicStatistics[i].aResultRangeName != nullptr)
        {
            OUString sResultRangeName = OUString::createFromAscii(lclBasicStatistics[i].aResultRangeName);
            aTemplate.autoReplaceRange("%" + sResultRangeName + "_ROW%", aResultRange);
        }
        output.nextColumn();
    }
    output.newLine();

    // Write ANOVA labels
    for(sal_Int32 i = 0; lclAnovaLabels[i] != 0; i++)
    {
        output.writeString(SC_RESSTR(lclAnovaLabels[i]));
        output.nextColumn();
    }
    output.nextRow();

    // Setup auto-replace strings
    aTemplate.autoReplaceRange(strWildcardRange, mInputRange);
    aTemplate.autoReplaceRange("%FIRST_COLUMN%", *aColumnRangeList[0]);
    aTemplate.autoReplaceRange("%FIRST_ROW%",    *aRowRangeList[0]);

    // Rows
    {
        // Label
        output.resetColumn();
        output.writeString("Rows");
        output.nextColumn();

        // Sum of Squares
        aTemplate.setTemplate("=SUMPRODUCT(%SUM_RANGE_ROW%;%MEAN_RANGE_ROW%) - SUM(%RANGE%)^2 / COUNT(%RANGE%)");
        aTemplate.autoReplaceAddress("%ROW_SS%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // Degree of freedom
        aTemplate.setTemplate("=MAX(%COUNT_RANGE_COLUMN%) - 1");
        aTemplate.autoReplaceAddress("%ROW_DF%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // MS
        aTemplate.setTemplate("=%ROW_SS% / %ROW_DF%");
        aTemplate.autoReplaceAddress("%MS_ROW%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // F
        aTemplate.setTemplate("=%MS_ROW% / %MS_ERROR%");
        aTemplate.applyAddress("%MS_ERROR%", output.current(-1, 2));
        aTemplate.autoReplaceAddress("%F_ROW%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // P-value
        aTemplate.setTemplate("=FDIST(%F_ROW%; %ROW_DF%; %ERROR_DF%");
        aTemplate.applyAddress("%ERROR_DF%",   output.current(-3, 2));
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // F critical
        aTemplate.setTemplate("=FINV(%ALPHA%; %ROW_DF%; %ERROR_DF%");
        aTemplate.applyAddress("%ERROR_DF%",  output.current(-4, 2));
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();
    }
    output.nextRow();

    // Columns
    {
        // Label
        output.resetColumn();
        output.writeString("Columns");
        output.nextColumn();

        // Sum of Squares
        aTemplate.setTemplate("=SUMPRODUCT(%SUM_RANGE_COLUMN%;%MEAN_RANGE_COLUMN%) - SUM(%RANGE%)^2 / COUNT(%RANGE%)");
        aTemplate.autoReplaceAddress("%COLUMN_SS%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // Degree of freedom
        aTemplate.setTemplate("=MAX(%COUNT_RANGE_ROW%) - 1");
        aTemplate.autoReplaceAddress("%COLUMN_DF%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // MS
        aTemplate.setTemplate("=%COLUMN_SS% / %COLUMN_DF%");
        aTemplate.autoReplaceAddress("%MS_COLUMN%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // F
        aTemplate.setTemplate("=%MS_COLUMN% / %MS_ERROR%");
        aTemplate.applyAddress("%MS_ERROR%", output.current(-1, 1));
        aTemplate.autoReplaceAddress("%F_COLUMN%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

         // P-value
        aTemplate.setTemplate("=FDIST(%F_COLUMN%; %COLUMN_DF%; %ERROR_DF%");
        aTemplate.applyAddress("%ERROR_DF%",   output.current(-3, 1));
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // F critical
        aTemplate.setTemplate("=FINV(%ALPHA%; %COLUMN_DF%; %ERROR_DF%");
        aTemplate.applyAddress("%ERROR_DF%",  output.current(-4, 1));
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();
    }
    output.nextRow();

    // Error
    {
        // Label
        output.resetColumn();
        output.writeString("Error");
        output.nextColumn();

        // Sum of Squares
        aTemplate.setTemplate("=SUMSQ(%RANGE%)+SUM(%RANGE%)^2/COUNT(%RANGE%) - (SUMPRODUCT(%SUM_RANGE_ROW%;%MEAN_RANGE_ROW%) + SUMPRODUCT(%SUM_RANGE_COLUMN%;%MEAN_RANGE_COLUMN%))");
        aTemplate.autoReplaceAddress("%ERROR_SS%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // Degree of freedom
        aTemplate.setTemplate("=%TOTAL_DF% - %ROW_DF% - %COLUMN_DF%");
        aTemplate.applyAddress("%TOTAL_DF%", output.current(0,1));
        aTemplate.autoReplaceAddress("%ERROR_DF%", output.current());
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // MS
        aTemplate.setTemplate("=%ERROR_SS% / %ERROR_DF%");
        output.writeFormula(aTemplate.getTemplate());
    }
    output.nextRow();

     // Total
    {
        // Label
        output.resetColumn();
        output.writeString("Total");
        output.nextColumn();

        // Sum of Squares
        aTemplate.setTemplate("=SUM(%ROW_SS%;%COLUMN_SS%;%ERROR_SS%)");
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // Degree of freedom
        aTemplate.setTemplate("=COUNT(%RANGE%)-1");
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();
    }
}

ScRange ScAnalysisOfVarianceDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter output(mOutputAddress, pDocShell, mDocument,
        formula::FormulaGrammar::mergeToGrammar(formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(mDocument);

    if (meFactor == SINGLE_FACTOR)
    {
        AnovaSingleFactor(output, aTemplate);
    }
    else if (meFactor == TWO_FACTOR)
    {
        AnovaTwoFactor(output, aTemplate);
    }

    return ScRange(output.mMinimumAddress, output.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
