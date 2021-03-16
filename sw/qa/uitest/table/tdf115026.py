# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict

class tdf115026(UITestCase):

    def test_pageBreak_and_tableAutoFormat(self):
        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.xUITest.executeCommand(".uno:InsertPagebreak")
        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")

        self.ui_test.execute_dialog_through_command(".uno:InsertTable")
        xDialog = self.xUITest.getTopFocusWindow()

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.execute_dialog_through_command(".uno:AutoFormat")
        xDialog = self.xUITest.getTopFocusWindow()

        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        self.assertEqual(get_state_as_dict(xWriterEdit)["CurrentPage"], "2")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
