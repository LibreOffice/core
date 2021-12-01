# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
#Bug 113252 - Basic Library Organizer is broken and closing dialogs crashes

class tdf113252(UITestCase):

   def test_tdf113252_macro_dialog(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            #Start LibreOffice. Go to Tools > Macros > Organize Macros > Basic
            with self.ui_test.execute_dialog_through_command(".uno:MacroDialog", close_button="close") as xDialog:

                #Click Button Organizer
                xorganize = xDialog.getChild("organize")
                with self.ui_test.execute_blocking_action(xorganize.executeAction, args=('CLICK', ()), close_button="close") as dialog:
                    xTabs = dialog.getChild("tabcontrol")
                    select_pos(xTabs, "0")
                    select_pos(xTabs, "1")
                    select_pos(xTabs, "2")
                    #Click button Close in the next dialog -> crash.


            self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "1")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
