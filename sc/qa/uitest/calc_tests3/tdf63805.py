# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 63805 - EDITING: 'Autofill - Date - Months' wrong if day of month exceeds max. days of month

class tdf63805(UITestCase):

    def test_tdf63805_autofill_Date_Months(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf63805.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #1 - A1: 2012-10-31
        enter_text_to_cell(gridwin, "A1", "2012-10-31")
        #2 - Select A1:A20
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A20"}))
        #3 - Edit -> fill -> Series -> Down - Date-Month-Increment=1 <ok>
        self.ui_test.execute_dialog_through_command(".uno:FillSeries")
        xDialog = self.xUITest.getTopFocusWindow()
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #5 -Expected: All Cells show last day of month / Actual: some months skipped
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 41213)
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 41243)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 41274)
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 41305)
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getValue(), 41333)
        self.assertEqual(get_cell_by_position(document, 0, 0, 5).getValue(), 41364)
        self.assertEqual(get_cell_by_position(document, 0, 0, 6).getValue(), 41394)
        self.assertEqual(get_cell_by_position(document, 0, 0, 7).getValue(), 41425)
        self.assertEqual(get_cell_by_position(document, 0, 0, 8).getValue(), 41455)
        self.assertEqual(get_cell_by_position(document, 0, 0, 9).getValue(), 41486)
        self.assertEqual(get_cell_by_position(document, 0, 0, 10).getValue(), 41517)
        self.assertEqual(get_cell_by_position(document, 0, 0, 11).getValue(), 41547)
        self.assertEqual(get_cell_by_position(document, 0, 0, 12).getValue(), 41578)
        self.assertEqual(get_cell_by_position(document, 0, 0, 13).getValue(), 41608)
        self.assertEqual(get_cell_by_position(document, 0, 0, 14).getValue(), 41639)
        self.assertEqual(get_cell_by_position(document, 0, 0, 15).getValue(), 41670)
        self.assertEqual(get_cell_by_position(document, 0, 0, 16).getValue(), 41698)
        self.assertEqual(get_cell_by_position(document, 0, 0, 17).getValue(), 41729)
        self.assertEqual(get_cell_by_position(document, 0, 0, 18).getValue(), 41759)
        self.assertEqual(get_cell_by_position(document, 0, 0, 19).getValue(), 41790)
        #4 - undo.
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 41213)
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 0)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
