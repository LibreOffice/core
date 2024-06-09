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

class tdf161457(UITestCase):

    def test_tdf161457(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf161457.ods")) as calc_doc:

            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = xCalcDoc.getChild("grid_window")

            # Click the autofilter dropdown in column B
            xGridWin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")

            self.assertEqual(2, len(xTreeList.getChildren()))

            xEntry1 = xTreeList.getChild(0)
            self.assertEqual("1.00", get_state_as_dict(xEntry1)['Text'])
            self.assertEqual("true", get_state_as_dict(xEntry1)['IsChecked'])
            self.assertEqual("false", get_state_as_dict(xEntry1)['IsSemiTransparent'])

            xEntry2 = xTreeList.getChild(1)
            self.assertEqual("5.00", get_state_as_dict(xEntry2)['Text'])
            self.assertEqual("true", get_state_as_dict(xEntry2)['IsChecked'])
            # Without the fix in place, this test would have failed with
            # AssertionError: 'true' != 'false'
            self.assertEqual("false", get_state_as_dict(xEntry2)['IsSemiTransparent'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
