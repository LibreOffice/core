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

#include "MatrixComparisonGenerator.hxx"

namespace
{
    static const OUString strWildcard1("%VAR1%");
    static const OUString strWildcard2("%VAR2%");

    static const OUString strWildcardNumber("%NUMBER%");

    void lclWriteCorrelationFormulas(
            AddressWalkerWriter& aOutput, FormulaTemplate& aTemplate,
            ScRangeList aRangeList, const OUString& aTemplateString)
    {
        for (size_t i = 0; i < aRangeList.size(); i++)
        {
            aOutput.resetRow();
            for (size_t j = 0; j < aRangeList.size(); j++)
            {
                if (j >= i)
                {
                    aTemplate.setTemplate(aTemplateString);
                    aTemplate.applyRange(strWildcard1, *aRangeList[i]);
                    aTemplate.applyRange(strWildcard2, *aRangeList[j]);
                    aOutput.writeFormula(aTemplate.getTemplate());
                }
                aOutput.nextRow();
            }
            aOutput.nextColumn();
        }
    }
}

ScMatrixComparisonGenerator::ScMatrixComparisonGenerator(
                                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                                    Window* pParent, ScViewData* pViewData, const OString& rID,
                                    const OUString& rUiXmlDescription) :
    ScStatisticsInputOutputDialog(pSfxBindings, pChildWindow, pParent, pViewData, rID, rUiXmlDescription)
{}

ScMatrixComparisonGenerator::~ScMatrixComparisonGenerator()
{}

sal_Int16 ScMatrixComparisonGenerator::GetUndoNameId()
{
    return STR_CORRELATION_UNDO_NAME;
}

ScRange ScMatrixComparisonGenerator::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter output(mOutputAddress, pDocShell, mDocument);
    FormulaTemplate aTemplate(mDocument, mAddressDetails);

    SCTAB inTab = mInputRange.aStart.Tab();

    ScRangeList aRangeList;

    if (mGroupedBy == BY_COLUMN)
        aRangeList = MakeColumnRangeList(inTab, mInputRange.aStart, mInputRange.aEnd);
    else
        aRangeList = MakeRowRangeList(inTab, mInputRange.aStart, mInputRange.aEnd);

    // labels
    output.writeString(getLabel());
    output.nextColumn();

    // write labels to columns
    for (size_t i = 0; i < aRangeList.size(); i++)
    {
        if (mGroupedBy == BY_COLUMN)
            aTemplate.setTemplate(SC_STRLOAD(RID_STATISTICS_DLGS, STR_COLUMN_LABEL_TEMPLATE));
        else
            aTemplate.setTemplate(SC_STRLOAD(RID_STATISTICS_DLGS, STR_ROW_LABEL_TEMPLATE));

        aTemplate.applyNumber(strWildcardNumber, i + 1);
        output.writeString(aTemplate.getTemplate());
        output.nextColumn();
    }

    // write labels to rows
    output.resetColumn();
    output.nextRow();
    for (size_t i = 0; i < aRangeList.size(); i++)
    {
        if (mGroupedBy == BY_COLUMN)
            aTemplate.setTemplate(SC_STRLOAD(RID_STATISTICS_DLGS, STR_COLUMN_LABEL_TEMPLATE));
        else
            aTemplate.setTemplate(SC_STRLOAD(RID_STATISTICS_DLGS, STR_ROW_LABEL_TEMPLATE));

        aTemplate.applyNumber(strWildcardNumber, i + 1);
        output.writeString(aTemplate.getTemplate());
        output.nextRow();
    }

    // write correlation formulas
    output.reset();
    output.push(1, 1);

    lclWriteCorrelationFormulas(output, aTemplate, aRangeList, getTemplate());

    return ScRange(output.mMinimumAddress, output.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
