# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos

class tdf122045(UITestCase):

    def test_tdf122045(self):

        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()

        self.ui_test.execute_dialog_through_command(".uno:PageDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "2")

        btncolor = xDialog.getChild("btncolor")
        btncolor.executeAction("CLICK", tuple())

        xApplyBtn = xDialog.getChild("apply")
        xApplyBtn.executeAction("CLICK", tuple())

        self.assertTrue(document.isModified())
        self.assertEqual("0x729fcf", hex(document.StyleFamilies.PageStyles.Standard.BackColor))

        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.assertTrue(document.isModified())
        self.assertEqual("0x729fcf", hex(document.StyleFamilies.PageStyles.Standard.BackColor))

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
