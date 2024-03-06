# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import unittest
from uitest.uihelper.testDialog import testChartDialog

dialogs = [
    {"command": "AllTitles", "closeButton": "cancel"},
    # {"command": "DataRanges", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/tdf99069.py
    # {"command": "DiagramArea", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartArea.py
    # {"command": "DiagramAxisA", "closeButton": "cancel"},
    {"command": "DiagramAxisAll", "closeButton": "cancel"},
    # {"command": "DiagramAxisB", "closeButton": "cancel"},
    #{"command": "DiagramAxisX", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartXAxis.py
    #{"command": "DiagramAxisY", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartYAxis.py
    # {"command": "DiagramAxisZ", "closeButton": "cancel"},
    {"command": "DiagramData", "closeButton": "close", "skipTestOK": True},
    {"command": "DiagramFloor", "closeButton": "cancel"},
    # {"command": "DiagramGridAll", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartGrid.py
    # {"command": "DiagramGridXHelp", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartGrid.py
    # {"command": "DiagramGridXMain", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartGrid.py
    # {"command": "DiagramGridYHelp", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartGrid.py
    # {"command": "DiagramGridYMain", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartGrid.py
    # {"command": "DiagramGridZHelp", "closeButton": "cancel"},
    # {"command": "DiagramGridZMain", "closeButton": "cancel"},
    {"command": "DiagramType", "closeButton": "cancel"},
    # {"command": "DiagramWall", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartWall.py
    # {"command": "FormatAxis", "closeButton": "cancel"},
    {"command": "FormatChartArea", "closeButton": "cancel"},
    # {"command": "FormatDataLabel", "closeButton": "cancel"},
    # {"command": "FormatDataLabels", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartDataLabels.py
    {"command": "FormatDataPoint", "closeButton": "cancel"},
    # {"command": "FormatDataSeries", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/formatDataSeries.py
    {"command": "FormatFloor", "closeButton": "cancel"},
    {"command": "FormatLegend", "closeButton": "cancel"},
    # {"command": "FormatMajorGrid", "closeButton": "cancel"},
    # {"command": "FormatMeanValue", "closeButton": "cancel"},
    # {"command": "FormatMinorGrid", "closeButton": "cancel"},
    {"command": "FormatSelection", "closeButton": "cancel"},
    # {"command": "FormatStockGain", "closeButton": "cancel"},
    # {"command": "FormatStockLoss", "closeButton": "cancel"},
    # {"command": "FormatTitle", "closeButton": "cancel"},
    # {"command": "FormatTrendline", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/tdf93506_trendline.py
    # {"command": "FormatTrendlineEquation", "closeButton": "cancel"},
    {"command": "FormatWall", "closeButton": "cancel"},
    # {"command": "FormatXErrorBars", "closeButton": "cancel"},
    # {"command": "FormatYErrorBars", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/tdf96432.py
    # {"command": "InsertMenuAxes", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartAxes.py
    # {"command": "InsertMenuDataLabels", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartDataLabels.py
    {"command": "InsertMenuDataTable", "closeButton": "cancel"},
    # {"command": "InsertMenuGrids", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartGrids.py
    # {"command": "InsertMenuLegend", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartLegend.py
    # {"command": "InsertMenuTitles", "closeButton": "cancel"},
        # Tested in sc/qa/uitest/chart/chartTitles.py
    # {"command": "InsertMenuTrendlines", "closeButton": "cancel"},
    {"command": "InsertMenuXErrorBars", "closeButton": "cancel"},
    {"command": "InsertMenuYErrorBars", "closeButton": "cancel"},
    # {"command": "InsertMinorGrid", "closeButton": "cancel"},
    # {"command": "InsertR2Value", "closeButton": "cancel"},
    {"command": "InsertRemoveAxes", "closeButton": "cancel"},
    {"command": "InsertTitles", "closeButton": "cancel"},
    # {"command": "InsertTrendline", "closeButton": "cancel"},
    {"command": "InsertXErrorBars", "closeButton": "cancel"},
    {"command": "InsertYErrorBars", "closeButton": "cancel"},
    {"command": "Legend", "closeButton": "cancel"},
    # {"command": "MainTitle", "closeButton": "cancel"},
    # {"command": "ObjectTitleDescription", "closeButton": "cancel"},
    # {"command": "RenameObject", "closeButton": "cancel"},
    # {"command": "SecondaryXTitle", "closeButton": "cancel"},
    # {"command": "SecondaryYTitle", "closeButton": "cancel"},
    # {"command": "SubTitle", "closeButton": "cancel"},
    {"command": "View3D", "closeButton": "cancel"},
    # {"command": "XTitle", "closeButton": "cancel"},
    # {"command": "YTitle", "closeButton": "cancel"},
    # {"command": "ZTitle", "closeButton": "cancel"},
]


def load_tests(loader, tests, pattern):
    return unittest.TestSuite(openDialogs(dialog)
                              for dialog in dialogs)

# Test to open all listed dialogs one by one, close it with the given close button
# and if there is an "OK" button open the dialog again and close it by using the OK button
# the test only checks if calc crashes by opening the dialog, see e.g. tdf#159879
class openDialogs(UITestCase):
    def check(self, dialog):
        testChartDialog(self, dialog)

dialogCount = 0
for dialog in dialogs:
    dialogCount = dialogCount + 1


    def ch(dialog):
        return lambda self: self.check(dialog)


    setattr(openDialogs, "test_%02d_%s" % (dialogCount, dialog["command"]), ch(dialog))
# vim: set shiftwidth=4 softtabstop=4 expandtab:
