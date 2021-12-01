# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 96432 - FORMATTING: Crash when transparency is applied to an error bar

class tdf96432(UITestCase):
   def test_tdf96432_chart_crash_transparency_error_bar_dialog(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf96432.ods")) as calc_doc:
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        xDataSeries = calc_doc.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
        self.assertEqual(0, xDataSeries[0].ErrorBarY.LineTransparence)

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")

        #Right-click on the chart; from the pop-up menu select "Format Y bars
        # The program presents dialog "Format Y bars", tab "Line".
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatYErrorBars"})) as xDialog:
            #Click on tab "Line".
            tabcontrol = xDialog.getChild("tabcontrol")
            select_pos(tabcontrol, "1")
            #Type a non-zero integer into the "Transparency:" Edit Field, or use the up arrow to select one
            xTransparency = xDialog.getChild("MTR_LINE_TRANSPARENT")
            xTransparency.executeAction("UP", tuple())

        #verify - we didn't crash
        gridwin.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

        xDataSeries = calc_doc.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
        self.assertEqual(5, xDataSeries[0].ErrorBarY.LineTransparence)

        #reopen and try again
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")

        #Right-click on the chart; from the pop-up menu select "Format Y bars
        # The program presents dialog "Format Y bars", tab "Line".
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatYErrorBars"})) as xDialog:
            #Click on tab "Line".
            tabcontrol = xDialog.getChild("tabcontrol")
            select_pos(tabcontrol, "1")
            #Type a non-zero integer into the "Transparency:" Edit Field, or use the up arrow to select one
            xTransparency = xDialog.getChild("MTR_LINE_TRANSPARENT")
            self.assertEqual(get_state_as_dict(xTransparency)["Text"][0], "5")
            xTransparency.executeAction("UP", tuple())

        #verify - we didn't crash
        gridwin.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))

        xDataSeries = calc_doc.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0].ChartTypes[0].DataSeries
        self.assertEqual(10, xDataSeries[0].ErrorBarY.LineTransparence)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
