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
#include "scresid.hxx"
#include "random.hxx"
#include "docfunc.hxx"
#include "globstr.hrc"
#include "sc.hrc"

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

static const OUString lclWildcardNumber("%NUMBER%");
static const OUString lclColumnLabelTemplate("Column %NUMBER%");
static const OUString lclWildcardRange("%RANGE%");

OUString lclCreateMultiParameterFormula(
            ScRangeList&        aRangeList, const OUString& aFormulaTemplate,
            const OUString&     aWildcard,  ScDocument*     pDocument,
            ScAddress::Details& aAddressDetails)
{
    OUString aResult;
    for (size_t i = 0; i < aRangeList.size(); i++)
    {
        OUString aRangeString;
        aRangeList[i]->Format( aRangeString, SCR_ABS, pDocument, aAddressDetails );
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
    get(mpAlpha,        "alpha-spin");
}

ScAnalysisOfVarianceDialog::~ScAnalysisOfVarianceDialog()
{}

sal_Bool ScAnalysisOfVarianceDialog::Close()
{
    return DoClose( ScAnalysisOfVarianceDialogWrapper::GetChildWindowId() );
}

void ScAnalysisOfVarianceDialog::CalculateInputAndWriteToOutput( )
{
    OUString aUndo(SC_RESSTR(STR_ANALYSIS_OF_VARIANCE_UNDO_NAME));

    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    ScAddress aStart = mInputRange.aStart;
    ScAddress aEnd   = mInputRange.aEnd;

    SCTAB outTab = mOutputAddress.Tab();
    SCCOL outCol = mOutputAddress.Col();
    SCROW outRow = mOutputAddress.Row();

    OUString aReferenceString;
    ScAddress aAddress;

    for (SCROW inTab = aStart.Tab(); inTab <= aEnd.Tab(); inTab++)
    {
        outCol = mOutputAddress.Col();

        // Write labels
        for(sal_Int32 i = 0; lclBasicStatisticsLabels[i] != NULL; i++)
        {
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aCalculationName(OUString::createFromAscii(lclBasicStatisticsLabels[i]));
            pDocShell->GetDocFunc().SetStringCell(aAddress, aCalculationName, true);
            outCol++;
        }
        outRow++;

        ScRangeList aRangeList;

        // Write statistic formulas for columns
        for (SCCOL inCol = aStart.Col(); inCol <= aEnd.Col(); inCol++)
        {
            outCol = mOutputAddress.Col();
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aNumberString = OUString::number(inCol - aStart.Col() + 1);
            OUString aGroupName = lclColumnLabelTemplate.replaceAll(lclWildcardNumber, aNumberString);
            pDocShell->GetDocFunc().SetStringCell(aAddress, aGroupName, true);
            outCol++;

            ScRange aColumnRange (
                ScAddress(inCol, aStart.Row(), inTab),
                ScAddress(inCol, aEnd.Row(), inTab)
            );
            aRangeList.Append(aColumnRange);

            aColumnRange.Format( aReferenceString, SCR_ABS, mDocument, mAddressDetails );
            OUString aFormulaString;
            OUString aFormulaTemplate;

            for(sal_Int32 i = 0; lclBasicStatisticsFormula[i] != NULL; i++)
            {
                aAddress = ScAddress(outCol, outRow, outTab);
                aFormulaTemplate = OUString::createFromAscii(lclBasicStatisticsFormula[i]);
                aFormulaString = aFormulaTemplate.replaceAll(lclWildcardRange, aReferenceString);
                pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
                outCol++;
            }
            outRow++;
        }

        outRow++; // Blank row

        // Write ANOVA labels
        outCol = mOutputAddress.Col();
        for(sal_Int32 i = 0; lclAnovaLabels[i] != NULL; i++)
        {
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aCalculationName(OUString::createFromAscii(lclAnovaLabels[i]));
            pDocShell->GetDocFunc().SetStringCell(aAddress, aCalculationName, true);
            outCol++;
        }
        outRow++;

        // Between Groups
        {
            // Label
            outCol = mOutputAddress.Col();
            aAddress = ScAddress(outCol, outRow, outTab);
            pDocShell->GetDocFunc().SetStringCell(aAddress, OUString("Between Groups"), true);
            outCol++;

            // Sum of Squares
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aReferenceTotal;
            OUString aReferenceWithin;
            ScAddress aAddressTotal(outCol, outRow+2, outTab);
            aAddressTotal.Format( aReferenceTotal, SCR_ABS, mDocument, mAddressDetails );
            ScAddress aAddressWithin(outCol, outRow+1, outTab);
            aAddressWithin.Format( aReferenceWithin, SCR_ABS, mDocument, mAddressDetails );
            OUString aFormulaString = "=" + aReferenceTotal + "-" + aReferenceWithin;
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
            outCol++;

            // Degree of freedom
            aAddress = ScAddress(outCol, outRow, outTab);
            aAddressTotal = ScAddress(outCol, outRow+2, outTab);
            aAddressTotal.Format( aReferenceTotal, SCR_ABS, mDocument, mAddressDetails );
            aAddressWithin = ScAddress(outCol, outRow+1, outTab);
            aAddressWithin.Format( aReferenceWithin, SCR_ABS, mDocument, mAddressDetails );
            aFormulaString = "=" + aReferenceTotal + "-" + aReferenceWithin;
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
            outCol++;

            // MS
            OUString aSSRef;
            ScAddress(outCol-2, outRow, outTab).Format( aSSRef, SCR_ABS, mDocument, mAddressDetails );
            OUString aDFRef;
            ScAddress(outCol-1, outRow, outTab).Format( aDFRef, SCR_ABS, mDocument, mAddressDetails );
            aFormulaString = "=" + aSSRef + "/" + aDFRef;
            aAddress = ScAddress(outCol, outRow, outTab);
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
            outCol++;

            // F
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aMSBetween;
            ScAddress(outCol-1, outRow, outTab).Format( aMSBetween, SCR_ABS, mDocument, mAddressDetails );
            OUString aMSWithin;
            ScAddress(outCol-1, outRow+1, outTab).Format( aMSWithin, SCR_ABS, mDocument, mAddressDetails );
            aFormulaString = "=" + aMSBetween + "/" + aMSWithin;
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
            outCol++;

            // P-value
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aFValue;
            ScAddress(outCol-1, outRow, outTab).Format( aFValue, SCR_ABS, mDocument, mAddressDetails );
            OUString aDFBetween;
            ScAddress(outCol-3, outRow, outTab).Format( aDFBetween, SCR_ABS, mDocument, mAddressDetails );
            OUString aDFWithin;
            ScAddress(outCol-3, outRow+1, outTab).Format( aDFWithin, SCR_ABS, mDocument, mAddressDetails );
            aFormulaString = "=FDIST("+ aFValue + ";" + aDFBetween + ";" + aDFWithin + ")";
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
            outCol++;

            // F critical
            double aAlphaValue = mpAlpha->GetValue() / 100.0;
            OUString aAlphaString = rtl::math::doubleToUString(
                                        aAlphaValue, rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                                        ScGlobal::pLocaleData->getNumDecimalSep()[0], true);
            aAddress = ScAddress(outCol, outRow, outTab);
            aFormulaString = "=FINV(" + aAlphaString + ";" + aDFBetween + ";" + aDFWithin + ")";
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
        }
        outRow++;

        // Within Groups
        {
            // Label
            outCol = mOutputAddress.Col();
            aAddress = ScAddress(outCol, outRow, outTab);
            pDocShell->GetDocFunc().SetStringCell(aAddress, OUString("Within Groups"), true);
            outCol++;

            // Sum of Squares
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aSum("=SUM(%RANGE%)");
            OUString aDevSQ("DEVSQ(%RANGE%)");
            OUString aSSPart = lclCreateMultiParameterFormula(aRangeList, aDevSQ, lclWildcardRange, mDocument, mAddressDetails);
            OUString aSS = aSum.replaceAll(lclWildcardRange, aSSPart);
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aSS), true);
            outCol++;

            // Degree of freedom
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aCountMinusOne("COUNT(%RANGE%)-1");
            OUString aDFPart = lclCreateMultiParameterFormula(aRangeList, aCountMinusOne, lclWildcardRange, mDocument, mAddressDetails);
            OUString aDF = aSum.replaceAll(lclWildcardRange, aDFPart);
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aDF), true);
            outCol++;

            // MS
            OUString aSSRef;
            OUString aDFRef;
            ScAddress aAddressSS(outCol-2, outRow, outTab);
            aAddressSS.Format( aSSRef, SCR_ABS, mDocument, mAddressDetails );
            ScAddress aAddressDF(outCol-1, outRow, outTab);
            aAddressDF.Format( aDFRef, SCR_ABS, mDocument, mAddressDetails );
            OUString aFormulaString = "=" + aSSRef + "/" + aDFRef;
            aAddress = ScAddress(outCol, outRow, outTab);
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
        }
        outRow++;

        // Total
        {
            // Label
            outCol = mOutputAddress.Col();
            aAddress = ScAddress(outCol, outRow, outTab);
            pDocShell->GetDocFunc().SetStringCell(aAddress, OUString("Total"), true);
            outCol++;

            // Sum of Squares
            OUString aDevSQ("DEVSQ(%RANGE%)");
            aRangeList.Format( aReferenceString, SCR_ABS, mDocument );
            OUString aFormulaString = aDevSQ.replaceAll(lclWildcardRange, aReferenceString);

            aAddress = ScAddress(outCol, outRow, outTab);
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, "=" + aFormulaString), true);
            outCol++;

            // Degree of freedom
            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aCount("COUNT(%RANGE%)");
            OUString aSumMinusOne("=SUM(%RANGE%)-1");
            OUString aDFPart = lclCreateMultiParameterFormula(aRangeList, aCount, lclWildcardRange, mDocument, mAddressDetails);
            OUString aDF = aSumMinusOne.replaceAll(lclWildcardRange, aDFPart);
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aDF), true);
        }
        outRow++;
    }

    ScRange aOutputRange(mOutputAddress, ScAddress(outTab, outRow, outTab) );
    pUndoManager->LeaveListAction();
    pDocShell->PostPaint( aOutputRange, PAINT_GRID );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
