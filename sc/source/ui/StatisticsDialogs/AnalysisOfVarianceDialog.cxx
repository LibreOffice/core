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

static const char* lclBasicStatisticsLabels[] =
{
    "Groups", "Count", "Sum", "Mean", "Variance", NULL
};

static const char* lclBasicStatisticsFormula[] =
{
    "=COUNT(%RANGE%)", "=SUM(%RANGE%)", "=AVERAGE(%RANGE%)", "=VAR(%RANGE%)", NULL
};

static const char* lclAnovaLabels[] =
{
    "Source of Variation", "SS", "df", "MS", "F", "P-value", "F critical", NULL
};

static const OUString strWildcardRange("%RANGE%");
static const OUString strWildcardNumber("%NUMBER%");
static const OUString strColumnLabelTemplate("Column %NUMBER%");
static const OUString strRowLabelTemplate("Row %NUMBER%");

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

sal_Bool ScAnalysisOfVarianceDialog::Close()
{
    return DoClose( ScAnalysisOfVarianceDialogWrapper::GetChildWindowId() );
}

void ScAnalysisOfVarianceDialog::CalculateInputAndWriteToOutput( )
{
    OUString aUndo( SC_STRLOAD( RID_STATISTICS_DLGS, STR_ANALYSIS_OF_VARIANCE_UNDO_NAME));

    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    AddressWalkerWriter output(mOutputAddress, pDocShell, mDocument);
    FormulaTemplate aTemplate(mDocument, mAddressDetails);

    // Write labels
    for(sal_Int32 i = 0; lclBasicStatisticsLabels[i] != NULL; i++)
    {
        output.writeString(lclBasicStatisticsLabels[i]);
        output.nextColumn();
    }
    output.nextRow();

    ScRangeList aRangeList;

    boost::scoped_ptr<DataRangeIterator> pIterator;
    if (mGroupedBy == BY_COLUMN)
        pIterator.reset(new DataRangeByColumnIterator(mInputRange));
    else
        pIterator.reset(new DataRangeByRowIterator(mInputRange));

    // Write statistic formulas for columns
    for( ; pIterator->hasNext(); pIterator->next() )
    {
        output.resetColumn();

        if (mGroupedBy == BY_COLUMN)
            aTemplate.setTemplate(strColumnLabelTemplate);
        else
            aTemplate.setTemplate(strRowLabelTemplate);

        aTemplate.applyString(strWildcardNumber, OUString::number(pIterator->index() + 1));
        pDocShell->GetDocFunc().SetStringCell(output.current(), aTemplate.getTemplate(), true);
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
    for(sal_Int32 i = 0; lclAnovaLabels[i] != NULL; i++)
    {
        output.writeString(lclAnovaLabels[i]);
        output.nextColumn();
    }
    output.nextRow();

    // Between Groups
    {
        // Label
        output.resetColumn();
        output.writeString("Between Groups");
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
        output.writeString("Within Groups");
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
        output.writeString("Total");
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

    ScRange aOutputRange(output.mMinimumAddress, output.mMaximumAddress);
    pUndoManager->LeaveListAction();
    pDocShell->PostPaint( aOutputRange, PAINT_GRID );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
