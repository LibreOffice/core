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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 125051 - EDITING: CRASH when start SpellCheck

class tdf125051(UITestCase):
    def test_tdf125051_crash_spelling_dialog(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "text")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.ui_test.execute_dialog_through_command(".uno:SpellDialog")
        xDialog = self.xUITest.getTopFocusWindow()  #Spelling dialog

        xDialog2 = self.xUITest.getTopFocusWindow()   # info dialog "The spellcheck of this sheet has been completed.-> OK"
        okBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(okBtn)

        xDialog = self.xUITest.getTopFocusWindow()  #Spelling dialog
        #open options
        optionsBtn = xDialog.getChild("options")
        def handle_options_dlg(dialog):
            #print(dialog.getChildren())
            xCancelBtn = dialog.getChild("cancel")
            self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.execute_blocking_action(optionsBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_options_dlg)

        closeBtn = xDialog.getChild("close")    #close Spelling dialog
        self.ui_test.close_dialog_through_button(closeBtn)

        #verify, we didn't crash
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "text")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
