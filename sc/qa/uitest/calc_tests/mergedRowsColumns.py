# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from libreoffice.calc.document import get_column
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class mergedRowsColumns(UITestCase):

    def test_merged_row_delete_tdf105412(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf105412.ods"))
        xTopWindow = self.xUITest.getTopFocusWindow()
        gridwin = xTopWindow.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A20"}))
        self.xUITest.executeCommand(".uno:SelectRow")
        self.xUITest.executeCommand(".uno:DeleteRows")
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Redo")

        self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 18).getString(), "L6")

        self.ui_test.close_doc()

    def test_merged_columns_delete(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf105412.ods"))
        xTopWindow = self.xUITest.getTopFocusWindow()
        gridwin = xTopWindow.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
        self.xUITest.executeCommand(".uno:SelectColumn")
        self.xUITest.executeCommand(".uno:DeleteColumns")
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Redo")

        self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 18).getString(), "L6")

        self.ui_test.close_doc()

    def test_undo_not_available_merged_cells_tdf37901(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf105412.ods"))
        xTopWindow = self.xUITest.getTopFocusWindow()
        gridwin = xTopWindow.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B1:B2"}))
        self.xUITest.executeCommand(".uno:MergeCells")
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getString(), "Key#")
        self.xUITest.executeCommand(".uno:ToggleMergeCells")
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getString(), "Key#")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:B2"}))
        self.xUITest.executeCommand(".uno:MergeCells")
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getString(), "Key#")
        self.xUITest.executeCommand(".uno:ToggleMergeCells")
        self.xUITest.executeCommand(".uno:Undo")

        self.ui_test.close_doc()

    def test_calculations_in_merged_cells_tdf51368(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf51368.ods"))
        xTopWindow = self.xUITest.getTopFocusWindow()
        gridwin = xTopWindow.getChild("grid_window")

        #move the content of the hidden cells into the first cell
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A11:A12"}))
        self.ui_test.execute_dialog_through_command(".uno:ToggleMergeCells")
        xDialog = self.xUITest.getTopFocusWindow()

        xMoveCells = xDialog.getChild("move-cells-radio")
        xMoveCells.executeAction("CLICK", tuple())
        self.ui_test.close_dialog_through_button(xDialog.getChild("ok"))
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 10).getString(), "2 3")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 11).getString(), "0")

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 10).getString(), "2")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 11).getString(), "3")

        #keep the contents of the hidden cells
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A11:A12"}))
        self.ui_test.execute_dialog_through_command(".uno:ToggleMergeCells")
        xDialog = self.xUITest.getTopFocusWindow()

        xMoveCells = xDialog.getChild("keep-content-radio")
        xMoveCells.executeAction("CLICK", tuple())
        self.ui_test.close_dialog_through_button(xDialog.getChild("ok"))
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 10).getString(), "2")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 11).getString(), "3")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 11).getString(), "3")

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 10).getString(), "2")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 11).getString(), "3")

        #Empty the contents of the hidden cells
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A11:A12"}))
        self.ui_test.execute_dialog_through_command(".uno:ToggleMergeCells")
        xDialog = self.xUITest.getTopFocusWindow()

        xMoveCells = xDialog.getChild("move-cells-radio")
        xMoveCells.executeAction("CLICK", tuple())
        self.ui_test.close_dialog_through_button(xDialog.getChild("ok"))
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 10).getString(), "2 3")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 11).getString(), "")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 11).getString(), "0")

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 10).getString(), "2")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 11).getString(), "3")

        #A21-A22
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A21:A22"}))
        self.ui_test.execute_dialog_through_command(".uno:ToggleMergeCells")
        xDialog = self.xUITest.getTopFocusWindow()

        xMoveCells = xDialog.getChild("keep-content-radio")
        xMoveCells.executeAction("CLICK", tuple())
        self.ui_test.close_dialog_through_button(xDialog.getChild("ok"))
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 20).getString(), "2")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 21).getString(), "3")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 21).getString(), "2")

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 20).getString(), "2")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 21).getString(), "2")

        #A30-A32
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A30:A32"}))
        self.ui_test.execute_dialog_through_command(".uno:ToggleMergeCells")
        xDialog = self.xUITest.getTopFocusWindow()

        xMoveCells = xDialog.getChild("keep-content-radio")
        xMoveCells.executeAction("CLICK", tuple())
        self.ui_test.close_dialog_through_button(xDialog.getChild("ok"))
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 31).getString(), "thisisbad")

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 30).getString(), "is")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 31).getString(), "thisisbad")

        #J12-K12
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "J12:K12"}))
        self.ui_test.execute_dialog_through_command(".uno:ToggleMergeCells")
        xDialog = self.xUITest.getTopFocusWindow()

        xMoveCells = xDialog.getChild("keep-content-radio")
        xMoveCells.executeAction("CLICK", tuple())
        self.ui_test.close_dialog_through_button(xDialog.getChild("ok"))
        self.assertEqual(get_cell_by_position(calc_doc, 0, 9, 11).getString(), "2")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 11, 11).getString(), "3")

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 9, 11).getString(), "2")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 11, 11).getString(), "3")

        #J22-K22
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "J22:K22"}))
        self.ui_test.execute_dialog_through_command(".uno:ToggleMergeCells")
        xDialog = self.xUITest.getTopFocusWindow()

        xMoveCells = xDialog.getChild("keep-content-radio")
        xMoveCells.executeAction("CLICK", tuple())
        self.ui_test.close_dialog_through_button(xDialog.getChild("ok"))
        self.assertEqual(get_cell_by_position(calc_doc, 0, 9, 21).getString(), "2")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 11, 21).getString(), "2")

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 9, 21).getString(), "2")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 11, 21).getString(), "2")
        self.ui_test.close_doc()

    def test_merge_merged_cells_tdf63766(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf105412.ods"))
        xTopWindow = self.xUITest.getTopFocusWindow()
        gridwin = xTopWindow.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C19:F22"}))
        self.xUITest.executeCommand(".uno:ToggleMergeCells")
        self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 18).getString(), "L6")
        self.xUITest.executeCommand(".uno:Undo")

        self.ui_test.close_doc()

    def test_move_merged_cells(self):
        self.ui_test.create_doc_in_start_center("calc")
        xTopWindow = self.xUITest.getTopFocusWindow()
        gridwin = xTopWindow.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B1:C1"}))
        self.xUITest.executeCommand(".uno:ToggleMergeCells")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
        self.xUITest.executeCommand(".uno:GoLeft")
        self.xUITest.executeCommand(".uno:GoLeft")
        self.assertEqual(get_state_as_dict(gridwin)["CurrentColumn"], "0")
        self.assertEqual(get_state_as_dict(gridwin)["CurrentRow"], "0")     #position A1

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:

