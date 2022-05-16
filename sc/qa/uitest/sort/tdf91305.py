# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 91305 - Sort button does not sort first cell if it has text format
class tdf91305(UITestCase):

    def test_tdf91305_sort_text_cells_columns(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #In column A enter texts
            enter_text_to_cell(gridwin, "A1", "cc")
            enter_text_to_cell(gridwin, "B1", "ff")
            enter_text_to_cell(gridwin, "C1", "aa")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C1"}))
            #Open sort dialog by DATA - SORT
            with self.ui_test.execute_dialog_through_command(".uno:DataSort") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                xleftright = xDialog.getChild("rbLeftRight")
                select_pos(xTabs, "0")
                xleftright.executeAction("CLICK", tuple())
            #verify
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "aa")
            self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "cc")
            self.assertEqual(get_cell_by_position(document, 0, 2, 0).getString(), "ff")
            #Open sort dialog by DATA - SORT
            with self.ui_test.execute_dialog_through_command(".uno:DataSort") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                xleftright = xDialog.getChild("rbLeftRight")
                xdown = xDialog.getChild("down")
                select_pos(xTabs, "0")
                xleftright.executeAction("CLICK", tuple())
                xdown.executeAction("CLICK", tuple())
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "ff")
            self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "cc")
            self.assertEqual(get_cell_by_position(document, 0, 2, 0).getString(), "aa")

        #comment 6 - Additional sub-bugs:
    def test_tdf91305_sort_text_cells_rows(self):
        #Selecting some empty cells and pressing SORT causes empty cells to move below.
        #No matter if you sort from A to Z or from Z to A.
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #In column A enter text
            enter_text_to_cell(gridwin, "A5", "ff")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))
            #sorting
            self.xUITest.executeCommand(".uno:SortAscending")
            self.xUITest.executeCommand(".uno:SortDescending")
            #verify
            self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "ff")
            # Assert that the correct range has been selected
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["MarkedArea"], "Sheet1.A1:Sheet1.A4")


        #2) Placing digit to the header position and running set of sorts will make digit to be on 2nd or last position.
    def test_tdf91305_sort_text_cells_1st_row_digit(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #In column A enter texts
            enter_text_to_cell(gridwin, "A1", "1")
            enter_text_to_cell(gridwin, "A2", "ff")
            enter_text_to_cell(gridwin, "A3", "aa")
            enter_text_to_cell(gridwin, "A4", "cc")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))
            #Press toolbarbutton for ascending sorting .uno:SortAsc
            self.xUITest.executeCommand(".uno:SortAscending")
            #verify
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "1")
            self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "aa")
            self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "cc")
            self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "ff")
            #Press toolbarbutton for descending sorting .uno:SortDescending
            self.xUITest.executeCommand(".uno:SortDescending")
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "ff")
            self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "cc")
            self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "aa")
            self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "1")
            #Undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "1")
            self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "aa")
            self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "cc")
            self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "ff")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
