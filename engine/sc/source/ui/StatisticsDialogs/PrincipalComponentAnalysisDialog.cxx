/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>

#include <svl/undo.hxx>

#include <docfunc.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <rangelst.hxx>
#include <reffact.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <tabvwsh.hxx>

#include <PrincipalComponentAnalysisDialog.hxx>
#include <TableFillingAndNavigationTools.hxx>

namespace
{
// The mean of a column goes on the first row of the new sheet and the standard
// deviation on the second, and every standardized value in that column points
// at those two cells.
const SCROW nMeanRow = 0;
const SCROW nDeviationRow = 1;
const SCROW nLabelRow = 2;
const SCROW nFirstValueRow = nLabelRow + 1;

OUString FillIn(TranslateId aMessageId, std::u16string_view aVariable, std::u16string_view aValue)
{
    return ScResId(aMessageId).replaceAll(aVariable, aValue);
}
}

ScPrincipalComponentAnalysisDialog::ScPrincipalComponentAnalysisDialog(SfxBindings* pSfxBindings,
                                                                       SfxChildWindow* pChildWindow,
                                                                       weld::Window* pParent,
                                                                       ScViewData& rViewData)
    : ScAnyRefDlgController(pSfxBindings, pChildWindow, pParent,
                            u"modules/scalc/ui/principalcomponentanalysisdialog.ui"_ustr,
                            u"PrincipalComponentAnalysisDialog"_ustr)
    , mrViewData(rViewData)
    , mrDocument(rViewData.GetDocument())
    , maInputRange(ScAddress::INITIALIZE_INVALID)
    , maAddressDetails(mrDocument.GetAddressConvention(), 0, 0)
    , maDataRange(ScAddress::INITIALIZE_INVALID)
    , mbWithLabels(false)
    , mbDialogLostFocus(false)
    , mxInputRangeLabel(m_xBuilder->weld_label(u"input-range-label"_ustr))
    , mxInputRangeEdit(new formula::RefEdit(m_xBuilder->weld_entry(u"input-range-edit"_ustr)))
    , mxInputRangeButton(
          new formula::RefButton(m_xBuilder->weld_button(u"input-range-button"_ustr)))
    , mxWithLabelsCheckBox(m_xBuilder->weld_check_button(u"withlabels-check"_ustr))
    , mxErrorMessage(m_xBuilder->weld_label(u"error-message"_ustr))
    , mxButtonCalculate(m_xBuilder->weld_button(u"ok"_ustr))
    , mxButtonCancel(m_xBuilder->weld_button(u"cancel"_ustr))
{
    mxInputRangeEdit->SetReferences(this, mxInputRangeLabel.get());
    mxInputRangeButton->SetReferences(this, mxInputRangeEdit.get());

    Init();
    GetRangeFromSelection();
    ValidateDialogInput();
}

ScPrincipalComponentAnalysisDialog::~ScPrincipalComponentAnalysisDialog() {}

void ScPrincipalComponentAnalysisDialog::Init()
{
    mxButtonCalculate->connect_clicked(
        LINK(this, ScPrincipalComponentAnalysisDialog, ButtonClicked));
    mxButtonCancel->connect_clicked(LINK(this, ScPrincipalComponentAnalysisDialog, ButtonClicked));

    mxInputRangeEdit->SetGetFocusHdl(
        LINK(this, ScPrincipalComponentAnalysisDialog, GetEditFocusHandler));
    mxInputRangeButton->SetGetFocusHdl(
        LINK(this, ScPrincipalComponentAnalysisDialog, GetButtonFocusHandler));
    mxInputRangeEdit->SetLoseFocusHdl(
        LINK(this, ScPrincipalComponentAnalysisDialog, LoseEditFocusHandler));
    mxInputRangeButton->SetLoseFocusHdl(
        LINK(this, ScPrincipalComponentAnalysisDialog, LoseButtonFocusHandler));
    mxInputRangeEdit->SetModifyHdl(
        LINK(this, ScPrincipalComponentAnalysisDialog, RefInputModifyHandler));

    mxWithLabelsCheckBox->connect_toggled(
        LINK(this, ScPrincipalComponentAnalysisDialog, CheckBoxToggled));

    mxInputRangeEdit->GrabFocus();
}

void ScPrincipalComponentAnalysisDialog::GetRangeFromSelection()
{
    mrViewData.GetSimpleArea(maInputRange);
    mxInputRangeEdit->SetText(
        maInputRange.Format(mrDocument, ScRefFlags::RANGE_ABS_3D, maAddressDetails));
}

void ScPrincipalComponentAnalysisDialog::SetActive()
{
    if (mbDialogLostFocus)
    {
        mbDialogLostFocus = false;
        mxInputRangeEdit->GrabFocus();
    }
    else
    {
        m_xDialog->grab_focus();
    }
    RefInputDone();
}

void ScPrincipalComponentAnalysisDialog::Close()
{
    DoClose(ScPrincipalComponentAnalysisDialogWrapper::GetChildWindowId());
}

void ScPrincipalComponentAnalysisDialog::SetReference(const ScRange& rReferenceRange,
                                                      ScDocument& rDocument)
{
    if (rReferenceRange.aStart != rReferenceRange.aEnd)
        RefInputStart(mxInputRangeEdit.get());

    maInputRange = rReferenceRange;
    mxInputRangeEdit->SetRefString(
        maInputRange.Format(rDocument, ScRefFlags::RANGE_ABS_3D, maAddressDetails));

    ValidateDialogInput();
}

void ScPrincipalComponentAnalysisDialog::ValidateDialogInput()
{
    mxButtonCalculate->set_sensitive(maInputRange.IsValid());
}

OUString ScPrincipalComponentAnalysisDialog::GetOutputSheetName() const
{
    OUString aSourceSheetName;
    mrDocument.GetName(maInputRange.aStart.Tab(), aSourceSheetName);
    return aSourceSheetName + "_PCA";
}

bool ScPrincipalComponentAnalysisDialog::InputIsValid()
{
    mxErrorMessage->set_label(OUString());

    if (!maInputRange.IsValid())
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_INVALID_INPUT_RANGE));
        return false;
    }

    maInputRange.PutInOrder();

    if (maInputRange.aStart.Tab() != maInputRange.aEnd.Tab())
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_RANGE_SPANS_SHEETS));
        return false;
    }

    mbWithLabels = mxWithLabelsCheckBox->get_active();

    maDataRange = maInputRange;
    if (mbWithLabels)
        maDataRange.aStart.IncRow();

    const SCROW nRowCount = maDataRange.aEnd.Row() - maDataRange.aStart.Row() + 1;
    const SCCOL nColumnCount = maDataRange.aEnd.Col() - maDataRange.aStart.Col() + 1;

    // Two rows are the fewest a standard deviation can be worked out from, and
    // a single column has nothing to be correlated with.
    if (nRowCount < 2)
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_TOO_FEW_DATA_ROWS));
        return false;
    }

    if (nColumnCount < 2)
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_TOO_FEW_DATA_COLUMNS));
        return false;
    }

    const sal_Int32 nRank = std::min<sal_Int32>(nRowCount, nColumnCount);
    // The right vectors have one row per column of the data, so with more
    // columns than rows they reach further down than the standardized values.
    const sal_Int32 nBlockHeight = std::max<sal_Int32>(nRowCount, nColumnCount);

    if (nColumnCount + 2 * nRank + 2 > mrDocument.MaxCol()
        || nFirstValueRow + nBlockHeight - 1 > mrDocument.MaxRow())
    {
        mxErrorMessage->set_label(ScResId(STR_MESSAGE_OUTPUT_TOO_LONG));
        return false;
    }

    // Every column is standardized against its own mean and standard
    // deviation, so all of them have to cover the same set of rows.
    for (SCCOL nColumn = maDataRange.aStart.Col(); nColumn <= maDataRange.aEnd.Col(); ++nColumn)
    {
        for (SCROW nRow = maDataRange.aStart.Row(); nRow <= maDataRange.aEnd.Row(); ++nRow)
        {
            const ScAddress aPosition(nColumn, nRow, maDataRange.aStart.Tab());
            if (!mrDocument.HasValueData(aPosition))
            {
                const OUString aPositionName
                    = aPosition.Format(ScRefFlags::ADDR_ABS_3D, &mrDocument, maAddressDetails);
                mxErrorMessage->set_label(
                    FillIn(STR_MESSAGE_NOT_A_NUMBER, u"%ADDRESS%", aPositionName));
                return false;
            }
        }
    }

    const OUString aOutputSheetName = GetOutputSheetName();
    SCTAB nExistingTab = 0;
    if (mrDocument.GetTable(aOutputSheetName, nExistingTab))
    {
        mxErrorMessage->set_label(
            FillIn(STR_MESSAGE_SHEET_ALREADY_EXISTS, u"%NAME%", aOutputSheetName));
        return false;
    }

    return true;
}

void ScPrincipalComponentAnalysisDialog::Calculate()
{
    ScDocShell* pDocShell = mrViewData.GetDocShell();
    const SCTAB nOutputTab = maDataRange.aStart.Tab() + 1;

    const OUString aUndo(ScResId(STR_PRINCIPAL_COMPONENT_ANALYSIS_UNDO_NAME));
    SfxUndoManager* pUndoManager = pDocShell->GetUndoManager();
    pUndoManager->EnterListAction(aUndo, aUndo, 0, mrViewData.GetViewShell()->GetViewShellId());

    ScRange aOutputRange(ScAddress::INITIALIZE_INVALID);
    if (pDocShell->GetDocFunc().InsertTable(nOutputTab, GetOutputSheetName(), true, false))
        aOutputRange = WriteOutput(*pDocShell, nOutputTab);

    pUndoManager->LeaveListAction();

    if (!aOutputRange.IsValid())
        return;

    pDocShell->PostPaint(aOutputRange, PaintPartFlags::Grid);
    mrViewData.GetViewShell()->SetTabNo(nOutputTab, true);
}

ScRange ScPrincipalComponentAnalysisDialog::WriteOutput(ScDocShell& rDocShell, SCTAB nOutputTab)
{
    const SCTAB nSourceTab = maDataRange.aStart.Tab();
    const SCCOL nColumnCount = maDataRange.aEnd.Col() - maDataRange.aStart.Col() + 1;
    const SCROW nRowCount = maDataRange.aEnd.Row() - maDataRange.aStart.Row() + 1;
    const SCCOL nRank = static_cast<SCCOL>(std::min<sal_Int32>(nRowCount, nColumnCount));

    AddressWalkerWriter aOutput(ScAddress(0, nMeanRow, nOutputTab), &rDocShell, mrDocument,
                                formula::FormulaGrammar::mergeToGrammar(
                                    formula::FormulaGrammar::GRAM_ENGLISH, maAddressDetails.eConv));
    FormulaTemplate aTemplate(&mrDocument);

    auto aSourceColumn = [this, nSourceTab](SCCOL nColumn) {
        const SCCOL nSourceColumn = maDataRange.aStart.Col() + nColumn;
        return ScRange(ScAddress(nSourceColumn, maDataRange.aStart.Row(), nSourceTab),
                       ScAddress(nSourceColumn, maDataRange.aEnd.Row(), nSourceTab));
    };

    for (SCCOL nColumn = 0; nColumn < nColumnCount; ++nColumn)
    {
        aTemplate.setTemplate("=AVERAGE(%COLUMN%)");
        aTemplate.applyRange(u"%COLUMN%", aSourceColumn(nColumn));
        aOutput.writeFormula(aTemplate.getTemplate());
        aOutput.nextColumn();
    }
    aOutput.newLine();

    for (SCCOL nColumn = 0; nColumn < nColumnCount; ++nColumn)
    {
        aTemplate.setTemplate("=STDEV(%COLUMN%)");
        aTemplate.applyRange(u"%COLUMN%", aSourceColumn(nColumn));
        aOutput.writeFormula(aTemplate.getTemplate());
        aOutput.nextColumn();
    }
    aOutput.newLine();

    auto aNumberedLabel = [&aTemplate](TranslateId aTemplateId, SCCOL nColumn) {
        aTemplate.setTemplate(ScResId(aTemplateId));
        aTemplate.applyNumber(u"%NUMBER%", nColumn + 1);
        return aTemplate.getTemplate();
    };

    // A header row naming every column of what follows. The features take the
    // labels of the source columns, and a source column with no label of its
    // own is named after its place in the range.
    for (SCCOL nColumn = 0; nColumn < nColumnCount; ++nColumn)
    {
        OUString aLabel;
        if (mbWithLabels)
            aLabel = mrDocument.GetString(maDataRange.aStart.Col() + nColumn,
                                          maInputRange.aStart.Row(), nSourceTab);
        if (aLabel.isEmpty())
            aLabel = aNumberedLabel(STR_FEATURE_LABEL_TEMPLATE, nColumn);
        aOutput.writeBoldString(aLabel);
        aOutput.nextColumn();
    }

    // One score column per component, holding where each observation sits
    // along it.
    for (SCCOL nColumn = 0; nColumn < nRank; ++nColumn)
    {
        aOutput.writeBoldString(aNumberedLabel(STR_COMPONENT_SCORE_LABEL_TEMPLATE, nColumn));
        aOutput.nextColumn();
    }

    aOutput.writeBoldString(ScResId(STR_SINGULAR_VALUE_LABEL));
    aOutput.nextColumn();

    // One loading column per component, holding how much each feature weighs
    // in it.
    for (SCCOL nColumn = 0; nColumn < nRank; ++nColumn)
    {
        aOutput.writeBoldString(aNumberedLabel(STR_COMPONENT_LOADING_LABEL_TEMPLATE, nColumn));
        aOutput.nextColumn();
    }

    aOutput.writeBoldString(ScResId(STR_VARIANCE_SHARE_LABEL));
    aOutput.nextColumn();
    aOutput.writeBoldString(ScResId(STR_CUMULATIVE_VARIANCE_SHARE_LABEL));
    aOutput.newLine();

    for (SCCOL nColumn = 0; nColumn < nColumnCount; ++nColumn)
    {
        aTemplate.setTemplate("=(%COLUMN%-%MEAN%)/%DEVIATION%");
        aTemplate.applyRange(u"%COLUMN%", aSourceColumn(nColumn));
        aTemplate.applyAddress(u"%MEAN%", ScAddress(nColumn, nMeanRow, nOutputTab), false);
        aTemplate.applyAddress(u"%DEVIATION%", ScAddress(nColumn, nDeviationRow, nOutputTab),
                               false);
        aOutput.writeMatrixFormula(aTemplate.getTemplate(), 1, nRowCount);
        aOutput.nextColumn();
    }

    // The three parts of the decomposition sit to the right of the
    // standardized values and start on the same row as them.
    const ScRange aValueRange(
        ScAddress(0, nFirstValueRow, nOutputTab),
        ScAddress(nColumnCount - 1, nFirstValueRow + nRowCount - 1, nOutputTab));
    aTemplate.autoReplaceRange(u"%VALUES%"_ustr, aValueRange);
    aTemplate.autoReplaceUses3D(false);

    // The left vectors, one row per observation and one column per component.
    aTemplate.setTemplate("=MSVD(%VALUES%;1)");
    aOutput.writeMatrixFormula(aTemplate.getTemplate(), nRank, nRowCount);

    for (SCCOL nColumn = 0; nColumn < nRank; ++nColumn)
        aOutput.nextColumn();

    // The singular values, in descending order.
    const ScRange aSingularValueRange(
        ScAddress(aOutput.mCurrentAddress.Col(), nFirstValueRow, nOutputTab),
        ScAddress(aOutput.mCurrentAddress.Col(), nFirstValueRow + nRank - 1, nOutputTab));
    aTemplate.setTemplate("=MSVD(%VALUES%;2)");
    aOutput.writeMatrixFormula(aTemplate.getTemplate(), 1, nRank);
    aOutput.nextColumn();

    // The right vectors, one row per column of the data and one column per
    // component.
    aTemplate.setTemplate("=MSVD(%VALUES%;3)");
    aOutput.writeMatrixFormula(aTemplate.getTemplate(), nRank, nColumnCount);

    for (SCCOL nColumn = 0; nColumn < nRank; ++nColumn)
        aOutput.nextColumn();

    // Both of the last two columns are a share of a whole, so they read as
    // percentages.
    const SCCOL nShareColumn = aOutput.mCurrentAddress.Col();
    aOutput.formatAsPercentage(2, nRank);

    // A singular value squared is the variance the data has along that
    // component, so the share of the total of the squares is the share of the
    // variance.
    aTemplate.setTemplate("=(%SINGULARVALUES%)^2/SUMSQ(%SINGULARVALUES%)");
    aTemplate.applyRange(u"%SINGULARVALUES%", aSingularValueRange, false);
    aOutput.writeMatrixFormula(aTemplate.getTemplate(), 1, nRank);
    aOutput.nextColumn();

    // The running total of those shares, which says how much of the variance
    // the first few components carry between them.
    for (SCROW nRow = 0; nRow < nRank; ++nRow)
    {
        const ScRange aSharesSoFar(ScAddress(nShareColumn, nFirstValueRow, nOutputTab),
                                   ScAddress(nShareColumn, nFirstValueRow + nRow, nOutputTab));
        aTemplate.setTemplate("=SUM(%SHARES%)");
        aTemplate.applyRange(u"%SHARES%", aSharesSoFar, false);
        aOutput.writeFormula(aTemplate.getTemplate());
        aOutput.nextRow();
    }

    const SCROW nBlockHeight = std::max<SCROW>(nRowCount, nColumnCount);
    return ScRange(
        ScAddress(0, nMeanRow, nOutputTab),
        ScAddress(nColumnCount + 2 * nRank + 2, nFirstValueRow + nBlockHeight - 1, nOutputTab));
}

IMPL_LINK(ScPrincipalComponentAnalysisDialog, ButtonClicked, weld::Button&, rButton, void)
{
    if (&rButton != mxButtonCalculate.get())
    {
        response(RET_CANCEL);
        return;
    }

    if (!InputIsValid())
        return;

    Calculate();
    response(RET_OK);
}

IMPL_LINK_NOARG(ScPrincipalComponentAnalysisDialog, CheckBoxToggled, weld::Toggleable&, void)
{
    ValidateDialogInput();
}

IMPL_LINK_NOARG(ScPrincipalComponentAnalysisDialog, GetEditFocusHandler, formula::RefEdit&, void)
{
    mxInputRangeEdit->SelectAll();
}

IMPL_LINK_NOARG(ScPrincipalComponentAnalysisDialog, GetButtonFocusHandler, formula::RefButton&,
                void)
{
    mxInputRangeEdit->SelectAll();
}

IMPL_LINK_NOARG(ScPrincipalComponentAnalysisDialog, LoseEditFocusHandler, formula::RefEdit&, void)
{
    mbDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScPrincipalComponentAnalysisDialog, LoseButtonFocusHandler, formula::RefButton&,
                void)
{
    mbDialogLostFocus = !m_xDialog->has_toplevel_focus();
}

IMPL_LINK_NOARG(ScPrincipalComponentAnalysisDialog, RefInputModifyHandler, formula::RefEdit&, void)
{
    ScRangeList aRangeList;
    const bool bValid = ParseWithNames(aRangeList, mxInputRangeEdit->GetText(), mrDocument);
    if (bValid && aRangeList.size() == 1)
    {
        maInputRange = aRangeList[0];
        // Highlight the resulting range.
        mxInputRangeEdit->StartUpdateData();
    }
    else
    {
        maInputRange = ScRange(ScAddress::INITIALIZE_INVALID);
    }

    ValidateDialogInput();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
