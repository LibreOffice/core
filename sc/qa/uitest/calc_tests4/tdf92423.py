# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 92423 - EDITING Text to columns... does not respect currently selected cells

class tdf92423(UITestCase):
    def test_tdf92423_text_to_columns(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #Make sure that tools-options-StarOffice Calc-General-Input settings-Show overwrite warning when pasting data is tagged.
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')                 # Calc
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
        xreplwarncb = xDialogOpt.getChild("replwarncb")
        if (get_state_as_dict(xreplwarncb)["Selected"]) == "false":
            xreplwarncb.executeAction("CLICK", tuple())
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #enter data
        enter_text_to_cell(gridwin, "A1", "1;2")
        enter_text_to_cell(gridwin, "A2", "2;3")
        enter_text_to_cell(gridwin, "A3", "3;4")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A3"}))
        #copy data
        self.xUITest.executeCommand(".uno:Copy")
        #move down
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoDown")
        #paste data, should be selected
        self.xUITest.executeCommand(".uno:Paste") #A7:A9
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["MarkedArea"], "Sheet1.A7:Sheet1.A9")
        # Data - Text to Columns
        self.ui_test.execute_dialog_through_command(".uno:TextToColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        xSemicolon = xDialog.getChild("semicolon")  #check semicolon checkbox
        if (get_state_as_dict(xSemicolon)["Selected"]) == "false":
            xSemicolon.executeAction("CLICK", tuple())
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 6).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 0, 7).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 8).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 1, 6).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 7).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 1, 8).getValue(), 4)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
