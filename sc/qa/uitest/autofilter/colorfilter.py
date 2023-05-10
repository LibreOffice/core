# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, select_pos, select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import is_row_hidden

class ColorFilterTest(UITestCase):

    def test_tdf142580(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf142580.xlsx")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A9"}))

            # Green Text Color rows are displayed
            self.assertFalse(is_row_hidden(doc, 0))
            self.assertFalse(is_row_hidden(doc, 1))
            self.assertFalse(is_row_hidden(doc, 2))
            self.assertFalse(is_row_hidden(doc, 3))
            self.assertTrue(is_row_hidden(doc, 4))
            self.assertTrue(is_row_hidden(doc, 5))
            self.assertTrue(is_row_hidden(doc, 6))
            self.assertTrue(is_row_hidden(doc, 7))
            self.assertTrue(is_row_hidden(doc, 8))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xField1 = xDialog.getChild("field1")
                xCond1 = xDialog.getChild("cond1")

                # tdf#143103: Without the fix in place, this test would have failed with
                # AssertionError: 'Font color' != ''
                self.assertEqual("Text color", get_state_as_dict(xField1)['DisplayText'])
                self.assertEqual("Font color", get_state_as_dict(xCond1)['DisplayText'])

                xColor1 = xDialog.getChild("color1")

                # tdf#142580: Without the fix in place, this test would have failed with
                # AssertionError: '3' != '1'
                self.assertEqual('3', get_state_as_dict(xColor1)["EntryCount"])

                # Select Orange Text Color
                select_pos(xColor1, "2")

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertTrue(is_row_hidden(doc, 1))
            self.assertTrue(is_row_hidden(doc, 2))
            self.assertTrue(is_row_hidden(doc, 3))
            self.assertFalse(is_row_hidden(doc, 4))
            self.assertFalse(is_row_hidden(doc, 5))
            self.assertFalse(is_row_hidden(doc, 6))
            self.assertTrue(is_row_hidden(doc, 7))
            self.assertTrue(is_row_hidden(doc, 8))

    def test_tdf142579(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf142579.xlsx")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A8"}))

            # Blue Background Color rows are displayed
            self.assertFalse(is_row_hidden(doc, 0))
            self.assertFalse(is_row_hidden(doc, 1))
            self.assertTrue(is_row_hidden(doc, 2))
            self.assertTrue(is_row_hidden(doc, 3))
            self.assertTrue(is_row_hidden(doc, 4))
            self.assertTrue(is_row_hidden(doc, 5))
            self.assertTrue(is_row_hidden(doc, 6))
            self.assertTrue(is_row_hidden(doc, 7))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xField1 = xDialog.getChild("field1")
                xCond1 = xDialog.getChild("cond1")

                # tdf#143103: Without the fix in place, this test would have failed with
                # AssertionError: 'aaa' != ''
                self.assertEqual("aaa", get_state_as_dict(xField1)['DisplayText'])
                self.assertEqual("Background color", get_state_as_dict(xCond1)['DisplayText'])

                xColor1 = xDialog.getChild("color1")

                # tdf#142579: Without the fix in place, this test would have failed with
                # AssertionError: '7' != '3'
                self.assertEqual('7', get_state_as_dict(xColor1)["EntryCount"])

                # Select Red Background Color
                select_pos(xColor1, "3")

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertTrue(is_row_hidden(doc, 1))
            self.assertTrue(is_row_hidden(doc, 2))
            self.assertTrue(is_row_hidden(doc, 3))
            self.assertTrue(is_row_hidden(doc, 4))
            self.assertTrue(is_row_hidden(doc, 5))
            self.assertTrue(is_row_hidden(doc, 6))
            self.assertFalse(is_row_hidden(doc, 7))

    def test_tdf144549(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf144549.ods")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))

            for i in range(12):
                self.assertFalse(is_row_hidden(doc, i))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xField1 = xDialog.getChild("field1")
                xCond1 = xDialog.getChild("cond1")

                self.assertEqual("Value", get_state_as_dict(xField1)['DisplayText'])

                select_by_text(xCond1, "Font color")
                self.assertEqual("Font color", get_state_as_dict(xCond1)['DisplayText'])

                xColor1 = xDialog.getChild("color1")

                # Without the fix in place, this test would have failed with
                # AssertionError: '3' != '1'
                self.assertEqual('3', get_state_as_dict(xColor1)["EntryCount"])

                select_pos(xColor1, "0")

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertTrue(is_row_hidden(doc, 1))
            self.assertTrue(is_row_hidden(doc, 2))
            self.assertTrue(is_row_hidden(doc, 3))
            self.assertFalse(is_row_hidden(doc, 4))
            self.assertFalse(is_row_hidden(doc, 5))
            self.assertTrue(is_row_hidden(doc, 6))
            self.assertTrue(is_row_hidden(doc, 7))
            self.assertTrue(is_row_hidden(doc, 8))
            self.assertTrue(is_row_hidden(doc, 9))
            self.assertFalse(is_row_hidden(doc, 10))
            self.assertTrue(is_row_hidden(doc, 11))
            self.assertTrue(is_row_hidden(doc, 12))

    def test_tdf142579_conditional_format(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf142579_cond_format.ods")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A9"}))

            for i in range(8):
                self.assertFalse(is_row_hidden(doc, i))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xField1 = xDialog.getChild("field1")
                xCond1 = xDialog.getChild("cond1")

                self.assertEqual("Another", get_state_as_dict(xField1)['DisplayText'])

                select_by_text(xCond1, "Background color")
                self.assertEqual("Background color", get_state_as_dict(xCond1)['DisplayText'])

                xColor1 = xDialog.getChild("color1")

                # tdf#142579: Without the fix in place, this test would have failed with
                # AssertionError: '2' != '1'
                self.assertEqual('2', get_state_as_dict(xColor1)["EntryCount"])

                select_pos(xColor1, "0")

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertFalse(is_row_hidden(doc, 1))
            self.assertTrue(is_row_hidden(doc, 2))
            self.assertFalse(is_row_hidden(doc, 3))
            self.assertTrue(is_row_hidden(doc, 4))
            self.assertTrue(is_row_hidden(doc, 5))
            self.assertFalse(is_row_hidden(doc, 6))
            self.assertFalse(is_row_hidden(doc, 7))

    def test_tdf76258(self):
        # Reuse existing document
        with self.ui_test.load_file(get_url_for_data_file("tdf142579.xlsx")) as doc:
            calcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = calcDoc.getChild("grid_window")

            # Blue Background Color rows are displayed
            self.assertFalse(is_row_hidden(doc, 0))
            self.assertFalse(is_row_hidden(doc, 1))
            self.assertTrue(is_row_hidden(doc, 2))
            self.assertTrue(is_row_hidden(doc, 3))
            self.assertTrue(is_row_hidden(doc, 4))
            self.assertTrue(is_row_hidden(doc, 5))
            self.assertTrue(is_row_hidden(doc, 6))
            self.assertTrue(is_row_hidden(doc, 7))

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
            self.assertEqual(7, len(xSubMenu.getChildren()))
            self.assertEqual('true', get_state_as_dict(xSubMenu.getChild('0'))['IsChecked'])
            self.assertEqual('#5A8AC6', get_state_as_dict(xSubMenu.getChild('0'))['Text'])
            self.assertEqual('false', get_state_as_dict(xSubMenu.getChild('1'))['IsChecked'])
            self.assertEqual('#90B0D9', get_state_as_dict(xSubMenu.getChild('1'))['Text'])
            self.assertEqual('false', get_state_as_dict(xSubMenu.getChild('2'))['IsChecked'])
            self.assertEqual('#C6D6EC', get_state_as_dict(xSubMenu.getChild('2'))['Text'])
            self.assertEqual('false', get_state_as_dict(xSubMenu.getChild('3'))['IsChecked'])
            self.assertEqual('#F8696B', get_state_as_dict(xSubMenu.getChild('3'))['Text'])
            self.assertEqual('false', get_state_as_dict(xSubMenu.getChild('4'))['IsChecked'])
            self.assertEqual('#FA9A9D', get_state_as_dict(xSubMenu.getChild('4'))['Text'])
            self.assertEqual('false', get_state_as_dict(xSubMenu.getChild('5'))['IsChecked'])
            self.assertEqual('#FBCBCE', get_state_as_dict(xSubMenu.getChild('5'))['Text'])
            self.assertEqual('false', get_state_as_dict(xSubMenu.getChild('6'))['IsChecked'])
            self.assertEqual('#FCFCFF', get_state_as_dict(xSubMenu.getChild('6'))['Text'])

            # Choose another one
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            xSubMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertTrue(is_row_hidden(doc, 1))
            self.assertFalse(is_row_hidden(doc, 2))
            self.assertTrue(is_row_hidden(doc, 3))
            self.assertTrue(is_row_hidden(doc, 4))
            self.assertTrue(is_row_hidden(doc, 5))
            self.assertTrue(is_row_hidden(doc, 6))
            self.assertTrue(is_row_hidden(doc, 7))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
