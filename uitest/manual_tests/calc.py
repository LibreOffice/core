# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position

from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell

import time

class ManualCalcTests(UITestCase):

    # http://manual-test.libreoffice.org/manage/case/189/
    def test_define_database_range(self):

        self.ui_test.create_doc_in_start_center("calc")

        # Select range A1:D10
        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
        xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:D10"}))

        # Execute "Define DB Range dialog"
        self.ui_test.execute_modeless_dialog_through_command(".uno:DefineDBName")

        xDefineNameDlg = self.xUITest.getTopFocusWindow()

        xEntryBox = xDefineNameDlg.getChild("entry")
        xEntryBox.executeAction("TYPE", mkPropertyValues({"TEXT": "my_database"}))

        xOkBtn = xDefineNameDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        # Deselect range
        xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

        # Execute "Select DB Range dialog"
        self.ui_test.execute_dialog_through_command(".uno:SelectDB")
        xSelectNameDlg = self.xUITest.getTopFocusWindow()

        xListBox = xSelectNameDlg.getChild("treeview")
        xListBoxState = get_state_as_dict(xListBox)
        self.assertEqual(xListBoxState["SelectEntryCount"], "1")
        self.assertEqual(xListBoxState["SelectEntryText"], "my_database")

        xOkBtn = xSelectNameDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        # Assert that the correct range has been selected
        gridWinState = get_state_as_dict(xGridWin)
        self.assertEqual(gridWinState["MarkedArea"], "Sheet1.A1:Sheet1.D10")

        self.ui_test.close_doc()

    # http://manual-test.libreoffice.org/manage/case/190/
    def test_sort_data(self):
        self.ui_test.create_doc_in_start_center("calc")

        # Insert data
        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
        enter_text_to_cell(xGridWin, "B1", "3")
        enter_text_to_cell(xGridWin, "B2", "25")
        enter_text_to_cell(xGridWin, "B3", "17")
        enter_text_to_cell(xGridWin, "B4", "9")
        enter_text_to_cell(xGridWin, "B5", "19")
        enter_text_to_cell(xGridWin, "B6", "0")
        enter_text_to_cell(xGridWin, "B7", "107")
        enter_text_to_cell(xGridWin, "B8", "89")
        enter_text_to_cell(xGridWin, "B9", "8")
        enter_text_to_cell(xGridWin, "B10", "33")

        xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "B1:B10"}))

        # Execute "Sort" dialog
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xSortDlg = self.xUITest.getTopFocusWindow()

        xOkBtn = xSortDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        document = self.ui_test.get_component()

        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 8)
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 9)
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getValue(), 17)
        self.assertEqual(get_cell_by_position(document, 0, 1, 5).getValue(), 19)
        self.assertEqual(get_cell_by_position(document, 0, 1, 6).getValue(), 25)
        self.assertEqual(get_cell_by_position(document, 0, 1, 7).getValue(), 33)
        self.assertEqual(get_cell_by_position(document, 0, 1, 8).getValue(), 89)
        self.assertEqual(get_cell_by_position(document, 0, 1, 9).getValue(), 107)

        time.sleep(2)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
