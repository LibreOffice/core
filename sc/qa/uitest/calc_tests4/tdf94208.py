# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 94208 - Undo after Control-D Fill doesn't broadcast deleted (undo-inserted) cells for calculation

class tdf94208(UITestCase):

    def test_tdf94208_Undo_fill_down_rows_selected(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #1 - Fill data
        enter_text_to_cell(gridwin, "A3", "Range")
        enter_text_to_cell(gridwin, "A4", "Range")
        enter_text_to_cell(gridwin, "A5", "Range")
        enter_text_to_cell(gridwin, "A6", "Range")
        enter_text_to_cell(gridwin, "A7", "Range")
        enter_text_to_cell(gridwin, "A8", "Range")
        enter_text_to_cell(gridwin, "B6", "test")
        #2 - Select rows three though eight.
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A3:A8"}))
        self.xUITest.executeCommand(".uno:SelectRow")
        #3 - type Control-D/Fill Down
        self.xUITest.executeCommand(".uno:FillDown")
        #4 - undo.
        self.xUITest.executeCommand(".uno:Undo")
        #5 -  Ouch! The data isn't right.
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "Range")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "Range")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "Range")
        self.assertEqual(get_cell_by_position(document, 0, 0, 5).getString(), "Range")
        self.assertEqual(get_cell_by_position(document, 0, 0, 6).getString(), "Range")
        self.assertEqual(get_cell_by_position(document, 0, 0, 6).getString(), "Range")
        self.assertEqual(get_cell_by_position(document, 0, 1, 5).getString(), "test")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
