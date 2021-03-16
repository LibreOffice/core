# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.calc import enter_text_to_cell

class CalcPasteOnly(UITestCase):

    def test_paste_only(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xTopWindow = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        gridwin = xTopWindow.getChild("grid_window")

        enter_text_to_cell(gridwin, "A1", "=SUM(A2:A3)")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:Copy")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
        self.xUITest.executeCommand(".uno:PasteOnlyText")
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getString(), "")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
        self.xUITest.executeCommand(".uno:PasteOnlyValue")
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getString(), "0")
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getValue(), 0)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E1"}))
        self.xUITest.executeCommand(".uno:PasteOnlyFormula")
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getString(), "0")
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 4, 0).getFormula(), "=SUM(E2:E3)")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
