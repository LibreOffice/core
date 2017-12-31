#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep

class WriterInsertHorizontalLine(UITestCase):

    def test_insert_horizontal_line(self):
        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        type_text(xWriterEdit, "Test horizontal line")     #write the text

        self.xUITest.executeCommand(".uno:StyleApply?Style:string=Horizontal%20Line&FamilyName:string=ParagraphStyles") #insert horizontal line

        self.xUITest.executeCommand(".uno:Undo")
        self.xUITest.executeCommand(".uno:Redo")



        self.ui_test.close_doc()

