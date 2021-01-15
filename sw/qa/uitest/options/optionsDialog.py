# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase

class optionsDialog(UITestCase):

    def test_moreIconsDialog(self):
        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")

        xDialog = self.xUITest.getTopFocusWindow()
        xPages = xDialog.getChild("pages")
        xLOEntry = xPages.getChild('0')
        xLOEntry.executeAction("EXPAND", tuple())
        xViewEntry = xLOEntry.getChild('2')
        xViewEntry.executeAction("SELECT", tuple())

        xMoreIconsBtn = xDialog.getChild("btnMoreIcons")

        def handle_more_icons_dlg(dialog):
            # Check it doesn't crash while opening it
            xCloseBtn = dialog.getChild("buttonClose")
            self.ui_test.wait_until_property_is_updated(xCloseBtn, "Enabled", "true")
            self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.execute_blocking_action(xMoreIconsBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_more_icons_dlg)

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

    def test_tdf138596(self):
        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xPages = xDialog.getChild("pages")
        xWriterEntry = xPages.getChild('3')
        xWriterEntry.executeAction("EXPAND", tuple())
        xFormattingAidsEntry = xWriterEntry.getChild('2')
        xFormattingAidsEntry.executeAction("SELECT", tuple())

        xApplyBtn = xDialog.getChild("apply")

        # Click apply button twice
        # Without the fix in place, this test would have crashed here
        xApplyBtn.executeAction("CLICK", tuple())
        xApplyBtn.executeAction("CLICK", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
