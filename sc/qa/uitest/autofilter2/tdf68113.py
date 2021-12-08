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

#Bug 68113 - UI Selecting "Not empty" unchecks all entries

class tdf68113(UITestCase):
    def test_tdf68113_empty_notempty_button(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = calcDoc.getChild("grid_window")

            enter_text_to_cell(xGridWin, "A1", "A")
            enter_text_to_cell(xGridWin, "A2", "1")
            enter_text_to_cell(xGridWin, "A3", "2")
            enter_text_to_cell(xGridWin, "A5", "4")
            enter_text_to_cell(xGridWin, "A7", "6")

            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A7"}))

            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            # Empty button
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xMenu = xFloatWindow.getChild("menu")
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))
            xSubFloatWindow = self.xUITest.getFloatWindow()
            xSubMenu = xSubFloatWindow.getChild("menu")
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            # Checkbox elements
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(5, len(xTreeList.getChildren()))
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('0'))['IsChecked'])
            self.assertEqual('false', get_state_as_dict(xTreeList.getChild('2'))['IsChecked'])
            self.assertEqual('false', get_state_as_dict(xTreeList.getChild('4'))['IsChecked'])
            xCancelBtn = xFloatWindow.getChild("cancel")
            xCancelBtn.executeAction("CLICK", tuple())

            # Not Empty button
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xMenu = xFloatWindow.getChild("menu")
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))
            xSubFloatWindow = self.xUITest.getFloatWindow()
            xSubMenu = xSubFloatWindow.getChild("menu")
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            # Checkbox elements
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(5, len(xTreeList.getChildren()))
            self.assertEqual('false', get_state_as_dict(xTreeList.getChild('0'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('2'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('4'))['IsChecked'])
            xCancelBtn = xFloatWindow.getChild("cancel")
            xCancelBtn.executeAction("CLICK", tuple())


# vim: set shiftwidth=4 softtabstop=4 expandtab:
