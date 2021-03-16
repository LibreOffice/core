# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text

from uitest.uihelper.common import select_pos

class tdf78068(UITestCase):

   def test_tdf78068_format_paragraph_crash(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #- add some text
        type_text(xWriterEdit, "Test")
        #- go to Format > Paragraph
        self.ui_test.execute_dialog_through_command(".uno:ParagraphDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xOK = xDialog.getChild("ok")
        xOK.executeAction("CLICK", tuple()) #close dialog

        self.assertEqual(document.Text.String[0:4], "Test")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
