# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import is_row_hidden

class tdf153972(UITestCase):
    def test_tdf153972(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf153972.ods")) as doc:
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = calcDoc.getChild("grid_window")

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xMenu = xFloatWindow.getChild("menu")

            # Filter by Color
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            xSubFloatWindow = self.xUITest.getFloatWindow()
            xSubMenu = xSubFloatWindow.getChild("background")

            # Without the fix in place, this test would have failed with
            # AssertionError: 4 != 0
            self.assertEqual(4, len(xSubMenu.getChildren()))
            self.assertEqual('#7FD41A', get_state_as_dict(xSubMenu.getChild('0'))['Text'])
            self.assertEqual('#FE0000', get_state_as_dict(xSubMenu.getChild('1'))['Text'])
            self.assertEqual('#FEFF00', get_state_as_dict(xSubMenu.getChild('2'))['Text'])
            self.assertEqual('No Fill', get_state_as_dict(xSubMenu.getChild('3'))['Text'])

            # Choose Red
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            self.assertTrue(is_row_hidden(doc, 1))
            self.assertTrue(is_row_hidden(doc, 2))
            self.assertTrue(is_row_hidden(doc, 3))
            self.assertTrue(is_row_hidden(doc, 4))
            self.assertTrue(is_row_hidden(doc, 5))
            self.assertFalse(is_row_hidden(doc, 6))
            self.assertTrue(is_row_hidden(doc, 7))
            self.assertTrue(is_row_hidden(doc, 8))
            self.assertFalse(is_row_hidden(doc, 9))
            self.assertTrue(is_row_hidden(doc, 10))
            self.assertFalse(is_row_hidden(doc, 11))
            self.assertTrue(is_row_hidden(doc, 12))
            self.assertFalse(is_row_hidden(doc, 13))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
