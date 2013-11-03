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
#include "docfunc.hxx"
#include "globstr.hrc"
#include "sc.hrc"

#include "ExponentialSmoothingDialog.hxx"

namespace
{
    static const OUString strWildcardRange("%RANGE%");

    class Template
    {
    public:
        OUString            mTemplate;
        ScDocument*         mDocument;
        ScAddress::Details  mAddressDetails;

        Template(ScDocument* aDocument, ScAddress::Details aAddressDetails) :
            mDocument(aDocument),
            mAddressDetails(aAddressDetails)
        {}

        void setTemplate(OUString aTemplate)
        {
            mTemplate = aTemplate;
        }

        OUString& getTemplate()
        {
            return mTemplate;
        }

        void applyRange(OUString aVariable, ScRange& aRange)
        {
            OUString aRangeString = aRange.Format(SCR_ABS, mDocument, mAddressDetails);

            mTemplate = mTemplate.replaceAll(aVariable, aRangeString);
        }

    };
}

ScExponentialSmoothingDialog::ScExponentialSmoothingDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    Window* pParent, ScViewData* pViewData ) :
    ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "DescriptiveStatisticsDialog", "modules/scalc/ui/descriptivestatisticsdialog.ui" )
{}

ScExponentialSmoothingDialog::~ScExponentialSmoothingDialog()
{}

sal_Bool ScExponentialSmoothingDialog::Close()
{
    return DoClose( ScExponentialSmoothingDialogWrapper::GetChildWindowId() );
}

void ScExponentialSmoothingDialog::CalculateInputAndWriteToOutput( )
{
    OUString aUndo("Exponential");
    ScDocShell* pDocShell = mViewData->GetDocShell();
    svl::IUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction( aUndo, aUndo );

    ScAddress aStart = mInputRange.aStart;
    ScAddress aEnd   = mInputRange.aEnd;

    SCTAB outTab = mOutputAddress.Tab();
    SCCOL outCol = mOutputAddress.Col();
    SCROW outRow = mOutputAddress.Row();

    ScAddress aAddress;

    SCROW inTab = aStart.Tab();

    Template aTemplate(mDocument, mAddressDetails);

    // Dampning factor
    ScAddress aDampFactorAddress(outCol, outRow, outTab);
    pDocShell->GetDocFunc().SetValueCell(aDampFactorAddress, 0.2, true);
    OUString aDampFactorAddressString = aDampFactorAddress.Format(SCR_ABS, mDocument, mAddressDetails);
    outRow++;

    // Exponential Smoothing
    SCROW firstOutRow = outRow;

    for (SCCOL inCol = aStart.Col(); inCol <= aEnd.Col(); inCol++)
    {
        outRow = firstOutRow;
        aAddress = ScAddress(outCol, outRow, outTab);

        SCROW inRow = aStart.Row();

        if (false)
        {
            ScRange aColumnRange (
                    ScAddress(inCol, mInputRange.aStart.Row(), inTab),
                    ScAddress(inCol, mInputRange.aEnd.Row(), inTab));

            OUString aAverageFormulaTemplate = OUString("=AVERAGE(%RANGE%)");
            OUString aColumnRangeString = aColumnRange.Format(SCR_ABS, mDocument, mAddressDetails);
            OUString aAverageFormulaString = aAverageFormulaTemplate.replaceAll(OUString("%RANGE%"), aColumnRangeString);

            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aAverageFormulaString), true);
        }
        else
        {
            ScAddress aFirstValueAddress(inCol, mInputRange.aStart.Row(), inTab);
            OUString aFirstValueAddressString = aFirstValueAddress.Format(SCR_ABS, mDocument, mAddressDetails);
            OUString aFirstValueFormulaTemplate = OUString("=%RANGE%");
            OUString aFistValueString = aFirstValueFormulaTemplate.replaceAll(OUString("%RANGE%"), aFirstValueAddressString);
            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFistValueString), true);
        }

        outRow++;

        for (inRow = aStart.Row() + 1; inRow <= aEnd.Row(); inRow++)
        {
            ScAddress aPreviousInputAddress(inCol,   inRow  - 1, inTab);
            ScAddress aPreviousOutputAddress(outCol, outRow - 1, outTab);

            OUString aPreviousInputAddressString  = aPreviousInputAddress.Format(SCR_ABS, mDocument, mAddressDetails);
            OUString aPreviousOutputAddressString = aPreviousOutputAddress.Format(SCR_ABS, mDocument, mAddressDetails);

            aAddress = ScAddress(outCol, outRow, outTab);
            OUString aFormulaTemplate = OUString("=%VALUE% * %PREVIOUS_INPUT% + (1 - %VALUE%) * %PREVIOUS_OUTPUT%");
            OUString aFormulaString = aFormulaTemplate;
            aFormulaString = aFormulaString.replaceAll(OUString("%PREVIOUS_INPUT%"), aPreviousInputAddressString);
            aFormulaString = aFormulaString.replaceAll(OUString("%PREVIOUS_OUTPUT%"), aPreviousOutputAddressString);
            aFormulaString = aFormulaString.replaceAll(OUString("%VALUE%"), aDampFactorAddressString);

            pDocShell->GetDocFunc().SetFormulaCell(aAddress, new ScFormulaCell(mDocument, aAddress, aFormulaString), true);
            outRow++;
        }
        outCol++;
    }

    ScRange aOutputRange(mOutputAddress, ScAddress(outTab, outRow, outTab) );
    pUndoManager->LeaveListAction();
    pDocShell->PostPaint( aOutputRange, PAINT_GRID );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
