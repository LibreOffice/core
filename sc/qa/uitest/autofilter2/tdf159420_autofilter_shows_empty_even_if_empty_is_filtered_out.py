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

class tdf159420(UITestCase):

    def testTdf159420(self):
        with self.ui_test.create_doc_in_start_center("calc") as calcDoc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = xCalcDoc.getChild("grid_window")

            # Fill the sheet with test data
            enter_text_to_cell(xGridWin, "A1", "a")
            enter_text_to_cell(xGridWin, "A2", "2")
            enter_text_to_cell(xGridWin, "A3", "2")
            enter_text_to_cell(xGridWin, "A4", "2")
            enter_text_to_cell(xGridWin, "A5", "4")

            enter_text_to_cell(xGridWin, "B1", "b")
            enter_text_to_cell(xGridWin, "B2", "")
            enter_text_to_cell(xGridWin, "B3", "")
            enter_text_to_cell(xGridWin, "B4", "8")
            enter_text_to_cell(xGridWin, "B5", "8")

            enter_text_to_cell(xGridWin, "C1", "c")

            # Select the data range and set autofilter
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C5"}))
            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            # Click the autofilter dropdown in column A
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")

            self.assertEqual(2, len(xTreeList.getChildren()))

            xEntry1 = xTreeList.getChild(0)
            self.assertEqual("2", get_state_as_dict(xEntry1)['Text'])
            self.assertEqual("true", get_state_as_dict(xEntry1)['IsChecked'])
            self.assertEqual("false", get_state_as_dict(xEntry1)['IsSemiTransparent'])

            xEntry2 = xTreeList.getChild(1)
            self.assertEqual("4", get_state_as_dict(xEntry2)['Text'])
            self.assertEqual("true", get_state_as_dict(xEntry2)['IsChecked'])
            self.assertEqual("false", get_state_as_dict(xEntry2)['IsSemiTransparent'])

            #  Uncheck the second entry
            xEntry2.executeAction("CLICK", tuple())

            xOkButton = xFloatWindow.getChild("ok")
            xOkButton.executeAction("CLICK", tuple())

            # Check that only row#2 is visible
            self.assertFalse(is_row_hidden(calcDoc, 1))
            self.assertFalse(is_row_hidden(calcDoc, 2))
            self.assertFalse(is_row_hidden(calcDoc, 3))
            self.assertTrue(is_row_hidden(calcDoc, 4))

            # Click the autofilter dropdown in column B
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")

            self.assertEqual(2, len(xTreeList.getChildren()))

            xEntry1 = xTreeList.getChild(0)
            self.assertEqual("(empty)", get_state_as_dict(xEntry1)['Text'])
            self.assertEqual("true", get_state_as_dict(xEntry1)['IsChecked'])
            self.assertEqual("false", get_state_as_dict(xEntry1)['IsSemiTransparent'])

            xEntry2 = xTreeList.getChild(1)
            self.assertEqual("8", get_state_as_dict(xEntry2)['Text'])
            self.assertEqual("true", get_state_as_dict(xEntry2)['IsChecked'])
            self.assertEqual("false", get_state_as_dict(xEntry2)['IsSemiTransparent'])

            #  Uncheck the first entry
            xEntry1.executeAction("CLICK", tuple())

            # Close the popup window
            xOkButton = xFloatWindow.getChild("ok")
            xOkButton.executeAction("CLICK", tuple())

            self.assertTrue(is_row_hidden(calcDoc, 1))
            self.assertTrue(is_row_hidden(calcDoc, 2))
            self.assertFalse(is_row_hidden(calcDoc, 3))
            self.assertTrue(is_row_hidden(calcDoc, 4))

            # Click the autofilter dropdown in column C
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")

            self.assertEqual(1, len(xTreeList.getChildren()))

            xEntry1 = xTreeList.getChild(0)
            self.assertEqual("(empty)", get_state_as_dict(xEntry1)['Text'])

            # Without the fix in place, this test would have failed with
            # AssertionError: 'true' != 'false'
            self.assertEqual("true", get_state_as_dict(xEntry1)['IsChecked'])
            self.assertEqual("false", get_state_as_dict(xEntry1)['IsSemiTransparent'])

            # Close the popup window
            xOkButton = xFloatWindow.getChild("ok")
            xOkButton.executeAction("CLICK", tuple())

            self.assertTrue(is_row_hidden(calcDoc, 1))
            self.assertTrue(is_row_hidden(calcDoc, 2))
            self.assertFalse(is_row_hidden(calcDoc, 3))
            self.assertTrue(is_row_hidden(calcDoc, 4))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
