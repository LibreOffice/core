# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase

class tdf116070(UITestCase):

    def test_tdf116070(self):
        self.ui_test.create_doc_in_start_center("writer")

        self.xUITest.executeCommand(".uno:InsertPagebreak")

        document = self.ui_test.get_component()

        self.assertEqual(document.CurrentController.PageCount, 2)

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        self.ui_test.wait_until_child_is_available(xWriterEdit, 'PageBreak')
        xPageBreak = xWriterEdit.getChild('PageBreak')

        self.ui_test.execute_dialog_through_action(xPageBreak, "EDIT")

        xDialog = self.xUITest.getTopFocusWindow()

        # Without the fix in place, this test would have crashed here
        okBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(okBtn)

        self.assertEqual(document.CurrentController.PageCount, 2)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
