# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# Copyright the LibreOffice contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.chart import select_chart_type

from libreoffice.uno.propertyvalue import mkPropertyValues

FEATURES = ["length", "width", "span", "mass"]
DIMENSIONS = ["dim 1", "dim 2"]
INSTANCE_COUNT = 8
RANGE = "$Sheet1.$A$1:$F$9"


class correlationCircle(UITestCase):
    def fill_sheet(self, document):
        sheet = document.Sheets[0]
        for column, name in enumerate(FEATURES + DIMENSIONS):
            sheet.getCellByPosition(column, 0).setString(name)

        for row in range(INSTANCE_COUNT):
            for column in range(len(FEATURES)):
                sheet.getCellByPosition(column, row + 1).setValue(row + column * 3 + 1)
            for column in range(len(DIMENSIONS)):
                sheet.getCellByPosition(len(FEATURES) + column, row + 1).setValue(
                    (row + 1) * (column + 1) * 0.25)

    def replace_text(self, xEntry, rText):
        xEntry.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
        xEntry.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
        xEntry.executeAction("TYPE", mkPropertyValues({"TEXT": rText}))

    def insert_correlation_circle(self, document):
        with self.ui_test.execute_dialog_through_command(
                ".uno:InsertObjectChart", close_button="finish") as xChartDlg:
            select_chart_type(self, xChartDlg, "Correlation Circle")

            # The second wizard step holds the range.
            xChartDlg.getChild("next").executeAction("CLICK", tuple())
            self.replace_text(xChartDlg.getChild("ED_RANGE"), RANGE)

            # Every column of the range before the last two is a feature, so
            # the first one is not a column of instance names.
            xFirstColumnAsLabel = xChartDlg.getChild("CB_FIRST_COLUMN_ASLABELS")
            if get_state_as_dict(xFirstColumnAsLabel)["Selected"] == "true":
                xFirstColumnAsLabel.executeAction("CLICK", tuple())

    def roles_of_series(self, document):
        xChartType = document.Sheets[0].Charts[0].getEmbeddedObject() \
            .getFirstDiagram().CoordinateSystems[0].ChartTypes[0]
        return [[(sequence.getValues().Role,
                  sequence.getValues().getSourceRangeRepresentation())
                 for sequence in series.getDataSequences()]
                for series in xChartType.DataSeries]

    # The last two columns of the range hold the dimensions and every column
    # before them becomes a series holding that column beside those two.
    def test_correlation_circle_one_range(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            self.fill_sheet(document)
            self.insert_correlation_circle(document)

            aRolesPerSeries = self.roles_of_series(document)

            self.assertEqual(len(FEATURES), len(aRolesPerSeries))
            for index, aRoles in enumerate(aRolesPerSeries):
                aColumn = chr(ord("A") + index)
                self.assertEqual(
                    [("values-x", "$Sheet1.$E$2:$E$9"),
                     ("values-y", "$Sheet1.$F$2:$F$9"),
                     ("values-feature", "$Sheet1.$%s$2:$%s$9" % (aColumn, aColumn))],
                    aRoles)

    # Reopening the ranges of such a chart shows the range it was given.
    def test_the_range_comes_back(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            self.fill_sheet(document)
            self.insert_correlation_circle(document)

            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")
            xGridWindow.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            xGridWindow.executeAction("ACTIVATE", tuple())
            xChartWindow = self.xUITest.getTopFocusWindow().getChild("chart_window")

            with self.ui_test.execute_dialog_through_action(
                    xChartWindow.getChild("CID/Page="), "COMMAND",
                    mkPropertyValues({"COMMAND": "DataRanges"}),
                    close_button="cancel") as xRangesDlg:
                self.assertEqual(
                    RANGE, get_state_as_dict(xRangesDlg.getChild("ED_RANGE"))["Text"])

            # Leave the chart, so that the document closes the way it would
            # after any other edit.
            xChartWindow.executeAction("TYPE", mkPropertyValues({"KEYCODE": "ESC"}))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
