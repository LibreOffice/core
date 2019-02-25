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

#Chart Display Legend dialog

class chartLegend(UITestCase):
   def test_chart_display_legend_dialog(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf98390.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuLegend"}))
    xDialog = self.xUITest.getTopFocusWindow()

    left = xDialog.getChild("left")
    right = xDialog.getChild("right")
    top = xDialog.getChild("top")
    bottom = xDialog.getChild("bottom")

    left.executeAction("CLICK", tuple())

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #reopen and verify InsertMenuLegend dialog
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuLegend"}))
    xDialog = self.xUITest.getTopFocusWindow()

    left = xDialog.getChild("left")
    right = xDialog.getChild("right")
    top = xDialog.getChild("top")
    bottom = xDialog.getChild("bottom")
    show = xDialog.getChild("show")

    self.assertEqual(get_state_as_dict(left)["Checked"], "true")
    self.assertEqual(get_state_as_dict(right)["Checked"], "false")
    self.assertEqual(get_state_as_dict(top)["Checked"], "false")
    self.assertEqual(get_state_as_dict(bottom)["Checked"], "false")

    show.executeAction("CLICK", tuple())

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #reopen and verify InsertMenuLegend dialog
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuLegend"}))
    xDialog = self.xUITest.getTopFocusWindow()

    left = xDialog.getChild("left")
    right = xDialog.getChild("right")
    top = xDialog.getChild("top")
    bottom = xDialog.getChild("bottom")
    show = xDialog.getChild("show")

    self.assertEqual(get_state_as_dict(left)["Checked"], "true")
    self.assertEqual(get_state_as_dict(right)["Checked"], "false")
    self.assertEqual(get_state_as_dict(top)["Checked"], "false")
    self.assertEqual(get_state_as_dict(bottom)["Checked"], "false")

    self.assertEqual(get_state_as_dict(show)["Selected"], "false")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)
    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
