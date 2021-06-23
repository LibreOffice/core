# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 73006 - EDITING: Results filled in wrong cells after Text to Columns

class tdf73006(UITestCase):
    def test_tdf73006_text_to_columns(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #Add data
        enter_text_to_cell(gridwin, "A2", "A B")
        #select column A
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:SelectColumn")
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
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "A")
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "B")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
