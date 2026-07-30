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

# The mean row, the standard deviation row and the label row come before the
# standardized values.
nFirstValueRow = 3


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

    def runAnalysis(self, gridwin, sRange, bWithLabels, close_button="ok"):
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": sRange}))
        with self.ui_test.execute_modeless_dialog_through_command(
                ".uno:PrincipalComponentAnalysisDialog", close_button=close_button) as xDialog:
            xInputRangeEdit = xDialog.getChild("input-range-edit")
            xInputRangeEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
            xInputRangeEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
            xInputRangeEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "$Sheet1." + sRange}))

            if bWithLabels:
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

            # The label row is copied over as it stands.
            for nColumn, (sLabel, _) in enumerate(aColumns):
                self.assertEqual(
                    get_cell_by_position(document, 1, nColumn, 2).getString(), sLabel)

            # The mean and the standard deviation of every column sit above the
            # standardized values.
            self.assertEqual(get_cell_by_position(document, 1, 0, 0).getValue(), 14)
            self.assertEqual(get_cell_by_position(document, 1, 1, 0).getValue(), 30)
            self.assertEqual(get_cell_by_position(document, 1, 2, 0).getValue(), 30)
            self.assertEqual(
                round(get_cell_by_position(document, 1, 0, 1).getValue(), 10), 3.1622776602)

            # The three parts of the decomposition all read the whole block of
            # standardized values and start on its first row. The left vectors
            # take up one column per component, so the singular values land
            # three columns further right and the right vectors one beyond
            # those.
            for nColumn, nPart in ((3, "1"), (6, "2"), (7, "3")):
                self.assertEqual(
                    get_cell_by_position(document, 1, nColumn, nFirstValueRow).getFormula(),
                    "{=MSVD($A$4:$C$8;" + nPart + ")}")

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

            # A second run cannot have the sheet it needs, so it reports that
            # and adds nothing.
            self.runAnalysis(gridwin, "$A$1:$C$6", True, close_button="cancel")
            self.assertIn("Sheet1_PCA", self.sErrorMessage)
            self.assertEqual(document.Sheets.getCount(), 2)

            # Undo takes the whole sheet away again.
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(document.Sheets.getCount(), 1)

    def test_the_first_row_holds_data_when_the_labels_box_is_left_clear(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            self.enterData(gridwin, False)

            self.runAnalysis(gridwin, "$A$1:$C$5", False)

            # Without a label row the standardized values start one row higher.
            self.assertEqual(get_cell_by_position(document, 1, 0, 0).getValue(), 14)
            self.assertEqual(
                round(get_cell_by_position(document, 1, 0, 2).getValue(), 10), -1.2649110641)

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

    def test_a_single_column_cannot_be_analysed(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            self.enterData(gridwin, True)

            self.runAnalysis(gridwin, "$A$1:$A$6", True, close_button="cancel")

            self.assertIn("two columns", self.sErrorMessage)
            self.assertEqual(document.Sheets.getCount(), 1)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
