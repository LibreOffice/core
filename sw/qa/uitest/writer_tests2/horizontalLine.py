# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, type_text

class WriterInsertHorizontalLine(UITestCase):

    def test_insert_horizontal_line(self):
        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        type_text(xWriterEdit, "Test horizontal line")     #write the text

        self.xUITest.executeCommand(".uno:StyleApply?Style:string=Horizontal%20Line&FamilyName:string=ParagraphStyles") #insert horizontal line

        self.ui_test.execute_dialog_through_command(".uno:EditStyle")  #open style dialog
        xDialog = self.xUITest.getTopFocusWindow()
        xStyleNametxt = xDialog.getChild("name")
        self.assertEqual(get_state_as_dict(xStyleNametxt)["Text"], "Horizontal Line") #check style name
        xCancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancBtn)

        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Redo")

        self.ui_test.execute_dialog_through_command(".uno:EditStyle")  #open style dialog
        xDialog = self.xUITest.getTopFocusWindow()
        xStyleNametxt = xDialog.getChild("name")
        self.assertEqual(get_state_as_dict(xStyleNametxt)["Text"], "Horizontal Line")  #check style name
        xCancBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
