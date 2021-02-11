# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class Tdf104026(UITestCase):

  def execute_conditional_format_manager_dialog(self, nCount):

    self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog")
    xCondFormatMgr = self.xUITest.getTopFocusWindow()

    aExpectedResults = ['A2\tCell value != $Sheet1.$B2', 'A3\tCell value != $Sheet1.$B3',
        'A4\tCell value != $Sheet1.$B4', 'A5\tCell value != $Sheet1.$B5',
        'A6\tCell value != $Sheet1.$B6', 'A7\tCell value != $Sheet1.$B7']

    xList = xCondFormatMgr.getChild("CONTAINER")
    self.assertEqual(nCount, len(xList.getChildren()))

    for i in range(nCount):
        self.assertEqual(aExpectedResults[i], get_state_as_dict(xList.getChild(str(i)))['Text'])

    xOKBtn = xCondFormatMgr.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

  def test_tdf104026(self):
    self.ui_test.load_file(get_url_for_data_file("tdf104026.ods"))

    self.execute_conditional_format_manager_dialog(6)

    xCalcDoc = self.xUITest.getTopFocusWindow()
    gridwin = xCalcDoc.getChild("grid_window")
    gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))

    self.xUITest.executeCommand(".uno:DeleteRows")

    # Without the fix in place, this test would have failed with
    # AssertionError: 'A2\tCell value != $Sheet1.$B2' != 'A2\tCell value != $Sheet1.$B#REF!'
    self.execute_conditional_format_manager_dialog(5)

    self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
