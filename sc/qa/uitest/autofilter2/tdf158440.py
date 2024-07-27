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

class tdf158440(UITestCase):

    def test_tdf158440(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf158440.ods")):

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xAll = xFloatWindow.getChild("toggle_all")
            xAll.executeAction("CLICK", tuple())

            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(529, len(xList.getChildren()))

            xFirstEntry = xList.getChild("1")
            xFirstEntry.executeAction("CLICK", tuple())

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))

            self.assertEqual("111", get_state_as_dict(gridwin)["CurrentRow"])
            self.assertEqual("0", get_state_as_dict(gridwin)["CurrentColumn"])

            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))

            # Without the fix in place, this test would have failed with
            # AssertionError: '535' != '111'
            self.assertEqual("535", get_state_as_dict(gridwin)["CurrentRow"])
            self.assertEqual("0", get_state_as_dict(gridwin)["CurrentColumn"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
