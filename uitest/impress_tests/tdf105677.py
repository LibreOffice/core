#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos

class tdf105677(UITestCase):

    def test_background_dialog(self):

        self.ui_test.create_doc_in_start_center("impress")

        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.execute_dialog_through_command(".uno:PageSetup")

        xPageSetupDlg = self.xUITest.getTopFocusWindow()
        tabcontrol = xPageSetupDlg.getChild("tabcontrol")
        select_pos(tabcontrol, "1")

        xColorBtn = xPageSetupDlg.getChild("btncolor")
        xColorBtn.executeAction("CLICK", tuple())

        xOkBtn = xPageSetupDlg.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        xConfirmDlg = self.xUITest.getTopFocusWindow()

        xYesBtn = xConfirmDlg.getChild("no")
        xYesBtn.executeAction("CLICK", tuple())

        document = self.ui_test.get_component()

        self.assertEqual(document.DrawPages.getByIndex(0).Background.FillColor, 7512015)

        #Somehow we need to use undo twice in order to revert the background
        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Undo")

        document = self.ui_test.get_component()
        self.assertEqual(document.DrawPages.getByIndex(0).Background, None)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
