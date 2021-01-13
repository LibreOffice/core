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
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text

#Bug 99069 - assertion "SolarMutex not locked" from <Cancel> Data Ranges dialog

class tdf99069(UITestCase):
   def test_tdf99069_chart_cancel_data_ranges_dialog(self):
    calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf99069.ods"))
    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    document = self.ui_test.get_component()
    #(1) Download and open example.ods attached to tdf#97266  with cell B1 active.
    #(2) In tool bar, click the chart icon.  Program presents Chart Wizard.
    self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart")
    xDialog = self.xUITest.getTopFocusWindow()
    #(3) In Chart Wizard, click <Finish>.  The program closes the wizard;
    # the chart shows a border with handles on each side and at each corner.
    xFinishBtn = xDialog.getChild("finish")
    self.ui_test.close_dialog_through_button(xFinishBtn)
    for _ in range(0,5):
        #(4) Click outside the chart, for example in cell C23.  The borders
        #disappear from the chart and the program restores the menubar to
        #the window.  (Yes, this step is necessary to the crash.)
        gridwin.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C23"}))
        #(5) Double-click on the chart.  The program shows a border around the
        # chart.  (It may be necessary to do this a second time before the
        # pop-up menu will offer "Data Ranges...".
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        gridwin.executeAction("ACTIVATE", tuple())
        xChartMainTop = self.xUITest.getTopFocusWindow()
        xChartMain = xChartMainTop.getChild("chart_window")

        #(6) Right-click on the chart; from the pop-up menu select "Data
        # Ranges...".  The program presents dialog "Data Ranges", tab "Data Range".
        xSeriesObj =  xChartMain.getChild("CID/D=0:CS=0:CT=0:Series=0")
        self.ui_test.execute_dialog_through_action(xSeriesObj, "COMMAND", mkPropertyValues({"COMMAND": "DataRanges"}))
        xDialog = self.xUITest.getTopFocusWindow()

        #(7) Click on tab "Data Series".  (Actually, tab "Data Range" crashes,
        # too.  This step is just a remnant of what I was doing when I
        # stumbled over the bug.)
        notebook = xDialog.getChild("tabcontrol")
        select_pos(notebook, "0")
        select_pos(notebook, "1")

        #(8) Click <Cancel>.  In the versions that I deemed bad while
        # bibisecting, the program crashed here five times out of seven.
        # The other two attempts, both on daily bibisect version 2016-02-18,
        # crashed after I closed the Data Ranges dialog an additional three
        # times, one of those times using by typing <Esc>.
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        #verify - we didn't crash
    gridwin.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))
    self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 0.529084)

    self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
