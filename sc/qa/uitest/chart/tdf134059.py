# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.calc.document import get_row

class Tdf134059(UITestCase):

  def assertSeriesNames(self, gridwin):

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")

    xPage = xChartMain.getChild("CID/Page=")

    self.ui_test.execute_dialog_through_action(xPage, "COMMAND", mkPropertyValues({"COMMAND": "DataRanges"}))
    xDialog = self.xUITest.getTopFocusWindow()

    xTabs = xDialog.getChild("tabcontrol")
    select_pos(xTabs, "1")

    xSeries = xDialog.getChild("LB_SERIES")
    self.assertEqual(3, len(xSeries.getChildren()))
    self.assertEqual("Col. 1", get_state_as_dict(xSeries.getChild('0'))['Text'])
    self.assertEqual("Col. 2", get_state_as_dict(xSeries.getChild('1'))['Text'])
    self.assertEqual("Col. 3", get_state_as_dict(xSeries.getChild('2'))['Text'])

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    gridwin.executeAction("DESELECT", mkPropertyValues({"OBJECT": "Object 1"}))

  def test_tdf134059(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf134059.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    self.assertSeriesNames(gridwin)

    # Hide row 10
    gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A10"}))
    self.ui_test._xUITest.executeCommand(".uno:HideRow")

    row = get_row(calc_doc, 9)
    self.assertFalse(row.getPropertyValue("IsVisible"))

    # Without the fix in place, this test would have failed with
    # AssertionError: 'Col. 1' != 'Column C'
    self.assertSeriesNames(gridwin)

    self.xUITest.executeCommand(".uno:Undo")

    self.assertTrue(row.getPropertyValue("IsVisible"))

    self.assertSeriesNames(gridwin)

    self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
