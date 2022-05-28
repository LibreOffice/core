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


# Bug 123013 - Can not change Trendline name in charts
class tdf123013(UITestCase):
   def test_tdf96432_chart_trendline_name(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf123013.ods")):
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
            #Click on tab "Type".
            tabcontrol = xDialog.getChild("tabcontrol")
            select_pos(tabcontrol, "0")
            #add name
            xentryname = xDialog.getChild("entry_name")
            xentryname.executeAction("TYPE", mkPropertyValues({"TEXT":"Tline"}))

        #reopen and try again
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")

        #Right-click on the chart; from the pop-up menu select "Format Y bars
        # The program presents dialog "Format Y bars", tab "Line".
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        with self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "FormatTrendline"})) as xDialog:
            #Click on tab "Type".
            tabcontrol = xDialog.getChild("tabcontrol")
            select_pos(tabcontrol, "0")
            #add name
            xentryname = xDialog.getChild("entry_name")
            self.assertEqual(get_state_as_dict(xentryname)["Text"], "Tline")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
