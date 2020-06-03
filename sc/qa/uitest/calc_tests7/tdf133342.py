# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class tdf133342(UITestCase):
    def test_tdf133342_Add_delete_decimals_of_percent_with_text(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf133342.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "12,35 %")
        #Select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #Add decimals
        self.xUITest.executeCommand(".uno:NumberFormatIncDecimals")
        #Space should preserved before percent sign
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "12,346 %")

        #Delete decimals
        self.xUITest.executeCommand(".uno:NumberFormatDecDecimals")
        self.xUITest.executeCommand(".uno:NumberFormatDecDecimals")
        self.xUITest.executeCommand(".uno:NumberFormatDecDecimals")
        #Space should preserved before percent sign
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "12 %")

        self.xUITest.executeCommand(".uno:NumberFormatDecDecimals")
        #Space should preserved before percent sign
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "12%")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
