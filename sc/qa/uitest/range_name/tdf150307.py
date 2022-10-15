# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text

class tdf150307(UITestCase):

    def check_navigator(self, xGridWin, nLen):
        xGridWin.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "ScNavigatorPanel"}))

        xCalcDoc = self.xUITest.getTopFocusWindow()
        xNavigatorPanel = xCalcDoc.getChild("NavigatorPanel")
        xContentBox = xNavigatorPanel.getChild('contentbox')
        xSheets = xContentBox.getChild("0")
        self.assertEqual('Sheets', get_state_as_dict(xSheets)['Text'])

        self.ui_test.wait_until_property_is_updated(xSheets, "Children", str(nLen))
        self.assertEqual(nLen, len(xSheets.getChildren()))
        self.assertEqual('Sheet1', get_state_as_dict(xSheets.getChild('0'))['Text'])

        if nLen == 2:
            self.assertEqual('Sheet1_2', get_state_as_dict(xSheets.getChild('1'))['Text'])

        xRangeNames = xContentBox.getChild("1")
        self.assertEqual('Range names', get_state_as_dict(xRangeNames)['Text'])

        self.ui_test.wait_until_property_is_updated(xRangeNames, "Children", str(nLen))
        self.assertEqual(nLen, len(xRangeNames.getChildren()))
        self.assertEqual('Test', get_state_as_dict(xRangeNames.getChild('0'))['Text'])

        if nLen == 2:
            self.assertEqual('Test (Sheet1_2)', get_state_as_dict(xRangeNames.getChild('1'))['Text'])

    def test_tdf150307(self):

        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = xCalcDoc.getChild("grid_window")

            self.xUITest.executeCommand(".uno:Sidebar")

            with self.ui_test.execute_modeless_dialog_through_command(".uno:AddName", close_button="add") as xAddNameDlg:
                xEdit = xAddNameDlg.getChild("edit")
                type_text(xEdit, "Test")

            self.check_navigator(xGridWin, 1)

            self.xUITest.executeCommand(".uno:DuplicateSheet")

            self.check_navigator(xGridWin, 2)

            with self.ui_test.execute_dialog_through_command(".uno:Remove", close_button="yes"):
                pass

            self.check_navigator(xGridWin, 1)

            self.xUITest.executeCommand(".uno:DuplicateSheet")

            self.check_navigator(xGridWin, 2)

            # Now test tdf#150829
            self.xUITest.executeCommand(".uno:Undo")

            self.check_navigator(xGridWin, 1)

            self.xUITest.executeCommand(".uno:Sidebar")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
