# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell

class tdf83126(UITestCase):

   def test_tdf132097(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A4", "Test")
            enter_text_to_cell(gridwin, "D1", "Test")
            enter_text_to_cell(gridwin, "D4", "Test")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                xRows = xDialog.getChild('rows')
                self.assertEqual('true', get_state_as_dict(xRows)['Checked'])

                xSearchTerm = xDialog.getChild("searchterm")

                xSearchTerm.executeAction("TYPE", mkPropertyValues({"TEXT":"Test"}))

                xSearchall = xDialog.getChild("searchall")
                with self.ui_test.execute_dialog_through_action(
                        xSearchall, "CLICK", event_name = "ModelessDialogVisible", close_button="close") as xResultsDialog:
                    xResults = xResultsDialog.getChild("results")
                    self.assertEqual(3, len(xResults.getChildren()))

                    # Without the fix in place, this test would have failed with
                    # AssertionError: 'Sheet1\t$D$1\tTest' != 'Sheet1\t$A$4\tTest'
                    self.assertEqual("Sheet1\t$D$1\tTest", get_state_as_dict(xResults.getChild('0'))['Text'])
                    self.assertEqual("Sheet1\t$A$4\tTest", get_state_as_dict(xResults.getChild('1'))['Text'])
                    self.assertEqual("Sheet1\t$D$4\tTest", get_state_as_dict(xResults.getChild('2'))['Text'])

                xCols = xDialog.getChild('cols')
                xCols.executeAction("CLICK", tuple())

                with self.ui_test.execute_dialog_through_action(
                        xSearchall, "CLICK", event_name = "ModelessDialogVisible", close_button="close") as xResultsDialog:
                    xResults = xResultsDialog.getChild("results")
                    self.assertEqual(3, len(xResults.getChildren()))
                    self.assertEqual("Sheet1\t$A$4\tTest", get_state_as_dict(xResults.getChild('0'))['Text'])
                    self.assertEqual("Sheet1\t$D$1\tTest", get_state_as_dict(xResults.getChild('1'))['Text'])
                    self.assertEqual("Sheet1\t$D$4\tTest", get_state_as_dict(xResults.getChild('2'))['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
