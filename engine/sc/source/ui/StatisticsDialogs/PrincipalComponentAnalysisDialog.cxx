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

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeManager.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>

#include <comphelper/diagnose_ex.hxx>
#include <comphelper/processfactory.hxx>
#include <svl/numformat.hxx>
#include <svl/undo.hxx>

#include <docfunc.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <docuno.hxx>
#include <drwlayer.hxx>
#include <global.hxx>
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

// Chart sizes and the gap between the two, in hundredths of a millimetre. The
// correlation circle is square because the circle in it only reads as a circle
// while both of its axes are drawn at the same scale.
const sal_Int32 nChartWidth = 12000;
const sal_Int32 nChartHeight = 8000;
const sal_Int32 nChartGap = 500;
const sal_Int32 nCircleChartSide = 13500;

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

    // Two columns past the last one written, where the chart is anchored.
    if (nColumnCount + 2 * nRank + 4 > mrDocument.MaxCol()
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
    // The column the scores start in further down is free this high up, so the
    // label for the row goes there, right of the values it names.
    aOutput.writeBoldString(ScResId(STRID_CALC_MEAN));
    aOutput.newLine();

    for (SCCOL nColumn = 0; nColumn < nColumnCount; ++nColumn)
    {
        aTemplate.setTemplate("=STDEV(%COLUMN%)");
        aTemplate.applyRange(u"%COLUMN%", aSourceColumn(nColumn));
        aOutput.writeFormula(aTemplate.getTemplate());
        aOutput.nextColumn();
    }
    aOutput.writeBoldString(ScResId(STRID_CALC_STD_DEVIATION));
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

    // The chart reads the two share columns together with the header row above
    // them, which names the two things it draws.
    const ScRange aShareRange(ScAddress(nShareColumn, nLabelRow, nOutputTab),
                              ScAddress(nShareColumn + 1, nFirstValueRow + nRank - 1, nOutputTab));
    AddVarianceChart(rDocShell, aShareRange, nShareColumn + 3);

    // The standardized values run straight into the scores, so one range holds
    // the features followed by the first two components, which is the order the
    // correlation circle reads them in.
    const ScRange aCorrelationRange(
        ScAddress(0, nLabelRow, nOutputTab),
        ScAddress(nColumnCount + 1, nFirstValueRow + nRowCount - 1, nOutputTab));
    AddCorrelationCircleChart(rDocShell, aCorrelationRange, nShareColumn + 3);

    const SCROW nBlockHeight = std::max<SCROW>(nRowCount, nColumnCount);
    return ScRange(
        ScAddress(0, nMeanRow, nOutputTab),
        ScAddress(nColumnCount + 2 * nRank + 2, nFirstValueRow + nBlockHeight - 1, nOutputTab));
}

OUString ScPrincipalComponentAnalysisDialog::GetChartName(SCTAB nOutputTab) const
{
    OUString aSheetName;
    mrDocument.GetName(nOutputTab, aSheetName);
    const OUString aBaseName = aSheetName + "_Chart";

    ScDrawLayer* pModel = mrDocument.GetDrawLayer();
    if (!pModel)
        return aBaseName;

    // A drawing object name has to be free across every sheet, not just this
    // one, so count up until nothing answers to it.
    OUString aName = aBaseName;
    SCTAB nFoundTab = 0;
    for (sal_Int32 nSuffix = 2; pModel->GetNamedObject(aName, SdrObjKind::OLE2, nFoundTab);
         ++nSuffix)
    {
        aName = aBaseName + "_" + OUString::number(nSuffix);
    }
    return aName;
}

css::uno::Reference<css::chart2::XTitle>
ScPrincipalComponentAnalysisDialog::MakeChartTitle(const OUString& rText)
{
    css::uno::Reference<css::lang::XMultiServiceFactory> xServiceFactory(
        comphelper::getProcessServiceFactory(), css::uno::UNO_SET_THROW);
    css::uno::Reference<css::chart2::XFormattedString> xTitleText(
        xServiceFactory->createInstance(u"com.sun.star.chart2.FormattedString"_ustr),
        css::uno::UNO_QUERY_THROW);
    xTitleText->setString(rText);
    css::uno::Reference<css::chart2::XTitle> xTitle(
        xServiceFactory->createInstance(u"com.sun.star.chart2.Title"_ustr),
        css::uno::UNO_QUERY_THROW);
    xTitle->setText({ xTitleText });
    return xTitle;
}

css::uno::Reference<css::chart2::XChartDocument>
ScPrincipalComponentAnalysisDialog::CreateSheetChart(ScDocShell& rDocShell,
                                                     const ScRange& rDataRange,
                                                     const css::awt::Rectangle& rRectangle)
{
    const SCTAB nOutputTab = rDataRange.aStart.Tab();
    css::uno::Reference<css::container::XIndexAccess> xSheets(rDocShell.GetModel()->getSheets(),
                                                              css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::table::XTableChartsSupplier> xSupplier(xSheets->getByIndex(nOutputTab),
                                                                    css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::table::XTableCharts> xCharts = xSupplier->getCharts();

    const css::table::CellRangeAddress aAddress(nOutputTab, rDataRange.aStart.Col(),
                                                rDataRange.aStart.Row(), rDataRange.aEnd.Col(),
                                                rDataRange.aEnd.Row());
    const OUString aChartName = GetChartName(nOutputTab);
    // The first cell of each column names the series it heads, and the rows
    // need no names of their own along the bottom.
    xCharts->addNewByName(aChartName, rRectangle, { aAddress }, true, false);

    css::uno::Reference<css::container::XNameAccess> xChartsByName(xCharts,
                                                                   css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::document::XEmbeddedObjectSupplier> xObjectSupplier(
        xChartsByName->getByName(aChartName), css::uno::UNO_QUERY_THROW);
    return css::uno::Reference<css::chart2::XChartDocument>(xObjectSupplier->getEmbeddedObject(),
                                                            css::uno::UNO_QUERY_THROW);
}

void ScPrincipalComponentAnalysisDialog::AddVarianceChart(ScDocShell& rDocShell,
                                                          const ScRange& rShareRange,
                                                          SCCOL nChartColumn)
{
    try
    {
        // Anchor the chart at the top of the sheet, clear of the numbers.
        const tools::Rectangle aCell = ScDrawLayer::GetCellRect(
            mrDocument, ScAddress(nChartColumn, nMeanRow, rShareRange.aStart.Tab()), false);
        css::uno::Reference<css::chart2::XChartDocument> xChartDocument = CreateSheetChart(
            rDocShell, rShareRange,
            css::awt::Rectangle(aCell.Left(), aCell.Top(), nChartWidth, nChartHeight));

        // Bars for the share each component carries with a line over them for
        // the running total. The template draws the last series of the two as
        // the line, which is the running total.
        css::uno::Reference<css::lang::XMultiServiceFactory> xTemplateFactory(
            xChartDocument->getChartTypeManager(), css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::chart2::XChartTypeTemplate> xTemplate(
            xTemplateFactory->createInstance(u"com.sun.star.chart2.template.ColumnWithLine"_ustr),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::chart2::XDiagram> xDiagram = xChartDocument->getFirstDiagram();
        xTemplate->changeDiagram(xDiagram);

        css::uno::Reference<css::chart2::XTitled> xTitled(xChartDocument,
                                                          css::uno::UNO_QUERY_THROW);
        xTitled->setTitleObject(MakeChartTitle(ScResId(STR_VARIANCE_SHARE_CHART_TITLE)));

        css::uno::Reference<css::chart2::XCoordinateSystemContainer> xCoordinateSystems(
            xDiagram, css::uno::UNO_QUERY_THROW);
        const auto aSystems = xCoordinateSystems->getCoordinateSystems();
        if (aSystems.hasElements())
        {
            // One bar per component stands along the bottom.
            css::uno::Reference<css::chart2::XTitled> xComponentAxis(
                aSystems[0]->getAxisByDimension(0, 0), css::uno::UNO_QUERY_THROW);
            xComponentAxis->setTitleObject(
                MakeChartTitle(ScResId(STR_PRINCIPAL_COMPONENTS_AXIS_TITLE)));

            const css::uno::Reference<css::chart2::XAxis> xShareAxis
                = aSystems[0]->getAxisByDimension(1, 0);
            css::uno::Reference<css::chart2::XTitled> xShareAxisTitled(xShareAxis,
                                                                       css::uno::UNO_QUERY_THROW);
            xShareAxisTitled->setTitleObject(
                MakeChartTitle(ScResId(STR_VARIANCE_SHARE_AXIS_TITLE)));

            // Both series are a share of a whole, so the axis they share counts
            // in percent.
            css::uno::Reference<css::beans::XPropertySet> xShareAxisProperties(
                xShareAxis, css::uno::UNO_QUERY_THROW);
            const sal_uInt32 nFormat = mrDocument.GetFormatTable()->GetStandardFormat(
                SvNumFormatType::PERCENT, ScGlobal::eLnge);
            xShareAxisProperties->setPropertyValue(u"NumberFormat"_ustr,
                                                   cpo::uno::Any(sal_Int32(nFormat)));
            xShareAxisProperties->setPropertyValue(u"LinkNumberFormatToSource"_ustr,
                                                   cpo::uno::Any(false));
        }
    }
    catch (const css::uno::Exception&)
    {
        // The sheet of numbers stands on its own, so a chart that cannot be
        // built is left out.
        TOOLS_WARN_EXCEPTION("sc.ui", "adding the variance chart");
    }
}

void ScPrincipalComponentAnalysisDialog::AddCorrelationCircleChart(ScDocShell& rDocShell,
                                                                   const ScRange& rDataRange,
                                                                   SCCOL nChartColumn)
{
    try
    {
        // Under the chart of the variance, in the same column.
        const tools::Rectangle aCell = ScDrawLayer::GetCellRect(
            mrDocument, ScAddress(nChartColumn, nMeanRow, rDataRange.aStart.Tab()), false);
        css::uno::Reference<css::chart2::XChartDocument> xChartDocument = CreateSheetChart(
            rDocShell, rDataRange,
            css::awt::Rectangle(aCell.Left(), aCell.Top() + nChartHeight + nChartGap,
                                nCircleChartSide, nCircleChartSide));

        // The template reads the last two columns of the range as the pair of
        // components the features are placed against, and every column before
        // them as a feature, which is the order they stand in on the sheet.
        css::uno::Reference<css::lang::XMultiServiceFactory> xTemplateFactory(
            xChartDocument->getChartTypeManager(), css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::chart2::XChartTypeTemplate> xTemplate(
            xTemplateFactory->createInstance(
                u"com.sun.star.chart2.template.CorrelationCircle"_ustr),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::chart2::XDiagram> xDiagram = xChartDocument->getFirstDiagram();
        xTemplate->changeDiagram(xDiagram);

        css::uno::Reference<css::chart2::XTitled> xTitled(xChartDocument,
                                                          css::uno::UNO_QUERY_THROW);
        xTitled->setTitleObject(MakeChartTitle(ScResId(STR_CORRELATION_CIRCLE_CHART_TITLE)));

        // The two directions are the pair of components the features are placed
        // against, each named after which of the pair it is.
        css::uno::Reference<css::chart2::XCoordinateSystemContainer> xCoordinateSystems(
            xDiagram, css::uno::UNO_QUERY_THROW);
        const auto aSystems = xCoordinateSystems->getCoordinateSystems();
        if (aSystems.hasElements())
        {
            for (sal_Int32 nDimension = 0; nDimension < 2; ++nDimension)
            {
                css::uno::Reference<css::chart2::XTitled> xAxis(
                    aSystems[0]->getAxisByDimension(nDimension, 0), css::uno::UNO_QUERY_THROW);
                xAxis->setTitleObject(
                    MakeChartTitle(FillIn(STR_PRINCIPAL_COMPONENT_AXIS_TITLE_TEMPLATE, u"%NUMBER%",
                                          OUString::number(nDimension + 1))));
            }
        }
    }
    catch (const css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("sc.ui", "adding the correlation circle chart");
    }
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
