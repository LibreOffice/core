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
from uitest.uihelper.common import select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_row
from uitest.uihelper.common import get_state_as_dict

#Bug 141559 - Add Clear Standard Filter to Autofilter widget

class tdf141559(UITestCase):
    def test_tdf141559_clear_filter(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "A2", "1")
            enter_text_to_cell(gridwin, "A3", "2")
            enter_text_to_cell(gridwin, "A4", "3")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))

            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            #Choose Standard Filter... button
            xMenu = xFloatWindow.getChild("menu")

            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))
            xSubFloatWindow = self.xUITest.getFloatWindow()
            xSubMenu = xSubFloatWindow.getChild("menu")

            nLastIdx = int(get_state_as_dict(xSubMenu)['Children']) - 1
            self.assertEqual(5, nLastIdx)

            # check last item: 'Standard Filter...' (new menu item 'Clear Filter' is optional)
            self.assertEqual('Standard Filter...', get_state_as_dict(xSubMenu.getChild(str(nLastIdx)))['Text'])

            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            self.assertEqual("Standard Filter...", get_state_as_dict(xSubMenu)['SelectEntryText'])
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            xDialog = self.xUITest.getTopFocusWindow()
            xfield1 = xDialog.getChild("field1")
            xcond1 = xDialog.getChild("cond1")
            xval1 = xDialog.getChild("val1")

            select_by_text(xfield1, "A")
            select_by_text(xcond1, ">")
            xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
            xOKBtn = xDialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOKBtn)

            row = get_row(document, 1)
            self.assertFalse(row.getPropertyValue("IsVisible"))

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            #Choose Clear Filter button
            xMenu = xFloatWindow.getChild("menu")

            # check last item: 'Clear Filter'
            nLastIdx = int(get_state_as_dict(xMenu)['Children']) - 1
            self.assertEqual(5, nLastIdx)
            self.assertEqual('Clear Filter', get_state_as_dict(xMenu.getChild(str(nLastIdx)))['Text'])

            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            # Use new menu item "Clear Filter" to remove the standard filter condition
            self.assertEqual("Clear Filter", get_state_as_dict(xMenu)['SelectEntryText'])
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            self.assertTrue(row.getPropertyValue("IsVisible"))


# vim: set shiftwidth=4 softtabstop=4 expandtab:
