# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_row
from uitest.uihelper.common import get_url_for_data_file

def is_row_hidden(doc, index):
    row = get_row(doc, index)
    val = row.getPropertyValue("IsVisible")
    return not val

#Bug 117276 - Autofilter settings being reset in some cases

class tdf117276_autofilter_reset(UITestCase):
    def get_values_count_in_AutoFilter(self, xGridWindow, columnIndex, buttonName = "cancel"):
        # open filter pop-up window
        self.assertIsNotNone(xGridWindow)
        xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": columnIndex, "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        self.assertIsNotNone(xFloatWindow)

        # get check list
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        self.assertIsNotNone(xCheckListMenu)

        xTreeList = xCheckListMenu.getChild("check_list_box")
        self.assertIsNotNone(xTreeList)

        valuesCount = len(xTreeList.getChildren())

        # close pop-up window
        xButton = xFloatWindow.getChild(buttonName)
        self.assertIsNotNone(xButton)
        xButton.executeAction("CLICK", tuple())

        return valuesCount

    def openAutoFilterAndCloseIt(self, xGridWindow, columnIndex, buttonName):
        self.get_values_count_in_AutoFilter(xGridWindow, columnIndex, buttonName)

    def check_state(self, document, xGridWindow):
        self.assertFalse(is_row_hidden(document, 0))  # column headers
        self.assertTrue(is_row_hidden(document, 1))
        self.assertTrue(is_row_hidden(document, 2))
        self.assertTrue(is_row_hidden(document, 3))
        self.assertTrue(is_row_hidden(document, 4))
        self.assertFalse(is_row_hidden(document, 5))

        self.assertEqual(1, self.get_values_count_in_AutoFilter(xGridWindow, "0"))
        self.assertEqual(2, self.get_values_count_in_AutoFilter(xGridWindow, "1"))

    def test_run(self):
        self.ui_test.create_doc_in_start_center("calc")
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = calcDoc.getChild("grid_window")

#        self.ui_test.execute_dialog_through_command(".uno:Insert")  # insert sheet
#        xDialog = self.xUITest.getTopFocusWindow()
#        xOKButton = xDialog.getChild("ok")
#        xOKButton.executeAction("CLICK", tuple())

        # 1. prepare document
        #    |    A       | B                      |
        #  -----------------------------------------
        #  1 | HEADER-A   | HEADER-B               |
        #  -----------------------------------------
        #  2 | Unique a2  | common value for B2:B4 |
        #  3 | Unique a3  | common value for B2:B4 |
        #  4 | Unique a4  | common value for B2:B4 |
        #  5 | Unique a5  | Unique b5              |
        #  6 | Unique a6  | Unique b6              |
        #
        # row-1
        enter_text_to_cell(xGridWindow, "A1", "HEADER-A")
        enter_text_to_cell(xGridWindow, "B1", "HEADER-B")

        # row-1
        enter_text_to_cell(xGridWindow, "A2", "Unique a2")
        enter_text_to_cell(xGridWindow, "B2", "common value for B2:B4")

        # row-2
        enter_text_to_cell(xGridWindow, "A3", "Unique a3")
        enter_text_to_cell(xGridWindow, "B3", "common value for B2:B4")

        # row-3
        enter_text_to_cell(xGridWindow, "A4", "Unique a4")
        enter_text_to_cell(xGridWindow, "B4", "common value for B2:B4")

        # row-4
        enter_text_to_cell(xGridWindow, "A5", "Unique a5")
        enter_text_to_cell(xGridWindow, "B5", "Unique b5")

        # row-5
        enter_text_to_cell(xGridWindow, "A6", "Unique a6")
        enter_text_to_cell(xGridWindow, "B6", "Unique b6")

        xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B6"}))

        # 2. switch on auto-filter
        self.ui_test.execute_dialog_through_command(".uno:DataFilterAutoFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xYesBtn = xDialog.getChild("yes")
        self.ui_test.close_dialog_through_button(xYesBtn)

        # autofilter still exist
        self.assertEqual(document.getPropertyValue("UnnamedDatabaseRanges").getByTable(0).AutoFilter, True)

        # 3. open filter of column A and deselect first 3 entries (Unique a2, Unique a3, Unique a4)
        xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")

        xEntry1 = xTreeList.getChild("0")
        xEntry1.executeAction("CLICK", tuple())   # Unique a2
        xEntry2 = xTreeList.getChild("1")
        xEntry2.executeAction("CLICK", tuple())   # Unique a3
        xEntry3 = xTreeList.getChild("2")
        xEntry3.executeAction("CLICK", tuple())   # Unique a4

        xOkButton = xFloatWindow.getChild("ok")
        xOkButton.executeAction("CLICK", tuple())

        # check filtering
        #    |    A       | B                      |
        #  -----------------------------------------
        #  1 | HEADER-A   | HEADER-B               |
        #  -----------------------------------------
        #  5 | Unique a5  | Unique b5              |
        #  6 | Unique a6  | Unique b6              |
        self.assertFalse(is_row_hidden(document, 0))  # column headers
        self.assertTrue(is_row_hidden(document, 1))
        self.assertTrue(is_row_hidden(document, 2))
        self.assertTrue(is_row_hidden(document, 3))
        self.assertFalse(is_row_hidden(document, 4))
        self.assertFalse(is_row_hidden(document, 5))

        self.assertEqual(5, self.get_values_count_in_AutoFilter(xGridWindow, "0"))
        self.assertEqual(2, self.get_values_count_in_AutoFilter(xGridWindow, "1"))

        # 4. open filter of column B and deselect "Unique b5"
        xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")

        xEntry = xTreeList.getChild("0")
        xEntry.executeAction("CLICK", tuple())   # Unique b5

        xOkButton = xFloatWindow.getChild("ok")
        xOkButton.executeAction("CLICK", tuple())

        # check filtering
        #    |    A       | B                      |
        #  -----------------------------------------
        #  1 | HEADER-A   | HEADER-B               |
        #  -----------------------------------------
        #  6 | Unique a6  | Unique b6              |
        self.check_state(document, xGridWindow)

        # 4. open filters of column A, B and close it using different buttons
        self.openAutoFilterAndCloseIt(xGridWindow, "1", "ok")
        self.check_state(document, xGridWindow)

        self.openAutoFilterAndCloseIt(xGridWindow, "1", "cancel")
        self.check_state(document, xGridWindow)

        self.openAutoFilterAndCloseIt(xGridWindow, "0", "cancel")
        self.check_state(document, xGridWindow)

        self.openAutoFilterAndCloseIt(xGridWindow, "0", "ok")
        self.check_state(document, xGridWindow)

        # finish
        self.ui_test.close_doc()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
