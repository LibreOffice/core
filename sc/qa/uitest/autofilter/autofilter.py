# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_row
from uitest.uihelper.calc import enter_text_to_cell

def is_row_hidden(doc, index):
    row = get_row(doc, index)
    val = row.getPropertyValue("IsVisible")
    return not val

class AutofilterTest(UITestCase):

    def test_tdf106214(self):
        doc = self.ui_test.load_file(get_url_for_data_file("autofilter.ods"))

        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")

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

        self.ui_test.close_doc()

    def test_filter_multiple_rows(self):
        doc = self.ui_test.load_file(get_url_for_data_file("autofilter.ods"))

        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")

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
        doc = self.ui_test.load_file(get_url_for_data_file("tdf133160.ods"))

        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "3"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")
        size1 = int(get_state_as_dict(xTreeList)["Size"].split('x')[0])
        xOkBtn = xFloatWindow.getChild("cancel")
        xOkBtn.executeAction("CLICK", tuple())

        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "3"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")
        size2 = int(get_state_as_dict(xTreeList)["Size"].split('x')[0])
        xOkBtn = xFloatWindow.getChild("cancel")
        xOkBtn.executeAction("CLICK", tuple())

        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "3", "ROW": "3"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")
        size3 = int(get_state_as_dict(xTreeList)["Size"].split('x')[0])
        xOkBtn = xFloatWindow.getChild("cancel")
        xOkBtn.executeAction("CLICK", tuple())

        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "4", "ROW": "3"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")
        size4 = int(get_state_as_dict(xTreeList)["Size"].split('x')[0])

        xOkBtn = xFloatWindow.getChild("cancel")
        xOkBtn.executeAction("CLICK", tuple())

        self.assertTrue(size1 < size2) # for me they were size1=176 size2=212 size3=459 size4=1012
        self.assertTrue(size2 < size3) # size1 is the minimum window width,  size2 based on its column width
        self.assertTrue(size3 < size4) # size3 is a long text width
        self.assertTrue(size4 < 1500)  # size4 is the maximum window width with a really long text

        self.ui_test.close_doc()

    def test_tdf134351(self):
        doc = self.ui_test.load_file(get_url_for_data_file("autofilter.ods"))

        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")

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

        self.ui_test.close_doc()

    def test_differentSearches(self):
        self.ui_test.create_doc_in_start_center("calc")
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()

        xGridWindow = calcDoc.getChild("grid_window")
        enter_text_to_cell(xGridWindow, "A1", "X")
        enter_text_to_cell(xGridWindow, "A2", "11")
        enter_text_to_cell(xGridWindow, "A3", "22")
        xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A3"}))

        self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

        xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

        xFloatWindow = self.xUITest.getFloatWindow()

        xCheckListMenu = xFloatWindow.getChild("check_list_menu")

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

        xCheckListMenu = xFloatWindow.getChild("check_list_menu")

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

        self.ui_test.close_doc()

    def test_tdf89244(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        enter_text_to_cell(gridwin, "A1", "AAA")
        enter_text_to_cell(gridwin, "A3", "BBB")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))

        self.xUITest.executeCommand(".uno:MergeCells")

        self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

        xFloatWindow = self.xUITest.getFloatWindow()

        xCheckListMenu = xFloatWindow.getChild("check_list_menu")

        xList = xCheckListMenu.getChild("check_list_box")

        self.assertEqual(2, len(xList.getChildren()))
        self.assertEqual("(empty)", get_state_as_dict(xList.getChild('0'))['Text'])
        self.assertEqual("BBB", get_state_as_dict(xList.getChild('1'))['Text'])

        xOkBtn = xFloatWindow.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_tdf116818(self):
        doc = self.ui_test.load_file(get_url_for_data_file("tdf116818.xlsx"))

        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_tree_box")
        self.assertEqual(3, len(xTreeList.getChildren()))
        xOkBtn = xFloatWindow.getChild("cancel")
        xOkBtn.executeAction("CLICK", tuple())

        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")
        self.assertEqual(5, len(xTreeList.getChildren()))
        xOkBtn = xFloatWindow.getChild("cancel")
        xOkBtn.executeAction("CLICK", tuple())

        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")
        self.assertEqual(3, len(xTreeList.getChildren()))
        xOkBtn = xFloatWindow.getChild("cancel")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_tdf140469(self):
        doc = self.ui_test.load_file(get_url_for_data_file("tdf140469.xlsx"))

        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_list_box")
        self.assertEqual(9, len(xTreeList.getChildren()))
        xOkBtn = xFloatWindow.getChild("cancel")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
