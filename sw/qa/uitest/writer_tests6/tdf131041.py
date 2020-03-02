#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_pos, get_state_as_dict

class tdf131041(UITestCase):

    def test_run(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()

        self.ui_test.execute_dialog_through_command(".uno:PageDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        tabcontrol = xDialog.getChild("tabcontrol")
        select_pos(tabcontrol, 4) #header

        xHeaderOn = xDialog.getChild("checkHeaderOn")
        xMoreBtn = xDialog.getChild("buttonMore")

        self.assertEqual(get_state_as_dict(xHeaderOn)["Selected"], "false")
        self.assertEqual(get_state_as_dict(xMoreBtn)["Enabled"], "false")

        xHeaderOn.executeAction("CLICK", tuple())

        self.assertEqual(get_state_as_dict(xHeaderOn)["Selected"], "true")
        self.assertEqual(get_state_as_dict(xMoreBtn)["Enabled"], "true")

        self.ui_test.execute_dialog_through_action(xMoreBtn, "CLICK")

        xBorderDlg = self.xUITest.getTopFocusWindow()

        #modify any property
        bottomft = xBorderDlg.getChild("bottommf")
        bottomft.executeAction("UP", tuple())

        #it would crash here
        okBtn = xBorderDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(okBtn)

        xDialog = self.xUITest.getTopFocusWindow()
        okBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(okBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
