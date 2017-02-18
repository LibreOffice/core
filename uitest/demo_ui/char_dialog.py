# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase

from uitest.debug import sleep

class CharDialogText(UITestCase):

    def test_select_char(self):
        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_dialog_through_command(".uno:InsertSymbol")
        xCharDialog = self.xUITest.getTopFocusWindow()
        print(xCharDialog.getChildren())

        sleep(5)

        xCharSet = xCharDialog.getChild("showcharset")

        xCharSet.executeAction("SELECT", mkPropertyValues({"COLUMN": "2", "ROW": "2"}))

        sleep(5)

        xCancelBtn = xCharDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
