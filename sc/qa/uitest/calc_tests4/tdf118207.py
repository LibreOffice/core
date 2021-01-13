# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 118207 - CRASH: Cutting and pasting a conditional format column and undoing

class tdf118207(UITestCase):
    def test_tdf118207(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf118189.xlsx"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Make sure that tools-options-LibreOffice Calc-General-Input settings-Show overwrite warning when pasting data is NOT tagged.
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xCalcEntry = xPages.getChild('3')                 # Calc
        xCalcEntry.executeAction("EXPAND", tuple())
        xCalcGeneralEntry = xCalcEntry.getChild('0')
        xCalcGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
        xreplwarncb = xDialogOpt.getChild("replwarncb")
        if (get_state_as_dict(xreplwarncb)["Selected"]) == "true":
            xreplwarncb.executeAction("CLICK", tuple())
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Select Column A
        #Cut it
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:SelectColumn")
        self.xUITest.executeCommand(".uno:Cut")
        #Paste it in Column B , Paste it in Column B again
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
        self.xUITest.executeCommand(".uno:Paste")
        self.xUITest.executeCommand(".uno:Paste")
        #Undo 3 times
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Undo")
        #-> CRASH
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString() , "On Back Order")
        #verify

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: