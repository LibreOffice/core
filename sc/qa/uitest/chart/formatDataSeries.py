# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import change_measurement_unit, select_pos
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

from libreoffice.uno.propertyvalue import mkPropertyValues


# test Chart - Data series dialog  /core/chart2/uiconfig/ui/tp_SeriesToAxis.ui
class chartFormatDataSeries(UITestCase):
   def test_chart_format_data_series_dialog(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf93506.ods")):
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        change_measurement_unit(self, "Centimeter")

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataSeries"})) as xDialog:
            #Click on tab "Options".
            tabcontrol = xDialog.getChild("tabcontrol")
            select_pos(tabcontrol, "0")

            secondaryYAxis = xDialog.getChild("RBT_OPT_AXIS_2")
            leaveGap = xDialog.getChild("RB_DONT_PAINT")
            includeHiddenCells = xDialog.getChild("CB_INCLUDE_HIDDEN_CELLS")
            hideLegend = xDialog.getChild("CB_LEGEND_ENTRY_HIDDEN")

            secondaryYAxis.executeAction("CLICK", tuple())
            leaveGap.executeAction("CLICK", tuple())
            includeHiddenCells.executeAction("CLICK", tuple())
            hideLegend.executeAction("CLICK", tuple())

            #Click on tab "Line".
            select_pos(tabcontrol, "1")

            xWidth = xDialog.getChild("MF_SYMBOL_WIDTH")
            xHeight = xDialog.getChild("MF_SYMBOL_HEIGHT")
            xKeep = xDialog.getChild("CB_SYMBOL_RATIO")

            xKeep.executeAction("CLICK", tuple())
            xWidth.executeAction("UP", tuple())
            xWidth.executeAction("UP", tuple())
            xHeight.executeAction("UP", tuple())


        #reopen and verify
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataSeries"})) as xDialog:
            #Click on tab "Options".
            tabcontrol = xDialog.getChild("tabcontrol")
            select_pos(tabcontrol, "0")

            secondaryYAxis = xDialog.getChild("RBT_OPT_AXIS_2")
            leaveGap = xDialog.getChild("RB_DONT_PAINT")
            includeHiddenCells = xDialog.getChild("CB_INCLUDE_HIDDEN_CELLS")
            hideLegend = xDialog.getChild("CB_LEGEND_ENTRY_HIDDEN")

            self.assertEqual(get_state_as_dict(secondaryYAxis)["Checked"], "true")
            self.assertEqual(get_state_as_dict(leaveGap)["Checked"], "true")
            self.assertEqual(get_state_as_dict(includeHiddenCells)["Selected"], "true")
            self.assertEqual(get_state_as_dict(hideLegend)["Selected"], "true")

            #Click on tab "Line".
            select_pos(tabcontrol, "1")

            xWidth = xDialog.getChild("MF_SYMBOL_WIDTH")
            xHeight = xDialog.getChild("MF_SYMBOL_HEIGHT")
            xKeep = xDialog.getChild("CB_SYMBOL_RATIO")

            self.assertEqual(get_state_as_dict(xKeep)["Selected"], "true") #checked as default even after reopen
            self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.35 cm")
            self.assertEqual(get_state_as_dict(xHeight)["Text"], "0.30 cm")
            #click up, should up both values, because Keep ratio is checked
            xWidth.executeAction("UP", tuple())
            xHeight.executeAction("UP", tuple())
            self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.41 cm")
            self.assertEqual(get_state_as_dict(xHeight)["Text"], "0.35 cm")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
