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
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Testcases Sorting TCS_Sorting

class CalcSorting(UITestCase):

    def test_Sortingbuttons_detect_columnheaders(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #In column A enter: Number; 3; 4; 6; 2 / In column B enter: Misc; s; d; f; g
        enter_text_to_cell(gridwin, "A1", "Number")
        enter_text_to_cell(gridwin, "A2", "3")
        enter_text_to_cell(gridwin, "A3", "4")
        enter_text_to_cell(gridwin, "A4", "6")
        enter_text_to_cell(gridwin, "A5", "2")
        enter_text_to_cell(gridwin, "B1", "Misc")
        enter_text_to_cell(gridwin, "B2", "s")
        enter_text_to_cell(gridwin, "B3", "d")
        enter_text_to_cell(gridwin, "B4", "f")
        enter_text_to_cell(gridwin, "B5", "g")
        #Select cell A3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        #Press toolbarbutton for descending sorting .uno:SortDescending
        self.xUITest.executeCommand(".uno:SortDescending")
        #Verify that cell A1 still contains "Number" and B1 "Misc"
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Number")
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "Misc")
        #UNDO
        self.xUITest.executeCommand(".uno:Undo")
        #Select cell B3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B3"}))
        #Press toolbar button for ascending sorting
        self.xUITest.executeCommand(".uno:SortAscending")
        #Verify that cell A1 still contains "Number" and B1 "Misc"
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Number")
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "Misc")
        #UNDO
        self.xUITest.executeCommand(".uno:Undo")
        #Select cell A3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        #Open sort dialog by DATA - SORT /Switch to tabpage Options
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        #Verify that option "Range contains column labels" is set
        xHeader = xDialog.getChild("header")
        self.assertEqual(get_state_as_dict(xHeader)["Selected"], "true")
        #Cancel dialog
        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)
        #Select Range A1:B5
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B5"}))
        #Press toolbarbutton for descending sorting
        self.xUITest.executeCommand(".uno:SortDescending")
        #Verify that cell A1 still contains "Number" and B1 "Misc"
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Number")
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "Misc")
        self.ui_test.close_doc()

    def test_Sortingbuttons_list_has_not_columnheaders(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #In column A enter: 5; 3; 4; 6; 2 / In column B enter: e; s; d; f; g
        enter_text_to_cell(gridwin, "A1", "5")
        enter_text_to_cell(gridwin, "A2", "3")
        enter_text_to_cell(gridwin, "A3", "4")
        enter_text_to_cell(gridwin, "A4", "6")
        enter_text_to_cell(gridwin, "A5", "2")
        enter_text_to_cell(gridwin, "B1", "e")
        enter_text_to_cell(gridwin, "B2", "s")
        enter_text_to_cell(gridwin, "B3", "d")
        enter_text_to_cell(gridwin, "B4", "f")
        enter_text_to_cell(gridwin, "B5", "g")
        #Select cell A3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        #Press toolbar button for ascending sorting
        self.xUITest.executeCommand(".uno:SortAscending")
        #Verify that cell A1 no longer contains "5" and B1 no longer contains "e"
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString() != "5", True)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString() != "e", True)
        #UNDO
        self.xUITest.executeCommand(".uno:Undo")
        #Select cell B3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B3"}))
        #Open sort dialog by DATA - SORT /Switch to tabpage Options
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        #Verify that option "Range contains column labels" is not set
        xHeader = xDialog.getChild("header")
        self.assertEqual(get_state_as_dict(xHeader)["Selected"], "false")
        #Cancel dialog
        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)
        self.ui_test.close_doc()

    def test_Sorting_default_to_selected_column(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #In column A enter: Number; 3; 4; 6; 2 / In column B enter: Misc; s; d; f; g
        enter_text_to_cell(gridwin, "A1", "Number")
        enter_text_to_cell(gridwin, "A2", "3")
        enter_text_to_cell(gridwin, "A3", "4")
        enter_text_to_cell(gridwin, "A4", "6")
        enter_text_to_cell(gridwin, "A5", "2")
        enter_text_to_cell(gridwin, "B1", "Misc")
        enter_text_to_cell(gridwin, "B2", "s")
        enter_text_to_cell(gridwin, "B3", "d")
        enter_text_to_cell(gridwin, "B4", "f")
        enter_text_to_cell(gridwin, "B5", "g")
        #Select cell A3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        #Press toolbarbutton for descending sorting .uno:SortDescending
        self.xUITest.executeCommand(".uno:SortDescending")
        #Verify that the sortorder was determined for column A (Number;2;3;4;6)
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Number")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 6)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getValue(), 2)
        #UNDO
        self.xUITest.executeCommand(".uno:Undo")
        #Select cell B3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B3"}))
        #Press toolbar button for ascending sorting
        self.xUITest.executeCommand(".uno:SortAscending")
        #Verify that the sortorder was determined for column B (Misc;s;g;f;d)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "Misc")
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "d")
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getString(), "f")
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getString(), "g")
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getString(), "s")

        self.ui_test.close_doc()

    def test_Sorting_default_to_selected_TAB_A_column(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #In column A enter: Number; 3; 4; 6; 2 / In column B enter: Misc; s; d; f; g
        enter_text_to_cell(gridwin, "A1", "Number")
        enter_text_to_cell(gridwin, "A2", "3")
        enter_text_to_cell(gridwin, "A3", "4")
        enter_text_to_cell(gridwin, "A4", "6")
        enter_text_to_cell(gridwin, "A5", "2")
        enter_text_to_cell(gridwin, "B1", "Misc")
        enter_text_to_cell(gridwin, "B2", "s")
        enter_text_to_cell(gridwin, "B3", "d")
        enter_text_to_cell(gridwin, "B4", "f")
        enter_text_to_cell(gridwin, "B5", "g")
        #Select Range A1:B5
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B5"}))
        #Move the active cell inside the range to column A by using the TAB key
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        #Press toolbar button for ascending sorting
        self.xUITest.executeCommand(".uno:SortAscending")
        #Verify that the sortorder was determined for column A (Number;2;3;4;6)
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Number")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getValue(), 6)

        self.ui_test.close_doc()

    def test_Sorting_default_to_selected_TAB_B_column(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #In column A enter: Number; 3; 4; 6; 2 / In column B enter: Misc; s; d; f; g
        enter_text_to_cell(gridwin, "A1", "Number")
        enter_text_to_cell(gridwin, "A2", "3")
        enter_text_to_cell(gridwin, "A3", "4")
        enter_text_to_cell(gridwin, "A4", "6")
        enter_text_to_cell(gridwin, "A5", "2")
        enter_text_to_cell(gridwin, "B1", "Misc")
        enter_text_to_cell(gridwin, "B2", "s")
        enter_text_to_cell(gridwin, "B3", "d")
        enter_text_to_cell(gridwin, "B4", "f")
        enter_text_to_cell(gridwin, "B5", "g")
        #Select Range A1:B5
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B5"}))
        #Move the active cell inside the range to column B by using the TAB key
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        #Press toolbar button for ascending sorting
        self.xUITest.executeCommand(".uno:SortAscending")
        #Verify that the sortorder was determined for column B (Misc;d;f;g;s)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "Misc")
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "d")
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getString(), "f")
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getString(), "g")
        self.assertEqual(get_cell_by_position(document, 0, 1, 4).getString(), "s")

        self.ui_test.close_doc()

    def test_Sorting_sort_criteria(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #In column A enter: Number; 3; 4; 6; 2 / In column B enter: Misc; s; d; f; g
        enter_text_to_cell(gridwin, "A1", "Number")
        enter_text_to_cell(gridwin, "A2", "3")
        enter_text_to_cell(gridwin, "A3", "4")
        enter_text_to_cell(gridwin, "A4", "6")
        enter_text_to_cell(gridwin, "A5", "2")
        enter_text_to_cell(gridwin, "B1", "Misc")
        enter_text_to_cell(gridwin, "B2", "s")
        enter_text_to_cell(gridwin, "B3", "d")
        enter_text_to_cell(gridwin, "B4", "f")
        enter_text_to_cell(gridwin, "B5", "g")
        #Select cell A3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        #Open sort dialog by DATA - SORT
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        #Verify that the first sort criteria is set to "Number(ascending)"
        xSortKey1 = xDialog.getChild("sortlb")
        xAsc = xDialog.getChild("up")
        self.assertEqual(get_state_as_dict(xSortKey1)["SelectEntryText"], "Number")
        self.assertEqual(get_state_as_dict(xAsc)["Checked"], "true")
        #Cancel dialog
        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)
        #Select cell B3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B3"}))
        #Open sort dialog by DATA - SORT
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        #Verify that the first sort criteria is set to "Misc (ascending)"
        xSortKey1 = xDialog.getChild("sortlb")
        xAsc = xDialog.getChild("up")
        self.assertEqual(get_state_as_dict(xSortKey1)["SelectEntryText"], "Misc")
        self.assertEqual(get_state_as_dict(xAsc)["Checked"], "true")
        #Cancel dialog
        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)
        #Select Range A1:B5
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B5"}))
        #Move the active cell inside the range to column A by using the TAB key
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        #Open sort dialog by DATA - SORT
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        #Verify that the first sort criteria is set to "Number(ascending)"
        xSortKey1 = xDialog.getChild("sortlb")
        xAsc = xDialog.getChild("up")
        self.assertEqual(get_state_as_dict(xSortKey1)["SelectEntryText"], "Number")
        self.assertEqual(get_state_as_dict(xAsc)["Checked"], "true")
        #Cancel dialog
        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)
        #Select Range A1:B5
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B5"}))
        #Move the active cell inside the range to column B by using the TAB key
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "TAB"}))
        #Open sort dialog by DATA - SORT
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        #Verify that the first sort criteria is set to "Misc (ascending)"
        xSortKey1 = xDialog.getChild("sortlb")
        xAsc = xDialog.getChild("up")
        self.assertEqual(get_state_as_dict(xSortKey1)["SelectEntryText"], "Misc")
        self.assertEqual(get_state_as_dict(xAsc)["Checked"], "true")
        #Cancel dialog
        xCanc = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCanc)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab: