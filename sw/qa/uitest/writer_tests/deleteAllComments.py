#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import time
from uitest.uihelper.common import get_state_as_dict, type_text

class DeleteAllComments(UITestCase):

    def test_comments_delete(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")
        

        type_text(xWriterEdit, "Test LibreOffice")

        document = self.ui_test.get_component()

        selection = self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:InsertAnnotation")
        cursor = document.getCurrentController().getViewCursor()
        xWriterDoc.executeAction("LEFT", tuple())
        type_text(xWriterEdit, "EEEEE")
        self.xUITest.executeCommand(".uno:InsertAnnotation")
        self.xUITest.executeCommand(".uno:DeleteAllNotes")
        self.assertEqual(document.Text.String[0:4], "Test")

        self.ui_test.close_doc()

    def test_comment_trackchanges(self):
#tdf111524
        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        type_text(xWriterEdit, "foo")

        document = self.ui_test.get_component()

        selection = self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:InsertAnnotation")
        self.xUITest.executeCommand(".uno:TrackChanges")
        self.xUITest.executeCommand(".uno:DeleteAllNotes")



        self.ui_test.close_doc()
