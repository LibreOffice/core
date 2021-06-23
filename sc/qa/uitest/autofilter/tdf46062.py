# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict

class tdf46062(UITestCase):

    def test_tdf46062(self):

        self.ui_test.create_doc_in_start_center("calc")
        calcDoc = self.xUITest.getTopFocusWindow()
        gridwin = calcDoc.getChild("grid_window")

        enter_text_to_cell(gridwin, "A1", "spaces")
        enter_text_to_cell(gridwin, "A2", " ")
        enter_text_to_cell(gridwin, "A3", "  ")
        enter_text_to_cell(gridwin, "A4", "   ")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))

        self.ui_test.execute_dialog_through_command(".uno:DataFilterAutoFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xYesBtn = xDialog.getChild("yes")
        self.ui_test.close_dialog_through_button(xYesBtn)

        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xList = xCheckListMenu.getChild("check_list_box")
        self.assertEqual(3, len(xList.getChildren()))

        # Without the fix in place, this test would have failed with
        # AssertionError: '" "' != ' '
        self.assertEqual('" "', get_state_as_dict(xList.getChild("0"))['Text'])
        self.assertEqual('"  "', get_state_as_dict(xList.getChild("1"))['Text'])
        self.assertEqual('"   "', get_state_as_dict(xList.getChild("2"))['Text'])

        xOkButton = xFloatWindow.getChild("ok")
        xOkButton.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
