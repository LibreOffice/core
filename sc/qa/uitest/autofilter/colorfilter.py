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
                # AssertionError: 'Text color' != ''
                self.assertEqual("Text color", get_state_as_dict(xField1)['DisplayText'])
                self.assertEqual("Text color", get_state_as_dict(xCond1)['DisplayText'])

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

# vim: set shiftwidth=4 softtabstop=4 expandtab:
