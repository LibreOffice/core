# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text, get_url_for_data_file

#Chart Enable Axes dialog

class chartAxes(UITestCase):
   def test_chart_enable_grids_dialog(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf98390.ods")) as calc_doc:
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuAxes"})) as xDialog:

            primaryX = xDialog.getChild("primaryX")
            primaryY = xDialog.getChild("primaryY")
            secondaryX = xDialog.getChild("secondaryX")
            secondaryY = xDialog.getChild("secondaryY")

            primaryX.executeAction("CLICK", tuple())
            primaryY.executeAction("CLICK", tuple())
            secondaryX.executeAction("CLICK", tuple())
            secondaryY.executeAction("CLICK", tuple())


        #reopen and verify Grids dialog
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuAxes"})) as xDialog:

            primaryX = xDialog.getChild("primaryX")
            primaryY = xDialog.getChild("primaryY")
            secondaryX = xDialog.getChild("secondaryX")
            secondaryY = xDialog.getChild("secondaryY")

            self.assertEqual(get_state_as_dict(primaryX)["Selected"], "false")
            self.assertEqual(get_state_as_dict(primaryY)["Selected"], "false")
            self.assertEqual(get_state_as_dict(secondaryX)["Selected"], "true")
            self.assertEqual(get_state_as_dict(secondaryY)["Selected"], "true")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
