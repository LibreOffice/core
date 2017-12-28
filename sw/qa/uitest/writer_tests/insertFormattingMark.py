#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
import time
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep

class insertFormattingMark(UITestCase):

    def test_insert_formatting_mark(self):

        self.ui_test.create_doc_in_start_center("writer")

        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        type_text(xWriterEdit, "Insert Formatting Mark ")

        document = self.ui_test.get_component()

        self.xUITest.executeCommand(".uno:InsertNonBreakingSpace")
        type_text(xWriterEdit, "Insert Formatting Mark ")
        self.xUITest.executeCommand(".uno:InsertHardHyphen")
        type_text(xWriterEdit, "Insert Formatting Mark ")
        self.xUITest.executeCommand(".uno:InsertSoftHyphen")
        type_text(xWriterEdit, "Insert Formatting Mark ")
        self.xUITest.executeCommand(".uno:InsertZWSP")
        type_text(xWriterEdit, "Insert Formatting Mark ")
        self.xUITest.executeCommand(".uno:InsertZWNBSP")
        type_text(xWriterEdit, "Insert Formatting Mark ")

        self.xUITest.executeCommand(".uno:InsertLRM")
        type_text(xWriterEdit, "Insert Formatting Mark ")

        self.xUITest.executeCommand(".uno:InsertRLM")
        type_text(xWriterEdit, "Insert Formatting Mark ")




        self.ui_test.close_doc()
