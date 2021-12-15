# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_by_text
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf119178(UITestCase):

    def test_tdf119178(self):

        with self.ui_test.create_doc_in_start_center("calc") as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A15", "test")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:ConditionalFormatDialog", close_button="") as xCondFormatDlg:

                xRange = xCondFormatDlg.getChild("edassign")
                xRange.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xRange.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xRange.executeAction("TYPE", mkPropertyValues({"TEXT": "$B$15"}))

                xType = xCondFormatDlg.getChild("type")
                select_by_text(xType, "Formula is")

                # After changing the type, the dialog has been recalculated
                xCondFormatDlg = self.xUITest.getTopFocusWindow()

                xStyle = xCondFormatDlg.getChild("style")
                select_by_text(xStyle, "Error")

                xFormula = xCondFormatDlg.getChild("formula")
                xFormula.executeAction("TYPE", mkPropertyValues({"TEXT": "$A15 = \"test\""}))

                xOkBtn = xCondFormatDlg.getChild("ok")
                self.ui_test.close_dialog_through_button(xOkBtn)

            # Check the conditional format is correctly displayed in the manager
            with self.ui_test.execute_dialog_through_command(".uno:ConditionalFormatManagerDialog") as xCondFormatMgr:
                aExpectedResult = 'B15\tFormula is $A15 = "test"'
                xList = xCondFormatMgr.getChild("CONTAINER")
                self.assertEqual(1, len(xList.getChildren()))

                # Without the fix in place, this test would have failed with
                # AssertionError: 'B15\tFormula is $A15 = "test"' != 'B15\tFormula is $A29 = "test"'
                self.assertEqual(aExpectedResult, get_state_as_dict(xList.getChild('0'))['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
