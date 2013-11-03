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

#include "MatrixComparisonGenerator.hxx"

namespace
{
    static const OUString strWildcard1("%VAR1%");
    static const OUString strWildcard2("%VAR2%");

    static const OUString strWildcardNumber("%NUMBER%");
    static const OUString strColumnLabelTemplate("Column %NUMBER%");
    static const OUString strRowLabelTemplate("Row %NUMBER%");
}

ScMatrixComparisonGenerator::ScMatrixComparisonGenerator(
                                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                                    Window* pParent, ScViewData* pViewData, const OString& rID,
                                    const OUString& rUiXmlDescription ) :
    ScStatisticsInputOutputDialog(pSfxBindings, pChildWindow, pParent, pViewData, rID, rUiXmlDescription)
{}

ScMatrixComparisonGenerator::~ScMatrixComparisonGenerator()
{}

void ScMatrixComparisonGenerator::CalculateInputAndWriteToOutput( )
{
    OUString aUndo(SC_STRLOAD( RID_STATISTICS_DLGS, STR_CORRELATION_UNDO_NAME));
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    ScAddress aStart = mInputRange.aStart;
    ScAddress aEnd   = mInputRange.aEnd;

    SCTAB outTab = mOutputAddress.Tab();
    SCCOL outCol = mOutputAddress.Col();
    SCROW outRow = mOutputAddress.Row();

    ScAddress aAddress;
    SCCOL aMaxCol = 0;
    SCROW aMaxRow = 0;

    SCTAB inTab = aStart.Tab();

    ScRangeList aRangeList;

    if (mGroupedBy == BY_COLUMN)
        aRangeList = MakeColumnRangeList(inTab, aStart, aEnd);
    else
        aRangeList = MakeRowRangeList(inTab, aStart, aEnd);

    // labels
    aAddress = ScAddress(outCol, outRow, outTab);
    pDocShell->GetDocFunc().SetStringCell(aAddress, getLabel(), true);
    outCol++;
    aMaxCol = outCol > aMaxCol ? outCol : aMaxCol;

    // write labels to columns
    for (size_t i = 0; i < aRangeList.size(); i++)
    {
        aAddress = ScAddress(outCol, outRow, outTab);
        OUString aLabel;
        if (mGroupedBy == BY_COLUMN)
            aLabel = strColumnLabelTemplate.replaceAll(strWildcardNumber, OUString::number(i + 1));
        else
            aLabel = strRowLabelTemplate.replaceAll(strWildcardNumber, OUString::number(i + 1));

        pDocShell->GetDocFunc().SetStringCell(aAddress, aLabel, true);
        outCol++;
        aMaxCol = outCol > aMaxCol ? outCol : aMaxCol;
    }

    // write labels to rows
    outCol = mOutputAddress.Col();
    outRow++;
    for (size_t i = 0; i < aRangeList.size(); i++)
    {
        aAddress = ScAddress(outCol, outRow, outTab);
        OUString aLabel;

        if (mGroupedBy == BY_COLUMN)
            aLabel = strColumnLabelTemplate.replaceAll(strWildcardNumber, OUString::number(i + 1));
        else
            aLabel = strRowLabelTemplate.replaceAll(strWildcardNumber, OUString::number(i + 1));

        pDocShell->GetDocFunc().SetStringCell(aAddress, aLabel, true);
        outRow++;
        aMaxRow = outRow > aMaxRow ? outRow : aMaxRow;
    }

    // write correlation formulas
    aAddress = ScAddress(
                mOutputAddress.Col() + 1,
                mOutputAddress.Row() + 1,
                inTab);

    if (mGroupedBy == BY_COLUMN)
        writeCorrelationFormulasByColumn(aAddress, aRangeList);
    else
        writeCorrelationFormulasByRow(aAddress, aRangeList);

    ScAddress aLastAddress = ScAddress(
                                mOutputAddress.Col() + aMaxCol,
                                mOutputAddress.Row() + aMaxRow,
                                outTab);

    ScRange aOutputRange(mOutputAddress, aLastAddress);
    pUndoManager->LeaveListAction();
    pDocShell->PostPaint(aOutputRange, PAINT_GRID);
}

void ScMatrixComparisonGenerator::writeCorrelationFormulasByColumn(ScAddress aOutputAddress, ScRangeList aRangeList)
{
    ScDocShell* pDocShell = mViewData->GetDocShell();
    ScAddress aAddress;

    SCTAB outTab = aOutputAddress.Tab();
    SCCOL outCol = aOutputAddress.Col();

    OUString aFormulaString;
    const OUString& aTemplate = getTemplate();

    for (size_t i = 0; i < aRangeList.size(); i++)
    {
        SCROW outRow = aOutputAddress.Row();
        for (size_t j = 0; j < aRangeList.size(); j++)
        {
            if (j >= i)
            {
                OUString aString1(aRangeList[i]->Format(SCR_ABS, mDocument, mAddressDetails));
                OUString aString2(aRangeList[j]->Format(SCR_ABS, mDocument, mAddressDetails));

                aAddress = ScAddress(outCol, outRow, outTab);
                aFormulaString = aTemplate.replaceAll(strWildcard1, aString1);
                aFormulaString = aFormulaString.replaceAll(strWildcard2, aString2);
                pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
            }
            outRow++;
        }
        outCol++;
    }
}

void ScMatrixComparisonGenerator::writeCorrelationFormulasByRow(ScAddress aOutputAddress, ScRangeList aRangeList)
{
    ScDocShell* pDocShell = mViewData->GetDocShell();
    ScAddress aAddress;

    SCTAB outTab = aOutputAddress.Tab();
    SCCOL outCol = aOutputAddress.Col();

    OUString aFormulaString;
    const OUString& aTemplate = getTemplate();

    for (size_t i = 0; i < aRangeList.size(); i++)
    {
        SCROW outRow = aOutputAddress.Row();
        for (size_t j = 0; j < aRangeList.size(); j++)
        {
            if (j >= i)
            {
                OUString aString1(aRangeList[i]->Format(SCR_ABS, mDocument, mAddressDetails));
                OUString aString2(aRangeList[j]->Format(SCR_ABS, mDocument, mAddressDetails));

                aAddress = ScAddress(outCol, outRow, outTab);
                aFormulaString = aTemplate.replaceAll(strWildcard1, aString1);
                aFormulaString = aFormulaString.replaceAll(strWildcard2, aString2);
                pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
            }
            outRow++;
        }
        outCol++;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
