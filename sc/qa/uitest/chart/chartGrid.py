# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

#Chart Grid dialog
#Bug 98390 - Crash when modifying minor grid

class chartGrid(UITestCase):
   def test_tdf98390_chart_grid_dialog(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf98390.ods")) as calc_doc:
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        change_measurement_unit(self, "Centimeter")

        #X Axis Major Grid
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridYMain"}))
        xDialog = self.xUITest.getTopFocusWindow()
        #tab "Line".
        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "0")

        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

        xWidth.executeAction("UP", tuple())
        transparency.executeAction("UP", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #reopen and verify X Axis Major Grid
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridYMain"}))
        xDialog = self.xUITest.getTopFocusWindow()

        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "0")
        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

        self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
        self.assertEqual(get_state_as_dict(transparency)["Text"], "5%")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #Y Axis Major Grid
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridXMain"}))
        xDialog = self.xUITest.getTopFocusWindow()

        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "0")

        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

        xWidth.executeAction("UP", tuple())
        transparency.executeAction("UP", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #reopen and verify Y Axis Major Grid
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridXMain"}))
        xDialog = self.xUITest.getTopFocusWindow()

        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "0")

        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

        self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
        self.assertEqual(get_state_as_dict(transparency)["Text"], "5%")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #Y Axis Minor Grid
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridXHelp"}))
        xDialog = self.xUITest.getTopFocusWindow()

        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "0")

        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

        xWidth.executeAction("UP", tuple())
        transparency.executeAction("UP", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #reopen and verify Y Axis Minor Grid
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridXHelp"}))
        xDialog = self.xUITest.getTopFocusWindow()

        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "0")
        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

        self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
        self.assertEqual(get_state_as_dict(transparency)["Text"], "5%")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #X Axis Minor Grid
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridYHelp"}))
        xDialog = self.xUITest.getTopFocusWindow()

        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "0")

        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

        xWidth.executeAction("UP", tuple())
        transparency.executeAction("UP", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #reopen and verify X Axis Minor Grid
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridYHelp"}))
        xDialog = self.xUITest.getTopFocusWindow()

        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "0")
        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

        self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.10 cm")
        self.assertEqual(get_state_as_dict(transparency)["Text"], "5%")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #All Grids
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridAll"}))
        xDialog = self.xUITest.getTopFocusWindow()

        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "0")

        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

        xWidth.executeAction("UP", tuple())
        transparency.executeAction("UP", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #reopen and verify X Axis Minor Grid  (changed in All Grids)
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramGridYHelp"}))
        xDialog = self.xUITest.getTopFocusWindow()

        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "0")
        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        transparency = xDialog.getChild("MTR_LINE_TRANSPARENT")

        self.assertEqual(get_state_as_dict(xWidth)["Text"], "0.20 cm")
        self.assertEqual(get_state_as_dict(transparency)["Text"], "10%")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
