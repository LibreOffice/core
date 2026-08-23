# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell

class tdf137617(UITestCase):

    GRAMMAR = '/org.openoffice.Office.Calc/Formula/Syntax/Grammar'
    # the values of the configuration item
    CALC_A1 = 0
    EXCEL_R1C1 = 2

    def test_tdf137617(self):

        # The document takes the syntax when it is created, see
        # ScDocShell::InitOptions. The item has no default, so name the syntax
        # to go back to explicitly, otherwise the following tests are left with
        # Excel R1C1.
        with self.ui_test.set_config(self.GRAMMAR, self.CALC_A1), \
                self.ui_test.set_config(self.GRAMMAR, self.EXCEL_R1C1), \
                self.ui_test.create_doc_in_start_center("calc"):

            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "Result1")
            enter_text_to_cell(gridwin, "A2", "Result2")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B2"}))

            with self.ui_test.execute_dialog_through_command(".uno:CreateNames") as xDialog:


                # Only left is selected
                self.assertEqual('true', get_state_as_dict(xDialog.getChild('left'))['Selected'])
                self.assertEqual('false', get_state_as_dict(xDialog.getChild('right'))['Selected'])
                self.assertEqual('false', get_state_as_dict(xDialog.getChild('bottom'))['Selected'])
                self.assertEqual('false', get_state_as_dict(xDialog.getChild('top'))['Selected'])

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))

            xPosWindow = calcDoc.getChild('pos_window')
            self.assertEqual('Result1', get_state_as_dict(xPosWindow)['Text'])

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B2"}))

            self.assertEqual('Result2', get_state_as_dict(xPosWindow)['Text'])


            enter_text_to_cell(gridwin, "C1", "Result3")
            enter_text_to_cell(gridwin, "D1", "Result4")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C1:D2"}))

            with self.ui_test.execute_dialog_through_command(".uno:CreateNames") as xDialog:


                # Only top is selected
                self.assertEqual('false', get_state_as_dict(xDialog.getChild('left'))['Selected'])
                self.assertEqual('false', get_state_as_dict(xDialog.getChild('right'))['Selected'])
                self.assertEqual('false', get_state_as_dict(xDialog.getChild('bottom'))['Selected'])
                self.assertEqual('true', get_state_as_dict(xDialog.getChild('top'))['Selected'])

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C2"}))

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Result3' != 'R2C3'
            self.assertEqual('Result3', get_state_as_dict(xPosWindow)['Text'])

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D2"}))

            self.assertEqual('Result4', get_state_as_dict(xPosWindow)['Text'])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
