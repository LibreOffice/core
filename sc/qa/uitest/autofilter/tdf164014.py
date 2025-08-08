# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import is_row_hidden
from uitest.uihelper.calc import enter_text_to_cell
from tempfile import TemporaryDirectory
from org.libreoffice.unotest import systemPathToFileUrl
import os.path

class AutofilterTest(UITestCase):

    def click_clear_filter_button(self, xMenu):
        xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
        xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
        xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
        xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
        xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))

        self.assertEqual("Clear Filter", get_state_as_dict(xMenu)['SelectEntryText'])
        xMenu.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RETURN"}))

    def test_tdf164014(self):

        with TemporaryDirectory() as tempdir:
            xFilePath = os.path.join(tempdir, "tdf164014-tmp.ods")

            with self.ui_test.load_file(get_url_for_data_file("tdf164014.ods")) as doc:

                self.assertFalse(is_row_hidden(doc, 0))

                for row in range(1, 25):
                    if row == 21 or row == 22:
                        self.assertFalse(is_row_hidden(doc, row))
                    else:
                        self.assertTrue(is_row_hidden(doc, row))

                xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
                xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

                xFloatWindow = self.xUITest.getFloatWindow()
                xCheckListMenu = xFloatWindow.getChild("FilterDropDown")

                xTreeList = xCheckListMenu.getChild("check_list_box")
                self.assertEqual(24, len(xTreeList.getChildren()))

                xEntry1 = xTreeList.getChild(0)
                self.assertEqual("uutdmtlbwk", get_state_as_dict(xEntry1)['Text'])
                self.assertEqual("true", get_state_as_dict(xEntry1)['IsChecked'])
                self.assertEqual("false", get_state_as_dict(xEntry1)['IsSemiTransparent'])

                xEntry2 = xTreeList.getChild(1)
                self.assertEqual("vsjhgdgzka", get_state_as_dict(xEntry2)['Text'])
                self.assertEqual("true", get_state_as_dict(xEntry2)['IsChecked'])
                self.assertEqual("false", get_state_as_dict(xEntry2)['IsSemiTransparent'])

                for it in range(2, 24):
                    xEntry = xTreeList.getChild(it)
                    self.assertEqual("false", get_state_as_dict(xEntry)['IsChecked'])
                    self.assertEqual("true", get_state_as_dict(xEntry)['IsSemiTransparent'])

                xEntry1.executeAction("CLICK", tuple())

                xOkBtn = xFloatWindow.getChild("ok")
                xOkBtn.executeAction("CLICK", tuple())

                for row in range(1, 25):
                    if row == 21:
                        self.assertFalse(is_row_hidden(doc, row))
                    else:
                        self.assertTrue(is_row_hidden(doc, row))

                with self.ui_test.execute_dialog_through_command(".uno:SaveAs", close_button="open") as xSaveDialog:
                    xFileName = xSaveDialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": xFilePath}))

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as doc:

                for row in range(1, 25):
                    if row == 21:
                        self.assertFalse(is_row_hidden(doc, row))
                    else:
                        self.assertTrue(is_row_hidden(doc, row))

                xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
                xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

                xFloatWindow = self.xUITest.getFloatWindow()
                xCheckListMenu = xFloatWindow.getChild("FilterDropDown")

                xTreeList = xCheckListMenu.getChild("check_list_box")
                self.assertEqual(24, len(xTreeList.getChildren()))

                xEntry1 = xTreeList.getChild(0)
                self.assertEqual("uutdmtlbwk", get_state_as_dict(xEntry1)['Text'])
                self.assertEqual("false", get_state_as_dict(xEntry1)['IsChecked'])
                self.assertEqual("false", get_state_as_dict(xEntry1)['IsSemiTransparent'])

                xEntry2 = xTreeList.getChild(1)
                self.assertEqual("vsjhgdgzka", get_state_as_dict(xEntry2)['Text'])
                self.assertEqual("true", get_state_as_dict(xEntry2)['IsChecked'])
                self.assertEqual("false", get_state_as_dict(xEntry2)['IsSemiTransparent'])

                for it in range(2, 24):
                    xEntry = xTreeList.getChild(it)
                    self.assertEqual("false", get_state_as_dict(xEntry)['IsChecked'])
                    self.assertEqual("true", get_state_as_dict(xEntry)['IsSemiTransparent'])

                self.click_clear_filter_button(xFloatWindow.getChild("menu"))

                for row in range(1, 25):
                    if row == 21 or row == 22:
                        self.assertFalse(is_row_hidden(doc, row))
                    else:
                        self.assertTrue(is_row_hidden(doc, row))

                xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
                xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))

                xFloatWindow = self.xUITest.getFloatWindow()
                xCheckListMenu = xFloatWindow.getChild("FilterDropDown")

                xTreeList = xCheckListMenu.getChild("check_list_box")
                self.assertEqual(7, len(xTreeList.getChildren()))

                xEntry1 = xTreeList.getChild(0)
                self.assertEqual("8", get_state_as_dict(xEntry1)['Text'])
                self.assertEqual("false", get_state_as_dict(xEntry1)['IsChecked'])
                self.assertEqual("false", get_state_as_dict(xEntry1)['IsSemiTransparent'])

                xEntry2 = xTreeList.getChild(1)
                self.assertEqual("16", get_state_as_dict(xEntry2)['Text'])
                self.assertEqual("true", get_state_as_dict(xEntry2)['IsChecked'])
                self.assertEqual("false", get_state_as_dict(xEntry2)['IsSemiTransparent'])

                for it in range(2, 7):
                    xEntry = xTreeList.getChild(it)
                    self.assertEqual("false", get_state_as_dict(xEntry)['IsChecked'])
                    self.assertEqual("false", get_state_as_dict(xEntry)['IsSemiTransparent'])

                self.click_clear_filter_button(xFloatWindow.getChild("menu"))

                for row in range(25):
                    self.assertFalse(is_row_hidden(doc, row))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
