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
#Bug 105268 - Auto Fill: The Next Value for "001-001-001" is "001-001-002" Rather than "001-001000"

class tdf105268(UITestCase):
    def test_tdf105268(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "001-001-001")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A3"}))
        self.ui_test.execute_dialog_through_command(".uno:FillSeries")
        xDialog = self.xUITest.getTopFocusWindow()
        xautofill = xDialog.getChild("autofill")
        xautofill.executeAction("CLICK", tuple())
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "001-001-001")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "001-001-002")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "001-001-003")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: