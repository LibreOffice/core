# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class protectSheet(UITestCase):
    def test_protect_sheet(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #enter password - lock
        self.ui_test.execute_dialog_through_command(".uno:Protect")
        xDialog = self.xUITest.getTopFocusWindow()
        xprotect = xDialog.getChild("protect")
        xpassword1 = xDialog.getChild("password1")
        xpassword2 = xDialog.getChild("password2")

        if (get_state_as_dict(xprotect)["Selected"]) == "false":
            xprotect.executeAction("CLICK", tuple())
        xpassword1.executeAction("TYPE", mkPropertyValues({"TEXT":"aa"}))
        xpassword2.executeAction("TYPE", mkPropertyValues({"TEXT":"aa"}))

        xOKBtn = xDialog.getChild("ok")
#        self.ui_test.close_dialog_through_button(xOKBtn)
        xOKBtn.executeAction("CLICK", tuple())
        #Unlock

        self.ui_test.execute_dialog_through_command(".uno:Protect")
        xDialog = self.xUITest.getTopFocusWindow()
        xpass1ed = xDialog.getChild("pass1ed")

        xpass1ed.executeAction("TYPE", mkPropertyValues({"TEXT":"aa"}))

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify - the sheet is unlocked
        enter_text_to_cell(gridwin, "B2", "A")
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "A")

        # test cancel button
        self.ui_test.execute_dialog_through_command(".uno:Protect")
        xDialog = self.xUITest.getTopFocusWindow()
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)
        enter_text_to_cell(gridwin, "B2", "B")
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "B")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
