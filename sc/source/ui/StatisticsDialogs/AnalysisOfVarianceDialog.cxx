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
#include "random.hxx"
#include "docfunc.hxx"
#include "StatisticsDialogs.hrc"
#include "TableFillingAndNavigationTools.hxx"

#include "AnalysisOfVarianceDialog.hxx"

namespace
{

static sal_Int16 lclBasicStatisticsLabels[] =
{
    STR_ANOVA_LABEL_GROUPS,
    STRID_CALC_COUNT,
    STRID_CALC_SUM,
    STRID_CALC_MEAN,
    STRID_CALC_VARIANCE,
    0
};

static const char* lclBasicStatisticsFormula[] =
{
    "=COUNT(%RANGE%)", "=SUM(%RANGE%)", "=AVERAGE(%RANGE%)", "=VAR(%RANGE%)", NULL
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

static const OUString strWildcardRange("%RANGE%");
static const OUString strWildcardNumber("%NUMBER%");

OUString lclCreateMultiParameterFormula(
            ScRangeList&        aRangeList, const OUString& aFormulaTemplate,
            const OUString&     aWildcard,  ScDocument*     pDocument,
            ScAddress::Details& aAddressDetails)
{
    OUString aResult;
    for (size_t i = 0; i < aRangeList.size(); i++)
    {
        OUString aRangeString(aRangeList[i]->Format(SCR_ABS, pDocument, aAddressDetails));
        OUString aFormulaString = aFormulaTemplate.replaceAll(aWildcard, aRangeString);
        aResult += aFormulaString;
        if(i != aRangeList.size() - 1) // Not Last
            aResult+= ";";
    }
    return aResult;
}

}

ScAnalysisOfVarianceDialog::ScAnalysisOfVarianceDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData ) :
    ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "AnalysisOfVarianceDialog", "modules/scalc/ui/analysisofvariancedialog.ui" )
{
    get(mpAlpha, "alpha-spin");
}

ScAnalysisOfVarianceDialog::~ScAnalysisOfVarianceDialog()
{}

bool ScAnalysisOfVarianceDialog::Close()
{
    return DoClose( ScAnalysisOfVarianceDialogWrapper::GetChildWindowId() );
}

sal_Int16 ScAnalysisOfVarianceDialog::GetUndoNameId()
{
    return STR_ANALYSIS_OF_VARIANCE_UNDO_NAME;
}

ScRange ScAnalysisOfVarianceDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter output(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar( formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(mDocument, mAddressDetails);

    output.writeBoldString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_ANOVA_SINGLE_FACTOR_LABEL));
    output.nextRow();
    output.nextRow();

    // Write labels
    for(sal_Int32 i = 0; lclBasicStatisticsLabels[i] != 0; i++)
    {
        output.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, lclBasicStatisticsLabels[i]));
        output.nextColumn();
    }
    output.nextRow();

    ScRangeList aRangeList;

    boost::scoped_ptr<DataRangeIterator> pIterator;
    if (mGroupedBy == BY_COLUMN)
        pIterator.reset(new DataRangeByColumnIterator(mInputRange));
    else
        pIterator.reset(new DataRangeByRowIterator(mInputRange));

    // Write statistic formulas for rows/columns
    for( ; pIterator->hasNext(); pIterator->next() )
    {
        output.resetColumn();

        if (mGroupedBy == BY_COLUMN)
            aTemplate.setTemplate(SC_STRLOAD(RID_STATISTICS_DLGS, STR_COLUMN_LABEL_TEMPLATE));
        else
            aTemplate.setTemplate(SC_STRLOAD(RID_STATISTICS_DLGS, STR_ROW_LABEL_TEMPLATE));

        aTemplate.applyNumber(strWildcardNumber, pIterator->index() + 1);
        output.writeString(aTemplate.getTemplate());

        output.nextColumn();

        ScRange aColumnRange = pIterator->get();

        aRangeList.Append(aColumnRange);

        for(sal_Int32 i = 0; lclBasicStatisticsFormula[i] != NULL; i++)
        {
            aTemplate.setTemplate(lclBasicStatisticsFormula[i]);
            aTemplate.applyRange(strWildcardRange, aColumnRange);
            output.writeFormula(aTemplate.getTemplate());
            output.nextColumn();
        }
        output.nextRow();
    }

    output.nextRow(); // Blank row

    // Write ANOVA labels
    output.resetColumn();
    for(sal_Int32 i = 0; lclAnovaLabels[i] != 0; i++)
    {
        output.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, lclAnovaLabels[i]));
        output.nextColumn();
    }
    output.nextRow();

    // Between Groups
    {
        // Label
        output.resetColumn();
        output.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_ANOVA_LABEL_BETWEEN_GROUPS));
        output.nextColumn();

        // Sum of Squares
        aTemplate.setTemplate("=%TOTAL% - %WITHIN%");
        aTemplate.applyAddress("%TOTAL%", output.current(0, 2));
        aTemplate.applyAddress("%WITHIN%", output.current(0, 1));
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // Degree of freedom
        aTemplate.setTemplate("=%TOTAL% - %WITHIN%");
        aTemplate.applyAddress("%TOTAL%", output.current(0, 2));
        aTemplate.applyAddress("%WITHIN%", output.current(0, 1));
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // MS
        aTemplate.setTemplate("=%SS_REF% / %DF_REF%");
        aTemplate.applyAddress("%SS_REF%", output.current(-2, 0));
        aTemplate.applyAddress("%DF_REF%", output.current(-1, 0));
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // F
        aTemplate.setTemplate("=%MS_BETWEEN% / %MS_WITHIN%");
        aTemplate.applyAddress("%MS_BETWEEN%", output.current(-1, 0));
        aTemplate.applyAddress("%MS_WITHIN%",  output.current(-1, 1));
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // P-value
        aTemplate.setTemplate("=FDIST(%F_VAL%; %DF_BETWEEN%; %DF_WITHIN%");
        aTemplate.applyAddress("%F_VAL%",       output.current(-1, 0));
        aTemplate.applyAddress("%DF_BETWEEN%",  output.current(-3, 0));
        aTemplate.applyAddress("%DF_WITHIN%",   output.current(-3, 1));
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // F critical
        double aAlphaValue = mpAlpha->GetValue() / 100.0;
        OUString aAlphaString = rtl::math::doubleToUString(
                                    aAlphaValue, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                                    ScGlobal::pLocaleData->getNumDecimalSep()[0], true);

        aTemplate.setTemplate("=FINV(%ALPHA%; %DF_BETWEEN%; %DF_WITHIN%");
        aTemplate.applyString("%ALPHA%",       aAlphaString);
        aTemplate.applyAddress("%DF_BETWEEN%", output.current(-4, 0));
        aTemplate.applyAddress("%DF_WITHIN%",  output.current(-4, 1));
        output.writeFormula(aTemplate.getTemplate());
    }
    output.nextRow();

    // Within Groups
    {
        // Label
        output.resetColumn();
        output.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_ANOVA_LABEL_WITHIN_GROUPS));
        output.nextColumn();

        // Sum of Squares
        OUString aSSPart = lclCreateMultiParameterFormula(aRangeList, OUString("DEVSQ(%RANGE%)"), strWildcardRange, mDocument, mAddressDetails);
        aTemplate.setTemplate("=SUM(%RANGE%)");
        aTemplate.applyString(strWildcardRange, aSSPart);
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // Degree of freedom
        OUString aDFPart = lclCreateMultiParameterFormula(aRangeList, OUString("COUNT(%RANGE%)-1"), strWildcardRange, mDocument, mAddressDetails);
        aTemplate.setTemplate("=SUM(%RANGE%)");
        aTemplate.applyString(strWildcardRange, aDFPart);
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // MS
        aTemplate.setTemplate("=%SS% / %DF%");
        aTemplate.applyAddress("%SS%", output.current(-2, 0));
        aTemplate.applyAddress("%DF%", output.current(-1, 0));
        output.writeFormula(aTemplate.getTemplate());
    }
    output.nextRow();

    // Total
    {
        // Label
        output.resetColumn();
        output.writeString(SC_STRLOAD(RID_STATISTICS_DLGS, STR_ANOVA_LABEL_TOTAL));
        output.nextColumn();

        // Sum of Squares
        aTemplate.setTemplate("=DEVSQ(%RANGE%)");
        aTemplate.applyRangeList(strWildcardRange, aRangeList);
        output.writeFormula(aTemplate.getTemplate());
        output.nextColumn();

        // Degree of freedom
        OUString aDFPart = lclCreateMultiParameterFormula(aRangeList, "COUNT(%RANGE%)", strWildcardRange, mDocument, mAddressDetails);
        aTemplate.setTemplate("=SUM(%RANGE%) - 1");
        aTemplate.applyString(strWildcardRange, aDFPart);
        output.writeFormula(aTemplate.getTemplate());
    }
    output.nextRow();

    return ScRange(output.mMinimumAddress, output.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
