# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Testcases Sorting TCS_Sorting Stable sorting

class CalcStableSorting(UITestCase):

    def test_Must_keep_sort_order_previous_sorting_toolbar_button_Ascending(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("stableSorting.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Select cell E1 ("Sales") and press toolbar button for ascending sorting.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E1"}))
        self.xUITest.executeCommand(".uno:SortAscending")
        #Select cell D1 ("Product") and press toolbar button for ascending sorting.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
        self.xUITest.executeCommand(".uno:SortAscending")
        #Select cell C1 ("Salesman") and press toolbar button for ascending sorting.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
        self.xUITest.executeCommand(".uno:SortAscending")
        # Select cell B1 ("Region") and press toolbar button for ascending sorting.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
        self.xUITest.executeCommand(".uno:SortAscending")
        #Verify that the numbers in column "CheckOrder" are ascending
        for i in range(1, 501):
            self.assertEqual(get_cell_by_position(document, 0, 5, i).getValue(), i)
        self.ui_test.close_doc()

    def test_Must_keep_sort_order_previous_sorting_toolbar_button_Descending(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("stableSorting.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Select cell E1 ("Sales") and press toolbar button for descending sorting.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E1"}))
        self.xUITest.executeCommand(".uno:SortDescending")
        #Select cell D1 ("Product") and press toolbar button for descending sorting.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
        self.xUITest.executeCommand(".uno:SortDescending")
        #Select cell C1 ("Salesman") and press toolbar button for descending sorting.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
        self.xUITest.executeCommand(".uno:SortDescending")
        # Select cell B1 ("Region") and press toolbar button for descending sorting.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
        self.xUITest.executeCommand(".uno:SortDescending")
        #Verify that the numbers in column "CheckOrder" are ascending
        for i in range(1, 501):
            j = 501 - i
            self.assertEqual(get_cell_by_position(document, 0, 5, i).getValue(), j)
        self.ui_test.close_doc()

    # def test_Must_keep_sort_order_previous_sorting_using_sort_dialog(self):
#   cannot test for now - criteria names are identical - Markus https://gerrit.libreoffice.org/#/c/52534/
        # calc_doc = self.ui_test.load_file(get_url_for_data_file("stableSorting.ods"))
        # xCalcDoc = self.xUITest.getTopFocusWindow()
        # gridwin = xCalcDoc.getChild("grid_window")
        # document = self.ui_test.get_component()
#        Select cell A1 and open sort dialog by DATA - SORT
        # gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
#        Open sort dialog by DATA - SORT /Switch to tabpage Options
        # self.ui_test.execute_dialog_through_command(".uno:DataSort")
        # xDialog = self.xUITest.getTopFocusWindow()
        # xTabs = xDialog.getChild("tabcontrol")
        # select_pos(xTabs, "1")
#        Check option "Range contains column labels"
        # xHeader = xDialog.getChild("header")
        # self.assertEqual(get_state_as_dict(xHeader)["Selected"], "true")
#        Switch to tabpage "Sort Criteria"
        # select_pos(xTabs, "0")
#        Choose "Salesman(ascending)" as first criteria
        # xSortKey1 = xDialog.getChild("sortlb")
        # xAsc = xDialog.getChild("up")
        # props = {"TEXT": "Salesman"}
        # actionProps = mkPropertyValues(props)
        # xSortKey1.executeAction("SELECT", actionProps)
        # self.assertEqual(get_state_as_dict(xAsc)["Checked"], "true")
        #Choose "Product (ascending)" as second criteria
        # xSortKey2 = xDialog.getChild("sortuserlb")
        # xAsc = xDialog.getChild("up")
        # props = {"TEXT": "Salesman"}
        # actionProps = mkPropertyValues(props)
        # xSortKey1.executeAction("SELECT", actionProps)
        # self.assertEqual(get_state_as_dict(xAsc)["Checked"], "true")
        # self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
