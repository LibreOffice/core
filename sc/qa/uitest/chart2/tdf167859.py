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


class tdf167859(UITestCase):

    def test_tdf167859(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf167859.ods")):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            gridwin.executeAction("ACTIVATE", tuple())
            xChartMainTop = self.xUITest.getTopFocusWindow()
            xChartMain = xChartMainTop.getChild("chart_window")
            xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")

            # Without the fix in place, this would have crashed here
            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramAxisA"})) as xDialog:

                #Click on tab "Scale".
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "0")

                xReverse = xDialog.getChild("CBX_REVERSE")
                self.assertEqual("false", get_state_as_dict(xReverse)["Selected"])

                xLogarithmicScale = xDialog.getChild("CBX_LOGARITHM")
                self.assertEqual("false", get_state_as_dict(xLogarithmicScale)["Selected"])

            with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DiagramAxisB"})) as xDialog:

                #Click on tab "Positioning".
                tabcontrol = xDialog.getChild("tabcontrol")
                select_pos(tabcontrol, "1")

                xAxisAt = xDialog.getChild("LB_CROSSES_OTHER_AXIS_AT")
                self.assertEqual("End", get_state_as_dict(xAxisAt)["DisplayText"])

                xPlaceLabels = xDialog.getChild("LB_PLACE_LABELS")
                self.assertEqual("Near axis", get_state_as_dict(xPlaceLabels)["DisplayText"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
