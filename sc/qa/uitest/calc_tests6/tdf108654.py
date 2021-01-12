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

#Bug 108654 - CRASH while undoing paste on a new sheet
class tdf108654(UITestCase):
    def test_tdf108654_Undo_paste_crash(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf108654.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Sheet3 - CTRL+A & CTRL+C
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Copy")
        #Create a new sheet & paste (CTRL-V)
        self.ui_test.execute_dialog_through_command(".uno:Insert")  #insert sheet
        xDialog = self.xUITest.getTopFocusWindow()
        xOKButton = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKButton)

        self.assertEqual(document.Sheets.getCount(), 4)

        self.xUITest.executeCommand(".uno:Paste")
        #Undo the action twice
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Undo")

        #-> CRASH
        self.assertEqual(document.Sheets.getCount(), 3)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
