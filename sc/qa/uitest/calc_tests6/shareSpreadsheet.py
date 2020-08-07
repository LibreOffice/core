# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

#just open dialog, select Share, check name and cancel [save not allowed in uitest]

class shareSpreadsheet(UITestCase):

   def test_share_spreadsheet(self):
        self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #dialog shareSpreadsheet
        self.ui_test.execute_dialog_through_command(".uno:ShareDocument")
        xDialog = self.xUITest.getTopFocusWindow()

        share = xDialog.getChild("share")
        share.executeAction("CLICK", tuple())
        users = xDialog.getChild("users")
        self.assertEqual(get_state_as_dict(users)["Children"], "1")
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
