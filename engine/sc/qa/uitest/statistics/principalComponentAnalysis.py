# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

# Three columns of five observations each. Height and Weight both rise in equal
# steps, so they standardize to the same column and the block has rank two.
aColumns = [
    ("Height", [10, 12, 14, 16, 18]),
    ("Weight", [20, 25, 30, 35, 40]),
    ("Age", [30, 20, 40, 10, 50]),
]

# The mean row, the standard deviation row, two blank rows and the header row
# come before the standardized values.
nHeaderRow = 4
nFirstValueRow = 5


class principalComponentAnalysis(UITestCase):
    def enterData(self, gridwin, bWithLabels):
        for nColumn, (sLabel, aValues) in enumerate(aColumns):
            sColumn = chr(ord("A") + nColumn)
            nRow = 1
            if bWithLabels:
                enter_text_to_cell(gridwin, sColumn + "1", sLabel)
                nRow = 2
            for fValue in aValues:
                enter_text_to_cell(gridwin, sColumn + str(nRow), str(fValue))
                nRow += 1

    def runAnalysis(self, gridwin, sRange, bWithLabels, close_button="ok",
                    aClearBoxes=()):
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": sRange}))
        with self.ui_test.execute_modeless_dialog_through_command(
                ".uno:PrincipalComponentAnalysisDialog", close_button=close_button) as xDialog:
            xInputRangeEdit = xDialog.getChild("input-range-edit")
            xInputRangeEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
            xInputRangeEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
            xInputRangeEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "$Sheet1." + sRange}))

            # Every box starts ticked, so one is only clicked to clear it.
            for sBox in aClearBoxes:
                xDialog.getChild(sBox).executeAction("CLICK", tuple())
            if not bWithLabels:
                xDialog.getChild("withlabels-check").executeAction("CLICK", tuple())

            if close_button != "ok":
                xDialog.getChild("ok").executeAction("CLICK", tuple())
                self.sErrorMessage = get_state_as_dict(xDialog.getChild("error-message"))["Text"]

    def test_the_new_sheet_holds_the_standardized_data_and_its_decomposition(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            self.enterData(gridwin, True)

            self.runAnalysis(gridwin, "$A$1:$C$6", True)

            self.assertEqual(document.Sheets.getByIndex(1).Name, "Sheet1_PCA")

            # The feature columns take the labels of the source columns, and
            # every column after them says what it holds.
            aExpectedHeader = [sLabel for sLabel, _ in aColumns] \
                + ["Score_1", "Score_2", "Score_3"] \
                + ["Singular Value"] \
                + ["Loading_1", "Loading_2", "Loading_3"] \
                + ["Variance Share", "Cumulative Share"]
            aHeader = [
                get_cell_by_position(document, 1, nColumn, nHeaderRow).getString()
                for nColumn in range(len(aExpectedHeader))]
            self.assertEqual(aHeader, aExpectedHeader)

            # The mean and the standard deviation of every column sit above the
            # standardized values, each row named just right of its values.
            self.assertEqual(get_cell_by_position(document, 1, 0, 0).getValue(), 14)
            self.assertEqual(get_cell_by_position(document, 1, 1, 0).getValue(), 30)
            self.assertEqual(get_cell_by_position(document, 1, 2, 0).getValue(), 30)
            self.assertEqual(
                round(get_cell_by_position(document, 1, 0, 1).getValue(), 10), 3.1622776602)
            self.assertEqual(
                get_cell_by_position(document, 1, len(aColumns), 0).getString(), "Mean")
            self.assertEqual(
                get_cell_by_position(document, 1, len(aColumns), 1).getString(),
                "Standard Deviation")

            # Two rows of nothing hold those two apart from the table below.
            for nRow in (2, 3):
                for nColumn in range(len(aColumns) + 1):
                    self.assertEqual(
                        get_cell_by_position(document, 1, nColumn, nRow).getString(), "")

            # Every column is wide enough for its longest entry, so none of them
            # is left at the width a new sheet starts with.
            oSheet = document.Sheets.getByIndex(1)
            nDefaultWidth = document.Sheets.getByIndex(0).Columns.getByIndex(0).Width
            for nColumn in range(len(aColumns) + 9):
                self.assertNotEqual(oSheet.Columns.getByIndex(nColumn).Width, nDefaultWidth)

            # The three parts of the decomposition all read the whole block of
            # standardized values and start on its first row. The left vectors
            # take up one column per component, so the singular values land
            # three columns further right and the right vectors one beyond
            # those.
            sValueRange = "$A$%d:$C$%d" % (nFirstValueRow + 1, nFirstValueRow + 5)
            for nColumn, nPart in ((3, "1"), (6, "2"), (7, "3")):
                self.assertEqual(
                    get_cell_by_position(document, 1, nColumn, nFirstValueRow).getFormula(),
                    "{=MSVD(" + sValueRange + ";" + nPart + ")}")

            # Standardizing leaves every column with a mean of zero and a
            # standard deviation of one.
            for nColumn in range(len(aColumns)):
                aStandardized = [
                    get_cell_by_position(document, 1, nColumn, nFirstValueRow + nRow).getValue()
                    for nRow in range(5)]
                self.assertEqual(round(sum(aStandardized), 10), 0)
                self.assertEqual(round(sum(x * x for x in aStandardized), 10), 4)

            # The squared singular values add up to the sum of the squares of
            # the standardized values, which is five observations less one for
            # each of the three columns.
            aSingularValues = [
                get_cell_by_position(document, 1, 6, nFirstValueRow + nRow).getValue()
                for nRow in range(3)]
            self.assertEqual(round(sum(x * x for x in aSingularValues), 8), 12)
            self.assertEqual(aSingularValues, sorted(aSingularValues, reverse=True))

            # Two of the three columns are a straight line in the row number,
            # so the third singular value is zero.
            self.assertEqual(round(aSingularValues[2], 10), 0)

            # Each component carries the share of the variance its squared
            # singular value is of the total, and the shares add up to one.
            fTotal = sum(x * x for x in aSingularValues)
            aShares = [
                get_cell_by_position(document, 1, 10, nFirstValueRow + nRow).getValue()
                for nRow in range(3)]
            for nRow in range(3):
                self.assertEqual(
                    round(aShares[nRow], 10),
                    round(aSingularValues[nRow] ** 2 / fTotal, 10))
            self.assertEqual(round(sum(aShares), 10), 1)

            # The column beside it is the running total of the shares, so it
            # reaches one on the last component.
            aCumulative = [
                get_cell_by_position(document, 1, 11, nFirstValueRow + nRow).getValue()
                for nRow in range(3)]
            for nRow in range(3):
                self.assertEqual(
                    round(aCumulative[nRow], 10), round(sum(aShares[:nRow + 1]), 10))
            self.assertEqual(round(aCumulative[2], 10), 1)

            # Both columns are shown as percentages.
            for nColumn in (10, 11):
                nFormatKey = get_cell_by_position(
                    document, 1, nColumn, nFirstValueRow).NumberFormat
                sFormat = document.getNumberFormats().getByKey(nFormatKey).FormatString
                self.assertIn("%", sFormat)

            # A chart on the sheet draws the two share columns, the per
            # component share as bars and the running total as a line over
            # them.
            oCharts = document.Sheets.getByIndex(1).Charts
            self.assertEqual(oCharts.getCount(), 2)
            oChart = oCharts.getByIndex(0)
            aRanges = [
                (oRange.StartColumn, oRange.StartRow, oRange.EndColumn, oRange.EndRow)
                for oRange in oChart.getRanges()]
            self.assertEqual(aRanges, [(10, nHeaderRow, 11, nFirstValueRow + 2)])

            oChartDocument = oChart.getEmbeddedObject()

            def sTitleOf(oTitled):
                return "".join(oString.getString()
                               for oString in oTitled.getTitleObject().getText())

            self.assertEqual(sTitleOf(oChartDocument), "Variance share by component")

            oDiagram = oChartDocument.getFirstDiagram()
            self.assertEqual(oDiagram.Wall.FillColor, 0xFFFFFF)
            aChartTypes = [
                oChartType.getChartType()
                for oSystem in oDiagram.getCoordinateSystems()
                for oChartType in oSystem.getChartTypes()]
            self.assertEqual(aChartTypes, ["com.sun.star.chart2.ColumnChartType",
                                           "com.sun.star.chart2.LineChartType"])

            # The bars read the share column and the line the running total
            # beside it.
            aSeriesRanges = []
            for oSystem in oDiagram.getCoordinateSystems():
                for oChartType in oSystem.getChartTypes():
                    for oSeries in oChartType.getDataSeries():
                        for oLabeled in oSeries.getDataSequences():
                            oValues = oLabeled.getValues()
                            if oValues.Role == "values-y":
                                aSeriesRanges.append(
                                    oValues.getSourceRangeRepresentation())
            self.assertEqual(len(aSeriesRanges), 2)
            self.assertIn("$K$", aSeriesRanges[0])
            self.assertIn("$L$", aSeriesRanges[1])

            # Both axes say what they carry.
            oSystem = oDiagram.getCoordinateSystems()[0]
            self.assertEqual(
                sTitleOf(oSystem.getAxisByDimension(0, 0)), "Principal Components")
            self.assertEqual(
                sTitleOf(oSystem.getAxisByDimension(1, 0)), "Variance share")

            # Both series are a share of a whole, so the axis they share counts
            # in percent of its own accord.
            oAxis = oSystem.getAxisByDimension(1, 0)
            self.assertFalse(oAxis.LinkNumberFormatToSource)
            sAxisFormat = document.getNumberFormats().getByKey(
                oAxis.NumberFormat).FormatString
            self.assertIn("%", sAxisFormat)

            # The second chart is a correlation circle over one range that runs
            # from the standardized features straight into the first two score
            # columns.
            oCircleChart = oCharts.getByIndex(1)
            aCircleRanges = [
                (oRange.StartColumn, oRange.StartRow, oRange.EndColumn, oRange.EndRow)
                for oRange in oCircleChart.getRanges()]
            self.assertEqual(
                aCircleRanges, [(0, nHeaderRow, len(aColumns) + 1, nFirstValueRow + 4)])

            oCircleDocument = oCircleChart.getEmbeddedObject()
            self.assertEqual(sTitleOf(oCircleDocument), "Correlation circle")
            oCircleDiagram = oCircleDocument.getFirstDiagram()
            self.assertEqual(oCircleDiagram.Wall.FillColor, 0xFFFFFF)

            # Each direction is named after which of the pair of components it
            # is.
            oCircleSystem = oCircleDiagram.getCoordinateSystems()[0]
            self.assertEqual(
                sTitleOf(oCircleSystem.getAxisByDimension(0, 0)), "Principal Component 1")
            self.assertEqual(
                sTitleOf(oCircleSystem.getAxisByDimension(1, 0)), "Principal Component 2")

            self.assertEqual(
                [oChartType.getChartType()
                 for oSystem in oCircleDiagram.getCoordinateSystems()
                 for oChartType in oSystem.getChartTypes()],
                ["com.sun.star.chart2.CorrelationCircleChartType"])

            # One series per feature, each holding its own feature column beside
            # the two component columns they all share.
            aFeatureRoles = []
            for oSystem in oCircleDiagram.getCoordinateSystems():
                for oChartType in oSystem.getChartTypes():
                    for oSeries in oChartType.getDataSeries():
                        aFeatureRoles.append(sorted(
                            oLabeled.getValues().Role
                            for oLabeled in oSeries.getDataSequences()))
            self.assertEqual(len(aFeatureRoles), len(aColumns))
            for aRoles in aFeatureRoles:
                self.assertEqual(aRoles, ["values-feature", "values-x", "values-y"])

            # A second run cannot have the sheet it needs, so it reports that
            # and adds nothing.
            self.runAnalysis(gridwin, "$A$1:$C$6", True, close_button="cancel")
            self.assertIn("Sheet1_PCA", self.sErrorMessage)
            self.assertEqual(document.Sheets.getCount(), 2)

            # Undo takes the whole sheet away again.
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(document.Sheets.getCount(), 1)

    def test_features_are_named_after_their_place_when_the_source_has_no_labels(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            self.enterData(gridwin, False)

            self.runAnalysis(gridwin, "$A$1:$C$5", False)

            # With nothing to take a name from, each feature is named after its
            # place in the range, and the header row is there all the same.
            for nColumn in range(len(aColumns)):
                self.assertEqual(
                    get_cell_by_position(document, 1, nColumn, nHeaderRow).getString(),
                    "Feature_" + str(nColumn + 1))

            # Every row of the source counts as data, so the mean covers all
            # five and the standardized values start below the header row.
            self.assertEqual(get_cell_by_position(document, 1, 0, 0).getValue(), 14)
            self.assertEqual(
                round(get_cell_by_position(document, 1, 0, nFirstValueRow).getValue(), 10),
                -1.2649110641)

    def test_a_labelled_source_column_left_blank_is_named_after_its_place(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            self.enterData(gridwin, True)
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
            self.xUITest.executeCommand(".uno:ClearContents")

            self.runAnalysis(gridwin, "$A$1:$C$6", True)

            # The labelled columns keep their names and the blank one falls back
            # to its place in the range.
            self.assertEqual(
                get_cell_by_position(document, 1, 0, nHeaderRow).getString(), "Height")
            self.assertEqual(
                get_cell_by_position(document, 1, 1, nHeaderRow).getString(), "Feature_2")
            self.assertEqual(
                get_cell_by_position(document, 1, 2, nHeaderRow).getString(), "Age")

    def test_a_cell_that_is_not_a_number_stops_the_analysis(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            self.enterData(gridwin, True)
            enter_text_to_cell(gridwin, "B4", "no reading")

            self.runAnalysis(gridwin, "$A$1:$C$6", True, close_button="cancel")

            self.assertIn("$B$4", self.sErrorMessage)
            self.assertEqual(document.Sheets.getCount(), 1)

    def test_an_empty_cell_stops_the_analysis(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            self.enterData(gridwin, True)
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C5"}))
            self.xUITest.executeCommand(".uno:ClearContents")

            self.runAnalysis(gridwin, "$A$1:$C$6", True, close_button="cancel")

            self.assertIn("$C$5", self.sErrorMessage)
            self.assertEqual(document.Sheets.getCount(), 1)

    def test_every_box_starts_ticked(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            self.enterData(gridwin, True)

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "$A$1:$C$6"}))
            with self.ui_test.execute_modeless_dialog_through_command(
                    ".uno:PrincipalComponentAnalysisDialog", close_button="cancel") as xDialog:
                for sBox in ("withlabels-check", "variance-chart-check",
                             "correlation-chart-check"):
                    self.assertEqual(
                        get_state_as_dict(xDialog.getChild(sBox))["Selected"], "true")

    def test_clearing_a_chart_box_leaves_that_chart_out(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            self.enterData(gridwin, True)

            # Only the correlation circle is asked for, so it is the one chart
            # on the sheet.
            self.runAnalysis(gridwin, "$A$1:$C$6", True,
                             aClearBoxes=("variance-chart-check",))
            oCharts = document.Sheets.getByIndex(1).Charts
            self.assertEqual(oCharts.getCount(), 1)
            self.assertEqual(
                oCharts.getByIndex(0).getEmbeddedObject().getFirstDiagram()
                .getCoordinateSystems()[0].getChartTypes()[0].getChartType(),
                "com.sun.star.chart2.CorrelationCircleChartType")

            # Clearing both leaves the numbers on their own.
            self.xUITest.executeCommand(".uno:Undo")
            self.runAnalysis(gridwin, "$A$1:$C$6", True,
                             aClearBoxes=("variance-chart-check", "correlation-chart-check"))
            self.assertEqual(document.Sheets.getByIndex(1).Charts.getCount(), 0)

    def test_a_single_column_cannot_be_analysed(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            self.enterData(gridwin, True)

            self.runAnalysis(gridwin, "$A$1:$A$6", True, close_button="cancel")

            self.assertIn("two columns", self.sErrorMessage)
            self.assertEqual(document.Sheets.getCount(), 1)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
