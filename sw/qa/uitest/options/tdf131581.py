# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class tdf131581(UITestCase):

    def test_tdf131581(self):
        self.ui_test.create_doc_in_start_center("writer")

        self.xUITest.getTopFocusWindow()

        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
        xDialogOpt = self.xUITest.getTopFocusWindow()

        xPages = xDialogOpt.getChild("pages")
        xLOEntry = xPages.getChild('0')
        xLOEntry.executeAction("EXPAND", tuple())
        xAdvancedEntry = xLOEntry.getChild('10')
        xAdvancedEntry.executeAction("SELECT", tuple())

        xExpertBtn = xDialogOpt.getChild("expertconfig")

        def handle_expert_dlg(dialog):
            # Without the fix in place, this would have hung
            xSearchBtn = dialog.getChild("searchButton")
            xSearchBtn.executeAction("CLICK", tuple())

            xOkBtn = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.execute_blocking_action(xExpertBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_expert_dlg)

        xOKBtn = xDialogOpt.getChild("ok")
        xOKBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
