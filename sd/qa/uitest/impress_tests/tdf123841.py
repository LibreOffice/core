#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos

class tdf123841(UITestCase):

    def test_tdf123841(self):

        self.ui_test.create_doc_in_start_center("draw")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.execute_dialog_through_command(".uno:Sidebar")

        xSidebarDlg = self.xUITest.getTopFocusWindow()
        rectangle = xPageSetupDlg.getChild("rectangle")

        rectangle.executeAction("CLICK", tuple())

        # Without the patch in place, this test would have failed with
        # AssertionError: '' != '0x00729fcf'
        document = self.ui_test.get_component()
        self.assertEqual(
          hex(document.DrawPages.getByIndex(0).Background.FillGradient.StartColor), '0x00729fcf')

# vim: set shiftwidth=4 softtabstop=4 expandtab:
