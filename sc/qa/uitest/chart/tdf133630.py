# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, get_url_for_data_file
from uitest.uihelper.common import change_measurement_unit
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf133630(UITestCase):

   def test_tdf133630(self):

        calc_doc = self.ui_test.load_file(get_url_for_data_file("chartArea.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        change_measurement_unit(self, "Centimeter")

        xCS = document.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().CoordinateSystems[0]
        self.assertEqual(0, xCS.getAxisByDimension(0, 0).LineWidth)
        self.assertEqual(80, xCS.ChartTypes[0].DataSeries[0].LineWidth)

        # First change the line width of one element
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:Axis=0,0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramAxisX"}))

        xDialog = self.xUITest.getTopFocusWindow()
        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "2")

        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        xWidth.executeAction("UP", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.assertEqual(100, xCS.getAxisByDimension(0, 0).LineWidth)
        self.assertEqual(80, xCS.ChartTypes[0].DataSeries[0].LineWidth)

        # Now change the line width of another element
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatDataSeries"}))

        xDialog = self.xUITest.getTopFocusWindow()
        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, "1")

        xWidth = xDialog.getChild("MTR_FLD_LINE_WIDTH")
        xWidth.executeAction("UP", tuple())

        # Without the fix in place, this test would have crashed here
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.assertEqual(100, xCS.getAxisByDimension(0, 0).LineWidth)
        self.assertEqual(100, xCS.ChartTypes[0].DataSeries[0].LineWidth)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
