# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Bug 62349 - Calc crashes when a newly created chart copied to clipboard

class tdf62349(UITestCase):
    def test_tdf62349_crash_copy_chart_clipboard(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf62349.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #3: select all data cells C5:H9
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C5:H9"}))
        # 4: create a chart with insert/chart menu
        self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart")
        xChartDlg = self.xUITest.getTopFocusWindow()

        xOkBtn = xChartDlg.getChild("finish")
        self.ui_test.close_dialog_through_button(xOkBtn)

        # 5: (single) click to an empty cell to finalize the chart
        gridwin.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))
        # 6: (single) click back inside the chart to select it
        gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
        # 7: press CTRL-C
        self.xUITest.executeCommand(".uno:Copy")
        self.xUITest.executeCommand(".uno:Paste")
        #check we didn't crash
        self.assertEqual(get_cell_by_position(document, 0, 2, 5).getString(), "group1")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
