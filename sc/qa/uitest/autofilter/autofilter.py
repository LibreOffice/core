# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import is_row_hidden
from uitest.uihelper.calc import enter_text_to_cell

class AutofilterTest(UITestCase):

    def test_tdf106214(self):
        with self.ui_test.load_file(get_url_for_data_file("autofilter.ods")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")

            xTreeList = xCheckListMenu.getChild("check_tree_box")
            xFirstEntry = xTreeList.getChild("0")

            xFirstEntry.executeAction("CLICK", tuple())

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertFalse(is_row_hidden(doc, 1))
            self.assertFalse(is_row_hidden(doc, 2))
            self.assertTrue(is_row_hidden(doc, 3))
            self.assertFalse(is_row_hidden(doc, 4))

    def test_filter_multiple_rows(self):
        with self.ui_test.load_file(get_url_for_data_file("autofilter.ods")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")

            xTreeList = xCheckListMenu.getChild("check_list_box")
            xFirstEntry = xTreeList.getChild("1")

            xFirstEntry.executeAction("CLICK", tuple())

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertFalse(is_row_hidden(doc, 1))
            self.assertTrue(is_row_hidden(doc, 2))
            self.assertTrue(is_row_hidden(doc, 3))
            self.assertFalse(is_row_hidden(doc, 4))

    def test_tdf133160(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf133160.ods")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "3"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            size1 = int(get_state_as_dict(xTreeList)["Size"].split('x')[0])
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "3"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            size2 = int(get_state_as_dict(xTreeList)["Size"].split('x')[0])
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "3", "ROW": "3"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            size3 = int(get_state_as_dict(xTreeList)["Size"].split('x')[0])
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "4", "ROW": "3"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            size4 = int(get_state_as_dict(xTreeList)["Size"].split('x')[0])

            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertTrue(size1 < size2) # for me they were size1=176 size2=212 size3=459 size4=1012
            self.assertTrue(size2 < size3) # size1 is the minimum window width,  size2 based on its column width
            self.assertTrue(size3 < size4) # size3 is a long text width
            self.assertTrue(size4 < 1500)  # size4 is the maximum window width with a really long text

    def test_tdf134351(self):
        with self.ui_test.load_file(get_url_for_data_file("autofilter.ods")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")

            xTreeList = xCheckListMenu.getChild("check_tree_box")

            self.assertEqual(2, len(xTreeList.getChildren()))
            self.assertTrue(get_state_as_dict(xTreeList.getChild('0'))['IsSelected'])
            self.assertTrue(get_state_as_dict(xTreeList.getChild('1'))['IsSelected'])

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertFalse(is_row_hidden(doc, 0))
            # Without the fix in place, this test would have failed here
            self.assertFalse(is_row_hidden(doc, 1))
            self.assertFalse(is_row_hidden(doc, 2))
            self.assertFalse(is_row_hidden(doc, 3))
            self.assertFalse(is_row_hidden(doc, 4))

    def test_differentSearches(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            calcDoc = self.xUITest.getTopFocusWindow()

            xGridWindow = calcDoc.getChild("grid_window")
            enter_text_to_cell(xGridWindow, "A1", "X")
            enter_text_to_cell(xGridWindow, "A2", "11")
            enter_text_to_cell(xGridWindow, "A3", "22")
            xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A3"}))

            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

            xFloatWindow = self.xUITest.getFloatWindow()

            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")

            xList = xCheckListMenu.getChild("check_list_box")

            self.assertEqual(2, len(xList.getChildren()))
            self.assertEqual("11", get_state_as_dict(xList.getChild('0'))['Text'])
            self.assertEqual("22", get_state_as_dict(xList.getChild('1'))['Text'])

            xSearchEdit = xFloatWindow.getChild("search_edit")
            xSearchEdit.executeAction("TYPE", mkPropertyValues({"TEXT" : "11"}))

            self.assertEqual(1, len(xList.getChildren()))
            self.assertEqual("11", get_state_as_dict(xList.getChild('0'))['Text'])

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertFalse(is_row_hidden(document, 0))
            self.assertFalse(is_row_hidden(document, 1))
            self.assertTrue(is_row_hidden(document, 2))

            xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

            xFloatWindow = self.xUITest.getFloatWindow()

            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")

            xList = xCheckListMenu.getChild("check_list_box")

            self.assertEqual(2, len(xList.getChildren()))
            self.assertEqual("11", get_state_as_dict(xList.getChild('0'))['Text'])
            self.assertEqual("22", get_state_as_dict(xList.getChild('1'))['Text'])

            xSearchEdit = xFloatWindow.getChild("search_edit")
            xSearchEdit.executeAction("TYPE", mkPropertyValues({"TEXT" : "22"}))

            self.assertEqual(1, len(xList.getChildren()))
            self.assertEqual("22", get_state_as_dict(xList.getChild('0'))['Text'])

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())


            self.assertFalse(is_row_hidden(document, 0))
            self.assertTrue(is_row_hidden(document, 1))
            self.assertFalse(is_row_hidden(document, 2))


    def test_tdf89244(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "AAA")
            enter_text_to_cell(gridwin, "A3", "BBB")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))

            self.xUITest.executeCommand(".uno:MergeCells")

            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

            xFloatWindow = self.xUITest.getFloatWindow()

            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")

            xList = xCheckListMenu.getChild("check_list_box")

            self.assertEqual(2, len(xList.getChildren()))
            self.assertEqual("(empty)", get_state_as_dict(xList.getChild('0'))['Text'])
            self.assertEqual("BBB", get_state_as_dict(xList.getChild('1'))['Text'])

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())


    def test_tdf116818(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf116818.xlsx")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_tree_box")
            self.assertEqual(3, len(xTreeList.getChildren()))
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(5, len(xTreeList.getChildren()))
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(3, len(xTreeList.getChildren()))
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

    def test_tdf140469(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf140469.xlsx")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(9, len(xTreeList.getChildren()))
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

    def test_tdf140462(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf140462.ods")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_tree_box")
            self.assertEqual(3, len(xTreeList.getChildren()))
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(3, len(xTreeList.getChildren()))
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(4, len(xTreeList.getChildren()))
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

    def test_tdf137626(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf137626.xlsx")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(3, len(xTreeList.getChildren()))
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

    def test_time_value(self):
        with self.ui_test.load_file(get_url_for_data_file("time_value.xlsx")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(5, len(xTreeList.getChildren()))
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('0'))['IsChecked'])
            self.assertEqual('false', get_state_as_dict(xTreeList.getChild('1'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('2'))['IsChecked'])
            self.assertEqual('false', get_state_as_dict(xTreeList.getChild('3'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('4'))['IsChecked'])
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(2, len(xTreeList.getChildren()))
            xOkBtn = xFloatWindow.getChild("cancel")
            xOkBtn.executeAction("CLICK", tuple())

    def test_tdf140968(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf140968.xlsx")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(5, len(xTreeList.getChildren()))
            self.assertEqual("0.000", get_state_as_dict(xTreeList.getChild('0'))['Text'])
            self.assertEqual("0.046", get_state_as_dict(xTreeList.getChild('1'))['Text'])
            self.assertEqual("0.365", get_state_as_dict(xTreeList.getChild('2'))['Text'])
            self.assertEqual("0.500", get_state_as_dict(xTreeList.getChild('3'))['Text'])
            self.assertEqual("0.516", get_state_as_dict(xTreeList.getChild('4'))['Text'])

            self.assertEqual('false', get_state_as_dict(xTreeList.getChild('0'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('1'))['IsChecked'])
            self.assertEqual('false', get_state_as_dict(xTreeList.getChild('2'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('3'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('4'))['IsChecked'])

            xFirstEntry = xTreeList.getChild("0")
            xFirstEntry.executeAction("CLICK", tuple())
            xFirstEntry = xTreeList.getChild("1")
            xFirstEntry.executeAction("CLICK", tuple())

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertTrue(is_row_hidden(doc, 1))
            self.assertTrue(is_row_hidden(doc, 2))
            self.assertTrue(is_row_hidden(doc, 3))
            self.assertTrue(is_row_hidden(doc, 4))
            self.assertFalse(is_row_hidden(doc, 5))
            self.assertFalse(is_row_hidden(doc, 6))
            self.assertFalse(is_row_hidden(doc, 7))

    def test_tdf142350(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "A2", "0")
            enter_text_to_cell(gridwin, "A3", "")
            enter_text_to_cell(gridwin, "A4", "1")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))

            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xList = xCheckListMenu.getChild("check_list_box")

            # tdf140745 show (empty) entry on top of the checkbox list
            self.assertEqual(3, len(xList.getChildren()))
            self.assertEqual("(empty)", get_state_as_dict(xList.getChild('0'))['Text'])
            self.assertEqual("0", get_state_as_dict(xList.getChild('1'))['Text'])
            self.assertEqual("1", get_state_as_dict(xList.getChild('2'))['Text'])

            xEntry = xList.getChild("0")
            xEntry.executeAction("CLICK", tuple())

            xOkButton = xFloatWindow.getChild("ok")
            xOkButton.executeAction("CLICK", tuple())

            self.assertFalse(is_row_hidden(document, 1))
            self.assertTrue(is_row_hidden(document, 2))
            self.assertFalse(is_row_hidden(document, 3))

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(3, len(xList.getChildren()))
            self.assertEqual('false', get_state_as_dict(xList.getChild('0'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xList.getChild('1'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xList.getChild('2'))['IsChecked'])
            xCloseButton = xFloatWindow.getChild("cancel")
            xCloseButton.executeAction("CLICK", tuple())


    def test_tdf138438(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf138438.ods")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            # Top 10 filter
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
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

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertTrue(is_row_hidden(doc, 1))
            self.assertTrue(is_row_hidden(doc, 2))
            self.assertFalse(is_row_hidden(doc, 3))
            self.assertFalse(is_row_hidden(doc, 4))
            self.assertFalse(is_row_hidden(doc, 5))
            self.assertFalse(is_row_hidden(doc, 6))
            self.assertTrue(is_row_hidden(doc, 7))
            self.assertFalse(is_row_hidden(doc, 8))

    def test_tdf142402(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf140968.xlsx")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B8"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xval1 = xDialog.getChild("val1")

                select_by_text(xval1, "0.365")


            self.assertFalse(is_row_hidden(doc, 0))
            self.assertFalse(is_row_hidden(doc, 1))
            self.assertTrue(is_row_hidden(doc, 2))
            self.assertTrue(is_row_hidden(doc, 3))
            self.assertTrue(is_row_hidden(doc, 4))
            self.assertTrue(is_row_hidden(doc, 5))
            self.assertTrue(is_row_hidden(doc, 6))

    def test_tdf142910(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf140968.xlsx")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            #Test '<' condition
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B8"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:

                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")

                select_by_text(xfield1, "Values")
                select_by_text(xcond1, "<")
                select_by_text(xval1, "0.365")


            self.assertFalse(is_row_hidden(doc, 0))
            self.assertTrue(is_row_hidden(doc, 1))
            self.assertFalse(is_row_hidden(doc, 2))
            self.assertFalse(is_row_hidden(doc, 3))
            self.assertFalse(is_row_hidden(doc, 4))
            self.assertFalse(is_row_hidden(doc, 5))
            self.assertTrue(is_row_hidden(doc, 6))
            self.assertTrue(is_row_hidden(doc, 7))

            #Test '>=' condition
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B8"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")

                select_by_text(xfield1, "Values")
                select_by_text(xcond1, ">=")
                select_by_text(xval1, "0.046")


            self.assertFalse(is_row_hidden(doc, 0))
            self.assertFalse(is_row_hidden(doc, 1))
            self.assertFalse(is_row_hidden(doc, 2))
            self.assertFalse(is_row_hidden(doc, 3))
            self.assertFalse(is_row_hidden(doc, 4))
            self.assertTrue(is_row_hidden(doc, 5))
            self.assertFalse(is_row_hidden(doc, 6))
            self.assertFalse(is_row_hidden(doc, 7))

    def test_tdf144253(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf144253.ods")) as doc:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "4", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(2, len(xTreeList.getChildren()))
            self.assertEqual("65.43", get_state_as_dict(xTreeList.getChild('0'))['Text'])
            self.assertEqual("83.33", get_state_as_dict(xTreeList.getChild('1'))['Text'])

            xFirstEntry = xTreeList.getChild("1")
            xFirstEntry.executeAction("CLICK", tuple())

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            self.assertFalse(is_row_hidden(doc, 0))
            self.assertTrue(is_row_hidden(doc, 1))
            self.assertFalse(is_row_hidden(doc, 2))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
