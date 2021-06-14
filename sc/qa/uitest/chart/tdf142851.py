# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

# Bug 142851 - UI Chart Type dialog empty with unsupported combo chart

class tdf142851(UITestCase):
   def test_tdf142851_combo_chart_type(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf142851.xlsx"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Diagram 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xChart = xChartMain.getChild("CID/Page=")

    self.ui_test.execute_dialog_through_action(xChart, "COMMAND",
        mkPropertyValues({"COMMAND" : "DiagramType"}))
    xDialog = self.xUITest.getTopFocusWindow()
    chartTypeVisible = get_state_as_dict(xDialog.getChild("charttype"))['Visible']
    subTypeVisible = get_state_as_dict(xDialog.getChild("subtype"))['Visible']
    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.assertEqual('true', chartTypeVisible)
    self.assertEqual('true', subTypeVisible)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
