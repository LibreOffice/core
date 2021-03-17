# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

#Bug 123520 - Y axis - positioning tab, textbox "Cross other axis at" date changed

class tdf123520(UITestCase):
   def test_tdf123520_chart_y_cross_other_axis(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf123520.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()

    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramAxisY"}))  # Y Axis
    xDialog = self.xUITest.getTopFocusWindow()
    #Click on tab "positioning".
    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "1")

    crossAxis = xDialog.getChild("LB_CROSSES_OTHER_AXIS_AT")
    crossAxisValue = xDialog.getChild("EDT_CROSSES_OTHER_AXIS_AT") #only available when crossAxis = Value
    placeLabels = xDialog.getChild("LB_PLACE_LABELS")
    innerMajorTick = xDialog.getChild("CB_TICKS_INNER")
    outerMajorTick = xDialog.getChild("CB_TICKS_OUTER")
    innerMinorTick = xDialog.getChild("CB_MINOR_INNER")
    outerMinorTick = xDialog.getChild("CB_MINOR_OUTER")
    placeMarks = xDialog.getChild("LB_PLACE_TICKS")
    crossAxisValue.executeAction("CLEAR", tuple())
    crossAxisValue.executeAction("TYPE", mkPropertyValues({"TEXT":"01.01.2018"}))
    #crossAxisValue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"TAB"}))
    #TAB doesn't works- add "a" at the end of textbox
    #workaround - edit another ui item, it should trigger leave of textbox
    select_by_text(placeLabels, "Outside start")
    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    #reopen and verify tab "positioning".
    gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
    gridwin.executeAction("ACTIVATE", tuple())
    xChartMainTop = self.xUITest.getTopFocusWindow()
    xChartMain = xChartMainTop.getChild("chart_window")
    xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
    self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramAxisY"}))
    xDialog = self.xUITest.getTopFocusWindow()

    tabcontrol = xDialog.getChild("tabcontrol")
    select_pos(tabcontrol, "1")

    crossAxis = xDialog.getChild("LB_CROSSES_OTHER_AXIS_AT")
    crossAxisValue = xDialog.getChild("EDT_CROSSES_OTHER_AXIS_AT") #only available when crossAxis = Value
    placeLabels = xDialog.getChild("LB_PLACE_LABELS")
    innerMajorTick = xDialog.getChild("CB_TICKS_INNER")
    outerMajorTick = xDialog.getChild("CB_TICKS_OUTER")
    innerMinorTick = xDialog.getChild("CB_MINOR_INNER")
    outerMinorTick = xDialog.getChild("CB_MINOR_OUTER")
    placeMarks = xDialog.getChild("LB_PLACE_TICKS")

    self.assertEqual(get_state_as_dict(crossAxis)["SelectEntryText"], "Value")
    self.assertEqual(get_state_as_dict(crossAxisValue)["Text"], "01.01.2018")
    self.assertEqual(get_state_as_dict(placeLabels)["SelectEntryText"], "Outside start")

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
