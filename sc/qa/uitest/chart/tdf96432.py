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

#Bug 96432 - FORMATTING: Crash when transparency is applied to an error bar

class tdf96432(UITestCase):
   def test_tdf96432_chart_crash_transparency_error_bar_dialog(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf96432.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")

    #Right-click on the chart; from the pop-up menu select "Format Y bars
    # The program presents dialog "Format Y bars", tab "Line".
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatYErrorBars"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Line".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "1")
    #Type a non-zero integer into the "Transparency:" Edit Field, or use the up arrow to select one
    xTransparency = xDialog.getChild("MTR_LINE_TRANSPARENT")
    xTransparency.executeAction("UP", tuple())
    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #verify - we didn't crash
    gridwin.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))
    self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 7)

    #reopen and try again
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")

    #Right-click on the chart; from the pop-up menu select "Format Y bars
    # The program presents dialog "Format Y bars", tab "Line".
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatYErrorBars"}))
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "Line".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "1")
    #Type a non-zero integer into the "Transparency:" Edit Field, or use the up arrow to select one
    xTransparency = xDialog.getChild("MTR_LINE_TRANSPARENT")
    self.assertEqual(get_state_as_dict(xTransparency)["Text"][0], "5")
    xTransparency.executeAction("UP", tuple())
    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #verify - we didn't crash
    gridwin.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))
    self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 7)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
