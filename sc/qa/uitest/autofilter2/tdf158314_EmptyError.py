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
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import is_row_hidden

# Bug 158314 - Autofilter dropdown list always shows "Empty" entry as active

class tdf158314_EmptyEntries(UITestCase):
    def testTdf158314(self):
        with self.ui_test.create_doc_in_start_center("calc") as calcDoc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = xCalcDoc.getChild("grid_window")

            # Fill the sheet with test data
            enter_text_to_cell(xGridWin, "A1", "a")
            enter_text_to_cell(xGridWin, "A2", "1")
            enter_text_to_cell(xGridWin, "A3", "2")
            enter_text_to_cell(xGridWin, "A4", "3")
            enter_text_to_cell(xGridWin, "A5", "4")

            enter_text_to_cell(xGridWin, "B1", "b")
            enter_text_to_cell(xGridWin, "B2", "5")
            enter_text_to_cell(xGridWin, "B3", "")
            enter_text_to_cell(xGridWin, "B4", "=1/0")
            enter_text_to_cell(xGridWin, "B5", "8")

            # Select the data range and set autofilter
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B5"}))
            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            # Click the autofilter dropdown in column A
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")

            #  Select the first entry only. Uncheck all other entries.
            for i in xTreeList.getChildren():
                if i != "0":
                    xEntry = xTreeList.getChild(i)
                    xEntry.executeAction("CLICK", tuple())

            xOkButton = xFloatWindow.getChild("ok")
            xOkButton.executeAction("CLICK", tuple())

            # Check that only row#2 is visible
            self.assertFalse(is_row_hidden(calcDoc, 1))
            self.assertTrue(is_row_hidden(calcDoc, 2))
            self.assertTrue(is_row_hidden(calcDoc, 3))
            self.assertTrue(is_row_hidden(calcDoc, 4))

            # Click the autofilter dropdown in column B
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")

            # There should be at least one entry in the dropdown
            # (i.e., the feature of showing inactive autofilter entries may be disabled)
            self.assertEqual(True, len(xTreeList.getChildren()) >= 1)

            for i in xTreeList.getChildren():
                state = get_state_as_dict(xTreeList.getChild(i))
                # The Text of the first element should be '5' and it should be checked and active
                if i == "0":
                    self.assertEqual("5", state["Text"])
                    self.assertEqual("true", state["IsChecked"])
                    self.assertEqual("true", state["IsSelected"])
                    self.assertEqual("false", state["IsSemiTransparent"])
                # All other elements (including the Empty and #DIV/0!) should be unchecked and inactive
                else:
                    self.assertEqual("false", state["IsChecked"])
                    self.assertEqual("false", state["IsSelected"])
                    self.assertEqual("true",  state["IsSemiTransparent"])

            # Close the popup window
            xOkButton = xFloatWindow.getChild("ok")
            xOkButton.executeAction("CLICK", tuple())

            # Check again that only row#2 is visible
            self.assertFalse(is_row_hidden(calcDoc, 1))
            self.assertTrue(is_row_hidden(calcDoc, 2))
            self.assertTrue(is_row_hidden(calcDoc, 3))
            self.assertTrue(is_row_hidden(calcDoc, 4))
# vim: set shiftwidth=4 softtabstop=4 expandtab:
