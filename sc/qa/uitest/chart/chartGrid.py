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


# Chart Grid dialog
# Bug 98390 - Crash when modifying minor grid
class chartGrid(UITestCase):
   def test_tdf98390_chart_grid_dialog(self):
    with self.ui_test.load_file(get_url_for_data_file("chart.ods")):

        with change_measurement_unit(self, "Centimeter"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            #X Axis Major Grid
            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridYMain"})) as xDialog:
                #tab "Line".
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")

                xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
                transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

                xWidth.executeAction("UP", tuple())
                transparency.executeAction("UP", tuple())


            #reopen and verify X Axis Major Grid
            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridYMain"})) as xDialog:

                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")
                xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
                transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

                self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
                self.assertEqual(get_state_as_dict(transparency)["Text"], "5%")

            #Y Axis Major Grid
            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridXMain"})) as xDialog:

                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")

                xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
                transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

                xWidth.executeAction("UP", tuple())
                transparency.executeAction("UP", tuple())


            #reopen and verify Y Axis Major Grid
            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridXMain"})) as xDialog:

                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")

                xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
                transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

                self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
                self.assertEqual(get_state_as_dict(transparency)["Text"], "5%")


            #Y Axis Minor Grid
            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridXHelp"})) as xDialog:

                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")

                xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
                transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

                xWidth.executeAction("UP", tuple())
                transparency.executeAction("UP", tuple())


            #reopen and verify Y Axis Minor Grid
            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridXHelp"})) as xDialog:

                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")
                xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
                transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

                self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
                self.assertEqual(get_state_as_dict(transparency)["Text"], "5%")


            #X Axis Minor Grid
            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridYHelp"})) as xDialog:

                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")

                xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
                transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

                xWidth.executeAction("UP", tuple())
                transparency.executeAction("UP", tuple())


            #reopen and verify X Axis Minor Grid
            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridYHelp"})) as xDialog:

                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")
                xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
                transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

                self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
                self.assertEqual(get_state_as_dict(transparency)["Text"], "5%")


            #All Grids
            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridAll"})) as xDialog:

                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")

                xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
                transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

                xWidth.executeAction("UP", tuple())
                transparency.executeAction("UP", tuple())


            #reopen and verify X Axis Minor Grid  (changed in All Grids)
            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridYHelp"})) as xDialog:

                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")
                xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
                transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

                self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.20 cm")
                self.assertEqual(get_state_as_dict(transparency)["Text"], "10%")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
