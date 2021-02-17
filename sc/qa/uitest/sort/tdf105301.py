# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf105301(UITestCase):

    def test_tdf105301(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf105301.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:R9"}))

        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        self.assertEqual("Column B", get_state_as_dict(xDialog.getChild("sortlb"))['DisplayText'])
        self.assertEqual("Column C", get_state_as_dict(xDialog.getChild("sortlb2"))['DisplayText'])
        self.assertEqual("Column D", get_state_as_dict(xDialog.getChild("sortlb3"))['DisplayText'])

        # Without the fix in place, this test would have failed with
        # AssertionError: 'Column E' != '- undefined -'
        self.assertEqual("Column E", get_state_as_dict(xDialog.getChild("sortlb4"))['DisplayText'])
        self.assertEqual("Column F", get_state_as_dict(xDialog.getChild("sortlb5"))['DisplayText'])
        self.assertEqual("Column G", get_state_as_dict(xDialog.getChild("sortlb6"))['DisplayText'])
        self.assertEqual("- undefined -", get_state_as_dict(xDialog.getChild("sortlb7"))['DisplayText'])

        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
