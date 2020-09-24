# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.path import get_srcdir_url

from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_row

import time

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

    def test_tdf134351(self):
        doc = self.ui_test.load_file(get_url_for_data_file("autofilter.ods"))

        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))

        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")

        xTreeList = xCheckListMenu.getChild("check_list_box")

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

# vim: set shiftwidth=4 softtabstop=4 expandtab:
