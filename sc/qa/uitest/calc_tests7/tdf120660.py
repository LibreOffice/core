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
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 120660 - cells not recalculated after copy/paste different range of cells

class tdf120660(UITestCase):
    def test_tdf120660_undo_recalculate(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf120660.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #Make sure that tools-options-LibreOffice Calc-General-Input settings-Show overwrite warning when pasting data is unselected
        #turn off message: You are pasting data into cells that already contain data
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xWriterEntry = xPages.getChild('3')                 # Calc
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterGeneralEntry = xWriterEntry.getChild('0')
        xWriterGeneralEntry.executeAction("SELECT", tuple())          #General / replwarncb
        xreplwarncb = xDialogOpt.getChild("replwarncb")
        if (get_state_as_dict(xreplwarncb)["Selected"]) == "true":
            xreplwarncb.executeAction("CLICK", tuple())
        xOKBtn = xDialogOpt.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A8:E8"}))
        self.xUITest.executeCommand(".uno:Copy")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A4:E4"}))
        self.xUITest.executeCommand(".uno:Paste")
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 1200)

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 2200)

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A8:D8"}))
        self.xUITest.executeCommand(".uno:Copy")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A4:D4"}))
        self.xUITest.executeCommand(".uno:Paste")
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 1200)

        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 2200)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab: