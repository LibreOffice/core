# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 85979 - Crash: data text to columns

class tdf85979(UITestCase):
    def test_td85979_text_to_columns(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf85979.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #(I selected C1 to C5, then Text to Columns, unselected "Tab" and selected "Space")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C1:C5"}))
        # Data - Text to Columns
        self.ui_test.execute_dialog_through_command(".uno:TextToColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        xspace = xDialog.getChild("space")
        if (get_state_as_dict(xspace)["Selected"]) == "false":
            xspace.executeAction("CLICK", tuple())
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 99)
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 9)
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 9)
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getValue(), 260)
        self.assertEqual(get_cell_by_position(document, 0, 3, 1).getValue(), 10)
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getValue(), 23)
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getValue(), 23)
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getValue(), 149)
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), 14)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 14)
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue(), 16)
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getValue(), 35)
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getValue(), 35)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
