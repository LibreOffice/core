# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 62349 - Calc crashes when a newly created chart copied to clipboard
class tdf62349(UITestCase):
    def test_tdf62349_crash_copy_chart_clipboard(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf62349.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #3: select all data cells C5:H9
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C5:H9"}))
            # 4: create a chart with insert/chart menu
            with self.ui_test.execute_dialog_through_command(".uno:InsertObjectChart", close_button="finish"):
                pass

            # 5: (single) click to an empty cell to finalize the chart
            gridwin.executeAction("DESELECT", mkPropertyValues({"OBJECT": ""}))
            # 6: (single) click back inside the chart to select it
            gridwin.executeAction("SELECT", mkPropertyValues({"OBJECT": "Object 1"}))
            # 7: press CTRL-C
            self.xUITest.executeCommand(".uno:Copy")
            self.xUITest.executeCommand(".uno:Paste")
            #check we didn't crash
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 5).getString(), "group1")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
