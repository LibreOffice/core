#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import time
from uitest.uihelper.common import get_state_as_dict, type_text

class tdf93068(UITestCase):

    def test_tdf93068(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xFontDlg = self.xUITest.getTopFocusWindow()
        xOKBtn = xFontDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xFontDlg = self.xUITest.getTopFocusWindow()
        xCancBtn = xFontDlg.getChild("cancel")
        xCancBtn.executeAction("CLICK", tuple())

        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xFontDlg = self.xUITest.getTopFocusWindow()
        xDiscardBtn = xFontDlg.getChild("reset")
        xDiscardBtn.executeAction("CLICK", tuple())
        xOKBtn = xFontDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()
