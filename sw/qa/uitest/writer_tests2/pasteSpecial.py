# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text

class PasteSpecial(UITestCase):

   def test_pasteSpecial(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        type_text(xWriterEdit, "test")

        for i in range(5):
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:Copy")

            self.ui_test.execute_dialog_through_command(".uno:PasteSpecial")
            xDialog = self.xUITest.getTopFocusWindow()

            xList = xDialog.getChild('list')
            xChild = xList.getChild(str(i))

            xChild.executeAction("SELECT", tuple())

            xOkBtn = xDialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOkBtn)

            self.xUITest.executeCommand(".uno:Undo")

            self.assertEqual(document.Text.String, "test")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
