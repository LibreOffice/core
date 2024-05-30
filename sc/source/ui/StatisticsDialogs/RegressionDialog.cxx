/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <string_view>

#include <document.hxx>
#include <reffact.hxx>
#include <TableFillingAndNavigationTools.hxx>
#include <RegressionDialog.hxx>
#include <scresid.hxx>
#include <strings.hrc>

/*
   Some regression basics
   ----------------------

   1. Linear regression fits using data, a linear function between the dependent variable and the independent variable(s).
      The basic form of this function is :-

      y = b + m_1*x_1 + m_2*x_2 + ... + m_k*x_k

      where y is the dependent variable
            x_1, x_2, ..., x_k are the k independent variables
            b is the intercept
            m_1, m_2, ..., m_k are the slopes corresponding to the variables x_1, x_2, ..., x_k respectively.


     This equation for n observations can be compactly written using matrices as :-

     y = X*A

     where y is the n dimensional column vector containing dependent variable observations.
     where X is matrix of shape n*(k+1) where a row looks like [ 1  x_1  x_2 ...  x_k ]
           A is the k+1 dimensional column vector              [ b  m_1  m_2 ...  m_k ]

     Calc formula LINEST(Y_array ; X_array) can be used to compute all entries in "A" along with many other statistics.


  2. Logarithmic regression is basically used to find a linear function between the dependent variable and
     the natural logarithm of the independent variable(s).
     So the basic form of this functions is :-

     y = b + m_1*ln(x_1) + m_2*ln(x_2) + ... + m_k*ln(x_k)

     This can be again written in a compact matrix form for n observations.

     y = ln(X)*A

     where y is the n dimensional column vector containing dependent variable observations.
     where X is matrix of shape n*(k+1) where a row looks like  [ e  x_1  x_2 ...  x_k ]
           A is the k+1 dimensional column vector               [ b  m_1  m_2 ...  m_k ]

     To estimate A, we use the formula =LINEST(Y_array ; LN(X_array))


  3. Power regression is used to fit the following model :-

     y = b * (x_1 ^ m_1) * (x_2 ^ m_2) * ... * (x_k ^ m_k)

     To reduce this to a linear function(so that we can still use LINEST()), we take natural logarithm on both sides

     ln(y) = c + m_1*ln(x_1) + m_2*ln(x_2) + ... + m_k*ln(x_k) ; where c = ln(b)


     This again can be written compactly in matrix form as :-

     ln(y) = ln(X)*A

     where y is the n dimensional column vector containing dependent variable observations.
     where X is matrix of shape n*(k+1) where a row looks like  [ e  x_1  x_2 ...  x_k ]
           A is the k+1 dimensional column vector               [ c  m_1  m_2 ...  m_k ]

     To estimate A, we use the formula =LINEST(LN(Y_array) ; LN(X_array))

     Once we get A, to get back y from x's we use the formula :-

     y = exp( ln(X)*A )



     Some references for computing confidence interval for the regression coefficients :-

     [1] https://en.wikipedia.org/wiki/Student%27s_t-test#Slope_of_a_regression_line
     [2] https://en.wikipedia.org/wiki/Simple_linear_regression#Normality_assumption
     [3] https://onlinecourses.science.psu.edu/stat414/node/280

 */

namespace
{
    enum class ScRegType {
        LINEAR,
        LOGARITHMIC,
        POWER
    };

    const TranslateId constRegressionModel[] =
    {
        STR_LABEL_LINEAR,
        STR_LABEL_LOGARITHMIC,
        STR_LABEL_POWER
    };

    OUString constTemplateLINEST[] =
    {
        u"=LINEST(%VARIABLE2_RANGE% ; %VARIABLE1_RANGE% ; %CALC_INTERCEPT% ; TRUE)"_ustr,
        u"=LINEST(%VARIABLE2_RANGE% ; LN(%VARIABLE1_RANGE%) ; %CALC_INTERCEPT% ; TRUE)"_ustr,
        u"=LINEST(LN(%VARIABLE2_RANGE%) ; LN(%VARIABLE1_RANGE%) ; %CALC_INTERCEPT% ; TRUE)"_ustr
    };

    OUString constRegressionFormula[] =
    {
        u"=MMULT(%XDATAMATRIX_RANGE% ; %SLOPES_RANGE%) + %INTERCEPT_ADDR%"_ustr,
        u"=MMULT(LN(%XDATAMATRIX_RANGE%) ; %SLOPES_RANGE%) + %INTERCEPT_ADDR%"_ustr,
        u"=EXP(MMULT(LN(%XDATAMATRIX_RANGE%) ; %SLOPES_RANGE%) + %INTERCEPT_ADDR%)"_ustr
    };

} // end anonymous namespace

static size_t lcl_GetNumRowsColsInRange(const ScRange& rRange, bool bRows)
{
    if (bRows)
        return rRange.aEnd.Row() - rRange.aStart.Row() + 1;

    return rRange.aEnd.Col() - rRange.aStart.Col() + 1;
}

ScRegressionDialog::ScRegressionDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    weld::Window* pParent, ScViewData& rViewData )
    : ScStatisticsTwoVariableDialog(
            pSfxBindings, pChildWindow, pParent, rViewData,
            u"modules/scalc/ui/regressiondialog.ui"_ustr, u"RegressionDialog"_ustr)
    , mbUnivariate(true)
    , mnNumIndependentVars(1)
    , mnNumObservations(0)
    , mbUse3DAddresses(false)
    , mbCalcIntercept(true)
    , mxWithLabelsCheckBox(m_xBuilder->weld_check_button(u"withlabels-check"_ustr))
    , mxLinearRadioButton(m_xBuilder->weld_radio_button(u"linear-radio"_ustr))
    , mxLogarithmicRadioButton(m_xBuilder->weld_radio_button(u"logarithmic-radio"_ustr))
    , mxErrorMessage(m_xBuilder->weld_label(u"error-message"_ustr))
    , mxConfidenceLevelField(m_xBuilder->weld_spin_button(u"confidencelevel-spin"_ustr))
    , mxCalcResidualsCheckBox(m_xBuilder->weld_check_button(u"calcresiduals-check"_ustr))
    , mxNoInterceptCheckBox(m_xBuilder->weld_check_button(u"nointercept-check"_ustr))
{
    mxWithLabelsCheckBox->connect_toggled(LINK(this, ScRegressionDialog, CheckBoxHdl));
    mxConfidenceLevelField->connect_value_changed(LINK(this, ScRegressionDialog, NumericFieldHdl));
}

ScRegressionDialog::~ScRegressionDialog()
{
}

void ScRegressionDialog::Close()
{
    DoClose(ScRegressionDialogWrapper::GetChildWindowId());
}

TranslateId ScRegressionDialog::GetUndoNameId()
{
    return STR_REGRESSION_UNDO_NAME;
}

ScRange ScRegressionDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter aOutput(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar( formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(&mDocument);
    aTemplate.autoReplaceUses3D(mbUse3DAddresses);
    mbCalcIntercept = !mxNoInterceptCheckBox->get_active();

    // max col of our output should account for
    // 1. constant term column,
    // 2. mnNumIndependentVars columns
    // 3. Actual Y column
    // 4. Predicted Y column
    // 5. Residual Column
    SCCOL nOutputMaxCol = mOutputAddress.Col() + mnNumIndependentVars + 3;

    ScRange aXDataRange(GetDataRange(mVariable1Range));
    ScRange aYDataRange(GetDataRange(mVariable2Range));

    aTemplate.autoReplaceRange(u"%VARIABLE1_RANGE%"_ustr, aXDataRange);
    aTemplate.autoReplaceRange(u"%VARIABLE2_RANGE%"_ustr, aYDataRange);
    size_t nRegressionIndex = GetRegressionTypeIndex();
    ScRegType eRegType = static_cast<ScRegType>(nRegressionIndex);
    bool bTakeLogX = eRegType == ScRegType::LOGARITHMIC || eRegType == ScRegType::POWER;

    WriteRawRegressionResults(aOutput, aTemplate, nRegressionIndex);
    WriteRegressionStatistics(aOutput, aTemplate);
    WriteRegressionANOVAResults(aOutput, aTemplate);
    WriteRegressionEstimatesWithCI(aOutput, aTemplate, bTakeLogX);
    if (mxCalcResidualsCheckBox->get_active())
        WritePredictionsWithResiduals(aOutput, aTemplate, nRegressionIndex);

    ScAddress aMaxAddress(aOutput.mMaximumAddress);
    aMaxAddress.SetCol(std::max(aMaxAddress.Col(), nOutputMaxCol));
    return ScRange(aOutput.mMinimumAddress, aMaxAddress);
}

bool ScRegressionDialog::InputRangesValid()
{
    if (!mVariable1Range.IsValid())
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_XINVALID_RANGE));
        return false;
    }

    if (!mVariable2Range.IsValid())
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_YINVALID_RANGE));
        return false;
    }

    if (!mOutputAddress.IsValid())
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_INVALID_OUTPUT_ADDR));
        return false;
    }

    {
        double fConfidenceLevel = mxConfidenceLevelField->get_value();
        if ( fConfidenceLevel <= 0.0 || fConfidenceLevel >= 100.0 )
        {
            mxErrorMessage->set_label(ScResId(STR_MESSAGE_INVALID_CONFIDENCE_LEVEL));
            return false;
        }
    }

    mVariable1Range.PutInOrder();
    mVariable2Range.PutInOrder();

    bool bGroupedByColumn = mGroupedBy == BY_COLUMN;

    bool bYHasSingleDim = (
        (bGroupedByColumn &&
         mVariable2Range.aStart.Col() == mVariable2Range.aEnd.Col()) ||
        (!bGroupedByColumn &&
         mVariable2Range.aStart.Row() == mVariable2Range.aEnd.Row()));

    if (!bYHasSingleDim)
    {
        if (bGroupedByColumn)
            mxErrorMessage->set_label(ScResId(STR_MESSAGE_YVARIABLE_MULTI_COLUMN));
        else
            mxErrorMessage->set_label(ScResId(STR_MESSAGE_YVARIABLE_MULTI_ROW));
        return false;
    }

    bool bWithLabels = mxWithLabelsCheckBox->get_active();

    size_t nYObs = lcl_GetNumRowsColsInRange(mVariable2Range, bGroupedByColumn);
    size_t nNumXVars = lcl_GetNumRowsColsInRange(mVariable1Range, !bGroupedByColumn);
    mbUnivariate = nNumXVars == 1;
    // Observation count mismatch check
    if (lcl_GetNumRowsColsInRange(mVariable1Range, bGroupedByColumn) != nYObs)
    {
        if (mbUnivariate)
            mxErrorMessage->set_label(ScResId(STR_MESSAGE_UNIVARIATE_NUMOBS_MISMATCH));
        else
            mxErrorMessage->set_label(ScResId(STR_MESSAGE_MULTIVARIATE_NUMOBS_MISMATCH));
        return false;
    }

    mnNumIndependentVars = nNumXVars;
    mnNumObservations = bWithLabels ? nYObs - 1 : nYObs;

    mbUse3DAddresses = mVariable1Range.aStart.Tab() != mOutputAddress.Tab() ||
        mVariable2Range.aStart.Tab() != mOutputAddress.Tab();

    mxErrorMessage->set_label(u""_ustr);

    return true;
}

size_t ScRegressionDialog::GetRegressionTypeIndex() const
{
    if (mxLinearRadioButton->get_active())
        return 0;
    if (mxLogarithmicRadioButton->get_active())
        return 1;
    return 2;
}

ScRange ScRegressionDialog::GetDataRange(const ScRange& rRange)
{
    if (!mxWithLabelsCheckBox->get_active())
        return rRange;

    ScRange aDataRange(rRange);
    if (mGroupedBy == BY_COLUMN)
        aDataRange.aStart.IncRow(1);
    else
        aDataRange.aStart.IncCol(1);

    return aDataRange;
}

OUString ScRegressionDialog::GetVariableNameFormula(bool bXVar, size_t nIndex, bool bWithLog)
{
    if (bXVar && nIndex == 0)
        return "=\"" + ScResId(STR_LABEL_INTERCEPT) + "\"";

    if (mxWithLabelsCheckBox->get_active())
    {
        ScAddress aAddr(bXVar ? mVariable1Range.aStart : mVariable2Range.aStart);
        if (mGroupedBy == BY_COLUMN)
            aAddr.IncCol(nIndex - 1);
        else
            aAddr.IncRow(nIndex - 1);

        ScRefFlags eAddrFlag = mbUse3DAddresses ? ScRefFlags::ADDR_ABS_3D : ScRefFlags::ADDR_ABS;
        return bWithLog ? OUString("=CONCAT(\"LN(\";" +
            aAddr.Format(eAddrFlag, &mDocument, mDocument.GetAddressConvention()) + ";\")\")") :
            OUString("=" + aAddr.Format(eAddrFlag, &mDocument, mDocument.GetAddressConvention()));
    }

    OUString aDefaultVarName;

    if (bXVar)
        aDefaultVarName = "X" + OUString::number(nIndex);
    else
        aDefaultVarName = "Y";

    return bWithLog ? OUString("=\"LN(" + aDefaultVarName + ")\"") :
        OUString("=\"" + aDefaultVarName + "\"");
}

OUString ScRegressionDialog::GetXVariableNameFormula(size_t nIndex, bool bWithLog)
{
    assert(nIndex <= mnNumIndependentVars);
    return GetVariableNameFormula(true, nIndex, bWithLog);
}

OUString ScRegressionDialog::GetYVariableNameFormula(bool bWithLog)
{
    return GetVariableNameFormula(false, 1, bWithLog);
}

void ScRegressionDialog::WriteRawRegressionResults(AddressWalkerWriter& rOutput, FormulaTemplate& rTemplate,
                                                   size_t nRegressionIndex)
{
    rOutput.writeBoldString(ScResId(STR_REGRESSION));
    rOutput.newLine();
    // REGRESSION MODEL
    rOutput.writeString(ScResId(STR_LABEL_REGRESSION_MODEL));
    rOutput.nextColumn();
    rOutput.writeString(ScResId(constRegressionModel[nRegressionIndex]));
    rOutput.newLine();
    rOutput.newLine();

    rOutput.writeString(ScResId(STR_LINEST_RAW_OUTPUT_TITLE));
    rOutput.newLine();
    rOutput.push();

    rTemplate.setTemplate(constTemplateLINEST[nRegressionIndex].
                          replaceFirst("%CALC_INTERCEPT%",
                                       mbCalcIntercept ? std::u16string_view(u"TRUE") : std::u16string_view(u"FALSE")));
    rOutput.writeMatrixFormula(rTemplate.getTemplate(), 1 + mnNumIndependentVars, 5);
    // Add LINEST result components to template
    // 1. Add ranges for coefficients and standard errors for independent vars and the intercept.
    // Note that these two are in the reverse order(m_n, m_n-1, ..., m_1, b) w.r.t what we expect.
    rTemplate.autoReplaceRange(u"%COEFFICIENTS_REV_RANGE%"_ustr, ScRange(rOutput.current(), rOutput.current(mnNumIndependentVars)));
    rTemplate.autoReplaceRange(u"%SERRORSX_REV_RANGE%"_ustr, ScRange(rOutput.current(0, 1), rOutput.current(mnNumIndependentVars, 1)));

    // 2. Add R-squared and standard error for y estimate.
    rTemplate.autoReplaceAddress(u"%RSQUARED_ADDR%"_ustr, rOutput.current(0, 2));
    rTemplate.autoReplaceAddress(u"%SERRORY_ADDR%"_ustr, rOutput.current(1, 2));

    // 3. Add F statistic and degrees of freedom
    rTemplate.autoReplaceAddress(u"%FSTATISTIC_ADDR%"_ustr, rOutput.current(0, 3));
    rTemplate.autoReplaceAddress(u"%DoFRESID_ADDR%"_ustr, rOutput.current(1, 3));

    // 4. Add regression sum of squares and residual sum of squares
    rTemplate.autoReplaceAddress(u"%SSREG_ADDR%"_ustr, rOutput.current(0, 4));
    rTemplate.autoReplaceAddress(u"%SSRESID_ADDR%"_ustr, rOutput.current(1, 4));

    rOutput.push(0, 4);
    rOutput.newLine();
}

void ScRegressionDialog::WriteRegressionStatistics(AddressWalkerWriter& rOutput, FormulaTemplate& rTemplate)
{
    rOutput.newLine();
    rOutput.writeString(ScResId(STR_LABEL_REGRESSION_STATISTICS));
    rOutput.newLine();

    const TranslateId aMeasureNames[] =
    {
        STR_LABEL_RSQUARED,
        STRID_CALC_STD_ERROR,
        STR_LABEL_XVARIABLES_COUNT,
        STR_OBSERVATIONS_LABEL,
        STR_LABEL_ADJUSTED_RSQUARED
    };

    OUString aMeasureFormulas[] =
    {
        u"=%RSQUARED_ADDR%"_ustr,
        u"=%SERRORY_ADDR%"_ustr,
        "=" + OUString::number(mnNumIndependentVars),
        "=" + OUString::number(mnNumObservations),
        OUString::Concat(
                "=1 - (1 - %RSQUARED_ADDR%)*(%NUMOBS_ADDR% - 1)/(%NUMOBS_ADDR% - %NUMXVARS_ADDR%") +
            (mbCalcIntercept ? std::u16string_view(u" - 1)") : std::u16string_view(u")"))
    };

    rTemplate.autoReplaceAddress(u"%NUMXVARS_ADDR%"_ustr, rOutput.current(1, 2));
    rTemplate.autoReplaceAddress(u"%NUMOBS_ADDR%"_ustr, rOutput.current(1, 3));

    for (size_t nIdx = 0; nIdx < SAL_N_ELEMENTS(aMeasureNames); ++nIdx)
    {
        rOutput.writeString(ScResId(aMeasureNames[nIdx]));
        rOutput.nextColumn();
        rTemplate.setTemplate(aMeasureFormulas[nIdx]);
        rOutput.writeFormula(rTemplate.getTemplate());
        rOutput.newLine();
    }
}

void ScRegressionDialog::WriteRegressionANOVAResults(AddressWalkerWriter& rOutput, FormulaTemplate& rTemplate)
{
    rOutput.newLine();
    rOutput.writeString(ScResId(STR_LABEL_ANOVA));
    rOutput.newLine();

    const size_t nColsInTable = 6;
    const size_t nRowsInTable = 4;
    OUString aTable[nRowsInTable][nColsInTable] =
    {
        {
            u""_ustr,
            ScResId(STR_ANOVA_LABEL_DF),
            ScResId(STR_ANOVA_LABEL_SS),
            ScResId(STR_ANOVA_LABEL_MS),
            ScResId(STR_ANOVA_LABEL_F),
            ScResId(STR_ANOVA_LABEL_SIGNIFICANCE_F)
        },
        {
            ScResId(STR_REGRESSION),
            u"=%NUMXVARS_ADDR%"_ustr,
            u"=%SSREG_ADDR%"_ustr,
            u"=%SSREG_ADDR% / %DoFREG_ADDR%"_ustr,
            u"=%FSTATISTIC_ADDR%"_ustr,
            u"=FDIST(%FSTATISTIC_ADDR% ; %DoFREG_ADDR% ; %DoFRESID_ADDR%)"_ustr
        },
        {
            ScResId(STR_LABEL_RESIDUAL),
            u"=%DoFRESID_ADDR%"_ustr,
            u"=%SSRESID_ADDR%"_ustr,
            u"=%SSRESID_ADDR% / %DoFRESID_ADDR%"_ustr,
            u""_ustr,
            u""_ustr
        },
        {
            ScResId(STR_ANOVA_LABEL_TOTAL),
            u"=%DoFREG_ADDR% + %DoFRESID_ADDR%"_ustr,
            u"=%SSREG_ADDR% + %SSRESID_ADDR%"_ustr,
            u""_ustr,
            u""_ustr,
            u""_ustr
        }
    };

    rTemplate.autoReplaceAddress(u"%DoFREG_ADDR%"_ustr, rOutput.current(1, 1));

    // Cell getter lambda
    std::function<CellValueGetter> aCellGetterFunc = [&aTable](size_t nRowIdx, size_t nColIdx) -> const OUString&
    {
        return aTable[nRowIdx][nColIdx];
    };

    // Cell writer lambda
    std::function<CellWriter> aCellWriterFunc = [&rOutput, &rTemplate]
        (const OUString& rContent, size_t /*nRowIdx*/, size_t /*nColIdx*/)
    {
        if (!rContent.isEmpty())
        {
            if (rContent.startsWith("="))
            {
                rTemplate.setTemplate(rContent);
                rOutput.writeFormula(rTemplate.getTemplate());
            }
            else
                rOutput.writeString(rContent);
        }
    };

    WriteTable(aCellGetterFunc, nRowsInTable, nColsInTable, rOutput, aCellWriterFunc);

    // User given confidence level
    rOutput.newLine();
    rOutput.writeString(ScResId(STR_LABEL_CONFIDENCE_LEVEL));
    rOutput.nextColumn();
    rOutput.writeValue(mxConfidenceLevelField->get_value() / 100.0);
    rTemplate.autoReplaceAddress(u"%CONFIDENCE_LEVEL_ADDR%"_ustr, rOutput.current());
    rOutput.newLine();
}

// Write slopes, intercept, their standard errors, t-statistics, p-value, confidence intervals
void ScRegressionDialog::WriteRegressionEstimatesWithCI(AddressWalkerWriter& rOutput, FormulaTemplate& rTemplate,
                                                        bool bTakeLogX)
{
    rOutput.newLine();
    ScAddress aEnd( rOutput.current(0, 1 + mnNumIndependentVars));
    ScRefFlags eAddrFlag = mbUse3DAddresses ? ScRefFlags::ADDR_ABS_3D : ScRefFlags::ADDR_ABS;
    aEnd.IncCol();
    const OUString aCoeffAddr( aEnd.Format( eAddrFlag, &mDocument, mDocument.GetAddressConvention()));
    aEnd.IncCol();
    const OUString aStErrAddr( aEnd.Format( eAddrFlag, &mDocument, mDocument.GetAddressConvention()));

    // Coefficients & Std.Errors ranges (column vectors) in this table (yet to populate).
    rTemplate.autoReplaceRange(u"%COEFFICIENTS_RANGE%"_ustr,
                               ScRange(rOutput.current(1, 1),
                                       rOutput.current(1, 1 + mnNumIndependentVars)));
    rTemplate.autoReplaceRange(u"%SLOPES_RANGE%"_ustr,  // Excludes the intercept
                               ScRange(rOutput.current(1, 2),
                                       rOutput.current(1, 1 + mnNumIndependentVars)));
    rTemplate.autoReplaceAddress(u"%INTERCEPT_ADDR%"_ustr, rOutput.current(1, 1));
    rTemplate.autoReplaceRange(u"%SERRORSX_RANGE%"_ustr,
                               ScRange(rOutput.current(2, 1),
                                       rOutput.current(2, 1 + mnNumIndependentVars)));
    // t-Statistics range in this table (yet to populate)
    rTemplate.autoReplaceRange(u"%TSTAT_RANGE%"_ustr,
                               ScRange(rOutput.current(3, 1),
                                       rOutput.current(3, 1 + mnNumIndependentVars)));

    const size_t nColsInTable = 7;
    const size_t nRowsInTable = 2;
    OUString aTable[nRowsInTable][nColsInTable] =
    {
        {
            u""_ustr,
            ScResId(STR_LABEL_COEFFICIENTS),
            ScResId(STRID_CALC_STD_ERROR),
            ScResId(STR_LABEL_TSTATISTIC),
            ScResId(STR_P_VALUE_LABEL),

            "=CONCAT(\"" + ScResId(STR_LABEL_LOWER) +
            " \" ; INT(%CONFIDENCE_LEVEL_ADDR% * 100) ; \"%\")",

            "=CONCAT(\"" + ScResId(STR_LABEL_UPPER) +
            " \" ; INT(%CONFIDENCE_LEVEL_ADDR% * 100) ; \"%\")",
        },

        // Following are matrix formulas of size numcols = 1, numrows = (mnNumIndependentVars + 1)
        {
            u""_ustr,
            // This puts the coefficients in the reverse order compared to that in LINEST output.
            "=INDEX(%COEFFICIENTS_REV_RANGE%; 1 ; ROW(" + aCoeffAddr + ")+1 - ROW())",
            // This puts the standard errors in the reverse order compared to that in LINEST output.
            "=INDEX(%SERRORSX_REV_RANGE%; 1 ; ROW(" + aStErrAddr + ")+1 - ROW())",
            // t-Statistic
            u"=%COEFFICIENTS_RANGE% / %SERRORSX_RANGE%"_ustr,
            // p-Value
            u"=TDIST(ABS(%TSTAT_RANGE%) ; %DoFRESID_ADDR% ; 2 )"_ustr,
            // Lower limit of confidence interval
            u"=%COEFFICIENTS_RANGE% - %SERRORSX_RANGE% * "
            "TINV(1 - %CONFIDENCE_LEVEL_ADDR% ; %DoFRESID_ADDR%)"_ustr,
            // Upper limit of confidence interval
            u"=%COEFFICIENTS_RANGE% + %SERRORSX_RANGE% * "
            "TINV(1 - %CONFIDENCE_LEVEL_ADDR% ; %DoFRESID_ADDR%)"_ustr
        }
    };

    // Cell getter lambda
    std::function<CellValueGetter> aCellGetterFunc = [&aTable](size_t nRowIdx, size_t nColIdx) -> const OUString&
    {
        return aTable[nRowIdx][nColIdx];
    };

    // Cell writer lambda
    size_t nNumIndependentVars = mnNumIndependentVars;
    std::function<CellWriter> aCellWriterFunc = [&rOutput, &rTemplate, nNumIndependentVars]
        (const OUString& rContent, size_t nRowIdx, size_t /*nColIdx*/)
    {
        if (!rContent.isEmpty())
        {
            if (rContent.startsWith("="))
            {
                rTemplate.setTemplate(rContent);
                if (nRowIdx == 0)
                    rOutput.writeFormula(rTemplate.getTemplate());
                else
                    rOutput.writeMatrixFormula(rTemplate.getTemplate(), 1, 1 + nNumIndependentVars);
            }
            else
                rOutput.writeString(rContent);
        }
    };

    WriteTable(aCellGetterFunc, nRowsInTable, nColsInTable, rOutput, aCellWriterFunc);

    // Go back to the second row and first column of the table to
    // fill the names of variables + intercept
    rOutput.push(0, -1);

    for (size_t nXvarIdx = 0; nXvarIdx <= mnNumIndependentVars; ++nXvarIdx)
    {
        rOutput.writeFormula(GetXVariableNameFormula(nXvarIdx, bTakeLogX));
        rOutput.newLine();
    }

}

// Re-write all observations in group-by column mode with predictions and residuals
void ScRegressionDialog::WritePredictionsWithResiduals(AddressWalkerWriter& rOutput, FormulaTemplate& rTemplate,
                                                       size_t nRegressionIndex)
{
    bool bGroupedByColumn = mGroupedBy == BY_COLUMN;
    rOutput.newLine();
    rOutput.push();

    // Range of X variables with rows as observations and columns as variables.
    ScRange aDataMatrixRange(rOutput.current(0, 1), rOutput.current(mnNumIndependentVars - 1, mnNumObservations));
    rTemplate.autoReplaceRange(u"%XDATAMATRIX_RANGE%"_ustr, aDataMatrixRange);

    // Write X variable names
    for (size_t nXvarIdx = 1; nXvarIdx <= mnNumIndependentVars; ++nXvarIdx)
    {
        // Here we write the X variables without any transformation(LN)
        rOutput.writeFormula(GetXVariableNameFormula(nXvarIdx, false));
        rOutput.nextColumn();
    }
    rOutput.reset();

    // Write the X data matrix
    rOutput.nextRow();
    OUString aDataMatrixFormula = bGroupedByColumn ? u"=%VARIABLE1_RANGE%"_ustr : u"=TRANSPOSE(%VARIABLE1_RANGE%)"_ustr;
    rTemplate.setTemplate(aDataMatrixFormula);
    rOutput.writeMatrixFormula(rTemplate.getTemplate(), mnNumIndependentVars, mnNumObservations);

    // Write predicted values
    rOutput.push(mnNumIndependentVars, -1);
    rOutput.writeString(ScResId(STR_LABEL_PREDICTEDY));
    rOutput.nextRow();
    rTemplate.setTemplate(constRegressionFormula[nRegressionIndex]);
    rOutput.writeMatrixFormula(rTemplate.getTemplate(), 1, mnNumObservations);
    rTemplate.autoReplaceRange(u"%PREDICTEDY_RANGE%"_ustr, ScRange(rOutput.current(), rOutput.current(0, mnNumObservations - 1)));

    // Write actual Y
    rOutput.push(1, -1);
    rOutput.writeFormula(GetYVariableNameFormula(false));
    rOutput.nextRow();
    OUString aYVectorFormula = bGroupedByColumn ? u"=%VARIABLE2_RANGE%"_ustr : u"=TRANSPOSE(%VARIABLE2_RANGE%)"_ustr;
    rTemplate.setTemplate(aYVectorFormula);
    rOutput.writeMatrixFormula(rTemplate.getTemplate(), 1, mnNumObservations);
    rTemplate.autoReplaceRange(u"%ACTUALY_RANGE%"_ustr, ScRange(rOutput.current(), rOutput.current(0, mnNumObservations - 1)));

    // Write residual
    rOutput.push(1, -1);
    rOutput.writeString(ScResId(STR_LABEL_RESIDUAL));
    rOutput.nextRow();
    rTemplate.setTemplate("=%ACTUALY_RANGE% - %PREDICTEDY_RANGE%");
    rOutput.writeMatrixFormula(rTemplate.getTemplate(), 1, mnNumObservations);
}

// Generic table writer
void ScRegressionDialog::WriteTable(const std::function<CellValueGetter>& rCellGetter,
                                    size_t nRowsInTable, size_t nColsInTable,
                                    AddressWalkerWriter& rOutput,
                                    const std::function<CellWriter>& rFunc)
{
    for (size_t nRowIdx = 0; nRowIdx < nRowsInTable; ++nRowIdx)
    {
        for (size_t nColIdx = 0; nColIdx < nColsInTable; ++nColIdx)
        {
            rFunc(rCellGetter(nRowIdx, nColIdx), nRowIdx, nColIdx);
            rOutput.nextColumn();
        }
        rOutput.newLine();
    }
}

IMPL_LINK_NOARG(ScRegressionDialog, CheckBoxHdl, weld::Toggleable&, void)
{
    ValidateDialogInput();
}

IMPL_LINK_NOARG(ScRegressionDialog, NumericFieldHdl, weld::SpinButton&, void)
{
    ValidateDialogInput();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
