# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell

class tdf120174(UITestCase):
    def test_tdf120174(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "121")
        enter_text_to_cell(gridwin, "B1", "=A1")
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 121) # B1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        # Delete contents of A1
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DELETE"}))
        # Before the fix the result would be still 121.
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 0) # B1
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 121) # B1
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
