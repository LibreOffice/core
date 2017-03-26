# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.path import get_srcdir_url

from libreoffice.uno.propertyvalue import mkPropertyValues

import time

def get_url_for_data_file(file_name):
    return get_srcdir_url() + "/uitest/calc_tests/data/" + file_name

class AutofilterTest(UITestCase):

    def test_launch_autofilter(self):
        doc = self.ui_test.load_file(get_url_for_data_file("autofilter.ods"))

        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
        xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "1"}))

        time.sleep(1)

        self.ui_test.close_doc()

    def test_hierarchy(self):
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

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
