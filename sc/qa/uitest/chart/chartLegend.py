# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

from libreoffice.uno.propertyvalue import mkPropertyValues


# Chart Display Legend dialog
class chartLegend(UITestCase):
   def test_chart_display_legend_dialog(self):
    with self.ui_test.load_file(get_url_for_data_file("chart.ods")):
      xCalcDoc = self.xUITest.getTopFocusWindow()
      gridwin = xCalcDoc.getChild("grid_window")

      gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
      gridwin.executeAction("ACTIVATE", tuple())
      xChartMainTop = self.xUITest.getTopFocusWindow()
      xChartMain = xChartMainTop.getChild("chart_window")
      xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
      with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuLegend"})) as xDialog:
        left = xDialog.getChild("left")

        left.executeAction("CLICK", tuple())

      #reopen and verify InsertMenuLegend dialog
      gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
      gridwin.executeAction("ACTIVATE", tuple())
      xChartMainTop = self.xUITest.getTopFocusWindow()
      xChartMain = xChartMainTop.getChild("chart_window")
      xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
      with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuLegend"})) as xDialog:
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

      #reopen and verify InsertMenuLegend dialog
      gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
      gridwin.executeAction("ACTIVATE", tuple())
      xChartMainTop = self.xUITest.getTopFocusWindow()
      xChartMain = xChartMainTop.getChild("chart_window")
      xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
      with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "InsertMenuLegend"})) as xDialog:
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

   def test_legends_move_with_arrows_keys(self):

    with self.ui_test.load_file(get_url_for_data_file("dataLabels.ods")):

      with change_measurement_unit(self, "Centimeter"):
          xCalcDoc = self.xUITest.getTopFocusWindow()
          gridwin = xCalcDoc.getChild("grid_window")

          gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
          gridwin.executeAction("ACTIVATE", tuple())
          xChartMainTop = self.xUITest.getTopFocusWindow()
          xChartMain = xChartMainTop.getChild("chart_window")

          # Select the legends
          xLegends = xChartMain.getChild("CID/D=0:Legend=")
          xLegends.executeAction("SELECT", tuple())

          with self.ui_test.execute_dialog_through_action(xLegends, "COMMAND", mkPropertyValues({"COMMAND": "TransformDialog"})) as xDialog:
            self.assertEqual("4.59", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_X"))['Value'])
            self.assertEqual("1.54", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_Y"))['Value'])

          xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
          xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "LEFT"}))

          # Check the position has changed after moving the label using the arrows keys
          with self.ui_test.execute_dialog_through_action(xLegends, "COMMAND", mkPropertyValues({"COMMAND": "TransformDialog"})) as xDialog:
            self.assertEqual("4.49", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_X"))['Value'])
            self.assertEqual("1.44", get_state_as_dict(xDialog.getChild("MTR_FLD_POS_Y"))['Value'])

   def test_Tdf147394(self):

    with self.ui_test.load_file(get_url_for_data_file("dataLabels.ods")) as calc_doc:
      xCalcDoc = self.xUITest.getTopFocusWindow()
      gridwin = xCalcDoc.getChild("grid_window")

      gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
      gridwin.executeAction("ACTIVATE", tuple())
      xChartMainTop = self.xUITest.getTopFocusWindow()
      xChartMain = xChartMainTop.getChild("chart_window")

      xLegend = calc_doc.Sheets[0].Charts[0].getEmbeddedObject().getFirstDiagram().Legend
      self.assertTrue(xLegend.Show)

      # Select the legends
      xLegends = xChartMain.getChild("CID/D=0:Legend=")
      xLegends.executeAction("SELECT", tuple())

      # Without the fix in place, this test would have crashed here
      xChartMain.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DELETE"}))

      self.assertFalse(xLegend.Show)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
