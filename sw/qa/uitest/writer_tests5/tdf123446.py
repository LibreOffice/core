# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import type_text
#Bug 123446 - Writer crashes after undoing + redoing ToC insertion in middle of word

class tdf123446(UITestCase):

   def test_tsd123446_undo_redo_ToC_crash(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        #- Add a word to an empty document.
        type_text(xWriterEdit, "LibreOffice")
        #- Change its style to Heading 2.
        self.xUITest.executeCommand(".uno:StyleApply?Style:string=Heading%202&FamilyName:string=ParagraphStyles")
        #- Position cursor somewhere in the middle of the word, and add Table of Contents
        #(no need to change anything in the dialog).
        self.xUITest.executeCommand(".uno:GoLeft")
        self.xUITest.executeCommand(".uno:GoLeft")
        self.xUITest.executeCommand(".uno:GoLeft")
        self.xUITest.executeCommand(".uno:GoLeft")

        self.ui_test.execute_dialog_through_command(".uno:InsertMultiIndex")
        xDialog = self.xUITest.getTopFocusWindow()
        xokbtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xokbtn)
        #- Undo the ToC insertion.
        self.xUITest.executeCommand(".uno:Undo")
        #- Redo the ToC insertion.
        self.xUITest.executeCommand(".uno:Redo")
        #=> Crash.  Now we verify the text
        # This second undo crash in Clang build https://bugs.documentfoundation.org/show_bug.cgi?id=123313#c9
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.Text.String[0:7], "LibreOf")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
