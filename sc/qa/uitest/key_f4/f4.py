# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
import os
from uitest.uihelper.common import get_state_as_dict
from uitest.debug import sleep
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.calc import enter_text_to_cell

class keyF4(UITestCase):
    def test_f4(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #enter data
        enter_text_to_cell(gridwin, "A1", "1")
        enter_text_to_cell(gridwin, "A2", "2")
        enter_text_to_cell(gridwin, "A3", "3")
        enter_text_to_cell(gridwin, "B1", "=A1")
        enter_text_to_cell(gridwin, "B2", "=A2")
        enter_text_to_cell(gridwin, "B3", "=A3")
        #select B1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
        #F4
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getFormula(), "=$A$1")
        #F4
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getFormula(), "=A$1")
        #F4
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getFormula(), "=$A1")

        #non continuous select
        #enter data
        enter_text_to_cell(gridwin, "C1", "=A1")
        enter_text_to_cell(gridwin, "C2", "=A2")
        enter_text_to_cell(gridwin, "C3", "=A3")

        #select C1 and C3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C3", "EXTEND":"1"}))
        #F4
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getFormula(), "=$A$1")
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getFormula(), "=$A$3")
        #F4
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getFormula(), "=A$1")
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getFormula(), "=A$3")
        #F4
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getFormula(), "=$A1")
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getFormula(), "=$A3")

        self.ui_test.close_doc()

#    def test_tdf39650_F4_R1C1(self):
#        #Bug 39650 - Shift+F4 conversion from relative to absolute does not work for R1C1 syntax
#        calc_doc = self.ui_test.create_doc_in_start_center("calc")
#        xCalcDoc = self.xUITest.getTopFocusWindow()
#        gridwin = xCalcDoc.getChild("grid_window")
#        document = self.ui_test.get_component()
#        #* Tools --> Options --> Calc --> Formula -->  Syntax = Excel R1C1
#        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
#        xDialogOpt = self.xUITest.getTopFocusWindow()

#        xPages = xDialogOpt.getChild("pages")
#        xCalcEntry = xPages.getChild('3')                 # Calc
#        xCalcEntry.executeAction("EXPAND", tuple())
#        xCalcFormulaEntry = xCalcEntry.getChild('4')
#        xCalcFormulaEntry.executeAction("SELECT", tuple())          #Formula

#        formulasyntax = xDialogOpt.getChild("formulasyntax")
#        #Excel R1C1
#        props = {"TEXT": "Excel R1C1"}
#        actionProps = mkPropertyValues(props)
#        formulasyntax.executeAction("SELECT", actionProps)

#        xOKBtn = xDialogOpt.getChild("ok")
#        self.ui_test.close_dialog_through_button(xOKBtn)

#        #In cell R3C3 enter "xxx".
#        enter_text_to_cell(gridwin, "C3", "xxx")
#        # In cell R2C2 type = and then click on the xxx in R3C3.
#        enter_text_to_cell(gridwin, "B2", "=R[1]C[1]")
#        # With cell R2C2 still selected, type Shift-F4.
#        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B2"}))
#        sleep(5)
#        #F4
#        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "F4"}))
#        sleep(5)
#        #This should change the formula in R2C2 from =R[1]C[1] to =R3C3.
#        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "xxx")
#        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getFormula(), "=R3C3") -> fails here AssertionError: '=$C$3' != '=R3C3'

#        #Give it back Tools --> Options --> Calc --> Formula -->  Syntax = Calc A1
#        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
#        xDialogOpt = self.xUITest.getTopFocusWindow()

#        xPages = xDialogOpt.getChild("pages")
#        xCalcEntry = xPages.getChild('3')                 # Calc
#        xCalcEntry.executeAction("EXPAND", tuple())
#        xCalcFormulaEntry = xCalcEntry.getChild('4')
#        xCalcFormulaEntry.executeAction("SELECT", tuple())          #Formula

#        formulasyntax = xDialogOpt.getChild("formulasyntax")
#        #Excel R1C1
#        props = {"TEXT": "Calc A1"}
#        actionProps = mkPropertyValues(props)
#        formulasyntax.executeAction("SELECT", actionProps)

#        xOKBtn = xDialogOpt.getChild("ok")
#        self.ui_test.close_dialog_through_button(xOKBtn)
#        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
