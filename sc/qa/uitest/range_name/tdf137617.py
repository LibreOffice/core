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
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_by_text
from uitest.uihelper.calc import enter_text_to_cell

class tdf137617(UITestCase):

    def test_tdf137617(self):

        with self.ui_test.create_doc_in_start_center("calc"):

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

            # Change formula syntax to "Excel R1C1"
            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialogOpt:

                xPages = xDialogOpt.getChild("pages")
                xCalcEntry = xPages.getChild('3')
                xCalcEntry.executeAction("EXPAND", tuple())
                xCalcFormulaEntry = xCalcEntry.getChild('4')
                xCalcFormulaEntry.executeAction("SELECT", tuple())

                xFormulaSyntax = xDialogOpt.getChild('formulasyntax')
                select_by_text(xFormulaSyntax, "Excel R1C1")


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

            # Change formula syntax back to "Calc A1"
            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialogOpt:

                xPages = xDialogOpt.getChild("pages")
                xCalcEntry = xPages.getChild('3')
                xCalcEntry.executeAction("EXPAND", tuple())
                xCalcFormulaEntry = xCalcEntry.getChild('4')
                xCalcFormulaEntry.executeAction("SELECT", tuple())

                xFormulaSyntax = xDialogOpt.getChild('formulasyntax')
                select_by_text(xFormulaSyntax, "Calc A1")



# vim: set shiftwidth=4 softtabstop=4 expandtab:
