# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
#Bug 113252 - Basic Library Organizer is broken and closing dialogs crashes

class tdf113252(UITestCase):

   def test_tdf113252_macro_dialog(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        #Start LibreOffice. Go to Tools > Macros > Organize Macros > Basic
        self.ui_test.execute_dialog_through_command(".uno:MacroDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        #Click Button Organizer
        xorganize = xDialog.getChild("organize")
        def handle_macro_dlg(dialog):

            xTabs = dialog.getChild("tabcontrol")
            select_pos(xTabs, "0")
            select_pos(xTabs, "1")
            select_pos(xTabs, "2")
            #Click on its button Close
            xCloseBtn = dialog.getChild("close")
            xCloseBtn.executeAction("CLICK", tuple())

        self.ui_test.execute_blocking_action(xorganize.executeAction, args=('CLICK', ()),
                dialog_handler=handle_macro_dlg)
        #Click button Close in the next dialog -> crash.

        xClose = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xClose)

        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "1")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
