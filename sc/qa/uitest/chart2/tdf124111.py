# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos

from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 124111 - Cannot enter negative number for cross other axis at value
class tdf124111(UITestCase):
   def test_tdf124111_chart_x_negative_cross(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf124111.ods")) as calc_doc:
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramAxisX"})) as xDialog:
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
            crossAxisValue.executeAction("DOWN", tuple())  #-1


        #reopen and verify tab "positioning".
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramAxisX"})) as xDialog:

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
            self.assertEqual(get_state_as_dict(crossAxisValue)["Text"], "-1")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
