#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class WriterInsertBreakDialog(UITestCase):

    def test_insert(self):

        self.ui_test.create_doc_in_start_center("writer")

        xDialog = self.launch_dialog("linerb")
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        xDialog = self.launch_dialog("columnrb")
        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        for i in range(11):
            xDialog = self.launch_dialog("pagerb")

            xStyleList = xDialog.getChild("stylelb")
            xStyleList.executeAction("SELECT", mkPropertyValues({"POS": str(i)}))

            xOkBtn = xDialog.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def launch_dialog(self, child):

        self.ui_test.execute_dialog_through_command(".uno:InsertBreak")
        xInsertBreakDlg = self.xUITest.getTopFocusWindow()

        xObject = xInsertBreakDlg.getChild(child)
        xObject.executeAction("CLICK", tuple())

        return xInsertBreakDlg

# vim: set shiftwidth=4 softtabstop=4 expandtab:
