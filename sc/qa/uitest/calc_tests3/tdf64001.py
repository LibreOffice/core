# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf64001(UITestCase):

    def test_tdf64001(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #1) Type TRUE in cell A1
        enter_text_to_cell(gridwin, "A1", "TRUE")
        #2) Autofill/drag A1 to A10, all cells show TRUE
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A10"}))
        self.ui_test.execute_dialog_through_command(".uno:FillSeries")
        xDialog = self.xUITest.getTopFocusWindow()
        xautofill = xDialog.getChild("autofill")
        xautofill.executeAction("CLICK", tuple())
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #3) Type FALSE in A11
        enter_text_to_cell(gridwin, "A11", "FALSE")
        #4) Enter in B1: =COUNTIF(A1:A11,TRUE) , hit enter
        enter_text_to_cell(gridwin, "B1", "=COUNTIF(A1:A11,TRUE)")
        #The formula changes to =COUNTIF(A1:A11,1) and displays result of 1 not 10.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 10)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: