# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep
import org.libreoffice.unotest
import pathlib
def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 123231 - Chart set trendline format regression type Power twice

class tdf123231(UITestCase):
   def test_tdf123231_chart_trendline_dialog_power(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf93506.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    #Change regression Type to Power
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")
    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    exponential = xDialog.getChild("exponential")  #type regression exponential
    power = xDialog.getChild("exponential")  #type regression power

    power.executeAction("CLICK", tuple())   #set power

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #reopen dialog and close dialog
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")
    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    exponential = xDialog.getChild("exponential")  #type regression exponential
    power = xDialog.getChild("exponential")  #type regression power
    self.assertEqual(get_state_as_dict(power)["Checked"], "true")
    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #reopen and verify Power
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Type".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "0")

    logarithmic = xDialog.getChild("logarithmic")  #type regression logarithmic
    exponential = xDialog.getChild("exponential")  #type regression exponential
    power = xDialog.getChild("exponential")  #type regression power

    self.assertEqual(get_state_as_dict(power)["Checked"], "true")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
