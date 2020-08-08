# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.path import get_srcdir_url

from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_row


def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/sc/qa/uitest/autofilter/data/" + file_name

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

        xTreeList = xCheckListMenu.getChild("check_list_box")
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

# vim: set shiftwidth=4 softtabstop=4 expandtab:
