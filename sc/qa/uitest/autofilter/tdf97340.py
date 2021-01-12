# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 97340 - Calc crashes on filtering with select checkbox with space bar

class tdf97340(UITestCase):
    def test_tdf97340_autofilter(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autofilter.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()

        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xTreeList = xCheckListMenu.getChild("check_tree_box")
        self.assertEqual(2, len(xTreeList.getChildren()))
        self.assertEqual("2016", get_state_as_dict(xTreeList.getChild('0'))['Text'])
        self.assertEqual("2017", get_state_as_dict(xTreeList.getChild('1'))['Text'])

        xsearchEdit = xFloatWindow.getChild("search_edit")
        xsearchEdit.executeAction("TYPE", mkPropertyValues({"TEXT":" "}))
        self.assertEqual(0, len(xTreeList.getChildren()))

        xsearchEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))

        #tdf#133785, without the fix in place, it would have been 0
        self.assertEqual(2, len(xTreeList.getChildren()))
        self.assertEqual("2016", get_state_as_dict(xTreeList.getChild('0'))['Text'])
        self.assertEqual("2017", get_state_as_dict(xTreeList.getChild('1'))['Text'])

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
